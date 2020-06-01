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

#include "Container.hpp"
#include "Monster.hpp"
#include "NPC.hpp"
#include "Player.hpp"
#include "World.hpp"
#include "data/Data.hpp"
#include "map/Field.hpp"
#include "netinterface/protocol/ServerCommands.hpp"
#include "script/LuaItemScript.hpp"
#include "tuningConstants.hpp"

// TODO find a better place for the constants
static const std::string message_overweight_german{"Du kannst nicht so viel tragen!"};
static const std::string message_overweight_english{"You can't carry that much!"};

// atomic functions //

auto World::putItemOnInvPos(Character *cc, unsigned char pos) -> bool {
    if (pos == BACKPACK) {
        if (cc->items[BACKPACK].getId() == 0) {
            if (g_item.isContainer()) {
                cc->items[BACKPACK] = g_item;
                cc->items[BACKPACK].setNumber(1);

                if (g_cont == nullptr) {
                    g_cont = new Container(g_item.getId());
                } else {
                    auto *temp = dynamic_cast<Player *>(cc);

                    if (temp != nullptr) {
                        closeShowcaseForOthers(temp, g_cont);
                    }
                }

                cc->backPackContents = g_cont;

                g_item.reset();
                g_cont = nullptr;
                cc->updateAppearanceForAll(true);
                return true;
            }
        }
    } else if (g_cont == nullptr) {
        if (pos < MAX_BODY_ITEMS) {
            if (cc->items[pos].getId() == 0 || cc->items[pos].getId() == g_item.getId()) {
                if ((pos == RIGHT_TOOL) || (pos == LEFT_TOOL)) {
                    if (Data::WeaponItems.exists(g_item.getId())) {
                        const auto &weapon = Data::WeaponItems[g_item.getId()];

                        if ((weapon.Type == 4) || (weapon.Type == 5) || (weapon.Type == 6) || (weapon.Type == 13)) {
                            if ((pos == RIGHT_TOOL) && (cc->items[LEFT_TOOL].getId() == 0)) {
                                if (cc->items[pos].getId() == 0 && g_item.getNumber() == 1) {
                                    cc->items[pos] = g_item;
                                    cc->items[LEFT_TOOL].setId(BLOCKEDITEM);
                                    cc->items[LEFT_TOOL].makePermanent();
                                    cc->items[LEFT_TOOL].setNumber(1);
                                    g_item.reset();
                                    cc->updateAppearanceForAll(true);
                                    return true;
                                }
                                // we don't want to add any more two handers...
                                return false;
                            }
                            if ((pos == LEFT_TOOL) && (cc->items[RIGHT_TOOL].getId() == 0)) {
                                if (cc->items[pos].getId() == 0 && g_item.getNumber() == 1) {
                                    cc->items[pos] = g_item;

                                    cc->items[RIGHT_TOOL].setId(BLOCKEDITEM);

                                    cc->items[RIGHT_TOOL].makePermanent();

                                    cc->items[RIGHT_TOOL].setNumber(1);

                                    g_item.reset();

                                    cc->updateAppearanceForAll(true);

                                    return true;
                                }

                                // we don't want to add any more two handers...

                                return false;
                            }
                            return false;
                        }
                    }

                    if (cc->items[pos].getId() == 0) {
                        if (!g_item.isStackable() && !g_item.isContainer()) {
                            if (g_item.getNumber() > 1) {
                                return false;
                            }
                        }

                        cc->items[pos] = g_item;
                        g_item.reset();

                        cc->updateAppearanceForAll(true);
                        return true;
                    }
                    if (!g_item.isStackable() && !g_item.isContainer()) {
                        return false;
                    }

                    if (!cc->items[pos].equalData(g_item)) {
                        return false;
                    }

                    if (g_item.getNumber() + cc->items[pos].getNumber() <= g_item.getMaxStack()) {
                        cc->items[pos].setNumber(cc->items[pos].getNumber() + g_item.getNumber());

                        cc->items[pos].setMinQuality(g_item);

                        g_item.reset();

                        cc->updateAppearanceForAll(true);

                        return true;
                    }
                    return false;
                }
                if (g_item.getNumber() == 1 && cc->items[pos].getId() == 0) {
                    if (Data::ArmorItems.exists(g_item.getId())) {
                        const auto &armor = Data::ArmorItems[g_item.getId()];

                        unsigned char flag;

                        switch (pos) {
                        case HEAD:

                            flag = FLAG_HEAD;

                            break;

                        case NECK:

                            flag = FLAG_NECK;

                            break;

                        case BREAST:

                            flag = FLAG_BREAST;

                            break;

                        case HANDS:

                            flag = FLAG_HANDS;

                            break;

                        case FINGER_LEFT_HAND:

                        case FINGER_RIGHT_HAND:

                            flag = FLAG_FINGER;

                            break;

                        case LEGS:

                            flag = FLAG_LEGS;

                            break;

                        case FEET:

                            flag = FLAG_FEET;

                            break;

                        case COAT:

                            flag = FLAG_COAT;

                            break;

                        default:

                            flag = 0xFF;

                            break;
                        }

                        if ((armor.BodyParts & flag) != 0) {
                            cc->items[pos] = g_item;

                            g_item.reset();

                            cc->updateAppearanceForAll(true);

                            return true;
                        }
                    }
                }
            }
        } else if (pos < MAX_BODY_ITEMS + MAX_BELT_SLOTS) {
            if (cc->items[pos].getId() == 0) {
                if (!g_item.isStackable() && !g_item.isContainer()) {
                    if (g_item.getNumber() > 1) {
                        return false;
                    }
                }

                cc->items[pos] = g_item;
                g_item.reset();
                cc->updateAppearanceForAll(true);
                return true;
            }
            if (cc->items[pos].getId() == g_item.getId()) {
                if (!g_item.isStackable() && !g_item.isContainer()) {
                    return false;
                }

                if (!cc->items[pos].equalData(g_item)) {
                    return false;
                }

                int temp = cc->items[pos].getNumber() + g_item.getNumber();

                if (temp <= g_item.getMaxStack()) {
                    cc->items[pos].setNumber(temp);

                    cc->items[pos].setMinQuality(g_item);

                    g_item.reset();

                    cc->updateAppearanceForAll(true);

                    return true;
                }
            }
        }
    }

    return false;
}

