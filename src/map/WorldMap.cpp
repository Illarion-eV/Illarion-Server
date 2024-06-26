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

#include "map/WorldMap.hpp"

#include "Config.hpp"
#include "Logger.hpp"
#include "Map.hpp"
#include "Monster.hpp"
#include "NPC.hpp"
#include "Player.hpp"
#include "World.hpp"
#include "db/Result.hpp"
#include "db/SelectQuery.hpp"
#include "stream.hpp"

#include <algorithm>
#include <boost/algorithm/string/replace.hpp>
#include <chrono>
#include <filesystem>
#include <iomanip>
#include <range/v3/all.hpp>
#include <regex>
#include <sstream>
#include <stdexcept>

namespace map {

void WorldMap::clear() {
    world_map.clear();
    maps.clear();
}

auto WorldMap::intersects(const Map &map) const -> bool {
    using namespace ranges;
    auto doIntersectMap = [&map](const auto &testMap) { return map.intersects(testMap); };
    auto overlap = find_if(maps, doIntersectMap);
    bool foundIntersection = overlap != maps.end();

    if (foundIntersection) {
        Logger::error(LogFacility::Script) << "Could not insert map " << map.getName() << " because it intersects with "
                                           << overlap->getName() << Log::end;
    }

    return foundIntersection;
}

auto WorldMap::insert(Map &&newMap) -> bool {
    if (intersects(newMap)) {
        return false;
    }

    maps.push_back(std::move(newMap));

    auto &map = maps.back();
    const auto z = map.getLevel();

    for (auto x = map.getMinX(); x <= map.getMaxX(); ++x) {
        for (auto y = map.getMinY(); y <= map.getMaxY(); ++y) {
            position p(x, y, z);
            world_map[p] = maps.size() - 1;
        }
    }

    return true;
}

auto WorldMap::insertPersistent(Field &&newField) -> bool {
    newField.makePersistent();
    return persistentFields.insert({newField.getPosition(), std::move(newField)}).second;
}

auto WorldMap::allMapsAged() -> bool {
    using std::chrono::milliseconds;
    using std::chrono::steady_clock;
    constexpr auto ageInterval = milliseconds(10);

    auto startTime = steady_clock::now();

    while (ageIndex < maps.size() && steady_clock::now() - startTime < ageInterval) {
        maps[ageIndex++].age();
    }

    if (ageIndex < maps.size()) {
        return false;
    }

    using namespace ranges;
    for_each(persistentFields | view::values, [](auto &field) { field.age(); });

    ageIndex = 0;
    return true;
}

auto WorldMap::import(const std::string &importDir, const std::string &mapName) -> bool {
    bool success = true;

    try {
        auto map = createMapFromHeaderFile(importDir, mapName);
        success = map.import(importDir, mapName) && insert(std::move(map));
    } catch (MapError &) {
        success = false;
    }

    if (!success) {
        Logger::alert(LogFacility::Script) << "---> Could not import " << mapName << Log::end;
    }

    return success;
}

auto WorldMap::createMapFromHeaderFile(const std::string &importDir, const std::string &mapName) -> Map {
    const std::string fileName = mapName + ".tiles.txt";
    std::ifstream headerFile(importDir + fileName);

    if (!headerFile) {
        Logger::error(LogFacility::Script) << "Could not open file: " << fileName << Log::end;
        throw MapError();
    }

    int lineNumber = 0;

    auto version = readHeaderLine(mapName, 'V', headerFile, lineNumber);

    if (version != 2) {
        Logger::error(LogFacility::Script)
                << fileName << ": incorrect version in line " << lineNumber << "! Expected: 2" << Log::end;
        throw MapError();
    }

    position origin{};
    origin.z = readHeaderLine(mapName, 'L', headerFile, lineNumber);
    origin.x = readHeaderLine(mapName, 'X', headerFile, lineNumber);
    origin.y = readHeaderLine(mapName, 'Y', headerFile, lineNumber);

    auto width = readHeaderLine(mapName, 'W', headerFile, lineNumber);

    if (width <= 0) {
        Logger::error(LogFacility::Script)
                << fileName << ": width has to be positive in line " << lineNumber << Log::end;
        throw MapError();
    }

    if (origin.x > std::numeric_limits<int16_t>::max() - width + 1) {
        Logger::error(LogFacility::Script)
                << fileName << ": x + width - 1 must not exceed " << std::numeric_limits<int16_t>::max() << " in line "
                << lineNumber << Log::end;
        throw MapError();
    }

    auto height = readHeaderLine(mapName, 'H', headerFile, lineNumber);

    if (height <= 0) {
        Logger::error(LogFacility::Script)
                << fileName << ": height has to be positive in line " << lineNumber << Log::end;
        throw MapError();
    }

    if (origin.y > std::numeric_limits<int16_t>::max() - height + 1) {
        Logger::error(LogFacility::Script)
                << fileName << ": y + height - 1 must not exceed " << std::numeric_limits<int16_t>::max() << " in line "
                << lineNumber << Log::end;
        throw MapError();
    }

    return Map(mapName, origin, width, height);
}

const std::regex headerExpression{R"(^(.): (-?\d+)$)"};

auto WorldMap::readHeaderLine(const std::string &mapName, char header, std::ifstream &headerFile, int &lineNumber)
        -> int16_t {
    using std::regex_match;
    using std::smatch;
    std::string line;

    while (std::getline(headerFile, line)) {
        ++lineNumber;

        if (!isCommentOrEmpty(line)) {
            smatch matches;

            if (regex_match(line, matches, headerExpression) && matches[1] == header) {
                int16_t headerValue = 0;
                stringToNumber(matches[2].str(), headerValue);
                return headerValue;
            }
        }
    }

    Logger::error(LogFacility::Script) << mapName << ": expected header '" << header << ": <int16_t>' but found '"
                                       << line << "' in line " << lineNumber << Log::end;
    throw MapError();
}

auto WorldMap::isCommentOrEmpty(const std::string &line) -> bool { return line.length() == 0 || line[0] == '#'; }

auto WorldMap::exportTo() const -> bool {
    const std::string exportDir = Config::instance().datadir() + std::string(MAPDIR) + "export/";

    for (const auto &map : maps) {
        int16_t minX = map.getMinX();
        int16_t minY = map.getMinY();
        // create base filename
        std::string filebase = exportDir + "e_" + std::to_string(minX) + "_" + std::to_string(minY) + "_" +
                               std::to_string(map.getLevel()) + ".";
        // export fields file
        std::ofstream fieldsf(filebase + "tiles.txt");
        // export items file
        std::ofstream itemsf(filebase + "items.txt");
        // export warps file
        std::ofstream warpsf(filebase + "warps.txt");

        if (!fieldsf.good() || !itemsf.good() || !warpsf.good()) {
            Logger::error(LogFacility::World)
                    << "Could not open output files for item export: " << filebase << "*.txt" << Log::end;
            return false;
        }

        // export tiles header
        fieldsf << "V: 2" << std::endl;
        fieldsf << "L: " << map.getLevel() << std::endl;
        fieldsf << "X: " << minX << std::endl;
        fieldsf << "Y: " << minY << std::endl;
        fieldsf << "W: " << map.getWidth() << std::endl;
        fieldsf << "H: " << map.getHeight() << std::endl;

        // iterate over the map and export...

        for (short int y = minY; y <= map.getMaxY(); ++y) {
            for (short int x = minX; x <= map.getMaxX(); ++x) {
                const Field &field = map.at(x, y);

                fieldsf << x - minX << ";" << y - minY << ";" << field.getTileCode() << ";" << field.getMusicId()
                        << std::endl;

                if (field.isWarp()) {
                    position target{};
                    field.getWarp(target);
                    warpsf << x - minX << ";" << y - minY << ";" << target.x << ";" << target.y << ";" << target.z
                           << std::endl;
                }

                for (const auto &item : field.getExportItems()) {
                    itemsf << x - minX << ";" << y - minY << ";" << item.getId() << ";" << item.getQuality();

                    std::for_each(item.getDataBegin(), item.getDataEnd(),
                                  [&](const std::pair<std::string, std::string> &data) {
                                      using boost::algorithm::replace_all;

                                      std::string key = data.first;
                                      std::string value = data.second;

                                      replace_all(key, "\\", "\\\\");
                                      replace_all(key, "=", "\\=");
                                      replace_all(key, ";", "\\;");
                                      replace_all(value, "\\", "\\\\");
                                      replace_all(value, "=", "\\=");
                                      replace_all(value, ";", "\\;");

                                      itemsf << ";" << key << "=" << value;
                                  });

                    itemsf << std::endl;
                }
            }
        }

        fieldsf.close();
        itemsf.close();
        warpsf.close();
    }

    return true;
}

auto WorldMap::importFromEditor() -> bool {
    clear();

    int numfiles = 0;
    int errors = 0;
    const std::regex tilesFilter{".*\\.tiles\\.txt"};
    const std::regex mapFilter{worldName + ".*"};
    const std::string path = Config::instance().datadir() + std::string(MAPDIR) + worldName;

    Logger::notice(LogFacility::Script) << "Removing old maps." << Log::end;

    for (std::filesystem::directory_iterator end, it(Config::instance().datadir() + std::string(MAPDIR)); it != end;
         ++it) {
        if (std::regex_match(it->path().filename().string(), mapFilter)) {
            std::filesystem::remove(it->path());
        }
    }

    Logger::notice(LogFacility::Script) << "Importing maps..." << Log::end;

    std::string importDir = Config::instance().datadir() + std::string(MAPDIR) + "import/";

    for (std::filesystem::recursive_directory_iterator end, it(importDir); it != end; ++it) {
        if (!std::filesystem::is_regular_file(it->status())) {
            continue;
        }

        if (!std::regex_match(it->path().filename().string(), tilesFilter)) {
            continue;
        }

        std::string map = it->path().string();

        std::string_view suffix{".tiles.txt"};

        map.resize(map.length() - suffix.length());
        map.erase(0, importDir.length());

        Logger::debug(LogFacility::World) << "Importing: " << map << Log::end;

        if (!import(importDir, map)) {
            ++errors;
        }

        ++numfiles;
    }

    if (numfiles <= 0) {
        perror("Could not import maps");
        return false;
    }

    Logger::notice(LogFacility::Script) << "Imported " << numfiles - errors << " out of " << numfiles << " maps."
                                        << Log::end;

    if (errors != 0) {
        Logger::alert(LogFacility::Script) << "Failed to import " << errors << " maps!" << Log::end;
    }

    saveToDisk();
    loadPersistentFields();
    return errors == 0;
}

auto WorldMap::loadFromDisk() -> bool {
    clear();
    const std::string path = Config::instance().datadir() + std::string(MAPDIR) + worldName;
    std::ifstream mapinitfile(path + "_initmaps", std::ios::binary | std::ios::in);

    if (!mapinitfile) {
        Logger::error(LogFacility::World)
                << "Error while loading maps: could not open " << (path + "_initmaps") << Log::end;
        return false;
    }
    unsigned short int size = 0;

    readFromStream(mapinitfile, size);

    Logger::info(LogFacility::World) << "Loading " << size << " maps." << Log::end;

    int16_t level = 0;
    int16_t minX = 0;
    int16_t minY = 0;
    uint16_t width = 0;
    uint16_t height = 0;
    std::ostringstream mapName;

    for (int i = 0; i < size; ++i) {
        readFromStream(mapinitfile, level);
        readFromStream(mapinitfile, minX);
        readFromStream(mapinitfile, minY);
        readFromStream(mapinitfile, width);
        readFromStream(mapinitfile, height);

        auto map = Map{"previously saved map", position{minX, minY, level}, width, height};

        mapName.str("");
        mapName << path << '_' << std::setw(coordinateChars) << level << '_' << std::setw(coordinateChars) << minX
                << '_' << std::setw(coordinateChars) << minY;

        if (map.load(mapName.str())) {
            insert(std::move(map));
        }
    }

    loadPersistentFields();

    return true;
}

void WorldMap::loadPersistentFields() {
    persistentFields.clear();

    using namespace Database;

    SelectQuery query;
    query.addColumn("map_tiles", "mt_x");
    query.addColumn("map_tiles", "mt_y");
    query.addColumn("map_tiles", "mt_z");
    query.addColumn("map_tiles", "mt_tile");
    query.addColumn("map_tiles", "mt_music");
    query.addServerTable("map_tiles");

    auto result = query.execute();
    Logger::info(LogFacility::World) << "Loading " << result.size() << " persistent fields." << Log::end;

    const bool isPersistent = true;

    for (const auto &row : result) {
        position pos(row["mt_x"].as<int16_t>(), row["mt_y"].as<int16_t>(), row["mt_z"].as<int16_t>());

        auto tile = row["mt_tile"].as<uint16_t>();
        auto music = row["mt_music"].as<uint16_t>();
        Field field(tile, music, pos, isPersistent);

        persistentFields.emplace(pos, std::move(field));
    }
}

void WorldMap::saveToDisk() const {
    const std::string path = Config::instance().datadir() + std::string(MAPDIR) + worldName;
    std::ofstream mapinitfile(path + "_initmaps", std::ios::binary | std::ios::out | std::ios::trunc);

    if (!mapinitfile.good()) {
        Logger::error(LogFacility::World) << "Could not create initmaps!" << Log::end;
    } else {
        const uint16_t size = maps.size();
        Logger::info(LogFacility::World) << "Saving " << size << " maps." << Log::end;
        writeToStream(mapinitfile, size);
        std::ostringstream mapName;

        for (const auto &map : maps) {
            const auto level = map.getLevel();
            const auto x = map.getMinX();
            const auto y = map.getMinY();
            const auto width = map.getWidth();
            const auto height = map.getHeight();
            writeToStream(mapinitfile, level);
            writeToStream(mapinitfile, x);
            writeToStream(mapinitfile, y);
            writeToStream(mapinitfile, width);
            writeToStream(mapinitfile, height);

            mapName.str("");
            mapName << path << '_' << std::setw(coordinateChars) << level << '_' << std::setw(coordinateChars) << x
                    << '_' << std::setw(coordinateChars) << y;

            map.save(mapName.str());
        }

        mapinitfile.close();
    }
}

auto WorldMap::createMap(const std::string &name, const position &origin, uint16_t width, uint16_t height,
                         uint16_t tile) -> bool {
    return insert({name, origin, width, height, tile});
}

void WorldMap::makePersistentAt(const position &pos) {
    try {
        Field &field = persistentFields.at(pos);
        field.makePersistent();
    } catch (std::out_of_range &) {
        try {
            Field &field = at(pos);
            Field newField(pos);
            std::swap(newField, field);
            insertPersistent(std::move(newField));
        } catch (FieldNotFound &) {
            Field newField(pos);
            insertPersistent(std::move(newField));
        }
    }
}

void WorldMap::removePersistenceAt(const position &pos) {
    bool existsInMap = world_map.count(pos) > 0;
    bool existsPersistent = persistentFields.count(pos) > 0;

    if (!existsInMap && existsPersistent) {
        // Clean up the Field, since it will be removed.
        // Usually a script should handle this, but we still
        // need a default to avoid an illegal server state!

        Range range{0, 0};
        auto players = World::get()->Players.findAllCharactersInRangeOf(pos, range);

        for (auto *player : players) {
            if (not player->Warp(pos)) {
                position start(Config::instance().playerstart_x, Config::instance().playerstart_y,
                               Config::instance().playerstart_z);
                player->Warp(start);
            }
        }

        auto monsters = World::get()->Monsters.findAllCharactersInRangeOf(pos, range);

        for (auto *monster : monsters) {
            monster->remove();
        }

        auto npcs = World::get()->Npc.findAllCharactersInRangeOf(pos, range);

        for (auto *npc : npcs) {
            World::get()->deleteNPC(npc->getId());
        }
    }

    auto fieldNode = persistentFields.extract(pos);

    if (!fieldNode.empty()) {
        fieldNode.mapped().removePersistence();

        try {
            Field &field = at(pos);
            std::swap(field, fieldNode.mapped());
        } catch (FieldNotFound &) {
            updateFieldToPlayersInScreen(pos);
        }
    }
}

auto WorldMap::isPersistentAt(const position &pos) const -> bool {
    bool persistent = false;

    try {
        const Field &field = persistentFields.at(pos);
        persistent = field.isPersistent();
    } catch (std::out_of_range &) {
    }

    return persistent;
}

auto walkableNear(WorldMap &worldMap, const position &pos) -> Field & {
    auto start = pos;
    auto testPos = pos;

    unsigned char d = 0;

    while (d <= nearbyFieldRange) {
        testPos.x = start.x - d;

        while (testPos.x <= start.x + d) {
            try {
                testPos.y = d + start.y;
                auto &field = worldMap.at(testPos);

                if (field.moveToPossible()) {
                    return field;
                }
            } catch (FieldNotFound &) {
            }

            try {
                testPos.y = start.y - d;
                auto &field = worldMap.at(testPos);

                if (field.moveToPossible()) {
                    return field;
                }
            } catch (FieldNotFound &) {
            }

            testPos.x++;
        }

        testPos.y = start.y - d;

        while (testPos.y <= d + start.y) {
            try {
                testPos.x = d + start.x;
                auto &field = worldMap.at(testPos);

                if (field.moveToPossible()) {
                    return field;
                }
            } catch (FieldNotFound &) {
            }

            try {
                testPos.x = start.x - d;
                auto &field = worldMap.at(testPos);

                if (field.moveToPossible()) {
                    return field;
                }
            } catch (FieldNotFound &) {
            }

            testPos.y++;
        }

        d++;
    }

    throw FieldNotFound();
}

} // namespace map
