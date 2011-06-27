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

CLuaLongTimeEffectScript::CLuaLongTimeEffectScript(std::string filename, LongTimeEffectStruct effectStruct) throw(ScriptException)
        : CLuaScript(filename), _effectStruct(effectStruct)
{
    init_functions();
}

CLuaLongTimeEffectScript::~CLuaLongTimeEffectScript() throw() {}

void CLuaLongTimeEffectScript::init_functions()
{
    luabind::object globals = luabind::globals(_luaState);
    globals["thisEffect"] = _effectStruct;
}

bool CLuaLongTimeEffectScript::callEffect(CLongTimeEffect * effect, CCharacter * target)
{
    try
    {
        CWorld::get()->setCurrentScript( this );
        CLogger::writeMessage("scripts","callEffect called for: " + target->name + " effect: " + CLogger::toString( effect->_effectId));
        fuse_ptr<CCharacter> fuse_target(target);
        return luabind::object_cast<bool>(call("callEffect")( effect, fuse_target ));
    }
    catch ( luabind::error &e)
    {
        writeErrorMsg();
        return false;
    }
}

void CLuaLongTimeEffectScript::doubleEffect(CLongTimeEffect * effect, CCharacter * target)
{
    try
    {
        //CWorld::get()->setCurrentScript( this );
        CLogger::writeMessage("scripts","doubleEffect called for: " + target->name + " effect: " + CLogger::toString( effect->_effectId));
        fuse_ptr<CCharacter> fuse_target(target);
        call("doubleEffect")( effect, fuse_target );
    }
    catch ( luabind::error  &e)
    {
        writeErrorMsg();
        
    }
}

void CLuaLongTimeEffectScript::loadEffect(CLongTimeEffect * effect, CPlayer * target)
{
    try
    {
        CWorld::get()->setCurrentScript( this );
        CLogger::writeMessage("scripts","loadEffect called for: " + target->name + " effect: " + CLogger::toString( effect->_effectId));
        fuse_ptr<CCharacter> fuse_target(target);
        call("loadEffect")( effect, fuse_target );
    }
    catch ( luabind::error &e)
    {
        writeErrorMsg();
    }
}

void CLuaLongTimeEffectScript::addEffect(CLongTimeEffect * effect, CCharacter * target)
{
    try
    {
        //CWorld::get()->setCurrentScript( this );
        CLogger::writeMessage("scripts","addEffect called for: " + target->name + " effect: " + CLogger::toString( effect->_effectId));
        fuse_ptr<CCharacter> fuse_target(target);
        call("addEffect")( effect, fuse_target );
    }
    catch ( luabind::error &e)
    {
        writeErrorMsg();
    }
}

void CLuaLongTimeEffectScript::removeEffect(CLongTimeEffect * effect, CCharacter * target)
{
    try
    {
        //CWorld::get()->setCurrentScript( this );
        CLogger::writeMessage("scripts","removeEffect called for: " + target->name + " effect: " + CLogger::toString( effect->_effectId));
        fuse_ptr<CCharacter> fuse_target(target);
        call("removeEffect")( effect, fuse_target );
    }
    catch ( luabind::error &e)
    {
        writeErrorMsg();
    }
}
