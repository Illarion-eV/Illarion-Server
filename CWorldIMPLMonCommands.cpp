#include "CWorld.hpp"
#include "CCommand.hpp"
#include <sstream>
#include <regex.h>
#include <list>
#include "CTilesModificatorTable.h"
#include "CTilesTable.h"
#include "CArmorObjectTable.h"
#include "CWeaponObjectTable.h"
#include "CContainerObjectTable.h"
#include "CCommonObjectTable.h"
#include "CNamesObjectTable.h"
#include "CMonsterTable.hpp"
#include "CSpellTable.hpp"
#include "CTriggerTable.hpp"
#include "CMonsterAttackTable.hpp"
#include "CNaturalArmorTable.hpp"
#include "CScheduledScriptsTable.hpp"
//We need this for the standard Fighting Script.
#include "script/CLuaWeaponScript.hpp"
#include <boost/shared_ptr.hpp>
#include "CLogger.hpp"
#include "CPlayer.hpp"

#include <iostream>

extern std::ofstream talkfile;

void set_spawn_command( CWorld*, CPlayer*, const std::string& );
void import_maps_command( CWorld*, CPlayer*, const std::string& );
void create_area_command( CWorld*, CPlayer*, const std::string& );
void set_login( CWorld*, CPlayer*, const std::string& );


void CWorld::montool_kill_command(CPlayer * c)
{
    if ( !c->hasGMRight(gmr_reload) )return;
    MONSTERVECTOR::iterator mIterator;
    uint32_t counter = 0;
    for ( mIterator = Monsters.begin() ; mIterator != Monsters.end(); ++mIterator)
    {
        //Kill the monster we have found
        (*mIterator)->increaseAttrib("hitpoints",-((*mIterator)->increaseAttrib("hitpoints",0)+1) );
        ++counter;
    }
    //monitoringClientList->sendCommand( new CSendMessageTS("Sucessfully killed: "+CLogger::toString(counter)+" monsters per nuke from: "+c->name+"("+CLogger::toString(c->id)+")",0));
}

void CWorld::montool_reload_command( CPlayer * c)
{
    /*
    if ( c->hasGMRight(gmr_reload) )
    {
        if (reload_tables(c) ) 
            //monitoringClientList->sendCommand( new CSendMessageTS("db tables loaded succesfully! from "+c->name+"("+CLogger::toString(c->id)+")",0));
        else
            //monitoringClientList->sendCommand( new CSendMessageTS("error while loading tables! from "+c->name+"("+CLogger::toString(c->id)+")",0));
    }
    */
}

void CWorld::montool_kickall_command( CPlayer * c)
{
    if ( c->hasGMRight( gmr_forcelogout ) ) 
    {
	    forceLogoutOfAllPlayers();
        //monitoringClientList->sendCommand( new CSendMessageTS("all players forced to logout! from "+c->name+"("+CLogger::toString(c->id)+")",0));
    }
}

void CWorld::montool_import_maps_command( CPlayer * c)
{
    if ( !c->hasGMRight(gmr_import) )return;
    {
	    load_maps();
        //monitoringClientList->sendCommand( new CSendMessageTS("maps imported! from "+c->name+"("+CLogger::toString(c->id)+")",0));
    }
}

void CWorld::montool_set_login(CPlayer * c, const std::string& st) 
{
    if ( !c->hasGMRight(gmr_loginstate) )return;
	configOptions["disable_login"] = st;
	//std::cout << "nologin set to " << st << std::endl;
	//monitoringClientList->sendCommand( new CSendMessageTS("set login set to: " + st +"! from "+c->name+"("+CLogger::toString(c->id)+")",0));
    
}
