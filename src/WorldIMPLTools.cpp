/*
 * Illarionserver - server for the game Illarion
 * Copyright 2011 Illarion e.V.
 *
 * This file is part of Illarionserver.
 *
 * Illarionserver  is  free  software:  you can redistribute it and/or modify it
 * under the terms of the  GNU Affero General Public License as published by the Free
 * Software Foundation, either version 3 of the License, or (at your option) any
 * later version.
 *
 * Illarionserver is distributed in the hope that it will be useful, but WITHOUT
 * ANY WARRANTY;  without  even  the  implied  warranty  of  MERCHANTABILITY  or
 * FITNESS FOR A PARTICULAR PURPOSE. See the GNU Affero General Public License for more
 * details.
 *
 * You should have received a copy of the GNU Affero General Public License along with
 * Illarionserver. If not, see <http://www.gnu.org/licenses/>.
 */

#include "Monster.hpp"
#include "NPC.hpp"
#include "Player.hpp"
#include "World.hpp"
#include "data/ArmorObjectTable.hpp"
#include "data/ContainerObjectTable.hpp"
#include "data/Data.hpp"
#include "data/MonsterTable.hpp"
#include "data/TilesModificatorTable.hpp"
#include "data/WeaponObjectTable.hpp"
#include "db/Connection.hpp"
#include "db/ConnectionManager.hpp"
#include "db/DeleteQuery.hpp"
#include "db/InsertQuery.hpp"
#include "map/Field.hpp"
#include "netinterface/protocol/ServerCommands.hpp"
#include "script/LuaWeaponScript.hpp"

#include <algorithm>
#include <cstdlib>
#include <list>
#include <range/v3/all.hpp>

extern MonsterTable *monsterDescriptions;
extern std::shared_ptr<LuaWeaponScript> standardFightingScript;

void World::deleteAllLostNPC() {
    for (const TYPE_OF_CHARACTER_ID &npcToDelete : LostNpcs) {
        const auto &npc = Npc.find(npcToDelete);

        if (npc != nullptr) {
            try {
                fieldAt(npc->getPosition()).removeChar();
            } catch (FieldNotFound &) {
            }

            sendRemoveCharToVisiblePlayers(npc->getId(), npc->getPosition());
            delete npc;
        }
    }

    LostNpcs.clear();
}

auto World::findTargetsInSight(const position &pos, uint8_t range, std::vector<Character *> &ret,
                               Character::face_to direction) const -> bool {
    bool found = false;

    for (const auto &candidate : getTargetsInRange(pos, range)) {
        bool indir = false;
        const position &candidatePos = candidate->getPosition();

        switch (direction) {
        case Character::north:

            if (candidatePos.y <= pos.y) {
                indir = true;
            }

            break;

        case Character::northeast:

            if (candidatePos.x - pos.x >= candidatePos.y - pos.y) {
                indir = true;
            }

            break;

        case Character::east:

            if (candidatePos.x >= pos.x) {
                indir = true;
            }

            break;

        case Character::southeast:

            if (candidatePos.y - pos.y >= pos.x - candidatePos.x) {
                indir = true;
            }

            break;

        case Character::south:

            if (candidatePos.y >= pos.y) {
                indir = true;
            }

            break;

        case Character::southwest:

            if (candidatePos.x - pos.x <= candidatePos.y - pos.y) {
                indir = true;
            }

            break;

        case Character::west:

            if (candidatePos.x <= pos.x) {
                indir = true;
            }

            break;

        case Character::northwest:

            if (candidatePos.y - pos.y >= pos.x - candidatePos.x) {
                indir = true;
            }

            break;

        default:
            indir = true;
            break;
        }

        if (indir) {
            std::list<BlockingObject> objects = LoS(pos, candidate->getPosition());

            if (objects.empty()) {
                ret.push_back(candidate);
                found = true;
            }
        }
    }

    return found;
}

