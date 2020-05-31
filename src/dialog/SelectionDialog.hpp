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

#ifndef SELECTION_DIALOG_HPP
#define SELECTION_DIALOG_HPP

#include "Item.hpp"
#include "dialog/Dialog.hpp"
#include <utility>

#include <vector>

using std::vector;

class Option {
private:
    TYPE_OF_ITEM_ID item;
    string name;

public:
    Option(TYPE_OF_ITEM_ID item, string name): item(item), name(std::move(name)) {};
    [[nodiscard]] auto getItem() const -> TYPE_OF_ITEM_ID {
        return item;
    };
    [[nodiscard]] auto getName() const -> const string & {
        return name;
    };
};

class SelectionDialog: public Dialog {
public:
    using index_type = uint8_t;
    using options_type = vector<Option>;
    using iterator_type = options_type::const_iterator;

private:
    string text;

    static const uint32_t MAXOPTIONS = 256;
    options_type options;

    bool success;

    index_type selectedIndex;

    bool close;

public:
    SelectionDialog(const string &title, string text, const luabind::object &callback);
    SelectionDialog(const SelectionDialog &selectionDialog);

    [[nodiscard]] auto getText() const -> const string &;

    [[nodiscard]] auto getOptionsSize() const -> index_type;
    [[nodiscard]] auto begin() const -> iterator_type;
    [[nodiscard]] auto end() const -> iterator_type;
    void addOption(TYPE_OF_ITEM_ID item, const string &name);

    [[nodiscard]] auto getSuccess() const -> bool;
    void setSuccess(bool success);

    [[nodiscard]] auto getSelectedIndex() const -> index_type;
    void setSelectedIndex(index_type index);

    void setCloseOnMove();
    [[nodiscard]] auto closeOnMove() const -> bool override;
};

#endif

