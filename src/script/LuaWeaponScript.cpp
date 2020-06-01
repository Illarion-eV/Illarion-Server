/*
 *  illarionserver - server for the game Illarion
 *  Copyright 2011 Illarion e.V.
 *
 *  This file is part of illarionserver.
 *
 *  illarionserver is free software: you can redistribute it and/or modify
 *  it under the terms of the GNU Affero General Public License as published by
 *  the Free Software Foundation, either version 3 of the License, or
 *  (at your option) any later version.
 *
 *  illarionserver is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU Affero General Public License for more details.
 *
 *  You should have received a copy of the GNU Affero General Public License
 *  along with illarionserver.  If not, see <http://www.gnu.org/licenses/>.
 */

#include "LuaWeaponScript.hpp"

#include "Character.hpp"
#include "character_ptr.hpp"

LuaWeaponScript::LuaWeaponScript(const std::string &filename) : LuaScript(filename) {}

LuaWeaponScript::LuaWeaponScript(const std::string &filename, const WeaponStruct &weapon) : LuaScript(filename) {}

LuaWeaponScript::~LuaWeaponScript() = default;

void LuaWeaponScript::onAttack(Character *Attacker, Character *Defender) {
    character_ptr fuse_Attacker(Attacker);
    character_ptr fuse_Defender(Defender);
    callEntrypoint("onAttack", fuse_Attacker, fuse_Defender);
}

auto LuaWeaponScript::setTarget(Character *Monster, const std::vector<Character *> &CandidateList) -> Character * {
    luabind::object luaCandidateList = luabind::newtable(_luaState);
    int index = 1;

    for (const auto &candidate : CandidateList) {
        character_ptr fuse_it(candidate);
        luaCandidateList[index++] = fuse_it;
    }

    character_ptr fuse_Monster(Monster);
    index = callEntrypoint<int>("setTarget", fuse_Monster, luaCandidateList) - 1;

    if (index >= 0 && index < (int)CandidateList.size()) {
        return CandidateList[index];
    }

    return nullptr;
}
