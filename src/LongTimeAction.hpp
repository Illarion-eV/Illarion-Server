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


#ifndef _CLONGTIMEACTION_HPP_
#define _CLONGTIMEACTION_HPP_

#include <memory>
#include "MilTimer.hpp"
#include "Item.hpp"
#include "script/LuaScript.hpp"

class Player;
class World;
class Character;

/**
* @ingroup Scriptclasses
* class which holds the last action of the player for recalling the script on success or aborting the script
* exportet to lua as <b>action</b>
*/
class LongTimeAction {
public:

    /**
    * what type of action was invoked
    */
    enum ActionType {
        ACTION_USE = 0,
        ACTION_MAGIC = 1,
        ACTION_CRAFT = 2
    };

    /**
    *@name Lua Definitions:
    *to lua exportet definitions like enums:
    */

    //@{
    /**
    *======================start grouping Lua Definitions===================
    *@ingroup Scriptenums
    */

    /**
    *holds the state of a long time action
    */
    enum LtaState {
        ST_NONE = 0, /**< no action invoked or started*/
        ST_ABORT = 1, /**< action was aborted*/
        ST_SUCCESS = 2 /**< action was sucessfully*/
    };


    /**
    *======================end grouping Lua Definitions===================
    */
    //@}

    /**
    *standard constructor which initializes the basic values
    *@param player the player which is the owner of the action
    *@param world the gameworld
    */
    LongTimeAction(Player *player, World *world);

    /**
    *sets the last action to the new values so the script can called correctly
    *@param script the script for the last action
    *@param srce the source object for the last action so we can determine if it was an item at which pos etc
    *@param trgt the target object for the last action so we can determine if it was an item at which pos etc
    *@param at the type of action lastly performed (cast or use)
    */
    void setLastAction(std::shared_ptr<LuaScript> script, SouTar srce, SouTar trgt, ActionType at);

    /**
    *starts an long time action which is aborted if the player talks, is attacked ....
    *@param timetowait time i 1/10 sec which the char has to wait before the Action is Sucessful
    *@param ani the animation which should be shown, 0 for no animation should be shown
    *@param redoani after which time 1/10 s the animation should be shown again
    *@param sound the sound which should be played
    *@param redosound which time 1/10 s the sound should be played again
    */
    void startLongTimeAction(unsigned short int timetowait, unsigned short int ani=0, unsigned short int redoani=0, unsigned short int sound=0, unsigned short int redosound=0);

    /**
    *changes the source of the last action.
    *@param cc source is a character the pointer to this character
    */
    void changeSource(Character *cc);

    /**
    *changes the source of the last action.
    *@param sI source is a item the new item
    */
    void changeSource(ScriptItem sI);

    /**
    *changes the Source of the last action.
    *@param pos source is a position the new position
    */
    void changeSource(position pos);

    /**
    *changes the Source of the last action to nothing
    */
    void changeSource();

    /**
    *changes the Target of the last action.
    *@param cc target is a character the pointer to this character
    */
    void changeTarget(Character *cc);

    /**
    *changes the Target of the last action.
    *@param sI target is a item the new item
    */
    void changeTarget(ScriptItem sI);

    /**
    *changes the target of the last action.
    *@param pos Target is a position the new position
    */
    void changeTarget(position pos);

    /**
    *changes the Target of the last action to nothing
    */
    void changeTarget();

    /**
    *checks if the last action is abborted or  (Action is disturbed by another player or event)
    *@return true, if the action should be aborted
    */
    bool actionDisturbed(Character *disturber);

    /**
    *aborts the last action without checking
    */
    void abortAction();

    /**
    *sucesses the last action
    */
    void successAction();

    /**
    *checks the last Action of the player if its maybe succseful
    *@return true if the action was successfully performed
    */
    bool checkAction();

    /**
    *checks if currently an action is running or not
    * @return true if there is a action running
    */
    inline bool actionRunning() {
        return _actionrunning;
    }



private:
    std::shared_ptr<LuaScript> _script = nullptr; /**< pointer to the last script*/
    SouTar _source; /**< source of the last script*/
    SouTar _target; /**< target of the last script*/

    TYPE_OF_CHARACTER_ID _targetId; /**< id of the target if its an character*/
    TYPE_OF_CHARACTER_ID _sourceId; /**< id of the source if its an character*/
    uint8_t _sourceCharType; /**< type of the source*/

    Player *_owner;  /**< the owner of the LongTimeAction Objectt*/
    World *_world;  /**< pointer to the gameworld*/

    bool _actionrunning = false; /**< boolean value, if true there is currently a action running*/

    std::unique_ptr<MilTimer> _timetowaitTimer = nullptr;  /**< timer which determines how many ms the action has to wait until it is sucessfull*/
    std::unique_ptr<MilTimer> _redoaniTimer = nullptr;  /**< timer which determines after how many ms the animation is shown again*/
    std::unique_ptr<MilTimer> _redosoundTimer = nullptr;  /**< timer which determines how many ms the sound is played again.*/

    ActionType _at = ACTION_USE; /**< type of the action @see ActionType*/

    unsigned short int _sound = 0; /**< id of the sound which is played to the action*/
    unsigned short int _ani = 0; /**< id of the animation which is shown to the action*/

    void checkTarget();
    void checkSource();

};

#endif
