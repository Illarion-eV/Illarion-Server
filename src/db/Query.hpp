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
/**
 * @brief Base class for constructing and executing SQL queries.
 * 
 * Query provides a foundation for building SQL statements with automatic
 * connection management and transaction handling. Key features:
 * - Automatic connection acquisition from ConnectionManager
 * - Automatic transaction management (creates/commits if needed)
 * - SQL identifier escaping for table/column names
 * - Value quoting for safe SQL generation
 * - String list building utilities
 * 
 * Subclasses like SelectQuery, InsertQuery, UpdateQuery, and DeleteQuery
 * provide type-safe builders for specific SQL operations.
 * 
 * @note Query is movable but not copyable
 * @see SelectQuery
 * @see InsertQuery
 * @see UpdateQuery
 * @see DeleteQuery
 */
class Query {
private:
    PConnection dbConnection; ///< Shared database connection
    std::string dbQuery; ///< SQL query string

public:
    /**
     * @brief Creates a query with auto-acquired connection.
     * 
     * @param query SQL query string to execute
     */
    explicit Query(const std::string &query);
    
    /**
     * @brief Creates a query with specified connection.
     * 
     * @param connection Database connection to use
     * @param query SQL query string to execute
     */
    Query(PConnection connection, std::string query);
    
    Query(const Query &) = delete;
    Query(Query &&) = default;
    auto operator=(const Query &) -> Query & = delete;
    auto operator=(Query &&) -> Query & = default;
    virtual ~Query() = default;

    /**
     * @brief Escapes a SQL identifier (table/column name) with double quotes.
     * 
     * If already quoted, returns as-is. Otherwise wraps in double quotes.
     * 
     * @param key Identifier to escape
     * @return Quoted identifier string
     */
    static auto escapeKey(const std::string &key) -> std::string;
    
    /**
     * @brief Chains two identifiers with a dot (for schema.table or table.column).
     * 
     * Both identifiers are escaped before chaining.
     * 
     * @param key1 First identifier (schema or table)
     * @param key2 Second identifier (table or column)
     * @return Escaped and chained identifier string
     */
    static auto escapeAndChainKeys(const std::string &key1, const std::string &key2) -> std::string;
    
    /**
     * @brief Appends an entry to a comma-separated list.
     * 
     * Adds ", " separator if list is non-empty.
     * 
     * @param list String list to append to
     * @param newEntry Entry to add
     */
    static void appendToStringList(std::string &list, const std::string &newEntry);
    
    /**
     * @brief Quotes a value for safe SQL inclusion.
     * 
     * @tparam T Type of value to quote
     * @param value Value to quote
     * @return Properly escaped SQL literal
     */
    template <typename T> [[nodiscard]] auto quote(T value) const -> std::string {
        return dbConnection->quote<T>(value);
    }

    /**
     * @brief Executes the query and returns the result.
     * 
     * Automatically manages transaction lifecycle: begins transaction if none active,
     * executes query, and commits if it created the transaction.
     * 
     * @return Query result set
     * @throws std::domain_error if connection or query string is missing
     */
    virtual auto execute() -> Result;

protected:
    /**
     * @brief Protected constructor for subclasses with auto-acquired connection.
     */
    Query();
    
    /**
     * @brief Protected constructor for subclasses with specified connection.
     * 
     * @param connection Database connection to use
     */
    explicit Query(PConnection /*connection*/);

    /**
     * @brief Sets the SQL query string.
     * 
     * @param query SQL query string
     */
    void setQuery(const std::string &query);
    
    /**
     * @brief Gets the database connection.
     * 
     * @return Shared pointer to connection
     */
    auto getConnection() -> PConnection;
};
} // namespace Database

#endif
