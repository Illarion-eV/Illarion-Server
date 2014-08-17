#   illarionserver - server for the game Illarion
#   Copyright 2011 Illarion e.V.
#
#   This file is part of illarionserver.
#
#   illarionserver is free software: you can redistribute it and/or modify
#   it under the terms of the GNU Affero General Public License as published by
#   the Free Software Foundation, either version 3 of the License, or
#   (at your option) any later version.
#
#   illarionserver is distributed in the hope that it will be useful,
#   but WITHOUT ANY WARRANTY; without even the implied warranty of
#   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
#   GNU Affero General Public License for more details.
#
#   You should have received a copy of the GNU Affero General Public License
#   along with illarionserver.  If not, see <http://www.gnu.org/licenses/>.


bin_PROGRAMS = illarion
noinst_LTLIBRARIES = libserver.la

AM_CXXFLAGS = -ggdb -pipe -Wall -Wformat-security -Wunused-result -Werror -Wno-deprecated -std=c++11 $(BOOST_CXXFLAGS) $(DEPS_CFLAGS) -fPIC
AM_CPPFLAGS = -D_THREAD_SAFE -D_REENTRANT -D_FORTIFY_SOURCE=2 $(subst -I,-isystem,$(BOOST_CPPFLAGS)) -I$(top_srcdir)/src -DPIC
AM_LDFLAGS = $(BOOST_THREAD_LDFLAGS) $(BOOST_FILESYSTEM_LDFLAGS) $(BOOST_SYSTEM_LDFLAGS) $(BOOST_REGEX_LDFLAGS)
LDADD = -L/usr/local/lib -lpthread -lssl $(DEPS_LIBS) libserver.la $(top_builddir)/src/script/libscriptbinding.la

LIBS = $(BOOST_THREAD_LIBS) $(BOOST_FILESYSTEM_LIBS) $(BOOST_SYSTEM_LIBS) $(BOOST_REGEX_LIBS)

SUBDIRS = script .

BUILT_SOURCES = version.hpp

libserver_la_SOURCES = \
main_help.cpp utility.cpp Logger.cpp Random.cpp Config.cpp Statistics.cpp a_star.cpp character_ptr.cpp \
\
data/Data.cpp data/QuestNodeTable.cpp data/QuestTable.cpp \
data/ArmorObjectTable.cpp data/CommonObjectTable.cpp data/ContainerObjectTable.cpp data/RaceAttributeTable.cpp \
data/MonsterAttackTable.cpp data/ScheduledScriptsTable.cpp data/LongTimeEffectTable.cpp data/NaturalArmorTable.cpp \
data/SpellTable.cpp data/ScriptVariablesTable.cpp data/NPCTable.cpp data/TriggerTable.cpp \
data/MonsterTable.cpp data/TilesModificatorTable.cpp data/TilesTable.cpp data/SkillTable.cpp data/WeaponObjectTable.cpp \
\
Map.cpp \
WorldMap.cpp Container.cpp NewClientView.cpp Item.cpp Showcase.cpp Field.cpp SpawnPoint.cpp \
\
World.cpp \
WorldIMPLAdmin.cpp WorldIMPLCharacterMoves.cpp WorldIMPLItemMoves.cpp WorldIMPLTalk.cpp \
WorldIMPLTools.cpp WorldIMPLScriptHelp.cpp WorldIMPLPlayer.cpp \
\
Timer.cpp MilTimer.cpp \
LongTimeEffect.cpp LongTimeAction.cpp LongTimeCharacterEffects.cpp \
\
Attribute.cpp Character.cpp CharacterContainer.cpp \
Player.cpp PlayerWorkoutCommands.cpp Monster.cpp NPC.cpp PlayerManager.cpp WaypointList.cpp \
\
dialog/Dialog.cpp dialog/InputDialog.cpp dialog/MessageDialog.cpp dialog/MerchantDialog.cpp \
dialog/SelectionDialog.cpp dialog/CraftingDialog.cpp \
\
db/ConnectionManager.cpp db/Connection.cpp db/Query.cpp \
db/SelectQuery.cpp db/InsertQuery.cpp db/UpdateQuery.cpp db/DeleteQuery.cpp \
db/QueryAssign.cpp db/QueryWhere.cpp db/QueryColumns.cpp db/QueryTables.cpp db/SchemaHelper.cpp \
\
netinterface/NetInterface.cpp InitialConnection.cpp netinterface/CommandFactory.cpp MonitoringClients.cpp \
netinterface/BasicCommand.cpp netinterface/BasicServerCommand.cpp netinterface/BasicClientCommand.cpp \
netinterface/protocol/ServerCommands.cpp netinterface/protocol/ClientCommands.cpp netinterface/ByteBuffer.cpp \
netinterface/protocol/BBIWIServerCommands.cpp netinterface/protocol/BBIWIClientCommands.cpp

