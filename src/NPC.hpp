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


#ifndef NPC_HPP
#define NPC_HPP

#include "Character.hpp"
#include <memory>

class LuaNPCScript;

/**
* struct to take a text
*/
struct NPCTalk {
    /**
    * german version of the spoken text
    */
    std::string SpeechText_ger;

    /**
    * englisch version of the spoken text
    */
    std::string SpeechText_eng;
};

class World;

/**
* defines a npc
*/
class NPC : public Character {

public:

    /**
    * constructor which creates the npc
    * @param id of the npc
    * @param name of the npc
    * @param type race of the npc
    * @param pos position on the map of the npc
    * @param dir in which direction the npc is watching
    * @param ishealer if true the npc ressurects death players
    * @param sex the sex of the npc
    */
    NPC(TYPE_OF_CHARACTER_ID id, const std::string &name, TYPE_OF_RACE_ID race, const position &pos, Character::face_to dir, bool ishealer, Character::sex_type sex,
        const appearance &appearance);

    /**
    * the destructor
    */
    ~NPC() override;

    auto getType() const -> unsigned short override {
        return npc;
    }

    /**
    * gets the healer state of this npc
    * @return if the npc is a healer or not
    */
    auto getHealer() const -> bool {
        return _ishealer;
    }

    /**
    * gets the initial position of the npc
    * @return the starting position of this npc
    */
    auto getStartPos() const -> position {
        return _startpos;
    }

    /**
    * gets a pointer to the script for this npc
    * @return the pointer for the script
    */
    auto getScript() const -> std::shared_ptr<LuaNPCScript> {
        return _script;
    }

    /**
    * adds a script to this npac
    * @param script a pointer to a lua script for this npc
    */
    void setScript(std::shared_ptr<LuaNPCScript> script) {
        _script = script;
    }

    /**
    * npc receives a spoken text from a character
    * triggeres a script entry
    * @param tt how loudly the text is spoken
    * @param message the text which is spoken
    * @param cc the character who has spokenthe text
    */
    void receiveText(talk_type tt, const std::string &message, Character *cc) override;

    auto to_string() const -> std::string override;

protected:

    static uint32_t counter;
    bool _ishealer;
    position _startpos;
    std::shared_ptr<LuaNPCScript> _script;
};

#endif // NPC_HPP
