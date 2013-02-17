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
#include <algorithm>

#include "data/LongTimeEffectTable.hpp"

#include "db/Connection.hpp"
#include "db/ConnectionManager.hpp"
#include "db/DeleteQuery.hpp"
#include "db/Result.hpp"
#include "db/SelectQuery.hpp"

#include "script/LuaLongTimeEffectScript.hpp"

#include "LongTimeEffect.hpp"
#include "Player.hpp"

extern LongTimeEffectTable *LongTimeEffects;

LongTimeCharacterEffects::LongTimeCharacterEffects(Character *owner) : owner(owner), time(0) {
}

bool LongTimeCharacterEffects::find(uint16_t effectid, LongTimeEffect*&effect) {
    for (auto it = effects.begin(); it != effects.end(); ++it) {
        if ((*it)->getEffectId() == effectid) {
            effect = *it;
            return true;
        }
    }

    effect = NULL;
    return false;
}

bool LongTimeCharacterEffects::find(std::string effectname, LongTimeEffect*&effect) {

    for (auto it = effects.begin(); it != effects.end(); ++it) {
        if ((*it)->getEffectName() == effectname) {
            effect = *it;
            return true;
        }
    }

    effect = NULL;
    return false;
}

void LongTimeCharacterEffects::addEffect(LongTimeEffect *effect) {
    LongTimeEffect *foundeffect;

    if (effect == 0) {
        LuaScript::triggerScriptError("LTE must not be nil!");
    }

    if (!find(effect->getEffectId(), foundeffect)) {
        effect->setExecutionTime(time);
        effects.push_back(effect);
        std::push_heap(effects.begin(), effects.end(), LongTimeEffect::priority);

        LongTimeEffectStruct effectStr;

        if (effect->isFirstAdd() && LongTimeEffects->find(effect->getEffectId(), effectStr)) {
            if (effectStr.script) {
                effectStr.script->addEffect(effect, owner);
            }
        }
    } else {
        LongTimeEffectStruct effectStr;

        if (LongTimeEffects->find(effect->getEffectId(), effectStr)) {
            if (effectStr.script) {
                effectStr.script->doubleEffect(foundeffect, owner);
            }
        }
    }

    effect->firstAdd(); //set first add for this effect
}

bool LongTimeCharacterEffects::removeEffect(uint16_t effectid) {
    for (auto it = effects.begin(); it != effects.end(); ++it) {
        if ((*it)->getEffectId() == effectid) {
            LongTimeEffectStruct effect;

            if (LongTimeEffects->find((*it)->getEffectId(), effect)) {
                if (effect.script) {
                    effect.script->removeEffect(*it, owner);
                }
            }

            delete *it;
            effects.erase(it);
            std::make_heap(effects.begin(), effects.end(), LongTimeEffect::priority);
            return true;
        }
    }

    return false;
}

bool LongTimeCharacterEffects::removeEffect(std::string name) {
    for (auto it = effects.begin(); it != effects.end(); ++it) {
        if ((*it)->getEffectName() == name) {
            LongTimeEffectStruct effect;

            if (LongTimeEffects->find((*it)->getEffectId(), effect)) {
                if (effect.script) {
                    effect.script->removeEffect(*it, owner);
                }
            }

            delete *it;
            effects.erase(it);
            std::make_heap(effects.begin(), effects.end(), LongTimeEffect::priority);
            return true;
        }
    }

    return false;
}

bool LongTimeCharacterEffects::removeEffect(LongTimeEffect *effect) {
    for (auto it = effects.begin(); it != effects.end(); ++it) {
        if (*it == effect) {
            LongTimeEffectStruct effectStr;

            if (LongTimeEffects->find((*it)->getEffectId(), effectStr)) {
                if (effectStr.script) {
                    effectStr.script->removeEffect(*it, owner);
                }
            }

            effects.erase(it);
            delete effect;
            std::make_heap(effects.begin(), effects.end(), LongTimeEffect::priority);
            return true;
        }
    }

    return false;
}

