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


#ifndef __DI_POSTGRES_HPP__INCLUDED__
#define __DI_POSTGRES_HPP__INCLUDED__

#include <cmath>
#include <iostream>
#include <vector>
#include <string>
#include <stdexcept>
#include <boost/lexical_cast.hpp>
#include <boost/preprocessor/repetition/enum.hpp>
#include <boost/preprocessor/repetition/enum_params.hpp>
#include <boost/preprocessor/repetition/enum_binary_params.hpp>
#include <boost/preprocessor/repetition/repeat.hpp>
#include <boost/preprocessor/repetition/repeat_from_to.hpp>
#include <boost/preprocessor/control/if.hpp>
#include <boost/preprocessor/comparison/greater.hpp>
#include <boost/shared_ptr.hpp>

#include <pqxx/pqxx>

#include "di.hpp"

// start of configuration parameters //
// start of configuration parameters //
// start of configuration parameters //

// define this if you want your program to throw an exception
// if a field which is NULL should be stored into a value which
// cannot represent NULL (= in every class not being a isnull<> or isnull_vector<>
#define FATAL_NULL_STORAGE

// if this is defined every query is one line of output
// (if enable_trace_query is true)
#define DATABASE_INTERFACE_ENABLE_TRACE_QUERY

// if this is defined every converted field is one line of output
// (if enable_trace_conversion is true)
#define DATABASE_INTERFACE_ENABLE_TRACE_CONVERSION

// if this is defined every selected tuple is one line of output
// (if enable_trace_result is true)
#define DATABASE_INTERFACE_ENABLE_TRACE_RESULT

// this defines by how much a float value is multiplied before it is
// rounded and appended behind a . to represent a fraction
#define DATABASE_INTERFACE_TIME_PRECISION 100000.0

// end of configuration parameters //
// end of configuration parameters //
// end of configuration parameters //

// implements the deactivateable query tracing
#ifdef DATABASE_INTERFACE_ENABLE_TRACE_QUERY
# define DATABASE_INTERFACE_TRACE_QUERY(something) \
    if( ::di::postgres::enable_trace_query ) { \
        std::cerr << "POSTGRES_TRACE_QUERY:'" << something << "'" << std::endl; \
    }
#else
# define DATABASE_INTERFACE_TRACE_QUERY(something)
#endif

// implement the deactivateable fatal null storages
#ifdef FATAL_NULL_STORAGE
# define UNRESOLVABLE_NULL_STORAGE \
    throw std::runtime_error("tried to store NULL value into non-null-holder")
#else
# define UNRESOLVABLE_NULL_STORAGE \
    std::cerr << "tried to store NULL value into non-null-holder at:" <<  __PRETTY_FUNCTION__ << std::endl
#endif

// implements the deactivateable conversion tracing
#ifdef DATABASE_INTERFACE_ENABLE_TRACE_CONVERSION
# define DATABASE_INTERFACE_TRACE_CONVERSION(something) \
    if( ::di::postgres::enable_trace_conversion ) { \
        std::cerr << "POSTGRES_TRACE_CONVERSION:'" << something << "'" << std::endl; \
    }
#else
# define DATABASE_INTERFACE_TRACE_CONVERSION(something)
#endif

// try/catch for postgres exception -> di::exception transformation
#define DATABASE_INTERFACE_POSTGRES_TRY \
    try {
#define DATABASE_INTERFACE_POSTGRES_CATCH_RETHROW \
    } catch(const pqxx::broken_connection& e) { \
        throw di::connection_exception(e.what()); \
    } catch(const pqxx::sql_error& e) { \
        throw di::exception(e.what(), e.query()); \
    } catch(const std::exception& e) { \
        throw di::exception(e.what()); \
    }

//! \brief old pqxx sqlesc, needs to be replaced once we use pqxx3 properly
inline std::string oldsqlesc(std::string s) {
    std::string result;

    for (size_t i=0; i<s.length(); ++i) {
        const unsigned char c(s[i]);

        if (c & 0x80) {
            throw std::runtime_error("non-ASCII text passed to sqlesc(); "
                                     "the libpq version that libpqxx was built with does not support this "
                                     "yet (minimum is postgres 7.2)");
        } else if (isprint(c)) {
            if (c=='\\' || c=='\'') {
                result += c;
            }

            result += c;
        } else {
            char s[8];
            // TODO: Number may be formatted according to locale!  :-(
            sprintf(s, "\\%03o", static_cast<unsigned int>(c));
            result.append(s, 4);
        }
    }

    return result;
}


