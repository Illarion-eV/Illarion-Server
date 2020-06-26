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

#ifndef CRAFTING_DIALOG_HPP
#define CRAFTING_DIALOG_HPP

#include "Item.hpp"
#include "dialog/Dialog.hpp"

#include <unordered_map>
#include <utility>
#include <vector>

using std::vector;

class Ingredient {
private:
    TYPE_OF_ITEM_ID item;
    uint8_t number;

public:
    explicit Ingredient(TYPE_OF_ITEM_ID item) : item(item), number(1){};
    Ingredient(TYPE_OF_ITEM_ID item, uint8_t number) : item(item), number(number){};
    [[nodiscard]] auto getItem() const -> TYPE_OF_ITEM_ID { return item; };
    [[nodiscard]] auto getNumber() const -> uint8_t { return number; };
};

class Craftable {
public:
    using index_t = uint8_t;
    using ingredients_t = vector<Ingredient>;
    using ingredient_iterator = ingredients_t::const_iterator;

private:
    static const uint32_t MAXINGREDIENTS = 256;
    uint8_t group;
    TYPE_OF_ITEM_ID item;
    string name;
    ingredients_t ingredients;
    uint16_t decisecondsToCraft;
    uint8_t craftedStackSize;

public:
    Craftable(uint8_t group, TYPE_OF_ITEM_ID item, string name, uint16_t decisecondsToCraft)
            : group(group), item(item), name(std::move(name)), decisecondsToCraft(decisecondsToCraft),
              craftedStackSize(1){};
    Craftable(uint8_t group, TYPE_OF_ITEM_ID item, string name, uint16_t decisecondsToCraft, uint8_t craftedStackSize)
            : group(group), item(item), name(std::move(name)), decisecondsToCraft(decisecondsToCraft),
              craftedStackSize(craftedStackSize){};
    [[nodiscard]] auto getGroup() const -> uint8_t { return group; };
    [[nodiscard]] auto getItem() const -> TYPE_OF_ITEM_ID { return item; };
    [[nodiscard]] auto getName() const -> const string & { return name; };
    [[nodiscard]] auto getDecisecondsToCraft() const -> uint16_t { return decisecondsToCraft; };
    [[nodiscard]] auto getCraftedStackSize() const -> uint8_t { return craftedStackSize; };
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
    [[nodiscard]] auto getIngredientsSize() const -> index_t { return ingredients.size(); };
    [[nodiscard]] auto begin() const -> ingredient_iterator { return ingredients.cbegin(); };
    [[nodiscard]] auto end() const -> ingredient_iterator { return ingredients.cend(); };
};

class CraftingDialog : public Dialog {
public:
    using index_t = uint8_t;
    using groups_t = vector<string>;
    using group_iterator = groups_t::const_iterator;
    using craftables_t = std::unordered_map<uint8_t, Craftable>;
    using craftable_iterator = craftables_t::const_iterator;

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
    uint16_t sfx{};
    uint16_t sfxDuration{};
    groups_t groups;
    craftables_t craftables;

    Result result{};

    uint8_t craftableId{};
    Item::number_type craftableAmount{};
    index_t ingredientIndex{};

    uint8_t lastAddedCraftableId{};

public:
    CraftingDialog(const string &title, uint16_t sfx, uint16_t sfxDuration, const luabind::object &callback);

    auto getSfx() const -> uint16_t;
    auto getSfxDuration() const -> uint16_t;

    void clearGroupsAndProducts();

    auto getGroupsSize() const -> index_t;
    auto getGroupsBegin() const -> group_iterator;
    auto getGroupsEnd() const -> group_iterator;
    void addGroup(const string &name);

    auto getCraftablesSize() const -> index_t;
    auto getCraftablesBegin() const -> craftable_iterator;
    auto getCraftablesEnd() const -> craftable_iterator;
    void addCraftable(uint8_t id, uint8_t group, TYPE_OF_ITEM_ID item, const string &name, uint16_t decisecondsToCraft);
    void addCraftable(uint8_t id, uint8_t group, TYPE_OF_ITEM_ID item, const string &name, uint16_t decisecondsToCraft,
                      uint8_t craftedStackSize);
    void addCraftableIngredient(TYPE_OF_ITEM_ID item);
    void addCraftableIngredient(TYPE_OF_ITEM_ID item, uint8_t number);

    auto getResult() const -> Result;
    void setResult(Result result);

    auto getCraftableId() const -> uint8_t;
    void setCraftableId(uint8_t index);
    auto getCraftableAmount() const -> Item::number_type;
    void setCraftableAmount(Item::number_type amount);
    auto getIngredientIndex() const -> index_t;
    void setIngredientIndex(index_t index);
    auto getCraftableTime() const -> uint16_t;

    auto closeOnMove() const -> bool override;

private:
    auto canAddCraftable(uint8_t group) -> bool;
};

#endif
