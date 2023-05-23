#include "tcp_sender.hh"
#include "tcp_config.hh"

#include <random>

using namespace std;

/* TCPSender constructor (uses a random ISN if none given) */
TCPSender::TCPSender( uint64_t initial_RTO_ms, optional<Wrap32> fixed_isn )
  : isn_( fixed_isn.value_or( Wrap32 { random_device()() } ) )
  , initial_RTO_ms_( initial_RTO_ms )
  , pushMap()
  , outStandingMap()
  , seqno_( 0 )
  , rto( initial_RTO_ms )
{}

uint64_t TCPSender::sequence_numbers_in_flight() const
{
  // Return the number of total sequence numbers in flight (outstanding + pushed)
  if ( outStandingMap.empty() && pushMap.empty() ) {
    return 0;
  }
  return seqno_ - lastAcked;
}

uint64_t TCPSender::consecutive_retransmissions() const
{
  return consecutiveRetransmissions;
}

optional<TCPSenderMessage> TCPSender::maybe_send()
{
  if ( !pushMap.empty() ) {
    if ( !timerRunning ) {
      timerRunning = true;
      totalTime = 0;
      rto = initial_RTO_ms_;
      consecutiveRetransmissions = 0;
    }
    auto first_pair = std::move( *pushMap.begin() );
    pushMap.erase( pushMap.begin() );
    outStandingMap.insert( first_pair );
    return first_pair.second;
  }
  return {};
}

void TCPSender::push( Reader& outbound_stream )
{
  // Get the window space possible after substracting what is in flight.
  // Account for when the window size is 0, pretend like it is 1 to ping the receiver.
  uint64_t size = 0;
  if ( windowSize > sequence_numbers_in_flight() ) {
    size = windowSize - sequence_numbers_in_flight();
  } else {
    if ( sequence_numbers_in_flight() > 0 ) {
      size = 0;
    } else {
      size = 1;
    }
  }

  while ( size > 0 ) {

    Buffer payload;
    TCPSenderMessage toPush;
    uint64_t local_seqno = seqno_;

    if ( seqno_ == 0 ) { // SYN?
      seqno_++;
      size--;
    }

    string my_buffer;
    uint64_t boundary
      = min( min( static_cast<uint64_t>( size ), static_cast<uint64_t>( outbound_stream.bytes_buffered() ) ),
             static_cast<uint64_t>( TCPConfig::MAX_PAYLOAD_SIZE ) );

    // Inner while loop to make the message payload as large as possible.
    while ( my_buffer.length() < boundary ) {
      string_view peeked = outbound_stream.peek();
      if ( peeked.length() > boundary - my_buffer.length() ) {
        peeked = peeked.substr( 0, boundary - my_buffer.length() );
      }
      my_buffer += ( ( std::string( peeked ) ) );
      outbound_stream.pop( peeked.length() );
    }

    seqno_ += my_buffer.length();
    size -= my_buffer.length();

    // Space for FIN? Add it to the payload.
    bool fin = false;
    if ( outbound_stream.is_finished() && seqno_ == outbound_stream.bytes_popped() + 1 && size > 0 ) {
      fin = true;
      size--;
      seqno_++;
    }

    toPush = { Wrap32::wrap( local_seqno, isn_ ), local_seqno == 0, Buffer( my_buffer ), fin };
    // If for some reason the payload is empty, break out of the loop. ( it means there is size available but no
    // data to send)
    if ( toPush.sequence_length() == 0 ) {
      break;
    }
    pushMap.insert( { local_seqno, toPush } );
  }
}

TCPSenderMessage TCPSender::send_empty_message() const
{
  return { Wrap32::wrap( seqno_, isn_ ), false, Buffer(), false };
}

void TCPSender::receive( const TCPReceiverMessage& msg )
{
  windowSize = msg.window_size;
  if ( msg.ackno.has_value() ) {
    // If the ackno is greater than the seqno, it is invalid.
    // If the ackno is less than the last acked, it is invalid.
    if ( msg.ackno.value().unwrap( isn_, seqno_ ) > seqno_
         || ( !outStandingMap.empty()
              && msg.ackno.value().unwrap( isn_, seqno_ )
                   < outStandingMap.begin()->first + outStandingMap.begin()->second.sequence_length() ) ) {
      return;
    }
    lastAcked = msg.ackno.value().unwrap( isn_, seqno_ );
    outStandingMap.erase( outStandingMap.begin(), outStandingMap.lower_bound( lastAcked ) );

    // What's outsdanding is now acked, so reset the timer.
    totalTime = 0;
    consecutiveRetransmissions = 0;
    rto = initial_RTO_ms_;
    timerRunning = false;
  }
}

void TCPSender::tick( const size_t ms_since_last_tick )
{
  totalTime += ms_since_last_tick;
  timerRunning = true;
  if ( totalTime >= rto ) {
    if ( !outStandingMap.empty() ) {
      auto first_pair = std::move( *outStandingMap.begin() );
      outStandingMap.erase( outStandingMap.begin() );
      pushMap.insert( first_pair );
      totalTime = 0;
      if ( windowSize > 0 ) {
        rto *= 2;
        consecutiveRetransmissions++;
      }
    }
  }
}
