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

#include "Player.hpp"
#include "World.hpp"
#include "Character.hpp"
#include <string>
#include "types.hpp"
#include "Logger.hpp"
#include "data/Data.hpp"
#include "data/MonsterTable.hpp"
#include "script/LuaNPCScript.hpp"
#include "script/LuaScript.hpp"
#include "script/LuaItemScript.hpp"
#include "script/LuaMagicScript.hpp"
#include "script/LuaMonsterScript.hpp"
#include "script/LuaTileScript.hpp"
#include "script/LuaLookAtPlayerScript.hpp"
#include "netinterface/protocol/ServerCommands.hpp"
#include "netinterface/protocol/BBIWIServerCommands.hpp"

#include "netinterface/protocol/ClientCommands.hpp"

extern MonsterTable *MonsterDescriptions;
extern std::shared_ptr<LuaLookAtPlayerScript>lookAtPlayerScript;

InputDialogTS::InputDialogTS() : BasicClientCommand(C_INPUTDIALOG_TS) {
}

void InputDialogTS::decodeData() {
    dialogId = getIntFromBuffer();
    success = getUnsignedCharFromBuffer() > 0;
    input = getStringFromBuffer();
}

void InputDialogTS::performAction(Player *player) {
    time(&(player->lastaction));
    player->executeInputDialog(dialogId, success, input);
}

ClientCommandPointer InputDialogTS::clone() {
    ClientCommandPointer cmd(new InputDialogTS());
    return cmd;
}

MessageDialogTS::MessageDialogTS() : BasicClientCommand(C_MESSAGEDIALOG_TS) {
}

void MessageDialogTS::decodeData() {
    dialogId = getIntFromBuffer();
}

void MessageDialogTS::performAction(Player *player) {
    time(&(player->lastaction));
    player->executeMessageDialog(dialogId);
}

ClientCommandPointer MessageDialogTS::clone() {
    ClientCommandPointer cmd(new MessageDialogTS());
    return cmd;
}

MerchantDialogTS::MerchantDialogTS() : BasicClientCommand(C_MERCHANTDIALOG_TS) {
}

void MerchantDialogTS::decodeData() {
    dialogId = getIntFromBuffer();
    result = getUnsignedCharFromBuffer();

    switch (result) {
    case 0:
        break;

    case 1:
        saleLocation = getUnsignedCharFromBuffer();
        saleSlot = getShortIntFromBuffer();
        saleAmount = getShortIntFromBuffer();
        break;

    case 2:
        purchaseIndex = getUnsignedCharFromBuffer();
        purchaseAmount = getShortIntFromBuffer();
        break;
    }
}

void MerchantDialogTS::performAction(Player *player) {
    time(&(player->lastaction));

    switch (result) {
    case 0:
        player->executeMerchantDialogAbort(dialogId);
        break;

    case 1:
        player->executeMerchantDialogSell(dialogId, saleLocation, saleSlot, saleAmount);
        break;

    case 2:
        player->executeMerchantDialogBuy(dialogId, purchaseIndex, purchaseAmount);
        break;
    }
}

ClientCommandPointer MerchantDialogTS::clone() {
    ClientCommandPointer cmd(new MerchantDialogTS());
    return cmd;
}

SelectionDialogTS::SelectionDialogTS() : BasicClientCommand(C_SELECTIONDIALOG_TS) {
}

void SelectionDialogTS::decodeData() {
    dialogId = getIntFromBuffer();
    success = getUnsignedCharFromBuffer() > 0;
    selectedIndex = getUnsignedCharFromBuffer();
}

void SelectionDialogTS::performAction(Player *player) {
    time(&(player->lastaction));
    player->executeSelectionDialog(dialogId, success, selectedIndex);
}

ClientCommandPointer SelectionDialogTS::clone() {
    ClientCommandPointer cmd(new SelectionDialogTS());
    return cmd;
}

CraftingDialogTS::CraftingDialogTS() : BasicClientCommand(C_CRAFTINGDIALOG_TS) {
}

void CraftingDialogTS::decodeData() {
    dialogId = getIntFromBuffer();
    result = getUnsignedCharFromBuffer();

    switch (result) {
    case 0:
        break;

    case 1:
        craftIndex = getUnsignedCharFromBuffer();
        craftAmount = getUnsignedCharFromBuffer();
        break;

    case 2:
        craftIndex = getUnsignedCharFromBuffer();
        break;

    case 3:
        craftIndex = getUnsignedCharFromBuffer();
        craftIngredient = getUnsignedCharFromBuffer();
        break;
    }
}

void CraftingDialogTS::performAction(Player *player) {
    time(&(player->lastaction));

    switch (result) {
    case 0:
        player->executeCraftingDialogAbort(dialogId);
        break;

    case 1:
        player->executeCraftingDialogCraft(dialogId, craftIndex, craftAmount);
        break;

    case 2:
        player->executeCraftingDialogLookAtCraftable(dialogId, craftIndex);
        break;

    case 3:
        player->executeCraftingDialogLookAtIngredient(dialogId, craftIndex, craftIngredient);
        break;
    }
}

ClientCommandPointer CraftingDialogTS::clone() {
    ClientCommandPointer cmd(new CraftingDialogTS());
    return cmd;
}

RequestAppearanceTS::RequestAppearanceTS() : BasicClientCommand(C_REQUESTAPPEARANCE_TS) {
}

void RequestAppearanceTS::decodeData() {
    id = getIntFromBuffer();
}

void RequestAppearanceTS::performAction(Player *player) {
    Character *ch = World::get()->Players.findID(id);

    if (ch == NULL) {
        ch = World::get()->Monsters.findID(id);

        if (ch == NULL) {
            ch = World::get()->Npc.findID(id);
        }
    }

    //check if we found a character befor
    if (ch != NULL) {
        ch->updateAppearanceForPlayer(player, true);
    }

}

ClientCommandPointer RequestAppearanceTS::clone() {
    ClientCommandPointer cmd(new RequestAppearanceTS());
    return cmd;
}

LookAtCharacterTS::LookAtCharacterTS() : BasicClientCommand(C_LOOKATCHARACTER_TS) {
}

void LookAtCharacterTS::decodeData() {
    id = getIntFromBuffer();
    mode = getUnsignedCharFromBuffer();
}

void LookAtCharacterTS::performAction(Player *player) {
    if (id < MONSTER_BASE) {
        Player *pl = World::get()->Players.findID(id);

        if (pl) {
            if (lookAtPlayerScript) {
                lookAtPlayerScript->lookAtPlayer(player, pl, mode);
            }
        }

        //code for player handling
    } else if (id >= MONSTER_BASE && id < NPC_BASE) {
        Monster *monster = World::get()->Monsters.findID(id);

        if (monster) {
            MonsterStruct mon;

            if (MonsterDescriptions->find(monster->getType(), mon)) {
                if (mon.script && mon.script->existsEntrypoint("lookAtMonster")) {
                    mon.script->lookAtMonster(player, monster, mode);
                    return;
                }

                std::string german = "Das ist: " + mon.nameDe;
                std::string english = "This is: " + mon.nameEn;

                ServerCommandPointer cmd(new CharDescription(id, player->nls(german, english)));
                player->Connection->addCommand(cmd);
            }
        }
    } else {
        NPC *npc = World::get()->Npc.findID(id);

        if (npc) {
            if (npc->getScript() && npc->getScript()->existsEntrypoint("lookAtNpc")) {
                npc->getScript()->lookAtNpc(player, mode);
                return;
            }

            std::string german = "Das ist: " + npc->getName();
            std::string english = "This is: " + npc->getName();

            ServerCommandPointer cmd(new CharDescription(id, player->nls(german, english)));
            player->Connection->addCommand(cmd);

        }
    }


}

