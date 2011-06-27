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


#define _XOPEN_SOURCE
#include <unistd.h>

#include "db/ConnectionManager.hpp"
#include "Player.hpp"
#include <sstream>
#include <memory>
#include "tuningConstants.hpp"
#include "ContainerObjectTable.hpp"
#include "CommonObjectTable.hpp"
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include "Field.hpp"
#include "World.hpp"
#include "netinterface/NetInterface.hpp"
#include "netinterface/BasicClientCommand.hpp"
//for login command
#include "netinterface/protocol/ClientCommands.hpp"
#include "netinterface/protocol/ServerCommands.hpp"
//#include "playersave.hpp"
#include "Random.hpp"
#include "SchedulerTaskClasses.hpp"
#include "netinterface/protocol/BBIWIServerCommands.hpp"
#include "Logger.hpp"
#include "tvector.hpp"
#include "PlayerManager.hpp"

//#define PLAYER_MOVE_DEBUG

template<> const std::string toString(const unsigned char& convertme) {
        return toString((unsigned short)convertme);
}

Player::Player(boost::shared_ptr<NetInterface> newConnection) throw (LogoutException)
               : Character(), mapshowcaseopen(false), onlinetime(0), Connection(newConnection),
turtleActive(false), clippingActive(true), admin(false) {
#ifdef Player_DEBUG
        std::cout << "Player Konstruktor Start" << std::endl;
#endif

        screenwidth = 0;
        screenheight = 0;
        character = player;
        SetAlive( true );
        SetMovement( walk );
        prefix="";
        suffix="";

        time( &lastaction );
        
        ltAction = new LongTimeAction( this, _world );

        // first check if we have a valid client

        boost::shared_ptr<BasicClientCommand> cmd = Connection->getCommand();
        if ( (cmd == NULL) || cmd->getDefinitionByte() != C_LOGIN_TS )throw LogoutException(UNSTABLECONNECTION);
        unsigned short int clientversion = boost::dynamic_pointer_cast<LoginCommandTS>(cmd)->clientVersion;
        name = boost::dynamic_pointer_cast<LoginCommandTS>(cmd)->loginName;
        pw = boost::dynamic_pointer_cast<LoginCommandTS>(cmd)->passwort;
        // set acceptable client version...
        unsigned short acceptVersion;
        std::stringstream stream;
        stream << configOptions["clientversion"];
        stream >> acceptVersion;
    monitoringClient = false;
    if ( clientversion == 200 )
    {
        monitoringClient = true;
    }
    else if ( clientversion != acceptVersion)
    {
               std::cerr << "old client: " << clientversion << " != " << acceptVersion << std::endl;
               throw LogoutException(OLDCLIENT);
    }

        // client version seems to be ok... now find out username/password he supplied...

        if (name == "" || pw == "") 
        {
               // no name/password retrieved... kick him...
               std::cerr << "no name/pwd recvd" << std::endl;
               throw LogoutException(WRONGPWD);
        }

        // player already online? if we don't use the monitoring client
        if ( !monitoringClient & (_world->Players.find(name) || PlayerManager::get()->findPlayer(name) ) ) 
        {
               std::cout << "double login by " << name  << std::endl;
               throw LogoutException(DOUBLEPLAYER);
        }

        std::cerr << "checking login data for: " << name << std::endl;
        // next check if username/password are ok
        check_logindata();
        std::cerr << "checking login data done" << std::endl;
         if (status == 7) {
               // no skill package chosen... kick him...
               std::cerr << "no skill package chosen" << std::endl;
               throw LogoutException(NOSKILLS);
               std::cerr << "no package chosen" << std::endl;
        }
        
        if ( !loadGMFlags() )
            throw LogoutException(UNSTABLECONNECTION); //Fehler beim Laden der GM daten
        std::cerr << "error loading gm flags" << std::endl;
        
        
    if ( !hasGMRight(gmr_allowlogin) && configOptions["disable_login"] == "true") throw Player::LogoutException(SERVERSHUTDOWN);  
        std::cerr << "no gm rights" << std::endl;
        if ( monitoringClient && !hasGMRight(gmr_ban) ) throw Player::LogoutException(NOACCOUNT);
        last_ip = Connection->getIPAdress();
        std::cerr << "write last IP" << std::endl;
        //we dont want to add more if we have a monitoring client
        if ( monitoringClient )
        {
            std::cout<<"login monitoring client: "<<name<<std::endl;
            return;
        }
            
        // now load inventory...
        if( !load() )
            throw LogoutException(ORRUPTDATA);
        std::cerr << "loading inventory done" << std::endl;

#ifdef Player_DEBUG
        std::cout << "Player Konstruktor Ende" << std::endl;
#endif

}

void Player::login() throw(LogoutException)
{
        // find a position for our player...
        short int x,y,z;
        bool target_position_found;
        Field* target_position;

        if (status == JAILED || status == JAILEDFORTIME) {
               // player is in jail...
               std::stringstream ssx(configOptions["jail_x"]);
               ssx >> x;
               std::stringstream ssy(configOptions["jail_y"]);
               ssy >> y;
               std::stringstream ssz(configOptions["jail_z"]);
               ssz >> z;
               target_position_found = _world->findEmptyCFieldNear(target_position, x, y, z);
        } else {
               // try to find a targetposition near the logout place...
               x = pos.x;
               y = pos.y;
               z = pos.z;
               target_position_found = _world->findEmptyCFieldNear(target_position, x, y, z);

               if (!target_position_found) {
                      // move player to startingpoint...
                      std::stringstream ssx(configOptions["playerstart_x"]);
                      ssx >> x;
                      std::stringstream ssy(configOptions["playerstart_y"]);
                      ssy >> y;
                      std::stringstream ssz(configOptions["playerstart_z"]);
                      ssz >> z;

                      target_position_found = _world->findEmptyCFieldNear(target_position, x, y, z);
               }

               if (!target_position_found) {
                      throw LogoutException(NOPLACE);
               }

        }

        // set player on target field...
        pos.x=x;
        pos.y=y;
        pos.z=z;
        target_position->SetPlayerOnField(true);
        
        //look if there is a tile over the player set the maps over the player
        _world->tmap = NULL;
        Field *testf;
        for ( int i = 0; i < RANGEUP; ++i ) 
        {
            if ( _world->GetPToCFieldAt( testf, pos.x, pos.y, pos.z + 1 + i, _world->tmap ) ) 
            {
                // Ebene ber der neuen Position vorhanden
                if ( ( ( under[ i ]  ) // Karte wurde vorher angezeigt
                  && ( testf->getTileId() == TRANSPARENT  ) ) // das neue Feld ber dem Spieler ist TRANSPARENT
                  || ( ( !under[ i ]  )     // Karte wurde vorher nicht angezeigt
                  && ( testf->getTileId() != TRANSPARENT  ) ) ) // das neue Feld ber dem Spieler ist nicht TRANSPARENT
                {
                    // Verdeckung des Spielers hat sich ge�dert
                    roofmap[ i ] = _world->tmap;
                    under[ i ] = ( testf->getTileId() != TRANSPARENT );
                } 
                else if ( _world->tmap != roofmap[ i ] ) 
                {
                // Spieler steht unter einer anderen Karte, ohne da�sich die Verdeckung
                // ge�dert hat
                   roofmap[ i ] = _world->tmap;
                   under[ i ] = ( testf->getTileId() != TRANSPARENT );
                }
            } 
            else 
            {
                // Spieler war vorher verdeckt, jetzt nicht mehr
                roofmap[ i ] = NULL;
                under[ i ] = false;
            }
            
        }
        sendWeather( _world->weather );
        
        // send player login data
        boost::shared_ptr<BasicServerCommand> cmd(new IdTC( id ) );
        Connection->addCommand( cmd );
        // position
        cmd.reset( new SetCoordinateTC( pos ) );
        Connection->addCommand( cmd );

        effects->load();
        
        //send the basic data to the monitoring client
        cmd.reset( new BBPlayerTC( id, name, pos.x, pos.y,pos.z ) );
        _world->monitoringClientList->sendCommand( cmd );
        
        // send weather and time before sending the map, to display everything correctly from the start
        _world->sendIGTime( this );
        _world->sendWeather( this );

        // send std::map, items and chars around...
        sendFullMap();
        _world->sendAllVisibleCharactersToPlayer( this , true );

        // sent inventory
        for ( unsigned short int i = 0; i < MAX_BELT_SLOTS + MAX_BODY_ITEMS; ++i ) {
               sendCharacterItemAtPos( i );
        }

        sendAllSkills();
        // notify other players
        _world->sendCharacterMoveToAllVisiblePlayers( this, NORMALMOVE, 4 );
        // additional nop info
        _world->sendSpinToAllVisiblePlayers( this );
        cmd.reset( new PlayerSpinTC( faceto, id ) );
        Connection->addCommand( cmd );

        // update skills for client
        sendAttrib("hitpoints", increaseAttrib( "hitpoints", 0 ) );
        sendAttrib("mana", increaseAttrib( "mana", 0 ) );
        sendAttrib("foodlevel", increaseAttrib( "mana", 0 ) );
        //changes
        sendAttrib("sex", increaseAttrib("sex",0));
        sendAttrib("age", increaseAttrib("age",0));
        sendAttrib("weight", increaseAttrib("weight",0));
        sendAttrib("age", increaseAttrib("age",0));
        sendAttrib("body_height", increaseAttrib("body_height",0));
        sendAttrib("attitude", increaseAttrib("attitude",0));
        sendAttrib("luck", increaseAttrib("luck",0));
        sendAttrib("strength", increaseAttrib("strength",0));
        sendAttrib("dexterity", increaseAttrib("dexterity",0));
        sendAttrib("constitution", increaseAttrib("constitution",0));
        sendAttrib("intelligence", increaseAttrib("intelligence",0));
        sendAttrib("perception", increaseAttrib("perception",0));
        sendAttrib("age", increaseAttrib("age",0));
        sendAttrib("willpower", increaseAttrib("willpower",0));
        sendAttrib("essence", increaseAttrib("essence",0));
        sendAttrib("agility", increaseAttrib("agility",0));
        //end of changes
        
        
        // send magic skills
        sendMagicFlags(magic.type);
        
        _world->welcomePlayer( this );

        // send start message
        if ( getPlayerLanguage() == Language::german ) {
               _world->sendTextInFileToPlayer( configOptions["datadir"] + std::string( "startnachricht.txt" ), this);
        } else {
               _world->sendTextInFileToPlayer( configOptions["datadir"] + std::string( "startmessage.txt" ), this );
        }

        // Send admin message if admin
        if ( isAdmin() || hasGMRight(gmr_isnotshownasgm) ) {
               _world->sendTextInFileToPlayer( configOptions["datadir"] + std::string( "adminmessage.txt" ), this );
        }

        time_t logintime;
        time(&logintime);
        time(&lastkeepalive);
        if ( ( LoadWeight() * 100 ) / maxLoadWeight() >= 50 ) {
               setEncumberedSent( true );
               std::string tmessage;
               switch ( getPlayerLanguage() ) {
                      case Language::german:
                             tmessage = "Du bist �berladen.";
                             break;
                      case Language::english:
                             tmessage = "You are encumbered.";
                             break;
                      case Language::french:
                             tmessage = "You are encumbered.";
                             break;
                      default:
                             tmessage = "You are encumbered.";
               }
               sendMessage( tmessage );
        } else {
               setEncumberedSent( false );
        }
        time( &lastsavetime );
}

void Player::closeAllShowcasesOfMapContainers() 
{

        if ( mapshowcaseopen ) 
        {
               for ( MAXCOUNTTYPE i = 0; i < MAXSHOWCASES; ++i )
               {
                      if ( !(showcases[ i ].inInventory()) ) 
                      {
                             showcases[ i ].clear();
                             boost::shared_ptr<BasicServerCommand> cmd(new ClearShowCaseTC(i) );
                             Connection->addCommand( cmd );
                      }
               }
               mapshowcaseopen = false;
        }
}

void Player::closeAllShowcases() {

        for ( MAXCOUNTTYPE i = 0; i < MAXSHOWCASES; ++i ) 
        {
               showcases[ i ].clear();
               boost::shared_ptr<BasicServerCommand> cmd(new ClearShowCaseTC(i) );
               Connection->addCommand( cmd );
        }
        mapshowcaseopen = false;

}

Player::~Player() {
#ifdef Player_DEBUG
        std::cout << "Player Destruktor Start/Ende" << std::endl;
#endif
}

bool Player::VerifyPassword( std::string chkpw ) {
        return ( pw == chkpw );
}



void Player::sendCharacterItemAtPos( unsigned char cpos ) 
{
        if ( cpos < ( MAX_BELT_SLOTS + MAX_BODY_ITEMS ) ) 
        { // gltiger Wert
            boost::shared_ptr<BasicServerCommand>cmd( new UpdateInventoryPosTC( cpos, characterItems[cpos].id, characterItems[ cpos].number ) );
               Connection->addCommand( cmd );
        }
}


