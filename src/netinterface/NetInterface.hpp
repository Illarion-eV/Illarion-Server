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
 * This is the interface for server-client communication over TCP/IP.
 * 
 * The network layer handles:
 * - Asynchronous socket I/O using Boost.Asio
 * - Command serialization and deserialization
 * - Send and receive queues
 * - Connection lifecycle management
 * - Client authentication
 */

/**
 * @ingroup Netinterface
 * @brief Manages network communication with a single game client.
 * 
 * NetInterface handles asynchronous TCP/IP communication with one connected client,
 * providing:
 * - Asynchronous message reading and writing using Boost.Asio
 * - Command queue management for outgoing messages
 * - Header parsing and CRC validation
 * - Connection state tracking
 * - Player association after login
 * - Graceful connection shutdown
 * 
 * Each NetInterface instance corresponds to one active client connection. Messages
 * are structured with a 6-byte header containing command ID, length, and CRC,
 * followed by variable-length payload data.
 * 
 * The interface operates in two modes:
 * 1. Pre-login: Only accepts LoginCommandTS, connection closes after processing
 * 2. Active: Processes all commands and associates with a Player object
 * 
 * @note NetInterface is non-copyable and non-movable
 * @see Player for client game state
 * @see CommandFactory for command deserialization
 */
class NetInterface : public std::enable_shared_from_this<NetInterface> {
public:
    /**
     * @brief Creates a network interface for a new client connection.
     * 
     * @param io_servicen Boost.Asio I/O service for async operations
     */
    explicit NetInterface(boost::asio::io_service &io_servicen);

    /**
     * @brief Destructor that closes socket and clears queues.
     * 
     * @warning Should only be called after async operations have stopped
     */
    ~NetInterface();

    NetInterface(const NetInterface &) = delete;
    auto operator=(const NetInterface &) -> NetInterface & = delete;
    NetInterface(NetInterface &&) = delete;
    auto operator=(NetInterface &&) -> NetInterface & = delete;

    /**
     * @brief Marks the connection as closed, stopping further I/O.
     */
    void closeConnection();
    
    /**
     * @brief Activates the connection and begins async read operations.
     * 
     * If player is nullptr, only LoginCommandTS is accepted and the connection
     * closes after receiving it. Otherwise, all commands are processed and
     * forwarded to the player.
     * 
     * @param player The Player object to associate with this connection (nullptr for login-only mode)
     * @return true if activation successful, false on error
     */
    auto activate(Player * /*player*/ = nullptr) -> bool;
    
    /**
     * @brief Increments inactive counter and checks if connection timed out.
     * 
     * @return true if connection should be considered inactive/timed out
     */
    auto nextInactive() -> bool;

    /**
     * @brief Adds a server command to the outgoing send queue.
     * 
     * Commands are sent asynchronously in FIFO order.
     * 
     * @param command The command to send to the client
     */
    void addCommand(const ServerCommandPointer &command);

    /**
     * @brief Sends a final command and initiates graceful connection shutdown.
     * 
     * @param command The last command to send before closing
     */
    void shutdownSend(const ServerCommandPointer &command);

    /**
     * @brief Gets the client's IP address.
     * 
     * @return IP address string (e.g., "192.168.1.100")
     */
    auto getIPAdress() -> std::string;

    std::atomic_bool online; ///< true if connection is active and processing commands

    using SERVERCOMMANDLIST = std::deque<ServerCommandPointer>; ///< Type for outgoing command queue

    /**
     * @brief Gets the underlying TCP socket.
     * 
     * @return Reference to Boost.Asio socket
     */
    auto getSocket() -> boost::asio::ip::tcp::socket & { return socket; }

    /**
     * @brief Gets the login data received during authentication.
     * 
     * @return Shared pointer to login command (nullptr if not yet received)
     */
    auto getLoginData() const -> std::shared_ptr<LoginCommandTS> { return loginData; }

private:
    /**
     * @brief Async callback for header read completion.
     * 
     * @param error Error code from async read operation
     */
    void handle_read_header(const boost::system::error_code &error);
    
    /**
     * @brief Async callback for payload data read completion.
     * 
     * @param error Error code from async read operation
     */
    void handle_read_data(const boost::system::error_code &error);

    /**
     * @brief Async callback for normal write completion.
     * 
     * @param error Error code from async write operation
     */
    void handle_write(const boost::system::error_code &error);
    
    /**
     * @brief Async callback for shutdown write completion.
     * 
     * @param error Error code from async write operation
     */
    void handle_write_shutdown(const boost::system::error_code &error);

    static constexpr auto headerSize = 6; ///< Size of message header in bytes
    static constexpr auto commandPosition = 0; ///< Byte offset of command ID in header
    static constexpr auto lengthPosition = 2; ///< Byte offset of payload length in header
    static constexpr auto crcPosition = 4; ///< Byte offset of CRC checksum in header
    std::array<unsigned char, headerSize> headerBuffer; ///< Buffer for reading message headers

    ClientCommandPointer cmd; ///< Currently processing client command
    ServerCommandPointer shutdownCmd; ///< Command to send during shutdown
    ServerCommandPointer cmdToWrite; ///< Command currently being written to socket

    SERVERCOMMANDLIST sendQueue; ///< Queue of commands waiting to be sent

    std::string ipadress; ///< Client's IP address string

    boost::asio::ip::tcp::socket socket; ///< TCP socket for client connection

    CommandFactory commandFactory; ///< Factory for deserializing client commands
    static constexpr auto maxInactive = 1000; ///< Maximum inactive cycles before timeout
    uint16_t inactive; ///< Current inactive cycle counter
    std::mutex sendQueueMutex; ///< Mutex protecting sendQueue access
    std::shared_ptr<LoginCommandTS> loginData; ///< Login credentials received from client

    Player *owner; ///< Associated player object (nullptr during pre-login)
};

#endif