ClientCommandPointer LookAtCharacterTS::clone() {
    ClientCommandPointer cmd(new LookAtCharacterTS());
    return cmd;
}

CastTS::CastTS() : BasicClientCommand(C_CAST_TS) {
}

void CastTS::decodeData() {
    spellId = static_cast<unsigned long int>(getIntFromBuffer());
    cid = getUnsignedCharFromBuffer();

    switch (cid) {
    case UID_KOORD:
        castPosition.x = static_cast<short int>(getShortIntFromBuffer());
        castPosition.y = static_cast<short int>(getShortIntFromBuffer());
        castPosition.z = static_cast<short int>(getShortIntFromBuffer());
        break;

    case UID_SHOWC:
        showcase = getUnsignedCharFromBuffer();
        pos = getUnsignedCharFromBuffer();
        break;

    case UID_INV:
        pos = getUnsignedCharFromBuffer();
        break;

    case UID_MAGICWAND:
        break;
    }
}

void CastTS::performAction(Player *player) {
    time(&(player->lastaction));
    player->ltAction->abortAction();

    bool paramOK = true;
    //CScript* skript = NULL;

    std::shared_ptr<LuaMagicScript> LuaMageScript;

    // berprfen, ob der Spieler die Runen beherrscht
    if ((spellId & player->magic.flags[ player->magic.type ]) == spellId) {
        Spell spell;
        spell.magicType = player->magic.type;
        spell.spellId = spellId;

        if (Data::Spells.exists(spell)) {
            LuaMageScript = Data::Spells.script(spell);
        }
    }

    Logger::info(LogFacility::Script) << *player << " can't cast the spell: " << spellId << " , flags: " << player->magic.flags[ player->magic.type ] << Log::end;

    //Source des Castens zuweisen
    SouTar Source, Target;
    Source.character = dynamic_cast<Character *>(player);
    Source.pos = player->pos;
    Source.Type = LUA_CHARACTER;

    switch (cid) {
    case UID_KOORD:
        Logger::info(LogFacility::Script) << *player << " trys to cast on a coordinate pos " << castPosition << Log::end;

        if (LuaMageScript) {
            Field *temp;

            if (!World::get()->GetPToCFieldAt(temp, castPosition)) {
                Logger::error(LogFacility::Script) << "cant find field for casting at pos " << castPosition << Log::end;
                paramOK = false;
            } else {
                // Feld gefunden
                if (temp->IsPlayerOnField() || temp->IsMonsterOnField() || temp->IsNPCOnField()) {
                    Character *tmpCharacter = World::get()->findCharacterOnField(castPosition);

                    if (tmpCharacter) {
                        //Nothing to do here
                    } else {
                        Logger::debug(LogFacility::Script) << "Character found at target field!" << Log::end;
                    }

                    // Character auf Feld ist ein Spieler
                    if ((tmpCharacter->character == Character::player) && (LuaMageScript)) {
                        Logger::debug(LogFacility::Script) << "Target Character: player" << Log::end;
                        //Lua Script zuweisung
                        Target.character = tmpCharacter;
                        Target.pos = tmpCharacter->pos;
                        Target.Type = LUA_CHARACTER;
                    }
                    // Character auf Feld ist ein NPC
                    else if ((tmpCharacter->character == Character::npc) && (LuaMageScript)) {
                        Logger::debug(LogFacility::Script) << "Target Character: NPC" << Log::end;
                        //Lua Script zuweisung
                        Target.character = tmpCharacter;
                        Target.pos = tmpCharacter->pos;
                        Target.Type = LUA_CHARACTER;
                    } else if ((tmpCharacter->character == Character::monster) && (LuaMageScript)) {
                        Logger::debug(LogFacility::Script) << "Target Character: monster" << Log::end;
                        //Lua Script zuweisung
                        Target.character = tmpCharacter;
                        Target.pos = tmpCharacter->pos;
                        Target.Type = LUA_CHARACTER;
                    }
                } else {
                    Item it;

                    if (temp->ViewTopItem(it)) {
                        Logger::debug(LogFacility::Script) << "Item found at target field!" << Log::end;

                        if (LuaMageScript) {
                            Target.pos = castPosition;
                            Target.Type = LUA_ITEM;
                            Target.item = it;
                            Target.item.type = ScriptItem::it_field;
                            Target.item.pos = castPosition;
                            Target.item.owner = player;
                        }
                    } else {
                        Logger::debug(LogFacility::Script) << "empty field!" << Log::end;

                        if (LuaMageScript) {
                            Target.pos = castPosition;
                            Target.Type = LUA_FIELD;
                        }
                    }
                }
            }
        }
        else {
            std::cerr<<"LuaMageScript false, paramOK = false!"<<std::endl;
            paramOK = false;
        }

        break;

    case UID_SHOWC:

#ifdef World_DEBUG
        std::cout << "showcase: " << (int) showcase << " pos: " << (int) pos << std::endl;
#endif

        Logger::info(LogFacility::Script) << *player << " is casting in showcas: " << showcase << " pos " << pos << Log::end;

        if (LuaMageScript) {
            if (player->isShowcaseOpen(showcase)) {
                Container *ps = player->getShowcaseContainer(showcase);

                if (ps != NULL) {
#ifdef World_DEBUG
                    std::cout << "Container gefunden" << std::endl;
#endif
                    ScriptItem tempi;
                    Container *tempc;

                    if (ps->viewItemNr(pos, tempi, tempc)) {
#ifdef World_DEBUG
                        std::cout << "pos gefunden" << std::endl;
#endif

                        if (LuaMageScript) {
                            Target.Type = LUA_ITEM;
                            ps->viewItemNr(pos, Target.item, tempc);
                            Target.item.pos = castPosition;
                            Target.item.type = ScriptItem::it_container;
                            Target.item.itempos = pos;
                            Target.item.owner = player;
                            Target.item.inside = ps;
                            Target.pos = castPosition;
                        }
                    } else {
                        std::cerr<<"ps->viewItemNr false, paramOK = false!"<<std::endl;
                        paramOK = false;
                    }
                } else {
                    std::cerr<<"ps!=Null false, paramOK = false!"<<std::endl;
                    paramOK = false;
                }
            } else {
                std::cerr << "showcase < MAXSHOWCASE false, paramOK = false!"<<std::endl;
                paramOK = false;
            }
        } // LuaMageScript == NULL ?

        break;

    case UID_MAGICWAND:

        //UID_MAGICWAND wird immer gesandt wenn kein Ziel gewaehlt wird.
#ifdef World_DEBUG
        std::cout << "UID_MAGICWAND" << std::endl;
#endif
        Logger::debug(LogFacility::Script) << "Cast with Wand" << Log::end;

        if ((player->attackmode) && (player->enemyid != 0) && (LuaMageScript)) {
            bool zauberstab=false;

            if ((player->characterItems[ LEFT_TOOL ].getId() != 0) && (player->characterItems[ LEFT_TOOL ].getId() != BLOCKEDITEM)) {
                const auto weaponId = player->characterItems[LEFT_TOOL].getId();

                if (Data::WeaponItems.exists(weaponId)) {
                    if (Data::WeaponItems[weaponId].WeaponType == 13) {
                        zauberstab = true;
                        std::cout << "Zauberstab in der Hand -> OK" << std::endl;
                    }
                }
            }

            if ((player->characterItems[ RIGHT_TOOL ].getId() != 0) && (player->characterItems[ RIGHT_TOOL ].getId() != BLOCKEDITEM)) {
                const auto weaponId = player->characterItems[RIGHT_TOOL].getId();

                if (Data::WeaponItems.exists(weaponId)) {
                    if (Data::WeaponItems[weaponId].WeaponType == 13) {
                        zauberstab = true;
                        std::cout << "Zauberstab in der Hand -> OK" << std::endl;
                    }
                }
            }


            if (zauberstab) {
                switch (player->enemytype) {

                    //Muss spaeter angepasst werden wenn es nur noch einen Charactervektor gibt.

                case Character::player:
#ifdef World_DEBUG
                    std::cout << "Gegner ist ein Spieler" << std::endl;
#endif

                    if (LuaMageScript) {
                        Target.Type = LUA_CHARACTER;
                        Target.character = World::get()->findCharacter(player->enemyid);

                        if (Target.character != NULL) {
                            Target.pos = Target.character->pos;
                        } else {
                            paramOK = false;
                            std::cerr << "Kein geeignetes Ziel fr Zauberstab gefunden (Target.Character == NULL)!" << std::endl;
                        }
                    }

                    break;

                case Character::npc:
#ifdef World_DEBUG
                    std::cout << "Gegner ist ein NPC" << std::endl;
#endif

                    if (LuaMageScript) {
                        Target.Type = LUA_CHARACTER;
                        Target.character = World::get()->findCharacter(player->enemyid);

                        if (Target.character != NULL) {
                            Target.pos = Target.character->pos;
                        } else {
                            paramOK = false;
                            std::cerr << "Kein geeignetes Ziel fr Zauberstab gefunden (Target.Character == NULL)!" << std::endl;
                        }
                    }

                    break;

                case Character::monster:
#ifdef World_DEBUG
                    std::cout << "Gegner ist ein Monster" << std::endl;
#endif

                    if (LuaMageScript) {
                        Target.Type = LUA_CHARACTER;
                        Target.character = World::get()->findCharacter(player->enemyid);

                        if (Target.character != NULL) {
                            Target.pos = Target.character->pos;
                        } else {
                            paramOK = false;
                            std::cerr << "Kein geeignetes Ziel fr Zauberstab gefunden (Target.Character == NULL)!" << std::endl;
                        }
                    }

                    break;
                } // switch
            } // zauberstab
            else {
                std::cout<<"Zauberstab = false, paramOK = false!"<<std::endl;
                paramOK = false;
            }

        } // enemyid != 0
        else {

            if (!LuaMageScript) {
                std::cout<<"LuaMageScript nicht gesetzt, paramOK = false!"<<std::endl;
                paramOK = false;
            }
        }

        break;

    case UID_INV:

#ifdef World_DEBUG
        std::cout << "cast mit Inv" << std::endl;
#endif
        Logger::debug(LogFacility::Script) << "cast in inventory" << Log::end;

        if (LuaMageScript) {
            if (pos < (MAX_BELT_SLOTS + MAX_BODY_ITEMS)) {
#ifdef World_DEBUG
                std::cout << "gltiger Wert" << std::endl;
#endif

                if (player->characterItems[ pos ].getId() != 0) {
#ifdef World_DEBUG
                    std::cout << "Position " << (int) pos << " am Koerper ist besetzt" << std::endl;
#endif

                    if (LuaMageScript) {
                        Target.Type = LUA_ITEM;
                        Target.item = (ScriptItem)player->characterItems[ pos ];
                        Target.item.pos = player->pos;

                        if (pos < MAX_BODY_ITEMS) {
                            Target.item.type = ScriptItem::it_inventory;
                        } else {
                            Target.item.type = ScriptItem::it_belt;
                        }

                        Target.item.itempos = pos;
                        Target.item.owner = player;
                        Target.pos = player->pos;
                    }
                } else {
                    std::cerr<<"cp->characterItems[pos].getId() != 0 false, paramOK = false!"<<std::endl;
                    paramOK = false;
                }
            } else {
                std::cerr<<"pos < (MAX_BELT_SLOTS + MAX_BODY_ITEMS), paramOK = false!"<<std::endl;
                paramOK = false;
            }
        } // skript != NULL ?

        break;

    } // end of switch ID

    if (LuaMageScript) {
        Logger::debug(LogFacility::Script) << "try to call magic script" << Log::end;
        player->ltAction->setLastAction(LuaMageScript, Source, Target, LongTimeAction::ACTION_MAGIC);
        //std::string msg;
#ifdef World_DEBUG
        std::cout<<"paramOK: "<<paramOK<<std::endl;
#endif

        if ((paramOK) && player->IsAlive() && (player->GetStatus() < 10)) {
            switch (Target.Type) {
            case LUA_NONE:
                LuaMageScript->CastMagic(player, static_cast<unsigned char>(LTS_NOLTACTION));
                //msg = "Casted spell: " + Logger::toString(spellId);
                break;

            case LUA_FIELD:
                LuaMageScript->CastMagicOnField(player, Target.pos, static_cast<unsigned char>(LTS_NOLTACTION));
                //msg = "Casted spell: " + Logger::toString(spellId) + " on field at pos(" + Logger::toString(Target.pos.x) + "," + Logger::toString(Target.pos.y) + "," + Logger::toString(Target.pos.z) + ")";
                break;

            case LUA_CHARACTER:
                LuaMageScript->CastMagicOnCharacter(player, Target.character, static_cast<unsigned char>(LTS_NOLTACTION));

                if (Target.character->character == Character::monster) {
                    MonsterStruct monStruct;
                    Monster *temp = dynamic_cast<Monster *>(Target.character);

                    if (MonsterDescriptions->find(temp->getType(), monStruct)) {
                        if (monStruct.script) {
                            monStruct.script->onCasted(temp,player);
                        }
                    } else {
                        std::cerr<<"Didn't finde Monster Description for: "<< temp->getType() << " can't call onCasted!"<<std::endl;
                    }

                }

                //msg = "Casted spell: " + Logger::toString(spellId) + " on character: " + Target.character->to_string() + "(" + Logger::toString(Target.character->getId()) + ")";
                break;

            case LUA_ITEM:
                LuaMageScript->CastMagicOnItem(player, Target.item, static_cast<unsigned char>(LTS_NOLTACTION));
                //msg = "Casted spell: " + Logger::toString(spellId) + " on item: " + Logger::toString(Target.item.getId());
                break;

            default:
                LuaMageScript->CastMagic(player, static_cast<unsigned char>(LTS_NOLTACTION));
                //msg = "Casted spell: " + Logger::toString(spellId) + " on item: " + Logger::toString(Target.item.getId());
                break;
            } //Ende Switch

            //monitoringClientList->sendCommand( new SendActionTS( player->getId(), *player, 2, msg));
        } //ENde if player->IsAlive

        Logger::debug(LogFacility::Script) << "all succeeded" << Log::end;
    }
}

