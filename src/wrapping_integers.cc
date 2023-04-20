#include "wrapping_integers.hh"
using namespace std;

Wrap32 Wrap32::wrap( uint64_t n, Wrap32 zero_point )
{
  return Wrap32 { static_cast<uint32_t>(n + zero_point.raw_value_) };
}

uint64_t Wrap32::unwrap( Wrap32 zero_point, uint64_t checkpoint ) const 
{
  // Compute the difference between the checkpoint and the zero point
  uint64_t diff = raw_value_ - zero_point.raw_value_;

  // Compute the number of times the wraparound occurred between the zero point and this value
  uint64_t wraps = (checkpoint) / UINT32_MAX;

  // Compute the absolute sequence number that wraps to this Wrap32, closest to the checkpoint
  // uint64_t result = zero_point.raw_value_ + (wraps * (uint64_t(1) << 32));
  // if (result <= checkpoint) {
  //   result += (uint64_t(1) << 32);
  // }
 
  uint64_t upper = diff + (wraps * (uint64_t(1) << 32));
  uint64_t lower = diff + ((wraps - 1) * (uint64_t(1) << 32));
  uint64_t result = llabs(upper - checkpoint) < llabs(lower - checkpoint) ? upper : lower;
  return result;
}
