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


#ifndef InitialConnection_HPP
#define InitialConnection_HPP

#include <memory>

#include <boost/asio.hpp>

#include "thread_safe_vector.hpp"
#include "Connection.hpp"

class NetInterface;

#define BACKLOG 10

class InitialConnection
    : public Connection,
      public std::enable_shared_from_this<InitialConnection> {

public:
    using NewPlayerVector = thread_safe_vector<std::shared_ptr<NetInterface>>;

    static auto create() -> std::shared_ptr<InitialConnection>;
    ~InitialConnection();

    auto getNewPlayers() -> NewPlayerVector &;

private:
    InitialConnection() = default;
    void run_service();

    boost::asio::io_service io_service;
    std::unique_ptr<boost::asio::ip::tcp::acceptor> acceptor = nullptr;

    void accept_connection(const std::shared_ptr<NetInterface> &connection,
                           const boost::system::error_code &error);

    NewPlayerVector newPlayers;
};

#endif
