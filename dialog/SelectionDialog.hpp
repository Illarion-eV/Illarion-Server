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

#ifndef _SELECTION_DIALOG_HPP_
#define _SELECTION_DIALOG_HPP_

#include "dialog/Dialog.hpp"
#include "Item.hpp"
#include <string>
#include <vector>

using std::string;
using std::vector;

class Option {
private:
    TYPE_OF_ITEM_ID item;
    string name;

public:
    Option(TYPE_OF_ITEM_ID item, string name): item(item), name(name) {};
    TYPE_OF_ITEM_ID getItem() {
        return item;
    };
    string &getName() {
        return name;
    };
};

class SelectionDialog: public Dialog {
public:
    typedef uint8_t index_type;

private:
    string text;

    static const uint32_t MAXOPTIONS = 256;
    vector<Option *> options;

    bool success;

    index_type selectedIndex;

public:
    SelectionDialog(string title, string text, luabind::object callback);
    SelectionDialog(const SelectionDialog &selectionDialog);
    ~SelectionDialog();

    string getText() const;

    index_type getOptionsSize() const;
    vector<Option *>::const_iterator getOptionsBegin() const;
    vector<Option *>::const_iterator getOptionsEnd() const;
    void addOption(TYPE_OF_ITEM_ID item, string name);

    bool getSuccess() const;
    void setSuccess(bool success);

    index_type getSelectedIndex() const;
    void setSelectedIndex(index_type index);
};

#endif

