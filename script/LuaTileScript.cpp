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

#include "LuaTileScript.hpp"
#include "Character.hpp"
#include "fuse_ptr.hpp"

LuaTileScript::LuaTileScript(std::string filename, TilesStruct tile) throw(ScriptException)
    : LuaScript(filename), thisTile(tile) {
    init_functions();
}

LuaTileScript::~LuaTileScript() throw() {}

void LuaTileScript::init_functions() {
    luabind::object globals = luabind::globals(_luaState);
    globals["thisTile"] = thisTile;
}

void LuaTileScript::useTile(Character *user, position pos, unsigned short int counter, unsigned int param, unsigned char ltastate) {
    fuse_ptr<Character> fuse_user(user);
    callEntrypoint("useTile", fuse_user, pos, counter, param, ltastate);
}

bool LuaTileScript::actionDisturbed(Character *performer, Character *disturber) {
    fuse_ptr<Character> fuse_performer(performer);
    fuse_ptr<Character> fuse_disturber(disturber);
    return callEntrypoint<bool>("actionDisturbed", fuse_performer, fuse_disturber);
}

