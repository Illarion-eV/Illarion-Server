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

#include "LongTimeCharacterEffects.hpp"

#include <string>

#include "data/LongTimeEffectTable.hpp"

#include "db/Connection.hpp"
#include "db/ConnectionManager.hpp"
#include "db/DeleteQuery.hpp"
#include "db/Result.hpp"
#include "db/SelectQuery.hpp"

#include "script/LuaLongTimeEffectScript.hpp"

#include "LongTimeEffect.hpp"
#include "MilTimer.hpp"
#include "Player.hpp"

extern LongTimeEffectTable *LongTimeEffects;

LongTimeCharacterEffects::LongTimeCharacterEffects(Character *owner) : _owner(owner) {
    _effectTimer = new MilTimer(100);
}

LongTimeCharacterEffects::~LongTimeCharacterEffects() {
    delete _effectTimer;
    _effectTimer = NULL;
    effectList.clear();
}

bool LongTimeCharacterEffects::find(uint16_t effectid, LongTimeEffect * &effect) {
    for (EFFECTLIST::iterator it = effectList.begin(); it != effectList.end(); ++it) {
        if ((*it)->_effectId == effectid) {
            effect = (*it);
            return true;
        }
    }

    effect = NULL;
    return false;
}

bool LongTimeCharacterEffects::find(std::string effectname, LongTimeEffect * &effect) {

    for (EFFECTLIST::iterator it = effectList.begin(); it != effectList.end(); ++it) {
        if ((*it)->_effectName == effectname) {
            effect = (*it);
            return true;
        }
    }

    effect = NULL;
    return false;
}

void LongTimeCharacterEffects::addEffect(LongTimeEffect *effect) {
    LongTimeEffect *foundeffect;

    if (!find(effect->_effectId, foundeffect)) {
        EFFECTLIST::iterator it = effectList.begin();
        bool inserted = false;

        while (!inserted && it != effectList.end()) {
            if ((*it)->_nextCalled >= effect->_nextCalled) {
                (*it)->_nextCalled -= effect->_nextCalled;
                effectList.insert(it, effect);
                inserted = true;
            } else {
                effect->_nextCalled -= (*it)->_nextCalled;
                it++;
            }
        }

        if (!inserted) {
            effectList.push_back(effect);
        }

        LongTimeEffectStruct effectStr;

        if (effect->_firstadd && LongTimeEffects->find(effect->_effectId, effectStr)) {
            if (effectStr.script) {
                effectStr.script->addEffect(effect , _owner);
            }
        }
    } else {
        LongTimeEffectStruct effectStr;

        if (LongTimeEffects->find(effect->_effectId, effectStr)) {
            if (effectStr.script) {

                effectStr.script->doubleEffect(foundeffect, _owner);
            }
        }
    }

    effect->firstAdd(); //set first add for this effect
}

bool LongTimeCharacterEffects::removeEffect(uint16_t effectid) {
    for (EFFECTLIST::iterator it = effectList.begin(); it != effectList.end(); ++it) {
        if ((*it)->_effectId == effectid) {
            LongTimeEffectStruct effect;

            if (LongTimeEffects->find((*it)->_effectId, effect)) {
                if (effect.script) {
                    effect.script->removeEffect((*it), _owner);
                }
            }

            effectList.erase(it);
            delete(*it);
            return true;
        }
    }

    return false;
}

bool LongTimeCharacterEffects::removeEffect(std::string name) {
    for (EFFECTLIST::iterator it = effectList.begin(); it != effectList.end(); ++it) {
        if ((*it)->_effectName == name) {
            LongTimeEffectStruct effect;

            if (LongTimeEffects->find((*it)->_effectId, effect)) {
                if (effect.script) {
                    effect.script->removeEffect((*it), _owner);
                }
            }

            effectList.erase(it);
            delete(*it);
            return true;
        }
    }

    return false;
}

bool LongTimeCharacterEffects::removeEffect(LongTimeEffect *effect) {
    for (EFFECTLIST::iterator it = effectList.begin(); it != effectList.end(); ++it) {
        if ((*it) == effect) {
            LongTimeEffectStruct effectStr;

            if (LongTimeEffects->find((*it)->_effectId, effectStr)) {
                if (effectStr.script) {
                    effectStr.script->removeEffect((*it), _owner);
                }
            }

            effectList.erase(it);
            delete effect;
            effect = NULL;
            return true;
        }
    }

    return false;
}

void LongTimeCharacterEffects::checkEffects() {
    if (_effectTimer->Next()) {
        //if (  _owner->character == Character::player ) std::cout<<"checkEffect: "<<checkChar->name<<" size: "<<effectList.size()<<std::endl;
        LongTimeEffect *effect;
        int emexit = 0;

        if (!effectList.empty() && (effectList.front()->_nextCalled > 0)) {
            effectList.front()->_nextCalled--;
            //if ( _owner->character == Character::player ) std::cout<<"decreased nextcalled: "<<effectList.front()->_nextCalled<<std::endl;
        } else {
            //if (  _owner->character == Character::player ) std::cout<<"calling an effectscript"<<std::endl;
            while (!effectList.empty() && (emexit < 200) && (effectList.front()->_nextCalled <= 0)) {
                emexit++;
                effect = effectList.front();
                effectList.pop_front();

                if (effect->callEffect(_owner)) {
                    addEffect(effect);
                } else {
                    LongTimeEffectStruct effectStr;

                    if (LongTimeEffects->find(effect->_effectId, effectStr)) {
                        if (effectStr.script) {
                            effectStr.script->removeEffect(effect, _owner);
                        }
                    }
                }
            }
        }
    }
}

