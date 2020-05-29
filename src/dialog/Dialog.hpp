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

#ifndef _DIALOG_HPP_
#define _DIALOG_HPP_

#include <string>
#include <luabind/object.hpp>

using std::string;

class Dialog {
private:
    string title;
    string className;
    luabind::object callback;

public:
    Dialog(const string &title, const string &className, const luabind::object &callback);
    Dialog(const Dialog &dialog);
    virtual ~Dialog() = default;
    [[nodiscard]] auto getTitle() const -> const string &;
    [[nodiscard]] auto getClassName() const -> const string &;
    [[nodiscard]] auto getCallback() const -> const luabind::object &;
    [[nodiscard]] virtual auto closeOnMove() const -> bool;
};

#endif

