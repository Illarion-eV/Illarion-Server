
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

   GCC 8.3
   GNU Make 4.2.1
   Automake 1.16.1 or CMake 3.13.4
   Boost 1.67.0
   PostgreSQL 11.9
   libpqxx 6.2.5
   Lua 5.2.4
   Luabind 0.9.1 with some bugfixes from
        http://illarion.org/repos/apt/debian (binary)
     or https://github.com/vilarion/luabind/tree/illarion (source)
   range-v3 0.4.0

   googletest 1.10 is included as a git submodule

Build

   With Automake:

     ./bootstrap
     mkdir .build
     cd .build
     ../configure
     make

   With CMake:

     mkdir .build
     cd .build
     cmake ..
     make

     ______________________________________________________________________

   Last modified: Jun 24, 2020
