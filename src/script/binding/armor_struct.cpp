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

auto armor_struct() -> luabind::scope {
    return luabind::class_<ArmorStruct>("ArmorStruct")
            .enum_("type")[luabind::value("clothing", 0), luabind::value("general", 1), luabind::value("light", 2),
                           luabind::value("medium", 3), luabind::value("heavy", 4), luabind::value("juwellery", 5)]
            .def_readonly("BodyParts", &ArmorStruct::BodyParts)
            .def_readonly("PunctureArmor", &ArmorStruct::PunctureArmor)
            .def_readonly("StrokeArmor", &ArmorStruct::StrokeArmor)
            .def_readonly("ThrustArmor", &ArmorStruct::ThrustArmor)
            .def_readonly("MagicDisturbance", &ArmorStruct::MagicDisturbance)
            .def_readonly("Absorb", &ArmorStruct::Absorb)
            .def_readonly("Stiffness", &ArmorStruct::Stiffness)
            .def_readonly("Type", &ArmorStruct::Type);
}

} // namespace binding
