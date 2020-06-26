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

#ifndef LUA_PLAYER_DEATH_SCRIPT_HPP
#define LUA_PLAYER_DEATH_SCRIPT_HPP

#include "LuaScript.hpp"

class Character;

class LuaPlayerDeathScript : public LuaScript {
public:
    explicit LuaPlayerDeathScript(const std::string &filename);
    ~LuaPlayerDeathScript() override = default;
    LuaPlayerDeathScript(const LuaPlayerDeathScript &) = delete;
    auto operator=(const LuaPlayerDeathScript &) -> LuaPlayerDeathScript & = delete;
    LuaPlayerDeathScript(LuaPlayerDeathScript &&) = default;
    auto operator=(LuaPlayerDeathScript &&) -> LuaPlayerDeathScript & = default;

    void playerDeath(Character *deadPlayer);
};

#endif
