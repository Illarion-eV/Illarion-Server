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

#include "Random.hpp"

std::mt19937 Random::rng;

double Random::uniform() {
    std::uniform_real_distribution<double> uniform(0, 1);
    return uniform(rng);
}

int Random::uniform(int min, int max) {
    std::uniform_int_distribution<int> uniform(min, max);
    return uniform(rng);
}

double Random::normal(double mean, double sd) {
    std::normal_distribution<double> norm(mean, sd);
    return norm(rng);
}