auto World::putItemOnInvPos(Player *cc, unsigned char pos) -> bool {
    if (putItemOnInvPos(dynamic_cast<Character *>(cc), pos)) {
        if (pos == LEFT_TOOL) {
            cc->sendCharacterItemAtPos(pos);
            cc->sendCharacterItemAtPos(RIGHT_TOOL);
        } else if (pos == RIGHT_TOOL) {
            cc->sendCharacterItemAtPos(pos);
            cc->sendCharacterItemAtPos(LEFT_TOOL);
        } else {
            cc->sendCharacterItemAtPos(pos);
        }

        return true;
    }
    return false;
}

auto World::takeItemFromInvPos(Character *cc, unsigned char pos, Item::number_type count) -> bool {
    if (pos == BACKPACK) {
        if (cc->items[BACKPACK].getId() != 0) {
            g_item = cc->items[BACKPACK];
            g_cont = cc->backPackContents;

            if (g_cont == nullptr) {
                g_cont = new Container(g_item.getId());
            }

            cc->items[BACKPACK].reset();
            cc->backPackContents = nullptr;
            cc->updateAppearanceForAll(true);
            return true;
        }
    } else if (pos < MAX_BODY_ITEMS + MAX_BELT_SLOTS) {
        if ((cc->items[pos].getId() != 0) && (cc->items[pos].getId() != BLOCKEDITEM)) {
            if ((pos == RIGHT_TOOL) || (pos == LEFT_TOOL)) {
                const auto weaponId = cc->items[pos].getId();

                if (Data::WeaponItems.exists(weaponId)) {
                    g_item = cc->items[pos];
                    g_cont = nullptr;

                    if (!g_item.isStackable() && !g_item.isContainer()) {
                        if (g_item.getNumber() > 1 && count > 1) {
                            g_item.reset();
                            g_cont = nullptr;
                            return false;
                        }
                    }

                    if (g_item.getNumber() > count) {
                        cc->items[pos].setNumber(cc->items[pos].getNumber() - count);
                        g_item.setNumber(count);
                    } else {
                        const auto &weapon = Data::WeaponItems[weaponId];

                        if ((weapon.Type == 4) || (weapon.Type == 5) || (weapon.Type == 6) || (weapon.Type == 13)) {
                            cc->items[LEFT_TOOL].reset();
                            cc->items[RIGHT_TOOL].reset();
                        } else {
                            cc->items[pos].reset();
                        }
                    }

                    cc->updateAppearanceForAll(true);
                    return true;
                }
            }

            g_item = cc->items[pos];
            g_cont = nullptr;

            if (g_item.isStackable() && count > 1 && !g_item.isContainer()) {
                if (g_item.getNumber() > count) {
                    cc->items[pos].setNumber(cc->items[pos].getNumber() - count);
                    g_item.setNumber(count);
                } else {
                    cc->items[pos].reset();
                }

                cc->updateAppearanceForAll(true);
                return true;
            }
            if (g_item.getNumber() > 1) {
                cc->items[pos].setNumber(cc->items[pos].getNumber() - 1);

                g_item.setNumber(1);

            } else {
                cc->items[pos].reset();
            }

            cc->updateAppearanceForAll(true);

            return true;
        }
    } // Rucksack oder normal

    g_item.reset();
    g_cont = nullptr;

    return false;
}

auto World::takeItemFromInvPos(Player *cc, unsigned char pos, Item::number_type count) -> bool {
    if (pos == BACKPACK) {
        if (cc->items[BACKPACK].getId() != 0) {
            if (cc->backPackContents != nullptr) {
                cc->closeShowcase(cc->backPackContents);
            }
        }
    }

    if (takeItemFromInvPos(dynamic_cast<Character *>(cc), pos, count)) {
        if (pos == LEFT_TOOL) {
            cc->sendCharacterItemAtPos(pos);
            cc->sendCharacterItemAtPos(RIGHT_TOOL);
        } else if (pos == RIGHT_TOOL) {
            cc->sendCharacterItemAtPos(pos);
            cc->sendCharacterItemAtPos(LEFT_TOOL);
        } else {
            cc->sendCharacterItemAtPos(pos);
        }

        return true;
    }
    return false;
}

auto World::takeItemFromShowcase(Player *cc, uint8_t showcase, unsigned char pos, Item::number_type count) -> bool {
    Container *ps = cc->getShowcaseContainer(showcase);

    if (ps != nullptr) {
        if (ps->TakeItemNr(pos, g_item, g_cont, count)) {
            g_item.resetWear();

            if (g_cont != nullptr) {
                g_cont->resetWear();
                sendContainerSlotChange(ps, pos, g_cont);
            } else {
                sendContainerSlotChange(ps, pos);
            }

            return true;
        }
    }

    g_item.reset();
    g_cont = nullptr;

    return false;
}

