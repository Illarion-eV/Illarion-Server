
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

   GCC 4.4.5
   GNU Make 3.81
   Automake 1.11.1
   Boost 1.52.0
   PostgreSQL 8.4.13
   libpqxx 3.1
   Lua 5.1.4
   Luabind 0.9.1 with some bugfixes: https://github.com/vilarion/luabind

Build

   Run inside the source directory:
   aclocal
   autoheader
   autoconf
   automake -a -c
   ./configure
   make
     ______________________________________________________________________

   Last modified: Aug 18, 2012
