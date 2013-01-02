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


#include "IdCounter.hpp"

IdCounter::IdCounter(std::string counterf): counterfile(counterf) {
    std::ifstream cfile(counterfile.c_str(), std::ios::binary | std::ios::in);

    // Destruktor schliesst Datei
    if (!cfile.good()) {
#ifdef IdCounter_DEBUG
        std::cout << "counterfile " << counterfile << " nicht gefunden, setze counter auf 0\n";
#endif
        counter = 0;
    } else {
        std::cout << "counterfile " << counterfile << " gefunden\n";

        // Counter einlesen
        cfile.read((char *) & counter, sizeof(counter));
    }
}

IdCounter::IdCounter(const std::string &counterf, const TYPE_OF_CHARACTER_ID &startid): counterfile(counterf) {
    set(startid);
}



IdCounter::~IdCounter() {}


TYPE_OF_CHARACTER_ID IdCounter::nextFreeId() {
    std::ofstream cfile(counterfile.c_str(), std::ios::binary |
                        std::ios::out |
                        std::ios::trunc);

    // Destruktor schliesst Datei
    if (!cfile.good()) {
        std::cerr << "konnte " << counterfile.c_str() << " nicht oeffnen\n";
        return 0;
    } else {
        counter++;
        // Daten schreiben:
        cfile.write((char *) & counter, sizeof(counter));
        return counter;
    }
}

bool IdCounter::set(TYPE_OF_CHARACTER_ID lastid) {
    std::ofstream cfile(counterfile.c_str(), std::ios::binary |
                        std::ios::out |
                        std::ios::trunc);

    // Destruktor schliesst Datei
    if (!cfile.good()) {
        std::cerr << "konnte " << counterfile.c_str() << " nicht oeffnen\n";
        return false;
    } else {
        counter = lastid;
        // Daten schreiben:
        cfile.write((char *) & counter, sizeof(counter));
        return true;
    }
}



bool IdCounter::reset() {
    return set(0);
}
