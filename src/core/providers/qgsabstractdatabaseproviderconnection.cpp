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

#include "qgsexception.h"
#include "qgsfeedback.h"
#include "qgsprovidersqlquerybuilder.h"
#include "qgssqlstatement.h"
#include "qgsvectorlayer.h"
#include "qgsweakrelation.h"

#include <QObject>
#include <QVariant>

#include "moc_qgsabstractdatabaseproviderconnection.cpp"

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

Qgis::DatabaseProviderConnectionCapabilities2 QgsAbstractDatabaseProviderConnection::capabilities2() const
{
  return mCapabilities2;
}

QgsAbstractDatabaseProviderConnection::GeometryColumnCapabilities QgsAbstractDatabaseProviderConnection::geometryColumnCapabilities()
{
  return mGeometryColumnCapabilities;
}

Qgis::SqlLayerDefinitionCapabilities QgsAbstractDatabaseProviderConnection::sqlLayerDefinitionCapabilities()
{
  return mSqlLayerDefinitionCapabilities;
}

QString QgsAbstractDatabaseProviderConnection::createVectorLayerExporterDestinationUri( const VectorLayerExporterOptions &, QVariantMap & ) const
{
  throw QgsProviderConnectionException( QObject::tr( "Operation 'createVectorLayerExporterDestinationUri' is not supported" ) );
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

void QgsAbstractDatabaseProviderConnection::checkCapability( Qgis::DatabaseProviderConnectionCapability2 capability ) const
{
  if ( ! mCapabilities2.testFlag( capability ) )
  {
    throw QgsProviderConnectionException( QObject::tr( "Operation '%1' is not supported for this connection" ).arg( qgsEnumValueToKey( capability ) ) );
  }
}

QString QgsAbstractDatabaseProviderConnection::sanitizeSqlForQueryLayer( const QString &sql ) const
{
  QString sanitizedSql { sql.trimmed() };
  while ( sanitizedSql.endsWith( ';' ) )
  {
    sanitizedSql.chop( 1 );
  }
  return sanitizedSql;
}

///@endcond

QString QgsAbstractDatabaseProviderConnection::providerKey() const
{
  return mProviderKey;
}

QMultiMap<Qgis::SqlKeywordCategory, QStringList> QgsAbstractDatabaseProviderConnection::sqlDictionary()
{
  return
  {
    // Common constants
    {
      Qgis::SqlKeywordCategory::Constant, {
        u"NULL"_s,
        u"FALSE"_s,
        u"TRUE"_s,
      }
    },
    // Common SQL reserved words
    // From: GET https://en.wikipedia.org/wiki/SQL_reserved_words| grep 'style="background: #ececec; color: black; font-weight: bold;'| sed -e 's/.*>//'|sort
    {
      Qgis::SqlKeywordCategory::Keyword,
      {
        u"ABORT "_s,
        u"ABORTSESSION"_s,
        u"ABS"_s,
        u"ABSOLUTE"_s,
        u"ACCESS"_s,
        u"ACCESSIBLE"_s,
        u"ACCESS_LOCK"_s,
        u"ACCOUNT"_s,
        u"ACOS"_s,
        u"ACOSH"_s,
        u"ACTION"_s,
        u"ADD"_s,
        u"ADD_MONTHS"_s,
        u"ADMIN"_s,
        u"AFTER"_s,
        u"AGGREGATE"_s,
        u"ALIAS"_s,
        u"ALL"_s,
        u"ALLOCATE"_s,
        u"ALLOW"_s,
        u"ALTER"_s,
        u"ALTERAND"_s,
        u"AMP"_s,
        u"ANALYSE"_s,
        u"ANALYZE"_s,
        u"AND"_s,
        u"ANSIDATE"_s,
        u"ANY"_s,
        u"ARE"_s,
        u"ARRAY"_s,
        u"ARRAY_AGG"_s,
        u"ARRAY_EXISTS"_s,
        u"ARRAY_MAX_CARDINALITY"_s,
        u"AS"_s,
        u"ASC"_s,
        u"ASENSITIVE"_s,
        u"ASIN"_s,
        u"ASINH"_s,
        u"ASSERTION"_s,
        u"ASSOCIATE"_s,
        u"ASUTIME"_s,
        u"ASYMMETRIC"_s,
        u"AT"_s,
        u"ATAN"_s,
        u"ATAN2"_s,
        u"ATANH"_s,
        u"ATOMIC"_s,
        u"AUDIT"_s,
        u"AUTHORIZATION"_s,
        u"AUX"_s,
        u"AUXILIARY"_s,
        u"AVE"_s,
        u"AVERAGE"_s,
        u"AVG"_s,
        u"BACKUP"_s,
        u"BEFORE"_s,
        u"BEGIN"_s,
        u"BEGIN_FRAME"_s,
        u"BEGIN_PARTITION"_s,
        u"BETWEEN"_s,
        u"BIGINT"_s,
        u"BINARY"_s,
        u"BIT"_s,
        u"BLOB"_s,
        u"BOOLEAN"_s,
        u"BOTH"_s,
        u"BREADTH"_s,
        u"BREAK"_s,
        u"BROWSE"_s,
        u"BT"_s,
        u"BUFFERPOOL"_s,
        u"BULK"_s,
        u"BUT"_s,
        u"BY"_s,
        u"BYTE"_s,
        u"BYTEINT"_s,
        u"BYTES"_s,
        u"CALL"_s,
        u"CALLED"_s,
        u"CAPTURE"_s,
        u"CARDINALITY"_s,
        u"CASCADE"_s,
        u"CASCADED"_s,
        u"CASE"_s,
        u"CASE_N"_s,
        u"CASESPECIFIC"_s,
        u"CAST"_s,
        u"CATALOG"_s,
        u"CCSID"_s,
        u"CD"_s,
        u"CEIL"_s,
        u"CEILING"_s,
        u"CHANGE"_s,
        u"CHAR"_s,
        u"CHAR2HEXINT"_s,
        u"CHARACTER"_s,
        u"CHARACTER_LENGTH"_s,
        u"CHARACTERS"_s,
        u"CHAR_LENGTH"_s,
        u"CHARS"_s,
        u"CHECK"_s,
        u"CHECKPOINT"_s,
        u"CLASS"_s,
        u"CLASSIFIER"_s,
        u"CLOB"_s,
        u"CLONE"_s,
        u"CLOSE"_s,
        u"CLUSTER"_s,
        u"CLUSTERED"_s,
        u"CM"_s,
        u"COALESCE"_s,
        u"COLLATE"_s,
        u"COLLATION"_s,
        u"COLLECT"_s,
        u"COLLECTION"_s,
        u"COLLID"_s,
        u"COLUMN"_s,
        u"COLUMN_VALUE"_s,
        u"COMMENT"_s,
        u"COMMIT"_s,
        u"COMPLETION"_s,
        u"COMPRESS"_s,
        u"COMPUTE"_s,
        u"CONCAT"_s,
        u"CONCURRENTLY"_s,
        u"CONDITION"_s,
        u"CONNECT"_s,
        u"CONNECTION"_s,
        u"CONSTRAINT"_s,
        u"CONSTRAINTS"_s,
        u"CONSTRUCTOR"_s,
        u"CONTAINS"_s,
        u"CONTAINSTABLE"_s,
        u"CONTENT"_s,
        u"CONTINUE"_s,
        u"CONVERT"_s,
        u"CONVERT_TABLE_HEADER"_s,
        u"COPY"_s,
        u"CORR"_s,
        u"CORRESPONDING"_s,
        u"COS"_s,
        u"COSH"_s,
        u"COUNT"_s,
        u"COVAR_POP"_s,
        u"COVAR_SAMP"_s,
        u"CREATE"_s,
        u"CROSS"_s,
        u"CS"_s,
        u"CSUM"_s,
        u"CT"_s,
        u"CUBE"_s,
        u"CUME_DIST"_s,
        u"CURRENT"_s,
        u"CURRENT_CATALOG"_s,
        u"CURRENT_DATE"_s,
        u"CURRENT_DEFAULT_TRANSFORM_GROUP"_s,
        u"CURRENT_LC_CTYPE"_s,
        u"CURRENT_PATH"_s,
        u"CURRENT_ROLE"_s,
        u"CURRENT_ROW"_s,
        u"CURRENT_SCHEMA"_s,
        u"CURRENT_SERVER"_s,
        u"CURRENT_TIME"_s,
        u"CURRENT_TIMESTAMP"_s,
        u"CURRENT_TIMEZONE"_s,
        u"CURRENT_TRANSFORM_GROUP_FOR_TYPE"_s,
        u"CURRENT_USER"_s,
        u"CURRVAL"_s,
        u"CURSOR"_s,
        u"CV"_s,
        u"CYCLE"_s,
        u"DATA"_s,
        u"DATABASE"_s,
        u"DATABASES"_s,
        u"DATABLOCKSIZE"_s,
        u"DATE"_s,
        u"DATEFORM"_s,
        u"DAY"_s,
        u"DAY_HOUR"_s,
        u"DAY_MICROSECOND"_s,
        u"DAY_MINUTE"_s,
        u"DAYS"_s,
        u"DAY_SECOND"_s,
        u"DBCC"_s,
        u"DBINFO"_s,
        u"DEALLOCATE"_s,
        u"DEC"_s,
        u"DECFLOAT"_s,
        u"DECIMAL"_s,
        u"DECLARE"_s,
        u"DEFAULT"_s,
        u"DEFERRABLE"_s,
        u"DEFERRED"_s,
        u"DEFINE"_s,
        u"DEGREES"_s,
        u"DEL"_s,
        u"DELAYED"_s,
        u"DELETE"_s,
        u"DENSE_RANK"_s,
        u"DENY"_s,
        u"DEPTH"_s,
        u"DEREF"_s,
        u"DESC"_s,
        u"DESCRIBE"_s,
        u"DESCRIPTOR"_s,
        u"DESTROY"_s,
        u"DESTRUCTOR"_s,
        u"DETERMINISTIC"_s,
        u"DIAGNOSTIC"_s,
        u"DIAGNOSTICS"_s,
        u"DICTIONARY"_s,
        u"DISABLE"_s,
        u"DISABLED"_s,
        u"DISALLOW"_s,
        u"DISCONNECT"_s,
        u"DISK"_s,
        u"DISTINCT"_s,
        u"DISTINCTROW"_s,
        u"DISTRIBUTED"_s,
        u"DIV"_s,
        u"DO"_s,
        u"DOCUMENT"_s,
        u"DOMAIN"_s,
        u"DOUBLE"_s,
        u"DROP"_s,
        u"DSSIZE"_s,
        u"DUAL"_s,
        u"DUMP"_s,
        u"DYNAMIC"_s,
        u"EACH"_s,
        u"ECHO"_s,
        u"EDITPROC"_s,
        u"ELEMENT"_s,
        u"ELSE"_s,
        u"ELSEIF"_s,
        u"EMPTY"_s,
        u"ENABLED"_s,
        u"ENCLOSED"_s,
        u"ENCODING"_s,
        u"ENCRYPTION"_s,
        u"END"_s,
        u"END-EXEC"_s,
        u"END_FRAME"_s,
        u"ENDING"_s,
        u"END_PARTITION"_s,
        u"EQ"_s,
        u"EQUALS"_s,
        u"ERASE"_s,
        u"ERRLVL"_s,
        u"ERROR"_s,
        u"ERRORFILES"_s,
        u"ERRORTABLES"_s,
        u"ESCAPE"_s,
        u"ESCAPED"_s,
        u"ET"_s,
        u"EVERY"_s,
        u"EXCEPT"_s,
        u"EXCEPTION"_s,
        u"EXCLUSIVE"_s,
        u"EXEC"_s,
        u"EXECUTE"_s,
        u"EXISTS"_s,
        u"EXIT"_s,
        u"EXP"_s,
        u"EXPLAIN"_s,
        u"EXTERNAL"_s,
        u"EXTRACT"_s,
        u"FALLBACK"_s,
        u"FALSE"_s,
        u"FASTEXPORT"_s,
        u"FENCED"_s,
        u"FETCH"_s,
        u"FIELDPROC"_s,
        u"FILE"_s,
        u"FILLFACTOR"_s,
        u"FILTER"_s,
        u"FINAL"_s,
        u"FIRST"_s,
        u"FIRST_VALUE"_s,
        u"FLOAT"_s,
        u"FLOAT4"_s,
        u"FLOAT8"_s,
        u"FLOOR"_s,
        u"FOR"_s,
        u"FORCE"_s,
        u"FOREIGN"_s,
        u"FORMAT"_s,
        u"FOUND"_s,
        u"FRAME_ROW"_s,
        u"FREE"_s,
        u"FREESPACE"_s,
        u"FREETEXT"_s,
        u"FREETEXTTABLE"_s,
        u"FREEZE"_s,
        u"FROM"_s,
        u"FULL"_s,
        u"FULLTEXT"_s,
        u"FUNCTION"_s,
        u"FUSION"_s,
        u"GE"_s,
        u"GENERAL"_s,
        u"GENERATED"_s,
        u"GET"_s,
        u"GIVE"_s,
        u"GLOBAL"_s,
        u"GO"_s,
        u"GOTO"_s,
        u"GRANT"_s,
        u"GRAPHIC"_s,
        u"GROUP"_s,
        u"GROUPING"_s,
        u"GROUPS"_s,
        u"GT"_s,
        u"HANDLER"_s,
        u"HASH"_s,
        u"HASHAMP"_s,
        u"HASHBAKAMP"_s,
        u"HASHBUCKET"_s,
        u"HASHROW"_s,
        u"HAVING"_s,
        u"HELP"_s,
        u"HIGH_PRIORITY"_s,
        u"HOLD"_s,
        u"HOLDLOCK"_s,
        u"HOST"_s,
        u"HOUR"_s,
        u"HOUR_MICROSECOND"_s,
        u"HOUR_MINUTE"_s,
        u"HOURS"_s,
        u"HOUR_SECOND"_s,
        u"IDENTIFIED"_s,
        u"IDENTITY"_s,
        u"IDENTITYCOL"_s,
        u"IDENTITY_INSERT"_s,
        u"IF"_s,
        u"IGNORE"_s,
        u"ILIKE"_s,
        u"IMMEDIATE"_s,
        u"IN"_s,
        u"INCLUSIVE"_s,
        u"INCONSISTENT"_s,
        u"INCREMENT"_s,
        u"INDEX"_s,
        u"INDICATOR"_s,
        u"INFILE"_s,
        u"INHERIT"_s,
        u"INITIAL"_s,
        u"INITIALIZE"_s,
        u"INITIALLY"_s,
        u"INITIATE"_s,
        u"INNER"_s,
        u"INOUT"_s,
        u"INPUT"_s,
        u"INS"_s,
        u"INSENSITIVE"_s,
        u"INSERT"_s,
        u"INSTEAD"_s,
        u"INT"_s,
        u"INT1"_s,
        u"INT2"_s,
        u"INT3"_s,
        u"INT4"_s,
        u"INT8"_s,
        u"INTEGER"_s,
        u"INTEGERDATE"_s,
        u"INTERSECT"_s,
        u"INTERSECTION"_s,
        u"INTERVAL"_s,
        u"INTO"_s,
        u"IO_AFTER_GTIDS"_s,
        u"IO_BEFORE_GTIDS"_s,
        u"IS"_s,
        u"ISNULL"_s,
        u"ISOBID"_s,
        u"ISOLATION"_s,
        u"ITERATE"_s,
        u"JAR"_s,
        u"JOIN"_s,
        u"JOURNAL"_s,
        u"JSON_ARRAY"_s,
        u"JSON_ARRAYAGG"_s,
        u"JSON_EXISTS"_s,
        u"JSON_OBJECT"_s,
        u"JSON_OBJECTAGG"_s,
        u"JSON_QUERY"_s,
        u"JSON_TABLE"_s,
        u"JSON_TABLE_PRIMITIVE"_s,
        u"JSON_VALUE"_s,
        u"KEEP"_s,
        u"KEY"_s,
        u"KEYS"_s,
        u"KILL"_s,
        u"KURTOSIS"_s,
        u"LABEL"_s,
        u"LAG"_s,
        u"LANGUAGE"_s,
        u"LARGE"_s,
        u"LAST"_s,
        u"LAST_VALUE"_s,
        u"LATERAL"_s,
        u"LC_CTYPE"_s,
        u"LE"_s,
        u"LEAD"_s,
        u"LEADING"_s,
        u"LEAVE"_s,
        u"LEFT"_s,
        u"LESS"_s,
        u"LEVEL"_s,
        u"LIKE"_s,
        u"LIKE_REGEX"_s,
        u"LIMIT"_s,
        u"LINEAR"_s,
        u"LINENO"_s,
        u"LINES"_s,
        u"LISTAGG"_s,
        u"LN"_s,
        u"LOAD"_s,
        u"LOADING"_s,
        u"LOCAL"_s,
        u"LOCALE"_s,
        u"LOCALTIME"_s,
        u"LOCALTIMESTAMP"_s,
        u"LOCATOR"_s,
        u"LOCATORS"_s,
        u"LOCK"_s,
        u"LOCKING"_s,
        u"LOCKMAX"_s,
        u"LOCKSIZE"_s,
        u"LOG"_s,
        u"LOG10"_s,
        u"LOGGING"_s,
        u"LOGON"_s,
        u"LONG"_s,
        u"LONGBLOB"_s,
        u"LONGTEXT"_s,
        u"LOOP"_s,
        u"LOWER"_s,
        u"LOW_PRIORITY"_s,
        u"LT"_s,
        u"MACRO"_s,
        u"MAINTAINED"_s,
        u"MAP"_s,
        u"MASTER_BIND"_s,
        u"MASTER_SSL_VERIFY_SERVER_CERT"_s,
        u"MATCH"_s,
        u"MATCHES"_s,
        u"MATCH_NUMBER"_s,
        u"MATCH_RECOGNIZE"_s,
        u"MATERIALIZED"_s,
        u"MAVG"_s,
        u"MAX"_s,
        u"MAXEXTENTS"_s,
        u"MAXIMUM"_s,
        u"MAXVALUE"_s,
        u"MCHARACTERS"_s,
        u"MDIFF"_s,
        u"MEDIUMBLOB"_s,
        u"MEDIUMINT"_s,
        u"MEDIUMTEXT"_s,
        u"MEMBER"_s,
        u"MERGE"_s,
        u"METHOD"_s,
        u"MICROSECOND"_s,
        u"MICROSECONDS"_s,
        u"MIDDLEINT"_s,
        u"MIN"_s,
        u"MINDEX"_s,
        u"MINIMUM"_s,
        u"MINUS"_s,
        u"MINUTE"_s,
        u"MINUTE_MICROSECOND"_s,
        u"MINUTES"_s,
        u"MINUTE_SECOND"_s,
        u"MLINREG"_s,
        u"MLOAD"_s,
        u"MLSLABEL"_s,
        u"MOD"_s,
        u"MODE"_s,
        u"MODIFIES"_s,
        u"MODIFY"_s,
        u"MODULE"_s,
        u"MONITOR"_s,
        u"MONRESOURCE"_s,
        u"MONSESSION"_s,
        u"MONTH"_s,
        u"MONTHS"_s,
        u"MSUBSTR"_s,
        u"MSUM"_s,
        u"MULTISET"_s,
        u"NAMED"_s,
        u"NAMES"_s,
        u"NATIONAL"_s,
        u"NATURAL"_s,
        u"NCHAR"_s,
        u"NCLOB"_s,
        u"NE"_s,
        u"NESTED_TABLE_ID"_s,
        u"NEW"_s,
        u"NEW_TABLE"_s,
        u"NEXT"_s,
        u"NEXTVAL"_s,
        u"NO"_s,
        u"NOAUDIT"_s,
        u"NOCHECK"_s,
        u"NOCOMPRESS"_s,
        u"NONCLUSTERED"_s,
        u"NONE"_s,
        u"NORMALIZE"_s,
        u"NOT"_s,
        u"NOTNULL"_s,
        u"NOWAIT"_s,
        u"NO_WRITE_TO_BINLOG"_s,
        u"NTH_VALUE"_s,
        u"NTILE"_s,
        u"NULL"_s,
        u"NULLIF"_s,
        u"NULLIFZERO"_s,
        u"NULLS"_s,
        u"NUMBER"_s,
        u"NUMERIC"_s,
        u"NUMPARTS"_s,
        u"OBID"_s,
        u"OBJECT"_s,
        u"OBJECTS"_s,
        u"OCCURRENCES_REGEX"_s,
        u"OCTET_LENGTH"_s,
        u"OF"_s,
        u"OFF"_s,
        u"OFFLINE"_s,
        u"OFFSET"_s,
        u"OFFSETS"_s,
        u"OLD"_s,
        u"OLD_TABLE"_s,
        u"OMIT"_s,
        u"ON"_s,
        u"ONE"_s,
        u"ONLINE"_s,
        u"ONLY"_s,
        u"OPEN"_s,
        u"OPENDATASOURCE"_s,
        u"OPENQUERY"_s,
        u"OPENROWSET"_s,
        u"OPENXML"_s,
        u"OPERATION"_s,
        u"OPTIMIZATION"_s,
        u"OPTIMIZE"_s,
        u"OPTIMIZER_COSTS"_s,
        u"OPTION"_s,
        u"OPTIONALLY"_s,
        u"OR"_s,
        u"ORDER"_s,
        u"ORDINALITY"_s,
        u"ORGANIZATION"_s,
        u"OUT"_s,
        u"OUTER"_s,
        u"OUTFILE"_s,
        u"OUTPUT"_s,
        u"OVER"_s,
        u"OVERLAPS"_s,
        u"OVERLAY"_s,
        u"OVERRIDE"_s,
        u"PACKAGE"_s,
        u"PAD"_s,
        u"PADDED"_s,
        u"PARAMETER"_s,
        u"PARAMETERS"_s,
        u"PART"_s,
        u"PARTIAL"_s,
        u"PARTITION"_s,
        u"PARTITIONED"_s,
        u"PARTITIONING"_s,
        u"PASSWORD"_s,
        u"PATH"_s,
        u"PATTERN"_s,
        u"PCTFREE"_s,
        u"PER"_s,
        u"PERCENT"_s,
        u"PERCENTILE_CONT"_s,
        u"PERCENTILE_DISC"_s,
        u"PERCENT_RANK"_s,
        u"PERIOD"_s,
        u"PERM"_s,
        u"PERMANENT"_s,
        u"PIECESIZE"_s,
        u"PIVOT"_s,
        u"PLACING"_s,
        u"PLAN"_s,
        u"PORTION"_s,
        u"POSITION"_s,
        u"POSITION_REGEX"_s,
        u"POSTFIX"_s,
        u"POWER"_s,
        u"PRECEDES"_s,
        u"PRECISION"_s,
        u"PREFIX"_s,
        u"PREORDER"_s,
        u"PREPARE"_s,
        u"PRESERVE"_s,
        u"PREVVAL"_s,
        u"PRIMARY"_s,
        u"PRINT"_s,
        u"PRIOR"_s,
        u"PRIQTY"_s,
        u"PRIVATE"_s,
        u"PRIVILEGES"_s,
        u"PROC"_s,
        u"PROCEDURE"_s,
        u"PROFILE"_s,
        u"PROGRAM"_s,
        u"PROPORTIONAL"_s,
        u"PROTECTION"_s,
        u"PSID"_s,
        u"PTF"_s,
        u"PUBLIC"_s,
        u"PURGE"_s,
        u"QUALIFIED"_s,
        u"QUALIFY"_s,
        u"QUANTILE"_s,
        u"QUERY"_s,
        u"QUERYNO"_s,
        u"RADIANS"_s,
        u"RAISERROR"_s,
        u"RANDOM"_s,
        u"RANGE"_s,
        u"RANGE_N"_s,
        u"RANK"_s,
        u"RAW"_s,
        u"READ"_s,
        u"READS"_s,
        u"READTEXT"_s,
        u"READ_WRITE"_s,
        u"REAL"_s,
        u"RECONFIGURE"_s,
        u"RECURSIVE"_s,
        u"REF"_s,
        u"REFERENCES"_s,
        u"REFERENCING"_s,
        u"REFRESH"_s,
        u"REGEXP"_s,
        u"REGR_AVGX"_s,
        u"REGR_AVGY"_s,
        u"REGR_COUNT"_s,
        u"REGR_INTERCEPT"_s,
        u"REGR_R2"_s,
        u"REGR_SLOPE"_s,
        u"REGR_SXX"_s,
        u"REGR_SXY"_s,
        u"REGR_SYY"_s,
        u"RELATIVE"_s,
        u"RELEASE"_s,
        u"RENAME"_s,
        u"REPEAT"_s,
        u"REPLACE"_s,
        u"REPLICATION"_s,
        u"REPOVERRIDE"_s,
        u"REQUEST"_s,
        u"REQUIRE"_s,
        u"RESIGNAL"_s,
        u"RESOURCE"_s,
        u"RESTART"_s,
        u"RESTORE"_s,
        u"RESTRICT"_s,
        u"RESULT"_s,
        u"RESULT_SET_LOCATOR"_s,
        u"RESUME"_s,
        u"RET"_s,
        u"RETRIEVE"_s,
        u"RETURN"_s,
        u"RETURNING"_s,
        u"RETURNS"_s,
        u"REVALIDATE"_s,
        u"REVERT"_s,
        u"REVOKE"_s,
        u"RIGHT"_s,
        u"RIGHTS"_s,
        u"RLIKE"_s,
        u"ROLE"_s,
        u"ROLLBACK"_s,
        u"ROLLFORWARD"_s,
        u"ROLLUP"_s,
        u"ROUND_CEILING"_s,
        u"ROUND_DOWN"_s,
        u"ROUND_FLOOR"_s,
        u"ROUND_HALF_DOWN"_s,
        u"ROUND_HALF_EVEN"_s,
        u"ROUND_HALF_UP"_s,
        u"ROUND_UP"_s,
        u"ROUTINE"_s,
        u"ROW"_s,
        u"ROWCOUNT"_s,
        u"ROWGUIDCOL"_s,
        u"ROWID"_s,
        u"ROWNUM"_s,
        u"ROW_NUMBER"_s,
        u"ROWS"_s,
        u"ROWSET"_s,
        u"RULE"_s,
        u"RUN"_s,
        u"RUNNING"_s,
        u"SAMPLE"_s,
        u"SAMPLEID"_s,
        u"SAVE"_s,
        u"SAVEPOINT"_s,
        u"SCHEMA"_s,
        u"SCHEMAS"_s,
        u"SCOPE"_s,
        u"SCRATCHPAD"_s,
        u"SCROLL"_s,
        u"SEARCH"_s,
        u"SECOND"_s,
        u"SECOND_MICROSECOND"_s,
        u"SECONDS"_s,
        u"SECQTY"_s,
        u"SECTION"_s,
        u"SECURITY"_s,
        u"SECURITYAUDIT"_s,
        u"SEEK"_s,
        u"SEL"_s,
        u"SELECT"_s,
        u"SEMANTICKEYPHRASETABLE"_s,
        u"SEMANTICSIMILARITYDETAILSTABLE"_s,
        u"SEMANTICSIMILARITYTABLE"_s,
        u"SENSITIVE"_s,
        u"SEPARATOR"_s,
        u"SEQUENCE"_s,
        u"SESSION"_s,
        u"SESSION_USER"_s,
        u"SET"_s,
        u"SETRESRATE"_s,
        u"SETS"_s,
        u"SETSESSRATE"_s,
        u"SETUSER"_s,
        u"SHARE"_s,
        u"SHOW"_s,
        u"SHUTDOWN"_s,
        u"SIGNAL"_s,
        u"SIMILAR"_s,
        u"SIMPLE"_s,
        u"SIN"_s,
        u"SINH"_s,
        u"SIZE"_s,
        u"SKEW"_s,
        u"SKIP"_s,
        u"SMALLINT"_s,
        u"SOME"_s,
        u"SOUNDEX"_s,
        u"SOURCE"_s,
        u"SPACE"_s,
        u"SPATIAL"_s,
        u"SPECIFIC"_s,
        u"SPECIFICTYPE"_s,
        u"SPOOL"_s,
        u"SQL"_s,
        u"SQL_BIG_RESULT"_s,
        u"SQL_CALC_FOUND_ROWS"_s,
        u"SQLEXCEPTION"_s,
        u"SQL_SMALL_RESULT"_s,
        u"SQLSTATE"_s,
        u"SQLTEXT"_s,
        u"SQLWARNING"_s,
        u"SQRT"_s,
        u"SS"_s,
        u"SSL"_s,
        u"STANDARD"_s,
        u"START"_s,
        u"STARTING"_s,
        u"STARTUP"_s,
        u"STATE"_s,
        u"STATEMENT"_s,
        u"STATIC"_s,
        u"STATISTICS"_s,
        u"STAY"_s,
        u"STDDEV_POP"_s,
        u"STDDEV_SAMP"_s,
        u"STEPINFO"_s,
        u"STOGROUP"_s,
        u"STORED"_s,
        u"STORES"_s,
        u"STRAIGHT_JOIN"_s,
        u"STRING_CS"_s,
        u"STRUCTURE"_s,
        u"STYLE"_s,
        u"SUBMULTISET"_s,
        u"SUBSCRIBER"_s,
        u"SUBSET"_s,
        u"SUBSTR"_s,
        u"SUBSTRING"_s,
        u"SUBSTRING_REGEX"_s,
        u"SUCCEEDS"_s,
        u"SUCCESSFUL"_s,
        u"SUM"_s,
        u"SUMMARY"_s,
        u"SUSPEND"_s,
        u"SYMMETRIC"_s,
        u"SYNONYM"_s,
        u"SYSDATE"_s,
        u"SYSTEM"_s,
        u"SYSTEM_TIME"_s,
        u"SYSTEM_USER"_s,
        u"SYSTIMESTAMP"_s,
        u"TABLE"_s,
        u"TABLESAMPLE"_s,
        u"TABLESPACE"_s,
        u"TAN"_s,
        u"TANH"_s,
        u"TBL_CS"_s,
        u"TEMPORARY"_s,
        u"TERMINATE"_s,
        u"TERMINATED"_s,
        u"TEXTSIZE"_s,
        u"THAN"_s,
        u"THEN"_s,
        u"THRESHOLD"_s,
        u"TIME"_s,
        u"TIMESTAMP"_s,
        u"TIMEZONE_HOUR"_s,
        u"TIMEZONE_MINUTE"_s,
        u"TINYBLOB"_s,
        u"TINYINT"_s,
        u"TINYTEXT"_s,
        u"TITLE"_s,
        u"TO"_s,
        u"TOP"_s,
        u"TRACE"_s,
        u"TRAILING"_s,
        u"TRAN"_s,
        u"TRANSACTION"_s,
        u"TRANSLATE"_s,
        u"TRANSLATE_CHK"_s,
        u"TRANSLATE_REGEX"_s,
        u"TRANSLATION"_s,
        u"TREAT"_s,
        u"TRIGGER"_s,
        u"TRIM"_s,
        u"TRIM_ARRAY"_s,
        u"TRUE"_s,
        u"TRUNCATE"_s,
        u"TRY_CONVERT"_s,
        u"TSEQUAL"_s,
        u"TYPE"_s,
        u"UC"_s,
        u"UESCAPE"_s,
        u"UID"_s,
        u"UNDEFINED"_s,
        u"UNDER"_s,
        u"UNDO"_s,
        u"UNION"_s,
        u"UNIQUE"_s,
        u"UNKNOWN"_s,
        u"UNLOCK"_s,
        u"UNNEST"_s,
        u"UNPIVOT"_s,
        u"UNSIGNED"_s,
        u"UNTIL"_s,
        u"UPD"_s,
        u"UPDATE"_s,
        u"UPDATETEXT"_s,
        u"UPPER"_s,
        u"UPPERCASE"_s,
        u"USAGE"_s,
        u"USE"_s,
        u"USER"_s,
        u"USING"_s,
        u"UTC_DATE"_s,
        u"UTC_TIME"_s,
        u"UTC_TIMESTAMP"_s,
        u"VALIDATE"_s,
        u"VALIDPROC"_s,
        u"VALUE"_s,
        u"VALUE_OF"_s,
        u"VALUES"_s,
        u"VARBINARY"_s,
        u"VARBYTE"_s,
        u"VARCHAR"_s,
        u"VARCHAR2"_s,
        u"VARCHARACTER"_s,
        u"VARGRAPHIC"_s,
        u"VARIABLE"_s,
        u"VARIADIC"_s,
        u"VARIANT"_s,
        u"VAR_POP"_s,
        u"VAR_SAMP"_s,
        u"VARYING"_s,
        u"VCAT"_s,
        u"VERBOSE"_s,
        u"VERSIONING"_s,
        u"VIEW"_s,
        u"VIRTUAL"_s,
        u"VOLATILE"_s,
        u"VOLUMES"_s,
        u"WAIT"_s,
        u"WAITFOR"_s,
        u"WHEN"_s,
        u"WHENEVER"_s,
        u"WHERE"_s,
        u"WHILE"_s,
        u"WIDTH_BUCKET"_s,
        u"WINDOW"_s,
        u"WITH"_s,
        u"WITHIN"_s,
        u"WITHIN_GROUP"_s,
        u"WITHOUT"_s,
        u"WLM"_s,
        u"WORK"_s,
        u"WRITE"_s,
        u"WRITETEXT"_s,
        u"XMLCAST"_s,
        u"XMLEXISTS"_s,
        u"XMLNAMESPACES"_s,
        u"XOR"_s,
        u"YEAR"_s,
        u"YEAR_MONTH"_s,
        u"YEARS"_s,
        u"ZEROFILL"_s,
        u"ZEROIFNULL"_s,
        u"ZONE"_s,
      }
    }
  };
}

QSet<QString> QgsAbstractDatabaseProviderConnection::illegalFieldNames() const
{
  return mIllegalFieldNames;
}

QString QgsAbstractDatabaseProviderConnection::defaultPrimaryKeyColumnName() const
{
  return u"pk"_s;
}

QString QgsAbstractDatabaseProviderConnection::defaultGeometryColumnName() const
{
  return u"geom"_s;
}

QList<Qgis::FieldDomainType> QgsAbstractDatabaseProviderConnection::supportedFieldDomainTypes() const
{
  return {};
}

QList<Qgis::RelationshipCardinality> QgsAbstractDatabaseProviderConnection::supportedRelationshipCardinalities() const
{
  return {};
}

QList<Qgis::RelationshipStrength> QgsAbstractDatabaseProviderConnection::supportedRelationshipStrengths() const
{
  return {};
}

Qgis::RelationshipCapabilities QgsAbstractDatabaseProviderConnection::supportedRelationshipCapabilities() const
{
  return Qgis::RelationshipCapabilities();
}

QStringList QgsAbstractDatabaseProviderConnection::relatedTableTypes() const
{
  return {};
}

QgsProviderSqlQueryBuilder *QgsAbstractDatabaseProviderConnection::queryBuilder() const
{
  return new QgsProviderSqlQueryBuilder();
}

void QgsAbstractDatabaseProviderConnection::createVectorTable( const QString &schema,
    const QString &name,
    const QgsFields &fields,
    Qgis::WkbType wkbType,
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


QList<QgsLayerMetadataProviderResult> QgsAbstractDatabaseProviderConnection::searchLayerMetadata( const QgsMetadataSearchContext &searchContext, const QString &searchString, const QgsRectangle &geographicExtent, QgsFeedback *feedback ) const
{
  Q_UNUSED( feedback );
  Q_UNUSED( searchContext );
  Q_UNUSED( searchString );
  Q_UNUSED( geographicExtent );
  throw QgsNotSupportedException( QObject::tr( "Provider %1 has no %2 method" ).arg( providerKey(), u"searchLayerMetadata"_s ) );
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

bool QgsAbstractDatabaseProviderConnection::validateSqlVectorLayer( const SqlVectorLayerOptions &, QString & ) const
{
  checkCapability( Capability::SqlLayers );
  return true;
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
  std::unique_ptr<QgsVectorLayer> vl { std::make_unique<QgsVectorLayer>( tableUri( schema, tableName ), u"temp_layer"_s, mProviderKey, options ) };
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
  auto vl = std::make_unique<QgsVectorLayer>( tableUri( schema, tableName ), u"temp_layer"_s, mProviderKey, options ) ;
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

void QgsAbstractDatabaseProviderConnection::renameField( const QString &schema, const QString &tableName, const QString &name, const QString &newName ) const
{
  checkCapability( Capability::RenameField );

  QgsVectorLayer::LayerOptions options { false, false };
  options.skipCrsValidation = true;
  auto vl = std::make_unique<QgsVectorLayer>( tableUri( schema, tableName ), u"temp_layer"_s, mProviderKey, options ) ;
  if ( ! vl->isValid() )
  {
    throw QgsProviderConnectionException( QObject::tr( "Could not create a vector layer for table '%1' in schema '%2'" )
                                          .arg( tableName, schema ) );
  }
  int existingIndex = vl->fields().lookupField( name );
  if ( existingIndex == -1 )
  {
    throw QgsProviderConnectionException( QObject::tr( "Field '%1' in table '%2' in does not exist" )
                                          .arg( name, tableName ) );

  }
  if ( vl->fields().lookupField( newName ) != -1 )
  {
    throw QgsProviderConnectionException( QObject::tr( "A field with name '%1' already exists in table '%2'" )
                                          .arg( newName, tableName ) );

  }
  if ( ! vl->dataProvider()->renameAttributes( {{existingIndex, newName}} ) )
  {
    throw QgsProviderConnectionException( QObject::tr( "Unknown error renaming field '%1' in table '%2' to '%3'" )
                                          .arg( name, tableName, newName ) );
  }
}

QList<QgsAbstractDatabaseProviderConnection::TableProperty> QgsAbstractDatabaseProviderConnection::tables( const QString &, const QgsAbstractDatabaseProviderConnection::TableFlags &, QgsFeedback * ) const
{
  checkCapability( Capability::Tables );
  return QList<QgsAbstractDatabaseProviderConnection::TableProperty>();
}


QgsAbstractDatabaseProviderConnection::TableProperty QgsAbstractDatabaseProviderConnection::table( const QString &schema, const QString &name, QgsFeedback *feedback ) const
{
  checkCapability( Capability::Tables );
  const QList<QgsAbstractDatabaseProviderConnection::TableProperty> constTables { tables( schema, TableFlags(), feedback ) };
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

void QgsAbstractDatabaseProviderConnection::TableProperty::addGeometryColumnType( Qgis::WkbType type, const QgsCoordinateReferenceSystem &crs )
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


QgsFields QgsAbstractDatabaseProviderConnection::fields( const QString &schema, const QString &tableName, QgsFeedback * ) const
{
  QgsVectorLayer::LayerOptions options { false, true };
  options.skipCrsValidation = true;
  QgsVectorLayer vl { tableUri( schema, tableName ), u"temp_layer"_s, mProviderKey, options };
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

void QgsAbstractDatabaseProviderConnection::updateFieldDomain( QgsFieldDomain *, const QString & ) const
{
  checkCapability( Qgis::DatabaseProviderConnectionCapability2::EditFieldDomain );
}

void QgsAbstractDatabaseProviderConnection::deleteFieldDomain( const QString &, const QString & ) const
{
  checkCapability( Qgis::DatabaseProviderConnectionCapability2::DeleteFieldDomain );
}

void QgsAbstractDatabaseProviderConnection::setFieldAlias( const QString &, const QString &, const QString &, const QString & ) const
{
  checkCapability( Qgis::DatabaseProviderConnectionCapability2::SetFieldAlias );
}

void QgsAbstractDatabaseProviderConnection::setTableComment( const QString &, const QString &, const QString & ) const
{
  checkCapability( Qgis::DatabaseProviderConnectionCapability2::SetTableComment );
}

void QgsAbstractDatabaseProviderConnection::setFieldComment( const QString &, const QString &, const QString &, const QString & ) const
{
  checkCapability( Qgis::DatabaseProviderConnectionCapability2::SetFieldComment );
}

QList< QgsWeakRelation > QgsAbstractDatabaseProviderConnection::relationships( const QString &, const QString & ) const
{
  checkCapability( Capability::RetrieveRelationships );
  return {};
}

void QgsAbstractDatabaseProviderConnection::addRelationship( const QgsWeakRelation & ) const
{
  checkCapability( Capability::AddRelationship );
}

void QgsAbstractDatabaseProviderConnection::updateRelationship( const QgsWeakRelation & ) const
{
  checkCapability( Capability::UpdateRelationship );
}

void QgsAbstractDatabaseProviderConnection::deleteRelationship( const QgsWeakRelation & ) const
{
  checkCapability( Capability::DeleteRelationship );
}

QString QgsAbstractDatabaseProviderConnection::TableProperty::defaultName() const
{
  QString n = mTableName;
  if ( mGeometryColumnCount > 1 ) n += '.' + mGeometryColumn;
  return n;
}

void QgsAbstractDatabaseProviderConnection::moveTableToSchema( const QString &schema, const QString &tableName, const QString &targetSchema ) const
{
  Q_UNUSED( schema );
  Q_UNUSED( tableName );
  Q_UNUSED( targetSchema );
  checkCapability( Capability::MoveTableToSchema );
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

bool QgsAbstractDatabaseProviderConnection::splitSimpleQuery( const QString &sql, QStringList &columns, QStringList &tables, QString &where )
{
  const QgsSQLStatement statement { sql };
  if ( statement.hasParserError() )
  {
    return false;
  }

  const QgsSQLStatement::NodeSelect *nodeSelect = dynamic_cast<const QgsSQLStatement::NodeSelect *>( statement.rootNode() );

  if ( !nodeSelect || !nodeSelect->joins().empty() || !nodeSelect->orderBy().empty() )
  {
    return false;
  }

  const QList<QgsSQLStatement::NodeTableDef *> tablesList { nodeSelect->tables() };

  if ( tablesList.empty() )
  {
    return false;
  }

  for ( const QgsSQLStatement::NodeTableDef *tableDef : tablesList )
  {
    tables.append( tableDef->name() );
  }

  const QList<QgsSQLStatement::NodeSelectedColumn *> columnsList { nodeSelect->columns() };

  if ( columnsList.empty() )
  {
    return false;
  }

  for ( const QgsSQLStatement::NodeSelectedColumn *colDef : columnsList )
  {
    columns.append( colDef->dump() );
  }

  if ( nodeSelect->where() )
  {
    where = nodeSelect->where()->dump();
  }

  return true;
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
  : mResultIterator( std::move( iterator ) )
{}

double QgsAbstractDatabaseProviderConnection::QueryResult::queryExecutionTime() const
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

