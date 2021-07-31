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

#ifndef DB_CONNECTION_HPP
#define DB_CONNECTION_HPP

#include <memory>
#include <pqxx/connection.hxx>
#include <pqxx/stream_to.hxx>
#include <pqxx/transaction.hxx>
#include <string>

namespace Database {
class Connection;

using PConnection = std::shared_ptr<Connection>;

class Connection {
private:
    /* The libpqxx representation of the connection to the database. */
    std::unique_ptr<pqxx::connection> internalConnection = nullptr;
    std::unique_ptr<pqxx::transaction_base> transaction = nullptr;

public:
    explicit Connection(const std::string &connectionString);
    Connection(const Connection &) = delete;
    Connection(Connection &&) = default;
    auto operator=(const Connection &) -> Connection & = delete;
    auto operator=(Connection &&) -> Connection & = default;
    ~Connection() = default;

    void beginTransaction();
    auto query(const std::string &query) -> pqxx::result;
    auto streamTo(pqxx::table_path path, std::initializer_list<std::string_view> columns) -> pqxx::stream_to;
    void commitTransaction();
    void rollbackTransaction();

    template <typename T> [[nodiscard]] inline auto quote(const T &t) const -> std::string {
        return internalConnection->quote(t);
    }

    [[nodiscard]] inline auto transactionActive() const -> bool { return bool(transaction); }
};

} // namespace Database

#endif
