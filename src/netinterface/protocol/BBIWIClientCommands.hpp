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


#ifndef _BBIWI_CLIENT_COMMANDS_HPP_
#define _BBIWI_CLIENT_COMMANDS_HPP_

#include "netinterface/BasicClientCommand.hpp"
#include "types.hpp"

class Player;

enum bbclientcommands {
    BB_KEEPALIVE_TS = 0x01,
    BB_BROADCAST_TS = 0x02,
    BB_DISCONNECT_TS = 0x03,
    BB_BAN_TS = 0x04,
    BB_TALKTO_TS = 0x05,
    BB_CHANGEATTRIB_TS = 0x06,
    BB_CHANGESKILL_TS = 0x07,
    BB_SERVERCOMMAND_TS = 0x08,
    BB_WARPPLAYER_TS = 0x09,
    BB_SPEAKAS_TS = 0x0A
};

class BBBroadCastTS : public BasicClientCommand {
    std::string msg;

public:
    BBBroadCastTS();
    virtual ~BBBroadCastTS();

    virtual void decodeData() override;
    virtual void performAction(Player *player) override;
    virtual ClientCommandPointer clone() override;
};

class BBSpeakAsTS : public BasicClientCommand {
    TYPE_OF_CHARACTER_ID id = 0;
    std::string message;
    
public:
    BBSpeakAsTS();
    virtual ~BBSpeakAsTS();

    virtual void decodeData() override;
    virtual void performAction(Player *player) override;
    virtual ClientCommandPointer clone() override;
};

class BBWarpPlayerTS : public BasicClientCommand {
    TYPE_OF_CHARACTER_ID id = 0;
    int16_t posx = 0, posy = 0, posz = 0;

public:
    BBWarpPlayerTS();
    virtual ~BBWarpPlayerTS();

    virtual void decodeData() override;
    virtual void performAction(Player *player) override;
    virtual ClientCommandPointer clone() override;
};

class BBServerCommandTS : public BasicClientCommand {
    std::string _command;

public:
    BBServerCommandTS();
    virtual ~BBServerCommandTS();

    virtual void decodeData() override;
    virtual void performAction(Player *player) override;
    virtual ClientCommandPointer clone() override;
};

class BBChangeAttribTS : public BasicClientCommand {
    TYPE_OF_CHARACTER_ID id = 0;
    std::string attrib;
    short int value;

public:
    BBChangeAttribTS();
    virtual ~BBChangeAttribTS();

    virtual void decodeData() override;
    virtual void performAction(Player *player) override;
    virtual ClientCommandPointer clone() override;
};

class BBChangeSkillTS : public BasicClientCommand {
    TYPE_OF_CHARACTER_ID id = 0;
    TYPE_OF_SKILL_ID skill = 0;
    short int value;

public:
    BBChangeSkillTS();
    virtual ~BBChangeSkillTS();

    virtual void decodeData() override;
    virtual void performAction(Player *player) override;
    virtual ClientCommandPointer clone() override;
};

class BBTalktoTS : public BasicClientCommand {
    TYPE_OF_CHARACTER_ID id = 0;
    std::string msg;
    
public:
    BBTalktoTS();
    virtual ~BBTalktoTS();

    virtual void decodeData() override;
    virtual void performAction(Player *player) override;
    virtual ClientCommandPointer clone() override;
};

class BBDisconnectTS : public BasicClientCommand {
public:
    BBDisconnectTS();
    virtual ~BBDisconnectTS();
    
    virtual void decodeData() override;
    virtual void performAction(Player *player) override;
    virtual ClientCommandPointer clone() override;
};

class BBKeepAliveTS : public BasicClientCommand {
public:
    BBKeepAliveTS();
    virtual ~BBKeepAliveTS();

    virtual void decodeData() override;
    virtual void performAction(Player *player) override;
    virtual ClientCommandPointer clone() override;
};

class BBBanTS : public BasicClientCommand {
    TYPE_OF_CHARACTER_ID id = 0;
    uint32_t time = 0;

public:
    BBBanTS();
    virtual ~BBBanTS();

    virtual void decodeData() override;
    virtual void performAction(Player *player) override;
    virtual ClientCommandPointer clone() override;
};

#endif
