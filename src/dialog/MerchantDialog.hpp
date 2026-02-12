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

/**
 * @brief Represents a product that can be bought or sold in a merchant dialog.
 */
class Product {
private:
    TYPE_OF_ITEM_ID item; ///< Item ID for the product.
    string name; ///< Display name of the product.
    TYPE_OF_WORTH price; ///< Price of the product in currency.

public:
    /**
     * @brief Constructs a product with item, name, and price.
     * @param item Item ID.
     * @param name Display name.
     * @param price Price in currency.
     */
    Product(TYPE_OF_ITEM_ID item, string name, TYPE_OF_WORTH price) : item(item), name(std::move(name)), price(price){};

    /**
     * @brief Gets the item ID.
     * @return The item ID.
     */
    [[nodiscard]] auto getItem() const -> TYPE_OF_ITEM_ID { return item; };

    /**
     * @brief Gets the product name.
     * @return The display name.
     */
    [[nodiscard]] auto getName() const -> const string & { return name; };

    /**
     * @brief Gets the product price.
     * @return The price value.
     */
    [[nodiscard]] auto getPrice() const -> TYPE_OF_WORTH { return price; };
};

/**
 * @brief A product offered for sale by the merchant, with stack size information.
 */
class OfferProduct : public Product {
private:
    TYPE_OF_BUY_STACK stack; ///< Default stack size when purchasing.

public:
    /**
     * @brief Constructs an offered product with stack size.
     * @param item Item ID.
     * @param name Display name.
     * @param price Price per stack.
     * @param stack Number of items per purchase.
     */
    OfferProduct(TYPE_OF_ITEM_ID item, const string &name, TYPE_OF_WORTH price, TYPE_OF_BUY_STACK stack)
            : Product(item, name, price), stack(stack){};

    /**
     * @brief Gets the stack size for this product.
     * @return The number of items per purchase.
     */
    [[nodiscard]] auto getStack() const -> TYPE_OF_BUY_STACK { return stack; };
};

/**
 * @brief A merchant/trading dialog for buying and selling items.
 *
 * This dialog implements a merchant interface with three lists:
 * - Offers: Items the merchant sells to the player
 * - Primary Requests: Items the merchant primarily wants to buy
 * - Secondary Requests: Items the merchant will also buy (usually at lower prices)
 *
 * The player can browse items, purchase from offers, sell items to the merchant,
 * or examine items for details.
 */
class MerchantDialog : public Dialog {
public:
    using index_type = uint8_t; ///< Type for product indices.
    using product_list = vector<Product>; ///< Container for products.
    using product_iterator = product_list::const_iterator; ///< Iterator for products.
    using offer_list = vector<OfferProduct>; ///< Container for offered products.
    using offer_iterator = offer_list::const_iterator; ///< Iterator for offers.

    /**
     * @brief Player's action result from the merchant dialog.
     */
    enum Result { 
        playerAborts = 0,   ///< Player closed dialog without action.
        playerSells = 1,    ///< Player sold an item to merchant.
        playerBuys = 2,     ///< Player purchased an item from merchant.
        playerLooksAt = 3   ///< Player examined an item's details.
    };

    /**
     * @brief Which product list the player is interacting with.
     */
    enum ListType { 
        listSell = 0,           ///< Merchant's offers (items for sale).
        listBuyPrimary = 1,     ///< Primary items merchant wants to buy.
        listBuySecondary = 2    ///< Secondary items merchant will buy.
    };

private:
    static const uint32_t MAXPRODUCTS = 256; ///< Maximum products per list.
    offer_list offers; ///< Items the merchant offers for sale.
    product_list primaryRequests; ///< Items the merchant primarily wants to buy.
    product_list secondaryRequests; ///< Items the merchant will also buy.

    Result result{playerAborts}; ///< The player's action result.

    index_type purchaseIndex{0}; ///< Index of item being purchased.
    Item::number_type purchaseAmount{0}; ///< Quantity of item being purchased.

    ScriptItem saleItem; ///< Item the player is selling to the merchant.

    ListType lookAtList{listSell}; ///< Which list the examined item is in.

public:
    /**
     * @brief Constructs a merchant dialog.
     * @param title The dialog window title (usually merchant's name).
     * @param callback Lua function called when transaction completes.
     */
    MerchantDialog(const string &title, const luabind::object &callback);

    /**
     * @brief Gets the number of items offered for sale.
     * @return The offer count.
     */
    auto getOffersSize() const -> index_type;

    /**
     * @brief Gets iterator to first offered item.
     * @return Begin iterator for offers.
     */
    auto getOffersBegin() const -> offer_iterator;