libserver_la_LIBADD = $(top_builddir)/src/script/libscriptbinding.la

illarion_SOURCES = main.cpp

noinst_HEADERS = Showcase.hpp Container.hpp dialog/Dialog.hpp \
		 dialog/CraftingDialog.hpp dialog/MessageDialog.hpp \
		 dialog/SelectionDialog.hpp dialog/InputDialog.hpp \
		 dialog/MerchantDialog.hpp MilTimer.hpp a_star.hpp \
		 tuningConstants.hpp db/Result.hpp db/SchemaHelper.hpp \
		 db/QueryColumns.hpp db/DeleteQuery.hpp db/QueryWhere.hpp \
		 db/QueryAssign.hpp db/Query.hpp db/Connection.hpp \
		 db/InsertQuery.hpp db/ConnectionManager.hpp \
		 db/QueryTables.hpp db/UpdateQuery.hpp db/SelectQuery.hpp \
		 globals.hpp make_unique.hpp World.hpp ItemLookAt.hpp Item.hpp \
		 CharacterContainer.hpp SchedulerTaskClasses.hpp \
		 thread_safe_vector.hpp Random.hpp NPC.hpp Scheduler.hpp Scheduler.tcc \
		 PlayerManager.hpp Character.hpp \
		 Attribute.hpp InitialConnection.hpp Logger.hpp utility.hpp \
		 MonitoringClients.hpp Field.hpp \
		 data/Data.hpp data/Table.hpp data/StructTable.hpp \
		 data/ScriptStructTable.hpp data/QuestScriptStructTable.hpp \
		 data/SpellTable.hpp \
		 data/QuestNodeTable.hpp data/TriggerTable.hpp \
		 data/CommonObjectTable.hpp data/ContainerObjectTable.hpp \
		 data/QuestTable.hpp data/MonsterAttackTable.hpp \
		 data/SkillTable.hpp data/ArmorObjectTable.hpp \
		 data/MonsterTable.hpp data/ScriptVariablesTable.hpp \
		 data/NPCTable.hpp data/LongTimeEffectTable.hpp \
		 data/TilesModificatorTable.hpp data/RaceAttributeTable.hpp \
		 data/ScheduledScriptsTable.hpp data/TilesTable.hpp \
		 data/Table.hpp data/WeaponObjectTable.hpp \
		 data/NaturalArmorTable.hpp main_help.hpp TableStructs.hpp \
		 WorldMap.hpp Connection.hpp Map.hpp Language.hpp \
		 NewClientView.hpp \
		 netinterface/BasicCommand.hpp \
		 netinterface/BasicClientCommand.hpp \
		 netinterface/ByteBuffer.hpp netinterface/CommandFactory.hpp \
		 netinterface/BasicServerCommand.hpp \
		 netinterface/NetInterface.hpp \
		 netinterface/protocol/BBIWIClientCommands.hpp \
		 netinterface/protocol/BBIWIServerCommands.hpp \
		 netinterface/protocol/ClientCommands.hpp \
		 netinterface/protocol/ServerCommands.hpp WaypointList.hpp \
		 Config.hpp Statistics.hpp Timer.hpp constants.hpp types.hpp \
		 LongTimeCharacterEffects.hpp LongTimeAction.hpp character_ptr.hpp \
		 Player.hpp SpawnPoint.hpp LongTimeEffect.hpp Monster.hpp

version.hpp: $(HEADERS) $(SOURCES) $(top_builddir)/src/script/libscriptbinding.la
	echo "#define SERVER_VERSION \"`git describe --long`\"" > version.hpp

$(top_builddir)/src/script/libscriptbinding.la: script
