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

#ifndef FIELD_HPP
#define FIELD_HPP

#include "Container.hpp"
#include "Item.hpp"
#include "constants.hpp"
#include "globals.hpp"

#include <vector>

namespace map {

/**
 * @brief Represents a single tile on the game world map.
 *
 * A Field contains all data for one map coordinate including:
 * - Tile graphics (primary, secondary, and overlay layers)
 * - Background music ID
 * - Items stacked on the ground
 * - Containers placed on the field
 * - Occupancy flags (player, NPC, monster)
 * - Warp destination (for teleport tiles)
 * - Persistence flag for database synchronization
 *
 * Field manages:
 * - Item stacking (up to MAXITEMS)
 * - Walkability calculation based on tiles and items
 * - Character occupation tracking for pathfinding
 * - Database persistence for dynamic changes
 * - Container hierarchy for nested storage
 *
 * Tile encoding uses bit-packing:
 * - Bits 0-4: Primary tile ID (5 bits)
 * - Bits 5-9: Secondary tile ID (5 bits)
 * - Bits 10-15: Overlay tile ID (6 bits)
 *
 * @note Thread safety: Fields should only be accessed through Map locks
 * @see Map for field access and iteration
 * @see Item for items placed on fields
 */
class Field {
private:
    static constexpr uint16_t TRANSPARENT = 0; ///< Tile ID for transparent/empty tiles
    static constexpr uint16_t tileIdBits = 10; ///< Bit shift for overlay tile extraction
    static constexpr uint16_t primaryTileIdBits = 5; ///< Bit shift for secondary tile extraction
    static constexpr uint16_t overlayTileBitMask = 0b1111'1100'0000'0000; ///< Mask for overlay tile (bits 10-15)
    static constexpr uint16_t secondaryTileBitMask = 0b0000'0011'1110'0000; ///< Mask for secondary tile (bits 5-9)
    static constexpr uint16_t primaryTileBitMask = 0b0000'0000'0001'1111; ///< Mask for primary tile (bits 0-4)

    uint16_t tile = 0; ///< Packed tile code (primary + secondary + overlay)
    uint16_t music = 0; ///< Background music ID for this field
    uint8_t flags = 0; ///< Occupancy and property flags (walkability, characters, special items)
    position here; ///< 3D coordinates of this field
    position warptarget{}; ///< Destination coordinates if this is a warp field
    std::vector<Item> items; ///< Items stacked on ground (bottom to top)
    bool persistent = false; ///< If true, changes are saved to database

public:
    Container::CONTAINERMAP containers; ///< Containers placed on this field (indexed by item stack position)

    /**
     * @brief Constructs field with just position (for temporary fields).
     * @param here Position in world coordinates
     */
    explicit Field(const position &here) : here(here){};

    /**
     * @brief Constructs field with full initialization.
     * @param tile Packed tile code (or primary tile ID)
     * @param music Background music ID
     * @param here Position in world coordinates
     * @param persistent If true, load from database and save changes
     */
    Field(uint16_t tile, uint16_t music, const position &here, bool persistent = false);
    Field(const Field &) = delete;
    auto operator=(const Field &) -> Field & = delete;
    Field(Field &&) noexcept = default;
    auto operator=(Field &&) -> Field & = default;
    ~Field() = default;

    /**
     * @brief Sets primary tile ID and updates database/clients.
     * @param id New tile ID from tiles table
     */
    void setTileId(uint16_t id);

    /**
     * @brief Returns primary or overlay tile ID based on layering.
     * @return Effective visible tile ID for rendering
     */
    [[nodiscard]] auto getTileId() const -> uint16_t;

    /**
     * @brief Returns secondary or primary tile ID for multi-layer tiles.
     * @return Secondary layer tile ID, or primary if no overlay
     */
    [[nodiscard]] auto getSecondaryTileId() const -> uint16_t;

    /**
     * @brief Returns raw packed tile code.
     * @return Full 16-bit tile encoding
     */
    [[nodiscard]] auto getTileCode() const -> uint16_t;

    /**
     * @brief Checks if field is transparent (empty space).
     * @return True if tile ID is 0 (no ground)
     */
    [[nodiscard]] auto isTransparent() const -> bool;

    /**
     * @brief Sets background music ID and updates database/clients.
     * @param id Music track ID from content tables
     */
    void setMusicId(uint16_t id);