auto World::putItemInShowcase(Player *cc, uint8_t showcase, TYPE_OF_CONTAINERSLOTS pos) -> bool {
    if (!g_item.isStackable() && !g_item.isContainer()) {
        if (g_item.getNumber() > 1) {
            return false;
        }
    }

    Container *ps = cc->getShowcaseContainer(showcase);

    if (ps != nullptr) {
        if (g_cont != nullptr) {
            if (ps != g_cont) {
                if (ps->InsertContainer(g_item, g_cont, pos)) {
                    sendContainerSlotChange(ps, pos, g_cont);
                    g_item.reset();
                    g_cont = nullptr;

                    return true;
                }
            } else {
                return false;
            }
        } else {
            if (ps->InsertItem(g_item, pos)) {
                sendContainerSlotChange(ps, pos);
                g_item.reset();
                return true;
            }
        }
    }

    return false;
}

auto World::takeItemFromMap(Character *cc, const position &itemPosition) -> bool {
    auto dx = abs(itemPosition.x - cc->getPosition().x);
    auto dy = abs(itemPosition.y - cc->getPosition().y);
    auto dz = abs(itemPosition.z - cc->getPosition().z);

    if (dx > 1 || dy > 1 || dz != 0) {
        return false;
    }

    try {
        map::Field &field = fieldAt(itemPosition);

        if (field.viewItemOnStack(g_item)) {
            if (g_item.isMovable()) {
                field.takeItemFromStack(g_item);

                if (!g_item.isStackable() && !g_item.isContainer()) {
                    if (g_item.getNumber() > 1) {
                        g_item.reset();
                        g_cont = nullptr;
                        return false;
                    }
                }

                if (Data::Triggers.exists(itemPosition)) {
                    const auto &script = Data::Triggers.script(itemPosition);

                    if (script) {
                        ScriptItem sItem(g_item);
                        sItem.pos = itemPosition;
                        sItem.type = ScriptItem::it_field;
                        script->TakeItemFromField(sItem, cc);
                    }
                }

                g_item.resetWear();

                if (g_item.isContainer()) {
                    auto it = field.containers.find(g_item.getNumber());

                    if (it != field.containers.end()) {
                        g_cont = it->second;
                        g_cont->resetWear();

                        field.containers.erase(it);
                    } else {
                        g_cont = new Container(g_item.getId());
                    }
                } else {
                    g_cont = nullptr;
                }

                sendRemoveItemFromMapToAllVisibleCharacters(itemPosition);
                return true;
            }
        }
    } catch (FieldNotFound &) {
    }

    g_item.reset();
    g_cont = nullptr;

    return false;
}

auto World::putItemOnMap(Character *cc, const position &itemPosition) -> bool {
    if (cc != nullptr) {
        if (cc->getPosition().z != itemPosition.z || !cc->isInRangeToField(itemPosition, MAXTHROWDISTANCE) ||
            (!cc->isInRangeToField(itemPosition, MAXDROPDISTANCE) && (g_item.getWeight() > MAXTHROWWEIGHT))) {
            static const std::string german = "Dies ist zu schwer um so weit geworfen zu werden.";
            static const std::string english = "This is too heavy to be thrown this far.";
            cc->inform(german, english, Character::informScriptMediumPriority);
            return false;
        }
    }

    if (!g_item.isStackable() && !g_item.isContainer()) {
        if (g_item.getNumber() > 1) {
            return false;
        }
    }

    try {
        map::Field &field = fieldAt(itemPosition);

        if (!Data::TilesModItems.passable(g_item.getId())) {
            if (!field.moveToPossible()) {
                return false;
            }
        }

        if (g_item.isContainer()) {
            if (g_cont == nullptr) {
                g_cont = new Container(g_item.getId());
            } else {
                closeShowcaseIfNotInRange(g_cont, itemPosition);
            }

            if (field.addContainerOnStackIfWalkable(g_item, g_cont)) {
                sendPutItemOnMapToAllVisibleCharacters(itemPosition, g_item);

                if ((cc != nullptr) && Data::Triggers.exists(itemPosition)) {
                    const auto &script = Data::Triggers.script(itemPosition);

                    if (script) {
                        ScriptItem sItem(g_item);
                        sItem.pos = itemPosition;
                        sItem.type = ScriptItem::it_field;
                        script->PutItemOnField(sItem, cc);
                    }
                }

                checkField(field, itemPosition);
                g_item.reset();
                g_cont = nullptr;

                return true;
            }
        } else {
            if (field.addItemOnStackIfWalkable(g_item)) {
                sendPutItemOnMapToAllVisibleCharacters(itemPosition, g_item);

                if ((cc != nullptr) && Data::Triggers.exists(itemPosition)) {
                    const auto &script = Data::Triggers.script(itemPosition);

                    if (script) {
                        ScriptItem sItem(g_item);
                        sItem.pos = itemPosition;
                        sItem.type = ScriptItem::it_field;
                        script->PutItemOnField(sItem, cc);
                    }
                }

                checkField(field, itemPosition);
                g_cont = nullptr;
                g_item.reset();

                return true;
            }
        }
    } catch (FieldNotFound &) {
    }

    return false;
}

