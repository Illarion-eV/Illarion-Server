//  illarionserver - server for the game Illarion
//  Copyright 2011 Illarion e.V.
//
//  This file is part of illarionserver.
//
//  illarionserver is free software: you can redistribute it and/or modify
//  it under the terms of the GNU Affero General Public License as published by
//  the Free Software Foundation, either version 3 of the License, or
//  (at your option) any later version.
//
//  illarionserver is distributed in the hope that it will be useful,
//  but WITHOUT ANY WARRANTY; without even the implied warranty of
//  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//  GNU Affero General Public License for more details.
//
//  You should have received a copy of the GNU Affero General Public License
//  along with illarionserver.  If not, see <http://www.gnu.org/licenses/>.


#include "Field.hpp"
#include "NewClientView.hpp"
#include "WorldMap.hpp"
#include <iostream>

NewClientView::~NewClientView() {}

NewClientView::NewClientView()
    : viewPosition(position(0, 0, 0)), exists(false), stripedir(dir_right),
      maxtiles(0) {
    for (auto &field : mapStripe) {
        field = nullptr;
    }
}

void NewClientView::fillStripe(position pos, stripedirection dir, int length,
                               const WorldMap &maps) {
    clearStripe();
    viewPosition = pos;
    stripedir = dir;
    readFields(length, maps);
}

void NewClientView::clearStripe() {
    for (auto &field : mapStripe) {
        field = nullptr;
    }

    exists = false;
    viewPosition.x = 0;
    viewPosition.y = 0;
    viewPosition.z = 0;
    maxtiles = 0;
}

void NewClientView::readFields(int length, const WorldMap &maps) {
    position pos = viewPosition;
    int x_inc = (stripedir == dir_right) ? 1 : -1;
    int tmp_maxtiles = 1;

    for (int i = 0; i < length; ++i) {
        try {
            Field &field = maps.at(pos);

            if (!field.isTransparent() || !field.items.empty()) {
                exists = true;
                mapStripe[i] = &field;
                maxtiles = tmp_maxtiles;
            }
        } catch (FieldNotFound &) {
        }

        ++tmp_maxtiles;
        //increase x due to perspective
        pos.x += x_inc;
        //increase y due to perspective
        ++pos.y;
    }
}

