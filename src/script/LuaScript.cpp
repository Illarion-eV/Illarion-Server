/*
 *  illarionserver - server for the game Illarion
 *  Copyright 2011 Illarion e.V.
 *
 *  This file is part of illarionserver.
 *
 *  illarionserver is free software: you can redistribute it and/or modify
 *  it under the terms of the GNU Affero General Public License as published by
 *  the Free Software Foundation, either version 3 of the License, or
 *  (at your option) any later version.
 *
 *  illarionserver is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU Affero General Public License for more details.
 *
 *  You should have received a copy of the GNU Affero General Public License
 *  along with illarionserver.  If not, see <http://www.gnu.org/licenses/>.
 */

#include "LuaScript.hpp"

extern "C" {
#include <lauxlib.h>
#include <lualib.h>
}

#include <iostream>
#include <algorithm>
#include <cxxabi.h>

#include "luabind/luabind.hpp"
#include <luabind/raw_policy.hpp>

#include <boost/algorithm/string.hpp>

#include "Monster.hpp"
#include "NPC.hpp"
#include "Player.hpp"
#include "World.hpp"
#include "Logger.hpp"
#include "Config.hpp"

#include "data/Data.hpp"

#include "script/forwarder.hpp"
#include "script/binding/binding.hpp"

lua_State *LuaScript::_luaState = 0;
bool LuaScript::initialized = false;

LuaScript::LuaScript() {
    initialize();

    _filename = "";
}

LuaScript::LuaScript(std::string filename) {
    initialize();

    _filename = filename;

    strcpy(luafile, Config::instance().scriptdir().c_str());
    std::replace(filename.begin(), filename.end(), '.', '/');
    strcat(luafile, (filename + ".lua").c_str());

    loadIntoLuaState();
}

LuaScript::LuaScript(const std::string &code, const std::string &scriptname) {
    initialize();

    luaL_getsubtable(_luaState, LUA_REGISTRYINDEX, "_LOADED");

    int errorCode = luaL_loadbuffer(_luaState, code.c_str(), code.length(), scriptname.c_str());
    handleLuaLoadError(errorCode);

    errorCode = lua_pcall(_luaState, 0, LUA_MULTRET, 0);
    handleLuaCallError(errorCode);

    lua_setfield(_luaState, -2, _filename.c_str());
    lua_pop(_luaState, 1);
}

void LuaScript::initialize() {
    if (!initialized) {
        initialized = true;
        _luaState = luaL_newstate();
        luabind::open(_luaState);

        // use another error function to surpress errors from
        // non-existant entry points and to display a backtrace
        luabind::set_pcall_callback(LuaScript::add_backtrace);

        init_base_functions();

        char path[100];
        strcpy(path, Config::instance().scriptdir().c_str());
        strcat(path, "?.lua");

        lua_pushglobaltable(_luaState);
        lua_pushstring(_luaState, "package");
        lua_gettable(_luaState, -2);
        
        lua_pushstring(_luaState, "path");
        lua_pushstring(_luaState, path);
        lua_settable(_luaState, -3);
    }
}

void LuaScript::loadIntoLuaState() {
    luaL_getsubtable(_luaState, LUA_REGISTRYINDEX, "_LOADED");

    int errorCode = luaL_loadfile(_luaState, luafile);
    handleLuaLoadError(errorCode);
    if (errorCode) return;

    errorCode = lua_pcall(_luaState, 0, 1, 0);
    handleLuaCallError(errorCode);
    if (errorCode) return;

    lua_setfield(_luaState, -2, _filename.c_str());
    lua_pop(_luaState, 1);
}

void LuaScript::handleLuaLoadError(int errorCode) {
    if (errorCode) {
        std::string errorFile(luafile);

        switch (errorCode) {
        case LUA_ERRFILE:
            throw ScriptException("Could not access script file: " + errorFile);
            break;

        case LUA_ERRSYNTAX:
            throw ScriptException("Syntax error in script file: " + errorFile);
            break;

        case LUA_ERRMEM:
            throw ScriptException("Insufficient memory for loading script file: " + errorFile);
            break;

        default:
            throw ScriptException("Could not load script file: " + errorFile);
            break;
        }
    }
}

