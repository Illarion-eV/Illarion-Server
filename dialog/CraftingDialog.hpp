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


using std::string;
using std::vector;


class Ingredient {
private:
    TYPE_OF_ITEM_ID item;
    uint8_t number;

public:
    Ingredient(TYPE_OF_ITEM_ID item): item(item), number(1) {};
    Ingredient(TYPE_OF_ITEM_ID item, uint8_t number): item(item), number(number) {};
    TYPE_OF_ITEM_ID getItem() {
        return item;
    };
    uint8_t getNumber() {
        return number;
    };
};


class Craftable {
public:
    typedef uint8_t index_t;
    typedef vector<Ingredient *> ingredients_t;
    
private:
    static const uint32_t MAXINGREDIENTS = 256;
    uint8_t group;
    TYPE_OF_ITEM_ID item;
    string name;
    ingredients_t ingredients;
    uint16_t decisecondsToCraft;
    uint8_t craftedStackSize;

public:
    Craftable(uint8_t group, TYPE_OF_ITEM_ID item, string name, uint16_t decisecondsToCraft): item(item), name(name), decisecondsToCraft(decisecondsToCraft), craftedStackSize(1) {};
    Craftable(uint8_t group, TYPE_OF_ITEM_ID item, string name, uint16_t decisecondsToCraft, uint8_t craftedStackSize): item(item), name(name), decisecondsToCraft(decisecondsToCraft), craftedStackSize(craftedStackSize) {};
    Craftable(const Craftable &craftable) {
        group = craftable.group;
        item = craftable.item;
        name = craftable.name;
        decisecondsToCraft = craftable.decisecondsToCraft;
        craftedStackSize = craftable.craftedStackSize;
        for (auto it = craftable.getIngredientsBegin(); it != craftable.getIngredientsEnd(); ++it) {
            Ingredient &ingredient = **it;
            addIngredient(ingredient.getItem(), ingredient.getNumber());
        }
    };
    ~Craftable() {
        for (auto it = ingredients.begin(); it < ingredients.end(); ++it) {
            delete *it;
            *it = 0;
        }
    };
    uint8_t getGroup() {
        return group;
    };
    TYPE_OF_ITEM_ID getItem() {
        return item;
    };
    string &getName() {
        return name;
    };
    uint16_t getDecisecondsToCraft() {
        return decisecondsToCraft;
    };
    uint8_t getCraftedStackSize() {
        return craftedStackSize;
    };
    void addIngredient(TYPE_OF_ITEM_ID item) {
        if (ingredients.size() < MAXINGREDIENTS) ingredients.push_back(new Ingredient(item));
    };
    void addIngredient(TYPE_OF_ITEM_ID item, uint8_t number) {
        if (ingredients.size() < MAXINGREDIENTS) ingredients.push_back(new Ingredient(item, number));
    };
    index_t getIngredientsSize() {
        return ingredients.size();
    };
    ingredients_t::const_iterator getIngredientsBegin() const {
        return ingredients.cbegin();
    };
    ingredients_t::const_iterator getIngredientsEnd() const {
        return ingredients.cend();
    };
};


class CraftingDialog: public Dialog {
public:
    typedef uint8_t index_t;
    typedef vector<string> groups_t;
    typedef vector<Craftable *> craftables_t;
    enum Result {
        playerAborts = 0,
        playerCrafts = 1,
        playerLooksAtCraftable = 2,
        playerLooksAtIngredient = 3
    };

private:
    static const uint32_t MAXCRAFTABLES = 256;
    groups_t groups;
    craftables_t craftables;

    Result result;

    index_t craftableIndex;
    Item::number_type craftableAmount;
    index_t ingredientIndex;

public:
    CraftingDialog(string title, luabind::object callback);
    CraftingDialog(const CraftingDialog &craftingDialog);
    ~CraftingDialog();

    index_t getGroupsSize() const;
    groups_t::const_iterator getGroupsBegin() const;
    groups_t::const_iterator getGroupsEnd() const;
    void addGroup(std::string name);

    index_t getCraftablesSize() const;
    craftables_t::const_iterator getCraftablesBegin() const;
    craftables_t::const_iterator getCraftablesEnd() const;
    void addCraftable(uint8_t group, TYPE_OF_ITEM_ID item, string name, uint16_t decisecondsToCraft);
    void addCraftable(uint8_t group, TYPE_OF_ITEM_ID item, string name, uint16_t decisecondsToCraft, uint8_t craftedStackSize);
    void addCraftableIngredient(TYPE_OF_ITEM_ID item);
    void addCraftableIngredient(TYPE_OF_ITEM_ID item, uint8_t number);

    Result getResult() const;
    void setResult(Result result);

    index_t getCraftableIndex() const;
    void setCraftableIndex(index_t index);
    Item::number_type getCraftableAmount() const;
    void setCraftableAmount(Item::number_type amount);
    index_t getIngredientIndex() const;
    void setIngredientIndex(index_t index);

private:
    bool canAddCraftable(uint8_t group);
};

#endif

