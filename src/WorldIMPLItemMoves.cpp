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


#include "World.hpp"
#include "data/TilesModificatorTable.hpp"
#include "script/LuaItemScript.hpp"
#include "script/LuaTriggerScript.hpp"
#include "script/LuaDepotScript.hpp"
#include "netinterface/protocol/ServerCommands.hpp"
#include "netinterface/NetInterface.hpp"
#include <map>

// TODO find a better place for the constants
static const std::string message_overweight_german { "Du kannst nicht so viel tragen!"
};
static const std::string message_overweight_english { "You can't carry that much!"
};

extern std::shared_ptr<LuaDepotScript>depotScript;

//
//   Constraints:
//
//   Immer wenn ein Item/Container in g_item/g_cont verschoben wird, muss die Variable tempWeight auf das Gesamtgewicht
//   der bewegten Items gesetzt werden. Sind g_item/g_cont leer, dann ist auch tempWeight==0.
//
//   Das Gesamtgewicht aller Items eines Character (ohne die in g_item/g_cont) steht immer in sumWeight und muss entsprechend aktualisiert werden.

//  atomare Funktionen  //
bool World::putItemOnInvPos(Character *cc, unsigned char pos) {
    if (pos == BACKPACK) {
        if (cc->characterItems[ BACKPACK ].getId() == 0) {

            if (g_item.isContainer()) {
                cc->characterItems[ BACKPACK ] = g_item;
                cc->characterItems[ BACKPACK ].setNumber(1);

                if (g_cont == NULL) {
                    g_cont = new Container(g_item.getId());
                } else {
                    Player *temp = dynamic_cast<Player *>(cc);

                    if (temp) {
                        closeShowcaseForOthers(temp, g_cont);
                    }
                }

                cc->backPackContents = g_cont;

                g_item.reset();
                g_cont = NULL;
                cc->updateAppearanceForAll(true);
                return true;
            }
        }
    } else if (g_cont == NULL) {
        if (pos < MAX_BODY_ITEMS) {
            if (cc->characterItems[ pos ].getId() == 0 || cc->characterItems[pos].getId() == g_item.getId()) {
                if ((pos == RIGHT_TOOL) || (pos == LEFT_TOOL)) {
                    if (Data::WeaponItems.exists(g_item.getId())) {
                        const auto &weapon = Data::WeaponItems[g_item.getId()];

                        if ((weapon.WeaponType==4) || (weapon.WeaponType==5) || (weapon.WeaponType==6) || (weapon.WeaponType==13)) {
                            if ((pos == RIGHT_TOOL) && (cc->characterItems[ LEFT_TOOL ].getId() == 0)) {
                                if (cc->characterItems[pos].getId() == 0 && g_item.getNumber() == 1) {
                                    cc->characterItems[ pos ] = g_item;
                                    cc->characterItems[ LEFT_TOOL ].setId(BLOCKEDITEM);
                                    cc->characterItems[ LEFT_TOOL ].makePermanent();
                                    cc->characterItems[ LEFT_TOOL ].setNumber(1);
                                    g_item.reset();
                                    cc->updateAppearanceForAll(true);
                                    return true;
                                }
                                // we don't want to add any more two handers...
                                else {
                                    return false;
                                }
                            } else if ((pos == LEFT_TOOL) && (cc->characterItems[ RIGHT_TOOL ].getId() == 0)) {
                                if (cc->characterItems[pos].getId() == 0 && g_item.getNumber() == 1) {
                                    //std::cout << "Zweihaender rechts  platziert" << std::endl;
                                    cc->characterItems[ pos ] = g_item;
                                    cc->characterItems[ RIGHT_TOOL ].setId(BLOCKEDITEM);
                                    cc->characterItems[ RIGHT_TOOL ].makePermanent();
                                    cc->characterItems[ RIGHT_TOOL ].setNumber(1);
                                    g_item.reset();
                                    cc->updateAppearanceForAll(true);
                                    return true;
                                }
                                // we don't want to add any more two handers...
                                else {
                                    return false;
                                }
                            } else {
                                return false;
                            }
                        }
                    }

                    if (cc->characterItems[pos].getId() == 0) {
                        if (!isStackable(g_item) && !g_item.isContainer()) {
                            if (g_item.getNumber() > 1) {
                                return false;
                            }
                        }

                        cc->characterItems[ pos ] = g_item;
                        g_item.reset();

                        cc->updateAppearanceForAll(true);
                        return true;
                    } else {
                        if (!isStackable(g_item) && !g_item.isContainer()) {
                            return false;
                        }

                        if (!cc->characterItems[pos].equalData(g_item)) {
                            return false;
                        }

                        if (g_item.getNumber() + cc->characterItems[pos].getNumber() < 250) {
                            cc->characterItems[pos].setNumber(cc->characterItems[pos].getNumber() + g_item.getNumber());
                            cc->characterItems[pos].setMinQuality(g_item);
                            g_item.reset();
                            cc->updateAppearanceForAll(true);
                            return true;
                        } else {
                            return false;
                        }
                    }
                } else {
                    if (g_item.getNumber() == 1 && cc->characterItems[pos].getId() == 0) {
                        if (Data::ArmorItems.exists(g_item.getId())) {
                            const auto &armor = Data::ArmorItems[g_item.getId()];
                            unsigned char flag;

                            switch (pos) {
                            case HEAD :
                                flag = FLAG_HEAD;
                                break;

                            case NECK :
                                flag = FLAG_NECK;
                                break;

                            case BREAST :
                                flag = FLAG_BREAST;
                                break;

                            case HANDS :
                                flag = FLAG_HANDS;
                                break;

                            case FINGER_LEFT_HAND :
                                flag = FLAG_FINGER;
                                break;

                            case FINGER_RIGHT_HAND :
                                flag = FLAG_FINGER;
                                break;

                            case LEGS :
                                flag = FLAG_LEGS;
                                break;

                            case FEET :
                                flag = FLAG_FEET;
                                break;

                            case OAT :
                                flag = FLAG_COAT;
                                break;

                            default :
                                flag = 0xFF;
                                break;
                            }

                            if ((armor.BodyParts & flag) != 0) {
                                cc->characterItems[ pos ] = g_item;
                                g_item.reset();
                                cc->updateAppearanceForAll(true);
                                return true;
                            }
                        }
                    }
                }
            }
        } else if (pos < MAX_BODY_ITEMS + MAX_BELT_SLOTS) {
            if (cc->characterItems[ pos ].getId() == 0) {

                if (!isStackable(g_item) && !g_item.isContainer()) {
                    if (g_item.getNumber() > 1) {
                        return false;
                    }
                }

                cc->characterItems[ pos ] = g_item;
                g_item.reset();
                cc->updateAppearanceForAll(true);
                return true;
            } else if (cc->characterItems[ pos ].getId() == g_item.getId()) {

                if (!isStackable(g_item) && !g_item.isContainer()) {
                    return false;
                }

                if (!cc->characterItems[pos].equalData(g_item)) {
                    return false;
                }

                int temp = cc->characterItems[ pos ].getNumber() + g_item.getNumber();

                if (temp <= g_item.getMaxStack()) {
                    cc->characterItems[ pos ].setNumber(temp);
                    cc->characterItems[ pos ].setMinQuality(g_item);
                    g_item.reset();
                    cc->updateAppearanceForAll(true);
                    return true;
                }
            }
        }
    }

    return false;
}



