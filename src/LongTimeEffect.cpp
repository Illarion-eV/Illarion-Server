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

#include "LongTimeEffect.hpp"

#include "Character.hpp"
#include "Player.hpp"
#include "TableStructs.hpp"
#include "World.hpp"
#include "data/Data.hpp"
#include "db/Connection.hpp"
#include "db/ConnectionManager.hpp"
#include "db/InsertQuery.hpp"

#include <boost/cstdint.hpp>
#include <iostream>
#include <sstream>

LongTimeEffect::LongTimeEffect(uint16_t effectId, int32_t executeIn)
        : effectId(effectId), effectName(Data::LongTimeEffects[effectId].effectname), executeIn(executeIn) {}

auto LongTimeEffect::callEffect(Character *target) -> bool {
    bool ret = false;
    const auto &script = Data::LongTimeEffects.script(effectId);

    if (script) {
        ret = script->callEffect(this, target);
        ++numberOfCalls;
    }

    return ret;
}

void LongTimeEffect::addValue(const std::string &name, uint32_t value) { values[name] = value; }

void LongTimeEffect::removeValue(const std::string &name) { values.erase(name); }

auto LongTimeEffect::findValue(const std::string &name, uint32_t &ret) -> bool {
    auto it = values.find(name);

    if (it != values.end()) {
        ret = it->second;
        return true;
    }
    return false;
}

auto LongTimeEffect::save(uint32_t playerid, int32_t currentTime) -> bool {
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
            const InsertQuery::columnIndex numberCalledColumn = insQuery.addColumn("plte_numbercalled");
            insQuery.addValue(userColumn, playerid);
            insQuery.addValue(effectColumn, effectId);
            insQuery.addValue(nextCalledColumn, executionTime - currentTime);
            insQuery.addValue(numberCalledColumn, numberOfCalls);
            insQuery.execute();
        }

        {
            InsertQuery insQuery(connection);
            insQuery.setServerTable("playerlteffectvalues");
            const InsertQuery::columnIndex userColumn = insQuery.addColumn("pev_playerid");
            const InsertQuery::columnIndex effectColumn = insQuery.addColumn("pev_effectid");
            const InsertQuery::columnIndex nameColumn = insQuery.addColumn("pev_name");
            const InsertQuery::columnIndex valueColumn = insQuery.addColumn("pev_value");

            for (const auto &value : values) {
                insQuery.addValue(nameColumn, value.first);
                insQuery.addValue(valueColumn, value.second);
                insQuery.addValue(userColumn, playerid);
                insQuery.addValue(effectColumn, effectId);
            }

            insQuery.execute();
        }

        connection->commitTransaction();
        return true;
    } catch (std::exception &e) {
        Logger::error(LogFacility::Other) << "Exception while saving long time effects: " << e.what() << Log::end;
        connection->rollbackTransaction();
        return false;
    }

    return true;
}

auto LongTimeEffect::getEffectId() const -> uint16_t { return effectId; }

auto LongTimeEffect::getEffectName() const -> std::string { return effectName; }

auto LongTimeEffect::getExecuteIn() const -> int32_t { return executeIn; }

void LongTimeEffect::setExecuteIn(int32_t time) { executeIn = time; }

auto LongTimeEffect::getExecutionTime() const -> int32_t { return executionTime; }

void LongTimeEffect::setExecutionTime(int32_t offset) { executionTime = offset + executeIn; }

auto LongTimeEffect::getNumberOfCalls() const -> uint32_t { return numberOfCalls; }

void LongTimeEffect::setNumberOfCalls(uint32_t calls) { numberOfCalls = calls; }

LTEPriority LongTimeEffect::priority = LTEPriority();
