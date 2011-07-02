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


#ifndef _CLUASCRIPT_HPP
#define _CLUASCRIPT_HPP

extern "C" {
#include <lua.h>
}

#include <stdexcept>
#include "globals.hpp"
#include "Item.hpp"
#include <luabind/object.hpp>

class Character;
class World;

/**
*@defgroup Scripts Scripts
* every script has a global member named <b>world</b> which is from Type cworld
* to have access to the game world
* Classes wich are exported to lua and has functions which are useable in lua
* for details click one of the classes
*/

/**
*@ingroup Scripts
*@defgroup Scriptclasses Classes for Scripts
*classes which has functions or members which are exportet to lua.
*/

/**
*@ingroup Scripts
*@defgroup Globalscriptfunctions Global Script functions
*functions or classfunctions which are exportet to lua
*/

/**
*@ingroup Scriptclasses
*@defgroup Scriptfunctions exported class functions
*class functions which are exported to lua
*/

/**
*@ingroup Scriptclasses
*@defgroup Scriptvariables exported class variables
*/

/**
*@ingroup Scripts
*@defgroup Scriptenums exported enums
*enums which are exported to lua
*/

/**
*@ingroup Scripts
*@defgroup Scripttypes types of scripts
*classes which holds scripts and the entrypoints
*/

/**
*base class for lua scripts
*defines the minimum of functions necessary to load scripts as well
*as initialising the basics (lua, luabind...)
*/

/**
*@ingroup Scripttypes
exception thrown if an error occurs during script loading/execution
*/
class ScriptException : public std::runtime_error {
public:
    /**
    * creates a new script exception and throws it
    */
    ScriptException(std::string s) throw() : std::runtime_error(s) {}
};

/**
*@ingroup Scriptenums
*enmus for target types of an action
*/
enum SouTarTypes {
    LUA_NONE = 0, /**< not a correct type (only for initialisation) */
    LUA_FIELD = 1,  /**< target was a field */
    LUA_ITEM = 2, /**< target was a item */
    LUA_CHARACTER = 3 /**< target was character*/
};

/**
*@ingroup Scriptenums
*states of long time actions
*/
enum LtaStates {
    LTS_NOLTACTION = 0, /**< no longtime action in this script */
    LTS_ACTIONABORTED = 1, /**< long time action was aborted */
    LTS_ACTIONSUCCESSFULL = 2  /**< long time action was performed sucessfulle*/
};

/**
* @ingroup Scriptclasses
* struct for Sources and Targets which are used by scripts to determine which script has to be called
*/
struct SouTar {
    Character *character;  /**< Source or target is a character, this is the pointer to it, otherwise NULL */
    ScriptItem item; /**< Source or target is a Item, this holds the information of the item */
    SouTarTypes Type; /**< Source or Target Type (if its an character, field or item) */
    position pos; /**< aboslute position of the object */
    /**
    * constructor which intializes the values
    */
    SouTar() {
        Type = LUA_NONE;
        item.id = 0;
        item.wear = 0;
        item.number = 0;
        character = NULL;
    }
};

/**
* @ingroup Scripttypes
* basic Script class all other scripts are from this basic type
*/
class LuaScript {
public:
    /**
    * initialise the libraries, exported functions and load the script
    * @param filename the filename of the script which has to be loaded
    * @throw ScriptException if an error occurs
    */
    LuaScript(std::string filename) throw(ScriptException);

    virtual ~LuaScript() throw();

    std::string getFileName() {
        return _filename;
    }


    /**
    * returns the current lua state
    */
    lua_State *getLuaState() {
        return _luaState;
    }

    static void shutdownLua();
    bool existsEntrypoint(std::string entrypoint);

protected:
    static lua_State *_luaState;
    static bool initialized;
    static void writeErrorMsg(); /**< returns a human readable error message */
    static void writeDebugMsg(std::string msg);   /**< writes a debug message to the script log */
public:
    luabind::object call(std::string entrypoint) throw(luabind::error);     /**< creates a lua call */

private:
    void init_base_functions(); /**< initialize basic functions of scripts */
    static int add_backtrace(lua_State *L); /**< adding a backtrace to script errors */

    /**
    * no copy constructor
    */
    LuaScript(const LuaScript &);
    LuaScript &operator=(const LuaScript &);

    std::string _filename;
    std::vector<std::string> vecPath;
};

#endif
