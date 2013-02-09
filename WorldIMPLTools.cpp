/*
 * Illarionserver - server for the game Illarion
 * Copyright 2011 Illarion e.V.
 *
 * This file is part of Illarionserver.
 *
 * Illarionserver  is  free  software:  you can redistribute it and/or modify it
 * under the terms of the  GNU  General  Public License as published by the Free
 * Software Foundation, either version 3 of the License, or (at your option) any
 * later version.
 *
 * Illarionserver is distributed in the hope that it will be useful, but WITHOUT
 * ANY WARRANTY;  without  even  the  implied  warranty  of  MERCHANTABILITY  or
 * FITNESS FOR A PARTICULAR PURPOSE. See the GNU General Public License for more
 * details.
 *
 * You should have received a copy of the GNU  General Public License along with
 * Illarionserver. If not, see <http://www.gnu.org/licenses/>.
 */

#include "World.hpp"

#include <list>
#include <stdlib.h>

#include "data/ArmorObjectTable.hpp"
#include "data/CommonObjectTable.hpp"
#include "data/ContainerObjectTable.hpp"
#include "data/MonsterTable.hpp"
#include "data/TilesModificatorTable.hpp"
#include "data/WeaponObjectTable.hpp"

#include "db/Connection.hpp"
#include "db/ConnectionManager.hpp"
#include "db/DeleteQuery.hpp"
#include "db/InsertQuery.hpp"

#include "netinterface/protocol/ServerCommands.hpp"

//Table with data of Monsters
extern MonsterTable *MonsterDescriptions;

class Field;

//function is quick and dirty, should be replaced later
void World::deleteAllLostNPC() {
    Field *tempf; //alte NPC's l?chen
    NPCVECTOR::iterator npcIteratorOld;
    std::vector< TYPE_OF_CHARACTER_ID>::iterator npcIteratorToDelete;

    for (npcIteratorToDelete = LostNpcs.begin(); npcIteratorToDelete != LostNpcs.end(); ++npcIteratorToDelete) {
        std::cout<<" Delete NPC: "<<(*npcIteratorToDelete)<<std::endl;

        for (npcIteratorOld = Npc.begin(); npcIteratorOld < Npc.end(); ++npcIteratorOld) {
            if ((*npcIteratorOld)->id == (*npcIteratorToDelete)) {
                if (GetPToCFieldAt(tempf, (*npcIteratorOld)->pos.x, (*npcIteratorOld)->pos.y, (*npcIteratorOld)->pos.z)) {
                    //tempf->SetNPCOnField( false );
                    tempf->removeChar();
                }

                sendRemoveCharToVisiblePlayers((*npcIteratorOld)->id, (*npcIteratorOld)->pos);
                delete(*npcIteratorOld);
                npcIteratorOld = Npc.erase(npcIteratorOld);
            }
        }

        std::cout<<" NPC was deleted"<<std::endl;
    }

    LostNpcs.clear();
}

bool World::findPlayersInSight(position pos, uint8_t range, std::vector<Player *> &ret, Character::face_to direction) {
    bool found = false;
    std::vector<Player *>playersinrange = Players.findAllAliveCharactersInRangeOfOnSameMap(pos.x,pos.y,pos.z,range);

    for (std::vector<Player *>::iterator pIterator = playersinrange.begin(); pIterator != playersinrange.end(); ++pIterator) {

        bool indir = false;

        switch (direction) {
        case Character::north:

            if ((*pIterator)->pos.y <= pos.y) {
                indir = true;
            }

            break;
        case Character::northeast:

            if ((*pIterator)->pos.x - pos.x >= (*pIterator)->pos.y - pos.y) {
                indir = true;
            }

            break;
        case Character::east:

            if ((*pIterator)->pos.x >= pos.x) {
                indir = true;
            }

            break;
        case Character::southeast:

            if ((*pIterator)->pos.y - pos.y >= pos.x - (*pIterator)->pos.x) {
                indir = true;
            }

            break;
        case Character::south:

            if ((*pIterator)->pos.y >= pos.y) {
                indir = true;
            }

            break;
        case Character::southwest:

            if ((*pIterator)->pos.x - pos.x <= (*pIterator)->pos.y - pos.y) {
                indir = true;
            }

            break;
        case Character::west:

            if ((*pIterator)->pos.x <= pos.x) {
                indir = true;
            }

            break;
        case Character::northwest:

            if ((*pIterator)->pos.y - pos.y >= pos.x - (*pIterator)->pos.x) {
                indir = true;
            }

            break;
        default:
            indir = true;
            break;
        }

        if (indir) {
            std::list<BlockingObject> objects = LoS(pos,(*pIterator)->pos);

            if (objects.empty()) {
                ret.push_back((*pIterator));
                found = true;
            }
        }
    }

    return found;
}

