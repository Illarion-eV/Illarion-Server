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
#include "script/LuaScript.hpp"
#include "data/Table.hpp"
#include <boost/shared_ptr.hpp>
#include <boost/filesystem.hpp>
#include <boost/filesystem/fstream.hpp>

struct NodeStruct {
    std::string entrypoint;
    boost::shared_ptr<LuaScript> script;
};

class QuestNodeTable {
private:
    typedef std::multimap<unsigned int, NodeStruct> TABLE;
    typedef TABLE::const_iterator TABLE_ITR;
    typedef std::pair<TABLE_ITR, TABLE_ITR> TABLE_ITRS;

    static QuestNodeTable *instance;
    TABLE itemNodes;
    TABLE npcNodes;

public:
    static QuestNodeTable *getInstance();
    void reload();
    TABLE_ITRS getItemNodes();
    TABLE_ITRS getNpcNodes();

private:
    void readQuest(boost::filesystem::ifstream &questFile, boost::filesystem::path &questPath);
    void clear();

    QuestNodeTable();
    ~QuestNodeTable();
    QuestNodeTable(const QuestNodeTable &);
};

#endif

