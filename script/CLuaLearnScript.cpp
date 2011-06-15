#include "CLuaLearnScript.hpp"
#include "CCharacter.hpp"
#include "CWorld.hpp"
#include "luabind/luabind.hpp"
#include <iostream>
#include <string>
#include "CLogger.hpp"
#include "fuse_ptr.hpp"

CLuaLearnScript::CLuaLearnScript(std::string filename) throw(ScriptException)
		: CLuaScript(filename)
{
}

CLuaLearnScript::~CLuaLearnScript() throw() {}

void CLuaLearnScript::learn( CCharacter * cc, std::string skill, uint8_t skillGroup, uint32_t actionPoints, uint8_t opponent, uint8_t leadAttrib )
{
    try
    {
        CWorld::get()->setCurrentScript( this );
        fuse_ptr<CCharacter> fuse_cc(cc);
        call("learn")( fuse_cc, skill, skillGroup, actionPoints, opponent, leadAttrib );
    }
    catch (luabind::error &e)
    {
        writeErrorMsg();
    }
}

void CLuaLearnScript::reduceMC( CCharacter * cc )
{
    try
    {  
        CWorld::get()->setCurrentScript( this );
        fuse_ptr<CCharacter> fuse_cc(cc);
        call("reduceMC")( fuse_cc );
    }
    catch (luabind::error &e)
    {  
        writeErrorMsg();
    }
}