auto World::LoS(const position &startingpos, const position &endingpos) const -> std::list<BlockingObject> {
    std::list<BlockingObject> ret;
    ret.clear();
    bool steep = std::abs(startingpos.y - endingpos.y) > std::abs(startingpos.x - endingpos.x);
    short int startx = startingpos.x;
    short int starty = startingpos.y;
    short int endx = endingpos.x;
    short int endy = endingpos.y;

    if (steep) {
        // swap x,y values for correct execution in negative range
        std::swap(startx, starty);
        std::swap(endx, endy);
    }

    bool swapped = startx > endx;

    if (swapped) {
        std::swap(startx, endx);
        std::swap(starty, endy);
    }

    short int deltax = endx - startx;
    short int deltay = std::abs(endy - starty);
    short int error = 0;
    short int ystep = 1;
    short int y = starty;

    if (starty > endy) {
        ystep = -1;
    }

    for (short int x = startx; x <= endx; ++x) {
        if (!(x == startx && y == starty) && !(x == endx && y == endy)) {
            BlockingObject bo;
            position pos{x, y, startingpos.z};

            if (steep) {
                pos.x = y;
                pos.y = x;
            }

            try {
                const map::Field &field = fieldAt(pos);

                if (field.hasPlayer()) {
                    bo.blockingType = BlockingObject::BT_CHARACTER;
                    bo.blockingChar = findCharacterOnField(pos);

                    if (swapped) {
                        ret.push_back(bo);
                    } else {
                        ret.push_front(bo);
                    }
                } else {
                    ScriptItem it;

                    for (size_t i = 0; i < field.itemCount(); ++i) {
                        auto testItem = field.getStackItem(i);

                        if (testItem.getVolume() > it.getVolume()) {
                            it = testItem;
                        }
                    }

                    if (it.isLarge()) {
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
            } catch (FieldNotFound &) {
            }
        }

        error += deltay;

        if (2 * error >= deltax) {
            y += ystep;
            error -= deltax;
        }
    }

    return ret;
}

// function which updates the playerlist.
void World::updatePlayerList() const {
    using namespace Database;

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

            Players.for_each([&](Player *player) { insQuery.addValue<TYPE_OF_CHARACTER_ID>(column, player->getId()); });

            insQuery.execute();
        }

        connection->commitTransaction();
    } catch (std::exception &e) {
        Logger::error(LogFacility::World) << "Exception during saving online player list: " << e.what() << Log::end;
        connection->rollbackTransaction();
    }
}

auto World::findCharacterOnField(const position &pos) const -> Character * {
    Character *tmpChr = Players.find(pos);

    if (tmpChr != nullptr) {
        return tmpChr;
    }

    tmpChr = Monsters.find(pos);

    if (tmpChr != nullptr) {
        return tmpChr;
    }

    tmpChr = Npc.find(pos);

    if (tmpChr != nullptr) {
        return tmpChr;
    }

    return nullptr;
}

auto World::findPlayerOnField(const position &pos) const -> Player * { return Players.find(pos); }

auto World::findCharacter(TYPE_OF_CHARACTER_ID id) -> Character * {
    if (id < MONSTER_BASE) {
        auto *tmpChr = dynamic_cast<Character *>(Players.find(id));

        if (tmpChr != nullptr) {
            return tmpChr;
        }
    } else if (id < NPC_BASE) {
        auto *tmpChr = dynamic_cast<Character *>(Monsters.find(id));

        if (tmpChr != nullptr) {
            return tmpChr;
        }
        using namespace ranges;

        auto idsMatch = [id](const auto &monster) { return monster->getId() == id; };

        auto result = find_if(newMonsters, idsMatch);

        if (result != newMonsters.end()) {
            return *result;
        }

    } else {
        auto *tmpChr = dynamic_cast<Character *>(Npc.find(id));

        if (tmpChr != nullptr) {
            return tmpChr;
        }
    }

    return nullptr;
}

void World::takeMonsterAndNPCFromMap() {
    Monsters.for_each([this](Monster *monster) {
        try {
            fieldAt(monster->getPosition()).removeMonster();
        } catch (FieldNotFound &) {
        }

        delete monster;
    });

    Npc.for_each([this](NPC *npc) {
        try {
            fieldAt(npc->getPosition()).removeNPC();
        } catch (FieldNotFound &) {
        }

        delete npc;
    });

    Monsters.clear();
    Npc.clear();
}

