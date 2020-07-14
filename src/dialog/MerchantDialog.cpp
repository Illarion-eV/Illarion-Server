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

#include "dialog/MerchantDialog.hpp"

#include "data/Data.hpp"

MerchantDialog::MerchantDialog(const string &title, const luabind::object &callback)
        : Dialog(title, "MerchantDialog", callback) {
    lookAtList = listSell;
    result = playerAborts;
    purchaseIndex = 0;
    purchaseAmount = 0;
}

auto MerchantDialog::getOffersSize() const -> index_type { return offers.size(); }

auto MerchantDialog::getOffersBegin() const -> offer_iterator { return offers.cbegin(); }

auto MerchantDialog::getOffersEnd() const -> offer_iterator { return offers.cend(); }

void MerchantDialog::addOffer(TYPE_OF_ITEM_ID item, const string &name, TYPE_OF_WORTH price) {
    const auto &itemStruct = Data::items()[item];
    addOffer(item, name, price, itemStruct.BuyStack);
}

void MerchantDialog::addOffer(TYPE_OF_ITEM_ID item, const string &name, TYPE_OF_WORTH price, TYPE_OF_BUY_STACK stack) {
    if (canAddOffer()) {
        offers.emplace_back(item, name, price, stack);
    }
}

auto MerchantDialog::getPrimaryRequestsSize() const -> index_type { return getProductsSize(primaryRequests); }

auto MerchantDialog::getPrimaryRequestsBegin() const -> product_iterator { return getProductsBegin(primaryRequests); }

auto MerchantDialog::getPrimaryRequestsEnd() const -> product_iterator { return getProductsEnd(primaryRequests); }

void MerchantDialog::addPrimaryRequest(TYPE_OF_ITEM_ID item, const string &name, TYPE_OF_WORTH price) {
    addProduct(primaryRequests, item, name, price);
}

auto MerchantDialog::getSecondaryRequestsSize() const -> index_type { return getProductsSize(secondaryRequests); }

auto MerchantDialog::getSecondaryRequestsBegin() const -> product_iterator {
    return getProductsBegin(secondaryRequests);
}

auto MerchantDialog::getSecondaryRequestsEnd() const -> product_iterator { return getProductsEnd(secondaryRequests); }

void MerchantDialog::addSecondaryRequest(TYPE_OF_ITEM_ID item, const string &name, TYPE_OF_WORTH price) {
    addProduct(secondaryRequests, item, name, price);
}

auto MerchantDialog::getResult() const -> Result { return result; }

void MerchantDialog::setResult(Result result) { this->result = result; }

auto MerchantDialog::getPurchaseIndex() const -> index_type { return purchaseIndex; }

void MerchantDialog::setPurchaseIndex(index_type index) { purchaseIndex = index; }

auto MerchantDialog::getPurchaseAmount() const -> Item::number_type { return purchaseAmount; }

void MerchantDialog::setPurchaseAmount(Item::number_type amount) { purchaseAmount = amount; }

auto MerchantDialog::getSaleItem() const -> const ScriptItem & { return saleItem; }

void MerchantDialog::setSaleItem(const ScriptItem &item) { saleItem = item; }

auto MerchantDialog::getLookAtList() const -> ListType { return lookAtList; }

void MerchantDialog::setLookAtList(ListType list) { lookAtList = list; }

auto MerchantDialog::closeOnMove() const -> bool { return true; }

auto MerchantDialog::getProductsSize(const product_list &products) -> index_type { return products.size(); }

auto MerchantDialog::getProductsBegin(const product_list &products) -> product_iterator { return products.cbegin(); }

auto MerchantDialog::getProductsEnd(const product_list &products) -> product_iterator { return products.cend(); }

void MerchantDialog::addProduct(product_list &products, TYPE_OF_ITEM_ID item, const string &name, TYPE_OF_WORTH price) {
    if (canAddProduct(products)) {
        products.emplace_back(item, name, price);
    }
}

auto MerchantDialog::canAddOffer() const -> bool { return offers.size() < MAXPRODUCTS; }

auto MerchantDialog::canAddProduct(const product_list &products) -> bool { return products.size() < MAXPRODUCTS; }