std::list<BlockingObject> World::LoS(position startingpos, position endingpos) {
    std::list<BlockingObject> ret;
    ret.clear();
    bool steep = std::abs(startingpos.y - endingpos.y) > std::abs(startingpos.x - endingpos.x);
    short int startx=startingpos.x;
    short int starty=startingpos.y;
    short int endx=endingpos.x;
    short int endy=endingpos.y;

    if (steep) {
        //change x,y values for correct algorithm in negativ range
        short int change;
        change = startx;
        startx = starty;
        starty = change;
        change = endx;
        endx = endy;
        endy = change;
    }

    bool swapped = startx > endx;

    if (swapped) {
        short int change;
        change = startx;
        startx = endx;
        endx = change;
        change = starty;
        starty = endy;
        endy = change;

    }

    short int deltax = endx - startx;
    short int deltay = std::abs(endy - starty);
    short int error = 0;
    short int ystep=1;
    short int y = starty;

    if (starty > endy) {
        ystep = -1;
    }

    for (short int x = startx; x <= endx; ++x) {
        if (!(x == startx && y == starty) && !(x == endx && y == endy)) {
            BlockingObject bo;
            Field *temp;

            if (steep) {
                if (GetPToCFieldAt(temp,y,x,startingpos.z)) {
                    if (temp->IsPlayerOnField()) {
                        bo.blockingType = BlockingObject::BT_CHARACTER;
                        bo.blockingChar = findCharacterOnField(y,x,startingpos.z);

                        if (swapped) {
                            ret.push_back(bo);
                        } else {
                            ret.push_front(bo);
                        }
                    } else if (!temp->IsPassable()) {
                        ScriptItem it;

                        if (temp->ViewTopItem(it)) {
                            bo.blockingType = BlockingObject::BT_ITEM;
                            it.pos = position(y,x,startingpos.z);
                            it.type = ScriptItem::it_field;
                            bo.blockingItem = it;

                            if (swapped) {
                                ret.push_back(bo);
                            } else {
                                ret.push_front(bo);
                            }
                        }
                    }
                }
            } else {
                if (GetPToCFieldAt(temp,x,y,startingpos.z)) {
                    if (temp->IsPlayerOnField()) {
                        bo.blockingType = BlockingObject::BT_CHARACTER;
                        bo.blockingChar = findCharacterOnField(x,y,startingpos.z);

                        if (swapped) {
                            ret.push_back(bo);
                        } else {
                            ret.push_front(bo);
                        }
                    } else if (!temp->IsPassable()) {
                        ScriptItem it;

                        if (temp->ViewTopItem(it)) {
                            bo.blockingType = BlockingObject::BT_ITEM;
                            it.pos = position(x,y,startingpos.z);
                            it.type = ScriptItem::it_field;
                            bo.blockingItem = it;

                            if (swapped) {
                                ret.push_back(bo);
                            } else {
                                ret.push_front(bo);
                            }
                        }
                    }
                }
            }
        }

        error += deltay;

        if (2*error >= deltax) {
            y+=ystep;
            error -= deltax;
        }
    }

    return ret;
}

