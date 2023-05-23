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

// Helper function to check if we know the ethernet address of the next hop
bool NetworkInterface::ethernetKnown( const Address& next_hop )
{
  if ( addressMap.contains( next_hop.ipv4_numeric() ) ) {
    if ( addressMap[next_hop.ipv4_numeric()].ethAddress.has_value() ) {
      return true;
    }
  }
  return false;
}

// Helper function to check if the ethernet address of the next hop has expired
bool NetworkInterface::expired( const Address& next_hop )
{
  if ( addressMap.contains( next_hop.ipv4_numeric() ) ) {
    if ( addressMap[next_hop.ipv4_numeric()].expirationTime < globalTime ) {
      return true;
    }
  }
  return false;
}

// Helper function to prepare an ARP frame
EthernetFrame NetworkInterface::prepArpFrame( const uint32_t& next_hop,
                                              const EthernetAddress& ethernet_address,
                                              const Address& ip_address,
                                              const EthernetAddress& targetAddress,
                                              const uint16_t arpType )
{
  ARPMessage arp; // maybe decompose this into a function
  arp.opcode = arpType;
  arp.sender_ethernet_address = ethernet_address;
  arp.sender_ip_address = ip_address.ipv4_numeric();
  arp.target_ip_address = next_hop;
  if ( arpType == ARPMessage::OPCODE_REPLY ) {
    arp.target_ethernet_address = targetAddress;
  }
  EthernetFrame frame = EthernetFrame( EthernetHeader( targetAddress, ethernet_address, EthernetHeader::TYPE_ARP ),
                                       serialize( arp ) );
  return frame;
}

// Helper function to send pending datagrams once we get the ethernet address of the next hop
void NetworkInterface::sendPendingDgrams( const uint32_t& next_hop )
{
  while ( !addressMap[next_hop].datagramQueue.empty() ) {
    InternetDatagram dgram = addressMap[next_hop].datagramQueue.front();
    addressMap[next_hop].datagramQueue.pop();
    send_datagram( dgram, Address::from_ipv4_numeric( next_hop ) );
  }
}

void NetworkInterface::send_datagram( const InternetDatagram& dgram, const Address& next_hop )
{
  /* Two cases:
    either we need to send an ARP message to get the ethernet address of the next hop
    Or we need to just send an ethernet frame to a known destination
  */
  uint32_t next_hop_ip = next_hop.ipv4_numeric();

  if ( !addressMap.contains( next_hop_ip )
       || ( ( addressMap.contains( next_hop_ip ) && !ethernetKnown( next_hop ) && expired( next_hop ) ) ) ) {

    EthernetFrame frame = prepArpFrame(
      next_hop.ipv4_numeric(), ethernet_address_, ip_address_, ETHERNET_BROADCAST, ARPMessage::OPCODE_REQUEST );
    addressMap[next_hop_ip].datagramQueue.push( dgram );
    addressMap[next_hop_ip].expirationTime = globalTime + 5000;
    ethernetQueue.push( frame );

  } else {
    if ( ethernetKnown( next_hop ) ) {
      EthernetFrame frame = EthernetFrame(
        EthernetHeader( addressMap[next_hop_ip].ethAddress.value(), ethernet_address_, EthernetHeader::TYPE_IPv4 ),
        serialize( dgram ) );
      ethernetQueue.push( frame );
    }
  }
}

// frame: the incoming Ethernet frame
optional<InternetDatagram> NetworkInterface::recv_frame( const EthernetFrame& frame )
{
  if ( frame.header.type == EthernetHeader::TYPE_IPv4 ) {
    InternetDatagram dgram;
    bool ipValid = parse( dgram, frame.payload );
    // check if the ethernet address is the same as the one we have or if it is broadcast
    if ( ipValid && ( frame.header.dst == ethernet_address_ || frame.header.dst == ETHERNET_BROADCAST ) ) {
      return dgram;
    } else {
      return {};
    }
  } else if ( frame.header.type == EthernetHeader::TYPE_ARP ) {
    ARPMessage arp;
    bool parseValid = parse( arp, frame.payload );
    addressMap[arp.sender_ip_address].ethAddress = arp.sender_ethernet_address;
    addressMap[arp.sender_ip_address].expirationTime = globalTime + 30000;
    sendPendingDgrams( arp.sender_ip_address );
    if ( parseValid && arp.opcode == ARPMessage::OPCODE_REQUEST
         && ( arp.target_ip_address == ip_address_.ipv4_numeric() || arp.target_ip_address == 0 ) ) {
      // if it’s an ARP request asking for our IP address, send an appropriate ARP reply.
      EthernetFrame replyFrame = prepArpFrame( arp.sender_ip_address,
                                               ethernet_address_,
                                               ip_address_,
                                               arp.sender_ethernet_address,
                                               ARPMessage::OPCODE_REPLY );
      ethernetQueue.push( replyFrame );
    }
  }
  return {};
}

// ms_since_last_tick: the number of milliseconds since the last call to this method
void NetworkInterface::tick( const size_t ms_since_last_tick )
{
  globalTime += ms_since_last_tick;
  for ( auto it = addressMap.begin(); it != addressMap.end(); ) {
    if ( it->second.ethAddress.has_value() && it->second.expirationTime < globalTime ) {
      it = addressMap.erase( it );
    } else {
      it++;
    }
  }
}

optional<EthernetFrame> NetworkInterface::maybe_send()
{
  if ( !ethernetQueue.empty() ) {
    EthernetFrame frame = ethernetQueue.front();
    ethernetQueue.pop();
    return frame;
  }
  return {};
}
