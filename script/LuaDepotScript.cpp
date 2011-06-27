//  illarionserver - server for the game Illarion
//  Copyright 2011 Illarion e.V.
//
//  This file is part of illarionserver.
//
//  illarionserver is free software: you can redistribute it and/or modify
//  it under the terms of the GNU General Public License as published by
//  the Free Software Foundation, either version 3 of the License, or
//  (at your option) any later version.
//
//  illarionserver is distributed in the hope that it will be useful,
//  but WITHOUT ANY WARRANTY; without even the implied warranty of
//  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//  GNU General Public License for more details.
//
//  You should have received a copy of the GNU General Public License
//  along with illarionserver.  If not, see <http://www.gnu.org/licenses/>.


#include "LuaDepotScript.hpp"
#include "Character.hpp"
#include "Item.hpp"
#include "World.hpp"
#include "luabind/luabind.hpp"
#include <iostream>
#include "Logger.hpp"
#include "fuse_ptr.hpp"

LuaDepotScript::LuaDepotScript(std::string filename) throw(ScriptException)
		: LuaScript(filename)
{
}

LuaDepotScript::~LuaDepotScript() throw() {}

bool LuaDepotScript::onOpenDepot(Character * cc, Item itm)
{
    try
    {
        World::get()->setCurrentScript( this );
        fuse_ptr<Character> fuse_cc(cc);
        return luabind::object_cast<bool>(call("onOpenDepot")( fuse_cc, itm ));
    }
    catch (luabind::error &e)
    {
        return true;
    }
    return true;
}
