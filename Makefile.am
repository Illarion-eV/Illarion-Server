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

testserver_SOURCES = fuse_ptr.hpp CArmorObjectTable.cpp \
CArmorObjectTable.h \
CNewClientView.hpp CNewClientView.cpp CCommonObjectTable.cpp CCommonObjectTable.h \
CContainer.cpp CContainer.h CContainerObjectTable.cpp CContainerObjectTable.h \
CContainerStack.cpp CContainerStack.h \
CField.cpp \
CField.hpp CIdCounter.cpp \
CNaturalArmorTable.hpp CNaturalArmorTable.cpp CMonsterAttackTable.hpp CMonsterAttackTable.cpp\
CRaceSizeTable.hpp CRaceSizeTable.cpp CIdCounter.h \
CLongTimeEffect.cpp CLongTimeEffect.hpp CMap.cpp CMap.hpp \
CMapException.hpp CMapVector.cpp CMapVector.hpp CMilTimer.cpp CMilTimer.hpp \
CNamesObjectTable.cpp CNamesObjectTable.h CNPCTable.h CNPCTable.cpp \
CSpawnPoint.cpp CSpawnPoint.hpp CMonsterTable.hpp CMonsterTable.cpp \
CTilesModificatorTable.cpp CTilesModificatorTable.h CTilesTable.cpp \
CTilesTable.h CTimer.cpp CTimer.h CWeaponObjectTable.cpp CWeaponObjectTable.h \
CScriptVariablesTable.hpp CScriptVariablesTable.cpp \
CWorld.cpp CWorld.hpp CWorldIMPLAdmin.cpp \
CPlayer.cpp CPlayerWorkoutCommands.cpp CPlayer.hpp \
CWorldIMPLCharacterMoves.cpp CWorldIMPLItemMoves.cpp CWorldIMPLTalk.cpp \
CWorldIMPLTools.cpp CWorldIMPLScriptHelp.cpp Connection.h InitialConnection.cpp InitialConnection.h \
Item.hpp Item.cpp Random.cpp Random.h TableStructs.hpp TccharacterVector.hpp \
CCharacter.cpp CCharacter.hpp CMonster.cpp CMonster.hpp \
CNPC.cpp CNPC.hpp constants.h globals.h main.cpp main_help.h \
tuningConstants.h tvector.hh CMonitoringClients.cpp CMonitoringClients.hpp\
tvector.hpp utility.cpp utility.hpp main_help.cpp CWorldIMPLPlayer.cpp CWorldIMPLMonCommands.cpp \
playersave.hh playersave.cpp CLongTimeAction.hpp CLongTimeAction.cpp CLongTimeCharacterEffects.hpp CLongTimeCharacterEffects.cpp \
CLogger.hpp CLogger.cpp CScheduler.h CScheduler.cpp CLongTimeEffectTable.hpp CLongTimeEffectTable.cpp \
Db/di.h db/ConnectionManager.h db/ConnectionManager.cpp db/postgres.h db/postgres.cpp \
script/CLuaScript.hpp script/CLuaScript.cpp script/CLuaNPCScript.hpp script/CLuaNPCScript.cpp script/CLuaItemScript.hpp script/CLuaItemScript.cpp \
script/CLuaMonsterScript.hpp script/CLuaMonsterScript.cpp CTriggerTable.cpp  CTriggerTable.hpp script/CLuaMagicScript.cpp script/CLuaMagicScript.hpp \
script/CLuaTileScript.cpp script/CLuaTileScript.hpp script/CLuaTriggerScript.cpp script/CLuaTriggerScript.hpp script/CLuaWeaponScript.hpp \
script/CLuaWeaponScript.cpp script/CLuaScheduledScript.hpp script/CLuaScheduledScript.cpp script/CLuaLongTimeEffectScript.hpp script/CLuaLongTimeEffectScript.cpp script/CLuaReloadScript.cpp script/CLuaReloadScript.hpp \
script/CLuaLoginScript.cpp script/CLuaLoginScript.hpp script/CLuaLogoutScript.cpp script/CLuaLogoutScript.hpp script/CLuaDepotScript.hpp script/CLuaDepotScript.cpp \
script/CLuaLookAtPlayerScript.cpp script/CLuaLookAtPlayerScript.hpp script/CLuaLearnScript.hpp script/CLuaLearnScript.cpp \
CScheduledScriptsTable.hpp CScheduledScriptsTable.cpp CSpellTable.cpp  CSpellTable.hpp \
netinterface/CNetInterface.hpp netinterface/CNetInterface.cpp \
netinterface/CCommandFactory.hpp netinterface/CCommandFactory.cpp netinterface/CBasicCommand.cpp netinterface/CBasicCommand.hpp netinterface/CBasicServerCommand.hpp \
netinterface/CBasicServerCommand.cpp netinterface/CBasicClientCommand.hpp netinterface/CBasicClientCommand.cpp \
netinterface/protocol/ClientCommands.hpp netinterface/protocol/ServerCommands.cpp netinterface/protocol/ServerCommands.hpp netinterface/CByteBuffer.cpp netinterface/CByteBuffer.hpp \
CPlayerManager.hpp CPlayerManager.cpp CWaypointList.hpp CWaypointList.cpp\
CLanguage.hpp CLanguage.cpp


testserver_LDADD = -lpthread -lssl -lcrypto -lpqxx -lpq -llua5.1 -lboost_thread -lboost_system -lluabind
testserver_LDFLAGS = 
CXXFLAGS = -ggdb -pipe -Wall -Werror -Wno-deprecated -D_THREAD_SAFE -D_REENTRANT -DCDataConnect_DEBUG -DAdminCommands_DEBUG
INCLUDES = -I/usr/include/postgresql -I/usr/include/lua5.1
