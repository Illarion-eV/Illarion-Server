
  Illarion

Objectives

   Illarion is the online multiplayer roleplaying game that is developed and
   maintained by the Illarion e.V. This repository contains the server 
   application.

Details

   The application in this repository is the Illarion Server. The official
   repository is https://github.com/Illarion-eV/Illarion-Server. The lead
   developer's repository is https://github.com/vilarion/Illarion-Server. 
   It works together with the Illarion Client found at
   https://github.com/Illarion-eV/Illarion-Java.

Requirements

   GCC 4.7.2 OR clang 3.1
   GNU Make 3.81
   Automake 1.11.6
   Boost 1.49.0
   PostgreSQL 9.1.7
   libpqxx 3.1
   Lua 5.2.1
   Luabind 0.9.1 with some bugfixes from http://illarion.org/repos/apt/debian

Build

   Run inside the source directory:
   ./bootstrap
   ./configure OR ./configure CXX=clang
   make

   for non-standard locations of luabind (i.e. in /usr/local/lib and not in /usr/lib)
   use LDFLAGS=-L/usr/local CPPFLAGS=-I/usr/local/include ./configure

   if you want to use multiple boost versions in parallel you should use the
   flag --layout=versioned to b2 on installing this version to make sure we link against
   the correct version
     ______________________________________________________________________

   Last modified: Apr 30, 2013
