//  illarionserver - server for the game Illarion
//  Copyright 2011 Illarion e.V.
//
//  This file is part of illarionserver.
//
//  illarionserver is free software: you can redistribute it and/or modify
//  it under the terms of the GNU General Public License as published by
//  the Free Software Foundation, either version 3 of the License, or
//  (at your option) any later version.
//
//  illarionserver is distributed in the hope that it will be useful,
//  but WITHOUT ANY WARRANTY; without even the implied warranty of
//  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//  GNU General Public License for more details.
//
//  You should have received a copy of the GNU General Public License
//  along with illarionserver.  If not, see <http://www.gnu.org/licenses/>.


#include "LuaLongTimeEffectScript.hpp"

#include "luabind/luabind.hpp"
#include "Logger.hpp"
#include <iostream>
#include "World.hpp"
#include "Character.hpp"
#include "LongTimeEffect.hpp"
#include "fuse_ptr.hpp"

LuaLongTimeEffectScript::LuaLongTimeEffectScript(std::string filename, LongTimeEffectStruct effectStruct) throw(ScriptException)
    : LuaScript(filename), _effectStruct(effectStruct) {
    init_functions();
}

LuaLongTimeEffectScript::~LuaLongTimeEffectScript() throw() {}

void LuaLongTimeEffectScript::init_functions() {
    luabind::object globals = luabind::globals(_luaState);
    globals["thisEffect"] = _effectStruct;
}

bool LuaLongTimeEffectScript::callEffect(LongTimeEffect *effect, Character *target) {
    try {
        World::get()->setCurrentScript(this);
        Logger::writeMessage("scripts","callEffect called for: " + target->name + " effect: " + Logger::toString(effect->_effectId));
        fuse_ptr<Character> fuse_target(target);
        return luabind::object_cast<bool>(call("callEffect")(effect, fuse_target));
    } catch (luabind::cast_failed &e) {
        writeCastErrorMsg("callEffect", "bool");
    } catch (luabind::error &e) {
        writeErrorMsg();
    }
    return false;
}

void LuaLongTimeEffectScript::doubleEffect(LongTimeEffect *effect, Character *target) {
    try {
        //CWorld::get()->setCurrentScript( this );
        Logger::writeMessage("scripts","doubleEffect called for: " + target->name + " effect: " + Logger::toString(effect->_effectId));
        fuse_ptr<Character> fuse_target(target);
        call("doubleEffect")(effect, fuse_target);
    } catch (luabind::error  &e) {
        writeErrorMsg();

    }
}

void LuaLongTimeEffectScript::loadEffect(LongTimeEffect *effect, Player *target) {
    try {
        World::get()->setCurrentScript(this);
        Logger::writeMessage("scripts","loadEffect called for: " + target->name + " effect: " + Logger::toString(effect->_effectId));
        fuse_ptr<Character> fuse_target(target);
        call("loadEffect")(effect, fuse_target);
    } catch (luabind::error &e) {
        writeErrorMsg();
    }
}

void LuaLongTimeEffectScript::addEffect(LongTimeEffect *effect, Character *target) {
    try {
        //CWorld::get()->setCurrentScript( this );
        Logger::writeMessage("scripts","addEffect called for: " + target->name + " effect: " + Logger::toString(effect->_effectId));
        fuse_ptr<Character> fuse_target(target);
        call("addEffect")(effect, fuse_target);
    } catch (luabind::error &e) {
        writeErrorMsg();
    }
}

void LuaLongTimeEffectScript::removeEffect(LongTimeEffect *effect, Character *target) {
    try {
        //CWorld::get()->setCurrentScript( this );
        Logger::writeMessage("scripts","removeEffect called for: " + target->name + " effect: " + Logger::toString(effect->_effectId));
        fuse_ptr<Character> fuse_target(target);
        call("removeEffect")(effect, fuse_target);
    } catch (luabind::error &e) {
        writeErrorMsg();
    }
}
