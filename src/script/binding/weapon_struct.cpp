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

#include "script/binding/binding.hpp"

#include <TableStructs.hpp>

namespace binding {

auto weapon_struct() -> luabind::scope {
    return luabind::class_<WeaponStruct>("WeaponStruct")
            .def(luabind::constructor<>())
            .enum_("type")[luabind::value("slashing", 1), luabind::value("concussion", 2),
                           luabind::value("puncture", 3), luabind::value("slashingTwoHand", 4),
                           luabind::value("concussionTwoHand", 5), luabind::value("punctureTwoHand", 6),
                           luabind::value("firearm", 7), luabind::value("arrow", 10), luabind::value("bolt", 11),
                           luabind::value("stone", 12), luabind::value("stave", 13), luabind::value("shield", 14)]
            .def_readonly("Attack", &WeaponStruct::Attack)
            .def_readonly("Defence", &WeaponStruct::Defence)
            .def_readonly("Accuracy", &WeaponStruct::Accuracy)
            .def_readonly("Range", &WeaponStruct::Range)
            .def_readonly("WeaponType", &WeaponStruct::Type)
            .def_readonly("AmmunitionType", &WeaponStruct::AmmunitionType)
            .def_readonly("ActionPoints", &WeaponStruct::ActionPoints)
            .def_readonly("MagicDisturbance", &WeaponStruct::MagicDisturbance)
            .def_readonly("PoisonStrength", &WeaponStruct::PoisonStrength);
}

} // namespace binding