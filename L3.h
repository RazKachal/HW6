#pragma once
#include "packets.hpp"
#include "L4.h"
#include <cstdint>
#include <string>

/**
 * @class l3_packet
 * @brief Represents a Layer 3 (Network) packet for the NIC simulation.
 *
 * This class implements the generic_packet interface for L3 packets,
 * providing validation, processing, and string conversion functionalities.
 */
class l3_packet : public generic_packet {
public:
    /**
     * @fn l3_packet
     * @brief Constructor for L3 packet from explicit fields.
     *
     * @param [in] src_ip - Source IPv4 address (array of 4 bytes).
     * @param [in] dst_ip - Destination IPv4 address (array of 4 bytes).
     * @param [in] ttl - Time To Live value.
     * @param [in] checksum - Checksum value.
     * @param [in] payload - L4 payload packet.
     */
    l3_packet(const uint8_t src_ip[IP_V4_SIZE], const uint8_t dst_ip[IP_V4_SIZE], uint8_t ttl, uint16_t checksum, l4_packet payload);

    /**
     * @fn l3_packet
     * @brief Constructor for L3 packet from a delimited string.
     *
     * @param [in] str - String representation of the packet.
     */
    l3_packet(const std::string& str);

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

    /**
     * @fn calc_checksum
     * @brief Calculate the checksum for the packet.
     *
     * @param [in] pkt - The L3 packet to calculate checksum for.
     *
     * @return The calculated checksum.
     */
    static uint16_t calc_checksum(const l3_packet& pkt);

    uint8_t src_ip[IP_V4_SIZE];   /**< Source IPv4 address */
    uint8_t dst_ip[IP_V4_SIZE];   /**< Destination IPv4 address */
    uint8_t ttl;                  /**< Time To Live */
    uint16_t checksum;            /**< Checksum */
    l4_packet payload;            /**< L4 payload packet */
};