    /**
     * @brief Returns background music ID for this field.
     * @return Music track ID
     */
    [[nodiscard]] auto getMusicId() const -> uint16_t;

    /**
     * @brief Checks if characters can walk through this field.
     * @return True if tile and items allow movement
     *
     * @note Considers tile properties, item blocking, and FLAG_MAKEPASSABLE
     */
    [[nodiscard]] auto isWalkable() const -> bool;

    /**
     * @brief Checks if movement to this field is allowed (walkable + unoccupied).
     * @return True if field is walkable and no character occupies it
     */
    [[nodiscard]] auto moveToPossible() const -> bool;

    /**
     * @brief Calculates movement cost for pathfinding.
     * @return Walking cost (lower = faster), or max value if unwalkable
     *
     * Uses minimum walking cost from primary and secondary tiles.
     */
    [[nodiscard]] auto getMovementCost() const -> TYPE_OF_WALKINGCOST;

    /**
     * @brief Checks if field contains items with special scripted behavior.
     * @return True if any item has FLAG_SPECIALITEM
     */
    [[nodiscard]] auto hasSpecialItem() const -> bool;

    /**
     * @brief Adds item to top of stack.
     * @param item Item to add
     * @return True if added successfully, false if stack full (>= MAXITEMS)
     */
    auto addItemOnStack(const Item &item) -> bool;

    /**
     * @brief Adds item only if field remains walkable after placement.
     * @param item Item to add
     * @return True if added and field still walkable
     */
    auto addItemOnStackIfWalkable(const Item &item) -> bool;

    /**
     * @brief Removes and returns top item from stack.
     * @param item Output parameter receiving the removed item
     * @return True if item removed, false if stack empty
     */
    auto takeItemFromStack(Item &item) -> bool;

    /**
     * @brief Increases quantity of top item on stack.
     * @param count Number to add to item's number field
     * @param erased Output: true if item was removed due to reaching 0 or overflow
     * @return Actual count added before removal/overflow
     */
    auto increaseItemOnStack(int count, bool &erased) -> int;

    /**
     * @brief Replaces top item with different item ID.
     * @param newId New item ID from items table
     * @param newQuality Optional quality override (0 = keep current)
     * @return True if swapped, false if stack empty
     */
    auto swapItemOnStack(TYPE_OF_ITEM_ID newId, uint16_t newQuality = 0) -> bool;

    /**
     * @brief Retrieves top item without removing it.
     * @param item Output parameter receiving item copy
     * @return True if item exists, false if stack empty
     */
    auto viewItemOnStack(Item &item) const -> bool;

    /**
     * @brief Gets item at specific position in stack for scripts.
     * @param pos Stack index (0 = bottom)
     * @return ScriptItem wrapper with position metadata
     */
    [[nodiscard]] auto getStackItem(uint8_t pos) const -> ScriptItem;

    /**
     * @brief Returns const reference to entire item stack.
     * @return Vector of items from bottom to top
     */
    [[nodiscard]] auto getItemStack() const -> const std::vector<Item> &;

    /**
     * @brief Returns number of items on field.
     * @return Item count (0-MAXITEMS)
     */
    [[nodiscard]] auto itemCount() const -> MAXCOUNTTYPE;

    /**
     * @brief Adds container item only if field remains walkable.
     * @param item Container item to add
     * @param container Pointer to Container object for nested storage
     * @return True if added and field still walkable
     */
    auto addContainerOnStackIfWalkable(Item item, Container *container) -> bool;

    /**
     * @brief Adds container item to field.
     * @param item Container item to add
     * @param container Pointer to Container object for nested storage
     * @return True if added successfully
     */
    auto addContainerOnStack(Item item, Container *container) -> bool;

    /**
     * @brief Retrieves container at specific stack position.
     * @param count Stack index
     * @return Pointer to Container, or nullptr if position invalid or not a container
     */
    [[nodiscard]] auto GetContainer(MAXCOUNTTYPE count) const -> Container *;

    /**
     * @brief Ages items on field (for decay/spoilage systems).
     *
     * Called periodically to update item durability and trigger decay scripts.
     */
    void age();

    /**
     * @brief Marks field as occupied by a player.
     */
    void setPlayer();

    /**
     * @brief Marks field as occupied by an NPC.
     */
    void setNPC();

    /**
     * @brief Marks field as occupied by a monster.
     */
    void setMonster();

    /**
     * @brief Removes player occupation flag.
     */
    void removePlayer();

