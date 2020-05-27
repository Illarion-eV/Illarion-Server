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


#include "map/Field.hpp"

#include <algorithm>
#include <range/v3/all.hpp>

#include "data/Data.hpp"
#include "globals.hpp"
#include "netinterface/protocol/ServerCommands.hpp"
#include "World.hpp"
#include "Player.hpp"

#include "db/ConnectionManager.hpp"
#include "db/Connection.hpp"
#include "db/DeleteQuery.hpp"
#include "db/InsertQuery.hpp"
#include "db/SelectQuery.hpp"
#include "db/UpdateQuery.hpp"
#include "db/Result.hpp"

namespace map {

void Field::setTileId(uint16_t id) {
    tile = id;
    updateDatabaseField();
    updateFlags();
}

uint16_t Field::getTileCode() const {
    return tile;
}

uint16_t Field::getTileId() const {
    if (((tile & 0xFC00) >> 10) > 0) {
        return tile & 0x001F;
    } else {
        return tile;
    }
}

uint16_t Field::getSecondaryTileId() const {
    if (((tile & 0xFC00) >> 10) > 0) {
        return (tile & 0x03E0) >> 5;
    } else {
        return tile;
    }
}

void Field::setMusicId(uint16_t id) {
    music = id;
    updateDatabaseField();
}

uint16_t Field::getMusicId() const {
    return music;
}

bool Field::isTransparent() const {
    return getTileId() == TRANSPARENT;
}

TYPE_OF_WALKINGCOST Field::getMovementCost() const {
    if (isWalkable()) {
        auto tileId = getTileId();
        const auto &primaryTile = Data::Tiles[tileId];
        TYPE_OF_WALKINGCOST tileWalkingCost = primaryTile.walkingCost;

        tileId = getSecondaryTileId();
        const auto &secondaryTile = Data::Tiles[tileId];
        uint16_t secondaryWalkingCost = secondaryTile.walkingCost;

        if (secondaryWalkingCost < tileWalkingCost) {
            tileWalkingCost = secondaryWalkingCost;
        }

        return tileWalkingCost;
    } else {
        return std::numeric_limits<TYPE_OF_WALKINGCOST>::max();
    }
}

ScriptItem Field::getStackItem(uint8_t pos) const {
    if (pos < items.size()) {
        ScriptItem result(items.at(pos));
        result.type = ScriptItem::it_field;
        result.itempos = pos;
        return result;
    }

    return {};
}

const std::vector<Item> &Field::getItemStack() const {
    return items;
}

bool Field::addItemOnStack(const Item &item) {
    if (items.size() < MAXITEMS) {
        items.push_back(item);
        updateDatabaseItems();
        updateFlags();

        return true;
    }

    return false;
}


bool Field::addItemOnStackIfWalkable(const Item &item) {
    if (isWalkable()) {
        return addItemOnStack(item);
    }

    return false;
}


bool Field::takeItemFromStack(Item &item) {
    if (items.empty()) {
        return false;
    }

    item = items.back();
    items.pop_back();
    updateDatabaseItems();
    updateFlags();

    return true;
}


int Field::increaseItemOnStack(int count, bool &erased) {
    if (items.empty()) {
        return false;
    }
    
    Item &item = items.back();
    count += item.getNumber();
    auto maxStack = item.getMaxStack();

    if (count > maxStack) {
        item.setNumber(maxStack);
        count -= maxStack;
        erased = false;
    } else if (count <= 0) {
        items.pop_back();
        updateFlags();
        erased = true;
    } else {
        item.setNumber(count);
        count = 0;
        erased = false;
    }

    updateDatabaseItems();
    return count;
}


bool Field::swapItemOnStack(TYPE_OF_ITEM_ID newId, uint16_t newQuality) {
    if (items.empty()) {
        return false;
    }

    Item &item = items.back();
    item.setId(newId);

    if (newQuality > 0) {
        item.setQuality(newQuality);
    }

    const auto &itemStruct = Data::Items[newId];

    if (itemStruct.isValid()) {
        item.setWear(itemStruct.AgeingSpeed);
    }

    updateDatabaseItems();
    updateFlags();
    return true;
}


bool Field::viewItemOnStack(Item &item) const {
    if (items.empty()) {
        return false;
    }

    item = items.back();

    return true;
}


MAXCOUNTTYPE Field::itemCount() const {
    return items.size();
}


bool Field::addContainerOnStackIfWalkable(Item item, Container *container) {
    if (isWalkable()) {
        if (items.size() < MAXITEMS - 1) {
            if (item.isContainer()) {
                MAXCOUNTTYPE count = 0;

                auto iterat = containers.find(count);

                while ((iterat != containers.end()) && (count < (MAXITEMS - 2))) {
                    ++count; 
                    iterat = containers.find(count);
                }

                if (count < MAXITEMS - 1) {
                    if (!container) {
                        container = new Container(item.getId());
                    }

                    containers.insert(iterat, Container::CONTAINERMAP::value_type(count, container));
                } else {
                    return false;
                }

                item.setNumber(count);

                if (!addItemOnStackIfWalkable(item)) {
                    containers.erase(count);
                } else {
                    return true;
                }
            }
        }
    }

    return false;
}

bool Field::addContainerOnStack(Item item, Container *container) {
    if (item.isContainer()) {
        MAXCOUNTTYPE count = 0;

        auto iterat = containers.find(count);

        while ((iterat != containers.end()) && (count < MAXITEMS - 2)) {
            ++count;
            iterat = containers.find(count);
        }

        if (count < MAXITEMS - 1) {
            if (!container) {
                container = new Container(item.getId());
            }

            containers.insert(iterat, Container::CONTAINERMAP::value_type(count, container));
        } else {
            return false;
        }

        item.setNumber(count);

        if (!addItemOnStack(item)) {
            containers.erase(count);
        } else {
            return true;
        }
    }

    return false;
}

void Field::save(std::ofstream &mapStream, std::ofstream &itemStream,
                 std::ofstream &warpStream,
                 std::ofstream &containerStream) const {

    mapStream.write((char *) & tile, sizeof(tile));
    mapStream.write((char *) & music, sizeof(music));
    mapStream.write((char *) & flags, sizeof(flags));

    uint8_t itemsSize = items.size();
    itemStream.write((char *) & itemsSize, sizeof(itemsSize));

    for (const auto &item : items) {
        item.save(itemStream);
    }

    if (isWarp()) {
        char b = 1;
        warpStream.write((char *) & b, sizeof(b));
        warpStream.write((char *) & warptarget, sizeof(warptarget));
    } else {
        char b = 0;
        warpStream.write((char *) & b, sizeof(b));
    }

    uint8_t containersSize = containers.size();
    containerStream.write((char *) & containersSize, sizeof(containersSize));
    
    for (const auto &container : containers) {
        containerStream.write((char *) & container.first, sizeof(container.first));
        container.second->Save(containerStream);
    }
}


std::vector<Item> Field::getExportItems() const {
    std::vector<Item> result;

    for (const auto &item : items) {
        if (item.isPermanent()) {
            result.push_back(item);
        } else {
            const auto &itemStruct = Data::Items[item.getId()];

            if (itemStruct.isValid() && itemStruct.AfterInfiniteRot > 0) {
                Item rottenItem = item;
                rottenItem.setId(itemStruct.AfterInfiniteRot);
                rottenItem.makePermanent();
                result.push_back(rottenItem);
            }
        }
    }

    return result;
}

void Field::load(std::ifstream &mapStream, std::ifstream &itemStream,
                 std::ifstream &warpStream, std::ifstream &containerStream) {

    mapStream.read((char *) & tile, sizeof(tile));
    mapStream.read((char *) & music, sizeof(music));
    mapStream.read((char *) & flags, sizeof(flags));

    unsetBits(FLAG_NPCONFIELD | FLAG_MONSTERONFIELD | FLAG_PLAYERONFIELD);

    MAXCOUNTTYPE size;
    itemStream.read((char *) & size, sizeof(size));

    items.clear();

    for (int i = 0; i < size; ++i) {
        Item item;
        item.load(itemStream);
        items.push_back(item);
    }

    char isWarp = 0;
    warpStream.read((char *) & isWarp, sizeof(isWarp));

    if (isWarp == 1) {
        position target;
        warpStream.read((char *) & target, sizeof(warptarget));
        setWarp(target);
    }

    containerStream.read((char *) & size, sizeof(size));

    for (auto &container : containers) {
        delete container.second;
        container.second = nullptr;
    }

    containers.clear();

    for (int i = 0; i < size; ++i) {
        MAXCOUNTTYPE key;
        containerStream.read((char *) & key, sizeof(key));
        
        for (const auto &item : items) {

            if (item.isContainer() && item.getNumber() == key) {
                auto container = new Container(item.getId());
                container->Load(containerStream);
                containers.insert(decltype(containers)::value_type(key, container));
            }
        }
    }

    updateFlags();
}

const position &Field::getPosition() const {
    return here;
}

void Field::makePersistent() {
    if (!isPersistent()) {
        persistent = true;
        insertIntoDatabase();
        updateDatabaseItems();
        updateDatabaseWarp();
    }
}

void Field::removePersistence() {
    if (isPersistent()) {
        persistent = false;
        removeFromDatabase();
    }
}

bool Field::isPersistent() const {
    return persistent;
}

void Field::age() {
    for (const auto &container : containers) {
        if (container.second) {
            container.second->doAge();
        }
    }

    if (!items.empty()) {
        auto it = items.begin();
        bool refreshItems = false;

        while (it < items.end()) {
            Item &item = *it;

            if (!item.survivesAgeing()) {
                const auto &itemStruct = Data::Items[item.getId()];
                refreshItems = true;

                if (itemStruct.isValid() && item.getId() != itemStruct.ObjectAfterRot) {
                    item.setId(itemStruct.ObjectAfterRot);

                    const auto &afterRotItemStruct = Data::Items[itemStruct.ObjectAfterRot];

                    if (afterRotItemStruct.isValid()) {
                        item.setWear(afterRotItemStruct.AgeingSpeed);
                    }

                    ++it;
                } else {
                    if (item.isContainer()) {
                        auto iterat = containers.find(item.getNumber());

                        if (iterat != containers.end()) {
                            delete iterat->second;
                            containers.erase(iterat);
                        }
                    }

                    it = items.erase(it);
                }
            } else {
                ++it;
            }
        }

        if (refreshItems) {
            std::vector<Player *> playersinview = World::get()->Players.findAllCharactersInScreen(here);

            for (const auto &player : playersinview) {
                ServerCommandPointer cmd = std::make_shared<ItemUpdate_TC>(here, getItemStack());
                player->Connection->addCommand(cmd);
            }

            updateDatabaseItems();
        }
    }

    updateFlags();
}

void Field::updateFlags() {

    unsetBits(FLAG_SPECIALITEM | FLAG_BLOCKPATH | FLAG_MAKEPASSABLE);

    if (Data::Tiles.exists(tile)) {
        const TilesStruct &tt = Data::Tiles[tile];
        setBits(tt.flags & FLAG_BLOCKPATH);
    }

    for (const auto &item : items) {
        if (Data::TilesModItems.exists(item.getId())) {
            const auto &mod = Data::TilesModItems[item.getId()];
            setBits(mod.Modificator & FLAG_SPECIALITEM);

            if (mod.Modificator & FLAG_MAKEPASSABLE) {
                unsetBits(FLAG_BLOCKPATH);
                setBits(FLAG_MAKEPASSABLE);
            } else if (mod.Modificator & FLAG_BLOCKPATH) {
                unsetBits(FLAG_MAKEPASSABLE);
                setBits(FLAG_BLOCKPATH);
            }
        }
    }
}

bool Field::hasMonster() const {
    return anyBitSet(FLAG_MONSTERONFIELD);
}

void Field::setMonster() {
    setBits(FLAG_MONSTERONFIELD);
}

void Field::removeMonster() {
    unsetBits(FLAG_MONSTERONFIELD);
}

bool Field::hasNPC() const {
    return anyBitSet(FLAG_NPCONFIELD);
}

void Field::setNPC() {
    setBits(FLAG_NPCONFIELD);
}

void Field::removeNPC() {
    unsetBits(FLAG_NPCONFIELD);
}

bool Field::hasPlayer() const {
    return anyBitSet(FLAG_PLAYERONFIELD);
}

void Field::setPlayer() {
    setBits(FLAG_PLAYERONFIELD);
}

void Field::removePlayer() {
    unsetBits(FLAG_PLAYERONFIELD);
}

bool Field::isWarp() const {
    return anyBitSet(FLAG_WARPFIELD);
}


void Field::setWarp(const position &pos) {
    warptarget = pos;
    setBits(FLAG_WARPFIELD);
    updateDatabaseWarp();
}


void Field::removeWarp() {
    unsetBits(FLAG_WARPFIELD);
    updateDatabaseWarp();
}


void Field::getWarp(position &pos) const {
    pos = warptarget;
}


bool Field::hasSpecialItem() const {
    return anyBitSet(FLAG_SPECIALITEM);
}


bool Field::isWalkable() const {
    return !anyBitSet(FLAG_BLOCKPATH) || anyBitSet(FLAG_MAKEPASSABLE);
}


bool Field::moveToPossible() const {
    return isWalkable() && !anyBitSet(FLAG_MONSTERONFIELD | FLAG_NPCONFIELD | FLAG_PLAYERONFIELD);
}

void Field::setChar() {
    setBits(FLAG_PLAYERONFIELD);
}

void Field::removeChar() {
    unsetBits(FLAG_PLAYERONFIELD);
}

inline void Field::setBits(uint8_t bits) {
    flags |= bits;
}

inline void Field::unsetBits(uint8_t bits) {
    flags &= ~bits;
}

inline bool Field::anyBitSet(uint8_t bits) const {
    return (flags & bits) != 0;
}

void Field::insertIntoDatabase() const noexcept {
    if (!isPersistent()) {
        return;
    }

    using namespace Database;
    auto connection = ConnectionManager::getInstance().getConnection();

    try {
        connection->beginTransaction();

        InsertQuery fieldQuery(connection);
        const auto xColumn = fieldQuery.addColumn("mt_x");
        const auto yColumn = fieldQuery.addColumn("mt_y");
        const auto zColumn = fieldQuery.addColumn("mt_z");
        const auto tileColumn = fieldQuery.addColumn("mt_tile");
        const auto musicColumn = fieldQuery.addColumn("mt_music");
        fieldQuery.addServerTable("map_tiles");

        fieldQuery.addValue<int16_t>(xColumn, here.x);
        fieldQuery.addValue<int16_t>(yColumn, here.y);
        fieldQuery.addValue<int16_t>(zColumn, here.z);
        fieldQuery.addValue<uint16_t>(tileColumn, tile);
        fieldQuery.addValue<uint16_t>(musicColumn, music);

        fieldQuery.execute();

        connection->commitTransaction();
    } catch (std::exception &e) {
        Logger::error(LogFacility::World) << "Error while inserting field into database: " << e.what() << Log::end;
        connection->rollbackTransaction();
    }
}

void Field::removeFromDatabase() const noexcept {
    if (isPersistent()) {
        return;
    }

    using namespace Database;
    auto connection = ConnectionManager::getInstance().getConnection();

    try {
        connection->beginTransaction();

        DeleteQuery fieldQuery(connection);
        fieldQuery.addEqualCondition<int16_t>("map_tiles", "mt_x", here.x);
        fieldQuery.addEqualCondition<int16_t>("map_tiles", "mt_y", here.y);
        fieldQuery.addEqualCondition<int16_t>("map_tiles", "mt_z", here.z);
        fieldQuery.addServerTable("map_tiles");
        fieldQuery.execute();

        connection->commitTransaction();
    } catch (std::exception &e) {
        Logger::error(LogFacility::World) << "Error while deleting field from database: " << e.what() << Log::end;
        connection->rollbackTransaction();
    }
}

void Field::updateDatabaseField() const noexcept {
    if (!isPersistent()) {
        return;
    }

    using namespace Database;
    auto connection = ConnectionManager::getInstance().getConnection();

    try {
        connection->beginTransaction();

        UpdateQuery fieldQuery(connection);
        fieldQuery.addAssignColumn<uint16_t>("mt_tile", tile);
        fieldQuery.addAssignColumn<uint16_t>("mt_music", music);
        fieldQuery.addEqualCondition<int16_t>("map_tiles", "mt_x", here.x);
        fieldQuery.addEqualCondition<int16_t>("map_tiles", "mt_y", here.y);
        fieldQuery.addEqualCondition<int16_t>("map_tiles", "mt_z", here.z);
        fieldQuery.addServerTable("map_tiles");
        fieldQuery.execute();

        connection->commitTransaction();
    } catch (std::exception &e) {
        Logger::error(LogFacility::World) << "Error while updating field in database: " << e.what() << Log::end;
        connection->rollbackTransaction();
    }
}

void Field::updateDatabaseItems() const noexcept {
    if (!isPersistent()) {
        return;
    }

    using namespace Database;
    auto connection = ConnectionManager::getInstance().getConnection();

    try {
        connection->beginTransaction();

        {
            DeleteQuery itemQuery(connection);
            itemQuery.addEqualCondition<int16_t>("map_items", "mi_x", here.x);
            itemQuery.addEqualCondition<int16_t>("map_items", "mi_y", here.y);
            itemQuery.addEqualCondition<int16_t>("map_items", "mi_z", here.z);
            itemQuery.addServerTable("map_items");
            itemQuery.execute();
        }

        if (itemCount() > 0) {
            InsertQuery itemQuery(connection);
            const auto xColumn = itemQuery.addColumn("mi_x");
            const auto yColumn = itemQuery.addColumn("mi_y");
            const auto zColumn = itemQuery.addColumn("mi_z");
            const auto stackPosColumn = itemQuery.addColumn("mi_stack_pos");
            const auto itemColumn = itemQuery.addColumn("mi_item");
            const auto qualityColumn = itemQuery.addColumn("mi_quality");
            const auto numberColumn = itemQuery.addColumn("mi_number");
            const auto wearColumn = itemQuery.addColumn("mi_wear");
            itemQuery.addServerTable("map_items");

            InsertQuery dataQuery(connection);
            const auto xDataColumn = dataQuery.addColumn("mid_x");
            const auto yDataColumn = dataQuery.addColumn("mid_y");
            const auto zDataColumn = dataQuery.addColumn("mid_z");
            const auto stackPosDataColumn = dataQuery.addColumn("mid_stack_pos");
            const auto keyDataColumn = dataQuery.addColumn("mid_key");
            const auto valueDataColumn = dataQuery.addColumn("mid_value");
            dataQuery.addServerTable("map_item_data");

            uint16_t stackPos = 0;
            auto movable = ranges::view::filter([](const Item &item) {return item.isMovable();});

            ranges::for_each(items | movable, [&](const auto &item) {
                itemQuery.addValue<int16_t>(xColumn, here.x);
                itemQuery.addValue<int16_t>(yColumn, here.y);
                itemQuery.addValue<int16_t>(zColumn, here.z);

                itemQuery.addValue<uint16_t>(stackPosColumn, stackPos);
                itemQuery.addValue<TYPE_OF_ITEM_ID>(itemColumn, item.getId());
                itemQuery.addValue<uint16_t>(qualityColumn, item.getQuality());
                itemQuery.addValue<uint16_t>(numberColumn, item.getNumber());
                itemQuery.addValue<uint16_t>(wearColumn, item.getWear());

                std::for_each(item.getDataBegin(), item.getDataEnd(), [&](const auto &data) {
                    dataQuery.addValue<int16_t>(xDataColumn, here.x);
                    dataQuery.addValue<int16_t>(yDataColumn, here.y);
                    dataQuery.addValue<int16_t>(zDataColumn, here.z);
                    dataQuery.addValue<uint16_t>(stackPosDataColumn, stackPos);
                    dataQuery.addValue<std::string>(keyDataColumn, data.first);
                    dataQuery.addValue<std::string>(valueDataColumn, data.second);
                });

                ++stackPos;
            });

            itemQuery.execute();
            dataQuery.execute();
        }

        connection->commitTransaction();
    } catch (std::exception &e) {
        Logger::error(LogFacility::World) << "Error while updating items in database: " << e.what() << Log::end;
        connection->rollbackTransaction();
    }
}

void Field::updateDatabaseWarp() const noexcept {
    if (!isPersistent()) {
        return;
    }

    using namespace Database;
    auto connection = ConnectionManager::getInstance().getConnection();

    try {
        connection->beginTransaction();

        {
            DeleteQuery warpQuery(connection);
            warpQuery.addEqualCondition<int16_t>("map_warps", "mw_start_x", here.x);
            warpQuery.addEqualCondition<int16_t>("map_warps", "mw_start_y", here.y);
            warpQuery.addEqualCondition<int16_t>("map_warps", "mw_start_z", here.z);
            warpQuery.addServerTable("map_warps");
            warpQuery.execute();
        }

        if (isWarp()) {
            InsertQuery warpQuery(connection);
            const auto xStartColumn = warpQuery.addColumn("mw_start_x");
            const auto yStartColumn = warpQuery.addColumn("mw_start_y");
            const auto zStartColumn = warpQuery.addColumn("mw_start_z");
            const auto xTargetColumn = warpQuery.addColumn("mw_target_x");
            const auto yTargetColumn = warpQuery.addColumn("mw_target_y");
            const auto zTargetColumn = warpQuery.addColumn("mw_target_z");

            warpQuery.addServerTable("map_warps");

            warpQuery.addValue<int16_t>(xStartColumn, here.x);
            warpQuery.addValue<int16_t>(yStartColumn, here.y);
            warpQuery.addValue<int16_t>(zStartColumn, here.z);
            warpQuery.addValue<int16_t>(xTargetColumn, warptarget.x);
            warpQuery.addValue<int16_t>(yTargetColumn, warptarget.y);
            warpQuery.addValue<int16_t>(zTargetColumn, warptarget.z);

            warpQuery.execute();
        }

        connection->commitTransaction();
    } catch (std::exception &e) {
        Logger::error(LogFacility::World) << "Error while updating warp in database: " << e.what() << Log::end;
        connection->rollbackTransaction();
    }
}

}
