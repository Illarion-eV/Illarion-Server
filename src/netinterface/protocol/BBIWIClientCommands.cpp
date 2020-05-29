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

#include "Player.hpp"
#include "World.hpp"
#include "Logger.hpp"
#include "MonitoringClients.hpp"
#include "netinterface/protocol/BBIWIServerCommands.hpp"

BBBroadCastTS::BBBroadCastTS() : BasicClientCommand(BB_BROADCAST_TS) {
}

BBBroadCastTS::~BBBroadCastTS() = default;

void BBBroadCastTS::decodeData() {
    msg = getStringFromBuffer();
}

void BBBroadCastTS::performAction(Player *player) {
    World::get()->broadcast_command(player, msg);
}

ClientCommandPointer BBBroadCastTS::clone() {
    ClientCommandPointer cmd = std::make_shared<BBBroadCastTS>();
    return cmd;
}

BBSpeakAsTS::BBSpeakAsTS() : BasicClientCommand(BB_SPEAKAS_TS) {
}

BBSpeakAsTS::~BBSpeakAsTS() = default;

void BBSpeakAsTS::decodeData() {
    id = getIntFromBuffer();
    message = getStringFromBuffer();
}

void BBSpeakAsTS::performAction(Player *player) {
    Player *tempPlayer = World::get()->Players.find(id);
    
    if (tempPlayer) {
        tempPlayer->talk(Character::tt_say, message);
        Logger::info(LogFacility::Admin) << *player << " talks as " << *tempPlayer << ": " << message << Log::end;
    }
}

ClientCommandPointer BBSpeakAsTS::clone() {
    ClientCommandPointer cmd = std::make_shared<BBSpeakAsTS>();
    return cmd;
}

BBWarpPlayerTS::BBWarpPlayerTS() : BasicClientCommand(BB_WARPPLAYER_TS) {
}

BBWarpPlayerTS::~BBWarpPlayerTS() = default;

void BBWarpPlayerTS::decodeData() {
    id = getIntFromBuffer();
    posx = getShortIntFromBuffer();
    posy = getShortIntFromBuffer();
    posz = getShortIntFromBuffer();
}

void BBWarpPlayerTS::performAction(Player *player) {
    Player *tempPlayer = World::get()->Players.find(id);

    if (tempPlayer) {
        tempPlayer->Warp(position(posx, posy, posz));
    }
}

ClientCommandPointer BBWarpPlayerTS::clone() {
    ClientCommandPointer cmd = std::make_shared<BBWarpPlayerTS>();
    return cmd;
}

BBServerCommandTS::BBServerCommandTS() : BasicClientCommand(BB_SERVERCOMMAND_TS) {
}

BBServerCommandTS::~BBServerCommandTS() = default;

void BBServerCommandTS::decodeData() {
    _command = getStringFromBuffer();
}

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

ClientCommandPointer BBServerCommandTS::clone() {
    ClientCommandPointer cmd = std::make_shared<BBServerCommandTS>();
    return cmd;
}

BBChangeAttribTS::BBChangeAttribTS() : BasicClientCommand(BB_CHANGEATTRIB_TS) {
}

BBChangeAttribTS::~BBChangeAttribTS() = default;

void BBChangeAttribTS::decodeData() {
    id = getIntFromBuffer();
    attrib = getStringFromBuffer();
    value = getShortIntFromBuffer();
}

void BBChangeAttribTS::performAction(Player *player) {
    Player *tempPlayer = World::get()->Players.find(id);
    
    if (tempPlayer) {
        auto oldValue = tempPlayer->increaseAttrib(attrib, 0);
        auto newValue = tempPlayer->increaseAttrib(attrib, value);

        Logger::info(LogFacility::Admin) << *player << " increases attribute " << attrib << " for " << *tempPlayer << " from " << oldValue << " to " << newValue << Log::end;
    }
}

ClientCommandPointer BBChangeAttribTS::clone() {
    ClientCommandPointer cmd = std::make_shared<BBChangeAttribTS>();
    return cmd;
}

BBChangeSkillTS::BBChangeSkillTS() : BasicClientCommand(BB_CHANGEATTRIB_TS) {
}

BBChangeSkillTS::~BBChangeSkillTS() = default;

void BBChangeSkillTS::decodeData() {
    id = getIntFromBuffer();
    skill = getUnsignedCharFromBuffer();
    value = getShortIntFromBuffer();
}

void BBChangeSkillTS::performAction(Player *player) {
    Player *tempPlayer = World::get()->Players.find(id);

    if (tempPlayer) {
        auto oldValue = tempPlayer->getSkill(skill);
        auto newValue = tempPlayer->increaseSkill(skill, value);
        Logger::info(LogFacility::Admin) << *player << " increases skill " << player->getSkillName(skill) << " for " << *tempPlayer << " from " << oldValue << " to " << newValue << Log::end;
    }
}

ClientCommandPointer BBChangeSkillTS::clone() {
    ClientCommandPointer cmd = std::make_shared<BBChangeSkillTS>();
    return cmd;
}

BBTalktoTS::BBTalktoTS() : BasicClientCommand(BB_TALKTO_TS) {
}

BBTalktoTS::~BBTalktoTS() = default;

void BBTalktoTS::decodeData() {
    id = getIntFromBuffer();
    msg = getStringFromBuffer();
}

void BBTalktoTS::performAction(Player *player) {
    World::get()->talkto_command(player, std::to_string(id) + "," + msg);
}

ClientCommandPointer BBTalktoTS::clone() {
    ClientCommandPointer cmd = std::make_shared<BBTalktoTS>();
    return cmd;
}

BBDisconnectTS::BBDisconnectTS() : BasicClientCommand(BB_DISCONNECT_TS) {
}

BBDisconnectTS::~BBDisconnectTS() = default;

void BBDisconnectTS::decodeData() {
}

void BBDisconnectTS::performAction(Player *player) {
    player->Connection->closeConnection();
}

ClientCommandPointer BBDisconnectTS::clone() {
    ClientCommandPointer cmd = std::make_shared<BBDisconnectTS>();
    return cmd;
}

BBKeepAliveTS::BBKeepAliveTS() : BasicClientCommand(BB_KEEPALIVE_TS) {
}

BBKeepAliveTS::~BBKeepAliveTS() = default;

void BBKeepAliveTS::decodeData() {
}

void BBKeepAliveTS::performAction(Player *player) {
    time(&(player->lastkeepalive));
}

ClientCommandPointer BBKeepAliveTS::clone() {
    ClientCommandPointer cmd = std::make_shared<BBKeepAliveTS>();
    return cmd;
}

BBBanTS::BBBanTS() : BasicClientCommand(BB_BAN_TS) {
}

BBBanTS::~BBBanTS() = default;

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

ClientCommandPointer BBBanTS::clone() {
    ClientCommandPointer cmd = std::make_shared<BBBanTS>();
    return cmd;
}

