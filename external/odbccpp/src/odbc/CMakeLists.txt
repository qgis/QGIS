# Public Header Files
set(public_headers
    Config.h
    Connection.h
    DatabaseMetaData.h
    DatabaseMetaDataBase.h
    DatabaseMetaDataUnicode.h
    Environment.h
    Exception.h
    Forwards.h
    ParameterMetaData.h
    PreparedStatement.h
    RefCounted.h
    ResultSet.h
    ResultSetMetaData.h
    ResultSetMetaDataBase.h
    ResultSetMetaDataUnicode.h
    Statement.h
    StatementBase.h
    Types.h
    Util.h
)

# Sources
set(odbccpp_sources
    Connection.cpp
    DatabaseMetaData.cpp
    DatabaseMetaDataBase.cpp
    DatabaseMetaDataUnicode.cpp
    Environment.cpp
    Exception.cpp
    ParameterMetaData.cpp
    PreparedStatement.cpp
    RefCounted.cpp
    ResultSet.cpp
    ResultSetMetaData.cpp
    ResultSetMetaDataBase.cpp
    ResultSetMetaDataUnicode.cpp
    Statement.cpp
    StatementBase.cpp
    Types.cpp
    Util.cpp
    internal/Batch.cpp
    internal/ParameterData.cpp
    internal/UtilInternal.cpp
)

# Static library
add_library(odbccpp_static
    STATIC
        ${odbccpp_sources}
)

target_compile_definitions(odbccpp_static
    PUBLIC
        ODBC_STATIC
        "NS_ODBC=qgs::odbc"
        "NS_ODBC_START=namespace qgs { namespace odbc {"
        "NS_ODBC_END=} }"
)

target_include_directories(odbccpp_static PUBLIC ${ODBC_INCLUDE_DIRS})
set_property(TARGET odbccpp_static PROPERTY POSITION_INDEPENDENT_CODE ON)
