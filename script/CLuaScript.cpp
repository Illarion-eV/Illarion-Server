#include "CLuaScript.hpp"

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
#include "CCharacter.hpp"
#include "CNPC.hpp"
#include "CField.hpp"
#include "Item.hpp"
#include "CPlayer.hpp"
#include "CWorld.hpp"
#include "UserMenuStruct.hpp"
#include "globals.h"
#include "TableStructs.hpp"
#include "CContainer.h"
#include "CLongTimeAction.hpp"
#include "CLongTimeEffect.hpp"
#include "CLongTimeCharacterEffects.hpp"
#include "CScriptVariablesTable.hpp"
#include "CLogger.hpp"
#include "CWaypointList.hpp"
#include "fuse_ptr.hpp"

extern CScriptVariablesTable * scriptVariables;

#ifdef GLOBAL_SCRIPTS
lua_State * CLuaScript::_luaState = 0;
bool CLuaScript::initialized = false;
#endif

CLuaScript::CLuaScript(std::string filename) throw(ScriptException) {
    _filename = filename;
    boost::split( vecPath, filename, boost::is_any_of(".") );
	
#ifdef GLOBAL_SCRIPTS
  if( !initialized )
  {
    initialized = true;
#endif
    // open lua and luabind
    _luaState = luaL_newstate();
    luabind::open(_luaState);
        
        // use another error function to surpress errors from
	// non-existant entry points and to display a backtrace
	luabind::set_pcall_callback(CLuaScript::add_backtrace);

	// next we initialise the objects/functions exports
	init_base_functions();

    // set package.path = configOptions["scriptdir"] + "?.lua"
    char path[100];
    strcpy( path, configOptions["scriptdir"].c_str() );
    strcat( path, "?.lua" );
    lua_pushstring( _luaState, "package" );
    lua_gettable( _luaState, LUA_GLOBALSINDEX );
    lua_pushstring( _luaState, "path" );
    lua_pushstring( _luaState, path );
    lua_settable( _luaState, -3 );
#ifdef GLOBAL_SCRIPTS
  }
#endif

	// last but not least, open the script file
    char luafile[200];
    strcpy( luafile, configOptions["scriptdir"].c_str() );
    std::replace( filename.begin(), filename.end(), '.', '/' );
    strcat( luafile, (filename + ".lua").c_str() );
	int err = luaL_dofile( _luaState, luafile );

	// if we get an error, throw an exception...
	if (err != 0)
    {
        std::string errstr(luafile);
		throw ScriptException("Could not load script file: " + errstr);
    }
}

CLuaScript::~CLuaScript() throw() {
}

void CLuaScript::shutdownLua()
{
    // shutdown lua
    if( initialized )
    {
        initialized = false;
        lua_close(_luaState);
        _luaState = 0;
    }
}

int CLuaScript::add_backtrace(lua_State* L)
{
   lua_Debug d;
   std::stringstream msg;

   if (lua_tostring(L, -1) != NULL){
       std::string err = lua_tostring(L, -1);
       lua_pop(L, 1);
       msg << err << std::endl;
   }

   int level = 0;
   while( lua_getstack(L, ++level, &d) ){
       lua_getinfo(L, "Sln", &d);
       msg << "#" << level << " called by: " << d.short_src << ":" << d.currentline;
       if (d.name != 0){
           msg << "(" << d.namewhat << " " << d.name << ")";
       }
       msg << std::endl;
   }
   if( level == 1 ){ // do not mind if an entry point is missing
       std::string empty = "";
       lua_pushstring(L, empty.c_str());
   } else
       lua_pushstring(L, msg.str().c_str());
   return 1;
}

void CLuaScript::writeErrorMsg()
{
    std::string err = lua_tostring(_luaState, -1);
    lua_pop(_luaState, 1);    
    if ( err.length() > 1 )
        CLogger::writeError( "scripts", err );
}

void CLuaScript::writeDebugMsg( std::string msg )
{
    lua_pushstring( _luaState, ("Debug Message: " + msg).c_str() );
    add_backtrace( _luaState );
    std::string backtrace = lua_tostring(_luaState, -1);
    lua_pop(_luaState, 1);
    CLogger::writeError( "scripts", backtrace );
}