//function which updates the playerlist.
void World::updatePlayerList() {
    using namespace Database;
    std::cout<<"Updateplayerlist start"<<std::endl;

    PConnection connection = ConnectionManager::getInstance().getConnection();

    try {
        connection->beginTransaction();

        DeleteQuery delQuery(connection);
        delQuery.setServerTable("onlineplayer");
        delQuery.execute();

        if (!Players.empty()) {
            InsertQuery insQuery(connection);
            insQuery.setServerTable("onlineplayer");
            const InsertQuery::columnIndex column = insQuery.addColumn("on_playerid");

            PLAYERVECTOR::iterator plIterator;

            for (plIterator = Players.begin(); plIterator != Players.end(); ++plIterator) {
                insQuery.addValue<TYPE_OF_CHARACTER_ID>(column, (*plIterator)->id);
            }

            insQuery.execute();
        }

        connection->commitTransaction();
    } catch (std::exception &e) {
        std::cerr<<"caught exception during online player save: "<<e.what()<<std::endl;
        connection->rollbackTransaction();
    }

    std::cout<<"updateplayerlist end"<<std::endl;
}

Character *World::findCharacterOnField(short int posx, short int posy, short int posz) {
    Character *tmpChr;
    tmpChr = Players.find(posx, posy, posz);

    if (tmpChr != NULL) {
        return tmpChr;
    }

    tmpChr = Monsters.find(posx, posy, posz);

    if (tmpChr != NULL) {
        return tmpChr;
    }

    tmpChr = Npc.find(posx, posy, posz);

    if (tmpChr != NULL) {
        return tmpChr;
    }

    return NULL;
}

Player *World::findPlayerOnField(short int posx, short int posy, short int posz) {
    return Players.find(posx, posy, posz);
}

Character *World::findCharacter(TYPE_OF_CHARACTER_ID id) {
    Character *tmpChr;
    tmpChr = dynamic_cast<Character *>(Players.findID(id));

    if (tmpChr != NULL) {
        return tmpChr;
    }

    tmpChr = dynamic_cast<Character *>(Monsters.findID(id));

    if (tmpChr != NULL) {
        return tmpChr;
    }

    tmpChr = dynamic_cast<Character *>(Npc.findID(id));

    if (tmpChr != NULL) {
        return tmpChr;
    }

    return NULL;
}


bool World::findPlayerWithLowestHP(std::vector < Player * > * ppvec, Player* &found) {
    found = NULL;
    std::vector < Player * > ::iterator pIterator;

    for (pIterator = ppvec->begin(); pIterator < ppvec->end(); ++pIterator) {
        if (found == NULL) {
            found = *pIterator;
        } else {
            if (found->getAttribute(Character::hitpoints) > (*pIterator)->getAttribute(Character::hitpoints)) {
                found = (*pIterator);
            }
        }
    }

    return (found != NULL);
}


void World::takeMonsterAndNPCFromMap() {
    Field *tempf;

    MONSTERVECTOR::iterator monsterIterator;

    for (monsterIterator = Monsters.begin(); monsterIterator < Monsters.end(); ++monsterIterator) {
        if (GetPToCFieldAt(tempf, (*monsterIterator)->pos.x, (*monsterIterator)->pos.y, (*monsterIterator)->pos.z)) {
            tempf->SetMonsterOnField(false);
        }

        delete(*monsterIterator);
        *monsterIterator = NULL;
    }

    NPCVECTOR::iterator npcIterator;

    for (npcIterator = Npc.begin(); npcIterator < Npc.end(); ++npcIterator) {
        if (GetPToCFieldAt(tempf, (*npcIterator)->pos.x, (*npcIterator)->pos.y, (*npcIterator)->pos.z)) {
            tempf->SetNPCOnField(false);
        }

        delete(*npcIterator);
        *npcIterator = NULL;
    }

    Monsters.clear();
    Npc.clear();
}


