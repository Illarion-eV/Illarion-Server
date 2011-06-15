#include "CLuaLookAtPlayerScript.hpp"
#include "CCharacter.hpp"
#include "CWorld.hpp"
#include "luabind/luabind.hpp"
#include <iostream>
#include "CLogger.hpp"
#include "fuse_ptr.hpp"

CLuaLookAtPlayerScript::CLuaLookAtPlayerScript(std::string filename) throw(ScriptException)
		: CLuaScript(filename)
{
}

CLuaLookAtPlayerScript::~CLuaLookAtPlayerScript() throw() {}

void CLuaLookAtPlayerScript::lookAtPlayer(CCharacter * source, CCharacter * target, unsigned char mode)
{
    try
    {
        CWorld::get()->setCurrentScript( this ); 
        fuse_ptr<CCharacter> fuse_source(source);
        fuse_ptr<CCharacter> fuse_target(target);
        call("lookAtPlayer")( fuse_source, fuse_target, mode );
    }
    catch (luabind::error &e)
    {
        writeErrorMsg();    
    }

}
