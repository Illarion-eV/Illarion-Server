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

#ifndef _SCHEMA_HELPER_HPP_
#define _SCHEMA_HELPER_HPP_

#include <string>

namespace Database {
class SchemaHelper {
private:
    static std::string serverSchema;
    static std::string accountSchema;

public:
    static inline void setSchemata(const std::string &server,
        const std::string &account) {
        serverSchema += "\"";
        serverSchema += server;
        serverSchema += "\"";

        accountSchema += "\"";
        accountSchema += account;
        accountSchema += "\"";
    }

    static inline std::string getServerSchema(void) {
        return serverSchema;
    }

    static inline std::string getAccountSchema(void) {
        return accountSchema;
    }
    
private:
    SchemaHelper(void);
    SchemaHelper(const SchemaHelper &org);
    virtual ~SchemaHelper(void);
};
}

#endif // _SCHEMA_HELPER_HPP_