namespace di {
namespace postgres {

namespace hidden {
template< typename T > std::string quote(const T &val);
template< typename T > std::string quote(const isnull<T>& val);
template<> inline std::string quote(const int64_t &val);
template<> inline std::string quote(const UnquotedSQL &val);
template<> inline std::string quote(const Date &_cv);
template<> inline std::string quote(const Time &_cv);
template<> inline std::string quote(const Timestamp &_cv);
template<> inline std::string quote(const Blob &blob);
}

////
//// postgres specific types
////
typedef di::SmallInt                    Int2;
typedef di::Integer                     Int4;
typedef di::BigInt                      Int8;
typedef di::Varchar                     Text;
typedef di::Blob                            Bytea;
typedef di::DoublePrecision     Float;
typedef uint32_t                            Oid;

////
//// postgres-specific connection and transaction
////
extern bool enable_trace_query;
extern bool enable_trace_conversion;
extern bool enable_trace_result;

//! \brief postgres specific connection
class Transaction;
class Connection: public ::di::Connection {
    friend class di::postgres::Transaction;
public:
    Connection(const ConnectionInfo &_con_info);
    virtual ~Connection() throw();

    virtual std::string createConString();

    virtual int execImmediate(const std::string &query);

    virtual bool isGood();

protected:
    const std::string con_string;
    boost::shared_ptr<pqxx::connection> con;
};

// this type is used in all ::di::postgres::exec functions
typedef pqxx::dbtransaction transaction_parent;

// this type is used in creation of transactions
typedef pqxx::transaction<pqxx::read_committed> pqxx_transaction;

//! \brief postgres specific transaction
class Transaction: public ::di::Transaction {
public:
    Transaction(Connection &_con);
    virtual ~Transaction() throw();

    virtual void commit();
    virtual void rollback();

    virtual operator pqxx_transaction&() {
        return *trns;
    }

public:
    //! \brief common 'gateway' to global functions
    template< typename T > std::string quote(const T &val) {
        return ::di::postgres::hidden::quote(val);
    }

    //! \brief common 'gateway' with naming the quoteholder
    template< typename T > std::string quote(
        const di::NamedData<T>& container) {
        return ::di::postgres::hidden::quote(container.var);
    }

    //! \brief wrapper for selecting next value from sequence
    std::string inline formatNextval(const std::string &seqName) {
        return std::string("nextval('") + seqName + std::string("')");
    }

