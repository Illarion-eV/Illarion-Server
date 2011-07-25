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

#ifndef _SPELL_TABLE_HPP_
#define _SPELL_TABLE_HPP_

#include <map>
#include <list>
#include <iostream>
#include <boost/shared_ptr.hpp>
#include <boost/unordered_map.hpp>
#include "data/Table.hpp"
#include "script/LuaMagicScript.hpp"

class World;

struct SpellStruct {
    unsigned short magictype;
    std::string scriptname;
    boost::shared_ptr<LuaMagicScript> script;
};

class SpellTable: public Table {
public:
    SpellTable();
    ~SpellTable();

    inline bool isDataOK() {
        return _dataOK;
    }

    bool find(unsigned long int magicfFlag,unsigned short int magic_type, SpellStruct &magicSpell);

private:
    virtual void reload();

    typedef boost::unordered_map<unsigned long int, SpellStruct> SpellMap;
    SpellMap Spells;

    void clearOldTable();

    bool _dataOK;
};
#endif

