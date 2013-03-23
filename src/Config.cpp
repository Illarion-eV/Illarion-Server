/*
 *  illarionserver - server for the game Illarion
 *  Copyright 2011 Illarion e.V.
 *
 *  This file is part of illarionserver.
 *
 *  illarionserver is free software: you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation, either version 3 of the License, or
 *  (at your option) any later version.
 *
 *  illarionserver is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with illarionserver.  If not, see <http://www.gnu.org/licenses/>.
 */

#include "Config.hpp"

#include <iostream>
#include <fstream>

Config *Config::_instance = nullptr;

namespace {
static std::map<std::string, ConfigEntryBase *> config_options;
}

bool Config::load(const std::string &config_file) {
    // make sure Config is instanciated
    Config::instance();

    std::ifstream configfile(config_file.c_str());

    // can't read config file
    if (!configfile.good()) {
        return false;
    }

    std::string temp;

    // read first token of a line while there are any tokens left...
    while (configfile >> temp && ! configfile.eof()) {

        if (temp[0] == '#') {
            // we found a comment... skip line
            configfile.ignore(255,'\n'); // ignore up to 255 chars until \n is found
            continue;
        }

        // store config options in map
        configfile.ignore(1); // ignore the blank char following the token
        auto pos = config_options.find(temp);

        if (pos == config_options.end()) {
            std::cerr << "invalid config entry: " << temp << std::endl;
            return false;
        }

        configfile >> *(pos->second);

        if (!configfile.good()) {
            return false;
        }
    }

    for (auto item : config_options) {
        if (!item.second->isInitialized()) {
            std::cout << "[WARN] config entry missing for key: " << item.first << " using default value: " << *(item.second) << std::endl;
        }
    }

    return true;
}

std::istream &operator>>(std::istream &is, ConfigEntryBase &config_entry) {
    config_entry.read(is);
    return is;
}

std::ostream &operator<<(std::ostream &os, ConfigEntryBase &config_entry) {
    config_entry.write(os);
    return os;
}

ConfigEntryBase::ConfigEntryBase(const std::string &config_name) : _config_name { config_name } {
    Config::register_entry(config_name, this);
}

void Config::register_entry(const std::string &config_name, ConfigEntryBase *entry) {
    config_options[config_name] = entry;
}