auto World::putItemAlwaysOnMap(Character *cc, const position &itemPosition) -> bool {
    try {
        map::Field &field = fieldAt(itemPosition);

        if (g_item.isContainer()) {
            // Container
            if (g_cont == nullptr) {
                g_cont = new Container(g_item.getId());
            }

            if (field.addContainerOnStack(g_item, g_cont)) {
                sendPutItemOnMapToAllVisibleCharacters(itemPosition, g_item);

                if ((cc != nullptr) && Data::Triggers.exists(itemPosition)) {
                    const auto &script = Data::Triggers.script(itemPosition);

                    if (script) {
                        ScriptItem sItem(g_item);
                        sItem.pos = itemPosition;
                        sItem.type = ScriptItem::it_field;
                        script->PutItemOnField(sItem, cc);
                    }
                }

                checkField(field, itemPosition);
                g_item.reset();
                g_cont = nullptr;

                return true;
            }
        } else {
            if (field.addItemOnStack(g_item)) {
                sendPutItemOnMapToAllVisibleCharacters(itemPosition, g_item);

                if ((cc != nullptr) && Data::Triggers.exists(itemPosition)) {
                    const auto &script = Data::Triggers.script(itemPosition);

                    if (script) {
                        ScriptItem sItem(g_item);
                        sItem.pos = itemPosition;
                        sItem.type = ScriptItem::it_field;
                        script->PutItemOnField(sItem, cc);
                    }
                }

                checkField(field, itemPosition);
                g_cont = nullptr;
                g_item.reset();

                return true;
            }
        }
    } catch (FieldNotFound &) {
    }

    return false;
}

void World::checkField(const map::Field &field, const position &itemPosition) {
    if (field.hasSpecialItem()) {
        if (field.hasPlayer()) {
            Player *temp = Players.find(itemPosition);

            if (temp != nullptr) {
                checkFieldAfterMove(temp, field);
            }
        } else if (field.hasMonster()) {
            Monster *temp = Monsters.find(itemPosition);

            if (temp != nullptr) {
                checkFieldAfterMove(temp, field);
            }
        } else if (field.hasNPC()) {
            NPC *temp = Npc.find(itemPosition);

            if (temp != nullptr) {
                checkFieldAfterMove(temp, field);
            }
        }
    }
}

// combined functions //

void World::dropItemFromShowcaseOnMap(Player *cp, uint8_t showcase, unsigned char pos, const position &newPosition,
                                      Item::number_type count) {
    if (count == 0) {
        return;
    }

    if (takeItemFromShowcase(cp, showcase, pos, count)) {
        ScriptItem s_item(g_item);
        ScriptItem t_item(g_item);
        s_item.pos = cp->getPosition();
        s_item.itempos = pos;
        s_item.type = ScriptItem::it_container;
        s_item.inside = cp->getShowcaseContainer(showcase);
        s_item.owner = cp;
        t_item.pos = newPosition;
        t_item.type = ScriptItem::it_field;
        t_item.owner = cp;
        std::shared_ptr<LuaItemScript> script = Data::Items.script(t_item.getId());

        if (script && script->existsEntrypoint("MoveItemBeforeMove")) {
            if (!script->MoveItemBeforeMove(cp, s_item, t_item)) {
                if (!putItemInShowcase(cp, showcase, pos)) {
                    Logger::error(LogFacility::Player) << "dropItemFromShowcaseOnMap failed: item " << g_item.getId()
                                                       << " lost for " << *cp << Log::end;
                    g_cont = nullptr;
                    g_item.reset();
                }

                return;
            }
        }

        if (!putItemOnMap(cp, newPosition)) {
            if (!putItemInShowcase(cp, showcase, pos)) {
                Logger::error(LogFacility::Player) << "dropItemFromShowcaseOnMap failed: item " << g_item.getId()
                                                   << " lost for " << *cp << Log::end;
                g_cont = nullptr;
                g_item.reset();
            }
        } else {
            cp->checkBurden();

            if (script) {
                script->MoveItemAfterMove(cp, s_item, t_item);
            }
        }
    }
}

void World::moveItemFromShowcaseToPlayer(Player *cp, uint8_t showcase, unsigned char pos, unsigned char cpos,
                                         Item::number_type count) {
    if (count == 0) {
        return;
    }

    if (takeItemFromShowcase(cp, showcase, pos, count)) {
        ScriptItem s_item(g_item);
        ScriptItem t_item(g_item);
        s_item.pos = cp->getPosition();
        s_item.itempos = pos;
        s_item.type = ScriptItem::it_container;
        s_item.inside = cp->getShowcaseContainer(showcase);
        s_item.owner = cp;
        t_item.pos = cp->getPosition();
        t_item.itempos = cpos;

        if (cpos < MAX_BODY_ITEMS) {
            t_item.type = ScriptItem::it_inventory;
        } else {
            t_item.type = ScriptItem::it_belt;
        }

        t_item.owner = cp;
        std::shared_ptr<LuaItemScript> script = Data::Items.script(t_item.getId());

        if (script && script->existsEntrypoint("MoveItemBeforeMove")) {
            if (!script->MoveItemBeforeMove(cp, s_item, t_item)) {
                if (!putItemInShowcase(cp, showcase, pos)) {
                    Logger::error(LogFacility::Player) << "moveItemFromShowcaseToPlayer failed: item " << g_item.getId()
                                                       << " lost for " << *cp << Log::end;
                    g_cont = nullptr;
                    g_item.reset();
                }

                return;
            }
        }

        bool NOK = false;

        if (!cp->weightOK(g_item.getId(), g_item.getNumber(), g_cont)) {
            cp->inform(message_overweight_german, message_overweight_english, Character::informScriptMediumPriority);
            NOK = true;
        }

        if (!NOK) {
            if (!putItemOnInvPos(cp, cpos)) {
                NOK = true;
            } else {
                cp->checkBurden();

                if (script) {
                    script->MoveItemAfterMove(cp, s_item, t_item);
                }
            }
        }

        if (NOK) {
            if (!putItemInShowcase(cp, showcase, pos)) {
                Logger::error(LogFacility::Player) << "moveItemFromShowcaseToPlayer failed: item " << g_item.getId()
                                                   << " lost for " << *cp << Log::end;
                g_cont = nullptr;
                g_item.reset();
            }
        }
    }
}

