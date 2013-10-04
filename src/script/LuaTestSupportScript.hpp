/*
 *  illarionserver - server for the game Illarion
 *  Copyright 2011 Illarion e.V.
 *
 *  This file is part of illarionserver.
 *
 *  illarionserver is free software: you can redistribute it and/or modify
 *  it under the terms of the GNU Affero General Public License as published by
 *  the Free Software Foundation, either version 3 of the License, or
 *  (at your option) any later version.
 *
 *  illarionserver is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU Affero General Public License for more details.
 *
 *  You should have received a copy of the GNU Affero General Public License
 *  along with illarionserver.  If not, see <http://www.gnu.org/licenses/>.
 */

#ifndef _LUA_TESTSUPPORT_SCRIPT_HPP_
#define _LUA_TESTSUPPORT_SCRIPT_HPP_

#include "LuaScript.hpp"
#include "Item.hpp"
#include "globals.hpp"

class World;
class Character;

class LuaTestSupportScript : public LuaScript {
public:
    LuaTestSupportScript(const std::string &code, const std::string &scriptname);
    virtual ~LuaTestSupportScript() noexcept;

    Item item_test(const Item& in);
    ScriptItem scriptitem_test(const ScriptItem& in);
    int int_test(int arg);
    position position_test(const position& pos);

private:

    LuaTestSupportScript(const LuaTestSupportScript &) = delete;
    LuaTestSupportScript &operator=(const LuaTestSupportScript &) = delete;
};

#endif