void Player::sendWeather( WeatherStruct weather )
{
    boost::shared_ptr<BasicServerCommand>cmd( new UpdateWeatherTC( weather ) );
    Connection->addCommand( cmd );
}


void Player::AgeInventory( ITEM_FUNCT funct ) 
{
    CommonStruct tempCommon;
    for ( unsigned char i = 0; i < MAX_BELT_SLOTS + MAX_BODY_ITEMS; ++i )
    {
        if ( characterItems[ i ].id != 0 ) 
        {
            if ( !CommonItems->find( characterItems[ i ].id, tempCommon ) ) 
            {
                tempCommon.rotsInInventory=false;
                tempCommon.ObjectAfterRot = characterItems[ i ].id;
            }
            if ( tempCommon.rotsInInventory )
            {
                if ( !funct( &characterItems[ i ] ) ) 
                {
                    if ( characterItems[ i ].id != tempCommon.ObjectAfterRot ) 
                    {
    #ifdef Character_DEBUG
                        std::cout << "INV:Ein Item wird umgewandelt von: " << characterItems[ i ].id << "  nach: " << tempCommon.ObjectAfterRot << "!\n";
    #endif
                        characterItems[ i ].id = tempCommon.ObjectAfterRot;
                        if ( CommonItems->find( tempCommon.ObjectAfterRot, tempCommon ) ) 
                        {
                            characterItems[ i ].wear = tempCommon.AgeingSpeed;
                        }
                        sendCharacterItemAtPos(i);
                    } 
                    else 
                    {
    #ifdef Character_DEBUG
                        std::cout << "INV:Ein Item wird gel�cht,ID:" << characterItems[ i ].id << "!\n";
    #endif
                        characterItems[ i ].id = 0;
                        characterItems[ i ].number = 0;
                        characterItems[ i ].wear = 0;
                        sendCharacterItemAtPos(i);
                    }

                    // The personal light might have changed!
                    updateAppearanceForAll( true );
                }
            }
        }
    }

    // Inhalt des Rucksacks altern
    if ( ( characterItems[ BACKPACK ].id != 0 ) && ( backPackContents != NULL ) ) 
    {
        backPackContents->doAge( funct, true);
        updateBackPackView();
    }
    
    std::map<uint32_t, Container*>::iterator depotIterator;
    for ( depotIterator = depotContents.begin(); depotIterator != depotContents.end(); depotIterator++ )
    {
        if ( depotIterator->second != NULL )
        {
            depotIterator->second->doAge( funct, true);
            for ( int i = 0; i < MAXSHOWCASES; ++i ) 
            {
                if ( showcases[ i ].contains( depotIterator->second ) ) 
                {
                    boost::shared_ptr<BasicServerCommand>cmd( new UpdateShowCaseTC( i, showcases[i].top()->items) );
                    Connection->addCommand( cmd );
                }
            }
        }
    }

}

void Player::learn( std::string skill, uint8_t skillGroup, uint32_t actionPoints, uint8_t opponent, uint8_t leadAttrib )
{
        
    uint16_t majorSkillValue = getSkill( skill );
    uint16_t minorSkillValue = getMinorSkill( skill );

    Character::learn( skill, skillGroup, actionPoints, opponent, leadAttrib );

    uint16_t newMajorSkillValue = getSkill( skill );
    uint16_t newMinorSkillValue = getMinorSkill( skill );
    if( newMinorSkillValue != minorSkillValue || newMajorSkillValue != majorSkillValue )
    {
        sendSkill( skill, skillGroup, newMajorSkillValue, newMinorSkillValue );
    }
}



int Player::createItem( TYPE_OF_ITEM_ID itemid, uint8_t count, uint16_t quali, uint32_t data ) {
        int temp = Character::createItem( itemid, count, quali, data );
        for ( unsigned char i = 0; i < MAX_BELT_SLOTS + MAX_BODY_ITEMS; ++i ) {
               if ( characterItems[ i ].id != 0 ) {
                      sendCharacterItemAtPos( i );
               }
        }

        updateBackPackView();

        return temp;
}


//int Player::createItem( TYPE_OF_ITEM_ID itemid, int count ) {
//    int tempo=5; //createItem( TYPE_OF_ITEM_ID itemid, int count, 333);
//    return tempo;
//}

int Player::_eraseItem( TYPE_OF_ITEM_ID itemid, int count, uint32_t data, bool useData ) {
        int temp = count;
#ifdef Player_DEBUG
        std::cout << "try to erase in player inventory " << count << " items of type " << itemid << "\n";
#endif
        if ( ( characterItems[ BACKPACK ].id != 0 ) && ( backPackContents != NULL ) ) {
               temp = backPackContents->_eraseItem( itemid, temp, data, useData );
               updateBackPackView();
#ifdef Player_DEBUG
               std::cout << "eraseItem: nach L�chen im Rucksack noch zu l�chen: " << temp << "\n";
#endif

        }

        if ( temp > 0 ) {
               // BACKPACK als Item erstmal auslassen
               for ( unsigned char i = MAX_BELT_SLOTS + MAX_BODY_ITEMS - 1; i > 0; --i ) {
                      if ( ( characterItems[ i ].id == itemid ) && ( !useData || characterItems[ i ].data == data ) && ( temp > 0 ) ) {
                             if ( temp >= characterItems[ i ].number ) {
                                    temp = temp - characterItems[ i ].number;
                                    characterItems[ i ].id = 0;
                                    characterItems[ i ].number = 0;
                                    characterItems[ i ].wear = 0;
                                    //std::cout<<"Try to find out if it was a two hander!"<<std::endl;
                                    if ( i == LEFT_TOOL || i == RIGHT_TOOL ) {
                                        unsigned char offhand = (i==LEFT_TOOL)?RIGHT_TOOL:LEFT_TOOL;
                                        if ( characterItems[ offhand ].id == 228 ) {
                                           // delete the occupied slot if the item was a two hander...
                                           //std::cout<<"Item was two hander try to delete occopied slot"<<std::endl;
                                           characterItems[ offhand ].id=0;
                                           characterItems[ offhand ].number=0;
                                           characterItems[ offhand ].wear=0;
                                           sendCharacterItemAtPos( offhand );
                                        }
                                    }
                             } else {
                                    characterItems[ i ].number = characterItems[ i ].number - temp;
                                    temp = 0;
                             }
                             sendCharacterItemAtPos( i );
                      }
               }
               if( World::get()->getItemStatsFromId( itemid ).Brightness > 0 ) updateAppearanceForAll( true );
        }
#ifdef Player_DEBUG
        std::cout << "eraseItem: am Ende noch zu l�chen: " << temp << "\n";
#endif
        return temp;
}


int Player::eraseItem( TYPE_OF_ITEM_ID itemid, int count )
{
    return _eraseItem( itemid, count, 0, false );
}


int Player::eraseItem( TYPE_OF_ITEM_ID itemid, int count, uint32_t data )
{
    return _eraseItem( itemid, count, data, true );
}


int Player::increaseAtPos( unsigned char pos, int count ) {
        int temp = count;

#ifdef Player_DEBUG
        std::cout << "increaseAtPos " << ( short int ) pos << " " << count << "\n";
#endif
        if ( ( pos > 0 ) && ( pos < MAX_BELT_SLOTS + MAX_BODY_ITEMS ) ) {
               if ( weightOK( characterItems[ pos ].id, count, NULL ) ) {

                      temp = characterItems[ pos ].number + count;
#ifdef Player_DEBUG
                      std::cout << "temp " << temp << "\n";
#endif

                      if ( temp > MAXITEMS ) {
                             characterItems[ pos ].number = MAXITEMS;
                             temp = temp - MAXITEMS;
                      } else if ( temp <= 0 ) {
                             bool updateBrightness = World::get()->getItemStatsFromId( characterItems[ pos ].id ).Brightness > 0;
                             temp = count + characterItems[ pos ].number;
                             characterItems[ pos ].number = 0;
                             characterItems[ pos ].id = 0;
                             //L�chen des Occopied Slots
                             if ( pos == RIGHT_TOOL && characterItems[LEFT_TOOL].id == BLOCKEDITEM) {
                                    characterItems[LEFT_TOOL].id = 0;
                                    characterItems[LEFT_TOOL].number = 0;
                                    characterItems[LEFT_TOOL].wear = 0;
                                    sendCharacterItemAtPos(LEFT_TOOL);
                             }
                             if( updateBrightness ) updateAppearanceForAll( true );
                      } else {
                             characterItems[ pos ].number = temp;
                             temp = 0;
                      }
               }
        }
        sendCharacterItemAtPos( pos );
        return temp;
}

int Player::createAtPos(unsigned char pos, TYPE_OF_ITEM_ID newid, int count) {
        int temp = Character::createAtPos(pos,newid,count);
        sendCharacterItemAtPos( pos );
        return temp;

}

bool Player::swapAtPos( unsigned char pos, TYPE_OF_ITEM_ID newid , uint16_t newQuality) {
        if ( Character::swapAtPos( pos, newid, newQuality ) )
    {
               sendCharacterItemAtPos( pos );
               return true;
        }
        return false;
}


void Player::updateBackPackView() {
        if ( backPackContents != NULL ) {
               for ( int i = 0; i < MAXSHOWCASES; ++i ) {
                      if ( showcases[ i ].contains( backPackContents ) ) 
                      {
                          boost::shared_ptr<BasicServerCommand>cmd( new UpdateShowCaseTC( i, showcases[i].top()->items) );
                          Connection->addCommand( cmd );                          
                      }
               }
        }
}


void Player::sendSkill( std::string name, unsigned char type, unsigned short int major, unsigned short int minor ) 
{
        boost::shared_ptr<BasicServerCommand>cmd( new UpdateSkillTC( name, type, major, minor ) );
        Connection->addCommand( cmd );
        cmd.reset( new BBSendSkillTC( id, type, name, major, minor) );
        _world->monitoringClientList->sendCommand( cmd );
}


void Player::sendAllSkills() 
{
    for ( SKILLMAP::const_iterator ptr = skills.begin(); ptr != skills.end(); ++ptr ) 
    {
        if (ptr->second.major>0)
        sendSkill( ptr->first, ptr->second.type, ptr->second.major, ptr->second.minor );
    }
}


void Player::sendMagicFlags(int type) 
{
    if ((type >= 0) && (type < 4)) 
    {
        boost::shared_ptr<BasicServerCommand>cmd( new UpdateMagicFlagsTC( type, magic.flags[ type ] ));
        Connection->addCommand( cmd );       
    }
}


void Player::sendAttrib( std::string name, unsigned short int value ) 
{
    boost::shared_ptr<BasicServerCommand> cmd( new UpdateAttribTC( name, value ) );
    Connection->addCommand( cmd );
    cmd.reset( new BBSendAttribTC( id, name, value) );
    _world->monitoringClientList->sendCommand( cmd );
}


unsigned short int Player::increaseAttrib( std::string name, short int amount ) 
{
    unsigned short int temp = Character::increaseAttrib( name, amount );
    if (amount != 0) 
    {
        sendAttrib( name, temp );
    }
    return temp;
}

void Player::tempChangeAttrib( std::string name, short int amount, uint16_t time)
{
    Character::tempChangeAttrib( name, amount, time );
    if ( amount != 0 )
    {
        sendAttrib( name, Character::increaseAttrib(name,0) );
    }
}


void Player::startMusic( short int title ) 
{
    boost::shared_ptr<BasicServerCommand>cmd( new MusicTC( title ) );
    Connection->addCommand( cmd );
}


void Player::defaultMusic()
{
    boost::shared_ptr<BasicServerCommand>cmd( new MusicDefaultTC() );
    Connection->addCommand( cmd );
}


// Setters and Getters //

unsigned char Player::GetStatus() {
        return status;
}


void Player::SetStatus( unsigned char thisStatus ) {
        status = thisStatus;
}


// What time does the status get reset?
time_t Player::GetStatusTime( ) {
        return statustime;
}


void Player::SetStatusTime( time_t thisStatustime ) {
        statustime = thisStatustime;
}


// Who banned/jailed the player?
std::string Player::GetStatusGM() {
        ConnectionManager::TransactionHolder transaction = dbmgr->getTransaction();
        std::stringstream query;
        query << "SELECT chr_name FROM chars WHERE chr_playerid = " << transaction.quote(statusgm);

        std::string statusgmstring;

        di::select<di::Varchar>(transaction, statusgmstring, query.str());

        return statusgmstring;
}


void Player::SetStatusGM( TYPE_OF_CHARACTER_ID thisStatusgm ) {
        statusgm = thisStatusgm;
}


// Why where they banned/jailed?
std::string Player::GetStatusReason() {
        return statusreason;
}


void Player::SetStatusReason( std::string thisStatusreason ) {
        statusreason = thisStatusreason;
}

// World Map Turtle Graphics
void Player::setTurtleActive( bool tturtleActive ) {
        turtleActive = tturtleActive;
        setClippingActive( !tturtleActive );
}


