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

#include "dialog/AuctionDialog.hpp"
#include "script/binding/binding.hpp"

namespace binding {

auto auction_dialog() -> Binding<AuctionDialog> {
    return luabind::class_<AuctionDialog>("AuctionDialog")
            .enum_("Result")[luabind::value("playerAborts", AuctionDialog::playerAborts),
                             luabind::value("playerBids", AuctionDialog::playerBids),
                             luabind::value("playerLooksAt", AuctionDialog::playerLooksAt)]
            .enum_("ListType")[luabind::value("listBid", AuctionDialog::listBid)]
            .def(luabind::constructor<std::string, luabind::object>())
            .def("addOffer", (void (AuctionDialog::*)(TYPE_OF_ITEM_ID, const std::string &, TYPE_OF_WORTH)) &
                                     AuctionDialog::addOffer)
            .def("addOffer",
                 (void (AuctionDialog::*)(TYPE_OF_ITEM_ID, const std::string &, TYPE_OF_WORTH, TYPE_OF_BUY_STACK)) &
                         AuctionDialog::addOffer)
            .def("addPrimaryRequest", &AuctionDialog::addPrimaryRequest)
            .def("addSecondaryRequest", &AuctionDialog::addSecondaryRequest)
            .def("getResult", &AuctionDialog::getResult)
            .def("getPurchaseIndex", &AuctionDialog::getPurchaseIndex)
            .def("getPurchaseAmount", &AuctionDialog::getPurchaseAmount)
            .def("getSaleItem", &AuctionDialog::getSaleItem)
            .def("getLookAtList", &AuctionDialog::getLookAtList);
}

} // namespace binding
