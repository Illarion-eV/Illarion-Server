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

MerchantDialog::MerchantDialog(std::string title, luabind::object callback)
    :Dialog(title, "MerchantDialog", callback) {
    result = playerAborts;
    purchaseIndex = 0;
    purchaseAmount = 0;
}

MerchantDialog::MerchantDialog(const MerchantDialog &merchantDialog) : Dialog(merchantDialog) {
    wares = merchantDialog.wares;
    result = merchantDialog.result;
    purchaseIndex = merchantDialog.purchaseIndex;
    purchaseAmount = merchantDialog.purchaseAmount;
    saleItem = merchantDialog.saleItem;
}

MerchantDialog::~MerchantDialog() {
    for (auto it = wares.begin(); it != wares.end(); ++it) {
        delete *it;
    }
}

MerchantDialog::index_type MerchantDialog::getProductsSize() const {
    return wares.size();
}

vector<Product *>::const_iterator MerchantDialog::getProductsBegin() const {
    return wares.cbegin();
}

vector<Product *>::const_iterator MerchantDialog::getProductsEnd() const {
    return wares.cend();
}

void MerchantDialog::addProduct(TYPE_OF_ITEM_ID item, string name, TYPE_OF_WORTH price) {
    if (wares.size() < MAXWARES) {
        Product *product = new Product();
        product->item = item;
        product->name = name;
        product->price = price;
        wares.push_back(product);
    }
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

