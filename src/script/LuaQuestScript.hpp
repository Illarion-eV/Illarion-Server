/*
 *  illarionserver - server for the game Illarion
 *  Copyright 2011 Illarion e.V.
 *
 *  This file is part of illarionserver.
 *
 *  illarionserver is free software: you can redistribute it and/or modify
 *  it under the terms of the GNU Affero General Public License as published by
 *  the Free Software Foundation, either version 3 of the License, or
 *  (at your option) any later version.
 *
 *  illarionserver is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU Affero General Public License for more details.
 *
 *  You should have received a copy of the GNU Affero General Public License
 *  along with illarionserver.  If not, see <http://www.gnu.org/licenses/>.
 */

#ifndef _LUA_QUEST_SCRIPT_HPP_
#define _LUA_QUEST_SCRIPT_HPP_

#include "LuaScript.hpp"
#include <string>
#include <vector>
#include "globals.hpp"
#include "types.hpp"

class Character;

class LuaQuestScript : public LuaScript {
public:
    LuaQuestScript(const std::string &filename, TYPE_OF_QUEST_ID quest);
    LuaQuestScript(const LuaQuestScript &) = delete;
    LuaQuestScript &operator=(const LuaQuestScript &) = delete;
    ~LuaQuestScript() override;

    std::string title(Character *user);
    std::string description(Character *user, TYPE_OF_QUESTSTATUS status);
    QuestAvailability available(Character *user, TYPE_OF_QUESTSTATUS status);
    position start();
    void targets(Character *user, TYPE_OF_QUESTSTATUS status, std::vector<position> &targets);
    TYPE_OF_QUESTSTATUS finalStatus();

private:
    TYPE_OF_QUEST_ID quest;
    static position getPosition(const luabind::object &potentialPosition);
};

#endif

