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

#include "Connection.hpp"

#include "db/SchemaHelper.hpp"

#include <iostream>
#include <chrono>
#include <thread>

#include <memory>
#include <pqxx/connection.hxx>
#include <pqxx/transaction.hxx>
#include <stdexcept>

using namespace Database;

Connection::Connection(const std::string &connectionString) {
    try {
        internalConnection = std::make_unique<pqxx::connection>(connectionString);
    } catch (const pqxx::broken_connection &e) {
        throw std::runtime_error("Failed to connect to the database: " + std::string(e.what()));
    } catch (const std::exception &e) {
        throw std::runtime_error("Unexpected error while initializing database connection: " + std::string(e.what()));
    }
}

void Connection::beginTransaction() {
    if (!internalConnection) {
        throw std::domain_error("Transaction not possible while internal connection is not set.");
    }

    rollbackTransaction();

    try {
        transaction = std::make_unique<pqxx::transaction<>>(*internalConnection);
    } catch (const pqxx::broken_connection &e) {
        throw std::runtime_error("Failed to begin transaction: " + std::string(e.what()));
    } catch (const std::exception &e) {
        throw std::runtime_error("Unexpected error while starting transaction: " + std::string(e.what()));
    }
}

void Connection::commitTransaction() {
    try {
        if (transaction) {
            transaction->commit();
            transaction.reset();
        }
    } catch (const pqxx::broken_connection &e) {
        throw std::runtime_error("Failed to commit transaction: " + std::string(e.what()));
    } catch (const std::exception &e) {
        throw std::runtime_error("Unexpected error during transaction commit: " + std::string(e.what()));
    }
}

void Connection::rollbackTransaction() {
    try {
        if (transaction) {
            transaction->abort();
            transaction.reset();
        }
    } catch (const pqxx::broken_connection &e) {
        std::cerr << "Warning: Failed to rollback transaction (connection issue): " << e.what() << std::endl;
    } catch (const std::exception &e) {
        std::cerr << "Warning: Unexpected error during transaction rollback: " << e.what() << std::endl;
    }
}

auto Connection::query(const std::string &query) -> pqxx::result {
    const int max_retries = 3;
    int retries = 0;

    while (retries < max_retries) {
        try {
            if (transaction) {
                return transaction->exec(query);
            }
            throw std::domain_error("No active transaction");
        } catch (const pqxx::broken_connection &e) {
            retries++;
            if (retries >= max_retries) {
                throw std::runtime_error("Database connection error after retries: " + std::string(e.what()));
            }
            std::cerr << "Retrying database query (" << retries << "/" << max_retries << "): " << e.what() << std::endl;
            std::this_thread::sleep_for(std::chrono::seconds(1)); // Wait before retrying
        } catch (const std::exception &e) {
            throw std::runtime_error("Unexpected error during query execution: " + std::string(e.what()));
        }
    }

    // Fallback return (though retries should handle most cases)
    return pqxx::result{};
}

auto Connection::streamTo(pqxx::table_path path, std::initializer_list<std::string_view> columns) -> pqxx::stream_to {
    if (transaction) {
        return pqxx::stream_to::table(*transaction, path, columns);
    }

    throw std::domain_error("No active transaction");
}
