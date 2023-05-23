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

EthernetFrame prepArpFrame(const Address& next_hop, const EthernetAddress& ethernet_address, const Address& ip_address, const EthernetAddress& targetAddress ,const uint16_t arpType) {
  ARPMessage arp; // maybe decompose this into a function
  arp.opcode = arpType;
  arp.sender_ethernet_address = ethernet_address;
  arp.sender_ip_address = ip_address.ipv4_numeric();
  arp.target_ip_address = next_hop.ipv4_numeric();
  EthernetFrame frame = 
    EthernetFrame(EthernetHeader(targetAddress, ethernet_address,
    EthernetHeader::TYPE_ARP), serialize(arp));
  return frame;
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
    } else if (expired(next_hop)) {
      ARPMessage arp; // maybe decompose this into a function
      arp.opcode = ARPMessage::OPCODE_REQUEST;
      arp.sender_ethernet_address = ethernet_address_;
      arp.sender_ip_address = ip_address_.ipv4_numeric();
      arp.target_ip_address = next_hop.ipv4_numeric();
      EthernetFrame frame = 
        EthernetFrame(EthernetHeader(ETHERNET_BROADCAST, ethernet_address_,
        EthernetHeader::TYPE_ARP), serialize(arp));
      addressMap[next_hop.ipv4_numeric()].datagramQueue.push(dgram);
      addressMap[next_hop.ipv4_numeric()].expirationTime = globalTime + 5000;
      ethernetQueue.push(frame);
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
    addressMap[next_hop.ipv4_numeric()].expirationTime = globalTime + 5000;
    ethernetQueue.push(frame);
  }

}

// frame: the incoming Ethernet frame
optional<InternetDatagram> NetworkInterface::recv_frame( const EthernetFrame& frame )
{

  if (frame.header.type == EthernetHeader::TYPE_IPv4) {
    InternetDatagram dgram;
    bool ipValid = parse(dgram, frame.payload);
    // check if the ethernet address is the same as the one we have or if it is broadcast
    if (ipValid && (frame.header.dst == ethernet_address_ || frame.header.dst == ETHERNET_BROADCAST)) {
      return dgram;
    } else {
      return {};
    }
  } else if (frame.header.type == EthernetHeader::TYPE_ARP) {
    ARPMessage arp;
    bool parseValid = parse(arp, frame.payload);
    addressMap[arp.sender_ip_address].ethAddress = arp.sender_ethernet_address;
    addressMap[arp.sender_ip_address].expirationTime = globalTime + 30000;
    while (!addressMap[arp.sender_ip_address].datagramQueue.empty()) {
      InternetDatagram dgram = addressMap[arp.sender_ip_address].datagramQueue.front();
      addressMap[arp.sender_ip_address].datagramQueue.pop();
      send_datagram(dgram, Address::from_ipv4_numeric(arp.sender_ip_address));
    }
    if (parseValid && arp.opcode == ARPMessage::OPCODE_REQUEST && (arp.target_ip_address == ip_address_.ipv4_numeric() || arp.target_ip_address == 0)) {
      // if it’s an ARP request asking for our IP address, send an appropriate ARP reply.
      ARPMessage arpReply;
      arpReply.opcode = ARPMessage::OPCODE_REPLY;
      arpReply.sender_ethernet_address = ethernet_address_;
      arpReply.sender_ip_address = ip_address_.ipv4_numeric();
      arpReply.target_ethernet_address = arp.sender_ethernet_address;
      arpReply.target_ip_address = arp.sender_ip_address;
      EthernetFrame replyFrame = 
        EthernetFrame(EthernetHeader(arpReply.target_ethernet_address, ethernet_address_,
        EthernetHeader::TYPE_ARP), serialize(arpReply));
      ethernetQueue.push(replyFrame);
      
    // } else if (arp.opcode == ARPMessage::OPCODE_REPLY) {
    //   addressMap[arp.sender_ip_address].ethAddress = arp.sender_ethernet_address;
    //   addressMap[arp.sender_ip_address].expirationTime = globalTime + 30000;
    //   while (!addressMap[arp.sender_ip_address].datagramQueue.empty()) {
    //     InternetDatagram dgram = addressMap[arp.sender_ip_address].datagramQueue.front();
    //     addressMap[arp.sender_ip_address].datagramQueue.pop();
    //     send_datagram(dgram, Address::from_ipv4_numeric(arp.sender_ip_address));
    //   }
    }
  }
  return {};
}

// ms_since_last_tick: the number of milliseconds since the last call to this method
void NetworkInterface::tick( const size_t ms_since_last_tick )
{
  globalTime += ms_since_last_tick;
  for (auto it = addressMap.begin(); it != addressMap.end();) {
    if (it->second.ethAddress.has_value() && it->second.expirationTime < globalTime) {
      it = addressMap.erase(it);
    } else {
      it++;
    }
  }
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
