/*
 *  illarionserver - server for the game Illarion
 *  Copyright 2011 Illarion e.V.
 *
 *  This file is part of illarionserver.
 *
 *  illarionserver is free software: you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation, either version 3 of the License, or
 *  (at your option) any later version.
 *
 *  illarionserver is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with illarionserver.  If not, see <http://www.gnu.org/licenses/>.
 */

#include "LuaScript.hpp"

extern "C" {
#include <lauxlib.h>
#include <lualib.h>
}

#include "luabind/luabind.hpp"
#include "luabind/operator.hpp"
#include <luabind/adopt_policy.hpp>
#include "luabind/out_value_policy.hpp"
#include <luabind/raw_policy.hpp>
#include <iostream>
#include <algorithm>
#include <boost/algorithm/string.hpp>
#include "Character.hpp"
#include "NPC.hpp"
#include "Field.hpp"
#include "Item.hpp"
#include "Player.hpp"
#include "World.hpp"
#include "globals.hpp"
#include "TableStructs.hpp"
#include "Container.hpp"
#include "LongTimeAction.hpp"
#include "LongTimeEffect.hpp"
#include "LongTimeCharacterEffects.hpp"
#include "data/ScriptVariablesTable.hpp"
#include "data/Data.hpp"
#include "Logger.hpp"
#include "WaypointList.hpp"
#include "fuse_ptr.hpp"
#include "dialog/InputDialog.hpp"
#include "dialog/MessageDialog.hpp"
#include "dialog/MerchantDialog.hpp"
#include "dialog/SelectionDialog.hpp"
#include "dialog/CraftingDialog.hpp"
#include "Random.hpp"
#include <cxxabi.h>
#include "Config.hpp"
#include "script/forwarder.hpp"

extern ScriptVariablesTable *scriptVariables;

lua_State *LuaScript::_luaState = 0;
bool LuaScript::initialized = false;

LuaScript::LuaScript() {
    initialize();

    _filename = "";
}

LuaScript::LuaScript(std::string filename) throw(ScriptException) {
    initialize();

    _filename = filename;
    boost::split(vecPath, filename, boost::is_any_of("."));

    strcpy(luafile, Config::instance().scriptdir().c_str());
    std::replace(filename.begin(), filename.end(), '.', '/');
    strcat(luafile, (filename + ".lua").c_str());

    loadIntoLuaState();
}

LuaScript::LuaScript(const std::string &code, const std::string &scriptname) throw(ScriptException) {
    initialize();

    int err = luaL_loadbuffer(_luaState, code.c_str(), code.length(), scriptname.c_str());

    if (err != 0) {
        std::string errstr(luafile);

        switch (err) {
        case LUA_ERRFILE:
            throw ScriptException("Could not access script file: " + errstr);
            break;

        case LUA_ERRSYNTAX:
            throw ScriptException("Syntax error in script file: " + errstr);
            break;

        case LUA_ERRMEM:
            throw ScriptException("Insufficient memory for loading script file: " + errstr);
            break;

        default:
            throw ScriptException("Could not load script file: " + errstr);
            break;
        }
    }

    err = lua_pcall(_luaState, 0, LUA_MULTRET, 0);

    if (err != 0) {
        std::string errstr(luafile);

        switch (err) {
        case LUA_ERRRUN:
            writeErrorMsg();
            break;

        case LUA_ERRMEM:
            throw ScriptException("Insufficient memory for running script file: " + errstr);
            break;

        default:
            throw ScriptException("Could not load script file: " + errstr);
            break;
        }
    }
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
        lua_pushstring(_luaState, "package");
        lua_gettable(_luaState, LUA_GLOBALSINDEX);
        lua_pushstring(_luaState, "path");
        lua_pushstring(_luaState, path);
        lua_settable(_luaState, -3);
    }
}

void LuaScript::loadIntoLuaState() {
    int err = luaL_loadfile(_luaState, luafile);

    if (err != 0) {
        std::string errstr(luafile);

        switch (err) {
        case LUA_ERRFILE:
            throw ScriptException("Could not access script file: " + errstr);
            break;

        case LUA_ERRSYNTAX:
            throw ScriptException("Syntax error in script file: " + errstr);
            break;

        case LUA_ERRMEM:
            throw ScriptException("Insufficient memory for loading script file: " + errstr);
            break;

        default:
            throw ScriptException("Could not load script file: " + errstr);
            break;
        }
    }

    err = lua_pcall(_luaState, 0, LUA_MULTRET, 0);

    if (err != 0) {
        std::string errstr(luafile);

        switch (err) {
        case LUA_ERRRUN:
            writeErrorMsg();
            break;

        case LUA_ERRMEM:
            throw ScriptException("Insufficient memory for running script file: " + errstr);
            break;

        default:
            throw ScriptException("Could not load script file: " + errstr);
            break;
        }
    }
}

