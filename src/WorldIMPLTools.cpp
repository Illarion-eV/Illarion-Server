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

#include "Player.hpp"
#include "NPC.hpp"
#include "Monster.hpp"
#include "Field.hpp"
#include "Map.hpp"

#include "data/Data.hpp"
#include "data/ArmorObjectTable.hpp"
#include "data/ContainerObjectTable.hpp"
#include "data/MonsterTable.hpp"
#include "data/TilesModificatorTable.hpp"
#include "data/WeaponObjectTable.hpp"

#include "db/Connection.hpp"
#include "db/ConnectionManager.hpp"
#include "db/DeleteQuery.hpp"
#include "db/InsertQuery.hpp"

#include "netinterface/protocol/ServerCommands.hpp"

extern MonsterTable *MonsterDescriptions;
extern std::vector<position> *contpos;

void World::deleteAllLostNPC() {
    Field *tempf;

    for (const TYPE_OF_CHARACTER_ID &npcToDelete : LostNpcs) {
        const auto &npc = Npc.find(npcToDelete);

        if (npc) {
            if (GetPToCFieldAt(tempf, npc->getPosition())) {
                tempf->removeChar();
            }

            sendRemoveCharToVisiblePlayers(npc->getId(), npc->getPosition());
            delete npc;
        }
    }

    LostNpcs.clear();
}

bool World::findPlayersInSight(const position &pos, uint8_t range, std::vector<Player *> &ret, Character::face_to direction) {
    bool found = false;

    for (const auto &player : Players.findAllAliveCharactersInRangeOfOnSameMap(pos, range)) {
        bool indir = false;
        const position &playerPos = player->getPosition();

        switch (direction) {
        case Character::north:

            if (playerPos.y <= pos.y) {
                indir = true;
            }

            break;

        case Character::northeast:

            if (playerPos.x - pos.x >= playerPos.y - pos.y) {
                indir = true;
            }

            break;

        case Character::east:

            if (playerPos.x >= pos.x) {
                indir = true;
            }

            break;

        case Character::southeast:

            if (playerPos.y - pos.y >= pos.x - playerPos.x) {
                indir = true;
            }

            break;

        case Character::south:

            if (playerPos.y >= pos.y) {
                indir = true;
            }

            break;

        case Character::southwest:

            if (playerPos.x - pos.x <= playerPos.y - pos.y) {
                indir = true;
            }

            break;

        case Character::west:

            if (playerPos.x <= pos.x) {
                indir = true;
            }

            break;

        case Character::northwest:

            if (playerPos.y - pos.y >= pos.x - playerPos.x) {
                indir = true;
            }

            break;

        default:
            indir = true;
            break;
        }

        if (indir) {
            std::list<BlockingObject> objects = LoS(pos, player->getPosition());

            if (objects.empty()) {
                ret.push_back(player);
                found = true;
            }
        }
    }

    return found;
}

