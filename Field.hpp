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


#ifndef FIELD_HH
#define FIELD_HH

// #define Field_DEBUG

#include "constants.hpp"
#include "globals.hpp"
#include "TableStructs.hpp"

#include <vector>
#include <sys/socket.h>

#include "Item.hpp"

class CommonObjectTable;
class TilesModificatorTable;
class ContainerObjectTable;
class TilesTable;

extern CommonObjectTable *CommonItems;
extern TilesModificatorTable *TilesModItems;
extern TilesTable *Tiles;
extern std::vector<int> *erasedcontainers;


class Field {
private:
    unsigned short int tile;
    unsigned short int music;
    unsigned char clientflags;
    unsigned char extraflags;
    position *warptarget;

public:
    void setTileId(unsigned short int id);
    void setMusicId(unsigned short int id);
    unsigned short int getMusicId();
    unsigned short int getTileCode();

    Field();
    Field(const Field &source);
    ~Field();
    Field &operator =(const Field &source);

    void Save(std::ostream *mapt, std::ostream *obj, std::ostream *warp);

    /**
    * changes the quality of the top item on this field
    * @return true if the item was deleted because of low quality
    */
    bool changeQualityOfTopItem(short int amount);

    /**
    * finds all non passable items on a field
    * @param nonpassitems byreference here the items are returned which are non passable
    */
    void giveNonPassableItems(ITEMVECTOR &nonpassitems);

    /**
    * finds all non movable items on a field
    * @param nonmoveitems byreference here the items are returned which are non movable
    */
    void giveExportItems(ITEMVECTOR &nonmoveitems);

    /**
    * loads this field from files
    * @param mapt mapfile where the tile id and the flags are loaded from
    * @param obj the itemfile where the items are loaded from
    * @param warp the file where the warpfield is loaded from
    */
    void Load(std::istream *mapt, std::istream *obj, std::istream *warp);

    /**
    * checks if a field is passable for characters
    * @return true if it is passable otherwise false
    */
    bool IsPassable() const;

    /**
    * checks if you can look through a tile
    * @return true if the tile is transparent otherwise false
    */
    bool IsTransparent();

    /**
    * checks if you can fall through a tile
    * @return if a character can fall through this tile otherwise false
    */
    bool IsPenetrateable();

    /**
    * checks if a character is on this field (earlier the players etc where seperated)
    * @return true if there is a character on this field
    */
    bool IsPlayerOnField();

    /**
    * checks if a npc is on field
    * @return true if there is a npc on this field
    */
    bool IsNPCOnField();

    /**
    * checks if a Monster is on a field
    * @return true if there is a monster on this field otherwise false
    */
    bool IsMonsterOnField();

    /**
    * checks if the field is a warpfield
    * @return true if the field is a warpfield otherwise false
    */
    bool IsWarpField();

    /**
    * checks if there lies a special item on this field
    * @return true if there is a special item on this field otherwise false
    */
    bool HasSpecialItem();

    /**
    * checks if the field is a special field (triggerfield for example)
    * @return true if the field is a special field otherwise false
    */
    bool IsSpecialField();

    /**
    * check if a move to this field is possible or not ( for example another character stands at it)
    * @return true if a move to the field is possible otherwise false
    */
    bool moveToPossible() const;

    /**
    * finds the height level of the field
    * @return the height level of this field
    */
    unsigned char GroundLevel();

    /**
    * calculates the flags for this field new
    */
    void updateFlags();

    /**
    * @name Field Lua Functions:
    * the Functions which are exported to Lua
    */

    //@{
    /**
    *=========================start grouping script functions===================
    * @ingroup Scriptfunctions
    */

    /**
    * reads the true tile id of the field
    * <b>Lua: [:tile]</b>
    * @return the id of the field
    */
    unsigned short int getTileId();

    unsigned short int getSecondaryTileId();

    /**
    * adds a item to the stack of this field but not if the field is not passable
    * <b>Lua: [:createItem]</b>
    * @param it the item which should be added to this field
    * @return true if the item was added false if the maximum number of items is reached or the field is not passable
    */
    bool addTopItem(const Item &it);

    /**
    * adds a item on the stack of this field
    * <b>Lua: [:createItemAlways]</b>
    * @param it the item which should be added to this field
    * @return true if the item is added or false if the maximum number of items on this field is reached
    */
    bool PutTopItem(const Item &it);

