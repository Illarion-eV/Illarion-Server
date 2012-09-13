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
#include <luabind/object.hpp>

class Character;
class Container;

class ItemLookAt {
public:
    static const uint8_t MAX_GEM_LEVEL = 10;
    static const uint8_t MAX_DURABILITY = 100;
    enum Rareness {
        commonItem = 1,
        uncommonItem = 2,
        rareItem = 3,
        epicItem = 4,
    };

    ItemLookAt(): rareness(commonItem), weight(0), worth(0), durabilityValue(0),
        diamondLevel(0), emeraldLevel(0), rubyLevel(0), sapphireLevel(0),
        amethystLevel(0), obsidianLevel(0), topazLevel(0), bonus(0) {}

    void setName(std::string name) {
        this->name = name;
    }
    const std::string &getName() const {
        return name;
    }

    void setRareness(Rareness rareness) {
        this->rareness = rareness;
    }
    Rareness getRareness() const {
        return rareness;
    }

    void setDescription(std::string description) {
        this->description = description;
    }
    const std::string &getDescription() const {
        return description;
    }

    void setCraftedBy(std::string craftedBy) {
        this->craftedBy = craftedBy;
    }
    const std::string &getCraftedBy() const {
        return craftedBy;
    }

    void setWeight(TYPE_OF_WEIGHT weight) {
        this->weight = weight;
    }
    TYPE_OF_WEIGHT getWeight() const {
        return weight;
    }

    void setWorth(TYPE_OF_WORTH worth) {
        this->worth = worth;
    }
    TYPE_OF_WORTH getWorth() const {
        return worth;
    }

    void setQualityText(std::string qualityText) {
        this->qualityText = qualityText;
    }
    const std::string &getQualityText() const {
        return qualityText;
    }

    void setDurabilityText(std::string durabilityText) {
        this->durabilityText = durabilityText;
    }
    const std::string &getDurabilityText() const {
        return durabilityText;
    }

    void setDurabilityValue(uint8_t durabilityValue) {
        if (durabilityValue <= MAX_DURABILITY) {
            this->durabilityValue = durabilityValue;
        }
    }
    uint8_t getDurabilityValue() const {
        return durabilityValue;
    }

    void setDiamondLevel(uint8_t diamondLevel) {
        if (diamondLevel <= MAX_GEM_LEVEL) {
            this->diamondLevel = diamondLevel;
        }
    }
    uint8_t getDiamondLevel() const {
        return diamondLevel;
    }

    void setEmeraldLevel(uint8_t emeraldLevel) {
        if (emeraldLevel <= MAX_GEM_LEVEL) {
            this->emeraldLevel = emeraldLevel;
        }
    }
    uint8_t getEmeraldLevel() const {
        return emeraldLevel;
    }

    void setRubyLevel(uint8_t rubyLevel) {
        if (rubyLevel <= MAX_GEM_LEVEL) {
            this->rubyLevel = rubyLevel;
        }
    }
    uint8_t getRubyLevel() const {
        return rubyLevel;
    }

    void setSapphireLevel(uint8_t sapphireLevel) {
        if (sapphireLevel <= MAX_GEM_LEVEL) {
            this->sapphireLevel = sapphireLevel;
        }
    }
    uint8_t getSapphireLevel() const {
        return sapphireLevel;
    }

    void setAmethystLevel(uint8_t amethystLevel) {
        if (amethystLevel <= MAX_GEM_LEVEL) {
            this->amethystLevel = amethystLevel;
        }
    }
    uint8_t getAmethystLevel() const {
        return amethystLevel;
    }

    void setObsidianLevel(uint8_t obsidianLevel) {
        if (obsidianLevel <= MAX_GEM_LEVEL) {
            this->obsidianLevel = obsidianLevel;
        }
    }
    uint8_t getObsidianLevel() const {
        return obsidianLevel;
    }

    void setTopazLevel(uint8_t topazLevel) {
        if (topazLevel <= MAX_GEM_LEVEL) {
            this->topazLevel = topazLevel;
        }
    }
    uint8_t getTopazLevel() const {
        return topazLevel;
    }

    void setBonus(uint8_t bonus) {
        this->bonus = bonus;
    }
    uint8_t getBonus() const {
        return bonus;
    }

private:
    std::string name;
    Rareness rareness;
    std::string description;
    std::string craftedBy;
    TYPE_OF_WEIGHT weight;
    TYPE_OF_WORTH worth;
    std::string qualityText;
    std::string durabilityText;
    uint8_t durabilityValue;
    uint8_t diamondLevel;
    uint8_t emeraldLevel;;
    uint8_t rubyLevel;
    uint8_t sapphireLevel;
    uint8_t amethystLevel;
    uint8_t obsidianLevel;
    uint8_t topazLevel;
    uint8_t bonus;
};

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
    Item(id_type id, number_type number, wear_type wear, quality_type quality, const luabind::object &datamap);

    inline id_type getId() const {
        return id;
    }
    inline void setId(id_type id) {
        this->id = id;
    }

    inline number_type getNumber() const {
        return number;
    }
    inline void setNumber(number_type number) {
        this->number = number;
    }

    inline wear_type getWear() const {
        return wear;
    }
    inline void setWear(wear_type wear) {
        this->wear = wear;
    }

    inline quality_type getQuality() const {
        return quality;
    }
    inline void setQuality(quality_type quality) {
        this->quality = quality;
    }
    inline quality_type getDurability() const {
        return quality % 100;
    }
    void setMinQuality(const Item &item);

    data_type getData() const;
    void setData(data_type data);
    data_type getOldData() const;
    void setOldData(data_type data);
    void setData(const luabind::object &datamap);
    bool hasData(const luabind::object &datamap);
    std::string getData(std::string key);
    void setData(std::string key, std::string value);
    void setData(std::string key, int32_t value);
    inline datamap_type::const_iterator getDataBegin() const {
        return datamap.cbegin();
    }
    inline datamap_type::const_iterator getDataEnd() const {
        return datamap.cend();
    }
    inline bool equalData(const Item &item) const {
        return datamap == item.datamap;
    }

    void reset();
    void resetWear();

    void save(std::ostream *obj) const;
    void load(std::istream *obj);

    bool survivesAgeing();
    bool isContainer() const;
    TYPE_OF_WEIGHT getWeight() const;
    TYPE_OF_WORTH getWorth() const;
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
