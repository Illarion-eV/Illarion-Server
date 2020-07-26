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

#ifndef CNETINTERFACE_HPP
#define CNETINTERFACE_HPP

/**
 *@ingroup Netinterface
 */

// include thread save vector
#include "netinterface/BasicClientCommand.hpp"
#include "netinterface/BasicServerCommand.hpp"
#include "netinterface/CommandFactory.hpp"
#include "thread_safe_vector.hpp"

#include <array>
#include <atomic>
#include <boost/asio.hpp>
#include <deque>
#include <memory>
#include <mutex>

class LoginCommandTS;

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
    explicit NetInterface(boost::asio::io_service &io_servicen);

    /**
     * destructor deletes all the classes which are constructed in the constructor
     * WARNING: shoudln't be called before the threads stopped correctly
     */
    ~NetInterface();

    NetInterface(const NetInterface &) = delete;
    auto operator=(const NetInterface &) -> NetInterface & = delete;
    NetInterface(NetInterface &&) = delete;
    auto operator=(NetInterface &&) -> NetInterface & = delete;

    void closeConnection(); /*<closes the connection to the client*/
    auto activate(Player * /*player*/ = nullptr)
            -> bool; /*<activates the connection starts the sending and receiving threads, if player == nullptr only
                        login command is accepted and processing stops afterwards*/
    auto nextInactive() -> bool;

    /**
     * adds a command to the send queue so it will be sended correctly to the connection
     * @param command the command which should be added
     */
    void addCommand(const ServerCommandPointer &command);

    void shutdownSend(const ServerCommandPointer &command);

    auto getIPAdress() -> std::string;

    std::atomic_bool online; /*< if connection is active*/

    using SERVERCOMMANDLIST = std::deque<ServerCommandPointer>;

    auto getSocket() -> boost::asio::ip::tcp::socket & { return socket; }

    auto getLoginData() const -> std::shared_ptr<LoginCommandTS> { return loginData; }

private:
    void handle_read_header(const boost::system::error_code &error);
    void handle_read_data(const boost::system::error_code &error);

    void handle_write(const boost::system::error_code &error);
    void handle_write_shutdown(const boost::system::error_code &error);

    // Buffer for the header of messages
    static constexpr auto headerSize = 6;
    static constexpr auto commandPosition = 0;
    static constexpr auto lengthPosition = 2;
    static constexpr auto crcPosition = 4;
    std::array<unsigned char, headerSize> headerBuffer;

    ClientCommandPointer cmd;
    ServerCommandPointer shutdownCmd;
    ServerCommandPointer cmdToWrite;

    SERVERCOMMANDLIST sendQueue;

    std::string ipadress;

    boost::asio::ip::tcp::socket socket;

    // Factory für Commands vom Client
    CommandFactory commandFactory;
    static constexpr auto maxInactive = 1000;
    uint16_t inactive;
    std::mutex sendQueueMutex;
    std::shared_ptr<LoginCommandTS> loginData;

    Player *owner;
};

#endif
