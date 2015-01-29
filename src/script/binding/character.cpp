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

#include "Character.hpp"
#include "Player.hpp"
#include "globals.hpp"
#include <luabind/adopt_policy.hpp>
#include <luabind/out_value_policy.hpp>
#include <script/forwarder.hpp>
#include <data/Data.hpp>
#include "script/binding/binding.hpp"

namespace binding {

    luabind::scope character() {
        using ::position;

        luabind::value_vector skills;

        for (const auto &skill: Data::Skills) {
            skills.push_back(luabind::value(skill.second.serverName.c_str(), skill.first));
        }

        luabind::value_vector races;

        for (const auto &race: Data::Races) {
            races.push_back(luabind::value(race.second.serverName.c_str(), race.first));
        }

        return luabind::class_<Character>("Character")
                .def("isNewPlayer", &Character::isNewPlayer)
                .def("pageGM", &Character::pageGM)
                .def("requestInputDialog", &Character::requestInputDialog, luabind::adopt(_2))
                .def("requestMessageDialog", &Character::requestMessageDialog, luabind::adopt(_2))
                .def("requestMerchantDialog", &Character::requestMerchantDialog, luabind::adopt(_2))
                .def("requestSelectionDialog", &Character::requestSelectionDialog, luabind::adopt(_2))
                .def("requestCraftingDialog", &Character::requestCraftingDialog, luabind::adopt(_2))
                .def("idleTime", &Character::idleTime)
                .def("sendBook", &Character::sendBook)
                .def("updateAppearance", &Character::forceUpdateAppearanceForAll)
                .def("performAnimation", &Character::performAnimation)
                .def("alterSpokenMessage", &Character::alterSpokenMessage)
                .def("actionRunning", &Character::actionRunning)
                .def("changeQualityAt", &Character::changeQualityAt)
                .def("isAdmin", &Character::isAdmin)
                .def("talk", (void(Character:: *)(Character::talk_type, const std::string &))&Character::talk)
                .def("talk", (void(Character:: *)(Character::talk_type, const std::string &, const std::string &))&Character::talk)
                .def("sendCharDescription", &Character::sendCharDescription)
                .def("startAction", &Character::startAction)
                .def("abortAction", &Character::abortAction)
                .def("successAction", &Character::successAction)
                .def("disturbAction", &Character::actionDisturbed)
                .def("changeSource", (void(Character:: *)(Character *))&Character::changeSource)
                .def("changeSource", (void(Character:: *)(const ScriptItem &))&Character::changeSource)
                .def("changeSource", (void(Character:: *)(const position &))&Character::changeSource)
                .def("changeSource", (void(Character:: *)(void))&Character::changeSource)
                .def("changeTarget", (void(Character:: *)(Character *))&Character::changeTarget)
                .def("changeTarget", (void(Character:: *)(const ScriptItem &))&Character::changeTarget)
                .def("changeTarget", (void(Character:: *)(const position &))&Character::changeTarget)
                .def("changeTarget", (void(Character:: *)(void))&Character::changeTarget)
                .def("inform", inform_lua1)
                .def("inform", inform_lua2)
                .def("inform", inform_lua3)
                .def("inform", inform_lua4)
                .def("introduce", &Character::introducePlayer)
                .def("move", &Character::move)
                .def("turn", (void(Character:: *)(direction))&Character::turn)
                .def("turn", (void(Character:: *)(const position &))&Character::turn)
                .def("getNextStepDir", &Character::getNextStepDir, luabind::pure_out_value(_3))
                .def("setRace", &Character::changeRace)
                .def("getRace", &Character::getRace)
                .def("getFaceTo", &Character::getFaceTo)
                .def("getType", &Character::getType)
                .def("createItem", create_item)
                .def("getLoot", getLoot)
                .def("increasePoisonValue", &Character::increasePoisonValue)
                .def("getPoisonValue", &Character::getPoisonValue)
                .def("setPoisonValue", &Character::setPoisonValue)
                .def("getMentalCapacity", &Character::getMentalCapacity)
                .def("setMentalCapacity", &Character::setMentalCapacity)
                .def("increaseMentalCapacity", &Character::increaseMentalCapacity)
                .def("setClippingActive", &Character::setClippingActive)
                .def("getClippingActive", &Character::getClippingActive)
                .def("countItem", &Character::countItem)
                .def("countItemAt", count_item_at1)
                .def("countItemAt", count_item_at2)
                .def("eraseItem", erase_item1)
                .def("eraseItem", erase_item2)
                .def("increaseAtPos", &Character::increaseAtPos)
                .def("swapAtPos", &Character::swapAtPos)
                .def("createAtPos", &Character::createAtPos)
                .def("getItemAt", &Character::GetItemAt)
                .enum_("skills")
                [
                    skills
                ]
                .def("getSkillName", &Character::getSkillName)
                .def("getSkill", &Character::getSkill)
                .def("getMinorSkill", &Character::getMinorSkill)
                .def("increaseAttrib", &Character::increaseAttrib)
                .def("setAttrib", &Character::setAttrib)
                .def("isBaseAttributeValid", &Character::isBaseAttribValid)
                .def("getBaseAttributeSum", &Character::getBaseAttributeSum)
                .def("getMaxAttributePoints", &Character::getMaxAttributePoints)
                .def("saveBaseAttributes", &Character::saveBaseAttributes)
                .def("setBaseAttribute", &Character::setBaseAttrib)
                .def("getBaseAttribute", &Character::getBaseAttrib)
                .def("increaseBaseAttribute", &Character::increaseBaseAttrib)
                .def("increaseSkill", &Character::increaseSkill)
                .def("increaseMinorSkill", &Character::increaseMinorSkill)
                .def("setSkill", &Character::setSkill)
                .def("setSkinColour", &Character::setSkinColour)
                .def("getSkinColour", &Character::getSkinColour)
                .def("setHairColour", &Character::setHairColour)
                .def("getHairColour", &Character::getHairColour)
                .def("setHair", &Character::setHair)
                .def("getHair", &Character::getHair)
                .def("setBeard", &Character::setBeard)
                .def("getBeard", &Character::getBeard)
                .def("learn", &Character::learn)
                .def("getSkillValue",&Character::getSkillValue)
                .def("teachMagic", &Character::teachMagic)
                .def("isInRange", &Character::isInRange)
                .def("isInRangeToPosition", &Character::isInRangeToField)
                .def("distanceMetric", &Character::distanceMetric)
                .def("distanceMetricToPosition", &Character::distanceMetricToPosition)
                .def("getMagicType", &Character::getMagicType)
                .def("setMagicType", &Character::setMagicType)
                .def("getMagicFlags", &Character::getMagicFlags)
                .def("warp", &Character::Warp)
                .def("forceWarp", &Character::forceWarp)
                .def("startMusic", &Character::startMusic)
                .def("defaultMusic", &Character::defaultMusic)
                .def("callAttackScript", &Character::callAttackScript)
                .def("getItemList", character_getItemList)
                .property("lastSpokenText", &Character::getLastSpokenText)
                .def("getPlayerLanguage", getPlayerLanguageLua)
                .def("getBackPack", &Character::GetBackPack)
                .def("getDepot", &Character::GetDepot)
                .def("setQuestProgress", &Character::setQuestProgress)
                .def("getQuestProgress", &Character::getQuestProgress, luabind::pure_out_value(_3))
                .def("getOnRoute",&Character::getOnRoute)
                .def("setOnRoute",&Character::setOnRoute)
                .def("getMonsterType", &Character::getMonsterType)
                .def("logAdmin", &Character::logAdmin)
                .def_readonly("effects", &Character::effects)
                .def_readonly("waypoints", &Character::waypoints)
                .property("pos", &Character::getPosition)
                .property("name", &Character::getName)
                .property("id", &Character::getId)
                .property("activeLanguage", &Character::getActiveLanguage, &Character::setActiveLanguage)
                .property("movepoints", &Character::getActionPoints, &Character::setActionPoints)
                .property("fightpoints", &Character::getFightPoints, &Character::setFightPoints)
                .property("isinvisible", &Character::isInvisible, &Character::setInvisible)
                .property("attackmode", &Character::getAttackMode)
                .def("stopAttack", &Character::stopAttack)
                .def("getAttackTarget", &Character::getAttackTarget)
                //.def_readonly("isTarget", &Character::isTarget)
                .enum_("body_pos")
                [
                    luabind::value("backpack", BACKPACK),
                    luabind::value("head", HEAD),
                    luabind::value("neck", NECK),
                    luabind::value("breast", BREAST),
                    luabind::value("hands", HANDS),
                    luabind::value("left_tool", LEFT_TOOL),
                    luabind::value("right_tool", RIGHT_TOOL),
                    luabind::value("finger_left_hand", FINGER_LEFT_HAND),
                    luabind::value("finger_right_hand", FINGER_RIGHT_HAND),
                    luabind::value("legs", LEGS),
                    luabind::value("feet", FEET),
                    luabind::value("coat", COAT),
                    luabind::value("belt_pos_1", MAX_BODY_ITEMS),
                    luabind::value("belt_pos_2", MAX_BODY_ITEMS + 1),
                    luabind::value("belt_pos_3", MAX_BODY_ITEMS + 2),
                    luabind::value("belt_pos_4", MAX_BODY_ITEMS + 3),
                    luabind::value("belt_pos_5", MAX_BODY_ITEMS + 4),
                    luabind::value("belt_pos_6", MAX_BODY_ITEMS + 5)
                ]
                .enum_("magic_flags")
                [
                    luabind::value("mage", MAGE),
                    luabind::value("priest", PRIEST),
                    luabind::value("bard", BARD),
                    luabind::value("druid", DRUID)
                ]
                .enum_("talk_type")
                [
                    luabind::value("say", Character::tt_say),
                    luabind::value("whisper", Character::tt_whisper),
                    luabind::value("yell", Character::tt_yell)
                ]
                .enum_("direction")
                [
                    luabind::value("dir_north", dir_north),
                    luabind::value("dir_northeast", dir_northeast),
                    luabind::value("dir_east", dir_east),
                    luabind::value("dir_southeast", dir_southeast),
                    luabind::value("dir_south", dir_south),
                    luabind::value("dir_southwest", dir_southwest),
                    luabind::value("dir_west", dir_west),
                    luabind::value("dir_northwest", dir_northwest),
                    luabind::value("dir_up", dir_up),
                    luabind::value("dir_down", dir_down)
                ]
                .enum_("character_type")
                [
                    luabind::value("player", Character::player),
                    luabind::value("monster", Character::monster),
                    luabind::value("npc", Character::npc)
                ]
                .enum_("sex_type")
                [
                    luabind::value("male", Character::male),
                    luabind::value("female", Character::female)
                ]
                .enum_("face_to")
                [
                    luabind::value("north", Character::north),
                    luabind::value("northeast", Character::northeast),
                    luabind::value("east", Character::east),
                    luabind::value("southeast", Character::southeast),
                    luabind::value("south", Character::south),
                    luabind::value("southwest", Character::southwest),
                    luabind::value("west", Character::west),
                    luabind::value("northwest", Character::northwest)
                ]
                .enum_("race_type")
                [
                    races
                ]
                .enum_("inform_type")
                [
                    luabind::value("lowPriority", Character::informScriptLowPriority),
                    luabind::value("mediumPriority", Character::informScriptMediumPriority),
                    luabind::value("highPriority", Character::informScriptHighPriority)
                ];
    }

}
