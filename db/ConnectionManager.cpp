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


#include "ConnectionManager.hpp"

#include <stdexcept>
#include <algorithm>
#include <iomanip>
#include <sstream>
#include <string>

boost::shared_ptr<ConnectionManager> dbmgr;
boost::shared_ptr<ConnectionManager> accdbmgr;

//! \brief constructor replacement (factory function)
//!
//! \param user
//! 	the database user, may be ""
//! \param password
//! 	the password of the user, may be ""
//! \param database
//! 	the database for the connection, must not be ""!
//! \param host
//! 	the hostname for the connection, may be "" for localhost
//! \param port
//! 	the port for the connection, may be ""
//!
//! this function creates the initial connections
//!
boost::shared_ptr<ConnectionManager>
ConnectionManager::CreateConnectionManager(
	const std::string& user,
	const std::string& password,
	const std::string& database,
	const std::string& host,
	const std::string& port,
	int initialConnectionCount) {
	// std::cerr << "creating Connection Manager with parameters: " <<
	// "user=" << user << ", password=" << password << ", database=" << database << ", host=" << host << ", port=" << port << std::endl;
	// construct
	boost::shared_ptr<ConnectionManager> conmgr(
		new ConnectionManager(user, password, database, host, port, initialConnectionCount)
	);

	// set weak pointer
	conmgr->this_weak = conmgr;

	// build initial connections
	conmgr->createConnections(initialConnectionCount);

	// get a db connection with transaction
	TransactionHolder trns = conmgr->getTransaction();

	return conmgr;
}


//! \brief constructor (private, construct with CreateConnectionManager)
//!
//! \param user
//! 	the database user, may be ""
//! \param password
//! 	the password of the user, may be ""
//! \param database
//! 	the database for the connection, must not be ""!
//! \param host
//! 	the hostname for the connection, may be "" for localhost
//! \param port
//! 	the port for the connection, may be ""
//!
ConnectionManager::ConnectionManager(
	const std::string& user,
	const std::string& password,
	const std::string& database,
	const std::string& host,
	const std::string& port,
	int initialConnectionCount):
con_info(user, password, database, host, port) {}

//! \brief destructor - kills all used connections, frees all unused
//!
//! rolls back all transactions currently open (with logging to stderr)
//!
ConnectionManager::~ConnectionManager() throw() {
	boost::recursive_mutex::scoped_lock lock(mutex); // protect

	std::cout << __PRETTY_FUNCTION__ << std::endl;

	// roll back all transactions
	std::map< int, boost::weak_ptr<Transaction> >::iterator itTrns;
	for( itTrns = transactionMap.begin(); itTrns != transactionMap.end(); ++itTrns ) {
		try {
			// may throw bad_weak_ptr
			boost::shared_ptr<Transaction> pt( itTrns->second.lock() );

			std::cerr << __PRETTY_FUNCTION__ << "rolling back transaction" << std::endl;
			pt->rollback();
		} catch(const boost::bad_weak_ptr&) {
			// that's ok
			std::cerr << __PRETTY_FUNCTION__ << "found dangling transaction" << std::endl;
		} catch(...) {
			// that's not ok but we can't do anything
			std::cerr << __PRETTY_FUNCTION__ << "unknown error in destructor" << std::endl;
		}
	}

	// clear transactionMap
	transactionMap.clear();

	// destroy all connections
	std::vector< boost::shared_ptr<Connection> >::iterator itCons;
	for( itCons = connections.begin(); itCons != connections.end(); ++itCons ) {
		try {
			itCons->reset();
		} catch(...) {
			// that's not ok but we can't do anything
			std::cerr << __PRETTY_FUNCTION__ << "caught ..." << std::endl;
		}
	}

	// clear connection vector
	connections.clear();
	unusedConnections.clear();
}

//! \brief creates amount new connections in connection pool
//!
//! \param amount
//! 	the amount of connections to be added
void ConnectionManager::createConnections(int amount) {
	boost::recursive_mutex::scoped_lock lock(mutex); // protect

	for( int atCon = 0; atCon != amount; ++atCon ) {
		bool created = false;
		// try to store the new connection on a position where a
		// previous connection was deleted
		for( unsigned int atExCon = 0; atExCon != connections.size(); ++atExCon ) {
			if( !connections.at(atExCon) ) {
				// found a position -> store here
				connections.at(atExCon) = boost::shared_ptr<Connection>(new Connection(con_info));
				unusedConnections.push_back(atExCon);
				created = true;
			}
		}
		if( !created ) {
			// did not find any position -> store at end
			connections.push_back( boost::shared_ptr<Connection>(new Connection(con_info)) );
			unusedConnections.push_back(connections.size() - 1);
		}
	}
}

