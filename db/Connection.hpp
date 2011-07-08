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

#ifndef _CONNECTION_HPP_
#define _CONNECTION_HPP_

#include <pqxx/connection.hxx>
#include <pqxx/transaction.hxx>

namespace Database {
class Connection;

typedef Connection *PConnection;  

class Connection {
private:
    /* The libpgxx representation of the connection to the database. */
    pqxx::connection *internalConnection;

    pqxx::transaction_base *transaction;

public:
    void beginTransaction(void);
    void commitTransaction(void);
    void rollbackTransaction(void);

    template<typename T> inline std::string quote(const T &t) {
        return internalConnection->quote(t);
    }

    inline bool transactionActive() {
        return (transaction != 0);
    }
private:
    /* The construction and destruction of the connections is handled by the
     * connection manager.
     */
    Connection(void);
    Connection(const Connection &org);
    Connection(pqxx::connection *connection);
    ~Connection(void);
    friend class ConnectionManager;

    /* The current transaction is required by the query class in order to
     * execute the query properly.
     */
    pqxx::transaction_base *getTransaction(void);
    friend class Query;
};
}

#endif // _CONNECTION_HPP_
