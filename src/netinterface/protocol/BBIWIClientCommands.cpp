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

#include "netinterface/protocol/BBIWIClientCommands.hpp"

#include "Logger.hpp"
#include "MonitoringClients.hpp"
#include "Player.hpp"
#include "World.hpp"
#include "netinterface/protocol/BBIWIServerCommands.hpp"

BBBroadCastTS::BBBroadCastTS() : BasicClientCommand(BB_BROADCAST_TS) {}

void BBBroadCastTS::decodeData() { msg = getStringFromBuffer(); }

void BBBroadCastTS::performAction(Player *player) { World::get()->broadcast_command(player, msg); }

auto BBBroadCastTS::clone() -> ClientCommandPointer {
    ClientCommandPointer cmd = std::make_shared<BBBroadCastTS>();
    return cmd;
}

BBSpeakAsTS::BBSpeakAsTS() : BasicClientCommand(BB_SPEAKAS_TS) {}

void BBSpeakAsTS::decodeData() {
    id = getIntFromBuffer();
    message = getStringFromBuffer();
}

void BBSpeakAsTS::performAction(Player *player) {
    Player *tempPlayer = World::get()->Players.find(id);

    if (tempPlayer != nullptr) {
        tempPlayer->talk(Character::tt_say, message);
        Logger::info(LogFacility::Admin) << *player << " talks as " << *tempPlayer << ": " << message << Log::end;
    }
}

auto BBSpeakAsTS::clone() -> ClientCommandPointer {
    ClientCommandPointer cmd = std::make_shared<BBSpeakAsTS>();
    return cmd;
}

BBWarpPlayerTS::BBWarpPlayerTS() : BasicClientCommand(BB_WARPPLAYER_TS) {}

void BBWarpPlayerTS::decodeData() {
    id = getIntFromBuffer();
    posx = getShortIntFromBuffer();
    posy = getShortIntFromBuffer();
    posz = getShortIntFromBuffer();
}

void BBWarpPlayerTS::performAction(Player *player) {
    Player *tempPlayer = World::get()->Players.find(id);

    if (tempPlayer != nullptr) {
        tempPlayer->Warp(position(posx, posy, posz));
    }
}

auto BBWarpPlayerTS::clone() -> ClientCommandPointer {
    ClientCommandPointer cmd = std::make_shared<BBWarpPlayerTS>();
    return cmd;
}

BBServerCommandTS::BBServerCommandTS() : BasicClientCommand(BB_SERVERCOMMAND_TS) {}

void BBServerCommandTS::decodeData() { _command = getStringFromBuffer(); }

void BBServerCommandTS::performAction(Player *player) {
    if (_command == "nuke") {
        World::get()->kill_command(player);
    }

    if (_command == "reload") {
        World::get()->reload_command(player);
    }

    if (_command == "kickall") {
        World::get()->kickall_command(player);
    }

    if (_command == "setloginfalse") {
        World::get()->set_login(player, "false");
    }

    if (_command == "setlogintrue") {
        World::get()->set_login(player, "true");
    }
}

auto BBServerCommandTS::clone() -> ClientCommandPointer {
    ClientCommandPointer cmd = std::make_shared<BBServerCommandTS>();
    return cmd;
}

BBChangeAttribTS::BBChangeAttribTS() : BasicClientCommand(BB_CHANGEATTRIB_TS) {}

void BBChangeAttribTS::decodeData() {
    id = getIntFromBuffer();
    attrib = getStringFromBuffer();
    value = getShortIntFromBuffer();
}

void BBChangeAttribTS::performAction(Player *player) {
    Player *tempPlayer = World::get()->Players.find(id);

    if (tempPlayer != nullptr) {
        auto oldValue = tempPlayer->increaseAttrib(attrib, 0);
        auto newValue = tempPlayer->increaseAttrib(attrib, value);

        Logger::info(LogFacility::Admin) << *player << " increases attribute " << attrib << " for " << *tempPlayer
                                         << " from " << oldValue << " to " << newValue << Log::end;
    }
}

auto BBChangeAttribTS::clone() -> ClientCommandPointer {
    ClientCommandPointer cmd = std::make_shared<BBChangeAttribTS>();
    return cmd;
}

BBChangeSkillTS::BBChangeSkillTS() : BasicClientCommand(BB_CHANGEATTRIB_TS) {}

void BBChangeSkillTS::decodeData() {
    id = getIntFromBuffer();
    skill = getUnsignedCharFromBuffer();
    value = getShortIntFromBuffer();
}

void BBChangeSkillTS::performAction(Player *player) {
    Player *tempPlayer = World::get()->Players.find(id);

    if (tempPlayer != nullptr) {
        auto oldValue = tempPlayer->getSkill(skill);
        auto newValue = tempPlayer->increaseSkill(skill, value);
        Logger::info(LogFacility::Admin) << *player << " increases skill " << player->getSkillName(skill) << " for "
                                         << *tempPlayer << " from " << oldValue << " to " << newValue << Log::end;
    }
}

auto BBChangeSkillTS::clone() -> ClientCommandPointer {
    ClientCommandPointer cmd = std::make_shared<BBChangeSkillTS>();
    return cmd;
}

BBTalktoTS::BBTalktoTS() : BasicClientCommand(BB_TALKTO_TS) {}

void BBTalktoTS::decodeData() {
    id = getIntFromBuffer();
    msg = getStringFromBuffer();
}

void BBTalktoTS::performAction(Player *player) { World::get()->talkto_command(player, std::to_string(id) + "," + msg); }

auto BBTalktoTS::clone() -> ClientCommandPointer {
    ClientCommandPointer cmd = std::make_shared<BBTalktoTS>();
    return cmd;
}

BBDisconnectTS::BBDisconnectTS() : BasicClientCommand(BB_DISCONNECT_TS) {}

void BBDisconnectTS::decodeData() {}

void BBDisconnectTS::performAction(Player *player) { player->Connection->closeConnection(); }

auto BBDisconnectTS::clone() -> ClientCommandPointer {
    ClientCommandPointer cmd = std::make_shared<BBDisconnectTS>();
    return cmd;
}

BBKeepAliveTS::BBKeepAliveTS() : BasicClientCommand(BB_KEEPALIVE_TS) {}

void BBKeepAliveTS::decodeData() {}

void BBKeepAliveTS::performAction(Player *player) { time(&(player->lastkeepalive)); }

auto BBKeepAliveTS::clone() -> ClientCommandPointer {
    ClientCommandPointer cmd = std::make_shared<BBKeepAliveTS>();
    return cmd;
}

BBBanTS::BBBanTS() : BasicClientCommand(BB_BAN_TS) {}

void BBBanTS::decodeData() {
    id = getIntFromBuffer();
    time = getIntFromBuffer();
}

void BBBanTS::performAction(Player *player) {
    std::string banString;

    if (time == 0) {
        banString = std::to_string(id);
    } else {
        banString = std::to_string(time) + " m " + std::to_string(id);
    }

    World::get()->ban_command(player, banString);
}

auto BBBanTS::clone() -> ClientCommandPointer {
    ClientCommandPointer cmd = std::make_shared<BBBanTS>();
    return cmd;
}
