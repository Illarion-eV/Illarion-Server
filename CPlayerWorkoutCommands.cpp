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
