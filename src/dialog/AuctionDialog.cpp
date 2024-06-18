/*
 * Illarionserver - server for the game Illarion
 * Copyright 2011 Illarion e.V.
 *
 * This file is part of Illarionserver.
 *
 * Illarionserver  is  free  software:  you can redistribute it and/or modify it
 * under the terms of the  GNU Affero General Public License as published by the Free
 * Software Foundation, either version 3 of the License, or (at your option) any
 * later version.
 *
 * Illarionserver is distributed in the hope that it will be useful, but WITHOUT
 * ANY WARRANTY;  without  even  the  implied  warranty  of  MERCHANTABILITY  or
 * FITNESS FOR A PARTICULAR PURPOSE. See the GNU Affero General Public License for more
 * details.
 *
 * You should have received a copy of the GNU Affero General Public License along with
 * Illarionserver. If not, see <http://www.gnu.org/licenses/>.
 */

#include "dialog/AuctionDialog.hpp"

#include "data/Data.hpp"

AuctionDialog::AuctionDialog(const string &title, const luabind::object &callback)
        : Dialog(title, "AuctionDialog", callback) {}

auto AuctionDialog::getOffersSize() const -> index_type { return offers.size(); }

auto AuctionDialog::getOffersBegin() const -> offer_iterator { return offers.cbegin(); }

auto AuctionDialog::getOffersEnd() const -> offer_iterator { return offers.cend(); }

void AuctionDialog::addOffer(TYPE_OF_ITEM_ID item, const string &name, TYPE_OF_WORTH price) {
    const auto &itemStruct = Data::items()[item];
    addOffer(item, name, price, itemStruct.BuyStack);
}

void AuctionDialog::addOffer(TYPE_OF_ITEM_ID item, const string &name, TYPE_OF_WORTH price, TYPE_OF_BUY_STACK stack) {
    if (canAddOffer()) {
        offers.emplace_back(item, name, price, stack);
    }
}

auto AuctionDialog::getPrimaryRequestsSize() const -> index_type { return getProductsSize(primaryRequests); }

auto AuctionDialog::getPrimaryRequestsBegin() const -> product_iterator { return getProductsBegin(primaryRequests); }

auto AuctionDialog::getPrimaryRequestsEnd() const -> product_iterator { return getProductsEnd(primaryRequests); }

void AuctionDialog::addPrimaryRequest(TYPE_OF_ITEM_ID item, const string &name, TYPE_OF_WORTH price) {
    addProduct(primaryRequests, item, name, price);
}

auto AuctionDialog::getSecondaryRequestsSize() const -> index_type { return getProductsSize(secondaryRequests); }

auto AuctionDialog::getSecondaryRequestsBegin() const -> product_iterator {
    return getProductsBegin(secondaryRequests);
}

auto AuctionDialog::getSecondaryRequestsEnd() const -> product_iterator { return getProductsEnd(secondaryRequests); }

void AuctionDialog::addSecondaryRequest(TYPE_OF_ITEM_ID item, const string &name, TYPE_OF_WORTH price) {
    addProduct(secondaryRequests, item, name, price);
}

auto AuctionDialog::getResult() const -> Result { return result; }

void AuctionDialog::setResult(Result result) { this->result = result; }

auto AuctionDialog::getPurchaseIndex() const -> index_type { return purchaseIndex; }

void AuctionDialog::setPurchaseIndex(index_type index) { purchaseIndex = index; }

auto AuctionDialog::getPurchaseAmount() const -> Item::number_type { return purchaseAmount; }

void AuctionDialog::setPurchaseAmount(Item::number_type amount) { purchaseAmount = amount; }

auto AuctionDialog::getSaleItem() const -> const ScriptItem & { return saleItem; }

void AuctionDialog::setSaleItem(const ScriptItem &item) { saleItem = item; }

auto AuctionDialog::getLookAtList() const -> ListType { return lookAtList; }

void AuctionDialog::setLookAtList(ListType list) { lookAtList = list; }

auto AuctionDialog::closeOnMove() const -> bool { return true; }

auto AuctionDialog::getProductsSize(const product_list &products) -> index_type { return products.size(); }

auto AuctionDialog::getProductsBegin(const product_list &products) -> product_iterator { return products.cbegin(); }

auto AuctionDialog::getProductsEnd(const product_list &products) -> product_iterator { return products.cend(); }

void AuctionDialog::addProduct(product_list &products, TYPE_OF_ITEM_ID item, const string &name, TYPE_OF_WORTH price) {
    if (canAddProduct(products)) {
        products.emplace_back(item, name, price);
    }
}

auto AuctionDialog::canAddOffer() const -> bool { return offers.size() < MAXPRODUCTS; }

auto AuctionDialog::canAddProduct(const product_list &products) -> bool { return products.size() < MAXPRODUCTS; }
