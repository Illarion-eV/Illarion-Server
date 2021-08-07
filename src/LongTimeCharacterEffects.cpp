/*
 * Illarionserver - server for the game Illarion
 * Copyright 2011 Illarion e.V.
 *
 * This file is part of Illarionserver.
 *
 * Illarionserver  is  free  software:  you can redistribute it and/or modify it
 * under the terms of the  GNU Affero General Public License as published by the Free
 * Software Foundation, either version 3 of the License, or (at your option) any
 * later version.
 *
 * Illarionserver is distributed in the hope that it will be useful, but WITHOUT
 * ANY WARRANTY;  without  even  the  implied  warranty  of  MERCHANTABILITY  or
 * FITNESS FOR A PARTICULAR PURPOSE. See the GNU Affero General Public License for more
 * details.
 *
 * You should have received a copy of the GNU Affero General Public License along with
 * Illarionserver. If not, see <http://www.gnu.org/licenses/>.
 */

#include "LongTimeCharacterEffects.hpp"

#include "LongTimeEffect.hpp"
#include "Player.hpp"
#include "data/Data.hpp"
#include "db/Connection.hpp"
#include "db/ConnectionManager.hpp"
#include "db/DeleteQuery.hpp"
#include "db/Result.hpp"
#include "db/SelectQuery.hpp"

#include <algorithm>
#include <range/v3/all.hpp>
#include <string>

LongTimeCharacterEffects::LongTimeCharacterEffects(Character *owner) : owner(owner), time(0) {}

auto LongTimeCharacterEffects::find(uint16_t effectid, LongTimeEffect *&effect) const -> bool {
    using namespace ranges;
    auto doesIdMatch = [effectid](const auto &e) { return e->getEffectId() == effectid; };
    auto result = find_if(effects, doesIdMatch);
    bool success = result != effects.end();

    if (success) {
        effect = result->get();
    } else {
        effect = nullptr;
    }

    return success;
}

auto LongTimeCharacterEffects::find(const std::string &effectname, LongTimeEffect *&effect) const -> bool {
    using namespace ranges;
    auto doesNameMatch = [&effectname](const auto &e) { return e->getEffectName() == effectname; };
    auto result = find_if(effects, doesNameMatch);
    bool success = result != effects.end();

    if (success) {
        effect = result->get();
    } else {
        effect = nullptr;
    }

    return success;
}

void LongTimeCharacterEffects::addEffect(LongTimeEffect *effect) { addEffect(std::unique_ptr<LongTimeEffect>(effect)); }
void LongTimeCharacterEffects::addEffect(std::unique_ptr<LongTimeEffect> effect) {
    LongTimeEffect *foundeffect = nullptr;

    if (effect == nullptr) {
        LuaScript::triggerScriptError("LTE must not be nil!");
    }

    if (!find(effect->getEffectId(), foundeffect)) {
        effect->setExecutionTime(time);

        if (effect->isFirstAdd()) {
            const auto &script = Data::longTimeEffects().script(effect->getEffectId());

            if (script) {
                script->addEffect(effect.get(), owner);
            }
        }

        effect->firstAdd();
        effects.push_back(std::move(effect));
        std::push_heap(effects.begin(), effects.end(), LongTimeEffect::priority);
    } else {
        const auto &script = Data::longTimeEffects().script(effect->getEffectId());

        if (script) {
            script->doubleEffect(foundeffect, owner);
        }
    }
}

auto LongTimeCharacterEffects::removeEffect(uint16_t effectid) -> bool {
    for (auto it = effects.begin(); it != effects.end(); ++it) {
        if ((*it)->getEffectId() == effectid) {
            const auto &script = Data::longTimeEffects().script(effectid);

            if (script) {
                script->removeEffect(it->get(), owner);
            }

            effects.erase(it);
            std::make_heap(effects.begin(), effects.end(), LongTimeEffect::priority);
            return true;
        }
    }

    return false;
}

auto LongTimeCharacterEffects::removeEffect(const std::string &name) -> bool {
    for (auto it = effects.begin(); it != effects.end(); ++it) {
        if ((*it)->getEffectName() == name) {
            const auto &script = Data::longTimeEffects().script((*it)->getEffectId());

            if (script) {
                script->removeEffect(it->get(), owner);
            }

            effects.erase(it);
            std::make_heap(effects.begin(), effects.end(), LongTimeEffect::priority);
            return true;
        }
    }

    return false;
}

auto LongTimeCharacterEffects::removeEffect(const LongTimeEffect *effect) -> bool {
    for (auto it = effects.begin(); it != effects.end(); ++it) {
        if (it->get() == effect) {
            const auto &script = Data::longTimeEffects().script((*it)->getEffectId());

            if (script) {
                script->removeEffect(it->get(), owner);
            }

            effects.erase(it);
            std::make_heap(effects.begin(), effects.end(), LongTimeEffect::priority);
            return true;
        }
    }

    return false;
}

