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

#ifndef _NPC_TABLE_HPP_
#define _NPC_TABLE_HPP_

#include <string>
#include <list>

#include "globals.hpp"
#include "World.hpp"
#include "NPC.hpp"
#include "Field.hpp"

class World;

struct NPCStruct {
    position NPCPos;
    std::string Name;
    TYPE_OF_RACE_ID type;
    Character::face_to faceto;
    std::vector<struct NPCTalk> speechTexts;
    unsigned short int walk_range;
    bool ishealer;
};


class NPCTable {
public:
    NPCTable();
    ~NPCTable();

    bool dataOK() const {
        return m_dataOK;
    };

private:
    void reload();
    bool m_dataOK;

    World *_world;
    std::list<struct NPCStruct> NPCList; //List zum aufnehmen aller Werte aus der DB
};

#endif

