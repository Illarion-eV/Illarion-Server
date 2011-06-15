#ifndef _BBIWI_SERVER_COMMANDS_HPP
#define _BBIWI_SERVER_COMMANDS_HPP

#include "netinterface/CBasicServerCommand.hpp"
#include "types.h"
#include <string>


/**
* @defgroup BBIWIServercommands BBIWI Servercommands
* @ingroup Netinterface
* commands which are sended from the server to the bbiwi client
*/

/**
* @ingroup BBIWIServerCommands
* defines the initializing bytes of BBIWI Server commands
*/
enum bbservercommands
{
	BB_MESSAGE_TC = 0x01, /*<message send from the Server to the client*/
	BB_PLAYER_TC = 0x02, /*<a complete package of player data is send*/
	BB_TALK_TC = 0x03, /*<a player says something*/
	BB_LOGOUT_TC = 0x04, /*<a player logs out*/
    BB_PLAYERMOVE_TC = 0x05, /*<a player moves*/
    BB_SENDATTRIB_TC = 0x06, /*<we got a attrib change*/
    BB_SENDSKILL_TC = 0x07, /*<we got a skillchange*/
    BB_SENDACTION_TC = 0x08
};

/**
* @ingroup BBIWIServerCommands
* command when a action is sendet
*/
class CBBSendActionTC : public CBasicServerCommand
{
    public:

        CBBSendActionTC(TYPE_OF_CHARACTER_ID id, std::string name, uint8_t type, std::string desc) : CBasicServerCommand( BB_SENDACTION_TC ) 
        {
            addIntToBuffer( id );
            addStringToBuffer( name );
            addUnsignedCharToBuffer( type );
            addStringToBuffer( desc );
        }
};


/**
* @ingroup BBIWIServerCommands
* command when skill is sended
*/
class CBBSendSkillTC : public CBasicServerCommand
{
    public:

        CBBSendSkillTC(TYPE_OF_CHARACTER_ID id, uint8_t type, std::string name, short int value, short int minor) : CBasicServerCommand( BB_SENDSKILL_TC ) 
        {
            addIntToBuffer( id );
            addUnsignedCharToBuffer( type );
            addStringToBuffer( name );
            addShortIntToBuffer( value );
            addShortIntToBuffer( minor );
        }
};

/**
* @ingroup BBIWIServerCommands
* command when a character talks
*/
class CBBSendAttribTC : public CBasicServerCommand
{
    public:

        CBBSendAttribTC(TYPE_OF_CHARACTER_ID id, std::string attr, short int value) : CBasicServerCommand( BB_SENDATTRIB_TC ) 
        {
            addIntToBuffer( id );
            addStringToBuffer( attr );
            addShortIntToBuffer( value );
        }
};

/**
* @ingroup BBIWIServerCommands 
* command when a character talks
*/
class CBBLogOutTC : public CBasicServerCommand
{
    public:

        CBBLogOutTC(TYPE_OF_CHARACTER_ID id, std::string name) : CBasicServerCommand( BB_LOGOUT_TC ) 
        {
            addIntToBuffer( id );
            addStringToBuffer( name );
        }
};

/**
* @ingroup BBIWIServerCommands
* command when a character talks
*/
class CBBTalkTC : public CBasicServerCommand
{
    public:

        CBBTalkTC(TYPE_OF_CHARACTER_ID id, std::string name, uint8_t tt, std::string msg) : CBasicServerCommand( BB_TALK_TC ) 
        {
            addIntToBuffer( id );
            addStringToBuffer( name );
            addUnsignedCharToBuffer( tt );
            addStringToBuffer( msg );
        }
};

/**
* @ingroup BBIWIServerCommands
* command when a player logged in 
*/
class CBBPlayerTC : public CBasicServerCommand
{
    public:
        CBBPlayerTC(TYPE_OF_CHARACTER_ID id, std::string name, int posx, int posy, int posz) : CBasicServerCommand( BB_PLAYER_TC ) 
        {
            addIntToBuffer( id );
            addStringToBuffer( name );
            addIntToBuffer( posx );
            addIntToBuffer( posy );
            addIntToBuffer( posz );
            std::cout<<"sended data of new player: "<<name<<std::endl;
        }
};

/**
* @ingroup BBIWIServerCommands
* command when the player has moved
*/
class CBBPlayerMoveTC : public CBasicServerCommand
{
    public:
        CBBPlayerMoveTC(TYPE_OF_CHARACTER_ID id, int newx, int newy, int newz) : CBasicServerCommand( BB_PLAYERMOVE_TC ) 
        {
            addIntToBuffer(id);
            addIntToBuffer(newx);
            addIntToBuffer(newy);
            addIntToBuffer(newz);
        }
};

/**
* @ingroup BBIWIServerCommands
* command when a general message arrived from server
*/
class CBBMessageTC : public CBasicServerCommand
{
    public:
        CBBMessageTC(std::string msg, uint8_t id) : CBasicServerCommand( BB_MESSAGE_TC ) 
        {
            addStringToBuffer( msg );
            addUnsignedCharToBuffer( id );
        }
};

#endif
