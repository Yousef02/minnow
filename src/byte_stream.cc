#include "byte_stream.hh"
#include <iostream>
#include <queue>
#include <stdexcept>
#include <string_view>

using namespace std;

ByteStream::ByteStream( uint64_t capacity ) : capacity_( capacity ), streamStack() {}

void Writer::push( string data )
{
  if ( !data.empty() && data.length() <= available_capacity() ) {
    streamStack.push( data );
    numPushed += data.length();
  } else if ( !data.empty() ) {
    streamStack.push( data.substr( 0, available_capacity() ) );
    numPushed += available_capacity();
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
  return capacity_ - ( numPushed - numPopped );
}

uint64_t Writer::bytes_pushed() const
{
  return numPushed;
}

std::string_view Reader::peek() const
{
  if ( !streamStack.empty() ) {
    return string_view { streamStack.front().data(), streamStack.front().length() };
  } else {
    return std::string_view();
  }
}

bool Reader::is_finished() const
{
  if ( streamClosed && streamStack.empty() ) {
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
  while ( !streamStack.empty() && len >= streamStack.front().length() ) {
    len -= streamStack.front().length();
    numPopped += streamStack.front().length();
    streamStack.pop();
  }
  if ( !streamStack.empty() && len > 0 ) {
    streamStack.front() = streamStack.front().substr( len );
    numPopped += len;
  }
}

uint64_t Reader::bytes_buffered() const
{
  return numPushed - numPopped;
}

uint64_t Reader::bytes_popped() const
{
  return numPopped;
}