// only invoked when ATTACK***_TS is received or when a monster attacks
bool World::characterAttacks(Character *cp) {

    if (cp->enemyid != cp->id) {

        if (cp->enemytype == Character::player) {
#ifdef World_DEBUG
            std::cout << "attack player" << std::endl;
#endif
            Player *temppl = Players.findID(cp->enemyid);

            // Ziel gefunden
            if (temppl != NULL) {
                // Ziel sichtbar
                if (cp->isInRange(temppl, temppl->getScreenRange())) {

                    // Ziel ist tot
                    if (!cp->attack(temppl)) {
                        sendSpinToAllVisiblePlayers(temppl);

                        cp->attackmode = false;

                        //set lasttargetseen to false if the player who was attacked is death
                        if (cp->character == Character::monster) {
                            Monster *mon = dynamic_cast<Monster *>(cp);
                            mon->lastTargetSeen = false;
                        }

                        // dead people cannot be attacked, reset counter
                        //temppl->nrOfAttackers=0;

                        if (cp->character == Character::player) {
                            boost::shared_ptr<BasicServerCommand>cmd(new TargetLostTC());
                            dynamic_cast<Player *>(cp)->Connection->addCommand(cmd);
                        }

                        boost::shared_ptr<BasicServerCommand>cmd(new TargetLostTC());
                        dynamic_cast<Player *>(temppl)->Connection->addCommand(cmd);
                        temppl->attackmode = false;
                    }

                    return true;
                }
            }
        } else if (cp->enemytype == Character::monster) {
#ifdef World_DEBUG
            std::cout << "attack monster" << std::endl;
#endif

            Monster *temppl = Monsters.findID(cp->enemyid);

            // Ziel gefunden
            if (temppl != NULL) {
                if (cp->isInRange(temppl, temppl->getScreenRange())) {
                    MonsterStruct monStruct;

                    if (MonsterDescriptions->find(temppl->getType(), monStruct)) {
                        if (monStruct.script) {
                            monStruct.script->onAttacked(temppl,cp);
                        } else {
                            std::cerr<<"No script initialized for monster: "<<temppl->getType()<<" on Attack not called!"<<std::endl;
                        }
                    } else {
                        std::cerr<<"Didn't finde Monster Description for: "<< temppl->getType() << " can't call onAttacked!"<<std::endl;
                    }

                    // Ziel ist tot
                    if (!cp->attack(temppl)) {
                        cp->attackmode = false;

                        if (cp->character == Character::player) {
                            boost::shared_ptr<BasicServerCommand>cmd(new TargetLostTC());
                            dynamic_cast<Player *>(cp)->Connection->addCommand(cmd);
                        }
                    } else {
                        //check for turning into attackackers direction
                        std::vector<Player *>temp;
                        temp.clear();
                        findPlayersInSight(temppl->pos, static_cast<uint8_t>(9), temp, temppl->faceto);

                        //add the current attacker to the list
                        if (cp->character == Character::player) {
                            temp.push_back(dynamic_cast<Player *>(cp));
                        }

                        Player *foundPl;

                        if (!temp.empty() && findPlayerWithLowestHP(&temp, foundPl)) {
                            temppl->turn(foundPl->pos);
                        }

                    }

                    return true;
                }
            }
        }

        // Ziel nicht gefunden/au�er Sichtweite
        cp->attackmode = false;

        if (cp->character == Character::player) {
            boost::shared_ptr<BasicServerCommand>cmd(new TargetLostTC());
            dynamic_cast<Player *>(cp)->Connection->addCommand(cmd);
        }

        return false;
    } else {
        return true;
    }

}


bool World::killMonster(Monster *monsterp) {
    if (monsterp != NULL) {
        MONSTERVECTOR::iterator newIt;

        if (Monsters.getIterator(monsterp->id, newIt)) {
            MONSTERVECTOR::iterator temp;
            killMonster(newIt, temp);
            return true;
        }
    }

    return false;
}


void World::killMonster(MONSTERVECTOR::iterator monsterIt, MONSTERVECTOR::iterator &newIt) {

    //( *monsterIt )->SetAlive( false );
    Field *tempf;

    if (GetPToCFieldAt(tempf, (*monsterIt)->pos.x, (*monsterIt)->pos.y, (*monsterIt)->pos.z)) {
        //tempf->SetMonsterOnField( false );
        tempf->removeChar();
    } else {
#ifdef World_DEBUG
        std::cout << "Feld nicht gefunden" << std::endl;
#endif

    }

    sendRemoveCharToVisiblePlayers((*monsterIt)->id, (*monsterIt)->pos);

    // delete our monster
    if (*monsterIt) {
        delete *monsterIt;
    }

    newIt = Monsters.erase(monsterIt);

}


