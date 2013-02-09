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
/*
 * TODO: The whole reloading/adding NPCs zu the main table is not written in a
 * very solid way and needs to be done again.
 */

#include "data/NPCTable.hpp"
#include "data/QuestNodeTable.hpp"

#include <iostream>

#include "db/SelectQuery.hpp"
#include "db/Result.hpp"

#include "Logger.hpp"
#include "types.hpp"

template< typename To, typename From> To stream_convert(const From &from) {
    std::stringstream stream;
    stream << from;
    To to;
    stream >> to;
    return to;
}

NPCTable::NPCTable() : _world(World::get()) {
    reload();
}

NPCTable::~NPCTable() {}

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
        query.addColumn("npc", "npc_skinred");
        query.addColumn("npc", "npc_skingreen");
        query.addColumn("npc", "npc_skinblue");
        query.addServerTable("npc");
        query.addOrderBy("npc", "npc_id", Database::SelectQuery::ASC);

        Database::Result results = query.execute();

        if (!results.empty()) {
            NPC *newNPC = 0;

            TYPE_OF_CHARACTER_ID npcID = 0;
            int32_t posx, posy, posz;
            std::string npcName;
            std::string scriptname;
            auto questNodes = QuestNodeTable::getInstance()->getNpcNodes();
            auto questItr = questNodes.first;
            auto questEnd = questNodes.second;

            for (Database::ResultConstIterator itr = results.begin();
                 itr != results.end(); ++itr) {
                try {
                    npcID = ((*itr)["npc_id"].as<TYPE_OF_CHARACTER_ID>());

                    posx = ((*itr)["npc_posx"].as<int32_t>());
                    posy = ((*itr)["npc_posy"].as<int32_t>());
                    posz = ((*itr)["npc_posz"].as<int32_t>());

                    npcName = ((*itr)["npc_name"].as<std::string>());

		    Character::appearance appearance;
		    appearance.hairtype = (uint8_t)((*itr)["npc_hair"].as<int16_t>());
		    appearance.beardtype = (uint8_t)((*itr)["npc_hair"].as<int16_t>());
		    appearance.hair = {
                        (uint8_t)((*itr)["npc_hairred"].as<int16_t>()),
                        (uint8_t)((*itr)["npc_hairgreen"].as<int16_t>()),
                        (uint8_t)((*itr)["npc_hairblue"].as<int16_t>())
		    };
		    appearance.skin = {
                        (uint8_t)((*itr)["npc_skinred"].as<int16_t>()),
                        (uint8_t)((*itr)["npc_skingreen"].as<int16_t>()),
                        (uint8_t)((*itr)["npc_skinblue"].as<int16_t>())
		    };

                    newNPC = new NPC(
                        npcID, npcName,
                        (Character::race_type)((*itr)["npc_type"].as<uint32_t>()),
                        position(posx, posy, posz),
                        (Character::face_to)((*itr)["npc_faceto"].as<uint32_t>()),
                        ((*itr)["npc_is_healer"].as<bool>()),
                        (Character::sex_type)((*itr)["npc_sex"].as<uint32_t>()),
                        appearance);

                    // add npc to npc list
                    _world->Npc.push_back(newNPC);

                    // set field to occupied
                    Field *tempf;

                    if (_world->GetPToCFieldAt(tempf, posx, posy, posz)) {
                        tempf->setChar();
                    }

                    if (!((*itr)["npc_script"].is_null())) {
                        scriptname = ((*itr)["npc_script"].as<std::string>());

                        try {
                            boost::shared_ptr<LuaNPCScript> script(new LuaNPCScript(scriptname, newNPC));

                            while (questItr != questEnd && questItr->first == npcID) {
                                script->addQuestScript(questItr->second.entrypoint, questItr->second.script);
                                ++questItr;
                            }

                            newNPC->setScript(script);
                        } catch (ScriptException &e) {
                            Logger::writeError("scripts", "Error while loading npc script: " + scriptname + ":\n" + e.what() + "\n");
                        }
                    }

                    newNPC = 0;
                } catch (NoSpace &s) {
                    std::cout << "no space available for npc: " << npcName << "(" << npcID << ") : " << s.what() << std::endl;
                }

                delete newNPC;
            }

        }

        m_dataOK = true;
    } catch (...) {
        std::cerr << "error while loading npcs" << std::endl;
        m_dataOK = false;
    }
}
