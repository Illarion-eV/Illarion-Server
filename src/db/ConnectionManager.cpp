/*
 * Illarionserver - server for the game Illarion
 * Copyright 2011 Illarion e.V.
 *
 * This file is part of Illarionserver.
 *
 * Illarionserver  is  free  software:  you can redistribute it and/or modify it
 * under the terms of the  GNU Affero General Public License as published by the Free
 * Software Foundation, either version 3 of the License, or (at your option) any
 * later version.
 *
 * Illarionserver is distributed in the hope that it will be useful, but WITHOUT
 * ANY WARRANTY;  without  even  the  implied  warranty  of  MERCHANTABILITY  or
 * FITNESS FOR A PARTICULAR PURPOSE. See the GNU Affero General Public License for more
 * details.
 *
 * You should have received a copy of the GNU Affero General Public License along with
 * Illarionserver. If not, see <http://www.gnu.org/licenses/>.
 */

#include "db/ConnectionManager.hpp"

#include "Config.hpp"
#include "db/Connection.hpp"

#include <boost/cstdint.hpp>
#include <pqxx/connection.hxx>
#include <sstream>
#include <string>

using namespace Database;
using std::string;

ConnectionManager ConnectionManager::instance;

auto ConnectionManager::getInstance() -> ConnectionManager & { return ConnectionManager::instance; }

void ConnectionManager::setupManager() {
    connectionString = "";
    addConnectionParameterIfValid("user", Config::instance().postgres_user);
    addConnectionParameterIfValid("password", Config::instance().postgres_pwd);
    addConnectionParameterIfValid("dbname", Config::instance().postgres_db);
    addConnectionParameterIfValid("host", Config::instance().postgres_host);
    addConnectionParameterIfValid("port", std::to_string(Config::instance().postgres_port));
    isOperational = true;
}

auto ConnectionManager::getConnection() -> PConnection {
    if (!isOperational) {
        throw std::logic_error("Connection Manager is not set up yet");
    }

    auto connPtr = std::make_shared<Connection>(connectionString);
    return connPtr;
}

void ConnectionManager::addConnectionParameterIfValid(const string &param, const string &value) {
    if (!value.empty()) {
        connectionString += " " + param + "=" + value;
    }
}
