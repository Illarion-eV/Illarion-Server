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

#ifndef CBASICSERVERCOMMAND_HPP
#define CBASICSERVERCOMMAND_HPP

#include "netinterface/BasicCommand.hpp"

#include <cstdint>
#include <memory>
#include <string>
#include <types.hpp>
#include <vector>

class BasicServerCommand;
/**
 * @brief Shared pointer type for server commands.
 */
using ServerCommandPointer = std::shared_ptr<BasicServerCommand>;

/**
 * @ingroup Netinterface
 * @brief Base class for commands sent from server to client.
 * 
 * BasicServerCommand handles serialization of outgoing messages with automatic:
 * - Header generation with command ID, length, and checksum
 * - Dynamic buffer resizing as data is added
 * - Type-safe data serialization (strings, ints, colors, etc.)
 * 
 * Each command consists of:
 * - Byte 0: Unique command ID
 * - Byte 1: Command ID XOR 0xFF (integrity check)
 * - Bytes 2-3: Payload length (big-endian uint16)
 * - Bytes 4-5: Checksum (sum of all data bytes mod 0xFFFF)
 * - Bytes 6+: Payload data
 * 
 * Usage pattern:
 * @code
 * BasicServerCommand cmd(CMD_ID);
 * cmd.addStringToBuffer("Hello");
 * cmd.addIntToBuffer(42);
 * cmd.addHeader(); // Finalize header with length and checksum
 * netInterface->addCommand(std::make_shared<BasicServerCommand>(std::move(cmd)));
 * @endcode
 * 
 * @note Command is movable but not copyable
 * @see BasicClientCommand for client-to-server commands
 * @see NetInterface for sending commands
 */
class BasicServerCommand : public BasicCommand {
public:
    /**
     * @brief Creates a server command with default buffer size (1000 bytes).
     * 
     * @param defByte The command ID
     */
    explicit BasicServerCommand(unsigned char defByte);

    /**
     * @brief Creates a server command with custom initial buffer size.
     * 
     * @param defByte The command ID
     * @param bsize Initial buffer size in bytes
     */
    BasicServerCommand(unsigned char defByte, uint16_t bsize);

    auto operator=(const BasicServerCommand &) -> BasicServerCommand & = delete;
    BasicServerCommand(const BasicServerCommand &) = delete;
    BasicServerCommand(BasicServerCommand &&) = default;
    auto operator=(BasicServerCommand &&) -> BasicServerCommand & = default;
    ~BasicServerCommand() = default;

    /**
     * @brief Gets the complete serialized command buffer.
     * 
     * @return Const reference to the byte buffer (header + payload)
     */
    [[nodiscard]] auto cmdData() const -> const std::vector<char> &;

    /**
     * @brief Gets the total command length in bytes.
     * 
     * @return Length including header and payload
     */
    [[nodiscard]] auto getLength() const -> int;

    /**
     * @brief Serializes a string to the buffer.
     * 
     * Writes null-terminated string data.
     * 
     * @param data String to serialize
     */
    void addStringToBuffer(const std::string &data);
    
    /**
     * @brief Serializes a 32-bit integer to the buffer (big-endian).
     * 
     * @param data Integer value
     */
    void addIntToBuffer(int data);
    
    /**
     * @brief Serializes a 16-bit integer to the buffer (big-endian).
     * 
     * @param data Short integer value
     */
    void addShortIntToBuffer(short int data);
    
    /**
     * @brief Serializes a single byte to the buffer.
     * 
     * @param data Byte value
     */
    void addUnsignedCharToBuffer(unsigned char data);
    
    /**
     * @brief Serializes a color value to the buffer.
     * 
     * @param c Color structure with RGB components
     */
    void addColourToBuffer(const Colour &c);

    /**
     * @brief Finalizes the header with length and checksum.
     * 
     * Must be called after all payload data has been added. Computes and writes
     * the payload length and checksum into the header bytes.
     */
    void addHeader();
    
    /**
     * @brief Initializes the header space in the buffer.
     * 
     * Reserves space for the 6-byte header at the start of the buffer.
     */
    void initHeader();

private:
    static constexpr uint16_t headerSize = 6; ///< Size of command header in bytes
    static constexpr uint16_t lengthPosition = 2; ///< Offset of length field in header
    static constexpr uint16_t crcPosition = 4; ///< Offset of checksum field in header
    static constexpr uint16_t defaultBufferSize = 1000; ///< Default initial buffer size
    uint16_t baseBufferSize = defaultBufferSize; ///< Base buffer size for resizing

    std::vector<char> buffer; ///< Serialized command data
    uint32_t checkSum = 0; ///< Running checksum of payload bytes

    uint16_t bufferPos = 0; ///< Current write position in buffer
    uint16_t bufferSizeMod = 1; ///< Buffer size multiplier (actual size = bufferSizeMod * baseBufferSize)

    /**
     * @brief Doubles the buffer size to accommodate more data.
     */
    void resizeBuffer();
};

#endif
