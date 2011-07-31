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


#include "LuaMonsterScript.hpp"
#include "Monster.hpp"
#include "World.hpp"
#include "luabind/luabind.hpp"
#include <iostream>
#include "Logger.hpp"
#include "fuse_ptr.hpp"

LuaMonsterScript::LuaMonsterScript(std::string filename) throw(ScriptException)
    : LuaScript(filename) {
}

LuaMonsterScript::~LuaMonsterScript() throw() {}

void LuaMonsterScript::onDeath(Character *Monster) {
    try {
        World::get()->setCurrentScript(this);
        //CWorld::get()->monitoringClientList->sendCommand( new SendMessageTS("onDeath called for: " + Monster->name,3));
        Logger::writeMessage("scripts","onDeath called for: " + Monster->name);
        fuse_ptr<Character> fuse_Monster(Monster);
        call("onDeath")(fuse_Monster);
    } catch (luabind::error &e) {
        writeErrorMsg();
        //CWorld::get()->monitoringClientList->sendCommand( new SendMessageTS("Error: onDeath called for: " + Monster->name + " " + e.what(),3));
    }
}

bool LuaMonsterScript::lookAtMonster(Character *source, Character *target, unsigned char mode) {
    try {
        World::get()->setCurrentScript(this);
        fuse_ptr<Character> fuse_source(source);
        fuse_ptr<Character> fuse_target(target);
        call("lookAtMonster")(fuse_source, fuse_target, mode);
        return true;
    } catch (luabind::error &e) {
        writeErrorMsg();
        return false;
    }
}

void LuaMonsterScript::receiveText(Character *Monster, Character::talk_type tt, std::string message, Character *cc) {
    try {
        World::get()->setCurrentScript(this);
#ifdef MAJORSCRIPTLOG
        // World::get()->monitoringClientList->sendCommand( new SendMessageTS("reiceveText called for: " + Monster->name,3));
        Logger::writeMessage("scripts","receiveText called for: " + Monster->name);
#endif
        fuse_ptr<Character> fuse_Monster(Monster);
        fuse_ptr<Character> fuse_cc(cc);
        call("receiveText")(fuse_Monster, (int)tt, message, fuse_cc);
    } catch (luabind::error &e) {
        writeErrorMsg();
        //CWorld::get()->monitoringClientList->sendCommand( new SendMessageTS("Error: receiveText called for: " + Monster->name + " " + e.what(),3));
    }
}

void LuaMonsterScript::onAttacked(Character *Monster, Character *attacker) {
    try {
        World::get()->setCurrentScript(this);
#ifdef MAJORSCRIPTLOG
        //CWorld::get()->monitoringClientList->sendCommand( new SendMessageTS("onAttack called for: " + Monster->name,3));
        Logger::writeMessage("scripts","onAttack called for: " + Monster->name);
#endif
        fuse_ptr<Character> fuse_Monster(Monster);
        fuse_ptr<Character> fuse_attacker(attacker);
        call("onAttacked")(fuse_Monster, fuse_attacker);
    } catch (luabind::error &e) {
        writeErrorMsg();
        //CWorld::get()->monitoringClientList->sendCommand( new SendMessageTS("Error: onAttack called for: " + Monster->name + " " + e.what(),3));

    }

}

void LuaMonsterScript::onCasted(Character *Monster, Character *caster) {
    try {
        World::get()->setCurrentScript(this);
#ifdef MAJORSCRIPTLOG
        //CWorld::get()->monitoringClientList->sendCommand( new SendMessageTS("onCasted called for: " + Monster->name,3));
        Logger::writeMessage("scripts","onCasted called for: " + Monster->name);
#endif
        fuse_ptr<Character> fuse_Monster(Monster);
        fuse_ptr<Character> fuse_caster(caster);
        call("onCasted")(fuse_Monster, fuse_caster);

    } catch (luabind::error &e) {
        writeErrorMsg();
        // World::get()->monitoringClientList->sendCommand( new SendMessageTS("Error: onCasted called for: " + Monster->name + " " + e.what(),3));
    }
}

void LuaMonsterScript::useMonster(Character *Monster, Character *user, unsigned short int counter, unsigned short int param, unsigned char ltastate) {
    try {
        World::get()->setCurrentScript(this);
        //CWorld::get()->monitoringClientList->sendCommand( new SendMessageTS("useMonster called for: " + Monster->name,3));
        Logger::writeMessage("scripts","useMonster called for: " + Monster->name);
        fuse_ptr<Character> fuse_Monster(Monster);
        fuse_ptr<Character> fuse_user(user);
        call("useMonster")(fuse_Monster, fuse_user, counter, param, ltastate);
    } catch (luabind::error &e) {
        writeErrorMsg();
        //CWorld::get()->monitoringClientList->sendCommand( new SendMessageTS("Error: useMonster called for: " + Monster->name + " " + e.what(),3));
    }
}

