#include "network_interface.hh"

#include "arp_message.hh"
#include "ethernet_frame.hh"

using namespace std;

// ethernet_address: Ethernet (what ARP calls "hardware") address of the interface
// ip_address: IP (what ARP calls "protocol") address of the interface
NetworkInterface::NetworkInterface( const EthernetAddress& ethernet_address, const Address& ip_address )
  : ethernet_address_( ethernet_address ), ip_address_( ip_address ), addressMap(), arpReqMap()
{
  cerr << "DEBUG: Network interface has Ethernet address " << to_string( ethernet_address_ ) << " and IP address "
       << ip_address.ip() << "\n";
}

// dgram: the IPv4 datagram to be sent
// next_hop: the IP address of the interface to send it to (typically a router or default gateway, but
// may also be another host if directly connected to the same network as the destination)

// Note: the Address type can be converted to a uint32_t (raw 32-bit IP address) by using the
// Address::ipv4_numeric() method.
void NetworkInterface::send_datagram( const InternetDatagram& dgram, const Address& next_hop )
{
  if (addressMap.contains(next_hop.ipv4_numeric())) {
    mappingStruct& mapping = addressMap[next_hop.ipv4_numeric()];
    mapping.datagramQueue.push(dgram);
    mapping.expirationTime = globalTime + 30000;
  } else {
    mappingStruct mapping;
    mapping.datagramQueue.push(dgram);
    mapping.expirationTime = globalTime + 30000;
    addressMap[next_hop.ipv4_numeric()] = mapping;
  }
  // if (next_hop.ipv4_numeric() == 0) {
  //   cerr << "DEBUG: Sending datagram to " << next_hop.ip() << " on interface " << ip_address_.ip() << "\n";
  //   EthernetFrame frame = EthernetFrame(ethernet_address_, EthernetAddress::BROADCAST, EtherType::IPV4, dgram.serialize());
  //   send_frame(frame, next_hop);
  // } else {
  //   cerr << "DEBUG: Sending datagram to " << next_hop.ip() << " on interface " << ip_address_.ip() << "\n";
  //   EthernetFrame frame = EthernetFrame(ethernet_address_, EthernetAddress::BROADCAST, EtherType::ARP, dgram.serialize());
  //   send_frame(frame, next_hop);
  // }
}

// frame: the incoming Ethernet frame
optional<InternetDatagram> NetworkInterface::recv_frame( const EthernetFrame& frame )
{
  (void)frame;
  return {};
}

// ms_since_last_tick: the number of milliseconds since the last call to this method
void NetworkInterface::tick( const size_t ms_since_last_tick )
{
  (void)ms_since_last_tick;
}

optional<EthernetFrame> NetworkInterface::maybe_send()
{
  return {};
}
