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

#ifndef _LUA_TILE_SCRIPT_HPP_
#define _LUA_TILE_SCRIPT_HPP_

#include "LuaScript.hpp"
#include "TableStructs.hpp"
#include "Item.hpp"

class World;
class Character;

class LuaTileScript : public LuaScript {
public:
    LuaTileScript(const std::string &filename, const TilesStruct &tile);
    ~LuaTileScript() override;

    void useTile(Character *user, const position &pos, unsigned char ltastate);
    bool actionDisturbed(Character *performer, Character *disturber);

private:

    LuaTileScript(const LuaTileScript &);
    LuaTileScript &operator=(const LuaTileScript &);
    TilesStruct thisTile;
    void init_functions() const;
};

#endif