void World::dropItemFromPlayerOnMap(Player *cp, unsigned char cpos, const position &newPosition,
                                    Item::number_type count) {
    if (count == 0) {
        return;
    }

    if (takeItemFromInvPos(cp, cpos, count)) {
        ScriptItem s_item(g_item);
        ScriptItem t_item(g_item);
        s_item.pos = cp->getPosition();

        if (cpos < MAX_BODY_ITEMS) {
            s_item.type = ScriptItem::it_inventory;
        } else {
            s_item.type = ScriptItem::it_belt;
        }

        s_item.itempos = cpos;
        s_item.owner = cp;
        t_item.pos = newPosition;
        t_item.type = ScriptItem::it_field;
        t_item.owner = cp;
        std::shared_ptr<LuaItemScript> script = Data::Items.script(t_item.getId());

        if (script && script->existsEntrypoint("MoveItemBeforeMove")) {
            if (!script->MoveItemBeforeMove(cp, s_item, t_item)) {
                if (!putItemOnInvPos(cp, cpos)) {
                    Logger::error(LogFacility::Player) << "dropItemFromPlayerOnMap failed: item " << g_item.getId()
                                                       << " lost for " << *cp << Log::end;
                    g_cont = nullptr;
                    g_item.reset();
                }

                return;
            }
        }

        if (!putItemOnMap(cp, newPosition)) {
            if (!putItemOnInvPos(cp, cpos)) {
                Logger::error(LogFacility::Player)
                        << "dropItemFromPlayerOnMap failed: item " << g_item.getId() << " lost for " << *cp << Log::end;
                g_cont = nullptr;
                g_item.reset();
            }
        } else {
            cp->checkBurden();

            if (script) {
                script->MoveItemAfterMove(cp, s_item, t_item);
            }
        }
    }
}

void World::moveItemBetweenBodyParts(Player *cp, unsigned char opos, unsigned char npos, Item::number_type count) {
    if (count == 0) {
        return;
    }

    if (takeItemFromInvPos(cp, opos, count)) {
        ScriptItem s_item(g_item);
        ScriptItem t_item(g_item);
        s_item.owner = cp;
        s_item.pos = cp->getPosition();

        if (opos < MAX_BODY_ITEMS) {
            s_item.type = ScriptItem::it_inventory;
        } else {
            s_item.type = ScriptItem::it_belt;
        }

        s_item.itempos = opos;
        t_item.owner = cp;
        t_item.pos = cp->getPosition();

        if (npos < MAX_BODY_ITEMS) {
            t_item.type = ScriptItem::it_inventory;
        } else {
            t_item.type = ScriptItem::it_belt;
        }

        t_item.itempos = npos;
        std::shared_ptr<LuaItemScript> script = Data::Items.script(t_item.getId());

        if (script && script->existsEntrypoint("MoveItemBeforeMove")) {
            if (!script->MoveItemBeforeMove(cp, s_item, t_item)) {
                if (!putItemOnInvPos(cp, opos)) {
                    Logger::error(LogFacility::Player) << "moveItemBetweenBodyParts failed: item " << g_item.getId()
                                                       << " lost for " << *cp << Log::end;
                    g_cont = nullptr;
                    g_item.reset();
                }

                return;
            }
        }

        if (!putItemOnInvPos(cp, npos)) {
            if (!putItemOnInvPos(cp, opos)) {
                Logger::error(LogFacility::Player) << "moveItemBetweenBodyParts failed: item " << g_item.getId()
                                                   << " lost for " << *cp << Log::end;
                g_cont = nullptr;
                g_item.reset();
            }
        } else {
            if (script) {
                script->MoveItemAfterMove(cp, s_item, t_item);
            }
        }
    }
}

void World::moveItemFromPlayerIntoShowcase(Player *cp, unsigned char cpos, uint8_t showcase, unsigned char pos,
                                           Item::number_type count) {
    if (count == 0) {
        return;
    }

    if (takeItemFromInvPos(cp, cpos, count)) {
        ScriptItem s_item(g_item);
        ScriptItem t_item(g_item);

        if (cpos < MAX_BODY_ITEMS) {
            s_item.type = ScriptItem::it_inventory;
        } else {
            s_item.type = ScriptItem::it_belt;
        }

        s_item.pos = cp->getPosition();
        s_item.itempos = cpos;
        s_item.owner = cp;

        t_item.type = ScriptItem::it_container;
        t_item.inside = cp->getShowcaseContainer(showcase);
        t_item.pos = cp->getPosition();
        t_item.owner = cp;
        t_item.itempos = pos;
        std::shared_ptr<LuaItemScript> script = Data::Items.script(t_item.getId());

        if (script && script->existsEntrypoint("MoveItemBeforeMove")) {
            if (!script->MoveItemBeforeMove(cp, s_item, t_item)) {
                if (!putItemOnInvPos(cp, cpos)) {
                    Logger::error(LogFacility::Player) << "moveItemFromPlayerIntoShowcase failed: item "
                                                       << g_item.getId() << " lost for " << *cp << Log::end;
                    g_cont = nullptr;
                    g_item.reset();
                }

                return;
            }
        }

        if (!putItemInShowcase(cp, showcase, pos)) {
            if (!putItemOnInvPos(cp, cpos)) {
                Logger::error(LogFacility::Player) << "moveItemFromPlayerIntoShowcase failed: item " << g_item.getId()
                                                   << " lost for " << *cp << Log::end;
                g_cont = nullptr;
                g_item.reset();
            }
        } else {
            cp->checkBurden();

            if (script) {
                script->MoveItemAfterMove(cp, s_item, t_item);
            }
        }
    }
}

