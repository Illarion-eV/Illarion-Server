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


#include "Map.hpp"

#include <vector>

#include "Logger.hpp"
#include "World.hpp"
#include "Player.hpp"

#include "netinterface/protocol/ServerCommands.hpp"

Map::Map(position origin, uint16_t width, uint16_t height)
    : origin(std::move(origin)), width(width), height(height),
      fields(width, std::vector<Field>(height, Field())) {}


Field &Map::at(int16_t x, int16_t y) {
    return fields[Conv_X_Koord(x)][Conv_Y_Koord(y)];
}

Field &Map::at(const MapPosition &pos) {
    return at(pos.x, pos.y);
}

Field &Map::walkableNear(int16_t &x, int16_t &y) {
    const int16_t r = 5;

    for (int16_t testX = x-r; testX <= x+r; ++testX) {   
        for (int16_t testY = y-r; testY <= y+r; ++testY) {
            Field &field = at(testX, testY);

            if (field.moveToPossible()) {
                x = testX;
                y = testY;
                return field;
            }
        }
    }

    throw FieldNotFound();
}

bool Map::Save(const std::string &name) const {
    Logger::debug(LogFacility::World) << "Saving map " << name << Log::end;

    std::ofstream map { (name + "_map").c_str(), std::ios::binary | std::ios::out };
    std::ofstream items { (name + "_item").c_str(), std::ios::binary | std::ios::out };
    std::ofstream warps { (name + "_warp").c_str(), std::ios::binary | std::ios::out };
    std::ofstream containers { (name + "_container").c_str(), std::ios::binary | std::ios::out };

    if (map.good() && items.good() && warps.good() && containers.good()) {
        map.write((char *) & width, sizeof(width));
        map.write((char *) & height, sizeof(height));
        map.write((char *) & origin, sizeof(origin));

        for (const auto &column : fields) {
            for (const auto &field : column) {
                field.save(map, items, warps, containers);
            }
        }

        return true;

    } else {
        Logger::error(LogFacility::World) << "Saving map failed: " << name << Log::end;
        return false;
    }
}

bool Map::Load(const std::string &name) {
    Logger::debug(LogFacility::World) << "Loading map " << name << Log::end;

    std::ifstream map { (name + "_map").c_str(), std::ios::binary | std::ios::in };
    std::ifstream items { (name + "_item").c_str(), std::ios::binary | std::ios::in };
    std::ifstream warps { (name + "_warp").c_str(), std::ios::binary | std::ios::in };
    std::ifstream containers { (name + "_container").c_str(), std::ios::binary | std::ios::in };

    if (map.good() && items.good() && warps.good() && containers.good()) {
        int16_t newWidth, newHeight;

        map.read((char *) & newWidth, sizeof(width));
        map.read((char *) & newHeight, sizeof(height));
        map.read((char *) & origin, sizeof(origin));

        if (newWidth == width && newHeight == height) {

            for (auto &column : fields) {
                for (auto &field : column) {
                    field.load(map, items, warps, containers);
                    field.updateFlags();
                }
            }

            return true;
        }
    }

    Logger::error(LogFacility::World) << "Map: ERROR LOADING FILES: " << name << Log::end;

    return false;

}


void Map::age() {
    for (int16_t x = 0; x < width; ++x) {
        for (int16_t y = 0; y < height; ++y) {
            int8_t rotstate = fields[x][y].age();

            if (rotstate != 0) {
                position pos(Conv_To_X(x), Conv_To_Y(y), origin.z);
                std::vector<Player *> playersinview = World::get()->Players.findAllCharactersInScreen(pos);

                for (const auto &player : playersinview) {
                    ServerCommandPointer cmd = std::make_shared<ItemUpdate_TC>(pos, fields[x][y].items);
                    player->Connection->addCommand(cmd);
                }
            }
        }
    }
}

uint16_t Map::getHeight() const { return height; }

uint16_t Map::getWidth() const { return width; }

int16_t Map::getMinX() const { return origin.x; }

int16_t Map::getMinY() const { return origin.y; }

int16_t Map::getMaxX() const { return origin.x + width - 1; }

int16_t Map::getMaxY() const { return origin.y + height - 1; }

int16_t Map::getLevel() const { return origin.z; }

inline uint16_t Map::Conv_X_Koord(int16_t x) const {
    uint16_t temp = x - origin.x;

    if (temp >= width) {
        throw FieldNotFound();
    }

    return temp;
}

inline uint16_t Map::Conv_Y_Koord(int16_t y) const {
    uint16_t temp = y - origin.y;

    if (temp >= height) {
        throw FieldNotFound();
    }

    return temp;
}

inline int16_t Map::Conv_To_X(uint16_t x) const {
    return x + origin.x;
}

inline int16_t Map::Conv_To_Y(uint16_t y) const {
    return y + origin.y;
}

bool Map::intersects(const position &origin2, uint16_t width,
                     uint16_t height) const {
    return origin2.z == origin.z && getMaxX() >= origin2.x &&
           origin.x <= origin2.x + width - 1 && getMaxY() >= origin2.y &&
           origin.y <= origin2.y + height - 1;
}

bool Map::intersects(const Map &map) const {
    return map.origin.z == origin.z && getMaxX() >= map.origin.x &&
           origin.x <= map.getMaxX() && getMaxY() >= map.origin.y &&
           origin.y <= map.getMaxY();
}

