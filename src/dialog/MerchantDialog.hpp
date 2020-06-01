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

#ifndef MERCHANT_DIALOG_HPP
#define MERCHANT_DIALOG_HPP

#include "Item.hpp"
#include "dialog/Dialog.hpp"

#include <utility>
#include <vector>

using std::vector;

class Product {
private:
    TYPE_OF_ITEM_ID item;
    string name;
    TYPE_OF_WORTH price;

public:
    Product(TYPE_OF_ITEM_ID item, string name, TYPE_OF_WORTH price) : item(item), name(std::move(name)), price(price){};
    [[nodiscard]] auto getItem() const -> TYPE_OF_ITEM_ID { return item; };
    [[nodiscard]] auto getName() const -> const string & { return name; };
    [[nodiscard]] auto getPrice() const -> TYPE_OF_WORTH { return price; };
};

class OfferProduct : public Product {
private:
    TYPE_OF_BUY_STACK stack;

public:
    OfferProduct(TYPE_OF_ITEM_ID item, const string &name, TYPE_OF_WORTH price, TYPE_OF_BUY_STACK stack)
            : Product(item, name, price), stack(stack){};
    [[nodiscard]] auto getStack() const -> TYPE_OF_BUY_STACK { return stack; };
};

class MerchantDialog : public Dialog {
public:
    using index_type = uint8_t;
    using product_list = vector<Product>;
    using product_iterator = product_list::const_iterator;
    using offer_list = vector<OfferProduct>;
    using offer_iterator = offer_list::const_iterator;

    enum Result { playerAborts = 0, playerSells = 1, playerBuys = 2, playerLooksAt = 3 };

    enum ListType { listSell = 0, listBuyPrimary = 1, listBuySecondary = 2 };

private:
    static const uint32_t MAXPRODUCTS = 256;
    offer_list offers;
    product_list primaryRequests;
    product_list secondaryRequests;

    Result result;

    index_type purchaseIndex;
    Item::number_type purchaseAmount;

    ScriptItem saleItem;

    ListType lookAtList;

public:
    MerchantDialog(const string &title, const luabind::object &callback);
    MerchantDialog(const MerchantDialog &merchantDialog);

    auto getOffersSize() const -> index_type;
    auto getOffersBegin() const -> offer_iterator;
    auto getOffersEnd() const -> offer_iterator;
    void addOffer(TYPE_OF_ITEM_ID item, const string &name, TYPE_OF_WORTH price);
    void addOffer(TYPE_OF_ITEM_ID item, const string &name, TYPE_OF_WORTH price, TYPE_OF_BUY_STACK stack);

    auto getPrimaryRequestsSize() const -> index_type;
    auto getPrimaryRequestsBegin() const -> product_iterator;
    auto getPrimaryRequestsEnd() const -> product_iterator;
    void addPrimaryRequest(TYPE_OF_ITEM_ID item, const string &name, TYPE_OF_WORTH price);

    auto getSecondaryRequestsSize() const -> index_type;
    auto getSecondaryRequestsBegin() const -> product_iterator;
    auto getSecondaryRequestsEnd() const -> product_iterator;
    void addSecondaryRequest(TYPE_OF_ITEM_ID item, const string &name, TYPE_OF_WORTH price);

    auto getResult() const -> Result;
    void setResult(Result result);

    auto getPurchaseIndex() const -> index_type;
    void setPurchaseIndex(index_type index);
    auto getPurchaseAmount() const -> Item::number_type;
    void setPurchaseAmount(Item::number_type amount);

    auto getSaleItem() const -> const ScriptItem &;
    void setSaleItem(const ScriptItem &item);

    auto getLookAtList() const -> ListType;
    void setLookAtList(ListType list);

    auto closeOnMove() const -> bool override;

private:
    static auto getProductsSize(const product_list &products) -> index_type;
    static auto getProductsBegin(const product_list &products) -> product_iterator;
    static auto getProductsEnd(const product_list &products) -> product_iterator;
    static void addProduct(product_list &products, TYPE_OF_ITEM_ID item, const string &name, TYPE_OF_WORTH price);
    auto canAddOffer() const -> bool;
    static auto canAddProduct(const product_list &products) -> bool;
};

#endif
