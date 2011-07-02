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


#include "db/ConnectionManager.hpp"
#include "LongTimeEffect.hpp"
#include "Character.hpp"
#include "Player.hpp"
#include "World.hpp"
#include "data/LongTimeEffectTable.hpp"
#include "TableStructs.hpp"
#include "script/LuaLongTimeEffectScript.hpp"

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
    ConnectionManager::TransactionHolder transaction = dbmgr->getTransaction();

    try {
        di::insert(transaction, static_cast<uint32_t>(playerid), static_cast<uint16_t>(_effectId), static_cast<int32_t>(_nextCalled), static_cast<uint32_t>(_lastCalled),static_cast<uint32_t>(_numberCalled), "INSERT INTO playerlteffects (plte_playerid, plte_effectid, plte_nextcalled, plte_lastcalled, plte_numberCalled)");

        for (VALUETABLE::iterator it = _values.begin(); it != _values.end(); ++it) {
            std::cout<<"inserting effektdata("<<_effectId<<") name: "<<it->first<<" value: "<<static_cast<uint32_t>(it->second)<<std::endl;
            di::insert(transaction, static_cast<uint32_t>(playerid), static_cast<uint16_t>(_effectId), it->first, static_cast<uint32_t>(it->second), "INSERT INTO playerlteffectvalues (pev_playerid, pev_effectid, pev_name, pev_value)");
        }

        transaction.commit();
        return true;
    } catch (std::exception &e) {
        std::cerr << "caught exception during saving lt effects: " << e.what() << std::endl;
        transaction.rollback();
        return false;
    }

    return true;
}