void World::moveItemFromMapIntoShowcase(Player *cp, const position &sourcePosition, uint8_t showcase,
                                        unsigned char showcaseSlot, Item::number_type count) {
    if (count == 0) {
        return;
    }

    if (cp != nullptr) {
        if (takeItemFromMap(cp, sourcePosition)) {
            ScriptItem s_item(g_item);
            ScriptItem t_item(g_item);
            s_item.pos = sourcePosition;
            s_item.type = ScriptItem::it_field;
            s_item.owner = cp;
            t_item.pos = cp->getPosition();
            t_item.type = ScriptItem::it_container;
            t_item.inside = cp->getShowcaseContainer(showcase);
            t_item.itempos = showcaseSlot;
            t_item.owner = cp;

            std::shared_ptr<LuaItemScript> script = Data::Items.script(t_item.getId());

            if (script && script->existsEntrypoint("MoveItemBeforeMove")) {
                if (!script->MoveItemBeforeMove(cp, s_item, t_item)) {
                    if (!putItemOnMap(cp, sourcePosition)) {
                        Logger::error(LogFacility::Player) << "moveItemFromMapIntoShowcase failed: item "
                                                           << g_item.getId() << " lost for " << *cp << Log::end;
                        g_cont = nullptr;
                        g_item.reset();
                    }

                    return;
                }
            }

            Item tempitem = g_item;

            if (count < g_item.getNumber()) {
                g_item.setNumber(count);
            }

            bool NOK = false;

            if (!cp->weightOK(g_item.getId(), g_item.getNumber(), g_cont)) {
                cp->inform(message_overweight_german, message_overweight_english,
                           Character::informScriptMediumPriority);
                NOK = true;
            }

            if (!NOK) {
                if (!putItemInShowcase(cp, showcase, showcaseSlot)) {
                    NOK = true;
                } else {
                    cp->checkBurden();

                    if (script) {
                        script->MoveItemAfterMove(cp, s_item, t_item);
                    }
                }
            }

            if (NOK) {
                g_item = tempitem;

                if (!putItemOnMap(cp, sourcePosition)) {
                    Logger::error(LogFacility::Player) << "moveItemFromMapIntoShowcase failed: item " << g_item.getId()
                                                       << " lost for " << *cp << Log::end;
                    g_cont = nullptr;
                    g_item.reset();
                }

                return;
            }

            if (count < tempitem.getNumber() && !tempitem.isContainer()) {
                g_item = tempitem;
                g_item.setNumber(g_item.getNumber() - count);

                if (!putItemOnMap(cp, sourcePosition)) {
                    Logger::error(LogFacility::Player) << "moveItemFromMapIntoShowcase failed: item " << g_item.getId()
                                                       << " lost for " << *cp << Log::end;
                    g_cont = nullptr;
                    g_item.reset();
                    return;
                }
            }

            return;
        }
    }
}

void World::moveItemFromMapToPlayer(Player *cp, const position &sourcePosition, unsigned char inventorySlot,
                                    Item::number_type count) {
    if (count == 0) {
        return;
    }

    if (cp != nullptr) {
        if (takeItemFromMap(cp, sourcePosition)) {
            ScriptItem s_item(g_item);
            ScriptItem t_item(g_item);
            s_item.pos = sourcePosition;
            s_item.type = ScriptItem::it_field;
            s_item.owner = cp;
            t_item.pos = cp->getPosition();

            if (inventorySlot < MAX_BODY_ITEMS) {
                t_item.type = ScriptItem::it_inventory;
            } else {
                t_item.type = ScriptItem::it_belt;
            }

            t_item.owner = cp;
            t_item.itempos = inventorySlot;
            std::shared_ptr<LuaItemScript> script = Data::Items.script(t_item.getId());

            if (script && script->existsEntrypoint("MoveItemBeforeMove")) {
                if (!script->MoveItemBeforeMove(cp, s_item, t_item)) {
                    if (!putItemOnMap(cp, sourcePosition)) {
                        Logger::error(LogFacility::Player) << "moveItemFromMapToPlayer failed: item " << g_item.getId()
                                                           << " lost for " << *cp << Log::end;
                        g_cont = nullptr;
                        g_item.reset();
                    }

                    return;
                }
            }

            Item tempitem = g_item;

            if (count < g_item.getNumber()) {
                g_item.setNumber(count);
            }

            bool NOK = false;

            if (!cp->weightOK(g_item.getId(), g_item.getNumber(), g_cont)) {
                cp->inform(message_overweight_german, message_overweight_english,
                           Character::informScriptMediumPriority);
                NOK = true;
            }

            if (!NOK) {
                if (!putItemOnInvPos(cp, inventorySlot)) {
                    NOK = true;
                } else {
                    cp->checkBurden();

                    if (script) {
                        script->MoveItemAfterMove(cp, s_item, t_item);
                    }
                }
            }

            if (NOK) {
                g_item = tempitem;

                if (!putItemOnMap(cp, sourcePosition)) {
                    Logger::error(LogFacility::Player) << "moveItemFromMapToPlayer failed: item " << g_item.getId()
                                                       << " lost for " << *cp << Log::end;
                    g_cont = nullptr;
                    g_item.reset();
                }

                return;
            }

            if (count < tempitem.getNumber() && !tempitem.isContainer()) {
                g_item = tempitem;
                g_item.setNumber(g_item.getNumber() - count);

                if (!putItemOnMap(cp, sourcePosition)) {
                    Logger::error(LogFacility::Player) << "moveItemFromMapToPlayer failed: item " << g_item.getId()
                                                       << " lost for " << *cp << Log::end;
                    g_cont = nullptr;
                    g_item.reset();
                    return;
                }
            }

            return;
        }
    }
}

