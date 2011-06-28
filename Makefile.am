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
noinst_PROGRAMS = testserver

testserver_SOURCES = \
\
main.cpp main_help.cpp utility.cpp IdCounter.cpp Logger.cpp Random.cpp \
\
data/ArmorObjectTable.cpp data/CommonObjectTable.cpp data/ContainerObjectTable.cpp data/RaceSizeTable.cpp \
data/MonsterAttackTable.cpp data/ScheduledScriptsTable.cpp data/LongTimeEffectTable.cpp data/NaturalArmorTable.cpp \
data/SpellTable.cpp data/ScriptVariablesTable.cpp data/NamesObjectTable.cpp data/NPCTable.cpp data/TriggerTable.cpp \
data/MonsterTable.cpp data/TilesModificatorTable.cpp data/TilesTable.cpp data/WeaponObjectTable.cpp \
\
Map.cpp \
MapVector.cpp Container.cpp NewClientView.cpp Item.cpp ContainerStack.cpp Field.cpp SpawnPoint.cpp \
\
World.cpp \
WorldIMPLAdmin.cpp WorldIMPLCharacterMoves.cpp WorldIMPLItemMoves.cpp WorldIMPLTalk.cpp \
WorldIMPLTools.cpp WorldIMPLScriptHelp.cpp WorldIMPLPlayer.cpp WorldIMPLMonCommands.cpp \
\
Timer.cpp MilTimer.cpp \
LongTimeEffect.cpp LongTimeAction.cpp LongTimeCharacterEffects.cpp Scheduler.cpp \
\
Character.cpp \
Player.cpp PlayerWorkoutCommands.cpp Monster.cpp NPC.cpp PlayerManager.cpp playersave.cpp WaypointList.cpp Language.cpp \
\
db/ConnectionManager.cpp db/postgres.cpp \
\
script/LuaScript.cpp \
script/LuaNPCScript.cpp script/LuaItemScript.cpp script/LuaMonsterScript.cpp \
script/LuaMagicScript.cpp script/LuaTileScript.cpp script/LuaTriggerScript.cpp \
script/LuaWeaponScript.cpp script/LuaScheduledScript.cpp script/LuaLongTimeEffectScript.cpp \
script/LuaReloadScript.cpp script/LuaLoginScript.cpp script/LuaLogoutScript.cpp \
script/LuaDepotScript.cpp script/LuaLookAtPlayerScript.cpp script/LuaLearnScript.cpp \
\
netinterface/NetInterface.cpp InitialConnection.cpp netinterface/CommandFactory.cpp \
netinterface/BasicCommand.cpp netinterface/BasicServerCommand.cpp netinterface/BasicClientCommand.cpp \
netinterface/protocol/ServerCommands.cpp netinterface/ByteBuffer.cpp MonitoringClients.cpp

testserver_LDADD = -lpthread -lssl -lcrypto -lpqxx -lpq -llua5.1 -lboost_thread -lboost_system -lluabind
testserver_LDFLAGS = 
CXXFLAGS = -ggdb -pipe -Wall -Werror -Wno-deprecated -D_THREAD_SAFE -D_REENTRANT -DCDataConnect_DEBUG -DAdminCommands_DEBUG
INCLUDES = -I/usr/include/postgresql -I/usr/include/lua5.1
