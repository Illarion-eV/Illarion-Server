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

#ifndef DIALOG_HPP
#define DIALOG_HPP

#include <luabind/object.hpp>
#include <string>

using std::string;

/**
 * @brief Base class for all player-facing dialog windows.
 *
 * This abstract class provides the foundation for interactive dialogs shown to players,
 * including a title, class identifier for client rendering, and a Lua callback function
 * that handles player responses.
 */
class Dialog {
private:
    string title;             ///< The dialog window title shown to the player.
    string className;         ///< CSS-like class name for client-side styling.
    luabind::object callback; ///< Lua function called when player responds to the dialog.

public:
    /**
     * @brief Constructs a dialog with title, class, and callback.
     * @param title The dialog window title.
     * @param className The CSS-like class for client rendering.
     * @param callback Lua function to invoke when player responds.
     */
    Dialog(string title, string className, const luabind::object &callback);

    Dialog(const Dialog &dialog) = default;
    auto operator=(const Dialog &) -> Dialog & = default;
    Dialog(Dialog &&) = default;
    auto operator=(Dialog &&) -> Dialog & = default;
    virtual ~Dialog() = default;

    /**
     * @brief Gets the dialog window title.
     * @return The title string.
     */
    [[nodiscard]] auto getTitle() const -> const string &;

    /**
     * @brief Gets the CSS-like class name for client rendering.
     * @return The class name string.
     */
    [[nodiscard]] auto getClassName() const -> const string &;

    /**
     * @brief Gets the Lua callback function.
     * @return The callback object.
     */
    [[nodiscard]] auto getCallback() const -> const luabind::object &;

    /**
     * @brief Determines if this dialog should close when the player moves.
     * @return true if dialog auto-closes on movement, false to keep it open.
     * @note Default implementation returns true; override for persistent dialogs.
     */
    [[nodiscard]] virtual auto closeOnMove() const -> bool;
};

#endif
