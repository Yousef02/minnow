#include "tcp_sender.hh"
#include "tcp_config.hh"

#include <random>

using namespace std;

/* TCPSender constructor (uses a random ISN if none given) */
TCPSender::TCPSender( uint64_t initial_RTO_ms, optional<Wrap32> fixed_isn )
  : isn_( fixed_isn.value_or( Wrap32 { random_device()() } ) ), initial_RTO_ms_( initial_RTO_ms ), pushMap(), outStandingMap(), seqno_(0)
{}

uint64_t TCPSender::sequence_numbers_in_flight() const
{
  if (outStandingMap.empty()) {
    return 0;
  }
  uint64_t lastKey = outStandingMap.rbegin()->first;
  uint64_t lastSeq = outStandingMap.rbegin()->second.sequence_length();
  uint64_t firstKey = outStandingMap.begin()->first;
  return lastKey + lastSeq - firstKey;
}

uint64_t TCPSender::consecutive_retransmissions() const
{
  // Your code here.
  return {};
}

optional<TCPSenderMessage> TCPSender::maybe_send()
{
  if (!pushMap.empty()) {
    auto first_pair = std::move(*pushMap.begin());
    pushMap.erase(pushMap.begin());
    outStandingMap.insert(first_pair);
    return first_pair.second;
  }
  return {};
}

void TCPSender::push( Reader& outbound_stream )
{
  Wrap32 seqno = isn_ + outbound_stream.bytes_popped();
  bool syn = false;
  bool fin = false;
  Buffer payload;
  TCPSenderMessage toPush;

  if (seqno_ == 0) {
    syn = true;
    toPush = {seqno.wrap(seqno_, isn_), syn, payload, fin};
    pushMap.insert({seqno_, toPush});
    seqno_++;
  }

  string my_buffer;
  uint64_t size = max<uint64_t>(1, windowSize);

  while (outbound_stream.bytes_buffered() > 0 && windowSize > 0) {
  uint64_t boundary = min(min(static_cast<uint64_t>(size), 
                                static_cast<uint64_t>(outbound_stream.bytes_buffered())), 
                                static_cast<uint64_t>(TCPConfig::MAX_PAYLOAD_SIZE));
    string_view peeked = outbound_stream.peek();
    if (peeked.length() > boundary) {
      peeked = peeked.substr(0, boundary);
    }
    my_buffer += ((std::string(peeked)));
    outbound_stream.pop(peeked.length());
    seqno_ += peeked.length();
  }

  payload = Buffer(my_buffer);
  toPush = {seqno, syn, payload, fin};
  // NEED TO ADD TO PUSH MAP

  if (outbound_stream.is_finished() && seqno_ == outbound_stream.bytes_popped() + 1) {
    fin = true;
    syn = false;
    Buffer emptyPayload = Buffer();
    toPush = {seqno, syn, emptyPayload, fin};
    seqno_++;  
  }


  // If the stream is finished send an empty message with the FIN, for Syn, just incrememnt the seqno
  // the syn flag message itself should not have one added to it, but stuff after should

}

TCPSenderMessage TCPSender::send_empty_message() const
{
  // Your code here.
  return {Wrap32{3}.wrap(seqno_, isn_), false, Buffer(), false};
}

void TCPSender::receive( const TCPReceiverMessage& msg )
{
  windowSize = msg.window_size;
  if (msg.ackno.has_value()) {
    lastAcked = msg.ackno.value().unwrap(isn_, seqno_);
    outStandingMap.erase(outStandingMap.begin(), outStandingMap.lower_bound(lastAcked));
  }
}

void TCPSender::tick( const size_t ms_since_last_tick )
{
  // Your code here.
  if (ms_since_last_tick > initial_RTO_ms_) {
    if (!outStandingMap.empty()) {
      auto first_pair = std::move(*outStandingMap.begin());
      outStandingMap.erase(outStandingMap.begin());
      pushMap.insert(first_pair);
    }
    initial_RTO_ms_ *= 2;
  }
}
