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

#include "InitialConnection.hpp"

#include "Config.hpp"
#include "Logger.hpp"
#include "netinterface/NetInterface.hpp"

#include <memory>
#include <thread>

auto InitialConnection::create() -> std::shared_ptr<InitialConnection> {
    std::shared_ptr<InitialConnection> ptr(new InitialConnection());
    std::thread servicethread([shared_this = ptr->shared_from_this()] { shared_this->run_service(); });
    servicethread.detach();
    return ptr;
}

auto InitialConnection::getNewPlayers() -> NewPlayerVector & { return newPlayers; }

void InitialConnection::run_service() {
    using boost::asio::ip::tcp;

    int port = Config::instance().port;

    auto endpoint = tcp::endpoint(tcp::v4(), port);
    acceptor = std::make_unique<tcp::acceptor>(io_service, endpoint);
    auto newConnection = std::make_shared<NetInterface>(io_service);
    using std::placeholders::_1;
    acceptor->async_accept(newConnection->getSocket(), [shared_this = shared_from_this(), newConnection](auto &&PH1) {
        shared_this->accept_connection(newConnection, PH1);
    });
    Logger::info(LogFacility::Other) << "Starting the IO Service!" << Log::end;
    io_service.run();
}

void InitialConnection::accept_connection(const std::shared_ptr<NetInterface> &connection,
                                          const boost::system::error_code &error) {
    if (!error) {
        if (connection->activate()) {
            newPlayers.push_back(connection);
        } else {
            Logger::error(LogFacility::Other) << "Error while activating connection!" << Log::end;
        }

        auto newConnection = std::make_shared<NetInterface>(io_service);
        using std::placeholders::_1;
        acceptor->async_accept(newConnection->getSocket(),
                               [shared_this = shared_from_this(), newConnection](auto &&PH1) {
                                   shared_this->accept_connection(newConnection, PH1);
                               });
    } else {
        Logger::error(LogFacility::Other) << "Could not accept connection: " << error.message() << Log::end;
    }
}

InitialConnection::~InitialConnection() { io_service.stop(); }
