//  illarionserver - server for the game Illarion
//  Copyright 2011 Illarion e.V.
//
//  This file is part of illarionserver.
//
//  illarionserver is free software: you can redistribute it and/or modify
//  it under the terms of the GNU General Public License as published by
//  the Free Software Foundation, either version 3 of the License, or
//  (at your option) any later version.
//
//  illarionserver is distributed in the hope that it will be useful,
//  but WITHOUT ANY WARRANTY; without even the implied warranty of
//  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//  GNU General Public License for more details.
//
//  You should have received a copy of the GNU General Public License
//  along with illarionserver.  If not, see <http://www.gnu.org/licenses/>.

#ifndef _CONNECTION_MANAGER_HPP_
#define _CONNECTION_MANAGER_HPP_

#include <string>

#include <dbng/Connection.hpp>

#include <boost/cstdint.hpp>

namespace Database {
class ConnectionManager;

typedef ConnectionManager *PConnectionManager;

class ConnectionManager {
private:
    /* Singleton instance */
    static ConnectionManager instance;

    /* Connection String that is used to establish a connection to the
     * postgre DB.
     */
    std::string *connectString;

    /* Ready flag. This is set true once the connection informations are
     * set.
     */
    bool isReady;

public:
    /* Get the singleton instance of this class. */
    static PConnectionManager getInstance(void);

    /* Get a new connection. This function throws a std::domain_error in
     * case the informations for the connection are not set yet.
     */
    PConnection getConnection(void);

    /* Release a connection. After this call the connection instance must
     * not be used anymore. Else a crash is certain.
     */
    void releaseConnection(const PConnection conn);

    /* Setup the connection informations to the postgre database. Once this
     * data is set its possible to establish connections to the database.
     */
    void setupManager(const std::string &user, const std::string &password,
                      const std::string &database, const std::string &host);
    void setupManager(const std::string &user, const std::string &password,
                      const std::string &database, const std::string &host,
                      const int32_t port);
private:
    ConnectionManager(void);
    ConnectionManager(const ConnectionManager &org);

};
}

#endif // _CONNECTION_MANAGER_HPP_
