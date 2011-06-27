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

extern RaceSizeTable * RaceSizes;


bool World::warpMonster( Monster* cm, Field* cfstart ) {
    if ( cfstart->IsWarpField() ) {
        position oldpos = cm->pos;
        cfstart->GetWarpField( cm->pos );
        Field * cfend;
        if ( findEmptyCFieldNear( cfend, cm->pos.x, cm->pos.y, cm->pos.z ) ) {
            cfstart->SetMonsterOnField( false );
            cfend->SetMonsterOnField( true );

            // allen anderen Spielern den Warp übermitteln
            sendCharacterWarpToAllVisiblePlayers( cm, oldpos, PUSH );
#ifdef World_CharMove_DEBUG
            std::cout << "warpMonster: neu " << cm->pos.x << " " << cm->pos.y << " " << ( short int ) cm->pos.z << "\n";
#endif
            return true;
        } else {
            cm->pos = oldpos;
        }
    }
    return false;
}

void World::checkFieldAfterMove( Character* cc, Field* cfstart ) {
    if ( cfstart->HasSpecialItem() )
    {
        TilesModificatorStruct tmod;
        ITEMVECTOR::iterator theIterator;
        for ( theIterator = cfstart->items.begin(); theIterator < cfstart->items.end(); ++theIterator ) {
            if ( TilesModItems->find( theIterator->id, tmod ) ) {
                if ( ( tmod.Modificator & FLAG_SPECIALITEM ) != 0 )
                {
                    
                    boost::shared_ptr<LuaItemScript> script = CommonItems->findScript( theIterator->id );
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
                    if (cc->character == Character::player) {
                        ((Player*)cc)->startMusic(which.flags);
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

void World::TriggerFieldMove( Character* cc, bool moveto) {
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

bool World::pushCharacter( Player* cp, TYPE_OF_CHARACTER_ID pushedCharId, unsigned char direction ) {
    Character * pushedChar = NULL;
    pushedChar = Players.findID( pushedCharId );
    if ( !pushedChar ) Monsters.findID( pushedCharId );
    if ( pushedChar && pushedChar->pos.z == cp->pos.z && abs(pushedChar->pos.x - cp->pos.x)<=1 && abs(pushedChar->pos.y - cp->pos.y)<=1 )
    {
        return pushedChar->move( (Character::direction)direction,false);
    }
        
    return false;
}

bool World::spinPlayer( Player* cp, unsigned char d ) {

    // die Blickrichtung ändern
    switch ( d ) {
        case 0 :
            cp->faceto = Character::north;
            break;
        case 1 :
            cp->faceto = Character::northeast;
            break;
        case 2 :
            cp->faceto = Character::east;
            break;
        case 3 :
            cp->faceto = Character::southeast;
            break;
        case 4 :
            cp->faceto = Character::south;
            break;
        case 5 :
            cp->faceto = Character::southwest;
            break;
        case 6 :
            cp->faceto = Character::west;
            break;
        case 7 :
            cp->faceto = Character::northwest;
            break;
    }

    // allen sichtbaren Spielern die Drehung übermitteln
    sendSpinToAllVisiblePlayers( cp );

    return true;

}


void World::sendSpinToAllVisiblePlayers( Character* cc ) 
{

    std::vector < Player* > temp = Players.findAllCharactersInRangeOf( cc->pos.x, cc->pos.y, cc->pos.z, MAXVIEW );
    std::vector < Player* > ::iterator titerator;

    for ( titerator = temp.begin(); titerator < temp.end(); ++titerator ) 
    {
        boost::shared_ptr<BasicServerCommand>cmd( new PlayerSpinTC( cc->faceto, cc->id ) );
        ( *titerator )->Connection->addCommand(cmd);
    }

}


void World::sendPassiveMoveToAllVisiblePlayers( Character* ccp ) {

    std::vector < Player* > temp = Players.findAllCharactersInRangeOf( ccp->pos.x, ccp->pos.y, ccp->pos.z, MAXVIEW+1 );

    std::vector < Player* >::iterator titerator;
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
            boost::shared_ptr<BasicServerCommand>cmd(new MoveAckTC( ccp->id, ccp->pos, PUSH, 0 ) );
            ( *titerator )->Connection->addCommand( cmd );
        }
    }

}


void World::sendCharacterMoveToAllVisibleChars( Character* cc, unsigned char waitpages) {
    // for now we only send events to players... TODO change this whole command
    sendCharacterMoveToAllVisiblePlayers( cc, NORMALMOVE, waitpages);
}

void World::sendCharacterMoveToAllVisiblePlayers( Character* cc, unsigned char netid, unsigned char waitpages ) {
    if (!cc->isinvisible) //Nur wenn Character nicht unsichtbar ist die Bewegung übertragen
    {

        std::vector < Player* > temp = Players.findAllCharactersInRangeOf( cc->pos.x, cc->pos.y, cc->pos.z, MAXVIEW+1 );

        std::vector < Player* > ::iterator titerator;
        char xoffs;
        char yoffs;
        char zoffs;
        for ( titerator = temp.begin(); titerator < temp.end(); ++titerator ) {
            xoffs = cc->pos.x - ( *titerator )->pos.x;
            yoffs = cc->pos.y - ( *titerator )->pos.y;
            zoffs = cc->pos.z - ( *titerator )->pos.z + RANGEDOWN;

            if ( ( xoffs != 0 ) || ( yoffs != 0 ) || ( zoffs != RANGEDOWN ) )
            {
                boost::shared_ptr<BasicServerCommand>cmd(new MoveAckTC( cc->id, cc->pos, netid, waitpages) );
                ( *titerator )->Connection->addCommand(cmd);
            }
        }
    }
}


void World::sendCharacterWarpToAllVisiblePlayers( Character* cc, position oldpos, unsigned char netid ) {
    if (!cc->isinvisible) {
        std::vector < Player* > ::iterator titerator;
        char xoffs;
        char yoffs;
        char zoffs;

        // wegwarpen
        sendRemoveCharToVisiblePlayers( cc->id, oldpos );
        
        // hinwarpen
        std::vector < Player* > temp;
        temp = Players.findAllCharactersInRangeOf(cc->pos.x, cc->pos.y, cc->pos.z, MAXVIEW );
        for ( titerator = temp.begin(); titerator < temp.end(); ++titerator ) 
        {
            if ( cc != ( *titerator ) ) 
            {
                xoffs = cc->pos.x - ( *titerator )->pos.x;
                yoffs = cc->pos.y - ( *titerator )->pos.y;
                zoffs = cc->pos.z - ( *titerator )->pos.z + RANGEDOWN;
                boost::shared_ptr<BasicServerCommand> cmd ( new MoveAckTC( cc->id, cc->pos, PUSH, 0 ) );
                ( *titerator )->Connection->addCommand( cmd );
            }
        }
    }
}


void World::sendAllVisibleCharactersToPlayer( Player* cp, bool sendSpin ) {
    std::vector < Player* > tempP = Players.findAllCharactersInRangeOf(cp->pos.x , cp->pos.y, cp->pos.z, MAXVIEW );
    sendCharsInVector< Player >( tempP, cp, sendSpin );

    std::vector < Monster* > tempM = Monsters.findAllCharactersInRangeOf( cp->pos.x , cp->pos.y, cp->pos.z, MAXVIEW );
    sendCharsInVector< Monster >( tempM, cp, sendSpin );

    std::vector < NPC* > tempN = Npc.findAllCharactersInRangeOf( cp->pos.x , cp->pos.y, cp->pos.z, MAXVIEW );
    sendCharsInVector< NPC >( tempN, cp, sendSpin );
}


template< class T >
void World::sendCharsInVector( std::vector < T* > &vec, Player* cp, bool sendSpin ) {
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
                // boost::shared_ptr<BasicServerCommand>cmd( new AppearanceTC( cc ) );
                // cp->sendCharAppearance( cc->id, cmd, false );
                boost::shared_ptr<BasicServerCommand>cmd( new MoveAckTC( cc->id, cc->pos, PUSH, 0 ) );
                cp->Connection->addCommand(cmd);
                cmd.reset( new PlayerSpinTC( cc->faceto, cc->id ) );
                if (sendSpin)cp->Connection->addCommand( cmd ); 
            }
        }
    }
}


bool World::addWarpField( position where, position target, unsigned short int starttilenr, TYPE_OF_ITEM_ID startitemnr ) {
#ifdef World_CharMove_DEBUG
    std::cout << "addWarpField: x: " << where.x << " y: " << where.y << " z: " << where.z << "\n";
#endif
    Field* cfstart;

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


bool World::makeSpecialField( position where, s_fieldattrib which ) {
    std::cout << "addSpecialField: x: " << where.x << " y: " << where.y << " z: " << where.z << "\n";
    Field* cfstart;

    // Startfeld vorhanden
    if ( GetPToCFieldAt( cfstart, where ) ) {
        cfstart->SetSpecialField( true );
        specialfields.insert( FIELDATTRIBHASH::value_type( where, which ) );

        return true;
    } else {
        return false;
    }
}


bool World::makeSpecialField( short int x, short int y, short int z, unsigned char type, unsigned long int value) {

    position where;
    s_fieldattrib which;
    where.x = x;
    where.y = y;
    where.z = z;
    which.type = type;
    which.flags = value;

    return makeSpecialField( where, which );

}


bool World::addWarpField( position where, position target, unsigned short int starttilenr, TYPE_OF_ITEM_ID startitemnr, unsigned short int targettilenr, TYPE_OF_ITEM_ID targetitemnr ) {

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


bool World::addWayUp( position where, unsigned short int starttilenr, TYPE_OF_ITEM_ID startitemnr, unsigned short int targettilenr, TYPE_OF_ITEM_ID targetitemnr ) {

    position target;
    target.x = where.x;
    target.y = where.y;
    target.z = where.z + 1;

    if ( addWarpField( where, target, starttilenr, startitemnr ) ) {
        std::cout << "nach oben ist ok" << std::endl;
        Field * cfp;
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


bool World::removeWarpField( position where ) {

#ifdef World_CharMove_DEBUG
    std::cout << "removeWarpField: x:" << where.x << " y: " << where.y << " z: " << where.z << "\n";
#endif
    Field* cfstart;
    if ( GetPToCFieldAt( cfstart, where ) ) { // Startfeld vorhanden
        cfstart->UnsetWarpField();
        return true;
    }
    return false;
}

/*
bool World::findWarpFieldTarget( position where, const position & target ) {

#ifdef Map_DEBUG
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

