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

/**
 * @brief Base class for configuration entries that can be loaded from config files.
 *
 * This abstract base class provides the interface for reading and writing configuration
 * values from/to streams. Configuration entries are registered globally and populated
 * when a config file is loaded.
 */
class ConfigEntryBase {
public:
    /**
     * @brief Constructs a config entry with a given name.
     * @param config_name The name of the configuration key in the config file.
     */
    explicit ConfigEntryBase(const std::string &config_name);

    friend auto operator>>(std::istream &is, ConfigEntryBase & /*config_entry*/) -> std::istream &;

    /**
     * @brief Reads the configuration value from an input stream.
     * @param is The input stream to read from.
     */
    virtual void read(std::istream &is) = 0;

    /**
     * @brief Writes the configuration value to an output stream.
     * @param os The output stream to write to.
     */
    virtual void write(std::ostream &os) const = 0;

    ConfigEntryBase(const ConfigEntryBase &) = default;
    auto operator=(const ConfigEntryBase &) -> ConfigEntryBase & = default;
    ConfigEntryBase(ConfigEntryBase &&) = default;
    auto operator=(ConfigEntryBase &&) -> ConfigEntryBase & = default;
    virtual ~ConfigEntryBase() = default;

    /**
     * @brief Checks if this configuration entry has been initialized from a config file.
     * @return true if the entry was loaded from a config file, false if using default value.
     */
    [[nodiscard]] auto isInitialized() const -> bool { return _initialized; }

protected:
    std::string _config_name;    ///< The configuration key name.
    bool _initialized = {false}; ///< Whether this entry was loaded from a config file.
};

/**
 * @brief Stream input operator for reading config entries.
 * @param is The input stream.
 * @param config_entry The config entry to read into.
 * @return The input stream.
 */
auto operator>>(std::istream &is, ConfigEntryBase &config_entry) -> std::istream &;

/**
 * @brief Stream output operator for writing config entries.
 * @param os The output stream.
 * @param config_entry The config entry to write.
 * @return The output stream.
 */
auto operator<<(std::ostream &os, const ConfigEntryBase &config_entry) -> std::ostream &;

/**
 * @brief Templated configuration entry that holds a specific value type.
 * @tparam T The type of value this configuration entry holds.
 *
 * This class provides type-safe configuration storage with default values and
 * automatic conversion to the underlying type.
 */
template <typename T> class ConfigEntry : public ConfigEntryBase {
public:
    /**
     * @brief Constructs a configuration entry with a name and optional default value.
     * @param config_name The configuration key name.
     * @param default_value The default value if not specified in the config file.
     */
    explicit ConfigEntry(const std::string &config_name, T default_value = {})
            : ConfigEntryBase{config_name}, _item{std::move(default_value)} {}

    /**
     * @brief Implicit conversion to the underlying type.
     * @return The configuration value.
     */
    operator T() const { return _item; }

    /**
     * @brief Function call operator to retrieve the value.
     * @return The configuration value.
     */
    auto operator()() const -> T { return _item; }

    /**
     * @brief Reads the value from a stream.
     * @param is The input stream to read from.
     */
    void read(std::istream &is) override {
        is >> _item;
        _initialized = true;
    }

    /**
     * @brief Writes the value to a stream.
     * @param os The output stream to write to.
     */
    void write(std::ostream &os) const override { os << _item; }

private:
    T _item; ///< The stored configuration value.
};

/**
 * @brief Singleton class managing all server configuration settings.
 *
 * This class holds all configuration entries for the server, including paths,
 * database settings, network ports, and game parameters. Configuration is loaded
 * from a file using the load() method.
 */
class Config {
public:
    /**
     * @brief Loads configuration from a file.
     * @param config_file Path to the configuration file.
     * @return true if the file was loaded successfully, false otherwise.
     */
    static auto load(const std::string &config_file) -> bool;

    /**
     * @brief Registers a configuration entry for automatic population.
     * @param config_name The configuration key name.
     * @param entry Pointer to the configuration entry to register.
     */
    static void register_entry(const std::string &config_name, ConfigEntryBase *entry);

    /**
     * @brief Gets the singleton instance of the Config.
     * @return Reference to the Config instance.
     */
    static auto instance() -> Config & {
        if (!_instance) {
            _instance = std::make_unique<Config>();
        }

        return *_instance;
    }

    const ConfigEntry<std::string> datadir{"datadir", "./data/"};       ///< Path to data directory.
    const ConfigEntry<std::string> scriptdir{"scriptdir", "./script/"}; ///< Path to script directory.

    const ConfigEntry<uint16_t> port{"port", 3012}; ///< Server listening port.

    const ConfigEntry<std::string> postgres_db{"postgres_db", "illarion"};                ///< PostgreSQL database name.
    const ConfigEntry<std::string> postgres_user{"postgres_user", "illarion"};            ///< PostgreSQL username.
    const ConfigEntry<std::string> postgres_pwd{"postgres_pwd", "illarion"};              ///< PostgreSQL password.
    const ConfigEntry<std::string> postgres_host{"postgres_host", "/var/run/postgresql"}; ///< PostgreSQL host.
    const ConfigEntry<uint16_t> postgres_port{"postgres_port", 5432};                     ///< PostgreSQL port.
    const ConfigEntry<std::string> postgres_schema_server{"postgres_schema_server", "server"};     ///< Server schema.
    const ConfigEntry<std::string> postgres_schema_account{"postgres_schema_account", "accounts"}; ///< Account schema.

    const ConfigEntry<int16_t> debug{"debug", 0}; ///< Debug level.

    const ConfigEntry<uint16_t> clientversion{"clientversion", 122}; ///< Required client version.
    const ConfigEntry<int16_t> playerstart_x{"playerstart_x", 0};    ///< Default player start X coordinate.
    const ConfigEntry<int16_t> playerstart_y{"playerstart_y", 0};    ///< Default player start Y coordinate.
    const ConfigEntry<int16_t> playerstart_z{"playerstart_z", 0};    ///< Default player start Z coordinate.

private:
    static std::unique_ptr<Config> _instance; ///< Singleton instance.
};

#endif
