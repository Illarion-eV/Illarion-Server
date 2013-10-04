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

#include "LuaTileScript.hpp"
#include "Character.hpp"
#include "character_ptr.hpp"

LuaTileScript::LuaTileScript(const std::string &filename, const TilesStruct &tile) throw(ScriptException)
    : LuaScript(filename), thisTile(tile) {
    init_functions();
}

LuaTileScript::~LuaTileScript() throw() {}

void LuaTileScript::init_functions() {
    luabind::object globals = luabind::globals(_luaState);
    globals["thisTile"] = thisTile;
}

void LuaTileScript::useTile(Character *user, const position &pos, unsigned char ltastate) {
    character_ptr fuse_user(user);
    callEntrypoint("useTile", fuse_user, pos, ltastate);
}

bool LuaTileScript::actionDisturbed(Character *performer, Character *disturber) {
    character_ptr fuse_performer(performer);
    character_ptr fuse_disturber(disturber);
    return callEntrypoint<bool>("actionDisturbed", fuse_performer, fuse_disturber);
}

