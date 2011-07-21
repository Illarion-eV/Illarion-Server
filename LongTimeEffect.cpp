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

#include "LongTimeEffect.hpp"

#include <sstream>
#include <iostream>

#include <boost/cstdint.hpp>

#include "data/LongTimeEffectTable.hpp"

#include "db/Connection.hpp"
#include "db/ConnectionManager.hpp"
#include "db/InsertQuery.hpp"

#include "script/LuaLongTimeEffectScript.hpp"

#include "Character.hpp"
#include "Player.hpp"
#include "TableStructs.hpp"
#include "World.hpp"

extern LongTimeEffectTable *LongTimeEffects;

LongTimeEffect::LongTimeEffect(uint16_t effectId, uint32_t nextCalled) : _effectId(effectId), _effectName(""),  _nextCalled(nextCalled), _numberCalled(0), _lastCalled(0), _firstadd(true) {
    LongTimeEffectStruct effect;
    LongTimeEffects->find(_effectId, effect);
    _effectName = effect.effectname;
    _values.clear();
}

LongTimeEffect::LongTimeEffect(std::string name, uint32_t nextCalled) : _effectId(0), _effectName(name), _nextCalled(nextCalled), _numberCalled(0), _lastCalled(0), _firstadd(true) {
    LongTimeEffectStruct effect;
    LongTimeEffects->find(_effectId, effect);
    _effectId = effect.effectid;
    _values.clear();
}

LongTimeEffect::~LongTimeEffect() {
    _values.clear();
}

bool LongTimeEffect::callEffect(Character *target) {
    bool ret = false;
    LongTimeEffectStruct effect;

    if (LongTimeEffects->find(_effectId, effect)) {
        if (effect.script) {
            ret = effect.script->callEffect(this, target);
            _lastCalled = _nextCalled;
            _numberCalled++;
        }
    } else {
        std::cout<<"can't find effect with id: "<<_effectId<<" to call the script."<<std::endl;
    }

    return ret;
}

void LongTimeEffect::addValue(std::string name, uint32_t value) {
    VALUETABLE::iterator it = _values.find(name.c_str());

    if (it != _values.end()) {
        it->second = value;
    } else {
        char *sname = new char[name.length() + 1];
        strcpy(sname, name.c_str());
        sname[ name.length()] = 0;
        _values[ sname ] = value;
    }
}

void LongTimeEffect::removeValue(std::string name) {
    VALUETABLE::iterator it = _values.find(name.c_str());

    if (it != _values.end()) {
        _values.erase(it);
    }
}

bool LongTimeEffect::findValue(std::string name, uint32_t &ret) {
    VALUETABLE::iterator it = _values.find(name.c_str());

    if (it != _values.end()) {
        ret = it->second;
        return true;
    } else {
        return false;
    }
}

bool LongTimeEffect::save(uint32_t playerid) {
    using namespace Database;
    PConnection connection = ConnectionManager::getInstance().getConnection();

    try {
        connection->beginTransaction();

        {
            InsertQuery insQuery(connection);
            insQuery.setServerTable("playerlteffects");
            const InsertQuery::columnIndex userColumn = insQuery.addColumn("plte_playerid");
            const InsertQuery::columnIndex effectColumn = insQuery.addColumn("plte_effectid");
            const InsertQuery::columnIndex nextCalledColumn = insQuery.addColumn("plte_nextcalled");
            const InsertQuery::columnIndex lastCalledColumn = insQuery.addColumn("plte_lastcalled");
            const InsertQuery::columnIndex numberCalledColumn = insQuery.addColumn("plte_numbercalled");
            insQuery.addValue(userColumn, playerid);
            insQuery.addValue(effectColumn, _effectId);
            insQuery.addValue(nextCalledColumn, _nextCalled);
            insQuery.addValue(lastCalledColumn, _lastCalled);
            insQuery.addValue(numberCalledColumn, _numberCalled);
            insQuery.execute();
        }

        {
            InsertQuery insQuery(connection);
            insQuery.setServerTable("playerlteffectvalues");
            const InsertQuery::columnIndex userColumn = insQuery.addColumn("pev_playerid");
            const InsertQuery::columnIndex effectColumn = insQuery.addColumn("pev_effectid");
            const InsertQuery::columnIndex nameColumn = insQuery.addColumn("pev_name");
            insQuery.addColumn("pev_value");

            insQuery.addValues<const char *, uint32_t, ltstr >(nameColumn, _values, InsertQuery::keysAndValues);
            insQuery.addValues(userColumn, playerid, InsertQuery::FILL);
            insQuery.addValues(effectColumn, _effectId, InsertQuery::FILL);
            insQuery.execute();
        }

        connection->commitTransaction();
        return true;
    } catch (std::exception &e) {
        std::cerr << "caught exception during saving lt effects: " << e.what() << std::endl;
        connection->rollbackTransaction();
        return false;
    }

    return true;
}