ClientCommandPointer CastTS::clone() {
    ClientCommandPointer cmd(new CastTS());
    return cmd;
}

UseTS::UseTS() : BasicClientCommand(C_USE_TS) {
}

void UseTS::decodeData() {
    useId = getUnsignedCharFromBuffer();

    switch (useId) {
    case UID_KOORD:
        usePosition.x = getShortIntFromBuffer();
        usePosition.y = getShortIntFromBuffer();
        usePosition.z = getShortIntFromBuffer();
        break;

    case UID_SHOWC:
        showcase = getUnsignedCharFromBuffer();
        pos = getUnsignedCharFromBuffer();
        break;

    case UID_INV:
        pos = getUnsignedCharFromBuffer();
        break;
    }
}

void UseTS::performAction(Player *player) {
    time(&(player->lastaction));
    player->ltAction->abortAction();

    Logger::debug(LogFacility::Script) << *player << " uses something" << Log::end;

    bool paramOK = true;

    std::shared_ptr<LuaItemScript> LuaScript;
    std::shared_ptr<LuaNPCScript> LuaNPCScript;
    std::shared_ptr<LuaMonsterScript> LuaMonsterScript;
    std::shared_ptr<LuaTileScript> LuaTileScript;
    SouTar Source, Target;
    CommonStruct com;

    switch (useId) {
    case UID_KOORD:

        Field *temp;

        Logger::debug(LogFacility::Script) << "UID_KOORD" << Log::end;
        Logger::debug(LogFacility::Script) << usePosition << Log::end;

        if (!World::get()->GetPToCFieldAt(temp, usePosition)) {
            Logger::debug(LogFacility::Script) << "Use UID_KOORD field not found!" << Log::end;
            Logger::debug(LogFacility::Script) << "Use UID_KOORD field not found at pos " << usePosition << Log::end;
            paramOK = false;
        } else {
            // Feld gefunden
            //Prfen ob sich irgendeine art Char auf dem Feld befindet (Spaeter nur noch IsCharOnField vorerst noch alle Arten pruefen
            if (temp->IsPlayerOnField() || temp->IsNPCOnField() || temp->IsMonsterOnField()) {
                Logger::debug(LogFacility::Script) << "Character on field found!" << Log::end;
                Character *tmpCharacter = World::get()->findCharacterOnField(usePosition);

                if (tmpCharacter) {
                    if (tmpCharacter->character == Character::player) {
                        Logger::debug(LogFacility::Script) << "Character is a player!" << Log::end;
                    } else if (tmpCharacter->character == Character::npc) {
                        Logger::debug(LogFacility::Script) << "Character is a NPC!" << Log::end;

                        NPC *scriptNPC = dynamic_cast<NPC *>(tmpCharacter);
                        LuaNPCScript = scriptNPC->getScript();

                        if (LuaNPCScript) {
                            Source.pos = scriptNPC->pos;
                            Source.character = scriptNPC;
                            Source.Type = LUA_CHARACTER;
                        }
                    } else if (tmpCharacter->character == Character::monster) {
                        Logger::debug(LogFacility::Script) << "Character is a monster!" << Log::end;

                        Monster *scriptMonster = dynamic_cast<Monster *>(tmpCharacter);
                        MonsterStruct monStruct;

                        if (MonsterDescriptions->find(scriptMonster->getType(),monStruct)) {
                            LuaMonsterScript = monStruct.script;
                        } else {
                            Logger::error(LogFacility::Script) << "try to use Monster but id: " << scriptMonster->getType() << " not found in database!" << Log::end;
                        }

                        if (LuaMonsterScript) {
                            Source.pos = scriptMonster->pos;
                            Source.character = scriptMonster;
                            Source.Type = LUA_CHARACTER;
                        }
                    }
                } else {
                    Logger::error(LogFacility::Script) << "Character on field " << usePosition << " not found!" << Log::end;
                }
            } else {
                Logger::debug(LogFacility::Script) << "no character on field!" << Log::end;
                Item it;

                if (temp->ViewTopItem(it)) {
                    Logger::debug(LogFacility::Script) << "Item on field" << Log::end;

                    LuaScript = Data::CommonItems.script(it.getId());

                    if (LuaScript) {
                        Source.Type = LUA_ITEM;
                        temp->ViewTopItem(Source.item);
                        Source.item.pos = usePosition;
                        Source.item.type = ScriptItem::it_field;
                        Source.item.owner = player;
                        Source.pos = usePosition;
                    }
                } else {
                    Logger::debug(LogFacility::Script) << "empty field!" << Log::end;

                    auto &script = Data::Tiles.script(temp->getTileId());

                    if (script) {
                        LuaTileScript = script;
                        Source.Type = LUA_FIELD;
                        Source.pos = usePosition;
                    }
                }
            }
        }

        break;

    case UID_SHOWC:
        Logger::debug(LogFacility::Script) << "showcase: " << static_cast<int>(showcase) << " pos: " << static_cast<int>(pos) << Log::end;

        if (player->isShowcaseOpen(showcase)) {
            Container *ps = player->getShowcaseContainer(showcase);

            if (ps != NULL) {
                Logger::debug(LogFacility::Script) << "Container gefunden!" << Log::end;
                ScriptItem tempi;
                Container *tempc;

                if (ps->viewItemNr(pos, tempi, tempc)) {
                    Logger::debug(LogFacility::Script) << "pos found item id: " << tempi.getId() << Log::end;

                    LuaScript = Data::CommonItems.script(tempi.getId());

                    if (LuaScript) {
                        Source.Type = LUA_ITEM;
                        ps->viewItemNr(pos, Source.item, tempc);
                        Source.item.pos = usePosition;
                        Source.item.type = ScriptItem::it_container;
                        Source.item.itempos = pos;
                        Source.item.owner = player;
                        Source.item.inside = ps;
                        Source.pos = usePosition;
                    }
                } else {
                    paramOK = false;
                }
            } else {
                paramOK = false;
            }
        } else {
            paramOK = false;
        }

        break;

    case UID_INV:

        if (pos < (MAX_BELT_SLOTS + MAX_BODY_ITEMS)) {
            Logger::debug(LogFacility::Script) << "position approved!" << Log::end;

            if (player->characterItems[ pos ].getId() != 0) {
                Logger::debug(LogFacility::Script) << "at position " << static_cast<int>(pos) << " on body, is an item with id: " << player->characterItems[ pos ].getId() << Log::end;

                LuaScript = Data::CommonItems.script(player->characterItems[ pos ].getId()) ;

                if (LuaScript) {
                    Source.Type = LUA_ITEM;
                    Source.item = (ScriptItem)player->characterItems[ pos ];
                    Source.item.pos = player->pos;

                    if (pos < MAX_BODY_ITEMS) {
                        Source.item.type = ScriptItem::it_inventory;
                    } else {
                        Source.item.type = ScriptItem::it_belt;
                    }

                    Source.item.itempos = pos;
                    Source.item.owner = player;
                    Source.pos = player->pos;
                }
            } else {
                paramOK = false;
            }
        } else {
            paramOK = false;
        }

        break;

    default:
        paramOK = false;
        break;
    }

    Logger::debug(LogFacility::Script) << "=========Use Script Start=============" << Log::end;
    Logger::debug(LogFacility::Script) << "Source pos (" << Source.pos.x << "," << Source.pos.y << "," << Source.pos.z << ")" << Log::end;
    Logger::debug(LogFacility::Script) << "Source type: " << Source.Type << Log::end;
    Logger::debug(LogFacility::Script) << "Source Character: " << *Source.character << Log::end;
    Logger::debug(LogFacility::Script) << "Target pos (" << Target.pos.x << "," << Target.pos.y << "," << Target.pos.z << ")" << Log::end;
    Logger::debug(LogFacility::Script) << "Target Type: " << Target.Type << Log::end;
    Logger::debug(LogFacility::Script) << "Target Character: " << *Target.character << Log::end;
    Logger::debug(LogFacility::Script) << "==========Use Script End=============" << Log::end;

    std::string msg;

    if (LuaScript) {
        player->ltAction->setLastAction(LuaScript, Source, Target, LongTimeAction::ACTION_USE);

        if ((paramOK) && player->IsAlive()) {

            if (Source.Type == LUA_ITEM) {
                LuaScript->UseItem(player, Source.item, static_cast<unsigned char>(LTS_NOLTACTION));
                msg = "Used Item: " + std::to_string(Source.item.getId()) + " with item: " + std::to_string(Target.item.getId());
            }
        }
    } else if (LuaNPCScript) {
        player->ltAction->setLastAction(LuaNPCScript, Source, Target, LongTimeAction::ACTION_USE);

        if ((paramOK) && player->IsAlive()) {
            if (Source.Type == LUA_CHARACTER && (Target.Type == LUA_NONE)) {
                LuaNPCScript->useNPC(player, static_cast<unsigned char>(LTS_NOLTACTION));
                msg = "Used NPC: " + Source.character->to_string();
            }
        }

    } else if (LuaMonsterScript) {
        player->ltAction->setLastAction(LuaMonsterScript, Source, Target, LongTimeAction::ACTION_USE);

        if ((paramOK) && player->IsAlive()) {
            if (Source.Type == LUA_CHARACTER && (Target.Type == LUA_NONE)) {
                LuaMonsterScript->useMonster(Source.character, player, static_cast<unsigned char>(LTS_NOLTACTION));
                msg = "Used Monster: " + Source.character->to_string();
            }
        }
    } else if (LuaTileScript) {
        player->ltAction->setLastAction(LuaTileScript, Source, Target, LongTimeAction::ACTION_USE);

        if ((paramOK) && player->IsAlive()) {
            if (Source.Type == LUA_FIELD && Target.Type == LUA_NONE) {
                LuaTileScript->useTile(player, Source.pos, static_cast<unsigned char>(LTS_NOLTACTION));
            }
        }
    }

    ServerCommandPointer cmd(new BBSendActionTC(player->getId(), player->getName(), 3,msg));
    World::get()->monitoringClientList->sendCommand(cmd);
}

