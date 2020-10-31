include( FetchContent )

FetchContent_Declare(
    googletest
    GIT_REPOSITORY https://github.com/google/googletest.git
    GIT_TAG        6a7ed316a5cdc07b6d26362c90770787513822d4 #origin/v1.10.x
    GIT_SHALLOW    ON
)

FetchContent_GetProperties( googletest )
if( NOT googletest_POPULATED )
    FetchContent_Populate( googletest )
    add_subdirectory( ${googletest_SOURCE_DIR} ${googletest_BINARY_DIR} )
endif()

FetchContent_Declare(
    luabind
    GIT_REPOSITORY https://github.com/vilarion/luabind.git
    GIT_TAG        a9287d342c70a37b68beca694ee75f2d5c747db4 #origin/master
    GIT_SHALLOW    ON
    PATCH_COMMAND  git checkout -- CMakeLists.txt && git apply "${CMAKE_CURRENT_SOURCE_DIR}/luabind.patch"
)

FetchContent_GetProperties( luabind )
if( NOT luabind_POPULATED )
    FetchContent_Populate( luabind )
    set( LUABIND_USE_CXX11 true )
    set( LUABIND_DYNAMIC_LINK false )
    if( CMAKE_CXX_COMPILER_ID STREQUAL "GNU" )
        string( REPLACE "-O3" "-O2" CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE}" )
    endif()
    add_subdirectory( ${luabind_SOURCE_DIR} ${luabind_BINARY_DIR} )
endif()

set( LUABIND_FOUND 1 CACHE BOOL "Luabind found" FORCE )
set( LUABIND_INCLUDE_DIRS "${luabind_SOURCE_DIR};${luabind_BINARY_DIR};${Boost_INCLUDE_DIRS};${LUA_INCLUDE_DIR}" CACHE STRING "Include directories for Luabind library" FORCE )
set( LUABIND_LIBRARIES "luabind;${LUA_LIBRARIES}" CACHE STRING "Link libraries for Luabind library" FORCE )
