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
#include <luabind/out_value_policy.hpp>
#include "Container.hpp"
#include "script/forwarder.hpp"
#include "script/binding/binding.hpp"

namespace binding {

    auto container() -> luabind::scope {
        return luabind::class_<Container>("Container")
        .def(luabind::constructor<TYPE_OF_ITEM_ID>())
        .def("getSlotCount", &Container::getSlotCount)
        .def("takeItemNr", &Container::TakeItemNr, luabind::pure_out_value(_3) + luabind::pure_out_value(_4))
        .def("viewItemNr", &Container::viewItemNr, luabind::pure_out_value(_3) + luabind::pure_out_value(_4))
        .def("changeQualityAt", &Container::changeQualityAt)
        .def("insertContainer", (bool(Container:: *)(const Item &, Container *))&Container::InsertContainer)
        .def("insertContainer", (bool(Container:: *)(const Item &, Container *, TYPE_OF_CONTAINERSLOTS))&Container::InsertContainer)
        .def("insertItem", (bool(Container:: *)(Item, bool))&Container::InsertItem)
        .def("insertItem", (bool(Container:: *)(Item, TYPE_OF_CONTAINERSLOTS))&Container::InsertItem)
        .def("insertItem", (bool(Container:: *)(const Item &))&Container::InsertItem)
        .def("countItem", container_count_item1)
        .def("countItem", container_count_item2)
        .def("eraseItem", container_erase_item1)
        .def("eraseItem", container_erase_item2)
        .def("increaseAtPos", &Container::increaseAtPos)
        .def("swapAtPos", &Container::swapAtPos)
        .def("weight", &Container::weight);
    }

}