ClientCommandPointer UseTS::clone() {
    ClientCommandPointer cmd(new UseTS());
    return cmd;
}

KeepAliveTS::KeepAliveTS() : BasicClientCommand(C_KEEPALIVE_TS) {
}

void KeepAliveTS::decodeData() {
}

void KeepAliveTS::performAction(Player *player) {
    Logger::debug(LogFacility::Player) << "KEEPALIVE_TS from player " << *player << Log::end;
    time(&(player->lastkeepalive));
}

ClientCommandPointer KeepAliveTS::clone() {
    ClientCommandPointer cmd(new KeepAliveTS());
    return cmd;
}

RequestSkillsTS::RequestSkillsTS() : BasicClientCommand(C_REQUESTSKILLS_TS) {
}

void RequestSkillsTS::decodeData() {
}

void RequestSkillsTS::performAction(Player *player) {
    time(&(player->lastaction));
    player->sendAllSkills();
}

ClientCommandPointer RequestSkillsTS::clone() {
    ClientCommandPointer cmd(new RequestSkillsTS());
    return cmd;
}

AttackStopTS::AttackStopTS() : BasicClientCommand(C_ATTACKSTOP_TS) {
}

void AttackStopTS::decodeData() {
}

void AttackStopTS::performAction(Player *player) {
    time(&(player->lastaction));
    player->ltAction->abortAction();
    player->attackmode = false;
    ServerCommandPointer cmd(new TargetLostTC());
    player->Connection->addCommand(cmd);
}

