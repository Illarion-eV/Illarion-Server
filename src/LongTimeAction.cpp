//  illarionserver - server for the game Illarion
//  Copyright 2011 Illarion e.V.
//
//  This file is part of illarionserver.
//
//  illarionserver is free software: you can redistribute it and/or modify
//  it under the terms of the GNU Affero General Public License as published by
//  the Free Software Foundation, either version 3 of the License, or
//  (at your option) any later version.
//
//  illarionserver is distributed in the hope that it will be useful,
//  but WITHOUT ANY WARRANTY; without even the implied warranty of
//  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//  GNU Affero General Public License for more details.
//
//  You should have received a copy of the GNU Affero General Public License
//  along with illarionserver.  If not, see <http://www.gnu.org/licenses/>.

#include "LongTimeAction.hpp"

#include "Character.hpp"
#include "Monster.hpp"
#include "NPC.hpp"
#include "Player.hpp"
#include "World.hpp"
#include "script/LuaItemScript.hpp"
#include "script/LuaMagicScript.hpp"
#include "script/LuaMonsterScript.hpp"
#include "script/LuaNPCScript.hpp"
#include "script/LuaScript.hpp"
#include "script/LuaTileScript.hpp"
#include "script/server.hpp"

#include <memory>
#include <utility>

LongTimeAction::LongTimeAction(Player *player, World *world) : _owner(player), _world(world) {}

void LongTimeAction::setLastAction(std::shared_ptr<LuaScript> script, const ActionParameters &parameters,
                                   ActionType at) {
    this->script = std::move(script);
    this->currentScriptParameters = parameters;
    currentScriptType = at;

    if (parameters.character != nullptr) {
        parameterId = parameters.character->getId();
        characterType = parameters.character->getType();
    }
}

auto LongTimeAction::checkAction() -> bool {
    if (_actionrunning) {
        // check if timetowaittimer is initialized and if we hit the next time
        if (_timetowaitTimer && _timetowaitTimer->intervalExceeded()) {
            successAction();
            return true;
        }

        if ((_redoaniTimer) && (_redoaniTimer->intervalExceeded()) && (_ani != 0)) {
            _world->gfx(_ani, _owner->getPosition());
        }

        if ((_redosoundTimer) && (_redosoundTimer->intervalExceeded()) && (_sound != 0)) {
            _world->makeSound(_sound, _owner->getPosition());
        }
    }

    return false;
}

void LongTimeAction::startLongTimeAction(unsigned short int timetowait, unsigned short int ani,
                                         unsigned short int redoani, unsigned short int sound,
                                         unsigned short int redosound) {
    if (currentActionType == ActionType::TALK) {
        abortAction(); // talking normally doesn't abort actions
    }

    currentActionType = currentScriptType;
    currentActionParameters = currentScriptParameters;

    _actionrunning = true;
    _ani = ani;
    _sound = sound;
    constexpr auto dsToMsFactor = 100;
    using std::chrono::milliseconds;

    _timetowaitTimer = std::make_unique<Timer>(milliseconds(timetowait * dsToMsFactor));

    if (_redoaniTimer) {
        _redoaniTimer.reset();
    }

    if (_ani != 0 && redoani != 0) {
        _redoaniTimer = std::make_unique<Timer>(milliseconds(redoani * dsToMsFactor));
    }

    if (_redosoundTimer) {
        _redosoundTimer.reset();
    }

    if (_sound != 0 && redosound != 0) {
        _redosoundTimer = std::make_unique<Timer>(milliseconds(redosound * dsToMsFactor));
    }

    if (_sound != 0) {
        _world->makeSound(_sound, _owner->getPosition());
    }

    if (_ani != 0) {
        _world->gfx(_ani, _owner->getPosition());
    }
}