// only invoked when ATTACK***_TS is received or when a monster attacks
auto World::characterAttacks(Character *cp) const -> bool {
    if (cp->enemyid != cp->getId()) {
        if (cp->enemytype == Character::player) {
            Player *temppl = Players.find(cp->enemyid);

            // Ziel gefunden
            if (temppl != nullptr) {
                // Ziel sichtbar
                if (cp->isInRange(temppl, temppl->getScreenRange())) {
                    // Ziel ist tot
                    if (!cp->attack(temppl)) {
                        sendSpinToAllVisiblePlayers(temppl);

                        cp->setAttackMode(false);

                        // set lasttargetseen to false if the player who was attacked is death
                        if (cp->getType() == Character::monster) {
                            auto *mon = dynamic_cast<Monster *>(cp);
                            mon->lastTargetSeen = false;
                        }

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

            if (temppl != nullptr) {
                if (cp->isInRange(temppl, temppl->getScreenRange())) {
                    const auto monsterType = temppl->getMonsterType();

                    if (monsterDescriptions->exists(monsterType)) {
                        const auto &monStruct = (*monsterDescriptions)[monsterType];

                        if (monStruct.script) {
                            monStruct.script->onAttacked(temppl, cp);
                        }
                    }

                    if (!cp->attack(temppl)) {
                        cp->setAttackMode(false);

                        if (cp->getType() == Character::player) {
                            ServerCommandPointer cmd = std::make_shared<TargetLostTC>();
                            dynamic_cast<Player *>(cp)->Connection->addCommand(cmd);
                        }
                    } else {
                        // check for turning into attackackers direction
                        std::vector<Character *> temp;
                        temp.clear();
                        findTargetsInSight(temppl->getPosition(), static_cast<uint8_t>(9), temp, temppl->getFaceTo());

                        // add the current attacker to the list
                        if (cp->getType() == Character::player) {
                            temp.push_back(dynamic_cast<Player *>(cp));
                        }

                        if (!temp.empty()) {
                            Character *target = standardFightingScript->setTarget(temppl, temp);

                            if (target != nullptr) {
                                temppl->turn(target->getPosition());
                            }
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
    }
    return true;
}

auto World::killMonster(TYPE_OF_CHARACTER_ID id) -> bool {
    auto *monster = Monsters.find(id);

    if (monster != nullptr) {
        const auto &monsterPos = monster->getPosition();

        try {
            fieldAt(monsterPos).removeChar();
        } catch (FieldNotFound &) {
        }

        sendRemoveCharToVisiblePlayers(monster->getId(), monsterPos);
        Monsters.erase(id);
        delete monster;

        return true;
    }

    return false;
}

auto World::fieldAt(const position &pos) -> map::Field & { return maps.at(pos); }

auto World::fieldAt(const position &pos) const -> const map::Field & { return maps.at(pos); }

auto World::fieldAtOrBelow(position &pos) -> map::Field & {
    for (size_t i = 0; i <= RANGEDOWN; ++i) {
        map::Field &field = fieldAt(pos);

        if (!field.isTransparent()) {
            return field;
        }

        --pos.z;
    }

    throw FieldNotFound();
}

auto World::walkableFieldNear(const position &pos) -> map::Field & { return walkableNear(maps, pos); }

void World::makePersistentAt(const position &pos) { maps.makePersistentAt(pos); }

void World::removePersistenceAt(const position &pos) { maps.removePersistenceAt(pos); }

auto World::isPersistentAt(const position &pos) const -> bool { return maps.isPersistentAt(pos); }

auto World::getItemAttrib(const std::string &s, TYPE_OF_ITEM_ID ItemID) -> int {
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

    // Item //
    else if (s == "agingspeed") {
        const auto &itemStruct = Data::Items[ItemID];

        if (itemStruct.isValid()) {
            return itemStruct.AgeingSpeed;
        }
    } else if (s == "objectafterrot") {
        const auto &itemStruct = Data::Items[ItemID];

        if (itemStruct.isValid()) {
            return itemStruct.ObjectAfterRot;
        }
    } else if (s == "weight") {
        const auto &itemStruct = Data::Items[ItemID];

        if (itemStruct.isValid()) {
            return itemStruct.Weight;
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

void World::updatePlayerView(short int startx, short int endx) {
    std::vector<Player *> temp;

    if (Players.findAllCharactersWithXInRangeOf(startx - 20, endx + 20, temp)) {
        for (const auto &player : temp) {
            player->sendFullMap();
            sendAllVisibleCharactersToPlayer(player, true);
        }
    }
}

void World::ageMaps() {
    if (not maps.allMapsAged()) {
        scheduler.addOneshotTask([&] { ageMaps(); }, std::chrono::seconds(1), "age_maps");
    }
}

void World::ageInventory() const {
    Players.for_each(&Player::ageInventory);
    Monsters.for_each(&Monster::ageInventory);
}

void World::Save() const { maps.saveToDisk(); }

void World::Load() {
    if (!maps.loadFromDisk()) {
        maps.importFromEditor();
    }
}

void World::import() { maps.importFromEditor(); }

auto World::getTime(const std::string &timeType) -> int {
    // return unix timestamp if requsted and quit function
    if (timeType == "unix") {
        return (int)time(nullptr);
    }

    // get current time and timezone data to get additional informations for time conversation
    time_t curr_unixtime = time(nullptr);
    struct tm *timestamp = localtime(&curr_unixtime);

    auto illaTime = (int)curr_unixtime;

    // in case its currently dst, correct the timestamp so the illarion time changes the timestamp as well
    if (timestamp->tm_isdst != 0) {
        illaTime += 3600;
    }

    // Illarion seconds since 17th February 2000
    // RL Seconds * 3
    illaTime = (illaTime - 950742000) * 3;

    if (timeType == "illarion") {
        return (int)illaTime;
    }

    // Calculating year
    // 31536000 == 60*60*24*365
    auto year = (int)(illaTime / 31536000);
    illaTime -= year * 31536000;

    // Calculating day
    // 86400 = 60*60*24
    auto day = (int)(illaTime / 86400);
    illaTime -= day * 86400;
    ++day;

    // Calculating month
    // 24 days per month
    auto month = (int)(day / 24);
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
    if (timeType == "year") {
        return year;
    }
    if (timeType == "month") {
        return month;
    }
    if (timeType == "day") {
        return day;
    }

    // Calculate the time of day
    // Calculating hour
    // 3600 = 60 * 60
    const auto hour = (int)(illaTime / 3600);
    illaTime -= hour * 3600;

    // Calculating minute
    const auto minute = (int)(illaTime / 60);

    // Calculating seconds
    illaTime -= minute * 60;

    // returning the last possible values
    if (timeType == "hour") {
        return hour;
    }
    if (timeType == "minute") {
        return minute;
    }
    if (timeType == "second") {
        return illaTime;
    }
    return -1;
}

auto World::findWarpFieldsInRange(const position &pos, short int range, std::vector<position> &warppositions) -> bool {
    for (int x = pos.x - range; x <= pos.x + range; ++x) {
        for (int y = pos.y - range; y <= pos.y + range; ++y) {
            try {
                const position p(x, y, pos.z);

                if (fieldAt(p).isWarp()) {
                    warppositions.push_back(p);
                }
            } catch (FieldNotFound &) {
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

void World::sendRemoveCharToVisiblePlayers(TYPE_OF_CHARACTER_ID id, const position &pos) const {
    ServerCommandPointer cmd = std::make_shared<RemoveCharTC>(id);

    for (const auto &player : Players.findAllCharactersInScreen(pos)) {
        player->sendCharRemove(id, cmd);
    }
}

void World::sendHealthToAllVisiblePlayers(Character *cc, Attribute::attribute_t health) const {
    if (!cc->isInvisible()) {
        const auto &charPos = cc->getPosition();

        for (const auto &player : Players.findAllCharactersInScreen(cc->getPosition())) {
            const auto &playerPos = player->getPosition();
            char xoffs = charPos.x - playerPos.x;
            char yoffs = charPos.y - playerPos.y;
            char zoffs = charPos.z - playerPos.z + RANGEDOWN;

            if ((xoffs != 0) || (yoffs != 0) || (zoffs != RANGEDOWN)) {
                ServerCommandPointer cmd = std::make_shared<UpdateAttribTC>(cc->getId(), "hitpoints", health);
                player->Connection->addCommand(cmd);
            }
        }
    }
}
