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

#ifndef _COMMON_OBJECT_TABLE_HPP_
#define _COMMON_OBJECT_TABLE_HPP_

#include <string>
#include <boost/shared_ptr.hpp>
#include "script/LuaItemScript.hpp"
#include <boost/unordered_map.hpp>
#include "data/Table.hpp"
#include "TableStructs.hpp"

class World;

class CommonObjectTable: public Table {
public:
    const CommonStruct &find(TYPE_OF_ITEM_ID id);
    boost::shared_ptr<LuaItemScript> findScript(TYPE_OF_ITEM_ID Id);

    CommonObjectTable();
    ~CommonObjectTable();

    inline bool dataOK() {
        return m_dataOK;
    }

private:
    virtual void reload();

    typedef boost::unordered_map<TYPE_OF_ITEM_ID, CommonStruct> TABLE;
    typedef boost::shared_ptr<LuaItemScript> iscript;
    typedef boost::unordered_map<TYPE_OF_ITEM_ID, iscript> SCRIPTTABLE;

    TABLE m_table;
    SCRIPTTABLE m_scripttable;

    void clearOldTable();
    TYPE_OF_ITEM_ID calcInfiniteRot(TYPE_OF_ITEM_ID id, std::map<TYPE_OF_ITEM_ID, bool> & visited, std::map<TYPE_OF_ITEM_ID, bool> & assigned);

    bool m_dataOK;

};

#endif

