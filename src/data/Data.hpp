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
 * FITNESS FOR A PARTICULAR PURPOSE. See the GNU Affero General Public License for more
 * details.
 *
 * You should have received a copy of the GNU  General Public License along with
 * Illarionserver. If not, see <http://www.gnu.org/licenses/>.
 */

#ifndef _DATA_HPP_
#define _DATA_HPP_

#include "data/ScriptVariablesTable.hpp"
#include "data/SkillTable.hpp"
#include "data/QuestTable.hpp"
#include "data/RaceAttributeTable.hpp"
#include "data/NaturalArmorTable.hpp"
#include "data/MonsterAttackTable.hpp"
#include "data/CommonObjectTable.hpp"
#include "data/NamesObjectTable.hpp"
#include "data/WeaponObjectTable.hpp"
#include "data/ArmorObjectTable.hpp"
#include "data/ContainerObjectTable.hpp"
#include "data/TilesModificatorTable.hpp"
#include "data/TilesTable.hpp"
#include "data/SpellTable.hpp"
#include "data/TriggerTable.hpp"
#include "data/LongTimeEffectTable.hpp"

namespace Data {

extern ScriptVariablesTable ScriptVariables;
extern SkillTable Skills;
extern QuestTable Quests;
extern RaceAttributeTable RaceAttributes;
extern NaturalArmorTable NaturalArmors;
extern MonsterAttackTable MonsterAttacks;
extern CommonObjectTable CommonItems;
extern NamesObjectTable ItemNames;
extern WeaponObjectTable WeaponItems;
extern ArmorObjectTable ArmorItems;
extern ContainerObjectTable ContainerItems;
extern TilesModificatorTable TilesModItems;
extern TilesTable Tiles;
extern SpellTable Spells;
extern TriggerTable Triggers;
extern LongTimeEffectTable LongTimeEffects;

std::vector<Table *> getTables();
bool reloadTables();
void reloadScripts();
void activateTables();
bool reload();

}

#endif

