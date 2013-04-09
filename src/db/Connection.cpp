/*
 * Illarionserver - server for the game Illarion
 * Copyright 2011 Illarion e.V.
 *
 * This file is part of Illarionserver.
 *
 * Illarionserver  is  free  software:  you can redistribute it and/or modify it
 * under the terms of the  GNU  General  Public License as published by the Free
 * Software Foundation, either version 3 of the License, or (at your option) any
 * later version.
 *
 * Illarionserver is distributed in the hope that it will be useful, but WITHOUT
 * ANY WARRANTY;  without  even  the  implied  warranty  of  MERCHANTABILITY  or
 * FITNESS FOR A PARTICULAR PURPOSE. See the GNU General Public License for more
 * details.
 *
 * You should have received a copy of the GNU  General Public License along with
 * Illarionserver. If not, see <http://www.gnu.org/licenses/>.
 */

#include "Connection.hpp"

#include <stdexcept>
#include <pqxx/connection.hxx>
#include <pqxx/transaction.hxx>
#include "make_unique.hpp"

using namespace Database;

Connection::Connection(const std::string &connectionString) {
    internalConnection = std::make_unique<pqxx::connection>(connectionString);
}

void Connection::beginTransaction() {
    if (!internalConnection) {
        throw std::domain_error(
            "Transaction not possible while internal connection is not set.");
    }

    rollbackTransaction();
    transaction = std::make_unique<pqxx::transaction<>>(*internalConnection);
};

void Connection::commitTransaction() {
    if (transaction) {
        transaction->commit();
        transaction.reset();
    }
};

void Connection::rollbackTransaction() {
    if (transaction) {
        transaction->abort();
        transaction.reset();
    }
};

pqxx::result Connection::query(const std::string &query) {
    if (transaction) {
        return transaction->exec(query);
    }

    throw std::domain_error("No active transaction");
}

