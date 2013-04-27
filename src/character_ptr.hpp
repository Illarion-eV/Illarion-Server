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

#ifndef _CHARACTER_PTR_HPP_
#define _CHARACTER_PTR_HPP_

#include "types.hpp"

class Character;

class character_ptr {
    TYPE_OF_CHARACTER_ID id;

public:
    character_ptr();
    character_ptr(Character *p);
    character_ptr(character_ptr const &p);

    Character *get() const;
    operator Character *() const;
    Character *operator->() const;
    operator bool() const;

private:
    Character *getPointerFromId(TYPE_OF_CHARACTER_ID) const;
};

Character *get_pointer(character_ptr const &p);

bool isValid(character_ptr const &p);

#endif

