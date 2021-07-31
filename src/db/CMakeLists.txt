find_package( Boost REQUIRED )
find_package( Pqxx REQUIRED )

add_library( db STATIC "" )
target_sources( db 
    PRIVATE
        Connection.cpp
        ConnectionManager.cpp
        DeleteQuery.cpp
        InsertQuery.cpp
        Query.cpp
        QueryAssign.cpp
        QueryColumns.cpp
        QueryTables.cpp
        QueryWhere.cpp
        SchemaHelper.cpp
        SelectQuery.cpp
        UpdateQuery.cpp
)

target_link_libraries( db PUBLIC Boost::boost Pqxx::Pqxx )
target_include_directories( db PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/.. )
target_include_directories( db PRIVATE ${CMAKE_CURRENT_SOURCE_DIR} )
target_compile_features( db PUBLIC cxx_std_20 )
