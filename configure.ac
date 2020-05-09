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


# Process this file with autoconf to produce a configure script.
AC_PREREQ(2.69)
AC_INIT(Illarion Server, 0.9.7, https://github.com/Illarion-eV/Illarion-Server/issues, illarion, https://illarion.org)
AC_CONFIG_MACRO_DIR([m4])
AM_INIT_AUTOMAKE([-Wno-portability foreign 1.16])
AC_CONFIG_HEADERS(config.h)

# Reduce verbosity
AM_SILENT_RULES([yes])

# Checks for programs.
AC_PROG_CXX(g++)
LT_INIT

# Checks for libraries.

# Checks for header files.
AC_HEADER_STDC
AC_CHECK_HEADERS([arpa/inet.h errno.h netdb.h netinet/in.h stdlib.h string.h sys/socket.h sys/time.h sys/timeb.h unistd.h])

# Checks for typedefs, structures, and compiler characteristics.
AC_C_CONST
AC_C_INLINE
AC_TYPE_SIZE_T
AC_HEADER_TIME
AC_STRUCT_TM

# Checks for library functions.
AC_CHECK_FUNCS([bzero ftime gethostbyname inet_ntoa memset select socket strerror strtol strtoul])

BOOST_REQUIRE([1.67])
BOOST_SYSTEM
BOOST_GRAPH

PKG_CHECK_MODULES([lua], [lua5.2])
PKG_CHECK_MODULES([libpqxx], [libpqxx >= 6.2])
PKG_CHECK_MODULES([luabind], [luabind >= 0.9.1])

AC_CONFIG_LINKS([test/maps/test.tiles.txt:test/maps/test.tiles.txt
       test/maps/test.items.txt:test/maps/test.items.txt
       test/maps/test.warps.txt:test/maps/test.warps.txt
       script/issue_10631_a.lua:test/script/issue_10631_a.lua
       script/issue_10631_b.lua:test/script/issue_10631_b.lua
])

AC_LANG(C++)

DEPS_CFLAGS="$lua_CFLAGS $libpqxx_CFLAGS"
DEPS_LIBS="$lua_LIBS $libpqxx_LIBS $luabind_LIBS"

ac_gtest_dir="/usr/src/gtest"
ac_gmock_dir="/usr/src/gmock"
AC_ARG_WITH([gtest],
            [AS_HELP_STRING([--with-gtest],
              [set custom path for gtest source])],
            [ac_gtest_dir=$withval],
            [])
AC_ARG_WITH([gmock],
            [AS_HELP_STRING([--with-gmock],
              [set custom path for gmock source])],
            [ac_gmock_dir=$withval],
            [])

if [ ! test -e $ac_gtest_dir/src/gtest-all.cc ]; then
	AC_MSG_ERROR([gtest sources not found])
fi

if [ ! test -e $ac_gmock_dir/src/gmock-all.cc ]; then
	AC_MSG_ERROR([gmock sources not found])
fi

AC_SUBST([GMOCK_DIR], [$ac_gmock_dir])
AC_SUBST([GTEST_DIR], [$ac_gtest_dir])
AC_SUBST([DEPS_CFLAGS])
AC_SUBST([DEPS_LIBS])

AC_OUTPUT([Makefile
	   src/Makefile
	   src/script/Makefile
	   test/Makefile
])