ClientCommandPointer AttackStopTS::clone() {
    ClientCommandPointer cmd(new AttackStopTS());
    return cmd;
}

LookAtInventoryItemTS::LookAtInventoryItemTS() : BasicClientCommand(C_LOOKATINVENTORYITEM_TS) {
}

void LookAtInventoryItemTS::decodeData() {
    pos = getUnsignedCharFromBuffer();
}

void LookAtInventoryItemTS::performAction(Player *player) {
    Logger::debug(LogFacility::World) << *player << " looks at an item in the inventory." << Log::end;

    if (player->IsAlive()) {
        World::get()->lookAtInventoryItem(player, pos);
        player->actionPoints -= P_LOOK_COST;
    }
}

ClientCommandPointer LookAtInventoryItemTS::clone() {
    ClientCommandPointer cmd(new LookAtInventoryItemTS());
    return cmd;
}

LookAtShowCaseItemTS::LookAtShowCaseItemTS() : BasicClientCommand(C_LOOKATSHOWCASEITEM_TS) {
}

void LookAtShowCaseItemTS::decodeData() {
    showcase = getUnsignedCharFromBuffer();
    pos = getUnsignedCharFromBuffer();
}

void LookAtShowCaseItemTS::performAction(Player *player) {
    Logger::debug(LogFacility::World) << *player << " looks at an item in a container." << Log::end;

    if (player->IsAlive()) {
        World::get()->lookAtShowcaseItem(player, showcase, pos);
        player->actionPoints -= P_LOOK_COST;
    }
}

ClientCommandPointer LookAtShowCaseItemTS::clone() {
    ClientCommandPointer cmd(new LookAtShowCaseItemTS());
    return cmd;
}

MoveItemFromPlayerToShowCaseTS::MoveItemFromPlayerToShowCaseTS() : BasicClientCommand(C_MOVEITEMFROMPLAYERTOSHOWCASE_TS) {
}

void MoveItemFromPlayerToShowCaseTS::decodeData() {
    cpos = getUnsignedCharFromBuffer();
    showcase = getUnsignedCharFromBuffer();
    pos = getUnsignedCharFromBuffer();
    count = getShortIntFromBuffer();
}