auto LongTimeAction::actionDisturbed(Character *disturber) -> bool {
    checkSource();

    if (_actionrunning) {
        bool disturbed = false;

        if (currentActionType == ActionType::TALK) {
            if (currentActionParameters.type == LUA_TALK) {
                auto &talkScript = script::server::playerTalk();
                if (talkScript.existsEntrypoint("actionDisturbed")) {
                    disturbed = talkScript.actionDisturbed(_owner, disturber);
                }
            }
        } else if (currentActionType == ActionType::CRAFT) {
            if (currentActionParameters.type == LUA_DIALOG) {
                _actionrunning = false;
                _owner->executeCraftingDialogCraftingAborted(currentActionParameters.dialog);
            }
        } else if (script) {
            if (currentActionType == ActionType::USE) {
                if (currentActionParameters.type == LUA_ITEM) {
                    std::shared_ptr<LuaItemScript> itemScript = std::dynamic_pointer_cast<LuaItemScript>(script);

                    if (itemScript->existsEntrypoint("actionDisturbed")) {
                        disturbed = itemScript->actionDisturbed(_owner, disturber);
                    }
                } else if (currentActionParameters.type == LUA_FIELD) {
                    std::shared_ptr<LuaTileScript> tileScript = std::dynamic_pointer_cast<LuaTileScript>(script);

                    if (tileScript->existsEntrypoint("actionDisturbed")) {
                        disturbed = tileScript->actionDisturbed(_owner, disturber);
                    }
                } else if (currentActionParameters.type == LUA_CHARACTER) {
                    if (characterType == Character::monster) {
                        std::shared_ptr<LuaMonsterScript> monsterScript =
                                std::dynamic_pointer_cast<LuaMonsterScript>(script);

                        if (monsterScript->existsEntrypoint("actionDisturbed")) {
                            disturbed = monsterScript->actionDisturbed(_owner, disturber);
                        }
                    } else if (characterType == Character::npc) {
                        std::shared_ptr<LuaNPCScript> npcScript = std::dynamic_pointer_cast<LuaNPCScript>(script);

                        if (npcScript->existsEntrypoint("actionDisturbed")) {
                            disturbed = npcScript->actionDisturbed(_owner, disturber);
                        }
                    }
                }
            } else if (currentActionType == ActionType::MAGIC) {
                std::shared_ptr<LuaMagicScript> magicScript = std::dynamic_pointer_cast<LuaMagicScript>(script);

                if (magicScript->existsEntrypoint("actionDisturbed")) {
                    disturbed = magicScript->actionDisturbed(_owner, disturber);
                }
            }
        } else {
            _actionrunning = false;
        }

        if (disturbed) {
            abortAction();
            return true;
        }
        return false;
    }

    return false;
}

void LongTimeAction::abortAction() {
    checkSource();

    if (_actionrunning) {
        if (currentActionType == ActionType::TALK) {
            if (currentActionParameters.type == LUA_TALK) {
                script::server::playerTalk().talk(_owner, currentActionParameters.talkType,
                                                  currentActionParameters.text, ST_ABORT);
            }
        } else if (currentActionType == ActionType::CRAFT) {
            if (currentActionParameters.type == LUA_DIALOG) {
                _owner->executeCraftingDialogCraftingAborted(currentActionParameters.dialog);
            }
        } else if (script) {
            if (currentActionType == ActionType::USE) {
                // a itemscript
                if (currentActionParameters.type == LUA_ITEM) {
                    std::shared_ptr<LuaItemScript> itScript = std::dynamic_pointer_cast<LuaItemScript>(script);
                    itScript->UseItem(_owner, currentActionParameters.item, ST_ABORT);
                }
                // a tilescript
                else if (currentActionParameters.type == LUA_FIELD) {
                    std::shared_ptr<LuaTileScript> tiScript = std::dynamic_pointer_cast<LuaTileScript>(script);
                    tiScript->useTile(_owner, currentActionParameters.pos, ST_ABORT);
                }
                // a character
                else if (currentActionParameters.type == LUA_CHARACTER) {
                    // a monster
                    if (characterType == Character::monster) {
                        std::shared_ptr<LuaMonsterScript> monScript =
                                std::dynamic_pointer_cast<LuaMonsterScript>(script);
                        monScript->useMonster(currentActionParameters.character, _owner, ST_ABORT);
                    }
                    // a npc
                    else if (characterType == Character::npc) {
                        std::shared_ptr<LuaNPCScript> npcScript = std::dynamic_pointer_cast<LuaNPCScript>(script);
                        npcScript->useNPC(_owner, ST_ABORT);
                    }
                }
            } else if (currentActionType == ActionType::MAGIC) {
                std::shared_ptr<LuaMagicScript> mgScript = std::dynamic_pointer_cast<LuaMagicScript>(script);

                if (currentActionParameters.type == LUA_NONE) {
                    mgScript->CastMagic(_owner, ST_ABORT);
                } else if (currentActionParameters.type == LUA_FIELD) {
                    mgScript->CastMagicOnField(_owner, currentActionParameters.pos, ST_ABORT);
                } else if (currentActionParameters.type == LUA_CHARACTER) {
                    mgScript->CastMagicOnCharacter(_owner, currentActionParameters.character, ST_ABORT);
                } else if (currentActionParameters.type == LUA_ITEM) {
                    mgScript->CastMagicOnItem(_owner, currentActionParameters.item, ST_ABORT);
                }
            }
        }
    }

    _actionrunning = false;
    script.reset();
    _redoaniTimer.reset();
    _redosoundTimer.reset();
    _timetowaitTimer.reset();
    _ani = 0;
    _sound = 0;
}

