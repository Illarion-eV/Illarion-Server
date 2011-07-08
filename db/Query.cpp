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

#include "db/Query.hpp"

#include <stdexcept>

using namespace Database;

Query::Query(const Query &org) {
    dbConnection = org.dbConnection;
    dbQuery = org.dbQuery;
}

Query::Query(const boost::shared_ptr<Connection> &connection) {
    dbConnection = connection;
}

Query::Query(const boost::shared_ptr<Connection> &connection, const std::string &query) {
    dbConnection = connection;
    dbQuery = query;
}

Result Query::execute() {
    if (dbConnection.get() == 0 || dbQuery.empty()) {
        throw new std::domain_error(
            "Connection and query is required to execute the query.");
    }

    bool ownTransaction = dbConnection->transactionActive();

    if (ownTransaction) {
        dbConnection->beginTransaction();
    }

    pqxx::transaction_base *trans = dbConnection->getTransaction();
    pqxx::result result = trans->exec(*dbQuery);

    if (ownTransaction) {
        dbConnection->commitTransaction();
    }

    return (Result) result;
}

void Query::setQuery(const std::string &query) {
    dbQuery = query;
}

Connection &getConnection() {
    return dbConnection;
}