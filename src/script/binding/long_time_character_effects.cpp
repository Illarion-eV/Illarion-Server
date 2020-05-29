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

#include <luabind/adopt_policy.hpp>
#include <luabind/out_value_policy.hpp>
#include "LongTimeCharacterEffects.hpp"
#include "LongTimeEffect.hpp"
#include "script/binding/binding.hpp"

namespace binding {

    auto long_time_character_effects() -> luabind::scope {
        return luabind::class_<LongTimeCharacterEffects>("LongTimeCharacterEffects")
        .def("addEffect",&LongTimeCharacterEffects::addEffect, luabind::adopt(_2))
        .def("removeEffect", (bool(LongTimeCharacterEffects:: *)(uint16_t))&LongTimeCharacterEffects::removeEffect)
        .def("removeEffect", (bool(LongTimeCharacterEffects:: *)(const std::string &))&LongTimeCharacterEffects::removeEffect)
        .def("removeEffect", (bool(LongTimeCharacterEffects:: *)(LongTimeEffect *))&LongTimeCharacterEffects::removeEffect)
        .def("find", (bool(LongTimeCharacterEffects:: *)(uint16_t,LongTimeEffect *&) const)&LongTimeCharacterEffects::find,luabind::pure_out_value(_3))
        .def("find", (bool(LongTimeCharacterEffects:: *)(const std::string &,LongTimeEffect *&) const)&LongTimeCharacterEffects::find,luabind::pure_out_value(_3));
    }

}
