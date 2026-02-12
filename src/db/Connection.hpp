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

/**
 * @brief Shared pointer type for database connections.
 */
using PConnection = std::shared_ptr<Connection>;

/**
 * @brief Manages a PostgreSQL database connection and transaction lifecycle.
 * 
 * Connection wraps the libpqxx library to provide a simplified interface for
 * database operations. It handles:
 * - Connection establishment to PostgreSQL
 * - Transaction management (begin, commit, rollback)
 * - Query execution within transactions
 * - Bulk data streaming for efficient inserts
 * - SQL string escaping and quoting
 * 
 * All queries must be executed within an active transaction. The connection
 * automatically rolls back any uncommitted transactions.
 * 
 * @note This class is movable but not copyable
 * @see ConnectionManager for connection pooling
 */
class Connection {
private:
    std::unique_ptr<pqxx::connection> internalConnection = nullptr; ///< PostgreSQL connection handle
    std::unique_ptr<pqxx::transaction_base> transaction = nullptr; ///< Active transaction (nullptr if none)

public:
    /**
     * @brief Creates a database connection using a PostgreSQL connection string.
     * 
     * @param connectionString libpqxx connection string (e.g., "host=localhost dbname=illarion")
     * @throws pqxx::broken_connection if connection fails
     */
    explicit Connection(const std::string &connectionString);
    
    Connection(const Connection &) = delete;
    Connection(Connection &&) = default;
    auto operator=(const Connection &) -> Connection & = delete;
    auto operator=(Connection &&) -> Connection & = default;
    ~Connection() = default;

    /**
     * @brief Starts a new transaction, rolling back any existing transaction.
     * 
     * @throws std::domain_error if internal connection is not established
     */
    void beginTransaction();
    
    /**
     * @brief Executes a SQL query within the active transaction.
     * 
     * @param query SQL query string
     * @return Query result set
     * @throws std::domain_error if no transaction is active
     */
    auto query(const std::string &query) -> pqxx::result;
    
    /**
     * @brief Creates a stream for bulk data insertion.
     * 
     * Provides efficient bulk loading using PostgreSQL COPY protocol.
     * 
     * @param path Target table path
     * @param columns Column names for the insertion
     * @return pqxx::stream_to object for streaming data
     * @throws std::domain_error if no transaction is active
     */
    auto streamTo(pqxx::table_path path, std::initializer_list<std::string_view> columns) -> pqxx::stream_to;
    
    /**
     * @brief Commits the active transaction.
     * 
     * If no transaction is active, does nothing. Resets transaction to nullptr after commit.
     */
    void commitTransaction();
    
    /**
     * @brief Rolls back the active transaction.
     * 
     * If no transaction is active, does nothing. Resets transaction to nullptr after rollback.
     */
    void rollbackTransaction();

    /**
     * @brief Quotes and escapes a value for safe SQL inclusion.
     * 
     * @tparam T Type of value to quote (string, int, etc.)
     * @param t Value to quote
     * @return Properly escaped SQL string literal
     */
    template <typename T> [[nodiscard]] inline auto quote(const T &t) const -> std::string {
        return internalConnection->quote(t);
    }

    /**
     * @brief Checks if a transaction is currently active.
     * 
     * @return true if transaction is active, false otherwise
     */
    [[nodiscard]] inline auto transactionActive() const -> bool { return bool(transaction); }
};

} // namespace Database

#endif