void World::moveItemBetweenShowcases(Player *cp, uint8_t source, unsigned char pos, uint8_t dest, unsigned char pos2,
                                     Item::number_type count) {
    if (count == 0) {
        return;
    }

    if (takeItemFromShowcase(cp, source, pos, count)) {
        ScriptItem s_item(g_item);
        ScriptItem t_item(g_item);
        s_item.pos = cp->getPosition();
        s_item.type = ScriptItem::it_container;
        s_item.inside = cp->getShowcaseContainer(source);
        s_item.itempos = pos;
        s_item.owner = cp;
        t_item.pos = cp->getPosition();
        t_item.type = ScriptItem::it_container;
        t_item.inside = cp->getShowcaseContainer(dest);
        t_item.itempos = pos2;
        t_item.owner = cp;
        std::shared_ptr<LuaItemScript> script = Data::Items.script(t_item.getId());

        if (script && script->existsEntrypoint("MoveItemBeforeMove")) {
            if (!script->MoveItemBeforeMove(cp, s_item, t_item)) {
                if (!putItemInShowcase(cp, source, pos)) {
                    Logger::error(LogFacility::Player) << "moveItemBetweenShowcases failed: item " << g_item.getId()
                                                       << " lost for " << *cp << Log::end;
                    g_cont = nullptr;
                    g_item.reset();
                }

                return;
            }
        }

        bool success = true;

        if (cp->isShowcaseInInventory(dest)) {
            if (!cp->weightOK(g_item.getId(), g_item.getNumber(), g_cont)) {
                cp->inform(message_overweight_german, message_overweight_english,
                           Character::informScriptMediumPriority);
                success = false;
            }
        }

        if (success) {
            if (!putItemInShowcase(cp, dest, pos2)) {
                success = false;
            } else {
                cp->checkBurden();

                if (script) {
                    script->MoveItemAfterMove(cp, s_item, t_item);
                }
            }
        }

        if (!success) {
            if (!putItemInShowcase(cp, source, pos)) {
                Logger::error(LogFacility::Player) << "moveItemBetweenShowcases failed: item " << g_item.getId()
                                                   << " lost for " << *cp << Log::end;
                g_cont = nullptr;
                g_item.reset();
            }
        }
    }
}

auto World::moveItemFromMapToMap(Player *cp, const position &oldPosition, const position &newPosition,
                                 Item::number_type count) -> bool {
    if (count == 0) {
        return false;
    }

    if (cp != nullptr) {
        if (takeItemFromMap(cp, oldPosition)) {
            ScriptItem s_item(g_item);
            ScriptItem t_item(g_item);
            s_item.pos = oldPosition;
            s_item.type = ScriptItem::it_field;
            s_item.owner = cp;
            t_item.pos = newPosition;
            t_item.type = ScriptItem::it_field;
            t_item.owner = cp;

            std::shared_ptr<LuaItemScript> script = Data::Items.script(t_item.getId());

            if (script && script->existsEntrypoint("MoveItemBeforeMove")) {
                if (!script->MoveItemBeforeMove(cp, s_item, t_item)) {
                    if (!putItemOnMap(cp, oldPosition)) {
                        Logger::error(LogFacility::Player)
                                << "moveItem failed: item " << g_item.getId() << " lost for " << *cp << Log::end;
                        g_cont = nullptr;
                        g_item.reset();
                    }

                    return false;
                }
            }

            Item tempitem = g_item;

            if (count < g_item.getNumber()) {
                g_item.setNumber(count);
            }

            if (!putItemOnMap(cp, newPosition)) {
                g_item = tempitem;

                if (!putItemOnMap(cp, oldPosition)) {
                    Logger::error(LogFacility::Player)
                            << "moveItem failed: item " << g_item.getId() << " lost for " << *cp << Log::end;
                    g_cont = nullptr;
                    g_item.reset();
                }

                return false;
            }

            if (count < tempitem.getNumber() && !tempitem.isContainer()) {
                g_item = tempitem;
                g_item.setNumber(g_item.getNumber() - count);

                if (!putItemOnMap(cp, oldPosition)) {
                    Logger::error(LogFacility::Player)
                            << "moveItem failed: item " << g_item.getId() << " lost for " << *cp << Log::end;
                    g_cont = nullptr;
                    g_item.reset();
                }

                return false;
            }

            if (script) {
                script->MoveItemAfterMove(cp, s_item, t_item);
            }

            return true;
        }
    }

    return false;
}

