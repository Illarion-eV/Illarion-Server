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

#ifndef MESSAGE_DIALOG_HPP
#define MESSAGE_DIALOG_HPP

/**
 * @brief A simple message dialog that displays text to the player.
 *
 * This dialog shows informational text with an "OK" button (or equivalent).
 * Commonly used for notifications, hints, or simple confirmations.
 */
class MessageDialog : public Dialog {
private:
    string text; ///< The message text to display to the player.

public:
    /**
     * @brief Constructs a message dialog.
     * @param title The dialog window title.
     * @param text The message text to display.
     * @param callback Lua function called when player closes the dialog.
     */
    MessageDialog(const string &title, string text, const luabind::object &callback);

    /**
     * @brief Gets the message text.
     * @return The message text string.
     */
    [[nodiscard]] auto getText() const -> const string &;
};

#endif
