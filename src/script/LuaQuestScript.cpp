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
#include "character_ptr.hpp"

LuaQuestScript::LuaQuestScript(const std::string &filename, TYPE_OF_QUEST_ID quest) throw(ScriptException)
    : LuaScript(filename), quest(quest) {
}

LuaQuestScript::~LuaQuestScript() throw() {}

std::string LuaQuestScript::title(Character *user) {
    character_ptr fuse_user(user);
    return callEntrypoint<std::string>("QuestTitle", fuse_user);
}

std::string LuaQuestScript::description(Character *user, TYPE_OF_QUESTSTATUS status) {
    character_ptr fuse_user(user);
    return callEntrypoint<std::string>("QuestDescription", fuse_user, status);
}

QuestAvailability LuaQuestScript::available(Character *user, TYPE_OF_QUESTSTATUS status) {
    character_ptr fuse_user(user);
    return callEntrypoint<QuestAvailability>("QuestAvailability", fuse_user, status);
}

position LuaQuestScript::start() {
    using namespace luabind;
    auto startPosition = callEntrypoint<object>("QuestStart");
    
    try {
        return getPosition(startPosition);
    } catch (std::logic_error &e) {
        std::stringstream error;
        error << "No valid QuestStart entrypoint in quest " << quest << ".";
        writeDebugMsg(error.str());
        throw;
    }
}

void LuaQuestScript::targets(Character *user, TYPE_OF_QUESTSTATUS status, std::vector<position> &targets) {
    using namespace luabind;
    targets.clear();
    character_ptr fuse_user(user);
    auto luaTargets = callEntrypoint<object>("QuestTargets", fuse_user, status);

    if (!luaTargets.is_valid()) {
        std::stringstream error;
        error << "No valid QuestTarget entrypoint in quest " << quest << ".";
        writeDebugMsg(error.str());
        return;
    }

    try {
        targets.push_back(getPosition(luaTargets));
        return;
    } catch (std::logic_error &e) {
    }

    auto mapType = type(luaTargets);

    if (mapType == LUA_TTABLE) {
        for (iterator it(luaTargets), end; it != end; ++it) {
            try {
                targets.push_back(getPosition(*it));
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

position LuaQuestScript::getPosition(const luabind::object &potentialPosition) {
    using namespace luabind;

    if (!potentialPosition.is_valid()) {
        throw std::logic_error("no position found");
    }

    try {
        return object_cast<position>(potentialPosition);
    } catch (cast_failed &e) {
    }

    auto positionType = type(potentialPosition);

    if (positionType == LUA_TTABLE) {
        try {
            int16_t x = object_cast<int16_t>(potentialPosition[1]);
            int16_t y = object_cast<int16_t>(potentialPosition[2]);
            int16_t z = object_cast<int16_t>(potentialPosition[3]);
            return position(x, y, z);
        } catch (cast_failed &e) {
        }
    }

    throw std::logic_error("no position found");
}
