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

#ifndef LUA_SCRIPT_HPP
#define LUA_SCRIPT_HPP

extern "C" {
#include <lua.h>
}

#include "Item.hpp"
#include "Logger.hpp"
#include "character_ptr.hpp"
#include "globals.hpp"

#include <chrono>
#include <luabind/luabind.hpp>
#include <luabind/object.hpp>
#include <map>
#include <stdexcept>

class Character;
class World;
class Dialog;

class ScriptException : public std::runtime_error {
public:
    explicit ScriptException(const std::string &s) : std::runtime_error(s) {}
};

class LuaScript {
public:
    LuaScript();
    explicit LuaScript(std::string filename);
    LuaScript(const std::string &code, const std::string &scriptname);
    LuaScript(const LuaScript &) = delete;
    auto operator=(const LuaScript &) -> LuaScript & = delete;
    LuaScript(LuaScript &&) = default;
    auto operator=(LuaScript &&) -> LuaScript & = default;
    virtual ~LuaScript();

    [[nodiscard]] auto getFileName() const -> std::string { return _filename; }

    static auto getLuaState() -> lua_State * { return _luaState; }

    static void shutdownLua();
    [[nodiscard]] auto existsEntrypoint(const std::string &entrypoint) const -> bool;
    void addQuestScript(const std::string &entrypoint, const std::shared_ptr<LuaScript> &script);

    template <typename T> static void executeDialogCallback(T &dialog) {
        luabind::object callback = dialog.getCallback();

        if (luabind::type(callback) != LUA_TFUNCTION) {
            return;
        }

        try {
            callback(dialog);
        } catch (luabind::error &e) {
            lua_State *L = e.state();
            Logger::error(LogFacility::Script) << "Exception in " << dialog.getClassName() << " callback"
                                               << ": " << std::string(lua_tostring(L, -1)) << Log::end;
            lua_pop(L, 1);
        }
    }

    template <typename U, typename T> static auto executeDialogCallback(T &dialog) -> U {
        luabind::object callback = dialog.getCallback();

        if (luabind::type(callback) != LUA_TFUNCTION) {
            return U();
        }

        try {
            return luabind::object_cast<U>(callback(dialog));
        } catch (luabind::cast_failed &e) {
            const std::string &expectedType = e.info().name();
            Logger::error(LogFacility::Script) << "Invalid return type in " << dialog.getClassName() << " callback: "
                                               << " Expected type " << expectedType << Log::end;
        } catch (luabind::error &e) {
            lua_State *L = e.state();
            Logger::error(LogFacility::Script) << "Exception in " << dialog.getClassName() << " callback"
                                               << ": " << lua_tostring(L, -1) << Log::end;
            lua_pop(L, 1);
        }

        return U();
    }

    [[noreturn]] static void triggerScriptError(const std::string &msg);
    static void writeDeprecatedMsg(const std::string &deprecatedEntity);
    static void writeDebugMsg(const std::string &msg);

protected:
    static lua_State *_luaState;
    static bool initialized;

    template <typename... Args> void callEntrypoint(const std::string &entrypoint, const Args &...args) {
        setCurrentWorldScript();

        if (!callQuestEntrypoint(entrypoint, args...)) {
            safeCall(entrypoint, args...);
        }
    }
    template <typename T, typename... Args>
    auto callEntrypoint(const std::string &entrypoint, const Args &...args) -> T {
        setCurrentWorldScript();
        callQuestEntrypoint(entrypoint, args...);
        return safeCall<T, Args...>(entrypoint, args...);
    }

private:
    static void initialize();
    void loadIntoLuaState();
    void handleLuaLoadError(int errorCode);
    void handleLuaCallError(int errorCode);
    static void init_base_functions();
    static auto add_backtrace(lua_State *L) -> int;
    static void writeErrorMsg();
    void writeCastErrorMsg(const std::string &entryPoint, const luabind::cast_failed &e) const;
    void checkRunTime(const std::string &entryPoint, const std::chrono::nanoseconds duration) const;
    void setCurrentWorldScript();
    auto buildEntrypoint(const std::string &entrypoint) -> luabind::object;
    [[nodiscard]] auto existsQuestEntrypoint(const std::string &entrypoint) const -> bool;

    template <typename... Args> auto callQuestEntrypoint(const std::string &entrypoint, const Args &...args) -> bool {
        auto entrypointRange = questScripts.equal_range(entrypoint);
        bool foundQuest = false;

        for (auto it = entrypointRange.first; it != entrypointRange.second; ++it) {
            foundQuest = foundQuest || it->second->safeCall<bool>(entrypoint, args...);
        }

        return foundQuest;
    }

    template <typename... Args> void safeCall(const std::string &entrypoint, const Args &...args) {
        try {
            using clock = std::chrono::steady_clock;

            auto luaEntrypoint = buildEntrypoint(entrypoint);

            const auto startTime = clock::now();
            luaEntrypoint(args...);
            const auto duration = clock::now() - startTime;

            checkRunTime(entrypoint, duration);
        } catch (const luabind::error &e) {
            writeErrorMsg();
        }
    }
    template <typename T, typename... Args> auto safeCall(const std::string &entrypoint, const Args &...args) -> T {
        try {
            using clock = std::chrono::steady_clock;

            auto luaEntrypoint = buildEntrypoint(entrypoint);

            const auto startTime = clock::now();
            auto result = luaEntrypoint(args...);
            const auto duration = clock::now() - startTime;

            checkRunTime(entrypoint, duration);

            return luabind::object_cast<T>(result);
        } catch (luabind::cast_failed &e) {
            writeCastErrorMsg(entrypoint, e);
        } catch (luabind::error &e) {
            writeErrorMsg();
        }

        return T();
    }

    std::string _filename{};
    std::string luafile{};
    using QuestScripts = std::multimap<const std::string, std::shared_ptr<LuaScript>>;
    QuestScripts questScripts;
};

#endif
