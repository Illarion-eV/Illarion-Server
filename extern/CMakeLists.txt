include( FetchContent )

FetchContent_Declare(
    googletest
    GIT_REPOSITORY https://github.com/google/googletest.git
    GIT_TAG        e2239ee6043f73722e7aa812a459f54a28552929 #origin/v1.11.0
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
    PATCH_COMMAND  git checkout -- CMakeLists.txt && git apply --reject --ignore-space-change --ignore-whitespace "${CMAKE_CURRENT_SOURCE_DIR}/luabind.patch"
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

FetchContent_Declare(
    range_v3
    GIT_REPOSITORY https://github.com/ericniebler/range-v3.git
    GIT_TAG        4989f3e9ff2efee1852942bb9328ef121369ba02 #origin/0.11.0
    GIT_SHALLOW    ON
)

FetchContent_GetProperties( range_v3 )
if( NOT range_v3_POPULATED )
    FetchContent_Populate( range_v3 )
    if( NOT CMAKE_BUILD_TYPE STREQUAL "Debug")
        set( CMAKE_BUILD_TYPE "Release" )
    endif()
    add_subdirectory( ${range_v3_SOURCE_DIR} ${range_v3_BINARY_DIR} )
endif()

FetchContent_Declare(
    pqxx
    GIT_REPOSITORY https://github.com/jtv/libpqxx.git
    GIT_TAG        221ddc8be329bafb376a3d83b9cd257fd52fc7b7 #pre 7.7.0
    GIT_SHALLOW    ON
)

FetchContent_GetProperties( pqxx )
if( NOT pqxx_POPULATED )
    FetchContent_Populate( pqxx )
    if( NOT CMAKE_BUILD_TYPE STREQUAL "Debug")
        set( CMAKE_BUILD_TYPE "Release" )
    endif()
    set( SKIP_BUILD_TEST on )
    set( BUILD_SHARED_LIBS off )
    add_subdirectory( ${pqxx_SOURCE_DIR} ${pqxx_BINARY_DIR} )
endif()

set( PQXX_FOUND 1 CACHE BOOL "libpqxx found" FORCE )
set( PQXX_INCLUDE_DIRS "${pqxx_SOURCE_DIR};${pqxx_BINARY_DIR}" CACHE STRING "Include directories for pqxx library" FORCE )
set( PQXX_LIBRARIES "pqxx" CACHE STRING "Link libraries for pqxx library" FORCE )
set( PQXX_VERSION_STRING "7.6" )
