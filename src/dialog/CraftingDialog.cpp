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
 * FITNESS FOR A PARTICULAR PURPOSE. See the GNU Affero General Public License for more
 * details.
 *
 * You should have received a copy of the GNU  General Public License along with
 * Illarionserver. If not, see <http://www.gnu.org/licenses/>.
 */

#include "dialog/CraftingDialog.hpp"

CraftingDialog::CraftingDialog(const string &title, uint16_t sfx, uint16_t sfxDuration, const luabind::object &callback): Dialog(title, "CraftingDialog", callback) {
    this->sfx = sfx;
    this->sfxDuration = sfxDuration;
    result = playerAborts;
    craftableId = 0;
    craftableAmount = 0;
    ingredientIndex = 0;
    lastAddedCraftableId = 0;
}

CraftingDialog::CraftingDialog(const CraftingDialog &craftingDialog): Dialog(craftingDialog) {
    sfx = craftingDialog.sfx;
    sfxDuration = craftingDialog.sfxDuration;
    result = craftingDialog.result;
    craftableId = craftingDialog.craftableId;
    craftableAmount = craftingDialog.craftableAmount;
    ingredientIndex = craftingDialog.ingredientIndex;
    groups = craftingDialog.groups;
    lastAddedCraftableId = craftingDialog.lastAddedCraftableId;
    craftables = craftingDialog.craftables;
}

uint16_t CraftingDialog::getSfx() const {
    return sfx;
}

uint16_t CraftingDialog::getSfxDuration() const {
    return sfxDuration;
}

void CraftingDialog::clearGroupsAndProducts() {
    groups.clear();
    craftables.clear();
}

auto CraftingDialog::getGroupsSize() const -> index_t {
    return groups.size();
}

auto CraftingDialog::getGroupsBegin() const -> group_iterator {
    return groups.cbegin();
}

auto CraftingDialog::getGroupsEnd() const -> group_iterator {
    return groups.cend();
}

void CraftingDialog::addGroup(const string &name) {
    if (groups.size() < 256) {
        groups.push_back(name);
    }
}

auto CraftingDialog::getCraftablesSize() const -> index_t {
    return craftables.size();
}

auto CraftingDialog::getCraftablesBegin() const -> craftable_iterator {
    return craftables.cbegin();
}

auto CraftingDialog::getCraftablesEnd() const -> craftable_iterator {
    return craftables.cend();
}

void CraftingDialog::addCraftable(uint8_t id, uint8_t group, TYPE_OF_ITEM_ID item, const string &name, uint16_t decisecondsToCraft) {
    if (canAddCraftable(group)) {
        craftables.insert(std::make_pair(id, Craftable(group, item, name, decisecondsToCraft)));
        lastAddedCraftableId = id;
    }
}

void CraftingDialog::addCraftable(uint8_t id, uint8_t group, TYPE_OF_ITEM_ID item, const string &name, uint16_t decisecondsToCraft, uint8_t craftedStackSize) {
    if (canAddCraftable(group)) {
        craftables.insert(std::make_pair(id, Craftable(group, item, name, decisecondsToCraft, craftedStackSize)));
        lastAddedCraftableId = id;
    }
}

void CraftingDialog::addCraftableIngredient(TYPE_OF_ITEM_ID item) {
    if (!craftables.empty()) {
        try {
            craftables.at(lastAddedCraftableId).addIngredient(item);
        } catch (std::out_of_range &) {
        }
    }
}

void CraftingDialog::addCraftableIngredient(TYPE_OF_ITEM_ID item, uint8_t number) {
    if (!craftables.empty()) {
        try {
            craftables.at(lastAddedCraftableId).addIngredient(item, number);
        } catch (std::out_of_range &) {
        }
    }
}

CraftingDialog::Result CraftingDialog::getResult() const {
    return result;
}

void CraftingDialog::setResult(Result result) {
    this->result = result;
}

uint8_t CraftingDialog::getCraftableId() const {
    return craftableId;
}

void CraftingDialog::setCraftableId(uint8_t id) {
    if (craftables.find(id) != craftables.end()) {
        craftableId = id;
    } else {
        craftableId = 0;
    }
}

Item::number_type CraftingDialog::getCraftableAmount() const {
    return craftableAmount;
}

void CraftingDialog::setCraftableAmount(Item::number_type amount) {
    craftableAmount = amount;
}

auto CraftingDialog::getIngredientIndex() const -> index_t {
    return ingredientIndex;
}

void CraftingDialog::setIngredientIndex(index_t index) {
    ingredientIndex = index;
}

uint16_t CraftingDialog::getCraftableTime() const {
    try {
        return craftables.at(craftableId).getDecisecondsToCraft();
    } catch (std::out_of_range &) {
        return 0;
    }
}

bool CraftingDialog::closeOnMove() const {
    return true;
}

bool CraftingDialog::canAddCraftable(uint8_t group) {
    return (groups.size()-1 >= group) && (craftables.size() < MAXCRAFTABLES);
}

