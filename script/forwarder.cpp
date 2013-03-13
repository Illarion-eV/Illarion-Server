#include "script/forwarder.hpp"
#include "globals.hpp"
#include "World.hpp"

std::shared_ptr<script_data_exchangemap> convert_to_map(const luabind::object& data) {
	auto mapType = luabind::type(data);
	if (mapType == LUA_TNIL)
		return {};

	if (mapType != LUA_TTABLE)
		throw std::logic_error("Usage of invalid data map type. Data maps must be tables or nil.");

	luabind::iterator end;

	auto result = std::make_shared<script_data_exchangemap>();
	for (auto it = luabind::iterator(data); it != end; ++it) {
		std::string key;

		try {
			key = luabind::object_cast<std::string>(it.key());
		} catch (luabind::cast_failed &e) {
			throw std::logic_error("Usage of invalid data map key. Data map keys must be strings.");
		}

		std::string value;

		try {
			value = luabind::object_cast<std::string>(*it);
		} catch (luabind::cast_failed &e) {
			try {
				int32_t intValue = luabind::object_cast<int32_t>(*it);
				std::stringstream ss;
				ss << intValue;
				value = ss.str();
			} catch (luabind::cast_failed &e) {
				throw std::logic_error("Usage of invalid data map value. Data map values must be numbers or strings.");
			}
		}

		result->push_back(std::make_pair(key, value));
	}

	return result;
}

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

int count_item_at1(const Character* character, const std::string& where, TYPE_OF_ITEM_ID id) {
	return character->countItemAt(where, id);
}

int count_item_at2(const Character* character, const std::string& where, TYPE_OF_ITEM_ID id, const luabind::object& data) {
	return character->countItemAt(where, id, convert_to_map(data).get());
}


int erase_item1(Character* character, TYPE_OF_ITEM_ID id, int count) {
	return character->eraseItem(id, count);
}

int erase_item2(Character* character, TYPE_OF_ITEM_ID id, int count, const luabind::object& data) {
	return character->eraseItem(id, count, convert_to_map(data).get());
}

int create_item(Character* character, Item::id_type id, Item::number_type number, Item::quality_type quality, const luabind::object& data) {
	return character->createItem(id, number, quality, convert_to_map(data).get());
}

ScriptItem world_createFromId(World* world, TYPE_OF_ITEM_ID id, unsigned short int count, position pos, bool always, int quali, const luabind::object& data) {
	return world->createFromId(id, count, pos, always, quali, convert_to_map(data).get());
}

