//  illarionserver - server for the game Illarion
//  Copyright 2011 Illarion e.V.
//
//  This file is part of illarionserver.
//
//  illarionserver is free software: you can redistribute it and/or modify
//  it under the terms of the GNU General Public License as published by
//  the Free Software Foundation, either version 3 of the License, or
//  (at your option) any later version.
//
//  illarionserver is distributed in the hope that it will be useful,
//  but WITHOUT ANY WARRANTY; without even the implied warranty of
//  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//  GNU General Public License for more details.
//
//  You should have received a copy of the GNU General Public License
//  along with illarionserver.  If not, see <http://www.gnu.org/licenses/>.


#ifndef _CBASICCLIENTCOMMAND_HPP_
#define _CBASICCLIENTCOMMAND_HPP_

#include "netinterface/BasicCommand.hpp"
#include <stdint.h>
#include <iostream>
#include <boost/shared_ptr.hpp>
class Player;

/**
*@ingroup Netinterface
*class which holds a basic Client Command should be overloaded to define the different commands
*which can be received by the server
*/

/**
* simple exception on an overflow
*/
class OverflowException {};

class BasicClientCommand : public BasicCommand {
public:
    /**
    * Constructor of a basic command
    * @param defByte the initializing byte of the command
    */
    BasicClientCommand(unsigned char defByte);

    void setHeaderData(uint16_t mlength, uint16_t mcheckSum);

    virtual ~BasicClientCommand();

    /**
     * returns the data ptr for the command message
     **/
    unsigned char *msg_data();


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
    virtual boost::shared_ptr<BasicClientCommand> clone() = 0;

    /**
    * returns if the receiving of the command was sucessfull
    * @return true if the command was receuved complete and without problems
    */
    bool isDataOk();

    /**
    * reads an unsigned char from the local command buffer
    * @return the char which was found in the buffer
    */
    unsigned char getUnsignedCharFromBuffer() throw(OverflowException);

    /**
    * read a string from the local command buffer
    * @return the string which was found in the buffer
    */
    std::string getStringFromBuffer();

    /**
    * reads an int from the local command buffer
    * @return the int which was in the buffer (32 bit)
    */
    int getIntFromBuffer();

    /**
    * reads a short int from the local command buffer
    * @return the short int which was in the buffer (16 bit)
    */
    short int getShortIntFromBuffer();

    /**
     *returns the length of the command without the header in bytes
     */
    uint16_t getLength() {
        return length;
    }

protected:

    bool dataOk; /*<true if data is ok, will set to false if a command wants to read more data from the buffer as is in it, or if the checksum isn't the same*/
    unsigned char *msg_buffer;  /*< the current buffer for this command*/
    uint16_t length; /*< the length of this command */
    uint16_t bytesRetrieved; /*< how much bytes are currently decoded */
    uint16_t checkSum; /*< the checksum transmitted in the header*/
    uint32_t crc; /*< the checksum of the data*/

    //! no copy operator for pure virtual types
    BasicClientCommand &operator=(const BasicClientCommand &);
};

#endif
