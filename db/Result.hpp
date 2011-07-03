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

#ifndef _RESULT_HPP_
#define _RESULT_HPP_

namespace Database {
/* This file contains just some namespaces that hide the pqxx implementation
 * of the SQL Query result handling.
 */
    class Result : public pqxx::result {
        public:
        typedef pqxx::result::tuple Tuple;
        typedef Tuple *PTuple;

        typedef pqxx::result::const_iterator ConstIterator;
        typedef ConstIterator *PConstIterator;
    };
    typedef Result *PResult;
}

#endif // _RESULT_HPP_
