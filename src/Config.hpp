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

#ifndef CONFIG_HPP
#define CONFIG_HPP

#include <iostream>
#include <memory>
#include <string>
#include <utility>

class ConfigEntryBase {
public:
    explicit ConfigEntryBase(const std::string &config_name);

    friend auto operator>>(std::istream &is, ConfigEntryBase & /*config_entry*/) -> std::istream &;
    virtual void read(std::istream &is) = 0;
    virtual void write(std::ostream &os) const = 0;

    ConfigEntryBase(const ConfigEntryBase &) = default;
    auto operator=(const ConfigEntryBase &) -> ConfigEntryBase & = default;
    ConfigEntryBase(ConfigEntryBase &&) = default;
    auto operator=(ConfigEntryBase &&) -> ConfigEntryBase & = default;
    virtual ~ConfigEntryBase() = default;

    [[nodiscard]] auto isInitialized() const -> bool { return _initialized; }

protected:
    std::string _config_name;
    bool _initialized = {false};
};

auto operator>>(std::istream &is, ConfigEntryBase &config_entry) -> std::istream &;
auto operator<<(std::ostream &os, const ConfigEntryBase &config_entry) -> std::ostream &;

template <typename T> class ConfigEntry : public ConfigEntryBase {
public:
    explicit ConfigEntry(const std::string &config_name, T default_value = {})
            : ConfigEntryBase{config_name}, _item{std::move(default_value)} {}

    operator T() const { return _item; }

    auto operator()() const -> T { return _item; }

    void read(std::istream &is) override {
        is >> _item;
        _initialized = true;
    }
    void write(std::ostream &os) const override { os << _item; }

private:
    T _item;
};

class Config {
public:
    static auto load(const std::string &config_file) -> bool;
    static void register_entry(const std::string &config_name, ConfigEntryBase *entry);

    static auto instance() -> Config & {
        if (!_instance) {
            _instance = std::make_unique<Config>();
        }

        return *_instance;
    }

    const ConfigEntry<std::string> datadir{"datadir", "./data/"};
    const ConfigEntry<std::string> scriptdir{"scriptdir", "./script/"};

    const ConfigEntry<uint16_t> port{"port", 3012};

    const ConfigEntry<std::string> postgres_db{"postgres_db", "illarion"};
    const ConfigEntry<std::string> postgres_user{"postgres_user", "illarion"};
    const ConfigEntry<std::string> postgres_pwd{"postgres_pwd", "illarion"};
    const ConfigEntry<std::string> postgres_host{"postgres_host", "/var/run/postgresql"};
    const ConfigEntry<uint16_t> postgres_port{"postgres_port", 5432};
    const ConfigEntry<std::string> postgres_schema_server{"postgres_schema_server", "server"};
    const ConfigEntry<std::string> postgres_schema_account{"postgres_schema_account", "accounts"};

    const ConfigEntry<int16_t> debug{"debug", 0};

    const ConfigEntry<uint16_t> clientversion{"clientversion", 122};
    const ConfigEntry<int16_t> playerstart_x{"playerstart_x", 0};
    const ConfigEntry<int16_t> playerstart_y{"playerstart_y", 0};
    const ConfigEntry<int16_t> playerstart_z{"playerstart_z", 0};

private:
    static std::unique_ptr<Config> _instance;
};

#endif
