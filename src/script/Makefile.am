AM_CXXFLAGS = -ggdb -pipe -Wall -Werror -Wno-deprecated -std=c++14 $(BOOST_CXXFLAGS) $(DEPS_CFLAGS)
AM_CPPFLAGS = -D_THREAD_SAFE -D_REENTRANT $(subst -I,-isystem,$(BOOST_CPPFLAGS)) -I$(top_srcdir)/src

noinst_LIBRARIES = libscriptbinding.a

libscriptbinding_a_SOURCES = LuaScript.cpp forwarder.cpp LuaTestSupportScript.cpp \
			     LuaNPCScript.cpp LuaItemScript.cpp LuaMonsterScript.cpp \
			     LuaMagicScript.cpp LuaTileScript.cpp LuaTriggerScript.cpp \
			     LuaWeaponScript.cpp LuaScheduledScript.cpp LuaLongTimeEffectScript.cpp \
			     LuaReloadScript.cpp LuaLoginScript.cpp LuaLogoutScript.cpp \
			     LuaDepotScript.cpp LuaLookAtPlayerScript.cpp LuaLearnScript.cpp \
			     LuaPlayerDeathScript.cpp LuaLookAtItemScript.cpp LuaQuestScript.cpp \
			     binding/armor_struct.cpp binding/attack_boni.cpp binding/binding.hpp \
				 binding/character.cpp binding/character_skillvalue.cpp binding/colour.cpp \
				 binding/common_struct.cpp binding/container.cpp binding/crafting_dialog.cpp \
				 binding/field.cpp binding/input_dialog.cpp binding/item.cpp \
				 binding/item_look_at.cpp binding/long_time_action.cpp binding/long_time_character_effects.cpp \
				 binding/long_time_effect.cpp binding/long_time_effect_struct.cpp binding/merchant_dialog.cpp \
				 binding/message_dialog.cpp binding/monster.cpp binding/monster_armor.cpp \
				 binding/npc.cpp binding/player.cpp binding/position.cpp \
				 binding/random.cpp binding/script_item.cpp binding/script_variables_table.cpp \
				 binding/selection_dialog.cpp binding/tiles_struct.cpp binding/waypoint_list.cpp \
				 binding/weapon_struct.cpp binding/weather_struct.cpp binding/world.cpp

noinst_HEADERS = forwarder.hpp \
		 LuaLookAtItemScript.hpp LuaTestSupportScript.hpp \
		 LuaLookAtPlayerScript.hpp LuaTriggerScript.hpp \
		 LuaLongTimeEffectScript.hpp LuaLearnScript.hpp \
		 LuaNPCScript.hpp LuaWeaponScript.hpp \
		 LuaMagicScript.hpp LuaScheduledScript.hpp \
		 LuaLoginScript.hpp LuaLogoutScript.hpp \
		 LuaReloadScript.hpp LuaQuestScript.hpp \
		 LuaDepotScript.hpp LuaMonsterScript.hpp \
		 LuaPlayerDeathScript.hpp LuaScript.hpp \
		 LuaTileScript.hpp LuaItemScript.hpp \
		 binding/binding.hpp
