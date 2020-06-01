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

#include "dialog/CraftingDialog.hpp"
#include "script/binding/binding.hpp"

namespace binding {

auto crafting_dialog() -> luabind::scope {
    return luabind::class_<CraftingDialog>("CraftingDialog")
            .enum_("Result")[luabind::value("playerAborts", CraftingDialog::playerAborts),
                             luabind::value("playerCrafts", CraftingDialog::playerCrafts),
                             luabind::value("playerLooksAtItem", CraftingDialog::playerLooksAtCraftable),
                             luabind::value("playerLooksAtIngredient", CraftingDialog::playerLooksAtIngredient),
                             luabind::value("playerCraftingComplete", CraftingDialog::playerCraftingComplete),
                             luabind::value("playerCraftingAborted", CraftingDialog::playerCraftingAborted)]
            .def(luabind::constructor<std::string, uint16_t, uint16_t, luabind::object>())
            .def("clearGroupsAndProducts", &CraftingDialog::clearGroupsAndProducts)
            .def("addGroup", &CraftingDialog::addGroup)
            .def("addCraftable",
                 (void (CraftingDialog::*)(uint8_t, uint8_t, TYPE_OF_ITEM_ID, const std::string &, uint16_t)) &
                         CraftingDialog::addCraftable)
            .def("addCraftable",
                 (void (CraftingDialog::*)(uint8_t, uint8_t, TYPE_OF_ITEM_ID, const std::string &, uint16_t, uint8_t)) &
                         CraftingDialog::addCraftable)
            .def("addCraftableIngredient",
                 (void (CraftingDialog::*)(TYPE_OF_ITEM_ID)) & CraftingDialog::addCraftableIngredient)
            .def("addCraftableIngredient",
                 (void (CraftingDialog::*)(TYPE_OF_ITEM_ID, uint8_t)) & CraftingDialog::addCraftableIngredient)
            .def("getResult", &CraftingDialog::getResult)
            .def("getCraftableId", &CraftingDialog::getCraftableId)
            .def("getCraftableAmount", &CraftingDialog::getCraftableAmount)
            .def("getIngredientIndex", &CraftingDialog::getIngredientIndex);
}

} // namespace binding