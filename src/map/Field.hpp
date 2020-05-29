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


#ifndef _FIELD_HPP_
#define _FIELD_HPP_

#include <vector>
#include <sys/socket.h>

#include "constants.hpp"
#include "globals.hpp"
#include "Container.hpp"
#include "Item.hpp"

namespace map {

class Field {
private:
    static const uint16_t TRANSPARENT = 0;

    uint16_t tile = 0;
    uint16_t music = 0;
    uint8_t flags = 0;
    position here;
    position warptarget;
    std::vector<Item> items;
    bool persistent = false;

public:
    Container::CONTAINERMAP containers;

public:
    explicit Field(const position &here): here(here) {};
    Field(uint16_t tile, uint16_t music, const position &here, bool persistent = false);
    Field(const Field &) = delete;
    auto operator=(const Field &) -> Field & = delete;
    Field(Field &&) = default;
    auto operator=(Field &&) -> Field & = default;

    void setTileId(uint16_t id);
    auto getTileId() const -> uint16_t;
    auto getSecondaryTileId() const -> uint16_t;
    auto getTileCode() const -> uint16_t;
    auto isTransparent() const -> bool;

    void setMusicId(uint16_t id);
    auto getMusicId() const -> uint16_t;

    auto isWalkable() const -> bool;
    auto moveToPossible() const -> bool;
    auto getMovementCost() const -> TYPE_OF_WALKINGCOST;
    auto hasSpecialItem() const -> bool;

    auto addItemOnStack(const Item &item) -> bool;
    auto addItemOnStackIfWalkable(const Item &item) -> bool;
    auto takeItemFromStack(Item &item) -> bool;
    auto increaseItemOnStack(int count, bool &erased) -> int;
    auto swapItemOnStack(TYPE_OF_ITEM_ID newId, uint16_t newQuality = 0) -> bool;
    auto viewItemOnStack(Item &item) const -> bool;
    auto getStackItem(uint8_t pos) const -> ScriptItem;
    auto getItemStack() const -> const std::vector<Item> &;
    auto itemCount() const -> MAXCOUNTTYPE;

    auto addContainerOnStackIfWalkable(Item item, Container *container) -> bool;
    auto addContainerOnStack(Item item, Container *container) -> bool;

    void age();

    void setPlayer();
    void setNPC();
    void setMonster();
    void removePlayer();
    void removeNPC();
    void removeMonster();
    auto hasPlayer() const -> bool;
    auto hasNPC() const -> bool;
    auto hasMonster() const -> bool;
    void setChar();
    void removeChar();

    void setWarp(const position &pos);
    void removeWarp();
    void getWarp(position &pos) const;
    auto isWarp() const -> bool;

    auto getExportItems() const -> std::vector<Item>;
    void save(std::ofstream &mapStream, std::ofstream &itemStream,
            std::ofstream &warpStream, std::ofstream &containerStream) const;
    void load(std::ifstream &mapStream, std::ifstream &itemStream,
            std::ifstream &warpStream, std::ifstream &containerStream);

    auto getPosition() const -> const position &;

    void makePersistent();
    void removePersistence();
    auto isPersistent() const -> bool;

private:
    void updateFlags();
    inline void setBits(uint8_t);
    inline void unsetBits(uint8_t);
    inline auto anyBitSet(uint8_t) const -> bool;

    void insertIntoDatabase() const noexcept;
    void removeFromDatabase() const noexcept;
    void updateDatabaseField() const noexcept;
    void updateDatabaseItems() const noexcept;
    void updateDatabaseWarp() const noexcept;
    void loadDatabaseWarp() noexcept;
    void loadDatabaseItems() noexcept;
};

void updateFieldToPlayersInScreen(const position &pos);

}

#endif