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


#include "World.hpp"
#include "netinterface/protocol/ServerCommands.hpp"
#include "TilesTable.hpp"
#include "TilesModificatorTable.hpp"
#include "CommonObjectTable.hpp"
#include "script/LuaItemScript.hpp"
#include "Logger.hpp"
#include "netinterface/protocol/BBIWIServerCommands.hpp"
#include "netinterface/NetInterface.hpp"
#include "netinterface/BasicServerCommand.hpp"
#include "Player.hpp"
#include "RaceSizeTable.hpp"

extern CRaceSizeTable * RaceSizes;


bool CWorld::warpMonster( CMonster* cm, CField* cfstart ) {
    if ( cfstart->IsWarpField() ) {
        position oldpos = cm->pos;
        cfstart->GetWarpField( cm->pos );
        CField * cfend;
        if ( findEmptyCFieldNear( cfend, cm->pos.x, cm->pos.y, cm->pos.z ) ) {
            cfstart->SetMonsterOnField( false );
            cfend->SetMonsterOnField( true );

            // allen anderen Spielern den Warp übermitteln
            sendCharacterWarpToAllVisiblePlayers( cm, oldpos, PUSH );
#ifdef CWorld_CharMove_DEBUG
            std::cout << "warpMonster: neu " << cm->pos.x << " " << cm->pos.y << " " << ( short int ) cm->pos.z << "\n";
#endif
            return true;
        } else {
            cm->pos = oldpos;
        }
    }
    return false;
}

void CWorld::checkFieldAfterMove( CCharacter* cc, CField* cfstart ) {
    if ( cfstart->HasSpecialItem() )
    {
        TilesModificatorStruct tmod;
        ITEMVECTOR::iterator theIterator;
        for ( theIterator = cfstart->items.begin(); theIterator < cfstart->items.end(); ++theIterator ) {
            if ( TilesModItems->find( theIterator->id, tmod ) ) {
                if ( ( tmod.Modificator & FLAG_SPECIALITEM ) != 0 )
                {
                    
                    boost::shared_ptr<CLuaItemScript> script = CommonItems->findScript( theIterator->id );
                    if ( script )
                    {
                        script->CharacterOnField(cc);
                        return;
                    }
                        
                    /*
                    if ( CommonItems->find( theIterator->id, com) ) //Script finden für das Item
                    {
                        if ( com.script ) {
                            com.script->CharacterOnField(cc);
                            return;
                        }

                    }
                    */
                }
            }
        }
    }

    if ( cfstart->IsSpecialField() ) {
        s_fieldattrib which;
        FIELDATTRIBHASH::iterator temp = specialfields.find( cc->pos );
        if ( specialfields.end() != temp ) {
            which = ( *temp ).second;
            switch (which.type) {
                case SOUNDFIELD:
                    makeSoundForAllPlayersInRange( cc->pos.x, cc->pos.y, cc->pos.z, 3, which.flags );
                    break;
                case MUSICFIELD:
                    if (cc->character == CCharacter::player) {
                        ((CPlayer*)cc)->startMusic(which.flags);
                    }
                    break;

                default:
                    ;

            }
        }
    }
    //Struct für Scriptdaten
    TriggerStruct trigger;
    //Schauen ob für die Position auf die der Char gegangen ist ein Triggerscript ausgeführt werden soll
    if ( Triggers->find(cc->pos,trigger) ) {
        //Wenn ein Gültiges Triggerscript für die Position verfügbar ist CharacterOnField ausführen
        if ( trigger.script ) {
            trigger.script->CharacterOnField(cc);
        }
    }
}

void CWorld::TriggerFieldMove( CCharacter* cc, bool moveto) {
    TriggerStruct trigger;
    if ( Triggers->find(cc->pos,trigger) ) {
        //Wenn ein Gültiges Triggerscript für die Position verfügbar ist CharacterOnField ausführen
        if ( trigger.script ) {
            if ( moveto )
                trigger.script->MoveToField(cc);
            else
                trigger.script->MoveFromField(cc);
        }
    }
}

