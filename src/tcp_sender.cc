#include "tcp_sender.hh"
#include "tcp_config.hh"

#include <random>

using namespace std;

/* TCPSender constructor (uses a random ISN if none given) */
TCPSender::TCPSender( uint64_t initial_RTO_ms, optional<Wrap32> fixed_isn )
  : isn_( fixed_isn.value_or( Wrap32 { random_device()() } ) ), initial_RTO_ms_( initial_RTO_ms ), pushMap(), outStandingMap(), seqno_(0), rto(initial_RTO_ms)
{}

uint64_t TCPSender::sequence_numbers_in_flight() const
{
  if (outStandingMap.empty() && pushMap.empty()) {
    return 0;
  }
  uint64_t outSum = 0;
  if (!outStandingMap.empty()) {
    uint64_t lastKey = outStandingMap.rbegin()->first;
    uint64_t lastSeq = outStandingMap.rbegin()->second.sequence_length();
    uint64_t firstKey = outStandingMap.begin()->first;
    outSum += lastKey + lastSeq - firstKey;
  }
  if (!pushMap.empty()) {
    uint64_t lastKey = pushMap.rbegin()->first;
    uint64_t lastSeq = pushMap.rbegin()->second.sequence_length();
    uint64_t firstKey = pushMap.begin()->first;
    outSum += lastKey + lastSeq - firstKey;
  }
  return outSum;
}

uint64_t TCPSender::consecutive_retransmissions() const
{
  return consecutiveRetransmissions;
}

optional<TCPSenderMessage> TCPSender::maybe_send()
{
  if (!pushMap.empty()) {
    if (!timerRunning) {
      timerRunning = true;
      totalTime = 0;
    }
    auto first_pair = std::move(*pushMap.begin());
    pushMap.erase(pushMap.begin());
    outStandingMap.insert(first_pair);
    return first_pair.second;
  }
  return {};
}

void TCPSender::push( Reader& outbound_stream )
{
  // Wrap32 seqno = isn_ + outbound_stream.bytes_popped();
  bool syn = false;
  bool fin = false;
  Buffer payload;
  TCPSenderMessage toPush;

  if (seqno_ == 0) {
    syn = true;
    toPush = {Wrap32::wrap(seqno_, isn_), syn, payload, fin};
    pushMap.insert({seqno_, toPush});
    seqno_++;
  }

  uint64_t size = max<uint64_t>(1, windowSize - sequence_numbers_in_flight());
  string my_buffer;
  while (outbound_stream.bytes_buffered() > 0 && my_buffer.length() < size) {
    uint64_t boundary = min(min(static_cast<uint64_t>(size), 
                                    static_cast<uint64_t>(outbound_stream.bytes_buffered())), 
                                    static_cast<uint64_t>(TCPConfig::MAX_PAYLOAD_SIZE));
    while (my_buffer.length() < boundary && outbound_stream.bytes_buffered() > 0) {
      string_view peeked = outbound_stream.peek();
      if (peeked.length() > boundary - my_buffer.length()) {
        peeked = peeked.substr(0, boundary - my_buffer.length());
      }
      my_buffer += ((std::string(peeked)));
      outbound_stream.pop(peeked.length());
    }
    toPush = {Wrap32::wrap(seqno_, isn_), false, Buffer(my_buffer), false};
    pushMap.insert({seqno_, toPush});
    seqno_ += my_buffer.length();
    
  }
  
  // NEED TO ADD TO PUSH MAP

  if (outbound_stream.is_finished() && seqno_ == outbound_stream.bytes_popped() + 1 
  && windowSize - sequence_numbers_in_flight() > 0) {
    fin = true;
    syn = false;
    Buffer emptyPayload = Buffer();
    toPush = {Wrap32::wrap(seqno_, isn_), syn, emptyPayload, fin};
    pushMap.insert({seqno_, toPush});
    seqno_++;  
  }


  // If the stream is finished send an empty message with the FIN, for Syn, just incrememnt the seqno
  // the syn flag message itself should not have one added to it, but stuff after should

}

TCPSenderMessage TCPSender::send_empty_message() const
{
  return {Wrap32::wrap(seqno_, isn_), false, Buffer(), false};
}

void TCPSender::receive( const TCPReceiverMessage& msg )
{
  windowSize = msg.window_size;
  if (msg.ackno.has_value()) {
    lastAcked = msg.ackno.value().unwrap(isn_, seqno_);
    outStandingMap.erase(outStandingMap.begin(), outStandingMap.lower_bound(lastAcked));

    totalTime = 0;
    consecutiveRetransmissions = 0;
    rto = initial_RTO_ms_;
  }
}

void TCPSender::tick( const size_t ms_since_last_tick )
{
  totalTime += ms_since_last_tick;
  if (totalTime >= rto) {
    if (!outStandingMap.empty()) {
      auto first_pair = std::move(*outStandingMap.begin());
      outStandingMap.erase(outStandingMap.begin());
      pushMap.insert(first_pair);
      totalTime = 0;
      if (windowSize > 0) {
        rto *= 2;
        consecutiveRetransmissions++;
      }
    } 
  }
}
