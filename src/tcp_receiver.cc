#include "tcp_receiver.hh"

using namespace std;

void TCPReceiver::receive( TCPSenderMessage message, Reassembler& reassembler, Writer& inbound_stream )
{
  uint64_t toinsert = 0;
  if ( message.SYN ) {
    isn = message.seqno;
  }
  if ( !isn.has_value() ) {
    return;
  } else {
    uint64_t abs_seqno = message.seqno.unwrap( isn.value(), inbound_stream.bytes_pushed() + 1 );
    if ( message.sequence_length() > 0 && message.SYN ) {
      toinsert = abs_seqno;
    } else {
      toinsert = abs_seqno - 1;
    }
    reassembler.insert( toinsert, message.payload.release(), message.FIN, inbound_stream );
  }
}

TCPReceiverMessage TCPReceiver::send( const Writer& inbound_stream ) const
{
  std::optional<Wrap32> ackno;
  if ( isn.has_value() && inbound_stream.is_closed() ) {
    ackno = Wrap32::wrap( inbound_stream.bytes_pushed() + 2, isn.value() );
  } else if ( isn.has_value() ) {
    ackno = Wrap32::wrap( inbound_stream.bytes_pushed() + 1, isn.value() );
  }
  uint16_t window_size = inbound_stream.available_capacity() >= UINT16_MAX ? UINT16_MAX
   : inbound_stream.available_capacity();
  return { ackno, window_size };
}
