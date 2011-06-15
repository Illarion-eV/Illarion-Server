#include "CLuaMagicScript.hpp"

#include "luabind/luabind.hpp"
#include <iostream>

#include "CWorld.hpp"
#include "CLogger.hpp"
#include "fuse_ptr.hpp"

CLuaMagicScript::CLuaMagicScript(std::string filename, unsigned long int MagicFlag) throw(ScriptException)
		: CLuaScript(filename), _MagicFlag(MagicFlag) {
	init_functions();
}

CLuaMagicScript::~CLuaMagicScript() throw() {}

void CLuaMagicScript::init_functions()
{
	luabind::object globals = luabind::globals(_luaState);
	globals["thisSpell"] = _MagicFlag;
}

void CLuaMagicScript::CastMagic(CCharacter * caster, unsigned short counter, unsigned short int param, unsigned char ltastate ) {
	// ToDo Script Calling
	try 
    {
        CWorld::get()->setCurrentScript( this ); 
        //CWorld::get()->monitoringClientList->sendCommand( new CSendMessageTS("CastMagic called for: " + CLogger::toString(_MagicFlag ),3));
        CLogger::writeMessage("scripts","CastMagic called for: " + CLogger::toString(_MagicFlag));
		fuse_ptr<CCharacter> fuse_caster(caster);
        call("CastMagic")( fuse_caster, counter, param, ltastate  );
	} catch (luabind::error &e) {
        //CWorld::get()->monitoringClientList->sendCommand( new CSendMessageTS("Error: CastMagic called for: " + CLogger::toString(_MagicFlag ) + " " + e.what(),3));
        writeErrorMsg();
	}
}

void CLuaMagicScript::CastMagicOnCharacter(CCharacter * caster, CCharacter * target, unsigned short counter, unsigned short int param, unsigned char ltastate ) {
	// ToDo Script Calling
	try 
    {
        CWorld::get()->setCurrentScript( this ); 
        //CWorld::get()->monitoringClientList->sendCommand( new CSendMessageTS("CastMagicOnCharacter called for: " + CLogger::toString(_MagicFlag ),3));
        CLogger::writeMessage("scripts","CastMagicOnCharacter called for: " + CLogger::toString(_MagicFlag));        
		fuse_ptr<CCharacter> fuse_caster(caster);
        fuse_ptr<CCharacter> fuse_target(target);
        call("CastMagicOnCharacter")( fuse_caster, fuse_target, counter, param, ltastate  );
	} catch (luabind::error &e) {
        //CWorld::get()->monitoringClientList->sendCommand( new CSendMessageTS("Error: CastMagicOnCharacter called for: " + CLogger::toString(_MagicFlag ) + " " + e.what(),3));
        writeErrorMsg();
	}
}

void CLuaMagicScript::CastMagicOnField(CCharacter * caster, position pos, unsigned short counter, unsigned short int param, unsigned char ltastate ) {
	// ToDo Script Calling
	try {
        CWorld::get()->setCurrentScript( this ); 
        // CWorld::get()->monitoringClientList->sendCommand( new CSendMessageTS("CastMagicOnField called for: " + CLogger::toString(_MagicFlag ),3));
        CLogger::writeMessage("scripts","CastMagicOnField called for: " + CLogger::toString(_MagicFlag)); 
		fuse_ptr<CCharacter> fuse_caster(caster);
        call("CastMagicOnField")( fuse_caster, pos, counter, param, ltastate  );
	} catch (luabind::error &e) {
        //CWorld::get()->monitoringClientList->sendCommand( new CSendMessageTS("Error: CastMagicOnField called for: " + CLogger::toString(_MagicFlag ) + " " + e.what(),3));
        writeErrorMsg();
	}
}

void CLuaMagicScript::CastMagicOnItem(CCharacter * caster, ScriptItem TargetItem, unsigned short counter, unsigned short int param, unsigned char ltastate ) {
	// ToDo Script Calling
	try {
        CWorld::get()->setCurrentScript( this ); 
        // CWorld::get()->monitoringClientList->sendCommand( new CSendMessageTS("CastMagicOnItem called for: " + CLogger::toString(_MagicFlag ),3));
        CLogger::writeMessage("scripts","CastMagicOnItem called for: " + CLogger::toString(_MagicFlag)); 
		fuse_ptr<CCharacter> fuse_caster(caster);
        call("CastMagicOnItem")( fuse_caster, TargetItem, counter, param, ltastate  );
	} catch (luabind::error &e) {
        //CWorld::get()->monitoringClientList->sendCommand( new CSendMessageTS("Error: CastMagicOnItem called for: " + CLogger::toString(_MagicFlag ) + " " + e.what(),3));
        writeErrorMsg();
	}
}

bool CLuaMagicScript::actionDisturbed(CCharacter * performer, CCharacter * disturber)
{
    try
    {
        CWorld::get()->setCurrentScript( this ); 
        // CWorld::get()->monitoringClientList->sendCommand( new CSendMessageTS("actionDisturbed called for: " + CLogger::toString(_MagicFlag ),3));
        CLogger::writeMessage("scripts","actionDisturbed called for: " + CLogger::toString(_MagicFlag)); 
        fuse_ptr<CCharacter> fuse_performer(performer);
        fuse_ptr<CCharacter> fuse_disturber(disturber);
        return luabind::object_cast<bool>(call("actionDisturbed")( fuse_performer, fuse_disturber ));
    }
    catch ( luabind::error &e)
    {
        return true;
    }
}
