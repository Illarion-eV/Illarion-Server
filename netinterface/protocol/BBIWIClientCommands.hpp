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


#ifndef _BBIWI_CLIENT_COMMANDS_HPP
#define _BBIWI_CLIENT_COMMANDS_HPP

/**
*@defgroup BBIWIClientcommands BBIWI Clientcommands
*@ingroup Netinterface
*commands which are sended from the BBIWI client
*/


#include "CWorld.hpp"
#include "CLogger.hpp"
#include <boost/shared_ptr.hpp>
#include "netinterface/CBasicClientCommand.hpp"
#include "netinterface/protocol/BBIWIServerCommands.hpp"

/**
* @ingroup BBIWIClientcommands
* defines the definition bytes for the different bbiwi commands
*/
enum bbclientcommands
{
	BB_KEEPALIVE_TS = 0x01, /*< a keepalive from client is received */
	BB_BROADCAST_TS = 0x02, /*< a broadcast from the cliet is received */
	BB_DISCONNECT_TS = 0x03, /*< a the monitoring client disconnects*/
    BB_BAN_TS = 0x04, /*< player should be banned*/
    BB_TALKTO_TS = 0x05, /*< talk directly to a player*/
    BB_CHANGEATTRIB_TS = 0x06, /*< change a attribute*/
    BB_CHANGESKILL_TS = 0x07, /*< change a skill*/
    BB_SERVERCOMMAND_TS = 0x08, /*< a servercommand like reload*/
    BB_WARPPLAYER_TS = 0x09, /*< a warp player to a special position*/
    BB_SPEAKAS_TS = 0x0A, /*< talk as the player*/
    BB_REQUESTSTATS_TS = 0x0B,
    BB_REQUESTSKILLS_TS = 0x0C    
};

/**
* @ingroup BBIWIClientcommands
* client send a broadcast to all players
*/
class CBBBroadCastTS : public CBasicClientCommand
{
    public:
        CBBBroadCastTS() : CBasicClientCommand( BB_BROADCAST_TS )
        {
        }
        
        ~CBBBroadCastTS(){};
        
        void decodeData()
        {
            msg = getStringFromBuffer();
        }
        
        void performAction( CPlayer * player )
        {
            CWorld::get()->sendMessageToAllPlayers( msg );
            CWorld::get()->monitoringClientList->sendCommand(boost::shared_ptr<CBasicServerCommand>(new CBBMessageTC("[Server] Broadcast:",0 ) ));
            CWorld::get()->monitoringClientList->sendCommand(boost::shared_ptr<CBasicServerCommand>(new CBBMessageTC( msg, 0 ) ));
            std::string message = "By: " + player->name + "(" + CLogger::toString(player->id) + ")";
            CWorld::get()->monitoringClientList->sendCommand(boost::shared_ptr<CBasicServerCommand>( new CBBMessageTC(message,0 ) ));
        }
        
        boost::shared_ptr<CBasicClientCommand> clone()
        {
            boost::shared_ptr<CBasicClientCommand>cmd( new CBBBroadCastTS() );
			return cmd;
        }
        
        std::string msg;

};

/**
* @ingroup BBIWIClientcommands
* client send a special command
*/
class CBBRequestStatTS : public CBasicClientCommand
{
    public:
        CBBRequestStatTS() : CBasicClientCommand( BB_REQUESTSTATS_TS )
        {
        }
        
        ~CBBRequestStatTS(){};
        
        void decodeData()
        {
            id = getIntFromBuffer();
            name = getStringFromBuffer();
        }
        
