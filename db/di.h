#ifndef __DI_HPP__INCLUDED__1543__
#define __DI_HPP__INCLUDED__1543__

#include <stdint.h>

#include <list>
#include <vector>
#include <string>
#include <stdexcept>
#include <sstream>
#include <iomanip>

// maximum of columns which can be selected/inserted in one select/insert call
// (this may be increased up to BOOST_PP_LIMIT_REPEAT
// but it slows down compilation considerably)
#define DATABASE_INTERFACE_SELECT_MAX_ARITY	40

namespace di {

//
// di exceptions
//

//! \brief all exceptions thrown by di:: are derived from this one
class exception: public std::runtime_error {
	public:
		exception(const std::string& _str):
		runtime_error(_str), query() {}
		exception(const std::string& _str, const std::string& _query):
		runtime_error(_str), query(_query) {}
		virtual ~exception() throw() {}

		virtual const char* what_query() const throw() { return query.c_str(); }

	protected:
		std::string query;
};

//! \brief connection problems normally cause this exception
//! (currently works only in postgres)
class connection_exception: public exception {
	public:
		connection_exception(const std::string& _str):
		exception(_str) {}
		connection_exception(const std::string& _str, const std::string& _query):
		exception(_str, _query) {}
		virtual ~connection_exception() throw() {}}
;

//
// connection and transaction
//

//! \brief holds all necessary information to create a database connection
//!
struct ConnectionInfo {
	//! the username
	std::string user;
	//! the password
	std::string password;
	//! the database name
	std::string database;
	//! the host where the server is running
	std::string host;
	//! the port the server is listening
	std::string port;

	//! \brief constructor
	ConnectionInfo(
		std::string _user = "",
		std::string _password = "",
		std::string _database = "",
		std::string _host = "",
		std::string _port = ""):
			user(_user),
			password(_password),
			database(_database),
			host(_host),
	port(_port) {}

	//! \brief destructor
	//!
	//! not virtual (because we won't derive)
	~ConnectionInfo() throw() {}}
;

//! \brief database connection wrapper
//!
class Connection {
	public:
		Connection(const ConnectionInfo& _con_info): con_info(_con_info) {}
		virtual ~Connection() throw() {}

		//! \brief execute query directly on connection
		//!
		//! this means: implicit commit on success
		//!
		//! \returns
		//!    the number of rows affected
		//!
		virtual int execImmediate(const std::string& query) = 0;

		//! \brief return connection info which was used to create connection
		virtual const ConnectionInfo& getConnectionInfo() { return con_info; }

		//! \brief returns true if the connection is good, false if not
		virtual bool isGood() = 0;

	protected:
		const ConnectionInfo con_info;

	private:
		//! make the class non-copyable
		Connection(const Connection&);
};

//! \brief database transaction wrapper
//!
//! holds the transaction and the connection
class Transaction {
	public:
		Transaction(Connection& _con): con(_con) {}
		virtual ~Transaction() throw() {}

		//! \brief get the corresponding connection
		virtual Connection& getConnection() { return con; }

		//! \brief generic commit
		virtual void commit() = 0;

		//! \brief generic rollback
		virtual void rollback() = 0;

		//! \brief generic rollback
		virtual inline void abort() { return rollback(); }

		//! \brief generic quoting
		//!
		//! quoting has to be implemented in derived classes
		//! therefore this function simply throws
		template< typename T >
		std::string inline quote(const T& val) {
			throw std::runtime_error("please call quote "
									 "from db specific transaction only!");
		}

		//! \brief generic sequence nextval
		//!
		//! quoting has to be implemented in derived classes
		//! therefore this function simply throws
		std::string inline formatNextval(const std::string& seqName) {
			throw std::runtime_error("please call formatNextval "
									 "from db specific transaction only!");
		}

	protected:
		Connection& con;

