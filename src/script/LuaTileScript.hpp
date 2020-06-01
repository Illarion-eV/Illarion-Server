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

#ifndef LUA_TILE_SCRIPT_HPP
#define LUA_TILE_SCRIPT_HPP

#include "Item.hpp"
#include "LuaScript.hpp"
#include "TableStructs.hpp"

class World;
class Character;

class LuaTileScript : public LuaScript {
public:
    LuaTileScript(const std::string &filename, TilesStruct tile);
    ~LuaTileScript() override;
    LuaTileScript(const LuaTileScript &) = delete;
    auto operator=(const LuaTileScript &) -> LuaTileScript & = delete;

    void useTile(Character *user, const position &pos, unsigned char ltastate);
    auto actionDisturbed(Character *performer, Character *disturber) -> bool;

private:
    TilesStruct thisTile;
    void init_functions() const;
};

#endif
