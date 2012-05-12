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

struct Product {
    TYPE_OF_ITEM_ID item;
    string name;
    TYPE_OF_WORTH price;
};

class MerchantDialog: public Dialog {
public:
    typedef uint8_t index_type;
    enum Result {
        playerAborts = 0,
        playerSells = 1,
        playerBuys = 2
    };

private:
    static const uint32_t MAXWARES = 256;
    vector<Product *> wares;

    Result result;

    index_type purchaseIndex;
    Item::number_type purchaseAmount;

    ScriptItem saleItem;

public:
    MerchantDialog(string title, luabind::object callback);
    MerchantDialog(const MerchantDialog &merchantDialog);
    ~MerchantDialog();

    index_type getProductsSize() const;
    vector<Product *>::const_iterator getProductsBegin() const;
    vector<Product *>::const_iterator getProductsEnd() const;
    void addProduct(TYPE_OF_ITEM_ID item, string name, TYPE_OF_WORTH price);

    Result getResult() const;
    void setResult(Result result);

    index_type getPurchaseIndex() const;
    void setPurchaseIndex(index_type index);
    Item::number_type getPurchaseAmount() const;
    void setPurchaseAmount(Item::number_type amount);

    ScriptItem getSaleItem() const;
    void setSaleItem(const ScriptItem &item);
};

#endif