bool CWorld::pushCharacter( CPlayer* cp, TYPE_OF_CHARACTER_ID pushedCharId, unsigned char direction ) {
    CCharacter * pushedChar = NULL;
    pushedChar = Players.findID( pushedCharId );
    if ( !pushedChar ) Monsters.findID( pushedCharId );
    if ( pushedChar && pushedChar->pos.z == cp->pos.z && abs(pushedChar->pos.x - cp->pos.x)<=1 && abs(pushedChar->pos.y - cp->pos.y)<=1 )
    {
        return pushedChar->move( (CCharacter::direction)direction,false);
    }
        
    return false;
}

bool CWorld::spinPlayer( CPlayer* cp, unsigned char d ) {

    // die Blickrichtung ändern
    switch ( d ) {
        case 0 :
            cp->faceto = CCharacter::north;
            break;
        case 1 :
            cp->faceto = CCharacter::northeast;
            break;
        case 2 :
            cp->faceto = CCharacter::east;
            break;
        case 3 :
            cp->faceto = CCharacter::southeast;
            break;
        case 4 :
            cp->faceto = CCharacter::south;
            break;
        case 5 :
            cp->faceto = CCharacter::southwest;
            break;
        case 6 :
            cp->faceto = CCharacter::west;
            break;
        case 7 :
            cp->faceto = CCharacter::northwest;
            break;
    }

    // allen sichtbaren Spielern die Drehung übermitteln
    sendSpinToAllVisiblePlayers( cp );

    return true;

}


void CWorld::sendSpinToAllVisiblePlayers( CCharacter* cc ) 
{

    std::vector < CPlayer* > temp = Players.findAllCharactersInRangeOf( cc->pos.x, cc->pos.y, cc->pos.z, MAXVIEW );
    std::vector < CPlayer* > ::iterator titerator;

    for ( titerator = temp.begin(); titerator < temp.end(); ++titerator ) 
    {
        boost::shared_ptr<CBasicServerCommand>cmd( new CPlayerSpinTC( cc->faceto, cc->id ) );
        ( *titerator )->Connection->addCommand(cmd);
    }

}


void CWorld::sendPassiveMoveToAllVisiblePlayers( CCharacter* ccp ) {

    std::vector < CPlayer* > temp = Players.findAllCharactersInRangeOf( ccp->pos.x, ccp->pos.y, ccp->pos.z, MAXVIEW+1 );

    std::vector < CPlayer* >::iterator titerator;
    char xoffs;
    char yoffs;
    char zoffs;
    for ( titerator = temp.begin(); titerator < temp.end(); ++titerator ) 
    {
        xoffs = ccp->pos.x - ( *titerator )->pos.x;
        yoffs = ccp->pos.y - ( *titerator )->pos.y;
        zoffs = ccp->pos.z - ( *titerator )->pos.z + RANGEDOWN;

        if ( ( xoffs != 0 ) || ( yoffs != 0 ) || ( zoffs != RANGEDOWN ) ) 
        {
            boost::shared_ptr<CBasicServerCommand>cmd(new CMoveAckTC( ccp->id, ccp->pos, PUSH, 0 ) );
            ( *titerator )->Connection->addCommand( cmd );
        }
    }

}


void CWorld::sendCharacterMoveToAllVisibleChars( CCharacter* cc, unsigned char waitpages) {
    // for now we only send events to players... TODO change this whole command
    sendCharacterMoveToAllVisiblePlayers( cc, NORMALMOVE, waitpages);
}

void CWorld::sendCharacterMoveToAllVisiblePlayers( CCharacter* cc, unsigned char netid, unsigned char waitpages ) {
    if (!cc->isinvisible) //Nur wenn Character nicht unsichtbar ist die Bewegung übertragen
    {

        std::vector < CPlayer* > temp = Players.findAllCharactersInRangeOf( cc->pos.x, cc->pos.y, cc->pos.z, MAXVIEW+1 );

        std::vector < CPlayer* > ::iterator titerator;
        char xoffs;
        char yoffs;
        char zoffs;
        for ( titerator = temp.begin(); titerator < temp.end(); ++titerator ) {
            xoffs = cc->pos.x - ( *titerator )->pos.x;
            yoffs = cc->pos.y - ( *titerator )->pos.y;
            zoffs = cc->pos.z - ( *titerator )->pos.z + RANGEDOWN;

            if ( ( xoffs != 0 ) || ( yoffs != 0 ) || ( zoffs != RANGEDOWN ) )
            {
                boost::shared_ptr<CBasicServerCommand>cmd(new CMoveAckTC( cc->id, cc->pos, netid, waitpages) );
                ( *titerator )->Connection->addCommand(cmd);
            }
        }
    }
}


