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

add_library(odbccpp
    STATIC
        ${odbccpp_sources})

target_include_directories(odbccpp
    PUBLIC
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/..>
        $<INSTALL_INTERFACE:${CMAKE_INSTALL_INCLUDEDIR}>
)

target_link_libraries(odbccpp
    PUBLIC
        ODBC::ODBC
)

target_compile_definitions(odbccpp
    PUBLIC
        ODBC_STATIC
        "NS_ODBC=qgs::odbc"
        "NS_ODBC_START=namespace qgs { namespace odbc {"
        "NS_ODBC_END=} }"
)
set_property(TARGET odbccpp PROPERTY POSITION_INDEPENDENT_CODE ON)

set_property(TARGET odbccpp PROPERTY public_headers ${public_headers})

install(TARGETS odbccpp
    EXPORT odbccppTargets
    LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
    ARCHIVE DESTINATION ${CMAKE_INSTALL_LIBDIR}
    RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
)

install(FILES ${public_headers}
    DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}/odbc
)

install(EXPORT odbccppTargets
    FILE odbccppTargets.cmake
    DESTINATION ${CMAKE_INSTALL_LIBDIR}/cmake/odbccpp
)
