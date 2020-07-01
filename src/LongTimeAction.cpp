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
#include "script/LuaTileScript.hpp"

#include <memory>
#include <utility>

LongTimeAction::LongTimeAction(Player *player, World *world) : _owner(player), _world(world) {}

void LongTimeAction::setLastAction(std::shared_ptr<LuaScript> script, const SouTar &srce, const SouTar &trgt,
                                   ActionType at) {
    _script = std::move(script);
    _source = srce;
    _target = trgt;
    _at = at;

    if (trgt.character != nullptr) {
        _targetId = _target.character->getId();
    }

    if (srce.character != nullptr) {
        _sourceId = _source.character->getId();
        _sourceCharType = _source.character->getType();
    }
}

auto LongTimeAction::checkAction() -> bool {
    if (_actionrunning) {
        // check if timetowaittimer is initialized and if we hit the next time
        if (_timetowaitTimer && _timetowaitTimer->Next()) {
            successAction();
            return true;
        }

        if ((_redoaniTimer) && (_redoaniTimer->Next()) && (_ani != 0)) {
            _world->gfx(_ani, _owner->getPosition());
        }

        if ((_redosoundTimer) && (_redosoundTimer->Next()) && (_sound != 0)) {
            _world->makeSound(_sound, _owner->getPosition());
        }
    }

    return false;
}

