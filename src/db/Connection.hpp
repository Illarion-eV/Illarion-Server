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

#ifndef _CONNECTION_HPP_
#define _CONNECTION_HPP_

#include <memory>
#include <string>
#include <pqxx/connection.hxx>
#include <pqxx/transaction.hxx>

namespace Database {
class Connection;

using PConnection = std::shared_ptr<Connection>;

class Connection {
private:
    /* The libpgxx representation of the connection to the database. */
    std::unique_ptr<pqxx::connection> internalConnection = nullptr;
    std::unique_ptr<pqxx::transaction_base> transaction = nullptr;

public:
    explicit Connection(const std::string &connectionString);
    void beginTransaction();
    auto query(const std::string &query) -> pqxx::result;
    void commitTransaction();
    void rollbackTransaction();

    template<typename T> inline auto quote(const T &t) const -> std::string {
        return internalConnection->quote(t);
    }

    inline auto transactionActive() const -> bool {
        return bool(transaction);
    }

private:
    Connection(const Connection &org) = delete;
    auto operator=(const Connection &org) -> Connection & = delete;
};

}

#endif // _CONNECTION_HPP_