	private:
		//! \brief private constructor - make the class non-copyable
		Transaction(const Transaction&);
};

//
// column types
//
typedef int16_t				SmallInt;
typedef int32_t				Integer;
typedef int64_t				BigInt;
typedef bool 					Boolean;
typedef char					Char;
typedef std::string 	Varchar;
typedef float					Real;
typedef double				DoublePrecision;

//! \brief pseudo column type to represent plain sql which must not be quoted
struct UnquotedSQL {
	UnquotedSQL(const std::string& _str): str(_str) {}
	~UnquotedSQL() {}

	std::string str;
};

//! \brief pseudo column type to represent plain sql which must not be quoted
template< typename T >
struct NamedData {
	NamedData(T& _var, const std::string& _name): var(_var), name(_name) {}
	~NamedData() {}

	T& var;
	std::string name;
};

//! \brief helper function to quickly create NamedData object
template< typename T >
NamedData<T> Named(T& _var, const std::string& _name) {
	return NamedData<T>(_var, _name);
}

//! \brief holds a binary data object
struct Blob {
	std::vector< uint8_t > data;

	Blob(): data() {}
	Blob(const Blob& _blob): data(_blob.data) {}
	Blob(const std::vector< uint8_t >& _data): data(_data) {}
	~Blob() {}

	//! \brief converts the blob to a string
	operator std::string() {
		return std::string(data.begin(), data.end());
	}

	//! \brief converts the blob to a data vector
	operator std::vector< uint8_t > () {
		return data;
	}

	//! \brief converts the blob to a const data vector
	operator const std::vector< uint8_t > () const {
		return data;
	}

	//! \brief returns a reference to the internal data vector
	operator std::vector< uint8_t >& () {
		return data;
	}

	//! \brief returns const a reference to the internal data vector
	operator const std::vector< uint8_t >& () const {
		return data;
	}
};

//! \brief holds a date (year, month, day)
struct Date {
	Date(): day(0), month(0), year(0) {}
	Date(int _day, int _month, int _year): day(_day), month(_month), year(_year) {}
	~Date() {}

	int day;
	int month;
	int year;
};

//! \brief holds a time (hour, minute, second, fraction)
struct Time {
	Time(): hour(0), minute(0), second(0), fraction(0.0) {}
	Time(int _hour, int _minute, int _second, double _fraction=0.0):
	hour(_hour), minute(_minute), second(_second), fraction(_fraction) {}
	~Time() {}

	int hour;
	int minute;
	int second;
	double fraction;
};

//! \brief holds a timestamp (date + time)
struct Timestamp {
	Timestamp(): d(), t() {}
	Timestamp(const Date& _d, const Time& _t): d(_d), t(_t) {}
	~Timestamp() {}

	Date d;
	Time t;
};

//
// null holder
//

//! \brief holds a field and the bool if it is null or not
template< typename T >
struct isnull {
	isnull(T& _ref):
	ref(_ref), var(false) {}
	isnull(T& _ref, bool _b):
	ref(_ref), var(_b) {}
	~isnull() {}

	operator bool() { return var; }

	T& ref;
	bool var;
};

//! \brief holds a vector and the vector<bool> if the fields are null or not
template< typename T >
struct isnull_vector  {
	isnull_vector(T& _ref):
	ref(_ref), var() {}
	~isnull_vector() {}

	typedef typename T::value_type value_type;

	T& ref;
	std::vector< bool > var;
};

} // namespace di

namespace std {
// date output
inline std::ostream& operator<<(std::ostream& o, const di::Date& d) {
	return o << d.year << "-" << d.month << "-" << d.day;
}

// time output
inline std::ostream& operator<<(std::ostream& o, const di::Time& t) {
	return o << t.hour << ":" << t.minute << ":" << t.second << "+" << t.fraction;
}

// timestamp output
inline std::ostream& operator<<(std::ostream& o, const di::Timestamp& ts) {
	return o << ts.d << " " << ts.t;
}
}


#endif
