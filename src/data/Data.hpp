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

#ifndef DATA_HPP
#define DATA_HPP

#include "data/ArmorObjectTable.hpp"
#include "data/ContainerObjectTable.hpp"
#include "data/ItemTable.hpp"
#include "data/LongTimeEffectTable.hpp"
#include "data/MonsterAttackTable.hpp"
#include "data/NaturalArmorTable.hpp"
#include "data/QuestTable.hpp"
#include "data/RaceTable.hpp"
#include "data/ScriptVariablesTable.hpp"
#include "data/SkillTable.hpp"
#include "data/SpellTable.hpp"
#include "data/TilesModificatorTable.hpp"
#include "data/TilesTable.hpp"
#include "data/TriggerTable.hpp"
#include "data/WeaponObjectTable.hpp"

namespace Data {

auto scriptVariables() -> ScriptVariablesTable &;
auto skills() -> SkillTable &;
auto quests() -> QuestTable &;
auto races() -> RaceTable &;
auto naturalArmors() -> NaturalArmorTable &;
auto monsterAttacks() -> MonsterAttackTable &;
auto items() -> ItemTable &;
auto weaponItems() -> WeaponObjectTable &;
auto armorItems() -> ArmorObjectTable &;
auto containerItems() -> ContainerObjectTable &;
auto tilesModItems() -> TilesModificatorTable &;
auto tiles() -> TilesTable &;
auto spells() -> SpellTable &;
auto triggers() -> TriggerTable &;
auto longTimeEffects() -> LongTimeEffectTable &;

auto getTables() -> std::vector<Table *>;
auto reloadTables() -> bool;
void reloadScripts();
void activateTables();
auto reload() -> bool;
void preReload();

} // namespace Data

#endif
