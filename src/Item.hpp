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

/**
 * @brief Represents a game item with properties and custom data.
 * 
 * Item is the fundamental class for all in-game objects that can be stored in inventories,
 * containers, on the ground, or equipped by characters. Each item has:
 * - A unique type ID determining its properties
 * - A stack count (number of identical items)
 * - Wear/durability tracking with automatic degradation
 * - Quality value encoding both crafting quality and current durability
 * - Custom data map for script-defined properties
 * 
 * Items can be stackable (multiple in one slot), perishable (wear down over time),
 * or permanent (never decay). Properties like weight, volume, and worth are looked up
 * from the ItemTable based on the item's ID.
 * 
 * @note Quality = (craftingQuality * 100) + durability, where durability ranges 0-99
 * @see ScriptItem for item instances with positional context
 * @see ItemTable for item type definitions
 */
class Item {
public:
    using id_type = uint16_t; ///< Item type identifier
    using number_type = uint16_t; ///< Stack count type
    using wear_type = uint8_t; ///< Wear/durability counter type
    using quality_type = uint16_t; ///< Quality value combining craft quality and durability
    using datamap_type = std::unordered_map<std::string, std::string>; ///< Custom script data storage

    static constexpr TYPE_OF_VOLUME LARGE_ITEM_VOLUME = 5000; ///< Volume threshold for large items
    static constexpr wear_type PERMANENT_WEAR = 255; ///< Special wear value indicating item never decays
    static constexpr quality_type defaultQuality = 333; ///< Default quality for new items (quality 3, durability 33)
    static constexpr quality_type maximumQuality = 999; ///< Maximum possible quality value (quality 9, durability 99)
    static constexpr quality_type maximumDurability = 99; ///< Maximum durability component of quality

    /**
     * @brief Default constructor creating an empty item slot.
     */
    Item() = default;
    
    /**
     * @brief Creates an item with specified properties.
     * 
     * @param id Item type ID from ItemTable
     * @param number Stack count (1 for non-stackable items)
     * @param wear Initial wear value (255 for permanent, 0 for destroyed)
     * @param quality Combined quality value (default = 333)
     */
    Item(id_type id, number_type number, wear_type wear, quality_type quality = defaultQuality)
            : id(id), number(number), wear(wear), quality(quality), datamap(1) {}
    
    /**
     * @brief Creates an item with custom script data.
     * 
     * @param id Item type ID from ItemTable
     * @param number Stack count
     * @param wear Initial wear value
     * @param quality Combined quality value
     * @param datamap Custom key-value data for scripts
     */
    Item(id_type id, number_type number, wear_type wear, quality_type quality, const script_data_exchangemap &datamap);

    inline auto getId() const -> id_type { return id; } ///< @return Item type ID
    inline void setId(id_type id) { this->id = id; } ///< @param id New item type ID

    inline auto getNumber() const -> number_type { return number; } ///< @return Current stack count
    inline void setNumber(number_type number) { this->number = number; } ///< @param number New stack count
    
    /**
     * @brief Increases stack count up to maximum and returns overflow.
     * 
     * @param count Number to add to the stack
     * @return Overflow count that didn't fit (0 if all fit)
     */
    auto increaseNumberBy(number_type count) -> number_type;

    inline auto getWear() const -> wear_type { return wear; } ///< @return Current wear value (255 = permanent)
    inline void setWear(wear_type wear) { this->wear = wear; } ///< @param wear New wear value

    inline auto getQuality() const -> quality_type { return quality; } ///< @return Combined quality value
    inline void setQuality(quality_type quality) { this->quality = quality; } ///< @param quality New quality value
    
    /**
     * @brief Extracts the durability component from quality.
     * 
     * @return Durability value (0-99)
     */
    inline auto getDurability() const -> quality_type { return quality % (maximumDurability + 1); }
    
    /**
     * @brief Sets quality to the minimum of this item and another.
     * 
     * Compares both craft quality and durability, taking the lower value of each.
     * 
     * @param item The item to compare against
     */
    void setMinQuality(const Item &item);

    /**
     * @brief Merges or clears custom data from a script data map.
     * 
     * If datamap is nullptr, clears all custom data. Otherwise, merges keys from
     * the provided map into existing data.
     * 
     * @param datamap Pointer to data map to merge (nullptr to clear)
     */
    void setData(script_data_exchangemap const *datamap);
    
