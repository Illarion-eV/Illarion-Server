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

#include "netinterface/protocol/BBIWIServerCommands.hpp"

BBLoginSuccessfulTC::BBLoginSuccessfulTC() : BasicServerCommand(BB_LOGINSUCCESSFUL_TC) {}

BBSendActionTC::BBSendActionTC(TYPE_OF_CHARACTER_ID id, uint8_t type, const std::string &desc)
        : BasicServerCommand(BB_SENDACTION_TC) {
    addIntToBuffer(id);
    addUnsignedCharToBuffer(type);
    addStringToBuffer(desc);
}

BBSendSkillTC::BBSendSkillTC(TYPE_OF_CHARACTER_ID id, TYPE_OF_SKILL_ID skill, short int value, short int minor)
        : BasicServerCommand(BB_SENDSKILL_TC) {
    addIntToBuffer(id);
    addUnsignedCharToBuffer(skill);
    addShortIntToBuffer(value);
    addShortIntToBuffer(minor);
}

BBSendAttribTC::BBSendAttribTC(TYPE_OF_CHARACTER_ID id, const std::string &attribute, short int value)
        : BasicServerCommand(BB_SENDATTRIB_TC) {
    addIntToBuffer(id);
    addStringToBuffer(attribute);
    addShortIntToBuffer(value);
}

BBLogOutTC::BBLogOutTC(TYPE_OF_CHARACTER_ID id) : BasicServerCommand(BB_LOGOUT_TC) {
    addIntToBuffer(id);
}

BBTalkTC::BBTalkTC(TYPE_OF_CHARACTER_ID id, uint8_t tt, const std::string &msg) : BasicServerCommand(BB_TALK_TC) {
    addIntToBuffer(id);
    addUnsignedCharToBuffer(tt);
    addStringToBuffer(msg);
}

BBPlayerTC::BBPlayerTC(TYPE_OF_CHARACTER_ID id, const std::string &name, const position &pos)
        : BasicServerCommand(BB_PLAYER_TC) {
    addIntToBuffer(id);
    addStringToBuffer(name);
    addShortIntToBuffer(pos.x);
    addShortIntToBuffer(pos.y);
    addShortIntToBuffer(pos.z);
}

BBPlayerMoveTC::BBPlayerMoveTC(TYPE_OF_CHARACTER_ID id, const position &pos) : BasicServerCommand(BB_PLAYERMOVE_TC) {
    addIntToBuffer(id);
    addShortIntToBuffer(pos.x);
    addShortIntToBuffer(pos.y);
    addShortIntToBuffer(pos.z);
}

BBMessageTC::BBMessageTC(const std::string &msg, uint8_t type) : BasicServerCommand(BB_MESSAGE_TC) {
    addStringToBuffer(msg);
    addUnsignedCharToBuffer(type);
}
