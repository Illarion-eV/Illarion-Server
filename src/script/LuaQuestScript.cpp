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

LuaQuestScript::LuaQuestScript(const std::string &filename, TYPE_OF_QUEST_ID quest) throw(ScriptException)
    : LuaScript(filename), quest(quest) {
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
        addTarget(targets, luaTargets);
        return;
    } catch (std::logic_error &e) {
    }

    if (mapType == LUA_TTABLE) {
        for (iterator it(luaTargets), end; it != end; ++it) {
            try {
                addTarget(targets, *it);
            } catch (std::logic_error &e) {
                std::stringstream error;
                error << "Usage of invalid target table entry in quest " << quest;
                error << " for status " << status << ". A target table entry ";
                error << "must be either a position or a table of three coordinates.";
                writeDebugMsg(error.str());
            }
        }
    } else if (mapType == LUA_TNIL) {
        // no targets to add
    } else {
        std::stringstream error;
        error << "Usage of invalid quest target type in quest " << quest;
        error << " for status " << status << ". You must return a table of positions and/or";
        error << " coordinates, a position, a table of three coordinates or nil in QuestTargets!";
        writeDebugMsg(error.str());
    }
}

TYPE_OF_QUESTSTATUS LuaQuestScript::finalStatus() {
    return callEntrypoint<TYPE_OF_QUESTSTATUS>("QuestFinalStatus");
}

void LuaQuestScript::addTarget(std::vector<position> &targets, const luabind::object &potentialTarget) {
    using namespace luabind;
    auto targetType = type(potentialTarget);

    try {
        targets.push_back(object_cast<position>(potentialTarget));
        return;
    } catch (cast_failed &e) {
    }

    if (targetType == LUA_TTABLE) {
        try {
            int16_t x = object_cast<int16_t>(potentialTarget[1]);
            int16_t y = object_cast<int16_t>(potentialTarget[2]);
            int16_t z = object_cast<int16_t>(potentialTarget[3]);
            targets.emplace_back(x, y, z);
            return;
        } catch (cast_failed &e) {
        }
    }

    throw std::logic_error("no target found");
}
