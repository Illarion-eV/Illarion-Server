/*
 *  illarionserver - server for the game Illarion
 *  Copyright 2011 Illarion e.V.
 *
 *  This file is part of illarionserver.
 *
 *  illarionserver is free software: you can redistribute it and/or modify
 *  it under the terms of the GNU Affero General Public License as published by
 *  the Free Software Foundation, either version 3 of the License, or
 *  (at your option) any later version.
 *
 *  illarionserver is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU Affero General Public License for more details.
 *
 *  You should have received a copy of the GNU Affero General Public License
 *  along with illarionserver.  If not, see <http://www.gnu.org/licenses/>.
 */

#include "Character.hpp"
#include "World.hpp"

#include <stdexcept>

character_ptr::character_ptr() { id = 0; }

character_ptr::character_ptr(Character *p) {
    if (p != nullptr) {
        id = p->getId();
    } else {
        id = 0;
    }
}

character_ptr::character_ptr(character_ptr const &p) { id = p.id; }

auto character_ptr::operator=(character_ptr const &p) -> character_ptr & = default;

auto character_ptr::get() const -> Character * {
    auto *ptr = getPointerFromId();

    if (ptr != nullptr) {
        return ptr;
    }
    throw std::logic_error(

            "Usage of invalid Character! Use isValidChar( char ) to check if a Character is still valid.");
}

character_ptr::operator Character *() const { return get(); }

auto character_ptr::operator->() const -> Character * { return get(); }

character_ptr::operator bool() const { return getPointerFromId() != nullptr; }

auto character_ptr::getPointerFromId() const -> Character * {
    if (id != 0) {
        return World::get()->findCharacter(id);
    }

    return nullptr;
}

auto get_pointer(character_ptr const &p) -> Character * { return p.get(); }

auto isValid(character_ptr const &p) -> bool { return bool(p); }
