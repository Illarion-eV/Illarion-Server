#ifndef _CLUAWEAPONSCRIPT_HPP
#define _CLUAWEAPONSCRIPT_HPP

#include "CLuaScript.hpp"
#include "Item.hpp"
#include "TableStructs.hpp"

class CWorld;
class CCharacter;

class CLuaWeaponScript : public CLuaScript
{
    public:
        CLuaWeaponScript(std::string filename) throw(ScriptException);
        virtual ~CLuaWeaponScript() throw ();

        //A character attacks another person with this weapon.
        //\param: Attacker, the person who attacks.
        //\param: AttackWeapon, An itemstruct which holds the current weapon of the person
        //\param: AttackWeaponStats, A weaponstruct which holds the current stats of the attackers weapon
        //\param: Defender, the person who is attacked.
        //\param: DefendWeapon, a Itemstruct which holds the current weopon of the defending person
        //\param: DefendWeaponSats, a weaponstruct which holds the current stats of the defenders weapon.       
        bool onAttack(CCharacter * Attacker, CCharacter * Defender, unsigned char pos);
        
        //A Character is defending himself with this weapon.
        //\param: Atacker, the person who attacks.
        //\param: Defender, the person who defends himself.
        bool onDefend(CCharacter * Attacker, CCharacter * Defender);
        
    private:
	CLuaWeaponScript(const CLuaWeaponScript&);
	CLuaWeaponScript& operator=(const CLuaWeaponScript&);
};
#endif