void CWorld::sendCharacterWarpToAllVisiblePlayers( CCharacter* cc, position oldpos, unsigned char netid ) {
    if (!cc->isinvisible) {
        std::vector < CPlayer* > ::iterator titerator;
        char xoffs;
        char yoffs;
        char zoffs;

        // wegwarpen
        sendRemoveCharToVisiblePlayers( cc->id, oldpos );
        
        // hinwarpen
        std::vector < CPlayer* > temp;
        temp = Players.findAllCharactersInRangeOf(cc->pos.x, cc->pos.y, cc->pos.z, MAXVIEW );
        for ( titerator = temp.begin(); titerator < temp.end(); ++titerator ) 
        {
            if ( cc != ( *titerator ) ) 
            {
                xoffs = cc->pos.x - ( *titerator )->pos.x;
                yoffs = cc->pos.y - ( *titerator )->pos.y;
                zoffs = cc->pos.z - ( *titerator )->pos.z + RANGEDOWN;
                boost::shared_ptr<CBasicServerCommand> cmd ( new CMoveAckTC( cc->id, cc->pos, PUSH, 0 ) );
                ( *titerator )->Connection->addCommand( cmd );
            }
        }
    }
}


void CWorld::sendAllVisibleCharactersToPlayer( CPlayer* cp, bool sendSpin ) {
    std::vector < CPlayer* > tempP = Players.findAllCharactersInRangeOf(cp->pos.x , cp->pos.y, cp->pos.z, MAXVIEW );
    sendCharsInVector< CPlayer >( tempP, cp, sendSpin );

    std::vector < CMonster* > tempM = Monsters.findAllCharactersInRangeOf( cp->pos.x , cp->pos.y, cp->pos.z, MAXVIEW );
    sendCharsInVector< CMonster >( tempM, cp, sendSpin );

    std::vector < CNPC* > tempN = Npc.findAllCharactersInRangeOf( cp->pos.x , cp->pos.y, cp->pos.z, MAXVIEW );
    sendCharsInVector< CNPC >( tempN, cp, sendSpin );
}


template< class T >
void CWorld::sendCharsInVector( std::vector < T* > &vec, CPlayer* cp, bool sendSpin ) {
    typename std::vector < T* > ::iterator titerator;
    char xoffs;
    char yoffs;
    char zoffs;
    T* cc;
    for ( titerator = vec.begin(); titerator < vec.end(); ++titerator )
    {
        cc = *titerator;
        if( !cc->isinvisible ) 
        {
            xoffs = cc->pos.x - cp->pos.x;
            yoffs = cc->pos.y - cp->pos.y;
            zoffs = cc->pos.z - cp->pos.z + RANGEDOWN;
            if ( ( xoffs != 0 ) || ( yoffs != 0 ) || ( zoffs != RANGEDOWN ) ) 
            {
                // boost::shared_ptr<CBasicServerCommand>cmd( new CAppearanceTC( cc ) );
                // cp->sendCharAppearance( cc->id, cmd, false );
                boost::shared_ptr<CBasicServerCommand>cmd( new CMoveAckTC( cc->id, cc->pos, PUSH, 0 ) );
                cp->Connection->addCommand(cmd);
                cmd.reset( new CPlayerSpinTC( cc->faceto, cc->id ) );
                if (sendSpin)cp->Connection->addCommand( cmd ); 
            }
        }
    }
}


