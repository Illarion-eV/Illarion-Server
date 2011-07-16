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

#include "data/MonsterTable.hpp"

#include <iostream>
#include <sstream>

#include "data/CommonObjectTable.hpp"

#include "db/Connection.hpp"
#include "db/ConnectionManager.hpp"
#include "db/SelectQuery.hpp"
#include "db/Result.hpp"

#include "World.hpp"
#include "Logger.hpp"

//! table with item attributes
extern CommonObjectTable *CommonItems;

//! wird von verschiedenen Funktionen als Zwischenvariable genutzt
extern CommonStruct tempCommon;

MonsterTable::MonsterTable() : m_dataOK(false), world(World::get()) {
    reload();
}


void MonsterTable::reload() {
#ifdef DataConnect_DEBUG
    std::cout << "MonsterTable: reload" << std::endl;
#endif

    try {
        using namespace Database;
        PConnection connection = ConnectionManager::getInstance().getConnection();
        connection->beginTransaction();

        SelectQuery monquery(connection);
        monquery.addColumn("monster", "mob_monsterid");
        monquery.addColumn("monster", "mob_name");
        monquery.addColumn("monster", "mob_race");
        monquery.addColumn("monster", "mob_hitpoints");
        monquery.addColumn("monster", "mob_movementtype");
        monquery.addColumn("monster", "mob_canattack");
        monquery.addColumn("monster", "mob_canhealself");
        monquery.addColumn("monster", "script");
        monquery.addColumn("monster", "mob_minsize");
        monquery.addColumn("monster", "mob_maxsize");
        monquery.addServerTable("monster");

        Database::Result monresults = monquery.execute();

        if (!monresults.empty()) {
            clearOldTable();
            MonsterStruct temprecord;

            std::string movementType;
            std::string scriptname;
            uint32_t id;

            for (Database::Result::ConstIterator itr = monresults.begin();
                 itr != monresults.end(); ++itr) {
                id = (*itr)["mob_monsterid"].as<uint32_t>();
                temprecord.name = (*itr)["mob_name"].as<std::string>();
                temprecord.race = (Character::race_type)((*itr)["mob_race"].as<uint16_t>());
                temprecord.hitpoints = (*itr)["mob_hitpoints"].as<uint16_t>();
                temprecord.canselfheal = (*itr)["mob_canhealself"].as<bool>();
                temprecord.canattack = (*itr)["mob_canattack"].as<bool>();
                temprecord.minsize = (*itr)["mob_minsize"].as<uint16_t>();
                temprecord.maxsize = (*itr)["mob_maxsize"].as<uint16_t>();

                movementType = (*itr)["mob_movementtype"].as<std::string>();

                if (movementType == "walk") {
                    temprecord.movement = Character::walk;
                } else if (movementType == "fly") {
                    temprecord.movement = Character::fly;
                } else if (movementType == "crawl") {
                    temprecord.movement = Character::crawl;
                } else {
                    //TODO: Some proper error handling for invalid data
                    temprecord.movement = Character::walk;
                }

                if (!(*itr)["script"].is_null()) {
                    scriptname = (*itr)["script"].as<std::string>();

                    if (!scriptname.empty()) {
                        try {
                            boost::shared_ptr<LuaMonsterScript> script(new LuaMonsterScript(scriptname));
                            temprecord.script = script;
                        } catch (ScriptException &e) {
                            Logger::writeError("scripts", "Error while loading script: " + scriptname + ":\n" + e.what() + "\n");
                        }
                    }
                }

                SelectQuery monAttrQuery(connection);
                monAttrQuery.addColumn("monster_attributes", "mobattr_name");
                monAttrQuery.addColumn("monster_attributes", "mobattr_min");
                monAttrQuery.addColumn("monster_attributes", "mobattr_max");
                monAttrQuery.addEqualCondition("monster_attributes", "mobattr_monsterid", id);
                monAttrQuery.addServerTable("monster_attributes");

                Database::Result monAttrResults = monAttrQuery.execute();

                if (!monAttrResults.empty()) {
                    std::string attribute;
                    uint16_t minValue, maxValue;

                    for (Database::Result::ConstIterator itr2 = monAttrResults.begin();
                         itr2 != monAttrResults.end(); ++itr2) {
                        attribute = (*itr2)["mobattr_name"].as<std::string>();
                        minValue = (*itr2)["mobattr_min"].as<uint16_t>();
                        maxValue = (*itr2)["mobattr_max"].as<uint16_t>();

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
                            std::cerr << "unknown attribute type: "<< attribute << std::endl;
                        }
                    } // for (Database::Result::ConstIterator itr2 = monAttrResults...
                } // if (!monAttrResults.empty())

                SelectQuery monSkillQuery(connection);
                monSkillQuery.addColumn("monster_skills", "mobsk_name");
                monSkillQuery.addColumn("monster_skills", "mobsk_minvalue");
                monSkillQuery.addColumn("monster_skills", "mobsk_maxvalue");
                monSkillQuery.addEqualCondition("monster_skills", "mobsk_monsterid", id);
                monSkillQuery.addServerTable("monster_skills");

                Database::Result monSkillResults = monSkillQuery.execute();

                if (!monSkillResults.empty()) {
                    std::string skill;
                    uint16_t minValue, maxValue;

                    for (Database::Result::ConstIterator itr2 = monSkillResults.begin();
                         itr2 != monSkillResults.end(); ++itr2) {
                        skill = (*itr2)["mobsk_name"].as<std::string>();
                        minValue = (*itr2)["mobsk_minvalue"].as<uint16_t>();
                        maxValue = (*itr2)["mobsk_maxvalue"].as<uint16_t>();

                        temprecord.skills[skill] = std::make_pair(minValue, maxValue);
                    } // for (Database::Result::ConstIterator itr2 = monSkillResults...
                } // if (!monSkillResults.empty())

                SelectQuery monItemQuery(connection);
                monItemQuery.addColumn("monster_items", "mobit_itemid");
                monItemQuery.addColumn("monster_items", "mobit_position");
                monItemQuery.addColumn("monster_items", "mobit_mincount");
                monItemQuery.addColumn("monster_items", "mobit_maxcount");
                monItemQuery.addColumn("monster_items", "mobit_propability");
                monItemQuery.addEqualCondition("monster_items", "mobit_monsterid", id);
                monItemQuery.addServerTable("monster_items");

                Database::Result monItemResults = monItemQuery.execute();

                if (!monItemResults.empty()) {
                    itemdef_t tempitem;
                    uint16_t location;
                    std::string position;

                    for (Database::Result::ConstIterator itr2 = monItemResults.begin();
                         itr2 != monItemResults.end(); ++itr2) {
                        tempitem.itemid = (*itr2)["mobit_itemid"].as<TYPE_OF_ITEM_ID>();
                        tempitem.propability = (*itr2)["mobit_propability"].as<uint16_t>();
                        tempitem.amount = std::make_pair(
                                              (*itr2)["mobit_mincount"].as<uint16_t>(),
                                              (*itr2)["mobit_maxcount"].as<uint16_t>());

                        position = (*itr2)["mobit_position"].as<std::string>();

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
                            std::cerr << "specified invalid itemslot: " <<  position << " for monster " << temprecord.name << std::endl;
                            location = 99;
                        }

                        CommonStruct tempCommon;

                        if (location < 99 && CommonItems->find(tempitem.itemid, tempCommon)) {
                            tempitem.AgeingSpeed = tempCommon.AgeingSpeed;
                            temprecord.items[location].push_back(tempitem);
                        } else if (location < 99) {
                            std::cerr << "couldn't find item: " <<  tempitem.itemid << " for monster " << temprecord.name << std::endl;
                        }
                    } // for (Database::Result::ConstIterator itr2 = monItemResults...
                } // if (!monItemResults.empty())

                m_table[id] = temprecord;
                m_dataOK = true;
            } // for (Database::Result::ConstIterator itr = monresults...
        } // if (!monresults.empty())

#ifdef DataConnect_DEBUG
        std::cout << "loaded " << m_table.size() << " monsters!" << std::endl;
#endif
    } catch (std::exception &e) {
        std::cerr << "exception: " << e.what() << std::endl;
        m_dataOK = false;
    }

}

bool MonsterTable::find(TYPE_OF_CHARACTER_ID Id, MonsterStruct &ret) {
    TABLE::iterator iterator;
    iterator = m_table.find(Id);

    if (iterator == m_table.end()) {
        return false;
    } else {
        ret = (*iterator).second;
        return true;
    }
}

void MonsterTable::clearOldTable() {
    m_table.clear();
}

MonsterTable::~MonsterTable() {
    clearOldTable();
}
