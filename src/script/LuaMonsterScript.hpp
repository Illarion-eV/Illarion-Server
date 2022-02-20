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

#ifndef LUA_MONSTER_SCRIPT_HPP
#define LUA_MONSTER_SCRIPT_HPP

#include "Character.hpp"
#include "Item.hpp"
#include "LuaScript.hpp"

class World;
enum LtaState : unsigned char;

class LuaMonsterScript : public LuaScript {
public:
    LuaMonsterScript() = default;
    explicit LuaMonsterScript(const std::string &filename);
    LuaMonsterScript(const LuaMonsterScript &) = delete;
    auto operator=(const LuaMonsterScript &) -> LuaMonsterScript & = delete;
    LuaMonsterScript(LuaMonsterScript &&) = default;
    auto operator=(LuaMonsterScript &&) -> LuaMonsterScript & = default;
    ~LuaMonsterScript() override = default;

    void receiveText(Character *Monster, Character::talk_type tt, const std::string &message, Character *cc);
    void onDeath(Character *Monster);
    void onAttacked(Character *Monster, Character *attacker);
    void onCasted(Character *Monster, Character *caster);
    void useMonster(Character *Monster, Character *user, LtaState actionState);
    void lookAtMonster(Character *source, Character *target, unsigned char mode);
    auto actionDisturbed(Character *performer, Character *disturber) -> bool;
    void onAttack(Character *Monster, Character *target);
    auto enemyOnSight(Character *Monster, Character *enemy) -> bool;
    auto enemyNear(Character *Monster, Character *enemy) -> bool;
    void abortRoute(Character *Monster);
    void onSpawn(Character *Monster);
    auto setTarget(Character *Monster, const std::vector<Character *> &CandidateList, Character *&Target) -> bool;
};

#endif
