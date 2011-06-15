#ifndef C_LUA_MAGIC_SCRIPT
#define C_LUA_MAGIC_SCRIPT

#include "CLuaScript.hpp"
#include "TableStructs.hpp"
#include "globals.h"

class CWorld;
class CCharacter;

class CLuaMagicScript : public CLuaScript {

	public:
		CLuaMagicScript(std::string filename, unsigned long int MagicFlag) throw(ScriptException);
		virtual ~CLuaMagicScript() throw();

		// Ein Character Zaubert einfach einen Spruch.
		// \param caster, Character welcher den Spruch Castet
		// \param counter, Eingestellter Counter Wert.
		void CastMagic( CCharacter * caster, unsigned short counter, unsigned short int param, unsigned char ltastate);

		// Ein Character Zaubert einen Spruch auf ein Item
		// \param caster, Character welcher den Spruch Castet.
		// \param TargetItem, Ziel des Spruches
		// \param counter, Eingestellter Counter wert.
		void CastMagicOnItem(CCharacter * caster, ScriptItem TargetItem, unsigned short counter, unsigned short int param, unsigned char ltastate);

		// Ein Character Zaubert einen Spruch auf einen anderen Character.
		// \param caster, Character welcher den Spruch Castet.
		// \param TargetCharacter, Ziel auf das gecastet wird
		// \param counter, Eingestellter Counter wert.
		void CastMagicOnCharacter(CCharacter * caster, CCharacter * target, unsigned short counter, unsigned short int param, unsigned char ltastate);

		// Ein Character Zaubert einen Spruch auf ein Freies Feld.
		// \param caster, Character der den Spruch Castet.
		// \param position, Position auf die gecastet wird.
		// \param counter, Eingestellter Counter Wert.
		void CastMagicOnField(CCharacter * caster, position pos, unsigned short counter, unsigned short int param, unsigned char ltastate);
        
        /**
        *a longtime action is disturbed by another person
        */
        bool actionDisturbed(CCharacter * performer, CCharacter * disturber);

	private:

		unsigned long int _MagicFlag;
		CLuaMagicScript(const CLuaMagicScript&);
		CLuaMagicScript& operator=(const CLuaMagicScript&);

		void init_functions();
};

#endif