void Player::setClippingActive( bool tclippingActive ) {
        clippingActive = tclippingActive;
}


bool Player::getTurtleActive( ) {
        return turtleActive;
}


bool Player::getClippingActive( ) {
        return clippingActive;
}


void Player::setTurtleTile( unsigned char tturtletile ) {
        turtletile = tturtletile;
}


unsigned char Player::getTurtleTile( ) {
        return turtletile;
}


void Player::setAdmin( uint32_t tAdmin ) {
        admin = tAdmin;
}


bool Player::isAdmin() {
        return (admin>0 && !hasGMRight(gmr_isnotshownasgm) );
}


void Player::setEncumberedSent( bool tEncumberedSent ) {
        encumberedSent = tEncumberedSent;
}


bool Player::wasEncumberedSent( ) {
        return encumberedSent;
}


void Player::setUnconsciousSent( bool tUnconsciousSent ) {

        unconsciousSent = tUnconsciousSent;
}


bool Player::wasUnconsciousSent( ) {
        return unconsciousSent;
}

void Player::check_logindata() throw (LogoutException) {

        try {

               ConnectionManager::TransactionHolder transaction = dbmgr->getTransaction();
               ConnectionManager::TransactionHolder account_transaction = accdbmgr->getTransaction();

               std::string real_pwd;

               TYPE_OF_CHARACTER_ID account_id;

               std::stringstream query;
               query << "SELECT chr_playerid, chr_accid, chr_status, chr_statustime, chr_onlinetime,";
               query << "chr_lastsavetime, chr_sex, chr_race, chr_prefix, chr_suffix FROM chars ";
               query << "where chr_name = " << transaction.quote(name);

               di::isnull<time_t> isnull_statustime(statustime);
               di::isnull<std::string> n_prefix(prefix), n_suffix(suffix);

               int rows = di::select<
                                di::Integer, di::Integer, di::Integer, di::Integer, di::Integer,
                                di::Integer, di::Integer, di::Integer, di::Varchar, di::Varchar
                                >(transaction, id, account_id, status, isnull_statustime, onlinetime,
                                     lastsavetime, battrib.truesex, race, n_prefix, n_suffix,
                                     query.str());

               // no char found?
               if (rows != 1)
                      throw LogoutException(NOCHARACTERFOUND);

               // first we check the status since we already retrieved it
               switch (status) {
                      case BANNED:
                             // player banned forever
                             throw LogoutException(BYGAMEMASTER);

                      case BANNEDFORTIME:
                             // player banned for some time...
                             // check if time is up?
                             time_t time_now;
                             time(&time_now);
                             if (time_now > statustime) {
                                    status=statustime=0;
                             } else
                                    throw LogoutException(BYGAMEMASTER);
                             break;

                      case WAITINGVALIDATION:
                             // player not approved yet
                             throw LogoutException(NOACCOUNT);
               }

               // next we get the infos for the account and check if the account is active
               int acc_state;

           Language::LanguageType mother_tongue;

               rows = di::select<
                         di::Varchar, di::Integer, di::Integer
                         >(account_transaction, real_pwd, mother_tongue, acc_state,
                              "SELECT acc_passwd, acc_lang, acc_state FROM account where acc_id =" + toString(account_id));

               if (rows != 1)
                      throw LogoutException(NOACCOUNT);

        setPlayerLanguage(mother_tongue);

               // check if account is active
               if (acc_state < 3) // TODO how is acc_state defined??
                      throw LogoutException(NOACCOUNT);
               if (acc_state > 3)
                      throw LogoutException(BYGAMEMASTER);

               // check password
               if (std::string(crypt(pw.c_str(),"$1$illarion1")) != real_pwd) {
                      std::cerr << "*** ";
                      time_t acttime = time(NULL);
                      std::string attempttime = ctime(&acttime);
                      attempttime[attempttime.size()-1] = ':';
                      std::cerr << attempttime;
                      std::cerr << " Player '" << name << "' sent PWD '" << pw
                      << "' but needs  '" << real_pwd << "' ip: "
                      << Connection->getIPAdress() << std::endl;

                      throw LogoutException(WRONGPWD);
               }

               character = Character::player;

               // get the remaining attributes from the player table
               rows = di::select<
                         di::Integer, di::Integer, di::Integer, di::Integer,
                         di::Integer, di::Integer, di::Integer, di::Integer, di::Integer, di::Integer,
                         di::Integer, di::Integer, di::Integer, di::Integer, di::Integer, di::Integer,
                         di::Integer, di::Integer, di::Integer, di::Integer, di::Integer, di::Integer,
                         di::Integer, di::BigInt, di::BigInt, di::BigInt, di::BigInt,
                         di::Integer, di::Integer, di::Integer, di::Integer, di::Integer,
                         di::Integer, di::Integer, di::Integer, di::Integer, di::Integer
                         >(transaction, pos.x, pos.y, pos.z, faceto,
                              battrib.trueage, battrib.trueweight, battrib.truebody_height, battrib.truehitpoints, battrib.truemana, battrib.trueattitude,
                              battrib.trueluck, battrib.truestrength, battrib.truedexterity, battrib.trueconstitution, battrib.trueagility, battrib.trueintelligence,
                              battrib.trueperception, battrib.truewillpower, battrib.trueessence, battrib.truefoodlevel, appearance, lifestate,
                              magic.type,  magic.flags[ MAGE ], magic.flags[ PRIEST ], magic.flags[ BARD ], magic.flags[ DRUID ],
                              poisonvalue, mental_capacity, hair, beard, hairred, hairgreen, hairblue, skinred, skingreen, skinblue,
                              "SELECT ply_posx, ply_posy, ply_posz, ply_faceto,"
                              "ply_age, ply_weight, ply_body_height, ply_hitpoints, ply_mana, ply_attitude,"
                              "ply_luck, ply_strength, ply_dexterity, ply_constitution, ply_agility, ply_intelligence,"
                              "ply_perception, ply_willpower, ply_essence, ply_foodlevel, ply_appearance, ply_lifestate,"
                              "ply_magictype, ply_magicflagsmage, ply_magicflagspriest, ply_magicflagsbard, ply_magicflagsdruid,"
                              "ply_poison, ply_mental_capacity, ply_hair, ply_beard, ply_hairred, ply_hairgreen,"
                              "ply_hairblue, ply_skinred, ply_skingreen, ply_skinblue "
                              "from player where ply_playerid = " + toString(id));

               if (rows != 1)
                      throw LogoutException(NOACCOUNT);
               
               //now set the temp values from the attributes to the true values
               
               battrib.sex = battrib.truesex;
               battrib.time_sex = 0; 
               
               battrib.age = battrib.trueage;
               battrib.time_age = 0;
               
               battrib.weight = battrib.trueweight;
               battrib.time_weight = 0;
               
               battrib.body_height = battrib.truebody_height;
               battrib.time_body_height = 0;
               
               battrib.hitpoints = battrib.truehitpoints;
               battrib.time_hitpoints = 0;
               
               battrib.mana = battrib.truemana;
               battrib.time_mana = 0;
               
               battrib.attitude = battrib.trueattitude;
               battrib.time_attitude = 0;
               
               battrib.luck = battrib.trueluck;
               battrib.time_luck = 0;
               
               battrib.strength = battrib.truestrength;
               battrib.time_strength = 0;
               
               battrib.dexterity = battrib.truedexterity;
               battrib.time_dexterity = 0;
               
               battrib.constitution = battrib.trueconstitution;
               battrib.time_constitution = 0;
               
               battrib.agility = battrib.trueagility;
               battrib.time_agility = 0;
               
               battrib.intelligence = battrib.trueintelligence;
               battrib.time_intelligence = 0;
               
               battrib.perception = battrib.trueperception;
               battrib.time_perception = 0;
               
               battrib.willpower = battrib.truewillpower;
               battrib.time_willpower = 0;
               
               battrib.essence = battrib.trueessence;
               battrib.time_essence = 0;
               
               battrib.foodlevel = battrib.truefoodlevel;
               battrib.time_foodlevel = 0;
               

        } catch (std::exception &e) {
               std::cerr << "exception on load player: " << e.what() << std::endl;
               throw LogoutException(NOCHARACTERFOUND);
        }
}

struct container_struct {
        Container& container;
        unsigned int id;
        unsigned int depotid;

        container_struct(Container& cc, unsigned int aboveid, unsigned int depot = 0)
                      : container(cc), id(aboveid), depotid(depot) { }}
;

bool Player::save() throw () 
{
        ConnectionManager::TransactionHolder transaction = dbmgr->getTransaction();

        try {
               time(&lastsavetime);

               {
                      // first we save the last login, ip, ban status
                      std::stringstream query;
                      query << "UPDATE chars SET chr_status = " << transaction.quote((int)status);
                      if (status != 0) {
                             query << ", chr_statustime = " << transaction.quote(statustime);
                             query << ", chr_statusgm = " << transaction.quote(statusgm);
                             query << ", chr_statusreason = " << transaction.quote(statusreason);
                      } else {
                             query << ", chr_statustime = NULL, chr_statusgm = NULL, chr_statusreason = NULL";
                      }
                      query << ", chr_lastip = " << transaction.quote(last_ip);
                      query << ", chr_onlinetime = " << transaction.quote(onlinetime);
                      query << ", chr_prefix = " << transaction.quote(prefix);
                      query << ", chr_suffix = " << transaction.quote(suffix);
                      query << ", chr_lastsavetime = " << transaction.quote(lastsavetime);
                      query << " WHERE chr_playerid = " << transaction.quote(id);

                      di::exec(transaction, query.str());
               }

               {
                      // next we store attributes
                      std::stringstream query;
                      query << "UPDATE player SET ply_posx = " << transaction.quote(pos.x);
                      query << ", ply_posy = " << transaction.quote(pos.y);
                      query << ", ply_posz = " << transaction.quote(pos.z);
                      query << ", ply_faceto = " << transaction.quote((int)faceto);
                      query << ", ply_hitpoints = " << transaction.quote(battrib.hitpoints);
                      query << ", ply_mana = " << transaction.quote(battrib.mana);
                      query << ", ply_foodlevel = " << transaction.quote(battrib.foodlevel);
                      query << ", ply_appearance = " << transaction.quote(appearance);
                      query << ", ply_lifestate = " << transaction.quote(lifestate);
                      query << ", ply_magictype = " << transaction.quote((int)magic.type);
                      query << ", ply_magicflagsmage = " << transaction.quote(magic.flags[ MAGE ]);
                      query << ", ply_magicflagspriest = " << transaction.quote(magic.flags[ PRIEST ]);
                      query << ", ply_magicflagsbard = " << transaction.quote(magic.flags[ BARD ]);
                      query << ", ply_magicflagsdruid = " << transaction.quote(magic.flags[ DRUID ]);
                      query << ", ply_poison = " << transaction.quote(poisonvalue);
                      query << ", ply_mental_capacity = " << transaction.quote(mental_capacity);
                      query << " WHERE ply_playerid = " << transaction.quote(id);

                      di::exec(transaction, query.str());
               }

               {
                      // get rid of the old skills
                      std::stringstream query;
                      query << "DELETE FROM playerskills WHERE psk_playerid = " << transaction.quote(id);
                      di::exec(transaction, query.str());

                      // now store the skills
                      for ( SKILLMAP::iterator skillptr = skills.begin(); skillptr != skills.end(); ++skillptr )
                             di::insert(transaction, id, skillptr->first, (uint16_t)skillptr->second.type, (uint16_t)skillptr->second.major, (uint16_t)skillptr->second.firsttry, (uint16_t)skillptr->second.minor, "INSERT INTO playerskills (psk_playerid, psk_name, psk_type, psk_value, psk_firsttry,  psk_minor)");
               }

               {
                      // last but not least: the inventory

                      // get rid of the old inventory
                      std::stringstream query;
                      query << "DELETE FROM playeritems WHERE pit_playerid = " << transaction.quote(id);
                      di::exec(transaction, query.str());
                }
                 
                {    
                      std::stringstream query;
                      query << "DELETE FROM playeritem_datavalues WHERE idv_playerid = " << transaction.quote(id);
                      di::exec(transaction, query.str());
                }
                
                {
                      std::list<container_struct> containers;

                      // add backpack to containerlist
                      if (characterItems[ BACKPACK ].id != 0 && backPackContents != NULL)
                             containers.push_back(container_struct(*backPackContents, BACKPACK+1));

                      // add depot to containerlist
                      std::map<uint32_t, Container*>::iterator it;

                      for ( it = depotContents.begin(); it != depotContents.end(); ++it )
                      {
                         containers.push_back(container_struct(*it->second, 0, it->first));
                      }
/*                    if (depotContents != NULL)
                             containers.push_back(container_struct(*depotContents, 0, 1));
*/
                      int linenumber = 0;

                      // save all items directly on the body...
                      for (int thisItemSlot = 0; thisItemSlot < MAX_BODY_ITEMS + MAX_BELT_SLOTS; ++thisItemSlot )
                      {
                                         //if there is no item on this place, set all other values to 0
                            if ( characterItems[ thisItemSlot ].id == 0 )
                            {
                                characterItems[ thisItemSlot ].wear = 0;
                                characterItems[ thisItemSlot ].number = 0;
                                characterItems[ thisItemSlot ].quality = 0;
                                characterItems[ thisItemSlot ].data = 0;
                                characterItems[ thisItemSlot ].data_map.clear();
                            }
                            di::insert(transaction, (int32_t)id, (int32_t)(++linenumber), 0, 0, (int32_t)characterItems[ thisItemSlot ].id,
                                                          (int16_t)characterItems[ thisItemSlot ].wear, (int16_t)characterItems[ thisItemSlot ].number, (int16_t)characterItems[ thisItemSlot ].quality, (int32_t)characterItems[ thisItemSlot ].data,
                                                          "INSERT INTO playeritems (pit_playerid, pit_linenumber, pit_in_container, pit_depot, pit_itemid, pit_wear, pit_number, pit_quality, pit_data)");
                            //Insert Datavalues for the maps
                            if( characterItems[ thisItemSlot ].id == 0 || characterItems[ thisItemSlot ].data_map.empty() ) 
                            {
                                //Leeren Datamapeintrag speichern
                                di::insert(transaction, static_cast<int32_t>(id), static_cast<int32_t>(linenumber), static_cast<int16_t>(0), 0,
                                    "INSERT INTO playeritem_datavalues (idv_playerid, idv_linenumber, idv_key, idv_value)");
                            }
                            else
                            {
                                for (Item::DATA_MAP::iterator it = characterItems[ thisItemSlot ].data_map.begin(); it != characterItems[ thisItemSlot ].data_map.end(); ++it)
                                {
                                    di::insert(transaction, static_cast<int32_t>(id), static_cast<int32_t>(linenumber), static_cast<int16_t>(it->first), it->second,
                                    "INSERT INTO playeritem_datavalues (idv_playerid, idv_linenumber, idv_key, idv_value)");
                                }
                            }
                      }
                      // add backpack contents...
                      while (!containers.empty()) 
                      {
                          // get container to save...
                          const container_struct actcont = containers.front();
                          containers.pop_front();

                          for (ITEMVECTOR::iterator item = actcont.container.items.begin(); item != actcont.container.items.end(); ++item) 
                          {
                                di::insert(transaction, (int32_t)id, (int32_t)(++linenumber), (int16_t)actcont.id, (int32_t)actcont.depotid,
                                          (int32_t)item->id, (int16_t)item->wear, (int16_t)item->number, (int16_t)item->quality, (int32_t)item->data,
                                          "INSERT INTO playeritems (pit_playerid, pit_linenumber, pit_in_container, pit_depot, pit_itemid, pit_wear, pit_number, pit_quality, pit_data)");
                                std::cerr<<"saving datamap2"<<std::endl;
                                for (Item::DATA_MAP::iterator it = item->data_map.begin(); it != item->data_map.end(); ++it)
                                {
                                    di::insert(transaction,static_cast<int32_t>(id), static_cast<int32_t>(linenumber), static_cast<int16_t>(it->first), it->second,
                                    "INSERT INTO playeritem_datavalues (idv_playerid, idv_linenumber, idv_key, idv_value)");
                                }
                                std::cerr<<"datamap saving 2 done"<<std::endl;
                                
                                // if it is a container, add it to the list of containers to save...
                                if ( ContainerItems->find( item->id ) ) 
                                {
                                        Container::ONTAINERMAP::iterator iterat = actcont.container.containers.find( item->number );
                                        if ( iterat != actcont.container.containers.end() )containers.push_back(container_struct(*(*iterat).second, linenumber));
                                }
                           }
                      }

               }

               transaction.commit();
               
               if ( !effects->save() )
               {
                   std::cerr<<"error while saving lteffects for player"<<name<<std::endl;
               }

               return true;
        } 
        catch (std::exception e) 
        {
               std::cerr << "Playersave caught exception: " << e.what() << std::endl;
               transaction.rollback();
               return false;
        }
}

