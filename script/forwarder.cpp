#include "script/forwarder.hpp"
#include "Logger.hpp"

uint32_t getPlayerLanguageLua(const Character* character) {
        return static_cast<uint32_t>(character->getPlayerLanguage());
}

void inform_lua2(const Character* character, const std::string& message, Character::informType type) {
	switch (type) {
		case Character::informScriptLowPriority:
		case Character::informScriptMediumPriority:
		case Character::informScriptHighPriority:
			break;
		default:
			type=Character::informScriptMediumPriority;
	}
	character->inform(message, type);
}

void inform_lua4(const Character* character, const std::string& message) {
	character->inform(message, Character::informScriptMediumPriority);
}

void inform_lua1(const Character* character, const std::string& german, const std::string& english, Character::informType type) {
	switch (type) {
		case Character::informScriptLowPriority:
		case Character::informScriptMediumPriority:
		case Character::informScriptHighPriority:
			break;
		default:
			type=Character::informScriptMediumPriority;
	}
	character->inform(german, english, type);
}

void inform_lua3(const Character* character, const std::string& german, const std::string& english) {
	character->inform(german, english, Character::informScriptMediumPriority);
}

void log_lua(const std::string &message) {
    Logger::info(LogFacility::Script) << message << Log::end;
}