    /**
     * @brief Checks if item has all specified data key-value pairs.
     * 
     * @param datamap Data to check for
     * @return true if all keys exist with matching values
     */
    auto hasData(const script_data_exchangemap &datamap) const -> bool;
    
    /**
     * @brief Checks if item has no custom data.
     * 
     * @return true if datamap is empty
     */
    auto hasNoData() const -> bool;
    
    /**
     * @brief Retrieves a custom data value by key.
     * 
     * @param key Data key to look up
     * @return Value string, or empty string if key doesn't exist
     */
    auto getData(const std::string &key) const -> std::string;
    
    /**
     * @brief Sets a custom data key-value pair.
     * 
     * If value is empty, removes the key. Otherwise, sets or updates the key.
     * 
     * @param key Data key
     * @param value Data value (empty to remove)
     */
    void setData(const std::string &key, const std::string &value);
    
    /**
     * @brief Sets a custom data key to an integer value.
     * 
     * @param key Data key
     * @param value Integer value (converted to string)
     */
    void setData(const std::string &key, int32_t value);
    
    inline auto getDataBegin() const -> datamap_type::const_iterator { return datamap.cbegin(); } ///< @return Iterator to first data entry
    inline auto getDataEnd() const -> datamap_type::const_iterator { return datamap.cend(); } ///< @return Iterator past last data entry
    
    /**
     * @brief Checks if custom data matches a script data map.
     * 
     * @param data Data map to compare (nullptr treated as empty)
     * @return true if data maps are equal
     */
    inline auto equalData(script_data_exchangemap const *data) const -> bool {
        Item item;
        item.setData(data);
        return equalData(item);
    }
    
    /**
     * @brief Checks if custom data matches another item's data.
     * 
     * @param item Item to compare data against
     * @return true if data maps are equal
     */
    inline auto equalData(const Item &item) const -> bool { return datamap == item.datamap; }

    /**
     * @brief Retrieves the depot ID for this item if it's a depot chest.
     * 
     * @return Depot number from custom data (default 1 if not set)
     */
    auto getDepot() const -> uint16_t;

    /**
     * @brief Resets item to empty state.
     * 
     * Clears all properties and custom data, making this an empty item slot.
     */
    void reset();
    
    /**
     * @brief Resets wear to item's ageing speed if applicable.
     * 
     * For non-rotting items, sets wear to the item type's AgeingSpeed if current
     * wear is lower.
     */
    void resetWear();

    /**
     * @brief Serializes item to binary file stream.
     * 
     * @param obj Output file stream
     */
    void save(std::ofstream &obj) const;
    
    /**
     * @brief Deserializes item from binary file stream.
     * 
     * @param obj Input file stream
     */
    void load(std::ifstream &obj);

    /**
     * @brief Decrements wear by one and checks if item survives.
     * 
     * Items with wear = 0 or PERMANENT_WEAR are not affected.
     * 
     * @return true if item still exists (wear > 0), false if destroyed
     */
    auto survivesAgeing() -> bool;
    
    /**
     * @brief Checks if this item type is a container.
     * 
     * @return true if item exists in ContainerObjectTable
     */
    auto isContainer() const -> bool;
    
    /**
     * @brief Gets the volume per item from ItemTable.
     * 
     * @return Volume value, or 0 if item type invalid
     */
    auto getVolume() const -> TYPE_OF_VOLUME;
    
    /**
     * @brief Gets total weight of this stack.
     * 
     * @return Weight per item * stack count, or 0 if invalid
     */
    auto getWeight() const -> TYPE_OF_WEIGHT;
    
    /**
     * @brief Gets total worth/value of this stack.
     * 
     * @return Worth per item * stack count, or 0 if invalid
     */
    auto getWorth() const -> TYPE_OF_WORTH;
    
    /**
     * @brief Gets maximum stack size for this item type.
     * 
     * @return Maximum stack count from ItemTable, or 0 if invalid
     */
    auto getMaxStack() const -> number_type;
    
    /**
     * @brief Checks if item is considered large.
     * 
     * @return true if volume >= LARGE_ITEM_VOLUME
     */
    auto isLarge() const -> bool;
    
    /**
     * @brief Checks if item can stack with others of same type.
     * 
     * @return true if MaxStack > 1
     */
    auto isStackable() const -> bool;
    