bool Player::loadGMFlags() throw()
{
    try
    {
        //Laden einer Transaktion
        ConnectionManager::TransactionHolder transaction = dbmgr->getTransaction();    
        std::stringstream Query;
        Query << "SELECT gm_rights_server, gm_charid FROM gms, player WHERE ply_playerid = gm_charid AND ply_playerid = " << transaction.quote( id );
        uint32_t gmrights;
        uint32_t id;
        size_t rows = di::select<di::Integer,di::Integer>(transaction,gmrights,id,Query.str() );
        if ( rows > 1 )
        {
                std::cerr<<" Too much GM right enterys for char with id: "<<id<<" !"<<std::endl;
                return false;
        }
        else if ( rows == 1)setAdmin( gmrights );
        else setAdmin(0); //No GM state
        return true;
    }
    catch ( std::exception &e )
    {
        std::cerr<<"exception: "<<e.what()<<" while loading GM Flags!"<<std::endl;
        return false;
    }
    return false;
}

bool Player::load() throw() {
        // maps containing all depots/containers involved
        std::map<int, Container*> depots, containers;
        std::map<int, Container*>::iterator it;

        bool dataOK=true; //Is the data loaded correct?

        try 
        {
            // load the skills first...
            ConnectionManager::TransactionHolder transaction = dbmgr->getTransaction();
            std::stringstream Query;

            Query << "SELECT psk_name, psk_type, psk_value, psk_minor, psk_firsttry FROM playerskills where psk_playerid = " << transaction.quote( id );
            std::vector<std::string> skillname;
            std::vector<uint8_t> skilltype;
            std::vector<uint16_t> skillvalue;
            std::vector<uint16_t> skillminor;
            std::vector<uint16_t> skillft;
            size_t rows = di::select_all<di::Varchar, di::Integer, di::Integer,di::Integer,di::Integer>
            (transaction, skillname, skilltype, skillvalue, skillminor, skillft,  Query.str() );
            if ( rows > 0 ) 
            {
                for ( unsigned int tuple = 0; tuple < rows; ++tuple)
                {
                    setSkill( skilltype[tuple] , skillname[tuple] , skillvalue[tuple], skillminor[tuple], skillft[tuple]  );
                }
            } 
            else 
            {
                std::cout<<"No Skills to load for "<<name<<"("<<id<<")"<<std::endl;
            }
               
            std::vector<uint16_t> ditemlinenumber;
            std::vector<int16_t> key;
            std::vector<std::string> value;
            //Query leeren
            Query.str("");
            Query.clear();
            Query << "SELECT idv_linenumber,  idv_key, idv_value FROM playeritem_datavalues where idv_playerid = "<< transaction.quote( id ) << " ORDER BY idv_linenumber";
            size_t datamaplines = di::select_all<di::Integer, di::Integer, di::Varchar>(transaction, ditemlinenumber, key, value, Query.str() ); 
                            
            // load inventory
            std::vector<uint16_t> itemlinenumber;
            std::vector<uint16_t> itemincontainer;
            std::vector<uint32_t> itemdepot;
            std::vector<TYPE_OF_ITEM_ID> itemid;
            std::vector<uint8_t> itemwear;
            std::vector<uint8_t> itemnumber;
            std::vector<uint16_t> itemquality;
            std::vector<uint32_t> itemdata;
            //Query leeren
            Query.str("");
            Query.clear();
            //neue Abfrage einfgen
            Query << "SELECT pit_linenumber,  pit_in_container, pit_depot, pit_itemid, pit_wear, pit_number, pit_quality, pit_data FROM playeritems where pit_playerid = "
            << transaction.quote( id ) << " ORDER BY pit_linenumber";
            rows = di::select_all<di::Integer, di::Integer, di::Integer, di::Integer, di::Integer, di::Integer, di::Integer, di::Integer>
                   (transaction, itemlinenumber, itemincontainer, itemdepot, itemid, itemwear, itemnumber, itemquality, itemdata, Query.str() );
               
            //Initialize the depot map         
            // create a new (empty) depot
            Query.str("");
             Query.clear();
            //Neue Abfrage zum laden der Depotids die belegt sind
            Query << "SELECT DISTINCT pit_depot FROM playeritems WHERE pit_playerid = " << transaction.quote( id );
            //Laden der Depotids die belegt sind
            std::vector<uint32_t> depotid;
            size_t zeilen = di::select_all<di::Integer>(transaction,depotid,Query.str() );
            //Laden des Depotvolumens
            ContainerStruct depotstruct;
            uint16_t depotsize = 0;
            if (ContainerItems->find(321,depotstruct))depotsize = depotstruct.ContainerVolume;
            //Ende Laden des Depotvolumens
        
            for ( size_t i = 1; i <= zeilen; ++i )
            {              
                if ( depotid[ i - 1 ] != 0 )
                {
                    depotContents[ depotid[ i - 1 ] ] = new Container(depotsize);
                    depots[ depotid [ i - 1 ] ] = depotContents[ depotid[ i - 1] ];
                }         
            }
// End of initializing the depot map

            unsigned int tempdepot, tempincont, linenumber;
            Container* tempc;
            Item tempi;
            unsigned int curdatalinenumber = 0;
            for ( unsigned int tuple = 0; tuple < rows; ++tuple ) 
            {
                tempincont = itemincontainer[tuple];
                tempdepot = itemdepot[tuple];
                linenumber = itemlinenumber[tuple];

                tempi.id = itemid[tuple];
                tempi.wear = itemwear[tuple];;
                tempi.number = itemnumber[tuple];
                tempi.quality = itemquality[tuple];
                tempi.data = itemdata[tuple];
                
                if ( ditemlinenumber[curdatalinenumber] != linenumber )
                {
                    std::cerr << "*** player '" << name << "' has invalid itemvalues!" << std::endl;
                    Logger::writeError("itemload","*** player '" + name + "' has invalid itemvalues!");
                    throw std::exception();
                }
                while ( curdatalinenumber < datamaplines && ditemlinenumber[curdatalinenumber] == linenumber)
                {
                    if ( key[curdatalinenumber] > 255 || key[curdatalinenumber] < 0 )
                    {
                        std::cerr << "*** player '" << name << "' has invalid itemvalues, key was larger than 255 or smaller than 0!" << std::endl;
                        Logger::writeError("itemload","*** player '" + name + "' has invalid itemvalues, key was larger than 255 or smaller than 0!");
                    }
                    else
                    {
                        uint8_t nkey = static_cast<uint8_t>(key[curdatalinenumber]);
                        tempi.setValue(nkey, value[curdatalinenumber]);
                    }
                    curdatalinenumber++;
                }

                // item is in a depot?
                if (tempdepot && (it = depots.find(tempdepot)) == depots.end() ) 
                {
                    // serious error occured! player data corrupted!
                    std::cerr << "*** player '" << name << "' has invalid depot contents!" << std::endl;
                    Logger::writeError("itemload","*** player '" + name + "' has invalid depot contents!");
                    throw std::exception();
                }
                // item is in a container?
                if (dataOK && tempincont && (it = containers.find(tempincont)) == containers.end() ) 
                {
                    // serious error occured! player data corrupted!
                    std::cerr << "*** player '" << name << "' has invalid container contents!" << std::endl;
                    Logger::writeError("itemload","*** player '" + name + "' has invalid depot contents 2!");
                    throw std::exception();
                }

                if ( ( dataOK && ( (!tempincont && ! tempdepot) && linenumber > MAX_BODY_ITEMS + MAX_BELT_SLOTS ) ) || (tempincont && tempdepot) ) 
                {
                    // serious error occured! player data corrupted!
                    std::cerr << "*** player '" << name << "' has invalid items!" << std::endl;
                    Logger::writeError("itemload","*** player '" + name + "' has invalid items!");
                    throw std::exception();
                }
                ContainerStruct cont;
                if ( ContainerItems->find( tempi.id, cont) ) 
                {
                    tempc = new Container(cont.ContainerVolume);
                    if (linenumber > MAX_BODY_ITEMS + MAX_BELT_SLOTS )
                    {
                        if (!it->second->InsertContainer( tempi, tempc ))
                        {
                            Logger::writeError("itemload","*** player '" + name + "' insert Container wasn't sucessful!");
                        }
                        else
                        {
                            
                        }
                    }
                    else
                    {
                        characterItems[ linenumber - 1 ] = tempi;
                    }
                    containers[linenumber] = tempc;
                } 
                else 
                {
                    if (linenumber >= MAX_BODY_ITEMS + MAX_BELT_SLOTS + 1)it->second->InsertItemOnLoad( tempi );
                    else characterItems[ linenumber - 1 ] = tempi;
                }
            }
            if ((it=containers.find(BACKPACK + 1)) != containers.end() ) backPackContents = it->second;
            else backPackContents = NULL;
            std::map<uint32_t,Container*>::iterator it2;
            for ( it = depots.begin(); it != depots.end(); ++it)
            {
                if ( (it2=depotContents.find( it->first )) != depotContents.end() )it2->second = it->second;
                else it2->second = NULL;
            }              
        }  
        catch (std::exception &e)
        {
            std::cerr << "exception: " << e.what() << std::endl;
            dataOK = false;
        }
        if (!dataOK) 
        {
            std::cerr<<"in load(): error while loading!"<<std::endl;
            //Fehler beim laden aufgetreten
            std::map<int, Container*>::reverse_iterator rit;
            std::map<uint32_t,Container*>::reverse_iterator rit2;
            // clean up...
            for (rit=containers.rbegin(); rit != containers.rend(); ++rit) delete rit->second;
            for (rit=depots.rbegin(); rit != depots.rend(); ++rit) delete rit->second;
            for ( rit2 = depotContents.rbegin(); rit2 != depotContents.rend(); ++rit2) delete rit2->second;
            backPackContents=NULL;
        }
        //#endif
    return dataOK;
}

