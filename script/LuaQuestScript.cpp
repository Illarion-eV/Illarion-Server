/*
 *  illarionserver - server for the game Illarion
 *  Copyright 2011 Illarion e.V.
 *
 *  This file is part of illarionserver.
 *
 *  illarionserver is free software: you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation, either version 3 of the License, or
 *  (at your option) any later version.
 *
 *  illarionserver is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with illarionserver.  If not, see <http://www.gnu.org/licenses/>.
 */

#include "LuaQuestScript.hpp"
#include "Character.hpp"
#include "fuse_ptr.hpp"

LuaQuestScript::LuaQuestScript(std::string filename) throw(ScriptException)
    : LuaScript(filename) {
}

LuaQuestScript::~LuaQuestScript() throw() {}

std::string LuaQuestScript::title(Character *user) {
    fuse_ptr<Character> fuse_user(user);
    return callEntrypoint<std::string>("QuestTitle", fuse_user);
}

std::string LuaQuestScript::description(Character *user, TYPE_OF_QUESTSTATUS status) {
    fuse_ptr<Character> fuse_user(user);
    return callEntrypoint<std::string>("QuestDescription", fuse_user, status);
}

void LuaQuestScript::targets(Character *user, TYPE_OF_QUESTSTATUS status, std::vector<position> &targets) {
    targets.clear();
    fuse_ptr<Character> fuse_user(user);
    luabind::object luaTargets = callEntrypoint<luabind::object>("QuestTargets", fuse_user, status);

    using namespace luabind;
    auto mapType = type(luaTargets);

    try {
        position value = object_cast<position>(luaTargets);
        targets.push_back(value);
        return;
    } catch (cast_failed &e) {
    }

    if (mapType == LUA_TTABLE) {
        for (iterator it(luaTargets), end; it != end; ++it) {
            try {
                position value = object_cast<position>(*it);
                targets.push_back(value);
            } catch (cast_failed &e) {
                throw std::logic_error("Usage of invalid target value. Target values must be positions.");
            }
        }
    } else if (mapType == LUA_TNIL) {
        // no targets to add
    } else {
        throw std::logic_error("Usage of invalid quest target type. Quest targets must be tables of positions, positions or nil.");
    }
}

TYPE_OF_QUESTSTATUS LuaQuestScript::finalStatus() {
    return callEntrypoint<TYPE_OF_QUESTSTATUS>("QuestFinalStatus");
}

