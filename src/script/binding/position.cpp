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

#include <luabind/luabind.hpp>
#include <luabind/operator.hpp>
#include "globals.hpp"
#include "script/binding/binding.hpp"

namespace binding {

    auto position() -> luabind::scope {
        return luabind::class_< ::position>("position")
                .def(luabind::constructor<>())
                .def(luabind::constructor<short int, short int, short int>())
                .def(luabind::const_self == luabind::const_self)
                .def(luabind::tostring(luabind::const_self))
                .def_readwrite("x", &position::x)
                .def_readwrite("y", &position::y)
                .def_readwrite("z", &position::z);
    }

}