#include "L4.h"
#include "common.hpp"
#include <sstream>
#include <iomanip>
#include <vector>
#include <cstdint>
#include <iostream> // Include iostream for std::cout

using namespace common;

l4_packet::l4_packet(uint16_t src_port, uint16_t dst_port, uint32_t address, const std::vector<uint8_t>& data)
    : src_port(src_port), dst_port(dst_port), address(address), data(data) {}

// New: parse from string using extract_between_delimiters
l4_packet::l4_packet(const std::string& str) {
    std::string src_port_str = extract_between_delimiters(str, '|', 0, 0);
    std::string dst_port_str = extract_between_delimiters(str, '|', 1, 1);
    std::string address_str  = extract_between_delimiters(str, '|', 2, 2);
    std::string data_str     = extract_between_delimiters(str, '|', 3, -1);

    src_port = static_cast<uint16_t>(std::stoi(src_port_str));
    dst_port = static_cast<uint16_t>(std::stoi(dst_port_str));
    address  = static_cast<uint32_t>(std::stoul(address_str));

    // Parse hex bytes
    data.clear();
    std::istringstream iss(data_str);
    std::string byte_str;
    while (iss >> byte_str) {
        data.push_back(static_cast<uint8_t>(std::stoul(byte_str, nullptr, 16)));
    }
}

bool l4_packet::validate_packet(open_port_vec open_ports, uint8_t[], uint8_t, uint8_t[]) {
    for (const auto& port : open_ports) {
        if (port.src_prt == src_port && port.dst_prt == dst_port)
            return true;
    }
    return false;
}

bool l4_packet::proccess_packet(open_port_vec &open_ports, uint8_t[], uint8_t, memory_dest &dst) {
    for (auto& port : open_ports) {
        if (port.src_prt == src_port && port.dst_prt == dst_port) {
            if (address + data.size() > DATA_ARR_SIZE) return false;
            for (size_t i = 0; i < data.size(); ++i)
                port.data[address + i] = data[i];
            dst = LOCAL_DRAM;
            return true;
        }
    }
    return false; // No matching port found - drop the packet
}

bool l4_packet::as_string(std::string &packet) {
    std::ostringstream oss;
    oss << src_port << "|" << dst_port << "|" << address << "|";
    for (size_t i = 0; i < data.size(); ++i) {
        oss << std::nouppercase << std::setw(2) << std::setfill('0') << std::hex << (int)data[i];
        if (i + 1 < data.size()) oss << " ";
    }
    packet = oss.str();
    return true;
}