#include "reassembler.hh"
#include <set>
#include <string>

using namespace std;

Reassembler::Reassembler() : ourSet() {}

void Reassembler::insert( uint64_t first_index, string data, bool is_last_substring, Writer& output )
{
  uint64_t adjusted_first_index = first_index;
  uint64_t first_unass_ind = output.bytes_pushed();

  if ( is_last_substring ) {
    seenLast = true;
  }

  // completely out of available capacity
  if ( first_index >= first_unass_ind + output.available_capacity() ) {
    return;
  }

  // if the data starts before the pushed bytes counter (it is partially or fully redundant)
  if ( first_index < first_unass_ind ) {
    // completely redundant
    if ( first_index + data.length() < first_unass_ind ) {
      return;
    }
    data = data.substr( first_unass_ind - first_index );
    adjusted_first_index = first_unass_ind;
  }

  // if the data ends past the last acceptable byte
  if ( adjusted_first_index + data.length() - 1 >= first_unass_ind + output.available_capacity() ) {
    // if (output.available_capacity() == 0) {
    //   return;
    // }
    data = data.substr( 0, output.available_capacity() - ( adjusted_first_index - first_unass_ind ) );
  }

  Element to_insert = { adjusted_first_index, data };

  // if the reassembler is empty, no overlapping possible. Otherwise check and resolve.
  if ( ourSet.empty() ) {
    setSize += to_insert.data.length();
    ourSet.insert( to_insert );
  } else {
    // lower_bound returns an iterator to the element with index equal or greater to the index we want to insert
    auto bound = ourSet.lower_bound( to_insert );
    // if there is an element before our current element, we want to start checking overlapping from there
    if ( bound != ourSet.begin() && bound->index != adjusted_first_index ) {
      --bound;
    }
    auto cur = bound;

    // data has already been inserted
    if ( to_insert.index >= bound->index
         && to_insert.index + to_insert.data.length() <= bound->index + bound->data.length() ) {
      return;
    }

    // first, resolve any partial overlapping at the start of the data being inserted
    if ( bound->index <= to_insert.index && bound->data.length() + bound->index > to_insert.index ) {
      Element modif_begin = to_insert;
      // new data should begin here
      uint64_t begin = bound->index + bound->data.length() - to_insert.index;
      modif_begin.data = to_insert.data.substr( begin );
      modif_begin.index += begin;
      to_insert = modif_begin;
      // move iterator forward
      ++cur;
    } else if ( bound->index + bound->data.length() <= to_insert.index ) {
      ++cur;
    }

    // Second, erase all data in the set that's included in what's being inserted
    while ( cur != ourSet.end() && cur->index >= to_insert.index
            && cur->index + cur->data.length() <= to_insert.index + to_insert.data.length() ) {
      setSize -= cur->data.length();
      cur = ourSet.erase( cur );
    }

    // lastly, resolve any partial overlapping at the end of the data being inserted
    if ( cur != ourSet.end() && cur->index < to_insert.index + to_insert.data.length()
         && to_insert.index + to_insert.data.length() < cur->data.length() + cur->index ) {
      Element modif_end = to_insert;
      uint64_t end = cur->index - to_insert.index;
      modif_end.data = to_insert.data.substr( 0, end );
      to_insert = modif_end;
    }

    ourSet.insert( to_insert );
    setSize += to_insert.data.length();
  }

  // push to the byteStream when possible
  auto firstInd = ourSet.begin();
  while ( firstInd != ourSet.end() && firstInd->index == output.bytes_pushed() ) {
    output.push( firstInd->data );
    setSize -= firstInd->data.length();
    firstInd = ourSet.erase( firstInd );
  }

  if ( seenLast && ourSet.empty() ) {
    output.close();
  }
}

uint64_t Reassembler::bytes_pending() const
{
  return setSize;
}