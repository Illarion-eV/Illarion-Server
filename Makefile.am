#   illarionserver - server for the game Illarion
#   Copyright 2011 Illarion e.V.
#
#   This file is part of illarionserver.
#
#   illarionserver is free software: you can redistribute it and/or modify
#   it under the terms of the GNU General Public License as published by
#   the Free Software Foundation, either version 3 of the License, or
#   (at your option) any later version.
#
#   illarionserver is distributed in the hope that it will be useful,
#   but WITHOUT ANY WARRANTY; without even the implied warranty of
#   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
#   GNU General Public License for more details.
#
#   You should have received a copy of the GNU General Public License
#   along with illarionserver.  If not, see <http://www.gnu.org/licenses/>.


AUTOMAKE_OPTIONS = foreign 
ACLOCAL_AMFLAGS = -Im4
noinst_PROGRAMS = testserver

EXTRA_DIST = .astylerc

testserver_SOURCES = \
\
main.cpp main_help.cpp utility.cpp Logger.cpp Random.cpp Config.cpp \
\
data/Data.cpp data/QuestNodeTable.cpp data/QuestTable.cpp \
data/ArmorObjectTable.cpp data/CommonObjectTable.cpp data/ContainerObjectTable.cpp data/RaceSizeTable.cpp \
data/MonsterAttackTable.cpp data/ScheduledScriptsTable.cpp data/LongTimeEffectTable.cpp data/NaturalArmorTable.cpp \
data/SpellTable.cpp data/ScriptVariablesTable.cpp data/NamesObjectTable.cpp data/NPCTable.cpp data/TriggerTable.cpp \
data/MonsterTable.cpp data/TilesModificatorTable.cpp data/TilesTable.cpp data/SkillTable.cpp data/WeaponObjectTable.cpp \
\
Map.cpp \
WorldMap.cpp Container.cpp NewClientView.cpp Item.cpp Showcase.cpp Field.cpp SpawnPoint.cpp \
\
World.cpp \
WorldIMPLAdmin.cpp WorldIMPLCharacterMoves.cpp WorldIMPLItemMoves.cpp WorldIMPLTalk.cpp \
WorldIMPLTools.cpp WorldIMPLScriptHelp.cpp WorldIMPLPlayer.cpp WorldIMPLMonCommands.cpp \
\
Timer.cpp MilTimer.cpp \
LongTimeEffect.cpp LongTimeAction.cpp LongTimeCharacterEffects.cpp Scheduler.cpp \
\
Attribute.cpp Character.cpp \
Player.cpp PlayerWorkoutCommands.cpp Monster.cpp NPC.cpp PlayerManager.cpp playersave.cpp WaypointList.cpp \
\
dialog/Dialog.cpp dialog/InputDialog.cpp dialog/MessageDialog.cpp dialog/MerchantDialog.cpp \
dialog/SelectionDialog.cpp dialog/CraftingDialog.cpp \
\
db/ConnectionManager.cpp db/Connection.cpp db/Query.cpp \
db/SelectQuery.cpp db/InsertQuery.cpp db/UpdateQuery.cpp db/DeleteQuery.cpp \
db/QueryAssign.cpp db/QueryWhere.cpp db/QueryColumns.cpp db/QueryTables.cpp db/SchemaHelper.cpp \
\
script/LuaScript.cpp script/forwarder.cpp \
script/LuaNPCScript.cpp script/LuaItemScript.cpp script/LuaMonsterScript.cpp \
script/LuaMagicScript.cpp script/LuaTileScript.cpp script/LuaTriggerScript.cpp \
script/LuaWeaponScript.cpp script/LuaScheduledScript.cpp script/LuaLongTimeEffectScript.cpp \
script/LuaReloadScript.cpp script/LuaLoginScript.cpp script/LuaLogoutScript.cpp \
script/LuaDepotScript.cpp script/LuaLookAtPlayerScript.cpp script/LuaLearnScript.cpp \
script/LuaPlayerDeathScript.cpp script/LuaLookAtItemScript.cpp script/LuaQuestScript.cpp \
\
netinterface/NetInterface.cpp InitialConnection.cpp netinterface/CommandFactory.cpp \
netinterface/BasicCommand.cpp netinterface/BasicServerCommand.cpp netinterface/BasicClientCommand.cpp \
netinterface/protocol/ServerCommands.cpp netinterface/ByteBuffer.cpp MonitoringClients.cpp

AM_CPPFLAGS = $(BOOST_CPPFLAGS)

