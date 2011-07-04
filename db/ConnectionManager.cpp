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

#include "ConnectionManager.hpp"
#include "Connection.hpp"

#include <sstream>
#include <string>
#include <stdexcept>

#include <boost/cstdint.hpp>
#include <pqxx/connection.hxx>

using namespace Database;
using std::string;

ConnectionManager ConnectionManager::instance;

ConnectionManager &ConnectionManager::getInstance() {
    return ConnectionManager::instance;
}

void ConnectionManager::setupManager(const Login &login, const Server &server) {
    connectionString = "";
    addConnectionParameterIfValid("user", login.user);
    addConnectionParameterIfValid("password", login.password);
    addConnectionParameterIfValid("dbname", login.database);
    addConnectionParameterIfValid("host", server.host);
    addConnectionParameterIfValid("port", server.port);
    isOperational = true;
}

PConnection ConnectionManager::getConnection() throw(std::logic_error) {
    if (!isOperational) {
        throw new std::logic_error("Connection Manager is not set up yet");
    }

    return new Connection(new pqxx::connection(connectionString));
}

void ConnectionManager::releaseConnection(PConnection &conn) {
    delete conn;
    conn = 0;
}

ConnectionManager::ConnectionManager() {
    isOperational = false;
};

ConnectionManager::ConnectionManager(const ConnectionManager &org) {
    throw new std::domain_error("Copy constructor not supported.");
}

void ConnectionManager::addConnectionParameterIfValid(const string &param,
        const string &value) {
    if (value.size() > 0) {
        connectionString += " " + param + "=" + value;
    }
}

void ConnectionManager::addConnectionParameterIfValid(const string &param,
        const uint16_t value) {
    if (value > 0) {
        std::stringstream valuestring;
        valuestring << value;
        connectionString += " " + param + "=" + valuestring.str();
    }
}

