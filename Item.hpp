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


#ifndef ITEM_HH
#define ITEM_HH

#include <cstdio>
#include <cstdlib>
#include <vector>
#include <string>
#include <boost/shared_ptr.hpp>
#include "globals.hpp"
#if __GNUC__ < 3
#include <hash_map>
#else
#include <ext/hash_map>
#include "fuse_ptr.hpp"

#if (__GNUC__ > 3) || (__GNUC__ == 3 && __GNUC_MINOR__ >= 1)
using __gnu_cxx::hash_map;
namespace __gnu_cxx {
template<>
struct hash<std::string> {
    hash<char *> h;
    size_t operator()(const std::string &s) const {
        return h(s.c_str());
    };
};
};
#endif

#if (__GNUC__ == 3 && __GNUC_MINOR__ < 1)
using std::hash_map;
/** BEGIN FIX **/
namespace std {
template<> struct hash< std::string > {
    size_t operator()(const std::string &x) const {
        return hash< const char * >()(x.c_str());
    };
};
};
/** END FIX **/

#endif

#endif

#define TYPE_OF_ITEM_ID                unsigned short int


class Character;
class Container;

/**
* a basic Item or more than one stackable items of the same type
*/
class Item {
public:
    /**
    *@name Item Lua Variables:
    *Variables which are exported to lua
    */

    //@{
    /**
    *======================start grouping Lua Variables===================
    *@ingroup Scriptvariables
    */



    /**
    * the id of the item(s)
    * <b>Lua: (r/w) [id]</b>
    */
    TYPE_OF_ITEM_ID id;

    /**
    * the number of the item(s)
    * <b>Lua: (r/w) [number]</b>
    */
    uint8_t number;

    /**
    * the age of the item(s)
    * <b>Lua: (r/w) [wear]</b>
    */
    uint8_t wear;

    /**
    * the quality of the item(s) < 100 not finished items
    * <b>Lua: (r/w) id</b>
    */
    uint16_t quality;

    /**
    * the data of the item(s) for different use
    * for example keys, or storing item specific informations
    * <b>Lua: (r/w) id</b>
    */
    uint32_t data;

    /**
    *======================end grouping Lua Variables===================
    */
    //@}

    /**
    *@name Item Lua Fucntions:
    *Functions which are exported to lua
    */

    //@{
    /**
    *======================start grouping Lua Functions===================
    *@ingroup Scriptvariables
    */

    /**
    * constructor initializes all values with null except quality, quality is initialized with 333
    */
    Item() : id(0), number(0), wear(0),quality(333),data(0),data_map(1) {}

    /**
    * constructor which initializes all values
    * @param _id the id of the item
    * @param _number the number of items
    * @param _wear the wear of the items
    * @param _quality the quality of the items
    * @param _data the data value of the items
    */
    Item(TYPE_OF_ITEM_ID _id, unsigned char _number, unsigned char _wear, uint16_t _quality = 333, uint32_t _data = 0) :
        id(_id), number(_number), wear(_wear), quality(_quality), data(_data),data_map(1) {}


    std::string getData(std::string key);
    void setData(std::string key, std::string value);
    void save(std::ostream *obj);
    void load(std::istream *obj);

    /**
    Simulate a deep copy by copying the pointer to the map
    the boost::shared_ptr here aren't threadsave !!! If a copy over different
    thread is necessary there must be some synchronisation implemented
    */
    typedef hash_map<std::string,std::string> DATA_MAP;
    DATA_MAP data_map;




};

/**
* @ingroup Scriptclasses
* a script item, modified basic items for scripts which knows it exact position or owner
*/
class ScriptItem : public Item {
public:
    /**
    *@name ScriptItem Lua Defintions:
    *definitions which are exported to lua like enums
    */

    //@{
    /**
    *======================start grouping Lua Definitions===================
    *@ingroup Scriptenums
    */

    /**
    * which type (where the item) is
    */
    enum itemtype {
        notdefined = 0, /**< not defined, we don't know it exactly*/
        it_showcase1 = 1, /**< item is shown in the left showcase*/
        it_showcase2 = 2, /**< item is shown in the right showcase*/
        it_field = 3, /**< item lies on the ground*/
        it_inventory = 4, /**< item is in the inventory (body) of the owner*/
        it_belt = 5,  /**< item is in the belt of the owner*/
        it_container = 6 /**< item is inside a container*/
    };

    /**
    *========================end grouping lua definitions====================
    */
    //@}

    /**
    * stores the type of item (showcase, field @see itemtype)
    */
    itemtype type;

    /**
    *@name ScriptItem Lua Variables:
    *Variables which are exported to lua
    */

    //@{
    /**
    *======================start grouping Lua Variables===================
    *@ingroup Scriptvariables
    */

    /**
    * position coordinates of the item
    * <b>Lua: (ro) [pos]</b>
    */
    position pos;

    /**
    * position at the body inside the container where the item is
    * <b>Lua: (ro) [itempos]</b>
    */
    unsigned char itempos;

    /**
    * pointer to the owner of the item
    * <b>Lua: (ro) [owner]</b>
    */
    Character *owner;
    fuse_ptr<Character> getOwnerForLua() {
        fuse_ptr<Character> fuse_owner(owner);
        return fuse_owner;
    };

    /**
    * stores a pointer to the container where the item is
    * <b>Lua: (ro) [inside]</b>
    */
    Container *inside;

    /**
    *==================================End of group scriptvariables==================
    */
    //@}

    /**
    *@name ScriptItem Lua Functions:
    *Functions which are exported to lua
    */

    //@{
    /**
    *======================start grouping Lua Functions===================
    *@ingroup Scripfunctions
    */

    /**
    * Constructor which initializes all variables with 0
    * <b>Lua: ScriptItem()</b>
    */
    ScriptItem() : Item(0,0,0), type(notdefined), pos(position(0,0,0)), itempos(255), owner(NULL), inside(NULL) {}

    /**
    * gets the type of the item
    * @return the type of this item
    */
    unsigned char getType() {
        return type;
    }



    /**
    *======================end grouping lua Functions=====================
    */
    //@}

    /**
    * copy constructor for script item
    */
    ScriptItem(const Item &source) : Item(source) {
        itempos = 0;
        pos = position(0,0,0);
        type = notdefined;
        owner = NULL;
        inside = NULL;
    }

} ;


/**
* defines a vector class for items
*/
typedef std::vector < Item > ITEMVECTOR;

/**
* defines a type of functions which takes a pointer to a item
*/
typedef bool (*ITEM_FUNCT)(Item *);

#endif
