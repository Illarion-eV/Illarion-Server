//  illarionserver - server for the game Illarion
//  Copyright 2011 Illarion e.V.
//
//  This file is part of illarionserver.
//
//  illarionserver is free software: you can redistribute it and/or modify
//  it under the terms of the GNU Affero General Public License as published by
//  the Free Software Foundation, either version 3 of the License, or
//  (at your option) any later version.
//
//  illarionserver is distributed in the hope that it will be useful,
//  but WITHOUT ANY WARRANTY; without even the implied warranty of
//  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//  GNU Affero General Public License for more details.
//
//  You should have received a copy of the GNU Affero General Public License
//  along with illarionserver.  If not, see <http://www.gnu.org/licenses/>.

#ifndef BBIWI_SERVER_COMMANDS_HPP
#define BBIWI_SERVER_COMMANDS_HPP

#include "globals.hpp"
#include "netinterface/BasicServerCommand.hpp"
#include "types.hpp"

#include <string>

enum bbservercommands {
    BB_LOGINSUCCESSFUL_TC = 0x00,
    BB_MESSAGE_TC = 0x01,    /*<message send from the Server to the client*/
    BB_PLAYER_TC = 0x02,     /*<a complete package of player data is send*/
    BB_TALK_TC = 0x03,       /*<a player says something*/
    BB_LOGOUT_TC = 0x04,     /*<a player logs out*/
    BB_PLAYERMOVE_TC = 0x05, /*<a player moves*/
    BB_SENDATTRIB_TC = 0x06, /*<we got a attrib change*/
    BB_SENDSKILL_TC = 0x07,  /*<we got a skillchange*/
    BB_SENDACTION_TC = 0x08
};

class BBLoginSuccessfulTC : public BasicServerCommand {
public:
    BBLoginSuccessfulTC();
};

class BBSendActionTC : public BasicServerCommand {
public:
    BBSendActionTC(TYPE_OF_CHARACTER_ID id, uint8_t type, const std::string &desc);
};

class BBSendSkillTC : public BasicServerCommand {
public:
    BBSendSkillTC(TYPE_OF_CHARACTER_ID id, TYPE_OF_SKILL_ID skill, short int value, short int minor);
};

class BBSendAttribTC : public BasicServerCommand {
public:
    BBSendAttribTC(TYPE_OF_CHARACTER_ID id, const std::string &attribute, short int value);
};

class BBLogOutTC : public BasicServerCommand {
public:
    explicit BBLogOutTC(TYPE_OF_CHARACTER_ID id);
};

class BBTalkTC : public BasicServerCommand {
public:
    BBTalkTC(TYPE_OF_CHARACTER_ID id, uint8_t tt, const std::string &msg);
};

class BBPlayerTC : public BasicServerCommand {
public:
    BBPlayerTC(TYPE_OF_CHARACTER_ID id, const std::string &name, const position &pos);
};

class BBPlayerMoveTC : public BasicServerCommand {
public:
    BBPlayerMoveTC(TYPE_OF_CHARACTER_ID id, const position &pos);
};

class BBMessageTC : public BasicServerCommand {
public:
    BBMessageTC(const std::string &msg, uint8_t type);
};

#endif
