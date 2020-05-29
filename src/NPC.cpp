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

#include "NPC.hpp"

#include "tuningConstants.hpp"
#include "World.hpp"
#include "map/Field.hpp"

#include "db/ConnectionManager.hpp"

#include "script/LuaNPCScript.hpp"

uint32_t NPC::counter = 0;

NPC::NPC(TYPE_OF_CHARACTER_ID id, const std::string &name, TYPE_OF_RACE_ID race, const position &pos, Character::face_to dir, bool ishealer, Character::sex_type sex,
         const Character::appearance &appearance) : Character(appearance),
    _ishealer(ishealer), _startpos(pos) {
    setName(name);
    setFaceTo(dir);
    setRace(race);
    setAttribute(Character::sex, sex);

    // take database id as npc id with appropriate offset so that npc ids are constant
    if (id == DYNNPC_BASE) {
        setId(DYNNPC_BASE + counter);
        ++counter;
    } else {
        setId(NPC_BASE + id);
    }

    map::Field &field = _world->fieldAt(_startpos);

    setPosition(_startpos);
    Logger::debug(LogFacility::World) << "New NPC spawned: pos: " << _startpos << " type: " << race << " Name: " << name<< " is_healer: " << _ishealer << " sex: " << getAttribute(Character::sex) << Log::end;

    field.setChar();

    _world->sendPassiveMoveToAllVisiblePlayers(this);
    _world->sendSpinToAllVisiblePlayers(this);

    setAttribute(Character::hitpoints, MAXHPS);
}


NPC::~NPC() = default;

void NPC::receiveText(talk_type tt, const std::string &message, Character *cc) {
    if (_script && cc != this && _script->existsEntrypoint("receiveText")) {
        // since we have a script, we tell it we got a message
        _script->receiveText(tt, message, cc);
    }
}

auto NPC::to_string() const -> std::string {
    return "NPC " + getName() + "(" + std::to_string(getId()) + ")";
}
