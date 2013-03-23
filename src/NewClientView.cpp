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


#include "NewClientView.hpp"
#include "Map.hpp"
#include "WorldMap.hpp"
#include <iostream>

NewClientView::~NewClientView() {}

NewClientView::NewClientView() : viewPosition(position(0,0,0)), exists(false), stripedir(dir_right), maxtiles(0) {
    for (int i = 0; i < 100; ++i) {
        mapStripe[i] = NULL;
    }
}

void NewClientView::fillStripe(position pos, stripedirection dir, int length, const WorldMap &maps) {
    clearStripe();
    viewPosition = pos;
    stripedir = dir;
    readFields(length, maps);
}

void NewClientView::clearStripe() {
    for (int i = 0; i < 100; ++i) {
        mapStripe[i] = NULL;
    }

    exists = false;
    viewPosition.x = 0;
    viewPosition.y = 0;
    viewPosition.z = 0;
    maxtiles = 0;
}

void NewClientView::readFields(int length, const WorldMap &maps) {
    int x = viewPosition.x;
    int y = viewPosition.y;
    int x_inc = (stripedir == dir_right) ? 1 : -1;

    WorldMap::map_vector_t good_maps;

    if (maps.findAllMapsInRangeOf(0, length-1, (stripedir == dir_right) ? length-1 : 0, (stripedir == dir_right) ? 0 : length-1, viewPosition, good_maps)) {
        WorldMap::map_t map;
        int tmp_maxtiles = 1;

        for (int i = 0; i < length; ++i) {
            Field *field = NULL;

            if (!map || !map->GetPToCFieldAt(field,x,y)) {
                map.reset();

                for (auto it = good_maps.begin(); it != good_maps.end(); ++it) {
                    if ((*it)->GetPToCFieldAt(field,x,y)) {
                        map = *it;
                        break;
                    }
                }
            }

            if (field)
                if (((field->getTileId() != TRANSPARENT) && (field->getTileId() != TRANSPARENTDISAPPEAR)) || !field->items.empty()) {
                    exists = true;
                    mapStripe[i] = field;
                    maxtiles = tmp_maxtiles;
                }

            ++tmp_maxtiles;
            //increase x due to perspective
            x += x_inc;
            //increase y due to perspective
            ++y;
        }
    }
}

