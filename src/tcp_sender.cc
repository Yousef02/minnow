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
  uint64_t outVal = UINT64_MAX;
  uint64_t pushVal = UINT64_MAX;
  if (!outStandingMap.empty()) {
    outVal = outStandingMap.begin()->first;
  }
  if (!pushMap.empty()) {
    pushVal = pushMap.begin()->first;
  }
  uint64_t mini = min(outVal, pushVal);
  return (seqno_ - mini);
  // uint64_t outSum = 0;
  // if (!outStandingMap.empty()) {
  //   uint64_t lastKey = outStandingMap.rbegin()->first;
  //   uint64_t lastSeq = outStandingMap.rbegin()->second.sequence_length();
  //   uint64_t firstKey = outStandingMap.begin()->first;
  //   outSum += lastKey + lastSeq - firstKey;
  // }
  // if (!pushMap.empty()) {
  //   uint64_t lastKey = pushMap.rbegin()->first;
  //   uint64_t lastSeq = pushMap.rbegin()->second.sequence_length();
  //   uint64_t firstKey = pushMap.begin()->first;
  //   outSum += lastKey + lastSeq - firstKey;
  // }
  // return outSum;
  // uint64_t mini = min(outStandingMap.begin()->first, pushMap.begin()->first);
  // uint64_t maxi = max(outStandingMap.rbegin()->first + outStandingMap.rbegin()->second.sequence_length(), pushMap.rbegin()->first + pushMap.rbegin()->second.sequence_length());
  // return (maxi - mini);
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
      rto = initial_RTO_ms_;
      consecutiveRetransmissions = 0;
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
  // bool syn = false;
  Buffer payload;
  TCPSenderMessage toPush;
  // uint64_t localWindowSize = windowSize;


  uint64_t size = 0;
  if (windowSize > sequence_numbers_in_flight()) {
    size = windowSize - sequence_numbers_in_flight();
  } else {
    if (sequence_numbers_in_flight() > 0) {
      size = 0;
    } else {
      size = 1;
    }
  }


  // uint64_t size = max<uint64_t>(1, windowSize - sequence_numbers_in_flight());
  while ( size > 0 ) {

    uint64_t local_seqno = seqno_;

    if (seqno_ == 0) {
      seqno_++;
      size--;
    }

    string my_buffer;
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
    seqno_ += my_buffer.length();
    size -= my_buffer.length();


    bool fin = false;
    if (outbound_stream.is_finished() && seqno_ == outbound_stream.bytes_popped() + 1 
      && size > 0) {
        fin = true;
        size--;
        seqno_++;  
      }


    toPush = {Wrap32::wrap(local_seqno, isn_), local_seqno == 0, Buffer(my_buffer), fin};
    if (toPush.sequence_length() == 0) {
      break;
    }
    pushMap.insert({local_seqno, toPush});

    







  }
  
  // NEED TO ADD TO PUSH MAP

  


  // If the stream is finished send an empty message with the FIN, for Syn, just incrememnt the seqno
  // the syn flag message itself should not have one added to it, but stuff after should

}

TCPSenderMessage TCPSender::send_empty_message() const
{
  return {Wrap32::wrap(seqno_, isn_), false, Buffer(), false};
}

void TCPSender::receive( const TCPReceiverMessage& msg )
{
  uint64_t ackd = lastAcked;
  windowSize = msg.window_size;
  if (msg.ackno.has_value()) {
    if (msg.ackno.value().unwrap(isn_, seqno_) > seqno_ 
    || (!outStandingMap.empty() && msg.ackno.value().unwrap(isn_, seqno_) < outStandingMap.begin()->first + outStandingMap.begin()->second.sequence_length())) {
      return;
    }
    lastAcked = msg.ackno.value().unwrap(isn_, seqno_);
    outStandingMap.erase(outStandingMap.begin(), outStandingMap.lower_bound(lastAcked));
    if (lastAcked > ackd) {
      totalTime = 0;
      consecutiveRetransmissions = 0;
      rto = initial_RTO_ms_;
      timerRunning = false;
    }
  }
}

void TCPSender::tick( const size_t ms_since_last_tick )
{
  totalTime += ms_since_last_tick;
  timerRunning = true;
  if (totalTime >= rto) {
    if (!outStandingMap.empty()) {
      // auto first_pair = std::move(*outStandingMap.begin());
      pushMap.insert(*outStandingMap.begin());
      // outStandingMap.erase(outStandingMap.begin());
      
      totalTime = 0;
      if (windowSize > 0) {
        rto *= 2;
        consecutiveRetransmissions++;
      }
    } 
  }
}
