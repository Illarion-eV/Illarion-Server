/*
 *  illarionserver - server for the game Illarion
 *  Copyright 2011 Illarion e.V.
 *
 *  This file is part of illarionserver.
 *
 *  illarionserver is free software: you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation, either version 3 of the License, or
 *  (at your option) any later version.
 *
 *  illarionserver is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with illarionserver.  If not, see <http://www.gnu.org/licenses/>.
 */

#include "LuaTestSupportScript.hpp"
#include "Item.hpp"

LuaTestSupportScript::LuaTestSupportScript(const std::string &code, const std::string &scriptname)
    : LuaScript(code, scriptname) {
}

LuaTestSupportScript::~LuaTestSupportScript() noexcept {}

Item LuaTestSupportScript::item_test(const Item& in) {
	return callEntrypoint<Item, Item>("item_test", in);
}

ScriptItem LuaTestSupportScript::scriptitem_test(const ScriptItem& in) {
	return callEntrypoint<ScriptItem, ScriptItem>("scriptitem_test", in);
}

int LuaTestSupportScript::int_test(int arg) {
	return callEntrypoint<int, int>("int_test", arg);
}

position LuaTestSupportScript::position_test(const position& pos) {
	return callEntrypoint<position, position>("position_test", pos);
}