Field *World::GetField(position pos) {
    WorldMap::map_t temp;

    if (maps.findMapForPos(pos, temp)) {
        Field *field = NULL;

        if (temp->GetPToCFieldAt(field, pos.x, pos.y)) {
            return field;
        } else {
            return NULL;
        }
    } else {
        return NULL;
    }
}


bool World::GetPToCFieldAt(Field* &fip, short int x, short int y, short int z) {

    WorldMap::map_t temp;

    if (maps.findMapForPos(x, y, z, temp)) {
        return temp->GetPToCFieldAt(fip, x, y);
    } else {
        return false;
    }

}


bool World::GetPToCFieldAt(Field* &fip, position pos) {

    WorldMap::map_t temp;

    if (maps.findMapForPos(pos, temp)) {
        return temp->GetPToCFieldAt(fip, pos.x, pos.y);
    }

    return false;

}


bool World::GetPToCFieldAt(Field* &fip, short int x, short int y, short int z, WorldMap::map_t &map) {

    if (maps.findMapForPos(x, y, z, map)) {
        return map->GetPToCFieldAt(fip, x, y);
    }

    return false;

}


bool World::GetPToCFieldAt(Field* &fip, position pos, WorldMap::map_t &map) {

    if (maps.findMapForPos(pos, map)) {
        return map->GetPToCFieldAt(fip, pos.x, pos.y);
    }

    return false;

}


bool World::findEmptyCFieldNear(Field* &cf, short int &x, short int &y, short int z) {

    WorldMap::map_t temp;

    if (maps.findMapForPos(x, y, z, temp)) {
        return temp->findEmptyCFieldNear(cf, x, y);
    }

    return false;

}


int World::getItemAttrib(std::string s, TYPE_OF_ITEM_ID ItemID) {

    // Armor //
    if (s == "bodyparts") {
        if (ArmorItems->find(ItemID, tempArmor)) {
            return tempArmor.BodyParts;
        }
    } else if (s == "strokearmor") {
        if (ArmorItems->find(ItemID, tempArmor)) {
            return tempArmor.StrokeArmor;
        }
    } else if (s == "thrustarmor") {
        if (ArmorItems->find(ItemID, tempArmor)) {
            return tempArmor.ThrustArmor;
        }
    } else if (s == "armormagicdisturbance") {
        if (ArmorItems->find(ItemID, tempArmor)) {
            return tempArmor.MagicDisturbance;
        }
    }

    // Common //
    else if (s == "agingspeed") {
        const CommonStruct &common = CommonItems->find(ItemID);

        if (common.isValid()) {
            return common.AgeingSpeed;
        }
    } else if (s == "objectafterrot") {
        const CommonStruct &common = CommonItems->find(ItemID);

        if (common.isValid()) {
            return common.ObjectAfterRot;
        }
    } else if (s == "weight") {
        const CommonStruct &common = CommonItems->find(ItemID);

        if (common.isValid()) {
            return common.Weight;
        }
    }

    // Tiles Modificator //
    else if (s == "modificator") {
        if (TilesModItems->find(ItemID, tempModificator)) {
            return tempModificator.Modificator;
        }
    }

    // Weapon //
    else if (s == "accuracy") {
        if (WeaponItems->find(ItemID, tempWeapon)) {
            return tempWeapon.Accuracy;
        }
    } else if (s == "attack") {
        if (WeaponItems->find(ItemID, tempWeapon)) {
            return tempWeapon.Attack;
        }
    } else if (s == "defence") {
        if (WeaponItems->find(ItemID, tempWeapon)) {
            return tempWeapon.Defence;
        }
    } else if (s == "range") {
        if (WeaponItems->find(ItemID, tempWeapon)) {
            return tempWeapon.Range;
        }
    } else if (s == "weapontype") {
        if (WeaponItems->find(ItemID, tempWeapon)) {
            return tempWeapon.WeaponType;
        }
    } else if (s == "weaponmagicdisturbance") {
        if (WeaponItems->find(ItemID, tempWeapon)) {
            return tempWeapon.MagicDisturbance;
        }
    }

    return 0;

}


