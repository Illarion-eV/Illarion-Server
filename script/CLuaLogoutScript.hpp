#ifndef _CLUALOGOUTSCRIPT_HPP
#define _CLUALOGOUTSCRIPT_HPP

#include "CLuaScript.hpp"
//#include "Item.hpp"
//#include "TableStructs.hpp"

class CWorld;
class CCharacter;

class CLuaLogoutScript : public CLuaScript
{
    public:
        CLuaLogoutScript(std::string filename) throw(ScriptException);
        virtual ~CLuaLogoutScript() throw ();

        bool onLogout(CCharacter * cc);

    private:
	CLuaLogoutScript(const CLuaLogoutScript&);
	CLuaLogoutScript& operator=(const CLuaLogoutScript&);
    
          

};
#endif
