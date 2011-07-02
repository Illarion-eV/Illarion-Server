//  illarionserver - server for the game Illarion
//  Copyright 2011 Illarion e.V.
//
//  This file is part of illarionserver.
//
//  illarionserver is free software: you can redistribute it and/or modify
//  it under the terms of the GNU General Public License as published by
//  the Free Software Foundation, either version 3 of the License, or
//  (at your option) any later version.
//
//  illarionserver is distributed in the hope that it will be useful,
//  but WITHOUT ANY WARRANTY; without even the implied warranty of
//  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//  GNU General Public License for more details.
//
//  You should have received a copy of the GNU General Public License
//  along with illarionserver.  If not, see <http://www.gnu.org/licenses/>.


#ifndef _CLUATILESCRIPT_HPP
#define _CLUATILESCRIPT_HPP

#include "LuaScript.hpp"
#include "TableStructs.hpp"
#include "globals.hpp"
#include "Item.hpp"

class World;
class Character;

class LuaTileScript : public LuaScript {
public:
    LuaTileScript(std::string filename,TilesStruct tile) throw(ScriptException);
    virtual ~LuaTileScript() throw();

    //Character uses an Tile
    //\param user: the Character which is using the tile
    //\param pos: the Position at which the tile is used
    void useTile(Character *user, position pos, unsigned short int counter, unsigned int param, unsigned char ltastate);

    //Character uses a tile with a item
    //\param user: The Character which is using the tile
    //\param pos: the position at which the tile is used
    //\param item: the ScriptItem whith which the field is used
    void useTileWithItem(Character *user, position pos, ScriptItem item, unsigned short int counter, unsigned int param, unsigned char ltastate);

    //Character uses a tile with another tile
    //\param user: the Character which is using the tile
    //\param pos: the position at which the tile is used
    //\param posnew: the other position
    void useTileWithField(Character *user, position pos, position posnew, unsigned short int counter, unsigned int param, unsigned char ltastate);

    //Character uses a tile with another character
    //\param user: the Character which is using the tile
    //\param pos: the position at which the tile is used
    //\param targetchar: the character whith which the field is used.
    void useTileWithCharacter(Character *user, position pos, Character *character, unsigned short int counter, unsigned int param, unsigned char ltastate);

    /**
    *a longtime action is disturbed by another person
    */
    bool actionDisturbed(Character *performer, Character *disturber);

private:

    LuaTileScript(const LuaTileScript &);
    LuaTileScript &operator=(const LuaTileScript &);
    TilesStruct thisTile;
    void init_functions();
};
#endif
