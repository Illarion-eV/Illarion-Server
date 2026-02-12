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

#ifndef CHARACTERCONTAINER_HPP
#define CHARACTERCONTAINER_HPP

#include "constants.hpp"
#include "globals.hpp"
#include "utility.hpp"

#include <functional>
#include <string>
#include <unordered_map>
#include <vector>

/**
 * @brief Template container providing spatial indexing for character lookup.
 * 
 * Efficiently stores and retrieves characters by ID, name, position, or range.
 * Maintains both direct ID lookup and spatial index for area queries.
 * 
 * @tparam T Character type (Player, Monster, or NPC)
 */
template <class T> class CharacterContainer {
public:
    using pointer = T *;

private:
    using for_each_type = std::function<void(pointer)>;
    using for_each_member_type = void (T::*)();
    using container_type = std::unordered_map<TYPE_OF_CHARACTER_ID, pointer>;
    using position_to_id_type = std::multimap<position, TYPE_OF_CHARACTER_ID, PositionComparison>;
    position_to_id_type position_to_id;
    container_type container;

    auto getPosition(TYPE_OF_CHARACTER_ID id, position &pos) -> bool;
    [[nodiscard]] auto projection_x_axis(const position &pos, int r) const
            -> iterator_range<position_to_id_type::const_iterator>;

public:
    [[nodiscard]] auto empty() const -> bool { return container.empty(); }

    auto size() const -> decltype(container.size()) { return container.size(); }

    void insert(pointer p) {
        const auto id = p->getId();

        if (!find(id)) {
            container.emplace(id, p);
            position_to_id.insert(std::make_pair(p->getPosition(), id));
        }
    }

    auto find(const std::string &name) const -> pointer;
    auto find(TYPE_OF_CHARACTER_ID id) const -> pointer;
    auto find(const position &pos) const -> pointer;
    void update(pointer p, const position &newPosition);
    auto erase(TYPE_OF_CHARACTER_ID id) -> bool;
    void clear() {
        container.clear();
        position_to_id.clear();
    }

    auto findAllCharactersInRangeOf(const position &pos, const Range &range) const -> std::vector<pointer>;
    auto findAllCharactersInScreen(const position &pos) const -> std::vector<pointer>;
    auto findAllAliveCharactersInRangeOf(const position &pos, const Range &range) const -> std::vector<pointer>;

    void for_each(const for_each_type &function) const {
        for (const auto &key_value : container) {
            function(key_value.second);
        }
    }

    void for_each(const for_each_member_type &function) const {
        for (const auto &key_value : container) {
            (key_value.second->*function)();
        }
    }
};

#endif