void LuaMonsterScript::useMonsterWithCharacter(Character *Monster, Character *user, Character *targetChar, unsigned short int counter, unsigned short param, unsigned char ltastate) {
    try {
        World::get()->setCurrentScript(this);
        //CWorld::get()->monitoringClientList->sendCommand( new SendMessageTS("useMonsterWithCharacter called for: " + Monster->name,3));
        Logger::writeMessage("scripts","useMonsterWithCharacter called for: " + Monster->name);
        fuse_ptr<Character> fuse_Monster(Monster);
        fuse_ptr<Character> fuse_user(user);
        fuse_ptr<Character> fuse_targetChar(targetChar);
        call("useMonsterWithCharacter")(fuse_Monster, fuse_user, fuse_targetChar, counter, param, ltastate);
    } catch (luabind::error &e) {
        writeErrorMsg();
        //CWorld::get()->monitoringClientList->sendCommand( new SendMessageTS("Error: useMonsterWithCharacter called for: " + Monster->name + " " + e.what(),3));
    }
}

void LuaMonsterScript::useMonsterWithField(Character *Monster, Character *user, position pos, unsigned short int counter, unsigned short param, unsigned char ltastate) {
    try {
        World::get()->setCurrentScript(this);
        //CWorld::get()->monitoringClientList->sendCommand( new SendMessageTS("useMonsterWithField called for: " + Monster->name,3));
        Logger::writeMessage("scripts","useMonsterWithField called for: " + Monster->name);
        fuse_ptr<Character> fuse_Monster(Monster);
        fuse_ptr<Character> fuse_user(user);
        call("useMonsterWithField")(fuse_Monster, fuse_user, pos, counter, param, ltastate);
    } catch (luabind::error &e) {
        writeErrorMsg();
        // World::get()->monitoringClientList->sendCommand( new SendMessageTS("Error: useMonsterWithField called for: " + Monster->name + " " + e.what(),3));
    }
}

void LuaMonsterScript::useMonsterWithItem(Character *Monster, Character *user, ScriptItem item, unsigned short int counter, unsigned short param, unsigned char ltastate) {
    try {
        World::get()->setCurrentScript(this);
        //CWorld::get()->monitoringClientList->sendCommand( new SendMessageTS("useMonsterWithItem called for: " + Monster->name,3));
        Logger::writeMessage("scripts","useMonsterWithItem called for: " + Monster->name);
        fuse_ptr<Character> fuse_Monster(Monster);
        fuse_ptr<Character> fuse_user(user);
        call("useMonsterWithItem")(fuse_Monster, fuse_user, item, counter, param, ltastate);
    } catch (luabind::error &e) {
        writeErrorMsg();
        //CWorld::get()->monitoringClientList->sendCommand( new SendMessageTS("Error: useMonsterWithItem called for: " + Monster->name + " " + e.what(),3));
    }
}

bool LuaMonsterScript::actionDisturbed(Character *performer, Character *disturber) {
    try {
        World::get()->setCurrentScript(this);
        //CWorld::get()->monitoringClientList->sendCommand( new SendMessageTS("actionDisturbed called",3));
        Logger::writeMessage("scripts","actionDisturbed called");
        fuse_ptr<Character> fuse_performer(performer);
        fuse_ptr<Character> fuse_disturber(disturber);
        return luabind::object_cast<bool>(call("actionDisturbed")(fuse_performer, fuse_disturber));
    } catch (luabind::cast_failed &e) {
        writeCastErrorMsg("actionDisturbed", "bool");
    } catch (luabind::error &e) {
        writeErrorMsg();
    }
    return true;
}

void LuaMonsterScript::onAttack(Character *Monster, Character *target) {
    try {
        World::get()->setCurrentScript(this);
#ifdef MAJORSCRIPTLOG
        //CWorld::get()->monitoringClientList->sendCommand( new SendMessageTS("onAttack called for: " + Monster->name,3));
        Logger::writeMessage("scripts","onAttack called for: " + Monster->name);
#endif
        fuse_ptr<Character> fuse_Monster(Monster);
        fuse_ptr<Character> fuse_target(target);
        call("onAttack")(fuse_Monster, fuse_target);
    } catch (luabind::error &e) {
        writeErrorMsg();
        // World::get()->monitoringClientList->sendCommand( new SendMessageTS("Error: onAttack called for: " + Monster->name + " " + e.what(),3));
    }
}