std::list<BlockingObject> World::LoS(const position &startingpos, const position &endingpos) const {
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
                const position pos(y, x, startingpos.z);

                if (GetPToCFieldAt(temp, pos)) {
                    if (temp->IsPlayerOnField()) {
                        bo.blockingType = BlockingObject::BT_CHARACTER;
                        bo.blockingChar = findCharacterOnField(pos);

                        if (swapped) {
                            ret.push_back(bo);
                        } else {
                            ret.push_front(bo);
                        }
                    } else if (!temp->IsPassable()) {
                        ScriptItem it;

                        if (temp->ViewTopItem(it)) {
                            bo.blockingType = BlockingObject::BT_ITEM;
                            it.pos = position(pos);
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
                const position pos(x, y, startingpos.z);

                if (GetPToCFieldAt(temp, pos)) {
                    if (temp->IsPlayerOnField()) {
                        bo.blockingType = BlockingObject::BT_CHARACTER;
                        bo.blockingChar = findCharacterOnField(pos);

                        if (swapped) {
                            ret.push_back(bo);
                        } else {
                            ret.push_front(bo);
                        }
                    } else if (!temp->IsPassable()) {
                        ScriptItem it;

                        if (temp->ViewTopItem(it)) {
                            bo.blockingType = BlockingObject::BT_ITEM;
                            it.pos = pos;
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

            Players.for_each([&](Player *player) {
                insQuery.addValue<TYPE_OF_CHARACTER_ID>(column, player->getId());
            });

            insQuery.execute();
        }

        connection->commitTransaction();
    } catch (std::exception &e) {
        std::cerr<<"caught exception during online player save: "<<e.what()<<std::endl;
        connection->rollbackTransaction();
    }

    std::cout<<"updateplayerlist end"<<std::endl;
}

Character *World::findCharacterOnField(const position &pos) const {
    Character *tmpChr;
    tmpChr = Players.find(pos);

    if (tmpChr) {
        return tmpChr;
    }

    tmpChr = Monsters.find(pos);

    if (tmpChr) {
        return tmpChr;
    }

    tmpChr = Npc.find(pos);

    if (tmpChr) {
        return tmpChr;
    }

    return nullptr;
}

Player *World::findPlayerOnField(const position &pos) const {
    return Players.find(pos);
}

Character *World::findCharacter(TYPE_OF_CHARACTER_ID id) {
    Character *tmpChr;
    tmpChr = dynamic_cast<Character *>(Players.find(id));

    if (tmpChr) {
        return tmpChr;
    }

    tmpChr = dynamic_cast<Character *>(Monsters.find(id));

    if (tmpChr) {
        return tmpChr;
    }

    tmpChr = dynamic_cast<Character *>(Npc.find(id));

    if (tmpChr) {
        return tmpChr;
    }

    return nullptr;
}


bool World::findPlayerWithLowestHP(const std::vector<Player *> &ppvec, Player *&found) {
    found = nullptr;

    for (const auto &player : ppvec) {
        if (!found) {
            found = player;
        } else {
            if (found->getAttribute(Character::hitpoints) > player->getAttribute(Character::hitpoints)) {
                found = player;
            }
        }
    }

    return found;
}


void World::takeMonsterAndNPCFromMap() {
    Monsters.for_each([this](Monster *monster) {
        Field *tempf;

        if (GetPToCFieldAt(tempf, monster->getPosition())) {
            tempf->SetMonsterOnField(false);
        }

        delete monster;
    });

    Npc.for_each([this](NPC *npc) {
        Field *tempf;

        if (GetPToCFieldAt(tempf, npc->getPosition())) {
            tempf->SetNPCOnField(false);
        }

        delete npc;
    });

    Monsters.clear();
    Npc.clear();
}


// only invoked when ATTACK***_TS is received or when a monster attacks
bool World::characterAttacks(Character *cp) {

    if (cp->enemyid != cp->getId()) {

        if (cp->enemytype == Character::player) {
#ifdef World_DEBUG
            std::cout << "attack player" << std::endl;
#endif
            Player *temppl = Players.find(cp->enemyid);

            // Ziel gefunden
            if (temppl != NULL) {
                // Ziel sichtbar
                if (cp->isInRange(temppl, temppl->getScreenRange())) {

                    // Ziel ist tot
                    if (!cp->attack(temppl)) {
                        sendSpinToAllVisiblePlayers(temppl);

                        cp->setAttackMode(false);

                        //set lasttargetseen to false if the player who was attacked is death
                        if (cp->getType() == Character::monster) {
                            Monster *mon = dynamic_cast<Monster *>(cp);
                            mon->lastTargetSeen = false;
                        }

                        // dead people cannot be attacked, reset counter
                        //temppl->nrOfAttackers=0;

                        if (cp->getType() == Character::player) {
                            ServerCommandPointer cmd = std::make_shared<TargetLostTC>();
                            dynamic_cast<Player *>(cp)->Connection->addCommand(cmd);
                        }

                        ServerCommandPointer cmd = std::make_shared<TargetLostTC>();
                        dynamic_cast<Player *>(temppl)->Connection->addCommand(cmd);
                        temppl->setAttackMode(false);
                    }

                    return true;
                }
            }
        } else if (cp->enemytype == Character::monster) {
            Monster *temppl = Monsters.find(cp->enemyid);

            // Ziel gefunden
            if (temppl != NULL) {
                if (cp->isInRange(temppl, temppl->getScreenRange())) {
                    MonsterStruct monStruct;

                    if (MonsterDescriptions->find(temppl->getMonsterType(), monStruct)) {
                        if (monStruct.script) {
                            monStruct.script->onAttacked(temppl,cp);
                        } else {
                            std::cerr<<"No script initialized for monster: "<<temppl->getMonsterType()<<" on Attack not called!"<<std::endl;
                        }
                    } else {
                        std::cerr<<"Didn't finde Monster Description for: "<< temppl->getMonsterType() << " can't call onAttacked!"<<std::endl;
                    }

                    // Ziel ist tot
                    if (!cp->attack(temppl)) {
                        cp->setAttackMode(false);

                        if (cp->getType() == Character::player) {
                            ServerCommandPointer cmd = std::make_shared<TargetLostTC>();
                            dynamic_cast<Player *>(cp)->Connection->addCommand(cmd);
                        }
                    } else {
                        //check for turning into attackackers direction
                        std::vector<Player *>temp;
                        temp.clear();
                        findPlayersInSight(temppl->getPosition(), static_cast<uint8_t>(9), temp, temppl->getFaceTo());

                        //add the current attacker to the list
                        if (cp->getType() == Character::player) {
                            temp.push_back(dynamic_cast<Player *>(cp));
                        }

                        Player *foundPl;

                        if (!temp.empty() && findPlayerWithLowestHP(temp, foundPl)) {
                            temppl->turn(foundPl->getPosition());
                        }

                    }

                    return true;
                }
            }
        }

        // target not found, out of view
        cp->setAttackMode(false);

        if (cp->getType() == Character::player) {
            ServerCommandPointer cmd = std::make_shared<TargetLostTC>();
            dynamic_cast<Player *>(cp)->Connection->addCommand(cmd);
        }

        return false;
    } else {
        return true;
    }

}


bool World::killMonster(TYPE_OF_CHARACTER_ID id) {
    auto monster = Monsters.find(id);

    if (monster) {
        const auto &monsterPos = monster->getPosition();
        Field *field;

        if (GetPToCFieldAt(field, monsterPos)) {
            field->removeChar();
        }

        sendRemoveCharToVisiblePlayers(monster->getId(), monsterPos);
        Monsters.erase(id);
        delete monster;

        return true;
    }

    return false;
}


Field *World::GetField(const position &pos) const {
    WorldMap::map_t temp;

    if (maps.findMapForPos(pos, temp)) {
        Field *field = nullptr;

        if (temp->GetPToCFieldAt(field, pos.x, pos.y)) {
            return field;
        } else {
            return nullptr;
        }
    } else {
        return nullptr;
    }
}


bool World::GetPToCFieldAt(Field *&fip, const position &pos) const {

    WorldMap::map_t temp;

    if (maps.findMapForPos(pos, temp)) {
        return temp->GetPToCFieldAt(fip, pos.x, pos.y);
    }

    return false;

}


bool World::GetPToCFieldAt(Field *&fip, const position &pos, WorldMap::map_t &map) const {

    if (maps.findMapForPos(pos, map)) {
        return map->GetPToCFieldAt(fip, pos.x, pos.y);
    }

    return false;

}


bool World::findEmptyCFieldNear(Field *&cf, position &pos) {

    WorldMap::map_t temp;

    if (maps.findMapForPos(pos, temp)) {
        return temp->findEmptyCFieldNear(cf, pos.x, pos.y);
    }

    return false;

}


int World::getItemAttrib(const std::string &s, TYPE_OF_ITEM_ID ItemID) {

    // Armor //
    if (s == "bodyparts") {
        if (Data::ArmorItems.exists(ItemID)) {
            return Data::ArmorItems[ItemID].BodyParts;
        }
    } else if (s == "strokearmor") {
        if (Data::ArmorItems.exists(ItemID)) {
            return Data::ArmorItems[ItemID].StrokeArmor;
        }
    } else if (s == "thrustarmor") {
        if (Data::ArmorItems.exists(ItemID)) {
            return Data::ArmorItems[ItemID].ThrustArmor;
        }
    } else if (s == "armormagicdisturbance") {
        if (Data::ArmorItems.exists(ItemID)) {
            return Data::ArmorItems[ItemID].MagicDisturbance;
        }
    }

    // Common //
    else if (s == "agingspeed") {
        const auto &common = Data::CommonItems[ItemID];

        if (common.isValid()) {
            return common.AgeingSpeed;
        }
    } else if (s == "objectafterrot") {
        const auto &common = Data::CommonItems[ItemID];

        if (common.isValid()) {
            return common.ObjectAfterRot;
        }
    } else if (s == "weight") {
        const auto &common = Data::CommonItems[ItemID];

        if (common.isValid()) {
            return common.Weight;
        }
    }

    // Tiles Modificator //
    else if (s == "modificator") {
        if (Data::TilesModItems.exists(ItemID)) {
            return Data::TilesModItems[ItemID].Modificator;
        }
    }

    // Weapon //
    else if (s == "accuracy") {
        if (Data::WeaponItems.exists(ItemID)) {
            return Data::WeaponItems[ItemID].Accuracy;
        }
    } else if (s == "attack") {
        if (Data::WeaponItems.exists(ItemID)) {
            return Data::WeaponItems[ItemID].Attack;
        }
    } else if (s == "defence") {
        if (Data::WeaponItems.exists(ItemID)) {
            return Data::WeaponItems[ItemID].Defence;
        }
    } else if (s == "range") {
        if (Data::WeaponItems.exists(ItemID)) {
            return Data::WeaponItems[ItemID].Range;
        }
    } else if (s == "weapontype") {
        if (Data::WeaponItems.exists(ItemID)) {
            return Data::WeaponItems[ItemID].Type;
        }
    } else if (s == "weaponmagicdisturbance") {
        if (Data::WeaponItems.exists(ItemID)) {
            return Data::WeaponItems[ItemID].MagicDisturbance;
        }
    }

    return 0;

}


void World::closeShowcasesForContainerPositions() {
    for (const auto &pos : *contpos) {
        for (const auto &player : Players.findAllCharactersInMaxRangeOf(pos, 1)) {
            player->closeAllShowcasesOfMapContainers();
        }
    }
}


void World::updatePlayerView(short int startx, short int endx) {
    std::vector<Player *> temp;

    if (Players.findAllCharactersWithXInRangeOf(startx - 20, endx + 20, temp)) {
        for (const auto &player : temp) {
            player->sendFullMap();
            sendAllVisibleCharactersToPlayer(player, true);
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
        for (const auto &map : mapsToage) {
            map->ageItemsInHorizontalRange(nextXtoage, lastXtoage);
        }

        closeShowcasesForContainerPositions();
        contpos->clear();
    }

    nextXtoage = lastXtoage + 1;

    return true;

}


void World::AgeInventory() {
    Players.for_each(&Player::ageInventory);
    Monsters.for_each(&Monster::ageInventory);
}


void World::Save(const std::string &prefix) {
    std::string path = directory + std::string(MAPDIR) + prefix;

    maps.saveToDisk(path);

    std::ofstream specialfile((path + "_specialfields").c_str(), std::ios::binary | std::ios::out | std::ios::trunc);

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

        for (const auto &field : specialfields) {
            specialfile.write((char *) & (field.first), sizeof(field.first));
            specialfile.write((char *) & (field.second.type), sizeof(field.second.type));
            specialfile.write((char *) & (field.second.flags), sizeof(field.second.flags));
        }

        specialfile.close();
    }

}


void World::Load(const std::string &prefix) {
    std::string path = directory + std::string(MAPDIR) + prefix;

    std::ifstream mapinitfile((path + "_initmaps").c_str(), std::ios::binary | std::ios::in);

    if (! mapinitfile.good()) {
        Logger::error(LogFacility::World) << "Error while loading maps: could not open " << (path + "_initmaps") << Log::end;
        Logger::info(LogFacility::World) << "trying to import maps" << Log::end;
        load_maps();
        return;
    } else {
        unsigned short int size;
        mapinitfile.read((char *) & size, sizeof(size));
        Logger::info(LogFacility::World) << "Loading " << size << " maps" << Log::end;

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

            sprintf(mname, "%s_%6d_%6d_%6d", path.c_str(), tZ_Level, tMin_X, tMin_Y);

            // if the map loads ok...
            if (tempMap->Load(mname, 0, 0)) {
                maps.InsertMap(tempMap);    // insert it
            }
        }

        mapinitfile.close();
    }

    std::ifstream specialfile((path + "_specialfields").c_str(), std::ios::binary | std::ios::in);

    if (! specialfile.good()) {
        Logger::error(LogFacility::World) << "Error while loading maps: could not open " << (path + "_specialfields") << Log::end;
        // TODO propably should terminate the server due to a severe error here...
        return;
    } else {
        unsigned short int size3;
        specialfile.read((char *) & size3, sizeof(size3));
        Logger::info(LogFacility::World) << "Loading " << size3 << " special fields" << Log::end;

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

int World::getTime(const std::string &timeType) {
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


bool World::findWarpFieldsInRange(const position &pos, short int range, std::vector<position> &warppositions) {
    for (int x = pos.x - range; x <= pos.x + range; ++x) {
        for (int y = pos.y - range; y <= pos.y + range; ++y) {
            const position p(x, y, pos.z);
            Field *cf = 0;

            if (GetPToCFieldAt(cf, p) && cf->IsWarpField()) {
                warppositions.push_back(p);
            }
        }
    }

    return !warppositions.empty();
}


void World::setWeatherPart(const std::string &type, char value) {
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

void World::sendRemoveCharToVisiblePlayers(TYPE_OF_CHARACTER_ID id, const position &pos) {
    ServerCommandPointer cmd = std::make_shared<RemoveCharTC>(id);

    for (const auto &player : Players.findAllCharactersInScreen(pos)) {
        player->sendCharRemove(id, cmd);
    }
}

void World::sendHealthToAllVisiblePlayers(Character *cc, Attribute::attribute_t health) {
    if (!cc->isInvisible()) {
        const auto &charPos = cc->getPosition();
        char xoffs;
        char yoffs;
        char zoffs;

        for (const auto &player : Players.findAllCharactersInScreen(cc->getPosition())) {
            const auto &playerPos = player->getPosition();
            xoffs = charPos.x - playerPos.x;
            yoffs = charPos.y - playerPos.y;
            zoffs = charPos.z - playerPos.z + RANGEDOWN;

            if ((xoffs != 0) || (yoffs != 0) || (zoffs != RANGEDOWN)) {
                ServerCommandPointer cmd = std::make_shared<UpdateAttribTC>(cc->getId(), "hitpoints", health);
                player->Connection->addCommand(cmd);
            }
        }
    }
}

