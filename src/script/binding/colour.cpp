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

#include "types.hpp"
#include "script/binding/binding.hpp"

namespace binding {

    auto colour() -> luabind::scope {
        return luabind::class_<Colour>("colour")
        .def(luabind::constructor<>())
        .def(luabind::constructor<uint8_t, uint8_t, uint8_t>())
        .def(luabind::constructor<uint8_t, uint8_t, uint8_t, uint8_t>())
        .def_readwrite("red", &Colour::red)
        .def_readwrite("green", &Colour::green)
        .def_readwrite("blue", &Colour::blue)
        .def_readwrite("alpha", &Colour::alpha);
    }

}