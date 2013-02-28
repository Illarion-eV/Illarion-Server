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

#ifndef _CRAFTING_DIALOG_HPP_
#define _CRAFTING_DIALOG_HPP_

#include "dialog/Dialog.hpp"
#include "Item.hpp"
#include <string>
#include <vector>
#include <unordered_map>


using std::string;
using std::vector;


class Ingredient {
private:
    TYPE_OF_ITEM_ID item;
    uint8_t number;

public:
    Ingredient(TYPE_OF_ITEM_ID item): item(item), number(1) {};
    Ingredient(TYPE_OF_ITEM_ID item, uint8_t number): item(item), number(number) {};
    TYPE_OF_ITEM_ID getItem() const {
        return item;
    };
    uint8_t getNumber() const {
        return number;
    };
};


class Craftable {
public:
    typedef uint8_t index_t;
    typedef vector<Ingredient> ingredients_t;
    typedef ingredients_t::const_iterator ingredient_iterator;

private:
    static const uint32_t MAXINGREDIENTS = 256;
    uint8_t group;
    TYPE_OF_ITEM_ID item;
    string name;
    ingredients_t ingredients;
    uint16_t decisecondsToCraft;
    uint8_t craftedStackSize;

public:
    Craftable(uint8_t group, TYPE_OF_ITEM_ID item, const string &name, uint16_t decisecondsToCraft): group(group), item(item), name(name), decisecondsToCraft(decisecondsToCraft), craftedStackSize(1) {};
    Craftable(uint8_t group, TYPE_OF_ITEM_ID item, const string &name, uint16_t decisecondsToCraft, uint8_t craftedStackSize): group(group), item(item), name(name), decisecondsToCraft(decisecondsToCraft), craftedStackSize(craftedStackSize) {};
    Craftable(const Craftable &craftable) {
        group = craftable.group;
        item = craftable.item;
        name = craftable.name;
        decisecondsToCraft = craftable.decisecondsToCraft;
        craftedStackSize = craftable.craftedStackSize;

        for (const auto &ingredient : craftable.ingredients) {
            addIngredient(ingredient.getItem(), ingredient.getNumber());
        }
    };
    uint8_t getGroup() const {
        return group;
    };
    TYPE_OF_ITEM_ID getItem() const {
        return item;
    };
    const string &getName() const {
        return name;
    };
    uint16_t getDecisecondsToCraft() const {
        return decisecondsToCraft;
    };
    uint8_t getCraftedStackSize() const {
        return craftedStackSize;
    };
    void addIngredient(TYPE_OF_ITEM_ID item) {
        if (ingredients.size() < MAXINGREDIENTS) {
            ingredients.emplace_back(item);
        }
    };
    void addIngredient(TYPE_OF_ITEM_ID item, uint8_t number) {
        if (ingredients.size() < MAXINGREDIENTS) {
            ingredients.emplace_back(item, number);
        }
    };
    index_t getIngredientsSize() const {
        return ingredients.size();
    };
    ingredient_iterator begin() const {
        return ingredients.cbegin();
    };
    ingredient_iterator end() const {
        return ingredients.cend();
    };
};


class CraftingDialog: public Dialog {
public:
    typedef uint8_t index_t;
    typedef vector<string> groups_t;
    typedef groups_t::const_iterator group_iterator;
    typedef std::unordered_map<uint8_t, Craftable> craftables_t;
    typedef craftables_t::const_iterator craftable_iterator;

    enum Result {
        playerAborts = 0,
        playerCrafts = 1,
        playerLooksAtCraftable = 2,
        playerLooksAtIngredient = 3,
        playerCraftingComplete = 4,
        playerCraftingAborted = 5
    };

private:
    static const uint32_t MAXCRAFTABLES = 256;
    uint16_t sfx;
    uint16_t sfxDuration;
    groups_t groups;
    craftables_t craftables;

    Result result;

    uint8_t craftableId;
    Item::number_type craftableAmount;
    index_t ingredientIndex;

    uint8_t lastAddedCraftableId;

public:
    CraftingDialog(const string &title, uint16_t sfx, uint16_t sfxDuration, const luabind::object &callback);
    CraftingDialog(const CraftingDialog &craftingDialog);

    uint16_t getSfx() const;
    uint16_t getSfxDuration() const;

    void clearGroupsAndProducts();

    index_t getGroupsSize() const;
    group_iterator getGroupsBegin() const;
    group_iterator getGroupsEnd() const;
    void addGroup(const std::string &name);

    index_t getCraftablesSize() const;
    craftable_iterator getCraftablesBegin() const;
    craftable_iterator getCraftablesEnd() const;
    void addCraftable(uint8_t id, uint8_t group, TYPE_OF_ITEM_ID item, const string &name, uint16_t decisecondsToCraft);
    void addCraftable(uint8_t id, uint8_t group, TYPE_OF_ITEM_ID item, const string &name, uint16_t decisecondsToCraft, uint8_t craftedStackSize);
    void addCraftableIngredient(TYPE_OF_ITEM_ID item);
    void addCraftableIngredient(TYPE_OF_ITEM_ID item, uint8_t number);

    Result getResult() const;
    void setResult(Result result);

    uint8_t getCraftableId() const;
    void setCraftableId(uint8_t index);
    Item::number_type getCraftableAmount() const;
    void setCraftableAmount(Item::number_type amount);
    index_t getIngredientIndex() const;
    void setIngredientIndex(index_t index);
    uint16_t getCraftableTime() const;

    virtual bool closeOnMove() const override;

private:
    bool canAddCraftable(uint8_t group);
};

#endif

