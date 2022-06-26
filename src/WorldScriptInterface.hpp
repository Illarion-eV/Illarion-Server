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

#ifndef WORLD_SCRIPT_INTERFACE_HPP
#define WORLD_SCRIPT_INTERFACE_HPP

#include <list>
#include <string>
#include <vector>

struct position;
struct ArmorStruct;
struct AttackBoni;
struct BlockingObject;
class Character;
class Monster;
struct MonsterArmor;
class NPC;
class Player;
struct WeaponStruct;

class WorldScriptInterface {
public:
    WorldScriptInterface() = default;
    virtual ~WorldScriptInterface() = default;
    WorldScriptInterface(const WorldScriptInterface &) = default;
    auto operator=(const WorldScriptInterface &) -> WorldScriptInterface & = default;
    WorldScriptInterface(WorldScriptInterface &&) = default;
    auto operator=(WorldScriptInterface &&) -> WorldScriptInterface & = default;

    // Related to Character

    virtual auto createDynamicNPC(const std::string &name, TYPE_OF_RACE_ID type, const position &pos,
                                  Character::sex_type sex, const std::string &scriptname) -> bool = 0;
    virtual auto createMonster(unsigned short id, const position &pos, short movepoints) -> character_ptr = 0;
    virtual auto deleteNPC(unsigned int npcid) -> bool = 0;
    [[nodiscard]] virtual auto getCharactersInRangeOf(const position &pos, uint8_t radius) const
            -> std::vector<Character *> = 0;
    [[nodiscard]] virtual auto getMonstersInRangeOf(const position &pos, uint8_t radius) const
            -> std::vector<Monster *> = 0;
    [[nodiscard]] virtual auto getNPCS() const -> std::vector<NPC *> = 0;
    [[nodiscard]] virtual auto getNPCSInRangeOf(const position &pos, uint8_t radius) const -> std::vector<NPC *> = 0;
    [[nodiscard]] virtual auto getPlayersInRangeOf(const position &pos, uint8_t radius) const
            -> std::vector<Player *> = 0;
    [[nodiscard]] virtual auto getPlayersOnline() const -> std::vector<Player *> = 0;
    [[nodiscard]] virtual auto getPlayerIdByName(const std::string &name, TYPE_OF_CHARACTER_ID &id) const -> bool = 0;

    // Related to Data

    virtual auto getArmorStruct(TYPE_OF_ITEM_ID id, ArmorStruct &ret) -> bool = 0;
    [[nodiscard]] virtual auto getItemName(TYPE_OF_ITEM_ID itemid, uint8_t language) const -> std::string = 0;
    [[nodiscard]] virtual auto getItemStats(const ScriptItem &item) const -> ItemStruct = 0;
    [[nodiscard]] virtual auto getItemStatsFromId(TYPE_OF_ITEM_ID id) const -> ItemStruct = 0;
    virtual auto getMonsterAttack(TYPE_OF_RACE_ID id, AttackBoni &ret) -> bool = 0;
    virtual auto getNaturalArmor(TYPE_OF_RACE_ID id, MonsterArmor &ret) -> bool = 0;
    virtual auto getWeaponStruct(TYPE_OF_ITEM_ID id, WeaponStruct &ret) -> bool = 0;

    // Related to Item

    virtual auto changeItem(ScriptItem item) -> bool = 0;
    virtual void changeQuality(ScriptItem item, int amount) = 0;
    virtual auto createFromId(TYPE_OF_ITEM_ID id, int count, const position &pos, bool always, int quality,
                              script_data_exchangemap const *data) -> ScriptItem = 0;
    virtual auto createFromItem(const ScriptItem &item, const position &pos, bool always) -> bool = 0;
    virtual auto erase(ScriptItem item, int amount) -> bool = 0;
    virtual auto increase(ScriptItem item, int count) -> bool = 0;
    virtual auto swap(ScriptItem item, TYPE_OF_ITEM_ID newItem, int newQuality = 0) -> bool = 0;

    // Related to WorldMap

    [[nodiscard]] virtual auto blockingLineOfSight(const position &start, const position &end) const
            -> std::list<BlockingObject> = 0;
    virtual void changeTile(short int tileid, const position &pos) = 0;
    virtual auto createSavedArea(uint16_t tile, const position &origin, uint16_t height, uint16_t width) -> bool = 0;
    virtual auto fieldAt(const position &pos) -> map::Field & = 0;
    [[nodiscard]] virtual auto fieldAt(const position &pos) const -> const map::Field & = 0;
    [[nodiscard]] virtual auto getCharacterOnField(const position &pos) const -> character_ptr = 0;
    [[nodiscard]] virtual auto isCharacterOnField(const position &pos) const -> bool = 0;
    virtual auto getItemOnField(const position &pos) -> ScriptItem = 0;
    virtual auto isItemOnField(const position &pos) -> bool = 0;
    virtual void makePersistentAt(const position &pos) = 0;
    virtual void removePersistenceAt(const position &pos) = 0;
    [[nodiscard]] virtual auto isPersistentAt(const position &pos) const -> bool = 0;

    // Other

    virtual void broadcast(const std::string &german, const std::string &english) const = 0;
    [[nodiscard]] virtual auto getTime(const std::string &timeType) const -> int = 0;
    virtual void gfx(unsigned short int gfxid, const position &pos) const = 0;
    virtual void makeSound(unsigned short int soundid, const position &pos) const = 0;
    virtual void sendMonitoringMessage(const std::string &msg, unsigned char id = 0) const = 0;
};

#endif
