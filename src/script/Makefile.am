AM_CXXFLAGS = -ggdb -pipe -Wall -Werror -Wno-deprecated -std=c++11 $(BOOST_CXXFLAGS) $(DEPS_CFLAGS) -fPIC
AM_CPPFLAGS = -D_THREAD_SAFE -D_REENTRANT -DTESTSERVER -DCDataConnect_DEBUG -DAdminCommands_DEBUG $(subst -I,-isystem,$(BOOST_CPPFLAGS)) -I$(top_srcdir)/src -DPIC

noinst_LTLIBRARIES = libscriptbinding.la

libscriptbinding_la_SOURCES = LuaScript.cpp forwarder.cpp LuaTestSupportScript.cpp \
			     LuaNPCScript.cpp LuaItemScript.cpp LuaMonsterScript.cpp \
			     LuaMagicScript.cpp LuaTileScript.cpp LuaTriggerScript.cpp \
			     LuaWeaponScript.cpp LuaScheduledScript.cpp LuaLongTimeEffectScript.cpp \
			     LuaReloadScript.cpp LuaLoginScript.cpp LuaLogoutScript.cpp \
			     LuaDepotScript.cpp LuaLookAtPlayerScript.cpp LuaLearnScript.cpp \
			     LuaPlayerDeathScript.cpp LuaLookAtItemScript.cpp LuaQuestScript.cpp

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
		 LuaTileScript.hpp LuaItemScript.hpp
