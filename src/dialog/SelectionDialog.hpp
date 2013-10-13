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

#ifndef _SELECTION_DIALOG_HPP_
#define _SELECTION_DIALOG_HPP_

#include "dialog/Dialog.hpp"
#include "Item.hpp"
#include <vector>

using std::vector;

class Option {
private:
    TYPE_OF_ITEM_ID item;
    string name;

public:
    Option(TYPE_OF_ITEM_ID item, string name): item(item), name(name) {};
    TYPE_OF_ITEM_ID getItem() const {
        return item;
    };
    const string &getName() const {
        return name;
    };
};

class SelectionDialog: public Dialog {
public:
    typedef uint8_t index_type;
    typedef vector<Option> options_type;
    typedef options_type::const_iterator iterator_type;

private:
    string text;

    static const uint32_t MAXOPTIONS = 256;
    options_type options;

    bool success;

    index_type selectedIndex;

    bool close;

public:
    SelectionDialog(const string &title, const string &text, const luabind::object &callback);
    SelectionDialog(const SelectionDialog &selectionDialog);

    const string &getText() const;

    index_type getOptionsSize() const;
    iterator_type begin() const;
    iterator_type end() const;
    void addOption(TYPE_OF_ITEM_ID item, const string &name);

    bool getSuccess() const;
    void setSuccess(bool success);

    index_type getSelectedIndex() const;
    void setSelectedIndex(index_type index);

    void setCloseOnMove();
    virtual bool closeOnMove() const override;
};

#endif

