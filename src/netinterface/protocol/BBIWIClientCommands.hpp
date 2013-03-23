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


#include "World.hpp"
#include "Logger.hpp"
#include <boost/shared_ptr.hpp>
#include "netinterface/BasicClientCommand.hpp"
#include "netinterface/protocol/BBIWIServerCommands.hpp"

/**
* @ingroup BBIWIClientcommands
* defines the definition bytes for the different bbiwi commands
*/
enum bbclientcommands {
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
class BBBroadCastTS : public BasicClientCommand {
public:
    BBBroadCastTS() : BasicClientCommand(BB_BROADCAST_TS) {
    }

    virtual ~BBBroadCastTS() {};

    virtual void decodeData() override {
        msg = getStringFromBuffer();
    }

    virtual void performAction(Player *player) override {
        World::get()->sendMessageToAllPlayers(msg);
        World::get()->monitoringClientList->sendCommand(ServerCommandPointer(new BBMessageTC("[Server] Broadcast:",0)));
        World::get()->monitoringClientList->sendCommand(ServerCommandPointer(new BBMessageTC(msg, 0)));
        std::string message = "By: " + player->to_string();
        World::get()->monitoringClientList->sendCommand(ServerCommandPointer(new BBMessageTC(message,0)));
    }

    virtual ClientCommandPointer clone() override {
        ClientCommandPointer cmd(new BBBroadCastTS());
        return cmd;
    }

    std::string msg;

};

/**
* @ingroup BBIWIClientcommands
* client send a special command
*/
class BBRequestStatTS : public BasicClientCommand {
public:
    BBRequestStatTS() : BasicClientCommand(BB_REQUESTSTATS_TS) {
    }

    virtual ~BBRequestStatTS() {};

    virtual void decodeData() override {
        id = getIntFromBuffer();
        name = getStringFromBuffer();
    }

    virtual void performAction(Player *player) override {
        Player *tempPlayer = World::get()->Players.find(name);

        if (!tempPlayer) {
            for (const auto &p : World::get()->Players) {
                if (p->getId() == id) {
                    tempPlayer = p;
                }
            }
        }

        if (tempPlayer) {
            ServerCommandPointer cmd(new BBSendAttribTC(tempPlayer->getId(), "sex", tempPlayer->increaseAttrib("sex",0)));
            player->Connection->addCommand(cmd);
            cmd.reset(new BBSendAttribTC(tempPlayer->getId(), "age", tempPlayer->increaseAttrib("age",0)));
            player->Connection->addCommand(cmd);
            cmd.reset(new BBSendAttribTC(tempPlayer->getId(), "weight", tempPlayer->increaseAttrib("weight",0)));
            player->Connection->addCommand(cmd);
            cmd.reset(new BBSendAttribTC(tempPlayer->getId(), "body_height", tempPlayer->increaseAttrib("body_height",0)));
            player->Connection->addCommand(cmd);
            cmd.reset(new BBSendAttribTC(tempPlayer->getId(), "attitude", tempPlayer->increaseAttrib("attitude",0)));
            player->Connection->addCommand(cmd);
            cmd.reset(new BBSendAttribTC(tempPlayer->getId(), "luck", tempPlayer->increaseAttrib("luck",0)));
            player->Connection->addCommand(cmd);
            cmd.reset(new BBSendAttribTC(tempPlayer->getId(), "strength", tempPlayer->increaseAttrib("strength",0)));
            player->Connection->addCommand(cmd);
            cmd.reset(new BBSendAttribTC(tempPlayer->getId(), "dexterity", tempPlayer->increaseAttrib("dexterity",0)));
            player->Connection->addCommand(cmd);
            cmd.reset(new BBSendAttribTC(tempPlayer->getId(), "constitution", tempPlayer->increaseAttrib("constitution",0)));
            player->Connection->addCommand(cmd);
            cmd.reset(new BBSendAttribTC(tempPlayer->getId(), "intelligence", tempPlayer->increaseAttrib("intelligence",0)));
            player->Connection->addCommand(cmd);
            cmd.reset(new BBSendAttribTC(tempPlayer->getId(), "perception", tempPlayer->increaseAttrib("perception",0)));
            player->Connection->addCommand(cmd);
            cmd.reset(new BBSendAttribTC(tempPlayer->getId(), "age", tempPlayer->increaseAttrib("age",0)));
            player->Connection->addCommand(cmd);
            cmd.reset(new BBSendAttribTC(tempPlayer->getId(), "willpower", tempPlayer->increaseAttrib("willpower",0)));
            player->Connection->addCommand(cmd);
            cmd.reset(new BBSendAttribTC(tempPlayer->getId(), "essence", tempPlayer->increaseAttrib("essence",0)));
            player->Connection->addCommand(cmd);
            cmd.reset(new BBSendAttribTC(tempPlayer->getId(), "agility", tempPlayer->increaseAttrib("agility",0)));
            player->Connection->addCommand(cmd);

        }
    }

    virtual ClientCommandPointer clone() override {
        ClientCommandPointer cmd(new BBRequestStatTS());
        return cmd;
    }