        void performAction( CPlayer * player )
        {
            CPlayer * tempPlayer=NULL;
            tempPlayer = CWorld::get()->Players.find(name);
            if ( tempPlayer == NULL )
            {
                CWorld::PLAYERVECTOR::iterator playerIterator;
                for ( playerIterator = CWorld::get()->Players.begin(); playerIterator != CWorld::get()->Players.end(); ++playerIterator ) 
                {
                    if ( ( *playerIterator )->id == id ) 
                    {
                        tempPlayer = ( *playerIterator );
                    }
                }
            }
            if ( tempPlayer != NULL )
            {
                boost::shared_ptr<CBasicServerCommand>cmd(new CBBSendAttribTC( tempPlayer->id, "sex", tempPlayer->increaseAttrib( "sex",0) ));
                player->Connection->addCommand( cmd );
                cmd.reset( new CBBSendAttribTC( tempPlayer->id, "age", tempPlayer->increaseAttrib( "age",0) ));
                player->Connection->addCommand( cmd );
                cmd.reset( new CBBSendAttribTC( tempPlayer->id, "weight", tempPlayer->increaseAttrib( "weight",0) ));
                player->Connection->addCommand( cmd );
                cmd.reset( new CBBSendAttribTC( tempPlayer->id, "body_height", tempPlayer->increaseAttrib( "body_height",0) ));
                player->Connection->addCommand( cmd );
                cmd.reset( new CBBSendAttribTC( tempPlayer->id, "attitude", tempPlayer->increaseAttrib( "attitude",0) ) );
                player->Connection->addCommand( cmd );
                cmd.reset( new CBBSendAttribTC( tempPlayer->id, "luck", tempPlayer->increaseAttrib( "luck",0) ));
                player->Connection->addCommand( cmd );
                cmd.reset( new CBBSendAttribTC( tempPlayer->id, "strength", tempPlayer->increaseAttrib( "strength",0) ) );
                player->Connection->addCommand( cmd );
                cmd.reset( new CBBSendAttribTC( tempPlayer->id, "dexterity", tempPlayer->increaseAttrib( "dexterity",0) ));
                player->Connection->addCommand( cmd );
                cmd.reset( new CBBSendAttribTC( tempPlayer->id, "constitution", tempPlayer->increaseAttrib( "constitution",0) ));
                player->Connection->addCommand( cmd );
                cmd.reset( new CBBSendAttribTC( tempPlayer->id, "intelligence", tempPlayer->increaseAttrib( "intelligence",0) ));
                player->Connection->addCommand( cmd );
                cmd.reset( new CBBSendAttribTC( tempPlayer->id, "perception", tempPlayer->increaseAttrib( "perception",0) ));
                player->Connection->addCommand( cmd );
                cmd.reset( new CBBSendAttribTC( tempPlayer->id, "age", tempPlayer->increaseAttrib( "age",0) ));
                player->Connection->addCommand( cmd );
                cmd.reset( new CBBSendAttribTC( tempPlayer->id, "willpower", tempPlayer->increaseAttrib( "willpower",0) )); 
                player->Connection->addCommand( cmd );
                cmd.reset( new CBBSendAttribTC( tempPlayer->id, "essence", tempPlayer->increaseAttrib( "essence",0) ));
                player->Connection->addCommand( cmd );
                cmd.reset( new CBBSendAttribTC( tempPlayer->id, "agility", tempPlayer->increaseAttrib( "agility",0) ) );
                player->Connection->addCommand( cmd );
                
            }        
        }
        
        boost::shared_ptr<CBasicClientCommand> clone()
        {
            boost::shared_ptr<CBasicClientCommand>cmd( new CBBRequestStatTS() );
			return cmd;
        }
        
        TYPE_OF_CHARACTER_ID id;
        std::string name;
};


/**
* @ingroup BBIWIClientcommands
* client requests all skills of another player
*/
class CBBRequestSkillsTS : public CBasicClientCommand
{
    public:
        CBBRequestSkillsTS() : CBasicClientCommand( BB_REQUESTSKILLS_TS )
        {
        }
        
        ~CBBRequestSkillsTS(){};
        
        void decodeData()
        {
            id = getIntFromBuffer();
            name = getStringFromBuffer();
            type = getUnsignedCharFromBuffer();
        }
        
        void performAction( CPlayer * player )
        {
            CPlayer * tempPlayer=NULL;
            tempPlayer = CWorld::get()->Players.find(name);
            if ( tempPlayer == NULL )
            {
                CWorld::PLAYERVECTOR::iterator playerIterator;
                for ( playerIterator = CWorld::get()->Players.begin(); playerIterator != CWorld::get()->Players.end(); ++playerIterator ) 
                {
                    if ( ( *playerIterator )->id == id ) 
                    {
                        tempPlayer = ( *playerIterator );
                    }
                }
            }
            if ( tempPlayer != NULL )
            {
                CCharacter::SKILLMAP::const_iterator sIterator;
                for ( sIterator = tempPlayer->skills.begin(); sIterator != tempPlayer->skills.end(); ++sIterator)
                {
                    if ( type == sIterator->second.type )
                    {
                        boost::shared_ptr<CBasicServerCommand>cmd(new CBBSendSkillTC( tempPlayer->id, sIterator->second.type, sIterator->first, sIterator->second.major, sIterator->second.minor ));
                        player->Connection->addCommand( cmd );
                    }
                }
            }            
        }
        