bool LuaMonsterScript::enemyOnSight(Character *Monster, Character *enemy) {
    try {
        World::get()->setCurrentScript(this);
#ifdef MAJORSCRIPTLOG
        //CWorld::get()->monitoringClientList->sendCommand( new SendMessageTS("enemyOnSight called for: " + Monster->name,3));
        Logger::writeMessage("scripts","enemyOnSight called for: " + Monster->name);
#endif
        fuse_ptr<Character> fuse_Monster(Monster);
        fuse_ptr<Character> fuse_enemy(enemy);
        return luabind::object_cast<bool>(call("enemyOnSight")(fuse_Monster, fuse_enemy));
    } catch (luabind::cast_failed &e) {
        writeCastErrorMsg("enemyOnSight", "bool");
    } catch (luabind::error &e) {
        writeErrorMsg();
    }

    return false;
}

bool LuaMonsterScript::enemyNear(Character *Monster, Character *enemy) {
    try {
        World::get()->setCurrentScript(this);
#ifdef MAJORSCRIPTLOG
        //CWorld::get()->monitoringClientList->sendCommand( new SendMessageTS("enemyNear called for: " + Monster->name,3));
        Logger::writeMessage("scripts","enemyNear called for: " + Monster->name);
#endif
        fuse_ptr<Character> fuse_Monster(Monster);
        fuse_ptr<Character> fuse_enemy(enemy);
        return luabind::object_cast<bool>(call("enemyNear")(fuse_Monster, fuse_enemy));
    } catch (luabind::cast_failed &e) {
        writeCastErrorMsg("enemyNear", "bool");
    } catch (luabind::error &e) {
        writeErrorMsg();
        //CWorld::get()->monitoringClientList->sendCommand( new SendMessageTS("Error: enemyNear called for: " + Monster->name + " " + e.what(),3));
    }

    return false;
}

void LuaMonsterScript::abortRoute(Character *Monster) {
    try {
        World::get()->setCurrentScript(this);
#ifdef MAJORSCRIPTLOG
        //CWorld::get()->monitoringClientList->sendCommand( new SendMessageTS("enemyNear called for: " + Monster->name,3));
        Logger::writeMessage("scripts","abortRoute called for: " + Monster->name);
#endif
        fuse_ptr<Character> fuse_Monster(Monster);
        call("abortRoute")(fuse_Monster);
    } catch (luabind::error &e) {
        writeErrorMsg();
        //CWorld::get()->monitoringClientList->sendCommand( new SendMessageTS("Error: enemyNear called for: " + Monster->name + " " + e.what(),3));
    }
}

void LuaMonsterScript::onSpawn(Character *Monster) {
    try {
        World::get()->setCurrentScript(this);
#ifdef MAJORSCRIPTLOG
        //CWorld::get()->monitoringClientList->sendCommand( new SendMessageTS("enemyNear called for: " + Monster->name,3));
        Logger::writeMessage("scripts","onSpawn called for: " + Monster->name);
#endif
        fuse_ptr<Character> fuse_Monster(Monster);
        call("onSpawn")(fuse_Monster);
    } catch (luabind::error &e) {
        writeErrorMsg();
        //CWorld::get()->monitoringClientList->sendCommand( new SendMessageTS("Error: enemyNear called for: " + Monster->name + " " + e.what(),3));
    }
}

bool LuaMonsterScript::setTarget(Character *Monster, std::vector<Player *> & CandidateList, Player * &Target) {
    Target = 0;

    if (!existsEntrypoint("setTarget")) {
        return false;
    }

    try {
        World::get()->setCurrentScript(this);
#ifdef MAJORSCRIPTLOG
        Logger::writeMessage("scripts","setTarget called for: " + Monster->name);
#endif
        luabind::object luaCandidateList = luabind::newtable(_luaState);
        int index = 1;
        std::vector<Player *>::iterator it;

        for (it = CandidateList.begin(); it != CandidateList.end(); ++it) {
            fuse_ptr<Character> fuse_it(*it);
            luaCandidateList[index++] = fuse_it;
        }

        fuse_ptr<Character> fuse_Monster(Monster);
        index = luabind::object_cast<int>(call("setTarget")(fuse_Monster, luaCandidateList)) - 1;

        if (index >= 0 && index < (int)CandidateList.size()) {
            Target =  CandidateList[index];
        }
    } catch (luabind::cast_failed &e) {
        writeCastErrorMsg("setTarget", "int");
    } catch (luabind::error &e) {
        writeErrorMsg();
    }

    return true;
}
