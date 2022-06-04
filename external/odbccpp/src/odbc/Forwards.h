#ifndef ODBC_FORWARDS_H_INCLUDED
#define ODBC_FORWARDS_H_INCLUDED
//------------------------------------------------------------------------------
#include <odbc/Config.h>
#include <odbc/RefCounted.h>
//------------------------------------------------------------------------------
NS_ODBC_START
//------------------------------------------------------------------------------
class Connection;
class DatabaseMetaData;
class DatabaseMetaDataBase;
class DatabaseMetaDataUnicode;
class Environment;
class Exception;
class ParameterMetaData;
class PreparedStatement;
class ResultSet;
class ResultSetMetaData;
class ResultSetMetaDataBase;
class ResultSetMetaDataUnicode;
class StatementBase;
class Statement;
class ValueBuffer;
//------------------------------------------------------------------------------
typedef Reference<Connection> ConnectionRef;
typedef Reference<DatabaseMetaData> DatabaseMetaDataRef;
typedef Reference<DatabaseMetaDataUnicode> DatabaseMetaDataUnicodeRef;
typedef Reference<Environment> EnvironmentRef;
typedef Reference<ParameterMetaData> ParameterMetaDataRef;
typedef Reference<PreparedStatement> PreparedStatementRef;
typedef Reference<ResultSet> ResultSetRef;
typedef Reference<ResultSetMetaData> ResultSetMetaDataRef;
typedef Reference<ResultSetMetaDataUnicode> ResultSetMetaDataUnicodeRef;
typedef Reference<StatementBase> StatementBaseRef;
typedef Reference<Statement> StatementRef;
typedef Reference<ValueBuffer> ValueBufferRef;
//------------------------------------------------------------------------------
NS_ODBC_END
//------------------------------------------------------------------------------
#endif
