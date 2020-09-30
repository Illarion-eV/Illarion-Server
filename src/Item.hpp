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

#ifndef ITEM_HPP
#define ITEM_HPP

#include "character_ptr.hpp"
#include "globals.hpp"
#include "types.hpp"

#include <string>
#include <unordered_map>
#include <vector>

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

    static constexpr TYPE_OF_VOLUME LARGE_ITEM_VOLUME = 5000;
    static constexpr wear_type PERMANENT_WEAR = 255;
    static constexpr quality_type defaultQuality = 333;
    static constexpr quality_type maximumQuality = 999;
    static constexpr quality_type maximumDurability = 99;

    Item() = default;
    Item(id_type id, number_type number, wear_type wear, quality_type quality = defaultQuality)
            : id(id), number(number), wear(wear), quality(quality), datamap(1) {}
    Item(id_type id, number_type number, wear_type wear, quality_type quality, const script_data_exchangemap &datamap);

    inline auto getId() const -> id_type { return id; }
    inline void setId(id_type id) { this->id = id; }

    inline auto getNumber() const -> number_type { return number; }
    inline void setNumber(number_type number) { this->number = number; }
    auto increaseNumberBy(number_type count) -> number_type;

    inline auto getWear() const -> wear_type { return wear; }
    inline void setWear(wear_type wear) { this->wear = wear; }

    inline auto getQuality() const -> quality_type { return quality; }
    inline void setQuality(quality_type quality) { this->quality = quality; }
    inline auto getDurability() const -> quality_type { return quality % (maximumDurability + 1); }
    void setMinQuality(const Item &item);

    // setData actually does either a clear (if the datamap is nil) or a merge of the keys in datamap
    void setData(script_data_exchangemap const *datamap);
    auto hasData(const script_data_exchangemap &datamap) const -> bool;
    auto hasNoData() const -> bool;
    auto getData(const std::string &key) const -> std::string;
    void setData(const std::string &key, const std::string &value);
    void setData(const std::string &key, int32_t value);
    inline auto getDataBegin() const -> datamap_type::const_iterator { return datamap.cbegin(); }
    inline auto getDataEnd() const -> datamap_type::const_iterator { return datamap.cend(); }
    inline auto equalData(script_data_exchangemap const *data) const -> bool {
        Item item;
        item.setData(data);
        return equalData(item);
    }
    inline auto equalData(const Item &item) const -> bool { return datamap == item.datamap; }

    auto getDepot() const -> uint16_t;

    void reset();
    void resetWear();

    void save(std::ofstream &obj) const;
    void load(std::ifstream &obj);

    auto survivesAgeing() -> bool;
    auto isContainer() const -> bool;
    auto getVolume() const -> TYPE_OF_VOLUME;
    auto getWeight() const -> TYPE_OF_WEIGHT;
    auto getWorth() const -> TYPE_OF_WORTH;
    auto getMaxStack() const -> number_type;
    auto isLarge() const -> bool;
    auto isStackable() const -> bool;
    auto isPermanent() const -> bool;
    auto isMovable() const -> bool;
    void makePermanent();

    auto operator==(const Item &rhs) const -> bool;

private:
    id_type id{0};
    number_type number{0};
    wear_type wear{0};
    quality_type quality{defaultQuality};
    datamap_type datamap{1};
};

class ScriptItem : public Item {
public:
    enum itemtype { notdefined = 0, it_field = 3, it_inventory = 4, it_belt = 5, it_container = 6 };
    static constexpr uint8_t maxItemPos = 255;

    itemtype type{notdefined};
    position pos{0, 0, 0};
    unsigned char itempos{maxItemPos};
    Character *owner{nullptr};
    auto getOwnerForLua() const -> character_ptr {
        character_ptr fuse_owner(owner);
        return fuse_owner;
    };
    Container *inside{nullptr};
    ScriptItem() : Item(0, 0, 0) {}
    auto getType() const -> unsigned char { return type; }
    explicit ScriptItem(const Item &source) : Item(source), itempos(0) {}

    auto getLookAt(Character * /*character*/) const -> ItemLookAt;

    auto operator==(const ScriptItem &rhs) const -> bool;
    auto cloneItem() const -> Item;
};

#endif
