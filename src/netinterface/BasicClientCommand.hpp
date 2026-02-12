//  illarionserver - server for the game Illarion
//  Copyright 2011 Illarion e.V.
//
//  This file is part of illarionserver.
//
//  illarionserver is free software: you can redistribute it and/or modify
//  it under the terms of the GNU Affero General Public License as published by
//  the Free Software Foundation, either version 3 of the License, or
//  (at your option) any later version.
//
//  illarionserver is distributed in the hope that it will be useful,
//  but WITHOUT ANY WARRANTY; without even the implied warranty of
//  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//  GNU Affero General Public License for more details.
//
//  You should have received a copy of the GNU Affero General Public License
//  along with illarionserver.  If not, see <http://www.gnu.org/licenses/>.

#ifndef CBASICCLIENTCOMMAND_HPP
#define CBASICCLIENTCOMMAND_HPP

#include "netinterface/BasicCommand.hpp"

#include <chrono>
#include <cstdint>
#include <memory>
#include <string>
#include <vector>

class Player;

/**
 * @brief Exception thrown when attempting to read beyond the command buffer size.
 */
class OverflowException {};

class BasicClientCommand;
using ClientCommandPointer = std::shared_ptr<BasicClientCommand>; ///< Shared pointer type for client commands

/**
 * @brief Base class for all commands received from game clients.
 *
 * Handles deserialization of network data from clients, including:
 * - Header parsing (length and checksum validation)
 * - Type-safe extraction of primitive types and strings from message buffer
 * - CRC checksum verification for data integrity
 * - Action point (AP) requirements for command execution
 * - Timestamp tracking for command arrival time
 *
 * Subclasses must implement:
 * - decodeData(): Parse command-specific data from buffer
 * - performAction(): Execute the command's game logic
 * - clone(): Create a new instance for the CommandFactory
 *
 * @note Thread-safe buffer reading with automatic overflow detection
 * @see BasicServerCommand for server-to-client commands
 * @see CommandFactory for command instantiation
 */
class BasicClientCommand : public BasicCommand {
public:
    /**
     * @brief Constructs a client command with command ID and AP requirement.
     * @param defByte Command identifier byte from network protocol
     * @param minAP Minimum action points required to execute (default: 0)
     */
    BasicClientCommand(unsigned char defByte, uint16_t minAP = 0);

    /**
     * @brief Sets header data received from network protocol.
     * @param mlength Message body length in bytes (excluding header)
     * @param mcheckSum CRC checksum for data integrity verification
     */
    void setHeaderData(uint16_t mlength, uint16_t mcheckSum);

    virtual ~BasicClientCommand() = default;

    auto operator=(const BasicClientCommand &) -> BasicClientCommand & = delete;
    BasicClientCommand(BasicClientCommand const &) = delete;
    BasicClientCommand(BasicClientCommand &&) = delete;
    auto operator=(BasicClientCommand &&) -> BasicClientCommand & = delete;

    /**
     * @brief Returns reference to internal message buffer for network data population.
     * @return Mutable reference to the raw byte buffer
     */
    auto msg_data() -> std::vector<unsigned char> &;

    /**
     * @brief Parses command-specific data from the message buffer.
     *
     * Pure virtual function that subclasses must implement to extract
     * their specific data fields using the getXXXFromBuffer() methods.
     */
    virtual void decodeData() = 0;

    /**
     * @brief Executes the command's game logic.
     * @param player The player character who issued this command
     *
     * @note Subclasses should check player->actionPoints >= minAP before executing
     */
    virtual void performAction(Player *player) = 0;

    /**
     * @brief Creates a new empty instance of the same command type.
     * @return Shared pointer to new command instance
     *
     * Used by CommandFactory to create command instances from templates.
     */
    virtual auto clone() -> ClientCommandPointer = 0;

    /**
     * @brief Validates command data integrity and completeness.
     * @return True if all data was received, buffer fully consumed, and checksum matches
     *
     * Checks three conditions:
     * - No buffer overflow occurred during reading
     * - All expected bytes were consumed (bytesRetrieved == length)
     * - CRC checksum matches transmitted checksum
     */
    [[nodiscard]] auto isDataOk() const -> bool;

    /**
     * @brief Extracts a single unsigned byte from the buffer.
     * @return Byte value from current buffer position
     * @throws OverflowException if attempting to read beyond buffer bounds
     *
     * @note Advances buffer position and updates running CRC
     */
    auto getUnsignedCharFromBuffer() -> unsigned char;

    /**
     * @brief Extracts a length-prefixed string from the buffer.
     * @return String decoded from buffer (16-bit length prefix + UTF-8 bytes)
     *
     * Format: [uint16_t length][char[length] data]
     */
    auto getStringFromBuffer() -> std::string;

    /**
     * @brief Extracts a 32-bit signed integer from the buffer.
     * @return Integer value in big-endian byte order
     */
    auto getIntFromBuffer() -> int;

    /**
     * @brief Extracts a 16-bit signed integer from the buffer.
     * @return Short integer value in big-endian byte order
     */
    auto getShortIntFromBuffer() -> short int;

    /**
     * @brief Returns command payload length without header.
     * @return Number of bytes in message body
     */
    [[nodiscard]] auto getLength() const -> uint16_t { return length; }

    /**
     * @brief Returns minimum action points required to execute command.
     * @return AP threshold value
     */
    [[nodiscard]] inline auto getMinAP() const -> uint16_t { return minAP; }

    /**
     * @brief Returns timestamp when command was received.
     * @return Steady clock time point for latency/timing analysis
     */
    [[nodiscard]] inline auto getIncomingTime() const -> std::chrono::steady_clock::time_point { return incomingTime; }

    /**
     * @brief Records current time as command arrival timestamp.
     */
    inline void setReceivedTime() { incomingTime = std::chrono::steady_clock::now(); }

protected:
    bool dataOk = true; ///< True if data is valid; set to false on buffer overflow or checksum mismatch
    std::vector<unsigned char> msg_buffer{}; ///< Raw message bytes received from network
    uint16_t length = 0; ///< Payload length in bytes (excluding header)
    uint16_t bytesRetrieved = 0; ///< Number of bytes consumed from buffer during decoding
    uint16_t checkSum = 0; ///< CRC checksum from message header
    uint32_t crc = 0; ///< Running CRC checksum calculated during buffer reading

    uint16_t minAP; ///< Minimum action points required to execute this command
    std::chrono::steady_clock::time_point incomingTime; ///< Timestamp when command arrived
};

#endif
