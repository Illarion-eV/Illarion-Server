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

#include "db/ConnectionManager.hpp"
#include "db/Connection.hpp"

#include <sstream>
#include <string>
#include <stdexcept>

#include <boost/cstdint.hpp>
#include <pqxx/connection.hxx>

using namespace Database;

ConnectionManager ConnectionManager::instance;

PConnectionManager ConnectionManager::getInstance() {
    return &ConnectionManager::instance;
}

PConnection ConnectionManager::getConnection() {
    if (!isReady) {
        throw new std::domain_error("Connection Manager is yet not setup");
    }

    return new Connection(new pqxx::connection(*(connectionString)));
}

void ConnectionManager::releaseConnection(PConnection &conn) {
    delete conn;
    conn = 0;
}

ConnectionManager::ConnectionManager() {
    isReady = false;
};

ConnectionManager::ConnectionManager(const ConnectionManager &org) {
    throw new std::domain_error("Copy constructor not supported.");
}

void ConnectionManager::setupManager(const std::string &user, const std::string &password,
                                     const std::string &database, const std::string &host) {
    buildConnectionString(user, password, database, host, "");
}

void ConnectionManager::setupManager(const std::string &user, const std::string &password,
                                     const std::string &database, const std::string &hos^t,
                                     const uint16_t port) {
    std::stringstream ss;
    ss << port;
    std::string portString;
    portString = ss.str();
    buildConnectionString(user, password, database, host, portString);
}

void ConnectionManager::buildConnectionString(const std::string &user,
    const std::string &password, const std::string &database,
    const std::string &host, const std::string &port) {

    std::stringstream ss;
        
    if (user.size() > 0) {
        ss << " user=" << user << " ";
    }

    if (password.size() > 0) {
        ss << " password=" << password << " ";
    }

    if (database.size() > 0) {
        ss << " dbname=" << database << " ";
    }

    if (host.size() > 0) {
        ss << " host=" << host << " ";
    }

    if (port.size() > 0) {
        ss << " port=" << port << " ";
    }

    connectionString = new std::string(ss.str());
    isReady = true;
}
}
