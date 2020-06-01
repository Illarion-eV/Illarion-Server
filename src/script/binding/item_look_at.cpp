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

#include "ItemLookAt.hpp"
#include "script/binding/binding.hpp"

namespace binding {

auto item_look_at() -> luabind::scope {
    return luabind::class_<ItemLookAt>("ItemLookAt")
            .def(luabind::constructor<>())
            .enum_("Rareness")[luabind::value("commonItem", ItemLookAt::commonItem),
                               luabind::value("uncommonItem", ItemLookAt::uncommonItem),
                               luabind::value("rareItem", ItemLookAt::rareItem),
                               luabind::value("epicItem", ItemLookAt::epicItem)]
            .property("name", &ItemLookAt::getName, &ItemLookAt::setName)
            .property("rareness", &ItemLookAt::getRareness, &ItemLookAt::setRareness)
            .property("description", &ItemLookAt::getDescription, &ItemLookAt::setDescription)
            .property("craftedBy", &ItemLookAt::getCraftedBy, &ItemLookAt::setCraftedBy)
            .property("type", &ItemLookAt::getType, &ItemLookAt::setType)
            .property("level", &ItemLookAt::getLevel, &ItemLookAt::setLevel)
            .property("usable", &ItemLookAt::isUsable, &ItemLookAt::setUsable)
            .property("weight", &ItemLookAt::getWeight, &ItemLookAt::setWeight)
            .property("worth", &ItemLookAt::getWorth, &ItemLookAt::setWorth)
            .property("qualityText", &ItemLookAt::getQualityText, &ItemLookAt::setQualityText)
            .property("durabilityText", &ItemLookAt::getDurabilityText, &ItemLookAt::setDurabilityText)
            .property("durabilityValue", &ItemLookAt::getDurabilityValue, &ItemLookAt::setDurabilityValue)
            .property("diamondLevel", &ItemLookAt::getDiamondLevel, &ItemLookAt::setDiamondLevel)
            .property("emeraldLevel", &ItemLookAt::getEmeraldLevel, &ItemLookAt::setEmeraldLevel)
            .property("rubyLevel", &ItemLookAt::getRubyLevel, &ItemLookAt::setRubyLevel)
            .property("sapphireLevel", &ItemLookAt::getSapphireLevel, &ItemLookAt::setSapphireLevel)
            .property("amethystLevel", &ItemLookAt::getAmethystLevel, &ItemLookAt::setAmethystLevel)
            .property("obsidianLevel", &ItemLookAt::getObsidianLevel, &ItemLookAt::setObsidianLevel)
            .property("topazLevel", &ItemLookAt::getTopazLevel, &ItemLookAt::setTopazLevel)
            .property("bonus", &ItemLookAt::getBonus, &ItemLookAt::setBonus);
}

} // namespace binding