void World::closeShowcasesForContainerPositions() {

    std::vector < Player * > temp;

    for (std::vector < position > ::iterator posit = contpos->begin(); posit < contpos->end(); ++posit) {
        temp=Players.findAllCharactersInMaxRangeOf(posit->x, posit->y, posit->z, 1);

        for (std::vector < Player * > ::iterator titerator = temp.begin(); titerator < temp.end(); ++titerator) {
            (*titerator)->closeAllShowcasesOfMapContainers();
        }

        temp.clear();
    }

}


void World::updatePlayerView(short int startx, short int endx) {

    std::vector < Player * > temp;

    if (Players.findAllCharactersWithXInRangeOf(startx - 20, endx + 20, temp)) {
        for (std::vector < Player * > ::iterator titerator = temp.begin(); titerator < temp.end(); ++titerator) {
#ifdef World_DEBUG
            std::cout << "update view for player " << (*titerator)->name << " " << startx << ":#" << (*titerator)->pos.x << "#:" << endx << "\n";
#endif

            (*titerator)->sendFullMap();
            sendAllVisibleCharactersToPlayer((*titerator), true);
        }
    }

}


bool World::DoAge() {

    if (nextXtoage >= maps.getHighX()) {
        // auf allen Karten alles abgearbeitet
        time_t temp = time(NULL);      // liefert die Sekunden seit dem 1.1.1970
        realgap = temp - last_age;

        // Zeit f�r neuen Durchlauf der Karte
        if (realgap >= gap) {
#ifdef World_DEBUG
            std::cout << "World.DoAge: Karte gealtert nach " << realgap << " Sekunden\n";
#endif
            ++timecount;

            last_age = temp;
            nextXtoage = maps.getLowX();

            AgeInventory();
            maps.ageContainers();
        } else {
            return false;
        }
    }

    // noch nicht die gesamte Karte durchlaufen ->
    // restliche Streifen bearbeiten
    lastXtoage = nextXtoage + ammount - 1;

    if (lastXtoage >= maps.getHighX()) {
        lastXtoage = maps.getHighX();
    }

    WorldMap::map_vector_t mapsToage;

    if (maps.findAllMapsWithXInRangeOf(nextXtoage, lastXtoage, mapsToage)) {
        for (auto mapI = mapsToage.begin(); mapI < mapsToage.end(); ++mapI) {
            (*mapI)->ageItemsInHorizontalRange(nextXtoage, lastXtoage);
        }

        closeShowcasesForContainerPositions();
        contpos->clear();
    }

    nextXtoage = lastXtoage + 1;

    return true;

}


void World::AgeInventory() {

    PLAYERVECTOR::iterator titerator;

    for (titerator = Players.begin(); titerator < Players.end(); ++titerator) {
        (*titerator)->ageInventory();
    }

    MONSTERVECTOR::iterator monsterIterator;

    for (monsterIterator = Monsters.begin(); monsterIterator < Monsters.end(); ++monsterIterator) {
        (*monsterIterator)->ageInventory();
    }

}


void World::saveAllPlayerNamesToFile(std::string name) {

    PLAYERVECTOR::iterator titerator;
    FILE *f;
    f = fopen(name.c_str(), "w");

    if (f != NULL) {
        std::list<std::string> players;

        for (titerator = Players.begin(); titerator < Players.end(); ++titerator) {
            if (!(*titerator)->isAdmin() || (*titerator)->hasGMRight(gmr_isnotshownasgm)) {
                players.push_back((*titerator)->name);
            }
        }

        fprintf(f, "%d\n", (int)players.size());

        for (std::list<std::string>::iterator it = players.begin(); it != players.end(); ++it) {
            fprintf(f, "%s\n",  it->c_str());
        }

        fclose(f);
    }

}


