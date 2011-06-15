#ifndef CLUALONGTIMEEFFECTSCRIPT_HPP_
#define CLUALONGTIMEEFFECTSCRIPT_HPP_

#include "CLuaScript.hpp"
#include "TableStructs.hpp"

class CWorld;
class CCharacter;
class CLongTimeEffect;
class CPlayer;

class CLuaLongTimeEffectScript : public CLuaScript 
{
	public:
		CLuaLongTimeEffectScript(std::string filename, LongTimeEffectStruct effectStruct) throw(ScriptException);
		virtual ~CLuaLongTimeEffectScript() throw();
		
		/**
        *calling the long time effect in a script
        *@param effect the current effect 
        *@param taget the character which is hidden by the effect
        *@return true if the effect should be added to the effectlist again
        */
        bool callEffect(CLongTimeEffect * effect, CCharacter * target);

        /**
        *invoked if a effect which exists on a player should be added again
        *@param effect the effect which is currently active
        *@param target the character which should get the effect again
        */
        void doubleEffect(CLongTimeEffect * effect, CCharacter * target);
        
        /**
        *invoked if a effect is loaded to a player
        */
        void loadEffect(CLongTimeEffect * effect, CPlayer * target);
        
        void addEffect(CLongTimeEffect * effect, CCharacter * target);
        void removeEffect(CLongTimeEffect * effect, CCharacter * target);
        
	private:
		LongTimeEffectStruct _effectStruct;
		CLuaLongTimeEffectScript(const CLuaItemScript&);
		CLuaLongTimeEffectScript& operator=(const CLuaLongTimeEffectScript&);
		void init_functions();
};
#endif
