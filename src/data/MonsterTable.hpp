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

#ifndef _MONSTER_TABLE_HPP_
#define _MONSTER_TABLE_HPP_

#include <string>
#include <map>
#include <vector>
#include <memory>
#include <boost/unordered_map.hpp>
#include "script/LuaMonsterScript.hpp"
#include "types.hpp"
#include "Character.hpp"

struct itemdef_t {
    TYPE_OF_ITEM_ID itemid;
    std::pair<unsigned short, unsigned short> amount;
    TYPE_OF_AGINGSPEED AgeingSpeed;
};

struct attributedef_t {
    std::pair<unsigned short, unsigned short> luck;
    std::pair<unsigned short, unsigned short> strength;
    std::pair<unsigned short, unsigned short> dexterity;
    std::pair<unsigned short, unsigned short> constitution;
    std::pair<unsigned short, unsigned short> agility;
    std::pair<unsigned short, unsigned short> intelligence;
    std::pair<unsigned short, unsigned short> perception;
    std::pair<unsigned short, unsigned short> willpower;
    std::pair<unsigned short, unsigned short> essence;

    attributedef_t():
        luck(std::make_pair(10, 10)),
        strength(std::make_pair(15, 15)),
        dexterity(std::make_pair(10, 10)),
        constitution(std::make_pair(8, 8)),
        agility(std::make_pair(10, 10)),
        intelligence(std::make_pair(10, 10)),
        perception(std::make_pair(10, 10)),
        willpower(std::make_pair(10, 10)),
        essence(std::make_pair(10, 10))
    {}

};

struct MonsterStruct {
    typedef std::map<TYPE_OF_SKILL_ID, std::pair<unsigned short, unsigned short> > skilltype;
    typedef std::map<unsigned short, std::vector<itemdef_t> > itemtype;
    std::string nameDe;
    std::string nameEn;
    TYPE_OF_RACE_ID race;
    unsigned short hitpoints;
    bool canselfheal;
    Character::movement_type movement;
    bool canattack;
    attributedef_t  attributes;
    skilltype skills;
    itemtype items;
    std::shared_ptr<LuaMonsterScript> script;
    uint16_t minsize;
    uint16_t maxsize;
};

class MonsterTable {
public:
    MonsterTable();
    ~MonsterTable();

    inline bool dataOK() {
        return m_dataOK;
    }

    bool find(TYPE_OF_CHARACTER_ID Id, MonsterStruct &ret);

private:
    typedef boost::unordered_map<TYPE_OF_CHARACTER_ID, MonsterStruct> TABLE;
    TABLE m_table;

    void clearOldTable();

    void reload();
    bool m_dataOK;

    World *world;
};
#endif