luabind::object CLuaScript::call( std::string entrypoint ) throw( luabind::error )
{
    luabind::object obj = luabind::globals( _luaState );
    std::string currentpath = "";
    for( std::vector<std::string>::iterator it = vecPath.begin(); it != vecPath.end(); ++it )
    {
        obj = obj[*it];
        currentpath = currentpath + "." + *it;
        if( luabind::type( obj ) != LUA_TTABLE )
        {
            lua_pushstring(_luaState, (
                "Error while loading entrypoint '" + entrypoint + "': " + currentpath.erase(0,1) + " is not a table, but is used with an index!"
            ).c_str() );
            throw luabind::error(_luaState);
        }
    }
    luabind::object callee = obj[entrypoint];
    return callee;
}

bool CLuaScript::existsEntrypoint( std::string entrypoint )
{
    luabind::object obj = luabind::globals( _luaState );
    for( std::vector<std::string>::iterator it = vecPath.begin(); it != vecPath.end(); ++it )
    {   
        obj = obj[*it];
        if( luabind::type( obj ) != LUA_TTABLE ) return false;
    }
    obj = obj[entrypoint];
    if( luabind::type( obj ) != LUA_TFUNCTION ) return false;
    return true;
}

static int dofile(lua_State * L, const char * fname)
{
    char path[100];
    strcpy( path, configOptions["scriptdir"].c_str() );
    strcat( path, fname );
    std::cout << "loading file: " << path << std::endl;
    int n = lua_gettop(L);
    int status = luaL_loadfile(L, path);
    if (status != 0) lua_error(L);
    lua_call(L, 0, LUA_MULTRET);
    return lua_gettop(L) - n;
}

void printerr(std::string err) {
	std::cout << "script error: " << err << std::endl;
}

bool equapos(position pos1, position pos2) {
	return ( pos1.x == pos2.x && pos1.y == pos2.y && pos1.z == pos2.z );
}

unsigned int LuaAnd(unsigned int operand1, unsigned int operand2) {
    return ( operand1 & operand2 );
}

uint64_t LuaAnd64(uint64_t operand1, uint64_t operand2) {
    return ( operand1 & operand2 );
}

unsigned int LuaOr(unsigned int operand1, unsigned int operand2) {
    return ( operand1 | operand2 );
}

uint64_t LuaOr64(uint64_t operand1, uint64_t operand2) {
    return ( operand1 | operand2 );
}

uint32_t LuaLShift32(uint32_t value, unsigned char bits)
{
    return value << bits;
}

uint32_t LuaRShift32(uint32_t value, unsigned char bits)
{
    return value >> bits;
}

uint64_t LuaRShift64(uint64_t value, unsigned char bits)
{
    return value >> bits;
}

uint64_t LuaLShift64(uint64_t value, unsigned char bits)
{
    return value << bits;
}

CCharacter * getCharForId(TYPE_OF_CHARACTER_ID id)
{
    CCharacter * ret = NULL;
    if ( id < MONSTER_BASE )
    {
        //player
        ret = CWorld::get()->Players.findID( id );
    }
    else if ( id >= MONSTER_BASE && id < NPC_BASE )
    {
        //monster
        ret = CWorld::get()->Monsters.findID( id );
    }
    else
    {
        ret = CWorld::get()->Npc.findID( id );
    }
    return ret;
}

