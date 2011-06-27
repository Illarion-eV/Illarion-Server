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


#include <iostream>
#include <sstream>
#include <locale>

#include "postgres.hpp"

namespace di {
namespace postgres {

//! enables tracing of sql queries
bool enable_trace_query = false;
//! enables tracing of data field conversions
bool enable_trace_conversion = false;
//! enables tracing of data tuples in selects
bool enable_trace_result = false;

Connection::Connection(const ConnectionInfo& _con_info):
		::di::Connection(_con_info),
		con_string(createConString()),
con() {
	DATABASE_INTERFACE_POSTGRES_TRY
	con = boost::shared_ptr<pqxx::connection>(
			  new pqxx::connection(con_string)
		  );
	DATABASE_INTERFACE_POSTGRES_CATCH_RETHROW
}

// in the destructor we must not rethrow -> no ATCH_RETHROW here
Connection::~Connection() throw() {
	try {
		con->deactivate();
		con->disconnect();
	} catch(const std::exception& e) {
		std::cerr << __PRETTY_FUNCTION__ << " caught " << e.what() << std::endl;
	}
}

std::string Connection::createConString() {
	DATABASE_INTERFACE_POSTGRES_TRY
	std::stringstream ss;
	if( con_info.user.size() > 0 ) {
		ss << " user=" << con_info.user << " ";
	}
	if( con_info.password.size() > 0 ) {
		ss << " password=" << con_info.password << " ";
	}
	if( con_info.database.size() > 0 ) {
		ss << " dbname=" << con_info.database << " ";
	}
	if( con_info.host.size() > 0 ) {
		ss << " host=" << con_info.host << " ";
	}
	if( con_info.port.size() > 0 ) {
		ss << " password=" << con_info.port << " ";
	}
	return ss.str();
	DATABASE_INTERFACE_POSTGRES_CATCH_RETHROW
}

int Connection::execImmediate(const std::string& query) {
	DATABASE_INTERFACE_POSTGRES_TRY
	// nontransaction means direct execution
	// nontransaction allows create/drop/vacuum ... statements
	pqxx::nontransaction trns(*con);
	pqxx::result res = trns.exec(query);
	return res.affected_rows();
	DATABASE_INTERFACE_POSTGRES_CATCH_RETHROW
}

bool Connection::isGood() {
	bool error = false;
	try {
		pqxx::transaction<pqxx::read_committed> trns(*con);
	} catch(...) {
		error = true;
	}
	return !error;
}

Transaction::Transaction(Connection& _con):
		::di::Transaction(_con),
		pqxxCon(_con),
trns() {
	DATABASE_INTERFACE_POSTGRES_TRY
	trns = boost::shared_ptr<pqxx_transaction>(
			   new pqxx_transaction(*(_con.con))
		   );
	trns->set_variable("DATESTYLE", "ISO");
	DATABASE_INTERFACE_POSTGRES_CATCH_RETHROW
}

Transaction::~Transaction() throw() {}

void Transaction::commit() {
	DATABASE_INTERFACE_POSTGRES_TRY
	trns->commit();
	DATABASE_INTERFACE_POSTGRES_CATCH_RETHROW
}

void Transaction::rollback() {
	DATABASE_INTERFACE_POSTGRES_TRY
	trns->abort();
	DATABASE_INTERFACE_POSTGRES_CATCH_RETHROW
}

void trace_result(pqxx::result& res) {
#ifdef DATABASE_INTERFACE_ENABLE_TRACE_RESULT
	DATABASE_INTERFACE_POSTGRES_TRY
	if( ::di::postgres::enable_trace_result ) {
		std::cerr << "POSTGRES_TRACE_RESULT: " << res.size() << " result rows, " <<
		res.affected_rows() << " affected rows" << std::endl;
		for(int i = 0; i < std::min(10, (int)res.size()); ++i ) {
			std::cerr << "POSTGRES_TRACE_RESULT(" << i << "): | ";
			for(unsigned int j = 0; j < res.at(i).size(); j++ ) {
				std::cerr << res.at(i).at(j).c_str() << " | ";
			}
			std::cerr << std::endl;
		}
	}
	DATABASE_INTERFACE_POSTGRES_CATCH_RETHROW
#endif
}

}
}
