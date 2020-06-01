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

#include "data/MonsterTable.hpp"

#include "Logger.hpp"
#include "data/Data.hpp"
#include "data/QuestNodeTable.hpp"
#include "db/Connection.hpp"
#include "db/ConnectionManager.hpp"
#include "db/Result.hpp"
#include "db/SelectQuery.hpp"

MonsterTable::MonsterTable() {
    Logger::info(LogFacility::Other) << "MonsterTable::constructor" << Log::end;

    try {
        using namespace Database;
        PConnection connection = ConnectionManager::getInstance().getConnection();
        connection->beginTransaction();

        SelectQuery monquery(connection);
        monquery.addColumn("monster", "mob_monsterid");
        monquery.addColumn("monster", "mob_name_de");
        monquery.addColumn("monster", "mob_name_en");
        monquery.addColumn("monster", "mob_race");
        monquery.addColumn("monster", "mob_hitpoints");
        monquery.addColumn("monster", "mob_movementtype");
        monquery.addColumn("monster", "mob_canattack");
        monquery.addColumn("monster", "mob_canhealself");
        monquery.addColumn("monster", "script");
        monquery.addColumn("monster", "mob_minsize");
        monquery.addColumn("monster", "mob_maxsize");
        monquery.addServerTable("monster");
        monquery.addOrderBy("monster", "mob_monsterid", Database::SelectQuery::ASC);

        Database::Result monresults = monquery.execute();

        if (!monresults.empty()) {
            auto questNodes = QuestNodeTable::getInstance().getMonsterNodes();
            auto questItr = questNodes.first;
            auto questEnd = questNodes.second;

            SelectQuery monLootDataQuery(connection);
            monLootDataQuery.addColumn("monster_drop_data", "mdd_id");
            monLootDataQuery.addColumn("monster_drop_data", "mdd_key");
            monLootDataQuery.addColumn("monster_drop_data", "mdd_value");
            monLootDataQuery.addServerTable("monster_drop_data");
            auto lootDataResults = monLootDataQuery.execute();

            std::map<uint32_t, std::map<std::string, std::string>> lootData;

            for (const auto &row : lootDataResults) {
                auto &lootDataEntry = lootData[row["mdd_id"].as<uint32_t>()];
                lootDataEntry.insert(
                        std::make_pair(row["mdd_key"].as<std::string>(), row["mdd_value"].as<std::string>()));
            }

            for (const auto &row : monresults) {
                MonsterStruct temprecord;
                const auto id = row["mob_monsterid"].as<uint32_t>();
                temprecord.nameDe = row["mob_name_de"].as<std::string>();
                temprecord.nameEn = row["mob_name_en"].as<std::string>();
                temprecord.race = row["mob_race"].as<TYPE_OF_RACE_ID>();
                temprecord.hitpoints = row["mob_hitpoints"].as<uint16_t>();
                temprecord.canselfheal = row["mob_canhealself"].as<bool>();
                temprecord.canattack = row["mob_canattack"].as<bool>();
                temprecord.minsize = row["mob_minsize"].as<uint16_t>();
                temprecord.maxsize = row["mob_maxsize"].as<uint16_t>();

                const auto movementType = row["mob_movementtype"].as<std::string>("");

                if (movementType == "walk") {
                    temprecord.movement = movement_type::walk;
                } else if (movementType == "fly") {
                    temprecord.movement = movement_type::fly;
                } else if (movementType == "crawl") {
                    temprecord.movement = movement_type::crawl;
                } else {
                    temprecord.movement = movement_type::walk;
                    Logger::error(LogFacility::Script) << "Monster " << temprecord.nameEn << " ( " << id
                                                       << ") has illegal movement type: " << movementType << Log::end;
                }

                if (!row["script"].is_null()) {
                    const auto scriptname = row["script"].as<std::string>();

                    try {
                        std::shared_ptr<LuaMonsterScript> script(new LuaMonsterScript(scriptname));
                        temprecord.script = script;
                    } catch (ScriptException &e) {
                        Logger::error(LogFacility::Script)
                                << "Error while loading monster script: " << scriptname << ": " << e.what() << Log::end;
                    }
                } else if (questItr != questEnd && questItr->first == id) {
                    std::shared_ptr<LuaMonsterScript> tmpScript = std::make_shared<LuaMonsterScript>();
                    temprecord.script = tmpScript;
                }

                while (questItr != questEnd && questItr->first == id) {
                    temprecord.script->addQuestScript(questItr->second.entrypoint, questItr->second.script);
                    ++questItr;
                }

                SelectQuery monAttrQuery(connection);
                monAttrQuery.addColumn("monster_attributes", "mobattr_name");
                monAttrQuery.addColumn("monster_attributes", "mobattr_min");
                monAttrQuery.addColumn("monster_attributes", "mobattr_max");
                monAttrQuery.addEqualCondition("monster_attributes", "mobattr_monsterid", id);
                monAttrQuery.addServerTable("monster_attributes");

                Database::Result monAttrResults = monAttrQuery.execute();

                for (const auto &attrRow : monAttrResults) {
                    const auto attribute = attrRow["mobattr_name"].as<std::string>("");
                    auto minValue = attrRow["mobattr_min"].as<uint16_t>();
                    auto maxValue = attrRow["mobattr_max"].as<uint16_t>();

                    if (attribute == "luck") {
                        temprecord.attributes.luck = std::make_pair(minValue, maxValue);
                    } else if (attribute == "strength") {
                        temprecord.attributes.strength = std::make_pair(minValue, maxValue);
                    } else if (attribute == "dexterity") {
                        temprecord.attributes.dexterity = std::make_pair(minValue, maxValue);
                    } else if (attribute == "constitution") {
                        temprecord.attributes.constitution = std::make_pair(minValue, maxValue);
                    } else if (attribute == "agility") {
                        temprecord.attributes.agility = std::make_pair(minValue, maxValue);
                    } else if (attribute == "intelligence") {
                        temprecord.attributes.intelligence = std::make_pair(minValue, maxValue);
                    } else if (attribute == "perception") {
                        temprecord.attributes.perception = std::make_pair(minValue, maxValue);
                    } else if (attribute == "willpower") {
                        temprecord.attributes.willpower = std::make_pair(minValue, maxValue);
                    } else if (attribute == "essence") {
                        temprecord.attributes.essence = std::make_pair(minValue, maxValue);
                    } else {
                        Logger::error(LogFacility::Other)
                                << "Unknown attribute type for monster " << id << ": " << attribute << Log::end;
                    }
                }

                SelectQuery monSkillQuery(connection);
                monSkillQuery.addColumn("monster_skills", "mobsk_skill_id");
                monSkillQuery.addColumn("monster_skills", "mobsk_minvalue");
                monSkillQuery.addColumn("monster_skills", "mobsk_maxvalue");
                monSkillQuery.addEqualCondition("monster_skills", "mobsk_monsterid", id);
                monSkillQuery.addServerTable("monster_skills");

                Database::Result monSkillResults = monSkillQuery.execute();

                for (const auto &skillRow : monSkillResults) {
                    TYPE_OF_SKILL_ID skill = TYPE_OF_SKILL_ID(skillRow["mobsk_skill_id"].as<uint16_t>());
                    auto minValue = skillRow["mobsk_minvalue"].as<uint16_t>();
                    auto maxValue = skillRow["mobsk_maxvalue"].as<uint16_t>();

                    temprecord.skills[skill] = std::make_pair(minValue, maxValue);
                }

                SelectQuery monItemQuery(connection);
                monItemQuery.addColumn("monster_items", "mobit_itemid");
                monItemQuery.addColumn("monster_items", "mobit_position");
                monItemQuery.addColumn("monster_items", "mobit_mincount");
                monItemQuery.addColumn("monster_items", "mobit_maxcount");
                monItemQuery.addEqualCondition("monster_items", "mobit_monsterid", id);
                monItemQuery.addServerTable("monster_items");

                Database::Result monItemResults = monItemQuery.execute();

                for (const auto &itemRow : monItemResults) {
                    itemdef_t tempitem;
                    tempitem.itemid = itemRow["mobit_itemid"].as<TYPE_OF_ITEM_ID>();
                    tempitem.amount = std::make_pair(itemRow["mobit_mincount"].as<uint16_t>(),
                                                     itemRow["mobit_maxcount"].as<uint16_t>());

                    const auto position = itemRow["mobit_position"].as<std::string>("");
                    uint16_t location;

                    if (position == "head") {
                        location = 1;
                    } else if (position == "neck") {
                        location = 2;
                    } else if (position == "breast") {
                        location = 3;
                    } else if (position == "hands") {
                        location = 4;
                    } else if (position == "left hand") {
                        location = 5;
                    } else if (position == "right hand") {
                        location = 6;
                    } else if (position == "left finger") {
                        location = 7;
                    } else if (position == "right finger") {
                        location = 8;
                    } else if (position == "legs") {
                        location = 9;
                    } else if (position == "feet") {
                        location = 10;
                    } else if (position == "coat") {
                        location = 11;
                    } else if (position == "belt1") {
                        location = 12;
                    } else if (position == "belt2") {
                        location = 13;
                    } else if (position == "belt3") {
                        location = 14;
                    } else if (position == "belt4") {
                        location = 15;
                    } else if (position == "belt5") {
                        location = 16;
                    } else if (position == "belt6") {
                        location = 17;
                    } else {
                        Logger::error(LogFacility::Other)
                                << "Invalid itemslot for monster " << id << ": " << position << Log::end;
                        location = 99;
                    }

                    const auto &itemStruct = Data::Items[tempitem.itemid];

                    if (location < 99 && itemStruct.isValid()) {
                        tempitem.AgeingSpeed = itemStruct.AgeingSpeed;
                        temprecord.items[location].push_back(tempitem);
                    } else if (location < 99) {
                        Logger::error(LogFacility::Other)
                                << "Invalid item for monster " << id << ": " << tempitem.itemid << Log::end;
                    }
                }

                SelectQuery monLootQuery(connection);
                monLootQuery.addColumn("monster_drop", "md_id");
                monLootQuery.addColumn("monster_drop", "md_category");
                monLootQuery.addColumn("monster_drop", "md_probability");
                monLootQuery.addColumn("monster_drop", "md_itemid");
                monLootQuery.addColumn("monster_drop", "md_amount_min");
                monLootQuery.addColumn("monster_drop", "md_amount_max");
                monLootQuery.addColumn("monster_drop", "md_quality_min");
                monLootQuery.addColumn("monster_drop", "md_quality_max");
                monLootQuery.addColumn("monster_drop", "md_durability_min");
                monLootQuery.addColumn("monster_drop", "md_durability_max");
                monLootQuery.addEqualCondition("monster_drop", "md_monsterid", id);
                monLootQuery.addServerTable("monster_drop");

                const auto monLootResults = monLootQuery.execute();

                for (const auto &lootRow : monLootResults) {
                    auto lootId = lootRow["md_id"].as<uint32_t>();
                    auto categoryId = lootRow["md_category"].as<uint16_t>();

                    auto &category = temprecord.loot[categoryId];
                    auto &lootItem = category[lootId];

                    lootItem.itemId = lootRow["md_itemid"].as<TYPE_OF_ITEM_ID>();
                    lootItem.probability = lootRow["md_probability"].as<double>();
                    lootItem.amount = std::make_pair(lootRow["md_amount_min"].as<uint16_t>(),
                                                     lootRow["md_amount_max"].as<uint16_t>());
                    lootItem.quality = std::make_pair(lootRow["md_quality_min"].as<uint16_t>(),
                                                      lootRow["md_quality_max"].as<uint16_t>());
                    lootItem.durability = std::make_pair(lootRow["md_durability_min"].as<uint16_t>(),
                                                         lootRow["md_durability_max"].as<uint16_t>());
                    lootItem.data = std::move(lootData[lootId]);
                }

                table[id] = temprecord;
                dataOK = true;
            }
        }

        connection->commitTransaction();
        Logger::info(LogFacility::Other) << "Loaded " << table.size() << " monsters!" << Log::end;
    } catch (std::exception &e) {
        Logger::error(LogFacility::Other) << "Exception in MonsterTable::reload: " << e.what() << Log::end;
        dataOK = false;
    }
}

auto MonsterTable::exists(TYPE_OF_CHARACTER_ID id) const -> bool {
    return table.count(id) > 0;
}

auto MonsterTable::operator[](TYPE_OF_CHARACTER_ID id) -> const MonsterStruct & {
    try {
        return table.at(id);
    } catch (std::out_of_range &) {
        Logger::error(LogFacility::Script) << "Table monster"
                                           << ": entry " << id << " was not found!" << Log::end;
        return table[id];
    }
}