bool CWorld::addWarpField( position where, position target, unsigned short int starttilenr, TYPE_OF_ITEM_ID startitemnr ) {
#ifdef CWorld_CharMove_DEBUG
    std::cout << "addWarpField: x: " << where.x << " y: " << where.y << " z: " << where.z << "\n";
#endif
    CField* cfstart;

    // Startfeld vorhanden
    if ( GetPToCFieldAt( cfstart, where ) ) {

        if ( starttilenr != 0 ) {
            cfstart->setTileId( starttilenr );
        }

        if ( startitemnr != 0 ) {
            Item warpfi;
            warpfi.id = startitemnr;
            warpfi.number = 1;
            warpfi.wear = 255;
            cfstart->PutTopItem( warpfi );
        }

        cfstart->SetWarpField( target );
        cfstart->updateFlags();

        return true;
    } else {
        return false;
    }

}


bool CWorld::makeSpecialField( position where, s_fieldattrib which ) {
    std::cout << "addSpecialField: x: " << where.x << " y: " << where.y << " z: " << where.z << "\n";
    CField* cfstart;

    // Startfeld vorhanden
    if ( GetPToCFieldAt( cfstart, where ) ) {
        cfstart->SetSpecialField( true );
        specialfields.insert( FIELDATTRIBHASH::value_type( where, which ) );

        return true;
    } else {
        return false;
    }
}


bool CWorld::makeSpecialField( short int x, short int y, short int z, unsigned char type, unsigned long int value) {

    position where;
    s_fieldattrib which;
    where.x = x;
    where.y = y;
    where.z = z;
    which.type = type;
    which.flags = value;

    return makeSpecialField( where, which );

}


bool CWorld::addWarpField( position where, position target, unsigned short int starttilenr, TYPE_OF_ITEM_ID startitemnr, unsigned short int targettilenr, TYPE_OF_ITEM_ID targetitemnr ) {

    if ( addWarpField( where, target, starttilenr, startitemnr ) ) {

        std::cout << "addWarp Unten -> Oben ist ok" << std::endl;

        if ( addWarpField( target, where, targettilenr, targetitemnr ) ) {
            std::cout << "adwarp Oben -> Unten ist ok" << std::endl;
            return true;
        } else {
            removeWarpField( where );
        }
    }

    return false;

}


bool CWorld::addWayUp( position where, unsigned short int starttilenr, TYPE_OF_ITEM_ID startitemnr, unsigned short int targettilenr, TYPE_OF_ITEM_ID targetitemnr ) {

    position target;
    target.x = where.x;
    target.y = where.y;
    target.z = where.z + 1;

    if ( addWarpField( where, target, starttilenr, startitemnr ) ) {
        std::cout << "nach oben ist ok" << std::endl;
        CField * cfp;
        if ( GetPToCFieldAt( cfp, target.x, target.y+1, target.z ) ) {
            std::cout << "Zielfeld gefunden" << std::endl;
            if ( targettilenr != 0 ) {
                cfp->setTileId( targettilenr );
            }
            if ( targetitemnr != 0 ) {
                Item trapdoor;
                trapdoor.id = targetitemnr;
                trapdoor.number = 1;
                trapdoor.wear = 255;
                std::cout << "vor PutItem" << std::endl;
                cfp->PutTopItem( trapdoor );
                std::cout << "nach PutItem" << std::endl;
            }

            target.y = target.y + 1;
            where.y = where.y + 1;

            cfp->SetWarpField( target );
            return true;
        } else {
            removeWarpField( where );
        }
    }

    return false;

}


bool CWorld::removeWarpField( position where ) {

#ifdef CWorld_CharMove_DEBUG
    std::cout << "removeWarpField: x:" << where.x << " y: " << where.y << " z: " << where.z << "\n";
#endif
    CField* cfstart;
    if ( GetPToCFieldAt( cfstart, where ) ) { // Startfeld vorhanden
        cfstart->UnsetWarpField();
        return true;
    }
    return false;
}

/*
bool CWorld::findWarpFieldTarget( position where, const position & target ) {

#ifdef CMap_DEBUG
    std::cout << "findWarpField: x: " << where.x << " y: " << where.y << " z: " << where.z << "\n";
#endif

    POSITIONHASH::iterator temp = warpfields.find( where );
    if ( warpfields.end() != temp ) {
        target = ( *temp ).second;
        return true;
    }

    return false;

}
*/