    /**
    * adds an item to the bottom of the stack on a field
    * <b>Lua: [:createItemGround]</b>
    * @param it the item which should be added
    * @return true if the item is added or false if the maximum number of items on this field is reached
    */
    bool PutGroundItem(const Item &it);

    /**
    * takes the top item from this field
    * <b>Lua: [:takeTopItem]</b>
    * @param it byreference returns the item from this field which was the top item
    * @return true if there was a item on this field otherwise false
    */
    bool TakeTopItem(Item &it);

    /**
    * increases/decreases the number of the top item on this field
    * <b>Lua: [:increaseTopItem]</b>
    * @param count the +/- value which should be added
    * @param erased by reference is true if the top item was deleted because the number of items where smaller then the number which should be erased
    * @return the number of which cant be processed for example if count + items on the filed > maximum items
    */
    int increaseTopItem(int count, bool &erased);

    /**
    * changes the id and quality of the top item
    * <b>Lua: [:swapTopItem]</b>
    * @param newid the new id if the top item
    * @param newQuality the new Quality of the top item
    * @return true if there is a top item otherwise false
    */
    bool swapTopItem(TYPE_OF_ITEM_ID newid, uint16_t newQuality = 0);

    /**
    * gets the top item of this field
    * <b>Lua: [:viewTopItem]</b>
    * @param it byreference returns the top item on the field
    * @return true if there is at least one item on this field which can be returned otherwise false
    */
    bool ViewTopItem(Item &it);

    /**
    * deletes all Items from this field
    * <b>Lua: [:deleteAllItems]</b>
    */
    void DeleteAllItems();

    /**
    * returns the script item on a specific position inside the stack of items
    * <b>Lua: [:getStackItem]</b>
    * @param spos the position inside the item stack on this field
    * @return the scriptitem struct of the item on spos
    */
    ScriptItem getStackItem(uint8_t spos);

    /**
    * returns the number of items which are inside the item stack of this field
    * @return the number of items on this field
    */
    MAXCOUNTTYPE NumberOfItems();

    /**
    *=================================end of grouping script functions================
    */
    //@}

    /**
    * ages all items on this field calls funct for all Items on the field
    * @param funct a function which takes a item as parameter
    * @return -1 if a container was deleted (update needed), 0 if nothing has changed, 1 an item changed (update needed)
    */
    int8_t DoAgeItems();

    /**
    * sets the player on this field state
    * if true  no other character can move to this field
    * @param t the new state ( true if a character is on the field )
    */
    void SetPlayerOnField(bool t);

    /**
    * sets the npc on this field state
    * if true  no other character can move to this field
    * @param t the new state ( true if a character is on the field )
    */
    void SetNPCOnField(bool t);

    /**
    * sets the monster on this field state
    * if true  no other character can move to this field
    * @param t the new state ( true if a character is on the field )
    */
    void SetMonsterOnField(bool t);

    /**
    * sets a char to this field so the field is occupied by a char
    */
    void setChar();

    /**
    * removes a char fro this field so the field is no longer occupied by a char
    */
    void removeChar();

    /**
    * sets the flag which shows if the field is a warpfield
    * if true the field is a warpfield
    * @param pos the new position of a warp destination
    */
    void SetWarpField(const position &pos);

    /**
    * unsets the field's warpfield property
    */
    void UnsetWarpField();

    /**
    * delivers the position of the warp destination
    * @param pos the position of the warp destination
    */
    void GetWarpField(position &pos);

    /**
    * sets the flag which shows if there is a special item on this field
    * if true there is a special item on the field otherwise its only a normal field
    * @param t the new state of the flag
    */
    void SetSpecialItem(bool t);

    /**
    * sets the flag which shows if the field is a special field
    * if true the field is a special field otherwise its only a normal field
    * @param t the new state of the flag
    */
    void SetSpecialField(bool t);

    /**
    * sets the height level of the field
    * @param z the new heightlevel with 0 <= z <= 3
    */
    void SetLevel(unsigned char z);

    /**
    * defines a type of function which takes a pointer to a field as param an returns nothing
    */
    typedef void (*FIELD_FUNCT)(Field *);

    /**
    * stores the items on this field
    */
    ITEMVECTOR items;
};

#endif
