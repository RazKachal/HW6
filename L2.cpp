#include "L2.h"
#include <sstream>
#include <iomanip>
#include <cstring>
#include <vector>
#include <iostream> // Include iostream for std::cout

using namespace common;

l2_packet::l2_packet(const uint8_t src_mac_[MAC_SIZE], const uint8_t dst_mac_[MAC_SIZE], uint16_t checksum, l3_packet payload)
    : checksum(checksum), payload(payload) {
    std::memcpy(src_mac, src_mac_, MAC_SIZE);
    std::memcpy(dst_mac, dst_mac_, MAC_SIZE);
}

uint16_t l2_packet::calc_checksum(const l2_packet& pkt) {
    uint16_t sum = 0;
    // Sum MAC addresses
    for (int i = 0; i < MAC_SIZE; ++i) sum += pkt.src_mac[i];
    for (int i = 0; i < MAC_SIZE; ++i) sum += pkt.dst_mac[i];
    
    // Sum IP addresses
    for (int i = 0; i < IP_V4_SIZE; ++i) sum += pkt.payload.src_ip[i];
    for (int i = 0; i < IP_V4_SIZE; ++i) sum += pkt.payload.dst_ip[i];
    
    // Sum TTL
    sum += pkt.payload.ttl;
    
    // Sum L3 checksum (as bytes)
    sum += (pkt.payload.checksum >> 8);   // High byte
    sum += (pkt.payload.checksum & 0xFF); // Low byte
    
    // Sum L4 data (ports, address) - treat as individual bytes
    sum += (pkt.payload.payload.src_port >> 8); // High byte
    sum += (pkt.payload.payload.src_port & 0xFF); // Low byte
    sum += (pkt.payload.payload.dst_port >> 8); // High byte
    sum += (pkt.payload.payload.dst_port & 0xFF); // Low byte
    sum += (pkt.payload.payload.address >> 8); // High byte
    sum += (pkt.payload.payload.address & 0xFF); // Low byte
    
    // Sum L4 data bytes
    for (const auto& byte : pkt.payload.payload.data) {
        sum += byte;
    }
    
    return sum;
}

std::string l2_packet::mac_to_str(const uint8_t mac[MAC_SIZE]) {
    std::ostringstream oss;
    for (int i = 0; i < MAC_SIZE; ++i) {
        oss << std::nouppercase << std::setw(2) << std::setfill('0') << std::hex << (int)mac[i];
        if (i < MAC_SIZE - 1) oss << ":";
    }
    return oss.str();
}

bool l2_packet::validate_packet(open_port_vec, uint8_t[], uint8_t, uint8_t mac[MAC_SIZE]) {
    if (std::memcmp(dst_mac, mac, MAC_SIZE) != 0) return false;
    if (calc_checksum(*this) != checksum) return false;
    return true;
}

bool l2_packet::proccess_packet(open_port_vec &open_ports, uint8_t ip[IP_V4_SIZE], uint8_t mask, memory_dest &dst) {
    // L2 validation should be done before calling proccess_packet
    // Here we just delegate to the L3 layer
    return payload.proccess_packet(open_ports, ip, mask, dst);
}

bool l2_packet::as_string(std::string &packet) {
    // When L2 packets are forwarded, only the L3 content is forwarded
    // This implements the "remove L2 layer" requirement
    return payload.as_string(packet);
}

l2_packet::l2_packet(const std::string& str) : payload(get_l3_string(str))
{
    std::string src_mac_str = extract_between_delimiters(str, '|', 0, 0);
    std::string dst_mac_str = extract_between_delimiters(str, '|', 1, 1);
    
    // Extract the L2 checksum (last field after the last '|')
    size_t last_delim = str.find_last_of('|');
    std::string cs_str = str.substr(last_delim + 1);
    
    // Parse MACs first
    std::istringstream iss1(src_mac_str), iss2(dst_mac_str);
    for (int i = 0; i < MAC_SIZE; ++i) {
        std::string byte;
        std::getline(iss1, byte, ':');
        src_mac[i] = static_cast<uint8_t>(std::stoul(byte, nullptr, 16));
        std::getline(iss2, byte, ':');
        dst_mac[i] = static_cast<uint8_t>(std::stoul(byte, nullptr, 16));
    }
    checksum = static_cast<uint16_t>(std::stoi(cs_str));
}

std::string l2_packet::get_l3_string(const std::string& str) {
    // Extract L3 string (from after second '|' to before last '|')
    size_t last_delim = str.find_last_of('|');
    size_t second_delim = str.find('|');
    second_delim = str.find('|', second_delim + 1);
    std::string l3_str = str.substr(second_delim + 1, last_delim - second_delim - 1);
    return l3_str;
}