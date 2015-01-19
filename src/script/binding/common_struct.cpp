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

    luabind::scope common_struct() {
        return luabind::class_<CommonStruct>("CommonStruct")
                .def_readonly("id", &CommonStruct::id)
                .def_readonly("AgeingSpeed", &CommonStruct::AgeingSpeed)
                .def_readonly("Weight", &CommonStruct::Weight)
                .def_readonly("ObjectAfterRot", &CommonStruct::ObjectAfterRot)
                .def_readonly("rotsInInventory", &CommonStruct::rotsInInventory)
                .def_readonly("Brightness", &CommonStruct::Brightness)
                .def_readonly("Worth", &CommonStruct::Worth)
                .def_readonly("MaxStack", &CommonStruct::MaxStack)
                .def_readonly("BuyStack", &CommonStruct::BuyStack)
                .def_readonly("English", &CommonStruct::English)
                .def_readonly("German", &CommonStruct::German)
                .def_readonly("EnglishDescription", &CommonStruct::EnglishDescription)
                .def_readonly("GermanDescription", &CommonStruct::GermanDescription)
                .def_readonly("Rareness", &CommonStruct::Rareness);
    }

}