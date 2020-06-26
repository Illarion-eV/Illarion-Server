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

#ifndef LUA_NPC_SCRIPT_HPP
#define LUA_NPC_SCRIPT_HPP

#include "Character.hpp"
#include "Item.hpp"
#include "LuaScript.hpp"

class NPC;
class World;

class LuaNPCScript : public LuaScript {
public:
    LuaNPCScript(const std::string &filename, NPC *thisnpc);
    ~LuaNPCScript() override = default;
    LuaNPCScript(const LuaNPCScript &) = delete;
    auto operator=(const LuaNPCScript &) -> LuaNPCScript & = delete;
    LuaNPCScript(LuaNPCScript &&) = default;
    auto operator=(LuaNPCScript &&) -> LuaNPCScript & = default;

    void nextCycle();
    void receiveText(Character::talk_type tt, const std::string &message, Character *cc);
    void useNPC(Character *user, unsigned char ltastate);
    void lookAtNpc(Character *source, unsigned char mode);
    auto actionDisturbed(Character *performer, Character *disturber) -> bool;
    void beforeReload();
    void characterOnSight(Character *enemy);
    void characterNear(Character *enemy);
    void abortRoute();

private:
    NPC *_thisnpc;

    void init_functions();
};

#endif
