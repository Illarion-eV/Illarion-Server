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

#include "netinterface/protocol/ClientCommands.hpp"

#include "Character.hpp"
#include "Logger.hpp"
#include "LongTimeAction.hpp"
#include "MonitoringClients.hpp"
#include "Monster.hpp"
#include "NPC.hpp"
#include "Player.hpp"
#include "World.hpp"
#include "data/Data.hpp"
#include "data/MonsterTable.hpp"
#include "map/Field.hpp"
#include "netinterface/protocol/BBIWIServerCommands.hpp"
#include "netinterface/protocol/ServerCommands.hpp"
#include "script/LuaItemScript.hpp"
#include "script/LuaMagicScript.hpp"
#include "script/LuaMonsterScript.hpp"
#include "script/LuaNPCScript.hpp"
#include "script/LuaScript.hpp"
#include "script/LuaTileScript.hpp"
#include "script/server.hpp"
#include "tuningConstants.hpp"
#include "types.hpp"

#include <string>

extern MonsterTable *monsterDescriptions;

InputDialogTS::InputDialogTS() : BasicClientCommand(C_INPUTDIALOG_TS) {}

void InputDialogTS::decodeData() {
    dialogId = getIntFromBuffer();
    success = getUnsignedCharFromBuffer() > 0;
    input = getStringFromBuffer();
}

void InputDialogTS::performAction(Player *player) {
    time(&(player->lastaction));
    player->executeInputDialog(dialogId, success, input);
}

auto InputDialogTS::clone() -> ClientCommandPointer {
    ClientCommandPointer cmd = std::make_shared<InputDialogTS>();
    return cmd;
}

MessageDialogTS::MessageDialogTS() : BasicClientCommand(C_MESSAGEDIALOG_TS) {}

void MessageDialogTS::decodeData() { dialogId = getIntFromBuffer(); }

void MessageDialogTS::performAction(Player *player) {
    time(&(player->lastaction));
    player->executeMessageDialog(dialogId);
}

auto MessageDialogTS::clone() -> ClientCommandPointer {
    ClientCommandPointer cmd = std::make_shared<MessageDialogTS>();
    return cmd;
}

MerchantDialogTS::MerchantDialogTS() : BasicClientCommand(C_MERCHANTDIALOG_TS) {}

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

    case 3:
        lookAtList = getUnsignedCharFromBuffer();
        lookAtSlot = getUnsignedCharFromBuffer();
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

    case 3:
        player->executeMerchantDialogLookAt(dialogId, lookAtList, lookAtSlot);
        break;
    }
}

auto MerchantDialogTS::clone() -> ClientCommandPointer {
    ClientCommandPointer cmd = std::make_shared<MerchantDialogTS>();
    return cmd;
}

SelectionDialogTS::SelectionDialogTS() : BasicClientCommand(C_SELECTIONDIALOG_TS) {}

void SelectionDialogTS::decodeData() {
    dialogId = getIntFromBuffer();
    success = getUnsignedCharFromBuffer() > 0;
    selectedIndex = getUnsignedCharFromBuffer();
}

void SelectionDialogTS::performAction(Player *player) {
    time(&(player->lastaction));
    player->executeSelectionDialog(dialogId, success, selectedIndex);
}

auto SelectionDialogTS::clone() -> ClientCommandPointer {
    ClientCommandPointer cmd = std::make_shared<SelectionDialogTS>();
    return cmd;
}

CraftingDialogTS::CraftingDialogTS() : BasicClientCommand(C_CRAFTINGDIALOG_TS) {}

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

auto CraftingDialogTS::clone() -> ClientCommandPointer {
    ClientCommandPointer cmd = std::make_shared<CraftingDialogTS>();
    return cmd;
}

RequestAppearanceTS::RequestAppearanceTS() : BasicClientCommand(C_REQUESTAPPEARANCE_TS) {}

void RequestAppearanceTS::decodeData() { id = getIntFromBuffer(); }

void RequestAppearanceTS::performAction(Player *player) {
    Character *ch = World::get()->Players.find(id);

    if (ch == nullptr) {
        ch = World::get()->Monsters.find(id);

        if (ch == nullptr) {
            ch = World::get()->Npc.find(id);
        }
    }

    // check if we found a character befor
    if (ch != nullptr) {
        ch->updateAppearanceForPlayer(player, true);
    }
}

auto RequestAppearanceTS::clone() -> ClientCommandPointer {
    ClientCommandPointer cmd = std::make_shared<RequestAppearanceTS>();
    return cmd;
}

LookAtCharacterTS::LookAtCharacterTS() : BasicClientCommand(C_LOOKATCHARACTER_TS, P_LOOK_COST) {}

void LookAtCharacterTS::decodeData() {
    id = getIntFromBuffer();
    mode = getUnsignedCharFromBuffer();
}

void LookAtCharacterTS::performAction(Player *player) {
    if (id < MONSTER_BASE) {
        Player *pl = World::get()->Players.find(id);

        if (pl != nullptr) {
            script::server::lookAtPlayer().lookAtPlayer(player, pl, mode);
        }

        // code for player handling
    } else if (id < NPC_BASE) {
        Monster *monster = World::get()->Monsters.find(id);

        if (monster != nullptr) {
            const auto monsterType = monster->getMonsterType();

            if (monsterDescriptions->exists(monsterType)) {
                const auto &mon = (*monsterDescriptions)[monsterType];

                if (mon.script && mon.script->existsEntrypoint("lookAtMonster")) {
                    mon.script->lookAtMonster(player, monster, mode);
                    return;
                }

                std::string german = "Das ist: " + mon.nameDe;
                std::string english = "This is: " + mon.nameEn;

                ServerCommandPointer cmd = std::make_shared<CharDescription>(id, player->nls(german, english));
                player->Connection->addCommand(cmd);
            }
        }
    } else {
        NPC *npc = World::get()->Npc.find(id);

        if (npc != nullptr) {
            if (npc->getScript() && npc->getScript()->existsEntrypoint("lookAtNpc")) {
                npc->getScript()->lookAtNpc(player, mode);
                return;
            }

            std::string german = "Das ist: " + npc->getName();
            std::string english = "This is: " + npc->getName();

            ServerCommandPointer cmd = std::make_shared<CharDescription>(id, player->nls(german, english));
            player->Connection->addCommand(cmd);
        }
    }
}

