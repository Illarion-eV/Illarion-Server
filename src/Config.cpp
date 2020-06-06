/*
 *  illarionserver - server for the game Illarion
 *  Copyright 2011 Illarion e.V.
 *
 *  This file is part of illarionserver.
 *
 *  illarionserver is free software: you can redistribute it and/or modify
 *  it under the terms of the GNU Affero General Public License as published by
 *  the Free Software Foundation, either version 3 of the License, or
 *  (at your option) any later version.
 *
 *  illarionserver is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU Affero General Public License for more details.
 *
 *  You should have received a copy of the GNU Affero General Public License
 *  along with illarionserver.  If not, see <http://www.gnu.org/licenses/>.
 */

#include "Config.hpp"

#include "Logger.hpp"

#include <fstream>
#include <map>

std::unique_ptr<Config> Config::_instance = nullptr;

namespace {
std::map<std::string, ConfigEntryBase *> config_options;
}

auto Config::load(const std::string &config_file) -> bool {
    // make sure Config is instantiated
    Config::instance();

    std::ifstream configfile(config_file);

    // can't read config file
    if (!configfile.good()) {
        return false;
    }

    std::string temp;

    // read first token of a line while there are any tokens left...
    while (configfile >> temp && !configfile.eof()) {
        if (temp[0] == '#') {
            // we found a comment... skip line
            configfile.ignore(255, '\n'); // ignore up to 255 chars until \n is found
            continue;
        }

        // store config options in map
        configfile.ignore(1); // ignore the blank char following the token
        auto pos = config_options.find(temp);

        if (pos == config_options.end()) {
            Logger::error(LogFacility::Other) << "Invalid config entry: " << temp << Log::end;
            configfile.ignore(255, '\n');
            continue;
        }

        configfile >> *(pos->second);

        if (!configfile.good()) {
            return false;
        }
    }

    for (const auto &item : config_options) {
        if (!item.second->isInitialized()) {
            std::ostringstream msg;
            msg << "Config entry missing for key: " << item.first << " using default value: " << *(item.second);
            Logger::warn(LogFacility::Other) << msg.str() << Log::end;
        }
    }

    return true;
}

auto operator>>(std::istream &is, ConfigEntryBase &config_entry) -> std::istream & {
    config_entry.read(is);
    return is;
}

auto operator<<(std::ostream &os, const ConfigEntryBase &config_entry) -> std::ostream & {
    config_entry.write(os);
    return os;
}

ConfigEntryBase::ConfigEntryBase(const std::string &config_name) : _config_name{config_name} {
    Config::register_entry(config_name, this);
}

void Config::register_entry(const std::string &config_name, ConfigEntryBase *entry) {
    config_options[config_name] = entry;
}
