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

#include "LuaLearnScript.hpp"

#include "Character.hpp"
#include "character_ptr.hpp"

#include <string>

LuaLearnScript::LuaLearnScript(const std::string &filename) : LuaScript(filename) {}

LuaLearnScript::~LuaLearnScript() = default;

void LuaLearnScript::learn(Character *cc, TYPE_OF_SKILL_ID skill, uint32_t actionPoints, uint8_t opponent) {
    character_ptr fuse_cc(cc);
    callEntrypoint("learn", fuse_cc, skill, actionPoints, opponent);
}

void LuaLearnScript::reduceMC(Character *cc) {
    character_ptr fuse_cc(cc);
    callEntrypoint("reduceMC", fuse_cc);
}
