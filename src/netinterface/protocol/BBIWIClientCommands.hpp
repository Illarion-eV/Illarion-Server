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

#ifndef BBIWI_CLIENT_COMMANDS_HPP
#define BBIWI_CLIENT_COMMANDS_HPP

#include "netinterface/BasicClientCommand.hpp"
#include "types.hpp"

class Player;

enum bbclientcommands
{
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
    ~BBBroadCastTS() override;

    void decodeData() override;
    void performAction(Player *player) override;
    auto clone() -> ClientCommandPointer override;
};

class BBSpeakAsTS : public BasicClientCommand {
    TYPE_OF_CHARACTER_ID id = 0;
    std::string message;

public:
    BBSpeakAsTS();
    ~BBSpeakAsTS() override;

    void decodeData() override;
    void performAction(Player *player) override;
    auto clone() -> ClientCommandPointer override;
};

class BBWarpPlayerTS : public BasicClientCommand {
    TYPE_OF_CHARACTER_ID id = 0;
    int16_t posx = 0, posy = 0, posz = 0;

public:
    BBWarpPlayerTS();
    ~BBWarpPlayerTS() override;

    void decodeData() override;
    void performAction(Player *player) override;
    auto clone() -> ClientCommandPointer override;
};

class BBServerCommandTS : public BasicClientCommand {
    std::string _command;

public:
    BBServerCommandTS();
    ~BBServerCommandTS() override;

    void decodeData() override;
    void performAction(Player *player) override;
    auto clone() -> ClientCommandPointer override;
};

class BBChangeAttribTS : public BasicClientCommand {
    TYPE_OF_CHARACTER_ID id = 0;
    std::string attrib;
    short int value{0};

public:
    BBChangeAttribTS();
    ~BBChangeAttribTS() override;

    void decodeData() override;
    void performAction(Player *player) override;
    auto clone() -> ClientCommandPointer override;
};

class BBChangeSkillTS : public BasicClientCommand {
    TYPE_OF_CHARACTER_ID id = 0;
    TYPE_OF_SKILL_ID skill = 0;
    short int value{0};

public:
    BBChangeSkillTS();
    ~BBChangeSkillTS() override;

    void decodeData() override;
    void performAction(Player *player) override;
    auto clone() -> ClientCommandPointer override;
};

class BBTalktoTS : public BasicClientCommand {
    TYPE_OF_CHARACTER_ID id = 0;
    std::string msg;

public:
    BBTalktoTS();
    ~BBTalktoTS() override;

    void decodeData() override;
    void performAction(Player *player) override;
    auto clone() -> ClientCommandPointer override;
};

class BBDisconnectTS : public BasicClientCommand {
public:
    BBDisconnectTS();
    ~BBDisconnectTS() override;

    void decodeData() override;
    void performAction(Player *player) override;
    auto clone() -> ClientCommandPointer override;
};

class BBKeepAliveTS : public BasicClientCommand {
public:
    BBKeepAliveTS();
    ~BBKeepAliveTS() override;

    void decodeData() override;
    void performAction(Player *player) override;
    auto clone() -> ClientCommandPointer override;
};

class BBBanTS : public BasicClientCommand {
    TYPE_OF_CHARACTER_ID id = 0;
    uint32_t time = 0;

public:
    BBBanTS();
    ~BBBanTS() override;

    void decodeData() override;
    void performAction(Player *player) override;
    auto clone() -> ClientCommandPointer override;
};

#endif
