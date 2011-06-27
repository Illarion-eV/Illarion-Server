//  illarionserver - server for the game Illarion
//  Copyright 2011 Illarion e.V.
//
//  This file is part of illarionserver.
//
//  illarionserver is free software: you can redistribute it and/or modify
//  it under the terms of the GNU General Public License as published by
//  the Free Software Foundation, either version 3 of the License, or
//  (at your option) any later version.
//
//  illarionserver is distributed in the hope that it will be useful,
//  but WITHOUT ANY WARRANTY; without even the implied warranty of
//  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//  GNU General Public License for more details.
//
//  You should have received a copy of the GNU General Public License
//  along with illarionserver.  If not, see <http://www.gnu.org/licenses/>.


#ifndef CLONGTIMEEFFECT_HPP_ 
#define CLONGTIMEEFFECT_HPP_


#include <string>
#include <cstring>
#include "types.hpp"
#include <map>

class CCharacter;
class CPlayer;

class CLongTimeEffect
{
    public:
    
        /**
        * destructor for the effect
        */
        ~CLongTimeEffect();

        /**
        * @name CLongTimeEffect Lua Functions:
        * Functions which are exported to lua
        */
        
        //@{
        /**
        *======================start grouping Lua functions===================
        *@ingroup Scriptfunctions
        */    
        
        /**
        * constructor for a long time effect adds other informations like the name from the db
        * <b>Lua: CLongTimeEffect( effectId, nextCalled )</b>
        * @param effectId id of the effect
        * @param nextCalled how much 1/10 s it needs to recall the effect
        */
        CLongTimeEffect(uint16_t effectId, uint32_t nextCalled);
        
        /**
        * constructor for a long time effect adds other informations like id from the db
        * <b>Lua: CLongTimeEffect( name, nextCalled)</b>
        * @param name the name of the effect
        * @param nextCalled how much 1/10s have to be passed before the effect is recalled
        */
        CLongTimeEffect(std::string name, uint32_t nextCalled);
        
        /**
        *adds a specific value to this effect, if this value currently exists 
        *the value is changed to value
        *<b>Lua: [:addValue]</b>
        *@param name of the value which should be added
        *@param value the value which should be added
        */
        void addValue(std::string name, uint32_t value);
        
        /**
        *removes a specific value from this effect
        *<b>Lua: [:addValue]</b>
        *@param name of the value which should be removed
        */
        void removeValue(std::string name);
        
        /**
        * tries to find a value for this effect
        * <b>Lua: [:findValue]</b>
        * @param name of the value which we want to get
        * @param ret by reference returns the value
        * @return true if there was a value with the given name otherwise false
        */
        bool findValue(std::string name, uint32_t &ret);
        
        /**
        *==========================end grouping lua functions=======================
        */
        //@}
        
        /**
        *effect is invoked and effects the character
        *@param target, the character which is victim of the effect
        *@return if true the effect has to be included again in the effect list
        */
        bool callEffect(CCharacter * target);        
        
        /**
        *saves this effect of a player to the db
        *@param playerid the player for whom the values are saved
        *@return true if saving was successfull
        */
        bool save( uint32_t playerid );
        
        /**
        *changes the state of first add if it was true to false
        */
        void firstAdd(){ if ( _firstadd ) _firstadd = false; }

        /**
        * @name CLongTimeEffect Lua Variables:
        * Variables which are exported to lua
        */
        
        //@{
        /**
        *======================start grouping Lua Variables===================
        *@ingroup Scriptvariables
        */    
        
        /**
        * id of this effect
        * <b>Lua: (ro) [effectId]</b>
        */
        uint16_t _effectId;
        
        /**
        * name of this effect
        * <b>Lua: (ro) [effectName]</b>
        */        
        std::string _effectName;
        
        /**
        * how much 1/10 s until the effect is recalled
        * <b>Lua: (r/w) [nextCalled]</b>
        */        
        uint32_t _nextCalled;
        
        /**
        * how often this effect was called before
        * <b>Lua: (ro) [numberCalled]</b>
        */
        uint32_t _numberCalled; 
        
        /**
        * how much time has passed since the last call
        * <b>Lua: (ro) [lastCalled]</b>
        */
        uint32_t _lastCalled; 
        
        /**
        *===========================end of grouping scriptvariables==================
        */
        //@}
        
        /**
        * if it is the first time the effect is added or not
        */
        bool _firstadd;
        
    private:
    
        /**
        * compare function for the valuetable
        */
        struct ltstr
        {
            bool operator()( const char* s1, const char * s2) const 
            {
                return strcmp( s1, s2) < 0;
            }
        };
        

        /**
        * defines the map for the values
        */
        typedef std::map < const char*, uint32_t, ltstr > VALUETABLE;
        
        /**
        * stores the values for this effect in the map
        */
        VALUETABLE _values;
        
};

#endif