//! \brief transaction has finished and connection can be reused
//!
//! indicates that the transaction from the transactionholder
//! has been aborted/committed and that the connection can
//! be set to the state 'unused'
//!
//! this function must not throw, it will be called from
//! the TransactionHolder destructor!
//!
void ConnectionManager::doneTransaction(int index) throw() {
	// protect marking connection as unused
	boost::recursive_mutex::scoped_lock lock(mutex);

	// mark as unused
	unusedConnections.push_back(index);
	transactionMap.erase(index);
}


//! \brief create transaction on next free Connection
//!
//! does check if the next free connection is good,
//! if not, this connection is dropped and another one used
//!
//! ~TransactionHolder releases the Connection
//!
//! \returns
//! 	a TransactionHolder which can be used to retrieve a
//! 	transaction. if this TransactionHolder is destructed
//! 	the connection is released for next usage
//!
TransactionHolder ConnectionManager::getTransaction() {
	boost::recursive_mutex::scoped_lock lock(mutex); // protect retrieval of new connection

	int index;
	boost::shared_ptr<Connection> used_con;

	// this loop is needed to remove connections
	// which got lost/destroyed from the pool
	do {
		// create a connection if there is no one left unused
		if( unusedConnections.size() == 0 ) {
			//std::cerr << "creating new connection" << std::endl;
			createConnections(1);
		}

		// get index of next unused connection
		index = unusedConnections.back();

		// use connection
		unusedConnections.pop_back();
		used_con = connections.at(index);

		// remove connection completely if it is not good
		if( !used_con->isGood() ) {
			std::cerr << "had to drop bad database connection!" << std::endl <<
			" (no harm done - creating another one)" << std::endl;
			connections.at(index).reset(); // sets to 0
			used_con.reset();
		}
	} while( !used_con );

	// create transaction
	Transaction* ptr = new Transaction(*used_con);
	boost::shared_ptr<Transaction> new_trns( ptr );

	boost::weak_ptr<Transaction> weak_trns = new_trns;
	transactionMap[index] = new_trns;

	// this transactionholder really is very tricky!
	return TransactionHolder(new_trns, this_weak, index);
}

//! \brief constructor
//!
//! \param _trns
//! 	the transaction to hold (public)
//! \param _con_mgr
//! 	the ConnectionManager where to release the connection
//! \param _index
//! 	the index of this connection in the ConnectionManager
//!
ConnectionManager::TransactionHolder::TransactionHolder_::TransactionHolder_(
	boost::shared_ptr<Transaction> _trns,
	boost::weak_ptr<ConnectionManager> _con_mgr,
	int _index):
trns(_trns), con_mgr(_con_mgr), index(_index) {}

//! \brief destructor
//!
//! releases the connection in the ConnectionManager
ConnectionManager::TransactionHolder::TransactionHolder_::~TransactionHolder_() throw() {
	try {
		// may throw boost::bad_weak_ptr
		boost::shared_ptr<ConnectionManager> pcon_mgr(con_mgr);

		// this calls ~trns -> weak pointer in transactionMap is invalid,
		// but that does not matter
		trns.reset();

		// unregister *this
		pcon_mgr->doneTransaction(this->index);
	} catch( const boost::bad_weak_ptr& ) {
		// that's ok, don't unregister
		std::cerr << __PRETTY_FUNCTION__ << "connection manager already destroyed" << std::endl;
	} catch(...) {
		// that's not ok, but we can't do anything
		std::cerr << __PRETTY_FUNCTION__ << "caught ..." << std::endl;
	}
}

ConnectionManager::TransactionHolder::TransactionHolder(
	boost::shared_ptr<Transaction> _trns,
	boost::weak_ptr<ConnectionManager> _con_mgr,
	int _index):
data(new TransactionHolder_(_trns, _con_mgr, _index)) {}

// destructor
ConnectionManager::TransactionHolder::~TransactionHolder() {}

// casts itself to the lowlevel transaction
ConnectionManager::TransactionHolder::operator Transaction&() {
	return *data;
}

// casts itself to the lowlevel transaction
Transaction& ConnectionManager::TransactionHolder::get() {
	return *data;
}

int ConnectionManager::TransactionHolder::getIndex() const {
	return data->getIndex();
}
