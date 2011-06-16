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
