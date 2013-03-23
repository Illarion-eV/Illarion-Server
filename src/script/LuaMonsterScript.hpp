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

#ifndef _LUA_MONSTER_SCRIPT_HPP_
#define _LUA_MONSTER_SCRIPT_HPP_

#include "LuaScript.hpp"
#include "Item.hpp"
#include "Character.hpp"

class Player;
class World;

class LuaMonsterScript : public LuaScript {
public:
    LuaMonsterScript() throw(ScriptException);
    LuaMonsterScript(const std::string &filename) throw(ScriptException);

    virtual ~LuaMonsterScript() throw();

    void receiveText(Character *Monster, Character::talk_type tt, const std::string &message, Character *cc);
    void onDeath(Character *Monster);
    void onAttacked(Character *Monster, Character *attacker);
    void onCasted(Character *Monster, Character *caster);
    void useMonster(Character *Monster, Character *user, unsigned char ltastate);
    void lookAtMonster(Character *source, Character *target, unsigned char mode);
    bool actionDisturbed(Character *performer, Character *disturber);
    void onAttack(Character *Monster, Character *target);
    bool enemyOnSight(Character *Monster, Character *enemy);
    bool enemyNear(Character *Monster, Character *enemy);
    void abortRoute(Character *Monster);
    void onSpawn(Character *Monster);
    bool setTarget(Character *Monster, std::vector<Player *> &CandidateList, Player *&Target);

private:
    LuaMonsterScript(const LuaMonsterScript &);
    LuaMonsterScript &operator=(const LuaMonsterScript &);
};

#endif