        boost::shared_ptr<CBasicClientCommand> clone()
        {
            boost::shared_ptr<CBasicClientCommand>cmd( new CBBRequestSkillsTS() );
			return cmd;
        }
        
        TYPE_OF_CHARACTER_ID id;
        std::string name;
        uint8_t type;

};


/**
* @ingroup BBIWIClientcommands
* client wants to speak as a player
*/
class CBBSpeakAsTS : public CBasicClientCommand
{
    public:
        CBBSpeakAsTS() : CBasicClientCommand( BB_SPEAKAS_TS )
        {
        }
        
        ~CBBSpeakAsTS(){};
        
        void decodeData()
        {
            id = getIntFromBuffer();
            name = getStringFromBuffer();
            message = getStringFromBuffer();
        }
        
        void performAction( CPlayer * player )
        {
            CPlayer * tempPlayer=NULL;
            tempPlayer = CWorld::get()->Players.find(name);
            if ( tempPlayer == NULL )
            {
                CWorld::PLAYERVECTOR::iterator playerIterator;
                for ( playerIterator = CWorld::get()->Players.begin(); playerIterator != CWorld::get()->Players.end(); ++playerIterator ) 
                {
                    if ( ( *playerIterator )->id == id ) 
                    {
                        tempPlayer = ( *playerIterator );
                    }
                }
            }
            if ( tempPlayer != NULL )
            {
                tempPlayer->talk(CCharacter::tt_say,message);
            }
            CLogger::writeMessage("bbiwi", player->name + " talked as other player: " + tempPlayer->name + " with message: " + message);
        }
        
        boost::shared_ptr<CBasicClientCommand> clone()
        {
            boost::shared_ptr<CBasicClientCommand>cmd( new CBBSpeakAsTS() );
			return cmd;
        }
        
        TYPE_OF_CHARACTER_ID id;
        std::string name;
        std::string message;

};



/**
* @ingroup BBIWIClientcommands
* client wants a player to warp
*/
class CBBWarpPlayerTS : public CBasicClientCommand
{
    public:
        CBBWarpPlayerTS() : CBasicClientCommand( BB_WARPPLAYER_TS )
        {
        }
        
        ~CBBWarpPlayerTS(){};
        
        void decodeData()
        {
            id = getIntFromBuffer();
            name = getStringFromBuffer();
            posx = getIntFromBuffer();
            posy = getIntFromBuffer();
            posz = getIntFromBuffer();
        }
        
        void performAction( CPlayer * player )
        {
            CPlayer * tempPlayer=NULL;
            tempPlayer = CWorld::get()->Players.find(name);
            if ( tempPlayer == NULL )
            {
                CWorld::PLAYERVECTOR::iterator playerIterator;
                for ( playerIterator = CWorld::get()->Players.begin(); playerIterator != CWorld::get()->Players.end(); ++playerIterator ) 
                {
                    if ( ( *playerIterator )->id == id ) 
                    {
                        tempPlayer = ( *playerIterator );
                    }
                }
            }
            if ( tempPlayer != NULL )
            {
                position * pos = new position(posx,posy,posz);
                tempPlayer->Warp( (*pos) );
            }        
        }
        
        boost::shared_ptr<CBasicClientCommand> clone()
        {
            boost::shared_ptr<CBasicClientCommand>cmd( new CBBWarpPlayerTS() );
			return cmd;
        }
        
        TYPE_OF_CHARACTER_ID id;
        std::string name;
        int32_t posx,posy,posz;

};

/**
* @ingroup BBIWIClientcommands
* client send a special command
*/
class CBBServerCommandTS : public CBasicClientCommand
{
    public:
        CBBServerCommandTS() : CBasicClientCommand( BB_SERVERCOMMAND_TS )
        {
        }
        
        ~CBBServerCommandTS(){};
        
        void decodeData()
        {
            _command = getStringFromBuffer();
        }
        
        void performAction( CPlayer * player )
        {
            if ( _command == "nuke" ) CWorld::get()->montool_kill_command(player);
            if ( _command == "reload" ) CWorld::get()->montool_reload_command(player);
            if ( _command == "kickall" ) CWorld::get()->montool_kickall_command(player);
            if ( _command == "importmaps" ) CWorld::get()->montool_import_maps_command(player);
            if ( _command == "setloginfalse" ) CWorld::get()->montool_set_login(player,"false");
            if ( _command == "setlogintrue" )CWorld::get()->montool_set_login(player,"true");
        }
        
