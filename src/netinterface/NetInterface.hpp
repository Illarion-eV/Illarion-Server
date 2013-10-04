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


#ifndef _CNETINTERFACE_HPP_
#define _CNETINTERFACE_HPP_

/**
*@ingroup Netinterface
*/


//include thread save vector
#include "tpvector.hpp"

#include "Connection.hpp"
#include "netinterface/BasicClientCommand.hpp"
#include "netinterface/BasicServerCommand.hpp"
#include "netinterface/CommandFactory.hpp"
#include <memory>
#include <boost/asio.hpp>
#include <deque>
#include <mutex>


/**
* @defgroup Netinterface Network Interface
* This is the interface for server client communication.
*/

/**
*@ingroup Netinterface
*class which holds the network interface and its thread for sending and receiving data
*/
class NetInterface : public std::enable_shared_from_this<NetInterface> {
public:

    /**
    * standard constructor, creates the two thread class receiver and sender
    */
    NetInterface(boost::asio::io_service &io_servicen);

    /**
    * destructor deletes all the classes which are constructed in the constructor
    * WARNING: shoudln't be called before the threads stopped correctly
    */
    ~NetInterface();

    void closeConnection(); /*<closes the connection to the client*/
    bool activate(); /*<activates the connection starts the sending and receiving threads*/
    bool nextInactive();

    /**
    * adds a command to the send queue so it will be sended correctly to the connection
    * @param command the command which should be added
    */
    void addCommand(const ServerCommandPointer &command);

    void shutdownSend(const ServerCommandPointer &command);

    std::string getIPAdress();

    /**
    * returns a command from the receive Queue if on is available
    * @return the command which was in the receive Queue or an empty (nullptr) Command if there wasn't something in the receive Queue
    */
    ClientCommandPointer getCommand();


    volatile bool online; /*< if connection is active*/

    /**
    * gets the number of received Commands
    * @return the number of commands which are currently in the received queue
    */
    uint16_t receivedSize() {
        return receiveQueue.size();
    }

    typedef std::deque<ClientCommandPointer> CLIENTCOMMANDLIST;
    typedef std::deque<ServerCommandPointer> SERVERCOMMANDLIST;

    boost::asio::ip::tcp::socket &getSocket() {
        return socket;
    }

private:


    void handle_read_header(const boost::system::error_code &error);
    void handle_read_data(const boost::system::error_code &error);

    void handle_write(const boost::system::error_code &error);
    void handle_write_shutdown(const boost::system::error_code &error);

    //Buffer for the header of messages
    unsigned char headerBuffer[6];

    ClientCommandPointer cmd;
    ServerCommandPointer shutdownCmd;
    ServerCommandPointer cmdToWrite;


    CLIENTCOMMANDLIST receiveQueue; /*<stores the commands which are received in a queue*/
    SERVERCOMMANDLIST sendQueue;

    std::string ipadress;

    boost::asio::ip::tcp::socket socket;

    //Factory für Commands vom Client
    CommandFactory commandFactory;
    uint16_t inactive;
    std::mutex sendQueueMutex;
    std::mutex receiveQueueMutex;



};


#endif