void Player::increasePoisonValue( short int value ) {
        std::string texteng="", textger="", tmessage; //string fr die unterschiedlichen Texte
        if ( (poisonvalue == 0) && value > 0) {
               switch ( getPlayerLanguage()) {
                      case Language::german:
                             tmessage = "Du bist vergiftet.";
                             break;
                      case Language::english:
                             tmessage = "You are poisoned.";
                             break;
                      case Language::french:
                             tmessage = "You are poisoned.";
                             break;
                      default:
                             tmessage = "You are poisoned.";
               }
               sendMessage(tmessage);
        }
        if ( (poisonvalue + value) >= MAXPOISONVALUE ) {
               poisonvalue = MAXPOISONVALUE;
        } else if ( (poisonvalue + value) <= 0 ) {
               poisonvalue = 0;
               switch ( getPlayerLanguage()) {
                      case Language::german:
                             tmessage = "Du fuehlst dich auf einmal besser.";
                             break;
                      case Language::english:
                             tmessage = "You feel better now.";
                             break;
                      case Language::french:
                             tmessage = "You feel better now.";
                             break;
                      default:
                             tmessage = "You feel better now.";
               }
               sendMessage(tmessage);
        } else {
               poisonvalue += value;
        }


        //============================================================================
}

void Player::SetAlive( bool t ) {
        if ( t ) 
        {
             lifestate = lifestate | 1;
        } 
        else if ( lifestate & 1 ) 
        {
             lifestate = lifestate & ( 0xFFFF - 1 );
             if ( death_consequences)
             {
                 PlayerDeath(); //Einzige �derung zum Orginal um Inventar zu droppen
                 ReduceSkills();
             }
        }
}

unsigned short int Player::setSkill( unsigned char typ, std::string sname, short int major, short int minor, uint16_t firsttry )
{
    Character::setSkill(typ,sname,major,minor,firsttry);
    sendSkill(sname,typ,major,minor);
    return major;
}

unsigned short int Player::increaseSkill( unsigned char typ, std::string name, short int amount ) {
        Character::increaseSkill(typ,name,amount);
        int major = getSkill(name);
        int minor = getSkill(name);
        sendSkill(name,typ,major,minor);
        return major;
}

void Player::PlayerDeath() {
        //Droppen aller Backpackitems und belt Items nach dem Tod
        bool isnewbie=true;
        SKILLMAP::iterator ptr;
        for ( ptr = skills.begin(); ptr!=skills.end(); ++ptr)//Schleife durch alle Skills um zu prfen ob es ein Newbie ist.
        {
               if( (ptr->second.major>15) && (ptr->second.type!=1) ) //Prfen ob ein Skill > 15 der nicht Sprachskill ist.
               {
                      isnewbie=false;
                      break;
               }
        }
        if( (!admin) && (!isnewbie) ) {
               if ( characterItems[ BACKPACK ].id != 0 )_world->dropItemFromPlayerOnMap(this, BACKPACK, pos.x, pos.y, pos.z, characterItems[ BACKPACK ].number ); //Droppen vom Backpack
               for ( unsigned char i = MAX_BODY_ITEMS; i <= MAX_BODY_ITEMS + MAX_BELT_SLOTS; ++i) {
                      if ( characterItems[ i ].id != 0 )_world->dropItemFromPlayerOnMap(this, i, pos.x, pos.y, pos.z, characterItems[ i ].number ); //Droppen des Grtels
               }
               for ( char c = 0; c < 2; ++c) //Droppen von zwei zufaelligen Char items
               {   
                   unsigned char pos = rnd(1,MAX_BODY_ITEMS);
                   if ( characterItems[ pos ].id != 0 )_world->dropItemFromPlayerOnMap(this, pos, this->pos.x, this->pos.y, this->pos.z, characterItems[ pos ].number );
               }
        } else {
               std::cout<<"Itemdrop by death char: "<<name<<"("<<id<<") was GM or Newbie \n";
        }
}

void Player::receiveText(talk_type tt, std::string message, Character* cc) 
{
    boost::shared_ptr<BasicServerCommand>cmd(new SayTC( cc->pos.x, cc->pos.y, cc->pos.z, message )); 
    switch (tt) 
    {
        case tt_say:
            Connection->addCommand( cmd );
            break;
        case tt_whisper:
            cmd.reset( new WhisperTC( cc->pos.x, cc->pos.y, cc->pos.z, message ) );
            Connection->addCommand( cmd );
            break;
        case tt_yell:
            cmd.reset( new ShoutTC( cc->pos.x, cc->pos.y, cc->pos.z, message ) );
            Connection->addCommand( cmd );
            break;
        }
}

void Player::introducePerson(Character* cc) 
{
    boost::shared_ptr<BasicServerCommand>cmd( new IntroduceTC( cc->id, cc->name ) );
    Connection->addCommand( cmd );
}

void Player::deleteAllSkills() {
        Character::deleteAllSkills();
        sendAllSkills();

}

void Player::teachMagic(unsigned char type,unsigned char flag) {
        //>0 braucht nicht abgefragt werden da unsinged char immer >=0 sein muss
        if ( ( type < 4 ) ) {
               unsigned long int tflags=0;
               for ( int i = 0; i < 4; ++i ) {
                      //  std::cout << magic.flags[ i ] << " ";
                      tflags|=magic.flags[ i ];
               }
               // Prfen ob der Char schon flags hat. Wenn neun dann neue Magierichtung.
               if (tflags == 0) {
                      //   std::cout << "tflags==0, PARAMTYPE type= " << type << std::endl;
                      switch (type) {
                             case 0:
                                    magic.type = MAGE;
                                    break;
                             case 1:
                                    magic.type = PRIEST;
                                    break;
                             case 2:
                                    magic.type = BARD;
                                    break;
                             case 3:
                                    magic.type = DRUID;
                                    break;
                             default:
                                    magic.type = MAGE;
                      }
               }
               unsigned long int temp = 1;
               temp <<= flag;
               //std::cout << "addiere flag " << temp << std::endl;
               magic.flags[ type ] |= temp;
               sendMagicFlags(magic.type);
        }
}

void Player::inform(std::string message) {
        receiveText(tt_say, message, this);
}

bool Player::encumberance(uint16_t &movementCost) {
        // Implement Encumberance
        // ToDo: Add Encumberance
        int perEncumb = ( LoadWeight() * 100 ) / maxLoadWeight();
        if ( !IsAlive() )
               perEncumb += 20;
        if ( perEncumb > 50 ) {
               if ( !wasEncumberedSent() ) {
                      setEncumberedSent( true );
                      std::string tmessage;
                      switch ( getPlayerLanguage()) {
                             case Language::german:
                                    tmessage = "Du bist �berladen.";
                                    break;
                             case Language::english:
                                    tmessage = "You are encumbered.";
                                    break;
                             case Language::french:
                                    tmessage = "You are encumbered.";
                                    break;
                             default:
                                    tmessage = "You are encumbered.";
                      }
                      inform(tmessage);
               }
               if ( perEncumb >= 50 && perEncumb < 55 )
                      movementCost = (short int) ( movementCost * 1.1 );
               else if ( perEncumb >= 55 && perEncumb < 60 )
                      movementCost = (short int) ( movementCost * 1.3 );
               else if ( perEncumb >= 60 && perEncumb < 65 )
                      movementCost = (short int) ( movementCost * 1.5 );
               else if ( perEncumb >= 65 && perEncumb < 70 )
                      movementCost = (short int) ( movementCost * 1.7 );
               else if ( perEncumb >= 70 && perEncumb < 75 )
                      movementCost = (short int) ( movementCost * 1.8 );
               else if ( perEncumb >= 75 && perEncumb < 84 )
                      movementCost = movementCost * 2;
               else if ( perEncumb >= 84 && perEncumb < 87 )
                      movementCost = movementCost * 3;
               else if ( perEncumb >= 87 && perEncumb < 90 )
                      movementCost = movementCost * 4;
               else if ( perEncumb >= 90 && perEncumb < 92 )
                      movementCost = movementCost * 5;
               else if ( perEncumb == 92 )
                      movementCost = movementCost * 6;
               else if ( perEncumb == 93 )
                      movementCost = movementCost * 7;
               else if ( perEncumb == 94 )
                      movementCost = movementCost * 8;
               else if ( perEncumb == 95 )
                      movementCost = movementCost * 9;
               else if ( perEncumb > 95 ) {
                      std::string tmessage;
                      switch ( getPlayerLanguage()) {
                             case Language::german:
                                    tmessage="Du kannst nicht laufen, wenn du so viel tr�gst.";
                                    break;
                             case Language::english:
                                    tmessage="You are carrying too much to move!";
                                    break;
                             case Language::french:
                                    tmessage="You are carrying too much to move!";
                                    break;
                             default:
                                    tmessage="You are carrying too much to move!";
                      }
                      inform(tmessage);
                      return false;
               } else {
                      setEncumberedSent( false );
               }
        } //perEncumb > 50
        return true;
}

