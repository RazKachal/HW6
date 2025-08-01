#pragma once
#include "packets.hpp"
#include "L3.h"
#include <cstdint>
#include <string>

/**
 * @class l2_packet
 * @brief Represents a Layer 2 (Data Link) packet for the NIC simulation.
 *
 * This class implements the generic_packet interface for L2 packets,
 * providing validation, processing, and string conversion functionalities.
 */
class l2_packet : public generic_packet {
public:
    /**
     * @fn l2_packet
     * @brief Constructor for L2 packet from explicit fields.
     *
     * @param [in] src_mac  - Source MAC address (array of 6 bytes).
     * @param [in] dst_mac  - Destination MAC address (array of 6 bytes).
     * @param [in] checksum - Checksum value.
     * @param [in] payload  - L3 payload packet.
     */
    l2_packet(const uint8_t src_mac[MAC_SIZE], const uint8_t dst_mac[MAC_SIZE], uint16_t checksum, l3_packet payload);

    /**
     * @fn l2_packet
     * @brief Constructor for L2 packet from a delimited string.
     *
     * @param [in] str - String representation of the packet.
     */
    l2_packet(const std::string& str);

    /**
     * @fn validate_packet
     * @brief Check whether the packet is valid.
     *
     * @param [in] open_ports - Vector containing all the NIC's open ports.
     * @param [in] ip         - NIC's IP address.
     * @param [in] mask       - NIC's mask.
     * @param [in] mac        - NIC's MAC address.
     *
     * @return true if the packet is valid, false otherwise.
     */
    bool validate_packet(open_port_vec open_ports, uint8_t ip[IP_V4_SIZE], uint8_t mask, uint8_t mac[MAC_SIZE]) override;

    /**
     * @fn proccess_packet
     * @brief Modify the packet and return the memory location it should be stored in.
     *
     * @param [in,out] open_ports - Vector containing all the NIC's open ports.
     * @param [in] ip             - NIC's IP address.
     * @param [in] mask           - NIC's mask.
     * @param [out] dst           - Reference to enum indicating the memory space.
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
     * @param [in] pkt - The L2 packet to calculate checksum for.
     *
     * @return The calculated checksum.
     */
    static uint16_t calc_checksum(const l2_packet& pkt);

    /**
     * @fn mac_to_str
     * @brief Convert a MAC address to string.
     *
     * @param [in] mac - The MAC address array.
     *
     * @return The MAC address as a string.
     */
    static std::string mac_to_str(const uint8_t mac[MAC_SIZE]);

private:
    /**
     * @fn get_l3_string
     * @brief Extract L3 string from L2 packet string.
     *
     * @param [in] str - The L2 packet string.
     *
     * @return The L3 string.
     */
    static std::string get_l3_string(const std::string& str);

public:
    uint8_t src_mac[MAC_SIZE];    /**< Source MAC address */
    uint8_t dst_mac[MAC_SIZE];    /**< Destination MAC address */
    uint16_t checksum;            /**< Checksum */
    l3_packet payload;            /**< Layer 3 packet payload */
};