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

#include "data/QuestNodeTable.hpp"
#include <boost/filesystem.hpp>
#include <boost/filesystem/fstream.hpp>
#include <boost/algorithm/string.hpp>
#include <boost/lexical_cast.hpp>
#include <boost/shared_ptr.hpp>
#include <vector>
#include <string>
#include "script/LuaScript.hpp"
#include "Logger.hpp"

QuestNodeTable *QuestNodeTable::instance = 0;

QuestNodeTable *QuestNodeTable::getInstance() {
    if (instance == 0) {
        instance = new QuestNodeTable();
    }

    return instance;
}

QuestNodeTable::QuestNodeTable() {
    reload();
}

QuestNodeTable::~QuestNodeTable() {}

QuestNodeTable::TABLE_ITRS QuestNodeTable::getItemNodes() {
    return QuestNodeTable::TABLE_ITRS(itemNodes.cbegin(), itemNodes.cend());
}

QuestNodeTable::TABLE_ITRS QuestNodeTable::getNpcNodes() {
    return QuestNodeTable::TABLE_ITRS(npcNodes.cbegin(), npcNodes.cend());
}

void QuestNodeTable::reload() {
    using namespace boost::filesystem;
    path dirPath = path("/usr/share/servers/testserver/scripts/questsystem");
    path filename = path("quest.txt");

    directory_iterator end_itr;

    clear();

    for (directory_iterator itr(dirPath); itr != end_itr; ++itr) {
        if (is_directory(itr->status())) {
            path filePath(itr->path() / filename);

            if (exists(filePath)) {
                ifstream questFile(filePath);

                if (questFile.is_open()) {
                    path questPath(itr->path());
                    readQuest(questFile, questPath);
                    questFile.close();
                }
            }
        }
    }
}

void QuestNodeTable::readQuest(boost::filesystem::ifstream &questFile, boost::filesystem::path &questPath) {
    std::string line;

    while (std::getline(questFile,line)) {
        if (line.empty()) {
            continue;
        }

        std::vector<std::string> entries;
        boost::split(entries, line, boost::is_any_of(","));

        if (entries.size() != 4) {
            Logger::writeError("scripts", "Syntax error while loading quest file: " + questPath.string() + "/quest.txt\n");
            return;
        }

        unsigned int id;

        try {
            id = boost::lexical_cast<unsigned int>(entries[1]);
        } catch (boost::bad_lexical_cast &) {
            Logger::writeError("scripts", "Conversion error while loading quest file: " + entries[1] + " is not an ID\n");
            return;
        }

        NodeStruct node;
        node.entrypoint = entries[2];
        std::string scriptPath = "questsystem." + questPath.filename() + "." + entries[3];

        try {
            node.script = boost::shared_ptr<LuaScript>(new LuaScript(scriptPath));
        } catch (ScriptException &e) {
            Logger::writeError("scripts", "Error while loading quest script: " + std::string(e.what()) + "\n");
            return;
        }

        if (entries[0] == "item") {
            itemNodes.insert(std::pair<unsigned int, NodeStruct>(id, node));
        } else if (entries[0] == "npc") {
            npcNodes.insert(std::pair<unsigned int, NodeStruct>(id, node));
        }
    }
}

void QuestNodeTable::clear() {
    itemNodes.clear();
    npcNodes.clear();
}