void World::Save(std::string prefix) {
    prefix = directory + std::string(MAPDIR) + prefix;

    maps.saveToDisk(prefix);

    std::ofstream specialfile((prefix + "_specialfields").c_str(), std::ios::binary | std::ios::out | std::ios::trunc);

    if (! specialfile.good()) {
#ifdef World_DEBUG
        std::cerr << "World::Save: Fehler beim Speichern der speziellen Felder, konnte _specialfields nicht erstellen" << std::endl;
#endif

    } else {
        unsigned short int size = specialfields.size();
#ifdef World_DEBUG
        std::cout << "World::Save: speichere " << size << " spezielle Felder" << std::endl;
#endif
        specialfile.write((char *) & size, sizeof(size));

        FIELDATTRIBHASH::const_iterator witerat = specialfields.begin();

        for (; witerat != specialfields.end(); ++witerat) {
            specialfile.write((char *) & (witerat->first), sizeof(witerat->first));
            specialfile.write((char *) & (witerat->second.type), sizeof(witerat->second.type));
            specialfile.write((char *) & (witerat->second.flags), sizeof(witerat->second.flags));
        }

        specialfile.close();
    }

}


void World::Load(std::string prefix) {
    prefix = directory + std::string(MAPDIR) + prefix;

    std::ifstream mapinitfile((prefix + "_initmaps").c_str(), std::ios::binary | std::ios::in);

    if (! mapinitfile.good()) {
        std::cerr << "World::Load: Fehler beim Laden der Karten, konnte initmaps nicht �ffnen" << std::endl;
    } else {
        unsigned short int size;
        mapinitfile.read((char *) & size, sizeof(size));
        std::cout << "World::Load: lade " << size << " Karten" << std::endl;

        short int tZ_Level;
        short int tMin_X;
        short int tMin_Y;

        short int tWidth;
        short int tHeight;

        char mname[ 200 ];

        for (int i = 0; i < size; ++i) {
            mapinitfile.read((char *) & tZ_Level, sizeof(tZ_Level));
            mapinitfile.read((char *) & tMin_X, sizeof(tMin_X));
            mapinitfile.read((char *) & tMin_Y, sizeof(tMin_Y));

            mapinitfile.read((char *) & tWidth, sizeof(tWidth));
            mapinitfile.read((char *) & tHeight, sizeof(tHeight));

            WorldMap::map_t tempMap(new Map(tWidth, tHeight));
            tempMap->Init(tMin_X, tMin_Y, tZ_Level);

            sprintf(mname, "%s_%6d_%6d_%6d", prefix.c_str(), tZ_Level, tMin_X, tMin_Y);

            // if the map loads ok...
            if (tempMap->Load(mname, 0, 0)) {
                maps.InsertMap(tempMap);    // insert it
            }
        }

        mapinitfile.close();
    }

    std::ifstream specialfile((prefix + "_specialfields").c_str(), std::ios::binary | std::ios::in);

    if (! specialfile.good()) {
        std::cerr << "World::Load: Fehler beim Laden der speziellen Felder, konnte _specialfields nicht �ffnen" << std::endl;
    } else {
        unsigned short int size3;
        specialfile.read((char *) & size3, sizeof(size3));
        std::cout << "World::Load: lade " << size3 << " spezielle Felder" << std::endl;

        position start;
        s_fieldattrib attrib;

        for (int i = 0 ; i < size3; ++i) {
            specialfile.read((char *) & start, sizeof(start));
            specialfile.read((char *) & (attrib.type), sizeof(attrib.type));
            specialfile.read((char *) & (attrib.flags), sizeof(attrib.flags));

            makeSpecialField(start, attrib);
        }

        specialfile.close();
    }

}

