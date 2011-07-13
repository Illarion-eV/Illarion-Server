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
/*
 * TODO: This file contains some insane bogus conversation stuff between C and
 *       C++ strings. Should be fixed to a proper usage of C++ strings.
 * TODO: This file possibly contains a memory leak when loading the variables
 *       a second time. Should be checked.
 */

#include "data/ScriptVariablesTable.hpp"

#include <iostream>

#include <boost/lexical_cast.hpp>

#include "db/Connection.hpp"
#include "db/ConnectionManager.hpp"
#include "db/DeleteQuery.hpp"
#include "db/InsertQuery.hpp"
#include "db/SelectQuery.hpp"
#include "db/Result.hpp"

ScriptVariablesTable::ScriptVariablesTable() : m_dataOK(false) {
    reload();
}

ScriptVariablesTable::~ScriptVariablesTable() {
    clearOldTable();
}

void ScriptVariablesTable::clearOldTable() {
    values_table.clear();
}

void ScriptVariablesTable::reload() {
    try {
        Database::SelectQuery query;
        query.addColumn("scriptvariables", "svt_ids");
        query.addColumn("scriptvariables", "svt_string");
        query.addServerTable("scriptvariables");

        Database::Result results = query.execute();

        if (!results.empty()) {
            values_table.clear();
            std::string key;
            std::string value;

            for (Database::Result::ConstIterator itr = results.begin();
                 itr != results.end(); ++itr) {

                key = (*itr)["svt_ids"].as<std::string>();
                value = (*itr)["svt_string"].as<std::string>();

                // The following lines are crappy:
                char *vname = new char[ value.length() + 1 ];
                strcpy(vname, value.c_str());
                vname[ value.length()] = 0;
                values_table[ vname ] = value;
            }
        }

        m_dataOK = true;

#ifdef DataConnect_DEBUG
        std::cout << "loaded " << rows << " values for scripts" << std::endl;
#endif
        return;
    } catch (std::exception &e) {
        std::cerr<<"catched error as reading scriptvariables: "<<e.what()<<std::endl;
        m_dataOK = false;
    }
}

bool ScriptVariablesTable::find(std::string id, std::string &ret) {
    STRINGTABLE::iterator it;
    it = values_table.find(id.c_str());

    if (it != values_table.end()) {
        ret = it->second;
        return true;
    } else {
        ret = "";
        return false;
    }
}

void ScriptVariablesTable::set(std::string id, luabind::object o) {
    char *vname = new char[ id.length() + 1 ];
    strcpy(vname, id.c_str());
    vname[ id.length()] = 0;
    std::string str = boost::lexical_cast<std::string>(o);
    values_table[ vname ] = str;
}

bool ScriptVariablesTable::remove(std::string id) {
    STRINGTABLE::iterator it = values_table.find(id.c_str());

    if (it != values_table.end()) {
        values_table.erase(it);
        return true;
    } else {
        return false;
    }
}

void ScriptVariablesTable::save() {
    try {
        using namespace Database;
        PConnection connection = ConnectionManager::getInstance().getConnection();
        connection->beginTransaction();

        DeleteQuery delQuery(connection);
        delQuery.setServerTable("scriptvariables");
        delQuery.execute();

        InsertQuery insQuery(connection);
        insQuery.setServerTable("scriptvariables");
        const InsertQuery::columnIndex column = insQuery.addColumn("svt_ids");
        insQuery.addColumn("svt_string");
        insQuery.addValues<const char *, std::string, ltstr>(column, values_table, InsertQuery::keysAndValues);

        connection->commitTransaction();
    } catch (std::exception &e) {
        std::cerr<<"exception: "<<e.what()<<" while saving Scriptvariables!"<<std::endl;
    }
}


