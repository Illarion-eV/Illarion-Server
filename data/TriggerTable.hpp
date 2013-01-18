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
#ifndef _TRIGGER_TABLE_HPP_
#define _TRIGGER_TABLE_HPP_

#include <map>
#include <iostream>
#include <boost/shared_ptr.hpp>
#include <list>
#include <boost/unordered_map.hpp>
#include "data/Table.hpp"
#include "globals.hpp"
#include "script/LuaTriggerScript.hpp"

class World;

struct TriggerStruct {
    position pos;
    std::string scriptname;
    boost::shared_ptr<LuaTriggerScript> script;
};



class TriggerTable: public Table {
public:
    TriggerTable();
    ~TriggerTable();

    inline bool isDataOK() {
        return _dataOK;
    }

    bool find(position pos, TriggerStruct &data);

private:
    virtual void reload();

    typedef boost::unordered_map<position, TriggerStruct> TriggerMap;
    TriggerMap Triggers;

    void clearOldTable();

    bool _dataOK;
};

#endif