    /**
     * @brief Removes NPC occupation flag.
     */
    void removeNPC();

    /**
     * @brief Removes monster occupation flag.
     */
    void removeMonster();

    /**
     * @brief Checks if a player occupies this field.
     * @return True if FLAG_PLAYERONFIELD is set
     */
    [[nodiscard]] auto hasPlayer() const -> bool;

    /**
     * @brief Checks if an NPC occupies this field.
     * @return True if FLAG_NPCONFIELD is set
     */
    [[nodiscard]] auto hasNPC() const -> bool;

    /**
     * @brief Checks if a monster occupies this field.
     * @return True if FLAG_MONSTERONFIELD is set
     */
    [[nodiscard]] auto hasMonster() const -> bool;

    /**
     * @brief Marks field as occupied by any character type.
     */
    void setChar();

    /**
     * @brief Removes all character occupation flags.
     */
    void removeChar();

    /**
     * @brief Sets this field as a warp/teleport tile.
     * @param pos Destination coordinates for teleportation
     */
    void setWarp(const position &pos);

    /**
     * @brief Removes warp destination from field.
     */
    void removeWarp();

    /**
     * @brief Retrieves warp destination coordinates.
     * @param pos Output parameter receiving warp target position
     */
    void getWarp(position &pos) const;

    /**
     * @brief Checks if field is a warp/teleport tile.
     * @return True if FLAG_WARPFIELD is set
     */
    [[nodiscard]] auto isWarp() const -> bool;

    /**
     * @brief Returns items for world export/backup.
     * @return Copy of items suitable for serialization
     */
    [[nodiscard]] auto getExportItems() const -> std::vector<Item>;

    /**
     * @brief Saves field state to binary file streams.
     * @param mapStream Map tile data output
     * @param itemStream Item stack data output
     * @param warpStream Warp destinations output
     * @param containerStream Container hierarchy output
     */
    void save(std::ofstream &mapStream, std::ofstream &itemStream, std::ofstream &warpStream,
              std::ofstream &containerStream) const;

    /**
     * @brief Loads field state from binary file streams.
     * @param mapStream Map tile data input
     * @param itemStream Item stack data input
     * @param warpStream Warp destinations input
     * @param containerStream Container hierarchy input
     */
    void load(std::ifstream &mapStream, std::ifstream &itemStream, std::ifstream &warpStream,
              std::ifstream &containerStream);

    /**
     * @brief Returns field's world coordinates.
     * @return Const reference to position
     */
    [[nodiscard]] auto getPosition() const -> const position &;

    /**
     * @brief Enables database persistence for this field.
     *
     * Future changes will be saved to database automatically.
     */
    void makePersistent();

    /**
     * @brief Disables database persistence and removes from database.
     */
    void removePersistence();

    /**
     * @brief Checks if field changes are persisted to database.
     * @return True if persistent flag set
     */
    [[nodiscard]] auto isPersistent() const -> bool;

private:
    /**
     * @brief Recalculates walkability and special flags from tiles and items.
     */
    void updateFlags();

    /**
     * @brief Sets specific flag bits.
     * @param bits Bitmask of flags to enable
     */
    inline void setBits(uint8_t /*bits*/);

    /**
     * @brief Clears specific flag bits.
     * @param bits Bitmask of flags to disable
     */
    inline void unsetBits(uint8_t /*bits*/);

    /**
     * @brief Checks if any of specified flag bits are set.
     * @param bits Bitmask to test
     * @return True if any bit in mask is set
     */
    [[nodiscard]] inline auto anyBitSet(uint8_t /*bits*/) const -> bool;

    void insertIntoDatabase() const noexcept; ///< Adds new field to database
    void removeFromDatabase() const noexcept; ///< Deletes field from database
    void updateDatabaseField() const noexcept; ///< Updates tile and music in database
    void updateDatabaseItems() const noexcept; ///< Syncs item stack to database
    void updateDatabaseWarp() const noexcept; ///< Syncs warp destination to database
    void loadDatabaseWarp() noexcept; ///< Loads warp data from database
    void loadDatabaseItems() noexcept; ///< Loads items and containers from database
};

/**
 * @brief Sends field update to all players who can see it.
 * @param pos Position of field to update
 *
 * Notifies clients about tile, item, or character changes on the field.
 */
void updateFieldToPlayersInScreen(const position &pos);

} // namespace map

#endif
