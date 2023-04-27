#include "wrapping_integers.hh"
using namespace std;

Wrap32 Wrap32::wrap( uint64_t n, Wrap32 zero_point )
{
  uint32_t t = static_cast<uint32_t>( ( n + zero_point.raw_value_ ) % ( 1ULL << 32 ) );
  return Wrap32 { t };
}

uint64_t Wrap32::unwrap( Wrap32 zero_point, uint64_t checkpoint ) const
{
  // Compute the difference between the checkpoint and the zero point
  uint64_t diff = raw_value_ - zero_point.raw_value_;

  // Compute the number of times the wraparound occurred between the zero point and this value
  uint64_t wraps = ( checkpoint ) / UINT32_MAX;

  if ( wraps == 0 ) {
    wraps = 1;
  }
  uint64_t upper = diff + ( wraps * ( uint64_t( 1 ) << 32 ) );
  uint64_t lower = diff + ( ( wraps - 1 ) * ( uint64_t( 1 ) << 32 ) );
  if ( upper < checkpoint ) {
    upper += ( uint64_t( 1 ) << 32 );
    lower += ( uint64_t( 1 ) << 32 );
  }
  uint64_t result;
  uint64_t diff_upper = ( upper > checkpoint ) ? ( upper - checkpoint ) : ( checkpoint - upper );
  uint64_t diff_lower = ( lower > checkpoint ) ? ( lower - checkpoint ) : ( checkpoint - lower );

  if ( diff_upper < diff_lower ) {
    result = upper;
  } else {
    result = lower;
  }

  return result;
}