    /**
     * @brief Gets iterator past last offered item.
     * @return End iterator for offers.
     */
    auto getOffersEnd() const -> offer_iterator;

    /**
     * @brief Adds an item to the merchant's sale offerings.
     * @param item Item ID to offer.
     * @param name Display name of the item.
     * @param price Price per item.
     */
    void addOffer(TYPE_OF_ITEM_ID item, const string &name, TYPE_OF_WORTH price);

    /**
     * @brief Adds an item with stack size to merchant's offerings.
     * @param item Item ID to offer.
     * @param name Display name of the item.
     * @param price Price per stack.
     * @param stack Number of items per purchase.
     */
    void addOffer(TYPE_OF_ITEM_ID item, const string &name, TYPE_OF_WORTH price, TYPE_OF_BUY_STACK stack);

    /**
     * @brief Gets the number of primary buy requests.
     * @return The primary request count.
     */
    auto getPrimaryRequestsSize() const -> index_type;

    /**
     * @brief Gets iterator to first primary request.
     * @return Begin iterator.
     */
    auto getPrimaryRequestsBegin() const -> product_iterator;

    /**
     * @brief Gets iterator past last primary request.
     * @return End iterator.
     */
    auto getPrimaryRequestsEnd() const -> product_iterator;

    /**
     * @brief Adds an item to primary buy requests.
     * @param item Item ID the merchant wants.
     * @param name Display name.
     * @param price Price merchant will pay.
     */
    void addPrimaryRequest(TYPE_OF_ITEM_ID item, const string &name, TYPE_OF_WORTH price);

    /**
     * @brief Gets the number of secondary buy requests.
     * @return The secondary request count.
     */
    auto getSecondaryRequestsSize() const -> index_type;

    /**
     * @brief Gets iterator to first secondary request.
     * @return Begin iterator.
     */
    auto getSecondaryRequestsBegin() const -> product_iterator;

    /**
     * @brief Gets iterator past last secondary request.
     * @return End iterator.
     */
    auto getSecondaryRequestsEnd() const -> product_iterator;

    /**
     * @brief Adds an item to secondary buy requests.
     * @param item Item ID the merchant will buy.
     * @param name Display name.
     * @param price Price merchant will pay.
     */
    void addSecondaryRequest(TYPE_OF_ITEM_ID item, const string &name, TYPE_OF_WORTH price);

    /**
     * @brief Gets the player's action result.
     * @return The result enum value.
     */
    auto getResult() const -> Result;

    /**
     * @brief Sets the player's action result.
     * @param result The result value.
     */
    void setResult(Result result);

    /**
     * @brief Gets the index of item being purchased/examined.
     * @return The item index.
     */
    auto getPurchaseIndex() const -> index_type;

    /**
     * @brief Sets the index of item being purchased/examined.
     * @param index The item index.
     */
    void setPurchaseIndex(index_type index);

    /**
     * @brief Gets the quantity being purchased.
     * @return The purchase amount.
     */
    auto getPurchaseAmount() const -> Item::number_type;

    /**
     * @brief Sets the quantity being purchased.
     * @param amount The purchase amount.
     */
    void setPurchaseAmount(Item::number_type amount);

    /**
     * @brief Gets the item the player is selling.
     * @return The sale item.
     */
    auto getSaleItem() const -> const ScriptItem &;

    /**
     * @brief Sets the item the player is selling.
     * @param item The item being sold.
     */
    void setSaleItem(const ScriptItem &item);

    /**
     * @brief Gets which list the examined item is in.
     * @return The list type.
     */
    auto getLookAtList() const -> ListType;

    /**
     * @brief Sets which list the examined item is in.
     * @param list The list type.
     */
    void setLookAtList(ListType list);

    /**
     * @brief Checks if this dialog closes on player movement.
     * @return Always false - merchant dialogs stay open.
     */
    auto closeOnMove() const -> bool override;

private:
    /**
     * @brief Helper to get product list size.
     */
    static auto getProductsSize(const product_list &products) -> index_type;

    /**
     * @brief Helper to get product list begin iterator.
     */
    static auto getProductsBegin(const product_list &products) -> product_iterator;

    /**
     * @brief Helper to get product list end iterator.
     */
    static auto getProductsEnd(const product_list &products) -> product_iterator;

    /**
     * @brief Helper to add product to a list.
     */
    static void addProduct(product_list &products, TYPE_OF_ITEM_ID item, const string &name, TYPE_OF_WORTH price);

    /**
     * @brief Checks if another offer can be added.
     */
    auto canAddOffer() const -> bool;

    /**
     * @brief Checks if another product can be added to a list.
     */
    static auto canAddProduct(const product_list &products) -> bool;
};

#endif
