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

testserver_SOURCES = fuse_ptr.hpp ArmorObjectTable.cpp \
ArmorObjectTable.hpp \
NewClientView.hpp NewClientView.cpp CommonObjectTable.cpp CommonObjectTable.hpp \
Container.cpp Container.hpp ContainerObjectTable.cpp ContainerObjectTable.hpp \
ContainerStack.cpp ContainerStack.hpp \
Field.cpp \
Field.hpp IdCounter.cpp \
NaturalArmorTable.hpp NaturalArmorTable.cpp MonsterAttackTable.hpp MonsterAttackTable.cpp\
RaceSizeTable.hpp RaceSizeTable.cpp IdCounter.hpp \
LongTimeEffect.cpp LongTimeEffect.hpp Map.cpp Map.hpp \
MapException.hpp MapVector.cpp MapVector.hpp MilTimer.cpp MilTimer.hpp \
ScriptVariablesTable.cpp ScriptVariablesTable.hpp \
NamesObjectTable.cpp NamesObjectTable.hpp NPCTable.hpp NPCTable.cpp \
SpawnPoint.cpp SpawnPoint.hpp MonsterTable.hpp MonsterTable.cpp \
TilesModificatorTable.cpp TilesModificatorTable.hpp TilesTable.cpp \
TilesTable.hpp Timer.cpp Timer.hpp WeaponObjectTable.cpp WeaponObjectTable.hpp \
World.cpp World.hpp WorldIMPLAdmin.cpp \
Player.cpp PlayerWorkoutCommands.cpp Player.hpp \
WorldIMPLCharacterMoves.cpp WorldIMPLItemMoves.cpp WorldIMPLTalk.cpp \
WorldIMPLTools.cpp WorldIMPLScriptHelp.cpp Connection.hpp InitialConnection.cpp InitialConnection.hpp \
Item.hpp Item.cpp Random.cpp Random.hpp TableStructs.hpp CharacterVector.hpp \
Character.cpp Character.hpp Monster.cpp Monster.hpp \
NPC.cpp NPC.hpp constants.hpp globals.hpp main.cpp main_help.hpp \
tuningConstants.h tvector.hpp MonitoringClients.cpp MonitoringClients.hpp\
tvector.hpp utility.cpp utility.hpp main_help.cpp WorldIMPLPlayer.cpp WorldIMPLMonCommands.cpp \
playersave.hpp playersave.cpp LongTimeAction.hpp LongTimeAction.cpp LongTimeCharacterEffects.hpp LongTimeCharacterEffects.cpp \
Logger.hpp Logger.cpp Scheduler.hpp Scheduler.cpp LongTimeEffectTable.hpp LongTimeEffectTable.cpp \
db/di.hpp db/ConnectionManager.hpp db/ConnectionManager.cpp db/postgres.hpp db/postgres.cpp \
script/LuaScript.hpp script/LuaScript.cpp script/LuaNPCScript.hpp script/LuaNPCScript.cpp script/LuaItemScript.hpp script/LuaItemScript.cpp \
script/LuaMonsterScript.hpp script/LuaMonsterScript.cpp TriggerTable.cpp  TriggerTable.hpp script/LuaMagicScript.cpp script/LuaMagicScript.hpp \
script/LuaTileScript.cpp script/LuaTileScript.hpp script/LuaTriggerScript.cpp script/LuaTriggerScript.hpp script/LuaWeaponScript.hpp \
script/LuaWeaponScript.cpp script/LuaScheduledScript.hpp script/LuaScheduledScript.cpp script/LuaLongTimeEffectScript.hpp script/LuaLongTimeEffectScript.cpp script/LuaReloadScript.cpp script/LuaReloadScript.hpp \
script/LuaLoginScript.cpp script/LuaLoginScript.hpp script/LuaLogoutScript.cpp script/LuaLogoutScript.hpp script/LuaDepotScript.hpp script/LuaDepotScript.cpp \
script/LuaLookAtPlayerScript.cpp script/LuaLookAtPlayerScript.hpp script/LuaLearnScript.hpp script/LuaLearnScript.cpp \
ScheduledScriptsTable.hpp ScheduledScriptsTable.cpp SpellTable.cpp SpellTable.hpp \
netinterface/NetInterface.hpp netinterface/NetInterface.cpp \
netinterface/CommandFactory.hpp netinterface/CommandFactory.cpp netinterface/BasicCommand.cpp netinterface/BasicCommand.hpp netinterface/BasicServerCommand.hpp \
netinterface/BasicServerCommand.cpp netinterface/BasicClientCommand.hpp netinterface/BasicClientCommand.cpp \
netinterface/protocol/ClientCommands.hpp netinterface/protocol/ServerCommands.cpp netinterface/protocol/ServerCommands.hpp netinterface/ByteBuffer.cpp netinterface/ByteBuffer.hpp \
PlayerManager.hpp PlayerManager.cpp WaypointList.hpp WaypointList.cpp\
Language.hpp Language.cpp


testserver_LDADD = -lpthread -lssl -lcrypto -lpqxx -lpq -llua5.1 -lboost_thread -lboost_system -lluabind
testserver_LDFLAGS = 
CXXFLAGS = -ggdb -pipe -Wall -Werror -Wno-deprecated -D_THREAD_SAFE -D_REENTRANT -DCDataConnect_DEBUG -DAdminCommands_DEBUG
INCLUDES = -I/usr/include/postgresql -I/usr/include/lua5.1