void LongTimeCharacterEffects::checkEffects() {
    ++time;

    LongTimeEffect *effect;
    int emexit = 0;

    while (!effects.empty() && (emexit < 200) && (effects.front()->getExecutionTime() <= time)) {
        ++emexit;
        std::pop_heap(effects.begin(), effects.end(), LongTimeEffect::priority);
        effect = effects.back();
        effects.pop_back();

        if (effect->callEffect(owner)) {
            addEffect(effect);
        } else {
            LongTimeEffectStruct effectStr;

            if (LongTimeEffects->find(effect->getEffectId(), effectStr)) {
                if (effectStr.script) {
                    effectStr.script->removeEffect(effect, owner);
                }
            }
        }
    }
}

bool LongTimeCharacterEffects::save() {
    using namespace Database;

    if (owner && owner->character != Character::player) {
        std::cerr << "called save for LongtimeCharacterEffects but owner was no player" << std::endl;
        return false;
    }

    Player *player = dynamic_cast<Player *>(owner);

    if (!owner) {
        std::cout << "error saving long time effects owner was NULL!" << std::endl;
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

    for (auto it = effects.begin(); it != effects.end(); ++it) {
        allok and_eq(*it)->save(player->id, time);
    }

    return allok;
}

bool LongTimeCharacterEffects::load() {
    using namespace Database;

    if (owner->character != Character::player) {
        std::cerr << "called load for LongtimeCharacterEffects but owner was no player" << std::endl;
        return false;
    }

    Player *player = dynamic_cast<Player *>(owner);

    PConnection connection = ConnectionManager::getInstance().getConnection();

    try {
        SelectQuery query(connection);
        query.addColumn("playerlteffects", "plte_effectid");
        query.addColumn("playerlteffects", "plte_nextcalled");
        query.addColumn("playerlteffects", "plte_numbercalled");
        query.addEqualCondition<TYPE_OF_CHARACTER_ID>("playerlteffects", "plte_playerid", player->id);
        query.addServerTable("playerlteffects");
        query.addOrderBy("playerlteffects", "plte_nextcalled", SelectQuery::ASC);

        Result results = query.execute();

        if (!results.empty()) {
            for (ResultConstIterator itr = results.begin();
                 itr != results.end(); ++itr) {
                uint16_t effectId = (*itr)["plte_effectid"].as<uint16_t>();
                LongTimeEffect *effect = new LongTimeEffect(effectId, (*itr)["plte_nextcalled"].as<int32_t>());

                effect->setExecutionTime(time);
                effect->firstAdd();
                effect->setNumberOfCalls((*itr)["plte_numberCalled"].as<uint32_t>());

                SelectQuery valuesQuery(connection);
                valuesQuery.addColumn("playerlteffectvalues", "pev_name");
                valuesQuery.addColumn("playerlteffectvalues", "pev_value");
                valuesQuery.addEqualCondition<TYPE_OF_CHARACTER_ID>("playerlteffectvalues", "pev_playerid", player->id);
                valuesQuery.addEqualCondition<uint16_t>("playerlteffectvalues", "pev_effectid", effectId);
                valuesQuery.addServerTable("playerlteffectvalues");

                Result valuesResults = valuesQuery.execute();

                if (!valuesResults.empty()) {
                    for (ResultConstIterator itr2 = valuesResults.begin();
                         itr2 != valuesResults.end(); ++itr2) {
                        effect->addValue(
                            (*itr2)["pev_name"].as<std::string>(),
                            (*itr2)["pev_value"].as<uint32_t>());
                    }
                }

                effects.push_back(effect);
                LongTimeEffectStruct effectStr;

                if (LongTimeEffects->find(effectId, effectStr)) {
                    if (effectStr.script) {
                        effectStr.script->loadEffect(effect, player);
                    }
                }

            }
        }

        std::make_heap(effects.begin(), effects.end(), LongTimeEffect::priority);

        return true;
    } catch (std::exception &e) {
        std::cerr << "Error while loading longtimeeffects for player: " << player->name << "(" << player->id << ") what: " << e.what() << std::endl;
        return false;
    }
}