    Connection &pqxxCon;
    boost::shared_ptr<pqxx_transaction> trns;
};

////
//// query execute functions
////
namespace hidden {

//! \brief quoting with the simplest possible method via pqxx
template< typename T > std::string quote(const T &val) {
    std::stringstream ss;
    ss << val;
    std::stringstream ss2;
    ss2 << "'" << oldsqlesc(ss.str()) << "'";
    return ss2.str();
}
//! \brief quoting NULL values
template< typename T > std::string quote(const isnull<T>& val) {
    if (val.var == true) {
        return "NULL";
    } else {
        return quote(val.ref);
    }
}
//! \brief quoting special: int64_t (long long from ice)
template<> inline std::string quote(const int64_t &val) {
    std::stringstream ss;
    ss << static_cast<int32_t>(val);
    return oldsqlesc(ss.str());
}
//! \brief quoting special: unquoted sql
template<> inline std::string quote(const UnquotedSQL &val) {
    return val.str;
}
//! \brief quoting special: date
template<> inline std::string quote(const Date &_cv) {
    std::stringstream ss;
    ss << "'" << _cv.year << "-" << _cv.month << "-" << _cv.day << "'";
    return ss.str();
}
//! \brief quoting special: time
template<> inline std::string quote(const Time &_cv) {
    std::stringstream ss;
    ss << "'" << _cv.hour << ":" << _cv.minute << ":" << _cv.second << "." <<
       static_cast<int>(floor(_cv.fraction*DATABASE_INTERFACE_TIME_PRECISION + 0.5)) << "'";
    return ss.str();
}
//! \brief quoting special: timestamp
template<> inline std::string quote(const Timestamp &_cv) {
    std::stringstream ss;
    ss << "'" << _cv.d.year << "-" << _cv.d.month << "-" << _cv.d.day << " " <<
       _cv.t.hour << ":" << _cv.t.minute << ":" << _cv.t.second << "." <<
       static_cast<int>(floor(_cv.t.fraction*DATABASE_INTERFACE_TIME_PRECISION + 0.5)) << "'";
    return ss.str();
}
//! \brief quoting Blob
template<> inline std::string quote(const Blob &blob) {
    const std::vector< uint8_t >& data = blob.data;
    std::stringstream ss;
    ss.fill('0');
    ss << "decode('";

    for (size_t l = 0; l < data.size(); l++) {
        ss << std::setw(2) << std::hex << (unsigned int)(data[l]);
    }

    ss << "','hex')";
    return ss.str();
}

} // namespace ::di::postgres::hidden

//
// extract from field into reference
//

//! \brief extracting data: the simple and sometimes working default (use pqxx)
template< typename SOURCE >
void ExtractField(const pqxx::result::field &f, SOURCE &var) {
    f.to(var);
}

//! \brief extracting data: 64bit (important for ::Ice::Long values)
template<> inline
void ExtractField(const pqxx::result::field &f, int64_t &var) {
    std::stringstream ss;
    std::string s;
    f.to(s);
    ss << s;
    ss >> var;
}

//! \brief extracting data: char
//! (important for single chars, they are not supported by pqxx)
template<> inline
void ExtractField(const pqxx::result::field &f, char &var) {
    std::string s;
    f.to(s);

    if (s.size() > 0) {
        var = s[0];
    }
}

//! \brief extracting data: Date
template<> inline
void ExtractField<Date>(const pqxx::result::field &f, Date &var) {
    std::string s;
    bool isnull = !f.to(s);

    if (isnull) {
        UNRESOLVABLE_NULL_STORAGE;
    } else {
        var.year = boost::lexical_cast<int>(s.substr(0,4));
        var.month = boost::lexical_cast<int>(s.substr(5,2));
        var.day = boost::lexical_cast<int>(s.substr(8,2));
    }
}

//! \brief extracting data: Timestamp
template<> inline
void ExtractField<Timestamp>(const pqxx::result::field &f, Timestamp &var) {
    std::string s;
    bool isnull = !f.to(s);

    if (isnull) {
        UNRESOLVABLE_NULL_STORAGE;
    } else {
        var.d.year = boost::lexical_cast<int>(s.substr(0,4));
        var.d.month = boost::lexical_cast<int>(s.substr(5,2));
        var.d.day = boost::lexical_cast<int>(s.substr(8,2));
        var.t.hour = boost::lexical_cast<int>(s.substr(11,2));
        var.t.minute = boost::lexical_cast<int>(s.substr(14,2));
        var.t.second = boost::lexical_cast<int>(s.substr(17,2));
        std::string str;

        if (s.find("+") >= s.size()) {
            str = s.substr(19,s.size()-19);
        } else {
            str = s.substr(19,s.size()-22);
        }

        if (str.size() > 0) {
            var.t.fraction = boost::lexical_cast<float>(str);
        } else {
            var.t.fraction = 0;
        }
    }
}

//! \brief extracting data: Time
template<> inline
void ExtractField<Time>(const pqxx::result::field &f, Time &var) {
    std::string s;
    bool isnull = !f.to(s);

    if (isnull) {
        UNRESOLVABLE_NULL_STORAGE;
    } else {
        var.hour = boost::lexical_cast<int>(s.substr(0,2));
        var.minute = boost::lexical_cast<int>(s.substr(3,2));
        var.second = boost::lexical_cast<int>(s.substr(6,2));
        std::string str = s.substr(9,s.size()-12);
        var.fraction = boost::lexical_cast<float>(str);
    }
}

//! \brief extracting data: Blob
template<> inline
void ExtractField<Blob>(const pqxx::result::field &f, Blob &var) {
    std::string s;
    bool isnull = !f.to(s);

    if (isnull) {
        UNRESOLVABLE_NULL_STORAGE;
    } else {
        std::stringstream ss;
        unsigned int i;

        for (i = 0; i < s.size(); ++i) {
            ss << s[i];

            if ((i % 2) == 1) {
                ss << ' ';
            }
        }

        unsigned int c;
        const unsigned int size = s.size()/2;
        var.data.reserve(size); // allocate all memory at once (saves much time!)

        for (i = 0; i < size; ++i) {
            ss >> std::setw(2) >> std::hex >> c;
            var.data.push_back(c);
        }
    }
}


//! \brief extracting data into variables: different types between column
//!     source and target reference (no null storage)
//!
//! extract value and copy-construct to target type
template< typename SOURCE, typename TARGET >
struct StoreField {
    static void store(const pqxx::result::field &f, TARGET &target) {
        DATABASE_INTERFACE_TRACE_CONVERSION(f.c_str());

        if (f.is_null()) {
            UNRESOLVABLE_NULL_STORAGE;
        }

        SOURCE temp;
        ExtractField(f, temp);

        target = TARGET(temp);
    }
};
//! \brief extracting data into variables: different types between column
//!     source and target reference (with null storage)
//!
//! extract value and copy-construct to target type
template< typename SOURCE, typename TARGET >
struct StoreField< SOURCE, isnull<TARGET> > {
    static void store(const pqxx::result::field &f, isnull<TARGET>& target) {
        DATABASE_INTERFACE_TRACE_CONVERSION(f.c_str());

        if (f.is_null()) {
            target.var = true;
        } else {
            target.var = false;
            SOURCE temp;
            ExtractField(f, temp);

            target.ref = TARGET(temp);
        }
    }
};
//! \brief extracting data into variables: same types between column
//!     source and target reference (without null storage)
template< typename TARGET >
struct StoreField< TARGET, TARGET> {
    static void store(const pqxx::result::field &f, TARGET &target) {
        DATABASE_INTERFACE_TRACE_CONVERSION(f.c_str());

        if (f.is_null()) {
            UNRESOLVABLE_NULL_STORAGE;
        }

        ExtractField(f, target);
    }
};


//! \brief extracting data into variables: different types between column
//!     source and target reference (no null storage)
//!
//! extract value and copy-construct to target type
template< typename SOURCE, typename TARGET, typename VALUETYPE >
struct StoreFieldAppend {
    static void store(const pqxx::result::field &f, TARGET &target) {
        DATABASE_INTERFACE_TRACE_CONVERSION(f.c_str());

        if (f.is_null()) {
            UNRESOLVABLE_NULL_STORAGE;
        }

        SOURCE temp;
        std::back_insert_iterator<TARGET> it(target);
        ExtractField(f, temp);

        *it = VALUETYPE(temp);
    }
};
//! \brief extracting data into variables: different types between column
//!     source and target reference (with null storage)
//!
//! extract value and copy-construct to target type
template< typename SOURCE, typename TARGET, typename VALUETYPE >
struct StoreFieldAppend< SOURCE, isnull_vector<TARGET>, VALUETYPE > {
    static void store(const pqxx::result::field &f, isnull_vector<TARGET>& target) {
        DATABASE_INTERFACE_TRACE_CONVERSION(f.c_str());

        if (f.is_null()) {
            target.var.push_back(true);
            target.ref.push_back(VALUETYPE());
        } else {
            target.var.push_back(false);

            SOURCE temp;
            std::back_insert_iterator<TARGET> it(target.ref);
            ExtractField(f, temp);

            *it = VALUETYPE(temp);
        }
    }
};

//! \brief debugging function
void trace_result(pqxx::result &res);

//! \brief termination function for ApplyResultOne
template< int field >
void ApplyResultOne(pqxx::result &res) {}

//! \brief apply the current field of the result
//!    to the first destination storage variable
#define DATABASE_INTERFACE_CREATE_TEMPLATE_SOURCE(z,para,data) \
    , SOURCE ## para
#define DATABASE_INTERFACE_CREATE_TEMPLATE_PARAM(z,para,data) \
    , target ## para
#define DATABASE_INTERFACE_CREATE_TEMPLATE(z,params,data) \
    template< int field, \
    BOOST_PP_ENUM_PARAMS(params, typename SOURCE), \
    BOOST_PP_ENUM_PARAMS(params, typename TARGET) > \
    void ApplyResultOne(pqxx::result& res, \
                        BOOST_PP_ENUM_BINARY_PARAMS(params, TARGET, & target)) { \
        StoreField< SOURCE0, TARGET0 >::store(res.at(0).at(field), target0); \
        ApplyResultOne<field+1 \
        BOOST_PP_REPEAT_FROM_TO(1,params,DATABASE_INTERFACE_CREATE_TEMPLATE_SOURCE,0) \
        >(res BOOST_PP_REPEAT_FROM_TO(1,params,DATABASE_INTERFACE_CREATE_TEMPLATE_PARAM,0)); \
    }
BOOST_PP_REPEAT_FROM_TO(1, DATABASE_INTERFACE_SELECT_MAX_ARITY, DATABASE_INTERFACE_CREATE_TEMPLATE, 0)
#undef DATABASE_INTERFACE_CREATE_TEMPLATE
#undef DATABASE_INTERFACE_CREATE_TEMPLATE_SOURCE
#undef DATABASE_INTERFACE_CREATE_TEMPLATE_PARAM

//! \brief postgres specific select for single row
//!
//! usage:
//! select< coltype1, coltype2 >( trns, targetref1, targetref2, query )
//!
//! \returns
//!     number of rows in result
#define DATABASE_INTERFACE_CREATE_TEMPLATE(z,params,data) \
    template< \
    BOOST_PP_ENUM_PARAMS(params, typename SOURCE), \
    BOOST_PP_ENUM_PARAMS(params, typename TARGET) > \
    int select( \
                ::di::postgres::transaction_parent& trns, \
                BOOST_PP_ENUM_BINARY_PARAMS(params, TARGET, & _param), \
                std::string& query) { \
        DATABASE_INTERFACE_TRACE_QUERY(query); \
        pqxx::result res = trns.exec(query); \
        trace_result(res); \
        if( res.size() > 0 ) \
            ApplyResultOne< 0, BOOST_PP_ENUM_PARAMS(params, SOURCE) >( \
                    res, \
                    BOOST_PP_ENUM_PARAMS(params, _param)); \
        return res.size(); \
    }
BOOST_PP_REPEAT_FROM_TO(1, DATABASE_INTERFACE_SELECT_MAX_ARITY, DATABASE_INTERFACE_CREATE_TEMPLATE, 0)
#undef DATABASE_INTERFACE_CREATE_TEMPLATE

//! \brief termination function for ApplyResultAll
template< int field >
void ApplyResultAll(pqxx::result &res) {}

//! \brief apppend the current fields of the result
//!    to the first destination storage variable array
#define DATABASE_INTERFACE_CREATE_TEMPLATE_SOURCE(z,para,data) \
    , SOURCE ## para
#define DATABASE_INTERFACE_CREATE_TEMPLATE_PARAM(z,para,data) \
    , target ## para
#define DATABASE_INTERFACE_CREATE_TEMPLATE(z,params,data) \
    template< int field, \
    BOOST_PP_ENUM_PARAMS(params, typename SOURCE), \
    BOOST_PP_ENUM_PARAMS(params, typename TARGET) > \
    void ApplyResultAll(pqxx::result& res, \
                        BOOST_PP_ENUM_BINARY_PARAMS(params, TARGET, & target)) { \
        for( pqxx::result::const_iterator it = res.begin(); it != res.end(); ++it ) { \
            StoreFieldAppend< SOURCE0, TARGET0, typename TARGET0::value_type >::store(it->at(field), target0); \
        } \
        ApplyResultAll<field+1 \
        BOOST_PP_REPEAT_FROM_TO(1,params,DATABASE_INTERFACE_CREATE_TEMPLATE_SOURCE,0) \
        >(res BOOST_PP_REPEAT_FROM_TO(1,params,DATABASE_INTERFACE_CREATE_TEMPLATE_PARAM,0)); \
    }
BOOST_PP_REPEAT_FROM_TO(1, DATABASE_INTERFACE_SELECT_MAX_ARITY, DATABASE_INTERFACE_CREATE_TEMPLATE, 0)
#undef DATABASE_INTERFACE_CREATE_TEMPLATE
#undef DATABASE_INTERFACE_CREATE_TEMPLATE_SOURCE
#undef DATABASE_INTERFACE_CREATE_TEMPLATE_PARAM

//! \brief postgres specific select for multiple rows
//!
//! usage:
//! select< coltype1, coltype2 >( trns, targetlistref1, targetlistref2, query )
//!
//! \returns
//!     number of rows in result
#define DATABASE_INTERFACE_CREATE_TEMPLATE(z,params,data) \
    template< \
    BOOST_PP_ENUM_PARAMS(params, typename SOURCE), \
    BOOST_PP_ENUM_PARAMS(params, typename TARGET) > \
    int select_all( \
                    ::di::postgres::transaction_parent& trns, \
                    BOOST_PP_ENUM_BINARY_PARAMS(params, TARGET, & _param), \
                    std::string& query) { \
        DATABASE_INTERFACE_TRACE_QUERY(query); \
        pqxx::result res = trns.exec(query); \
        trace_result(res); \
        if( res.size() > 0 ) \
            ApplyResultAll< 0, BOOST_PP_ENUM_PARAMS(params, SOURCE) >( \
                    res, \
                    BOOST_PP_ENUM_PARAMS(params, _param)); \
        return res.size(); \
    }
BOOST_PP_REPEAT_FROM_TO(1, DATABASE_INTERFACE_SELECT_MAX_ARITY, DATABASE_INTERFACE_CREATE_TEMPLATE, 0)
#undef DATABASE_INTERFACE_CREATE_TEMPLATE


//! \brief termination function for InsertColumns
template< int field >
std::string InsertColumns(di::postgres::Transaction &trns) {
    return "";
}

//! \brief postgres specific insert query builder
//!
//! build query for first column and call recursively
#define DATABASE_INTERFACE_CREATE_TEMPLATE_PARAM(z,para,data) \
    , target ## para
#define DATABASE_INTERFACE_CREATE_TEMPLATE(z,params,data) \
    template< int field, \
    BOOST_PP_ENUM_PARAMS(params, typename SOURCE) > \
    std::string InsertColumns(di::postgres::Transaction& trns, BOOST_PP_ENUM_BINARY_PARAMS(params, const SOURCE,& target) ) { \
        std::string ret; \
        if( field != 0 ) ret += ", "; \
        ret += trns.quote(target0); \
        return ret + InsertColumns<field+1>(trns BOOST_PP_REPEAT_FROM_TO(1,params,DATABASE_INTERFACE_CREATE_TEMPLATE_PARAM,0) ); \
    }
BOOST_PP_REPEAT_FROM_TO(1, DATABASE_INTERFACE_SELECT_MAX_ARITY, DATABASE_INTERFACE_CREATE_TEMPLATE, 0)
#undef DATABASE_INTERFACE_CREATE_TEMPLATE
#undef DATABASE_INTERFACE_CREATE_TEMPLATE_SOURCE
#undef DATABASE_INTERFACE_CREATE_TEMPLATE_PARAM


//! \brief postgres specific insert (single row)
//!
//! usage:
//! insert( trns, srcval1, srcval2, query )
//!
//! \returns
//!     number of rows affected
#define DATABASE_INTERFACE_CREATE_TEMPLATE(z,params,data) \
    template< BOOST_PP_ENUM_PARAMS(params, typename SOURCE) > \
    int insert( \
                ::di::postgres::Transaction& trns, \
                BOOST_PP_ENUM_BINARY_PARAMS(params, const SOURCE, & _param), \
                std::string& query) { \
        query += " VALUES (" + ::di::postgres::InsertColumns<0>(trns, \
                 BOOST_PP_ENUM_PARAMS(params, _param)) + ")"; \
        DATABASE_INTERFACE_TRACE_QUERY(query); \
        pqxx::result res = trns.trns->exec(query); \
        trace_result(res); \
        return res.affected_rows(); \
    }
BOOST_PP_REPEAT_FROM_TO(1, DATABASE_INTERFACE_SELECT_MAX_ARITY, DATABASE_INTERFACE_CREATE_TEMPLATE, 0)
#undef DATABASE_INTERFACE_CREATE_TEMPLATE

} // namespace ::di::postgres


//! \brief postgres specific select
//!
//! allows transparent ::di::select(trns,...,qry)
//! and easy exchange of transaction (different dbms)
#define DATABASE_INTERFACE_CREATE_TEMPLATE(z,params,data) \
    template< \
    BOOST_PP_ENUM_PARAMS(params, typename SOURCE), \
    BOOST_PP_ENUM_PARAMS(params, typename TARGET) > \
    int select( \
                ::di::postgres::Transaction& trns, \
                BOOST_PP_ENUM_BINARY_PARAMS(params, TARGET, & _param), \
                std::string query) { \
        return ::di::postgres::select< BOOST_PP_ENUM_PARAMS(params, SOURCE) >( \
                trns, \
                BOOST_PP_ENUM_PARAMS(params, _param), \
                query); \
    }
BOOST_PP_REPEAT_FROM_TO(1, DATABASE_INTERFACE_SELECT_MAX_ARITY, DATABASE_INTERFACE_CREATE_TEMPLATE, 0)
#undef DATABASE_INTERFACE_CREATE_TEMPLATE

//! \brief postgres specific select_all
//!
//! allows transparent ::di::select_all(trns,...,qry)
//! and easy exchange of transaction (different dbms)
#define DATABASE_INTERFACE_CREATE_TEMPLATE(z,params,data) \
    template< \
    BOOST_PP_ENUM_PARAMS(params, typename SOURCE), \
    BOOST_PP_ENUM_PARAMS(params, typename TARGET) > \
    int select_all( \
                    ::di::postgres::Transaction& trns, \
                    BOOST_PP_ENUM_BINARY_PARAMS(params, TARGET, & _param), \
                    std::string query) { \
        return ::di::postgres::select_all< BOOST_PP_ENUM_PARAMS(params, SOURCE) >( \
                trns, \
                BOOST_PP_ENUM_PARAMS(params, _param), \
                query); \
    }
BOOST_PP_REPEAT_FROM_TO(1, DATABASE_INTERFACE_SELECT_MAX_ARITY, DATABASE_INTERFACE_CREATE_TEMPLATE, 0)
#undef DATABASE_INTERFACE_CREATE_TEMPLATE

//! \brief postgres specific insert
//!
//! allows transparent ::di::insert(trns,...,qry)
//! and easy exchange of transaction (different dbms)
#define DATABASE_INTERFACE_CREATE_TEMPLATE(z,params,data) \
    template< \
    BOOST_PP_ENUM_PARAMS(params, typename SOURCE) > \
    int insert( \
                ::di::postgres::Transaction& trns, \
                BOOST_PP_ENUM_BINARY_PARAMS(params, const SOURCE, & _param), \
                std::string query) { \
        return ::di::postgres::insert( \
                                       trns, \
                                       BOOST_PP_ENUM_PARAMS(params, _param), \
                                       query); \
    }
BOOST_PP_REPEAT_FROM_TO(1, DATABASE_INTERFACE_SELECT_MAX_ARITY, DATABASE_INTERFACE_CREATE_TEMPLATE, 0)
#undef DATABASE_INTERFACE_CREATE_TEMPLATE

//! \brief postgres specific exec
//!
//! allows transparent ::di::exec(trns,qry)
//! and easy exchange of transaction (different dbms)
int inline exec(::di::postgres::Transaction &trns, const std::string &query) {
    DATABASE_INTERFACE_TRACE_QUERY(query);
    ::di::postgres::transaction_parent &trn = trns;
    pqxx::result res = trn.exec(query);
    ::di::postgres::trace_result(res);
    return res.affected_rows();
}

} // namespace ::di

#endif
