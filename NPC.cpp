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


#include "db/ConnectionManager.hpp"
#include "NPC.hpp"
#include "tuningConstants.hpp"
#include "data/CommonObjectTable.hpp"
#include <iostream>
#include "World.hpp"

NPC::NPC(TYPE_OF_CHARACTER_ID _id, std::string _name, Character::race_type _race, position _pos, Character::face_to dir, bool ishealer, Character::sex_type sex,
         uint8_t _hair, uint8_t _beard, uint8_t _hairred, uint8_t _hairgreen, uint8_t _hairblue, uint8_t _skinred, uint8_t _skingreen, uint8_t _skinblue) : Character(),
    _ishealer(ishealer), _startpos(_pos) {
#ifdef Character_DEBUG
    cout << "NPC Konstruktor Start" << endl;
#endif
    name = _name;
    pos=_pos;
    faceto=dir;
    race=_race;
    setAttribute(Character::sex, sex);
    character = npc;
    actionPoints = NP_MAX_AP;
    hair = _hair;
    beard = _beard;
    hairred = _hairred;
    hairgreen = _hairgreen;
    hairblue = _hairblue;
    skinred = _skinred;
    skingreen = _skingreen;
    skinblue = _skinblue;

    // take database id as npc id with appropriate offset so that npc ids are constant
    if (_id == DYNNPC_BASE) {
        id = DYNNPC_BASE + _world->Npc.size();
    } else {
        id = NPC_BASE + _id;    // _world->npcidc->nextFreeId();
    }

    Field *tmpField;

    if (!_world->findEmptyCFieldNear(tmpField,pos.x, pos.y, pos.z)) {
        throw NoSpace();
    }

    _startpos = pos;
    std::cout << "NewNPC spwaned: pos: " << pos.x << " " << pos.y << " " << pos.z << " type: " << _race << " Name: " << _name<< " is_healer: " << _ishealer << " sex: " << getAttribute(Character::sex) << std::endl;

    tmpField->setChar();

    _world->sendPassiveMoveToAllVisiblePlayers(this);
    _world->sendSpinToAllVisiblePlayers(this);

    setAttribute(Character::hitpoints, MAXHPS);

#ifdef Character_DEBUG
    cout << "NPC Konstruktor Ende" << endl;
#endif
}


NPC::~NPC() {
#ifdef Character_DEBUG
    cout << "NPC Destruktor Start/Ende" << endl;
#endif
}

void NPC::receiveText(talk_type tt, std::string message, Character *cc) {
    if (_script && cc != this && _script->existsEntrypoint("receiveText")) {
        // since we have a script, we tell it we got a message
        _script->receiveText(tt, message, cc);
    }
}