bool World::putItemOnInvPos(Player *cc, unsigned char pos) {
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
    } else {
        return false;
    }
}

bool World::takeItemFromInvPos(Character *cc, unsigned char pos, Item::number_type count) {
    if (pos == BACKPACK) {
        if (cc->characterItems[ BACKPACK ].getId() != 0) {
            g_item = cc->characterItems[ BACKPACK ];
            g_cont = cc->backPackContents;

            if (g_cont == NULL) {
                g_cont = new Container(g_item.getId());
            }

            cc->characterItems[ BACKPACK ].reset();
            cc->backPackContents = NULL;
            cc->updateAppearanceForAll(true);
            return true;
        }
    } else if (pos < MAX_BODY_ITEMS + MAX_BELT_SLOTS) {
        if ((cc->characterItems[ pos ].getId() != 0) && (cc->characterItems[ pos ].getId() != BLOCKEDITEM)) {
            if ((pos == RIGHT_TOOL) || (pos == LEFT_TOOL)) {
                const auto weaponId = cc->characterItems[pos].getId();

                if (Data::WeaponItems.exists(weaponId)) {
                    g_item = cc->characterItems[ pos ];
                    g_cont = NULL;

                    if (!isStackable(g_item) && !g_item.isContainer()) {
                        if (g_item.getNumber() > 1 && count > 1) {
                            g_item.reset();
                            g_cont = NULL;
                            return false;
                        }
                    }

                    if (g_item.getNumber() > count) {
                        cc->characterItems[ pos ].setNumber(cc->characterItems[pos].getNumber() - count);
                        g_item.setNumber(count);
                    } else {
                        const auto &weapon = Data::WeaponItems[weaponId];

                        if ((weapon.WeaponType==4) || (weapon.WeaponType==5) || (weapon.WeaponType==6) || (weapon.WeaponType==13)) {
                            cc->characterItems[ LEFT_TOOL ].reset();
                            cc->characterItems[ RIGHT_TOOL ].reset();
                        } else {
                            cc->characterItems[ pos ].reset();
                        }
                    }

                    cc->updateAppearanceForAll(true);
                    return true;
                }
            }

            g_item = cc->characterItems[ pos ];
            g_cont = NULL;

            if (isStackable(g_item) && count > 1 && !g_item.isContainer()) {
                if (g_item.getNumber() > count) {
                    cc->characterItems[ pos ].setNumber(cc->characterItems[ pos ].getNumber() - count);
                    g_item.setNumber(count);
                } else {
                    cc->characterItems[ pos ].reset();
                }

                cc->updateAppearanceForAll(true);
                return true;
            } else {
                if (g_item.getNumber() > 1) {
                    cc->characterItems[ pos ].setNumber(cc->characterItems[ pos ].getNumber() - 1);
                    g_item.setNumber(1);
                } else {
                    cc->characterItems[ pos ].reset();
                }

                cc->updateAppearanceForAll(true);
                return true;
            }
        }
    } // Rucksack oder normal

    g_item.reset();
    g_cont = NULL;

    return false;
}



