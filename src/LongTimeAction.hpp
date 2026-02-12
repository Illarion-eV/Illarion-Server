//  illarionserver - server for the game Illarion
//  Copyright 2011 Illarion e.V.
//
//  This file is part of illarionserver.
//
//  illarionserver is free software: you can redistribute it and/or modify
//  it under the terms of the GNU Affero General Public License as published by
//  the Free Software Foundation, either version 3 of the License, or
//  (at your option) any later version.
//
//  illarionserver is distributed in the hope that it will be useful,
//  but WITHOUT ANY WARRANTY; without even the implied warranty of
//  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//  GNU Affero General Public License for more details.
//
//  You should have received a copy of the GNU Affero General Public License
//  along with illarionserver.  If not, see <http://www.gnu.org/licenses/>.

#ifndef LONG_TIME_ACTION_HPP
#define LONG_TIME_ACTION_HPP

#include "Item.hpp"
#include "Timer.hpp"
#include "Character.hpp"

#include <memory>

class Player;
class World;
class Character;
class LuaScript;

enum ActionParameterTypes { LUA_NONE = 0, LUA_FIELD = 1, LUA_ITEM = 2, LUA_CHARACTER = 3, LUA_DIALOG = 4, LUA_TALK };

enum LtaState : unsigned char {
    ST_NONE = 0,   /**< no action invoked or started*/
    ST_ABORT = 1,  /**< action was aborted*/
    ST_SUCCESS = 2 /**< action was sucessfully*/
};

/**
 * @brief Parameters passed to Lua scripts for various action callbacks.
 * 
 * Contains context information about the source of an action (character, item,
 * field position, dialog, or text) to provide scripts with necessary data.
 */
struct ActionParameters {
    Character *character = nullptr;
    ScriptItem item;
    ActionParameterTypes type = LUA_NONE;
    position pos{};
    unsigned int dialog = 0;
    std::string text;
    int talkType;
};

/**
 * @brief Manages long-duration player actions that can be interrupted.
 * 
 * Tracks actions like crafting, using items, or casting spells that take time to complete.
 * Actions can be aborted by combat, movement, or other disruptions, and trigger Lua
 * script callbacks on success or failure.
 * 
 * @ingroup Scriptclasses
 */
class LongTimeAction {
public:
    /**
     * what type of action was invoked
     */
    enum class ActionType { USE, MAGIC, CRAFT, TALK };

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
     *@param at the type of action lastly performed (cast or use)
     */
    void setLastAction(std::shared_ptr<LuaScript> script, const ActionParameters &parameters, ActionType at);

    /**
     *starts an long time action which is aborted if the player talks, is attacked ....
     *@param timetowait time i 1/10 sec which the char has to wait before the Action is Sucessful
     *@param ani the animation which should be shown, 0 for no animation should be shown
     *@param redoani after which time 1/10 s the animation should be shown again
     *@param sound the sound which should be played
     *@param redosound which time 1/10 s the sound should be played again
     */
    void startLongTimeAction(unsigned short int timetowait, unsigned short int ani = 0, unsigned short int redoani = 0,
                             unsigned short int sound = 0, unsigned short int redosound = 0);

    /**
     *changes the source of the last action.
     *@param cc source is a character the pointer to this character
     */
    void changeSource(Character *cc);

    /**
     *changes the source of the last action.
     *@param sI source is a item the new item
     */
    void changeSource(const ScriptItem &sI);

    /**
     *changes the Source of the last action.
     *@param pos source is a position the new position
     */
    void changeSource(position pos);

    /**
     * Changes the Source of the last action for this player.
     * Also sets the ActionType to TALK and aborts other non-talk actions currently active.
     * <b>Lua: [:changeSource]</b>
     * @param text the text of the player talk.
     * @param talkType the type of player talk done (whisper, shout, talk).
     */
    void changeSource(const std::string &text, Character::talk_type talkType);

    /**
     *changes the Source of the last action to nothing
     */
    void changeSource();

    /**
     *checks if the last action is abborted or  (Action is disturbed by another player or event)
     *@return true, if the action should be aborted
     */
    auto actionDisturbed(Character *disturber) -> bool;

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
    auto checkAction() -> bool;

    /**
     *checks if currently an action is running or not
     * @return true if there is a action running
     */
    inline auto actionRunning() const -> bool { return _actionrunning; }

private:
    std::shared_ptr<LuaScript> script = nullptr; /**< pointer to the last script*/
    ActionParameters currentScriptParameters{};
    ActionParameters currentActionParameters{};
    TYPE_OF_CHARACTER_ID parameterId = 0; /**< id of the source if its an character*/
    uint8_t characterType = 0;            /**< type of the source*/

    Player *_owner; /**< the owner of the LongTimeAction Objectt*/
    World *_world;  /**< pointer to the gameworld*/

    bool _actionrunning = false; /**< boolean value, if true there is currently a action running*/

    std::unique_ptr<Timer> _timetowaitTimer =
            nullptr; /**< timer which determines how many ms the action has to wait until it is sucessful*/
    std::unique_ptr<Timer> _redoaniTimer =
            nullptr; /**< timer which determines after how many ms the animation is shown again*/
    std::unique_ptr<Timer> _redosoundTimer =
            nullptr; /**< timer which determines how many ms the sound is played again.*/

    ActionType currentScriptType = ActionType::USE;
    ActionType currentActionType = ActionType::USE;

    unsigned short int _sound = 0; /**< id of the sound which is played to the action*/
    unsigned short int _ani = 0;   /**< id of the animation which is shown to the action*/

    void checkSource();
};

#endif
