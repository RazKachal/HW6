#include "L3.h"
#include <sstream>
#include <iomanip>
#include <cstring>
#include <vector>
#include <iostream> // For debug output

using namespace common;

static bool ip_in_net(const uint8_t ip[IP_V4_SIZE], const uint8_t net_ip[IP_V4_SIZE], uint8_t mask) {
    int bits = mask;
    for (int i = 0; i < IP_V4_SIZE; ++i) {
        uint8_t mask_byte = bits >= 8 ? 0xFF : (bits == 0 ? 0 : (0xFF << (8 - bits)));
        if ((ip[i] & mask_byte) != (net_ip[i] & mask_byte)) return false;
        bits = bits > 8 ? bits - 8 : 0;
    }
    return true;
}

l3_packet::l3_packet(const uint8_t src_ip_[IP_V4_SIZE], const uint8_t dst_ip_[IP_V4_SIZE], uint8_t ttl, uint16_t checksum, l4_packet payload)
    : ttl(ttl), checksum(checksum), payload(payload) {
    std::memcpy(src_ip, src_ip_, IP_V4_SIZE);
    std::memcpy(dst_ip, dst_ip_, IP_V4_SIZE);
}

uint16_t l3_packet::calc_checksum(const l3_packet& pkt) {
    uint16_t sum = 0;
    // Sum IP addresses
    for (int i = 0; i < IP_V4_SIZE; ++i) sum += pkt.src_ip[i];
    for (int i = 0; i < IP_V4_SIZE; ++i) sum += pkt.dst_ip[i];
    // Sum TTL
    sum += pkt.ttl;

    // Sum L4 header fields (ports and address) as individual bytes
    sum += (pkt.payload.src_port >> 8);         // High byte
    sum += (pkt.payload.src_port & 0xFF);       // Low byte
    sum += (pkt.payload.dst_port >> 8);         // High byte
    sum += (pkt.payload.dst_port & 0xFF);       // Low byte
    sum += (pkt.payload.address >> 8);          // High byte
    sum += (pkt.payload.address & 0xFF);        // Low byte
    
    // Sum L4 data bytes - this was missing!
    for (const auto& byte : pkt.payload.data) {
        sum += byte;
    }
        
    return sum;
}

bool l3_packet::validate_packet(open_port_vec, uint8_t[], uint8_t, uint8_t[]) {
    bool valid_checksum = (calc_checksum(*this) == checksum);
    bool valid_ttl = (ttl > 0);
    return (valid_checksum && valid_ttl);
}

bool l3_packet::proccess_packet(open_port_vec &open_ports, uint8_t ip[IP_V4_SIZE], uint8_t mask, memory_dest &dst) {
    bool src_in = ip_in_net(src_ip, ip, mask);
    bool dst_in = ip_in_net(dst_ip, ip, mask);
    bool dst_is_me = std::memcmp(dst_ip, ip, IP_V4_SIZE) == 0;

    if (!validate_packet(open_ports, ip, mask, nullptr)) return false;

    if (dst_is_me) { // For me (write to LOCAL DRAM)
        if (payload.proccess_packet(open_ports, ip, mask, dst)) {
            // Only write to LOCAL DRAM if L4 ports match
            return true;
        }
        // If not written to LOCAL DRAM, drop the packet (do not forward)
        return false;
    }
    if (!src_in && dst_in) { // Incoming
        if (--ttl == 0) return false;
        checksum = calc_checksum(*this);
        dst = RQ;
        return true;
    }
    if (src_in && !dst_in) { // Outgoing
        std::memcpy(src_ip, ip, IP_V4_SIZE);
        if (--ttl == 0) return false;
        checksum = calc_checksum(*this);
        dst = TQ;
        return true;
    }
    if (!src_in && !dst_in) { // Transit
        if (--ttl == 0) return false;
        checksum = calc_checksum(*this);
        dst = TQ;
        return true;
    }
    // Both in net or not relevant
    return false;
}

bool l3_packet::as_string(std::string &packet) {
    std::ostringstream oss;
    for (int i = 0; i < IP_V4_SIZE; ++i) {
        oss << (int)src_ip[i];
        if (i < IP_V4_SIZE - 1) oss << ".";
    }
    oss << "|";
    for (int i = 0; i < IP_V4_SIZE; ++i) {
        oss << (int)dst_ip[i];
        if (i < IP_V4_SIZE - 1) oss << ".";
    }
    oss << "|" << (int)ttl << "|" << checksum << "|";
    std::string l4str;
    payload.as_string(l4str);
    oss << l4str;
    packet = oss.str();
    return true;
}

l3_packet::l3_packet(const std::string& str)
    : payload(extract_between_delimiters(str, '|', 4, -1)) // L4 string
{
    std::string src_ip_str = extract_between_delimiters(str, '|', 0, 0);
    std::string dst_ip_str = extract_between_delimiters(str, '|', 1, 1);
    std::string ttl_str    = extract_between_delimiters(str, '|', 2, 2);
    std::string cs_str     = extract_between_delimiters(str, '|', 3, 3);

    // Parse IPs
    std::istringstream iss1(src_ip_str), iss2(dst_ip_str);
    for (int i = 0; i < IP_V4_SIZE; ++i) {
        std::string byte;
        std::getline(iss1, byte, '.');
        src_ip[i] = static_cast<uint8_t>(std::stoi(byte));
        std::getline(iss2, byte, '.');
        dst_ip[i] = static_cast<uint8_t>(std::stoi(byte));
    }
    ttl = static_cast<uint8_t>(std::stoi(ttl_str));
    checksum = static_cast<uint16_t>(std::stoi(cs_str));
}