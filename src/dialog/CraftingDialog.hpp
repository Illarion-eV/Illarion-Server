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

#include <map>
#include <utility>
#include <vector>

using std::vector;

/**
 * @brief Ingredient requirement for crafting
 *
 * Specifies an item type and quantity required as input for a crafting recipe.
 */
class Ingredient {
private:
    TYPE_OF_ITEM_ID item;   ///< Item ID required
    uint8_t number;         ///< Quantity needed

public:
    /**
     * @brief Construct ingredient with single item
     * @param item Item ID required
     */
    explicit Ingredient(TYPE_OF_ITEM_ID item) : item(item), number(1){};

    /**
     * @brief Construct ingredient with specific quantity
     * @param item Item ID required
     * @param number Quantity needed
     */
    Ingredient(TYPE_OF_ITEM_ID item, uint8_t number) : item(item), number(number){};

    /**
     * @brief Get item ID
     * @return Required item ID
     */
    [[nodiscard]] auto getItem() const -> TYPE_OF_ITEM_ID { return item; };

    /**
     * @brief Get required quantity
     * @return Number of items needed
     */
    [[nodiscard]] auto getNumber() const -> uint8_t { return number; };
};

/**
 * @brief Craftable item recipe definition
 *
 * Defines a complete crafting recipe including:
 * - Output item and quantity
 * - Required ingredients and quantities
 * - Crafting time
 * - Display name and category group
 */
class Craftable {
public:
    using index_t = uint8_t;
    using ingredients_t = vector<Ingredient>;
    using ingredient_iterator = ingredients_t::const_iterator;

private:
    static const uint32_t MAXINGREDIENTS = 256;
    uint8_t group;                  ///< Category/group ID for organization
    TYPE_OF_ITEM_ID item;           ///< Item produced by this recipe
    string name;                    ///< Display name of the recipe
    ingredients_t ingredients;      ///< Required ingredients
    uint16_t decisecondsToCraft;    ///< Crafting time in deciseconds (1/10 sec)
    uint8_t craftedStackSize;       ///< Number of items produced

public:
    /**
     * @brief Construct craftable with single output item
     * @param group Category group
     * @param item Item ID produced
     * @param name Display name
     * @param decisecondsToCraft Crafting time in deciseconds
     */
    Craftable(uint8_t group, TYPE_OF_ITEM_ID item, string name, uint16_t decisecondsToCraft)
            : group(group), item(item), name(std::move(name)), decisecondsToCraft(decisecondsToCraft),
              craftedStackSize(1){};

    /**
     * @brief Construct craftable with stack output
     * @param group Category group
     * @param item Item ID produced
     * @param name Display name
     * @param decisecondsToCraft Crafting time in deciseconds
     * @param craftedStackSize Number of items produced
     */
    Craftable(uint8_t group, TYPE_OF_ITEM_ID item, string name, uint16_t decisecondsToCraft, uint8_t craftedStackSize)
            : group(group), item(item), name(std::move(name)), decisecondsToCraft(decisecondsToCraft),
              craftedStackSize(craftedStackSize){};

    [[nodiscard]] auto getGroup() const -> uint8_t { return group; };
    [[nodiscard]] auto getItem() const -> TYPE_OF_ITEM_ID { return item; };
    [[nodiscard]] auto getName() const -> const string & { return name; };
    [[nodiscard]] auto getDecisecondsToCraft() const -> uint16_t { return decisecondsToCraft; };
    [[nodiscard]] auto getCraftedStackSize() const -> uint8_t { return craftedStackSize; };

    /**
     * @brief Add single ingredient requirement
     * @param item Item ID required
     */
    void addIngredient(TYPE_OF_ITEM_ID item) {
        if (ingredients.size() < MAXINGREDIENTS) {
            ingredients.emplace_back(item);
        }
    };

    /**
     * @brief Add ingredient with specific quantity
     * @param item Item ID required
     * @param number Quantity needed
     */
    void addIngredient(TYPE_OF_ITEM_ID item, uint8_t number) {
        if (ingredients.size() < MAXINGREDIENTS) {
            ingredients.emplace_back(item, number);
        }
    };

    [[nodiscard]] auto getIngredientsSize() const -> index_t { return ingredients.size(); };
    [[nodiscard]] auto begin() const -> ingredient_iterator { return ingredients.cbegin(); };
    [[nodiscard]] auto end() const -> ingredient_iterator { return ingredients.cend(); };
};

/**
 * @brief Interactive crafting dialog
 *
 * Presents a crafting interface to players showing:
 * - Recipe categories/groups
 * - Available recipes with ingredient requirements
 * - Crafting progress and completion
 * - Item inspection for outputs and ingredients
 *
 * Supports:
 * - Multiple recipe categories for organization
 * - Variable crafting times
 * - Stack crafting (multiple outputs)
 * - Sound effects during crafting
 * - Callbacks for result handling
 *
 * Results indicate player actions:
 * - Selecting and starting a craft
 * - Inspecting items
 * - Aborting crafting
 * - Completing crafts
 */