auto LookAtCharacterTS::clone() -> ClientCommandPointer {
    ClientCommandPointer cmd = std::make_shared<LookAtCharacterTS>();
    return cmd;
}

CastTS::CastTS() : BasicClientCommand(C_CAST_TS) {}

void CastTS::decodeData() {
    spellId = static_cast<unsigned long int>(getIntFromBuffer());
    cid = getUnsignedCharFromBuffer();

    switch (cid) {
    case UID_KOORD:
        castPosition.x = getShortIntFromBuffer();
        castPosition.y = getShortIntFromBuffer();
        castPosition.z = getShortIntFromBuffer();
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

    std::shared_ptr<LuaMagicScript> LuaMageScript;

    if ((spellId & player->getMagicFlags(player->getMagicType())) == spellId) {
        Spell spell{};
        spell.magicType = player->getMagicType();
        spell.spellId = spellId;

        if (Data::spells().exists(spell)) {
            LuaMageScript = Data::spells().script(spell);
        }
    }

    SouTar Source;
    Source.character = dynamic_cast<Character *>(player);
    Source.pos = player->getPosition();
    Source.Type = LUA_CHARACTER;

    switch (cid) {
    case UID_KOORD:
        Logger::debug(LogFacility::Script)
                << *player << " trys to cast on a coordinate pos " << castPosition << Log::end;

        if (LuaMageScript) {
            try {
                map::Field &field = World::get()->fieldAt(castPosition);

                if (field.hasPlayer() || field.hasMonster() || field.hasNPC()) {
                    Character *tmpCharacter = World::get()->findCharacterOnField(castPosition);

                    if (tmpCharacter != nullptr) {
                        Logger::debug(LogFacility::Script) << "Character found at target field!" << Log::end;

                        if (tmpCharacter->getType() == Character::player) {
                            Logger::debug(LogFacility::Script) << "Target Character: player" << Log::end;
                            Source.character = tmpCharacter;
                            Source.pos = tmpCharacter->getPosition();
                            Source.Type = LUA_CHARACTER;
                        } else if (tmpCharacter->getType() == Character::npc) {
                            Logger::debug(LogFacility::Script) << "Target Character: NPC" << Log::end;
                            Source.character = tmpCharacter;
                            Source.pos = tmpCharacter->getPosition();
                            Source.Type = LUA_CHARACTER;
                        } else if (tmpCharacter->getType() == Character::monster) {
                            Logger::debug(LogFacility::Script) << "Target Character: monster" << Log::end;
                            Source.character = tmpCharacter;
                            Source.pos = tmpCharacter->getPosition();
                            Source.Type = LUA_CHARACTER;
                        }
                    }
                } else {
                    Item item;

                    if (field.viewItemOnStack(item)) {
                        Logger::debug(LogFacility::Script) << "Item found at target field!" << Log::end;
                        Source.pos = castPosition;
                        Source.Type = LUA_ITEM;
                        Source.item = static_cast<ScriptItem>(item);
                        Source.item.type = ScriptItem::it_field;
                        Source.item.pos = castPosition;
                        Source.item.owner = player;
                    } else {
                        Logger::debug(LogFacility::Script) << "empty field!" << Log::end;
                        Source.pos = castPosition;
                        Source.Type = LUA_FIELD;
                    }
                }
            } catch (FieldNotFound &) {
                Logger::error(LogFacility::Script) << "cant find field for casting at pos " << castPosition << Log::end;
                paramOK = false;
            }
        } else {
            paramOK = false;
        }

        break;

    case UID_SHOWC:

        Logger::debug(LogFacility::Script)
                << *player << " is casting in showcas: " << showcase << " pos " << pos << Log::end;

        if (LuaMageScript) {
            if (player->isShowcaseOpen(showcase)) {
                Container *ps = player->getShowcaseContainer(showcase);

                if (ps != nullptr) {
                    ScriptItem tempi;
                    Container *tempc = nullptr;

                    if (ps->viewItemNr(pos, tempi, tempc)) {
                        Source.Type = LUA_ITEM;
                        ps->viewItemNr(pos, Source.item, tempc);
                        Source.item.pos = castPosition;
                        Source.item.type = ScriptItem::it_container;
                        Source.item.itempos = pos;
                        Source.item.owner = player;
                        Source.item.inside = ps;
                        Source.pos = castPosition;
                    } else {
                        paramOK = false;
                    }
                } else {
                    paramOK = false;
                }
            } else {
                paramOK = false;
            }
        }

        break;

    case UID_MAGICWAND:
        Logger::debug(LogFacility::Script) << "Cast with Wand" << Log::end;

        if (player->getAttackMode() && player->enemyid != 0 && LuaMageScript) {
            bool zauberstab = false;

            if ((player->items[LEFT_TOOL].getId() != 0) && (player->items[LEFT_TOOL].getId() != BLOCKEDITEM)) {
                const auto weaponId = player->items[LEFT_TOOL].getId();

                if (Data::weaponItems().exists(weaponId)) {
                    if (Data::weaponItems()[weaponId].Type == WeaponStruct::stave) {
                        zauberstab = true;
                    }
                }
            }

            if ((player->items[RIGHT_TOOL].getId() != 0) && (player->items[RIGHT_TOOL].getId() != BLOCKEDITEM)) {
                const auto weaponId = player->items[RIGHT_TOOL].getId();

                if (Data::weaponItems().exists(weaponId)) {
                    if (Data::weaponItems()[weaponId].Type == WeaponStruct::stave) {
                        zauberstab = true;
                    }
                }
            }

            if (zauberstab) {
                Source.Type = LUA_CHARACTER;
                Source.character = World::get()->findCharacter(player->enemyid);

                if (Source.character != nullptr) {
                    Source.pos = Source.character->getPosition();
                } else {
                    paramOK = false;
                }
            } else {
                paramOK = false;
            }

        } // enemyid != 0
        else {
            if (!LuaMageScript) {
                paramOK = false;
            }
        }

        break;

    case UID_INV:

        Logger::debug(LogFacility::Script) << "cast in inventory" << Log::end;

        if (LuaMageScript) {
            if (pos < (MAX_BELT_SLOTS + MAX_BODY_ITEMS)) {
                if (player->items.at(pos).getId() != 0) {
                    Source.Type = LUA_ITEM;
                    Source.item = static_cast<ScriptItem>(player->items.at(pos));
                    Source.item.pos = player->getPosition();

                    if (pos < MAX_BODY_ITEMS) {
                        Source.item.type = ScriptItem::it_inventory;
                    } else {
                        Source.item.type = ScriptItem::it_belt;
                    }

                    Source.item.itempos = pos;
                    Source.item.owner = player;
                    Source.pos = player->getPosition();
                } else {
                    paramOK = false;
                }
            } else {
                paramOK = false;
            }
        }

        break;
    }

    if (LuaMageScript) {
        Logger::debug(LogFacility::Script) << "try to call magic script" << Log::end;
        player->ltAction->setLastAction(LuaMageScript, Source, LongTimeAction::ACTION_MAGIC);

        if ((paramOK) && player->isAlive()) {
            switch (Source.Type) {
            case LUA_NONE:
                LuaMageScript->CastMagic(player, static_cast<unsigned char>(LTS_NOLTACTION));
                break;

            case LUA_FIELD:
                LuaMageScript->CastMagicOnField(player, Source.pos, static_cast<unsigned char>(LTS_NOLTACTION));
                break;

            case LUA_CHARACTER:
                LuaMageScript->CastMagicOnCharacter(player, Source.character,
                                                    static_cast<unsigned char>(LTS_NOLTACTION));

                if (Source.character->getType() == Character::monster) {
                    auto *temp = dynamic_cast<Monster *>(Source.character);
                    const auto monsterType = temp->getMonsterType();

                    if (monsterDescriptions->exists(monsterType)) {
                        const auto &monStruct = (*monsterDescriptions)[monsterType];

                        if (monStruct.script) {
                            monStruct.script->onCasted(temp, player);
                        }
                    }
                }

                break;

            case LUA_ITEM:
                LuaMageScript->CastMagicOnItem(player, Source.item, static_cast<unsigned char>(LTS_NOLTACTION));
                break;

            default:
                LuaMageScript->CastMagic(player, static_cast<unsigned char>(LTS_NOLTACTION));
                break;
            }
        }

        Logger::debug(LogFacility::Script) << "all succeeded" << Log::end;
    }
}

auto CastTS::clone() -> ClientCommandPointer {
    ClientCommandPointer cmd = std::make_shared<CastTS>();
    return cmd;
}

UseTS::UseTS() : BasicClientCommand(C_USE_TS, P_MIN_AP) {}

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
    SouTar Source;
    ItemStruct com;

    switch (useId) {
    case UID_KOORD:

        Logger::debug(LogFacility::Script) << "UID_KOORD" << Log::end;
        Logger::debug(LogFacility::Script) << usePosition << Log::end;

        try {
            map::Field &field = World::get()->fieldAt(usePosition);

            if (field.hasPlayer() || field.hasNPC() || field.hasMonster()) {
                Logger::debug(LogFacility::Script) << "Character on field found!" << Log::end;
                Character *tmpCharacter = World::get()->findCharacterOnField(usePosition);

                if (tmpCharacter != nullptr) {
                    if (tmpCharacter->getType() == Character::player) {
                        Logger::debug(LogFacility::Script) << "Character is a player!" << Log::end;
                    } else if (tmpCharacter->getType() == Character::npc) {
                        Logger::debug(LogFacility::Script) << "Character is a NPC!" << Log::end;

                        NPC *scriptNPC = dynamic_cast<NPC *>(tmpCharacter);
                        LuaNPCScript = scriptNPC->getScript();

                        if (LuaNPCScript) {
                            Source.pos = scriptNPC->getPosition();
                            Source.character = scriptNPC;
                            Source.Type = LUA_CHARACTER;
                        }
                    } else if (tmpCharacter->getType() == Character::monster) {
                        Logger::debug(LogFacility::Script) << "Character is a monster!" << Log::end;

                        auto *scriptMonster = dynamic_cast<Monster *>(tmpCharacter);
                        const auto monsterType = scriptMonster->getMonsterType();

                        if (monsterDescriptions->exists(monsterType)) {
                            LuaMonsterScript = (*monsterDescriptions)[monsterType].script;
                        } else {
                            Logger::error(LogFacility::Script)
                                    << "try to use Monster but id: " << scriptMonster->getMonsterType()
                                    << " not found in database!" << Log::end;
                        }

                        if (LuaMonsterScript) {
                            Source.pos = scriptMonster->getPosition();
                            Source.character = scriptMonster;
                            Source.Type = LUA_CHARACTER;
                        }
                    }
                } else {
                    Logger::error(LogFacility::Script)
                            << "Character on field " << usePosition << " not found!" << Log::end;
                }
            } else {
                Logger::debug(LogFacility::Script) << "no character on field!" << Log::end;
                Item item;

                if (field.viewItemOnStack(item)) {
                    Logger::debug(LogFacility::Script) << "Item on field" << Log::end;

                    LuaScript = Data::items().script(item.getId());

                    if (LuaScript) {
                        Source.Type = LUA_ITEM;
                        field.viewItemOnStack(Source.item);
                        Source.item.pos = usePosition;
                        Source.item.type = ScriptItem::it_field;
                        Source.item.owner = player;
                        Source.pos = usePosition;
                    }
                } else {
                    Logger::debug(LogFacility::Script) << "empty field!" << Log::end;

                    const auto &script = Data::tiles().script(field.getTileId());

                    if (script) {
                        LuaTileScript = script;
                        Source.Type = LUA_FIELD;
                        Source.pos = usePosition;
                    }
                }
            }
        } catch (FieldNotFound &) {
            Logger::debug(LogFacility::Script) << "Use UID_KOORD field not found!" << Log::end;
            Logger::debug(LogFacility::Script) << "Use UID_KOORD field not found at pos " << usePosition << Log::end;
            paramOK = false;
        }

        break;

    case UID_SHOWC:
        Logger::debug(LogFacility::Script)
                << "showcase: " << static_cast<int>(showcase) << " pos: " << static_cast<int>(pos) << Log::end;

        if (player->isShowcaseOpen(showcase)) {
            Container *ps = player->getShowcaseContainer(showcase);

            if (ps != nullptr) {
                Logger::debug(LogFacility::Script) << "Container gefunden!" << Log::end;
                ScriptItem tempi;
                Container *tempc = nullptr;

                if (ps->viewItemNr(pos, tempi, tempc)) {
                    Logger::debug(LogFacility::Script) << "pos found item id: " << tempi.getId() << Log::end;

                    LuaScript = Data::items().script(tempi.getId());

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

            if (player->items.at(pos).getId() != 0) {
                Logger::debug(LogFacility::Script)
                        << "at position " << static_cast<int>(pos)
                        << " on body, is an item with id: " << player->items.at(pos).getId() << Log::end;

                LuaScript = Data::items().script(player->items.at(pos).getId());

                if (LuaScript) {
                    Source.Type = LUA_ITEM;
                    Source.item = static_cast<ScriptItem>(player->items.at(pos));
                    Source.item.pos = player->getPosition();

                    if (pos < MAX_BODY_ITEMS) {
                        Source.item.type = ScriptItem::it_inventory;
                    } else {
                        Source.item.type = ScriptItem::it_belt;
                    }

                    Source.item.itempos = pos;
                    Source.item.owner = player;
                    Source.pos = player->getPosition();
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
    Logger::debug(LogFacility::Script) << "Source pos (" << Source.pos.x << "," << Source.pos.y << "," << Source.pos.z
                                       << ")" << Log::end;
    Logger::debug(LogFacility::Script) << "Source type: " << Source.Type << Log::end;
    Logger::debug(LogFacility::Script) << "Source Character: " << *Source.character << Log::end;
    Logger::debug(LogFacility::Script) << "==========Use Script End=============" << Log::end;

    std::string msg;

    if (LuaScript) {
        player->ltAction->setLastAction(LuaScript, Source, LongTimeAction::ACTION_USE);

        if ((paramOK) && player->isAlive()) {
            if (Source.Type == LUA_ITEM) {
                LuaScript->UseItem(player, Source.item, static_cast<unsigned char>(LTS_NOLTACTION));
                msg = "Used Item: " + std::to_string(Source.item.getId());
            }
        }
    } else if (LuaNPCScript) {
        player->ltAction->setLastAction(LuaNPCScript, Source, LongTimeAction::ACTION_USE);

        if ((paramOK) && player->isAlive()) {
            if (Source.Type == LUA_CHARACTER) {
                LuaNPCScript->useNPC(player, static_cast<unsigned char>(LTS_NOLTACTION));
                msg = "Used NPC: " + Source.character->to_string();
            }
        }

    } else if (LuaMonsterScript) {
        player->ltAction->setLastAction(LuaMonsterScript, Source, LongTimeAction::ACTION_USE);

        if ((paramOK) && player->isAlive()) {
            if (Source.Type == LUA_CHARACTER) {
                LuaMonsterScript->useMonster(Source.character, player, static_cast<unsigned char>(LTS_NOLTACTION));
                msg = "Used Monster: " + Source.character->to_string();
            }
        }
    } else if (LuaTileScript) {
        player->ltAction->setLastAction(LuaTileScript, Source, LongTimeAction::ACTION_USE);

        if ((paramOK) && player->isAlive()) {
            if (Source.Type == LUA_FIELD) {
                LuaTileScript->useTile(player, Source.pos, static_cast<unsigned char>(LTS_NOLTACTION));
            }
        }
    }

    ServerCommandPointer cmd = std::make_shared<BBSendActionTC>(player->getId(), 3, msg);
    World::get()->monitoringClientList->sendCommand(cmd);
}

auto UseTS::clone() -> ClientCommandPointer {
    ClientCommandPointer cmd = std::make_shared<UseTS>();
    return cmd;
}

KeepAliveTS::KeepAliveTS() : BasicClientCommand(C_KEEPALIVE_TS) {}

void KeepAliveTS::decodeData() {}

void KeepAliveTS::performAction(Player *player) {
    Logger::debug(LogFacility::Player) << "KEEPALIVE_TS from player " << *player << Log::end;
    time(&(player->lastkeepalive));
    ServerCommandPointer cmd = std::make_shared<KeepAliveTC>();
    player->Connection->addCommand(cmd);
}

auto KeepAliveTS::clone() -> ClientCommandPointer {
    ClientCommandPointer cmd = std::make_shared<KeepAliveTS>();
    return cmd;
}

RequestSkillsTS::RequestSkillsTS() : BasicClientCommand(C_REQUESTSKILLS_TS) {}

void RequestSkillsTS::decodeData() {}

void RequestSkillsTS::performAction(Player *player) {
    time(&(player->lastaction));
    player->sendAllSkills();
}

auto RequestSkillsTS::clone() -> ClientCommandPointer {
    ClientCommandPointer cmd = std::make_shared<RequestSkillsTS>();
    return cmd;
}

AttackStopTS::AttackStopTS() : BasicClientCommand(C_ATTACKSTOP_TS) {}

void AttackStopTS::decodeData() {}

void AttackStopTS::performAction(Player *player) {
    time(&(player->lastaction));
    player->ltAction->abortAction();
    player->setAttackMode(false);
    ServerCommandPointer cmd = std::make_shared<TargetLostTC>();
    player->Connection->addCommand(cmd);
}

auto AttackStopTS::clone() -> ClientCommandPointer {
    ClientCommandPointer cmd = std::make_shared<AttackStopTS>();
    return cmd;
}

LookAtInventoryItemTS::LookAtInventoryItemTS() : BasicClientCommand(C_LOOKATINVENTORYITEM_TS, P_LOOK_COST) {}

void LookAtInventoryItemTS::decodeData() { pos = getUnsignedCharFromBuffer(); }

void LookAtInventoryItemTS::performAction(Player *player) {
    Logger::debug(LogFacility::World) << *player << " looks at an item in the inventory." << Log::end;

    if (player->isAlive()) {
        World::get()->lookAtInventoryItem(player, pos);
        player->increaseActionPoints(-P_LOOK_COST);
    }
}

auto LookAtInventoryItemTS::clone() -> ClientCommandPointer {
    ClientCommandPointer cmd = std::make_shared<LookAtInventoryItemTS>();
    return cmd;
}

LookAtShowCaseItemTS::LookAtShowCaseItemTS() : BasicClientCommand(C_LOOKATSHOWCASEITEM_TS, P_LOOK_COST) {}

void LookAtShowCaseItemTS::decodeData() {
    showcase = getUnsignedCharFromBuffer();
    pos = getUnsignedCharFromBuffer();
}

void LookAtShowCaseItemTS::performAction(Player *player) {
    Logger::debug(LogFacility::World) << *player << " looks at an item in a container." << Log::end;

    if (player->isAlive()) {
        World::get()->lookAtShowcaseItem(player, showcase, pos);
        player->increaseActionPoints(-P_LOOK_COST);
    }
}

auto LookAtShowCaseItemTS::clone() -> ClientCommandPointer {
    ClientCommandPointer cmd = std::make_shared<LookAtShowCaseItemTS>();
    return cmd;
}

MoveItemFromPlayerToShowCaseTS::MoveItemFromPlayerToShowCaseTS()
        : BasicClientCommand(C_MOVEITEMFROMPLAYERTOSHOWCASE_TS, P_ITEMMOVE_COST) {}

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

    if (player->isAlive()) {
        World::get()->moveItemFromPlayerIntoShowcase(player, cpos, showcase, pos, count);
        player->increaseActionPoints(-P_ITEMMOVE_COST);
    }
}

auto MoveItemFromPlayerToShowCaseTS::clone() -> ClientCommandPointer {
    ClientCommandPointer cmd = std::make_shared<MoveItemFromPlayerToShowCaseTS>();
    return cmd;
}

MoveItemFromShowCaseToPlayerTS::MoveItemFromShowCaseToPlayerTS()
        : BasicClientCommand(C_MOVEITEMFROMSHOWCASETOPLAYER_TS, P_ITEMMOVE_COST) {}

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

    if (player->isAlive()) {
        World::get()->moveItemFromShowcaseToPlayer(player, showcase, pos, cpos, count);
        player->increaseActionPoints(-P_ITEMMOVE_COST);
    }
}

auto MoveItemFromShowCaseToPlayerTS::clone() -> ClientCommandPointer {
    ClientCommandPointer cmd = std::make_shared<MoveItemFromShowCaseToPlayerTS>();
    return cmd;
}

MoveItemInsideInventoryTS::MoveItemInsideInventoryTS()
        : BasicClientCommand(C_MOVEITEMINSIDEINVENTORY_TS, P_ITEMMOVE_COST) {}

void MoveItemInsideInventoryTS::decodeData() {
    opos = getUnsignedCharFromBuffer();
    npos = getUnsignedCharFromBuffer();
    count = getShortIntFromBuffer();
}

void MoveItemInsideInventoryTS::performAction(Player *player) {
    time(&(player->lastaction));
    player->ltAction->abortAction();
    Logger::debug(LogFacility::World) << *player << "moves an item inside the inventory!" << Log::end;

    if (player->isAlive()) {
        World::get()->moveItemBetweenBodyParts(player, opos, npos, count);
        player->increaseActionPoints(-P_ITEMMOVE_COST);
    }
}

auto MoveItemInsideInventoryTS::clone() -> ClientCommandPointer {
    ClientCommandPointer cmd = std::make_shared<MoveItemInsideInventoryTS>();
    return cmd;
}

DropItemFromInventoryOnMapTS::DropItemFromInventoryOnMapTS()
        : BasicClientCommand(C_DROPITEMFROMPLAYERONMAP_TS, P_ITEMMOVE_COST) {}

void DropItemFromInventoryOnMapTS::decodeData() {
    pos = getUnsignedCharFromBuffer();
    mapPosition.x = getShortIntFromBuffer();
    mapPosition.y = getShortIntFromBuffer();
    mapPosition.z = getShortIntFromBuffer();
    count = getShortIntFromBuffer();
}

void DropItemFromInventoryOnMapTS::performAction(Player *player) {
    time(&(player->lastaction));
    player->ltAction->abortAction();
    Logger::debug(LogFacility::World) << *player << " throws an item from inventory on the map!" << Log::end;
    World::get()->dropItemFromPlayerOnMap(player, pos, mapPosition, count);
    player->increaseActionPoints(-P_ITEMMOVE_COST);
}

auto DropItemFromInventoryOnMapTS::clone() -> ClientCommandPointer {
    ClientCommandPointer cmd = std::make_shared<DropItemFromInventoryOnMapTS>();
    return cmd;
}

MoveItemFromMapToPlayerTS::MoveItemFromMapToPlayerTS()
        : BasicClientCommand(C_MOVEITEMFROMMAPTOPLAYER_TS, P_ITEMMOVE_COST) {}

void MoveItemFromMapToPlayerTS::decodeData() {
    sourcePosition.x = getShortIntFromBuffer();
    sourcePosition.y = getShortIntFromBuffer();
    sourcePosition.z = getShortIntFromBuffer();
    inventorySlot = getUnsignedCharFromBuffer();
    count = getShortIntFromBuffer();
}

void MoveItemFromMapToPlayerTS::performAction(Player *player) {
    time(&(player->lastaction));
    player->ltAction->abortAction();
    Logger::debug(LogFacility::World) << *player << " moves an item from map " << sourcePosition
                                      << " to inventory slot " << inventorySlot << Log::end;

    if (player->isAlive()) {
        World::get()->moveItemFromMapToPlayer(player, sourcePosition, inventorySlot, count);
        player->increaseActionPoints(-P_ITEMMOVE_COST);
    }
}

auto MoveItemFromMapToPlayerTS::clone() -> ClientCommandPointer {
    ClientCommandPointer cmd = std::make_shared<MoveItemFromMapToPlayerTS>();
    return cmd;
}

MoveItemFromMapIntoShowCaseTS::MoveItemFromMapIntoShowCaseTS()
        : BasicClientCommand(C_MOVEITEMFROMMAPINTOSHOWCASE_TS, P_ITEMMOVE_COST) {}

void MoveItemFromMapIntoShowCaseTS::decodeData() {
    sourcePosition.x = getShortIntFromBuffer();
    sourcePosition.y = getShortIntFromBuffer();
    sourcePosition.z = getShortIntFromBuffer();
    showcase = getUnsignedCharFromBuffer();
    showcaseSlot = getUnsignedCharFromBuffer();
    count = getShortIntFromBuffer();
}

void MoveItemFromMapIntoShowCaseTS::performAction(Player *player) {
    time(&(player->lastaction));
    player->ltAction->abortAction();
    Logger::debug(LogFacility::World) << *player << " moves an item from map " << sourcePosition << " to showcase "
                                      << (int)showcase << " slot " << (int)showcaseSlot << Log::end;

    if (player->isAlive()) {
        World::get()->moveItemFromMapIntoShowcase(player, sourcePosition, showcase, showcaseSlot, count);
        player->increaseActionPoints(-P_ITEMMOVE_COST);
    }
}

auto MoveItemFromMapIntoShowCaseTS::clone() -> ClientCommandPointer {
    ClientCommandPointer cmd = std::make_shared<MoveItemFromMapIntoShowCaseTS>();
    return cmd;
}

MoveItemFromMapToMapTS::MoveItemFromMapToMapTS() : BasicClientCommand(C_MOVEITEMFROMMAPTOMAP_TS, P_ITEMMOVE_COST) {}

void MoveItemFromMapToMapTS::decodeData() {
    sourcePosition.x = getShortIntFromBuffer();
    sourcePosition.y = getShortIntFromBuffer();
    sourcePosition.z = getShortIntFromBuffer();
    targetPosition.x = getShortIntFromBuffer();
    targetPosition.y = getShortIntFromBuffer();
    targetPosition.z = getShortIntFromBuffer();
    count = getShortIntFromBuffer();
}

void MoveItemFromMapToMapTS::performAction(Player *player) {
    time(&(player->lastaction));
    player->ltAction->abortAction();
    Logger::debug(LogFacility::World) << *player << " moves an item from map " << sourcePosition << " to map "
                                      << targetPosition << Log::end;

    if (player->isAlive()) {
        World::get()->moveItemFromMapToMap(player, sourcePosition, targetPosition, count);
        player->increaseActionPoints(-P_ITEMMOVE_COST);
    }
}

auto MoveItemFromMapToMapTS::clone() -> ClientCommandPointer {
    ClientCommandPointer cmd = std::make_shared<MoveItemFromMapToMapTS>();
    return cmd;
}

MoveItemBetweenShowCasesTS::MoveItemBetweenShowCasesTS()
        : BasicClientCommand(C_MOVEITEMBETWEENSHOWCASES_TS, P_ITEMMOVE_COST) {}

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

    if (player->isAlive()) {
        World::get()->moveItemBetweenShowcases(player, source, spos, dest, dpos, count);
        player->increaseActionPoints(-P_ITEMMOVE_COST);
    }
}

auto MoveItemBetweenShowCasesTS::clone() -> ClientCommandPointer {
    ClientCommandPointer cmd = std::make_shared<MoveItemBetweenShowCasesTS>();
    return cmd;
}

DropItemFromShowCaseOnMapTS::DropItemFromShowCaseOnMapTS()
        : BasicClientCommand(C_DROPITEMFROMSHOWCASEONMAP_TS, P_ITEMMOVE_COST) {}

void DropItemFromShowCaseOnMapTS::decodeData() {
    showcase = getUnsignedCharFromBuffer();
    pos = getUnsignedCharFromBuffer();
    mapPosition.x = getShortIntFromBuffer();
    mapPosition.y = getShortIntFromBuffer();
    mapPosition.z = getShortIntFromBuffer();
    count = getShortIntFromBuffer();
}

void DropItemFromShowCaseOnMapTS::performAction(Player *player) {
    time(&(player->lastaction));
    player->ltAction->abortAction();
    Logger::debug(LogFacility::World) << *player << " moves an item from showcase to the map!" << Log::end;
    World::get()->dropItemFromShowcaseOnMap(player, showcase, pos, mapPosition, count);
    player->increaseActionPoints(-P_ITEMMOVE_COST);
}

auto DropItemFromShowCaseOnMapTS::clone() -> ClientCommandPointer {
    ClientCommandPointer cmd = std::make_shared<DropItemFromShowCaseOnMapTS>();
    return cmd;
}

CloseContainerInShowCaseTS::CloseContainerInShowCaseTS() : BasicClientCommand(C_CLOSECONTAINERINSHOWCASE_TS) {}

void CloseContainerInShowCaseTS::decodeData() { showcase = getUnsignedCharFromBuffer(); }

void CloseContainerInShowCaseTS::performAction(Player *player) {
    time(&(player->lastaction));
    player->ltAction->abortAction();
    Logger::debug(LogFacility::World) << *player << " closes a container in the showcase" << Log::end;

    if (player->isAlive()) {
        player->closeShowcase(showcase);
    }
}

auto CloseContainerInShowCaseTS::clone() -> ClientCommandPointer {
    ClientCommandPointer cmd = std::make_shared<CloseContainerInShowCaseTS>();
    return cmd;
}

LookIntoShowCaseContainerTS::LookIntoShowCaseContainerTS()
        : BasicClientCommand(C_LOOKINTOSHOWCASECONTAINER_TS, P_LOOK_COST) {}

void LookIntoShowCaseContainerTS::decodeData() {
    showcase = getUnsignedCharFromBuffer();
    pos = getUnsignedCharFromBuffer();
}

void LookIntoShowCaseContainerTS::performAction(Player *player) {
    time(&(player->lastaction));
    player->ltAction->abortAction();
    Logger::debug(LogFacility::World) << *player << " looks into a container in a showcase!" << Log::end;
    player->lookIntoShowcaseContainer(showcase, pos);
    player->increaseActionPoints(-P_LOOK_COST);
}

auto LookIntoShowCaseContainerTS::clone() -> ClientCommandPointer {
    ClientCommandPointer cmd = std::make_shared<LookIntoShowCaseContainerTS>();
    return cmd;
}

LookIntoInventoryTS::LookIntoInventoryTS() : BasicClientCommand(C_LOOKINTOINVENTORY_TS, P_LOOK_COST) {}

void LookIntoInventoryTS::decodeData() {}

void LookIntoInventoryTS::performAction(Player *player) {
    time(&(player->lastaction));
    player->ltAction->abortAction();
    Logger::debug(LogFacility::World) << *player << " looks into his backpack" << Log::end;
    player->lookIntoBackPack();
    player->increaseActionPoints(-P_LOOK_COST);
}

auto LookIntoInventoryTS::clone() -> ClientCommandPointer {
    ClientCommandPointer cmd = std::make_shared<LookIntoInventoryTS>();
    return cmd;
}

LookIntoContainerOnFieldTS::LookIntoContainerOnFieldTS()
        : BasicClientCommand(C_LOOKINTOCONTAINERONFIELD_TS, P_LOOK_COST) {}

void LookIntoContainerOnFieldTS::decodeData() { dir = to_direction(getUnsignedCharFromBuffer()); }

void LookIntoContainerOnFieldTS::performAction(Player *player) {
    time(&(player->lastaction));
    player->ltAction->abortAction();
    Logger::debug(LogFacility::World) << *player << " looks into a container on the map" << Log::end;

    if (player->isAlive()) {
        player->lookIntoContainerOnField(dir);
        player->increaseActionPoints(-P_LOOK_COST);
    }
}

auto LookIntoContainerOnFieldTS::clone() -> ClientCommandPointer {
    ClientCommandPointer cmd = std::make_shared<LookIntoContainerOnFieldTS>();
    return cmd;
}

PickUpItemTS::PickUpItemTS() : BasicClientCommand(C_PICKUPITEM_TS, P_ITEMMOVE_COST) {}

void PickUpItemTS::decodeData() {
    pos.x = getShortIntFromBuffer();
    pos.y = getShortIntFromBuffer();
    pos.z = getShortIntFromBuffer();
}

void PickUpItemTS::performAction(Player *player) {
    time(&(player->lastaction));
    player->ltAction->abortAction();
    Logger::debug(LogFacility::World) << *player << " tries to pick up item at " << pos << Log::end;

    if (player->isAlive()) {
        World::get()->pickUpItemFromMap(player, pos);
        player->increaseActionPoints(-P_ITEMMOVE_COST);
    }
}

auto PickUpItemTS::clone() -> ClientCommandPointer {
    ClientCommandPointer cmd = std::make_shared<PickUpItemTS>();
    return cmd;
}

PickUpAllItemsTS::PickUpAllItemsTS() : BasicClientCommand(C_PICKUPALLITEMS_TS, P_ITEMMOVE_COST) {}

void PickUpAllItemsTS::decodeData() {}

void PickUpAllItemsTS::performAction(Player *player) {
    time(&(player->lastaction));
    player->ltAction->abortAction();
    Logger::debug(LogFacility::World) << *player << " tries to pick up all nearby items" << Log::end;

    if (player->isAlive()) {
        World::get()->pickUpAllItemsFromMap(player);
        player->increaseActionPoints(-P_ITEMMOVE_COST);
    }
}

auto PickUpAllItemsTS::clone() -> ClientCommandPointer {
    ClientCommandPointer cmd = std::make_shared<PickUpAllItemsTS>();
    return cmd;
}

LogOutTS::LogOutTS() : BasicClientCommand(C_LOGOUT_TS) {}

void LogOutTS::decodeData() {}

void LogOutTS::performAction(Player *player) {
    player->ltAction->abortAction();
    Logger::info(LogFacility::Player) << *player << " loggt aus" << Log::end;
    player->Connection->closeConnection();
}

auto LogOutTS::clone() -> ClientCommandPointer {
    ClientCommandPointer cmd = std::make_shared<LogOutTS>();
    return cmd;
}

WhisperTS::WhisperTS() : BasicClientCommand(C_WHISPER_TS) {}

void WhisperTS::decodeData() { text = getStringFromBuffer(); }

void WhisperTS::performAction(Player *player) {
    time(&(player->lastaction));
    Logger::debug(LogFacility::World) << *player << " whispers something!" << Log::end;
    player->talk(Character::tt_whisper, text);
}

auto WhisperTS::clone() -> ClientCommandPointer {
    ClientCommandPointer cmd = std::make_shared<WhisperTS>();
    return cmd;
}

ShoutTS::ShoutTS() : BasicClientCommand(C_SHOUT_TS) {}

void ShoutTS::decodeData() { text = getStringFromBuffer(); }

void ShoutTS::performAction(Player *player) {
    time(&(player->lastaction));
    player->talk(Character::tt_yell, text);
}

auto ShoutTS::clone() -> ClientCommandPointer {
    ClientCommandPointer cmd = std::make_shared<ShoutTS>();
    return cmd;
}

SayTS::SayTS() : BasicClientCommand(C_SAY_TS) {}

void SayTS::decodeData() { text = getStringFromBuffer(); }

void SayTS::performAction(Player *player) {
    time(&(player->lastaction));
    Logger::debug(LogFacility::World) << *player << " whispers something!" << Log::end;

    if (!World::get()->parseGMCommands(player, text)) {
        if (!World::get()->parsePlayerCommands(player, text)) { // did we issue a player command?
            player->talk(Character::tt_say, text);
        }
    }
}

auto SayTS::clone() -> ClientCommandPointer {
    ClientCommandPointer cmd = std::make_shared<SayTS>();
    return cmd;
}

RefreshTS::RefreshTS() : BasicClientCommand(C_REFRESH_TS) {}

void RefreshTS::decodeData() {}

void RefreshTS::performAction(Player *player) {
    Logger::debug(LogFacility::World) << *player << " want sended a refresh_ts, sending map!" << Log::end;
    player->sendFullMap();
    World::get()->sendAllVisibleCharactersToPlayer(player, true);
}

auto RefreshTS::clone() -> ClientCommandPointer {
    ClientCommandPointer cmd = std::make_shared<RefreshTS>();
    return cmd;
}

IntroduceTS::IntroduceTS() : BasicClientCommand(C_INTRODUCE_TS) {}

void IntroduceTS::decodeData() {}

void IntroduceTS::performAction(Player *player) {
    time(&(player->lastaction));
    Logger::debug(LogFacility::World) << *player << " introduces himself!" << Log::end;

    if (player->isAlive()) {
        World::get()->introduceMyself(player);
    }
}

auto IntroduceTS::clone() -> ClientCommandPointer {
    ClientCommandPointer cmd = std::make_shared<IntroduceTS>();
    return cmd;
}

CustomNameTS::CustomNameTS() : BasicClientCommand(C_CUSTOMNAME_TS) {}

void CustomNameTS::decodeData() {
    playerId = getIntFromBuffer();
    playerName = getStringFromBuffer();
}

void CustomNameTS::performAction(Player *player) {
    time(&(player->lastaction));
    Logger::debug(LogFacility::Player) << *player << " names " << playerId << " as " << playerName << Log::end;

    player->namePlayer(playerId, playerName);
}

auto CustomNameTS::clone() -> ClientCommandPointer {
    ClientCommandPointer cmd = std::make_shared<CustomNameTS>();
    return cmd;
}

AttackPlayerTS::AttackPlayerTS() : BasicClientCommand(C_ATTACKPLAYER_TS) {}

void AttackPlayerTS::decodeData() { enemyid = getIntFromBuffer(); }

void AttackPlayerTS::performAction(Player *player) {
    time(&(player->lastaction));

    if (player->isAlive()) {
        player->ltAction->abortAction();

        player->enemyid = enemyid;
        player->setAttackMode(true);
        player->enemytype = Character::player;

        if (player->enemyid >= MONSTER_BASE) {
            player->enemytype = Character::monster;
        }

        if (player->enemyid >= NPC_BASE) {
            player->enemytype = Character::npc;
        }

        ServerCommandPointer cmd = std::make_shared<AttackAcknowledgedTC>();
        player->Connection->addCommand(cmd);
        // monitoringClientList->sendCommand( new SendActionTS(player->getId(), player->getName(), 0, "Starts an
        // attack: " + Logger::toString(player->enemyid) ) );
        World::get()->characterAttacks(player);
    } else {
        player->setAttackMode(false);
    }
}

auto AttackPlayerTS::clone() -> ClientCommandPointer {
    ClientCommandPointer cmd = std::make_shared<AttackPlayerTS>();
    return cmd;
}

LookAtMapItemTS::LookAtMapItemTS() : BasicClientCommand(C_LOOKATMAPITEM_TS, P_LOOK_COST) {}

void LookAtMapItemTS::decodeData() {
    pos.x = getShortIntFromBuffer();
    pos.y = getShortIntFromBuffer();
    pos.z = getShortIntFromBuffer();
    stackPos = getUnsignedCharFromBuffer();
}

void LookAtMapItemTS::performAction(Player *player) {
    Logger::debug(LogFacility::World) << *player << " looks at a map item." << Log::end;

    if (player->isAlive()) {
        World::get()->lookAtMapItem(player, pos, stackPos);
        player->increaseActionPoints(-P_LOOK_COST);
    }
}

auto LookAtMapItemTS::clone() -> ClientCommandPointer {
    ClientCommandPointer cmd = std::make_shared<LookAtMapItemTS>();
    return cmd;
}

PlayerSpinTS::PlayerSpinTS() : BasicClientCommand(C_PLAYERSPIN_TS, P_SPIN_COST) {}

void PlayerSpinTS::decodeData() { dir = to_direction(getUnsignedCharFromBuffer()); }

void PlayerSpinTS::performAction(Player *player) {
    time(&(player->lastaction));
    player->ltAction->abortAction();
    Logger::debug(LogFacility::World) << *player << " changes his dircetion to " << (int)dir << Log::end;

    player->turn(dir);
}

auto PlayerSpinTS::clone() -> ClientCommandPointer {
    ClientCommandPointer cmd = std::make_shared<PlayerSpinTS>();
    return cmd;
}

CharMoveTS::CharMoveTS() : BasicClientCommand(C_CHARMOVE_TS, P_MIN_AP) {}

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
            Logger::info(LogFacility::World) << "Turtle was active, new tile set at pos: " << player->getPosition()
                                             << " tile: " << player->getTurtleTile() << Log::end;
        }

        if (player->move(static_cast<direction>(dir), mode)) {
            player->closeAllShowcasesOfMapContainers();
        }
    }
}