bool World::takeItemFromInvPos(Player *cc, unsigned char pos, Item::number_type count) {
    if (pos == BACKPACK) {
        if (cc->characterItems[ BACKPACK ].getId() != 0) {
            if (cc->backPackContents != NULL) {
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
    } else {
        return false;
    }
}



bool World::takeItemFromShowcase(Player *cc, uint8_t showcase, unsigned char pos, Item::number_type count) {
    Container *ps = cc->getShowcaseContainer(showcase);

    if (ps != NULL) {
        if (ps->TakeItemNr(pos, g_item, g_cont, count)) {
            if (g_cont != NULL) {
                sendChangesOfContainerContentsCM(ps, g_cont);
            } else {
                sendChangesOfContainerContentsIM(ps);
            }

            return true;
        }
    }

    g_item.reset();
    g_cont = NULL;

    return false;

}


bool World::putItemInShowcase(Player *cc, uint8_t showcase, TYPE_OF_CONTAINERSLOTS pos) {
    if (!isStackable(g_item) && !g_item.isContainer()) {
        if (g_item.getNumber() > 1) {
            return false;
        }
    }

    Container *ps = cc->getShowcaseContainer(showcase);

    if (ps) {
        if (g_cont) {

#ifdef World_BagOnlyInDepot

            for (const auto &depot : cc->depotContents) {
                if (depot.second == ps) {
                    isdepot = true;
                    break;
                }
            }

            if (isdepot)
#endif
            {
                if (ps != g_cont) {
                    if (ps->InsertContainer(g_item, g_cont, pos)) {
                        sendChangesOfContainerContentsCM(ps, g_cont);
                        g_item.reset();
                        g_cont = NULL;

                        return true;
                    }
                }
                //Falls man eine Tasche in eine Tasche legen will dies unterbinden
                else {
                    return false;
                }
            }
        } else {
            if (ps->InsertItem(g_item, pos)) {
                sendChangesOfContainerContentsIM(ps);
                g_item.reset();
#ifdef World_ItemMove_DEBUG
                std::cout << "putItemInShowcase.. Ende 2" << std::endl;
#endif
                return true;
            }
        }
    }

#ifdef World_ItemMove_DEBUG
    std::cout << "putItemInShowcase.. Ende 3" << std::endl;
#endif

    return false;

}



bool World::takeItemFromMap(Character *cc, short int x, short int y, short int z) {
    Field *tempf;

    if (GetPToCFieldAt(tempf, x, y, z, tmap)) {

        if (tempf->ViewTopItem(g_item)) {
            const auto &tempCommon = Data::CommonItems[g_item.getId()];

            if (tempCommon.isValid()) {
                if (tempCommon.Weight < 30000 && !g_item.isPermanent()) {
                    tempf->TakeTopItem(g_item);

                    if (!isStackable(g_item) && !g_item.isContainer()) {
                        if (g_item.getNumber() > 1) {
                            g_item.reset();
                            g_cont = NULL;
                            return false;
                        }

                    }

                    position pos(x, y, z);

                    if (Data::Triggers.exists(pos)) {
                        const auto &script = Data::Triggers.script(pos);

                        if (script) {
                            ScriptItem sItem = g_item;
                            sItem.pos = pos;
                            sItem.type = ScriptItem::it_field;
                            script->TakeItemFromField(sItem, cc);
                        }
                    }

                    g_item.resetWear();

                    if (g_item.isContainer()) {
                        MAP_POSITION opos;
                        opos.x = x;
                        opos.y = y;
                        Map::CONTAINERHASH::iterator conmapo = tmap->maincontainers.find(opos);

                        // containermap fr das Feld gefunden
                        if (conmapo != tmap->maincontainers.end()) {
                            Container::CONTAINERMAP::iterator iv = (*conmapo).second.find(g_item.getNumber());

                            // der Inhalt des angegebenen Containers mit der id g_item.getNumber() wurde gefunden
                            if (iv != (*conmapo).second.end()) {
                                g_cont = (*iv).second;

                                // Verweis auf den Container in der Containermap fr das Feld loeschen
                                (*conmapo).second.erase(iv);

                                if ((*conmapo).second.empty()) {
                                    // kein Container mehr auf dem Feld -> Containermap fr das Feld loeschen
                                    tmap->maincontainers.erase(conmapo);
                                }

                                sendRemoveItemFromMapToAllVisibleCharacters(cc->getId(), x, y, z, tempf);

                                return true;
                            } else {
                                g_cont = new Container(g_item.getId());
                                sendRemoveItemFromMapToAllVisibleCharacters(cc->getId(), x, y, z, tempf);

                                return true;
                            }
                        } else {
                            g_cont = new Container(g_item.getId());
                            sendRemoveItemFromMapToAllVisibleCharacters(cc->getId(), x, y, z, tempf);

                            return true;
                        }
                    } else {
                        // normales Item
                        g_cont = NULL;
                        sendRemoveItemFromMapToAllVisibleCharacters(cc->getId(), x, y, z, tempf);

                        return true;
                    }
                } // tragbar
            } // item bekannt
        } // Item vorhanden ?
    } // Feld vorhanden ?

    g_item.reset();
    g_cont = NULL;

    return false;
}


bool World::putItemOnMap(Character *cc, short int x, short int y, short int z) {
    Field *tempf;

    //only permit throwing within certain range and weight limits
    position pos(x, y, z);

    if (cc) {
        if (cc->pos.z != z ||
            !cc->isInRangeToField(pos, MAXTHROWDISTANCE) ||
            (!cc->isInRangeToField(pos, MAXDROPDISTANCE) && (g_item.getWeight() > MAXTHROWWEIGHT))) {
            static const std::string german = "Dies ist zu schwer um so weit geworfen zu werden.";
            static const std::string english = "This is too heavy to be thrown this far.";
            cc->inform(german, english, Character::informScriptMediumPriority);
            return false;
        }
    }

    //stacking von Items verhindern
    if (!isStackable(g_item) && !g_item.isContainer()) {
        if (g_item.getNumber() > 1) {
            return false;
        }
    }

    //Ende stacking von Items verhindern.

    if (GetPToCFieldAt(tempf, x, y, z, tmap)) {
        MAP_POSITION npos;
        npos.x = x;
        npos.y = y;

        if (Data::TilesModItems.nonPassable(g_item.getId())) {     // nicht passierbares Item, zB. eine grosse Kiste
            if (! tempf->moveToPossible()) {   // das Feld ist nicht betretbar

                return false;
            }
        }

        if (g_item.isContainer()) {
            // Container
            if (g_cont == NULL) {
                g_cont = new Container(g_item.getId());
            } else
                // close the showcase for everyone not in range
            {
                closeShowcaseIfNotInRange(g_cont,x,y,z);
            }

            if (tmap->addContainerToPos(g_item, g_cont, npos)) {
                sendPutItemOnMapToAllVisibleCharacters(x, y, z, g_item, tempf);
                position pos(x, y, z);

                if (cc && Data::Triggers.exists(pos)) {
                    const auto &script = Data::Triggers.script(pos);

                    if (script) {
                        ScriptItem sItem = g_item;
                        sItem.pos = position(x,y,z);
                        sItem.type = ScriptItem::it_field;
                        script->PutItemOnField(sItem,cc);
                    }
                }

                checkField(tempf, x, y, z);
                g_item.reset();
                g_cont = NULL;

                return true;
            }
        } else {
            if (tempf->addTopItem(g_item)) {
                sendPutItemOnMapToAllVisibleCharacters(x, y, z, g_item, tempf);
                position pos(x, y, z);

                if (cc && Data::Triggers.exists(pos)) {
                    const auto &script = Data::Triggers.script(pos);

                    if (script) {
                        ScriptItem sItem = g_item;
                        sItem.pos = position(x,y,z);
                        sItem.type = ScriptItem::it_field;
                        script->PutItemOnField(sItem,cc);
                    }
                }

                checkField(tempf, x, y, z);
                g_cont = NULL;
                g_item.reset();

                return true;
            }
        }
    }

    return false;

}


bool World::putItemAlwaysOnMap(Character *cc, short int x, short int y, short int z) {
    Field *tempf;

    if (GetPToCFieldAt(tempf, x, y, z, tmap)) {
        MAP_POSITION npos;
        npos.x = x;
        npos.y = y;

        if (g_item.isContainer()) {
            // Container
            if (g_cont == NULL) {
                g_cont = new Container(g_item.getId());
            }

            if (tmap->addAlwaysContainerToPos(g_item, g_cont, npos)) {
                sendPutItemOnMapToAllVisibleCharacters(x, y, z, g_item, tempf);
                position pos(x, y, z);

                if (cc && Data::Triggers.exists(pos)) {
                    const auto &script = Data::Triggers.script(pos);

                    if (script) {
                        ScriptItem sItem = g_item;
                        sItem.pos = position(x,y,z);
                        sItem.type = ScriptItem::it_field;
                        script->PutItemOnField(sItem,cc);
                    }
                }

                checkField(tempf, x, y, z);
                g_item.reset();
                g_cont = NULL;

                return true;
            }
        } else {
            if (tempf->PutTopItem(g_item)) {
                sendPutItemOnMapToAllVisibleCharacters(x, y, z, g_item, tempf);
                position pos(x, y, z);

                if (cc && Data::Triggers.exists(pos)) {
                    const auto &script = Data::Triggers.script(pos);

                    if (script) {
                        ScriptItem sItem;
                        sItem = g_item;
                        sItem.pos = position(x,y,z);
                        sItem.type = ScriptItem::it_field;
                        script->PutItemOnField(sItem,cc);
                    }
                }

                checkField(tempf, x, y, z);
                g_cont = NULL;
                g_item.reset();

                return true;
            }
        }
    }

    return false;

}



void World::checkField(Field *cfstart, short int x, short int y, short int z) {
    if (cfstart != NULL) {
        if (cfstart->HasSpecialItem()) {
            if (cfstart->IsPlayerOnField()) {
                Player *temp;

                if (Players.find(x, y, z, temp)) {
                    checkFieldAfterMove(temp, cfstart);
                }
            } else if (cfstart->IsMonsterOnField()) {
                Monster *temp2;

                if (Monsters.find(x, y, z, temp2)) {
                    checkFieldAfterMove(temp2, cfstart);
                }
            } else if (cfstart->IsNPCOnField()) {
                NPC *temp3;

                if (Npc.find(x, y, z, temp3)) {
                    checkFieldAfterMove(temp3, cfstart);
                }
            }
        }
    }
}


///////// zusammengesetzte Funktionen  ///////////////
void World::dropItemFromShowcaseOnMap(Player *cp, uint8_t showcase, unsigned char pos, short int xc, short int yc, short int zc, Item::number_type count) {
    if (count == 0) {
        return;
    }

    if (takeItemFromShowcase(cp, showcase, pos, count)) {
        ScriptItem s_item = g_item,t_item = g_item;
        s_item.pos = cp->pos;
        s_item.itempos = pos;
        s_item.type = ScriptItem::it_container;
        s_item.inside = cp->getShowcaseContainer(showcase);
        s_item.owner = cp;
        t_item.pos = position(xc, yc, zc);
        t_item.type = ScriptItem::it_field;
        t_item.owner = cp;
        std::shared_ptr<LuaItemScript> script = Data::CommonItems.script(t_item.getId());

        if (script && script->existsEntrypoint("MoveItemBeforeMove")) {
            if (!script->MoveItemBeforeMove(cp, s_item, t_item)) {
                if (!putItemInShowcase(cp, showcase, pos)) {
                    Logger::error(LogFacility::Player) << "dropItemFromShowcaseOnMap failed: item " << g_item.getId() << " lost for " << cp->to_string() << Log::end;
                    g_cont = NULL;
                    g_item.reset();
                }

                return;
            }
        }

        short int new_x = xc;
        short int new_y = yc;
        short int new_z = zc;

        if (!putItemOnMap(cp, new_x, new_y, new_z)) {
            if (!putItemInShowcase(cp, showcase, pos)) {
                Logger::error(LogFacility::Player) << "dropItemFromShowcaseOnMap failed: item " << g_item.getId() << " lost for " << cp->to_string() << Log::end;
                g_cont = NULL;
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



void World::moveItemFromShowcaseToPlayer(Player *cp, uint8_t showcase, unsigned char pos, unsigned char cpos, Item::number_type count) {
    if (count == 0) {
        return;
    }

    if (takeItemFromShowcase(cp, showcase, pos, count)) {
        ScriptItem s_item = g_item, t_item = g_item;
        s_item.pos = cp->pos;
        s_item.itempos = pos;
        s_item.type = ScriptItem::it_container;
        s_item.inside = cp->getShowcaseContainer(showcase);
        s_item.owner = cp;
        t_item.pos = cp->pos;
        t_item.itempos = cpos;

        if (cpos < MAX_BODY_ITEMS) {
            t_item.type = ScriptItem::it_inventory;
        } else {
            t_item.type = ScriptItem::it_belt;
        }

        t_item.owner = cp;
        std::shared_ptr<LuaItemScript> script = Data::CommonItems.script(t_item.getId());

        if (script && script->existsEntrypoint("MoveItemBeforeMove")) {
            if (!script->MoveItemBeforeMove(cp, s_item,t_item)) {
                if (!putItemInShowcase(cp, showcase, pos)) {
                    Logger::error(LogFacility::Player) << "moveItemFromShowcaseToPlayer failed: item " << g_item.getId() << " lost for " << cp->to_string() << Log::end;
                    g_cont = NULL;
                    g_item.reset();
                }

                return;
            }
        }

        bool NOK = false;

        if (! cp->weightOK(g_item.getId(), g_item.getNumber(), g_cont)) {
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
                Logger::error(LogFacility::Player) << "moveItemFromShowcaseToPlayer failed: item " << g_item.getId() << " lost for " << cp->to_string() << Log::end;
                g_cont = NULL;
                g_item.reset();
            }
        }
    }
}



void World::dropItemFromPlayerOnMap(Player *cp, unsigned char cpos, short int xc, short int yc, short int zc, Item::number_type count) {
    if (count == 0) {
        return;
    }

    if (takeItemFromInvPos(cp, cpos, count)) {
        ScriptItem s_item = g_item, t_item = g_item;
        s_item.pos = cp->pos;

        if (cpos < MAX_BODY_ITEMS) {
            s_item.type = ScriptItem::it_inventory;
        } else {
            s_item.type = ScriptItem::it_belt;
        }

        s_item.itempos = cpos;
        s_item.owner = cp;
        t_item.pos = position(xc, yc, zc);
        t_item.type = ScriptItem::it_field;
        t_item.owner = cp;
        std::shared_ptr<LuaItemScript> script = Data::CommonItems.script(t_item.getId());

        if (script && script->existsEntrypoint("MoveItemBeforeMove")) {
            if (!script->MoveItemBeforeMove(cp, s_item, t_item)) {
                if (!putItemOnInvPos(cp, cpos)) {
                    Logger::error(LogFacility::Player) << "dropItemFromPlayerOnMap failed: item " << g_item.getId() << " lost for " << cp->to_string() << Log::end;
                    g_cont = NULL;
                    g_item.reset();
                }

                return;
            }
        }

        short int new_x = xc;
        short int new_y = yc;
        short int new_z = zc;

        if (!putItemOnMap(cp, new_x, new_y, new_z)) {
            if (!putItemOnInvPos(cp, cpos)) {
                Logger::error(LogFacility::Player) << "dropItemFromPlayerOnMap failed: item " << g_item.getId() << " lost for " << cp->to_string() << Log::end;
                g_cont = NULL;
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



void World::dropItemFromMonsterOnMap(Monster *cm, unsigned char cpos, char xo, char yo, char zo, Item::number_type count) {
    if (count == 0) {
        return;
    }

    if (takeItemFromInvPos(cm, cpos, count)) {
        short int new_x = cm->pos.x + xo;
        short int new_y = cm->pos.y + yo;
        short int new_z = cm->pos.z + zo;

        if (!putItemOnMap(cm, new_x, new_y, new_z)) {
            if (!putItemOnInvPos(cm, cpos)) {
                Logger::error(LogFacility::World) << "dropItemFromMonsterOnMap failed: item " << g_item.getId() << " lost for " << cm->to_string() << Log::end;
                g_cont = NULL;
                g_item.reset();
            }
        }
    }
}



void World::moveItemBetweenBodyParts(Player *cp, unsigned char opos, unsigned char npos, Item::number_type count) {
    if (count == 0) {
        return;
    }

    if (takeItemFromInvPos(cp, opos, count)) {
        ScriptItem s_item = g_item, t_item = g_item;
        s_item.owner = cp;
        s_item.pos = cp->pos;

        if (opos < MAX_BODY_ITEMS) {
            s_item.type = ScriptItem::it_inventory;
        } else {
            s_item.type = ScriptItem::it_belt;
        }

        s_item.itempos = opos;
        t_item.owner = cp;
        t_item.pos = cp->pos;

        if (npos < MAX_BODY_ITEMS) {
            t_item.type = ScriptItem::it_inventory;
        } else {
            t_item.type = ScriptItem::it_belt;
        }

        t_item.itempos = npos;
        std::shared_ptr<LuaItemScript> script = Data::CommonItems.script(t_item.getId());

        if (script && script->existsEntrypoint("MoveItemBeforeMove")) {
            if (!script->MoveItemBeforeMove(cp, s_item, t_item)) {
                if (!putItemOnInvPos(cp, opos)) {
                    Logger::error(LogFacility::Player) << "moveItemBetweenBodyParts failed: item " << g_item.getId() << " lost for " << cp->to_string() << Log::end;
                    g_cont = NULL;
                    g_item.reset();
                }

                return;
            }
        }

        if (!putItemOnInvPos(cp, npos)) {
            if (!putItemOnInvPos(cp, opos)) {
                Logger::error(LogFacility::Player) << "moveItemBetweenBodyParts failed: item " << g_item.getId() << " lost for " << cp->to_string() << Log::end;
                g_cont = NULL;
                g_item.reset();
            }
        } else {
            if (script) {
                script->MoveItemAfterMove(cp, s_item, t_item);
            }
        }
    }
}



void World::moveItemFromPlayerIntoShowcase(Player *cp, unsigned char cpos, uint8_t showcase, unsigned char pos, Item::number_type count) {
    if (count == 0) {
        return;
    }

    if (takeItemFromInvPos(cp, cpos, count)) {
        ScriptItem s_item = g_item, t_item = g_item;

        if (cpos < MAX_BODY_ITEMS) {
            s_item.type = ScriptItem::it_inventory;
        } else {
            s_item.type = ScriptItem::it_belt;
        }

        s_item.pos = cp->pos;
        s_item.itempos = cpos;
        s_item.owner = cp;

        t_item.type = ScriptItem::it_container;
        t_item.inside = cp->getShowcaseContainer(showcase);
        t_item.pos = cp->pos;
        t_item.owner = cp;
        t_item.itempos = pos;
        std::shared_ptr<LuaItemScript> script = Data::CommonItems.script(t_item.getId());

        if (script && script->existsEntrypoint("MoveItemBeforeMove")) {
            if (!script->MoveItemBeforeMove(cp, s_item, t_item)) {
                if (!putItemOnInvPos(cp, cpos)) {
                    Logger::error(LogFacility::Player) << "moveItemFromPlayerIntoShowcase failed: item " << g_item.getId() << " lost for " << cp->to_string() << Log::end;
                    g_cont = NULL;
                    g_item.reset();
                }

                return;
            }
        }

        if (!putItemInShowcase(cp, showcase, pos)) {
            if (!putItemOnInvPos(cp, cpos)) {
                Logger::error(LogFacility::Player) << "moveItemFromPlayerIntoShowcase failed: item " << g_item.getId() << " lost for " << cp->to_string() << Log::end;
                g_cont = NULL;
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



void World::moveItemFromMapIntoShowcase(Player *cp, char direction, uint8_t showcase, unsigned char pos, Item::number_type count) {
    if (count == 0) {
        return;
    }

    if (direction < 11) {
        short int old_x = cp->pos.x + moveSteps[(int)direction ][ 0 ];
        short int old_y = cp->pos.y + moveSteps[(int)direction ][ 1 ];
        short int old_z = cp->pos.z + moveSteps[(int)direction ][ 2 ];

        if (takeItemFromMap(cp, old_x, old_y, old_z)) {
            ScriptItem s_item = g_item, t_item = g_item;
            s_item.pos = position(old_x, old_y, old_z);
            s_item.type = ScriptItem::it_field;
            s_item.owner = cp;
            t_item.pos = cp->pos;
            t_item.type = ScriptItem::it_container;
            t_item.inside = cp->getShowcaseContainer(showcase);
            t_item.itempos = pos;
            t_item.owner = cp;

            std::shared_ptr<LuaItemScript> script = Data::CommonItems.script(t_item.getId());

            if (script && script->existsEntrypoint("MoveItemBeforeMove")) {
                if (!script->MoveItemBeforeMove(cp, s_item, t_item)) {
                    if (!putItemOnMap(cp, old_x, old_y, old_z)) {
                        Logger::error(LogFacility::Player) << "moveItemFromMapIntoShowcase failed: item " << g_item.getId() << " lost for " << cp->to_string() << Log::end;
                        g_cont = NULL;
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

            if (! cp->weightOK(g_item.getId(), g_item.getNumber(), g_cont)) {
                cp->inform(message_overweight_german, message_overweight_english, Character::informScriptMediumPriority);
                NOK = true;
            }

            if (!NOK) {
                if (!putItemInShowcase(cp, showcase,pos)) {
                    NOK =true;
                } else {
                    cp->checkBurden();

                    if (script) {
                        script->MoveItemAfterMove(cp, s_item, t_item);
                    }
                }
            }

            if (NOK) {
                g_item = tempitem;

                if (!putItemOnMap(cp, old_x, old_y, old_z)) {
                    Logger::error(LogFacility::Player) << "moveItemFromMapIntoShowcase failed: item " << g_item.getId() << " lost for " << cp->to_string() << Log::end;
                    g_cont = NULL;
                    g_item.reset();
                }

                return;
            }

            if (count < tempitem.getNumber() && !tempitem.isContainer()) {
                g_item = tempitem;
                g_item.setNumber(g_item.getNumber() - count);

                if (!putItemOnMap(cp, old_x, old_y, old_z)) {
                    Logger::error(LogFacility::Player) << "moveItemFromMapIntoShowcase failed: item " << g_item.getId() << " lost for " << cp->to_string() << Log::end;
                    g_cont = NULL;
                    g_item.reset();
                    return;
                }
            }

            return;
        }
    }
}



void World::moveItemFromMapToPlayer(Player *cp, char direction, unsigned char cpos, Item::number_type count) {
    if (count == 0) {
        return;
    }

    if (direction < 11) {
        short int old_x = cp->pos.x + moveSteps[(int)direction ][ 0 ];
        short int old_y = cp->pos.y + moveSteps[(int)direction ][ 1 ];
        short int old_z = cp->pos.z + moveSteps[(int)direction ][ 2 ];

        if (takeItemFromMap(cp, old_x, old_y, old_z)) {
            ScriptItem s_item = g_item, t_item = g_item;
            s_item.pos = position(old_x, old_y, old_z);
            s_item.type = ScriptItem::it_field;
            s_item.owner = cp;
            t_item.pos = cp->pos;

            if (cpos < MAX_BODY_ITEMS) {
                t_item.type = ScriptItem::it_inventory;
            } else {
                t_item.type = ScriptItem::it_belt;
            }

            t_item.owner = cp;
            t_item.itempos = cpos;
            std::shared_ptr<LuaItemScript> script = Data::CommonItems.script(t_item.getId());

            if (script && script->existsEntrypoint("MoveItemBeforeMove")) {
                if (!script->MoveItemBeforeMove(cp, s_item, t_item)) {
                    if (!putItemOnMap(cp, old_x, old_y, old_z)) {
                        Logger::error(LogFacility::Player) << "moveItemFromMapToPlayer failed: item " << g_item.getId() << " lost for " << cp->to_string() << Log::end;
                        g_cont = NULL;
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

            if (! cp->weightOK(g_item.getId(), g_item.getNumber(), g_cont)) {
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
                g_item = tempitem;

                if (! putItemOnMap(cp, old_x, old_y, old_z)) {
                    Logger::error(LogFacility::Player) << "moveItemFromMapToPlayer failed: item " << g_item.getId() << " lost for " << cp->to_string() << Log::end;
                    g_cont = NULL;
                    g_item.reset();
                }

                return;
            }

            if (count < tempitem.getNumber() && !tempitem.isContainer()) {
                g_item = tempitem;
                g_item.setNumber(g_item.getNumber() - count);

                if (!putItemOnMap(cp, old_x, old_y, old_z)) {
                    Logger::error(LogFacility::Player) << "moveItemFromMapToPlayer failed: item " << g_item.getId() << " lost for " << cp->to_string() << Log::end;
                    g_cont = NULL;
                    g_item.reset();
                    return;
                }
            }

            return;
        }
    }
}


void World::moveItemBetweenShowcases(Player *cp, uint8_t source, unsigned char pos, uint8_t dest, unsigned char pos2, Item::number_type count) {
    if (count == 0) {
        return;
    }

    if (takeItemFromShowcase(cp, source, pos, count)) {
        ScriptItem s_item = g_item, t_item = g_item;
        s_item.pos = cp->pos;
        s_item.type = ScriptItem::it_container;
        s_item.inside = cp->getShowcaseContainer(source);
        s_item.itempos = pos;
        s_item.owner = cp;
        t_item.pos = cp->pos;
        t_item.type = ScriptItem::it_container;
        t_item.inside = cp->getShowcaseContainer(dest);
        t_item.itempos = pos2;
        t_item.owner = cp;
        std::shared_ptr<LuaItemScript> script = Data::CommonItems.script(t_item.getId());

        if (script && script->existsEntrypoint("MoveItemBeforeMove")) {
            if (!script->MoveItemBeforeMove(cp, s_item, t_item)) {
                if (!putItemInShowcase(cp, dest, pos2)) {
                    Logger::error(LogFacility::Player) << "moveItemBetweenShowcases failed: item " << g_item.getId() << " lost for " << cp->to_string() << Log::end;
                    g_cont = NULL;
                    g_item.reset();
                }

                return;
            }
        }

        bool NOK = false;

        if (cp->isShowcaseInInventory(dest)) {
            if (! cp->weightOK(g_item.getId(), g_item.getNumber(), g_cont)) {
                cp->inform(message_overweight_german, message_overweight_english, Character::informScriptMediumPriority);
                NOK=true;
            }
        }

        if (!NOK) {
            if (!putItemInShowcase(cp, dest, pos2)) {
                NOK=true;
            } else {
                cp->checkBurden();

                if (script) {
                    script->MoveItemAfterMove(cp, s_item, t_item);
                }
            }
        }

        if (NOK) {
            if (! putItemInShowcase(cp, source, pos)) {
                Logger::error(LogFacility::Player) << "moveItemBetweenShowcases failed: item " << g_item.getId() << " lost for " << cp->to_string() << Log::end;
                g_cont = NULL;
                g_item.reset();
            }
        }
    }
}


bool World::moveItem(Character *cc, unsigned char d, short int xc, short int yc, short int zc, Item::number_type count) {
    if (count == 0) {
        return false;
    }

    if ((d < 11) && (cc != NULL)) {
        short int old_x = cc->pos.x + moveSteps[ d ][ 0 ];
        short int old_y = cc->pos.y + moveSteps[ d ][ 1 ];
        short int old_z = cc->pos.z + moveSteps[ d ][ 2 ];

        short int new_x = xc;
        short int new_y = yc;
        short int new_z = zc;

        if (takeItemFromMap(cc, old_x, old_y, old_z)) {
            ScriptItem s_item = g_item, t_item = g_item;
            s_item.pos = position(old_x, old_y, old_z);
            s_item.type = ScriptItem::it_field;
            s_item.owner = cc;
            t_item.pos = position(new_x, new_y, new_z);
            t_item.type = ScriptItem::it_field;
            t_item.owner = cc;

            std::shared_ptr<LuaItemScript> script = Data::CommonItems.script(t_item.getId());

            if (script && script->existsEntrypoint("MoveItemBeforeMove")) {
                if (!script->MoveItemBeforeMove(cc, s_item, t_item)) {
                    if (!putItemOnMap(cc, old_x, old_y, old_z)) {
                        Logger::error(LogFacility::Player) << "moveItem failed: item " << g_item.getId() << " lost for " << cc->to_string() << Log::end;
                        g_cont = NULL;
                        g_item.reset();
                    }

                    return false;
                }
            }

            Item tempitem=g_item;

            if (count < g_item.getNumber()) {
                g_item.setNumber(count);
            }

            if (!putItemOnMap(cc, new_x, new_y, new_z)) {
                g_item = tempitem;

                if (!putItemOnMap(cc, old_x, old_y, old_z)) {
                    Logger::error(LogFacility::Player) << "moveItem failed: item " << g_item.getId() << " lost for " << cc->to_string() << Log::end;
                    g_cont = NULL;
                    g_item.reset();
                }

                return false;
            }

            if (count < tempitem.getNumber() && !tempitem.isContainer()) {
                g_item = tempitem;
                g_item.setNumber(g_item.getNumber() - count);

                if (!putItemOnMap(cc, old_x, old_y, old_z)) {
                    Logger::error(LogFacility::Player) << "moveItem failed: item " << g_item.getId() << " lost for " << cc->to_string() << Log::end;
                    g_cont = NULL;
                    g_item.reset();
                }

                return false;
            }

            if (script) {
                script->MoveItemAfterMove(cc, s_item, t_item);
            }

            return true;

        }
    }

    return false;
}


void World::lookIntoShowcaseContainer(Player *cp, uint8_t showcase, unsigned char pos) {
    if ((cp != NULL) && cp->isShowcaseOpen(showcase)) {
        Container *top = cp->getShowcaseContainer(showcase);
        bool allowedToOpenContainer = false;

        std::map<uint32_t,Container *>::iterator it;

        for (const auto &depot : cp->depotContents) {
            if (depot.second == top) {
                allowedToOpenContainer = true;
                break;
            }
        }

        if (top != NULL && allowedToOpenContainer) {
            Container *tempc;
            ScriptItem tempi;

            if (top->viewItemNr(pos, tempi, tempc)) {
                if (tempc != NULL) {
                    cp->openShowcase(tempc, cp->isShowcaseInInventory(showcase));
                }
            }
        }
    }
}



bool World::lookIntoBackPack(Player *cp) {
    if (cp) {
        if ((cp->characterItems[ BACKPACK ].getId() != 0) && (cp->backPackContents != NULL)) {
            cp->openShowcase(cp->backPackContents, true);
            return true;
        }
    }

    return false;
}


bool World::lookIntoContainerOnField(Player *cp, char direction) {
    if ((direction < 11) && (cp != NULL)) {
        short int old_x = moveSteps[(int)direction ][ 0 ] + cp->pos.x;
        short int old_y = moveSteps[(int)direction ][ 1 ] + cp->pos.y;
        short int old_z = moveSteps[(int)direction ][ 2 ] + cp->pos.z;

        Field *cfold;

        if (GetPToCFieldAt(cfold, old_x, old_y, old_z, tmap)) {
            Item titem;

            if (cfold->ViewTopItem(titem)) {
                if (titem.getId() != DEPOTITEM && titem.isContainer()) {
                    MAP_POSITION opos;
                    opos.x = old_x;
                    opos.y = old_y;
                    Map::CONTAINERHASH::iterator conmapo = tmap->maincontainers.find(opos);

                    if (conmapo != tmap->maincontainers.end()) {
                        Container::CONTAINERMAP::iterator iv = (*conmapo).second.find(titem.getNumber());

                        if (iv != (*conmapo).second.end()) {
                            cp->openShowcase((*iv).second, false);
                            return true;
                        }
                    }
                } else {
                    if (titem.getId() == DEPOTITEM) {
                        if (depotScript && depotScript->existsEntrypoint("onOpenDepot")) {
                            if (depotScript->onOpenDepot(cp, titem)) {
                                cp->openDepot(titem.getDepot());
                            }
                        } else {
                            cp->openDepot(titem.getDepot());
                        }
                    } else {

                    }
                }
            }
        }
    }

    return false;
}



void World::closeContainerInShowcase(Player *cp, uint8_t showcase) {
    if (cp) {
        cp->closeShowcase(showcase);
    }
}


void World::sendRemoveItemFromMapToAllVisibleCharacters(TYPE_OF_ITEM_ID id, short int xo, short int yo, short int zo, Field *cfp) {
    if (cfp) {
        for (const auto &player : Players.findAllCharactersInScreen(xo, yo, zo)) {
            ServerCommandPointer cmd(new ItemRemoveTC(xo, yo, zo));
            player->Connection->addCommand(cmd);
        }
    }
}


void World::sendSwapItemOnMapToAllVisibleCharacter(TYPE_OF_ITEM_ID id, short int xn, short int yn, short int zn, Item &it, Field *cfp) {
    if (cfp) {
        std::vector < Player * > temp = Players.findAllCharactersInScreen(xn, yn, zn);

        std::vector < Player * > ::iterator titerator;

        for (const auto &player : Players.findAllCharactersInScreen(xn, yn, zn)) {
            ServerCommandPointer cmd(new ItemSwapTC(xn, yn, zn, id, it));
            player->Connection->addCommand(cmd);
        }
    }
}

void World::sendPutItemOnMapToAllVisibleCharacters(short int xn, short int yn, short int zn, Item &it, Field *cfp) {
    if (cfp) {
        for (const auto &player : Players.findAllCharactersInScreen(xn, yn, zn)) {
            ServerCommandPointer cmd(new ItemPutTC(xn, yn, zn, it));
            player->Connection->addCommand(cmd);
        }
    }
}


void World::sendChangesOfContainerContentsCM(Container *cc, Container *moved) {
    if (cc && moved) {
        for (const auto &player : Players) {
            player->updateShowcase(cc);
            player->closeShowcase(moved);
        }
    }
}

void World::sendChangesOfContainerContentsIM(Container *cc) {
    if (cc) {
        for (const auto &player : Players) {
            player->updateShowcase(cc);
        }
    }
}

void World::closeShowcaseForOthers(Player *target, Container *moved) {
    if (moved) {
        PLAYERVECTOR::iterator titerator;

        for (const auto &player : Players) {
            if (target == player) {
                continue;
            }

            player->closeShowcase(moved);
        }
    }
}

void World::closeShowcaseIfNotInRange(Container *moved, short int x, short int y, short int z) {
    if (moved) {
        for (const auto &player : Players) {
            if (abs(x-player->pos.x) <= 1 && abs(y-player->pos.y) <= 1 && z == player->pos.z) {
                continue;
            }

            player->closeShowcase(moved);
        }
    }
}

bool World::isStackable(Item item) {
    const auto &com = Data::CommonItems[item.getId()];
    return com.MaxStack > 1;
}
