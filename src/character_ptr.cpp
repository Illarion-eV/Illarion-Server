/*
 *  illarionserver - server for the game Illarion
 *  Copyright 2011 Illarion e.V.
 *
 *  This file is part of illarionserver.
 *
 *  illarionserver is free software: you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation, either version 3 of the License, or
 *  (at your option) any later version.
 *
 *  illarionserver is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with illarionserver.  If not, see <http://www.gnu.org/licenses/>.
 */

#include <stdexcept>
#include "Character.hpp"
#include "World.hpp"

character_ptr::character_ptr() {
    id = 0;
}

character_ptr::character_ptr(Character *p) {
    id = p->getId();
}

character_ptr::character_ptr(character_ptr const &p) {
    id = p.id;
}

Character *character_ptr::get() const {
    auto ptr = getPointerFromId(id);
    
    if (ptr) {
        return ptr;
    } else {
        throw std::logic_error("Usage of invalid Character! Use isCharValid( char ) to check if a Character is still valid.");
    }
}

character_ptr::operator Character *() const {
    return get();
}

Character *character_ptr::operator->() const {
    return get();
}

character_ptr::operator bool() const {
    return getPointerFromId(id) != nullptr;
}

Character *character_ptr::getPointerFromId(TYPE_OF_CHARACTER_ID) const {
    return World::get()->findCharacter(id);
}

Character *get_pointer(character_ptr const &p) {
    return p.get();
}

bool isValid(character_ptr const &p) {
    return bool(p);
}

