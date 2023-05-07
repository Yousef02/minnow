#include "tcp_sender.hh"
#include "tcp_config.hh"

#include <random>

using namespace std;

/* TCPSender constructor (uses a random ISN if none given) */
TCPSender::TCPSender( uint64_t initial_RTO_ms, optional<Wrap32> fixed_isn )
  : isn_( fixed_isn.value_or( Wrap32 { random_device()() } ) ), initial_RTO_ms_( initial_RTO_ms ), pushQueue(), outStanding()
{}

uint64_t TCPSender::sequence_numbers_in_flight() const
{
  // Your code here.
  return {};
}

uint64_t TCPSender::consecutive_retransmissions() const
{
  // Your code here.
  return {};
}

optional<TCPSenderMessage> TCPSender::maybe_send()
{
  // Your code here.
  return {};
}

void TCPSender::push( Reader& outbound_stream )
{
  Wrap32 seqno = isn_ + outbound_stream.bytes_popped();
  bool syn = false;
  bool fin = false;
  Buffer payload;
  TCPSenderMessage toPush;

  if (outbound_stream.bytes_popped() == 0) {
    syn = true;
  } 

  string my_buffer;
  while (outbound_stream.bytes_buffered() > 0 && windowSize > 0) {
    string_view peeked = outbound_stream.peek();
    my_buffer += ((std::string(peeked)));
    outbound_stream.pop(peeked.length());
  }

  payload = Buffer(my_buffer);
  toPush = {seqno, syn, payload, fin};

  if (outbound_stream.is_finished()) {
    fin = true;
    syn = false;
    Buffer emptyPayload = Buffer();
    toPush = {seqno, syn, emptyPayload, fin};
  }

  pushQueue.push(toPush);

  // If the stream is finished send an empty message with the FIN, for Syn, just incrememnt the seqno
  // the syn flag message itself should not have one added to it, but stuff after should

}

TCPSenderMessage TCPSender::send_empty_message() const
{
  // Your code here.
  return {};
}

void TCPSender::receive( const TCPReceiverMessage& msg )
{
  // Your code here.
  (void)msg;
}

void TCPSender::tick( const size_t ms_since_last_tick )
{
  // Your code here.
  (void)ms_since_last_tick;
}
