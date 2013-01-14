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


#ifndef NEWCLIENTVIEW_HPP_
#define NEWCLIENTVIEW_HPP_

#define MAP_DIMENSION 17 // map extends into all 4 directions for this number of tiles
#define MAP_DOWN_EXTRA 3 // extra downwards extension

#include "globals.hpp"
#include "constants.hpp"
#include "WorldMap.hpp"

// forward declarations
class Map;
class Field;

/**
* class which holds isometric view specific data
*/
class NewClientView {

public:

    /**
    * in which direction is the stripe
    */
    enum stripedirection {
        dir_right,
        dir_down
    };

    /**
    * defines one mapstripe
    */
    typedef Field *MAPSTRIPE[ 100 /*MAP_DIMENSION + 1 + MAP_DOWN_EXTRA + 6*/ ];

    /**
    * stores the pointers to the fields inside a specific mapstripe
    */
    MAPSTRIPE mapStripe;

    /**
    * returns the initial position of this stripe
    * @return the starting position of the stripe
    */
    position getViewPosition() {
        return viewPosition;
    }

    /**
    * returns if the stripe exists
    * @return true if the stripe exists otherwise false
    */
    bool getExists() {
        return exists;
    }

    /**
    * returns the number of tiles in the view
    * @return the number of maximal tiles in the view
    */
    uint8_t getMaxTiles() {
        return maxtiles;
    }

    /**
    * reduces the tiles which should be sended (for smaller areas )
    * @param number the number which should be reduced
    */
    inline void removeLastTiles(uint8_t number) {
        number -= (MAP_DIMENSION + 2 - maxtiles) < number ? (MAP_DIMENSION + 2 - maxtiles) : number;

        if (number > 0) {
            if (maxtiles <= number) {
                maxtiles = 0;
                exists = false;
            } else {
                maxtiles -= number;
            }
        }
    }

    /**
    * the stripedirection, in which direction the mapstripe shows
    * @return the current direction of the mapstripe
    */
    stripedirection getStripeDirection() {
        return stripedir;
    }

    /**
    * constructor
    */
    NewClientView();

    /**
    * destructor
    */
    ~NewClientView();

    /**
    * fills the stripe with the specific isometric data
    * @param pos the starting position of the stripe
    * @param dir the direction in which the stipe looks
        * @param length number of tiles to be read
    * @param maps the maps from which we want to calculate the stripes
    */
    void fillStripe(position pos, stripedirection dir, int length, const WorldMap &maps);

    /**
    * clears all current stripe infos
    */
    void clearStripe();

private:

    /**
    * reads all fields for the current stripe on a specific map from startingpos towards direction stripedir
        * @param length number of tiles to be read
    * @param maps the map vector from which we want to read the fields
    */
    void readFields(int length, const WorldMap &maps);

    /**
    * the starting position of the current view
    */
    position viewPosition;

    /**
    * if there is a view stripe
    */
    bool exists;

    /**
    * which direction do we read
    */
    stripedirection stripedir;

    /**
    * how many tiles are stored
    */
    uint8_t maxtiles;

    /**
    * pointer to a temporary field
    */
    Field *tempCField;

};

#endif
