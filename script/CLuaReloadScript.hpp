#ifndef _CLUARELOADSCRIPT_HPP
#define _CLUARELOADSCRIPT_HPP

#include "CLuaScript.hpp"

class CLuaReloadScript : public CLuaScript
{
    public:
        CLuaReloadScript( std::string filename ) throw ( ScriptException );
        virtual ~CLuaReloadScript() throw ();

        bool onReload();
        
    private:
	CLuaReloadScript(const CLuaReloadScript&);
	CLuaReloadScript& operator=(const CLuaReloadScript&);
};
#endif
