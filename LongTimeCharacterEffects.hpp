/*
 * Illarionserver - server for the game Illarion
 * Copyright 2011 Illarion e.V.
 *
 * This file is part of Illarionserver.
 *
 * Illarionserver  is  free  software:  you can redistribute it and/or modify it
 * under the terms of the  GNU  General  Public License as published by the Free
 * Software Foundation, either version 3 of the License, or (at your option) any
 * later version.
 *
 * Illarionserver is distributed in the hope that it will be useful, but WITHOUT
 * ANY WARRANTY;  without  even  the  implied  warranty  of  MERCHANTABILITY  or
 * FITNESS FOR A PARTICULAR PURPOSE. See the GNU General Public License for more
 * details.
 *
 * You should have received a copy of the GNU  General Public License along with
 * Illarionserver. If not, see <http://www.gnu.org/licenses/>.
 */

#ifndef LONGTIMECHARACTEREFFECTS_HPP_
#define LONGTIMECHARACTEREFFECTS_HPP_

#include "LongTimeEffect.hpp"
#include <string>
#include <list>

class MilTimer;


/**
* @ingroup Scriptclasses
* holds all the effects which are currently on a character
*/
class LongTimeCharacterEffects {
public:

    /**
    * constructor which creates the list of effects
    * @param owner the character on which theese effects
    */
    LongTimeCharacterEffects(Character *owner);

    /**
    * the destructor
    */
    ~LongTimeCharacterEffects();

    /**
    * @name LongTimeCharacterEffect Lua Functions:
    * the Functions which are exported to Lua
    */

    //@{
    /**
    *============================start of group lua functions================
    * @ingroup Scriptfunctions
    */

    /**
    * adds a effect to the effectlis of the owner
    * maybe starts a script entry
    * @param effect the effect which should be added
    */
    void addEffect(LongTimeEffect *effect);

    /**
    * searches a effect on the character and returns it
    * @param effectid the effect id of the effect which we try to find
    * @param effect byreferece pointer to a effect, here the effect which we found is returned
    * @return true if we found a effect with effectid on the owner otherwise false
    */
    bool find(uint16_t effectid, LongTimeEffect * &effect);

    /**
    * searches a effect on the character and returns it
    * @param effectname the name of the effect which we try to find
    * @param effect byreferece pointer to a effect, here the effect which we found is returned
    * @return true if we found a effect with name on the owner otherwise false
    */
    bool find(std::string effectname, LongTimeEffect * &effect);
    //luabind::object getEffectList();

    /**
    * removes a effect from the owner of the effectlist
    * @param effectid the id of the effecht which should be removed
    * @return true if there was a effect to remove otherwise false
    */
    bool removeEffect(uint16_t effectid);

    /**
    * removes a effect from the owner of the effectlist
    * @param name the name of the effecht which should be removed
    * @return true if there was a effect to remove otherwise false
    */
    bool removeEffect(std::string name);

    /**
    * removes a effect from the owner of the effectlist
    * @param effect a pointer to the effecht which should be removed
    * @return true if there was a effect to remove otherwise false
    */
    bool removeEffect(LongTimeEffect *effect);

    /**
    *====================== end of group lua functions =====================
    */
    //@}

    /**
    * adds a effect to the end of the effectlist
    * @param effect the effect which is added to the end of the list
    */
    void push_backEffect(LongTimeEffect *effect);

    /**
    * checks if a effect should be invoked in this cycle
    * maybe starts a scriptentrypoint
    */
    void checkEffects();

    /**
    * saves the effect list of the character ( only for players )
    */
    bool save();

    /**
    * loads the effect list of the character ( only for players )
    * starts a script entry point
    */
    bool load();
private:

    /**
    * defines the list which stores the effects which are currently on the character
    */
    typedef std::list < LongTimeEffect *> EFFECTLIST;

    /**
    * stores the effects which are currently on the character
    */
    EFFECTLIST effectList;

    /**
    * stores a pointer to the owner of the effects
    */
    Character *_owner;

};

#endif
