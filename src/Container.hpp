//  illarionserver - server for the game Illarion
//  Copyright 2011 Illarion e.V.
//
//  This file is part of illarionserver.
//
//  illarionserver is free software: you can redistribute it and/or modify
//  it under the terms of the GNU Affero General Public License as published by
//  the Free Software Foundation, either version 3 of the License, or
//  (at your option) any later version.
//
//  illarionserver is distributed in the hope that it will be useful,
//  but WITHOUT ANY WARRANTY; without even the implied warranty of
//  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//  GNU Affero General Public License for more details.
//
//  You should have received a copy of the GNU Affero General Public License
//  along with illarionserver.  If not, see <http://www.gnu.org/licenses/>.

#ifndef CONTAINER_HPP
#define CONTAINER_HPP

#include "Item.hpp"
#include "TableStructs.hpp"

#include <fstream>
#include <iostream>
#include <map>

class ItemTable;

/**
 * @brief Exception thrown when container operations exceed maximum recursion depth.
 * 
 * Prevents infinite loops when processing nested containers.
 */
class RecursionException : public std::exception {};

/**
 * @brief Represents a container that can hold items and nested containers.
 * 
 * Container manages a collection of items stored in numbered slots, with support for:
 * - Item stacking and automatic merging
 * - Nested containers (bags within bags)
 * - Weight calculation including contents
 * - Item aging and wear management
 * - Serialization to/from file streams
 * - Depot functionality for persistent player storage
 * 
 * Containers are non-copyable and non-movable. They maintain two maps:
 * - items: Maps slot numbers to Item objects
 * - containers: Maps slot numbers to nested Container pointers (for container items)
 * 
 * Slot capacity is determined by the container's item type definition in ContainerObjectTable.
 * 
 * @see Item
 * @see ContainerObjectTable
 * @note Container owns and manages the lifetime of nested Container pointers
 */
class Container {
public:
    using ITEMMAP = std::map<TYPE_OF_CONTAINERSLOTS, Item>; ///< Map of slot positions to items
    using CONTAINERMAP = std::map<TYPE_OF_CONTAINERSLOTS, Container *>; ///< Map of slot positions to nested containers

    /**
     * @brief Creates a new container of the specified type.
     * 
     * @param itemId The container item type ID from ContainerObjectTable
     */
    explicit Container(Item::id_type itemId);
    
    Container(const Container &) = delete;
    Container(Container &&) = delete;
    auto operator=(const Container &) -> Container & = delete;
    auto operator=(Container &&) -> Container & = delete;
    
    /**
     * @brief Destructor that cleans up all items and nested containers.
     */
    virtual ~Container();

    /**
     * @brief Removes an item or portion of a stack from a specific slot.
     * 
     * If the item is a container, also retrieves the nested Container pointer.
     * For stackable items, can take a partial count.
     * 
     * @param nr Slot number to take from
     * @param item Output parameter receiving the removed item
     * @param cc Output parameter receiving nested container pointer (nullptr if not a container)
     * @param count Number of items to take from stack (entire stack if >= current number)
     * @return true if item was successfully taken
     */
    auto TakeItemNr(TYPE_OF_CONTAINERSLOTS nr, Item &item, Container *&cc, Item::number_type count) -> bool;
    
    /**
     * @brief Views an item at a specific slot without removing it.
     * 
     * @param nr Slot number to view
     * @param item Output ScriptItem with item data and location context
     * @param cc Output parameter receiving nested container pointer if applicable
     * @return true if item exists at that slot
     */
    auto viewItemNr(TYPE_OF_CONTAINERSLOTS nr, ScriptItem &item, Container *&cc) -> bool;
    
    /**
     * @brief Modifies the durability of an item at a specific slot.
     * 
     * @param nr Slot number
     * @param amount Durability change (positive to repair, negative to damage)
     * @return true if item exists and quality was changed
     */
    auto changeQualityAt(TYPE_OF_CONTAINERSLOTS nr, short int amount) -> bool;
    
    /**
     * @brief Inserts a container with its contents into the first available slot.
     * 
     * @param item The container item
     * @param cc Pointer to the Container object to insert
     * @return true if successfully inserted
     */
    auto InsertContainer(const Item &item, Container *cc) -> bool;
    
    /**
     * @brief Inserts a container with its contents at a specific slot.
     * 
     * @param item The container item
     * @param cc Pointer to the Container object to insert
     * @param pos Target slot position
     * @return true if successfully inserted
     */
    auto InsertContainer(const Item &item, Container *cc, TYPE_OF_CONTAINERSLOTS pos) -> bool;
    
    /**
     * @brief Attempts to merge a stackable item with existing stacks.
     * 
     * Finds matching items (same ID and data) and adds to their stacks up to MaxStack limit.
     * 
     * @param item Item to merge
     * @return Number of items that couldn't be merged (0 if fully merged)
     */
    auto mergeItem(Item item) -> Item::number_type;
    
    /**
     * @brief Inserts an item with automatic stacking/merging.
     * 
     * @param item Item to insert
     * @param merge If true, attempts to merge with existing stacks first
     * @return true if item was fully inserted
     */
    auto InsertItem(Item item, bool merge) -> bool;
    
    /**
     * @brief Inserts an item at a specific slot, stacking if possible.
     * 
     * @param item Item to insert
     * @param pos Target slot position
     * @return true if successfully inserted
     */
    auto InsertItem(Item item, TYPE_OF_CONTAINERSLOTS /*pos*/) -> bool;
    
    /**
     * @brief Inserts an item with automatic merging enabled.
     * 
     * @param item Item to insert
     * @return true if item was fully inserted
     */
    auto InsertItem(const Item &item) -> bool;

    /**
     * @brief Serializes container and contents to binary stream.
     * 
     * @param where Output file stream
     */
    void Save(std::ofstream &where) const;
    
