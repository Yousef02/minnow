#include "byte_stream.hh"
#include <iostream>
#include <queue>
#include <stdexcept>
#include <string_view>

using namespace std;

ByteStream::ByteStream( uint64_t capacity ) : capacity_( capacity ), streamStack() {}

void Writer::push( string data )
{
  // Keep pushing chars as long as we have space
  for ( char c : data ) {
    if ( trackCap >= capacity_ ) {
      break;
    }
    streamStack.push( c );
    popped = false;
    trackCap += 1;
    numPushed += 1;
  }
}

void Writer::close()
{
  streamClosed = true;
}

void Writer::set_error()
{
  errorSig = true;
}

bool Writer::is_closed() const
{
  if ( streamClosed ) {
    return true;
  }
  return false;
}

uint64_t Writer::available_capacity() const
{
  return capacity_ - trackCap;
}

uint64_t Writer::bytes_pushed() const
{
  return numPushed;
}

std::string_view Reader::peek() const
{
  if ( !streamStack.empty() ) {
    return std::string_view( &streamStack.front(), 1 );
  } else {
    return std::string_view();
  }
}

bool Reader::is_finished() const
{
  if ( streamClosed && popped ) {
    return true;
  }
  return false;
}

bool Reader::has_error() const
{
  if ( errorSig ) {
    return true;
  }
  return false;
}

void Reader::pop( uint64_t len )
{
  for ( uint64_t i = 0; i < len; i++ ) {
    streamStack.pop();
    trackCap -= 1;
    numPopped += 1;
  }
  if ( streamStack.empty() ) {
    popped = true;
  }
}

uint64_t Reader::bytes_buffered() const
{
  return streamStack.size();
}

uint64_t Reader::bytes_popped() const
{
  return numPopped;
}