        boost::shared_ptr<CBasicClientCommand> clone()
        {
            boost::shared_ptr<CBasicClientCommand>cmd( new CBBServerCommandTS() );
			return cmd;
        }
        
        std::string _command;

};

/**
* @ingroup BBIWIClientcommands
* client wants to change skill of someone
*/
class CBBChangeAttribTS : public CBasicClientCommand
{
    public:
        CBBChangeAttribTS() : CBasicClientCommand( BB_CHANGEATTRIB_TS )
        {
        }
        
        ~CBBChangeAttribTS(){};
        
        void decodeData()
        {
            _plid = getIntFromBuffer();
            _plname = getStringFromBuffer();
            _attr = getStringFromBuffer();
            _value = getShortIntFromBuffer();
            std::cout<<"received ChangeAttrib: "<<_plid<<" "<<_plname<<" "<<_attr<<" "<<_value<<std::endl;
        }
        
        void performAction( CPlayer * player )
        {
            CPlayer * tempPlayer=NULL;
            tempPlayer = CWorld::get()->Players.find(_plname);
            if ( tempPlayer == NULL )
            {
                CWorld::PLAYERVECTOR::iterator playerIterator;
                for ( playerIterator = CWorld::get()->Players.begin(); playerIterator != CWorld::get()->Players.end(); ++playerIterator ) 
                {
                    if ( ( *playerIterator )->id == _plid ) 
                    {
                        tempPlayer = ( *playerIterator );
                    }
                }
            }
            if ( tempPlayer != NULL )
            {
                tempPlayer->increaseAttrib(_attr,_value);
            }
        }
        
        boost::shared_ptr<CBasicClientCommand> clone()
        {
            boost::shared_ptr<CBasicClientCommand>cmd( new CBBChangeAttribTS() );
			return cmd;
        }
        
        TYPE_OF_CHARACTER_ID _plid;
        std::string _plname;
        std::string _attr;
        short int _value;

};

/**
* @ingroup BBIWIClientcommands
* client wants to change an attrib of someone
*/
class CBBChangeSkillTS : public CBasicClientCommand
{
    public:
        CBBChangeSkillTS() : CBasicClientCommand( BB_CHANGEATTRIB_TS )
        {
        }
        
        ~CBBChangeSkillTS(){};
        
        void decodeData()
        {
            _plid = getIntFromBuffer();
            _plname = getStringFromBuffer();
            _type = getUnsignedCharFromBuffer();
            _skill = getStringFromBuffer();
            _value = getShortIntFromBuffer();
            std::cout<<"received ChangeSkill: "<<_plid<<" "<<_plname<<" "<<_type<<" "<<_skill<<" "<<_value<<std::endl;
        }
        
        void performAction( CPlayer * player )
        {
            CPlayer * tempPlayer=NULL;
            tempPlayer = CWorld::get()->Players.find(_plname);
            if ( tempPlayer == NULL )
            {
                CWorld::PLAYERVECTOR::iterator playerIterator;
                for ( playerIterator = CWorld::get()->Players.begin(); playerIterator != CWorld::get()->Players.end(); ++playerIterator ) 
                {
                    if ( ( *playerIterator )->id == _plid ) 
                    {
                        tempPlayer = ( *playerIterator );
                    }
                }
            }
            if ( tempPlayer != NULL )
            {
                tempPlayer->increaseSkill( _type, _skill, _value);
            }
        }
        
        boost::shared_ptr<CBasicClientCommand> clone()
        {
            boost::shared_ptr<CBasicClientCommand>cmd( new CBBChangeSkillTS() );
			return cmd;
        }
        
        TYPE_OF_CHARACTER_ID _plid;
        std::string _plname;
        uint8_t _type;
        std::string _skill;
        short int _value;

};

/**
* @ingroup BBIWIClientcommands
* client talks to someone
*/
class CBBTalktoTS : public CBasicClientCommand
{
    public:
        CBBTalktoTS() : CBasicClientCommand( BB_TALKTO_TS )
        {
        }
        
        virtual ~CBBTalktoTS(){};
        
        void decodeData()
        {
            playerid = getIntFromBuffer();
            playername = getStringFromBuffer();
            msg = getStringFromBuffer();
        }
        
