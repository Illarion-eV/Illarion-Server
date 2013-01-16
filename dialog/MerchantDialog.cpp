/*
 * Illarionserver - server for the game Illarion
 * Copyright 2011 Illarion e.V.
 *
 * This file is part of Illarionserver.
 *
 * Illarionserver  is  free  software:  you can redistribute it and/or modify it
 * under the terms of the  GNU  General  Public License as published by the Free
 * Software Foundation, either version 3 of the License, or (at your option) any
 * later version.
 *
 * Illarionserver is distributed in the hope that it will be useful, but WITHOUT
 * ANY WARRANTY;  without  even  the  implied  warranty  of  MERCHANTABILITY  or
 * FITNESS FOR A PARTICULAR PURPOSE. See the GNU General Public License for more
 * details.
 *
 * You should have received a copy of the GNU  General Public License along with
 * Illarionserver. If not, see <http://www.gnu.org/licenses/>.
 */

#include "dialog/MerchantDialog.hpp"
#include "data/CommonObjectTable.hpp"

extern CommonObjectTable *CommonItems;

MerchantDialog::MerchantDialog(std::string title, luabind::object callback)
    :Dialog(title, "MerchantDialog", callback) {
    result = playerAborts;
    purchaseIndex = 0;
    purchaseAmount = 0;
}

MerchantDialog::MerchantDialog(const MerchantDialog &merchantDialog) : Dialog(merchantDialog) {
    for (auto it = merchantDialog.offers.begin(); it != merchantDialog.offers.end(); ++it) {
        OfferProduct &product = *(OfferProduct *)*it;
        addProduct(offers, product.getItem(), product.getName(), product.getPrice(), product.getStack());
    }

    for (auto it = merchantDialog.primaryRequests.begin(); it != merchantDialog.primaryRequests.end(); ++it) {
        Product &product = **it;
        addProduct(primaryRequests, product.getItem(), product.getName(), product.getPrice());
    }

    for (auto it = merchantDialog.secondaryRequests.begin(); it != merchantDialog.secondaryRequests.end(); ++it) {
        Product &product = **it;
        addProduct(secondaryRequests, product.getItem(), product.getName(), product.getPrice());
    }

    result = merchantDialog.result;
    purchaseIndex = merchantDialog.purchaseIndex;
    purchaseAmount = merchantDialog.purchaseAmount;
    saleItem = merchantDialog.saleItem;
}

MerchantDialog::~MerchantDialog() {
    for (auto it = offers.begin(); it != offers.end(); ++it) {
        delete *it;
    }

    for (auto it = primaryRequests.begin(); it != primaryRequests.end(); ++it) {
        delete *it;
    }

    for (auto it = secondaryRequests.begin(); it != secondaryRequests.end(); ++it) {
        delete *it;
    }
}

MerchantDialog::index_type MerchantDialog::getOffersSize() const {
    return getProductsSize(offers);
}

MerchantDialog::product_list::const_iterator MerchantDialog::getOffersBegin() const {
    return getProductsBegin(offers);
}

MerchantDialog::product_list::const_iterator MerchantDialog::getOffersEnd() const {
    return getProductsEnd(offers);
}

void MerchantDialog::addOffer(TYPE_OF_ITEM_ID item, string name, TYPE_OF_WORTH price) {
    CommonStruct common;
    CommonItems->find(item, common);
    addProduct(offers, item, name, price, common.BuyStack);
}

void MerchantDialog::addOffer(TYPE_OF_ITEM_ID item, string name, TYPE_OF_WORTH price, TYPE_OF_BUY_STACK stack) {
    addProduct(offers, item, name, price, stack);
}

MerchantDialog::index_type MerchantDialog::getPrimaryRequestsSize() const {
    return getProductsSize(primaryRequests);
}

MerchantDialog::product_list::const_iterator MerchantDialog::getPrimaryRequestsBegin() const {
    return getProductsBegin(primaryRequests);
}

MerchantDialog::product_list::const_iterator MerchantDialog::getPrimaryRequestsEnd() const {
    return getProductsEnd(primaryRequests);
}

void MerchantDialog::addPrimaryRequest(TYPE_OF_ITEM_ID item, string name, TYPE_OF_WORTH price) {
    addProduct(primaryRequests, item, name, price);
}

MerchantDialog::index_type MerchantDialog::getSecondaryRequestsSize() const {
    return getProductsSize(secondaryRequests);
}

MerchantDialog::product_list::const_iterator MerchantDialog::getSecondaryRequestsBegin() const {
    return getProductsBegin(secondaryRequests);
}

MerchantDialog::product_list::const_iterator MerchantDialog::getSecondaryRequestsEnd() const {
    return getProductsEnd(secondaryRequests);
}

void MerchantDialog::addSecondaryRequest(TYPE_OF_ITEM_ID item, string name, TYPE_OF_WORTH price) {
    addProduct(secondaryRequests, item, name, price);
}


MerchantDialog::Result MerchantDialog::getResult() const {
    return result;
}

void MerchantDialog::setResult(Result result) {
    this->result = result;
}

MerchantDialog::index_type MerchantDialog::getPurchaseIndex() const {
    return purchaseIndex;
}

void MerchantDialog::setPurchaseIndex(index_type index) {
    purchaseIndex = index;
}

Item::number_type MerchantDialog::getPurchaseAmount() const {
    return purchaseAmount;
}

void MerchantDialog::setPurchaseAmount(Item::number_type amount) {
    purchaseAmount = amount;
}

ScriptItem MerchantDialog::getSaleItem() const {
    return saleItem;
}

void MerchantDialog::setSaleItem(const ScriptItem &item) {
    saleItem = item;
}

bool MerchantDialog::closeOnMove() const {
    return true;
}

MerchantDialog::index_type MerchantDialog::getProductsSize(const MerchantDialog::product_list &products) const {
    return products.size();
}

MerchantDialog::product_list::const_iterator MerchantDialog::getProductsBegin(const product_list &products) const {
    return products.cbegin();
}

MerchantDialog::product_list::const_iterator MerchantDialog::getProductsEnd(const product_list &products) const {
    return products.cend();
}

void MerchantDialog::addProduct(MerchantDialog::product_list &products, TYPE_OF_ITEM_ID item, string &name, TYPE_OF_WORTH price) {
    if (canAddProduct(products)) {
        Product *product = new Product(item, name, price);
        products.push_back(product);
    }
}

void MerchantDialog::addProduct(MerchantDialog::product_list &products, TYPE_OF_ITEM_ID item, string &name, TYPE_OF_WORTH price, TYPE_OF_BUY_STACK stack) {
    if (canAddProduct(products)) {
        Product *product = new OfferProduct(item, name, price, stack);
        products.push_back(product);
    }
}

bool MerchantDialog::canAddProduct(MerchantDialog::product_list &products) {
    return products.size() < MAXPRODUCTS;
}

