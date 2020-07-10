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

#include "luabind/luabind.hpp"

class ArmorStruct;
class AttackBoni;
class Character;
class Character;
class SkillValue;
class Colour;
class ItemStruct;
class Container;
class CraftingDialog;
namespace map {
class Field;
}
class InputDialog;
class Item;
class ItemLookAt;
class LongTimeAction;
class LongTimeCharacterEffects;
class LongTimeEffect;
class LongTimeEffectStruct;
class MerchantDialog;
class MessageDialog;
class Monster;
class MonsterArmor;
class NPC;
class Player;
class position;
class Random;
class ScriptItem;
class ScriptVariablesTable;
class SelectionDialog;
class TilesStruct;
class WaypointList;
class WeaponStruct;
class WeatherStruct;
class World;

namespace binding {

template <class T, class U = luabind::detail::unspecified>
using Binding = luabind::class_<T, U, luabind::detail::unspecified, luabind::detail::unspecified>;

auto armor_struct() -> Binding<ArmorStruct>;
auto attack_boni() -> Binding<AttackBoni>;
auto character() -> Binding<Character>;
auto character_skillvalue() -> Binding<SkillValue>;
auto colour() -> Binding<Colour>;
auto item_struct() -> Binding<ItemStruct>;
auto container() -> Binding<Container>;
auto crafting_dialog() -> Binding<CraftingDialog>;
auto field() -> Binding<map::Field>;
auto input_dialog() -> Binding<InputDialog>;
auto item() -> Binding<Item>;
auto item_look_at() -> Binding<ItemLookAt>;
auto long_time_action() -> Binding<LongTimeAction>;
auto long_time_character_effects() -> Binding<LongTimeCharacterEffects>;
auto long_time_effect() -> Binding<LongTimeEffect>;
auto long_time_effect_struct() -> Binding<LongTimeEffectStruct>;
auto merchant_dialog() -> Binding<MerchantDialog>;
auto message_dialog() -> Binding<MessageDialog>;
auto monster() -> Binding<Monster, Character>;
auto monster_armor() -> Binding<MonsterArmor>;
auto npc() -> Binding<NPC, Character>;
auto player() -> Binding<Player, Character>;
auto position() -> Binding<::position>;
auto random() -> Binding<Random>;
auto script_item() -> Binding<ScriptItem, Item>;
auto script_variables_table() -> Binding<ScriptVariablesTable>;
auto selection_dialog() -> Binding<SelectionDialog>;
auto tiles_struct() -> Binding<TilesStruct>;
auto waypoint_list() -> Binding<WaypointList>;
auto weapon_struct() -> Binding<WeaponStruct>;
auto weather_struct() -> Binding<WeatherStruct>;
auto world() -> Binding<World>;

} // namespace binding
