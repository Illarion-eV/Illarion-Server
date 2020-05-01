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

#ifndef __CONFIG_HPP__
#define __CONFIG_HPP__

#include <memory>
#include <string>
#include <iostream>

class ConfigEntryBase {
public:
    explicit ConfigEntryBase(const std::string &config_name);

    friend std::istream &operator>>(std::istream &is, ConfigEntryBase &);
    virtual void read(std::istream &is) = 0;
    virtual void write(std::ostream &os) const= 0;

    virtual ~ConfigEntryBase() { }

    bool isInitialized() const {
        return _initialized;
    }
protected:
    std::string _config_name;
    bool _initialized = { false };
};

std::istream &operator>>(std::istream &is, ConfigEntryBase &config_entry);
std::ostream &operator<<(std::ostream &os, const ConfigEntryBase &config_entry);

template<typename T>
class ConfigEntry : public ConfigEntryBase {
public:

    ConfigEntry(const std::string &config_name, const T &default_value = {}) : ConfigEntryBase { config_name }, _item { default_value } {}

    operator T() const {
        return _item;
    }

    T operator()() const {
        return _item;
    }

    virtual void read(std::istream &is) override {
        is >> _item;
        _initialized = true;
    }
    virtual void write(std::ostream &os) const override {
        os << _item;
    }

private:
    T _item;
};

class Config {
public:
    static bool load(const std::string &config_file);
    static void register_entry(const std::string &config_name, ConfigEntryBase *entry);

    static Config &instance() {
        if (!_instance) {
            _instance = std::make_unique<Config>();
        }

        return *_instance;
    }

    ConfigEntry<std::string> datadir = { "datadir", "./data/" };
    ConfigEntry<std::string> scriptdir = { "scriptdir", "./script/" };

    ConfigEntry<uint16_t> port = { "port", 3012 };

    ConfigEntry<std::string> postgres_db = { "postgres_db", "illarion" };
    ConfigEntry<std::string> postgres_user = { "postgres_user", "illarion" };
    ConfigEntry<std::string> postgres_pwd = { "postgres_pwd", "illarion" };
    ConfigEntry<std::string> postgres_host = { "postgres_host", "/var/run/postgresql" };
    ConfigEntry<uint16_t> postgres_port = { "postgres_port", 5432 };
    ConfigEntry<std::string> postgres_schema_server = { "postgres_schema_server", "server" };
    ConfigEntry<std::string> postgres_schema_account = { "postgres_schema_account", "accounts" };

    ConfigEntry<int16_t> debug = { "debug", 0 };

    ConfigEntry<uint16_t> clientversion = { "clientversion", 122 };
    ConfigEntry<int16_t> playerstart_x = { "playerstart_x", 0 };
    ConfigEntry<int16_t> playerstart_y = { "playerstart_y", 0 };
    ConfigEntry<int16_t> playerstart_z = { "playerstart_z", 0 };

private:
    static std::unique_ptr<Config> _instance;
};

#endif
