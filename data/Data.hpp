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

#ifndef _DATA_HPP_
#define _DATA_HPP_

#include "data/NamesObjectTable.hpp"
#include "data/WeaponObjectTable.hpp"
#include "data/ArmorObjectTable.hpp"
#include "data/ContainerObjectTable.hpp"
#include "data/TilesModificatorTable.hpp"
#include "data/TilesTable.hpp"

namespace Data {

extern NamesObjectTable ItemNames;
extern WeaponObjectTable WeaponItems;
extern ArmorObjectTable ArmorItems;
extern ContainerObjectTable ContainerItems;
extern TilesTable Tiles;
extern TilesModificatorTable TilesModItems;

std::vector<Table *> getTables();
bool reloadTables();
void reloadScripts();
void activateReload();
bool reload();

}

#endif

