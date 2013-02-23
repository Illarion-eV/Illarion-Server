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

#include "db/Connection.hpp"
#include "db/ConnectionManager.hpp"
#include "db/InsertQuery.hpp"

#include "data/Data.hpp"

#include "Character.hpp"
#include "Player.hpp"
#include "TableStructs.hpp"
#include "World.hpp"

LongTimeEffect::LongTimeEffect(uint16_t effectId, int32_t executeIn):
    effectId(effectId),
    effectName(Data::LongTimeEffects[effectId].effectname),
    executeIn(executeIn) {
}

bool LongTimeEffect::callEffect(Character *target) {
    bool ret = false;
    const auto &script = Data::LongTimeEffects.script(effectId);

    if (script) {
        ret = script->callEffect(this, target);
        ++numberOfCalls;
    }

    return ret;
}

void LongTimeEffect::addValue(std::string name, uint32_t value) {
    values[name] = value;
}

void LongTimeEffect::removeValue(std::string name) {
    values.erase(name);
}

bool LongTimeEffect::findValue(std::string name, uint32_t &ret) {
    auto it = values.find(name);

    if (it != values.end()) {
        ret = it->second;
        return true;
    } else {
        return false;
    }
}

bool LongTimeEffect::save(uint32_t playerid, int32_t currentTime) {
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

            for (auto it = values.begin(); it != values.end(); ++it) {
                insQuery.addValue(nameColumn, it->first);
                insQuery.addValue(valueColumn, it->second);
                insQuery.addValue(userColumn, playerid);
                insQuery.addValue(effectColumn, effectId);
            }

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

uint16_t LongTimeEffect::getEffectId() const {
    return effectId;
}

std::string LongTimeEffect::getEffectName() const {
    return effectName;
}

int32_t LongTimeEffect::getExecuteIn() const {
    return executeIn;
}

void LongTimeEffect::setExecuteIn(int32_t time) {
    executeIn = time;
}

int32_t LongTimeEffect::getExecutionTime() const {
    return executionTime;
}

void LongTimeEffect::setExecutionTime(int32_t offset) {
    executionTime = offset + executeIn;
}

uint32_t LongTimeEffect::getNumberOfCalls() const {
    return numberOfCalls;
}

void LongTimeEffect::setNumberOfCalls(uint32_t calls) {
    numberOfCalls = calls;
}

LTEPriority LongTimeEffect::priority = LTEPriority();

