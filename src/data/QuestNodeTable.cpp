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

#include "data/QuestNodeTable.hpp"

#include "Config.hpp"
#include "Logger.hpp"
#include "script/LuaScript.hpp"

#include <boost/algorithm/string.hpp>
#include <boost/lexical_cast.hpp>
#include <filesystem>
#include <fstream>
#include <memory>
#include <string>
#include <vector>

std::unique_ptr<QuestNodeTable> QuestNodeTable::instance = nullptr;

auto QuestNodeTable::getInstance() -> QuestNodeTable & {
    if (!instance) {
        instance = std::make_unique<QuestNodeTable>();
    }

    return *instance;
}

QuestNodeTable::QuestNodeTable() { reload(); }

auto QuestNodeTable::getItemNodes() const -> TableRange<TYPE_OF_ITEM_ID> {
    return {itemNodes.cbegin(), itemNodes.cend()};
}

auto QuestNodeTable::getNpcNodes() const -> TableRange<unsigned int> { return {npcNodes.cbegin(), npcNodes.cend()}; }

auto QuestNodeTable::getMonsterNodes() const -> TableRange<unsigned int> {
    return {monsterNodes.cbegin(), monsterNodes.cend()};
}

auto QuestNodeTable::getTriggerNodes() const -> TableRange<position> {
    return {triggerNodes.cbegin(), triggerNodes.cend()};
}

void QuestNodeTable::reload() {
    using namespace std::filesystem;
    path dirPath = path(Config::instance().scriptdir() + "questsystem");
    path filename = path("quest.txt");

    directory_iterator end_itr;

    clear();

    for (directory_iterator itr(dirPath); itr != end_itr; ++itr) {
        if (is_directory(itr->status())) {
            path filePath(itr->path() / filename);

            if (exists(filePath)) {
                std::ifstream questFile(filePath);

                if (questFile.is_open()) {
                    path questPath(itr->path());
                    readQuest(questFile, questPath);
                    questFile.close();
                }
            }
        }
    }
}

void QuestNodeTable::readQuest(std::ifstream &questFile, std::filesystem::path &questPath) {
    std::string line;

    while (std::getline(questFile, line)) {
        if (line.empty()) {
            continue;
        }

        std::vector<std::string> entries;
        boost::split(entries, line, boost::is_any_of(","));
        const auto &type = entries[typePosition];

        if ((type != "triggerfield" && entries.size() != normalEntryCount) ||
            (type == "triggerfield" && entries.size() != triggerfieldEntryCount)) {
            Logger::error(LogFacility::Script)
                    << "Syntax error while loading quest file: " << questPath.string() << "/quest.txt" << Log::end;
            return;
        }

        if (type == "triggerfield") {
            position pos{};
            const auto &x = entries[triggerCoordinateXPosition];

            try {
                pos.x = boost::lexical_cast<signed short>(x);
            } catch (boost::bad_lexical_cast &) {
                Logger::error(LogFacility::Script)
                        << "Conversion error while loading quest file: " << x << " is not a map coordinate" << Log::end;
                return;
            }

            const auto &y = entries[triggerCoordinateYPosition];

            try {
                pos.y = boost::lexical_cast<signed short>(y);
            } catch (boost::bad_lexical_cast &) {
                Logger::error(LogFacility::Script)
                        << "Conversion error while loading quest file: " << y << " is not a map coordinate" << Log::end;
                return;
            }

            const auto &z = entries[triggerCoordinateZPosition];

            try {
                pos.z = boost::lexical_cast<signed short>(z);
            } catch (boost::bad_lexical_cast &) {
                Logger::error(LogFacility::Script)
                        << "Conversion error while loading quest file: " << z << " is not a map coordinate" << Log::end;
                return;
            }

            const auto &functionName = entries[triggerFunctionPosition];
            const auto &scriptName = entries[triggerScriptPosition];
            NodeStruct node;
            node.entrypoint = functionName;
            std::string scriptPath = "questsystem." + questPath.filename().string() + "." + scriptName;

            try {
                node.script = std::make_shared<LuaScript>(scriptPath);
            } catch (ScriptException &e) {
                Logger::error(LogFacility::Script) << "Error while loading quest script: " << e.what() << Log::end;
                return;
            }

            triggerNodes.emplace(pos, node);

        } else {
            unsigned int id = 0;

            const auto &idString = entries[idPosition];

            try {
                id = boost::lexical_cast<unsigned int>(idString);
            } catch (boost::bad_lexical_cast &) {
                Logger::error(LogFacility::Script)
                        << "Conversion error while loading quest file: " << idString << " is not an ID" << Log::end;
                return;
            }

            const auto &functionName = entries[functionPosition];
            const auto &scriptName = entries[scriptPosition];
            NodeStruct node;
            node.entrypoint = functionName;
            std::string scriptPath = "questsystem." + questPath.filename().string() + "." + scriptName;

            try {
                node.script = std::make_shared<LuaScript>(scriptPath);
            } catch (ScriptException &e) {
                Logger::error(LogFacility::Script) << "Error while loading quest script: " << e.what() << Log::end;
                return;
            }

            if (type == "item") {
                itemNodes.insert(std::pair<unsigned int, NodeStruct>(id, node));
            } else if (type == "npc") {
                npcNodes.insert(std::pair<unsigned int, NodeStruct>(id, node));
            } else if (type == "monster") {
                monsterNodes.insert(std::pair<unsigned int, NodeStruct>(id, node));
            }
        }
    }
}

void QuestNodeTable::clear() {
    itemNodes.clear();
    npcNodes.clear();
    monsterNodes.clear();
    triggerNodes.clear();
}