int World::getTime(std::string timeType) {
    int minute,hour,day,month,year,illaTime;
    time_t curr_unixtime;
    struct tm *timestamp;

    // return unix timestamp if requsted and quit function
    if (timeType=="unix") {
        return (int)time(NULL);
    }

    // get current time and timezone data to get additional informations for time conversation
    curr_unixtime = time(NULL);
    timestamp = localtime(&curr_unixtime);

    illaTime = (int)curr_unixtime;

    // in case its currently dst, correct the timestamp so the illarion time changes the timestamp as well
    if (timestamp->tm_isdst) {
        illaTime+=3600;
    }

    // Illarion seconds since 17th February 2000
    // RL Seconds * 3
    illaTime = (illaTime - 950742000) * 3;

    if (timeType=="illarion") {
        return (int)illaTime;
    }

    // Calculating year
    // 31536000 == 60*60*24*365
    year = (int)(illaTime / 31536000);
    illaTime -= year * 31536000;

    // Calculating day
    // 86400 = 60*60*24
    day = (int)(illaTime / 86400);
    illaTime -= day * 86400;
    ++day;

    // Calculating month
    // 24 days per month
    month = (int)(day / 24);
    day -= month * 24;

    // checking for range borders and fixing the date
    if (day == 0) {
        if (month > 0 && month < 16) {
            day = 24;
        } else {
            day = 5;
        }
    } else {
        month++;
    }

    if (month == 0) {
        month = 16;
        --year;
    }

    // Date calculation is done, return the date if it was requested
    if (timeType=="year") {
        return year;
    } else if (timeType=="month") {
        return month;
    } else if (timeType=="day") {
        return day;
    }

    // Calculate the time of day
    // Calculating hour
    // 3600 = 60 * 60
    hour = (int)(illaTime / 3600);
    illaTime -= hour * 3600;

    //Calculating minute
    minute = (int)(illaTime / 60);

    //Calculating seconds
    illaTime -= minute * 60;

    // returning the last possible values
    if (timeType=="hour") {
        return hour;
    } else if (timeType=="minute") {
        return minute;
    } else if (timeType=="second") {
        return illaTime;
    } else {
        return -1;
    }
}


bool World::findWarpFieldsInRange(position pos, short int range, std::vector< boost::shared_ptr< position > > & warppositions) {
    int x,y;
    Field *cf = 0;

    for (x=pos.x-range; x<=pos.x+range; ++x)
        for (y=pos.y-range; y<=pos.y+range; ++y)
            if (GetPToCFieldAt(cf, x, y, pos.z) && cf->IsWarpField()) {
                boost::shared_ptr<position> p(new position(x, y, pos.z));
                warppositions.push_back(p);
            }

    return !warppositions.empty();
}


void World::setWeatherPart(std::string type, char value) {
    if (type == "cloud_density") {
        weather.cloud_density = value;
    } else if (type == "fog_density") {
        weather.fog_density = value;
    } else if (type == "wind_dir") {
        weather.wind_dir = value;
    } else if (type == "gust_strength") {
        weather.gust_strength = value;
    } else if (type == "percipitation_strength") {
        weather.percipitation_strength = value;
    } else if (type == "percipitation_type") {
        weather.per_type = value;
    } else if (type == "thunderstorm") {
        weather.thunderstorm = value;
    } else if (type == "temperature") {
        weather.temperature = value;
    }

    sendWeatherToAllPlayers();
}

void World::sendRemoveCharToVisiblePlayers(TYPE_OF_CHARACTER_ID id, position &pos) {
    std::vector < Player * > temp = Players.findAllCharactersInScreen(pos.x, pos.y, pos.z);
    std::vector < Player * > ::iterator titerator;
    boost::shared_ptr<BasicServerCommand>cmd(new RemoveCharTC(id));

    for (titerator = temp.begin(); titerator < temp.end(); ++titerator) {
        (*titerator)->sendCharRemove(id, cmd);
    }
}

void World::sendHealthToAllVisiblePlayers(Character *cc, Attribute::attribute_t health) {
    if (!cc->isinvisible) {

        std::vector < Player * > temp = Players.findAllCharactersInScreen(cc->pos.x, cc->pos.y, cc->pos.z);

        std::vector < Player * > ::iterator titerator;
        char xoffs;
        char yoffs;
        char zoffs;

        for (titerator = temp.begin(); titerator < temp.end(); ++titerator) {
            xoffs = cc->pos.x - (*titerator)->pos.x;
            yoffs = cc->pos.y - (*titerator)->pos.y;
            zoffs = cc->pos.z - (*titerator)->pos.z + RANGEDOWN;

            if ((xoffs != 0) || (yoffs != 0) || (zoffs != RANGEDOWN)) {
                boost::shared_ptr<BasicServerCommand>cmd(new UpdateAttribTC(cc->id, "hitpoints", health));
                (*titerator)->Connection->addCommand(cmd);
            }
        }
    }
}