void LuaScript::handleLuaCallError(int errorCode) {
    if (errorCode) {
        std::string errorFile(luafile);

        switch (errorCode) {
        case LUA_ERRRUN:
            writeErrorMsg();
            break;

        case LUA_ERRMEM:
            throw ScriptException("Insufficient memory for running script file: " + errorFile);
            break;

        default:
            throw ScriptException("Could not load script file: " + errorFile);
            break;
        }
    }
}

LuaScript::~LuaScript() {
}

void LuaScript::shutdownLua() {
    World::get()->invalidatePlayerDialogs();

    if (initialized) {
        initialized = false;
        lua_close(_luaState);
        _luaState = 0;
    }
}

int LuaScript::add_backtrace(lua_State *L) {
    lua_Debug d;
    std::stringstream msg;

    if (lua_tostring(L, -1) != nullptr) {
        std::string err = lua_tostring(L, -1);
        lua_pop(L, 1);
        msg << err << std::endl;
    }

    int level = 0;

    while (lua_getstack(L, ++level, &d)) {
        lua_getinfo(L, "Sln", &d);
        msg << "#" << level << " called by: " << d.short_src << ":" << d.currentline;

        if (d.name != 0) {
            msg << "(" << d.namewhat << " " << d.name << ")";
        }

        msg << std::endl;
    }

    if (level == 1) { // do not mind if an entry point is missing
        std::string empty = "";
        lua_pushstring(L, empty.c_str());
    } else {
        lua_pushstring(L, msg.str().c_str());
    }

    return 1;
}

void LuaScript::triggerScriptError(const std::string &msg) {
    lua_pushstring(_luaState, msg.c_str());
    throw luabind::error(_luaState);
}

void LuaScript::writeErrorMsg() {
    const char *c_err = lua_tostring(_luaState, -1);
    lua_pop(_luaState, 1);

    std::string err;

    if (c_err) {
        err = c_err;
    } else {
        err = "UNKNOWN ERROR, CONTACT SERVER DEVELOPER";
        lua_pushstring(_luaState, err.c_str());
        add_backtrace(_luaState);
        err = lua_tostring(_luaState, -1);
        lua_pop(_luaState, 1);
    }

    if (err.length() > 1) {
        Logger::error(LogFacility::Script) << err << Log::end;
    }
}

void LuaScript::writeCastErrorMsg(const std::string &entryPoint, const luabind::cast_failed &e) const {
    std::string script = getFileName();
    char *expectedType = abi::__cxa_demangle(e.info().name(), 0, 0, 0);
    Logger::error(LogFacility::Script) << "Invalid return type in " << script << "." << entryPoint << ": " << "Expected type " << expectedType << Log::end;
    free(expectedType);
}

void LuaScript::writeDebugMsg(const std::string &msg) {
    if (Config::instance().debug) {
        lua_pushstring(_luaState, ("Debug Message: " + msg).c_str());
        add_backtrace(_luaState);
        std::string backtrace = lua_tostring(_luaState, -1);
        lua_pop(_luaState, 1);

        if (backtrace.length() > 0) {
            Logger::notice(LogFacility::Script) << backtrace << Log::end;
        }
    }
}

void LuaScript::writeDeprecatedMsg(const std::string &deprecatedEntity) {
    lua_pushstring(_luaState, ("Use of DEPRECATED " + deprecatedEntity).c_str());
    add_backtrace(_luaState);
    std::string backtrace = lua_tostring(_luaState, -1);
    lua_pop(_luaState, 1);

    if (backtrace.length() > 0) {
        Logger::warn(LogFacility::Script) << backtrace << Log::end;
    }
}

luabind::object LuaScript::buildEntrypoint(const std::string &entrypoint) {
    luabind::object obj = luabind::registry(_luaState);
    obj = obj["_LOADED"][_filename];

    if (luabind::type(obj) != LUA_TTABLE) {
        triggerScriptError(
            "Error while loading entrypoint '" + entrypoint + "' from module " +
            _filename + ". Check if the script returns its module as table.");
    }

    luabind::object callee = obj[entrypoint];
    return callee;
}

