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
/*
 * TODO: The whole reloading/adding NPCs zu the main table is not written in a
 * very solid way and needs to be done again.
 */

#include "data/NPCTable.hpp"

#include "Logger.hpp"
#include "data/QuestNodeTable.hpp"
#include "db/Result.hpp"
#include "db/SelectQuery.hpp"
#include "script/LuaNPCScript.hpp"
#include "types.hpp"

#include <iostream>

NPCTable::NPCTable() : _world(World::get()) { reload(); }

NPCTable::~NPCTable() = default;

void NPCTable::reload() {
    try {
        Database::SelectQuery query;
        query.addColumn("npc", "npc_id");
        query.addColumn("npc", "npc_type");
        query.addColumn("npc", "npc_posx");
        query.addColumn("npc", "npc_posy");
        query.addColumn("npc", "npc_posz");
        query.addColumn("npc", "npc_name");
        query.addColumn("npc", "npc_faceto");
        query.addColumn("npc", "npc_is_healer");
        query.addColumn("npc", "npc_script");
        query.addColumn("npc", "npc_sex");
        query.addColumn("npc", "npc_hair");
        query.addColumn("npc", "npc_beard");
        query.addColumn("npc", "npc_hairred");
        query.addColumn("npc", "npc_hairgreen");
        query.addColumn("npc", "npc_hairblue");
        query.addColumn("npc", "npc_hairalpha");
        query.addColumn("npc", "npc_skinred");
        query.addColumn("npc", "npc_skingreen");
        query.addColumn("npc", "npc_skinblue");
        query.addColumn("npc", "npc_skinalpha");
        query.addServerTable("npc");
        query.addOrderBy("npc", "npc_id", Database::SelectQuery::ASC);

        Database::Result results = query.execute();

        if (!results.empty()) {
            auto questNodes = QuestNodeTable::getInstance().getNpcNodes();
            auto questItr = questNodes.first;
            auto questEnd = questNodes.second;

            for (const auto &row : results) {
                auto npcID = row["npc_id"].as<TYPE_OF_CHARACTER_ID>();

                const position pos(row["npc_posx"].as<int16_t>(), row["npc_posy"].as<int16_t>(),
                                   row["npc_posz"].as<int16_t>());

                auto npcName = row["npc_name"].as<std::string>();

                Character::appearance appearance;
                appearance.hairtype = uint8_t(row["npc_hair"].as<int16_t>());
                appearance.beardtype = uint8_t(row["npc_hair"].as<int16_t>());
                appearance.hair = {
                        uint8_t(row["npc_hairred"].as<int16_t>()), uint8_t(row["npc_hairgreen"].as<int16_t>()),
                        uint8_t(row["npc_hairblue"].as<int16_t>()), uint8_t(row["npc_hairalpha"].as<int16_t>())};
                appearance.skin = {
                        (uint8_t)(row["npc_skinred"].as<int16_t>()), (uint8_t)(row["npc_skingreen"].as<int16_t>()),
                        (uint8_t)(row["npc_skinblue"].as<int16_t>()), (uint8_t)(row["npc_skinalpha"].as<int16_t>())};

                NPC *newNPC = nullptr;

                try {
                    newNPC = new NPC(npcID, npcName, row["npc_type"].as<TYPE_OF_RACE_ID>(), pos,
                                     Character::face_to(row["npc_faceto"].as<uint32_t>()),
                                     row["npc_is_healer"].as<bool>(),
                                     Character::sex_type(row["npc_sex"].as<uint32_t>()), appearance);

                    // add npc to npc list
                    _world->Npc.insert(newNPC);

                    if (!row["npc_script"].is_null()) {
                        const auto scriptname = row["npc_script"].as<std::string>();

                        try {
                            std::shared_ptr<LuaNPCScript> script(new LuaNPCScript(scriptname, newNPC));

                            while (questItr != questEnd && questItr->first == npcID) {
                                script->addQuestScript(questItr->second.entrypoint, questItr->second.script);
                                ++questItr;
                            }

                            newNPC->setScript(script);
                        } catch (ScriptException &e) {
                            Logger::error(LogFacility::Script)
                                    << "Error while loading npc script: " << scriptname << ": " << e.what() << Log::end;
                        }
                    }

                    newNPC = nullptr;
                } catch (FieldNotFound &) {
                    Logger::error(LogFacility::Other)
                            << "No space available for NPC " << npcName << "(" << npcID << ") near " << pos << Log::end;
                }

                delete newNPC;
            }
        }

        m_dataOK = true;
    } catch (...) {
        Logger::error(LogFacility::Other) << "Error while loading NPCs" << Log::end;
        m_dataOK = false;
    }
}
