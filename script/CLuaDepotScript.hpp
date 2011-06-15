#ifndef _CLUADEPOTSCRIPT_HPP
#define _CLUADEPOTSCRIPT_HPP

#include "CLuaScript.hpp"
#include "Item.hpp"

class CWorld;
class CCharacter;

class CLuaDepotScript : public CLuaScript
{
    public:
        CLuaDepotScript(std::string filename) throw(ScriptException);
        virtual ~CLuaDepotScript() throw ();

        bool onOpenDepot(CCharacter * cc, Item itm);
        
    private:
    
	CLuaDepotScript(const CLuaDepotScript&);
	CLuaDepotScript& operator=(const CLuaDepotScript&);
    
          

};
#endif
