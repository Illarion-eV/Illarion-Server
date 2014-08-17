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

#include "data/Data.hpp"
#include "Logger.hpp"
#include "script/LuaLongTimeEffectScript.hpp"
#include "Config.hpp"

#include <stdlib.h>

namespace Data {

ScriptVariablesTable ScriptVariables;
SkillTable Skills;
QuestTable Quests;
RaceAttributeTable RaceAttributes;
NaturalArmorTable NaturalArmors;
MonsterAttackTable MonsterAttacks;
CommonObjectTable CommonItems;
WeaponObjectTable WeaponItems;
ArmorObjectTable ArmorItems;
ContainerObjectTable ContainerItems;
TilesModificatorTable TilesModItems;
TilesTable Tiles;
SpellTable Spells;
TriggerTable Triggers;
LongTimeEffectTable LongTimeEffects;

std::vector<Table *> getTables() {
    return {
        &ScriptVariables,
        &Quests,
        &RaceAttributes,
        &NaturalArmors,
        &MonsterAttacks,
        &CommonItems,
        &WeaponItems,
        &ArmorItems,
        &ContainerItems,
        &TilesModItems,
        &Tiles,
        &Spells,
        &Triggers,
        &LongTimeEffects
    };
}

bool reloadTables() {
    bool success = true;

    Logger::notice(LogFacility::Script) << "Loading data and scripts ..." << Log::end;

    std::string preReloadCommand = "sh " + Config::instance().datadir() + "pre-reload 2>/dev/null >/dev/null";
    if (!system(preReloadCommand.c_str())) {
        Logger::info(LogFacility::Other) << "Error calling pre-reload hook." << Log::end;
    }

    for (auto &table : getTables()) {
        success = success && table->reloadBuffer();

        if (!success) {
            break;
        }
    }

    return success;
}

void reloadScripts() {
    for (auto &table : getTables()) {
        table->reloadScripts();
    }
}

void activateTables() {
    for (auto &table : getTables()) {
        table->activateBuffer();
    }
}

bool reload() {
    if (reloadTables()) {
        activateTables();
        reloadScripts();
        return true;
    }

    return false;
}

}