bool Player::move(direction dir, uint8_t mode)
{
#ifdef PLAYER_MOVE_DEBUG
    std::cout<<"Player::move position old: ";
    std::cout << pos.x << " " << pos.y << " " << pos.z << std::endl;
#endif
    //Ggf Scriptausfhrung wenn der Spieler auf ein Triggerfeld tritt
    _world->TriggerFieldMove(this,false);
    closeAllShowcasesOfMapContainers();
    // if we actively move we look into that direction...
    if ( mode != PUSH && dir != dir_up && dir != dir_down)
        faceto = (Character::face_to)dir;

    size_t steps;
    size_t j = 0;
    bool cont = true;
    if( mode != RUNNING )
        steps = 1;
    else
        steps = 2;

    position newpos, oldpos;
    uint8_t waitpages;
    while( j < steps && cont )
    {

        // check if we can move to our target field
        newpos = pos;
        oldpos = pos;

        newpos.x += _world->moveSteps[ dir ][ 0 ];
        newpos.y += _world->moveSteps[ dir ][ 1 ];
        newpos.z += _world->moveSteps[ dir ][ 2 ];

        Field * cfold=NULL;
        Field * cfnew=NULL;

        unsigned short int diffz = 0;

        bool fieldfound = false;

        // get the old tile... we need it to update the old tile as well as for the walking cost
        if (!_world->GetPToCFieldAt( cfold, pos.x, pos.y, pos.z ))
               return false;
        // we need to search for tiles below this level
        for (size_t i = 0; i < RANGEDOWN + 1 && !fieldfound; ++i) {
               fieldfound = _world->GetPToCFieldAt( cfnew, newpos.x, newpos.y, newpos.z, _world->tmap);
               // did we hit a targetfield?
               if (!fieldfound || cfnew->getTileId() == TRANSPARENTDISAPPEAR || cfnew->getTileId() == TRANSPARENT) {
                      fieldfound = false;
                      --newpos.z;
               }
        }
        if ( cfnew != NULL && fieldfound && ( cfnew->moveToPossible() || ( getClippingActive() == false && (isAdmin() || hasGMRight(gmr_isnotshownasgm) ) ) ) )
        {
                      uint16_t walkcost = getMovementCost(cfnew);
#ifdef PLAYER_MOVE_DEBUG
                      std::cout<< "Player::move Walkcost beforce encumberance: " << walkcost << std::endl;
#endif
                      if ( !encumberance( walkcost ) ) {
#ifdef PLAYER_MOVE_DEBUG
                             std::cout<< "Player::move Walkcost after encumberance Char overloadet: " << walkcost << std::endl;
#endif
                             //Char ueberladen
                             boost::shared_ptr<BasicServerCommand>cmd(new MoveAckTC( id, pos, NOMOVE, 0 ) );
                             Connection->addCommand( cmd );
                             return false;
                      } 
                      else 
                      {
#ifdef PLAYER_MOVE_DEBUG
                             std::cout<< "Player::move Walkcost after Char not overloadet encumberance: " << walkcost << std::endl;
#endif
                             int16_t diff = ( P_MIN_AP - actionPoints + walkcost) * 10;
                             // necessary to get smooth movement in client (dunno how this one is supposed to work exactly)
                             if (diff < 60) waitpages = 4;
                             else waitpages = (diff * 667) / 10000;
                             //if( mode != RUNNING || ( j == 1 && cont ) )
                             //    actionPoints -= walkcost; //abziehen der Movecosten
                      }
#ifdef PLAYER_MOVE_DEBUG
                      std::cout << "Player::move : Bewegung moeglich" << std::endl;
#endif
                      // Spieler vom alten Feld nehmen
                      cfold->removeChar();

                      // Spieler auf das neue Feld setzen
                      cfnew->setChar();
                      if ( newpos.z != oldpos.z )
                      {
                          //Z Coordinate hat sich ge�ndert komplettes update senden (Sp�ter durch teilupdate ersetzen)
                          updatePos(newpos);
                          boost::shared_ptr<BasicServerCommand>cmd( new MoveAckTC( id, pos, NOMOVE, 0 ) );
                          Connection->addCommand( cmd );
                          // Koordinate
                          cmd.reset( new SetCoordinateTC( pos ) );
                          Connection->addCommand( cmd );
                          sendFullMap();
                          cont = false;
                      }
                      else
                      {
                          if( mode != RUNNING || ( j == 1 && cont ) )
                          {   
                              boost::shared_ptr<BasicServerCommand> cmd(new MoveAckTC( id, newpos, mode, waitpages ) );
                              Connection->addCommand( cmd );
                          }
                          if( j == 1 && cont )
                          {
                              sendStepStripes( dir );
                          }
                          pos.x = newpos.x;
                          pos.y = newpos.y;
                          if( mode != RUNNING || ( j == 1 && cont ) )
                          {
                              sendStepStripes( dir );
                          }
                      }
                      // allen anderen Spielern die Bewegung bermitteln
                      if( mode != RUNNING || j == 1 || !cont )
                      {  
                          _world->sendCharacterMoveToAllVisiblePlayers( this, mode, waitpages );

#ifdef PLAYER_MOVE_DEBUG
                          std::cout << "Player move position new: ";
                          std::cout << pos.x << " " << pos.y << " " << pos.z << std::endl;
#endif

                          _world->sendAllVisibleCharactersToPlayer( this, true );
                      }
                      if ( oldpos.z - newpos.z > 0 ) 
                      {
                             int perEncumb = ( LoadWeight() * 100 ) / maxLoadWeight();
                             int damagePerLevel = ( 500 * perEncumb ) / ( battrib.agility / 2 );
                             int damage = damagePerLevel;
                             // Take half again as much damage for falling 2nd level
                             if ( diffz > 1 ) {
                                    // Damage = Damage * 2.5 in int terms.
                                    damage = damage * 3 - damage / 2;
                             }
                             if ( damage > 32767 ) {
                                    damage = 32767;
                             }
                             increaseAttrib( "hitpoints", -damage );
                      }
                      //Prfen ob Zielfeld ein Teleporter und evtl Spieler teleportieren
                      if ( cfnew->IsWarpField() ) {
                             position oldpos = pos, newpos;
                             cfnew->GetWarpField( newpos );
                             Warp(newpos);
                             cont = false;
                      }
                      //Prfen ob das Feld ein spezielles feld ist.
                      _world->checkFieldAfterMove( this, cfnew );

                      //Ggf Scriptausfhrung beim Betreten eines Triggerfeldes
                      _world->TriggerFieldMove(this,true);
                      //send the move to the monitoring clients
                      boost::shared_ptr<BasicServerCommand>cmd(new BBPlayerMoveTC(id, pos.x, pos.y, pos.z));
                      _world->monitoringClientList->sendCommand( cmd );
                      if( mode != RUNNING || j == 1 )
                      {
                          return true;
                      }
        } // neues Feld vorhanden und passierbar?
        else
        {
            if( j == 1 )
            {
                boost::shared_ptr<BasicServerCommand> cmd(new MoveAckTC( id, pos, NORMALMOVE, waitpages ) );
                Connection->addCommand( cmd );
                _world->sendCharacterMoveToAllVisiblePlayers( this, mode, waitpages );
                _world->sendAllVisibleCharactersToPlayer( this, true );
                return true;
            }
            else if( j == 0 )
            {
                boost::shared_ptr<BasicServerCommand>cmd(new MoveAckTC( id, pos, NOMOVE, 0 ) );
                Connection->addCommand( cmd );
                return false;
            }
        }
        ++j;
    } // loop (steps)
    
#ifdef PLAYER_MOVE_DEBUG
    std::cout << "movePlayer: Bewegung nicht moeglich \n";
#endif
    boost::shared_ptr<BasicServerCommand>cmd( new MoveAckTC( id, pos, NOMOVE, 0 ) );
    Connection->addCommand( cmd );
    return false;
}


void Player::startPlayerMenu(UserMenuStruct menu) {
        //anzahl der Items ermitteln
        boost::shared_ptr<BasicServerCommand>cmd( new StartPlayerMenuTC(menu));
        Connection->addCommand( cmd );
}

bool Player::Warp(position newPos) {
        bool warped = Character::Warp(newPos);
        if (warped) 
        {
                //look if there is a tile over the player set the maps over the player
                closeAllShowcasesOfMapContainers();
                _world->tmap = NULL;
                bool update = false;
                Field *testf;
                for ( int i = 0; i < RANGEUP; ++i )
                {
                    if ( _world->GetPToCFieldAt( testf, pos.x, pos.y, pos.z + 1 + i, _world->tmap ) ) 
                    {
                    // Ebene ber der neuen Position vorhanden
                    if ( ( ( under[ i ]  ) // Karte wurde vorher angezeigt
                            && ( testf->getTileId() == TRANSPARENT  ) ) // das neue Feld ber dem Spieler ist TRANSPARENT
                            || ( ( !under[ i ]  )     // Karte wurde vorher nicht angezeigt
                            && ( testf->getTileId() != TRANSPARENT  ) ) ) // das neue Feld ber dem Spieler ist nicht TRANSPARENT
                    {
                            // Verdeckung des Spielers hat sich ge�dert
                            update = true;
                            roofmap[ i ] = _world->tmap;
                            under[ i ] = ( testf->getTileId() != TRANSPARENT );
                    } 
                    else if ( _world->tmap != roofmap[ i ] ) 
                    {
                            // Spieler steht unter einer anderen Karte, ohne da�sich die Verdeckung
                            // ge�dert hat
                            update = true;
                            roofmap[ i ] = _world->tmap;
                        under[ i ] = ( testf->getTileId() != TRANSPARENT );
                    }
                } 
                else 
                {
                    // Spieler war vorher verdeckt, jetzt nicht mehr
                    if ( under[ i ] ) {
                       update = true;
                    }
                    roofmap[ i ] = NULL;
                    under[ i ] = false;
                }
                
            }
            if (update) sendWeather(_world->weather);   
            boost::shared_ptr<BasicServerCommand>cmd( new SetCoordinateTC( pos ) );
          Connection->addCommand( cmd );
          sendFullMap();
          visibleChars.clear();
          _world->sendAllVisibleCharactersToPlayer( this, true );
          cmd.reset( new BBPlayerMoveTC(id, pos.x, pos.y, pos.z) );
          _world->monitoringClientList->sendCommand( cmd );
          return true;
        }
        return false;
}

bool Player::forceWarp(position newPos) {
        bool warped = Character::forceWarp(newPos);
        if (warped) 
        {
                //look if there is a tile over the player set the maps over the player
                closeAllShowcasesOfMapContainers();
                _world->tmap = NULL;
                bool update = false;
                Field *testf;
                for ( int i = 0; i < RANGEUP; ++i )
                {
                    if ( _world->GetPToCFieldAt( testf, pos.x, pos.y, pos.z + 1 + i, _world->tmap ) ) 
                    {
                    // Ebene ber der neuen Position vorhanden
                    if ( ( ( under[ i ]  ) // Karte wurde vorher angezeigt
                            && ( testf->getTileId() == TRANSPARENT  ) ) // das neue Feld ber dem Spieler ist TRANSPARENT
                            || ( ( !under[ i ]  )     // Karte wurde vorher nicht angezeigt
                            && ( testf->getTileId() != TRANSPARENT  ) ) ) // das neue Feld ber dem Spieler ist nicht TRANSPARENT
                    {
                            // Verdeckung des Spielers hat sich ge�dert
                            update = true;
                            roofmap[ i ] = _world->tmap;
                            under[ i ] = ( testf->getTileId() != TRANSPARENT );
                    } 
                    else if ( _world->tmap != roofmap[ i ] ) 
                    {
                            // Spieler steht unter einer anderen Karte, ohne da�sich die Verdeckung
                            // ge�dert hat
                            update = true;
                            roofmap[ i ] = _world->tmap;
                        under[ i ] = ( testf->getTileId() != TRANSPARENT );
                    }
                } 
                else 
                {
                    // Spieler war vorher verdeckt, jetzt nicht mehr
                    if ( under[ i ] ) {
                       update = true;
                    }
                    roofmap[ i ] = NULL;
                    under[ i ] = false;
                }
                
            }
            
            if (update) sendWeather(_world->weather);   
            boost::shared_ptr<BasicServerCommand>cmd( new SetCoordinateTC( pos ) );
            Connection->addCommand( cmd );
          sendFullMap();
            _world->sendAllVisibleCharactersToPlayer( this, true );
            cmd.reset( new BBPlayerMoveTC(id, pos.x, pos.y, pos.z));
            _world->monitoringClientList->sendCommand( cmd );
            return true;
        }
        return false;
}

void Player::LTIncreaseHP(unsigned short int value, unsigned short int count, unsigned short int time) {
        //Schedulerobject erzeugen
        SchedulerObject * Heal = new SIncreaseHealtPoints(id,value,count,_world->scheduler->GetCurrentCycle()+time,time);
        //Task hinzufgen
        _world->scheduler->AddTask(Heal);
}

void Player::LTIncreaseMana(unsigned short int value, unsigned short int count, unsigned short int time) {
        //Schedulerobject erzeugen
        SchedulerObject * Heal = new SIncreaseManaPoints(id,value,count,_world->scheduler->GetCurrentCycle()+time,time);
        //Task hinzufgen
        _world->scheduler->AddTask(Heal);
}

void Player::openDepot(uint16_t depotid) {
        //_world->lookIntoDepot(this, 0 );
#ifdef PLAYER_PlayerDepot_DEBUG
        std::cout << "lookIntoDepot: Spieler " << cp->name << " schaut in sein Depot" << std::endl;
#endif
    //std::map<uint32_t,Container*>::iterator it;
    if ( depotContents.find( depotid ) != depotContents.end() )
    {
            if ( depotContents[ depotid ] != NULL )
        {
#ifdef PLAYER_PlayerDepot_DEBUG
                      std::cout << "Depotinhalt vorhanden" << std::endl;
#endif
                      // bisher ge�fnete Container im showcase schlie�n
                      showcases[ 0 ].clear();
                      // updaten des showcases des Spielers
                      showcases[ 0 ].startContainer( depotContents[ depotid ], false );
                      // �derungen an den Client schicken
                      boost::shared_ptr<BasicServerCommand>cmd( new UpdateShowCaseTC( 0, depotContents[depotid]->items) );
                      Connection->addCommand( cmd );                      
#ifdef PLAYER_PlayerDepot_DEBUG
                      std::cout << "lookIntoDepot: Ende" << std::endl;
#endif
                      mapshowcaseopen = true;
               }
        }
        else
        {
#ifdef PLAYER_PlayerDepot_DEBUG
        std::cout << "Depot mit der ID: "<<depotid<<" wird neu erstellt!"<<std:endl
#endif  
        depotContents[ depotid ] = new Container(0);
        showcases[ 0 ].clear();
        showcases[ 0 ].startContainer( depotContents[ depotid ], false);
        boost::shared_ptr<BasicServerCommand>cmd( new UpdateShowCaseTC( 0, depotContents[ depotid]->items) );
        Connection->addCommand( cmd );
        mapshowcaseopen = true;
        }
#ifdef PLAYER_PlayerDepot_DEBUG
        std::cout << "lookIntoDepot: Ende" << std::endl;
#endif
}

void Player::changeQualityItem(TYPE_OF_ITEM_ID id, short int amount) {
        Character::changeQualityItem(id, amount);
        updateBackPackView();
        for ( unsigned char i = MAX_BELT_SLOTS + MAX_BODY_ITEMS - 1; i > 0; --i ) {
               //Update des Inventorys.
               sendCharacterItemAtPos(i);
        }
}

void Player::changeQualityAt(unsigned char pos, short int amount) {
        //std::cout<<"in Player changeQualityAt!"<<std::endl;
        Character::changeQualityAt( pos, amount);
        //std::cout<<"ende des ccharacter ChangeQualityAt aufrufes"<<std::endl;
        sendCharacterItemAtPos( pos );
        sendCharacterItemAtPos( LEFT_TOOL ); //Item in der Linken hand nochmals senden um ggf ein gel�chtes Belegt an zu zeigen.
        sendCharacterItemAtPos( RIGHT_TOOL ); //Item in der rechten Hand nochmals senden um ggf ein gel�chtes Belegt an zu zeigen.
}

