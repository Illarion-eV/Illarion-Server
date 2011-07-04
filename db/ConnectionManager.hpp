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

#ifndef _CONNECTION_MANAGER_HPP_
#define _CONNECTION_MANAGER_HPP_

#include <string>

#include "Connection.hpp"

#include <boost/cstdint.hpp>

using std::string;

namespace Database {
class ConnectionManager;

class ConnectionManager {
private:
    static ConnectionManager instance;
    string connectionString;
    bool isOperational;

public:
    struct Login {
        string database;
        string user;
        string password;
        Login(string database, string user, string password) :
            database(database), user(user), password(password) {};
    };
    struct Server {
        string host;
        uint16_t port;
        Server(string host) : host(host), port(0) {};
        Server(string host, uint16_t port) : host(host), port(port) {};
    };

    static ConnectionManager &getInstance();
    void setupManager(const Login &login, const Server &server);
    PConnection getConnection() throw(std::logic_error);
    void releaseConnection(PConnection &conn);

private:
    ConnectionManager();
    ConnectionManager(const ConnectionManager &org);
    void addConnectionParameterIfValid(const string &param, const string &value);
    void addConnectionParameterIfValid(const string &param, const uint16_t value);
};
}

#endif
