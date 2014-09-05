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

#include "LuaMonsterScript.hpp"
#include "Monster.hpp"
#include "World.hpp"
#include "Character.hpp"
#include "character_ptr.hpp"

LuaMonsterScript::LuaMonsterScript()
    : LuaScript() {
}

LuaMonsterScript::LuaMonsterScript(const std::string &filename)
    : LuaScript(filename) {
}

LuaMonsterScript::~LuaMonsterScript() {}

void LuaMonsterScript::onDeath(Character *Monster) {
    character_ptr fuse_Monster(Monster);
    callEntrypoint("onDeath", fuse_Monster);
}

void LuaMonsterScript::lookAtMonster(Character *source, Character *target, unsigned char mode) {
    character_ptr fuse_source(source);
    character_ptr fuse_target(target);
    callEntrypoint("lookAtMonster", fuse_source, fuse_target, mode);
}

void LuaMonsterScript::receiveText(Character *Monster, Character::talk_type tt, const std::string &message, Character *cc) {
    character_ptr fuse_Monster(Monster);
    character_ptr fuse_cc(cc);
    callEntrypoint("receiveText", fuse_Monster, (int)tt, message, fuse_cc);
}

void LuaMonsterScript::onAttacked(Character *Monster, Character *attacker) {
    character_ptr fuse_Monster(Monster);
    character_ptr fuse_attacker(attacker);
    callEntrypoint("onAttacked", fuse_Monster, fuse_attacker);
}

void LuaMonsterScript::onCasted(Character *Monster, Character *caster) {
    character_ptr fuse_Monster(Monster);
    character_ptr fuse_caster(caster);
    callEntrypoint("onCasted", fuse_Monster, fuse_caster);
}

void LuaMonsterScript::useMonster(Character *Monster, Character *user, unsigned char ltastate) {
    character_ptr fuse_Monster(Monster);
    character_ptr fuse_user(user);
    callEntrypoint("useMonster", fuse_Monster, fuse_user, ltastate);
}

bool LuaMonsterScript::actionDisturbed(Character *performer, Character *disturber) {
    character_ptr fuse_performer(performer);
    character_ptr fuse_disturber(disturber);
    return callEntrypoint<bool>("actionDisturbed", fuse_performer, fuse_disturber);
}

void LuaMonsterScript::onAttack(Character *Monster, Character *target) {
    character_ptr fuse_Monster(Monster);
    character_ptr fuse_target(target);
    callEntrypoint("onAttack", fuse_Monster, fuse_target);
}

bool LuaMonsterScript::enemyOnSight(Character *Monster, Character *enemy) {
    character_ptr fuse_Monster(Monster);
    character_ptr fuse_enemy(enemy);
    return callEntrypoint<bool>("enemyOnSight", fuse_Monster, fuse_enemy);
}

bool LuaMonsterScript::enemyNear(Character *Monster, Character *enemy) {
    character_ptr fuse_Monster(Monster);
    character_ptr fuse_enemy(enemy);
    return callEntrypoint<bool>("enemyNear", fuse_Monster, fuse_enemy);
}

void LuaMonsterScript::abortRoute(Character *Monster) {
    character_ptr fuse_Monster(Monster);
    callEntrypoint("abortRoute", fuse_Monster);
}

void LuaMonsterScript::onSpawn(Character *Monster) {
    character_ptr fuse_Monster(Monster);
    callEntrypoint("onSpawn", fuse_Monster);
}

bool LuaMonsterScript::setTarget(Character *Monster, const std::vector<Character *> &CandidateList, Character *&Target) {
    Target = nullptr;

    if (!existsEntrypoint("setTarget")) {
        return false;
    }

    luabind::object luaCandidateList = luabind::newtable(_luaState);
    int index = 1;

    for (const auto &candidate : CandidateList) {
        character_ptr fuse_it(candidate);
        luaCandidateList[index++] = fuse_it;
    }

    character_ptr fuse_Monster(Monster);
    index = callEntrypoint<int>("setTarget", fuse_Monster, luaCandidateList) - 1;

    if (index >= 0 && index < (int)CandidateList.size()) {
        Target = CandidateList[index];
    }

    return true;
}

