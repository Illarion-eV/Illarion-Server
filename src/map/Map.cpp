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


#include "map/Map.hpp"

#include <regex>
#include <vector>
#include <boost/algorithm/string/replace.hpp>
#include <boost/lexical_cast.hpp>

#include "Logger.hpp"

namespace map {

Map::Map(std::string name, position origin, uint16_t width, uint16_t height)
    : origin(origin), width(width), height(height),
      fields(),
      name(std::move(name)) {
    for (auto x = origin.x; x < origin.x + width; ++x) {
        std::vector<Field> column;

        for (auto y = origin.y; y < origin.y + height; ++y) {
            column.emplace_back(position(x, y, origin.z));
        }

        fields.push_back(std::move(column));
    }
}

Map::Map(std::string name, position origin, uint16_t width, uint16_t height,
         uint16_t tile)
    : Map(std::move(name), origin, width, height) {

    for (auto &column : fields) {
        for (auto &field : column) {
            field.setTileId(tile);
        }
    }
}

Field &Map::at(int16_t x, int16_t y) {
    return fields[Conv_X_Koord(x)][Conv_Y_Koord(y)];
}

const Field &Map::at(int16_t x, int16_t y) const {
    return fields[Conv_X_Koord(x)][Conv_Y_Koord(y)];
}

Field &Map::at(const MapPosition &pos) {
    return at(pos.x, pos.y);
}

const Field &Map::at(const MapPosition &pos) const {
    return at(pos.x, pos.y);
}

bool Map::Save(const std::string &name) const {
    Logger::debug(LogFacility::World) << "Saving map " << name << Log::end;

    std::ofstream map { name + "_map", std::ios::binary | std::ios::out };
    std::ofstream items { name + "_item", std::ios::binary | std::ios::out };
    std::ofstream warps { name + "_warp", std::ios::binary | std::ios::out };
    std::ofstream containers { name + "_container", std::ios::binary | std::ios::out };

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

const std::regex headerExpression {R"(^[VLXYWH]: -?\d+$)"};
const std::regex tileExpression {R"(^(\d+);(\d+);(\d+);(\d+)$)"};
const std::regex itemExpression {R"(^(\d+);(\d+);(\d+);(\d+)(;.*)?$)"};
const std::regex warpExpression {R"(^(\d+);(\d+);(-?\d+);(-?\d+);(-?\d+)$)"};
const std::regex dataExpression {R"(;([^\\;=]*(?:\\[\\;=][^\\;=]*)*)=([^\\;=]*(?:\\[\\;=][^\\;=]*)*)(?:;.*)?)"};

bool Map::import(const std::string &importDir, const std::string &mapName) {
    bool success = importFields(importDir, mapName);
    success and_eq importWarps(importDir, mapName);
    success and_eq importItems(importDir, mapName);
    return success;
}

bool Map::importFields(const std::string &importDir,
                       const std::string &mapName) {
    using std::smatch;
    using std::regex_match;

    const std::string fileName = mapName + ".tiles.txt";
    std::ifstream mapFile(importDir + fileName);

    if (!mapFile) {
        Logger::error(LogFacility::Script)
            << "Could not open file: " << fileName << Log::end;
        return false;
    }

    std::string line;
    int lineNumber = 0;
    int headerLinesSkipped = 0;
    bool success = true;

    while (std::getline(mapFile, line)) {
        ++lineNumber;

        if (line.length() != 0 && line[0] != '#') {
            if (headerLinesSkipped < 6 && regex_match(line, headerExpression)) {
                ++headerLinesSkipped;
                continue;
            }

            smatch matches;

            if (regex_match(line, matches, tileExpression)) {
                try {
                    auto x = boost::lexical_cast<uint16_t>(matches[1]);

                    if (x >= width) {
                        Logger::error(LogFacility::Script)
                            << fileName
                            << ": x must be less than width in line "
                            << lineNumber << Log::end;
                        success = false;
                    }

                    auto y = boost::lexical_cast<uint16_t>(matches[2]);

                    if (y >= height) {
                        Logger::error(LogFacility::Script)
                            << fileName
                            << ": y must be less than height in line "
                            << lineNumber << Log::end;
                        success = false;
                    }

                    auto tile = boost::lexical_cast<uint16_t>(matches[3]);
                    auto music = boost::lexical_cast<uint16_t>(matches[4]);

                    if (success) {
                        auto &field = fields[x][y];

                        if (field.getTileCode() || field.getMusicId()) {
                            Logger::warn(LogFacility::Script)
                                << fileName << ": tile on (" << x << ", " << y
                                << ") is already present, ignoring line "
                                << lineNumber << Log::end;
                        } else {
                            field.setTileId(tile);
                            field.setMusicId(music);
                        }
                    }
                } catch (boost::bad_lexical_cast &) {
                    Logger::error(LogFacility::Script)
                        << fileName << ": expected "
                                       "<uint16_t>;<uint16_t>;<uint16_t>;<"
                                       "uint16_t> but found '" << line
                        << "' in line " << lineNumber << Log::end;

                    success = false;
                }
            } else {
                Logger::error(LogFacility::Script)
                    << fileName
                    << ": expected <uint16_t>;<uint16_t>;<uint16_t>;<uint16_t> "
                       "but found '" << line << "' in line " << lineNumber
                    << Log::end;
                success = false;
            }
        }
    }

    return success;
}

bool Map::importItems(const std::string &importDir,
                      const std::string &mapName) {
    using std::smatch;
    using std::regex_match;

    const std::string fileName = mapName + ".items.txt";
    std::ifstream itemFile(importDir + fileName);

    if (!itemFile) {
        Logger::error(LogFacility::Script)
            << "Could not open file: " << fileName << Log::end;
        return false;
    }

    std::string line;
    int lineNumber = 0;
    bool success = true;

    while (std::getline(itemFile, line)) {
        ++lineNumber;

        if (line.length() != 0 && line[0] != '#') {
            smatch matches;

            if (regex_match(line, matches, itemExpression)) {
                try {
                    auto x = boost::lexical_cast<uint16_t>(matches[1]);

                    if (x >= width) {
                        Logger::error(LogFacility::Script)
                            << fileName
                            << ": x must be less than width in line "
                            << lineNumber << Log::end;
                        success = false;
                    }

                    auto y = boost::lexical_cast<uint16_t>(matches[2]);

                    if (y >= height) {
                        Logger::error(LogFacility::Script)
                            << fileName
                            << ": y must be less than height in line "
                            << lineNumber << Log::end;
                        success = false;
                    }

                    auto itemId = boost::lexical_cast<uint16_t>(matches[3]);
                    auto quality = boost::lexical_cast<uint16_t>(matches[4]);

                    if (quality > 999) {
                        Logger::error(LogFacility::Script)
                            << fileName
                            << ": quality must be less than 1000 in line "
                            << lineNumber << Log::end;
                        success = false;
                    }

                    Item item;
                    item.setId(itemId);
                    item.setQuality(quality);
                    item.setNumber(1);
                    item.makePermanent();

                    std::string data = matches[5];

                    while (data.length() > 0) {
                        std::smatch match;

                        if (std::regex_match(data, match, dataExpression)) {
                            std::string key = match[1];
                            std::string value = match[2];

                            if (key.length() == 0) {
                                Logger::error(LogFacility::Script)
                                    << fileName << ": data key must not have "
                                                   "zero length in line "
                                    << lineNumber << Log::end;
                                success = false;
                            }

                            if (value.length() == 0) {
                                Logger::error(LogFacility::Script)
                                    << fileName << ": data value must not have "
                                                   "zero length in line "
                                    << lineNumber << Log::end;
                                success = false;
                            }

                            if (key.length() > 255) {
                                Logger::error(LogFacility::Script)
                                    << fileName << ": data key must not exceed "
                                                   "255 characters in line "
                                    << lineNumber << Log::end;
                                success = false;
                            }

                            if (value.length() > 255) {
                                Logger::error(LogFacility::Script)
                                    << fileName
                                    << ": data value must not exceed 255 "
                                       "characters in line " << lineNumber
                                    << Log::end;
                                success = false;
                            }

                            data.erase(0, key.length() + value.length() + 2);

                            unescape(key);
                            unescape(value);

                            item.setData(key, value);
                        } else {
                            Logger::error(LogFacility::Script)
                                << fileName << ": invalid data sequence '"
                                << data << "' in line " << lineNumber
                                << Log::end;
                            success = false;
                            break;
                        }
                    }

                    if (success) {
                        auto &field = fields[x][y];

                        if (item.isContainer()) {
                            field.addContainerOnStack(item, nullptr);
                        } else {
                            field.addItemOnStack(item);
                        }
                    }
                } catch (boost::bad_lexical_cast &) {
                    Logger::error(LogFacility::Script)
                        << fileName << ": expected "
                                       "<uint16_t>;<uint16_t>;<uint16_t>;<"
                                       "uint16_t> but found '" << line
                        << "' in line " << lineNumber << Log::end;
                    success = false;
                }
            } else {
                Logger::error(LogFacility::Script)
                    << fileName
                    << ": expected "
                       "<uint16_t>;<uint16_t>;<uint16_t>;<uint16_t> "
                       "but found '" << line << "' in line " << lineNumber
                    << Log::end;
                success = false;
            }
        }
    }

    return success;
}

void Map::unescape(std::string &input) {
    using boost::algorithm::replace_all;

    replace_all(input, "\\\\", "\\");
    replace_all(input, "\\=", "=");
    replace_all(input, "\\;", ";");
}

bool Map::importWarps(const std::string &importDir,
                      const std::string &mapName) {
    using std::smatch;
    using std::regex_match;

    const std::string fileName = mapName + ".warps.txt";
    std::ifstream warpFile(importDir + fileName);

    if (!warpFile) {
        Logger::error(LogFacility::Script)
            << "Could not open file: " << fileName << Log::end;
        return false;
    }

    std::string line;
    int lineNumber = 0;
    bool success = true;

    while (std::getline(warpFile, line)) {
        ++lineNumber;

        if (line.length() != 0 && line[0] != '#') {
            smatch matches;

            if (regex_match(line, matches, warpExpression)) {
                try {
                    auto x = boost::lexical_cast<uint16_t>(matches[1]);

                    if (x >= width) {
                        Logger::error(LogFacility::Script)
                            << fileName
                            << ": x must be less than width in line "
                            << lineNumber << Log::end;
                        success = false;
                    }

                    auto y = boost::lexical_cast<uint16_t>(matches[2]);

                    if (y >= height) {
                        Logger::error(LogFacility::Script)
                            << fileName
                            << ": y must be less than height in line "
                            << lineNumber << Log::end;
                        success = false;
                    }

                    position target;
                    target.x = boost::lexical_cast<int16_t>(matches[3]);
                    target.y = boost::lexical_cast<int16_t>(matches[4]);
                    target.z = boost::lexical_cast<int16_t>(matches[5]);

                    if (success) {
                        auto &field = fields[x][y];

                        if (field.isWarp()) {
                            Logger::warn(LogFacility::Script)
                                << fileName << ": warp on (" << x << ", " << y
                                << ") is already present, ignoring line "
                                << lineNumber << Log::end;
                        } else {
                            field.setWarp(target);
                        }
                    }
                } catch (boost::bad_lexical_cast &) {
                    Logger::error(LogFacility::Script)
                        << fileName << ": expected "
                                       "<uint16_t>;<uint16_t>;<int16_t>;<"
                                       "int16_t>;<int16_t> but found '" << line
                        << "' in line " << lineNumber << Log::end;
                    success = false;
                }
            } else {
                Logger::error(LogFacility::Script)
                    << fileName
                    << ": expected "
                       "<uint16_t>;<uint16_t>;<int16_t>;<int16_t>;<int16_t> "
                       "but found '" << line << "' in line " << lineNumber
                    << Log::end;
                success = false;
            }
        }
    }

    return success;
}

bool Map::Load(const std::string &name) {
    Logger::debug(LogFacility::World) << "Loading map " << name << Log::end;

    this->name = name;

    std::ifstream map { name + "_map", std::ios::binary | std::ios::in };
    std::ifstream items { name + "_item", std::ios::binary | std::ios::in };
    std::ifstream warps { name + "_warp", std::ios::binary | std::ios::in };
    std::ifstream containers { name + "_container", std::ios::binary | std::ios::in };

    if (map.good() && items.good() && warps.good() && containers.good()) {
        int16_t newWidth, newHeight;

        map.read((char *) & newWidth, sizeof(width));
        map.read((char *) & newHeight, sizeof(height));
        map.read((char *) & origin, sizeof(origin));

        if (newWidth == width && newHeight == height) {

            for (auto &column : fields) {
                for (auto &field : column) {
                    field.load(map, items, warps, containers);
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
            fields[x][y].age();
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

const std::string &Map::getName() const { return name; }

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

bool Map::intersects(const Map &map) const {
    return map.origin.z == origin.z && getMaxX() >= map.origin.x &&
           origin.x <= map.getMaxX() && getMaxY() >= map.origin.y &&
           origin.y <= map.getMaxY();
}

}
