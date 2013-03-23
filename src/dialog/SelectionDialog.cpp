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

SelectionDialog::SelectionDialog(const string &title, const string &text, const luabind::object &callback)
    :Dialog(title, "SelectionDialog", callback), text(text) {
    success = false;
    close = false;
}

SelectionDialog::SelectionDialog(const SelectionDialog &selectionDialog) : Dialog(selectionDialog) {
    text = selectionDialog.text;

    for (const auto &option : selectionDialog.options) {
        addOption(option.getItem(), option.getName());
    }

    success = selectionDialog.success;
    selectedIndex = selectionDialog.selectedIndex;
    close = selectionDialog.close;
}

const string &SelectionDialog::getText() const {
    return text;
}

auto SelectionDialog::getOptionsSize() const -> index_type {
    return options.size();
}

auto SelectionDialog::begin() const -> iterator_type {
    return options.cbegin();
}

auto SelectionDialog::end() const -> iterator_type {
    return options.cend();
}

void SelectionDialog::addOption(TYPE_OF_ITEM_ID item, const string &name) {
    if (options.size() < MAXOPTIONS) {
        options.emplace_back(item, name);
    }
}

bool SelectionDialog::getSuccess() const {
    return success;
}

void SelectionDialog::setSuccess(bool success) {
    this->success = success;
}

auto SelectionDialog::getSelectedIndex() const -> index_type {
    return selectedIndex;
}

void SelectionDialog::setSelectedIndex(index_type index) {
    selectedIndex = index;
}

void SelectionDialog::setCloseOnMove() {
    close = true;
}

bool SelectionDialog::closeOnMove() const {
    return close;
}

