add_library(db
    Connection.cpp
    Connection.hpp
    ConnectionManager.cpp
    ConnectionManager.hpp
    DeleteQuery.cpp
    DeleteQuery.hpp
    InsertQuery.cpp
    InsertQuery.hpp
    Query.cpp
    Query.hpp
    QueryAssign.cpp
    QueryAssign.hpp
    QueryColumns.cpp
    QueryColumns.hpp
    QueryTables.cpp
    QueryTables.hpp
    QueryWhere.cpp
    QueryWhere.hpp
    Result.hpp
    SchemaHelper.cpp
    SchemaHelper.hpp
    SelectQuery.cpp
    SelectQuery.hpp
    UpdateQuery.cpp
    UpdateQuery.hpp)

target_link_libraries(db /usr/lib/x86_64-linux-gnu/libpqxx-6.2.so)
target_compile_options(db PRIVATE ${WARNINGS})
