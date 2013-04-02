/*
 *  illarionserver - server for the game Illarion
 *  Copyright 2011 Illarion e.V.
 *
 *  This file is part of illarionserver.
 *
 *  illarionserver is free software: you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation, either version 3 of the License, or
 *  (at your option) any later version.
 *
 *  illarionserver is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with illarionserver.  If not, see <http://www.gnu.org/licenses/>.
 */

#include "LuaMonsterScript.hpp"
#include "Monster.hpp"
#include "World.hpp"
#include "Character.hpp"
#include "Player.hpp"
#include "fuse_ptr.hpp"

LuaMonsterScript::LuaMonsterScript() throw(ScriptException)
    : LuaScript() {
}

LuaMonsterScript::LuaMonsterScript(const std::string &filename) throw(ScriptException)
    : LuaScript(filename) {
}

LuaMonsterScript::~LuaMonsterScript() throw() {}

void LuaMonsterScript::onDeath(Character *Monster) {
    fuse_ptr<Character> fuse_Monster(Monster);
    callEntrypoint("onDeath", fuse_Monster);
}

void LuaMonsterScript::lookAtMonster(Character *source, Character *target, unsigned char mode) {
    fuse_ptr<Character> fuse_source(source);
    fuse_ptr<Character> fuse_target(target);
    callEntrypoint("lookAtMonster", fuse_source, fuse_target, mode);
}

void LuaMonsterScript::receiveText(Character *Monster, Character::talk_type tt, const std::string &message, Character *cc) {
    fuse_ptr<Character> fuse_Monster(Monster);
    fuse_ptr<Character> fuse_cc(cc);
    callEntrypoint("receiveText", fuse_Monster, (int)tt, message, fuse_cc);
}

void LuaMonsterScript::onAttacked(Character *Monster, Character *attacker) {
    fuse_ptr<Character> fuse_Monster(Monster);
    fuse_ptr<Character> fuse_attacker(attacker);
    callEntrypoint("onAttacked", fuse_Monster, fuse_attacker);
}

void LuaMonsterScript::onCasted(Character *Monster, Character *caster) {
    fuse_ptr<Character> fuse_Monster(Monster);
    fuse_ptr<Character> fuse_caster(caster);
    callEntrypoint("onCasted", fuse_Monster, fuse_caster);
}

void LuaMonsterScript::useMonster(Character *Monster, Character *user, unsigned char ltastate) {
    fuse_ptr<Character> fuse_Monster(Monster);
    fuse_ptr<Character> fuse_user(user);
    callEntrypoint("useMonster", fuse_Monster, fuse_user, ltastate);
}

bool LuaMonsterScript::actionDisturbed(Character *performer, Character *disturber) {
    fuse_ptr<Character> fuse_performer(performer);
    fuse_ptr<Character> fuse_disturber(disturber);
    return callEntrypoint<bool>("actionDisturbed", fuse_performer, fuse_disturber);
}

void LuaMonsterScript::onAttack(Character *Monster, Character *target) {
    fuse_ptr<Character> fuse_Monster(Monster);
    fuse_ptr<Character> fuse_target(target);
    callEntrypoint("onAttack", fuse_Monster, fuse_target);
}

bool LuaMonsterScript::enemyOnSight(Character *Monster, Character *enemy) {
    fuse_ptr<Character> fuse_Monster(Monster);
    fuse_ptr<Character> fuse_enemy(enemy);
    return callEntrypoint<bool>("enemyOnSight", fuse_Monster, fuse_enemy);
}

bool LuaMonsterScript::enemyNear(Character *Monster, Character *enemy) {
    fuse_ptr<Character> fuse_Monster(Monster);
    fuse_ptr<Character> fuse_enemy(enemy);
    return callEntrypoint<bool>("enemyNear", fuse_Monster, fuse_enemy);
}

void LuaMonsterScript::abortRoute(Character *Monster) {
    fuse_ptr<Character> fuse_Monster(Monster);
    callEntrypoint("abortRoute", fuse_Monster);
}

void LuaMonsterScript::onSpawn(Character *Monster) {
    fuse_ptr<Character> fuse_Monster(Monster);
    callEntrypoint("onSpawn", fuse_Monster);
}

bool LuaMonsterScript::setTarget(Character *Monster, const std::vector<Player *> &CandidateList, Player *&Target) {
    Target = 0;

    if (!existsEntrypoint("setTarget")) {
        return false;
    }

    luabind::object luaCandidateList = luabind::newtable(_luaState);
    int index = 1;

    for (const auto &player : CandidateList) {
        fuse_ptr<Character> fuse_it(player);
        luaCandidateList[index++] = fuse_it;
    }

    fuse_ptr<Character> fuse_Monster(Monster);
    index = callEntrypoint<int>("setTarget", fuse_Monster, luaCandidateList) - 1;

    if (index >= 0 && index < (int)CandidateList.size()) {
        Target = CandidateList[index];
    }

    return true;
}

