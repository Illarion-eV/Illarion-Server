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

#include "dialog/CraftingDialog.hpp"

CraftingDialog::CraftingDialog(const string &title, uint16_t sfx, uint16_t sfxDuration, const luabind::object &callback)
        : Dialog(title, "CraftingDialog", callback) {
    this->sfx = sfx;
    this->sfxDuration = sfxDuration;
    result = playerAborts;
    craftableId = 0;
    craftableAmount = 0;
    ingredientIndex = 0;
    lastAddedCraftableId = 0;
}

auto CraftingDialog::getSfx() const -> uint16_t { return sfx; }

auto CraftingDialog::getSfxDuration() const -> uint16_t { return sfxDuration; }

void CraftingDialog::clearGroupsAndProducts() {
    groups.clear();
    craftables.clear();
}

auto CraftingDialog::getGroupsSize() const -> index_t { return groups.size(); }

auto CraftingDialog::getGroupsBegin() const -> group_iterator { return groups.cbegin(); }

auto CraftingDialog::getGroupsEnd() const -> group_iterator { return groups.cend(); }

void CraftingDialog::addGroup(const string &name) {
    if (groups.size() < maximumGroups) {
        groups.push_back(name);
    }
}

auto CraftingDialog::getCraftablesSize() const -> index_t { return craftables.size(); }

auto CraftingDialog::getCraftablesBegin() const -> craftable_iterator { return craftables.cbegin(); }

auto CraftingDialog::getCraftablesEnd() const -> craftable_iterator { return craftables.cend(); }

void CraftingDialog::addCraftable(uint8_t id, uint8_t group, TYPE_OF_ITEM_ID item, const string &name,
                                  uint16_t decisecondsToCraft) {
    if (canAddCraftable(group)) {
        craftables.insert(std::make_pair(id, Craftable(group, item, name, decisecondsToCraft)));
        lastAddedCraftableId = id;
    }
}

void CraftingDialog::addCraftable(uint8_t id, uint8_t group, TYPE_OF_ITEM_ID item, const string &name,
                                  uint16_t decisecondsToCraft, uint8_t craftedStackSize) {
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

auto CraftingDialog::getResult() const -> CraftingDialog::Result { return result; }

void CraftingDialog::setResult(Result result) { this->result = result; }

auto CraftingDialog::getCraftableId() const -> uint8_t { return craftableId; }

void CraftingDialog::setCraftableId(uint8_t index) {
    if (craftables.find(index) != craftables.end()) {
        craftableId = index;
    } else {
        craftableId = 0;
    }
}

auto CraftingDialog::getCraftableAmount() const -> Item::number_type { return craftableAmount; }

void CraftingDialog::setCraftableAmount(Item::number_type amount) { craftableAmount = amount; }

auto CraftingDialog::getIngredientIndex() const -> index_t { return ingredientIndex; }

void CraftingDialog::setIngredientIndex(index_t index) { ingredientIndex = index; }

auto CraftingDialog::getCraftableTime() const -> uint16_t {
    try {
        return craftables.at(craftableId).getDecisecondsToCraft();
    } catch (std::out_of_range &) {
        return 0;
    }
}

auto CraftingDialog::closeOnMove() const -> bool { return true; }

auto CraftingDialog::canAddCraftable(uint8_t group) -> bool {
    return (groups.size() - 1 >= group) && (craftables.size() < maximumCraftables);
}
