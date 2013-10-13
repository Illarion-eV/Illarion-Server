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

#ifndef _QUERY_HPP_
#define _QUERY_HPP_

#include <string>

#include "db/Connection.hpp"
#include "db/Result.hpp"

namespace Database {
class Query {
private:
    PConnection dbConnection;
    std::string dbQuery;

public:
    Query(const std::string &query);
    Query(const PConnection connection, const std::string &query);

    static std::string escapeKey(const std::string &key);
    static std::string escapeAndChainKeys(const std::string &key1, const std::string &key2);
    static void appendToStringList(std::string &list, const std::string &newEntry);
    template <typename T> std::string quote(T value) const {
        return dbConnection->quote<T>(value);
    };

    virtual Result execute();

protected:
    Query();
    Query(const PConnection connection);
    Query(const Query &org) = delete;
    Query &operator=(const Query &org) = delete;

    void setQuery(const std::string &query);
    PConnection getConnection();
};
}

#endif // _QUERY_HPP_
