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

#include "LongTimeEffect.hpp"
#include "script/binding/binding.hpp"

#include <luabind/luabind.hpp>
#include <luabind/out_value_policy.hpp>

namespace binding {

auto long_time_effect() -> Binding<LongTimeEffect> {
    return luabind::class_<LongTimeEffect>("LongTimeEffect")
            .def(luabind::constructor<uint16_t, uint32_t>())
            .def("addValue", &LongTimeEffect::addValue)
            .def("removeValue", &LongTimeEffect::removeValue)
            .def("findValue", &LongTimeEffect::findValue, luabind::pure_out_value(_3))
            .property("effectId", &LongTimeEffect::getEffectId)
            .property("effectName", &LongTimeEffect::getEffectName)
            .property("nextCalled", &LongTimeEffect::getExecuteIn, &LongTimeEffect::setExecuteIn)
            .property("numberCalled", &LongTimeEffect::getNumberOfCalls);
}

} // namespace binding
