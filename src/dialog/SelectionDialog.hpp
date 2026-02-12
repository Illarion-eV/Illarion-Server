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

/**
 * @brief Represents a selectable option in a selection dialog.
 *
 * Each option has an associated item ID (for icon display) and a text name.
 */
class Option {
private:
    TYPE_OF_ITEM_ID item; ///< Item ID for displaying an icon next to the option.
    string name; ///< Display text for this option.

public:
    /**
     * @brief Constructs an option with item and name.
     * @param item Item ID for the option icon.
     * @param name Display text for the option.
     */
    Option(TYPE_OF_ITEM_ID item, string name) : item(item), name(std::move(name)){};

    /**
     * @brief Gets the item ID for icon display.
     * @return The item ID.
     */
    [[nodiscard]] auto getItem() const -> TYPE_OF_ITEM_ID { return item; };

    /**
     * @brief Gets the display name of this option.
     * @return The option name.
     */
    [[nodiscard]] auto getName() const -> const string & { return name; };
};

/**
 * @brief A dialog that presents a list of options for the player to choose from.
 *
 * This dialog shows descriptive text and a list of selectable options, each with
 * an optional item icon. The player selects one option and confirms or cancels.
 * Commonly used for menus, choices in quests, or selecting from a list of items.
 */
class SelectionDialog : public Dialog {
public:
    using index_type = uint8_t; ///< Type for option indices.
    using options_type = vector<Option>; ///< Container type for options.
    using iterator_type = options_type::const_iterator; ///< Iterator type for options.

private:
    string text; ///< Descriptive text explaining the selection.

    static const uint32_t MAXOPTIONS = 256; ///< Maximum number of options allowed.
    options_type options; ///< List of available options.

    bool success{false}; ///< True if player confirmed a selection, false if cancelled.

    index_type selectedIndex{0}; ///< Index of the option selected by the player.

    bool close{false}; ///< Whether this dialog should close on player movement.

public:
    /**
     * @brief Constructs a selection dialog.
     * @param title The dialog window title.
     * @param text Descriptive text explaining the selection.
     * @param callback Lua function called when player makes a selection or cancels.
     */
    SelectionDialog(const string &title, string text, const luabind::object &callback);

    /**
     * @brief Gets the descriptive text.
     * @return The description text.
     */
    [[nodiscard]] auto getText() const -> const string &;

    /**
     * @brief Gets the number of available options.
     * @return The option count.
     */
    [[nodiscard]] auto getOptionsSize() const -> index_type;

    /**
     * @brief Gets an iterator to the first option.
     * @return Begin iterator for options.
     */
    [[nodiscard]] auto begin() const -> iterator_type;

    /**
     * @brief Gets an iterator past the last option.
     * @return End iterator for options.
     */
    [[nodiscard]] auto end() const -> iterator_type;

    /**
     * @brief Adds an option to the selection list.
     * @param item Item ID for the option icon.
     * @param name Display text for the option.
     */
    void addOption(TYPE_OF_ITEM_ID item, const string &name);

    /**
     * @brief Checks if the player confirmed a selection.
     * @return true if player selected and confirmed, false if cancelled.
     */
    [[nodiscard]] auto getSuccess() const -> bool;

    /**
     * @brief Sets the success flag when player responds.
     * @param success true for confirmation, false for cancellation.
     */
    void setSuccess(bool success);

    /**
     * @brief Gets the index of the selected option.
     * @return The selected option index (0-based).
     */
    [[nodiscard]] auto getSelectedIndex() const -> index_type;

    /**
     * @brief Sets the selected option index.
     * @param index The option index selected by the player.
     */
    void setSelectedIndex(index_type index);

    /**
     * @brief Marks this dialog to close when the player moves.
     */
    void setCloseOnMove();

    /**
     * @brief Checks if this dialog closes on player movement.
     * @return true if dialog should close on movement.
     */
    [[nodiscard]] auto closeOnMove() const -> bool override;
};

#endif
