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
#include "data/ContainerObjectTable.hpp"
#include "data/WeaponObjectTable.hpp"
#include "data/ArmorObjectTable.hpp"
#include "data/CommonObjectTable.hpp"
#include "data/TilesModificatorTable.hpp"
#include "script/LuaItemScript.hpp"
#include "script/LuaTriggerScript.hpp"
#include "script/LuaDepotScript.hpp"
#include "netinterface/protocol/ServerCommands.hpp"
#include "netinterface/NetInterface.hpp"
#include <map>

extern boost::shared_ptr<LuaDepotScript>depotScript;

//
//   Constraints:
//
//   Immer wenn ein Item/Container in g_item/g_cont verschoben wird, muss die Variable tempWeight auf das Gesamtgewicht
//   der bewegten Items gesetzt werden. Sind g_item/g_cont leer, dann ist auch tempWeight==0.
//
//   Das Gesamtgewicht aller Items eines Character (ohne die in g_item/g_cont) steht immer in sumWeight und muss entsprechend aktualisiert werden.

//  atomare Funktionen  //
bool World::putItemOnInvPos(Character *cc, unsigned char pos) {
    bool gotWeapon = false;

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
                    if (WeaponItems->find(g_item.getId(), tempWeapon)) {
                        //We take a weapon
                        gotWeapon = true;

                        if ((tempWeapon.WeaponType==4) || (tempWeapon.WeaponType==5) || (tempWeapon.WeaponType==6) || (tempWeapon.WeaponType==13)) {
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
                        if (ArmorItems->find(g_item.getId(), tempArmor)) {
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

                            if ((tempArmor.BodyParts & flag) != 0) {
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

                if (temp <= MAXITEMS) {
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
    if (putItemOnInvPos((Character *) cc, pos)) {
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

bool World::takeItemFromInvPos(Character *cc, unsigned char pos, unsigned char count) {
    bool takeWeapon = false;

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
                if (WeaponItems->find(cc->characterItems[ pos ].getId(), tempWeapon)) {
                    takeWeapon = true;
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

                        if ((tempWeapon.WeaponType==4) || (tempWeapon.WeaponType==5) || (tempWeapon.WeaponType==6) || (tempWeapon.WeaponType==13)) {
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



bool World::takeItemFromInvPos(Player *cc, unsigned char pos, unsigned char count) {
    if (pos == BACKPACK) {
        if (cc->characterItems[ BACKPACK ].getId() != 0) {
            if (cc->backPackContents != NULL) {
                for (int i = 0; i < MAXSHOWCASES; ++i) {
                    if (cc->showcases[ i ].contains(cc->backPackContents)) {
                        cc->showcases[ i ].clear();
                        boost::shared_ptr<BasicServerCommand>cmd(new ClearShowCaseTC(i));
                        cc->Connection->addCommand(cmd);
                    }
                }
            }
        }
    }

    if (takeItemFromInvPos((Character *) cc, pos, count)) {
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



bool World::takeItemFromShowcase(Player *cc, unsigned char showcase, unsigned char pos, unsigned char count) {
    if (showcase < MAXSHOWCASES) {
        Container *ps = cc->showcases[ showcase ].top();

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
    }

    g_item.reset();
    g_cont = NULL;

    return false;

}


bool World::putItemInShowcase(Player *cc, unsigned char showcase, TYPE_OF_CONTAINERSLOTS pos) {
    if (!isStackable(g_item) && !g_item.isContainer()) {
        if (g_item.getNumber() > 1) {
            return false;
        }
    }

    if (showcase < MAXSHOWCASES) {
        Container *ps = cc->showcases[ showcase ].top();

        if (ps != NULL) {
            if (g_cont != NULL) {

#ifdef World_BagOnlyInDepot

                for (auto it = cc->depotContents.begin(); it != cc->depotContents.end(); ++it) {
                    if (it->second == ps) {
                        isdepot = true;
                        break;
                    }
                }

                if (isdepot)
#endif
                {
                    if (!cc->showcases[ showcase ].contains(g_cont)) {
                        if (ps->InsertContainer(g_item, g_cont)) {
                            sendChangesOfContainerContentsCM(ps, g_cont);
                            g_item.reset();
                            g_cont = NULL;

                            if (cc->showcases[ showcase ].inInventory()) {}

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
            if (CommonItems->find(g_item.getId(), tempCommon)) {
                if (tempCommon.Weight < 30000 && !g_item.isPermanent()) {
                    tempf->TakeTopItem(g_item);

                    if (!isStackable(g_item) && !g_item.isContainer()) {
                        if (g_item.getNumber() > 1) {
                            g_item.reset();
                            g_cont = NULL;
                            return false;
                        }

                    }

                    TriggerStruct Trigger;

                    if (Triggers->find(position(x,y,z),Trigger)) {
                        if (Trigger.script) {
                            ScriptItem sItem = g_item;
                            sItem.pos = position(x,y,z);
                            sItem.type = ScriptItem::it_field;
                            Trigger.script->TakeItemFromField(sItem,cc);
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

                                sendRemoveItemFromMapToAllVisibleCharacters(cc->id, x, y, z, tempf);

                                return true;
                            } else {
                                g_cont = new Container(g_item.getId());
                                sendRemoveItemFromMapToAllVisibleCharacters(cc->id, x, y, z, tempf);

                                return true;
                            }
                        } else {
                            g_cont = new Container(g_item.getId());
                            sendRemoveItemFromMapToAllVisibleCharacters(cc->id, x, y, z, tempf);

                            return true;
                        }
                    } else {
                        // normales Item
                        g_cont = NULL;
                        sendRemoveItemFromMapToAllVisibleCharacters(cc->id, x, y, z, tempf);

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
            (!cc->isInRangeToField(pos, MAXDROPDISTANCE) && g_item.getNumber() * g_item.getWeight() > MAXTHROWWEIGHT)) {
            std::string german = "Dies ist zu schwer um so weit geworfen zu werden.";
            std::string english = "This is too heavy to be thrown this far.";
            cc->informLua(german, english);
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

        if (TilesModItems->nonPassable(g_item.getId())) {     // nicht passierbares Item, zB. eine grosse Kiste
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
                //Ausfhren eines Triggerscriptes. Falls eins fr das Feld vorhanden ist.
                //Dies prft nicht nach ob ggf das entfernen des Items nicht geklappt hat
                TriggerStruct Trigger;

                if (cc && Triggers->find(position(x,y,z),Trigger)) {
                    if (Trigger.script) {
                        ScriptItem sItem = g_item;
                        sItem.pos = position(x,y,z);
                        sItem.type = ScriptItem::it_field;
                        Trigger.script->PutItemOnField(sItem,cc);
                    }
                }

                //=======================Ende des Triggerscriptes
                checkField(tempf, x, y, z);
                g_item.reset();
                g_cont = NULL;

                return true;
            }
        } else {
            // normales Item
            if (tempf->addTopItem(g_item)) {
                sendPutItemOnMapToAllVisibleCharacters(x, y, z, g_item, tempf);
                //Ausfhren eines Triggerscriptes. Falls eins fr das Feld vorhanden ist.
                //Dies prft nicht nach ob ggf das entfernen des Items nicht geklappt hat
                TriggerStruct Trigger;

                if (cc && Triggers->find(position(x,y,z),Trigger)) {
                    if (Trigger.script) {
                        ScriptItem sItem = g_item;
                        sItem.pos = position(x,y,z);
                        sItem.type = ScriptItem::it_field;
                        Trigger.script->PutItemOnField(sItem,cc);
                    }
                }

                //=======================Ende des Triggerscriptes
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
                //Ausfhren eines Triggerscriptes. Falls eins fr das Feld vorhanden ist.
                //Dies prft nicht nach ob ggf das entfernen des Items nicht geklappt hat
                TriggerStruct Trigger;

                if (cc && Triggers->find(position(x,y,z),Trigger)) {
                    if (Trigger.script) {
                        ScriptItem sItem = g_item;
                        sItem.pos = position(x,y,z);
                        sItem.type = ScriptItem::it_field;
                        Trigger.script->PutItemOnField(sItem,cc);
                    }
                }

                //=======================Ende des Triggerscriptes
                checkField(tempf, x, y, z);
                g_item.reset();
                g_cont = NULL;

                return true;
            }
        } else {
            // normales Item
            if (tempf->PutTopItem(g_item)) {
                sendPutItemOnMapToAllVisibleCharacters(x, y, z, g_item, tempf);
                //Ausfhren eines Triggerscriptes. Falls eins fr das Feld vorhanden ist.
                //Dies prft nicht nach ob ggf das entfernen des Items nicht geklappt hat
                TriggerStruct Trigger;

                if (cc && Triggers->find(position(x,y,z),Trigger)) {
                    if (Trigger.script) {
                        ScriptItem sItem;
                        sItem = g_item;
                        sItem.pos = position(x,y,z);
                        sItem.type = ScriptItem::it_field;
                        Trigger.script->PutItemOnField(sItem,cc);
                    }
                }

                //=======================Ende des Triggerscriptes
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
void World::dropItemFromShowcaseOnMap(Player *cp, unsigned char showcase, unsigned char pos, short int xc, short int yc, short int zc, unsigned char count) {
#ifdef World_ItemMove_DEBUG
    std::cout << "dropItemFromShowcaseOnMap: Spieler " << cp->name << " wirft ein Item auf die Karte" << std::endl;
#endif

    if (count == 0) {
        return;
    }

    if (takeItemFromShowcase(cp, showcase, pos, count)) {
        std::cout << "dropItemFromShowcaseOnMap: Item genommen" << std::endl;
        //CommonStruct com;
        //Ausfhren eines MoveItemScripts
        ScriptItem s_item = g_item,t_item = g_item; //Item einmal source und einmal target, das erste ist das Item vor dem bewegen das andere nach dem bewegen
        s_item.pos = cp->pos;
        s_item.itempos = pos;

        if (showcase == 0) {
            s_item.type = ScriptItem::it_showcase1;
        } else {
            s_item.type = ScriptItem::it_showcase2;
        }

        s_item.owner = cp;
        t_item.pos = position(xc, yc, zc);
        t_item.type = ScriptItem::it_field;
        t_item.owner = cp;
        boost::shared_ptr<LuaItemScript> script = CommonItems->findScript(t_item.getId());

        if (script && script->existsEntrypoint("MoveItemBeforeMove")) {
            if (!script->MoveItemBeforeMove(cp, s_item, t_item)) {
                if (!putItemInShowcase(cp, showcase,0)) {
                    std::cerr<<"DropItemFromShowcase wurde von Script unterbunden. Datenverlust beim zurcklegen! Spieler: "<<cp->name<<std::endl;
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
            std::cout << "Item konnte nicht auf die Karte gelegt werden -> zurcklegen" << std::endl;

            if (!putItemInShowcase(cp, showcase,0)) {
                std::cerr << "dropItemFromShowcaseOnMap: Datenverlust beim Zurcklegen, Spieler " << cp->name << std::endl;
                g_cont = NULL;
                g_item.reset();
            }
        } else {

            if (script) {
                script->MoveItemAfterMove(cp, s_item, t_item);
            }
        }
    }

#ifdef World_ItemMove_DEBUG
    std::cout << "dropItemFromShowcaseOnMap: beendet" << std::endl;
#endif
}



void World::moveItemFromShowcaseToPlayer(Player *cp, unsigned char showcase, unsigned char pos, unsigned char cpos, unsigned char count) {
    bool NOK = false;
    //CommonStruct com;
#ifdef World_ItemMove_DEBUG
    std::cout << "moveItemFromShowcaseToPlayer: Spieler " << cp->name << " nimmt ein Item auf\n"
              << "showcase: " << (short int) showcase << " from position: " << (short int) pos << " to position: " << (short int) cpos << std::endl;
#endif

    if (count == 0) {
        return;
    }

    if (takeItemFromShowcase(cp, showcase, pos, count)) {
#ifdef World_ItemMove_DEBUG
        std::cout << "Item genommen" << std::endl;
#endif

        //Ausfhren eines MoveItemScripts
        //Erzeugen von source und TargetItem
        ScriptItem s_item = g_item, t_item = g_item;
        s_item.pos = cp->pos;
        s_item.itempos = pos;

        if (showcase == 0) {
            s_item.type = ScriptItem::it_showcase1;
        } else {
            s_item.type = ScriptItem::it_showcase2;
        }

        s_item.owner = cp;
        t_item.pos = cp->pos;
        t_item.itempos = cpos;

        if (cpos < MAX_BODY_ITEMS) {
            t_item.type = ScriptItem::it_inventory;
        } else {
            t_item.type = ScriptItem::it_belt;
        }

        t_item.owner = cp;
        //Ende Erzeugen von Source und Target Item
        boost::shared_ptr<LuaItemScript> script = CommonItems->findScript(t_item.getId());

        if (script && script->existsEntrypoint("MoveItemBeforeMove")) {
            if (!script->MoveItemBeforeMove(cp, s_item,t_item)) {
                if (!putItemInShowcase(cp, showcase,0)) {
                    std::cerr<<"MoveItemFromShowcaseToPlayer wurde von Script unterbunden. Datenverlust beim zurcklegen! Spieler: "<<cp->name<<std::endl;
                    g_cont = NULL;
                    g_item.reset();
                }

                return;
            }
        }

        if (! cp->weightOK(g_item.getId(), g_item.getNumber(), g_cont)) {
            message(zuschwer, cp);
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

                //Ende des SCripte nach erfolgreichen ausfhren des Moves.
            }
        }

        if (NOK) {
            if (!putItemInShowcase(cp, showcase,0)) {
                std::cerr << "moveItemFromShowcaseToPlayer: Datenverlust beim Zurcklegen, Spieler " << cp->name << std::endl;
                g_cont = NULL;
                g_item.reset();
            }
        }
    }

#ifdef World_ItemMove_DEBUG
    std::cout << "moveItemFromShowcaseToPlayer: Ende" << std::endl;
#endif
}



void World::dropItemFromPlayerOnMap(Player *cp, unsigned char cpos, short int xc, short int yc, short int zc, unsigned char count) {
#ifdef World_ItemMove_DEBUG
    std::cout << "dropItemFromPlayerOnMap: Spieler " << cp->name << " wirft ein Item auf die Karte" << std::endl;
#endif

    if (count == 0) {
        return;
    }

    //CommonStruct com;
    if (takeItemFromInvPos(cp, cpos, count)) {
        std::cout << "dropItemFromPlayerOnMap Item genommen" << std::endl;
        //Ausfhren eines MoveItemScripts
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
        boost::shared_ptr<LuaItemScript> script = CommonItems->findScript(t_item.getId());

        if (script && script->existsEntrypoint("MoveItemBeforeMove")) {
            if (!script->MoveItemBeforeMove(cp, s_item, t_item)) {
                //std::cout<<"Legen des Items vom Spieler auf Karte vom Script unterbunden, zurck legen"<<std::endl;
                if (!putItemOnInvPos(cp, cpos)) {
                    std::cerr<<"MoveItemFromPlayerOnMap wurde von Script unterbunden. Datenverlust beim zurcklegen! Spieler: "<<cp->name<<std::endl;
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
            std::cout << "Item konnte nicht auf die Karte gelegt werden -> zurcklegen" << std::endl;

            if (!putItemOnInvPos(cp, cpos)) {
                std::cerr << "dropItemFromPlayerOnMap: Datenverlust beim Zurcklegen, Spieler " << cp->name << std::endl;
                g_cont = NULL;
                g_item.reset();
            }
        } else if (script) {
            script->MoveItemAfterMove(cp, s_item, t_item);
        }

    }

#ifdef World_ItemMove_DEBUG
    std::cout << "dropItemFromPlayerOnMap: Ende" << std::endl;
#endif
}



void World::dropItemFromMonsterOnMap(Monster *cm, unsigned char cpos, char xo, char yo, char zo, unsigned char count) {
#ifdef World_ItemMove_DEBUG
    std::cout << "dropItemFromMonsterOnMap: Monster " << cm->name << " wirft ein Item auf die Karte" << std::endl;
#endif

    if (count == 0) {
        return;
    }

    if (takeItemFromInvPos(cm, cpos, count)) {          // take item from monster position
        std::cout << "Item genommen" << std::endl;
        short int new_x = cm->pos.x + xo;               // select coordinates for item to drop
        short int new_y = cm->pos.y + yo;
        short int new_z = cm->pos.z + zo;

        if (!putItemOnMap(cm, new_x, new_y, new_z)) {      // put item there
            std::cout << "Item konnte nicht auf die Karte gelegt werden -> zurcklegen" << std::endl;

            if (!putItemOnInvPos(cm, cpos)) {
                std::cerr << "dropItemFromMonsterOnMap: Datenverlust beim Zurcklegen, Monster " << cm->name << std::endl;
                g_cont = NULL;
                g_item.reset();
            }
        }
    }

#ifdef World_ItemMove_DEBUG
    std::cout << "dropItemFromMonsterOnMap: Ende" << std::endl;
#endif
}



void World::moveItemBetweenBodyParts(Player *cp, unsigned char opos, unsigned char npos, unsigned char count) {
#ifdef World_ItemMove_DEBUG
    std::cout << "moveItemBetweenBodyParts: Spieler " << cp->name << " verschiebt ein Item" << std::endl;
#endif

    //CommonStruct com;
    if (count == 0) {
        return;
    }

    if (takeItemFromInvPos(cp, opos, count)) {
        std::cout << "Item genommen" << std::endl;
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
        boost::shared_ptr<LuaItemScript> script = CommonItems->findScript(t_item.getId());

        if (script && script->existsEntrypoint("MoveItemBeforeMove")) {
            if (!script->MoveItemBeforeMove(cp, s_item, t_item)) {
                if (!putItemOnInvPos(cp, opos)) {
                    std::cerr<<"MoveItemFromPlayerOnMap wurde von Script unterbunden. Datenverlust beim zurcklegen! Spieler: "<<cp->name<<std::endl;
                    g_cont = NULL;
                    g_item.reset();
                }

                return;
            }
        }

        if (!putItemOnInvPos(cp, npos)) {
            std::cout << "Item konnte nicht verschoben werden -> zurcklegen" << std::endl;

            if (!putItemOnInvPos(cp, opos)) {
                std::cerr << "moveItemBetweenBodyParts: Datenverlust beim Zurcklegen, Spieler " << cp->name << std::endl;
                g_cont = NULL;
                g_item.reset();
            }
        } else {
            // if (com.script) com.script->MoveItemAfterMove(cp, s_item, t_item);
            if (script) {
                script->MoveItemAfterMove(cp, s_item, t_item);
            }
        }
    }

#ifdef World_ItemMove_DEBUG
    std::cout << "moveItemBetweenBodyParts: Ende" << std::endl;
#endif
}



void World::moveItemFromPlayerIntoShowcase(Player *cp, unsigned char cpos, unsigned char showcase, unsigned char pos, unsigned char count) {
#ifdef World_ItemMove_DEBUG
    std::cout << "moveItemFromPlayerIntoShowcase: Spieler " << cp->name << " verschiebt Item von der Karte in ein showcase" << std::endl;
#endif
    //  CommonStruct com;

    if (count == 0) {
        return;
    }

    if (takeItemFromInvPos(cp, cpos, count)) {
        std::cout << "Item genommen" << std::endl;
        //Ausfhren eines Move Item Scriptes
        ScriptItem s_item = g_item, t_item = g_item;

        if (cpos < MAX_BODY_ITEMS) {
            s_item.type = ScriptItem::it_inventory;
        } else {
            s_item.type = ScriptItem::it_belt;
        }

        s_item.pos = cp->pos;
        s_item.itempos = cpos;
        s_item.owner = cp;

        if (showcase == 0) {
            t_item.type = ScriptItem::it_showcase1;
        } else {
            t_item.type = ScriptItem::it_showcase2;
        }

        t_item.pos = cp->pos;
        t_item.owner = cp;
        t_item.itempos = pos;
        boost::shared_ptr<LuaItemScript> script = CommonItems->findScript(t_item.getId());

        if (script && script->existsEntrypoint("MoveItemBeforeMove")) {
            if (!script->MoveItemBeforeMove(cp, s_item, t_item)) {
                if (!putItemOnInvPos(cp, cpos)) {
                    std::cerr<<"MoveItemFromPlayerOnMap wurde von Script unterbunden. Datenverlust beim zurcklegen! Spieler: "<<cp->name<<std::endl;
                    g_cont = NULL;
                    g_item.reset();
                }

                return;
            }
        }

        if (!putItemInShowcase(cp, showcase, pos)) {
            std::cout << "Item konnte nicht eingefgt werden -> zurcklegen" << std::endl;

            if (!putItemOnInvPos(cp, cpos)) {
                std::cerr << "moveItemFromPlayerIntoShowcase: Datenverlust beim Zurcklegen, Spieler " << cp->name << std::endl;
                g_cont = NULL;
                g_item.reset();
            }
        } else {
            if (script) {
                script->MoveItemAfterMove(cp, s_item, t_item);
            }
        }
    }

#ifdef World_ItemMove_DEBUG
    std::cout << "moveItemFromPlayerIntoShowcase: Ende" << std::endl;
#endif
}



void World::moveItemFromMapIntoShowcase(Player *cp, char direction, unsigned char showcase, unsigned char pos, unsigned char count) {

    bool NOK = false;
    //CommonStruct com;

#ifdef World_ItemMove_DEBUG
    std::cout << "moveItemFromMapIntoShowcase: Spieler " << cp->name << " verschiebt Item von der Karte in ein showcase" << std::endl;
#endif

    if (count == 0) {
        return;
    }

    if (direction < 11) {
        short int old_x = cp->pos.x + moveSteps[(int)direction ][ 0 ];
        short int old_y = cp->pos.y + moveSteps[(int)direction ][ 1 ];
        short int old_z = cp->pos.z + moveSteps[(int)direction ][ 2 ];

        if (takeItemFromMap(cp, old_x, old_y, old_z)) {
            std::cout << "Item genommen" << std::endl;
            ScriptItem s_item = g_item, t_item = g_item;
            s_item.pos = position(old_x, old_y, old_z);
            s_item.type = ScriptItem::it_field;
            s_item.owner = cp;
            t_item.pos = cp->pos;

            if (showcase == 0) {
                t_item.type = ScriptItem::it_showcase1;
            } else {
                t_item.type = ScriptItem::it_showcase2;
            }

            t_item.itempos = pos;
            t_item.owner = cp;

            //Ausfhren eines Move Item Scriptes
            boost::shared_ptr<LuaItemScript> script = CommonItems->findScript(t_item.getId());

            if (script && script->existsEntrypoint("MoveItemBeforeMove")) {
                if (!script->MoveItemBeforeMove(cp, s_item, t_item)) {
                    if (!putItemOnMap(cp, old_x, old_y, old_z)) {
                        std::cerr<<"MoveItemFromMapIntoShowcase wurde von Script unterbunden. Datenverlust beim zurcklegen! Spieler: "<<cp->name<<std::endl;
                        g_cont = NULL;
                        g_item.reset();
                    }

                    return;
                }
            }

            Item tempitem = g_item;

            if (count < g_item.getNumber()) {
                std::cout << "nicht alles verschieben" << std::endl;
                g_item.setNumber(count);
            }

            if (! cp->weightOK(g_item.getId(), g_item.getNumber(), g_cont)) {
                message(zuschwer, cp);
                NOK = true;
            }

            if (!NOK) {
                if (!putItemInShowcase(cp, showcase,pos)) {
                    NOK =true;
                }

                cp->checkBurden();

                if (script) {
                    script->MoveItemAfterMove(cp, s_item, t_item);
                }


            }

            if (NOK) {
                std::cout << "Item konnte nicht eingefgt werden -> zurcklegen" << std::endl;
                g_item = tempitem;

                if (!putItemOnMap(cp, old_x, old_y, old_z)) {
                    std::cerr << "moveItemFromMapIntoShowcase: Datenverlust beim Zurcklegen, Spieler " << cp->name << std::endl;
                    g_cont = NULL;
                    g_item.reset();
                }

                return;
            }

            if (count < tempitem.getNumber() && !tempitem.isContainer()) {
                g_item = tempitem;
                g_item.setNumber(g_item.getNumber() - count);

                if (!putItemOnMap(cp, old_x, old_y, old_z)) {
                    std::cerr << "moveItemFromMapIntoShowcase : Datenverlust beim teilweisen Verschieben, Spieler " << cp->name << std::endl;
                    g_cont = NULL;
                    g_item.reset();
                    return;
                }
            }

            return;
        }
    }

#ifdef World_ItemMove_DEBUG
    std::cout << "moveItemFromMapIntoShowcase: Ende" << std::endl;
#endif

    return;

}



void World::moveItemFromMapToPlayer(Player *cp, char direction, unsigned char cpos, unsigned char count) {
    bool NOK = false;
    //CommonStruct com;

#ifdef World_ItemMove_DEBUG
    std::cout << "moveItemFromMapToPlayer: Spieler " << cp->name << " verschiebt Item von der Karte an den Koerper" << std::endl;
#endif

    if (count == 0) {
        return;
    }

    if (direction < 11) {
        short int old_x = cp->pos.x + moveSteps[(int)direction ][ 0 ];
        short int old_y = cp->pos.y + moveSteps[(int)direction ][ 1 ];
        short int old_z = cp->pos.z + moveSteps[(int)direction ][ 2 ];

        if (takeItemFromMap(cp, old_x, old_y, old_z)) {
#ifdef World_ItemMove_DEBUG
            std::cout << "Item genommen" << std::endl;
#endif
            ScriptItem s_item = g_item, t_item = g_item;
            //Ausfhren eines Move Item Scriptes
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
            boost::shared_ptr<LuaItemScript> script = CommonItems->findScript(t_item.getId());

            if (script && script->existsEntrypoint("MoveItemBeforeMove")) {
                if (!script->MoveItemBeforeMove(cp, s_item, t_item)) {
                    if (!putItemOnMap(cp, old_x, old_y, old_z)) {
                        std::cerr<<"MoveItemFromMapToPlayer wurde von Script unterbunden. Datenverlust beim zurcklegen! Spieler: "<<cp->name<<std::endl;
                        g_cont = NULL;
                        g_item.reset();
                    }

                    return;
                }
            }

            Item tempitem = g_item;

            if (count < g_item.getNumber()) {
#ifdef World_ItemMove_DEBUG
                std::cout << "nicht alles verschieben" << std::endl;
#endif
                g_item.setNumber(count);
            }

            if (! cp->weightOK(g_item.getId(), g_item.getNumber(), g_cont)) {
                message(zuschwer, cp);
                NOK = true;
            }

            if (!NOK) {
                if (!putItemOnInvPos(cp, cpos)) {
                    NOK = true;
                }

                cp->checkBurden();

                if (script) {
                    script->MoveItemAfterMove(cp, s_item, t_item);
                }
            }

            if (NOK) {
#ifdef World_ItemMove_DEBUG
                std::cout << "Item konnte nicht eingefgt werden -> zurcklegen" << std::endl;
#endif
                g_item = tempitem;

                if (! putItemOnMap(cp, old_x, old_y, old_z)) {
                    std::cerr << "moveItemFromMapToPlayer: Datenverlust beim Zurcklegen, Spieler " << cp->name << std::endl;
                    g_cont = NULL;
                    g_item.reset();
                }

                return;
            }

            if (count < tempitem.getNumber() && !tempitem.isContainer()) {
                g_item = tempitem;
                g_item.setNumber(g_item.getNumber() - count);

                if (!putItemOnMap(cp, old_x, old_y, old_z)) {
                    std::cerr << "moveItemFromMapToPlayer : Datenverlust beim teilweisen Verschieben, Spieler " << cp->name << std::endl;
                    g_cont = NULL;
                    g_item.reset();
                    return;
                }
            }

            return;
        }
    }

#ifdef World_ItemMove_DEBUG
    std::cout << "moveItemFromMapToPlayer: Ende" << std::endl;
#endif
}


void World::moveItemBetweenShowcases(Player *cp, unsigned char source, unsigned char pos, unsigned char dest, unsigned char pos2, unsigned char count) {
    bool NOK = false;
    //CommonStruct com;
#ifdef World_ItemMove_DEBUG
    std::cout << "moveItemBetweenShowcases: Spieler " << cp->name << " verschiebt Item zwischen showcases" << std::endl;
#endif

    if (count == 0) {
        return;
    }

    if (takeItemFromShowcase(cp, source, pos, count)) {
#ifdef World_ItemMove_DEBUG
        std::cout << "Item genommen" << std::endl;
#endif
        ScriptItem s_item = g_item, t_item = g_item;
        s_item.pos = cp->pos;

        if (source == 0) {
            s_item.type = ScriptItem::it_showcase1;
        } else {
            s_item.type = ScriptItem::it_showcase2;
        }

        s_item.itempos = pos;
        s_item.owner = cp;
        t_item.pos = cp->pos;

        if (dest == 0) {
            t_item.type = ScriptItem::it_showcase1;
        } else {
            t_item.type = ScriptItem::it_showcase2;
        }

        t_item.itempos = pos2;
        t_item.owner = cp;
        //Ausfhren eines Move Item Scriptes
        boost::shared_ptr<LuaItemScript> script = CommonItems->findScript(t_item.getId());

        if (script && script->existsEntrypoint("MoveItemBeforeMove")) {
            if (!script->MoveItemBeforeMove(cp, s_item, t_item)) {
                if (!putItemInShowcase(cp, dest, pos2)) {
                    std::cerr<<"MoveItemBetweenShowcases wurde von Script unterbunden. Datenverlust beim zurcklegen! Spieler: "<<cp->name<<std::endl;
                    g_cont = NULL;
                    g_item.reset();
                }

                return;
            }
        }

        if (dest < MAXSHOWCASES) {
            if (cp->showcases[ dest ].inInventory()) {
                if (! cp->weightOK(g_item.getId(), g_item.getNumber(), g_cont)) {
                    message(zuschwer, cp);
                    NOK=true;
                }
            }
        }

        if (!NOK) {
            if (!putItemInShowcase(cp, dest,pos2)) {
                NOK=true;
            }

            cp->checkBurden();

            if (script) {
                script->MoveItemAfterMove(cp, s_item, t_item);
            }

        }

        if (NOK) {
            std::cout << "Item konnte nicht eingefgt werden -> zurcklegen" << std::endl;

            if (! putItemInShowcase(cp, source,0)) {
                std::cerr << "moveItemBetweenShowcases: Datenverlust beim Zurcklegen, Spieler " << cp->name << std::endl;
                g_cont = NULL;
                g_item.reset();
            }
        }
    }

#ifdef World_ItemMove_DEBUG
    std::cout << "moveItemBetweenShowcases: Ende" << std::endl;
#endif
}


bool World::moveItem(Character *cc, unsigned char d, short int xc, short int yc, short int zc, unsigned char count) {
#ifdef World_ItemMove_DEBUG
    std::cout << "moveItem: Character " << cc->name << " bewegt Item" << std::endl;
#endif

    //CommonStruct com;
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

            //Ausfhren eines Move Item Scriptes
            boost::shared_ptr<LuaItemScript> script = CommonItems->findScript(t_item.getId());

            if (script && script->existsEntrypoint("MoveItemBeforeMove")) {
                if (!script->MoveItemBeforeMove(cc, s_item, t_item)) {
                    if (!putItemOnMap(cc, old_x, old_y, old_z)) {
                        std::cerr<<"MoveItemOnMap wurde von Script unterbunden. Datenverlust beim zurcklegen! Spieler: "<<cc->name<<std::endl;
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
                    std::cerr << "moveitem (Map-Map): Datenverlust beim Zurcklegen, Character " << cc->name << std::endl;
                    g_cont = NULL;
                    g_item.reset();
                }

                return false;
            }
            /*
            else if ( com.script )
            {
                   com.script->MoveItemAfterMove(cc, s_item, t_item );
            }*/
            else if (script) {
                script->MoveItemAfterMove(cc, s_item, t_item);
            }

            //Wenn anzahl < als die anzahl des Tempitem und das Tempitem kein Container dann fehlerbehandlung
            if (count < tempitem.getNumber() && !tempitem.isContainer()) {
                g_item = tempitem;
                g_item.setNumber(g_item.getNumber() - count);

                if (!putItemOnMap(cc, old_x, old_y, old_z)) {
                    std::cerr << "moveitem (Map-Map): Datenverlust beim teilweisen Verschieben, Character " << cc->name << std::endl;
                    g_cont = NULL;
                    g_item.reset();
                    return false;
                }
            }
            /*
            else if ( com.script )
            {
                   com.script->MoveItemAfterMove(cc, s_item, t_item );
            }*/
            else if (script) {
                script->MoveItemAfterMove(cc, s_item, t_item);
            }

            return true;

        }
    }

#ifdef World_ItemMove_DEBUG
    std::cout << "moveItem: Ende" << std::endl;
#endif

    return false;

}



void World::lookIntoShowcaseContainer(Player *cp, unsigned char showcase, unsigned char pos) {
#ifdef World_ItemMove_DEBUG
    std::cout << "lookIntoShowcaseContainer: Spieler " << cp->name << " oeffnet einen Container im showcase" << std::endl;
#endif

    if ((showcase < MAXSHOWCASES) && (cp != NULL)) {
        Container *top = cp->showcases[ showcase ].top();
        bool allowedToOpenContainer = false;

//Loop through all depots if ps is a depot if yes is depot set to true
        std::map<uint32_t,Container *>::iterator it;

        for (it = cp->depotContents.begin(); it != cp->depotContents.end(); ++it) {
            if (it->second == top) {
                allowedToOpenContainer = true;
                break;
            }
        }

//end of loop through all the depots


        if (top != NULL && allowedToOpenContainer) {
            Container *tempc;
            ScriptItem tempi;

            if (top->viewItemNr(pos, tempi, tempc)) {
#ifdef World_ItemMove_DEBUG
                std::cout << "pos gefunden" << std::endl;
#endif

                if (tempc != NULL) {
#ifdef World_ItemMove_DEBUG
                    std::cout << "Container gefunden" << std::endl;
#endif
                    // updaten der showcases des Spielers
                    cp->showcases[ showcase ].openContainer(tempc);
                    // Aenderungen an den Client schicken
                    boost::shared_ptr<BasicServerCommand>cmd(new UpdateShowCaseTC(showcase, tempc->getSlotCount(), tempc->getItems()));
                    cp->Connection->addCommand(cmd);
                }
            }
        }
    }

#ifdef World_ItemMove_DEBUG
    std::cout << "lookIntoShowcaseContainer: Ende" << std::endl;
#endif
}



bool World::lookIntoBackPack(Player *cp, unsigned char showcase) {
#ifdef World_ItemMove_DEBUG
    std::cout << "lookIntoBackPack: Spieler " << cp->name << " schaut in seinen Rucksack" << std::endl;
#endif

    if ((showcase < MAXSHOWCASES) && (cp != NULL)) {
        if ((cp->characterItems[ BACKPACK ].getId() != 0) && (cp->backPackContents != NULL)) {
#ifdef World_ItemMove_DEBUG
            std::cout << "Rucksackinhalt vorhanden" << std::endl;
#endif
            // bisher geoeffnete Container im showcase schliessen
            cp->showcases[ showcase ].clear();
            // updaten des showcases des Spielers
            cp->showcases[ showcase ].startContainer(cp->backPackContents, true);
            // Aenderungen an den Client schicken
            boost::shared_ptr<BasicServerCommand>cmd(new UpdateShowCaseTC(showcase, cp->backPackContents->getSlotCount(), cp->backPackContents->getItems()));
            cp->Connection->addCommand(cmd);
#ifdef World_ItemMove_DEBUG
            std::cout << "lookIntoBackPack: Ende" << std::endl;
#endif
            return true;
        }
    }

#ifdef World_ItemMove_DEBUG
    std::cout << "lookIntoBackPack: Ende" << std::endl;
#endif
    return false;
}


bool World::lookIntoContainerOnField(Player *cp, char direction, unsigned char showcase) {


#ifdef World_ItemMove_DEBUG
    std::cout << "lookIntoContainerOnField: Spieler " << cp->name << " schaut in einen Container" << std::endl;
#endif

    if ((showcase < MAXSHOWCASES) && (direction < 11) && (cp != NULL)) {
        // Position des Item
        short int old_x = moveSteps[(int)direction ][ 0 ] + cp->pos.x;
        short int old_y = moveSteps[(int)direction ][ 1 ] + cp->pos.y;
        short int old_z = moveSteps[(int)direction ][ 2 ] + cp->pos.z;

        Field *cfold;

        if (GetPToCFieldAt(cfold, old_x, old_y, old_z, tmap)) {
#ifdef World_ItemMove_DEBUG
            std::cout << "Feld vorhanden" << std::endl;
#endif
            Item titem;

            if (cfold->ViewTopItem(titem)) {
#ifdef World_ItemMove_DEBUG
                std::cout << "mindesten 1 Item vorhanden" << std::endl;
#endif

                if (titem.getId() != DEPOTITEM && titem.isContainer()) {
#ifdef World_ItemMove_DEBUG
                    std::cout << "item ist ein Container" << std::endl;
#endif
                    MAP_POSITION opos;
                    opos.x = old_x;
                    opos.y = old_y;
                    Map::CONTAINERHASH::iterator conmapo = tmap->maincontainers.find(opos);

                    if (conmapo != tmap->maincontainers.end()) {
#ifdef World_ItemMove_DEBUG
                        std::cout << "containermap fr das Feld gefunden" << std::endl;
#endif
                        Container::CONTAINERMAP::iterator iv = (*conmapo).second.find(titem.getNumber());

                        if (iv != (*conmapo).second.end()) {
#ifdef World_ItemMove_DEBUG
                            std::cout << "der Inhalt des angegebenen Items mit der id titem.number wurde gefunden" << std::endl;
#endif
                            // updaten der showcases des Spielers
                            cp->showcases[ showcase ].startContainer((*iv).second, false);
                            cp->mapshowcaseopen = true;
                            // Aenderungen an den Client schicken
                            boost::shared_ptr<BasicServerCommand>cmd(new UpdateShowCaseTC(showcase, iv->second->getSlotCount(), iv->second->getItems()));
                            cp->Connection->addCommand(cmd);

#ifdef World_ItemMove_DEBUG
                            std::cout << "lookIntoContainerOnField: Ende 1" << std::endl;
#endif
                            return true;
                        } else {
#ifdef World_ItemMove_DEBUG
                            std::cout << "lookIntoContainerOnField: kein Containerinhalt vorhanden" << std::endl;
#endif

                        }
                    } else {
#ifdef World_ItemMove_DEBUG
                        std::cout << "lookIntoContainerOnField: kein Container vorhanden" << std::endl;
#endif

                    }
                } else {
                    // check if we got a depot there...
                    if (titem.getId() == DEPOTITEM) {
                        //titem.data + 1 so no 0 depot is used.
                        if (depotScript && depotScript->existsEntrypoint("onOpenDepot")) {
                            if (depotScript->onOpenDepot(cp, titem)) {
                                cp->openDepot(titem.getData()+1);
                            }
                        } else {
                            cp->openDepot(titem.getData()+1);
                        }

                        //lookIntoDepot(cp, showcase);
                    } else {
#ifdef World_ItemMove_DEBUG
                        std::cout << "lookIntoContainerOnField: das oberste Item ist kein Container" << std::endl;
#endif

                    }

                }
            }
        }
    }

#ifdef World_ItemMove_DEBUG
    std::cout << "lookIntoContainerOnField: Ende 2" << std::endl;
#endif
    return false;
}



void World::closeContainerInShowcase(Player *cp, unsigned char showcase) {
#ifdef World_ItemMove_DEBUG
    std::cout << "closeContainerInShowcase: Spieler " << cp->name << " schliesst einen Container" << std::endl;
#endif

    if ((showcase < MAXSHOWCASES) && (cp != NULL)) {
        if (!cp->showcases[ showcase ].closeContainer()) {
            // Container war der letzte geoeffnete -> den showcase loeschen
            boost::shared_ptr<BasicServerCommand>cmd(new ClearShowCaseTC(showcase));
            cp->Connection->addCommand(cmd);
        } else {
            // Aenderungen an den Client schicken
            Container *temp = cp->showcases[ showcase ].top();
            boost::shared_ptr<BasicServerCommand>cmd(new UpdateShowCaseTC(showcase, temp->getSlotCount(), temp->getItems()));
            cp->Connection->addCommand(cmd);
        }
    }

#ifdef World_ItemMove_DEBUG
    std::cout << "closeContainerInShowcase: Ende" << std::endl;
#endif
}


void World::sendRemoveItemFromMapToAllVisibleCharacters(TYPE_OF_ITEM_ID id, short int xo, short int yo, short int zo, Field *cfp) {
    if (cfp != NULL) {
        std::vector < Player * > temp = Players.findAllCharactersInRangeOf(xo, yo, zo, MAXVIEW);

        std::vector < Player * > ::iterator titerator;

        for (titerator = temp.begin(); titerator < temp.end(); ++titerator) {
            // cfp->SetLevel( zoffs ); // DEAD_CODE ???
            boost::shared_ptr<BasicServerCommand>cmd(new ItemRemoveTC(xo, yo, zo));
            (*titerator)->Connection->addCommand(cmd);
        }
    }
}


void World::sendSwapItemOnMapToAllVisibleCharacter(TYPE_OF_ITEM_ID id, short int xn, short int yn, short int zn, Item &it, Field *cfp) {
    if (cfp != NULL) {
        std::vector < Player * > temp = Players.findAllCharactersInRangeOf(xn, yn, zn, MAXVIEW);

        std::vector < Player * > ::iterator titerator;

        for (titerator = temp.begin(); titerator < temp.end(); ++titerator) {
            std::cout<<"adding swap to "<<(*titerator)->name<<"("<<(*titerator)->id<<")"<<std::endl;
            boost::shared_ptr<BasicServerCommand>cmd(new ItemSwapTC(xn, yn, zn, id, it));
            (*titerator)->Connection->addCommand(cmd);
            std::cout<<"adding swap to "<<(*titerator)->name<<"("<<(*titerator)->id<<") ended."<<std::endl;

        }
    }
}

void World::sendPutItemOnMapToAllVisibleCharacters(short int xn, short int yn, short int zn, Item &it, Field *cfp) {
    if (cfp != NULL) {
        std::vector < Player * > temp = Players.findAllCharactersInRangeOf(xn, yn, zn, MAXVIEW);

        std::vector < Player * > ::iterator titerator;

        for (titerator = temp.begin(); titerator < temp.end(); ++titerator) {
            // cfp->SetLevel( zoffs ); // DEAD_CODE ???
            boost::shared_ptr<BasicServerCommand>cmd(new ItemPutTC(xn, yn, zn, it));
            (*titerator)->Connection->addCommand(cmd);
        }
    }
}


void World::sendChangesOfContainerContentsCM(Container *cc, Container *moved) {
    if ((cc != NULL) && (moved != NULL)) {
        PLAYERVECTOR::iterator titerator;

        Container *ps;

        for (titerator = Players.begin(); titerator < Players.end(); ++titerator) {
            for (MAXCOUNTTYPE i = 0; i < MAXSHOWCASES; ++i) {
                ps = (*titerator)->showcases[ i ].top();

                if ((ps == cc) && (ps != NULL)) {
                    boost::shared_ptr<BasicServerCommand>cmd(new UpdateShowCaseTC(i, ps->getSlotCount(), ps->getItems()));
                    (*titerator)->Connection->addCommand(cmd);
                } else if ((*titerator)->showcases[ i ].contains(moved)) {
                    (*titerator)->showcases[ i ].clear();
                    boost::shared_ptr<BasicServerCommand>cmd(new ClearShowCaseTC(i));
                    (*titerator)->Connection->addCommand(cmd);
                }
            }
        }
    }
}



void World::sendChangesOfContainerContentsIM(Container *cc) {
    if (cc != NULL) {
        PLAYERVECTOR::iterator titerator;

        Container *ps;

        for (titerator = Players.begin(); titerator < Players.end(); ++titerator) {
            for (int i = 0; i < MAXSHOWCASES; ++i) {
                ps = (*titerator)->showcases[ i ].top();

                if (ps == cc) {
                    boost::shared_ptr<BasicServerCommand>cmd(new UpdateShowCaseTC(i, ps->getSlotCount(), ps->getItems()));
                    (*titerator)->Connection->addCommand(cmd);
                }
            }
        }
    }
}

//! close the showcase for everyone except the person who took it...
void World::closeShowcaseForOthers(Player *target, Container *moved) {
    if (moved != NULL) {
        PLAYERVECTOR::iterator titerator;

        for (titerator = Players.begin(); titerator < Players.end(); ++titerator) {
            if (target == *titerator) {
                continue;
            }

            for (MAXCOUNTTYPE i = 0; i < MAXSHOWCASES; ++i) {
                if ((*titerator)->showcases[ i ].contains(moved)) {
                    (*titerator)->showcases[ i ].clear();
                    boost::shared_ptr<BasicServerCommand>cmd(new ClearShowCaseTC(i));
                    (*titerator)->Connection->addCommand(cmd);
                }
            }
        }
    }
}

void World::closeShowcaseIfNotInRange(Container *moved, short int x, short int y, short int z) {
    if (moved != NULL) {
        PLAYERVECTOR::iterator titerator;

        for (titerator = Players.begin(); titerator < Players.end(); ++titerator) {
            if (abs(x-(*titerator)->pos.x) <= 1 && abs(y-(*titerator)->pos.y) <= 1 && z == (*titerator)->pos.z) {
                continue;
            }

            for (MAXCOUNTTYPE i = 0; i < MAXSHOWCASES; ++i) {
                if ((*titerator)->showcases[ i ].contains(moved)) {
                    (*titerator)->showcases[ i ].clear();
                    boost::shared_ptr<BasicServerCommand>cmd(new ClearShowCaseTC(i));
                    (*titerator)->Connection->addCommand(cmd);
                }
            }
        }
    }
}

bool World::isStackable(Item item) {
    CommonStruct com;

    if (CommonItems->find(item.getId(), com)) {
        return com.isStackable;
    } else {
        std::cerr<<"Item mit folgender Id konnte bei isStackable(Item item) nicht gefunden werden: "<<item.getId()<<"!"<<std::endl;
        return false;
    }
}