void LongTimeAction::startLongTimeAction(unsigned short int timetowait, unsigned short int ani,
                                         unsigned short int redoani, unsigned short int sound,
                                         unsigned short int redosound) {
    _actionrunning = true;
    _ani = ani;
    _sound = sound;
    constexpr auto dsToMsFactor = 100;

    _timetowaitTimer = std::make_unique<MilTimer>(timetowait * dsToMsFactor);

    if (_redoaniTimer) {
        _redoaniTimer.reset();
    }

    if (_ani != 0 && redoani != 0) {
        _redoaniTimer = std::make_unique<MilTimer>(redoani * dsToMsFactor);
    }

    if (_redosoundTimer) {
        _redosoundTimer.reset();
    }

    if (_sound != 0 && redosound != 0) {
        _redosoundTimer = std::make_unique<MilTimer>(redosound * dsToMsFactor);
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
    checkTarget();

    if (_actionrunning) {
        if (_at == ACTION_CRAFT) {
            if (_source.Type == LUA_DIALOG) {
                _actionrunning = false;
                _owner->executeCraftingDialogCraftingAborted(_source.dialog);
            }
        } else if (_script) {
            bool disturbed = false;

            if (_at == ACTION_USE) {
                if (_source.Type == LUA_ITEM) {
                    std::shared_ptr<LuaItemScript> itemScript = std::dynamic_pointer_cast<LuaItemScript>(_script);

                    if (itemScript->existsEntrypoint("actionDisturbed")) {
                        disturbed = itemScript->actionDisturbed(_owner, disturber);
                    }
                } else if (_source.Type == LUA_FIELD) {
                    std::shared_ptr<LuaTileScript> tileScript = std::dynamic_pointer_cast<LuaTileScript>(_script);

                    if (tileScript->existsEntrypoint("actionDisturbed")) {
                        disturbed = tileScript->actionDisturbed(_owner, disturber);
                    }
                } else if (_source.Type == LUA_CHARACTER) {
                    if (_sourceCharType == Character::monster) {
                        std::shared_ptr<LuaMonsterScript> monsterScript =
                                std::dynamic_pointer_cast<LuaMonsterScript>(_script);

                        if (monsterScript->existsEntrypoint("actionDisturbed")) {
                            disturbed = monsterScript->actionDisturbed(_owner, disturber);
                        }
                    } else if (_sourceCharType == Character::npc) {
                        std::shared_ptr<LuaNPCScript> npcScript = std::dynamic_pointer_cast<LuaNPCScript>(_script);

                        if (npcScript->existsEntrypoint("actionDisturbed")) {
                            disturbed = npcScript->actionDisturbed(_owner, disturber);
                        }
                    }
                }
            } else if (_at == ACTION_MAGIC) {
                std::shared_ptr<LuaMagicScript> magicScript = std::dynamic_pointer_cast<LuaMagicScript>(_script);

                if (magicScript->existsEntrypoint("actionDisturbed")) {
                    disturbed = magicScript->actionDisturbed(_owner, disturber);
                }
            }

            if (disturbed) {
                abortAction();
                return true;
            }
            return false;

        } else {
            _actionrunning = false;
        }
    }

    return false;
}

void LongTimeAction::abortAction() {
    checkSource();
    checkTarget();

    if (_actionrunning) {
        if (_at == ACTION_CRAFT) {
            if (_source.Type == LUA_DIALOG) {
                _owner->executeCraftingDialogCraftingAborted(_source.dialog);
            }
        } else if (_script) {
            if (_at == ACTION_USE) {
                // a itemscript
                if (_source.Type == LUA_ITEM) {
                    std::shared_ptr<LuaItemScript> itScript = std::dynamic_pointer_cast<LuaItemScript>(_script);

                    if (_target.Type == LUA_ITEM || _target.Type == LUA_NONE) {
                        itScript->UseItem(_owner, _source.item, static_cast<unsigned char>(LTS_ACTIONABORTED));
                    }
                }
                // a tilescript
                else if (_source.Type == LUA_FIELD) {
                    std::shared_ptr<LuaTileScript> tiScript = std::dynamic_pointer_cast<LuaTileScript>(_script);

                    if (_target.Type == LUA_NONE) {
                        tiScript->useTile(_owner, _source.pos, static_cast<unsigned char>(LTS_ACTIONABORTED));
                    }
                }
                // a character
                else if (_source.Type == LUA_CHARACTER) {
                    // a monster
                    if (_sourceCharType == Character::monster) {
                        std::shared_ptr<LuaMonsterScript> monScript =
                                std::dynamic_pointer_cast<LuaMonsterScript>(_script);

                        if (_target.Type == LUA_NONE) {
                            monScript->useMonster(_source.character, _owner,
                                                  static_cast<unsigned char>(LTS_ACTIONABORTED));
                        }
                    }
                    // a npc
                    else if (_sourceCharType == Character::npc) {
                        std::shared_ptr<LuaNPCScript> npcScript = std::dynamic_pointer_cast<LuaNPCScript>(_script);

                        if (_target.Type == LUA_NONE) {
                            npcScript->useNPC(_owner, static_cast<unsigned char>(LTS_ACTIONABORTED));
                        }
                    }
                }
            } else if (_at == ACTION_MAGIC) {
                std::shared_ptr<LuaMagicScript> mgScript = std::dynamic_pointer_cast<LuaMagicScript>(_script);

                if (_target.Type == LUA_NONE) {
                    mgScript->CastMagic(_owner, static_cast<unsigned char>(LTS_ACTIONABORTED));
                } else if (_target.Type == LUA_FIELD) {
                    mgScript->CastMagicOnField(_owner, _target.pos, static_cast<unsigned char>(LTS_ACTIONABORTED));
                } else if (_target.Type == LUA_CHARACTER) {
                    mgScript->CastMagicOnCharacter(_owner, _target.character,
                                                   static_cast<unsigned char>(LTS_ACTIONABORTED));
                }
                // Todo add ki handling here
                else if (_target.Type == LUA_ITEM) {
                    mgScript->CastMagicOnItem(_owner, _target.item, static_cast<unsigned char>(LTS_ACTIONABORTED));
                }
            }
        }
    }

    _actionrunning = false;
    _script.reset();
    _redoaniTimer.reset();
    _redosoundTimer.reset();
    _timetowaitTimer.reset();
    _ani = 0;
    _sound = 0;
}

void LongTimeAction::successAction() {
    checkSource();
    checkTarget();

    if (_actionrunning) {
        _actionrunning = false;

        if (_at == ACTION_CRAFT) {
            if (_source.Type == LUA_DIALOG) {
                _owner->executeCraftingDialogCraftingComplete(_source.dialog);
                return;
            }
        } else if (_script) {
            if (_at == ACTION_USE) {
                // a itemscript
                if (_source.Type == LUA_ITEM) {
                    std::shared_ptr<LuaItemScript> itScript = std::dynamic_pointer_cast<LuaItemScript>(_script);

                    if (_target.Type == LUA_ITEM || _target.Type == LUA_NONE) {
                        itScript->UseItem(_owner, _source.item, static_cast<unsigned char>(LTS_ACTIONSUCCESSFULL));
                    }
                }
                // a tilescript
                else if (_source.Type == LUA_FIELD) {
                    std::shared_ptr<LuaTileScript> tiScript = std::dynamic_pointer_cast<LuaTileScript>(_script);

                    if (_target.Type == LUA_NONE) {
                        tiScript->useTile(_owner, _source.pos, static_cast<unsigned char>(LTS_ACTIONSUCCESSFULL));
                    }
                }
                // a character
                else if (_source.Type == LUA_CHARACTER) {
                    // a monster
                    if (_sourceCharType == Character::monster) {
                        std::shared_ptr<LuaMonsterScript> monScript =
                                std::dynamic_pointer_cast<LuaMonsterScript>(_script);

                        if (_target.Type == LUA_NONE) {
                            monScript->useMonster(_source.character, _owner,
                                                  static_cast<unsigned char>(LTS_ACTIONSUCCESSFULL));
                        }
                    }
                    // a npc
                    else if (_sourceCharType == Character::npc) {
                        std::shared_ptr<LuaNPCScript> npcScript = std::dynamic_pointer_cast<LuaNPCScript>(_script);

                        if (_target.Type == LUA_NONE) {
                            npcScript->useNPC(_owner, static_cast<unsigned char>(LTS_ACTIONSUCCESSFULL));
                        }
                    }
                }
            } else if (_at == ACTION_MAGIC) {
                std::shared_ptr<LuaMagicScript> mgScript = std::dynamic_pointer_cast<LuaMagicScript>(_script);

                if (_target.Type == LUA_NONE) {
                    mgScript->CastMagic(_owner, static_cast<unsigned char>(LTS_ACTIONSUCCESSFULL));
                } else if (_target.Type == LUA_FIELD) {
                    mgScript->CastMagicOnField(_owner, _target.pos, static_cast<unsigned char>(LTS_ACTIONSUCCESSFULL));
                } else if (_target.Type == LUA_CHARACTER) {
                    mgScript->CastMagicOnCharacter(_owner, _target.character,
                                                   static_cast<unsigned char>(LTS_ACTIONSUCCESSFULL));
                    // Todo add ki handling here
                } else if (_target.Type == LUA_ITEM) {
                    mgScript->CastMagicOnItem(_owner, _target.item, static_cast<unsigned char>(LTS_ACTIONSUCCESSFULL));
                }
            }
        }
    }

    if (!_actionrunning) {
        _script.reset();
        _redoaniTimer.reset();
        _redosoundTimer.reset();
        _timetowaitTimer.reset();
        _ani = 0;
        _sound = 0;
    }
}

void LongTimeAction::changeSource(Character *cc) {
    _source.Type = LUA_CHARACTER;
    _source.pos = cc->getPosition();
    _source.character = cc;
    _sourceId = cc->getId();
    _sourceCharType = cc->getType();
}

void LongTimeAction::changeSource(const ScriptItem &sI) {
    _source.Type = LUA_ITEM;
    _source.pos = sI.pos;
    _source.item = sI;
    _sourceId = 0;
    _sourceCharType = Character::player;
}

void LongTimeAction::changeSource(position pos) {
    _source.Type = LUA_FIELD;
    _source.pos = pos;
    _sourceId = 0;
    _sourceCharType = Character::player;
}

void LongTimeAction::changeSource() {
    _source.Type = LUA_NONE;
    _sourceId = 0;
    _sourceCharType = Character::player;
}

void LongTimeAction::changeTarget(Character *cc) {
    _target.Type = LUA_CHARACTER;
    _target.pos = cc->getPosition();
    _target.character = cc;
    _targetId = cc->getId();
}

void LongTimeAction::changeTarget(const ScriptItem &sI) {
    _target.Type = LUA_ITEM;
    _target.pos = sI.pos;
    _target.item = sI;
    _targetId = 0;
}

void LongTimeAction::checkTarget() {
    if (_targetId == 0) {
        _source.character = nullptr;
        return;
    }
    if (_targetId < MONSTER_BASE) {
        // player

        if (World::get()->Players.find(_targetId) == nullptr) {
            _target.character = nullptr;
        }

    } else if (_targetId >= MONSTER_BASE && _targetId < NPC_BASE) {
        // monster

        if (World::get()->Monsters.find(_targetId) == nullptr) {
            _target.character = nullptr;
        }

    } else {
        if (World::get()->Npc.find(_targetId) == nullptr) {
            _target.character = nullptr;
        }
    }
}

void LongTimeAction::checkSource() {
    if (_sourceId == 0) {
        _source.character = nullptr;
        _sourceCharType = 0;
        return;
    }
    if (_sourceId < MONSTER_BASE) {
        // player

        if (World::get()->Players.find(_sourceId) == nullptr) {
            _source.character = nullptr;

            _sourceCharType = 0;
        }

    } else if (_sourceId >= MONSTER_BASE && _sourceId < NPC_BASE) {
        // monster

        if (World::get()->Monsters.find(_sourceId) == nullptr) {
            _source.character = nullptr;

            _sourceCharType = 0;
        }

    } else {
        if (World::get()->Npc.find(_sourceId) == nullptr) {
            _source.character = nullptr;

            _sourceCharType = 0;
        }
    }
}

void LongTimeAction::changeTarget(position pos) {
    _target.Type = LUA_FIELD;
    _target.pos = pos;
    _targetId = 0;
}

void LongTimeAction::changeTarget() { _target.Type = LUA_NONE; }
