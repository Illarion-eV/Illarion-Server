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

#include "data/ScriptVariablesTable.hpp"
#include "script/binding/binding.hpp"

#include <luabind/luabind.hpp>
#include <luabind/out_value_policy.hpp>

namespace binding {

auto script_variables_table() -> Binding<ScriptVariablesTable> {
    return luabind::class_<ScriptVariablesTable>("ScriptVariables")
            .def("find", &ScriptVariablesTable::find, luabind::pure_out_value(_3))
            .def("set",
                 (void (ScriptVariablesTable::*)(const std::string &, const std::string &)) & ScriptVariablesTable::set)
            .def("set", (void (ScriptVariablesTable::*)(const std::string &, int32_t)) & ScriptVariablesTable::set)
            .def("remove", &ScriptVariablesTable::remove)
            .def("save", &ScriptVariablesTable::save);
}

} // namespace binding
