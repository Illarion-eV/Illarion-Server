#   illarionserver - server for the game Illarion
#   Copyright 2011 Illarion e.V.
#
#   This file is part of illarionserver.
#
#   illarionserver is free software: you can redistribute it and/or modify
#   it under the terms of the GNU Affero General Public License as published by
#   the Free Software Foundation, either version 3 of the License, or
#   (at your option) any later version.
#
#   illarionserver is distributed in the hope that it will be useful,
#   but WITHOUT ANY WARRANTY; without even the implied warranty of
#   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
#   GNU Affero General Public License for more details.
#
#   You should have received a copy of the GNU Affero General Public License
#   along with illarionserver.  If not, see <http://www.gnu.org/licenses/>.


check_LTLIBRARIES = libgmock.la
check_PROGRAMS = test_binding ItemTest CharacterContainerTest test_container \
                 test_binding_item test_binding_scriptitem test_binding_position \
                 test_binding_longtimeaction test_binding_weatherstruct \
                 test_binding_character test_map_import

AM_CXXFLAGS = -ggdb -pipe -Wall -Wno-deprecated -std=c++11 $(BOOST_CXXFLAGS) $(DEPS_CFLAGS)
AM_CPPFLAGS = -D_THREAD_SAFE -D_REENTRANT $(BOOST_CPPFLAGS) -I$(top_srcdir)/src
AM_LDFLAGS = $(BOOST_FILESYSTEM_LDFLAGS) $(BOOST_SYSTEM_LDFLAGS)
LDADD = $(DEPS_LIBS) $(top_builddir)/src/libserver.la libgmock.la

LIBS = $(BOOST_THREAD_LIBS) $(BOOST_FILESYSTEM_LIBS) $(BOOST_SYSTEM_LIBS)

nodist_libgmock_la_SOURCES = $(GMOCK_DIR)/src/gmock-all.cc $(GTEST_DIR)/src/gtest-all.cc
libgmock_la_CPPFLAGS = $(AM_CPPFLAGS) -I$(GMOCK_DIR) -I$(GTEST_DIR)

TESTS = $(check_PROGRAMS)

test_binding_SOURCES = test_binding.cpp

test_binding_item_SOURCES = test_binding_item.cpp
test_binding_scriptitem_SOURCES = test_binding_scriptitem.cpp
test_binding_position_SOURCES = test_binding_position.cpp
test_binding_longtimeaction_SOURCES = test_binding_longtimeaction.cpp
test_binding_weatherstruct_SOURCES = test_binding_weatherstruct.cpp
test_binding_character_SOURCES = test_binding_character.cpp

ItemTest_SOURCES = ItemTest.cpp

CharacterContainerTest_SOURCES = CharacterContainerTest.cpp

test_container_SOURCES = test_container.cpp

test_map_import_SOURCES = test_map_import.cpp