void LuaScript::addQuestScript(const std::string &entrypoint, const std::shared_ptr<LuaScript> &script) {
    questScripts.insert(std::pair<const std::string, std::shared_ptr<LuaScript> >(entrypoint, script));
}

void LuaScript::setCurrentWorldScript() {
    World::get()->setCurrentScript(this);
}

bool LuaScript::existsQuestEntrypoint(const std::string &entrypoint) const {
    return questScripts.find(entrypoint) != questScripts.end();
}

bool LuaScript::existsEntrypoint(const std::string &entrypoint) const {
    luabind::object obj = luabind::registry(_luaState);
    obj = obj["_LOADED"][_filename];

    if (luabind::type(obj) != LUA_TTABLE) {
        return existsQuestEntrypoint(entrypoint);
    }

    obj = obj[entrypoint];

    if (luabind::type(obj) != LUA_TFUNCTION) {
        return existsQuestEntrypoint(entrypoint);
    }

    return true;
}

static int dofile(lua_State *L, const char *fname) {
    char path[100];
    strcpy(path, Config::instance().scriptdir().c_str());
    strcat(path, fname);
    int n = lua_gettop(L);
    int status = luaL_loadfile(L, path);

    if (status != 0) {
        lua_error(L);
    }

    lua_call(L, 0, LUA_MULTRET);
    return lua_gettop(L) - n;
}

Character *getCharForId(TYPE_OF_CHARACTER_ID id) {
    Character *ret = nullptr;

    if (id < MONSTER_BASE) {
        //player
        ret = World::get()->Players.find(id);
    } else if (id >= MONSTER_BASE && id < NPC_BASE) {
        //monster
        ret = World::get()->Monsters.find(id);
    } else {
        ret = World::get()->Npc.find(id);
    }

    return ret;
}

void LuaScript::init_base_functions() {
    static const luaL_Reg lualibs[] = {
        {"_G", luaopen_base},
        {LUA_LOADLIBNAME, luaopen_package},
        {LUA_TABLIBNAME, luaopen_table},
        {LUA_IOLIBNAME, luaopen_io},
        // {LUA_OSLIBNAME, luaopen_os},
        {LUA_STRLIBNAME, luaopen_string},
        {LUA_MATHLIBNAME, luaopen_math},
        // {LUA_DBLIBNAME, luaopen_debug},
        {LUA_BITLIBNAME, luaopen_bit32},
        {nullptr, nullptr}
    };
    const luaL_Reg *lib = lualibs;

    for (; lib->func; lib++) {
        luaL_requiref(_luaState, lib->name, lib->func, 1);
        lua_pop(_luaState, 1);  // remove lib
    }



    luabind::module(_luaState)
    [
        binding::armor_struct(),
        binding::attack_boni(),
        binding::character(),
        binding::character_skillvalue(),
        binding::colour(),
        binding::item_struct(),
        binding::container(),
        binding::crafting_dialog(),
        binding::field(),
        binding::input_dialog(),
        binding::item(),
        binding::item_look_at(),
        binding::long_time_action(),
        binding::long_time_character_effects(),
        binding::long_time_effect(),
        binding::long_time_effect_struct(),
        binding::merchant_dialog(),
        binding::message_dialog(),
        binding::monster(),
        binding::monster_armor(),
        binding::npc(),
        binding::player(),
        binding::position(),
        binding::random(),
        binding::script_item(),
        binding::script_variables_table(),
        binding::selection_dialog(),
        binding::tiles_struct(),
        binding::waypoint_list(),
        binding::weapon_struct(),
        binding::weather_struct(),
        binding::world(),

        luabind::def("dofile", &dofile, luabind::raw(_1)),
        luabind::def("getCharForId",getCharForId),
        luabind::def("isValidChar", &isValid),
        luabind::def("debug", &LuaScript::writeDebugMsg),
        luabind::def("log", log_lua)
    ];

    luabind::object globals = luabind::globals(_luaState);
    globals["world"] = World::get();
    globals["ScriptVars"] = &Data::ScriptVariables;
}

