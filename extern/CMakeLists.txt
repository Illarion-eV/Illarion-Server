include( FetchContent )

FetchContent_Declare(
    googletest
    GIT_REPOSITORY https://github.com/google/googletest.git
    GIT_TAG        release-1.10.0
)

FetchContent_GetProperties( googletest )
if( NOT googletest_POPULATED )
    FetchContent_Populate( googletest )
    add_subdirectory( ${googletest_SOURCE_DIR} ${googletest_BINARY_DIR} )
endif()

FetchContent_Declare(
    luabind
    GIT_REPOSITORY https://github.com/vilarion/luabind.git
    GIT_TAG        master
)

FetchContent_GetProperties( luabind )
if( NOT luabind_POPULATED )
    FetchContent_Populate( luabind )
    set( LUABIND_USE_CXX11 true )
    set( LUABIND_DYNAMIC_LINK false )
    add_subdirectory( ${luabind_SOURCE_DIR} ${luabind_BINARY_DIR} )
endif()

set( LUABIND_FOUND 1 CACHE BOOL "Luabind found" FORCE )
set( LUABIND_INCLUDE_DIRS "${luabind_SOURCE_DIR};${luabind_BINARY_DIR};${Boost_INCLUDE_DIRS};${LUA_INCLUDE_DIR}" CACHE STRING "Include directories for Luabind library" FORCE )
set( LUABIND_LIBRARIES "luabind;${LUA_LIBRARIES}" CACHE STRING "Link libraries for Luabind library" FORCE )