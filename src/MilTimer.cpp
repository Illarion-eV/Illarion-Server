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


#include "MilTimer.hpp"

MilTimer::MilTimer(long milsec) {
    gap = milsec;
    ftime(&last);
}



bool MilTimer::Next() {
    timeb now;
    ftime(&now);

    long temp = ((now.time - last.time) * 1000) + (now.millitm - last.millitm);

    if (gap > temp) {
        return false;
    } else {
        last = now;
        return true;
    }
}