/**
* initializes the base functions of the script
* opens mathlib, strlib, baselib,tablib,iolib
*/ 
void CLuaScript::init_base_functions() {
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

    luabind::module(_luaState)
	[
        luabind::class_<CLongTimeAction>("Action")
        .enum_("state")
        [
            luabind::value("none",0), /**< no action */
            luabind::value("abort",1), /**< action aborted */
            luabind::value("success",2) /**< action successed */
        ],
        luabind::class_<position>("position")
		.def(luabind::constructor<>())
		.def(luabind::constructor<int, int, int>())
		.def(luabind::self == luabind::const_self )
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
        luabind::class_<CCharacter>("CCharacter")
        .property("death_consequences", &CCharacter::getDeathConsequences, &CCharacter::setDeathConsequences)
        .def("idleTime", &CCharacter::idleTime)
        .def("sendBook", &CCharacter::sendBook)
        .def("updateAppearance", &CCharacter::forceUpdateAppearanceForAll)
        .def("performAnimation", &CCharacter::performAnimation)
        .def("moveDepotContentFrom", &CCharacter::moveDepotContentFrom)
        .def("tempChangeAttrib", &CCharacter::tempChangeAttrib)
		.def("alterSpokenMessage", &CCharacter::alterSpokenMessage)
        .def("actionRunning", &CCharacter::actionRunning)
		.def("changeQualityAt", &CCharacter::changeQualityAt)
		.def("changeQualityItem", &CCharacter::changeQualityItem)
		.def("isAdmin", &CCharacter::isAdmin)
		.def("talk", &CCharacter::talk)
        .def("sendCharDescription", &CCharacter::sendCharDescription )
        .def("talkLanguage", &CCharacter::talkLanguage)
        .def("startAction", &CCharacter::startAction)
        .def("abortAction", &CCharacter::abortAction)
        .def("successAction", &CCharacter::successAction)
        .def("disturbAction", &CCharacter::actionDisturbed)
        .def("changeSource", (void(CCharacter::*)(CCharacter*))&CCharacter::changeSource)
        .def("changeSource", (void(CCharacter::*)(ScriptItem))&CCharacter::changeSource)
        .def("changeSource", (void(CCharacter::*)(position))&CCharacter::changeSource)
        .def("changeSource", (void(CCharacter::*)(void))&CCharacter::changeSource)
        .def("changeTarget", (void(CCharacter::*)(CCharacter*))&CCharacter::changeTarget)
        .def("changeTarget", (void(CCharacter::*)(ScriptItem))&CCharacter::changeTarget)
        .def("changeTarget", (void(CCharacter::*)(position))&CCharacter::changeTarget)
        .def("changeTarget", (void(CCharacter::*)(void))&CCharacter::changeTarget)        
		.def("inform", &CCharacter::inform)
		.def("introduce", &CCharacter::introducePerson)
		.def("move", &CCharacter::move)
		.def("getNextStepDir", &CCharacter::getNextStepDir, luabind::pure_out_value(_4))
		.def("getStepList",&CCharacter::getLuaStepList)
		.def("get_race", &CCharacter::get_race)
		.def("get_face_to", &CCharacter::get_face_to)
		.def("get_type", &CCharacter::get_character)
		.def("createItem", &CCharacter::createItem)
		.def("increasePoisonValue", &CCharacter::increasePoisonValue)
		.def("getPoisonValue", &CCharacter::getPoisonValue)
		.def("setPoisonValue", &CCharacter::setPoisonValue)
		.def("getMentalCapacity", &CCharacter::getMentalCapacity)
		.def("setMentalCapacity", &CCharacter::setMentalCapacity)
		.def("increaseMentalCapacity", &CCharacter::increaseMentalCapacity)
		.def("setClippingActive", &CCharacter::setClippingActive)
		.def("getClippingActive", &CCharacter::getClippingActive)
		.def("countItem", &CCharacter::countItem)
		.def("countItemAt", (int(CCharacter::*)(std::string, TYPE_OF_ITEM_ID, uint32_t))&CCharacter::countItemAt)
		.def("countItemAt", (int(CCharacter::*)(std::string, TYPE_OF_ITEM_ID))&CCharacter::countItemAt)
        .def("eraseItem", (int(CCharacter::*)(TYPE_OF_ITEM_ID, int, uint32_t))&CCharacter::eraseItem)
		.def("eraseItem", (int(CCharacter::*)(TYPE_OF_ITEM_ID, int))&CCharacter::eraseItem)
		.def("increaseAtPos", &CCharacter::increaseAtPos)
		.def("swapAtPos", &CCharacter::swapAtPos)
		.def("createAtPos", &CCharacter::createAtPos)
		.def("getItemAt", &CCharacter::GetItemAt)
		.def("getSkill", &CCharacter::getSkill)
        .def("getMinorSkill", &CCharacter::getMinorSkill)
		.def("increaseAttrib", &CCharacter::increaseAttrib)
		.def("setAttrib", &CCharacter::setAttrib)
		.def("increaseSkill", &CCharacter::increaseSkill)
        .def("increaseMinorSkill", &CCharacter::increaseMinorSkill)
        .def("setSkill", &CCharacter::setSkill)
		.def("learn", &CCharacter::learn)
        .def("getSkillValue",&CCharacter::getSkillValue)
		.def("teachMagic", &CCharacter::teachMagic)
		.def("isInRange", &CCharacter::isInRange)
		.def("isInRangeToPosition", &CCharacter::isInRangeToField)
		.def("distanceMetric", &CCharacter::distanceMetric)
		.def("distanceMetricToPosition", &CCharacter::distanceMetricToPosition)
		.def("getMagicType", &CCharacter::get_magic_type)
		.def("setMagicType", &CCharacter::set_magic_type)
		.def("getMagicFlags", &CCharacter::get_magic_flags)
		.def("warp", &CCharacter::Warp)
		.def("forceWarp", &CCharacter::forceWarp)
		.def("sendMenu", &CCharacter::startPlayerMenu)
		.def("startMusic", &CCharacter::startMusic)
        .def("defaultMusic", &CCharacter::defaultMusic)
		.def("callAttackScript", &CCharacter::callAttackScript)
		.def("callDefendScript", &CCharacter::callDefendScript)
		.def("getItemList", &CCharacter::getItemList)
		.def_readonly("lastSpokenText", &CCharacter::lastSpokenText)
		.def("getPlayerLanguage", &CCharacter::getPlayerLanguage)
		.def("getBackPack", &CCharacter::GetBackPack)
        .def("getDepot", &CCharacter::GetDepot)
        .def("setQuestProgress", &CCharacter::setQuestProgress)
        .def("getQuestProgress", &CCharacter::getQuestProgress)
        .def("LTIncreaseHP", &CCharacter::LTIncreaseHP)
        .def("LTIncreaseMana", &CCharacter::LTIncreaseMana)
		.def("getOnRoute",&CCharacter::getOnRoute)
		.def("setOnRoute",&CCharacter::setOnRoute)
        .def("get_mon_type", &CCharacter::getType)
        .def_readonly("effects", &CCharacter::effects)
		.def_readonly("waypoints", &CCharacter::waypoints)
		.def_readonly("pos", &CCharacter::pos)
		.def_readonly("name", &CCharacter::name)
		.def_readonly("id", &CCharacter::id)
		.def_readwrite("activeLanguage", &CCharacter::activeLanguage)
		.def_readwrite("movepoints", &CCharacter::actionPoints)
        .def_readwrite("isinvisible", &CCharacter::isinvisible)
		.def_readonly("attackmode", &CCharacter::attackmode)
		//.def_readonly("isTarget", &CCharacter::isTarget)
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
			luabind::value("female",1),
			luabind::value("neuter",2)
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
		],
        luabind::class_<CCharacter::skillvalue>("skillvalue")
        .def(luabind::constructor<>())
        .def_readwrite("type",&CCharacter::skillvalue::type)
        .def_readwrite("major",&CCharacter::skillvalue::major)
        .def_readwrite("minor",&CCharacter::skillvalue::minor)
        .def_readwrite("firsttry",&CCharacter::skillvalue::firsttry),
		luabind::class_<CNPC, CCharacter>("CNPC"),
        luabind::class_<CMonster, CCharacter>("CMonster"),
		luabind::class_<CWaypointList>("CWaypointList")
		.def("addFromList", &CWaypointList::addFromList)
		.def("getWaypoints",&CWaypointList::getWaypoints)
		.def("addWaypoint",&CWaypointList::addWaypoint)
		.def("clear",&CWaypointList::clear),
		luabind::class_<CLongTimeCharacterEffects>("CLongTimeCharacterEffects")
        .def("addEffect",&CLongTimeCharacterEffects::addEffect, luabind::adopt(_2))
        .def("removeEffect", (bool(CLongTimeCharacterEffects::*)(uint16_t))&CLongTimeCharacterEffects::removeEffect)
        .def("removeEffect", (bool(CLongTimeCharacterEffects::*)(std::string))&CLongTimeCharacterEffects::removeEffect)
        .def("removeEffect", (bool(CLongTimeCharacterEffects::*)(CLongTimeEffect*))&CLongTimeCharacterEffects::removeEffect)
        .def("find", (bool(CLongTimeCharacterEffects::*)(uint16_t,CLongTimeEffect*&))&CLongTimeCharacterEffects::find,luabind::pure_out_value(_3))
        .def("find", (bool(CLongTimeCharacterEffects::*)(std::string,CLongTimeEffect*&))&CLongTimeCharacterEffects::find,luabind::pure_out_value(_3)),
		luabind::class_<CField>("Field")
		.def("tile", &CField::getTileId)
		//.def("changeQualityOfTopItem", &CField::changeQualityOfTopItem)
        .def("getStackItem", &CField::getStackItem)
		.def("createItemAlways", &CField::PutTopItem)
		.def("createItem", &CField::addTopItem)
		.def("createItemGround", &CField::PutGroundItem)
		.def("takeTopItem", &CField::TakeTopItem, luabind::pure_out_value(_2) )
		.def("increaseTopItem", &CField::increaseTopItem, luabind::pure_out_value(_3) )
		.def("swapTopItem", &CField::swapTopItem)
		.def("viewTopItem", &CField::ViewTopItem)
        .def("countItems", &CField::NumberOfItems)
		.def("deleteAllItems", &CField::DeleteAllItems),
		luabind::class_<UserMenuStruct>("MenuStruct")
		.def(luabind::constructor<>())
		.def("addItem", &UserMenuStruct::AddItem),
		luabind::class_<Item>("Item")
		.def(luabind::constructor<>())
		.def(luabind::constructor<TYPE_OF_ITEM_ID,unsigned char, unsigned char>())
		.def_readwrite("id", &Item::id)
		.def_readwrite("wear", &Item::wear)
		.def_readwrite("number", &Item::number)
		.def_readwrite("quality", &Item::quality)
        //.def_readwrite("data", &Item::data),
        .def("setValue", &Item::setValue)
        .def("getValue", &Item::getValue)
        .property("data", &Item::getData, &Item::setData),
		luabind::class_<ScriptItem,Item>("scriptItem") //Spezielle Itemklasse fr Scripte die auch die Position des Items und den eigentmer kennt.
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
        luabind::class_<CLongTimeEffect>("CLongTimeEffect")
        .def(luabind::constructor<uint16_t,uint32_t>() )
        .def(luabind::constructor<std::string,uint32_t>() )
        .def("addValue",&CLongTimeEffect::addValue)
        .def("removeValue",&CLongTimeEffect::removeValue)
        .def("findValue",&CLongTimeEffect::findValue, luabind::pure_out_value(_3) )
        .def_readonly("effectId",&CLongTimeEffect::_effectId)
        .def_readonly("effectName",&CLongTimeEffect::_effectName)
        .def_readwrite("nextCalled",&CLongTimeEffect::_nextCalled)
        .def_readonly("numberCalled",&CLongTimeEffect::_numberCalled)
        .def_readonly("lastCalled",&CLongTimeEffect::_lastCalled),
        luabind::class_<LongTimeEffectStruct>("LongTimeEffectStruct")
        .def(luabind::constructor<>() )
        .def_readonly("effectId",&LongTimeEffectStruct::effectid)
        .def_readonly("effectName", &LongTimeEffectStruct::effectname)
        .def_readonly("scriptName", &LongTimeEffectStruct::scriptname),
		luabind::class_<CPlayer, CCharacter>("CPlayer")
        .enum_("player_language")
        [
            luabind::value("german",0),
            luabind::value("english",1)
        ],
		luabind::class_<CWorld>("CWorld")
        .def("LoS", &CWorld::LuaLoS)
        .def("deleteNPC", &CWorld::deleteNPC)
        .def("createDynamicNPC", &CWorld::createDynamicNPC )
        .def("getPlayersOnline", &CWorld::getPlayersOnline )
        .def("getNPCS", &CWorld::getNPCS )
        .def("getCharactersInRangeOf", &CWorld::getCharactersInRangeOf )
        .def("getPlayersInRangeOf", &CWorld::getPlayersInRangeOf )
        .def("getMonstersInRangeOf", &CWorld::getMonstersInRangeOf )
        .def("getNPCSInRangeOf", &CWorld::getNPCSInRangeOf )
		.def("getArmorStruct", &CWorld::getArmorStruct, luabind::pure_out_value(_3) )
		.def("getWeaponStruct", &CWorld::getWeaponStruct, luabind::pure_out_value(_3))
		.def("getNaturalArmor", &CWorld::getNaturalArmor, luabind::pure_out_value(_3) )
        .def("getMonsterAttack", &CWorld::getMonsterAttack, luabind::pure_out_value(_3) )
		.def("changeQualityOfTopItem", &CWorld::changeQualityOfItemAt)
		.def("changeQuality", &CWorld::changeQuality)
		.def("itemInform", &CWorld::ItemInform)
		.def("changeItem", &CWorld::changeItem)
		.def("isCharacterOnField", &CWorld::isCharacterOnField)
		.def("getCharacterOnField", &CWorld::getCharacterOnField)
		.def("putItemOnMap", &CWorld::putItemOnMap)
		.def("putItemAlwaysOnMap", &CWorld::putItemAlwaysOnMap)
		.def("getField", &CWorld::GetField)
		.def("getTime", &CWorld::getTime)
		.def("erase", &CWorld::erase)
		.def("increase", &CWorld::increase)
		.def("swap", &CWorld::swap)
		.def("createItemFromId", &CWorld::createFromId)
		.def("createItemFromItem", &CWorld::createFromItem)
		.def("createMonster", &CWorld::createMonster)
		.def("gfx", &CWorld::gfx)
        .def("makeSound", &CWorld::makeSound)
		.def("getItemStats", &CWorld::getItemStats)
		.def("getItemStatsFromId", &CWorld::getItemStatsFromId)
        .def("sendWeatherToAllPlayers", &CWorld::sendWeatherToAllPlayers)
        .def("setWeatherPart", &CWorld::setWeatherPart)
        .def("setWeather", &CWorld::setWeather)
		.def("isItemOnField", &CWorld::isItemOnField)
		.def("getItemOnField", &CWorld::getItemOnField)
		.def("changeTile", &CWorld::changeTile)
		.def("sendMapUpdate", &CWorld::sendMapUpdate)
		.def("getItemName", &CWorld::getItemName)
		.def("createSavedArea", &CWorld::createSavedArea)
        .def("sendMonitoringMessage", &CWorld::sendMonitoringMessage)
		.def_readwrite("g_item", &CWorld::g_item)
        .def_readwrite("weather", &CWorld::weather),
	        luabind::def("dofile", &dofile, luabind::raw(_1)),
		luabind::def("printerr", printerr),
		luabind::def("equapos", equapos),
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
		.def_readonly("id", &CommonStruct::id )
		.def_readonly("AgeingSpeed", &CommonStruct::AgeingSpeed )
		.def_readonly("Weight", &CommonStruct::Weight )
		.def_readonly("Volume", &CommonStruct::Volume )
		.def_readonly("ObjectAfterRot", &CommonStruct::ObjectAfterRot )
        .def_readonly("Worth", &CommonStruct::Worth ),
		luabind::class_<TilesStruct>("TilesStruct")
		.def_readonly("flags", &TilesStruct::flags )
		.def_readonly("German", &TilesStruct::German )
		.def_readonly("English", &TilesStruct::English )
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
        luabind::class_<CContainer>("Container")
        .def(luabind::constructor<uint16_t>() )
        .def("takeItemNr", &CContainer::TakeItemNr, luabind::pure_out_value(_3) + luabind::pure_out_value(_4) )
        .def("viewItemNr", &CContainer::viewItemNr, luabind::pure_out_value(_3) + luabind::pure_out_value(_4) )
        .def("changeQualityAt", &CContainer::changeQualityAt)
        .def("changeQuality", &CContainer::changeQuality)
        .def("insertContainer", &CContainer::InsertContainer)
        .def("insertItem", (bool(CContainer::*)(Item,bool))&CContainer::InsertItem) 
        .def("insertItem", (bool(CContainer::*)(Item,unsigned char))&CContainer::InsertItem)
        .def("insertItem", (bool(CContainer::*)(Item))&CContainer::InsertItem)
        .def("countItem",(int(CContainer::*)(TYPE_OF_ITEM_ID,uint32_t))&CContainer::countItem)
        .def("countItem",(int(CContainer::*)(TYPE_OF_ITEM_ID))&CContainer::countItem)            
        .def("eraseItem", (int(CContainer::*)(TYPE_OF_ITEM_ID, int, uint32_t))&CContainer::eraseItem)
        .def("eraseItem", (int(CContainer::*)(TYPE_OF_ITEM_ID, int))&CContainer::eraseItem)
        .def("increaseAtPos", &CContainer::increaseAtPos)
        .def("swapAtPos", &CContainer::swapAtPos)
        .def("weight", &CContainer::weight)
        .def("Volume", &CContainer::Volume),
        luabind::class_<CScriptVariablesTable>("ScriptVariables")
        .def("find", &CScriptVariablesTable::find, luabind::pure_out_value(_3) )
        .def("set", &CScriptVariablesTable::set)
        .def("remove", &CScriptVariablesTable::remove )
        .def("save" , &CScriptVariablesTable::save ),
        luabind::def("isValidChar", &isValid<CCharacter> ),
        luabind::def("debug", &CLuaScript::writeDebugMsg )
	];

	luabind::object globals = luabind::globals(_luaState);
    globals["world"] = CWorld::get();
    globals["ScriptVars"] = scriptVariables;
}