    /**
     * @brief Checks if item never decays.
     * 
     * @return true if wear == PERMANENT_WEAR
     */
    auto isPermanent() const -> bool;
    
    /**
     * @brief Checks if item can be picked up and moved.
     * 
     * @return true if weight < MAXWEIGHT and not permanent
     */
    auto isMovable() const -> bool;
    
    /**
     * @brief Makes item permanent (never decay).
     */
    void makePermanent();

    /**
     * @brief Compares all item properties for equality.
     * 
     * @param rhs Item to compare against
     * @return true if all properties match (id, number, wear, quality, datamap)
     */
    auto operator==(const Item &rhs) const -> bool;

private:
    id_type id{0}; ///< Item type identifier from ItemTable
    number_type number{0}; ///< Number of items in this stack
    wear_type wear{0}; ///< Wear/decay counter (255 = permanent, 0 = destroyed)
    quality_type quality{defaultQuality}; ///< Combined quality: (craftQuality * 100) + durability
    datamap_type datamap{1}; ///< Custom script data key-value storage
};

/**
 * @brief Extended item with positional and ownership context for script interactions.
 * 
 * ScriptItem adds location and ownership information to the base Item class, allowing
 * Lua scripts to determine where an item is located (field, inventory, container) and
 * who owns it. This context is essential for:
 * - Item use scripts that need to know the user
 * - Container management scripts
 * - Field trigger items
 * - Inventory position-specific logic
 * 
 * The type field determines how to interpret pos, itempos, owner, and inside:
 * - it_field: Item on ground (pos = world position, itempos unused)
 * - it_inventory: In character slot (pos = character position, itempos = slot number)
 * - it_belt: In character belt (pos = character position, itempos = belt position)
 * - it_container: In a container (inside = container pointer, itempos = slot in container)
 * 
 * @see Item for base item properties
 * @see ItemLookAt for item examination data
 */
class ScriptItem : public Item {
public:
    /**
     * @brief Identifies where the item is located in the game world.
     */
    enum itemtype { 
        notdefined = 0, ///< Location not set
        it_field = 3,   ///< Item on a map tile
        it_inventory = 4, ///< Item in character inventory slot
        it_belt = 5,    ///< Item in character belt
        it_container = 6 ///< Item inside a container
    };
    
    static constexpr uint8_t maxItemPos = 255; ///< Maximum valid item position value

    itemtype type{notdefined}; ///< Location type of this item instance
    position pos{0, 0, 0}; ///< World position or owner position depending on type
    unsigned char itempos{maxItemPos}; ///< Slot/position within inventory, belt, or container
    Character *owner{nullptr}; ///< Character who owns this item (for inventory/belt types)
    
    /**
     * @brief Gets owner wrapped in a safe pointer for Lua scripts.
     * 
     * @return character_ptr wrapper preventing dangling references
     */
    auto getOwnerForLua() const -> character_ptr {
        character_ptr fuse_owner(owner);
        return fuse_owner;
    };
    
    Container *inside{nullptr}; ///< Container holding this item (for it_container type)
    
    /**
     * @brief Default constructor creating an empty script item.
     */
    ScriptItem() : Item(0, 0, 0) {}
    
    /**
     * @brief Gets the location type of this item.
     * 
     * @return itemtype enumeration value
     */
    auto getType() const -> unsigned char { return type; }
    
    /**
     * @brief Creates a ScriptItem from a base Item.
     * 
     * @param source Item to copy properties from
     */
    explicit ScriptItem(const Item &source) : Item(source), itempos(0) {}

    /**
     * @brief Generates look-at/examination text for this item.
     * 
     * Calls the item's LookAtItem script if available, otherwise uses default lookup.
     * 
     * @param character The character examining the item
     * @return ItemLookAt structure with examination details
     */
    auto getLookAt(Character * /*character*/) const -> ItemLookAt;

    /**
     * @brief Compares all properties including location context.
     * 
     * @param rhs ScriptItem to compare against
     * @return true if all Item properties and location data match
     */
    auto operator==(const ScriptItem &rhs) const -> bool;
    
    /**
     * @brief Creates a plain Item copy without location context.
     * 
     * @return New Item with same id, number, wear, quality, and custom data
     */
    auto cloneItem() const -> Item;
};

#endif