void MoveItemFromPlayerToShowCaseTS::performAction(Player *player) {
    time(&(player->lastaction));
    player->ltAction->abortAction();
    Logger::debug(LogFacility::World) << *player << "moves an item from the inventory to showcase!" << Log::end;

    if (player->IsAlive()) {
        World::get()->moveItemFromPlayerIntoShowcase(player, cpos, showcase, pos, count);
        player->actionPoints -= P_ITEMMOVE_COST;
    }
}

ClientCommandPointer MoveItemFromPlayerToShowCaseTS::clone() {
    ClientCommandPointer cmd(new MoveItemFromPlayerToShowCaseTS());
    return cmd;
}

MoveItemFromShowCaseToPlayerTS::MoveItemFromShowCaseToPlayerTS() : BasicClientCommand(C_MOVEITEMFROMSHOWCASETOPLAYER_TS) {
}

void MoveItemFromShowCaseToPlayerTS::decodeData() {
    showcase = getUnsignedCharFromBuffer();
    pos = getUnsignedCharFromBuffer();
    cpos = getUnsignedCharFromBuffer();
    count = getShortIntFromBuffer();
}

void MoveItemFromShowCaseToPlayerTS::performAction(Player *player) {
    time(&(player->lastaction));
    player->ltAction->abortAction();
    Logger::debug(LogFacility::World) << *player << " moves an item from the shocase to the inventory!" << Log::end;

    if (player->IsAlive()) {
        World::get()->moveItemFromShowcaseToPlayer(player, showcase, pos, cpos, count);
        player->actionPoints -= P_ITEMMOVE_COST;
    }
}

ClientCommandPointer MoveItemFromShowCaseToPlayerTS::clone() {
    ClientCommandPointer cmd(new MoveItemFromShowCaseToPlayerTS());
    return cmd;
}

MoveItemInsideInventoryTS::MoveItemInsideInventoryTS() : BasicClientCommand(C_MOVEITEMINSIDEINVENTORY_TS) {
}

void MoveItemInsideInventoryTS::decodeData() {
    opos = getUnsignedCharFromBuffer();
    npos = getUnsignedCharFromBuffer();
    count = getShortIntFromBuffer();
}

void MoveItemInsideInventoryTS::performAction(Player *player) {
    time(&(player->lastaction));
    player->ltAction->abortAction();
    Logger::debug(LogFacility::World) << *player << "moves an item inside the inventory!" << Log::end;

    if (player->IsAlive()) {
        World::get()->moveItemBetweenBodyParts(player, opos, npos, count);
        player->actionPoints -= P_ITEMMOVE_COST;
    }
}

ClientCommandPointer MoveItemInsideInventoryTS::clone() {
    ClientCommandPointer cmd(new MoveItemInsideInventoryTS());
    return cmd;
}

DropItemFromInventoryOnMapTS::DropItemFromInventoryOnMapTS() : BasicClientCommand(C_DROPITEMFROMPLAYERONMAP_TS) {
}

void DropItemFromInventoryOnMapTS::decodeData() {
    pos = getUnsignedCharFromBuffer();
    mapPosition.x = static_cast<short int>(getShortIntFromBuffer());
    mapPosition.y = static_cast<short int>(getShortIntFromBuffer());
    mapPosition.z = static_cast<short int>(getShortIntFromBuffer());
    count = getShortIntFromBuffer();
}

void DropItemFromInventoryOnMapTS::performAction(Player *player) {
    time(&(player->lastaction));
    player->ltAction->abortAction();
    Logger::debug(LogFacility::World) << *player << " throws an item from inventory on the map!" << Log::end;
    World::get()->dropItemFromPlayerOnMap(player, pos, mapPosition, count);
    player->actionPoints -= P_ITEMMOVE_COST;
}

ClientCommandPointer DropItemFromInventoryOnMapTS::clone() {
    ClientCommandPointer cmd(new DropItemFromInventoryOnMapTS());
    return cmd;
}

MoveItemFromMapToPlayerTS::MoveItemFromMapToPlayerTS() : BasicClientCommand(C_MOVEITEMFROMMAPTOPLAYER_TS) {
}

void MoveItemFromMapToPlayerTS::decodeData() {
    dir = to_direction(getUnsignedCharFromBuffer());
    pos = getUnsignedCharFromBuffer();
    count = getShortIntFromBuffer();
}

void MoveItemFromMapToPlayerTS::performAction(Player *player) {
    time(&(player->lastaction));
    player->ltAction->abortAction();
    Logger::debug(LogFacility::World) << *player << " moves an Item from the map to the inventory!" << Log::end;

    if (player->IsAlive()) {
        World::get()->moveItemFromMapToPlayer(player, dir, pos, count);
        player->actionPoints -= P_ITEMMOVE_COST;
    }
}

ClientCommandPointer MoveItemFromMapToPlayerTS::clone() {
    ClientCommandPointer cmd(new MoveItemFromMapToPlayerTS());
    return cmd;
}

MoveItemFromMapIntoShowCaseTS::MoveItemFromMapIntoShowCaseTS() : BasicClientCommand(C_MOVEITEMFROMMAPINTOSHOWCASE_TS) {
}

void MoveItemFromMapIntoShowCaseTS::decodeData() {
    dir = to_direction(getUnsignedCharFromBuffer());
    showcase = getUnsignedCharFromBuffer();
    pos = getUnsignedCharFromBuffer();
    count = getShortIntFromBuffer();
}

void MoveItemFromMapIntoShowCaseTS::performAction(Player *player) {
    time(&(player->lastaction));
    player->ltAction->abortAction();
    Logger::debug(LogFacility::World) << *player << " moves an item from the map to the showcase!" << Log::end;

    if (player->IsAlive()) {
        World::get()->moveItemFromMapIntoShowcase(player, dir, showcase, pos, count);
        player->actionPoints -= P_ITEMMOVE_COST;
    }
}

ClientCommandPointer MoveItemFromMapIntoShowCaseTS::clone() {
    ClientCommandPointer cmd(new MoveItemFromMapIntoShowCaseTS());
    return cmd;
}

MoveItemBetweenShowCasesTS::MoveItemBetweenShowCasesTS() : BasicClientCommand(C_MOVEITEMBETWEENSHOWCASES_TS) {
}

void MoveItemBetweenShowCasesTS::decodeData() {
    source = getUnsignedCharFromBuffer();
    spos = getUnsignedCharFromBuffer();
    dest = getUnsignedCharFromBuffer();
    dpos = getUnsignedCharFromBuffer();
    count = getShortIntFromBuffer();
}

void MoveItemBetweenShowCasesTS::performAction(Player *player) {
    time(&(player->lastaction));
    player->ltAction->abortAction();
    Logger::debug(LogFacility::World) << *player << " moves an item between showcases!" << Log::end;

    if (player->IsAlive()) {
        World::get()->moveItemBetweenShowcases(player, source, spos, dest, dpos, count);
        player->actionPoints -= P_ITEMMOVE_COST;
    }
}

ClientCommandPointer MoveItemBetweenShowCasesTS::clone() {
    ClientCommandPointer cmd(new MoveItemBetweenShowCasesTS());
    return cmd;
}

