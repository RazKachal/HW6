#pragma once
#include "packets.hpp"
#include <vector>
#include <cstdint>
#include <string>

/**
 * @class l4_packet
 * @brief Represents a Layer 4 (Transport) packet for the NIC simulation.
 *
 * This class implements the generic_packet interface for L4 packets,
 * providing validation, processing, and string conversion functionalities.
 */
class l4_packet : public generic_packet {
public:
    /**
     * @fn l4_packet
     * @brief Constructor for L4 packet from explicit fields.
     *
     * @param [in] src_port - Source port.
     * @param [in] dst_port - Destination port.
     * @param [in] address - Address in the data array.
     * @param [in] data - Vector of packet data bytes.
     */
    l4_packet(uint16_t src_port, uint16_t dst_port, uint32_t address, const std::vector<uint8_t>& data);

    /**
     * @fn l4_packet
     * @brief Constructor for L4 packet from a delimited string.
     *
     * @param [in] str - String representation of the packet.
     */
    l4_packet(const std::string& str);

    /**
     * @fn validate_packet
     * @brief Check whether the packet is valid.
     *
     * @param [in] open_ports - Vector containing all the NIC's open ports.
     * @param [in] ip - NIC's IP address.
     * @param [in] mask - NIC's mask.
     * @param [in] mac - NIC's MAC address.
     *
     * @return true if the packet is valid, false otherwise.
     */
    bool validate_packet(open_port_vec open_ports, uint8_t ip[IP_V4_SIZE], uint8_t mask, uint8_t mac[MAC_SIZE]) override;

    /**
     * @fn proccess_packet
     * @brief Modify the packet and return the memory location it should be stored in.
     *
     * @param [in/out] open_ports - Vector containing all the NIC's open ports.
     * @param [in] ip - NIC's IP address.
     * @param [in] mask - NIC's mask.
     * @param [out] dst - Reference to enum indicating the memory space.
     *
     * @return true on success, false on failure.
     */
    bool proccess_packet(open_port_vec &open_ports, uint8_t ip[IP_V4_SIZE], uint8_t mask, memory_dest &dst) override;

    /**
     * @fn as_string
     * @brief Convert the packet to string.
     *
     * @param [out] packet - Output string representation of the packet.
     *
     * @return true on success, false on failure.
     */
    bool as_string(std::string &packet) override;

    uint16_t src_port;   /**< Source port */
    uint16_t dst_port;   /**< Destination port */
    uint32_t address;    /**< Address in the data array */
    std::vector<uint8_t> data; /**< Packet data bytes */
};