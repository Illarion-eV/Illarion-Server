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

#ifndef WORLDMAP_HPP
#define WORLDMAP_HPP

#include "globals.hpp"
#include "map/Map.hpp"

#include <unordered_map>
#include <vector>

namespace map {

class Field;

class WorldMap {
    std::vector<Map> maps;
    std::unordered_map<position, int> world_map;
    std::unordered_map<position, Field> persistentFields;
    size_t ageIndex = 0;

public:
    auto at(const position &pos) -> Field & { return atImpl(*this, pos); }
    auto at(const position &pos) const -> const Field & { return atImpl(*this, pos); }
    auto intersects(const Map &map) const -> bool;

    auto allMapsAged() -> bool;

    auto import(const std::string &importDir, const std::string &mapName) -> bool;
    auto exportTo() const -> bool;
    auto importFromEditor() -> bool;
    auto loadFromDisk() -> bool;
    void saveToDisk() const;
    auto createMap(const std::string &name, const position &origin, uint16_t width, uint16_t height, uint16_t tile)
            -> bool;

    void makePersistentAt(const position &pos);
    void removePersistenceAt(const position &pos);
    auto isPersistentAt(const position &pos) const -> bool;

private:
    const std::string worldName{"Illarion"};
    static constexpr auto coordinateChars = 6;
    auto insert(Map &&newMap) -> bool;
    auto insertPersistent(Field &&newField) -> bool;
    void loadPersistentFields();
    void clear();
    static auto createMapFromHeaderFile(const std::string &importDir, const std::string &mapName) -> Map;
    static auto readHeaderLine(const std::string &mapName, char header, std::ifstream &headerFile, int &lineNumber)
            -> int16_t;
    static auto isCommentOrEmpty(const std::string &line) -> bool;

    template <class T> static auto atImpl(T &t, const position &pos) -> decltype(t.at(pos)) {
        if (t.persistentFields.count(pos) > 0) {
            return t.persistentFields.at(pos);
        }
        try {
            return t.maps.at(t.world_map.at(pos)).at(pos.x, pos.y);

        } catch (std::out_of_range &) {
            throw FieldNotFound();
        }
    }
};

auto walkableNear(WorldMap &worldMap, const position &pos) -> Field &;
auto walkableNear(const WorldMap &worldMap, const position &pos) -> const Field &;

} // namespace map

#endif