DropItemFromShowCaseOnMapTS::DropItemFromShowCaseOnMapTS() : BasicClientCommand(C_DROPITEMFROMSHOWCASEONMAP_TS) {
}

void DropItemFromShowCaseOnMapTS::decodeData() {
    showcase = getUnsignedCharFromBuffer();
    pos = getUnsignedCharFromBuffer();
    mapPosition.x = static_cast<short int>(getShortIntFromBuffer());
    mapPosition.y = static_cast<short int>(getShortIntFromBuffer());
    mapPosition.z = static_cast<short int>(getShortIntFromBuffer());
    count = getShortIntFromBuffer();
}

void DropItemFromShowCaseOnMapTS::performAction(Player *player) {
    time(&(player->lastaction));
    player->ltAction->abortAction();
    Logger::debug(LogFacility::World) << *player << " moves an item from showcase to the map!" << Log::end;
    World::get()->dropItemFromShowcaseOnMap(player, showcase, pos, mapPosition, count);
    player->actionPoints -= P_ITEMMOVE_COST;
}

ClientCommandPointer DropItemFromShowCaseOnMapTS::clone() {
    ClientCommandPointer cmd(new DropItemFromShowCaseOnMapTS());
    return cmd;
}

CloseContainerInShowCaseTS::CloseContainerInShowCaseTS() : BasicClientCommand(C_CLOSECONTAINERINSHOWCASE_TS) {
}

void CloseContainerInShowCaseTS::decodeData() {
    showcase = getUnsignedCharFromBuffer();
}

void CloseContainerInShowCaseTS::performAction(Player *player) {
    time(&(player->lastaction));
    player->ltAction->abortAction();
    Logger::debug(LogFacility::World) << *player << " closes a container in the showcase" << Log::end;

    if (player->IsAlive()) {
        player->closeShowcase(showcase);
    }
}

ClientCommandPointer CloseContainerInShowCaseTS::clone() {
    ClientCommandPointer cmd(new CloseContainerInShowCaseTS());
    return cmd;
}

LookIntoShowCaseContainerTS::LookIntoShowCaseContainerTS() : BasicClientCommand(C_LOOKINTOSHOWCASECONTAINER_TS) {
}

void LookIntoShowCaseContainerTS::decodeData() {
    showcase = getUnsignedCharFromBuffer();
    pos = getUnsignedCharFromBuffer();
}

void LookIntoShowCaseContainerTS::performAction(Player *player) {
    time(&(player->lastaction));
    player->ltAction->abortAction();
    Logger::debug(LogFacility::World) << *player << " looks into a container in a showcase!" << Log::end;
    player->lookIntoShowcaseContainer(showcase, pos);
    player->actionPoints -= P_LOOK_COST;
}

ClientCommandPointer LookIntoShowCaseContainerTS::clone() {
    ClientCommandPointer cmd(new LookIntoShowCaseContainerTS());
    return cmd;
}

LookIntoInventoryTS::LookIntoInventoryTS() : BasicClientCommand(C_LOOKINTOINVENTORY_TS) {
}

void LookIntoInventoryTS::decodeData() {
}

void LookIntoInventoryTS::performAction(Player *player) {
    time(&(player->lastaction));
    player->ltAction->abortAction();
    Logger::debug(LogFacility::World) << *player << " looks into his backpack" << Log::end;
    player->lookIntoBackPack();
    player->actionPoints -= P_LOOK_COST;
}

ClientCommandPointer LookIntoInventoryTS::clone() {
    ClientCommandPointer cmd(new LookIntoInventoryTS());
    return cmd;
}

LookIntoContainerOnFieldTS::LookIntoContainerOnFieldTS() : BasicClientCommand(C_LOOKINTOCONTAINERONFIELD_TS) {
}

void LookIntoContainerOnFieldTS::decodeData() {
    dir = to_direction(getUnsignedCharFromBuffer());
}

void LookIntoContainerOnFieldTS::performAction(Player *player) {
    time(&(player->lastaction));
    player->ltAction->abortAction();
    Logger::debug(LogFacility::World) << *player << " looks into a container on the map" << Log::end;

    if (player->IsAlive()) {
        player->lookIntoContainerOnField(dir);
        player->actionPoints -= P_LOOK_COST;
    }
}

ClientCommandPointer LookIntoContainerOnFieldTS::clone() {
    ClientCommandPointer cmd(new LookIntoContainerOnFieldTS());
    return cmd;
}

LogOutTS::LogOutTS() : BasicClientCommand(C_LOGOUT_TS) {
}

void LogOutTS::decodeData() {
}

void LogOutTS::performAction(Player *player) {
    player->ltAction->abortAction();
    Logger::info(LogFacility::Player) << *player << " loggt aus" << Log::end;
    player->Connection->closeConnection();
}

ClientCommandPointer LogOutTS::clone() {
    ClientCommandPointer cmd(new LogOutTS());
    return cmd;
}

WhisperTS::WhisperTS() : BasicClientCommand(C_WHISPER_TS) {
}

void WhisperTS::decodeData() {
    text = getStringFromBuffer();
}

void WhisperTS::performAction(Player *player) {
    time(&(player->lastaction));
    Logger::debug(LogFacility::World) << *player << " whispers something!" << Log::end;
    player->talk(Character::tt_whisper, text);
}

ClientCommandPointer WhisperTS::clone() {
    ClientCommandPointer cmd(new WhisperTS());
    return cmd;
}

ShoutTS::ShoutTS() : BasicClientCommand(C_SHOUT_TS) {
}

void ShoutTS::decodeData() {
    text = getStringFromBuffer();
}

void ShoutTS::performAction(Player *player) {
    time(&(player->lastaction));
    player->talk(Character::tt_yell, text);
}

ClientCommandPointer ShoutTS::clone() {
    ClientCommandPointer cmd(new ShoutTS());
    return cmd;
}

SayTS::SayTS() : BasicClientCommand(C_SAY_TS) {
}

void SayTS::decodeData() {
    text = getStringFromBuffer();
}

void SayTS::performAction(Player *player) {
    time(&(player->lastaction));
    Logger::debug(LogFacility::World) << *player << " whispers something!" << Log::end;

    if (!World::get()->parseGMCommands(player, text)) {
        if (!World::get()->parsePlayerCommands(player, text)) {    // did we issue a player command?
            player->talk(Character::tt_say, text);
        }
    }
}

ClientCommandPointer SayTS::clone() {
    ClientCommandPointer cmd(new SayTS());
    return cmd;
}

RefreshTS::RefreshTS() : BasicClientCommand(C_REFRESH_TS) {
}

void RefreshTS::decodeData() {
}

void RefreshTS::performAction(Player *player) {
    Logger::debug(LogFacility::World) << *player << " want sended a refresh_ts, sending map!" << Log::end;
    player->sendFullMap();
    World::get()->sendAllVisibleCharactersToPlayer(player, true);
}

ClientCommandPointer RefreshTS::clone() {
    ClientCommandPointer cmd(new RefreshTS());
    return cmd;
}

