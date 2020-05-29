/*
 * illarionserver - server for the game Illarion
 * Copyright 2011 Illarion e.V.
 *
 * This file is part of illarionserver.
 *
 * illarionserver is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Affero General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * illarionserver is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Affero General Public License for more details.
 *
 * You should have received a copy of the GNU Affero General Public License
 * along with illarionserver.  If not, see <http://www.gnu.org/licenses/>.
 */

#ifndef _ITEM_HPP_
#define _ITEM_HPP_

#include <vector>
#include <string>
#include <unordered_map>

#include "types.hpp"
#include "globals.hpp"
#include "character_ptr.hpp"

class Character;
class Container;
class ItemLookAt;

class Item {
public:
    using id_type = uint16_t;
    using number_type = uint16_t;
    using wear_type = uint8_t;
    using quality_type = uint16_t;
    using datamap_type = std::unordered_map<std::string, std::string>;

    static const TYPE_OF_VOLUME LARGE_ITEM_VOLUME = 5000;
    static const wear_type PERMANENT_WEAR = 255;

    Item() = default;
    Item(id_type id, number_type number, wear_type wear, quality_type quality = 333) :
        id(id), number(number), wear(wear), quality(quality), datamap(1) {}
    Item(id_type id, number_type number, wear_type wear, quality_type quality, const script_data_exchangemap &datamap);

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
    number_type increaseNumberBy(number_type count);

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

    // setData actually does either a clear (if the datamap is nil) or a merge of the keys in datamap
    // TODO split into clear and add or merge function to make usage more obvious
    void setData(script_data_exchangemap const *datamap);
    bool hasData(const script_data_exchangemap &datamap) const;
    bool hasNoData() const;
    std::string getData(const std::string &key) const;
    void setData(const std::string &key, const std::string &value);
    void setData(const std::string &key, int32_t value);
    inline datamap_type::const_iterator getDataBegin() const {
        return datamap.cbegin();
    }
    inline datamap_type::const_iterator getDataEnd() const {
        return datamap.cend();
    }
    inline bool equalData(script_data_exchangemap const *data) const {
        Item item;
        item.setData(data);
        return equalData(item);
    }
    inline bool equalData(const Item &item) const {
        return datamap == item.datamap;
    }

    uint16_t getDepot() const;

    void reset();
    void resetWear();

    void save(std::ostream &obj) const;
    void load(std::istream &obj);

    bool survivesAgeing();
    bool isContainer() const;
    TYPE_OF_VOLUME getVolume() const;
    TYPE_OF_WEIGHT getWeight() const;
    TYPE_OF_WORTH getWorth() const;
    number_type getMaxStack() const;
    bool isLarge() const;
    bool isStackable() const;
    bool isPermanent() const;
    bool isMovable() const;
    void makePermanent();

    bool operator==(const Item& rhs) const;
private:
    id_type id{0};
    number_type number{0};
    wear_type wear{0};
    quality_type quality{333};
    datamap_type datamap{1};
};

class ScriptItem : public Item {
public:
    enum itemtype {
        notdefined = 0,
        it_field = 3,
        it_inventory = 4,
        it_belt = 5,
        it_container = 6
    };

    itemtype type{notdefined};
    position pos;
    unsigned char itempos{255};
    Character *owner{nullptr};
    character_ptr getOwnerForLua() {
        character_ptr fuse_owner(owner);
        return fuse_owner;
    };
    Container *inside{nullptr};
    ScriptItem() : Item(0,0,0),  pos(position(0, 0, 0)) {}
    unsigned char getType() const {
        return type;
    }
    ScriptItem(const ScriptItem &source) = default;
    explicit ScriptItem(const Item &source) : Item(source), pos(position(0, 0, 0)) {
        itempos = 0;
        type = notdefined;
        owner = nullptr;
        inside = nullptr;
    }

    ItemLookAt getLookAt(Character *) const;

    bool operator==(const ScriptItem& rhs) const;
};

#endif