    TYPE_OF_CHARACTER_ID id;
    std::string name;
};


/**
* @ingroup BBIWIClientcommands
* client requests all skills of another player
*/
class BBRequestSkillsTS : public BasicClientCommand {
public:
    BBRequestSkillsTS() : BasicClientCommand(BB_REQUESTSKILLS_TS) {
    }

    virtual ~BBRequestSkillsTS() {};

    virtual void decodeData() override {
        id = getIntFromBuffer();
        name = getStringFromBuffer();
    }

    virtual void performAction(Player *player) override {
        Player *tempPlayer = World::get()->Players.find(name);

        if (!tempPlayer) {
            for (const auto &p : World::get()->Players) {
                if (p->getId() == id) {
                    tempPlayer = p;
                }
            }
        }

        if (tempPlayer) {
            Character::SKILLMAP::const_iterator sIterator;

            for (const auto &skill : tempPlayer->skills) {
                ServerCommandPointer cmd(new BBSendSkillTC(tempPlayer->getId(), skill.first, skill.second.major, skill.second.minor));
                player->Connection->addCommand(cmd);
            }
        }
    }

    virtual ClientCommandPointer clone() override {
        ClientCommandPointer cmd(new BBRequestSkillsTS());
        return cmd;
    }

    TYPE_OF_CHARACTER_ID id;
    std::string name;
};


/**
* @ingroup BBIWIClientcommands
* client wants to speak as a player
*/
class BBSpeakAsTS : public BasicClientCommand {
public:
    BBSpeakAsTS() : BasicClientCommand(BB_SPEAKAS_TS) {
    }

    virtual ~BBSpeakAsTS() {};

    virtual void decodeData() override {
        id = getIntFromBuffer();
        name = getStringFromBuffer();
        message = getStringFromBuffer();
    }

    virtual void performAction(Player *player) override {
        Player *tempPlayer = World::get()->Players.find(name);

        if (!tempPlayer) {
            for (const auto &p : World::get()->Players) {
                if (p->getId() == id) {
                    tempPlayer = p;
                }
            }
        }

        if (tempPlayer) {
            tempPlayer->talk(Character::tt_say,message);
            Logger::info(LogFacility::Admin) << player->to_string() << " talked as other player: " << tempPlayer->to_string() << " with message: " << message << Log::end;
        }
    }

