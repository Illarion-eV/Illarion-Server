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


#include <vector>

// database interface
#include "di.hpp"
#include "postgres.hpp"

#include <boost/shared_ptr.hpp>
#include <boost/weak_ptr.hpp>
#include <boost/thread.hpp>

namespace currentdb = ::di::postgres;

//! \brief used to maintain a high number of connections and to reuse them
//!
//! \todo cleanup sweep
//!     (regularly check connections for 'isalive')
//! \todo reduce sweep
//!     (remove unused connections if there are too many of them)
class ConnectionManager {
public:
    //! the connection used for the database
    typedef currentdb::Connection Connection;
    //! a transaction used for the database
    typedef currentdb::Transaction Transaction;

public:
    //! \brief constructor replacement (factory function)
    //!
    //! \param user
    //!     the database user, may be ""
    //! \param password
    //!     the password of the user, may be ""
    //! \param database
    //!     the database for the connection, must not be ""!
    //! \param host
    //!     the hostname for the connection, may be "" for localhost
    //! \param port
    //!     the port for the connection, may be ""
    //!
    //! this function creates the initial connections
    //!
    static
    boost::shared_ptr<ConnectionManager> CreateConnectionManager(
        const std::string &user = "",
        const std::string &password = "",
        const std::string &database = "illarionserver",
        const std::string &host = "",
        const std::string &port = "",
        int initialConnectionCount = 3);

    //! \brief destructor - kills all used connections, frees all unused
    //!
    ~ConnectionManager() throw();

private:
    //! \brief constructor (private, construct with CreateConnectionManager)
    //!
    //! \param user
    //!     the database user, may be ""
    //! \param password
    //!     the password of the user, may be ""
    //! \param database
    //!     the database for the connection, must not be ""!
    //! \param host
    //!     the hostname for the connection, may be "" for localhost
    //! \param port
    //!     the port for the connection, may be ""
    //!
    ConnectionManager(
        const std::string &user = "",
        const std::string &password = "",
        const std::string &database = "illarionserver",
        const std::string &host = "",
        const std::string &port = "",
        int initialConnectionCount = 1);

    //! ConnectionManager must not be copy constructed !
    ConnectionManager(const ConnectionManager &) {}

public:
    //! \brief the holder of a pointer to a transactionholder
    //!
    //! this class really is a shared pointer pointing to a
    //! sub-transaction-holder and capable of casting itself
    //! to the lowlevel transaction held by the sub-holder
    //!
    class TransactionHolder {
    protected:
        //! \brief the holder of a ``used'' connection with a transaction
        //!
        //! this class releases the connection after leaving the scope
        //! this is the sub-holder-class of TransactionHolder
        //!
        class TransactionHolder_ {
        public:
            //! \brief constructor
            //!
            //! \param _trns
            //!     the transaction to hold (public)
            //! \param _con_mgr
            //!     the ConnectionManager where to release the connection
            //! \param _index
            //!     the index of this connection in the ConnectionManager
            //!
            TransactionHolder_(
                boost::shared_ptr<Transaction> _trns,
                boost::weak_ptr<ConnectionManager> _con_mgr,
                int _index);

            //! \brief destructor
            //!
            //! releases the connection in the ConnectionManager
            //!
            virtual ~TransactionHolder_() throw();

            //! \brief retrieves the Transaction
            //!
            //! \returns
            //!     the held Transaction
            //!
            boost::weak_ptr<Transaction> getTransaction() {
                return trns;
            }

            //! \brief casts itself to Transaction
            //!
            operator Transaction&() {
                return *trns.get();
            }

            //! \brief returns the index it was initialized with
            int getIndex() const {
                return index;
            }

        protected:
            //! the held transaction
            boost::shared_ptr<Transaction> trns;

            //! the manager of the connection
            //! this must be weak to allow destruction of the manager
            boost::weak_ptr<ConnectionManager> con_mgr;

            //! the index of the used connection in the manager
            int index;

        private:
            //! TransactionHolder_ must not be copy constructed !
            TransactionHolder_(const TransactionHolder_ &) {}}
        ;

    protected:
        boost::shared_ptr<TransactionHolder_> data;

    public:
        //! \brief constructor
        //!
        //! \param _trns
        //!     the transaction to hold (public)
        //! \param _con_mgr
        //!     the ConnectionManager where to release the connection
        //! \param _index
        //!     the index of this connection in the ConnectionManager
        //!
        TransactionHolder(
            boost::shared_ptr<Transaction> _trns,
            boost::weak_ptr<ConnectionManager> _con_mgr,
            int _index);

        //! \brief destructor
        //!
        virtual ~TransactionHolder();

        //! \brief casts itself to Transaction&
        //!
        operator Transaction&();

        //! \brief casts itself to Transaction&
        //!
        Transaction &get();

        //! \brief returns the index stored
        int getIndex() const;

        //! \brief forward the quoting to the db_interface transaction
        template< typename T >
        std::string quote(const T &var) {
            return data->operator Transaction&().quote(var);
        }

        //! \brief forward the quoting to the db_interface transaction
        std::string formatNextval(const std::string &seqName) {
            return data->operator Transaction&().formatNextval(seqName);
        }

        //! \brief forward the commit to the real transaction
        void commit() {
            data->operator Transaction&().commit();
        }

        //! \brief forward the rollback to the real transaction
        void rollback() {
            data->operator Transaction&().rollback();
        }

    };

    //! \brief create transaction on next free Connection
    //!
    //! ~TransactionHolder releases the Connection
    //!
    //! \returns
    //!     a TransactionHolder which can be used to retrieve a
    //!     transaction. if this TransactionHolder is destructed
    //!     the connection is released for next usage
    //!
    TransactionHolder getTransaction();

protected:
    //! the parameters given for connection creation
    di::ConnectionInfo con_info;

    //! the weak_ptr pointing to this manager
    boost::weak_ptr<ConnectionManager> this_weak;

protected:
    //! mutex for managing the connections
    boost::recursive_mutex mutex;

    //! the currently opened connections
    std::vector< boost::shared_ptr<Connection> > connections;
    //! the assigned transactions
    //! this must be weak to allow destruction of the transactions
    std::map< int, boost::weak_ptr<Transaction> > transactionMap;

    //! indexes of unused connections
    std::vector< int > unusedConnections;

protected:
    //! \brief creates amount new connections in connection pool
    //!
    //! \param amount
    //!     the amount of connections to be added
    void createConnections(int amount);

    //! \brief transaction has finished and connection can be reused
    //!
    //! indicates that the transaction from the transactionholder
    //! has been aborted/committed and that the connection can
    //! be set to the state 'unused'
    //!
    //! this function must not throw, it will be called from
    //! the TransactionHolder destructor!
    //!
    void doneTransaction(int index) throw();
};

typedef ConnectionManager::Connection ManagerConnection;
typedef ConnectionManager::Transaction Transaction;
typedef ConnectionManager::TransactionHolder TransactionHolder;

//! the connection manager used in the database interface
//! (it is threading-safe)
extern boost::shared_ptr<ConnectionManager> dbmgr;
//! and the connection manager for the account db
extern boost::shared_ptr<ConnectionManager> accdbmgr;
