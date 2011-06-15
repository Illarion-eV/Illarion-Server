#include "CLuaLongTimeEffectScript.hpp"

#include "luabind/luabind.hpp"
#include "CLogger.hpp"
#include <iostream>
#include "CWorld.hpp"
#include "CCharacter.hpp"
#include "CLongTimeEffect.hpp"
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
