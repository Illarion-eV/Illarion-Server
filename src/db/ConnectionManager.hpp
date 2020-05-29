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

#ifndef _CONNECTION_MANAGER_HPP_
#define _CONNECTION_MANAGER_HPP_

#include <string>
#include <stdexcept>

#include <boost/cstdint.hpp>

#include "db/Connection.hpp"

using std::string;

namespace Database {
class ConnectionManager {
private:
    static ConnectionManager instance;
    string connectionString;
    bool isOperational;

public:
    static auto getInstance() -> ConnectionManager &;
    void setupManager();
    auto getConnection() -> PConnection;
private:
    ConnectionManager();
    ConnectionManager(const ConnectionManager &org) = delete;
    auto operator=(const ConnectionManager &org) -> ConnectionManager & = delete;
    void addConnectionParameterIfValid(const string &param, const string &value);
};
}

#endif
