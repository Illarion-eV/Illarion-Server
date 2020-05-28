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


#ifndef _CBASICSERVERCOMMAND_HPP_
#define _CBASICSERVERCOMMAND_HPP_

#include "netinterface/BasicCommand.hpp"
#include <string>
#include <stdint.h>
#include <vector>
#include <memory>
#include <types.hpp>

class BasicServerCommand;
using ServerCommandPointer = std::shared_ptr<BasicServerCommand>;

/**
*@ingroup Netinterface
*Class which represents a basic command that is sent from the
*server to the client. Each constructor prepares a 6 byte header as follows:
*- Byte 1: Unique command id
*- Byte 2: Byte 1 xor 0xFF (complement)
*- Byte 3+4: Length of the following data segment
*- Byte 5+6: Checksum consisting of the sum of all data bytes mod 0xFFFF
*
*Once all data has been added to the command, the header needs to be finalized with addHeader()
*/
class BasicServerCommand : public BasicCommand {
public:

    /**
    * Constructor which creates the server command.
    * In this case the internal data buffer is 1000 bytes large.
    * @param defByte The id of this command
    */
    explicit BasicServerCommand(unsigned char defByte);

    /**
    * Constructor which creates the server command.
    * @param defByte The id of this command
    * @param bsize The initial buffer size of this command
    */
    BasicServerCommand(unsigned char defByte, uint16_t bsize);

    BasicServerCommand &operator=(const BasicServerCommand &) = delete;
    BasicServerCommand (const BasicServerCommand &) = delete;    
    /**
    * Standard destructor
    */
    ~BasicServerCommand();

    /**
    * Function which returns the data buffer of the command.
    * @return The data buffer of the command
    */
    char *cmdData();

    /**
     * Returns the length of the command in bytes
     * @return The length of the command
     */
    int getLength() const;

    void addStringToBuffer(const std::string &data);
    void addIntToBuffer(int data);
    void addShortIntToBuffer(short int data);
    void addUnsignedCharToBuffer(unsigned char data);
    void addColourToBuffer(const Colour &c);

    /**
    * Adds all the header information to the top of the buffer
    * which depends on the commands data, like length and checksum
    */
    void addHeader();

private:
    uint16_t STDBUFFERSIZE; /*<the size of the standard buffer*/

    char *buffer;  /*<a pointer to the receive buffer*/
    uint32_t checkSum; /*<the checksum*/

    uint16_t bufferPos; /*<stores the current buffer position and the size of the used buffer*/
    uint16_t bufferSizeMod; /*<holds the current size of the buffer mod * stdbuffersize = current buffersize*/

    /**
    * if there is a buffer overflow this function creates a 2*STDBUFFERSIZE
    * large buffer and copies all the data into this new buffer and deletes the old one
    */
    void resizeBuffer();



};

#endif
