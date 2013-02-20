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

#ifndef _QUEST_NODE_TABLE_HPP_
#define _QUEST_NODE_TABLE_HPP_

#include <string>
#include <vector>
#include "script/LuaScript.hpp"
#include <memory>
#include <boost/filesystem.hpp>
#include <boost/filesystem/fstream.hpp>

struct NodeStruct {
    std::string entrypoint;
    std::shared_ptr<LuaScript> script;
};

struct TriggerNodeStruct {
    position pos;
    std::string entrypoint;
    std::shared_ptr<LuaScript> script;
};

class QuestNodeTable {
private:
    typedef std::multimap<unsigned int, NodeStruct> TABLE;
    typedef TABLE::const_iterator TABLE_ITR;
    typedef std::vector<TriggerNodeStruct> TRIGGERVECTOR;
    typedef TRIGGERVECTOR::const_iterator TRIGGERVECTOR_ITR;
    typedef std::pair<TRIGGERVECTOR_ITR, TRIGGERVECTOR_ITR> TRIGGERVECTOR_ITRS;

    static QuestNodeTable *instance;
    TABLE itemNodes;
    TABLE npcNodes;
    TABLE monsterNodes;
    TRIGGERVECTOR triggerNodes;

public:
    typedef std::pair<TABLE_ITR, TABLE_ITR> TABLE_ITRS;

    static QuestNodeTable *getInstance();
    void reload();
    TABLE_ITRS getItemNodes();
    TABLE_ITRS getNpcNodes();
    TABLE_ITRS getMonsterNodes();
    TRIGGERVECTOR_ITRS getTriggerNodes();

private:
    void readQuest(boost::filesystem::ifstream &questFile, boost::filesystem::path &questPath);
    void clear();

    QuestNodeTable();
    ~QuestNodeTable();
    QuestNodeTable(const QuestNodeTable &);
};

#endif

