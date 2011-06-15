#ifndef _CLUALEARNSCRIPT_HPP
#define _CLUALEARNSCRIPT_HPP

#include "CLuaScript.hpp"

class CWorld;
class CCharacter;

class CLuaLearnScript : public CLuaScript
{
    public:
        CLuaLearnScript(std::string filename) throw(ScriptException);
        virtual ~CLuaLearnScript() throw ();

        void learn( CCharacter * cc, std::string skill, uint8_t skillGroup, uint32_t actionPoints, uint8_t opponent, uint8_t leadAttrib );
        void reduceMC( CCharacter * cc );
        
    private:
    
	CLuaLearnScript(const CLuaLearnScript&);
	CLuaLearnScript& operator=(const CLuaLearnScript&);
};
#endif