auto CharMoveTS::clone() -> ClientCommandPointer {
    ClientCommandPointer cmd = std::make_shared<CharMoveTS>();
    return cmd;
}

LoginCommandTS::LoginCommandTS() : BasicClientCommand(C_LOGIN_TS) {}

void LoginCommandTS::decodeData() {
    clientVersion = getUnsignedCharFromBuffer();
    loginName = getStringFromBuffer();
    password = getStringFromBuffer();
}

void LoginCommandTS::performAction(Player *player) { time(&(player->lastaction)); }

auto LoginCommandTS::clone() -> ClientCommandPointer {
    ClientCommandPointer cmd = std::make_shared<LoginCommandTS>();
    return cmd;
}

auto LoginCommandTS::getClientVersion() const -> unsigned short { return clientVersion; }

auto LoginCommandTS::getLoginName() const -> const std::string & { return loginName; }

auto LoginCommandTS::getPassword() const -> const std::string & { return password; }

ScreenSizeCommandTS::ScreenSizeCommandTS() : BasicClientCommand(C_SCREENSIZE_TS) {}

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

auto ScreenSizeCommandTS::clone() -> ClientCommandPointer {
    ClientCommandPointer cmd = std::make_shared<ScreenSizeCommandTS>();
    return cmd;
}