bool Player::hasGMRight(gm_rights right)
{
    return ( ( right & admin ) == static_cast<uint32_t>(right) );
}

void Player::setQuestProgress( uint16_t questid, uint32_t progress ) throw()
{
  //Laden einer Transaktion
  ConnectionManager::TransactionHolder transaction = dbmgr->getTransaction();

  try
  {
      std::stringstream Query;
      Query << "SELECT qpg_progress FROM questprogress WHERE qpg_userid = " << transaction.quote(id) << " AND qpg_questid = " << transaction.quote(questid);
      uint32_t oldprogress;
      size_t rows = di::select<di::Integer>( transaction, oldprogress, Query.str() );
      if ( rows > 0 ) // Update
      {
          if ( oldprogress != progress ) // Only update when the progress has changed
          {
              save(); // mandatory to avoid loss of integrity in case of server crash/kill
              
              std::stringstream qry;

              qry << "UPDATE questprogress SET qpg_progress = " << transaction.quote( progress );
              qry << " WHERE qpg_userid = " << transaction.quote( id );
              qry << " AND qpg_questid = " << transaction.quote(questid);

              di::exec( transaction, qry.str() );
              transaction.commit();
          }
      }
      else            // Insert
      {
          save(); // mandatory to avoid loss of integrity in case of server crash/kill
          
          di::insert( transaction, id, questid, progress, "INSERT INTO questprogress (qpg_userid, qpg_questid, qpg_progress)" );
          transaction.commit();
      }
  }
  catch ( std::exception &e )
  {
      std::cerr<<"exception: "<<e.what()<<" while setting QuestProgress!"<<std::endl;
      transaction.rollback();
  }
}

uint32_t Player::getQuestProgress( uint16_t questid ) throw()
{
  try
  {
      //Laden einer Transaktion
      ConnectionManager::TransactionHolder transaction = dbmgr->getTransaction();
      std::stringstream Query;
      Query << "SELECT qpg_progress FROM questprogress WHERE qpg_userid = " << transaction.quote(id) << " AND qpg_questid = " << transaction.quote(questid);
      uint32_t progress;
      size_t rows = di::select<di::Integer>( transaction, progress, Query.str() );
      if ( rows > 1 )
      {
              std::cerr<<" Too many QuestProgress Entries for char with id: "<<id<<" and quest: "<<questid<<" !"<<std::endl;
              return 0;
      }
      else if ( rows == 1) return progress;
      else return 0; //No Progress so far for that quest
  }
  catch ( std::exception &e )
  {
      std::cerr<<"exception: "<<e.what()<<" while getting QuestProgress!"<<std::endl;
      return 0;
  }
  return 0;
}

bool Player::moveDepotContentFrom( uint32_t sourcecharid, uint32_t targetdepotid, uint32_t sourcedepotid ) throw()
{
    Player * sourcechar = _world->Players.findID(sourcecharid);
    
    if (!sourcechar) { // Player offline -> use SQL to move depot content
        //sendMessage( "moveDepotContentFrom: Player offline - INIT" );
        
        try
        {
            //Laden einer Transaktion
            ConnectionManager::TransactionHolder transaction = dbmgr->getTransaction();
            std::map<uint32_t, Container*> depots, containers;
            std::map<uint32_t, Container*>::iterator it;
            std::stringstream qry;
            size_t rows;
            std::vector<uint16_t> itemlinenumber;
            std::vector<uint16_t> itemincontainer;
            std::vector<uint32_t> itemdepot;
            std::vector<TYPE_OF_ITEM_ID> itemid;
            std::vector<uint8_t> itemwear;
            std::vector<uint8_t> itemnumber;
            std::vector<uint16_t> itemquality;
            std::vector<uint32_t> itemdata;            
                        
            qry << "SELECT pit_linenumber,  pit_in_container, pit_depot, pit_itemid, pit_wear, pit_number, pit_quality, pit_data FROM playeritems";
            qry << " WHERE pit_playerid = " << transaction.quote( sourcecharid ) << " ORDER BY pit_linenumber";
            
            rows = di::select_all<di::Integer, di::Integer, di::Integer, di::Integer, di::Integer, di::Integer, di::Integer, di::Integer>(transaction, itemlinenumber, itemincontainer, itemdepot, itemid, itemwear, itemnumber, itemquality, itemdata, qry.str() );
            
            qry.str("");
            qry.clear();
            
            // testing whether depot exists
            qry << "SELECT DISTINCT pit_depot FROM playeritems WHERE pit_playerid = " << transaction.quote( sourcecharid );
            qry << " AND pit_depot = " << transaction.quote( sourcedepotid+1 );
            std::vector<uint32_t> depotid;
            if ( ! di::select_all<di::Integer>( transaction, depotid, qry.str() ) ) return false;
               
            // load volume
            ContainerStruct depotstruct;
            uint16_t depotsize = 0;
            if (ContainerItems->find(321,depotstruct))depotsize = depotstruct.ContainerVolume;
            
            depots[ sourcedepotid+1 ] = new Container(depotsize);

            unsigned int tempdepot, tempincont, linenumber;
            Container* tempc;
            Item tempi;
            
            qry.str("");
            qry.clear();
            qry << "DELETE FROM playeritems WHERE pit_playerid = " << transaction.quote( sourcecharid );
            qry << " AND pit_linenumber IN (";

            for ( unsigned int tuple = 0; tuple < rows; ++tuple ) {
                    tempincont = itemincontainer[tuple];
                    tempdepot = itemdepot[tuple];
                    linenumber = itemlinenumber[tuple];

                    tempi.id = itemid[tuple];
                    tempi.wear = itemwear[tuple];
                    tempi.number = itemnumber[tuple];
                    tempi.quality = itemquality[tuple];
                    tempi.data = itemdata[tuple];

                    // item is in a depot?
                    if ( tempdepot && (it = depots.find(tempdepot)) == depots.end() ) {
                            // item not needed since it is in a wrong depot
                            continue;
                    }
                    
                    // item is in a container?
                    if ( tempincont && ( it = containers.find(tempincont)) == containers.end() ) {
                            // item not needed since it is in a wrong container
                            continue;
                    }

                    if ( !tempincont && !tempdepot ) {
                            // item not needed since it is in no container and no depot
                            continue;
                    }
                    
                    ContainerStruct cont;
                    if ( ContainerItems->find( tempi.id, cont) ) {
                            tempc = new Container(cont.ContainerVolume);
                            it->second->InsertContainer( tempi, tempc );
                            containers[linenumber] = tempc;
                    } else {
                            it->second->InsertItem( tempi, false );
                    }
                    
                    qry << transaction.quote( linenumber ) << ",";
            }
            
            qry << transaction.quote( linenumber ) << ")";
            
            
            if ( (it=depotContents.find(targetdepotid + 1)) == depotContents.end() )
                depotContents[targetdepotid+1] = depots[ sourcedepotid+1 ];
            else {
                delete it->second;
                it->second = depots[ sourcedepotid+1 ];
            }
            
            di::exec( transaction, qry.str() );
            qry.str("");
            qry.clear();
            qry << "SELECT resort_items(" << transaction.quote( sourcecharid ) << ")";
            di::exec( transaction, qry.str() );
            transaction.commit();
                   
            closeAllShowcases();     
            //sendMessage( "moveDepotContentFrom: Player offline - SUCCESS" );
            return true;
        }
        catch ( std::exception &e )
        {
            std::cerr<<"exception: "<<e.what()<<" while moving offline depot!"<<std::endl;
            return false;
        }
    } else {           // Player online  -> use internal structures to move depot content
        //sendMessage( "moveDepotContentFrom: Player online - INIT" );

        std::map<uint32_t, Container*>::iterator it_source, it_target;
        // source depot does not exist?
        if ( (it_source=(sourcechar->depotContents).find(sourcedepotid + 1)) == sourcechar->depotContents.end() )
            return false;

        // save the depot
        Container * depotToMove = it_source->second;

        // remove depot from sourcechar
        sourcechar->depotContents.erase( it_source );
        
        // target depot does not exist?
        if ( (it_target=depotContents.find(targetdepotid + 1)) == depotContents.end() )
            depotContents[targetdepotid+1] = depotToMove;
        else {
            delete it_target->second;
            it_target->second = depotToMove;
        }

        closeAllShowcases();
        sourcechar->closeAllShowcases();
        //sendMessage( "moveDepotContentFrom: Player online - SUCCESS" );
        return true;
    }
        
    return false;
}

void Player::tempAttribCheck()
{
    if ( battrib.truesex != battrib.sex )
    {
        if ( battrib.time_sex <= 0 )
        {
            battrib.time_sex = 0;
            battrib.sex = battrib.truesex;
            sendAttrib("sex", Character::increaseAttrib("sex",0));
        }
        battrib.time_sex--;
    }
    if ( battrib.trueage != battrib.age )
    {
        if ( battrib.time_age <= 0 )
        {
            battrib.time_age = 0;
            battrib.age = battrib.trueage;
            sendAttrib("age", Character::increaseAttrib("age",0));
        }
        battrib.time_age--;
    }   
    if ( battrib.trueweight != battrib.weight )
    {
        if ( battrib.time_weight <= 0 )
        {
            battrib.time_weight = 0;
            battrib.weight = battrib.trueweight;
            sendAttrib("weight", Character::increaseAttrib("weight",0));
        }
        battrib.time_weight--;
    }   
    if ( battrib.truebody_height != battrib.body_height )
    {
        if ( battrib.time_body_height <= 0 )
        {
            battrib.time_body_height = 0;
            battrib.body_height = battrib.truebody_height;
            sendAttrib("body_height", Character::increaseAttrib("body_height",0));
        }
        battrib.time_body_height--;
    }
    if ( battrib.truehitpoints != battrib.hitpoints )
    {
        if ( battrib.time_hitpoints <= 0 )
        {
            battrib.time_hitpoints = 0;
            battrib.hitpoints = battrib.truehitpoints;
            sendAttrib("hitpoints", Character::increaseAttrib("hitpoints",0));
        }
        battrib.time_hitpoints--;
    }
    if ( battrib.truemana != battrib.mana )
    {
        if ( battrib.time_mana <= 0 )
        {
            battrib.time_mana = 0;
            battrib.mana = battrib.truemana;
            sendAttrib("mana", Character::increaseAttrib("mana",0));
        }
        battrib.time_mana--;
    }
    if ( battrib.trueattitude != battrib.attitude )
    {
        if ( battrib.time_attitude <= 0 )
        {
            battrib.time_attitude = 0;
            battrib.attitude = battrib.trueattitude;
            sendAttrib("attitude", Character::increaseAttrib("attitude",0));
        }
        battrib.time_attitude--;
    }
    if ( battrib.trueluck != battrib.luck )
    {
        if ( battrib.time_luck <= 0 )
        {
            battrib.time_luck = 0;
            battrib.luck = battrib.trueluck;
            sendAttrib("luck", Character::increaseAttrib("luck",0));
        }
        battrib.time_luck--;
    }
    if ( battrib.truestrength != battrib.strength )
    {
        if ( battrib.time_strength <= 0 )
        {
            battrib.time_strength = 0;
            battrib.strength = battrib.truestrength;
            sendAttrib("strenght", Character::increaseAttrib("strenght",0));
        }
        battrib.time_strength--;
    } 
    if ( battrib.truedexterity != battrib.dexterity )
    {
        if ( battrib.time_dexterity <= 0 )
        {
            battrib.time_dexterity = 0;
            battrib.dexterity = battrib.truedexterity;
            sendAttrib("dexterity", Character::increaseAttrib("dexterity",0));
        }
        battrib.time_dexterity--;
    } 
    if ( battrib.trueconstitution != battrib.constitution )
    {
        if ( battrib.time_constitution <= 0 )
        {
            battrib.time_constitution = 0;
            battrib.constitution = battrib.trueconstitution;
            sendAttrib("constitution", Character::increaseAttrib("constitution",0));
        }
        battrib.time_constitution--;
    }    
    if ( battrib.trueagility != battrib.agility )
    {
        if ( battrib.time_agility <= 0 )
        {
            battrib.time_agility = 0;
            battrib.agility = battrib.trueagility;
            sendAttrib("agility", Character::increaseAttrib("agility",0));
        }
        battrib.time_agility--;
    }  
    if ( battrib.trueintelligence != battrib.intelligence )
    {
        if ( battrib.time_intelligence <= 0 )
        {
            battrib.time_intelligence = 0;
            battrib.intelligence = battrib.trueintelligence;
            sendAttrib("intelligence", Character::increaseAttrib("inteligence",0));
        }
        battrib.time_intelligence--;
    } 
    if ( battrib.trueperception != battrib.perception )
    {
        if ( battrib.time_perception <= 0 )
        {
            battrib.time_perception = 0;
            battrib.perception = battrib.trueperception;
            sendAttrib("perception", Character::increaseAttrib("perception",0));
        }
        battrib.time_perception--;
    }
    if ( battrib.truewillpower != battrib.willpower )
    {
        if ( battrib.time_willpower <= 0 )
        {
            battrib.time_willpower = 0;
            battrib.willpower = battrib.truewillpower;
            sendAttrib("willpower", Character::increaseAttrib("willpower",0));
        }
        battrib.time_willpower--;
    }
    if ( battrib.trueessence != battrib.essence )
    {
        if ( battrib.time_essence <= 0 )
        {
            battrib.time_essence = 0;
            battrib.essence = battrib.trueessence;
            sendAttrib("essence", Character::increaseAttrib("essence",0));
        }
        battrib.time_essence--;
    }    
    if ( battrib.truefoodlevel != battrib.foodlevel )
    {
        if ( battrib.time_foodlevel <= 0 )
        {
            battrib.time_foodlevel = 0;
            battrib.foodlevel = battrib.truefoodlevel;
            sendAttrib("foodlevel", Character::increaseAttrib("foodlevel",0));
        }
        battrib.time_foodlevel--;
    }  
}

