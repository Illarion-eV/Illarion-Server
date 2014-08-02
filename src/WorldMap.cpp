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


#include "WorldMap.hpp"
#include "Map.hpp"
#include "Logger.hpp"

#include <algorithm>
#include <stdexcept>
#include <boost/algorithm/string/replace.hpp>
#include <chrono>

void WorldMap::clear() {
    maps.clear();
    world_map.clear();
}

bool WorldMap::mapInRangeOf(const position &origin, uint16_t width,
                            uint16_t height) const {
    return std::any_of(maps.begin(), maps.end(), [&](const map_t &map) {
        return map->intersects(origin, width, height);
    });
}

bool WorldMap::mapInRangeOf(const Map &map) const {
    return std::any_of(maps.begin(), maps.end(), [&](const map_t &testMap) {
        return testMap->intersects(map);
    });
}

auto WorldMap::findAllMapsInRangeOf(char rnorth, char rsouth, char reast,
                                    char rwest,
                                    position pos) const -> map_vector_t {
    map_vector_t result;

    const position upperleft(pos.x - rwest, pos.y - rnorth, pos.z);
    const uint16_t width = reast + rwest + 1;
    const uint16_t height = rnorth + rsouth + 1;

    for (const auto &map : maps) {
        if (map->intersects(upperleft, width, height)) {
            result.push_back(map);
        }
    }

    return result;
}

auto WorldMap::findMapForPos(const position &pos) const -> map_t {
    try {
        return world_map.at(pos);
    } catch (std::out_of_range &e) {
        return {};
    }
}

bool WorldMap::InsertMap(map_t newMap) {
    if (newMap) {
        if (mapInRangeOf(*newMap)) return false;

        maps.push_back(newMap);

        auto z = newMap->getLevel();

        for (auto x = newMap->getMinX(); x <= newMap->getMaxX(); ++x) {
            for (auto y = newMap->getMinY(); y <= newMap->getMaxY(); ++y) {
                position p(x, y, z);
                world_map[p] = newMap;
            }
        }

        return true;
    }

    return false;

}

bool WorldMap::allMapsAged() {
    using std::chrono::steady_clock;
    using std::chrono::milliseconds;

    auto startTime = steady_clock::now();

    while (ageIndex < maps.size() && steady_clock::now() - startTime < milliseconds(10)) {
        maps[ageIndex++]->age();
    }

    if (ageIndex < maps.size()) {
        return false;
    }

    ageIndex = 0;
    return true;
}

bool WorldMap::exportTo(const std::string &exportDir) const {
    for (const auto &map : maps) {
        int16_t minX = map->getMinX();
        int16_t minY = map->getMinY();
        // create base filename
        std::string filebase = exportDir + "e_" + std::to_string(minX)
                               + "_" + std::to_string(minY)
                               + "_" + std::to_string(map->getLevel()) + ".";
        // export fields file
        std::ofstream fieldsf((filebase + "tiles.txt").c_str());
        // export items file
        std::ofstream itemsf((filebase + "items.txt").c_str());
        // export warps file
        std::ofstream warpsf((filebase + "warps.txt").c_str());

        if (!fieldsf.good() || !itemsf.good() || !warpsf.good()) {
            Logger::error(LogFacility::World) << "Could not open output files for item export: " << filebase << "*.txt" << Log::end;
            return false;
        }

        // export tiles header
        fieldsf << "V: 2" << std::endl;
        fieldsf << "L: " << map->getLevel() << std::endl;
        fieldsf << "X: " << minX << std::endl;
        fieldsf << "Y: " << minY << std::endl;
        fieldsf << "W: " << map->getWidth() << std::endl;
        fieldsf << "H: " << map->getHeight() << std::endl;

        // iterate over the map and export...
        short int x, y;

        for (y = minY; y <= map->getMaxY(); ++y) {
            for (x = minX; x <= map->getMaxX(); ++x) {
                Field field;

                if (map->GetCFieldAt(field, x, y)) {
                    fieldsf << x-minX << ";" << y-minY << ";" << field.getTileCode() << ";" << field.getMusicId() << std::endl;

                    if (field.IsWarpField()) {
                        position target;
                        field.GetWarpField(target);
                        warpsf << x-minX << ";" << y-minY << ";" << target.x << ";" << target.y << ";" << target.z << std::endl;
                    }

                    for (const auto &item : field.getExportItems()) {
                        itemsf << x-minX << ";" << y-minY << ";" << item.getId() << ";" << item.getQuality();

                        std::for_each(item.getDataBegin(), item.getDataEnd(), [&](const std::pair<std::string, std::string> &data) {
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
        }

        fieldsf.close();
        itemsf.close();
        warpsf.close();

    }

    return true;
}

void WorldMap::saveToDisk(const std::string &prefix) const {
    std::ofstream mapinitfile((prefix + "_initmaps").c_str(), std::ios::binary | std::ios::out | std::ios::trunc);

    if (!mapinitfile.good()) {
        Logger::error(LogFacility::World) << "Could not create initmaps!" << Log::end;
    } else {
        uint16_t size = maps.size();
        Logger::info(LogFacility::World) << "Saving " << size << " maps." << Log::end;
        mapinitfile.write((char *) & size, sizeof(size));
        char mname[200];

        for (const auto &map : maps) {
            const auto level = map->getLevel();
            const auto x = map->getMinX();
            const auto y = map->getMinY();
            const auto width = map->getWidth();
            const auto height = map->getHeight();
            mapinitfile.write((char *) &level, sizeof(level));
            mapinitfile.write((char *) &x, sizeof(x));
            mapinitfile.write((char *) &y, sizeof(y));
            mapinitfile.write((char *) &width, sizeof(width));
            mapinitfile.write((char *) &height, sizeof(height));

            sprintf(mname, "%s_%6d_%6d_%6d", prefix.c_str(), map->getLevel(), map->getMinX(), map->getMinY());
            map->Save(mname);
        }

        mapinitfile.close();
    }
}

