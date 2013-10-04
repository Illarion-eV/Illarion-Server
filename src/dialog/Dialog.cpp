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
 * FITNESS FOR A PARTICULAR PURPOSE. See the GNU Affero General Public License for more
 * details.
 *
 * You should have received a copy of the GNU  General Public License along with
 * Illarionserver. If not, see <http://www.gnu.org/licenses/>.
 */

#include "dialog/Dialog.hpp"
#include <luabind/luabind.hpp>
#include "Logger.hpp"
#include <stdexcept>

Dialog::Dialog(const string &title, const string &className, const luabind::object &callback)
    : title(title), className(className), callback(callback) {
    auto type = luabind::type(callback);

    if (type != LUA_TFUNCTION && type != LUA_TNIL) {
        throw std::logic_error("Usage of invalid callback type. Dialog callbacks must be functions or nil.");
    }
}

Dialog::Dialog(const Dialog &dialog) {
    title = dialog.title;
    className = dialog.className;
    callback = dialog.callback;
}

const string &Dialog::getClassName() const {
    return className;
}

const string &Dialog::getTitle() const {
    return title;
}

const luabind::object &Dialog::getCallback() const {
    return callback;
}

bool Dialog::closeOnMove() const {
    return false;
}

