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

#ifndef LOGGER_HPP
#define LOGGER_HPP

#include <sstream>
#include <string>
#include <syslog.h>
#include <type_traits>

/**
 * @brief Logging facility categories for organizing log messages.
 *
 * Each facility corresponds to a syslog LOCAL facility for filtering and routing.
 */
enum class LogFacility {
    Database = LOG_LOCAL1, ///< Database-related operations.
    World = LOG_LOCAL2,    ///< World and game mechanics.
    Script = LOG_LOCAL3,   ///< Script execution and errors.
    Player = LOG_LOCAL4,   ///< Player actions and events.
    Chat = LOG_LOCAL5,     ///< Chat and communication.
    Admin = LOG_LOCAL6,    ///< Administrative actions.
    Other = LOG_LOCAL7     ///< Miscellaneous messages.
};

/**
 * @brief Logging priority levels from emergency to debug.
 *
 * These correspond to standard syslog priority levels.
 */
enum class LogPriority {
    EMERGENCY = LOG_EMERG, ///< System is unusable.
    ALERT = LOG_ALERT,     ///< Action must be taken immediately.
    CRITICAL = LOG_CRIT,   ///< Critical conditions.
    ERROR = LOG_ERR,       ///< Error conditions.
    WARNING = LOG_WARNING, ///< Warning conditions.
    NOTICE = LOG_NOTICE,   ///< Normal but significant condition.
    INFO = LOG_INFO,       ///< Informational messages.
    DEBUG = LOG_DEBUG      ///< Debug-level messages.
};

/**
 * @brief Compile-time check if a log priority is enabled.
 * @param priority The priority level to check.
 * @return true if the priority should be logged, false if it should be optimized out.
 * @note DEBUG level is currently disabled at compile time.
 */
constexpr auto isLogEnabled(LogPriority priority) -> bool { return priority != LogPriority::DEBUG; }

/**
 * @brief Sends a log message to syslog.
 * @param priority The priority level of the message.
 * @param facility The facility category for the message.
 * @param message The message text to log.
 */
void log_message(LogPriority priority, LogFacility facility, const std::string &message);

/**
 * @brief Logging utilities and stream manipulators.
 */
namespace Log {
/**
 * @brief Stream manipulator to end and flush a log message.
 */
class end_t {};
static const end_t end __attribute__((unused)); ///< Log stream terminator.
} // namespace Log

/**
 * @brief No-op stream that discards all input for disabled log levels.
 *
 * This is used for compile-time optimization of disabled log levels,
 * ensuring zero runtime overhead for disabled logging.
 */
class NullStream {
public:
    /**
     * @brief Default constructor.
     */
    inline constexpr NullStream() = default;

    /**
     * @brief No-op facility setter.
     * @param facility The facility (ignored).
     * @return Reference to self for chaining.
     */
    inline auto operator()(LogFacility facility) const -> const NullStream & { return *this; }

    /**
     * @brief No-op insertion operator that discards all data.
     * @tparam T Any type (discarded).
     * @param unused The value to discard.
     * @return Reference to self for chaining.
     */
    template <typename T> inline auto operator<<(const T & /*unused*/) const -> const NullStream & { return *this; }
};

/**
 * @brief Active log stream that accumulates messages and sends them to syslog.
 * @tparam priority The priority level for this stream.
 */
template <LogPriority priority> class LogStream {
public:
    /**
     * @brief Sets the logging facility for this message.
     * @param facility The facility category.
     * @return Reference to self for chaining.
     */
    inline auto operator()(LogFacility facility) -> LogStream & {
        _facility = facility;
        return *this;
    }

    /**
     * @brief Default constructor.
     */
    inline LogStream() = default;

    /**
     * @brief Insertion operator to append data to the log message.
     * @tparam T The type of data to append.
     * @param data The data to append.
     * @return Reference to self for chaining.
     * @note Pointers (except char*) are not allowed for safety.
     */
    template <typename T> inline auto operator<<(const T &data) -> LogStream & {
        static_assert(!std::is_pointer<T>::value || std::is_same<T, const char *>::value ||
                              std::is_same<T, char *>::value,
                      "Logger cannot log pointers!");
        _ss << data;
        return *this;
    }

    /**
     * @brief Logs and flushes the accumulated message.
     * @param unused The Log::end manipulator.
     * @return Reference to self for chaining.
     */
    auto operator<<(const Log::end_t & /*unused*/) -> LogStream & {
        log_message(priority, _facility, _ss.str());
        _ss.str({});
        return *this;
    }

private:
    std::stringstream _ss;                      ///< Accumulator for the log message.
    LogFacility _facility = LogFacility::Other; ///< The facility for this message.
};

/**
 * @brief Type selector that chooses between active LogStream and NullStream.
 * @tparam priority The priority level to check.
 *
 * At compile time, selects NullStream for disabled priorities (zero overhead)
 * or LogStream for enabled priorities.
 */
template <LogPriority priority> class LogType {
public:
    using type = std::conditional_t<isLogEnabled(priority), LogStream<priority>, NullStream>; ///< The selected type.
};

/**
 * @brief Static logging interface with priority-specific streams.
 *
 * Usage:
 * @code
 * Logger::error(LogFacility::Database) << "Connection failed: " << error << Log::end;
 * Logger::info(LogFacility::Player) << "Player " << name << " logged in" << Log::end;
 * @endcode
 */
class Logger {
public:
    static LogType<LogPriority::EMERGENCY>::type emergency; ///< Emergency level logging.
    static LogType<LogPriority::ALERT>::type alert;         ///< Alert level logging.
    static LogType<LogPriority::CRITICAL>::type critical;   ///< Critical level logging.
    static LogType<LogPriority::ERROR>::type error;         ///< Error level logging.
    static LogType<LogPriority::WARNING>::type warn;        ///< Warning level logging.
    static LogType<LogPriority::NOTICE>::type notice;       ///< Notice level logging.
    static LogType<LogPriority::INFO>::type info;           ///< Info level logging.
    static LogType<LogPriority::DEBUG>::type debug;         ///< Debug level logging (disabled).
};

#endif