    virtual ClientCommandPointer clone() override {
        ClientCommandPointer cmd(new BBSpeakAsTS());
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
class BBWarpPlayerTS : public BasicClientCommand {
public:
    BBWarpPlayerTS() : BasicClientCommand(BB_WARPPLAYER_TS) {
    }

    virtual ~BBWarpPlayerTS() {};

    virtual void decodeData() override {
        id = getIntFromBuffer();
        name = getStringFromBuffer();
        posx = getIntFromBuffer();
        posy = getIntFromBuffer();
        posz = getIntFromBuffer();
    }

    virtual void performAction(Player *player) override {
        Player *tempPlayer = World::get()->Players.find(name);

        if (!tempPlayer) {
            for (const auto &p : World::get()->Players) {
                if (p->getId() == id) {
                    tempPlayer = p;
                }
            }
        }

        if (tempPlayer) {
            tempPlayer->Warp(position(posx,posy,posz));
        }
    }

    virtual ClientCommandPointer clone() override {
        ClientCommandPointer cmd(new BBWarpPlayerTS());
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
class BBServerCommandTS : public BasicClientCommand {
public:
    BBServerCommandTS() : BasicClientCommand(BB_SERVERCOMMAND_TS) {
    }

    virtual ~BBServerCommandTS() {};

    virtual void decodeData() override {
        _command = getStringFromBuffer();
    }

    virtual void performAction(Player *player) override {
        if (_command == "nuke") {
            World::get()->montool_kill_command(player);
        }

        if (_command == "reload") {
            World::get()->montool_reload_command(player);
        }

        if (_command == "kickall") {
            World::get()->montool_kickall_command(player);
        }

        if (_command == "setloginfalse") {
            World::get()->montool_set_login(player,"false");
        }

        if (_command == "setlogintrue") {
            World::get()->montool_set_login(player,"true");
        }
    }

    virtual ClientCommandPointer clone() override {
        ClientCommandPointer cmd(new BBServerCommandTS());
        return cmd;
    }

    std::string _command;

};

/**
* @ingroup BBIWIClientcommands
* client wants to change skill of someone
*/
class BBChangeAttribTS : public BasicClientCommand {
public:
    BBChangeAttribTS() : BasicClientCommand(BB_CHANGEATTRIB_TS), _value(0) {
    }

    virtual ~BBChangeAttribTS() {};

    virtual void decodeData() override {
        _plid = getIntFromBuffer();
        _plname = getStringFromBuffer();
        _attr = getStringFromBuffer();
        _value = getShortIntFromBuffer();
        std::cout<<"received ChangeAttrib: "<<_plid<<" "<<_plname<<" "<<_attr<<" "<<_value<<std::endl;
    }

    virtual void performAction(Player *player) override {
        Player *tempPlayer = World::get()->Players.find(_plname);

        if (!tempPlayer) {
            for (const auto &p : World::get()->Players) {
                if (p->getId() == _plid) {
                    tempPlayer = p;
                }
            }
        }

        if (tempPlayer) {
            tempPlayer->increaseAttrib(_attr,_value);
        }
    }

    virtual ClientCommandPointer clone() override {
        ClientCommandPointer cmd(new BBChangeAttribTS());
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
class BBChangeSkillTS : public BasicClientCommand {
public:
    BBChangeSkillTS() : BasicClientCommand(BB_CHANGEATTRIB_TS), _value(0) {
    }

    virtual ~BBChangeSkillTS() {};

    virtual void decodeData() override {
        _plid = getIntFromBuffer();
        _plname = getStringFromBuffer();
        _skill = getUnsignedCharFromBuffer();
        _value = getShortIntFromBuffer();
        std::cout<<"received ChangeSkill: "<<_plid<<" "<<_plname<<" "<<_skill<<" "<<_value<<std::endl;
    }

    virtual void performAction(Player *player) override {
        Player *tempPlayer = World::get()->Players.find(_plname);

        if (!tempPlayer) {
            for (const auto &p : World::get()->Players) {
                if (p->getId() == _plid) {
                    tempPlayer = p;
                }
            }
        }

        if (tempPlayer) {
            tempPlayer->increaseSkill(_skill, _value);
        }
    }

    virtual ClientCommandPointer clone() override {
        ClientCommandPointer cmd(new BBChangeSkillTS());
        return cmd;
    }

    TYPE_OF_CHARACTER_ID _plid;
    std::string _plname;
    TYPE_OF_SKILL_ID _skill;
    short int _value;

};

/**
* @ingroup BBIWIClientcommands
* client talks to someone
*/
class BBTalktoTS : public BasicClientCommand {
public:
    BBTalktoTS() : BasicClientCommand(BB_TALKTO_TS) {
    }

    virtual ~BBTalktoTS() {};

    virtual void decodeData() override {
        playerid = getIntFromBuffer();
        playername = getStringFromBuffer();
        msg = getStringFromBuffer();
    }

    virtual void performAction(Player *player) override {
        Player *tempPlayer = World::get()->Players.find(playername);

        if (!tempPlayer) {
            for (const auto &p : World::get()->Players) {
                if (p->getId() == playerid) {
                    tempPlayer = p;
                }
            }
        }

        if (tempPlayer) {
            tempPlayer->inform(msg, Player::informGM);
        }
    }

    virtual ClientCommandPointer clone() override {
        ClientCommandPointer cmd(new BBTalktoTS());
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
class BBDisconnectTS : public BasicClientCommand {
public:
    BBDisconnectTS() : BasicClientCommand(BB_DISCONNECT_TS) {
    }

    virtual ~BBDisconnectTS() {};

    virtual void decodeData() override {

    }

    virtual void performAction(Player *player) override {
        //we want to disconnect so we close the connection
        player->Connection->closeConnection();

    }

    virtual ClientCommandPointer clone() override {
        ClientCommandPointer cmd(new BBDisconnectTS());
        return cmd;
    }

};


/**
* @ingroup BBIWIClientcommands
* character received a keepalive
*/
class BBKeepAliveTS : public BasicClientCommand {
public:
    BBKeepAliveTS() : BasicClientCommand(BB_KEEPALIVE_TS) {

    }

    virtual ~BBKeepAliveTS() {};

    virtual void decodeData() override {

    }

    virtual void performAction(Player *player) override {
        time(&(player->lastkeepalive));
    }

    virtual ClientCommandPointer clone() override {
        ClientCommandPointer cmd(new BBKeepAliveTS());
        return cmd;
    }

};

/**
* @ingroup BBIWIClientcommands
* character is banned for an amount of time
*/
class BBBanTS : public BasicClientCommand {
public:
    BBBanTS() : BasicClientCommand(BB_BAN_TS) {
    }

    virtual ~BBBanTS() {};

    virtual void decodeData() override {
        id = getIntFromBuffer();
        name = getStringFromBuffer();
        time = getIntFromBuffer();
    }

    virtual void performAction(Player *player) override {
        Player *tempPlayer = World::get()->Players.find(name);

        if (!tempPlayer) {
            for (const auto &p : World::get()->Players) {
                if (p->getId() == id) {
                    tempPlayer = p;
                }
            }
        }

        if (tempPlayer) {
            World::get()->ban(tempPlayer, time, player->getId());
            ServerCommandPointer cmd(new BBMessageTC(tempPlayer->to_string() + " banned by: " + player->to_string(), 0));
            World::get()->monitoringClientList->sendCommand(cmd);
        } else {
            ServerCommandPointer cmd(new BBMessageTC("Cannot find the player: " + name + "(" + std::to_string(id) + ")",0));
            player->Connection->addCommand(cmd);
        }
    }

    virtual ClientCommandPointer clone() override {
        ClientCommandPointer cmd(new BBBanTS());
        return cmd;
    }
    TYPE_OF_CHARACTER_ID id; /*<which character is banned*/
    std::string name; /*<which character is banned*/
    uint32_t time; /*<how long is he banned*/
};

#endif
