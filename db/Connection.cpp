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

using namespace Database;

Connection::Connection() {
    internalConnection = 0;
    transaction = 0;
};

Connection::Connection(pqxx::connection *connection) {
    internalConnection = connection;
    transaction = 0;
}

Connection::~Connection() {
    if (internalConnection != 0) {
        internalConnection->disconnect();
        delete internalConnection;
    }
}

void Connection::beginTransaction() {
    if (internalConnection == 0) {
        throw new std::domain_error(
            "Transaction not possible while internal connection is not set.");
    }

    rollbackTransaction();
    transaction = new pqxx::transaction<>(*internalConnection);

};

void Connection::commitTransaction() {
    if (transaction != 0) {
        transaction->commit();
        delete transaction;
        transaction = 0;
    }
};

void Connection::rollbackTransaction() {
    if (transaction != 0) {
        transaction->abort();
        delete transaction;
        transaction = 0;
    }
};

pqxx::transaction_base *Connection::getTransaction() {
    return transaction;
}
