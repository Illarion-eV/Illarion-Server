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

#include "Logger.hpp"
#include "globals.hpp"
#include "stream.hpp"

#include <boost/algorithm/string/replace.hpp>
#include <limits>
#include <regex>
#include <vector>

namespace map {

Map::Map(std::string name, position origin, uint16_t width, uint16_t height)
        : origin(origin), width(width), height(height),

          name(std::move(name)) {
    for (auto x = origin.x; x < origin.x + width; ++x) {
        std::vector<Field> column;

        for (auto y = origin.y; y < origin.y + height; ++y) {
            column.emplace_back(position(x, y, origin.z));
        }

        fields.push_back(std::move(column));
    }
}

Map::Map(std::string name, position origin, uint16_t width, uint16_t height, uint16_t tile)
        : Map(std::move(name), origin, width, height) {
    for (auto &column : fields) {
        for (auto &field : column) {
            field.setTileId(tile);
        }
    }
}

auto Map::at(int16_t x, int16_t y) -> Field & { return fields[convertWorldXToMap(x)][convertWorldYToMap(y)]; }

auto Map::at(int16_t x, int16_t y) const -> const Field & {
    return fields[convertWorldXToMap(x)][convertWorldYToMap(y)];
}

auto Map::at(const MapPosition &pos) -> Field & { return at(pos.x, pos.y); }

auto Map::at(const MapPosition &pos) const -> const Field & { return at(pos.x, pos.y); }

void Map::save(const std::string &name) const {
    Logger::debug(LogFacility::World) << "Saving map " << name << Log::end;

    std::ofstream map{name + "_map", std::ios::binary | std::ios::out};
    std::ofstream items{name + "_item", std::ios::binary | std::ios::out};
    std::ofstream warps{name + "_warp", std::ios::binary | std::ios::out};
    std::ofstream containers{name + "_container", std::ios::binary | std::ios::out};

    if (map.good() && items.good() && warps.good() && containers.good()) {
        writeToStream(map, width);
        writeToStream(map, height);
        writeToStream(map, origin);

        for (const auto &column : fields) {
            for (const auto &field : column) {
                field.save(map, items, warps, containers);
            }
        }
    } else {
        Logger::error(LogFacility::World) << "Saving map failed: " << name << Log::end;
    }
}

const std::regex headerExpression{R"(^[VLXYWH]: -?\d+$)"};
const std::regex tileExpression{R"(^(\d+);(\d+);(\d+);(\d+)$)"};
const std::regex itemExpression{R"(^(\d+);(\d+);(\d+);(\d+)(;.*)?$)"};
const std::regex warpExpression{R"(^(\d+);(\d+);(-?\d+);(-?\d+);(-?\d+)$)"};
const std::regex dataExpression{R"(;([^\\;=]*(?:\\[\\;=][^\\;=]*)*)=([^\\;=]*(?:\\[\\;=][^\\;=]*)*)(?:;.*)?)"};

auto Map::import(const std::string &importDir, const std::string &mapName) -> bool {
    bool success = importFields(importDir, mapName);
    success and_eq importWarps(importDir, mapName);
    success and_eq importItems(importDir, mapName);
    return success;
}

auto Map::importFields(const std::string &importDir, const std::string &mapName) -> bool {
    using std::regex_match;
    using std::smatch;

    const std::string fileName = mapName + ".tiles.txt";
    std::ifstream mapFile(importDir + fileName);

    if (!mapFile) {
        Logger::error(LogFacility::Script) << "Could not open file: " << fileName << Log::end;
        return false;
    }

    std::string line;
    int lineNumber = 0;
    int headerLinesSkipped = 0;
    const auto headerLines = 6;
    bool success = true;

    while (std::getline(mapFile, line)) {
        ++lineNumber;

        if (line.length() != 0 && line[0] != '#') {
            if (headerLinesSkipped < headerLines && regex_match(line, headerExpression)) {
                ++headerLinesSkipped;
                continue;
            }

            if (smatch fieldMatch; regex_match(line, fieldMatch, tileExpression)) {
                const auto xPosition = 1;
                auto x = std::numeric_limits<uint16_t>::max();
                stringToNumber(fieldMatch[xPosition].str(), x);

                if (x >= width) {
                    Logger::error(LogFacility::Script)
                            << fileName << ": x must be less than width in line " << lineNumber << Log::end;
                    success = false;
                }

                const auto yPosition = 2;
                auto y = std::numeric_limits<uint16_t>::max();
                stringToNumber(fieldMatch[yPosition].str(), y);

                if (y >= height) {
                    Logger::error(LogFacility::Script)
                            << fileName << ": y must be less than height in line " << lineNumber << Log::end;
                    success = false;
                }

                const auto tilePosition = 3;
                const auto musicPosition = 4;
                auto tile = 0;
                auto music = 0;
                stringToNumber(fieldMatch[tilePosition].str(), tile);
                stringToNumber(fieldMatch[musicPosition].str(), music);

                if (success) {
                    auto &field = fields[x][y];

                    if ((field.getTileCode() != 0) || (field.getMusicId() != 0)) {
                        Logger::warn(LogFacility::Script)
                                << fileName << ": tile on (" << x << ", " << y << ") is already present, ignoring line "
                                << lineNumber << Log::end;
                    } else {
                        field.setTileId(tile);
                        field.setMusicId(music);
                    }
                }
            } else {
                Logger::error(LogFacility::Script) << fileName
                                                   << ": expected <uint16_t>;<uint16_t>;<uint16_t>;<uint16_t> "
                                                      "but found '"
                                                   << line << "' in line " << lineNumber << Log::end;
                success = false;
            }
        }
    }

    return success;
}

auto Map::importItems(const std::string &importDir, const std::string &mapName) -> bool {
    using std::regex_match;
    using std::smatch;

    const std::string fileName = mapName + ".items.txt";
    std::ifstream itemFile(importDir + fileName);

    if (!itemFile) {
        Logger::error(LogFacility::Script) << "Could not open file: " << fileName << Log::end;
        return false;
    }

    std::string line;
    int lineNumber = 0;
    bool success = true;

    while (std::getline(itemFile, line)) {
        ++lineNumber;

        if (line.length() != 0 && line[0] != '#') {
            if (smatch itemMatch; regex_match(line, itemMatch, itemExpression)) {
                const auto xPosition = 1;
                auto x = std::numeric_limits<uint16_t>::max();
                stringToNumber(itemMatch[xPosition].str(), x);

                if (x >= width) {
                    Logger::error(LogFacility::Script)
                            << fileName << ": x must be less than width in line " << lineNumber << Log::end;
                    success = false;
                }

                const auto yPosition = 2;
                auto y = std::numeric_limits<uint16_t>::max();
                stringToNumber(itemMatch[yPosition].str(), y);

                if (y >= height) {
                    Logger::error(LogFacility::Script)
                            << fileName << ": y must be less than height in line " << lineNumber << Log::end;
                    success = false;
                }

                const auto idPosition = 3;
                const auto qualityPosition = 4;
                auto itemId = 0;
                stringToNumber(itemMatch[idPosition].str(), itemId);
                auto quality = 0;
                stringToNumber(itemMatch[qualityPosition].str(), quality);

                if (quality > Item::maximumQuality) {
                    Logger::error(LogFacility::Script) << fileName << ": quality must not exceed "
                                                       << Item::maximumQuality << " in line " << lineNumber << Log::end;
                    success = false;
                }

                Item item;
                item.setId(itemId);
                item.setQuality(quality);
                item.setNumber(1);
                item.makePermanent();

                const auto dataPosition = 5;
                std::string data = itemMatch[dataPosition];

                while (data.length() > 0) {
                    if (smatch dataMatch; regex_match(data, dataMatch, dataExpression)) {
                        const auto keyPosition = 1;
                        const auto valuePosition = 2;
                        std::string key = dataMatch[keyPosition];
                        std::string value = dataMatch[valuePosition];

                        if (key.length() == 0) {
                            Logger::error(LogFacility::Script) << fileName
                                                               << ": data key must not have "
                                                                  "zero length in line "
                                                               << lineNumber << Log::end;
                            success = false;
                        }

                        if (value.length() == 0) {
                            Logger::error(LogFacility::Script) << fileName
                                                               << ": data value must not have "
                                                                  "zero length in line "
                                                               << lineNumber << Log::end;
                            success = false;
                        }

                        if (key.length() > maxDataKeyLength) {
                            Logger::error(LogFacility::Script)
                                    << fileName << ": data key must not exceed " << maxDataKeyLength
                                    << " characters in line " << lineNumber << Log::end;
                            success = false;
                        }

                        if (value.length() > maxDataValueLength) {
                            Logger::error(LogFacility::Script)
                                    << fileName << ": data value must not exceed " << maxDataValueLength
                                    << " characters in line " << lineNumber << Log::end;
                            success = false;
                        }

                        data.erase(0, key.length() + value.length() + 2);

                        unescape(key);
                        unescape(value);

                        item.setData(key, value);
                    } else {
                        Logger::error(LogFacility::Script) << fileName << ": invalid data sequence '" << data
                                                           << "' in line " << lineNumber << Log::end;
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
            } else {
                Logger::error(LogFacility::Script) << fileName
                                                   << ": expected "
                                                      "<uint16_t>;<uint16_t>;<uint16_t>;<uint16_t> "
                                                      "but found '"
                                                   << line << "' in line " << lineNumber << Log::end;
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

auto Map::importWarps(const std::string &importDir, const std::string &mapName) -> bool {
    using std::regex_match;
    using std::smatch;

    const std::string fileName = mapName + ".warps.txt";
    std::ifstream warpFile(importDir + fileName);

    if (!warpFile) {
        Logger::error(LogFacility::Script) << "Could not open file: " << fileName << Log::end;
        return false;
    }

    std::string line;
    int lineNumber = 0;
    bool success = true;

    while (std::getline(warpFile, line)) {
        ++lineNumber;

        if (line.length() != 0 && line[0] != '#') {
            if (smatch warpMatch; regex_match(line, warpMatch, warpExpression)) {
                const auto warpXPosition = 1;
                auto x = std::numeric_limits<uint16_t>::max();
                stringToNumber(warpMatch[warpXPosition].str(), x);

                if (x >= width) {
                    Logger::error(LogFacility::Script)
                            << fileName << ": x must be less than width in line " << lineNumber << Log::end;
                    success = false;
                }

                const auto warpYPosition = 2;
                auto y = std::numeric_limits<uint16_t>::max();
                stringToNumber(warpMatch[warpYPosition].str(), y);

                if (y >= height) {
                    Logger::error(LogFacility::Script)
                            << fileName << ": y must be less than height in line " << lineNumber << Log::end;
                    success = false;
                }

                const auto targetXPosition = 3;
                const auto targetYPosition = 4;
                const auto targetZPosition = 5;
                position target{};
                stringToNumber(warpMatch[targetXPosition].str(), target.x);
                stringToNumber(warpMatch[targetYPosition].str(), target.y);
                stringToNumber(warpMatch[targetZPosition].str(), target.z);

                if (success) {
                    auto &field = fields[x][y];

                    if (field.isWarp()) {
                        Logger::warn(LogFacility::Script)
                                << fileName << ": warp on (" << x << ", " << y << ") is already present, ignoring line "
                                << lineNumber << Log::end;
                    } else {
                        field.setWarp(target);
                    }
                }
            } else {
                Logger::error(LogFacility::Script) << fileName
                                                   << ": expected "
                                                      "<uint16_t>;<uint16_t>;<int16_t>;<int16_t>;<int16_t> "
                                                      "but found '"
                                                   << line << "' in line " << lineNumber << Log::end;
                success = false;
            }
        }
    }

    return success;
}

auto Map::load(const std::string &name) -> bool {
    Logger::debug(LogFacility::World) << "Loading map " << name << Log::end;

    this->name = name;

    std::ifstream map{name + "_map", std::ios::binary | std::ios::in};
    std::ifstream items{name + "_item", std::ios::binary | std::ios::in};
    std::ifstream warps{name + "_warp", std::ios::binary | std::ios::in};
    std::ifstream containers{name + "_container", std::ios::binary | std::ios::in};

    if (map.good() && items.good() && warps.good() && containers.good()) {
        int16_t newWidth = 0;
        int16_t newHeight = 0;

        readFromStream(map, newWidth);
        readFromStream(map, newHeight);
        readFromStream(map, origin);

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
    for (auto &row : fields) {
        for (auto &field : row) {
            field.age();
        }
    }
}

auto Map::getHeight() const -> uint16_t { return height; }

auto Map::getWidth() const -> uint16_t { return width; }

auto Map::getMinX() const -> int16_t { return origin.x; }

auto Map::getMinY() const -> int16_t { return origin.y; }

auto Map::getMaxX() const -> int16_t { return origin.x + width - 1; }

auto Map::getMaxY() const -> int16_t { return origin.y + height - 1; }

auto Map::getLevel() const -> int16_t { return origin.z; }

auto Map::getName() const -> const std::string & { return name; }

inline auto Map::convertWorldXToMap(int16_t x) const -> uint16_t {
    uint16_t temp = x - origin.x;

    if (temp >= width) {
        throw FieldNotFound();
    }

    return temp;
}

inline auto Map::convertWorldYToMap(int16_t y) const -> uint16_t {
    uint16_t temp = y - origin.y;

    if (temp >= height) {
        throw FieldNotFound();
    }

    return temp;
}

auto Map::intersects(const Map &map) const -> bool {
    return map.origin.z == origin.z && getMaxX() >= map.origin.x && origin.x <= map.getMaxX() &&
           getMaxY() >= map.origin.y && origin.y <= map.getMaxY();
}

} // namespace map
