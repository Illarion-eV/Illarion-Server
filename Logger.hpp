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


#ifndef LOGGER_HPP
#define LOGGER_HPP

#include <sstream>
#include <syslog.h>

#define LOGLEVEL_ERROR   3
#define LOGLEVEL_WARNING 4
#define LOGLEVEL_NOTICE  5
#define LOGLEVEL_INFO    6
#define LOGLEVEL_DEBUG   7

#ifndef MIN_LOGLEVEL
#define MIN_LOGLEVEL     6
#endif

enum class LogFacility {
    Database = LOG_LOCAL1,
    World = LOG_LOCAL2,
    Script = LOG_LOCAL3,
    Player = LOG_LOCAL4,
    Chat = LOG_LOCAL5,
    Admin = LOG_LOCAL6,
    Other = LOG_LOCAL7
};

enum class LogPriority {
    EMERGENCY = LOG_EMERG,
    ALERT = LOG_ALERT,
    CRITICAL = LOG_CRIT,
    ERROR = LOG_ERR,
    WARNING = LOG_WARNING,
    NOTICE = LOG_NOTICE,
    INFO = LOG_INFO,
    DEBUG = LOG_DEBUG
};

void log_message(LogPriority priority, LogFacility facility, const std::string &message);

namespace Log {
class end_t {
};
static end_t end __attribute__((unused));
}

class NullStream {
public:
    inline constexpr NullStream() { }
    inline NullStream &operator()(LogFacility facility) {
        return *this;
    }

    template<typename T>
    inline NullStream &operator<<(const T &) {
        return *this;
    }
};

template<LogPriority priority>
class LogStream {
public:
    inline LogStream &operator()(LogFacility facility) {
        _facility = facility;
        return *this;
    }

    inline LogStream() { }
    template<typename T>
    inline LogStream &operator<<(const T &data) {
        static_assert(!std::is_pointer<T>::value || std::is_same<T, const char *>::value || std::is_same<T, char *>::value, "Logger cannot log pointers!");
        _ss << data;
        return *this;
    }

    LogStream &operator<<(const Log::end_t &) {
        log_message(priority, _facility, _ss.str());
        _ss.str( {});
        return *this;
    }

private:
    std::stringstream _ss;
    LogFacility _facility;
};

template<LogPriority priority>
class LogType {
public:
    typedef LogStream<priority> type;
};

#define DEACTIVATE_LOG(priority) \
template<> \
class LogType<priority> { \
    public: \
        typedef NullStream type; \
};

#if MIN_LOGLEVEL < LOGLEVEL_DEBUG
DEACTIVATE_LOG(LogPriority::DEBUG)
#endif
#if MIN_LOGLEVEL < LOGLEVEL_INFO
DEACTIVATE_LOG(LogPriority::INFO)
#endif
#if MIN_LOGLEVEL < LOGLEVEL_NOTICE
DEACTIVATE_LOG(LogPriority::NOTICE)
#endif

#undef DEACTIVATE_LOG

class Logger {
public:
    static LogType<LogPriority::EMERGENCY>::type emergency;
    static LogType<LogPriority::ALERT>::type alert;
    static LogType<LogPriority::CRITICAL>::type critical;
    static LogType<LogPriority::ERROR>::type error;
    static LogType<LogPriority::WARNING>::type warn;
    static LogType<LogPriority::NOTICE>::type notice;
    static LogType<LogPriority::INFO>::type info;
    static LogType<LogPriority::DEBUG>::type debug;
};

#endif