        void performAction( CPlayer * player )
        {
            CPlayer * tempPlayer=NULL;
            tempPlayer = CWorld::get()->Players.find(playername);
            if ( tempPlayer == NULL )
            {
                CWorld::PLAYERVECTOR::iterator playerIterator;
                for ( playerIterator =CWorld::get()->Players.begin(); playerIterator !=CWorld::get()->Players.end(); ++playerIterator ) 
                {
                    if ( ( *playerIterator )->id == playerid ) 
                    {
                        tempPlayer = ( *playerIterator );
                    }
                }
            }
            if ( tempPlayer != NULL )
            {
                CWorld::get()->sendMessageToPlayer(tempPlayer, msg);
            }
        }
        
        boost::shared_ptr<CBasicClientCommand> clone()
        {
            boost::shared_ptr<CBasicClientCommand>cmd( new CBBTalktoTS() );
			return cmd;
        }
        
        TYPE_OF_CHARACTER_ID playerid;
        std::string playername;
        std::string msg;

};

/**
* @ingroup BBIWIClientcommands
* client disconnects
*/
class CBBDisconnectTS : public CBasicClientCommand
{
    public:
        CBBDisconnectTS() : CBasicClientCommand( BB_DISCONNECT_TS )
        {
        }
        
        virtual ~CBBDisconnectTS(){};
        
        void decodeData()
        {

        }
        
        void performAction( CPlayer * player )
        {
            //we want to disconnect so we close the connection
            player->Connection->closeConnection();

        }
        
        boost::shared_ptr<CBasicClientCommand> clone()
        {
            boost::shared_ptr<CBasicClientCommand>cmd( new CBBDisconnectTS() );
			return cmd;
        }

};


/**
* @ingroup BBIWIClientcommands
* character received a keepalive
*/
class CBBKeepAliveTS : public CBasicClientCommand
{
    public:
        CBBKeepAliveTS() : CBasicClientCommand( BB_KEEPALIVE_TS )
        {

        }
        
        ~CBBKeepAliveTS(){};
        
        void decodeData()
        {

        }
        
        void performAction( CPlayer * player )
        {
            time( &(player->lastkeepalive) );
        }
        
        boost::shared_ptr<CBasicClientCommand> clone()
        {
            boost::shared_ptr<CBasicClientCommand>cmd( new CBBKeepAliveTS() );
			return cmd;
        }

};

/**
* @ingroup BBIWIClientcommands
* character is banned for an amount of time
*/
class CBBBanTS : public CBasicClientCommand
{
    public:
        CBBBanTS() : CBasicClientCommand( BB_BAN_TS )
        {
        }
        
        ~CBBBanTS(){};
        
        void decodeData()
        {
            id = getIntFromBuffer();
            name = getStringFromBuffer();
            time = getIntFromBuffer();
        }
        
        void performAction( CPlayer * player )
        {
            CPlayer * tempPlayer=NULL;
            tempPlayer =CWorld::get()->Players.find(name);
            if ( tempPlayer == NULL )
            {
                CWorld::PLAYERVECTOR::iterator playerIterator;
                for ( playerIterator =CWorld::get()->Players.begin(); playerIterator !=CWorld::get()->Players.end(); ++playerIterator ) 
                {
                    if ( ( *playerIterator )->id == id ) 
                    {
                        tempPlayer = ( *playerIterator );
                    }
                }
            }
            if ( tempPlayer != NULL )
            {
                CWorld::get()->ban(tempPlayer, time, player->id);
                boost::shared_ptr<CBasicServerCommand>cmd( new CBBMessageTC( "Player: " + name + "(" + CLogger::toString(id) + ")" + " banned by: " + player->name + "(" + CLogger::toString(player->id) + ")", 0) );
                CWorld::get()->monitoringClientList->sendCommand( cmd );
            }
            else
            {
                boost::shared_ptr<CBasicServerCommand>cmd(new CBBMessageTC( "Cannot find the player: " + name + "(" + CLogger::toString(id) + ")",0 ));
                player->Connection->addCommand( cmd );
            }
        }
        
        boost::shared_ptr<CBasicClientCommand> clone()
        {
            boost::shared_ptr<CBasicClientCommand>cmd( new CBBBanTS() );
			return cmd;
        }
        TYPE_OF_CHARACTER_ID id; /*<which character is banned*/
        std::string name; /*<which character is banned*/
        uint32_t time; /*<how long is he banned*/
};

#endif
