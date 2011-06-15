#ifndef _CLUALOGINSCRIPT_HPP
#define _CLUALOGINSCRIPT_HPP

#include "CLuaScript.hpp"
//#include "Item.hpp"
//#include "TableStructs.hpp"

class CWorld;
class CCharacter;

class CLuaLoginScript : public CLuaScript
{
    public:
        CLuaLoginScript(std::string filename) throw(ScriptException);
        virtual ~CLuaLoginScript() throw ();

        bool onLogin(CCharacter * cc);
        
    private:
    
	CLuaLoginScript(const CLuaLoginScript&);
	CLuaLoginScript& operator=(const CLuaLoginScript&);
};
#endif
