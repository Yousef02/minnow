#include "router.hh"

#include <iostream>
#include <limits>

using namespace std;

// route_prefix: The "up-to-32-bit" IPv4 address prefix to match the datagram's destination address against
// prefix_length: For this route to be applicable, how many high-order (most-significant) bits of
//    the route_prefix will need to match the corresponding bits of the datagram's destination address?
// next_hop: The IP address of the next hop. Will be empty if the network is directly attached to the router (in
//    which case, the next hop address should be the datagram's final destination).
// interface_num: The index of the interface to send the datagram out on.
void Router::add_route( const uint32_t route_prefix,
                        const uint8_t prefix_length,
                        const optional<Address> next_hop,
                        const size_t interface_num )
{
  cerr << "DEBUG: adding route " << Address::from_ipv4_numeric( route_prefix ).ip() << "/"
       << static_cast<int>( prefix_length ) << " => " << ( next_hop.has_value() ? next_hop->ip() : "(direct)" )
       << " on interface " << interface_num << "\n";

  routes_.push_back( { route_prefix, prefix_length, next_hop, interface_num } );
}

void Router::route()
{

  for ( auto& interface : interfaces_ ) {
    auto datagram = interface.maybe_receive();
    while ( datagram.has_value() ) {
      auto route = routes_.end();
      uint8_t max_prefix_length = 0;
      for ( auto it = routes_.begin(); it != routes_.end(); it++ ) {
        // check if the length is 0 bits, because shifting by 32 is undefined behavior
        if ( it->prefix_length == 0 ) {
          route = it;
          max_prefix_length = 0;
        } else {
          // check if the prefix matches the datagram's destination address
          if ( ( it->route_prefix >> ( 32 - it->prefix_length ) )
               == ( datagram->header.dst >> ( 32 - it->prefix_length ) ) ) {
            // if the prefix length is longer than the current max, update the max and the route
            if ( it->prefix_length > max_prefix_length ) {
              max_prefix_length = it->prefix_length;
              route = it;
            }
          }
        }
      }
      if ( route != routes_.end() ) {
        // if the ttl is 0 or is going to hit 0, drop the datagram. Otherwise, decrement the ttl
        if ( datagram->header.ttl <= 1 ) {
          datagram = interface.maybe_receive();
          continue;
        } else {
          datagram->header.ttl--;
          datagram->header.compute_checksum();
        }

        if ( route->next_hop.has_value() ) {
          interfaces_[route->interface_num].send_datagram( *datagram, *route->next_hop );
        } else {
          interfaces_[route->interface_num].send_datagram( *datagram,
                                                           Address::from_ipv4_numeric( datagram->header.dst ) );
        }
      }
      datagram = interface.maybe_receive();
    }
  }
}
