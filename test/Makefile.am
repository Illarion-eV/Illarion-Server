#   illarionserver - server for the game Illarion
#   Copyright 2011 Illarion e.V.
#
#   This file is part of illarionserver.
#
#   illarionserver is free software: you can redistribute it and/or modify
#   it under the terms of the GNU General Public License as published by
#   the Free Software Foundation, either version 3 of the License, or
#   (at your option) any later version.
#
#   illarionserver is distributed in the hope that it will be useful,
#   but WITHOUT ANY WARRANTY; without even the implied warranty of
#   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
#   GNU General Public License for more details.
#
#   You should have received a copy of the GNU General Public License
#   along with illarionserver.  If not, see <http://www.gnu.org/licenses/>.


check_LTLIBRARIES = libgmock.la
check_PROGRAMS = test_binding ItemTest

AM_CXXFLAGS = -ggdb -pipe -Wall -Werror -Wno-deprecated -std=c++11 $(BOOST_CXXFLAGS) $(DEPS_CFLAGS)
AM_CPPFLAGS = -D_THREAD_SAFE -D_REENTRANT -DTESTSERVER -DCDataConnect_DEBUG -DAdminCommands_DEBUG $(BOOST_CPPFLAGS) -I$(top_srcdir)/src
AM_LDFLAGS = $(BOOST_THREAD_LDFLAGS) $(BOOST_FILESYSTEM_LDFLAGS) $(BOOST_SYSTEM_LDFLAGS)
LDADD = -L/usr/local/lib -lpthread -lssl -lcrypto $(DEPS_LIBS) -lcrypt $(top_builddir)/src/libserver.la

LIBS = $(BOOST_THREAD_LIBS) $(BOOST_FILESYSTEM_LIBS) $(BOOST_SYSTEM_LIBS)

nodist_libgmock_la_SOURCES = $(GMOCK_DIR)/src/gmock-all.cc $(GTEST_DIR)/src/gtest-all.cc
libgmock_la_CPPFLAGS = $(AM_CPPFLAGS) -I$(GMOCK_DIR) -I$(GTEST_DIR)

TESTS = test_binding ItemTest

test_binding_SOURCES = test_binding.cpp
test_binding_LDADD = $(LDADD) libgmock.la

ItemTest_SOURCES = ItemTest.cpp
ItemTest_LDADD = $(LDADD) libgmock.la

