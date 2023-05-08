#pragma once

#include "byte_stream.hh"
#include "tcp_receiver_message.hh"
#include "tcp_sender_message.hh"
#include <queue>
#include <map>

class TCPSender
{
  Wrap32 isn_;
  uint64_t initial_RTO_ms_;

public:
  std::map<uint64_t, TCPSenderMessage> pushMap;
  std::map<uint64_t, TCPSenderMessage> outStandingMap;
  uint64_t lastAcked = 0;
  uint64_t windowSize = 1;
  uint64_t seqno_ = 0;
  uint64_t totalTime = 0;
  uint64_t rto;
  uint64_t consecutiveRetransmissions = 0;
  bool timerRunning = false;
  /* Construct TCP sender with given default Retransmission Timeout and possible ISN */
  TCPSender( uint64_t initial_RTO_ms, std::optional<Wrap32> fixed_isn );

  /* Push bytes from the outbound stream */
  void push( Reader& outbound_stream );

  /* Send a TCPSenderMessage if needed (or empty optional otherwise) */
  std::optional<TCPSenderMessage> maybe_send();

  /* Generate an empty TCPSenderMessage */
  TCPSenderMessage send_empty_message() const;

  /* Receive an act on a TCPReceiverMessage from the peer's receiver */
  void receive( const TCPReceiverMessage& msg );

  /* Time has passed by the given # of milliseconds since the last time the tick() method was called. */
  void tick( uint64_t ms_since_last_tick );

  /* Accessors for use in testing */
  uint64_t sequence_numbers_in_flight() const;  // How many sequence numbers are outstanding?
  uint64_t consecutive_retransmissions() const; // How many consecutive *re*transmissions have happened?
};
