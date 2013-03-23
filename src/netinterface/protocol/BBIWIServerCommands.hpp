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


#ifndef _BBIWI_SERVER_COMMANDS_HPP
#define _BBIWI_SERVER_COMMANDS_HPP

#include "netinterface/BasicServerCommand.hpp"
#include "types.hpp"
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
enum bbservercommands {
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
class BBSendActionTC : public BasicServerCommand {
public:

    BBSendActionTC(TYPE_OF_CHARACTER_ID id, std::string name, uint8_t type, std::string desc) : BasicServerCommand(BB_SENDACTION_TC) {
        addIntToBuffer(id);
        addStringToBuffer(name);
        addUnsignedCharToBuffer(type);
        addStringToBuffer(desc);
    }
};


/**
* @ingroup BBIWIServerCommands
* command when skill is sended
*/
class BBSendSkillTC : public BasicServerCommand {
public:

    BBSendSkillTC(TYPE_OF_CHARACTER_ID id, TYPE_OF_SKILL_ID skill, short int value, short int minor) : BasicServerCommand(BB_SENDSKILL_TC) {
        addIntToBuffer(id);
        addUnsignedCharToBuffer(skill);
        addShortIntToBuffer(value);
        addShortIntToBuffer(minor);
    }
};

/**
* @ingroup BBIWIServerCommands
* command when a character talks
*/
class BBSendAttribTC : public BasicServerCommand {
public:

    BBSendAttribTC(TYPE_OF_CHARACTER_ID id, std::string attr, short int value) : BasicServerCommand(BB_SENDATTRIB_TC) {
        addIntToBuffer(id);
        addStringToBuffer(attr);
        addShortIntToBuffer(value);
    }
};

/**
* @ingroup BBIWIServerCommands
* command when a character talks
*/
class BBLogOutTC : public BasicServerCommand {
public:

    BBLogOutTC(TYPE_OF_CHARACTER_ID id, std::string name) : BasicServerCommand(BB_LOGOUT_TC) {
        addIntToBuffer(id);
        addStringToBuffer(name);
    }
};

/**
* @ingroup BBIWIServerCommands
* command when a character talks
*/
class BBTalkTC : public BasicServerCommand {
public:

    BBTalkTC(TYPE_OF_CHARACTER_ID id, std::string name, uint8_t tt, std::string msg) : BasicServerCommand(BB_TALK_TC) {
        addIntToBuffer(id);
        addStringToBuffer(name);
        addUnsignedCharToBuffer(tt);
        addStringToBuffer(msg);
    }
};

/**
* @ingroup BBIWIServerCommands
* command when a player logged in
*/
class BBPlayerTC : public BasicServerCommand {
public:
    BBPlayerTC(TYPE_OF_CHARACTER_ID id, std::string name, int posx, int posy, int posz) : BasicServerCommand(BB_PLAYER_TC) {
        addIntToBuffer(id);
        addStringToBuffer(name);
        addIntToBuffer(posx);
        addIntToBuffer(posy);
        addIntToBuffer(posz);
        std::cout<<"sended data of new player: "<<name<<std::endl;
    }
};

/**
* @ingroup BBIWIServerCommands
* command when the player has moved
*/
class BBPlayerMoveTC : public BasicServerCommand {
public:
    BBPlayerMoveTC(TYPE_OF_CHARACTER_ID id, int newx, int newy, int newz) : BasicServerCommand(BB_PLAYERMOVE_TC) {
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
class BBMessageTC : public BasicServerCommand {
public:
    BBMessageTC(std::string msg, uint8_t id) : BasicServerCommand(BB_MESSAGE_TC) {
        addStringToBuffer(msg);
        addUnsignedCharToBuffer(id);
    }
};

#endif
