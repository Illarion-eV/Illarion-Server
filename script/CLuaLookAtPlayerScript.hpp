#ifndef _CLUALOOKATPLAYERSCRIPT_HPP
#define _CLUALOOKATPLAYERSCRIPT_HPP

#include "CLuaScript.hpp"
//#include "Item.hpp"
//#include "TableStructs.hpp"

class CWorld;
class CCharacter;

class CLuaLookAtPlayerScript : public CLuaScript
{
    public:
        CLuaLookAtPlayerScript(std::string filename) throw(ScriptException);
        virtual ~CLuaLookAtPlayerScript() throw ();

        void lookAtPlayer(CCharacter * source, CCharacter * target, unsigned char mode);
        
    private:
    
	CLuaLookAtPlayerScript(const CLuaLookAtPlayerScript&);
	CLuaLookAtPlayerScript& operator=(const CLuaLookAtPlayerScript&);
};
#endif