void LongTimeCharacterEffects::checkEffects() {
    ++time;
    constexpr auto scriptLimit = 200;
    int emexit = 0;

    while (!effects.empty() && (emexit < scriptLimit) && (effects.front()->getExecutionTime() <= time)) {
        ++emexit;
        std::pop_heap(effects.begin(), effects.end(), LongTimeEffect::priority);
        std::unique_ptr<LongTimeEffect> effect = std::move(effects.back());
        effects.pop_back();

        if (effect->callEffect(owner)) {
            addEffect(std::move(effect));
        } else {
            const auto &script = Data::longTimeEffects().script(effect->getEffectId());

            if (script) {
                script->removeEffect(effect.get(), owner);
            }
        }
    }
}

auto LongTimeCharacterEffects::save() -> bool {
    using namespace Database;

    if ((owner != nullptr) && owner->getType() != Character::player) {
        return false;
    }

    auto *player = dynamic_cast<Player *>(owner);

    if (owner == nullptr) {
        return false;
    }

    PConnection connection = ConnectionManager::getInstance().getConnection();
    connection->beginTransaction();

    try {
        {
            DeleteQuery query(connection);
            query.addEqualCondition<TYPE_OF_CHARACTER_ID>("playerlteffects", "plte_playerid", player->getId());
            query.setServerTable("playerlteffects");
            query.execute();
        }
        {
            DeleteQuery query(connection);
            query.addEqualCondition<TYPE_OF_CHARACTER_ID>("playerlteffectvalues", "pev_playerid", player->getId());
            query.setServerTable("playerlteffectvalues");
            query.execute();
        }
        connection->commitTransaction();
    } catch (std::exception &e) {
        Logger::error(LogFacility::Database)
                << "Error while saving long time effects for " << player->to_string() << ": " << e.what() << Log::end;
        connection->rollbackTransaction();
        return false;
    }

    bool allok = true;

    for (auto &effect : effects) {
        allok and_eq effect->save(player->getId(), time);
    }

    return allok;
}

auto LongTimeCharacterEffects::load() -> bool {
    using namespace Database;

    if (owner->getType() != Character::player) {
        return false;
    }

    auto *player = dynamic_cast<Player *>(owner);

    PConnection connection = ConnectionManager::getInstance().getConnection();

    try {
        SelectQuery query(connection);
        query.addColumn("playerlteffects", "plte_effectid");
        query.addColumn("playerlteffects", "plte_nextcalled");
        query.addColumn("playerlteffects", "plte_numbercalled");
        query.addEqualCondition<TYPE_OF_CHARACTER_ID>("playerlteffects", "plte_playerid", player->getId());
        query.addServerTable("playerlteffects");
        query.addOrderBy("playerlteffects", "plte_nextcalled", SelectQuery::ASC);

        Result results = query.execute();

        if (!results.empty()) {
            for (const auto &row : results) {
                auto effectId = row["plte_effectid"].as<uint16_t>();
                auto effect = std::make_unique<LongTimeEffect>(effectId, row["plte_nextcalled"].as<int32_t>());

                effect->setExecutionTime(time);
                effect->firstAdd();
                effect->setNumberOfCalls(row["plte_numberCalled"].as<uint32_t>());

                SelectQuery valuesQuery(connection);
                valuesQuery.addColumn("playerlteffectvalues", "pev_name");
                valuesQuery.addColumn("playerlteffectvalues", "pev_value");
                valuesQuery.addEqualCondition<TYPE_OF_CHARACTER_ID>("playerlteffectvalues", "pev_playerid",
                                                                    player->getId());
                valuesQuery.addEqualCondition<uint16_t>("playerlteffectvalues", "pev_effectid", effectId);
                valuesQuery.addServerTable("playerlteffectvalues");

                Result valuesResults = valuesQuery.execute();

                if (!valuesResults.empty()) {
                    for (const auto &valueRow : valuesResults) {
                        effect->addValue(valueRow["pev_name"].as<std::string>(), valueRow["pev_value"].as<uint32_t>());
                    }
                }

                const auto &script = Data::longTimeEffects().script(effectId);

                if (script) {
                    script->loadEffect(effect.get(), player);
                }

                effects.push_back(std::move(effect));
            }
        }

        std::make_heap(effects.begin(), effects.end(), LongTimeEffect::priority);

        return true;
    } catch (std::exception &e) {
        Logger::error(LogFacility::Database)
                << "Error while loading long time effects for " << player->to_string() << ": " << e.what() << Log::end;
        return false;
    }
}
