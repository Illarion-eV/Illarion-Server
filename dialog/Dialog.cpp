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

#include "dialog/Dialog.hpp"
#include <luabind/luabind.hpp>
#include "Logger.hpp"
#include <stdexcept>

Dialog::Dialog(std::string title, std::string className, luabind::object callback)
    : title(title), className(className), callback(callback) {
    if (luabind::type(callback) != LUA_TFUNCTION) {
        throw std::logic_error("Usage of invalid callback type. Dialog callbacks must be functions.");
    }
}

Dialog::Dialog(const Dialog &dialog) {
    title = dialog.title;
    className = dialog.className;
    callback = dialog.callback;
}

std::string Dialog::getClassName() const {
    return className;
}

std::string Dialog::getTitle() const {
    return title;
}

luabind::object Dialog::getCallback() const {
    return callback;
}