class CraftingDialog : public Dialog {
public:
    using index_t = uint8_t;
    using groups_t = vector<string>;
    using group_iterator = groups_t::const_iterator;
    using craftables_t = std::map<uint8_t, Craftable>;
    using craftable_iterator = craftables_t::const_iterator;

    /**
     * @brief Player actions in crafting dialog
     */
    enum Result {
        playerAborts = 0,              ///< Player closed dialog
        playerCrafts = 1,              ///< Player started crafting
        playerLooksAtCraftable = 2,    ///< Player inspected output item
        playerLooksAtIngredient = 3,   ///< Player inspected ingredient
        playerCraftingComplete = 4,    ///< Crafting finished successfully
        playerCraftingAborted = 5      ///< Crafting was interrupted
    };

private:
    static constexpr auto maximumCraftables = 256;  ///< Max recipes that can be shown
    static constexpr auto maximumGroups = 256;      ///< Max category groups
    uint16_t sfx{};                 ///< Sound effect ID during crafting
    uint16_t sfxDuration{};         ///< Duration of sound effect
    groups_t groups;                ///< Recipe category names
    craftables_t craftables;        ///< Available recipes

    Result result{playerAborts};    ///< Last player action

    uint8_t craftableId{0};                 ///< Selected recipe ID
    Item::number_type craftableAmount{0};   ///< Quantity to craft
    index_t ingredientIndex{0};             ///< Inspected ingredient index

    uint8_t lastAddedCraftableId{0};        ///< Last added recipe (for ingredient assignment)

public:
    /**
     * @brief Construct crafting dialog
     * @param title Dialog title
     * @param sfx Sound effect ID for crafting
     * @param sfxDuration Sound effect duration in deciseconds
     * @param callback Lua callback function
     */
    CraftingDialog(const string &title, uint16_t sfx, uint16_t sfxDuration, const luabind::object &callback);

    [[nodiscard]] auto getSfx() const -> uint16_t;
    [[nodiscard]] auto getSfxDuration() const -> uint16_t;

    /**
     * @brief Clear all groups and recipes
     *
     * Removes all categories and craftable items, allowing the dialog
     * to be repopulated with new recipes.
     */
    void clearGroupsAndProducts();

    [[nodiscard]] auto getGroupsSize() const -> index_t;
    [[nodiscard]] auto getGroupsBegin() const -> group_iterator;
    [[nodiscard]] auto getGroupsEnd() const -> group_iterator;

    /**
     * @brief Add a recipe category
     * @param name Category name to display
     */
    void addGroup(const string &name);

    [[nodiscard]] auto getCraftablesSize() const -> index_t;
    [[nodiscard]] auto getCraftablesBegin() const -> craftable_iterator;
    [[nodiscard]] auto getCraftablesEnd() const -> craftable_iterator;

    /**
     * @brief Add a craftable recipe
     * @param id Recipe ID
     * @param group Category group ID
     * @param item Output item ID
     * @param name Recipe display name
     * @param decisecondsToCraft Crafting time in 1/10 seconds
     */
    void addCraftable(uint8_t id, uint8_t group, TYPE_OF_ITEM_ID item, const string &name, uint16_t decisecondsToCraft);

    /**
     * @brief Add craftable recipe with stack output
     * @param id Recipe ID
     * @param group Category group ID
     * @param item Output item ID
     * @param name Recipe display name
     * @param decisecondsToCraft Crafting time in 1/10 seconds
     * @param craftedStackSize Number of items produced
     */
    void addCraftable(uint8_t id, uint8_t group, TYPE_OF_ITEM_ID item, const string &name, uint16_t decisecondsToCraft,
                      uint8_t craftedStackSize);

    /**
     * @brief Add ingredient to last added craftable
     * @param item Ingredient item ID
     */
    void addCraftableIngredient(TYPE_OF_ITEM_ID item);

    /**
     * @brief Add ingredient with quantity to last added craftable
     * @param item Ingredient item ID
     * @param number Quantity required
     */
    void addCraftableIngredient(TYPE_OF_ITEM_ID item, uint8_t number);

    [[nodiscard]] auto getResult() const -> Result;
    void setResult(Result result);

    [[nodiscard]] auto getCraftableId() const -> uint8_t;
    void setCraftableId(uint8_t index);
    [[nodiscard]] auto getCraftableAmount() const -> Item::number_type;
    void setCraftableAmount(Item::number_type amount);
    [[nodiscard]] auto getIngredientIndex() const -> index_t;
    void setIngredientIndex(index_t index);
    [[nodiscard]] auto getCraftableTime() const -> uint16_t;

    /**
     * @brief Check if dialog closes when player moves
     * @return true - crafting dialog closes on movement
     */
    [[nodiscard]] auto closeOnMove() const -> bool override;

private:
    /**
     * @brief Validate group ID exists
     * @param group Group ID to check
     * @return true if group is valid
     */
    [[nodiscard]] auto canAddCraftable(uint8_t group) const -> bool;
};

#endif