void LongTimeAction::successAction() {
    checkSource();

    if (_actionrunning) {
        _actionrunning = false;

        if (currentActionType == ActionType::TALK) {
            if (currentActionParameters.type == LUA_TALK) {
                script::server::playerTalk().talk(_owner, currentActionParameters.talkType,
                                                  currentActionParameters.text, ST_SUCCESS);
            }
        } else if (currentActionType == ActionType::CRAFT) {
            if (currentActionParameters.type == LUA_DIALOG) {
                _owner->executeCraftingDialogCraftingComplete(currentActionParameters.dialog);
                return;
            }
        } else if (script) {
            if (currentActionType == ActionType::USE) {
                // a itemscript
                if (currentActionParameters.type == LUA_ITEM) {
                    std::shared_ptr<LuaItemScript> itScript = std::dynamic_pointer_cast<LuaItemScript>(script);
                    itScript->UseItem(_owner, currentActionParameters.item, ST_SUCCESS);
                }
                // a tilescript
                else if (currentActionParameters.type == LUA_FIELD) {
                    std::shared_ptr<LuaTileScript> tiScript = std::dynamic_pointer_cast<LuaTileScript>(script);
                    tiScript->useTile(_owner, currentActionParameters.pos, ST_SUCCESS);
                }
                // a character
                else if (currentActionParameters.type == LUA_CHARACTER) {
                    // a monster
                    if (characterType == Character::monster) {
                        std::shared_ptr<LuaMonsterScript> monScript =
                                std::dynamic_pointer_cast<LuaMonsterScript>(script);
                        monScript->useMonster(currentActionParameters.character, _owner, ST_SUCCESS);
                    }
                    // a npc
                    else if (characterType == Character::npc) {
                        std::shared_ptr<LuaNPCScript> npcScript = std::dynamic_pointer_cast<LuaNPCScript>(script);
                        npcScript->useNPC(_owner, ST_SUCCESS);
                    }
                }
            } else if (currentActionType == ActionType::MAGIC) {
                std::shared_ptr<LuaMagicScript> mgScript = std::dynamic_pointer_cast<LuaMagicScript>(script);

                if (currentActionParameters.type == LUA_NONE) {
                    mgScript->CastMagic(_owner, ST_SUCCESS);
                } else if (currentActionParameters.type == LUA_FIELD) {
                    mgScript->CastMagicOnField(_owner, currentActionParameters.pos, ST_SUCCESS);
                } else if (currentActionParameters.type == LUA_CHARACTER) {
                    mgScript->CastMagicOnCharacter(_owner, currentActionParameters.character, ST_SUCCESS);
                    // Todo add ki handling here
                } else if (currentActionParameters.type == LUA_ITEM) {
                    mgScript->CastMagicOnItem(_owner, currentActionParameters.item, ST_SUCCESS);
                }
            }
        }
    }

    if (!_actionrunning) {
        script.reset();
        _redoaniTimer.reset();
        _redosoundTimer.reset();
        _timetowaitTimer.reset();
        _ani = 0;
        _sound = 0;
    }
}

void LongTimeAction::changeSource(Character *cc) {
    currentActionParameters.type = LUA_CHARACTER;
    currentActionParameters.pos = cc->getPosition();
    currentActionParameters.character = cc;
    parameterId = cc->getId();
    characterType = cc->getType();
}

void LongTimeAction::changeSource(const ScriptItem &sI) {
    currentActionParameters.type = LUA_ITEM;
    currentActionParameters.pos = sI.pos;
    currentActionParameters.item = sI;
    parameterId = 0;
    characterType = Character::player;
}

void LongTimeAction::changeSource(position pos) {
    currentActionParameters.type = LUA_FIELD;
    currentActionParameters.pos = pos;
    parameterId = 0;
    characterType = Character::player;
}

void LongTimeAction::changeSource(const std::string &text) { currentActionParameters.text = text; }

void LongTimeAction::changeSource() {
    currentActionParameters.type = LUA_NONE;
    parameterId = 0;
    characterType = Character::player;
}

void LongTimeAction::checkSource() {
    if (parameterId == 0) {
        currentActionParameters.character = nullptr;
        characterType = 0;
        return;
    }
    if (parameterId < MONSTER_BASE) {
        // player

        if (World::get()->Players.find(parameterId) == nullptr) {
            currentActionParameters.character = nullptr;

            characterType = 0;
        }

    } else if (parameterId < NPC_BASE) {
        // monster

        if (World::get()->Monsters.find(parameterId) == nullptr) {
            currentActionParameters.character = nullptr;

            characterType = 0;
        }

    } else {
        if (World::get()->Npc.find(parameterId) == nullptr) {
            currentActionParameters.character = nullptr;

            characterType = 0;
        }
    }
}
