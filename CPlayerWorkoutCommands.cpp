#include "CWorld.hpp"
#include "CPlayer.hpp"
#include <boost/shared_ptr.hpp>
#include "netinterface/CNetInterface.hpp"
#include "netinterface/protocol/ClientCommands.hpp"


void CPlayer::workoutCommands()
{
#ifdef _PLAYER_AUTO_SAVE_    
    checkSave();
#endif    
    boost::shared_ptr<CBasicClientCommand> cmd = Connection->getCommand();
    if ( cmd )
    {
        cmd->performAction( this );
    }
#ifdef DO_UNCONSCIOUS
	else if ( IsAlive() && IsConscious() )
#else
	else if ( IsAlive() )
#endif
	{
		if ( attackmode ) 
        {
            //cp->ltAction->abortAction();
			CWorld::get()->characterAttacks( this );
		}
    }
    cmd.reset();
}