IntroduceTS::IntroduceTS() : BasicClientCommand(C_INTRODUCE_TS) {
}

void IntroduceTS::decodeData() {
}

void IntroduceTS::performAction(Player *player) {
    time(&(player->lastaction));
    Logger::debug(LogFacility::World) << *player << " introduces himself!" << Log::end;

    if (player->IsAlive()) {
        World::get()->introduceMyself(player);
    }
}

ClientCommandPointer IntroduceTS::clone() {
    ClientCommandPointer cmd(new IntroduceTS());
    return cmd;
}

AttackPlayerTS::AttackPlayerTS() : BasicClientCommand(C_ATTACKPLAYER_TS) {
}

void AttackPlayerTS::decodeData() {
    enemyid = getIntFromBuffer();
}

void AttackPlayerTS::performAction(Player *player) {
    time(&(player->lastaction));

    if (player->IsAlive() && player->GetStatus() < 10) {
        player->ltAction->abortAction();

        if (player->IsAlive()) {
            player->enemyid = enemyid;
            player->attackmode = true;
            player->enemytype = Character::player;

            if (player->enemyid >= MONSTER_BASE) {
                player->enemytype = Character::monster;
            }

            if (player->enemyid >= NPC_BASE) {
                player->enemytype = Character::npc;
            }

            ServerCommandPointer cmd(new AttackAcknowledgedTC());
            player->Connection->addCommand(cmd);
            //monitoringClientList->sendCommand( new SendActionTS(player->getId(), player->getName(), 0, "Starts an attack: " + Logger::toString(player->enemyid) ) );
            World::get()->characterAttacks(player);
        } else {
            player->attackmode = false;
        }
    }
}

ClientCommandPointer AttackPlayerTS::clone() {
    ClientCommandPointer cmd(new AttackPlayerTS());
    return cmd;
}

LookAtMapItemTS::LookAtMapItemTS() : BasicClientCommand(C_LOOKATMAPITEM_TS) {
}

void LookAtMapItemTS::decodeData() {
    pos.x = getShortIntFromBuffer();
    pos.y = getShortIntFromBuffer();
    pos.z = getShortIntFromBuffer();
}

void LookAtMapItemTS::performAction(Player *player) {
    Logger::debug(LogFacility::World) << *player << " looks at a map item." << Log::end;

    if (player->IsAlive()) {
        World::get()->lookAtMapItem(player, pos);
        player->actionPoints -= P_LOOK_COST;
    }

}

ClientCommandPointer LookAtMapItemTS::clone() {
    ClientCommandPointer cmd(new LookAtMapItemTS());
    return cmd;
}

PSpinActionTS::PSpinActionTS(uint8_t dir) : BasicClientCommand(C_PSPINRSTART_TS) {
    direction = dir;
}

void PSpinActionTS::decodeData() {
}

void PSpinActionTS::performAction(Player *player) {
    time(&(player->lastaction));
    player->ltAction->abortAction();
    Logger::debug(LogFacility::World) << *player << " changes his dircetion to " << direction << Log::end;

    if (World::get()->spinPlayer(player, direction)) {
        player->actionPoints -= P_SPIN_COST;
    }
}

ClientCommandPointer PSpinActionTS::clone() {
    ClientCommandPointer cmd(new PSpinActionTS(direction));
    return cmd;
}

CharMoveTS::CharMoveTS() : BasicClientCommand(C_CHARMOVE_TS) {
}

void CharMoveTS::decodeData() {
    charid = getIntFromBuffer();
    dir = getUnsignedCharFromBuffer();
    mode = getUnsignedCharFromBuffer();
}

void CharMoveTS::performAction(Player *player) {
    time(&(player->lastaction));

    if (charid == player->getId() && (mode == NORMALMOVE || mode == RUNNING)) {
        player->ltAction->abortAction();
        Logger::debug(LogFacility::World) << "Playermove by " << *player << Log::end;

        if (player->getTurtleActive() && player->hasGMRight(gmr_settiles) && mode == NORMALMOVE) {
            World::get()->setNextTile(player, player->getTurtleTile());
            Logger::info(LogFacility::World) << "Turtle was active, new tile set at pos: " << player->pos.x << "," << player->pos.y << "," << player->pos.z << " tile: " << player->getTurtleTile() << Log::end;
        }

        if (player->move(static_cast<direction>(dir), mode)) {
            player->closeAllShowcasesOfMapContainers();
        }
    } else if (mode == PUSH) {
        player->ltAction->abortAction();
        Logger::debug(LogFacility::World) << "Player pushes another: " << *player << Log::end;

        if (player->IsAlive()) {
            if (World::get()->pushCharacter(player, charid, static_cast<direction>(dir))) {
                player->actionPoints -= P_PUSH_COST;
            }
        }
    }
}

ClientCommandPointer CharMoveTS::clone() {
    ClientCommandPointer cmd(new CharMoveTS());
    return cmd;
}

IMoverActionTS::IMoverActionTS(uint8_t dir) : BasicClientCommand(C_IMOVERSTART_TS) {
    this->dir = to_direction(dir);
}

void IMoverActionTS::decodeData() {
    pos.x = getShortIntFromBuffer();
    pos.y = getShortIntFromBuffer();
    pos.z = getShortIntFromBuffer();
    count = getShortIntFromBuffer();
}

void IMoverActionTS::performAction(Player *player) {
    time(&(player->lastaction));
    player->ltAction->abortAction();
    Logger::debug(LogFacility::World) << *player << " tryes to move an Item!" << Log::end;

    if (player->IsAlive()) {
        World::get()->moveItem(player, dir, pos, count);
        player->actionPoints -= P_ITEMMOVE_COST;
    }
}

ClientCommandPointer IMoverActionTS::clone() {
    ClientCommandPointer cmd(new IMoverActionTS(dir));
    return cmd;
}

LoginCommandTS::LoginCommandTS() : BasicClientCommand(C_LOGIN_TS) {
}

void LoginCommandTS::decodeData() {
    clientVersion = getUnsignedCharFromBuffer();
    loginName = getStringFromBuffer();
    password = getStringFromBuffer();
}

void LoginCommandTS::performAction(Player *player) {
    time(&(player->lastaction));
}

ClientCommandPointer LoginCommandTS::clone() {
    ClientCommandPointer cmd(new LoginCommandTS());
    return cmd;
}

unsigned short LoginCommandTS::getClientVersion() const {
    return clientVersion;
}

const std::string &LoginCommandTS::getLoginName() const {
    return loginName;
}

const std::string &LoginCommandTS::getPassword() const {
    return password;
}

ScreenSizeCommandTS::ScreenSizeCommandTS() : BasicClientCommand(C_SCREENSIZE_TS) {
}

void ScreenSizeCommandTS::decodeData() {
    width = getUnsignedCharFromBuffer();
    height = getUnsignedCharFromBuffer();
}

void ScreenSizeCommandTS::performAction(Player *player) {
    player->screenwidth = width;
    player->screenheight = height;
    player->sendFullMap();
    player->sendCharacters();
}

ClientCommandPointer ScreenSizeCommandTS::clone() {
    ClientCommandPointer cmd(new ScreenSizeCommandTS());
    return cmd;
}

