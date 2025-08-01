#include "NIC_sim.hpp"
#include <fstream>
#include <sstream>
#include <regex>
#include <cctype>
#include <iostream>
#include <iomanip>
#include <memory>
#include <vector>
#include <string>

// --- Free function: extract_between_delimiters ---
static std::string extract_between_delimiters(const std::string& input,
                                              char delimiter,
                                              int start_index,
                                              int end_index = -1)
{
    std::vector<size_t> positions;
    size_t pos = input.find(delimiter);
    size_t start = 0;

    /* Create position vector. */
    while (pos != std::string::npos) {
        positions.push_back(pos);
        pos = input.find(delimiter, pos + 1);
    }

    /* Check start index. */
    if (start_index < 0 || start_index > static_cast<int>(positions.size())) {
        return "";
    }

    if (start_index > 0) {
        if (start_index > static_cast<int>(positions.size())) return ""; // invalid
        start = positions[start_index - 1] + 1;
    }

    /* Extract everything after start delimiter to end of string. */
    if (end_index == -1) {
        return input.substr(start);
    }

    /* Validate end_index. */
    if (end_index < start_index || end_index >= static_cast<int>(positions.size())) {
        return "";
    }

    size_t len = positions[end_index] - start;

    return input.substr(start, len);
}
// -------------------------------------------------

/**
 * @fn packet_factory
 * @brief Gets a string representing a packet, creates the corresponding
 *        packet type, and returns a pointer to a generic_packet.
 *
 * @param [in] packet - String representation of a packet.
 *
 * @return Pointer to a generic_packet object.
 */
generic_packet* nic_sim::packet_factory(std::string &packet) {
    std::string first_field = extract_between_delimiters(packet, '|', 0, 0);

    // Check for MAC address (L2): format XX:XX:XX:XX:XX:XX (hexadecimal)
    std::regex mac_regex("^[0-9A-Fa-f]{2}(:[0-9A-Fa-f]{2}){5}$");
    if (std::regex_match(first_field, mac_regex)) {
        return new l2_packet(packet);
    }

    // Check for IPv4 address (L3): format X.X.X.X (decimal, 0-255)
    std::regex ip_regex("^([0-9]{1,3}\\.){3}[0-9]{1,3}$");
    if (std::regex_match(first_field, ip_regex)) {
        return new l3_packet(packet);
    }

    // Otherwise, assume L4 (ports are decimal numbers)
    bool is_number = !first_field.empty() && std::all_of(first_field.begin(), first_field.end(), ::isdigit);
    if (is_number) {
        return new l4_packet(packet);
    }

    // Unknown format
    return nullptr;
}

/**
 * @fn nic_sim
 * @brief Constructor of the class.
 *
 * @param [in] param_file - File name containing the NIC's parameters.
 */
nic_sim::nic_sim(std::string param_file) {
    std::ifstream fin(param_file);
    std::string line;

    // 1. Read MAC address
    if (std::getline(fin, line)) {
        std::istringstream iss(line);
        for (int i = 0; i < MAC_SIZE; ++i) {
            std::string byte;
            std::getline(iss, byte, ':');
            mac[i] = static_cast<uint8_t>(std::stoul(byte, nullptr, 16));
        }
    }

    // 2. Read IP address and mask
    if (std::getline(fin, line)) {
        size_t slash = line.find('/');
        std::string ip_str = line.substr(0, slash);
        std::string mask_str = line.substr(slash + 1);
        std::istringstream iss(ip_str);
        for (int i = 0; i < IP_V4_SIZE; ++i) {
            std::string byte;
            std::getline(iss, byte, '.');
            ip[i] = static_cast<uint8_t>(std::stoi(byte));
        }
        mask = static_cast<uint8_t>(std::stoi(mask_str));
    }

    // 3. Read open ports
    while (std::getline(fin, line)) {
        std::regex port_regex("src_prt:([0-9]+), dst_port:([0-9]+)");
        std::smatch match;
        if (std::regex_search(line, match, port_regex)) {
            uint16_t src = static_cast<uint16_t>(std::stoi(match[1]));
            uint16_t dst = static_cast<uint16_t>(std::stoi(match[2]));
            open_ports.emplace_back(dst, src);
        }
    }
}

/**
 * @fn nic_flow
 * @brief Process and store to relevant location all packets in packet_file.
 *
 * @param [in] packet_file - Name of file containing packets as strings.
 */
void nic_sim::nic_flow(std::string packet_file) {
    std::ifstream fin(packet_file);
    std::string line;
    while (std::getline(fin, line)) {
        if (line.empty()) continue;
        std::unique_ptr<generic_packet> pkt(packet_factory(line));
        if (!pkt) continue;
        memory_dest dst;
        if (pkt->validate_packet(open_ports, ip, mask, mac)) {
            if (pkt->proccess_packet(open_ports, ip, mask, dst)) {
                std::string pkt_str;
                pkt->as_string(pkt_str);
                if (dst == memory_dest::RQ) RQ.push_back(pkt_str);
                else if (dst == memory_dest::TQ) TQ.push_back(pkt_str);
                // LOCAL_DRAM: already written to open_port struct
            }
        }
    }
}

/**
 * @fn nic_print_results
 * @brief Prints all data stored in memory to stdout in the required format.
 */
void nic_sim::nic_print_results() {
    // LOCAL DRAM
    std::cout << "LOCAL DRAM:" << std::endl;
    for (const auto& port : open_ports) {
        std::cout << port.src_prt << " " << port.dst_prt << ": ";
        for (int i = 0; i < DATA_ARR_SIZE; ++i) {
            std::cout << std::nouppercase << std::hex << std::setw(2) << std::setfill('0')
                      << static_cast<int>(port.data[i]);
            if (i + 1 < DATA_ARR_SIZE) std::cout << " ";
        }
        std::cout << std::dec << std::endl;
    }
    std::cout << std::endl; // Blank line after LOCAL DRAM

    // RQ
    std::cout << "RQ:" << std::endl;
    for (const auto& pkt : RQ) std::cout << pkt << std::endl;
    std::cout << std::endl; // Blank line after RQ

    // TQ
    std::cout << "TQ:" << std::endl;
    for (const auto& pkt : TQ) std::cout << pkt << std::endl;
}

/**
 * @fn ~nic_sim
 * @brief Destructor of the class.
 */
nic_sim::~nic_sim() {}