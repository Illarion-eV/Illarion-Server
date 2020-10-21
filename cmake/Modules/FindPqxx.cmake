if( Pqxx_FIND_REQUIRED )
  find_package( PostgreSQL REQUIRED )
else()
  find_package( PostgreSQL )
endif()

# Find PostGreSQL C++ library and header file
# Sets
#   PQXX_FOUND         to 0 or 1 depending on result
#   PQXX_INCLUDE_DIRS  to the directory containing pqxx/pqxx
#   PQXX_LIBRARIES     to the psxx library (and any dependents required)
#   PQXX_VERSION_STRING - the version of Luabind found
# If PQXX_REQUIRED is defined, then a fatal error message will be generated if libpqxx is not found
if( NOT PQXX_INCLUDE_DIRS OR NOT PQXX_LIBRARIES )
  file( TO_CMAKE_PATH "$ENV{PQXX_DIR}" _PQXX_DIR )

  find_library( PQXX_LIBRARY
    NAMES libpqxx pqxx
    PATHS
      ${_PQXX_DIR}/lib/${CMAKE_LIBRARY_ARCHITECTURE}
      ${CMAKE_INSTALL_PREFIX}/lib/${CMAKE_LIBRARY_ARCHITECTURE}
      /usr/local/pgsql/lib/${CMAKE_LIBRARY_ARCHITECTURE}
      /usr/local/lib/${CMAKE_LIBRARY_ARCHITECTURE}
      /usr/lib/${CMAKE_LIBRARY_ARCHITECTURE}
      ${_PQXX_DIR}/lib
      ${_PQXX_DIR}
      ${CMAKE_INSTALL_PREFIX}/lib
      ${CMAKE_INSTALL_PREFIX}/bin
      /usr/local/pgsql/lib
      /usr/local/lib
      /usr/lib
    DOC "Location of libpqxx library"
    NO_DEFAULT_PATH
  )

  find_path( PQXX_HEADER_DIR
    NAMES pqxx/pqxx
    PATHS
      ${_PQXX_DIR}/include
      ${_PQXX_DIR}
      ${CMAKE_INSTALL_PREFIX}/include
      /usr/local/pgsql/include
      /usr/local/include
      /usr/include
    DOC "Path to pqxx/pqxx header file. Do not include the 'pqxx' directory in this value."
    NO_DEFAULT_PATH
  )

  if( PQXX_HEADER_DIR )
    file(STRINGS "${PQXX_HEADER_DIR}/pqxx/version.hxx" pqxx_version_str REGEX "^#define[\t ]+PQXX_VERSION[\t ]+\"[^\"]+\".*")
    if( luabind_version_str )
        string(REGEX REPLACE "^#define[\t ]+PQXX_VERSION[\t ]+\"([^\"]+)\".*" "\\1" PQXX_VERSION_STRING "${pqxx_version_str}")
    endif()
  endif()

  if( PostgreSQL_FOUND AND PQXX_HEADER_DIR AND PQXX_LIBRARY )
    set( PQXX_FOUND TRUE CACHE BOOL "PQXX found" FORCE )
    set( PQXX_INCLUDE_DIRS "${PQXX_HEADER_DIR};${PostgreSQL_INCLUDE_DIRS}" CACHE STRING "Include directories for PostGreSQL C++ library"  FORCE )
    set( PQXX_LIBRARIES "${PQXX_LIBRARY};${PostgreSQL_LIBRARIES}" CACHE STRING "Link libraries for PostGreSQL C++ interface" FORCE )

    mark_as_advanced( PQXX_INCLUDE_DIRS PQXX_LIBRARIES )
  endif()

  find_package_handle_standard_args( Pqxx
    FOUND_VAR PQXX_FOUND
    REQUIRED_VARS PQXX_INCLUDE_DIRS PQXX_LIBRARIES
    VERSION_VAR PQXX_VERSION_STRING )
endif()

if( NOT TARGET Pqxx::Pqxx AND PQXX_FOUND )
  add_library( Pqxx::Pqxx INTERFACE IMPORTED )
  target_include_directories( Pqxx::Pqxx INTERFACE ${PQXX_HEADER_PATH} )

  if( TARGET PostgreSQL::PostgreSQL )
    target_link_libraries( Pqxx::Pqxx INTERFACE ${PQXX_LIBRARY} PostgreSQL::PostgreSQL )
  else()
    # Support outdated cmake versions
    target_link_libraries( Pqxx::Pqxx INTERFACE ${PQXX_LIBRARIES} )
    target_include_directories( Pqxx::Pqxx INTERFACE ${PQXX_INCLUDE_DIRS} )
  endif()
endif()