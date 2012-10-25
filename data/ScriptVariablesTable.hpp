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

#ifndef _SCRIPT_VARIABLES_TABLE_HPP_
#define _SCRIPT_VARIABLES_TABLE_HPP_

#include <boost/unordered_map.hpp>
#include <string>
#include <luabind/object.hpp>
#include "data/Table.hpp"

class ScriptVariablesTable: public Table {
public:

    ScriptVariablesTable();
    ~ScriptVariablesTable();

    bool find(std::string id, std::string &ret);
    void set(std::string id, std::string value);
    void set(std::string id, int32_t value);
    bool remove(std::string id);

    void save();

    bool isDataOk() {
        return m_dataOK;
    }

private:

    typedef boost::unordered_map<std::string, std::string> STRINGTABLE;
    STRINGTABLE values_table;

    void clearOldTable();

    virtual void reload();

    bool m_dataOK;
};

#endif

