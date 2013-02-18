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

#include "data/Data.hpp"

namespace Data {

SkillTable Skills;
QuestTable Quests;
RaceSizeTable RaceSizes;
NamesObjectTable ItemNames;
WeaponObjectTable WeaponItems;
ArmorObjectTable ArmorItems;
ContainerObjectTable ContainerItems;
TilesTable Tiles;
TilesModificatorTable TilesModItems;

std::vector<Table *> getTables() {
    return {
        &Skills,
        &Quests,
        &RaceSizes,
        &ItemNames,
        &WeaponItems,
        &ArmorItems,
        &ContainerItems,
        &Tiles,
        &TilesModItems
    };
}

bool reloadTables() {
    bool success = true;

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

void activateReload() {
    for (auto &table : getTables()) {
        table->activateBuffer();
    }
}

bool reload() {
    if (reloadTables()) {
        reloadScripts();
        activateReload();
        return true;
    }

    return false;

}

}

