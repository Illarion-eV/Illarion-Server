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

#ifndef INPUT_DIALOG_HPP
#define INPUT_DIALOG_HPP

#include "dialog/Dialog.hpp"

/**
 * @brief A dialog that prompts the player to enter text input.
 *
 * This dialog collects text input from the player, supporting both single-line
 * and multi-line text entry with configurable character limits. The success flag
 * indicates whether the player confirmed or cancelled the input.
 */
class InputDialog : public Dialog {
private:
    string description;  ///< Descriptive text prompting the player for input.
    bool multiline{};    ///< Whether to allow multi-line text entry.
    short maxChars{};    ///< Maximum number of characters allowed (0 = unlimited).
    bool success{false}; ///< True if player confirmed input, false if cancelled.
    string input;        ///< The text entered by the player.

public:
    /**
     * @brief Constructs an input dialog.
     * @param title The dialog window title.
     * @param description Prompt text explaining what input is needed.
     * @param multiline Whether to show a multi-line text area.
     * @param maxChars Maximum character limit (0 for no limit).
     * @param callback Lua function called when player submits or cancels.
     */
    InputDialog(const string &title, string description, bool multiline, short maxChars,
                const luabind::object &callback);

    /**
     * @brief Gets the input prompt description.
     * @return The description text.
     */
    [[nodiscard]] auto getDescription() const -> const string &;

    /**
     * @brief Checks if multi-line input is enabled.
     * @return true for multi-line text area, false for single-line input.
     */
    [[nodiscard]] auto isMultiline() const -> bool;

    /**
     * @brief Gets the maximum character limit.
     * @return The character limit, or 0 for unlimited.
     */
    [[nodiscard]] auto getMaxChars() const -> short;

    /**
     * @brief Checks if the player confirmed the input.
     * @return true if player clicked OK/Submit, false if cancelled.
     */
    [[nodiscard]] auto getSuccess() const -> bool;

    /**
     * @brief Sets the success flag when player responds.
     * @param success true for confirmation, false for cancellation.
     */
    void setSuccess(bool success);

    /**
     * @brief Gets the text entered by the player.
     * @return The input string (empty if cancelled).
     */
    [[nodiscard]] auto getInput() const -> const string &;

    /**
     * @brief Sets the player's input text.
     * @param input The text entered by the player.
     */
    void setInput(string input);
};

#endif