void Player::startAction(unsigned short int wait, unsigned short int ani, unsigned short int redoani, unsigned short int sound, unsigned short int redosound)
{
    ltAction->startLongTimeAction(wait, ani, redoani, sound, redosound);
}
 
void Player::abortAction()
{
    ltAction->abortAction();
}

void Player::successAction()
{
    ltAction->successAction();
}

void Player::actionDisturbed(Character * disturber)
{
    ltAction->actionDisturbed(disturber);
}

void Player::changeSource( Character * cc )
{
    ltAction->changeSource(cc);
}

void Player::changeSource( ScriptItem sI )
{
    ltAction->changeSource( sI );
}

void Player::changeSource( position pos )
{
    ltAction->changeSource( pos );
}

void Player::changeSource()
{
    ltAction->changeSource();
}

void Player::changeTarget( Character * cc )
{
    ltAction->changeTarget( cc );
}

void Player::changeTarget( ScriptItem sI )
{
    ltAction->changeTarget( sI );
}

void Player::changeTarget( position pos )
{
    ltAction->changeTarget( pos );
}

void Player::changeTarget()
{
    ltAction->changeTarget();
}

const unsigned short int Player::getPlayerLanguage() {
    return _player_language->_language;
}

void Player::setPlayerLanguage(Language::LanguageType mother_tongue) {
    _player_language = Language::create(mother_tongue);
}

void Player::sendMessage(std::string message) 
{
    boost::shared_ptr<BasicServerCommand>cmd(new SayTC( pos.x, pos.y, pos.z, message ));
    Connection->addCommand( cmd );
}

void Player::sendRelativeArea(int8_t zoffs)
{
      if( (screenwidth == 0) && (screenheight == 0) )
      {   
          // static view
            int x = pos.x;
            int y = pos.y - MAP_DIMENSION;
            int z = pos.z + zoffs;
            int e = zoffs * 3;
            if ( zoffs < 0 )
            {
                x -= e;
                y += e;
                e = 0;
            }
            //schleife von 0ben nach unten durch alle tiles
            World * world = World::get();
            for ( int i=0; i <= (MAP_DIMENSION + MAP_DOWN_EXTRA + e) * 2; ++i )
            {
                world->clientview.fillStripe( position(x,y,z), NewClientView::dir_right, MAP_DIMENSION+1-(i%2), &(World::get()->maps) );
                if ( world->clientview.getExists() ) Connection->addCommand(boost::shared_ptr<BasicServerCommand>( new MapStripeTC(position(x,y,z), NewClientView::dir_right) ) );
                  if ( i % 2 == 0 )
                      y += 1;
                  else
                      x -= 1;
            }
        }
        else
      {
          // dynamic view
          int x = pos.x - screenwidth + screenheight;
            int y = pos.y - screenwidth - screenheight;
            int z = pos.z + zoffs;
            int e = zoffs * 3;
            if ( zoffs < 0 )
            {
                x -= e;
                y += e;
                e = 0;
            }
            //schleife von 0ben nach unten durch alle tiles
            World * world = World::get();
            for ( int i=0; i <= ( 2*screenheight + MAP_DOWN_EXTRA + e) * 2; ++i )
            {
                world->clientview.fillStripe( position(x,y,z), NewClientView::dir_right, 2*screenwidth+1-(i%2), &(World::get()->maps) );
                if ( world->clientview.getExists() ) Connection->addCommand(boost::shared_ptr<BasicServerCommand>( new MapStripeTC(position(x,y,z), NewClientView::dir_right) ) );
                  if ( i % 2 == 0 )
                      y += 1;
                  else
                      x -= 1;
            }
      }
}

void Player::sendFullMap()
{
    for ( int8_t i = -2; i <= 2; ++i)sendRelativeArea( i );
    Connection->addCommand( boost::shared_ptr<BasicServerCommand>( new MapCompleteTC() ) );
}

void Player::sendDirStripe( viewdir direction, bool extraStripeForDiagonalMove )
{
    if( (screenwidth == 0) && (screenheight == 0) )
    {   
        // static view
        int x,y,z,e,l;
        NewClientView::stripedirection dir;
        int length = MAP_DIMENSION + 1;
        switch ( direction )
        {
            case upper:
                x = pos.x;
                y = pos.y - MAP_DIMENSION;
                dir = NewClientView::dir_right;
                if (extraStripeForDiagonalMove) --x;
                break;
            case left:
                x = pos.x;
                y = pos.y - MAP_DIMENSION;
                dir = NewClientView::dir_down;
                length += MAP_DOWN_EXTRA;
                if (extraStripeForDiagonalMove) ++x;
                break;
            case right:
                x = pos.x + MAP_DIMENSION;
                y = pos.y;
                dir = NewClientView::dir_down;
                length += MAP_DOWN_EXTRA;
                if (extraStripeForDiagonalMove) --y;
                break;
            case lower:
                x = pos.x - MAP_DIMENSION - MAP_DOWN_EXTRA;
                y = pos.y + MAP_DOWN_EXTRA;
                dir = NewClientView::dir_right;
                if (extraStripeForDiagonalMove) --y;
                break;
        }
        NewClientView * view = &(World::get()->clientview);
        for ( z = - 2; z <= 2; ++z)
        {
            e = (direction != lower && z > 0) ? z*3 : 0; // left, right and upper stripes moved up if z>0 to provide the client with info for detecting roofs
            l = (dir == NewClientView::dir_down && z > 0) ? e : 0; // right and left stripes have to become longer then
            if (extraStripeForDiagonalMove) ++l;
            view->fillStripe( position(x-z*3+e,y+z*3-e,pos.z+z), dir, length+l, &World::get()->maps );
            if ( view->getExists() ) Connection->addCommand(boost::shared_ptr<BasicServerCommand>( new MapStripeTC( position(x-z*3+e,y+z*3-e,pos.z+z), dir ) ) );
        }
    }
    else
    {
        // dynamic view
        int x,y,z,e,l;
        NewClientView::stripedirection dir;
        int length;
        switch ( direction )
        {
            case upper:
                x = pos.x - screenwidth + screenheight;
                y = pos.y - screenwidth - screenheight;
                dir = NewClientView::dir_right;
                length = 2*screenwidth + 1;
                if (extraStripeForDiagonalMove) --x;
                break;
            case left:
                x = pos.x - screenwidth + screenheight;
                y = pos.y - screenwidth - screenheight;
                dir = NewClientView::dir_down;
                length = 2*screenheight + 1 + MAP_DOWN_EXTRA;
                if (extraStripeForDiagonalMove) ++x;
                break;
            case right:
                x = pos.x + screenwidth + screenheight;
                y = pos.y + screenwidth - screenheight;
                dir = NewClientView::dir_down;
                length = 2*screenheight + 1 + MAP_DOWN_EXTRA;
                if (extraStripeForDiagonalMove) --y;
                break;
            case lower:
                x = pos.x - screenwidth - screenheight - MAP_DOWN_EXTRA;
                y = pos.y - screenwidth + screenheight + MAP_DOWN_EXTRA;
                dir = NewClientView::dir_right;
                length = 2*screenwidth + 1;
                if (extraStripeForDiagonalMove) --y;
                break;
        }
        NewClientView * view = &(World::get()->clientview);
        for ( z = - 2; z <= 2; ++z)
        {
            e = (direction != lower && z > 0) ? z*3 : 0; // left, right and upper stripes moved up if z>0 to provide the client with info for detecting roofs
            l = (dir == NewClientView::dir_down && z > 0) ? e : 0; // right and left stripes have to become longer then
            if (extraStripeForDiagonalMove) ++l;
            view->fillStripe( position(x-z*3+e,y+z*3-e,pos.z+z), dir, length+l, &World::get()->maps );
            if ( view->getExists() ) Connection->addCommand(boost::shared_ptr<BasicServerCommand>( new MapStripeTC( position(x-z*3+e,y+z*3-e,pos.z+z), dir ) ) );
        }
    }
}

void Player::sendStepStripes( direction dir )
{
                          switch( dir )
                          {
                            case ( dir_north ):
                                //bewegung nach norden (Mapstripe links und oben)
                                sendDirStripe( upper, false );
                                sendDirStripe( left, false );
                                break;
                            case ( dir_northeast ):
                                //bewegung nach nordosten (Mapstripe oben)
                                sendDirStripe( upper, true );
                                sendDirStripe( upper, false );
                                break;
                            case ( dir_east ) :
                                //bewegung nach osten (Mapstripe oben und rechts)
                                sendDirStripe( upper, false );
                                sendDirStripe( right, false );
                                break;
                            case ( dir_southeast ):
                                //bewegung suedosten (Mapstripe  rechts)
                                sendDirStripe( right, true );
                                sendDirStripe( right, false );
                                break;
                            case ( dir_south ):
                                //bewegung sueden (Mapstripe rechts und unten)
                                sendDirStripe( right, false );
                                sendDirStripe( lower, false );
                                break;
                            case ( dir_southwest ):
                                //bewegung suedwesten ( Mapstripe unten )
                                sendDirStripe( lower, true );
                                sendDirStripe( lower, false );
                                break;
                            case ( dir_west ):
                                //bewegung westen ( Mapstripe unten und links)
                                sendDirStripe( lower, false );
                                sendDirStripe( left, false );
                                break;
                            case ( dir_northwest ):
                                //bewegung nordwesten ( Mapstripe links )
                                sendDirStripe( left, true );
                                sendDirStripe( left, false );
                                break;
                            case ( dir_up ):
                            case ( dir_down ):
                                break;
                          }
}

void Player::sendSingleStripe( viewdir direction, int8_t zoffs )
{/* NOT USED YET
    int x,y,z;
    NewClientView::stripedirection dir;
    switch ( direction )
    {
        case upper:
            x = pos.x;
            y = pos.y - MAP_DIMENSION;
            dir = NewClientView::dir_right;
            break;
        case left:
            x = pos.x;
            y = pos.y - MAP_DIMENSION;
            dir = NewClientView::dir_down;
            break;
        case right:
            x = pos.x + MAP_DIMENSION;
            y = pos.y;
            dir = NewClientView::dir_down;
            break;
        case lower:
            x = pos.x - MAP_DIMENSION;
            y = pos.y;
            dir = NewClientView::dir_right;
            break;
    }
    z = pos.z + zoffs;
    NewClientView * view = &(World::get()->clientview);
    view->fillStripe( position(x,y,z), dir, &CWWorld::get()->maps );
    if ( view->getExists() ) Connection->addCommand(boost::shared_ptr<BasicServerCommand>( new MapStripeTC( position(x,y,z), dir ) ) );    
*/}

uint32_t Player::idleTime()
{
    time_t now;
    time( &now );
    return now - lastaction;
}

void Player::sendBook( uint16_t bookID )
{
    boost::shared_ptr<BasicServerCommand>cmd( new BookTC( bookID ) );
    Connection->addCommand( cmd );
}

void Player::sendCharDescription( TYPE_OF_CHARACTER_ID id,const std::string& desc)
{
    boost::shared_ptr<BasicServerCommand>cmd( new CharDescription( id, desc) );
    Connection->addCommand( cmd );
}

void Player::sendCharAppearance( TYPE_OF_CHARACTER_ID id, boost::shared_ptr<BasicServerCommand> appearance, bool always )
{
    //send appearance always or only if the char in question just appeared
    if( always || visibleChars.insert( id ).second )
    {
        Connection->addCommand( appearance );
    }
}

void Player::sendCharRemove( TYPE_OF_CHARACTER_ID id, boost::shared_ptr<BasicServerCommand> removechar )
{
    if( this->id != id )
    {
        visibleChars.erase( id );
        Connection->addCommand( removechar );
    }
}

