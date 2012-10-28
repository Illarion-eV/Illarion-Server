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

#include "dialog/SelectionDialog.hpp"

SelectionDialog::SelectionDialog(std::string title, std::string text, luabind::object callback)
    :Dialog(title, "SelectionDialog", callback), text(text) {
    success = false;
}

SelectionDialog::SelectionDialog(const SelectionDialog &selectionDialog) : Dialog(selectionDialog) {
    text = selectionDialog.text;

    for (auto it = selectionDialog.options.begin(); it != selectionDialog.options.end(); ++it) {
        Option &option = **it;
        addOption(option.getItem(), option.getName());
    }

    success = selectionDialog.success;
    selectedIndex = selectionDialog.selectedIndex;
}

SelectionDialog::~SelectionDialog() {
    for (auto it = options.begin(); it != options.end(); ++it) {
        delete *it;
    }
}

std::string SelectionDialog::getText() const {
    return text;
}

SelectionDialog::index_type SelectionDialog::getOptionsSize() const {
    return options.size();
}

vector<Option *>::const_iterator SelectionDialog::getOptionsBegin() const {
    return options.cbegin();
}

vector<Option *>::const_iterator SelectionDialog::getOptionsEnd() const {
    return options.cend();
}

void SelectionDialog::addOption(TYPE_OF_ITEM_ID item, string name) {
    if (options.size() < MAXOPTIONS) {
        Option *option = new Option(item, name);
        options.push_back(option);
    }
}

bool SelectionDialog::getSuccess() const {
    return success;
}

void SelectionDialog::setSuccess(bool success) {
    this->success = success;
}

SelectionDialog::index_type SelectionDialog::getSelectedIndex() const {
    return selectedIndex;
}

void SelectionDialog::setSelectedIndex(index_type index) {
    selectedIndex = index;
}