testserver_LDADD = -L/usr/local/lib -lpthread -lssl -lcrypto $(DEPS_LIBS) -lcrypt
testserver_LDFLAGS = $(BOOST_THREAD_LDFLAGS) $(BOOST_FILESYSTEM_LDFLAGS) $(BOOST_SYSTEM_LDFLAGS)
testserver_CXXFLAGS = -ggdb -pipe -Wall -Werror -Wno-deprecated -std=c++11 -D_THREAD_SAFE -D_REENTRANT -DTESTSERVER -DCDataConnect_DEBUG -DAdminCommands_DEBUG $(BOOST_CXXFLAGS) $(DEPS_CFLAGS)
LIBS = $(BOOST_THREAD_LIBS) $(BOOST_FILESYSTEM_LIBS) $(BOOST_SYSTEM_LIBS)

style: .astylerc
	astyle -n --options=$(top_srcdir)/.astylerc "${top_srcdir}/*.cpp" "${top_srcdir}/*.hpp"

noinst_HEADERS = Showcase.hpp Container.hpp dialog/Dialog.hpp \
		 dialog/CraftingDialog.hpp dialog/MessageDialog.hpp \
		 dialog/SelectionDialog.hpp dialog/InputDialog.hpp \
		 dialog/MerchantDialog.hpp MilTimer.hpp a_star.hpp \
		 tuningConstants.hpp db/Result.hpp db/SchemaHelper.hpp \
		 db/QueryColumns.hpp db/DeleteQuery.hpp db/QueryWhere.hpp \
		 db/QueryAssign.hpp db/Query.hpp db/Connection.hpp \
		 db/InsertQuery.hpp db/ConnectionManager.hpp \
		 db/QueryTables.hpp db/UpdateQuery.hpp db/SelectQuery.hpp \
		 globals.hpp make_unique.hpp World.hpp Item.hpp \
		 TccharacterVector.hpp SchedulerTaskClasses.hpp \
		 script/LuaLookAtItemScript.hpp \
		 script/LuaLookAtPlayerScript.hpp script/LuaTriggerScript.hpp \
		 script/LuaLongTimeEffectScript.hpp script/LuaLearnScript.hpp \
		 script/LuaNPCScript.hpp script/LuaWeaponScript.hpp \
		 script/LuaMagicScript.hpp script/LuaScheduledScript.hpp \
		 script/LuaLoginScript.hpp script/LuaLogoutScript.hpp \
		 script/LuaReloadScript.hpp script/LuaQuestScript.hpp \
		 script/LuaDepotScript.hpp script/LuaMonsterScript.hpp \
		 script/LuaPlayerDeathScript.hpp script/LuaScript.hpp \
		 script/LuaTileScript.hpp script/LuaItemScript.hpp \
		 tpvector.hpp Random.hpp tvector.hpp NPC.hpp Scheduler.hpp \
		 MapException.hpp PlayerManager.hpp Character.hpp \
		 Attribute.hpp InitialConnection.hpp Logger.hpp utility.hpp \
		 MonitoringClients.hpp playersave.hpp Field.hpp \
		 data/Data.hpp data/Table.hpp data/StructTable.hpp \
         data/ScriptStructTable.hpp data/QuestScriptStructTable.hpp \
         data/NamesObjectTable.hpp data/SpellTable.hpp \
		 data/QuestNodeTable.hpp data/TriggerTable.hpp \
		 data/CommonObjectTable.hpp data/ContainerObjectTable.hpp \
		 data/QuestTable.hpp data/MonsterAttackTable.hpp \
		 data/SkillTable.hpp data/ArmorObjectTable.hpp \
		 data/MonsterTable.hpp data/ScriptVariablesTable.hpp \
		 data/NPCTable.hpp data/LongTimeEffectTable.hpp \
		 data/TilesModificatorTable.hpp data/RaceSizeTable.hpp \
		 data/ScheduledScriptsTable.hpp data/TilesTable.hpp \
		 data/Table.hpp data/WeaponObjectTable.hpp \
		 data/NaturalArmorTable.hpp main_help.hpp TableStructs.hpp \
		 WorldMap.hpp Connection.hpp Map.hpp Language.hpp \
		 NewClientView.hpp netinterface/Receiver.hpp \
		 netinterface/BasicCommand.hpp \
		 netinterface/BasicClientCommand.hpp netinterface/Sender.hpp \
		 netinterface/ByteBuffer.hpp netinterface/CommandFactory.hpp \
		 netinterface/BasicServerCommand.hpp \
		 netinterface/NetInterface.hpp \
		 netinterface/protocol/BBIWIClientCommands.hpp \
		 netinterface/protocol/BBIWIServerCommands.hpp \
		 netinterface/protocol/ClientCommands.hpp \
		 netinterface/protocol/ServerCommands.hpp WaypointList.hpp \
		 Config.hpp Timer.hpp constants.hpp types.hpp \
		 LongTimeCharacterEffects.hpp LongTimeAction.hpp fuse_ptr.hpp \
		 Player.hpp SpawnPoint.hpp LongTimeEffect.hpp Monster.hpp