LuaScript::~LuaScript() throw() {
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

    if (lua_tostring(L, -1) != NULL) {
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

void LuaScript::triggerScriptError(const std::string &msg) throw(luabind::error) {
    lua_pushstring(_luaState, msg.c_str());
    throw luabind::error(_luaState);
}

void LuaScript::writeErrorMsg() {
    const char *cerr = lua_tostring(_luaState, -1);
    lua_pop(_luaState, 1);

    std::string err;

    if (cerr) {
        err = cerr;
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

void LuaScript::writeCastErrorMsg(const std::string &entryPoint, const luabind::cast_failed &e) {
    std::string script = getFileName();
    char *expectedType = abi::__cxa_demangle(e.info().name(), 0, 0, 0);
    Logger::error(LogFacility::Script) << "Invalid return type in " << script << "." << entryPoint << ": " << "Expected type " << expectedType << Log::end;
    free(expectedType);
}

void LuaScript::writeDebugMsg(const std::string &msg) {
#ifdef TESTSERVER
    lua_pushstring(_luaState, ("Debug Message: " + msg).c_str());
    add_backtrace(_luaState);
    std::string backtrace = lua_tostring(_luaState, -1);
    lua_pop(_luaState, 1);

    if (backtrace.length() > 0) {
        Logger::notice(LogFacility::Script) << backtrace << Log::end;
    }

#endif
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

bool LuaScript::isTestserver() {
#ifdef TESTSERVER
    return true;
#else
    return false;
#endif
}

luabind::object LuaScript::buildEntrypoint(const std::string &entrypoint) throw(luabind::error) {
    luabind::object obj = luabind::globals(_luaState);
    std::string currentpath = "";

    for (std::vector<std::string>::iterator it = vecPath.begin(); it != vecPath.end(); ++it) {
        obj = obj[*it];
        currentpath = currentpath + "." + *it;

        if (luabind::type(obj) != LUA_TTABLE) {
            triggerScriptError("Error while loading entrypoint '" + entrypoint
                               + "': " + currentpath.erase(0,1)
                               + " does not exist! Check module statement in "
                               + _filename + "!"
                              );
        }
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

bool LuaScript::existsQuestEntrypoint(const std::string &entrypoint) {
    return questScripts.find(entrypoint) != questScripts.end();
}

bool LuaScript::existsEntrypoint(const std::string &entrypoint) {
    luabind::object obj = luabind::globals(_luaState);

    for (auto it = vecPath.begin(); it != vecPath.end(); ++it) {
        obj = obj[*it];

        if (luabind::type(obj) != LUA_TTABLE) {
            return existsQuestEntrypoint(entrypoint);
        }
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
    std::cout << "loading file: " << path << std::endl;
    int n = lua_gettop(L);
    int status = luaL_loadfile(L, path);

    if (status != 0) {
        lua_error(L);
    }

    lua_call(L, 0, LUA_MULTRET);
    return lua_gettop(L) - n;
}

void printerr(std::string err) {
    std::cout << "script error: " << err << std::endl;
}

unsigned int LuaAnd(unsigned int operand1, unsigned int operand2) {
    return (operand1 & operand2);
}

uint64_t LuaAnd64(uint64_t operand1, uint64_t operand2) {
    return (operand1 & operand2);
}

unsigned int LuaOr(unsigned int operand1, unsigned int operand2) {
    return (operand1 | operand2);
}

uint64_t LuaOr64(uint64_t operand1, uint64_t operand2) {
    return (operand1 | operand2);
}

uint32_t LuaLShift32(uint32_t value, unsigned char bits) {
    return value << bits;
}

uint32_t LuaRShift32(uint32_t value, unsigned char bits) {
    return value >> bits;
}

uint64_t LuaRShift64(uint64_t value, unsigned char bits) {
    return value >> bits;
}

uint64_t LuaLShift64(uint64_t value, unsigned char bits) {
    return value << bits;
}

Character *getCharForId(TYPE_OF_CHARACTER_ID id) {
    Character *ret = NULL;

    if (id < MONSTER_BASE) {
        //player
        ret = World::get()->Players.findID(id);
    } else if (id >= MONSTER_BASE && id < NPC_BASE) {
        //monster
        ret = World::get()->Monsters.findID(id);
    } else {
        ret = World::get()->Npc.findID(id);
    }

    return ret;
}

void LuaScript::init_base_functions() {
    static const luaL_Reg lualibs[] = {
        {"", luaopen_base},
        {LUA_LOADLIBNAME, luaopen_package},
        {LUA_TABLIBNAME, luaopen_table},
        {LUA_IOLIBNAME, luaopen_io},
        // {LUA_OSLIBNAME, luaopen_os},
        {LUA_STRLIBNAME, luaopen_string},
        {LUA_MATHLIBNAME, luaopen_math},
        // {LUA_DBLIBNAME, luaopen_debug},
        {NULL, NULL}
    };
    const luaL_Reg *lib = lualibs;

    for (; lib->func; lib++) {
        lua_pushcfunction(_luaState, lib->func);
        lua_pushstring(_luaState, lib->name);
        lua_call(_luaState, 1, 0);
    }

    luabind::value_vector skills;

    for (const auto &skill : Data::Skills) {
        skills.push_back(luabind::value(skill.second.serverName.c_str(), skill.first));
    }

    luabind::module(_luaState)
    [
        luabind::class_<InputDialog>("InputDialog")
        .def(luabind::constructor<std::string, std::string, bool, unsigned short, luabind::object>())
        .def("getSuccess", &InputDialog::getSuccess)
        .def("getInput", &InputDialog::getInput),
        luabind::class_<MessageDialog>("MessageDialog")
        .def(luabind::constructor<std::string, std::string, luabind::object>()),
        luabind::class_<MerchantDialog>("MerchantDialog")
        .enum_("Result")
        [
            luabind::value("playerAborts",0),
            luabind::value("playerSells",1),
            luabind::value("playerBuys",2)
        ]
        .def(luabind::constructor<std::string, luabind::object>())
        .def("addOffer", (void(MerchantDialog:: *)(TYPE_OF_ITEM_ID, const std::string &, TYPE_OF_WORTH))&MerchantDialog::addOffer)
        .def("addOffer", (void(MerchantDialog:: *)(TYPE_OF_ITEM_ID, const std::string &, TYPE_OF_WORTH, TYPE_OF_BUY_STACK))&MerchantDialog::addOffer)
        .def("addPrimaryRequest", &MerchantDialog::addPrimaryRequest)
        .def("addSecondaryRequest", &MerchantDialog::addSecondaryRequest)
        .def("getResult", &MerchantDialog::getResult)
        .def("getPurchaseIndex", &MerchantDialog::getPurchaseIndex)
        .def("getPurchaseAmount", &MerchantDialog::getPurchaseAmount)
        .def("getSaleItem", &MerchantDialog::getSaleItem),
        luabind::class_<SelectionDialog>("SelectionDialog")
        .def(luabind::constructor<std::string, std::string, luabind::object>())
        .def("addOption", &SelectionDialog::addOption)
        .def("getSuccess", &SelectionDialog::getSuccess)
        .def("getSelectedIndex", &SelectionDialog::getSelectedIndex)
        .def("setCloseOnMove", &SelectionDialog::setCloseOnMove),
        luabind::class_<CraftingDialog>("CraftingDialog")
        .enum_("Result")
        [
            luabind::value("playerAborts", 0),
            luabind::value("playerCrafts", 1),
            luabind::value("playerLooksAtItem", 2),
            luabind::value("playerLooksAtIngredient", 3),
            luabind::value("playerCraftingComplete", 4),
            luabind::value("playerCraftingAborted", 5)
        ]
        .def(luabind::constructor<std::string, uint16_t, uint16_t, luabind::object>())
        .def("clearGroupsAndProducts", &CraftingDialog::clearGroupsAndProducts)
        .def("addGroup", &CraftingDialog::addGroup)
        .def("addCraftable", (void(CraftingDialog:: *)(uint8_t, uint8_t, TYPE_OF_ITEM_ID, const std::string &, uint16_t))&CraftingDialog::addCraftable)
        .def("addCraftable", (void(CraftingDialog:: *)(uint8_t, uint8_t, TYPE_OF_ITEM_ID, const std::string &, uint16_t, uint8_t))&CraftingDialog::addCraftable)
        .def("addCraftableIngredient", (void(CraftingDialog:: *)(TYPE_OF_ITEM_ID))&CraftingDialog::addCraftableIngredient)
        .def("addCraftableIngredient", (void(CraftingDialog:: *)(TYPE_OF_ITEM_ID, uint8_t))&CraftingDialog::addCraftableIngredient)
        .def("getResult", &CraftingDialog::getResult)
        .def("getCraftableId", &CraftingDialog::getCraftableId)
        .def("getCraftableAmount", &CraftingDialog::getCraftableAmount)
        .def("getIngredientIndex", &CraftingDialog::getIngredientIndex),
        luabind::class_<LongTimeAction>("Action")
        .enum_("state")
        [
            luabind::value("none",0), /**< no action */
            luabind::value("abort",1), /**< action aborted */
            luabind::value("success",2) /**< action successed */
        ],
        luabind::class_<position>("position")
        .def(luabind::constructor<>())
        .def(luabind::constructor<short int, short int, short int>())
        .def(luabind::self == luabind::const_self)
        .def(luabind::tostring(luabind::const_self))
        .def_readwrite("x",&position::x)
        .def_readwrite("y",&position::y)
        .def_readwrite("z",&position::z),
        luabind::class_<WeatherStruct>("WeatherStruct")
        .def(luabind::constructor<>())
        .def(luabind::constructor<char,char,char,char,char,char,char,char>())
        .def_readwrite("cloud_density",&WeatherStruct::cloud_density)
        .def_readwrite("fog_density",&WeatherStruct::fog_density)
        .def_readwrite("wind_dir",&WeatherStruct::wind_dir)
        .def_readwrite("gust_strength",&WeatherStruct::gust_strength)
        .def_readwrite("percipitation_strength",&WeatherStruct::percipitation_strength)
        .def_readwrite("percipitation_type",&WeatherStruct::per_type)
        .def_readwrite("thunderstorm", &WeatherStruct::thunderstorm)
        .def_readwrite("temperature", &WeatherStruct::temperature),
        luabind::class_<Character>("Character")
        .def("pageGM", &Character::pageGM)
        .def("requestInputDialog", &Character::requestInputDialog, luabind::adopt(_2))
        .def("requestMessageDialog", &Character::requestMessageDialog, luabind::adopt(_2))
        .def("requestMerchantDialog", &Character::requestMerchantDialog, luabind::adopt(_2))
        .def("requestSelectionDialog", &Character::requestSelectionDialog, luabind::adopt(_2))
        .def("requestCraftingDialog", &Character::requestCraftingDialog, luabind::adopt(_2))
        .def("requestCraftingLookAt", &Character::requestCraftingLookAt)
        .def("requestCraftingLookAtIngredient", &Character::requestCraftingLookAtIngredient)
        .def("idleTime", &Character::idleTime)
        .def("sendBook", &Character::sendBook)
        .def("updateAppearance", &Character::forceUpdateAppearanceForAll)
        .def("performAnimation", &Character::performAnimation)
        .def("alterSpokenMessage", &Character::alterSpokenMessage)
        .def("actionRunning", &Character::actionRunning)
        .def("changeQualityAt", &Character::changeQualityAt)
        .def("isAdmin", &Character::isAdmin)
        .def("talk", (void(Character:: *)(Character::talk_type, const std::string &))&Character::talk)
        .def("talk", (void(Character:: *)(Character::talk_type, const std::string &, const std::string &))&Character::talk)
        .def("sendCharDescription", &Character::sendCharDescription)
        .def("startAction", &Character::startAction)
        .def("abortAction", &Character::abortAction)
        .def("successAction", &Character::successAction)
        .def("disturbAction", &Character::actionDisturbed)
        .def("changeSource", (void(Character:: *)(Character *))&Character::changeSource)
        .def("changeSource", (void(Character:: *)(const ScriptItem &))&Character::changeSource)
        .def("changeSource", (void(Character:: *)(const position &))&Character::changeSource)
        .def("changeSource", (void(Character:: *)(void))&Character::changeSource)
        .def("changeTarget", (void(Character:: *)(Character *))&Character::changeTarget)
        .def("changeTarget", (void(Character:: *)(const ScriptItem &))&Character::changeTarget)
        .def("changeTarget", (void(Character:: *)(const position &))&Character::changeTarget)
        .def("changeTarget", (void(Character:: *)(void))&Character::changeTarget)
        .def("inform", inform_lua1)
        .def("inform", inform_lua2)
        .def("inform", inform_lua3)
        .def("inform", inform_lua4)
        .def("introduce", &Character::introducePlayer)
        .def("move", &Character::move)
        .def("getNextStepDir", &Character::getNextStepDir, luabind::pure_out_value(_3))
        .def("getRace", &Character::get_race)
        .def("getFaceTo", &Character::get_face_to)
        .def("getType", &Character::get_character)
        .def("createItem", create_item)
        .def("increasePoisonValue", &Character::increasePoisonValue)
        .def("getPoisonValue", &Character::getPoisonValue)
        .def("setPoisonValue", &Character::setPoisonValue)
        .def("getMentalCapacity", &Character::getMentalCapacity)
        .def("setMentalCapacity", &Character::setMentalCapacity)
        .def("increaseMentalCapacity", &Character::increaseMentalCapacity)
        .def("setClippingActive", &Character::setClippingActive)
        .def("getClippingActive", &Character::getClippingActive)
        .def("countItem", &Character::countItem)
        .def("countItemAt", count_item_at1)
        .def("countItemAt", count_item_at2)
        .def("eraseItem", erase_item1)
        .def("eraseItem", erase_item2)
        .def("increaseAtPos", &Character::increaseAtPos)
        .def("swapAtPos", &Character::swapAtPos)
        .def("createAtPos", &Character::createAtPos)
        .def("getItemAt", &Character::GetItemAt)
        .enum_("skills")
        [
            skills
        ]
        .def("getSkillName", &Character::getSkillName)
        .def("getSkill", &Character::getSkill)
        .def("getMinorSkill", &Character::getMinorSkill)
        .def("increaseAttrib", &Character::increaseAttrib)
        .def("setAttrib", &Character::setAttrib)
        .def("increaseSkill", &Character::increaseSkill)
        .def("increaseMinorSkill", &Character::increaseMinorSkill)
        .def("setSkill", &Character::setSkill)
        .def("setSkinColor", &Character::setSkinColor)
        .def("getSkinColor", &Character::getSkinColor, luabind::pure_out_value(_2) + luabind::pure_out_value(_3) + luabind::pure_out_value(_4))
        .def("setHairColor", &Character::setHairColor)
        .def("getHairColor", &Character::getHairColor, luabind::pure_out_value(_2) + luabind::pure_out_value(_3) + luabind::pure_out_value(_4))
        .def("setHair", &Character::setHair)
        .def("getHair", &Character::getHair)
        .def("setBeard", &Character::setBeard)
        .def("getBeard", &Character::getBeard)
        .def("learn", &Character::learn)
        .def("getSkillValue",&Character::getSkillValue)
        .def("teachMagic", &Character::teachMagic)
        .def("isInRange", &Character::isInRange)
        .def("isInRangeToPosition", &Character::isInRangeToField)
        .def("distanceMetric", &Character::distanceMetric)
        .def("distanceMetricToPosition", &Character::distanceMetricToPosition)
        .def("getMagicType", &Character::get_magic_type)
        .def("setMagicType", &Character::set_magic_type)
        .def("getMagicFlags", &Character::get_magic_flags)
        .def("warp", &Character::Warp)
        .def("forceWarp", &Character::forceWarp)
        .def("startMusic", &Character::startMusic)
        .def("defaultMusic", &Character::defaultMusic)
        .def("callAttackScript", &Character::callAttackScript)
        .def("getItemList", character_getItemList)
        .property("lastSpokenText", &Character::getLastSpokenText)
        .def("getPlayerLanguage", getPlayerLanguageLua)
        .def("getBackPack", &Character::GetBackPack)
        .def("getDepot", &Character::GetDepot)
        .def("setQuestProgress", &Character::setQuestProgress)
        .def("getQuestProgress", &Character::getQuestProgress, luabind::pure_out_value(_3))
        .def("getOnRoute",&Character::getOnRoute)
        .def("setOnRoute",&Character::setOnRoute)
        .def("getMonsterType", &Character::getType)
        .def_readonly("effects", &Character::effects)
        .def_readonly("waypoints", &Character::waypoints)
        .property("pos", &Character::getPosition)
        .property("name", &Character::getName)
        .property("id", &Character::getId)
        .property("activeLanguage", &Character::getActiveLanguage, &Character::setActiveLanguage)
        .property("movepoints", &Character::getActionPoints, &Character::setActionPoints)
        .property("fightpoints", &Character::getFightPoints, &Character::setFightPoints)
        .property("isinvisible", &Character::isInvisible, &Character::setInvisible)
        .property("attackmode", &Character::getAttackMode)
        //.def_readonly("isTarget", &Character::isTarget)
        .enum_("body_pos")
        [
            luabind::value("backpack", 0),
            luabind::value("head", 1),
            luabind::value("neck", 2),
            luabind::value("breast", 3),
            luabind::value("hands", 4),
            luabind::value("left_tool", 5),
            luabind::value("right_tool", 6),
            luabind::value("finger_left_hand", 7),
            luabind::value("finger_right_hand", 8),
            luabind::value("legs", 9),
            luabind::value("feet", 10),
            luabind::value("coat", 11),
            luabind::value("belt_pos_1", 12),
            luabind::value("belt_pos_2", 13),
            luabind::value("belt_pos_3", 14),
            luabind::value("belt_pos_4", 15),
            luabind::value("belt_pos_5", 16),
            luabind::value("belt_pos_6", 17)
        ]
        .enum_("magic_flags")
        [
            luabind::value("mage", 0),
            luabind::value("priest", 1),
            luabind::value("bard", 2),
            luabind::value("druid", 3)
        ]
        .enum_("talk_type")
        [
            luabind::value("say",0),
            luabind::value("whisper",1),
            luabind::value("yell",2)
        ]
        .enum_("direction")
        [
            luabind::value("dir_north",0),
            luabind::value("dir_northeast",1),
            luabind::value("dir_east",2),
            luabind::value("dir_southeast",3),
            luabind::value("dir_south",4),
            luabind::value("dir_southwest",5),
            luabind::value("dir_west",6),
            luabind::value("dir_northwest",7),
            luabind::value("dir_up",8),
            luabind::value("dir_down",9)
        ]
        .enum_("character_type")
        [
            luabind::value("player",0),
            luabind::value("monster",1),
            luabind::value("npc",2)
        ]
        .enum_("sex_type")
        [
            luabind::value("male",0),
            luabind::value("female",1)
        ]
        .enum_("face_to")
        [
            luabind::value("north",0),
            luabind::value("northeast",1),
            luabind::value("east",2),
            luabind::value("southeast",3),
            luabind::value("south",4),
            luabind::value("southwest",5),
            luabind::value("west",6),
            luabind::value("northwest",7)
        ]
        .enum_("race_type")
        [
            luabind::value("human",0),
            luabind::value("dwarf",1),
            luabind::value("halfling",2),
            luabind::value("elf",3),
            luabind::value("orc",4),
            luabind::value("lizardman",5),
            luabind::value("gnome",6),
            luabind::value("fairy",7),
            luabind::value("goblin",8),
            luabind::value("troll",9),
            luabind::value("mumie",10),
            luabind::value("skeleton",11),
            luabind::value("beholder",12),
            luabind::value("cloud",13),
            luabind::value("healer",14),
            luabind::value("buyer",15),
            luabind::value("seller",16),
            luabind::value("insects",17),
            luabind::value("sheep",18),
            luabind::value("spider",19),
            luabind::value("demonskeleton",20),
            luabind::value("rotworm",21),
            luabind::value("bigdemon",22),
            luabind::value("scorpion",23),
            luabind::value("pig",24),
            luabind::value("unknown",25),
            luabind::value("skull",26),
            luabind::value("wasp",27),
            luabind::value("foresttroll",28),
            luabind::value("shadowskeleton",29),
            luabind::value("stonegolem",30),
            luabind::value("mgoblin",31),
            luabind::value("gnoll",32),
            luabind::value("dragon",33),
            luabind::value("mdrow",34),
            luabind::value("fdrow",35),
            luabind::value("lesserdemon",36)
        ]
        .enum_("inform_type")
        [
            luabind::value("lowPriority",100),
            luabind::value("mediumPriority",101),
            luabind::value("highPriority",102)
        ],
        luabind::class_<Character::skillvalue>("skillvalue")
        .def(luabind::constructor<>())
        .def_readwrite("major",&Character::skillvalue::major)
        .def_readwrite("minor",&Character::skillvalue::minor),
        luabind::class_<NPC, Character>("NPC"),
        luabind::class_<Monster, Character>("Monster"),
        luabind::class_<WaypointList>("WaypointList")
        .def("addFromList", &waypointlist_addFromList)
        .def("getWaypoints",&waypointlist_getWaypoints)
        .def("addWaypoint",&WaypointList::addWaypoint)
        .def("clear",&WaypointList::clear),
        luabind::class_<LongTimeCharacterEffects>("LongTimeCharacterEffects")
        .def("addEffect",&LongTimeCharacterEffects::addEffect, luabind::adopt(_2))
        .def("removeEffect", (bool(LongTimeCharacterEffects:: *)(uint16_t))&LongTimeCharacterEffects::removeEffect)
        .def("removeEffect", (bool(LongTimeCharacterEffects:: *)(const std::string &))&LongTimeCharacterEffects::removeEffect)
        .def("removeEffect", (bool(LongTimeCharacterEffects:: *)(LongTimeEffect *))&LongTimeCharacterEffects::removeEffect)
        .def("find", (bool(LongTimeCharacterEffects:: *)(uint16_t,LongTimeEffect *&))&LongTimeCharacterEffects::find,luabind::pure_out_value(_3))
        .def("find", (bool(LongTimeCharacterEffects:: *)(const std::string &,LongTimeEffect *&))&LongTimeCharacterEffects::find,luabind::pure_out_value(_3)),
        luabind::class_<Field>("Field")
        .def("tile", &Field::getTileId)
        //.def("changeQualityOfTopItem", &Field::changeQualityOfTopItem)
        .def("getStackItem", &Field::getStackItem)
        .def("createItemAlways", &Field::PutTopItem)
        .def("createItem", &Field::addTopItem)
        .def("createItemGround", &Field::PutGroundItem)
        .def("takeTopItem", &Field::TakeTopItem, luabind::pure_out_value(_2))
        .def("increaseTopItem", &Field::increaseTopItem, luabind::pure_out_value(_3))
        .def("swapTopItem", &Field::swapTopItem)
        .def("viewTopItem", &Field::ViewTopItem)
        .def("countItems", &Field::NumberOfItems)
        .def("deleteAllItems", &Field::DeleteAllItems)
        .def("isPassable", &Field::IsPassable),
        luabind::class_<ItemLookAt>("ItemLookAt")
        .def(luabind::constructor<>())
        .enum_("Rareness")
        [
            luabind::value("commonItem", ItemLookAt::commonItem),
            luabind::value("uncommonItem", ItemLookAt::uncommonItem),
            luabind::value("rareItem", ItemLookAt::rareItem),
            luabind::value("epicItem", ItemLookAt::epicItem)
        ]
        .property("name", &ItemLookAt::getName, &ItemLookAt::setName)
        .property("rareness", &ItemLookAt::getRareness, &ItemLookAt::setRareness)
        .property("description", &ItemLookAt::getDescription, &ItemLookAt::setDescription)
        .property("craftedBy", &ItemLookAt::getCraftedBy, &ItemLookAt::setCraftedBy)
        .property("weight", &ItemLookAt::getWeight, &ItemLookAt::setWeight)
        .property("worth", &ItemLookAt::getWorth, &ItemLookAt::setWorth)
        .property("qualityText", &ItemLookAt::getQualityText, &ItemLookAt::setQualityText)
        .property("durabilityText", &ItemLookAt::getDurabilityText, &ItemLookAt::setDurabilityText)
        .property("durabilityValue", &ItemLookAt::getDurabilityValue, &ItemLookAt::setDurabilityValue)
        .property("diamondLevel", &ItemLookAt::getDiamondLevel, &ItemLookAt::setDiamondLevel)
        .property("emeraldLevel", &ItemLookAt::getEmeraldLevel, &ItemLookAt::setEmeraldLevel)
        .property("rubyLevel", &ItemLookAt::getRubyLevel, &ItemLookAt::setRubyLevel)
        .property("sapphireLevel", &ItemLookAt::getSapphireLevel, &ItemLookAt::setSapphireLevel)
        .property("amethystLevel", &ItemLookAt::getAmethystLevel, &ItemLookAt::setAmethystLevel)
        .property("obsidianLevel", &ItemLookAt::getObsidianLevel, &ItemLookAt::setObsidianLevel)
        .property("topazLevel", &ItemLookAt::getTopazLevel, &ItemLookAt::setTopazLevel)
        .property("bonus", &ItemLookAt::getBonus, &ItemLookAt::setBonus),
        luabind::class_<Item>("Item")
        .def(luabind::constructor<>())
        .def(luabind::constructor<Item::id_type, Item::number_type, Item::wear_type, Item::quality_type>())
        .property("id", &Item::getId, &Item::setId)
        .property("wear", &Item::getWear, &Item::setWear)
        .property("number", &Item::getNumber, &Item::setNumber)
        .property("quality", &Item::getQuality, &Item::setQuality)
        .def("setData", (void(Item:: *)(const std::string &, const std::string &))&Item::setData)
        .def("setData", (void(Item:: *)(const std::string &, int32_t))&Item::setData)
        .def("getData", (std::string(Item:: *)(const std::string &))&Item::getData),
        luabind::class_<ScriptItem,Item>("scriptItem")
        .def(luabind::constructor<>())
        .def_readonly("owner", &ScriptItem::getOwnerForLua)
        .def_readonly("pos", &ScriptItem::pos)
        .def("getType", &ScriptItem::getType)
        .def_readonly("itempos", &ScriptItem::itempos)
        .def_readonly("inside", &ScriptItem::inside)
        .enum_("Types")
        [
            luabind::value("notdefined",0),
            luabind::value("showcase1",1),
            luabind::value("showcase2",2),
            luabind::value("field",3),
            luabind::value("inventory",4),
            luabind::value("belt",5),
            luabind::value("container",6)
        ],
        luabind::class_<LongTimeEffect>("LongTimeEffect")
        .def(luabind::constructor<uint16_t, uint32_t>())
        .def("addValue",&LongTimeEffect::addValue)
        .def("removeValue",&LongTimeEffect::removeValue)
        .def("findValue",&LongTimeEffect::findValue, luabind::pure_out_value(_3))
        .property("effectId",&LongTimeEffect::getEffectId)
        .property("effectName",&LongTimeEffect::getEffectName)
        .property("nextCalled",&LongTimeEffect::getExecuteIn, &LongTimeEffect::setExecuteIn)
        .property("numberCalled",&LongTimeEffect::getNumberOfCalls),
        luabind::class_<LongTimeEffectStruct>("LongTimeEffectStruct")
        .def(luabind::constructor<>())
        .def_readonly("effectId",&LongTimeEffectStruct::effectid)
        .def_readonly("effectName", &LongTimeEffectStruct::effectname)
        .def_readonly("scriptName", &LongTimeEffectStruct::scriptname),
        luabind::class_<Player, Character>("Player")
        .enum_("player_language")
        [
            luabind::value("german", static_cast<uint32_t>(Language::german)),
            luabind::value("english", static_cast<uint32_t>(Language::english))
        ],
        luabind::class_<World>("World")
        .def("LoS", &world_LuaLoS)
        .def("deleteNPC", &World::deleteNPC)
        .def("createDynamicNPC", &World::createDynamicNPC)
        .def("getPlayersOnline", &world_getPlayersOnline)
        .def("getNPCS", &world_getNPCS)
        .def("getCharactersInRangeOf", &world_getCharactersInRangeOf)
        .def("getPlayersInRangeOf", &world_getPlayersInRangeOf)
        .def("getMonstersInRangeOf", &world_getMonstersInRangeOf)
        .def("getNPCSInRangeOf", &world_getNPCSInRangeOf)
        .def("getArmorStruct", &World::getArmorStruct, luabind::pure_out_value(_3))
        .def("getWeaponStruct", &World::getWeaponStruct, luabind::pure_out_value(_3))
        .def("getNaturalArmor", &World::getNaturalArmor, luabind::pure_out_value(_3))
        .def("getMonsterAttack", &World::getMonsterAttack, luabind::pure_out_value(_3))
        .def("changeQualityOfTopItem", &World::changeQualityOfItemAt)
        .def("changeQuality", &World::changeQuality)
        .def("itemInform", &World::itemInform)
        .def("changeItem", &World::changeItem)
        .def("isCharacterOnField", &World::isCharacterOnField)
        .def("getCharacterOnField", &World::getCharacterOnField)
        .def("putItemOnMap", &World::putItemOnMap)
        .def("putItemAlwaysOnMap", &World::putItemAlwaysOnMap)
        .def("getField", &World::GetField)
        .def("getTime", &World::getTime)
        .def("erase", &World::erase)
        .def("increase", &World::increase)
        .def("swap", &World::swap)
        .def("createItemFromId", world_createFromId)
        .def("createItemFromItem", &World::createFromItem)
        .def("createMonster", &World::createMonster)
        .def("gfx", &World::gfx)
        .def("makeSound", &World::makeSound)
        .def("getItemStats", &World::getItemStats)
        .def("getItemStatsFromId", &World::getItemStatsFromId)
        .def("sendWeatherToAllPlayers", &World::sendWeatherToAllPlayers)
        .def("setWeatherPart", &World::setWeatherPart)
        .def("setWeather", &World::setWeather)
        .def("isItemOnField", &World::isItemOnField)
        .def("getItemOnField", &World::getItemOnField)
        .def("changeTile", &World::changeTile)
        .def("sendMapUpdate", &World::sendMapUpdate)
        .def("getItemName", &World::getItemName)
        .def("createSavedArea", &World::createSavedArea)
        .def("sendMonitoringMessage", &World::sendMonitoringMessage)
        .def_readwrite("g_item", &World::g_item)
        .def_readwrite("weather", &World::weather),
        luabind::def("dofile", &dofile, luabind::raw(_1)),
        luabind::def("printerr", printerr),
        luabind::def("LuaAnd", LuaAnd),
        luabind::def("getCharForId",getCharForId),
        luabind::def("LuaOr", LuaOr),
        luabind::def("LuaAnd64", LuaAnd64),
        luabind::def("LuaOr64", LuaOr64),
        luabind::def("LuaLShift32",LuaLShift32),
        luabind::def("LuaRShift32",LuaRShift32),
        luabind::def("LuaLShift64",LuaLShift64),
        luabind::def("LuaLShift64",LuaLShift64),
        luabind::class_<CommonStruct>("CommonStruct")
        .def_readonly("id", &CommonStruct::id)
        .def_readonly("AgeingSpeed", &CommonStruct::AgeingSpeed)
        .def_readonly("Weight", &CommonStruct::Weight)
        .def_readonly("ObjectAfterRot", &CommonStruct::ObjectAfterRot)
        .def_readonly("rotsInInventory", &CommonStruct::rotsInInventory)
        .def_readonly("Brightness", &CommonStruct::Brightness)
        .def_readonly("Worth", &CommonStruct::Worth)
        .def_readonly("MaxStack", &CommonStruct::MaxStack)
        .def_readonly("BuyStack", &CommonStruct::BuyStack),
        luabind::class_<TilesStruct>("TilesStruct")
        .def_readonly("flags", &TilesStruct::flags)
        .def_readonly("German", &TilesStruct::German)
        .def_readonly("English", &TilesStruct::English)
        .def_readonly("walkingCost", &TilesStruct::walkingCost),
        luabind::class_<WeaponStruct>("WeaponStruct")
        .def(luabind::constructor<>())
        .def_readonly("Attack", &WeaponStruct::Attack)
        .def_readonly("Defence", &WeaponStruct::Defence)
        .def_readonly("Accuracy", &WeaponStruct::Accuracy)
        .def_readonly("Range", &WeaponStruct::Range)
        .def_readonly("WeaponType", &WeaponStruct::WeaponType)
        .def_readonly("AmmunitionType", &WeaponStruct::AmmunitionType)
        .def_readonly("ActionPoints",&WeaponStruct::ActionPoints)
        .def_readonly("MagicDisturbance",&WeaponStruct::MagicDisturbance)
        .def_readonly("PoisonStrength",&WeaponStruct::PoisonStrength),
        luabind::class_<ArmorStruct>("ArmorStruct")
        .def(luabind::constructor<>())
        .def_readonly("BodyParts",&ArmorStruct::BodyParts)
        .def_readonly("PunctureArmor",&ArmorStruct::PunctureArmor)
        .def_readonly("StrokeArmor",&ArmorStruct::StrokeArmor)
        .def_readonly("ThrustArmor",&ArmorStruct::ThrustArmor)
        .def_readonly("MagicDisturbance",&ArmorStruct::MagicDisturbance)
        .def_readonly("Absorb",&ArmorStruct::Absorb)
        .def_readonly("Stiffness",&ArmorStruct::Stiffness),
        luabind::class_<MonsterArmor>("NaturalArmor")
        .def(luabind::constructor<>())
        .def_readonly("strokeArmor",&MonsterArmor::strokeArmor)
        .def_readonly("thrustArmor",&MonsterArmor::thrustArmor)
        .def_readonly("punctureArmor",&MonsterArmor::punctureArmor),
        luabind::class_<AttackBoni>("AttackBoni")
        .def(luabind::constructor<>())
        .def_readonly("attackType",&AttackBoni::attackType)
        .def_readonly("attackValue",&AttackBoni::attackValue)
        .def_readonly("actionPointsLost",&AttackBoni::actionPointsLost),
        luabind::class_<Container>("Container")
        .def(luabind::constructor<TYPE_OF_ITEM_ID>())
        .def("getSlotCount", &Container::getSlotCount)
        .def("takeItemNr", &Container::TakeItemNr, luabind::pure_out_value(_3) + luabind::pure_out_value(_4))
        .def("viewItemNr", &Container::viewItemNr, luabind::pure_out_value(_3) + luabind::pure_out_value(_4))
        .def("changeQualityAt", &Container::changeQualityAt)
        .def("insertContainer", (bool(Container:: *)(Item,Container *))&Container::InsertContainer)
        .def("insertContainer", (bool(Container:: *)(Item,Container *,TYPE_OF_CONTAINERSLOTS))&Container::InsertContainer)
        .def("insertItem", (bool(Container:: *)(Item,bool))&Container::InsertItem)
        .def("insertItem", (bool(Container:: *)(Item,TYPE_OF_CONTAINERSLOTS))&Container::InsertItem)
        .def("insertItem", (bool(Container:: *)(Item))&Container::InsertItem)
        .def("countItem", container_count_item1)
        .def("countItem", container_count_item2)
        .def("eraseItem", container_erase_item1)
        .def("eraseItem", container_erase_item2)
        .def("increaseAtPos", &Container::increaseAtPos)
        .def("swapAtPos", &Container::swapAtPos)
        .def("weight", &Container::weight),
        luabind::class_<ScriptVariablesTable>("ScriptVariables")
        .def("find", &ScriptVariablesTable::find, luabind::pure_out_value(_3))
        .def("set", (void(ScriptVariablesTable:: *)(const std::string &, const std::string &))&ScriptVariablesTable::set)
        .def("set", (void(ScriptVariablesTable:: *)(const std::string &, int32_t))&ScriptVariablesTable::set)
        .def("remove", &ScriptVariablesTable::remove)
        .def("save" , &ScriptVariablesTable::save),
        luabind::class_<Random>("Random")
        .scope
        [
            luabind::def("uniform", (double( *)()) &Random::uniform),
            luabind::def("uniform", (int( *)(int, int)) &Random::uniform),
            luabind::def("normal", &Random::normal)
        ],
        luabind::def("isValidChar", &isValid<Character>),
        luabind::def("debug", &LuaScript::writeDebugMsg),
        luabind::def("log", log_lua),
        luabind::def("isTestserver", &LuaScript::isTestserver)
    ];

    luabind::object globals = luabind::globals(_luaState);
    globals["world"] = World::get();
    globals["ScriptVars"] = &Data::ScriptVariables;
}

