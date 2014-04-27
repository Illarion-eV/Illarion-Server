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

#include <functional>
#include <thread>

#include "make_unique.hpp"
#include "Config.hpp"
#include "Logger.hpp"

#include "netinterface/NetInterface.hpp"

InitialConnection::InitialConnection() {
    std::thread servicethread(std::bind(&InitialConnection::run_service,this));
    servicethread.detach();
}


InitialConnection::TVECTORPLAYER &InitialConnection::get_Player_Vector() {
    return playerVector;
}

void InitialConnection::run_service() {
    int port = Config::instance().port;

    boost::asio::ip::tcp::endpoint endpoint = boost::asio::ip::tcp::endpoint(boost::asio::ip::tcp::v4(), port);
    acceptor = std::make_unique<boost::asio::ip::tcp::acceptor>(io_service,endpoint);
    auto newConnection = std::make_shared<NetInterface>(io_service);
    using std::placeholders::_1;
    acceptor->async_accept(newConnection->getSocket(), std::bind(&InitialConnection::accept_connection, this, newConnection, _1));
    Logger::info(LogFacility::Other) << "Starting the IO Service!" << Log::end;
    io_service.run();
}

//CInternetConnection* InitialConnection::accept_connection() {
void InitialConnection::accept_connection(std::shared_ptr<NetInterface> connection, const boost::system::error_code &error) {
    if (!error) {
        if (connection->activate()) {
            playerVector.push_back(connection);
        } else {
            Logger::error(LogFacility::Other) << "Error while activating connection!" << Log::end;
        }

        auto newConnection = std::make_shared<NetInterface>(io_service);
        using std::placeholders::_1;
        acceptor->async_accept(newConnection->getSocket(), std::bind(&InitialConnection::accept_connection, this, newConnection, _1));
    } else {
        Logger::error(LogFacility::Other) << "Could not accept connection: " << error.message() << Log::end;
    }
}



InitialConnection::~InitialConnection() {
    io_service.stop();
}
