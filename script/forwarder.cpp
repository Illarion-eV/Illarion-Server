#include "script/forwarder.hpp"

#include "Character.hpp"

uint32_t getPlayerLanguageLua(const Character* character) {
        return static_cast<uint32_t>(character->getPlayerLanguage());
}
