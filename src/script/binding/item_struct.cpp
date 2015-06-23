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

#include "TableStructs.hpp"
#include "script/binding/binding.hpp"

namespace binding {

    luabind::scope item_struct() {
        return luabind::class_<ItemStruct>("ItemStruct")
                .def_readonly("id", &ItemStruct::id)
                .def_readonly("AgeingSpeed", &ItemStruct::AgeingSpeed)
                .def_readonly("Weight", &ItemStruct::Weight)
                .def_readonly("ObjectAfterRot", &ItemStruct::ObjectAfterRot)
                .def_readonly("rotsInInventory", &ItemStruct::rotsInInventory)
                .def_readonly("Brightness", &ItemStruct::Brightness)
                .def_readonly("Worth", &ItemStruct::Worth)
                .def_readonly("MaxStack", &ItemStruct::MaxStack)
                .def_readonly("BuyStack", &ItemStruct::BuyStack)
                .def_readonly("English", &ItemStruct::English)
                .def_readonly("German", &ItemStruct::German)
                .def_readonly("EnglishDescription", &ItemStruct::EnglishDescription)
                .def_readonly("GermanDescription", &ItemStruct::GermanDescription)
                .def_readonly("Rareness", &ItemStruct::Rareness)
                .def_readonly("Level", &ItemStruct::Level);
    }

}