auto World::pickUpItemFromMap(Player *cp, const position &itemPosition) -> bool {
    if (cp != nullptr) {
        if (takeItemFromMap(cp, itemPosition)) {
            ScriptItem s_item(g_item);
            ScriptItem t_item(g_item);
            s_item.pos = itemPosition;
            s_item.type = ScriptItem::it_field;
            s_item.owner = cp;
            t_item.pos = cp->getPosition();
            t_item.owner = cp;

            std::shared_ptr<LuaItemScript> script = Data::Items.script(t_item.getId());

            if (script && script->existsEntrypoint("MoveItemBeforeMove")) {
                if (!script->MoveItemBeforeMove(cp, s_item, t_item)) {
                    if (!putItemOnMap(cp, itemPosition)) {
                        Logger::error(LogFacility::Player) << "pickUpItemFromMap failed: item " << g_item.getId()
                                                           << " lost for " << *cp << Log::end;
                        g_cont = nullptr;
                        g_item.reset();
                    }

                    return false;
                }
            }

            Item tempitem = g_item;
            bool NOK = false;

            if (!cp->weightOK(g_item.getId(), g_item.getNumber(), g_cont)) {
                cp->inform(message_overweight_german, message_overweight_english,
                           Character::informScriptMediumPriority);
                NOK = true;
            }

            if (!NOK) {
                script_data_exchangemap data_map;

                for (auto it = g_item.getDataBegin(); it != g_item.getDataEnd(); ++it) {
                    data_map.push_back(*it);
                }

                bool backpackPresent = cp->backPackContents != nullptr;
                TYPE_OF_CONTAINERSLOTS freeSlot = 0;

                if (backpackPresent) {
                    freeSlot = cp->backPackContents->getFirstFreeSlot();
                }

                if (g_item.isContainer() && backpackPresent && freeSlot < cp->backPackContents->getSlotCount()) {
                    if (g_cont == nullptr) {
                        g_cont = new Container(g_item.getId());
                    } else {
                        closeShowcaseForOthers(cp, g_cont);
                    }

                    cp->backPackContents->InsertContainer(g_item, g_cont, freeSlot);
                } else {
                    if (cp->createItem(g_item.getId(), g_item.getNumber(), g_item.getQuality(), &data_map) > 0) {
                        NOK = true;
                    } else {
                        if (g_item.isContainer()) {
                            if (g_cont == nullptr) {
                                g_cont = new Container(g_item.getId());
                            } else {
                                closeShowcaseForOthers(cp, g_cont);
                            }

                            if (!backpackPresent) {
                                delete cp->backPackContents;
                                cp->backPackContents = g_cont;
                            }
                        }

                        g_item.reset();
                        g_cont = nullptr;
                        cp->checkBurden();

                        if (script) {
                            script->MoveItemAfterMove(cp, s_item, t_item);
                        }
                    }
                }
            }

            if (NOK) {
                g_item = tempitem;

                if (!putItemOnMap(cp, itemPosition)) {
                    Logger::error(LogFacility::Player) << "moveItemFromMapIntoShowcase failed: item " << g_item.getId()
                                                       << " lost for " << *cp << Log::end;
                    g_cont = nullptr;
                    g_item.reset();
                }

                return false;
            }

            return true;
        }
    }

    return false;
}

void World::pickUpAllItemsFromMap(Player *cp) {
    for (short int dy = -1; dy <= 1; ++dy) {
        for (short int dx = -1; dx <= 1; ++dx) {
            auto pos = cp->getPosition();
            pos.x += dx;
            pos.y += dy;

            while (pickUpItemFromMap(cp, pos)) {
            }
        }
    }
}

void World::sendRemoveItemFromMapToAllVisibleCharacters(const position &itemPosition) {
    for (const auto &player : Players.findAllCharactersInScreen(itemPosition)) {
        ServerCommandPointer cmd = std::make_shared<ItemRemoveTC>(itemPosition);
        player->Connection->addCommand(cmd);
    }
}

void World::sendSwapItemOnMapToAllVisibleCharacter(TYPE_OF_ITEM_ID id, const position &itemPosition, const Item &it) {
    for (const auto &player : Players.findAllCharactersInScreen(itemPosition)) {
        ServerCommandPointer cmd = std::make_shared<ItemSwapTC>(itemPosition, id, it);
        player->Connection->addCommand(cmd);
    }
}

void World::sendPutItemOnMapToAllVisibleCharacters(const position &itemPosition, const Item &it) {
    for (const auto &player : Players.findAllCharactersInScreen(itemPosition)) {
        ServerCommandPointer cmd = std::make_shared<ItemPutTC>(itemPosition, it);
        player->Connection->addCommand(cmd);
    }
}

void World::sendContainerSlotChange(Container *cc, TYPE_OF_CONTAINERSLOTS slot, Container *moved) {
    if ((cc != nullptr) && (moved != nullptr)) {
        Players.for_each([cc, slot, moved](Player *player) {
            player->updateShowcaseSlot(cc, slot);
            player->closeShowcase(moved);
        });
    }
}

void World::sendContainerSlotChange(Container *cc, TYPE_OF_CONTAINERSLOTS slot) {
    if (cc != nullptr) {
        Players.for_each([cc, slot](Player *player) { player->updateShowcaseSlot(cc, slot); });
    }
}

void World::closeShowcaseForOthers(Player *target, Container *moved) {
    if (moved != nullptr) {
        Players.for_each([target, moved](Player *player) {
            if (target != player) {
                player->closeShowcase(moved);
            }
        });
    }
}

void World::closeShowcaseIfNotInRange(Container *moved, const position &showcasePosition) {
    if (moved != nullptr) {
        Players.for_each([&showcasePosition, moved](Player *player) {
            const auto &pos = player->getPosition();
            if (abs(showcasePosition.x - pos.x) > 1 || abs(showcasePosition.y - pos.y) > 1 ||
                showcasePosition.z != pos.z) {
                player->closeShowcase(moved);
            }
        });
    }
}
