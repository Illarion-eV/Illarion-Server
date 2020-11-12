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
class OverflowException {};

class BasicClientCommand;
using ClientCommandPointer = std::shared_ptr<BasicClientCommand>;

class BasicClientCommand : public BasicCommand {
public:
    /**
     * Constructor of a basic command
     * @param defByte the initializing byte of the command
     */
    BasicClientCommand(unsigned char defByte, uint16_t minAP = 0);

    void setHeaderData(uint16_t mlength, uint16_t mcheckSum);

    virtual ~BasicClientCommand() = default;

    auto operator=(const BasicClientCommand &) -> BasicClientCommand & = delete;
    BasicClientCommand(BasicClientCommand const &) = delete;
    BasicClientCommand(BasicClientCommand &&) = delete;
    auto operator=(BasicClientCommand &&) -> BasicClientCommand & = delete;

    /**
     * returns the data ptr for the command message
     **/
    auto msg_data() -> std::vector<unsigned char> &;

    /**
     * virtual function which should be overloaded in the concrete classes to get the data
     * of the command
     */
    virtual void decodeData() = 0;

    /**
     * performs the concrete action of the command
     * @param player the player which received the command
     */
    virtual void performAction(Player *player) = 0;

    /**
     * a copy function which returns an empty version of the command
     * (for usage of the commandFactory class
     */
    virtual auto clone() -> ClientCommandPointer = 0;

    /**
     * returns if the receiving of the command was sucessfull
     * @return true if the command was receuved complete and without problems
     */
    [[nodiscard]] auto isDataOk() const -> bool;

    /**
     * reads an unsigned char from the local command buffer
     * @return the char which was found in the buffer
     */
    auto getUnsignedCharFromBuffer() -> unsigned char;

    /**
     * read a string from the local command buffer
     * @return the string which was found in the buffer
     */
    auto getStringFromBuffer() -> std::string;

    /**
     * reads an int from the local command buffer
     * @return the int which was in the buffer (32 bit)
     */
    auto getIntFromBuffer() -> int;

    /**
     * reads a short int from the local command buffer
     * @return the short int which was in the buffer (16 bit)
     */
    auto getShortIntFromBuffer() -> short int;

    /**
     *returns the length of the command without the header in bytes
     */
    [[nodiscard]] auto getLength() const -> uint16_t { return length; }

    [[nodiscard]] inline auto getMinAP() const -> uint16_t { return minAP; }

    [[nodiscard]] inline auto getIncomingTime() const -> std::chrono::steady_clock::time_point { return incomingTime; }

    inline void setReceivedTime() { incomingTime = std::chrono::steady_clock::now(); }

protected:
    bool dataOk = true; /*<true if data is ok, will set to false if a command wants to read more data from the buffer as
                    is in it, or if the checksum isn't the same*/
    std::vector<unsigned char> msg_buffer{}; /*< the current buffer for this command*/
    uint16_t length = 0;                     /*< the length of this command */
    uint16_t bytesRetrieved = 0;             /*< how much bytes are currently decoded */
    uint16_t checkSum = 0;                   /*< the checksum transmitted in the header*/
    uint32_t crc = 0;                        /*< the checksum of the data*/

    uint16_t minAP; /*< number of ap necessary to perform command */
    std::chrono::steady_clock::time_point incomingTime;
};

#endif