bool LongTimeCharacterEffects::save() {
    using namespace Database;

    if (_owner->character != Character::player) {
        std::cout<<"called save for LongtimeCharacterEffects but owner was no player"<<std::endl;
        return false;
    }

    Player *player = dynamic_cast<Player *>(_owner);

    if (!_owner) {
        std::cout<<"error saving long time effects owner was NULL!"<<std::endl;
        return false;
    }

    PConnection connection = ConnectionManager::getInstance().getConnection();
    connection->beginTransaction();

    try {
        {
            DeleteQuery query(connection);
            query.addEqualCondition<TYPE_OF_CHARACTER_ID>("playerlteffects", "plte_playerid", player->id);
            query.setServerTable("playerlteffects");
            query.execute();
        }
        {
            DeleteQuery query(connection);
            query.addEqualCondition<TYPE_OF_CHARACTER_ID>("playerlteffectvalues", "pev_playerid", player->id);
            query.setServerTable("playerlteffectvalues");
            query.execute();
        }
        connection->commitTransaction();
    } catch (std::exception &e) {
        std::cerr << "caught exception during saving LongTimeCharacterEffects effects: " << e.what() << std::endl;
        connection->rollbackTransaction();
        return false;
    }

    std::cout<<"deleting old data was successfull inserting new one"<<std::endl;
    bool allok = true;

    for (EFFECTLIST::iterator it = effectList.begin(); it != effectList.end(); ++it) {
        allok &= (*it)->save(player->id);
    }

    //di::postgres::enable_trace_query = false;
    std::cout<<"saved data: "<<allok<<std::endl;
    return allok;
}

bool LongTimeCharacterEffects::load() {
    using namespace Database;
    std::cout<<"try to load effects" <<std::endl;

    if (_owner->character != Character::player) {
        std::cout<<"called load for LongtimeCharacterEffects but owner was no player"<<std::endl;
        return false;
    }

    Player *player = dynamic_cast<Player *>(_owner);

    PConnection connection = ConnectionManager::getInstance().getConnection();
    connection->beginTransaction();

    try {
        SelectQuery query(connection);
        query.addColumn("playerlteffects", "plte_effectid");
        query.addColumn("playerlteffects", "plte_nextcalled");
        query.addColumn("playerlteffects", "plte_lastcalled");
        query.addColumn("playerlteffects", "plte_numberCalled");
        query.addEqualCondition<TYPE_OF_CHARACTER_ID>("playerlteffects", "plte_playerid", player->id);
        query.addServerTable("playerlteffects");
        query.addOrderBy("playerlteffects", "plte_nextcalled", SelectQuery::ASC);

        Result results = query.execute();

        if (!results.empty()) {
            for (Result::ConstIterator itr = results.begin();
                 itr != results.end(); ++itr) {
                LongTimeEffect *effect = new LongTimeEffect(
                    (*itr)["plte_effectid"].as<uint16_t>(),
                    (*itr)["plte_nextcalled"].as<uint32_t>());

                effect->firstAdd();
                effect->_lastCalled = (*itr)["plte_lastcalled"].as<uint32_t>();
                effect->_numberCalled = (*itr)["plte_numberCalled"].as<uint32_t>();

                SelectQuery valuesQuery(connection);
                valuesQuery.addColumn("playerlteffectvalues", "pev_name");
                valuesQuery.addColumn("playerlteffectvalues", "pev_value");
                valuesQuery.addEqualCondition<TYPE_OF_CHARACTER_ID>("playerlteffectvalues", "pev_playerid", player->id);
                valuesQuery.addEqualCondition<uint16_t>("playerlteffectvalues", "pev_effectid", effect->_effectId);
                valuesQuery.addServerTable("playerlteffectvalues");

                Result valuesResults = valuesQuery.execute();

                if (!valuesResults.empty()) {
                    for (Result::ConstIterator itr2 = valuesResults.begin();
                         itr2 != valuesResults.end(); ++itr2) {
                        effect->addValue(
                            (*itr)["pev_name"].as<std::string>(),
                            (*itr)["pev_value"].as<uint32_t>());
                    }
                }

                effectList.push_back(effect);
                LongTimeEffectStruct effectStr;

                if (LongTimeEffects->find(effect->_effectId, effectStr)) {
                    if (effectStr.script) {
                        effectStr.script->loadEffect(effect, player);
                    }
                }

            }
        }

        std::cout<<"effects laoded" <<std::endl;

        return true;
    } catch (std::exception &e) {
        std::cerr << "Error while loading longtimeeffects for player: " << player->name << "(" << player->id << ") what: " << e.what() << std::endl;
        return false;
    }
}
