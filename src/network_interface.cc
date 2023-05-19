#include "network_interface.hh"

#include "arp_message.hh"
#include "ethernet_frame.hh"

using namespace std;

// ethernet_address: Ethernet (what ARP calls "hardware") address of the interface
// ip_address: IP (what ARP calls "protocol") address of the interface
NetworkInterface::NetworkInterface( const EthernetAddress& ethernet_address, const Address& ip_address )
  : ethernet_address_( ethernet_address ), ip_address_( ip_address ), addressMap(), ethernetQueue()
{
  cerr << "DEBUG: Network interface has Ethernet address " << to_string( ethernet_address_ ) << " and IP address "
       << ip_address.ip() << "\n";
}

// dgram: the IPv4 datagram to be sent
// next_hop: the IP address of the interface to send it to (typically a router or default gateway, but
// may also be another host if directly connected to the same network as the destination)

// Note: the Address type can be converted to a uint32_t (raw 32-bit IP address) by using the
// Address::ipv4_numeric() method.
bool NetworkInterface::ethernetKnown(const Address& next_hop) {
  if (addressMap.contains(next_hop.ipv4_numeric())) {
    if (addressMap[next_hop.ipv4_numeric()].ethAddress.has_value()) {
      return true;
    }
  }
  return false;
}
bool NetworkInterface::expired(const Address& next_hop) {
  if (addressMap.contains(next_hop.ipv4_numeric())) {
    if (addressMap[next_hop.ipv4_numeric()].expirationTime < globalTime) {
      return true;
    }
  }
  return false;
}

void NetworkInterface::send_datagram( const InternetDatagram& dgram, const Address& next_hop )
{

  // Question: if we do the smart method, does that mean that there is nothing exired in the map?
  if (addressMap.contains(next_hop.ipv4_numeric())) {
    if (ethernetKnown(next_hop)) {
      EthernetFrame frame = 
        EthernetFrame(EthernetHeader(addressMap[next_hop.ipv4_numeric()].ethAddress.value(), 
        ethernet_address_, 
        EthernetHeader::TYPE_IPv4), 
        serialize(dgram));
      ethernetQueue.push(frame);
    } else {
      EthernetFrame frame = 
        EthernetFrame(EthernetHeader(ETHERNET_BROADCAST, ethernet_address_,
        EthernetHeader::TYPE_ARP), serialize(dgram));
      addressMap[next_hop.ipv4_numeric()].datagramQueue.push(dgram);
    }
  } else {
    ARPMessage arp; // maybe decompose this into a function
      arp.opcode = ARPMessage::OPCODE_REQUEST;
      arp.sender_ethernet_address = ethernet_address_;
      arp.sender_ip_address = ip_address_.ipv4_numeric();
      arp.target_ip_address = next_hop.ipv4_numeric();
    EthernetFrame frame = 
      EthernetFrame(EthernetHeader(ETHERNET_BROADCAST, ethernet_address_,
      EthernetHeader::TYPE_ARP), serialize(arp));
    addressMap[next_hop.ipv4_numeric()].datagramQueue.push(dgram);
    ethernetQueue.push(frame);
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
  if (!ethernetQueue.empty()) {
    EthernetFrame frame = ethernetQueue.front();
    ethernetQueue.pop();
    return frame;
  }
  return {};
}
