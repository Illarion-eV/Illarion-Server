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

auto weapon_struct() -> luabind::scope {
    return luabind::class_<WeaponStruct>("WeaponStruct")
            .def(luabind::constructor<>())
            .enum_("type")[luabind::value("slashing", WeaponStruct::slashing),
                           luabind::value("concussion", WeaponStruct::concussion),
                           luabind::value("puncture", WeaponStruct::puncture),
                           luabind::value("slashingTwoHand", WeaponStruct::slashingTwoHand),
                           luabind::value("concussionTwoHand", WeaponStruct::concussionTwoHand),
                           luabind::value("punctureTwoHand", WeaponStruct::punctureTwoHand),
                           luabind::value("firearm", WeaponStruct::firearm),
                           luabind::value("arrow", WeaponStruct::arrow), luabind::value("bolt", WeaponStruct::bolt),
                           luabind::value("stone", WeaponStruct::stone), luabind::value("stave", WeaponStruct::stave),
                           luabind::value("shield", WeaponStruct::shield)]
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
