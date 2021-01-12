# Public Header Files
SET(public_headers
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
SET(odbccpp_sources
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
ADD_LIBRARY(odbccpp_static
    STATIC
        ${odbccpp_sources}
)
TARGET_COMPILE_DEFINITIONS(odbccpp_static
    PUBLIC
        ODBC_STATIC
)
SET_PROPERTY(TARGET odbccpp_static PROPERTY POSITION_INDEPENDENT_CODE ON)

# Shared library
ADD_LIBRARY(odbccpp
    SHARED
        ${odbccpp_sources}
)
TARGET_COMPILE_DEFINITIONS(odbccpp
    PRIVATE
        ODBC_EXPORTS
)

TARGET_LINK_LIBRARIES(odbccpp
    PUBLIC
        ${ODBC_LIBRARIES}
)

SET_PROPERTY(TARGET odbccpp PROPERTY public_headers ${public_headers})

# Installation
INSTALL(TARGETS odbccpp_static DESTINATION lib)
INSTALL(TARGETS odbccpp DESTINATION lib)
INSTALL(FILES ${public_headers} DESTINATION include/odbc)
