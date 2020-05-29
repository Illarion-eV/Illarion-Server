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
    auto operator=(const LuaQuestScript &) -> LuaQuestScript & = delete;
    ~LuaQuestScript() override;

    auto title(Character *user) -> std::string;
    auto description(Character *user, TYPE_OF_QUESTSTATUS status) -> std::string;
    auto available(Character *user, TYPE_OF_QUESTSTATUS status) -> QuestAvailability;
    auto start() -> position;
    void targets(Character *user, TYPE_OF_QUESTSTATUS status, std::vector<position> &targets);
    auto finalStatus() -> TYPE_OF_QUESTSTATUS;

private:
    TYPE_OF_QUEST_ID quest;
    static auto getPosition(const luabind::object &potentialPosition) -> position;
};

#endif

