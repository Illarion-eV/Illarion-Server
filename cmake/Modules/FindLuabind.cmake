# Locate Lua library.
# This module defines:
#   LUABIND_FOUND - if false, do not try to link to Luabind
#   LUABIND_LIBRARIES - luabind libary and lua library
#   LUABIND_INCLUDE_DIRS - where to find luabind/luabind.hpp
#   LUABIND_VERSION_STRING - the version of Luabind found
#   LUABIND_VERSION_MAJOR - the major version of Luabind
#   LUABIND_VERSION_MINOR - the minor version of Luabind
#   LUABIND_VERSION_PATCH - the patch version of Luabind

find_package( Lua )

if( TARGET Luabind::Luabind )
    return()
endif()

if( NOT LUABIND_FOUND )
    file( TO_CMAKE_PATH "$ENV{LUABIND_DIR}" _LUABIND_DIR )

    find_library( LUABIND_LIBRARY
        NAMES libluabind luabind
        PATHS
            ${_LUABIND_DIR}/lib/${CMAKE_LIBRARY_ARCHITECTURE}
            ${CMAKE_INSTALL_PREFIX}/lib/${CMAKE_LIBRARY_ARCHITECTURE}
            /usr/local/lib/${CMAKE_LIBRARY_ARCHITECTURE}
            /usr/lib/${CMAKE_LIBRARY_ARCHITECTURE}
            ${_LUABIND_DIR}/lib
            ${_LUABIND_DIR}
            ${CMAKE_INSTALL_PREFIX}/lib
            ${CMAKE_INSTALL_PREFIX}/bin
            /usr/local/lib
            /usr/lib
        DOC "Location of luabind library"
        NO_DEFAULT_PATH
    )

    find_path( LUABIND_HEADER_DIR
        NAMES luabind/luabind.hpp
        PATHS
            ${_LUABIND_DIR}/include
            ${_LUABIND_DIR}
            ${CMAKE_INSTALL_PREFIX}/include
            /usr/local/pgsql/include
            /usr/local/include
            /usr/include
        DOC "Path to luabind/luabind.hpp header file. Do not include the 'luabind' directory in this value."
        NO_DEFAULT_PATH
    )

    if( LUABIND_HEADER_DIR )
        file(STRINGS "${LUABIND_HEADER_DIR}/luabind/version.hpp" luabind_version_str REGEX "^#define[\t ]+LUABIND_VERSION[\t ]+.*")
        if( luabind_version_str )
            string(REGEX REPLACE "^#define[\t ]+LUABIND_VERSION[\t ]+([0-9]*).*" "\\1" _LUABIND_VERSION_NUM "${luabind_version_str}")
            math( EXPR LUABIND_VERSION_MAJOR "${luabind_version_str} / 10000" )
            math( EXPR LUABIND_VERSION_MINOR "${luabind_version_str} % 10000 / 100" )
            math( EXPR LUABIND_VERSION_PATCH "${luabind_version_str} % 100" )
            set( LUABIND_VERSION_STRING "${LUABIND_VERSION_MAJOR}.${LUABIND_VERSION_MINOR}.${LUABIND_VERSION_PATCH}" )
        endif()
    endif()

    if( LUA_FOUND AND LUABIND_LIBRARY AND LUABIND_HEADER_DIR )
        set( LUABIND_FOUND 1 CACHE BOOL "Luabind found" FORCE )
        set( LUABIND_INCLUDE_DIRS "${LUABIND_HEADER_DIR};${LUA_INCLUDE_DIR}" CACHE STRING "Include directories for Luabind library" FORCE )
        set( LUABIND_LIBRARIES "${LUABIND_LIBRARY};${LUA_LIBRARIES}" CACHE STRING "Link libraries for Luabind library" FORCE )
    endif()

    find_package_handle_standard_args( Luabind
        FOUND_VAR LUABIND_FOUND
        REQUIRED_VARS LUABIND_INCLUDE_DIRS LUABIND_LIBRARIES
        VERSION_VAR LUABIND_VERSION_STRING )
endif()

if( LUABIND_INCLUDE_DIRS AND LUABIND_LIBRARIES )
    if( NOT TARGET Luabind::Luabind )
        add_library( Luabind::Luabind INTERFACE IMPORTED )
        target_link_libraries( Luabind::Luabind INTERFACE ${LUABIND_LIBRARIES} )
        target_include_directories( Luabind::Luabind INTERFACE ${LUABIND_INCLUDE_DIRS} )
    endif()
endif()