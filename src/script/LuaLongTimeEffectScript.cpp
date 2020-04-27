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

#include "LuaLongTimeEffectScript.hpp"

#include "luabind/luabind.hpp"
#include "Character.hpp"
#include "LongTimeEffect.hpp"
#include "character_ptr.hpp"

LuaLongTimeEffectScript::LuaLongTimeEffectScript(const std::string &filename, const LongTimeEffectStruct &effectStruct)
    : LuaScript(filename), _effectStruct(effectStruct) {
    init_functions();
}

LuaLongTimeEffectScript::~LuaLongTimeEffectScript() {}

void LuaLongTimeEffectScript::init_functions() const {
    luabind::object globals = luabind::globals(_luaState);
    globals["thisEffect"] = _effectStruct;
}

bool LuaLongTimeEffectScript::callEffect(LongTimeEffect *effect, Character *target) {
    character_ptr fuse_target(target);
    return callEntrypoint<bool>("callEffect", effect, fuse_target);
}

void LuaLongTimeEffectScript::doubleEffect(LongTimeEffect *effect, Character *target) {
    character_ptr fuse_target(target);
    callEntrypoint("doubleEffect", effect, fuse_target);
}

void LuaLongTimeEffectScript::loadEffect(LongTimeEffect *effect, Character *target) {
    character_ptr fuse_target(target);
    callEntrypoint("loadEffect", effect, fuse_target);
}

void LuaLongTimeEffectScript::addEffect(LongTimeEffect *effect, Character *target) {
    character_ptr fuse_target(target);
    callEntrypoint("addEffect", effect, fuse_target);
}

void LuaLongTimeEffectScript::removeEffect(LongTimeEffect *effect, Character *target) {
    character_ptr fuse_target(target);
    callEntrypoint("removeEffect", effect, fuse_target);
}

