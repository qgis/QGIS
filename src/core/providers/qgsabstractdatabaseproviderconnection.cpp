/***************************************************************************
  qgsabstractdatabaseproviderconnection.cpp - QgsAbstractDatabaseProviderConnection

 ---------------------
 begin                : 2.8.2019
 copyright            : (C) 2019 by Alessandro Pasotti
 email                : elpaso at itopen dot it
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/
#include "qgsabstractdatabaseproviderconnection.h"
#include "qgsvectorlayer.h"
#include "qgsexception.h"
#include "qgslogger.h"
#include "qgsfeedback.h"

#include <QVariant>
#include <QObject>

QgsAbstractDatabaseProviderConnection::QgsAbstractDatabaseProviderConnection( const QString &name ):
  QgsAbstractProviderConnection( name )
{

}

QgsAbstractDatabaseProviderConnection::QgsAbstractDatabaseProviderConnection( const QString &uri, const QVariantMap &configuration ):
  QgsAbstractProviderConnection( uri, configuration )
{

}
QgsAbstractDatabaseProviderConnection::Capabilities QgsAbstractDatabaseProviderConnection::capabilities() const
{
  return mCapabilities;
}

QgsAbstractDatabaseProviderConnection::GeometryColumnCapabilities QgsAbstractDatabaseProviderConnection::geometryColumnCapabilities()
{
  return mGeometryColumnCapabilities;
}

Qgis::SqlLayerDefinitionCapabilities QgsAbstractDatabaseProviderConnection::sqlLayerDefinitionCapabilities()
{
  return mSqlLayerDefinitionCapabilities;
}


QString QgsAbstractDatabaseProviderConnection::tableUri( const QString &schema, const QString &name ) const
{
  Q_UNUSED( schema )
  Q_UNUSED( name )
  throw QgsProviderConnectionException( QObject::tr( "Operation 'tableUri' is not supported" ) );
}


///@cond PRIVATE
void QgsAbstractDatabaseProviderConnection::checkCapability( QgsAbstractDatabaseProviderConnection::Capability capability ) const
{
  if ( ! mCapabilities.testFlag( capability ) )
  {
    static QMetaEnum metaEnum = QMetaEnum::fromType<QgsAbstractDatabaseProviderConnection::Capability>();
    const QString capName { metaEnum.valueToKey( capability ) };
    throw QgsProviderConnectionException( QObject::tr( "Operation '%1' is not supported for this connection" ).arg( capName ) );
  }
}

QString QgsAbstractDatabaseProviderConnection::providerKey() const
{
  return mProviderKey;
}

///@endcond


QMultiMap<Qgis::SqlKeywordCategory, QStringList> QgsAbstractDatabaseProviderConnection::sqlDictionary()
{
  return
  {
    // Common constants
    {
      Qgis::SqlKeywordCategory::Constant, {
        QStringLiteral( "NULL" ),
        QStringLiteral( "FALSE" ),
        QStringLiteral( "TRUE" ),
      }
    },
    // Common SQL reserved words
    // From: GET https://en.wikipedia.org/wiki/SQL_reserved_words| grep 'style="background: #ececec; color: black; font-weight: bold;'| sed -e 's/.*>//'|sort
    {
      Qgis::SqlKeywordCategory::Keyword,
      {
        QStringLiteral( "ABORT " ),
        QStringLiteral( "ABORTSESSION" ),
        QStringLiteral( "ABS" ),
        QStringLiteral( "ABSOLUTE" ),
        QStringLiteral( "ACCESS" ),
        QStringLiteral( "ACCESSIBLE" ),
        QStringLiteral( "ACCESS_LOCK" ),
        QStringLiteral( "ACCOUNT" ),
        QStringLiteral( "ACOS" ),
        QStringLiteral( "ACOSH" ),
        QStringLiteral( "ACTION" ),
        QStringLiteral( "ADD" ),
        QStringLiteral( "ADD_MONTHS" ),
        QStringLiteral( "ADMIN" ),
        QStringLiteral( "AFTER" ),
        QStringLiteral( "AGGREGATE" ),
        QStringLiteral( "ALIAS" ),
        QStringLiteral( "ALL" ),
        QStringLiteral( "ALLOCATE" ),
        QStringLiteral( "ALLOW" ),
        QStringLiteral( "ALTER" ),
        QStringLiteral( "ALTERAND" ),
        QStringLiteral( "AMP" ),
        QStringLiteral( "ANALYSE" ),
        QStringLiteral( "ANALYZE" ),
        QStringLiteral( "AND" ),
        QStringLiteral( "ANSIDATE" ),
        QStringLiteral( "ANY" ),
        QStringLiteral( "ARE" ),
        QStringLiteral( "ARRAY" ),
        QStringLiteral( "ARRAY_AGG" ),
        QStringLiteral( "ARRAY_EXISTS" ),
        QStringLiteral( "ARRAY_MAX_CARDINALITY" ),
        QStringLiteral( "AS" ),
        QStringLiteral( "ASC" ),
        QStringLiteral( "ASENSITIVE" ),
        QStringLiteral( "ASIN" ),
        QStringLiteral( "ASINH" ),
        QStringLiteral( "ASSERTION" ),
        QStringLiteral( "ASSOCIATE" ),
        QStringLiteral( "ASUTIME" ),
        QStringLiteral( "ASYMMETRIC" ),
        QStringLiteral( "AT" ),
        QStringLiteral( "ATAN" ),
        QStringLiteral( "ATAN2" ),
        QStringLiteral( "ATANH" ),
        QStringLiteral( "ATOMIC" ),
        QStringLiteral( "AUDIT" ),
        QStringLiteral( "AUTHORIZATION" ),
        QStringLiteral( "AUX" ),
        QStringLiteral( "AUXILIARY" ),
        QStringLiteral( "AVE" ),
        QStringLiteral( "AVERAGE" ),
        QStringLiteral( "AVG" ),
        QStringLiteral( "BACKUP" ),
        QStringLiteral( "BEFORE" ),
        QStringLiteral( "BEGIN" ),
        QStringLiteral( "BEGIN_FRAME" ),
        QStringLiteral( "BEGIN_PARTITION" ),
        QStringLiteral( "BETWEEN" ),
        QStringLiteral( "BIGINT" ),
        QStringLiteral( "BINARY" ),
        QStringLiteral( "BIT" ),
        QStringLiteral( "BLOB" ),
        QStringLiteral( "BOOLEAN" ),
        QStringLiteral( "BOTH" ),
        QStringLiteral( "BREADTH" ),
        QStringLiteral( "BREAK" ),
        QStringLiteral( "BROWSE" ),
        QStringLiteral( "BT" ),
        QStringLiteral( "BUFFERPOOL" ),
        QStringLiteral( "BULK" ),
        QStringLiteral( "BUT" ),
        QStringLiteral( "BY" ),
        QStringLiteral( "BYTE" ),
        QStringLiteral( "BYTEINT" ),
        QStringLiteral( "BYTES" ),
        QStringLiteral( "CALL" ),
        QStringLiteral( "CALLED" ),
        QStringLiteral( "CAPTURE" ),
        QStringLiteral( "CARDINALITY" ),
        QStringLiteral( "CASCADE" ),
        QStringLiteral( "CASCADED" ),
        QStringLiteral( "CASE" ),
        QStringLiteral( "CASE_N" ),
        QStringLiteral( "CASESPECIFIC" ),
        QStringLiteral( "CAST" ),
        QStringLiteral( "CATALOG" ),
        QStringLiteral( "CCSID" ),
        QStringLiteral( "CD" ),
        QStringLiteral( "CEIL" ),
        QStringLiteral( "CEILING" ),
        QStringLiteral( "CHANGE" ),
        QStringLiteral( "CHAR" ),
        QStringLiteral( "CHAR2HEXINT" ),
        QStringLiteral( "CHARACTER" ),
        QStringLiteral( "CHARACTER_LENGTH" ),
        QStringLiteral( "CHARACTERS" ),
        QStringLiteral( "CHAR_LENGTH" ),
        QStringLiteral( "CHARS" ),
        QStringLiteral( "CHECK" ),
        QStringLiteral( "CHECKPOINT" ),
        QStringLiteral( "CLASS" ),
        QStringLiteral( "CLASSIFIER" ),
        QStringLiteral( "CLOB" ),
        QStringLiteral( "CLONE" ),
        QStringLiteral( "CLOSE" ),
        QStringLiteral( "CLUSTER" ),
        QStringLiteral( "CLUSTERED" ),
        QStringLiteral( "CM" ),
        QStringLiteral( "COALESCE" ),
        QStringLiteral( "COLLATE" ),
        QStringLiteral( "COLLATION" ),
        QStringLiteral( "COLLECT" ),
        QStringLiteral( "COLLECTION" ),
        QStringLiteral( "COLLID" ),
        QStringLiteral( "COLUMN" ),
        QStringLiteral( "COLUMN_VALUE" ),
        QStringLiteral( "COMMENT" ),
        QStringLiteral( "COMMIT" ),
        QStringLiteral( "COMPLETION" ),
        QStringLiteral( "COMPRESS" ),
        QStringLiteral( "COMPUTE" ),
        QStringLiteral( "CONCAT" ),
        QStringLiteral( "CONCURRENTLY" ),
        QStringLiteral( "CONDITION" ),
        QStringLiteral( "CONNECT" ),
        QStringLiteral( "CONNECTION" ),
        QStringLiteral( "CONSTRAINT" ),
        QStringLiteral( "CONSTRAINTS" ),
        QStringLiteral( "CONSTRUCTOR" ),
        QStringLiteral( "CONTAINS" ),
        QStringLiteral( "CONTAINSTABLE" ),
        QStringLiteral( "CONTENT" ),
        QStringLiteral( "CONTINUE" ),
        QStringLiteral( "CONVERT" ),
        QStringLiteral( "CONVERT_TABLE_HEADER" ),
        QStringLiteral( "COPY" ),
        QStringLiteral( "CORR" ),
        QStringLiteral( "CORRESPONDING" ),
        QStringLiteral( "COS" ),
        QStringLiteral( "COSH" ),
        QStringLiteral( "COUNT" ),
        QStringLiteral( "COVAR_POP" ),
        QStringLiteral( "COVAR_SAMP" ),
        QStringLiteral( "CREATE" ),
        QStringLiteral( "CROSS" ),
        QStringLiteral( "CS" ),
        QStringLiteral( "CSUM" ),
        QStringLiteral( "CT" ),
        QStringLiteral( "CUBE" ),
        QStringLiteral( "CUME_DIST" ),
        QStringLiteral( "CURRENT" ),
        QStringLiteral( "CURRENT_CATALOG" ),
        QStringLiteral( "CURRENT_DATE" ),
        QStringLiteral( "CURRENT_DEFAULT_TRANSFORM_GROUP" ),
        QStringLiteral( "CURRENT_LC_CTYPE" ),
        QStringLiteral( "CURRENT_PATH" ),
        QStringLiteral( "CURRENT_ROLE" ),
        QStringLiteral( "CURRENT_ROW" ),
        QStringLiteral( "CURRENT_SCHEMA" ),
        QStringLiteral( "CURRENT_SERVER" ),
        QStringLiteral( "CURRENT_TIME" ),
        QStringLiteral( "CURRENT_TIMESTAMP" ),
        QStringLiteral( "CURRENT_TIMEZONE" ),
        QStringLiteral( "CURRENT_TRANSFORM_GROUP_FOR_TYPE" ),
        QStringLiteral( "CURRENT_USER" ),
        QStringLiteral( "CURRVAL" ),
        QStringLiteral( "CURSOR" ),
        QStringLiteral( "CV" ),
        QStringLiteral( "CYCLE" ),
        QStringLiteral( "DATA" ),
        QStringLiteral( "DATABASE" ),
        QStringLiteral( "DATABASES" ),
        QStringLiteral( "DATABLOCKSIZE" ),
        QStringLiteral( "DATE" ),
        QStringLiteral( "DATEFORM" ),
        QStringLiteral( "DAY" ),
        QStringLiteral( "DAY_HOUR" ),
        QStringLiteral( "DAY_MICROSECOND" ),
        QStringLiteral( "DAY_MINUTE" ),
        QStringLiteral( "DAYS" ),
        QStringLiteral( "DAY_SECOND" ),
        QStringLiteral( "DBCC" ),
        QStringLiteral( "DBINFO" ),
        QStringLiteral( "DEALLOCATE" ),
        QStringLiteral( "DEC" ),
        QStringLiteral( "DECFLOAT" ),
        QStringLiteral( "DECIMAL" ),
        QStringLiteral( "DECLARE" ),
        QStringLiteral( "DEFAULT" ),
        QStringLiteral( "DEFERRABLE" ),
        QStringLiteral( "DEFERRED" ),
        QStringLiteral( "DEFINE" ),
        QStringLiteral( "DEGREES" ),
        QStringLiteral( "DEL" ),
        QStringLiteral( "DELAYED" ),
        QStringLiteral( "DELETE" ),
        QStringLiteral( "DENSE_RANK" ),
        QStringLiteral( "DENY" ),
        QStringLiteral( "DEPTH" ),
        QStringLiteral( "DEREF" ),
        QStringLiteral( "DESC" ),
        QStringLiteral( "DESCRIBE" ),
        QStringLiteral( "DESCRIPTOR" ),
        QStringLiteral( "DESTROY" ),
        QStringLiteral( "DESTRUCTOR" ),
        QStringLiteral( "DETERMINISTIC" ),
        QStringLiteral( "DIAGNOSTIC" ),
        QStringLiteral( "DIAGNOSTICS" ),
        QStringLiteral( "DICTIONARY" ),
        QStringLiteral( "DISABLE" ),
        QStringLiteral( "DISABLED" ),
        QStringLiteral( "DISALLOW" ),
        QStringLiteral( "DISCONNECT" ),
        QStringLiteral( "DISK" ),
        QStringLiteral( "DISTINCT" ),
        QStringLiteral( "DISTINCTROW" ),
        QStringLiteral( "DISTRIBUTED" ),
        QStringLiteral( "DIV" ),
        QStringLiteral( "DO" ),
        QStringLiteral( "DOCUMENT" ),
        QStringLiteral( "DOMAIN" ),
        QStringLiteral( "DOUBLE" ),
        QStringLiteral( "DROP" ),
        QStringLiteral( "DSSIZE" ),
        QStringLiteral( "DUAL" ),
        QStringLiteral( "DUMP" ),
        QStringLiteral( "DYNAMIC" ),
        QStringLiteral( "EACH" ),
        QStringLiteral( "ECHO" ),
        QStringLiteral( "EDITPROC" ),
        QStringLiteral( "ELEMENT" ),
        QStringLiteral( "ELSE" ),
        QStringLiteral( "ELSEIF" ),
        QStringLiteral( "EMPTY" ),
        QStringLiteral( "ENABLED" ),
        QStringLiteral( "ENCLOSED" ),
        QStringLiteral( "ENCODING" ),
        QStringLiteral( "ENCRYPTION" ),
        QStringLiteral( "END" ),
        QStringLiteral( "END-EXEC" ),
        QStringLiteral( "END_FRAME" ),
        QStringLiteral( "ENDING" ),
        QStringLiteral( "END_PARTITION" ),
        QStringLiteral( "EQ" ),
        QStringLiteral( "EQUALS" ),
        QStringLiteral( "ERASE" ),
        QStringLiteral( "ERRLVL" ),
        QStringLiteral( "ERROR" ),
        QStringLiteral( "ERRORFILES" ),
        QStringLiteral( "ERRORTABLES" ),
        QStringLiteral( "ESCAPE" ),
        QStringLiteral( "ESCAPED" ),
        QStringLiteral( "ET" ),
        QStringLiteral( "EVERY" ),
        QStringLiteral( "EXCEPT" ),
        QStringLiteral( "EXCEPTION" ),
        QStringLiteral( "EXCLUSIVE" ),
        QStringLiteral( "EXEC" ),
        QStringLiteral( "EXECUTE" ),
        QStringLiteral( "EXISTS" ),
        QStringLiteral( "EXIT" ),
        QStringLiteral( "EXP" ),
        QStringLiteral( "EXPLAIN" ),
        QStringLiteral( "EXTERNAL" ),
        QStringLiteral( "EXTRACT" ),
        QStringLiteral( "FALLBACK" ),
        QStringLiteral( "FALSE" ),
        QStringLiteral( "FASTEXPORT" ),
        QStringLiteral( "FENCED" ),
        QStringLiteral( "FETCH" ),
        QStringLiteral( "FIELDPROC" ),
        QStringLiteral( "FILE" ),
        QStringLiteral( "FILLFACTOR" ),
        QStringLiteral( "FILTER" ),
        QStringLiteral( "FINAL" ),
        QStringLiteral( "FIRST" ),
        QStringLiteral( "FIRST_VALUE" ),
        QStringLiteral( "FLOAT" ),
        QStringLiteral( "FLOAT4" ),
        QStringLiteral( "FLOAT8" ),
        QStringLiteral( "FLOOR" ),
        QStringLiteral( "FOR" ),
        QStringLiteral( "FORCE" ),
        QStringLiteral( "FOREIGN" ),
        QStringLiteral( "FORMAT" ),
        QStringLiteral( "FOUND" ),
        QStringLiteral( "FRAME_ROW" ),
        QStringLiteral( "FREE" ),
        QStringLiteral( "FREESPACE" ),
        QStringLiteral( "FREETEXT" ),
        QStringLiteral( "FREETEXTTABLE" ),
        QStringLiteral( "FREEZE" ),
        QStringLiteral( "FROM" ),
        QStringLiteral( "FULL" ),
        QStringLiteral( "FULLTEXT" ),
        QStringLiteral( "FUNCTION" ),
        QStringLiteral( "FUSION" ),
        QStringLiteral( "GE" ),
        QStringLiteral( "GENERAL" ),
        QStringLiteral( "GENERATED" ),
        QStringLiteral( "GET" ),
        QStringLiteral( "GIVE" ),
        QStringLiteral( "GLOBAL" ),
        QStringLiteral( "GO" ),
        QStringLiteral( "GOTO" ),
        QStringLiteral( "GRANT" ),
        QStringLiteral( "GRAPHIC" ),
        QStringLiteral( "GROUP" ),
        QStringLiteral( "GROUPING" ),
        QStringLiteral( "GROUPS" ),
        QStringLiteral( "GT" ),
        QStringLiteral( "HANDLER" ),
        QStringLiteral( "HASH" ),
        QStringLiteral( "HASHAMP" ),
        QStringLiteral( "HASHBAKAMP" ),
        QStringLiteral( "HASHBUCKET" ),
        QStringLiteral( "HASHROW" ),
        QStringLiteral( "HAVING" ),
        QStringLiteral( "HELP" ),
        QStringLiteral( "HIGH_PRIORITY" ),
        QStringLiteral( "HOLD" ),
        QStringLiteral( "HOLDLOCK" ),
        QStringLiteral( "HOST" ),
        QStringLiteral( "HOUR" ),
        QStringLiteral( "HOUR_MICROSECOND" ),
        QStringLiteral( "HOUR_MINUTE" ),
        QStringLiteral( "HOURS" ),
        QStringLiteral( "HOUR_SECOND" ),
        QStringLiteral( "IDENTIFIED" ),
        QStringLiteral( "IDENTITY" ),
        QStringLiteral( "IDENTITYCOL" ),
        QStringLiteral( "IDENTITY_INSERT" ),
        QStringLiteral( "IF" ),
        QStringLiteral( "IGNORE" ),
        QStringLiteral( "ILIKE" ),
        QStringLiteral( "IMMEDIATE" ),
        QStringLiteral( "IN" ),
        QStringLiteral( "INCLUSIVE" ),
        QStringLiteral( "INCONSISTENT" ),
        QStringLiteral( "INCREMENT" ),
        QStringLiteral( "INDEX" ),
        QStringLiteral( "INDICATOR" ),
        QStringLiteral( "INFILE" ),
        QStringLiteral( "INHERIT" ),
        QStringLiteral( "INITIAL" ),
        QStringLiteral( "INITIALIZE" ),
        QStringLiteral( "INITIALLY" ),
        QStringLiteral( "INITIATE" ),
        QStringLiteral( "INNER" ),
        QStringLiteral( "INOUT" ),
        QStringLiteral( "INPUT" ),
        QStringLiteral( "INS" ),
        QStringLiteral( "INSENSITIVE" ),
        QStringLiteral( "INSERT" ),
        QStringLiteral( "INSTEAD" ),
        QStringLiteral( "INT" ),
        QStringLiteral( "INT1" ),
        QStringLiteral( "INT2" ),
        QStringLiteral( "INT3" ),
        QStringLiteral( "INT4" ),
        QStringLiteral( "INT8" ),
        QStringLiteral( "INTEGER" ),
        QStringLiteral( "INTEGERDATE" ),
        QStringLiteral( "INTERSECT" ),
        QStringLiteral( "INTERSECTION" ),
        QStringLiteral( "INTERVAL" ),
        QStringLiteral( "INTO" ),
        QStringLiteral( "IO_AFTER_GTIDS" ),
        QStringLiteral( "IO_BEFORE_GTIDS" ),
        QStringLiteral( "IS" ),
        QStringLiteral( "ISNULL" ),
        QStringLiteral( "ISOBID" ),
        QStringLiteral( "ISOLATION" ),
        QStringLiteral( "ITERATE" ),
        QStringLiteral( "JAR" ),
        QStringLiteral( "JOIN" ),
        QStringLiteral( "JOURNAL" ),
        QStringLiteral( "JSON_ARRAY" ),
        QStringLiteral( "JSON_ARRAYAGG" ),
        QStringLiteral( "JSON_EXISTS" ),
        QStringLiteral( "JSON_OBJECT" ),
        QStringLiteral( "JSON_OBJECTAGG" ),
        QStringLiteral( "JSON_QUERY" ),
        QStringLiteral( "JSON_TABLE" ),
        QStringLiteral( "JSON_TABLE_PRIMITIVE" ),
        QStringLiteral( "JSON_VALUE" ),
        QStringLiteral( "KEEP" ),
        QStringLiteral( "KEY" ),
        QStringLiteral( "KEYS" ),
        QStringLiteral( "KILL" ),
        QStringLiteral( "KURTOSIS" ),
        QStringLiteral( "LABEL" ),
        QStringLiteral( "LAG" ),
        QStringLiteral( "LANGUAGE" ),
        QStringLiteral( "LARGE" ),
        QStringLiteral( "LAST" ),
        QStringLiteral( "LAST_VALUE" ),
        QStringLiteral( "LATERAL" ),
        QStringLiteral( "LC_CTYPE" ),
        QStringLiteral( "LE" ),
        QStringLiteral( "LEAD" ),
        QStringLiteral( "LEADING" ),
        QStringLiteral( "LEAVE" ),
        QStringLiteral( "LEFT" ),
        QStringLiteral( "LESS" ),
        QStringLiteral( "LEVEL" ),
        QStringLiteral( "LIKE" ),
        QStringLiteral( "LIKE_REGEX" ),
        QStringLiteral( "LIMIT" ),
        QStringLiteral( "LINEAR" ),
        QStringLiteral( "LINENO" ),
        QStringLiteral( "LINES" ),
        QStringLiteral( "LISTAGG" ),
        QStringLiteral( "LN" ),
        QStringLiteral( "LOAD" ),
        QStringLiteral( "LOADING" ),
        QStringLiteral( "LOCAL" ),
        QStringLiteral( "LOCALE" ),
        QStringLiteral( "LOCALTIME" ),
        QStringLiteral( "LOCALTIMESTAMP" ),
        QStringLiteral( "LOCATOR" ),
        QStringLiteral( "LOCATORS" ),
        QStringLiteral( "LOCK" ),
        QStringLiteral( "LOCKING" ),
        QStringLiteral( "LOCKMAX" ),
        QStringLiteral( "LOCKSIZE" ),
        QStringLiteral( "LOG" ),
        QStringLiteral( "LOG10" ),
        QStringLiteral( "LOGGING" ),
        QStringLiteral( "LOGON" ),
        QStringLiteral( "LONG" ),
        QStringLiteral( "LONGBLOB" ),
        QStringLiteral( "LONGTEXT" ),
        QStringLiteral( "LOOP" ),
        QStringLiteral( "LOWER" ),
        QStringLiteral( "LOW_PRIORITY" ),
        QStringLiteral( "LT" ),
        QStringLiteral( "MACRO" ),
        QStringLiteral( "MAINTAINED" ),
        QStringLiteral( "MAP" ),
        QStringLiteral( "MASTER_BIND" ),
        QStringLiteral( "MASTER_SSL_VERIFY_SERVER_CERT" ),
        QStringLiteral( "MATCH" ),
        QStringLiteral( "MATCHES" ),
        QStringLiteral( "MATCH_NUMBER" ),
        QStringLiteral( "MATCH_RECOGNIZE" ),
        QStringLiteral( "MATERIALIZED" ),
        QStringLiteral( "MAVG" ),
        QStringLiteral( "MAX" ),
        QStringLiteral( "MAXEXTENTS" ),
        QStringLiteral( "MAXIMUM" ),
        QStringLiteral( "MAXVALUE" ),
        QStringLiteral( "MCHARACTERS" ),
        QStringLiteral( "MDIFF" ),
        QStringLiteral( "MEDIUMBLOB" ),
        QStringLiteral( "MEDIUMINT" ),
        QStringLiteral( "MEDIUMTEXT" ),
        QStringLiteral( "MEMBER" ),
        QStringLiteral( "MERGE" ),
        QStringLiteral( "METHOD" ),
        QStringLiteral( "MICROSECOND" ),
        QStringLiteral( "MICROSECONDS" ),
        QStringLiteral( "MIDDLEINT" ),
        QStringLiteral( "MIN" ),
        QStringLiteral( "MINDEX" ),
        QStringLiteral( "MINIMUM" ),
        QStringLiteral( "MINUS" ),
        QStringLiteral( "MINUTE" ),
        QStringLiteral( "MINUTE_MICROSECOND" ),
        QStringLiteral( "MINUTES" ),
        QStringLiteral( "MINUTE_SECOND" ),
        QStringLiteral( "MLINREG" ),
        QStringLiteral( "MLOAD" ),
        QStringLiteral( "MLSLABEL" ),
        QStringLiteral( "MOD" ),
        QStringLiteral( "MODE" ),
        QStringLiteral( "MODIFIES" ),
        QStringLiteral( "MODIFY" ),
        QStringLiteral( "MODULE" ),
        QStringLiteral( "MONITOR" ),
        QStringLiteral( "MONRESOURCE" ),
        QStringLiteral( "MONSESSION" ),
        QStringLiteral( "MONTH" ),
        QStringLiteral( "MONTHS" ),
        QStringLiteral( "MSUBSTR" ),
        QStringLiteral( "MSUM" ),
        QStringLiteral( "MULTISET" ),
        QStringLiteral( "NAMED" ),
        QStringLiteral( "NAMES" ),
        QStringLiteral( "NATIONAL" ),
        QStringLiteral( "NATURAL" ),
        QStringLiteral( "NCHAR" ),
        QStringLiteral( "NCLOB" ),
        QStringLiteral( "NE" ),
        QStringLiteral( "NESTED_TABLE_ID" ),
        QStringLiteral( "NEW" ),
        QStringLiteral( "NEW_TABLE" ),
        QStringLiteral( "NEXT" ),
        QStringLiteral( "NEXTVAL" ),
        QStringLiteral( "NO" ),
        QStringLiteral( "NOAUDIT" ),
        QStringLiteral( "NOCHECK" ),
        QStringLiteral( "NOCOMPRESS" ),
        QStringLiteral( "NONCLUSTERED" ),
        QStringLiteral( "NONE" ),
        QStringLiteral( "NORMALIZE" ),
        QStringLiteral( "NOT" ),
        QStringLiteral( "NOTNULL" ),
        QStringLiteral( "NOWAIT" ),
        QStringLiteral( "NO_WRITE_TO_BINLOG" ),
        QStringLiteral( "NTH_VALUE" ),
        QStringLiteral( "NTILE" ),
        QStringLiteral( "NULL" ),
        QStringLiteral( "NULLIF" ),
        QStringLiteral( "NULLIFZERO" ),
        QStringLiteral( "NULLS" ),
        QStringLiteral( "NUMBER" ),
        QStringLiteral( "NUMERIC" ),
        QStringLiteral( "NUMPARTS" ),
        QStringLiteral( "OBID" ),
        QStringLiteral( "OBJECT" ),
        QStringLiteral( "OBJECTS" ),
        QStringLiteral( "OCCURRENCES_REGEX" ),
        QStringLiteral( "OCTET_LENGTH" ),
        QStringLiteral( "OF" ),
        QStringLiteral( "OFF" ),
        QStringLiteral( "OFFLINE" ),
        QStringLiteral( "OFFSET" ),
        QStringLiteral( "OFFSETS" ),
        QStringLiteral( "OLD" ),
        QStringLiteral( "OLD_TABLE" ),
        QStringLiteral( "OMIT" ),
        QStringLiteral( "ON" ),
        QStringLiteral( "ONE" ),
        QStringLiteral( "ONLINE" ),
        QStringLiteral( "ONLY" ),
        QStringLiteral( "OPEN" ),
        QStringLiteral( "OPENDATASOURCE" ),
        QStringLiteral( "OPENQUERY" ),
        QStringLiteral( "OPENROWSET" ),
        QStringLiteral( "OPENXML" ),
        QStringLiteral( "OPERATION" ),
        QStringLiteral( "OPTIMIZATION" ),
        QStringLiteral( "OPTIMIZE" ),
        QStringLiteral( "OPTIMIZER_COSTS" ),
        QStringLiteral( "OPTION" ),
        QStringLiteral( "OPTIONALLY" ),
        QStringLiteral( "OR" ),
        QStringLiteral( "ORDER" ),
        QStringLiteral( "ORDINALITY" ),
        QStringLiteral( "ORGANIZATION" ),
        QStringLiteral( "OUT" ),
        QStringLiteral( "OUTER" ),
        QStringLiteral( "OUTFILE" ),
        QStringLiteral( "OUTPUT" ),
        QStringLiteral( "OVER" ),
        QStringLiteral( "OVERLAPS" ),
        QStringLiteral( "OVERLAY" ),
        QStringLiteral( "OVERRIDE" ),
        QStringLiteral( "PACKAGE" ),
        QStringLiteral( "PAD" ),
        QStringLiteral( "PADDED" ),
        QStringLiteral( "PARAMETER" ),
        QStringLiteral( "PARAMETERS" ),
        QStringLiteral( "PART" ),
        QStringLiteral( "PARTIAL" ),
        QStringLiteral( "PARTITION" ),
        QStringLiteral( "PARTITIONED" ),
        QStringLiteral( "PARTITIONING" ),
        QStringLiteral( "PASSWORD" ),
        QStringLiteral( "PATH" ),
        QStringLiteral( "PATTERN" ),
        QStringLiteral( "PCTFREE" ),
        QStringLiteral( "PER" ),
        QStringLiteral( "PERCENT" ),
        QStringLiteral( "PERCENTILE_CONT" ),
        QStringLiteral( "PERCENTILE_DISC" ),
        QStringLiteral( "PERCENT_RANK" ),
        QStringLiteral( "PERIOD" ),
        QStringLiteral( "PERM" ),
        QStringLiteral( "PERMANENT" ),
        QStringLiteral( "PIECESIZE" ),
        QStringLiteral( "PIVOT" ),
        QStringLiteral( "PLACING" ),
        QStringLiteral( "PLAN" ),
        QStringLiteral( "PORTION" ),
        QStringLiteral( "POSITION" ),
        QStringLiteral( "POSITION_REGEX" ),
        QStringLiteral( "POSTFIX" ),
        QStringLiteral( "POWER" ),
        QStringLiteral( "PRECEDES" ),
        QStringLiteral( "PRECISION" ),
        QStringLiteral( "PREFIX" ),
        QStringLiteral( "PREORDER" ),
        QStringLiteral( "PREPARE" ),
        QStringLiteral( "PRESERVE" ),
        QStringLiteral( "PREVVAL" ),
        QStringLiteral( "PRIMARY" ),
        QStringLiteral( "PRINT" ),
        QStringLiteral( "PRIOR" ),
        QStringLiteral( "PRIQTY" ),
        QStringLiteral( "PRIVATE" ),
        QStringLiteral( "PRIVILEGES" ),
        QStringLiteral( "PROC" ),
        QStringLiteral( "PROCEDURE" ),
        QStringLiteral( "PROFILE" ),
        QStringLiteral( "PROGRAM" ),
        QStringLiteral( "PROPORTIONAL" ),
        QStringLiteral( "PROTECTION" ),
        QStringLiteral( "PSID" ),
        QStringLiteral( "PTF" ),
        QStringLiteral( "PUBLIC" ),
        QStringLiteral( "PURGE" ),
        QStringLiteral( "QUALIFIED" ),
        QStringLiteral( "QUALIFY" ),
        QStringLiteral( "QUANTILE" ),
        QStringLiteral( "QUERY" ),
        QStringLiteral( "QUERYNO" ),
        QStringLiteral( "RADIANS" ),
        QStringLiteral( "RAISERROR" ),
        QStringLiteral( "RANDOM" ),
        QStringLiteral( "RANGE" ),
        QStringLiteral( "RANGE_N" ),
        QStringLiteral( "RANK" ),
        QStringLiteral( "RAW" ),
        QStringLiteral( "READ" ),
        QStringLiteral( "READS" ),
        QStringLiteral( "READTEXT" ),
        QStringLiteral( "READ_WRITE" ),
        QStringLiteral( "REAL" ),
        QStringLiteral( "RECONFIGURE" ),
        QStringLiteral( "RECURSIVE" ),
        QStringLiteral( "REF" ),
        QStringLiteral( "REFERENCES" ),
        QStringLiteral( "REFERENCING" ),
        QStringLiteral( "REFRESH" ),
        QStringLiteral( "REGEXP" ),
        QStringLiteral( "REGR_AVGX" ),
        QStringLiteral( "REGR_AVGY" ),
        QStringLiteral( "REGR_COUNT" ),
        QStringLiteral( "REGR_INTERCEPT" ),
        QStringLiteral( "REGR_R2" ),
        QStringLiteral( "REGR_SLOPE" ),
        QStringLiteral( "REGR_SXX" ),
        QStringLiteral( "REGR_SXY" ),
        QStringLiteral( "REGR_SYY" ),
        QStringLiteral( "RELATIVE" ),
        QStringLiteral( "RELEASE" ),
        QStringLiteral( "RENAME" ),
        QStringLiteral( "REPEAT" ),
        QStringLiteral( "REPLACE" ),
        QStringLiteral( "REPLICATION" ),
        QStringLiteral( "REPOVERRIDE" ),
        QStringLiteral( "REQUEST" ),
        QStringLiteral( "REQUIRE" ),
        QStringLiteral( "RESIGNAL" ),
        QStringLiteral( "RESOURCE" ),
        QStringLiteral( "RESTART" ),
        QStringLiteral( "RESTORE" ),
        QStringLiteral( "RESTRICT" ),
        QStringLiteral( "RESULT" ),
        QStringLiteral( "RESULT_SET_LOCATOR" ),
        QStringLiteral( "RESUME" ),
        QStringLiteral( "RET" ),
        QStringLiteral( "RETRIEVE" ),
        QStringLiteral( "RETURN" ),
        QStringLiteral( "RETURNING" ),
        QStringLiteral( "RETURNS" ),
        QStringLiteral( "REVALIDATE" ),
        QStringLiteral( "REVERT" ),
        QStringLiteral( "REVOKE" ),
        QStringLiteral( "RIGHT" ),
        QStringLiteral( "RIGHTS" ),
        QStringLiteral( "RLIKE" ),
        QStringLiteral( "ROLE" ),
        QStringLiteral( "ROLLBACK" ),
        QStringLiteral( "ROLLFORWARD" ),
        QStringLiteral( "ROLLUP" ),
        QStringLiteral( "ROUND_CEILING" ),
        QStringLiteral( "ROUND_DOWN" ),
        QStringLiteral( "ROUND_FLOOR" ),
        QStringLiteral( "ROUND_HALF_DOWN" ),
        QStringLiteral( "ROUND_HALF_EVEN" ),
        QStringLiteral( "ROUND_HALF_UP" ),
        QStringLiteral( "ROUND_UP" ),
        QStringLiteral( "ROUTINE" ),
        QStringLiteral( "ROW" ),
        QStringLiteral( "ROWCOUNT" ),
        QStringLiteral( "ROWGUIDCOL" ),
        QStringLiteral( "ROWID" ),
        QStringLiteral( "ROWNUM" ),
        QStringLiteral( "ROW_NUMBER" ),
        QStringLiteral( "ROWS" ),
        QStringLiteral( "ROWSET" ),
        QStringLiteral( "RULE" ),
        QStringLiteral( "RUN" ),
        QStringLiteral( "RUNNING" ),
        QStringLiteral( "SAMPLE" ),
        QStringLiteral( "SAMPLEID" ),
        QStringLiteral( "SAVE" ),
        QStringLiteral( "SAVEPOINT" ),
        QStringLiteral( "SCHEMA" ),
        QStringLiteral( "SCHEMAS" ),
        QStringLiteral( "SCOPE" ),
        QStringLiteral( "SCRATCHPAD" ),
        QStringLiteral( "SCROLL" ),
        QStringLiteral( "SEARCH" ),
        QStringLiteral( "SECOND" ),
        QStringLiteral( "SECOND_MICROSECOND" ),
        QStringLiteral( "SECONDS" ),
        QStringLiteral( "SECQTY" ),
        QStringLiteral( "SECTION" ),
        QStringLiteral( "SECURITY" ),
        QStringLiteral( "SECURITYAUDIT" ),
        QStringLiteral( "SEEK" ),
        QStringLiteral( "SEL" ),
        QStringLiteral( "SELECT" ),
        QStringLiteral( "SEMANTICKEYPHRASETABLE" ),
        QStringLiteral( "SEMANTICSIMILARITYDETAILSTABLE" ),
        QStringLiteral( "SEMANTICSIMILARITYTABLE" ),
        QStringLiteral( "SENSITIVE" ),
        QStringLiteral( "SEPARATOR" ),
        QStringLiteral( "SEQUENCE" ),
        QStringLiteral( "SESSION" ),
        QStringLiteral( "SESSION_USER" ),
        QStringLiteral( "SET" ),
        QStringLiteral( "SETRESRATE" ),
        QStringLiteral( "SETS" ),
        QStringLiteral( "SETSESSRATE" ),
        QStringLiteral( "SETUSER" ),
        QStringLiteral( "SHARE" ),
        QStringLiteral( "SHOW" ),
        QStringLiteral( "SHUTDOWN" ),
        QStringLiteral( "SIGNAL" ),
        QStringLiteral( "SIMILAR" ),
        QStringLiteral( "SIMPLE" ),
        QStringLiteral( "SIN" ),
        QStringLiteral( "SINH" ),
        QStringLiteral( "SIZE" ),
        QStringLiteral( "SKEW" ),
        QStringLiteral( "SKIP" ),
        QStringLiteral( "SMALLINT" ),
        QStringLiteral( "SOME" ),
        QStringLiteral( "SOUNDEX" ),
        QStringLiteral( "SOURCE" ),
        QStringLiteral( "SPACE" ),
        QStringLiteral( "SPATIAL" ),
        QStringLiteral( "SPECIFIC" ),
        QStringLiteral( "SPECIFICTYPE" ),
        QStringLiteral( "SPOOL" ),
        QStringLiteral( "SQL" ),
        QStringLiteral( "SQL_BIG_RESULT" ),
        QStringLiteral( "SQL_CALC_FOUND_ROWS" ),
        QStringLiteral( "SQLEXCEPTION" ),
        QStringLiteral( "SQL_SMALL_RESULT" ),
        QStringLiteral( "SQLSTATE" ),
        QStringLiteral( "SQLTEXT" ),
        QStringLiteral( "SQLWARNING" ),
        QStringLiteral( "SQRT" ),
        QStringLiteral( "SS" ),
        QStringLiteral( "SSL" ),
        QStringLiteral( "STANDARD" ),
        QStringLiteral( "START" ),
        QStringLiteral( "STARTING" ),
        QStringLiteral( "STARTUP" ),
        QStringLiteral( "STATE" ),
        QStringLiteral( "STATEMENT" ),
        QStringLiteral( "STATIC" ),
        QStringLiteral( "STATISTICS" ),
        QStringLiteral( "STAY" ),
        QStringLiteral( "STDDEV_POP" ),
        QStringLiteral( "STDDEV_SAMP" ),
        QStringLiteral( "STEPINFO" ),
        QStringLiteral( "STOGROUP" ),
        QStringLiteral( "STORED" ),
        QStringLiteral( "STORES" ),
        QStringLiteral( "STRAIGHT_JOIN" ),
        QStringLiteral( "STRING_CS" ),
        QStringLiteral( "STRUCTURE" ),
        QStringLiteral( "STYLE" ),
        QStringLiteral( "SUBMULTISET" ),
        QStringLiteral( "SUBSCRIBER" ),
        QStringLiteral( "SUBSET" ),
        QStringLiteral( "SUBSTR" ),
        QStringLiteral( "SUBSTRING" ),
        QStringLiteral( "SUBSTRING_REGEX" ),
        QStringLiteral( "SUCCEEDS" ),
        QStringLiteral( "SUCCESSFUL" ),
        QStringLiteral( "SUM" ),
        QStringLiteral( "SUMMARY" ),
        QStringLiteral( "SUSPEND" ),
        QStringLiteral( "SYMMETRIC" ),
        QStringLiteral( "SYNONYM" ),
        QStringLiteral( "SYSDATE" ),
        QStringLiteral( "SYSTEM" ),
        QStringLiteral( "SYSTEM_TIME" ),
        QStringLiteral( "SYSTEM_USER" ),
        QStringLiteral( "SYSTIMESTAMP" ),
        QStringLiteral( "TABLE" ),
        QStringLiteral( "TABLESAMPLE" ),
        QStringLiteral( "TABLESPACE" ),
        QStringLiteral( "TAN" ),
        QStringLiteral( "TANH" ),
        QStringLiteral( "TBL_CS" ),
        QStringLiteral( "TEMPORARY" ),
        QStringLiteral( "TERMINATE" ),
        QStringLiteral( "TERMINATED" ),
        QStringLiteral( "TEXTSIZE" ),
        QStringLiteral( "THAN" ),
        QStringLiteral( "THEN" ),
        QStringLiteral( "THRESHOLD" ),
        QStringLiteral( "TIME" ),
        QStringLiteral( "TIMESTAMP" ),
        QStringLiteral( "TIMEZONE_HOUR" ),
        QStringLiteral( "TIMEZONE_MINUTE" ),
        QStringLiteral( "TINYBLOB" ),
        QStringLiteral( "TINYINT" ),
        QStringLiteral( "TINYTEXT" ),
        QStringLiteral( "TITLE" ),
        QStringLiteral( "TO" ),
        QStringLiteral( "TOP" ),
        QStringLiteral( "TRACE" ),
        QStringLiteral( "TRAILING" ),
        QStringLiteral( "TRAN" ),
        QStringLiteral( "TRANSACTION" ),
        QStringLiteral( "TRANSLATE" ),
        QStringLiteral( "TRANSLATE_CHK" ),
        QStringLiteral( "TRANSLATE_REGEX" ),
        QStringLiteral( "TRANSLATION" ),
        QStringLiteral( "TREAT" ),
        QStringLiteral( "TRIGGER" ),
        QStringLiteral( "TRIM" ),
        QStringLiteral( "TRIM_ARRAY" ),
        QStringLiteral( "TRUE" ),
        QStringLiteral( "TRUNCATE" ),
        QStringLiteral( "TRY_CONVERT" ),
        QStringLiteral( "TSEQUAL" ),
        QStringLiteral( "TYPE" ),
        QStringLiteral( "UC" ),
        QStringLiteral( "UESCAPE" ),
        QStringLiteral( "UID" ),
        QStringLiteral( "UNDEFINED" ),
        QStringLiteral( "UNDER" ),
        QStringLiteral( "UNDO" ),
        QStringLiteral( "UNION" ),
        QStringLiteral( "UNIQUE" ),
        QStringLiteral( "UNKNOWN" ),
        QStringLiteral( "UNLOCK" ),
        QStringLiteral( "UNNEST" ),
        QStringLiteral( "UNPIVOT" ),
        QStringLiteral( "UNSIGNED" ),
        QStringLiteral( "UNTIL" ),
        QStringLiteral( "UPD" ),
        QStringLiteral( "UPDATE" ),
        QStringLiteral( "UPDATETEXT" ),
        QStringLiteral( "UPPER" ),
        QStringLiteral( "UPPERCASE" ),
        QStringLiteral( "USAGE" ),
        QStringLiteral( "USE" ),
        QStringLiteral( "USER" ),
        QStringLiteral( "USING" ),
        QStringLiteral( "UTC_DATE" ),
        QStringLiteral( "UTC_TIME" ),
        QStringLiteral( "UTC_TIMESTAMP" ),
        QStringLiteral( "VALIDATE" ),
        QStringLiteral( "VALIDPROC" ),
        QStringLiteral( "VALUE" ),
        QStringLiteral( "VALUE_OF" ),
        QStringLiteral( "VALUES" ),
        QStringLiteral( "VARBINARY" ),
        QStringLiteral( "VARBYTE" ),
        QStringLiteral( "VARCHAR" ),
        QStringLiteral( "VARCHAR2" ),
        QStringLiteral( "VARCHARACTER" ),
        QStringLiteral( "VARGRAPHIC" ),
        QStringLiteral( "VARIABLE" ),
        QStringLiteral( "VARIADIC" ),
        QStringLiteral( "VARIANT" ),
        QStringLiteral( "VAR_POP" ),
        QStringLiteral( "VAR_SAMP" ),
        QStringLiteral( "VARYING" ),
        QStringLiteral( "VCAT" ),
        QStringLiteral( "VERBOSE" ),
        QStringLiteral( "VERSIONING" ),
        QStringLiteral( "VIEW" ),
        QStringLiteral( "VIRTUAL" ),
        QStringLiteral( "VOLATILE" ),
        QStringLiteral( "VOLUMES" ),
        QStringLiteral( "WAIT" ),
        QStringLiteral( "WAITFOR" ),
        QStringLiteral( "WHEN" ),
        QStringLiteral( "WHENEVER" ),
        QStringLiteral( "WHERE" ),
        QStringLiteral( "WHILE" ),
        QStringLiteral( "WIDTH_BUCKET" ),
        QStringLiteral( "WINDOW" ),
        QStringLiteral( "WITH" ),
        QStringLiteral( "WITHIN" ),
        QStringLiteral( "WITHIN_GROUP" ),
        QStringLiteral( "WITHOUT" ),
        QStringLiteral( "WLM" ),
        QStringLiteral( "WORK" ),
        QStringLiteral( "WRITE" ),
        QStringLiteral( "WRITETEXT" ),
        QStringLiteral( "XMLCAST" ),
        QStringLiteral( "XMLEXISTS" ),
        QStringLiteral( "XMLNAMESPACES" ),
        QStringLiteral( "XOR" ),
        QStringLiteral( "YEAR" ),
        QStringLiteral( "YEAR_MONTH" ),
        QStringLiteral( "YEARS" ),
        QStringLiteral( "ZEROFILL" ),
        QStringLiteral( "ZEROIFNULL" ),
        QStringLiteral( "ZONE" ),
      }
    }
  };
}

void QgsAbstractDatabaseProviderConnection::createVectorTable( const QString &schema,
    const QString &name,
    const QgsFields &fields,
    QgsWkbTypes::Type wkbType,
    const QgsCoordinateReferenceSystem &srs,
    bool overwrite,
    const QMap<QString, QVariant> *
    options ) const
{
  Q_UNUSED( schema );
  Q_UNUSED( name );
  Q_UNUSED( fields );
  Q_UNUSED( srs );
  Q_UNUSED( overwrite );
  Q_UNUSED( options );
  Q_UNUSED( wkbType );
  throw QgsProviderConnectionException( QObject::tr( "Operation 'createVectorTable' is not supported" ) );
}

void QgsAbstractDatabaseProviderConnection::renameVectorTable( const QString &, const QString &, const QString & ) const
{
  checkCapability( Capability::RenameVectorTable );
}


QgsAbstractDatabaseProviderConnection::SqlVectorLayerOptions QgsAbstractDatabaseProviderConnection::sqlOptions( const QString & )
{
  checkCapability( Capability::SqlLayers );
  return SqlVectorLayerOptions();
}

void QgsAbstractDatabaseProviderConnection::renameRasterTable( const QString &, const QString &, const QString & ) const
{
  checkCapability( Capability::RenameRasterTable );
}

void QgsAbstractDatabaseProviderConnection::dropVectorTable( const QString &, const QString & ) const
{
  checkCapability( Capability::DropVectorTable );
}

bool QgsAbstractDatabaseProviderConnection::tableExists( const QString &schema, const QString &name ) const
{
  checkCapability( Capability::TableExists );
  const QList<QgsAbstractDatabaseProviderConnection::TableProperty> constTables { tables( schema ) };
  for ( const auto &t : constTables )
  {
    if ( t.tableName() == name )
    {
      return true;
    }
  }
  return false;
}

void QgsAbstractDatabaseProviderConnection::dropRasterTable( const QString &, const QString & ) const
{
  checkCapability( Capability::DropRasterTable );
}

void QgsAbstractDatabaseProviderConnection::createSchema( const QString & ) const
{
  checkCapability( Capability::CreateSchema );
}

void QgsAbstractDatabaseProviderConnection::dropSchema( const QString &, bool ) const
{
  checkCapability( Capability::DropSchema );
}

void QgsAbstractDatabaseProviderConnection::renameSchema( const QString &, const QString & ) const
{
  checkCapability( Capability::RenameSchema );
}

QList<QList<QVariant>> QgsAbstractDatabaseProviderConnection::executeSql( const QString &sql, QgsFeedback *feedback ) const
{
  return execSql( sql, feedback ).rows();
}


QgsAbstractDatabaseProviderConnection::QueryResult QgsAbstractDatabaseProviderConnection::execSql( const QString &, QgsFeedback * ) const
{
  checkCapability( Capability::ExecuteSql );
  return QueryResult();
}


void QgsAbstractDatabaseProviderConnection::vacuum( const QString &, const QString & ) const
{
  checkCapability( Capability::Vacuum );
}

void QgsAbstractDatabaseProviderConnection::createSpatialIndex( const QString &, const QString &, const QgsAbstractDatabaseProviderConnection::SpatialIndexOptions & ) const
{
  checkCapability( Capability::CreateSpatialIndex );
}

QgsVectorLayer *QgsAbstractDatabaseProviderConnection::createSqlVectorLayer( const SqlVectorLayerOptions & ) const
{
  checkCapability( Capability::SqlLayers );
  return nullptr;
}

void QgsAbstractDatabaseProviderConnection::deleteSpatialIndex( const QString &, const QString &, const QString & ) const
{
  checkCapability( Capability::DeleteSpatialIndex );
}

bool QgsAbstractDatabaseProviderConnection::spatialIndexExists( const QString &, const QString &, const QString & ) const
{
  checkCapability( Capability::SpatialIndexExists );
  return false;
}

void QgsAbstractDatabaseProviderConnection::deleteField( const QString &fieldName, const QString &schema, const QString &tableName, bool ) const
{
  checkCapability( Capability::DeleteField );

  QgsVectorLayer::LayerOptions options { false, false };
  options.skipCrsValidation = true;
  std::unique_ptr<QgsVectorLayer> vl { std::make_unique<QgsVectorLayer>( tableUri( schema, tableName ), QStringLiteral( "temp_layer" ), mProviderKey, options ) };
  if ( ! vl->isValid() )
  {
    throw QgsProviderConnectionException( QObject::tr( "Could not create a vector layer for table '%1' in schema '%2'" )
                                          .arg( tableName, schema ) );
  }
  if ( vl->fields().lookupField( fieldName ) == -1 )
  {
    throw QgsProviderConnectionException( QObject::tr( "Could not find field '%1' in table '%2' in schema '%3'" )
                                          .arg( fieldName, tableName, schema ) );

  }
  if ( ! vl->dataProvider()->deleteAttributes( { vl->fields().lookupField( fieldName ) } ) )
  {
    throw QgsProviderConnectionException( QObject::tr( "Unknown error deleting field '%1' in table '%2' in schema '%3'" )
                                          .arg( fieldName, tableName, schema ) );
  }
}

void QgsAbstractDatabaseProviderConnection::addField( const QgsField &field, const QString &schema, const QString &tableName ) const
{
  checkCapability( Capability::AddField );

  QgsVectorLayer::LayerOptions options { false, false };
  options.skipCrsValidation = true;
  std::unique_ptr<QgsVectorLayer> vl( std::make_unique<QgsVectorLayer>( tableUri( schema, tableName ), QStringLiteral( "temp_layer" ), mProviderKey, options ) );
  if ( ! vl->isValid() )
  {
    throw QgsProviderConnectionException( QObject::tr( "Could not create a vector layer for table '%1' in schema '%2'" )
                                          .arg( tableName, schema ) );
  }
  if ( vl->fields().lookupField( field.name() ) != -1 )
  {
    throw QgsProviderConnectionException( QObject::tr( "Field '%1' in table '%2' in schema '%3' already exists" )
                                          .arg( field.name(), tableName, schema ) );

  }
  if ( ! vl->dataProvider()->addAttributes( { field  } ) )
  {
    throw QgsProviderConnectionException( QObject::tr( "Unknown error adding field '%1' in table '%2' in schema '%3'" )
                                          .arg( field.name(), tableName, schema ) );
  }
}

QList<QgsAbstractDatabaseProviderConnection::TableProperty> QgsAbstractDatabaseProviderConnection::tables( const QString &, const QgsAbstractDatabaseProviderConnection::TableFlags & ) const
{
  checkCapability( Capability::Tables );
  return QList<QgsAbstractDatabaseProviderConnection::TableProperty>();
}


QgsAbstractDatabaseProviderConnection::TableProperty QgsAbstractDatabaseProviderConnection::table( const QString &schema, const QString &name ) const
{
  checkCapability( Capability::Tables );
  const QList<QgsAbstractDatabaseProviderConnection::TableProperty> constTables { tables( schema ) };
  for ( const auto &t : constTables )
  {
    if ( t.tableName() == name )
    {
      return t;
    }
  }
  throw QgsProviderConnectionException( QObject::tr( "Table '%1' was not found in schema '%2'" )
                                        .arg( name, schema ) );
}

QList<QgsAbstractDatabaseProviderConnection::TableProperty> QgsAbstractDatabaseProviderConnection::tablesInt( const QString &schema, const int flags ) const
{
  return tables( schema, static_cast<QgsAbstractDatabaseProviderConnection::TableFlags>( flags ) );
}


QStringList QgsAbstractDatabaseProviderConnection::schemas( ) const
{
  checkCapability( Capability::Schemas );
  return QStringList();
}

QString QgsAbstractDatabaseProviderConnection::TableProperty::tableName() const
{
  return mTableName;
}

void QgsAbstractDatabaseProviderConnection::TableProperty::setTableName( const QString &name )
{
  mTableName = name;
}

void QgsAbstractDatabaseProviderConnection::TableProperty::addGeometryColumnType( const QgsWkbTypes::Type &type, const QgsCoordinateReferenceSystem &crs )
{
  // Do not add the type if it's already present
  const QgsAbstractDatabaseProviderConnection::TableProperty::GeometryColumnType toAdd { type, crs };
  for ( const auto &t : std::as_const( mGeometryColumnTypes ) )
  {
    if ( t == toAdd )
    {
      return;
    }
  }
  mGeometryColumnTypes.push_back( toAdd );
}

QList<QgsAbstractDatabaseProviderConnection::TableProperty::GeometryColumnType> QgsAbstractDatabaseProviderConnection::TableProperty::geometryColumnTypes() const
{
  return mGeometryColumnTypes;
}


QgsFields QgsAbstractDatabaseProviderConnection::fields( const QString &schema, const QString &tableName ) const
{
  QgsVectorLayer::LayerOptions options { false, true };
  options.skipCrsValidation = true;
  QgsVectorLayer vl { tableUri( schema, tableName ), QStringLiteral( "temp_layer" ), mProviderKey, options };
  if ( vl.isValid() )
  {
    // Note: this implementation works for providers that do not hide any "special" field (geometry or PKs)
    return vl.fields();
  }
  else
  {
    throw QgsProviderConnectionException( QObject::tr( "Error retrieving fields information for uri: %1" ).arg( vl.publicSource() ) );
  }
}

QStringList QgsAbstractDatabaseProviderConnection::fieldDomainNames() const
{
  checkCapability( Capability::ListFieldDomains );
  return QStringList();
}

QgsFieldDomain *QgsAbstractDatabaseProviderConnection::fieldDomain( const QString & ) const
{
  checkCapability( Capability::RetrieveFieldDomain );
  return nullptr;
}

void QgsAbstractDatabaseProviderConnection::setFieldDomainName( const QString &, const QString &, const QString &, const QString & ) const
{
  checkCapability( Capability::SetFieldDomain );
}

void QgsAbstractDatabaseProviderConnection::addFieldDomain( const QgsFieldDomain &, const QString & ) const
{
  checkCapability( Capability::AddFieldDomain );
}

QString QgsAbstractDatabaseProviderConnection::TableProperty::defaultName() const
{
  QString n = mTableName;
  if ( mGeometryColumnCount > 1 ) n += '.' + mGeometryColumn;
  return n;
}

QgsAbstractDatabaseProviderConnection::TableProperty QgsAbstractDatabaseProviderConnection::TableProperty::at( int index ) const
{
  TableProperty property;

  Q_ASSERT( index >= 0 && index < mGeometryColumnTypes.size() );

  property.mGeometryColumnTypes << mGeometryColumnTypes[ index ];
  property.mSchema = mSchema;
  property.mTableName = mTableName;
  property.mGeometryColumn = mGeometryColumn;
  property.mPkColumns = mPkColumns;
  property.mGeometryColumnCount = mGeometryColumnCount;
  property.mFlags = mFlags;
  property.mComment = mComment;
  property.mInfo = mInfo;
  return property;
}

void QgsAbstractDatabaseProviderConnection::TableProperty::setFlag( const QgsAbstractDatabaseProviderConnection::TableFlag &flag )
{
  mFlags.setFlag( flag );
}

int QgsAbstractDatabaseProviderConnection::TableProperty::maxCoordinateDimensions() const
{
  int res = 0;
  for ( const TableProperty::GeometryColumnType &ct : std::as_const( mGeometryColumnTypes ) )
  {
    res = std::max( res, QgsWkbTypes::coordDimensions( ct.wkbType ) );
  }
  return res;
}

bool QgsAbstractDatabaseProviderConnection::TableProperty::operator==( const QgsAbstractDatabaseProviderConnection::TableProperty &other ) const
{
  return mSchema == other.mSchema &&
         mTableName == other.mTableName &&
         mGeometryColumn == other.mGeometryColumn &&
         mGeometryColumnCount == other.mGeometryColumnCount &&
         mPkColumns == other.mPkColumns &&
         mFlags == other.mFlags &&
         mComment == other.mComment &&
         mInfo == other.mInfo;
}


void QgsAbstractDatabaseProviderConnection::TableProperty::setGeometryColumnTypes( const QList<QgsAbstractDatabaseProviderConnection::TableProperty::GeometryColumnType> &columnTypes )
{
  mGeometryColumnTypes = columnTypes;
}


int QgsAbstractDatabaseProviderConnection::TableProperty::geometryColumnCount() const
{
  return mGeometryColumnCount;
}

void QgsAbstractDatabaseProviderConnection::TableProperty::setGeometryColumnCount( int geometryColumnCount )
{
  mGeometryColumnCount = geometryColumnCount;
}

QVariantMap QgsAbstractDatabaseProviderConnection::TableProperty::info() const
{
  return mInfo;
}

void QgsAbstractDatabaseProviderConnection::TableProperty::setInfo( const QVariantMap &info )
{
  mInfo = info;
}

QString QgsAbstractDatabaseProviderConnection::TableProperty::comment() const
{
  return mComment;
}

void QgsAbstractDatabaseProviderConnection::TableProperty::setComment( const QString &comment )
{
  mComment = comment;
}

QgsAbstractDatabaseProviderConnection::TableFlags QgsAbstractDatabaseProviderConnection::TableProperty::flags() const
{
  return mFlags;
}

void QgsAbstractDatabaseProviderConnection::TableProperty::setFlags( const QgsAbstractDatabaseProviderConnection::TableFlags &flags )
{
  mFlags = flags;
}

QList<QgsCoordinateReferenceSystem> QgsAbstractDatabaseProviderConnection::TableProperty::crsList() const
{
  QList<QgsCoordinateReferenceSystem> crss;
  for ( const auto &t : std::as_const( mGeometryColumnTypes ) )
  {
    crss.push_back( t.crs );
  }
  return crss;
}

QStringList QgsAbstractDatabaseProviderConnection::TableProperty::primaryKeyColumns() const
{
  return mPkColumns;
}

void QgsAbstractDatabaseProviderConnection::TableProperty::setPrimaryKeyColumns( const QStringList &pkColumns )
{
  mPkColumns = pkColumns;
}

QString QgsAbstractDatabaseProviderConnection::TableProperty::geometryColumn() const
{
  return mGeometryColumn;
}

void QgsAbstractDatabaseProviderConnection::TableProperty::setGeometryColumn( const QString &geometryColumn )
{
  mGeometryColumn = geometryColumn;
}

QString QgsAbstractDatabaseProviderConnection::TableProperty::schema() const
{
  return mSchema;
}

void QgsAbstractDatabaseProviderConnection::TableProperty::setSchema( const QString &schema )
{
  mSchema = schema;
}


///@cond PRIVATE

QStringList QgsAbstractDatabaseProviderConnection::QueryResult::columns() const
{
  return mColumns;
}

QList<QList<QVariant> > QgsAbstractDatabaseProviderConnection::QueryResult::rows( QgsFeedback *feedback )
{

  QList<QList<QVariant> > rows;

  while ( mResultIterator &&
          mResultIterator->hasNextRow() &&
          ( ! feedback || ! feedback->isCanceled() ) )
  {
    const QVariantList row( mResultIterator->nextRow() );
    if ( row.isEmpty() )
    {
      break;
    }
    else
    {
      rows.push_back( row );
    }
  }
  return rows;
}

QList<QVariant> QgsAbstractDatabaseProviderConnection::QueryResult::nextRow() const
{
  if ( ! mResultIterator )
  {
    return QList<QVariant>();
  }
  return mResultIterator->nextRow();
}


long long QgsAbstractDatabaseProviderConnection::QueryResult::fetchedRowCount() const
{
  if ( ! mResultIterator )
  {
    return 0;
  }
  return mResultIterator->fetchedRowCount();
}

long long QgsAbstractDatabaseProviderConnection::QueryResult::rowCount() const
{
  if ( ! mResultIterator )
  {
    return static_cast<long long>( Qgis::FeatureCountState::UnknownCount );
  }
  return mResultIterator->rowCount();
}


bool QgsAbstractDatabaseProviderConnection::QueryResult::hasNextRow() const
{
  if ( ! mResultIterator )
  {
    return false;
  }
  return mResultIterator->hasNextRow();
}

void QgsAbstractDatabaseProviderConnection::QueryResult::appendColumn( const QString &columnName )
{
  mColumns.push_back( columnName );
}


QgsAbstractDatabaseProviderConnection::QueryResult::QueryResult( std::shared_ptr<QgsAbstractDatabaseProviderConnection::QueryResult::QueryResultIterator> iterator )
  : mResultIterator( iterator )
{}

double QgsAbstractDatabaseProviderConnection::QueryResult::queryExecutionTime()
{
  return mQueryExecutionTime;
}

void QgsAbstractDatabaseProviderConnection::QueryResult::setQueryExecutionTime( double queryExecutionTime )
{
  mQueryExecutionTime = queryExecutionTime;
}


QVariantList QgsAbstractDatabaseProviderConnection::QueryResult::QueryResultIterator::nextRow()
{
  QMutexLocker lock( &mMutex );
  const QVariantList row = nextRowPrivate();
  if ( ! row.isEmpty() )
  {
    mFetchedRowCount++;
  }
  return row;
}

bool QgsAbstractDatabaseProviderConnection::QueryResult::QueryResultIterator::hasNextRow() const
{
  QMutexLocker lock( &mMutex );
  return hasNextRowPrivate();
}

long long QgsAbstractDatabaseProviderConnection::QueryResult::QueryResultIterator::fetchedRowCount()
{
  QMutexLocker lock( &mMutex );
  return mFetchedRowCount;
}

long long QgsAbstractDatabaseProviderConnection::QueryResult::QueryResultIterator::rowCount()
{
  QMutexLocker lock( &mMutex );
  return rowCountPrivate();
}

///@endcond private

