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

#ifndef QUERY_HPP
#define QUERY_HPP

#include "db/Connection.hpp"
#include "db/Result.hpp"

#include <string>

namespace Database {
class Query {
private:
    PConnection dbConnection;
    std::string dbQuery;

public:
    explicit Query(const std::string &query);
    Query(PConnection connection, std::string query);
    Query(const Query &) = delete;
    Query(Query &&) = default;
    auto operator=(const Query &) -> Query & = delete;
    auto operator=(Query &&) -> Query & = default;
    virtual ~Query() = default;

    static auto escapeKey(const std::string &key) -> std::string;
    static auto escapeAndChainKeys(const std::string &key1, const std::string &key2) -> std::string;
    static void appendToStringList(std::string &list, const std::string &newEntry);
    template <typename T> [[nodiscard]] auto quote(T value) const -> std::string {
        return dbConnection->quote<T>(value);
    }

    virtual auto execute() -> Result;

protected:
    Query();
    explicit Query(PConnection /*connection*/);

    void setQuery(const std::string &query);
    auto getConnection() -> PConnection;
};
} // namespace Database

#endif