    /**
     * @brief Deserializes container and contents from binary stream.
     * 
     * @param where Input file stream
     */
    void Load(std::ifstream &where);

    /**
     * @brief Ages all items, reducing wear by one.
     * 
     * Items with wear reaching 0 are destroyed. Recursively ages nested containers.
     * 
     * @param inventory If true, only ages items marked as rotting in inventory
     */
    void doAge(bool inventory = false);
    
    /**
     * @brief Resets wear on all items that don't rot in inventory.
     * 
     * Sets wear to the item's AgeingSpeed for applicable items.
     */
    void resetWear();

    /**
     * @brief Counts items of a specific type, optionally matching custom data.
     * 
     * Recursively searches nested containers.
     * 
     * @param itemid Item type ID to count
     * @param data Optional custom data that items must match (nullptr = ignore data)
     * @return Total count of matching items
     */
    virtual auto countItem(Item::id_type itemid, script_data_exchangemap const *data = nullptr) const -> int;

    /**
     * @brief Adds items of a specific type to a list.
     * 
     * Recursively searches nested containers, creating ScriptItem entries with location context.
     * 
     * @param itemid Item type ID to find
     * @param list Output vector to append ScriptItems to
     */
    void addContentToList(Item::id_type itemid, std::vector<ScriptItem> &list);
    
    /**
     * @brief Adds all items to a list.
     * 
     * Recursively searches nested containers.
     * 
     * @param list Output vector to append ScriptItems to
     */
    void addContentToList(std::vector<ScriptItem> &list);

    /**
     * @brief Gets all items of a specific type as a list.
     * 
     * @param itemid Item type ID to find
     * @return Vector of ScriptItems matching the type
     */
    auto getItemList(Item::id_type itemid) -> std::vector<ScriptItem>;
    
    /**
     * @brief Gets all items as a list.
     * 
     * @return Vector of all ScriptItems in this container and nested containers
     */
    auto getItemList() -> std::vector<ScriptItem>;

    /**
     * @brief Removes a specific count of items, optionally matching custom data.
     * 
     * Recursively searches nested containers. Removes from stacks as needed.
     * 
     * @param itemid Item type ID to remove
     * @param count Number of items to remove
     * @param data Optional custom data that items must match (nullptr = ignore data)
     * @return Number of items actually removed
     */
    virtual auto eraseItem(Item::id_type itemid, Item::number_type count, script_data_exchangemap const *data = nullptr)
            -> int;

    /**
     * @brief Increases the stack count of an item at a specific position.
     * 
     * @param pos Slot position
     * @param count Number to add to stack (capped at MaxStack)
     * @return New stack count
     */
    auto increaseAtPos(unsigned char pos, int count) -> int;

    /**
     * @brief Replaces the item at a position with a different item type.
     * 
     * @param pos Slot position
     * @param newid New item type ID
     * @param newQuality New quality value (0 to keep existing quality)
     * @return true if item was successfully swapped
     */
    auto swapAtPos(unsigned char pos, Item::id_type newid, Item::quality_type newQuality = 0) -> bool;

    /**
     * @brief Updates an item in the container based on ScriptItem changes.
     * 
     * @param item ScriptItem with updated properties
     * @return true if item was found and updated
     */
    auto changeItem(ScriptItem &item) -> bool;

    /**
     * @brief Calculates total weight of container and all contents.
     * 
     * Recursively includes weight of nested containers and their contents.
     * 
     * @return Total weight in game units
     */
    auto weight() -> int;

    /**
     * @brief Gets the maximum number of slots this container can hold.
     * 
     * @return Slot count from ContainerObjectTable
     */
    [[nodiscard]] virtual auto getSlotCount() const -> TYPE_OF_CONTAINERSLOTS;

    /**
     * @brief Gets the items map.
     * 
     * @return Const reference to slot->Item map
     */
    [[nodiscard]] inline auto getItems() const -> const ITEMMAP & { return items; }
    
    /**
     * @brief Gets the nested containers map.
     * 
     * @return Const reference to slot->Container* map
     */
    [[nodiscard]] inline auto getContainers() const -> const CONTAINERMAP & { return containers; }

    /**
     * @brief Finds the first available empty slot.
     * 
     * @return Slot number of first free slot, or getSlotCount() if full
     */
    [[nodiscard]] auto getFirstFreeSlot() const -> TYPE_OF_CONTAINERSLOTS;


    /**
     * @brief Checks if this container is a depot chest.
     * 
     * @return true if itemId matches any of the DEPOTITEMS constants
     */
    [[nodiscard]] inline auto isDepot() const -> bool {

        for (const auto& DEPOTITEM : DEPOTITEMS) {
            if (itemId == DEPOTITEM) {
                return true;
            }
        }
        return false;
    }

private:
    static constexpr auto maximumRecursionDepth = 100; ///< Maximum nesting depth to prevent infinite recursion

    /**
     * @brief Inserts an item into the first available slot.
     * 
     * @param item Item to insert
     */
    void insertIntoFirstFreeSlot(Item &item);
    
    /**
     * @brief Inserts a container into the first available slot.
     * 
     * @param item Container item
     * @param container Container pointer
     */
    void insertIntoFirstFreeSlot(Item &item, Container *container);
    
    /**
     * @brief Calculates weight recursively with depth tracking.
     * 
     * @param rekt Current recursion depth
     * @return Total weight
     * @throws RecursionException if maximumRecursionDepth exceeded
     */
    auto recursiveWeight(int rekt) -> int;

    Item::id_type itemId{}; ///< The item type ID of this container
    ITEMMAP items; ///< Map of slot positions to items
    CONTAINERMAP containers; ///< Map of slot positions to nested containers
};

#endif
