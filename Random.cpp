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


#include "Random.hpp"
#include <boost/random/uniform_01.hpp>
#include <boost/random/uniform_int.hpp>
#include <boost/random/normal_distribution.hpp>
#include <boost/random/variate_generator.hpp>

boost::mt19937 Random::rng;

double Random::uniform() {
    boost::uniform_01<double> uniform;
    return uniform(rng);
}

int Random::uniform(int min, int max) {
    boost::uniform_int<int> uniform(min, max);
    return uniform(rng);
}

double Random::normal(double mean, double sd) {
    boost::normal_distribution<double> norm(mean, sd);
    boost::variate_generator<boost::mt19937 &, boost::normal_distribution<double> > gen(rng, norm);
    return gen();
}

