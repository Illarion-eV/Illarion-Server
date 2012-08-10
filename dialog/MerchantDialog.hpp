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

#ifndef _MERCHANT_DIALOG_HPP_
#define _MERCHANT_DIALOG_HPP_

#include "dialog/Dialog.hpp"
#include "Item.hpp"
#include <string>
#include <vector>


using std::string;
using std::vector;


class Product {
private:
    TYPE_OF_ITEM_ID item;
    string name;
    TYPE_OF_WORTH price;

public:
    Product(TYPE_OF_ITEM_ID item, string name, TYPE_OF_WORTH price): item(item), name(name), price(price) {};
    TYPE_OF_ITEM_ID getItem() {return item;};
    string& getName() {return name;};
    TYPE_OF_WORTH getPrice() {return price;};
};


class OfferProduct: public Product {
private:
    TYPE_OF_BUY_STACK stack;

public:
    OfferProduct(TYPE_OF_ITEM_ID item, string name, TYPE_OF_WORTH price, TYPE_OF_BUY_STACK stack): Product(item, name, price), stack(stack) {};
    TYPE_OF_BUY_STACK getStack() {return stack;};
};


class MerchantDialog: public Dialog {
public:
    typedef uint8_t index_type;
    typedef vector<Product *> product_list;
    enum Result {
        playerAborts = 0,
        playerSells = 1,
        playerBuys = 2
    };

private:
    static const uint32_t MAXPRODUCTS = 256;
    product_list offers;
    product_list primaryRequests;
    product_list secondaryRequests;

    Result result;

    index_type purchaseIndex;
    Item::number_type purchaseAmount;

    ScriptItem saleItem;

public:
    MerchantDialog(string title, luabind::object callback);
    MerchantDialog(const MerchantDialog &merchantDialog);
    ~MerchantDialog();

    index_type getOffersSize() const;
    product_list::const_iterator getOffersBegin() const;
    product_list::const_iterator getOffersEnd() const;
    void addOffer(TYPE_OF_ITEM_ID item, string name, TYPE_OF_WORTH price, TYPE_OF_BUY_STACK stack);

    index_type getPrimaryRequestsSize() const;
    product_list::const_iterator getPrimaryRequestsBegin() const;
    product_list::const_iterator getPrimaryRequestsEnd() const;
    void addPrimaryRequest(TYPE_OF_ITEM_ID item, string name, TYPE_OF_WORTH price);

    index_type getSecondaryRequestsSize() const;
    product_list::const_iterator getSecondaryRequestsBegin() const;
    product_list::const_iterator getSecondaryRequestsEnd() const;
    void addSecondaryRequest(TYPE_OF_ITEM_ID item, string name, TYPE_OF_WORTH price);

    Result getResult() const;
    void setResult(Result result);

    index_type getPurchaseIndex() const;
    void setPurchaseIndex(index_type index);
    Item::number_type getPurchaseAmount() const;
    void setPurchaseAmount(Item::number_type amount);

    ScriptItem getSaleItem() const;
    void setSaleItem(const ScriptItem &item);

private:
    index_type getProductsSize(const product_list &products) const;
    product_list::const_iterator getProductsBegin(const product_list &products) const;
    product_list::const_iterator getProductsEnd(const product_list &products) const;
    void addProduct(product_list &products, TYPE_OF_ITEM_ID item, string &name, TYPE_OF_WORTH price);
    void addProduct(product_list &products, TYPE_OF_ITEM_ID item, string &name, TYPE_OF_WORTH price, TYPE_OF_BUY_STACK stack);
    bool canAddProduct(product_list &products);
};

#endif

