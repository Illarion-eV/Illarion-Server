#include "CLuaDepotScript.hpp"
#include "CCharacter.hpp"
#include "Item.hpp"
#include "CWorld.hpp"
#include "luabind/luabind.hpp"
#include <iostream>
#include "CLogger.hpp"
#include "fuse_ptr.hpp"

CLuaDepotScript::CLuaDepotScript(std::string filename) throw(ScriptException)
		: CLuaScript(filename)
{
}

CLuaDepotScript::~CLuaDepotScript() throw() {}

bool CLuaDepotScript::onOpenDepot(CCharacter * cc, Item itm)
{
    try
    {
        CWorld::get()->setCurrentScript( this );
        fuse_ptr<CCharacter> fuse_cc(cc);
        return luabind::object_cast<bool>(call("onOpenDepot")( fuse_cc, itm ));
    }
    catch (luabind::error &e)
    {
        return true;
    }
    return true;
}
