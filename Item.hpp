/*
 * illarionserver - server for the game Illarion
 * Copyright 2011 Illarion e.V.
 *
 * This file is part of illarionserver.
 *
 * illarionserver is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * illarionserver is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with illarionserver.  If not, see <http://www.gnu.org/licenses/>.
 */

#ifndef _ITEM_HPP_
#define _ITEM_HPP_

#include <cstdio>
#include <cstdlib>
#include <vector>
#include <string>
#include <boost/shared_ptr.hpp>
#include <boost/unordered_map.hpp>
#include "types.hpp"
#include "globals.hpp"
#include "fuse_ptr.hpp"

class Character;
class Container;

class Item {
public:
    typedef uint16_t id_type;
    typedef uint8_t  number_type;
    typedef uint8_t  wear_type;
    typedef uint16_t quality_type;
    typedef uint32_t data_type;
    typedef boost::unordered_map<std::string, std::string> datamap_type;
    
    static const number_type MAX_NUMBER = 250;
    static const wear_type PERMANENT_WEAR = 255;

    Item(): id(0), number(0), wear(0), quality(333), data(0), datamap(1) {}
    Item(id_type id, number_type number, wear_type wear, quality_type quality = 333, data_type data = 0) :
        id(id), number(number), wear(wear), quality(quality), data(data),datamap(1) {}
    
    inline id_type getId() const {return id;}
    inline void setId(id_type id) {this->id = id;}

    inline number_type getNumber() const {return number;}
    inline void setNumber(number_type number) {this->number = number;}
//    bool increaseNumber(number_type number);
//    bool reduceNumber(number_type number);
    
    inline wear_type getWear() const {return wear;}
    inline void setWear(wear_type wear) {this->wear = wear;}
    
    inline quality_type getQuality() const {return quality;}
    inline void setQuality(quality_type quality) {this->quality = quality;}
    inline quality_type getDurability() const {return quality % 100;}
    void setMinQuality(const Item &item);

    inline data_type getData() const {return data;}
    inline void setData(data_type data) {this->data = data;}
    inline data_type getOldData() const {return getData();}
    inline void setOldData(data_type data) {setData(data);}

    std::string getData(std::string key);
    void setData(std::string key, std::string value);
    inline datamap_type::const_iterator getDataBegin() const {return datamap.cbegin();}
    inline datamap_type::const_iterator getDataEnd() const {return datamap.cend();}

    void reset();
    
    void save(std::ostream *obj) const;
    void load(std::istream *obj);
    
    bool survivesAging();
    bool isContainer() const;
    bool isComplete() const;
    bool isPermanent() const;
    void makePermanent();
    
private:
    id_type id;
    number_type number;
    wear_type wear;
    quality_type quality;
    data_type data;
    datamap_type datamap;
};

class ScriptItem : public Item {
public:
    enum itemtype {
        notdefined = 0,
        it_showcase1 = 1,
        it_showcase2 = 2,
        it_field = 3,
        it_inventory = 4,
        it_belt = 5,
        it_container = 6
    };

    itemtype type;
    position pos;
    unsigned char itempos;
    Character *owner;
    fuse_ptr<Character> getOwnerForLua() {
        fuse_ptr<Character> fuse_owner(owner);
        return fuse_owner;
    };
    Container *inside;
    ScriptItem() : Item(0,0,0), type(notdefined), pos(position(0,0,0)), itempos(255), owner(NULL), inside(NULL) {}
    unsigned char getType() {
        return type;
    }
    ScriptItem(const Item &source) : Item(source) {
        itempos = 0;
        pos = position(0,0,0);
        type = notdefined;
        owner = NULL;
        inside = NULL;
    }

} ;

typedef std::vector < Item > ITEMVECTOR;

#endif
