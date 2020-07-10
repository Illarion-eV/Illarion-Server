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
#include "Container.hpp"
#include "Item.hpp"
#include "script/binding/binding.hpp"

namespace binding {

auto script_item() -> Binding<ScriptItem, Item> {
    return luabind::class_<ScriptItem, Item>("scriptItem")
            .def(luabind::constructor<>())
            .def_readonly("owner", &ScriptItem::getOwnerForLua)
            .def_readonly("pos", &ScriptItem::pos)
            .def("getType", &ScriptItem::getType)
            .def_readonly("itempos", &ScriptItem::itempos)
            .def_readonly("inside", &ScriptItem::inside)
            .enum_("Types")[luabind::value("notdefined", ScriptItem::notdefined),
                            luabind::value("field", ScriptItem::it_field),
                            luabind::value("inventory", ScriptItem::it_inventory),
                            luabind::value("belt", ScriptItem::it_belt),
                            luabind::value("container", ScriptItem::it_container)];
}

} // namespace binding
