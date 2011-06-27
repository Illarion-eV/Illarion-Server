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
ArmorObjectTable.cpp \
NewClientView.cpp CommonObjectTable.cpp \
Container.cpp ContainerObjectTable.cpp \
ContainerStack.cpp \
Field.cpp \
IdCounter.cpp \
NaturalArmorTable.cpp MonsterAttackTable.cpp\
RaceSizeTable.cpp \
LongTimeEffect.cpp Map.cpp \
MapVector.cpp MilTimer.cpp \
ScriptVariablesTable.cpp \
NamesObjectTable.cpp NPCTable.cpp \
SpawnPoint.cpp MonsterTable.cpp \
TilesModificatorTable.cpp TilesTable.cpp \
TilesTable.hpp Timer.cpp Timer.hpp WeaponObjectTable.cpp \
World.cpp WorldIMPLAdmin.cpp \
Player.cpp PlayerWorkoutCommands.cpp \
WorldIMPLCharacterMoves.cpp WorldIMPLItemMoves.cpp WorldIMPLTalk.cpp \
WorldIMPLTools.cpp WorldIMPLScriptHelp.cpp InitialConnection.cpp \
Item.hpp Item.cpp Random.cpp \
Character.cpp Monster.cpp \
NPC.cpp main.cpp \
MonitoringClients.cpp \
utility.cpp main_help.cpp WorldIMPLPlayer.cpp WorldIMPLMonCommands.cpp \
playersave.cpp LongTimeAction.cpp LongTimeCharacterEffects.cpp \
Logger.cpp Scheduler.cpp LongTimeEffectTable.cpp \
\
db/ConnectionManager.cpp db/postgres.cpp \
\
script/LuaScript.cpp script/LuaNPCScript.cpp script/LuaItemScript.cpp \
script/LuaMonsterScript.cpp TriggerTable.cpp script/LuaMagicScript.cpp \
script/LuaTileScript.cpp script/LuaTriggerScript.cpp \
script/LuaWeaponScript.cpp script/LuaScheduledScript.cpp script/LuaLongTimeEffectScript.cpp script/LuaReloadScript.cpp \
script/LuaLoginScript.cpp script/LuaLogoutScript.cpp script/LuaDepotScript.cpp \
script/LuaLookAtPlayerScript.cpp script/LuaLearnScript.cpp \
ScheduledScriptsTable.cpp SpellTable.cpp \
\
netinterface/NetInterface.cpp \
netinterface/CommandFactory.cpp netinterface/BasicCommand.cpp \
netinterface/BasicServerCommand.cpp netinterface/BasicClientCommand.cpp \
netinterface/protocol/ServerCommands.cpp netinterface/ByteBuffer.cpp \
PlayerManager.cpp WaypointList.cpp \
Language.cpp


testserver_LDADD = -lpthread -lssl -lcrypto -lpqxx -lpq -llua5.1 -lboost_thread -lboost_system -lluabind
testserver_LDFLAGS = 
CXXFLAGS = -ggdb -pipe -Wall -Werror -Wno-deprecated -D_THREAD_SAFE -D_REENTRANT -DCDataConnect_DEBUG -DAdminCommands_DEBUG
INCLUDES = -I/usr/include/postgresql -I/usr/include/lua5.1
