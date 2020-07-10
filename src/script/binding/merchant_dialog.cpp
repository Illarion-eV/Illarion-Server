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

#include "dialog/MerchantDialog.hpp"
#include "script/binding/binding.hpp"

namespace binding {

auto merchant_dialog() -> Binding<MerchantDialog> {
    return luabind::class_<MerchantDialog>("MerchantDialog")
            .enum_("Result")[luabind::value("playerAborts", MerchantDialog::playerAborts),
                             luabind::value("playerSells", MerchantDialog::playerSells),
                             luabind::value("playerBuys", MerchantDialog::playerBuys),
                             luabind::value("playerLooksAt", MerchantDialog::playerLooksAt)]
            .enum_("ListType")[luabind::value("listSell", MerchantDialog::listSell),
                               luabind::value("listBuyPrimary", MerchantDialog::listBuyPrimary),
                               luabind::value("listBuySecondary", MerchantDialog::listBuySecondary)]
            .def(luabind::constructor<std::string, luabind::object>())
            .def("addOffer", (void (MerchantDialog::*)(TYPE_OF_ITEM_ID, const std::string &, TYPE_OF_WORTH)) &
                                     MerchantDialog::addOffer)
            .def("addOffer",
                 (void (MerchantDialog::*)(TYPE_OF_ITEM_ID, const std::string &, TYPE_OF_WORTH, TYPE_OF_BUY_STACK)) &
                         MerchantDialog::addOffer)
            .def("addPrimaryRequest", &MerchantDialog::addPrimaryRequest)
            .def("addSecondaryRequest", &MerchantDialog::addSecondaryRequest)
            .def("getResult", &MerchantDialog::getResult)
            .def("getPurchaseIndex", &MerchantDialog::getPurchaseIndex)
            .def("getPurchaseAmount", &MerchantDialog::getPurchaseAmount)
            .def("getSaleItem", &MerchantDialog::getSaleItem)
            .def("getLookAtList", &MerchantDialog::getLookAtList);
}

} // namespace binding
