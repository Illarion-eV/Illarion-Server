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

#ifndef _DIALOG_HPP_
#define _DIALOG_HPP_

#include <string>
#include <luabind/object.hpp>

class Dialog {
private:
    std::string title;
    std::string className;
    luabind::object callback;

public:
    Dialog(std::string title, std::string className, luabind::object callback);
    Dialog(const Dialog &dialog);
    virtual ~Dialog() {};
    std::string getTitle() const;
    std::string getClassName() const;
    luabind::object getCallback() const;
    virtual bool closeOnMove() const;
};

#endif

