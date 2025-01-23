/***************************************************************************
    qgsdamengdatabase.cpp
    ---------------------
    begin                : 2025/01/14
    copyright            : ( C ) 2025 by Haiyang Zhao
    email                : zhaohaiyang@dameng.com
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   ( at your option ) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "qgsdamengdatabase.h"

#if defined ( Q_OS_WIN32 )
#include <qt_windows.h>
#endif
#include <qcoreapplication.h>
#include <qvariant.h>
#include <qdatetime.h>
#include <qstringlist.h>
#include <qvarlengtharray.h>
#include <qvector.h>
#include <qmath.h>

#include <QDebug>

QT_BEGIN_NAMESPACE

static QString quotedString( const QString &v )
{
  QString result = v;
  result.replace( '\'', QLatin1String( "''" ) );
  if ( result.contains( '\\' ) )
    return result.replace( '\\', QLatin1String( "\\\\" ) ).prepend( "E'" ).append( '\'' );
  else
    return result.prepend( '\'' ).append( '\'' );
}

static QString qWarnDMHandle( int handleType, dhandle handle, int *nativeCode = 0 )
{
  sdint4          nativeCode_ = 0;
  sdint2          msgLen = 0;
  DPIRETURN       r = DSQL_NO_DATA;
  QVarLengthArray<sdbyte> description_( 512 );
  QString         result;
  int             i = 1;

  description_[0] = 0;
  do
  {
    r = dpi_get_diag_rec( handleType, handle, i, &nativeCode_, 0, 0, &msgLen );

    if ( ( r == DSQL_SUCCESS || r == DSQL_SUCCESS_WITH_INFO ) && msgLen > 0 )
      description_.resize( msgLen + 1 );

    r = dpi_get_diag_rec( handleType, handle, i, &nativeCode_, description_.data(), description_.size(), &msgLen );

    if ( r == DSQL_SUCCESS || r == DSQL_SUCCESS_WITH_INFO )
    {
      if ( nativeCode )
        *nativeCode = nativeCode_;

      QString tmpstore = QString::fromUtf8( ( const char* )description_.constData(), msgLen );

      if ( result != tmpstore )
      {
        if ( !result.isEmpty() )
          result += QLatin1Char( ' ' );

        result += tmpstore;
      }
    }
    else if ( r == DSQL_ERROR || r == DSQL_INVALID_HANDLE )
    {
      return result;
    }

    ++i;
  } while ( r != DSQL_NO_DATA );

  return result;
}

static QString qDMWarn( const dhandle hStmt, const dhandle envHandle = 0,
  const dhandle pDbC = 0, int *nativeCode = 0 )
{
  QString result;

  if ( envHandle )
    result += qWarnDMHandle( DSQL_HANDLE_ENV, envHandle, nativeCode );

  if ( pDbC )
  {
    const QString dMessage = qWarnDMHandle( DSQL_HANDLE_DBC, pDbC, nativeCode );
    if ( !dMessage.isEmpty() )
    {
      if ( !result.isEmpty() )
        result += QLatin1Char( ' ' );
      result += dMessage;
    }
  }

  if ( hStmt )
  {
    const QString hMessage = qWarnDMHandle( DSQL_HANDLE_STMT, hStmt, nativeCode );
    if ( !hMessage.isEmpty() )
    {
      if ( !result.isEmpty() )
        result += QLatin1Char( ' ' );
      result += hMessage;
    }
  }

  return result;
}

static QString qDMWarn( const QgsDMResultPrivate *dm, int *nativeCode = 0 )
{
  return qDMWarn( dm->hStmt, dm->dpEnv(), dm->dpDbc(), nativeCode );
}

static QString qDMWarn( const QgsDMDriverPrivate *dm, int *nativeCode = 0 )
{
  return qDMWarn( 0, dm->hEnv, dm->hDbc, nativeCode );
}

static void qSqlWarning( const QString &message, const QgsDMResultPrivate *dm )
{
  qWarning() << message << "\tError:" << qDMWarn( dm );
}

static void qSqlWarning( const QString &message, const QgsDMDriverPrivate *dm )
{
  qWarning() << message << "\tError:" << qDMWarn( dm );
}

static void qSqlWarning( const QString &message, const dhandle hStmt )
{
  qWarning() << message << "\tError:" << qDMWarn( hStmt );
}

static QSqlError qMakeError( QSqlError::ErrorType type, const QgsDMResultPrivate *p )
{
  int     nativeCode = -1;

  QString message = qDMWarn( p, &nativeCode );

  return QSqlError( "", message, type, QString::number( nativeCode ) );
}

static QSqlError qMakeError( QSqlError::ErrorType type, const QgsDMDriverPrivate *p )
{
  int     nativeCode = -1;

  QString message = qDMWarn( p, &nativeCode );

  return QSqlError( "", message, type, QString::number( nativeCode ) );
}

/* Functions for DSQL Type to QVariant Type */
static QVariant::Type qDecodeDMType( sdint2 dtype, bool isSigned = true )
{
  QVariant::Type type = QVariant::Invalid;

  switch ( dtype )
  {
  case DSQL_DEC:
  case DSQL_FLOAT:
  case DSQL_DOUBLE:
    type = QVariant::Double;
    break;
  case DSQL_SMALLINT:
  case DSQL_INT:
    type = isSigned ? QVariant::Int : QVariant::UInt;
    break;
  case DSQL_BIT:
    type = QVariant::Bool;
    break;
  case DSQL_TINYINT:
    type = QVariant::UInt;
    break;
  case DSQL_BIGINT:
    type = isSigned ? QVariant::LongLong : QVariant::ULongLong;
    break;
  case DSQL_ROWID:
    type = QVariant::String;
    break;
  case DSQL_BINARY:
  case DSQL_VARBINARY:
  case DSQL_BLOB:
    type = QVariant::ByteArray;
    break;
  case DSQL_DATE:
    type = QVariant::Date;
    break;
  case DSQL_TIME:
    type = QVariant::Time;
    break;
  case DSQL_TIMESTAMP:
    type = QVariant::String;
    break;
  case DSQL_CHAR:
  case DSQL_VARCHAR:
  case DSQL_CLOB:
  case DSQL_TIME_TZ:
  case DSQL_TIMESTAMP_TZ:
    type = QVariant::String;
    break;
  
  case DSQL_INTERVAL_YEAR:
  case DSQL_INTERVAL_MONTH:
  case DSQL_INTERVAL_DAY:
  case DSQL_INTERVAL_HOUR:
  case DSQL_INTERVAL_MINUTE:
  case DSQL_INTERVAL_SECOND:
  case DSQL_INTERVAL_YEAR_TO_MONTH:
  case DSQL_INTERVAL_DAY_TO_HOUR:
  case DSQL_INTERVAL_DAY_TO_MINUTE:
  case DSQL_INTERVAL_DAY_TO_SECOND:
  case DSQL_INTERVAL_HOUR_TO_MINUTE:
  case DSQL_INTERVAL_HOUR_TO_SECOND:
  case DSQL_INTERVAL_MINUTE_TO_SECOND:
    type = QVariant::String;
    break;

  case DSQL_CLASS:
  case DSQL_ARRAY:
  case DSQL_RECORD:
    type = QVariant::String;
    break;

  default:
    type = QVariant::ByteArray;
    break;
  }

  return type;
}

/* Functions for DSQL Type to C Type */
static sdint4 qCTypefromSql( sdint4 sqlType )
{
  sdint4 cType;
  switch ( sqlType )
  {
  case DSQL_CHAR:	
  case DSQL_VARCHAR:
  case DSQL_CLOB:
    cType = DSQL_C_NCHAR;
    break;
  case DSQL_BLOB:
  case DSQL_BINARY:
  case DSQL_VARBINARY:
    cType = DSQL_C_BINARY;
    break;
  case DSQL_BIT:
    cType = DSQL_C_BIT;
    break;
  case DSQL_TINYINT:
    cType = DSQL_C_UTINYINT;
    break;
  case DSQL_SMALLINT:
    cType = DSQL_C_SSHORT;
    break;
  case DSQL_INT:
    cType = DSQL_C_SLONG;
    break;
  case DSQL_BIGINT:
    cType = DSQL_C_SBIGINT;
    break;
  case DSQL_FLOAT:
    cType = DSQL_C_FLOAT;
    break;
  case DSQL_DOUBLE:
    cType = DSQL_C_DOUBLE;
    break;
  case DSQL_DEC:
    cType = DSQL_C_NUMERIC;
    break;
  case DSQL_DATE:
    cType = DSQL_C_DATE;
    break;
  case DSQL_TIME:
    cType = DSQL_C_TIME;
    break;
  case DSQL_TIMESTAMP:
    cType = DSQL_C_TIMESTAMP;
    break;
  case DSQL_TIME_TZ:
  case DSQL_TIMESTAMP_TZ:
    cType = DSQL_C_TIMESTAMP;
    break;
  case DSQL_CLASS:
    cType = DSQL_C_CLASS;
    break;
  case DSQL_RECORD:
    cType = DSQL_C_RECORD;
    break;
  case DSQL_ARRAY:
    cType = DSQL_C_ARRAY;
    break;
  case DSQL_SARRAY:
    cType = DSQL_C_SARRAY;
    break;
  case DSQL_ROWID:
    cType = DSQL_C_BINARY;
    break;
  case DSQL_RSET:
    cType = DSQL_C_RSET;
    break;
  case DSQL_BFILE:
    cType = DSQL_C_BFILE;
    break;
  default:
    cType = DSQL_C_NCHAR;
    break;
  }

  return cType;
}

/* Functions for Get String Data from Dameng */
static QString qGetStringData( dhstmt hStmt, int column, int colSize )
{
  QString         fieldVal;
  DPIRETURN       r = DSQL_ERROR;
  slength         lengthIndicator = 0;

  // NB! colSize must be a multiple of 2 for unicode enabled DBs
  if ( colSize <= 0 )
  {
    colSize = 256;
  }
  else if ( colSize > 65536 )
  {
    // limit buffer size to 64 KB
    colSize = 65536;
  }
  else
  {
    colSize++; // make sure there is room for more than the 0 termination
  }
  r = dpi_get_data( hStmt, column + 1, DSQL_C_NCHAR, NULL, 0, &lengthIndicator );

  if ( ( r == DSQL_SUCCESS || r == DSQL_SUCCESS_WITH_INFO ) && lengthIndicator > 0 )
    colSize = lengthIndicator + 1;

  QVarLengthArray<char> buf( colSize );

  while ( true )
  {
    r = dpi_get_data( hStmt, column + 1, DSQL_C_NCHAR, ( dpointer )buf.data(), colSize, &lengthIndicator );

    if ( r == DSQL_SUCCESS || r == DSQL_SUCCESS_WITH_INFO )
    {
      if ( lengthIndicator == DSQL_NULL_DATA )
      {
        fieldVal.clear();
        break;
      }

      // if DSQL_SUCCESS_WITH_INFO is returned, indicating that
      // more data can be fetched, the length indicator does NOT
      // contain the number of bytes returned - it contains the
      // total number of bytes that CAN be fetched
      // colSize-1: remove 0 termination when there is more data to fetch
      int rSize = ( r == DSQL_SUCCESS_WITH_INFO ) ? colSize : lengthIndicator;
      // Remove any trailing \0 as some drivers misguidedly append one
      int realsize = qMin( rSize, buf.size() );
      if ( realsize > 0 && buf[realsize - 1] == 0 )
        realsize--;
      fieldVal += QString::fromUtf8( ( const char* )( buf.constData() ), realsize );

      if ( lengthIndicator < static_cast<slength>( colSize ) )
      {
        // workaround for Drivermanagers that don't return SQL_NO_DATA
        break;
      }
    }
    else if ( r == DSQL_NO_DATA )
    {
      break;
    }
    else
    {
      qWarning() << "qGetStringData: Error while fetching data (" << qWarnDMHandle( DSQL_HANDLE_STMT, hStmt ) << ')';
      fieldVal.clear();
      break;
    }
  }

  return fieldVal;
}

static QVariant qGetBinaryData( dhstmt hStmt, int column, int type )
{
  QByteArray      fieldVal;
  sdint2          colNameLen;
  sdint2          colType;
  ulength         colSize;
  sdint2          colScale;
  sdint2          nullable;
  slength         lengthIndicator = 0;
  DPIRETURN       r = DSQL_ERROR;

  QVarLengthArray<sdbyte> colName( COLNAME_SIZE );

  r = dpi_desc_column( hStmt, column + 1, colName.data(), COLNAME_SIZE, &colNameLen, &colType, &colSize, &colScale, &nullable );
  if ( r != DSQL_SUCCESS )
    qWarning() << QCoreApplication::translate( "QgsDMResult", "Unable to describe column %1" ).arg( column );

  // dpi_desc_column may return 0 if size cannot be determined
  if ( !colSize )
    colSize = 255;
  else if ( colSize > 65536 ) // read the field in 64 KB chunks
    colSize = 65536;

  fieldVal.resize( colSize );

  ulong read = 0;
  while ( true )
  {
    r = dpi_get_data( hStmt, column + 1, type, ( dpointer )( fieldVal.constData() + read ), colSize, &lengthIndicator );
    if ( r != DSQL_SUCCESS && r != DSQL_SUCCESS_WITH_INFO )
      break;

    if ( lengthIndicator == DSQL_NULL_DATA )
      return QVariant( QVariant::ByteArray );

    //handle the type of binary
    if ( type == DSQL_C_NCHAR && read == 0 && fieldVal[0] == '\0' )
    {
      colSize = 65536;
      fieldVal.resize( read );
      continue;
    }

    if ( lengthIndicator > static_cast<slength>( colSize ) )
    {
      read += colSize;
      colSize = 65536;
    }
    else
    {
      read += lengthIndicator;
    }

    if ( r == DSQL_SUCCESS )
    {
      // the whole field was read in one chunk
      if ( type == DSQL_C_BINARY )
        fieldVal.resize( read + 1 );
      if ( type == DSQL_C_NCHAR )
      {
        fieldVal.data()[read] = '\0';
        fieldVal.append( fieldVal.data() );
        fieldVal.resize( read + 1 );
      }
      break;
    }

    fieldVal.resize( fieldVal.size() + colSize );
  }

  return fieldVal;
}

static QVariant qGetIntData( dhstmt hStmt, int column, bool isSigned = true )
{
  sdint4          intBuf = 0;
  slength         lengthIndicator = 0;
  DPIRETURN       r;

  r = dpi_get_data( hStmt, column + 1, isSigned ? DSQL_C_SLONG : DSQL_C_ULONG, ( dpointer )&intBuf, sizeof( intBuf ), &lengthIndicator );

  if ( r != DSQL_SUCCESS && r != DSQL_SUCCESS_WITH_INFO )
    return QVariant( QVariant::Invalid );

  if ( lengthIndicator == DSQL_NULL_DATA )
    return QVariant();

  if ( isSigned )
    return static_cast<int>( intBuf );
  else
    return static_cast<uint>( intBuf );
}

static QVariant qGetBigIntData( dhstmt hStmt, int column, bool isSigned = true )
{
  long long int       lngbuf = 0;
  slength             lengthIndicator = 0;
  DPIRETURN           r;

  r = dpi_get_data( hStmt, column + 1, isSigned ? DSQL_C_SBIGINT : DSQL_C_UBIGINT, ( dpointer )&lngbuf, sizeof( lngbuf ), &lengthIndicator );
  if ( r != DSQL_SUCCESS && r != DSQL_SUCCESS_WITH_INFO )
    return QVariant( QVariant::Invalid );

  if ( lengthIndicator == DSQL_NULL_DATA )
    return QVariant();

  if ( isSigned )
    return qint64( lngbuf );
  else
    return quint64( lngbuf );
}

static QVariant qGetDoubleData( dhstmt hStmt, int column )
{
  double          dblbuf;
  slength         lengthIndicator = 0;
  DPIRETURN       r;

  r = dpi_get_data( hStmt, column + 1, DSQL_C_DOUBLE, ( dpointer )&dblbuf, 0, &lengthIndicator );

  if ( r != DSQL_SUCCESS && r != DSQL_SUCCESS_WITH_INFO )
  {
    return QVariant( QVariant::Invalid );
  }

  if ( lengthIndicator == DSQL_NULL_DATA )
    return QVariant();

  return dblbuf;
}


static bool isAutoValue( const dhstmt hStmt, int column )
{
  slength     nNumericAttribute = 0; // Check for auto-increment
  const DPIRETURN r = ::dpi_col_attr( hStmt, column + 1, DSQL_DESC_AUTO_UNIQUE_VALUE, 0, 0, 0, &nNumericAttribute );
  if ( r != DSQL_SUCCESS && r != DSQL_SUCCESS_WITH_INFO )
  {
    qSqlWarning( QCoreApplication::translate( "QgsDMResult", "Unable to get autovalue attribute for column %1"
            ).arg( QString::number( column ) ), hStmt );
    return false;
  }
  return nNumericAttribute != DSQL_FALSE;
}

////////////////////////////////////////////////////////////////////////////

bool QgsDMResultPrivate::isStmtHandleValid()
{
  return disconnectCount == drv_d_func()->disconnectCount;
}

void QgsDMResultPrivate::updateStmtHandleState()
{
  disconnectCount = drv_d_func()->disconnectCount;
}

/* Functions for Get Type of CLASS, ARRAY and RECORD. */
void QgsDMResult::qGetClassData( dhobj &obj, dhobjdesc &objDesc, QString &res, bool &isNull )
{
  sdint2      cnt;
  slength     strLen = 0;
  slength     length = 0;
  udint4      type = 0;
  schar       objTypename[128];
  schar       objSchema[128];

  dpi_get_obj_attr( obj, 0, DSQL_ATTR_OBJ_VAL_COUNT, &cnt, sizeof( cnt ), NULL );
  
  schar **strVal = new schar*[( cnt + 1 ) * sizeof( schar * )];
  dhobj *obj2 = new dhobj[cnt + 1];
  dhobjdesc *objDesc2 = new dhobjdesc[cnt + 1];

  for ( int j = 1; j < cnt + 1; j++)
  {
    strVal[j] = nullptr;

    dpi_get_obj_desc_attr( objDesc, j, DSQL_ATTR_OBJ_TYPE, &type, sizeof( type ), NULL );

    if ( type == DSQL_CLASS )
      dpi_get_obj_val( obj, j, DSQL_C_CLASS, &obj2[j], sizeof( obj2[j] ), &length );
    else if ( type == DSQL_ARRAY )
      dpi_get_obj_val( obj, j, DSQL_C_ARRAY, &obj2[j], sizeof( obj2[j] ), &length );
    else if ( type == DSQL_RECORD )
      dpi_get_obj_val( ( dhobj )obj, j, DSQL_C_RECORD, &obj2[j], sizeof( obj2[j] ), &length );
    else
    {
      dpi_get_obj_val( ( dhobj )obj, j, DSQL_C_NCHAR, 0, 0, &strLen );
      if( type == DSQL_BLOB || type == DSQL_BINARY || type == DSQL_VARBINARY )
        strLen *= 2;
      if ( strLen == 0 )
      {
        strVal[j] = new schar[2];
      }
      else
      {
        strVal[j] = new schar[strLen];
        isNull = false;
        dpi_get_obj_val( ( dhobj )obj, j, DSQL_C_NCHAR, strVal[j], strLen, &strLen );
      }
    }

    if ( type == DSQL_ARRAY || type == DSQL_CLASS || type == DSQL_RECORD )
    {
      dpi_get_obj_desc_attr( objDesc, j, DSQL_ATTR_OBJ_DESC, &objDesc2[j], sizeof( objDesc2[j] ), NULL );
      dpi_get_obj_desc_attr( objDesc2[j], 0, DSQL_ATTR_OBJ_NAME, objTypename, sizeof( objTypename ), NULL );
      dpi_get_obj_desc_attr( objDesc2[j], 0, DSQL_ATTR_OBJ_SCHAME, objSchema, sizeof( objSchema ), NULL );

      if ( type == DSQL_RECORD )
        res += QString::fromUtf8( ( char* )objTypename ) + '(';
      else
        res += QStringLiteral( "%1.%2(" ).arg( ( char* )objSchema ).arg( ( char* )objTypename );

      qGetClassData( obj2[j], objDesc2[j], res, isNull );
    }
    else
    {
      if ( type == DSQL_BLOB || type == DSQL_BINARY || type == DSQL_VARBINARY )
      {
        res += "0x";
        res += QString::fromUtf8( ( char* )strVal[j] ).toUpper();
      }
      else
        res += QString::fromUtf8( ( char* )strVal[j] );
    }

    res += j == cnt ? ")" : ",";

    if ( strVal[j] != nullptr )
      delete[] strVal[j];
  }

  delete[] obj2;
  delete[] objDesc2;

  if ( strVal != nullptr )
    delete[] strVal;

  return;
}

QString QgsDMResult::qGetClassData( QQueue<DmObj *> &objData, int &field, int &sql_type )
{
  Q_UNUSED( field )
  DmObj*    data = objData.dequeue();
  dhobj     obj = data->obj;
  dhobjdesc objDesc = data->objDesc;
  QString   fieldVal;

  if ( sql_type == DSQL_ARRAY || sql_type == DSQL_CLASS )
  {
    schar objTypeName[128];
    schar objSchema[128];

    dpi_get_obj_desc_attr( objDesc, 0, DSQL_ATTR_OBJ_NAME, objTypeName, sizeof( objTypeName ), NULL );
    dpi_get_obj_desc_attr( objDesc, 0, DSQL_ATTR_OBJ_SCHAME, objSchema, sizeof( objSchema ), NULL );

    fieldVal = QStringLiteral( "%1.%2(" ).arg( ( char* )objSchema ).arg( ( char* )objTypeName );
    bool isNull = true;
    qGetClassData( obj, objDesc, fieldVal, isNull );
    if( isNull )
      return QString();
  }

  dpi_free_obj_desc( objDesc );
  dpi_free_obj( obj );
  delete data;

  return fieldVal;
}

QgsDMResult::QgsDMResult( const QgsDMDriver *db )
  : QSqlResult(*new QgsDMResultPrivate( this, db ) )
{
}

QgsDMResult::~QgsDMResult()
{
  Q_D( QgsDMResult );
  if ( d->hStmt && driver() && d->isStmtHandleValid() && driver()->isOpen() )
  {
    DPIRETURN r = dpi_free_stmt( d->hStmt );
    d->hStmt = NULL;
    if ( r != DSQL_SUCCESS )
      qSqlWarning( QCoreApplication::translate( "QgsDMResult", "Unable to free statement handle" ), d );
  }

  if(!mTypeName.isEmpty() )
    mTypeName.clear();
  if(!mSqlType.isEmpty() )
    mSqlType.clear();
}

dhstmt *QgsDMResult::getStmt()
{
  Q_D( QgsDMResult );

  return &d->hStmt;
}

bool QgsDMResult::reset( const QString &query )
{
  Q_D( QgsDMResult );
  setActive( false );
  setAt( QSql::BeforeFirstRow );
  d->rInf.clear();
  d->fieldCache.clear();
  d->fieldCached.clear();
  d->fieldCacheIdx = 0;

  // Always reallocate the statement handle - the statement attributes
  // are not reset if dpi_free_stmt() is called which causes some problems.
  DPIRETURN       r;

  if ( d->hStmt && d->isStmtHandleValid() )
  {
    r = dpi_free_stmt( d->hStmt );
    d->hStmt = NULL;
    if ( r != DSQL_SUCCESS )
    {
      qSqlWarning( QCoreApplication::translate( "QgsDMResult", "Unable to free statement handle" ), d );
      return false;
    }
  }

  r = dpi_alloc_stmt( d->dpDbc(), &d->hStmt );
  if ( r != DSQL_SUCCESS )
  {
    qSqlWarning( QCoreApplication::translate( "QgsDMResult", "Unable to allocate statement handle" ), d );
    return false;
  }

  d->updateStmtHandleState();

  if ( isForwardOnly() )
  {
    r = dpi_set_stmt_attr( d->hStmt, DSQL_ATTR_CURSOR_TYPE, ( dpointer )DSQL_CURSOR_FORWARD_ONLY, 0 );
  }
  else
  {
    r = dpi_set_stmt_attr( d->hStmt, DSQL_ATTR_CURSOR_TYPE, ( dpointer )DSQL_CURSOR_STATIC, 0 );
  }

  if ( r != DSQL_SUCCESS && r != DSQL_SUCCESS_WITH_INFO )
  {
    setLastError( qMakeError( QSqlError::StatementError, d ) );
    return false;
  }

  r = dpi_exec_direct( d->hStmt, ( sdbyte* )query.toUtf8().data() );
  if ( r != DSQL_SUCCESS && r != DSQL_SUCCESS_WITH_INFO && r != DSQL_NO_DATA )
  {
    setLastError( qMakeError( QSqlError::StatementError, d ) );
    return false;
  }

  ulength     isScrollable = 0;

  r = dpi_get_stmt_attr( d->hStmt, DSQL_ATTR_CURSOR_SCROLLABLE, &isScrollable, sizeof( isScrollable ), NULL );
  if ( r == DSQL_SUCCESS || r == DSQL_SUCCESS_WITH_INFO )
    setForwardOnly( isScrollable == DSQL_NONSCROLLABLE );

  sdint2      count;

  dpi_number_columns( d->hStmt, &count );
  if ( count )
  {
    d->fieldCache.resize( count );
    d->fieldCached.resize( count );

    setSelect( true );
    for ( int i = 0; i < count; ++i )
    {
      d->rInf.append( qMakeFieldInfo( d, i ) );
      if ( ftype( i ) == DSQL_CLASS )
        mGeoType.insert( i,getGeoType( i ) );
      d->fieldCached[i] = false;
    }
  }
  else
  {
    setSelect( false );
  }
  setActive( true );

  return true;
}

void QgsDMResult::setForwardOnly( bool forward )
{
  Q_D( QgsDMResult );
  d->forwardOnly = forward;
}

/* Functions for Get data after exec SQL. */
bool QgsDMResult::fetch( int i )
{
  Q_D( QgsDMResult );
  if ( !driver()->isOpen() )
    return false;

  if ( isForwardOnly() && i < at() )
    return false;

  if ( i == at() )
    return true;

  d->clearValues();

  int     actualIdx = i + 1;

  if ( actualIdx <= 0 )
  {
    setAt( QSql::BeforeFirstRow );
    return false;
  }

  DPIRETURN       r;
  if ( isForwardOnly() )
  {
    bool        ok = true;

    while ( ok && i > at() )
      ok = fetchNext();

    return ok;
  }
  else
  {
    r = dpi_fetch_scroll( d->hStmt, DSQL_FETCH_ABSOLUTE, actualIdx, NULL );
  }

  if ( r != DSQL_SUCCESS )
  {
    if ( r != DSQL_NO_DATA )
      setLastError( qMakeError( QSqlError::ConnectionError, d ) );

    return false;
  }

  setAt( i );

  return true;
}

bool QgsDMResult::fetchBinary()
{
  Q_D( QgsDMResult );

  d->clearValues();

  d->r = dpi_fetch( d->hStmt, NULL );

  if ( d->r != DSQL_SUCCESS && d->r != DSQL_SUCCESS_WITH_INFO )
  {
    if ( d->r != DSQL_NO_DATA )
      setLastError( qMakeError( QSqlError::ConnectionError, d ) );

    return false;
  }

  setAt( at() + 1 );

  return true;
}

bool QgsDMResult::fetchNext()
{
  Q_D( QgsDMResult );

  if ( !mObjData.isEmpty() )
    mObjData.clear();
  for ( int i = 0; i < mColNum; ++i )
  {
    if ( ftype( i ) == DSQL_CLASS || ftype( i ) == DSQL_ARRAY )
    {
      DmObj *obj = new DmObj();
      d->r = dpi_get_stmt_attr( d->hStmt, DSQL_ATTR_IMP_ROW_DESC, ( dpointer )&obj->hdescCol, 0, 0 );
      d->r = dpi_get_desc_field( obj->hdescCol, static_cast<sdint2>( i + 1 ), DSQL_DESC_OBJ_DESCRIPTOR,
                                    ( dpointer )&obj->objDesc, sizeof( dhobjdesc ), NULL );

      d->r = dpi_alloc_obj( d->dpDbc(), &obj->obj );
      if ( ftype( i ) == DSQL_CLASS )
        d->r = dpi_bind_col( d->hStmt, static_cast<udint2>( i + 1 ), DSQL_C_CLASS, &obj->obj, sizeof( obj->obj ), &obj->length );
      if ( ftype( i ) == DSQL_ARRAY )
        d->r = dpi_bind_col( d->hStmt, static_cast<udint2>( i + 1 ), DSQL_C_ARRAY, &obj->obj, sizeof( obj->obj ), &obj->length );

      d->r = dpi_bind_obj_desc( obj->obj, obj->objDesc );

      mObjData.enqueue( obj );
    }
  }

  d->clearValues();

  d->r = dpi_fetch( d->hStmt, NULL );

  if ( d->r != DSQL_SUCCESS && d->r != DSQL_SUCCESS_WITH_INFO )
  {
    if ( d->r != DSQL_NO_DATA )
      setLastError( qMakeError( QSqlError::ConnectionError, d ) );

    return false;
  }

  setAt( at() + 1 );

  return true;
}

bool QgsDMResult::fetchScroll()
{
  Q_D( QgsDMResult );
  DPIRETURN           r = 0;

  d->clearValues();

  r = dpi_fetch_scroll( d->hStmt, DSQL_FETCH_NEXT, 0, NULL );

  if ( r != DSQL_SUCCESS && r != DSQL_SUCCESS_WITH_INFO )
  {
    if ( r != DSQL_NO_DATA )
      setLastError( qMakeError( QSqlError::ConnectionError, d ) );

    return false;
  }

  setAt( at() + 1 );
  return true;
}

bool QgsDMResult::fetchFirst()
{
  Q_D( QgsDMResult );
  if ( isForwardOnly() && at() != QSql::BeforeFirstRow )
    return false;

  DPIRETURN       r;

  d->clearValues();

  if ( isForwardOnly() )
  {
    return fetchNext();
  }

  r = dpi_fetch_scroll( d->hStmt, DSQL_FETCH_FIRST, 0, NULL );

  if ( r != DSQL_SUCCESS )
  {
    if ( r != DSQL_NO_DATA )
      setLastError( qMakeError( QSqlError::ConnectionError, d ) );

    return false;
  }

  setAt( 0 );

  return true;
}

bool QgsDMResult::fetchPrevious()
{
  Q_D( QgsDMResult );
  if ( isForwardOnly() )
    return false;

  DPIRETURN       r;

  d->clearValues();

  r = dpi_fetch_scroll( d->hStmt, DSQL_FETCH_PRIOR, 0, NULL );
  if ( r != DSQL_SUCCESS )
  {
    if ( r != DSQL_NO_DATA )
      setLastError( qMakeError( QSqlError::ConnectionError, d ) );

    return false;
  }

  setAt( at() - 1 );

  return true;
}

bool QgsDMResult::fetchLast()
{
  Q_D( QgsDMResult );
  DPIRETURN       r;

  d->clearValues();

  if ( isForwardOnly() )
  {
    // cannot seek to last row in forwardOnly mode, so we have to use brute force
    int i = at();

    if ( i == QSql::AfterLastRow )
      return false;

    if ( i == QSql::BeforeFirstRow )
      i = 0;

    while ( fetchNext() )
      ++i;

    setAt( i );

    return true;
  }

  r = dpi_fetch_scroll( d->hStmt, DSQL_FETCH_LAST, 0, NULL );

  if ( r != DSQL_SUCCESS )
  {
    if ( r != DSQL_NO_DATA )
      setLastError( qMakeError( QSqlError::ConnectionError, d ) );

    return false;
  }

  ulength     currRow;

  r = dpi_get_stmt_attr( d->hStmt, DSQL_ATTR_ROW_NUMBER, &currRow, sizeof( currRow ), 0 );

  if ( r != DSQL_SUCCESS )
    return false;

  setAt( currRow - 1 );

  return true;
}

// creates a QSqlField from a valid hStmt generated
// by dpi_columns. The hStmt has to point to a valid position.
QSqlField QgsDMResult::qMakeFieldInfo( const dhstmt hStmt, const QgsDMDriverPrivate *p )
{
  QString fname = qGetStringData( hStmt, 3, -1 );
  int     sqlType = qGetIntData( hStmt, 4 ).toInt(); // column type
  int     cType;

  switch ( sqlType )
  {
  case 1:                     //DM_SQL_CHAR
    cType = DSQL_CHAR;
    break;
  case 12:                    //DM_SQL_VARCHAR
    cType = DSQL_VARCHAR;
    break;
  case -7:                    //DM_SQL_BIT
    cType = DSQL_BIT;
    break;
  case -6:                    //DM_SQL_TINYINT
    cType = DSQL_TINYINT;
    break;
  case 5:                     //DM_SQL_SMALLINT
    cType = DSQL_SMALLINT;
    break;
  case 4:                     //DM_SQL_INTEGER
    cType = DSQL_INT;
    break;
  case 6:                     //DM_SQL_FLOAT
  case 7:                     //DM_SQL_REAL
    cType = DSQL_FLOAT;
    break;
  case 8:                     //DM_SQL_DOUBLE
    cType = DSQL_DOUBLE;
    break;
  case 3:                     //DM_SQL_DECIMAL
  case 2:                     //DM_SQL_NUMERIC
    cType = DSQL_DEC;
    break;
  case 91:                    //DM_SQL_TYPE_DATE
    cType = DSQL_DATE;
    break;
  case 92:                    //DM_SQL_TYPE_TIME
    cType = DSQL_TIME;
    break;
  case 9:                     //DM_SQL_DATATIME
  case 93:                    //DM_SQL_TYPE_TIMESTAMP
    cType = DSQL_TIMESTAMP;
    break;
  case 94:                    //DM_SQL_TYPE_TIME_WITH_TIME_ZONE
    cType = DSQL_TIME_TZ;
    break;
  case 95:                    //DM_SQL_TYPE_DATETIME_WITH_TIME_ZONE
    cType = DSQL_TIMESTAMP_TZ;
    break;
  case -5:                    //DM_SQL_BIGINT
    cType = DSQL_BIGINT;
    break;
  case -2:                    //DM_SQL_BINARY
    cType = DSQL_BINARY;
    break;
  case -3:                    //DM_SQL_VARBINARY
    cType = DSQL_VARBINARY;
    break;
  case 101:                   //DM_SQL_INTERVAL_YEAR
    cType = DSQL_INTERVAL_YEAR;
    break;
  case 102:                   //DM_SQL_INTERVAL_MONTH
    cType = DSQL_INTERVAL_MONTH;
    break;
  case 107:                   //DM_SQL_INTERVAL_YEAR_TO_MONTH
    cType = DSQL_INTERVAL_YEAR_TO_MONTH;
    break;
  case 103:                   //DM_SQL_INTERVAL_DAY
    cType = DSQL_INTERVAL_DAY;
    break;
  case 104:                   //DM_SQL_INTERVAL_HOUR
    cType = DSQL_INTERVAL_HOUR;
    break;
  case 105:                   //DM_SQL_INTERVAL_MINUTE
    cType = DSQL_INTERVAL_MINUTE;
    break;
  case 106:                   //DM_SQL_INTERVAL_SECOND
    cType = DSQL_INTERVAL_SECOND;
    break;
  case 108:                   //DM_SQL_INTERVAL_DAY_TO_HOUR
    cType = DSQL_INTERVAL_DAY_TO_HOUR;
    break;
  case 109:                   //DM_SQL_INTERVAL_DAY_TO_MINUTE
    cType = DSQL_INTERVAL_DAY_TO_MINUTE;
    break;
  case 110:                   //DM_SQL_INTERVAL_DAY_TO_SECOND
    cType = DSQL_INTERVAL_DAY_TO_SECOND;
    break;
  case 111:                   //DM_SQL_INTERVAL_HOUR_TO_MINUTE
    cType = DSQL_INTERVAL_HOUR_TO_MINUTE;
    break;
  case 112:                   //DM_SQL_INTERVAL_HOUR_TO_SECOND
    cType = DSQL_INTERVAL_HOUR_TO_SECOND;
    break;
  case 113:                   //DM_SQL_INTERVAL_MINUTE_TO_SECOND
    cType = DSQL_INTERVAL_MINUTE_TO_SECOND;
    break;
  case -4:                    //DM_SQL_LONGVARBINARY
    cType = DSQL_BLOB;
    break;
  case -1:                    //DM_SQL_LONGVARCHAR
    cType = DSQL_CLOB;
    break;
  default:
    cType = DSQL_VARCHAR;
  }

  QSqlField f( fname, qDecodeDMType( cType, p ) );

  QVariant var = qGetIntData( hStmt, 6 );
  f.setLength( var.isNull() ? -1 : var.toInt() ); // column size

  var = qGetIntData( hStmt, 8 ).toInt();
  f.setPrecision( var.isNull() ? -1 : var.toInt() ); // precision

  f.setSqlType( cType );

  int required = qGetIntData( hStmt, 10 ).toInt(); // nullable-flag
  // required can be DSQL_NO_NULLS, DSQL_NULLABLE or DSQL_NULLABLE_UNKNOWN
  if ( required == DSQL_NO_NULLS )
    f.setRequired( true );
  else if ( required == DSQL_NULLABLE )
    f.setRequired( false );
  // else we don't know

  return f;
}

QSqlField QgsDMResult::qMakeFieldInfo( const QgsDMResultPrivate *p, int i )
{
  QString errorMessage;
  const QSqlField result = qMakeFieldInfo( p->hStmt, i, &errorMessage );
  if ( !errorMessage.isEmpty() )
    qSqlWarning( errorMessage, p );
  return result;
}

QSqlField QgsDMResult::qMakeFieldInfo( const dhstmt hStmt, int i, QString *errorMessage )
{
  sdint2          colNameLen;
  sdint2          colType;
  ulength         colSize;
  sdint2          colScale;
  sdint2          nullable;
  DPIRETURN       r = DSQL_ERROR;
  QVarLengthArray<sdbyte> colName( COLNAME_SIZE );
  errorMessage->clear();
  r = dpi_desc_column( hStmt, i + 1, colName.data(), static_cast<sdint2>( COLNAME_SIZE ), &colNameLen, &colType, &colSize, &colScale, &nullable );

  if ( r != DSQL_SUCCESS )
  {
    *errorMessage = QCoreApplication::translate( "QgsDMResult", "Unable to describe column %1" ).arg( QString::number( i ) );
    return QSqlField();
  }

  slength unsignedFlag = DSQL_FALSE;

  r = dpi_col_attr( hStmt, i + 1, DSQL_DESC_UNSIGNED, 0, 0, 0, &unsignedFlag );

  if ( r != DSQL_SUCCESS )
  {
    qSqlWarning( QCoreApplication::translate( "QgsDMResult", "Unable to get column attributes for column %1"
            ).arg( QString::number( i ) ), hStmt );
  }

  QString qColName = QString::fromUtf8( ( const char* )colName.constData() );

  mSqlType.append( colType );
  QVariant::Type type = qDecodeDMType( colType, unsignedFlag == DSQL_FALSE );
  QSqlField f( qColName, type );
  f.setSqlType( colType );
  f.setLength( colSize == 0 ? -1 : static_cast<int>( colSize ) );
  f.setPrecision( colScale == 0 ? -1 : static_cast<int>( colScale ) );
  if ( nullable == DSQL_NO_NULLS )
    f.setRequired( true );
  else if ( nullable == DSQL_NULLABLE )
    f.setRequired( false );
  // else we don't know
  f.setAutoValue( isAutoValue( hStmt, i ) );
  return f;
}

void QgsDMResult::finish()
{
  if ( isActive() ) {
    setLastError( QSqlError() );
    setAt( QSql::BeforeFirstRow );
    detachFromResultSet();
    setActive( false );
  }
}

QVariant QgsDMResult::value( int field )
{
  if ( isActive() && isValid() && ( field > -1 ) )
    return data( field );
  qWarning( "not positioned on a valid record" );
  return QVariant();
}

void QgsDMResult::getBinarydata( int field, byte* &data, slength &size )
{
  Q_D( QgsDMResult );
  slength         lengthIndicator;

  dpi_get_data( d->hStmt, field + 1, DSQL_C_BINARY, ( dpointer )data, size + 1, &lengthIndicator );
}

/* Functions for Get available data after fetch. */
QVariant QgsDMResult::data( int field )
{
  Q_D( QgsDMResult );
  if ( field >= d->rInf.count() || field < 0 )
  {
    qWarning() << "column" << field << "out of range";
    return QVariant();
  }

  if ( field < d->fieldCacheIdx )
  {
    if ( d->fieldCached.at( field ) == true )
      return d->fieldCache.at( field );
  }

  DPIRETURN   r = DSQL_SUCCESS;
  slength     lengthIndicator = 0;

  const QSqlField     info = d->rInf.field( field );
  QString str_val;

  switch ( info.type() )
  {
  case QVariant::LongLong:
    d->fieldCache[field] = qGetBigIntData( d->hStmt, field );
    break;
  case QVariant::ULongLong:
    d->fieldCache[field] = qGetBigIntData( d->hStmt, field, false );
    break;
  case QVariant::Int:
    d->fieldCache[field] = qGetIntData( d->hStmt, field );
    break;
  case QVariant::UInt:
    d->fieldCache[field] = qGetIntData( d->hStmt, field, false );
    break;
  case QVariant::Date:
  {
    dpi_date_t      dbuf;

    r = dpi_get_data( d->hStmt, field + 1, DSQL_C_DATE, ( dpointer )&dbuf, sizeof( dbuf ), &lengthIndicator );

    if ( ( r == DSQL_SUCCESS || r == DSQL_SUCCESS_WITH_INFO ) && ( lengthIndicator != DSQL_NULL_DATA ) )
      d->fieldCache[field] = QVariant( QDate( dbuf.year, dbuf.month, dbuf.day ) );
    else
      d->fieldCache[field] = QVariant( QVariant::Date );
  }
  break;
  case QVariant::Time:
  {
    dpi_timestamp_t  tbuf;

    r = dpi_get_data( d->hStmt, field + 1, DSQL_C_TIMESTAMP, ( dpointer )&tbuf, sizeof( tbuf ), &lengthIndicator );
    if ( ( r == DSQL_SUCCESS || r == DSQL_SUCCESS_WITH_INFO ) && ( lengthIndicator != DSQL_NULL_DATA ) )
      d->fieldCache[field] = QVariant( QTime( tbuf.hour, tbuf.minute, tbuf.second, tbuf.fraction / 1000000 ) );
    else
      d->fieldCache[field] = QVariant( QVariant::Time );
  }
  break;
  case QVariant::DateTime:
  {
    dpi_timestamp_t dtbuf;

    r = dpi_get_data( d->hStmt, field + 1, DSQL_C_TIMESTAMP, ( dpointer )&dtbuf, sizeof( dtbuf ), &lengthIndicator );
    if ( ( r == DSQL_SUCCESS || r == DSQL_SUCCESS_WITH_INFO ) && ( lengthIndicator != DSQL_NULL_DATA ) )
      d->fieldCache[field] = QVariant( QDateTime( QDate( dtbuf.year, dtbuf.month, dtbuf.day ), QTime( dtbuf.hour, dtbuf.minute, dtbuf.second, dtbuf.fraction / 1000000 ) ) );
    else
      d->fieldCache[field] = QVariant( QVariant::DateTime );
  }
  break;
  case QVariant::ByteArray:
    str_val += QString( "0x" ) + qGetBinaryData( d->hStmt, field, DSQL_C_NCHAR ).toString();
    d->fieldCache[field] = str_val;
    break;
  case QVariant::String:
    if ( mSqlType[field] == DSQL_ARRAY || mSqlType[field] == DSQL_CLASS )
    {
      d->fieldCache[field] = qGetClassData( mObjData, field, mSqlType[field] );
    }
    else {
      d->fieldCache[field] = QVariant( qGetStringData( d->hStmt, field, info.length() ) );
    }
    break;
  case QVariant::Double:
    switch ( numericalPrecisionPolicy() )
    {
    case QSql::LowPrecisionInt32:
      d->fieldCache[field] = qGetIntData( d->hStmt, field );
      break;
    case QSql::LowPrecisionInt64:
      d->fieldCache[field] = qGetBigIntData( d->hStmt, field );
      break;
    case QSql::LowPrecisionDouble:
      d->fieldCache[field] = qGetDoubleData( d->hStmt, field );
      break;
    case QSql::HighPrecision:
      d->fieldCache[field] = qGetStringData( d->hStmt, field, info.length() );
      break;
    }
    break;
  default:
    d->fieldCache[field] = QVariant( qGetStringData( d->hStmt, field, info.length() ) );
    break;
  }

  if ( field >= d->fieldCacheIdx )
    d->fieldCacheIdx = field + 1;
  d->fieldCached[field] = true;

  return d->fieldCache[field];
}

bool QgsDMResult::isNull( int field )
{
  Q_D( const QgsDMResult );

  if ( field < 0 || field >= d->fieldCache.size() )
    return true;

  // since there is no good way to find out whether the value is NULL
  // without fetching the field we'll fetch it here.
  // ( data() also sets the NULL flag )
  if ( d->fieldCached.at( field ) == false )
    data( field );
  return d->fieldCache.at( field ).isNull();
}

int QgsDMResult::size()
{
  Q_D( QgsDMResult );
  sdint8      affectedRowCount = 0;
  DPIRETURN   r;

  if ( isActive() && driver()->hasFeature( QSqlDriver::QuerySize ) )
  {
    r = dpi_row_count( d->hStmt, &affectedRowCount );
    if ( r == DSQL_SUCCESS )
      return ( int )affectedRowCount;
    else
      qSqlWarning( QCoreApplication::translate( "QgsDMResult", "Unable to count affected rows" ), d );
  }

  return -1;
}

int QgsDMResult::numRowsAffected()
{
  Q_D( QgsDMResult );
  sdint8      affectedRowCount = 0;
  DPIRETURN   r;

  r = dpi_row_count( d->hStmt, &affectedRowCount );
  if ( r == DSQL_SUCCESS )
    return ( int )affectedRowCount;
  else
    qSqlWarning( QCoreApplication::translate( "QgsDMResult", "Unable to count affected rows" ), d );

  return -1;
}

bool QgsDMResult::prepare( const QString &query )
{
  Q_D( QgsDMResult );
  setActive( false );
  setAt( QSql::BeforeFirstRow );

  d->rInf.clear();
  mResMsg.clear();

  if ( d->hStmt && d->isStmtHandleValid() )
  {
    d->r = dpi_free_stmt( d->hStmt );
    d->hStmt = NULL;

    if ( d->r != DSQL_SUCCESS && d->r != DSQL_INVALID_HANDLE )
    {
      qSqlWarning( QCoreApplication::translate( "QgsDMResult", "Unable to free statement handle" ), d );
      return false;
    }
  }

  d->r = dpi_alloc_stmt( d->dpDbc(), &d->hStmt );
  if ( d->r != DSQL_SUCCESS )
  {
    mResMsg = QCoreApplication::translate( "QgsDMResult", "Unable to allocate statement handle" );
    qSqlWarning( mResMsg, d );
    return false;
  }

  d->updateStmtHandleState();

  if ( isForwardOnly() )
  {
    d->r = dpi_set_stmt_attr( d->hStmt, DSQL_ATTR_CURSOR_TYPE, ( dpointer )DSQL_CURSOR_FORWARD_ONLY, 0 );
  }
  else
  {
    d->r = dpi_set_stmt_attr( d->hStmt, DSQL_ATTR_CURSOR_TYPE, ( dpointer )DSQL_CURSOR_STATIC, 0 );
  }
  if ( !execstatus() )
  {
    setLastError( qMakeError( QSqlError::StatementError, d));
    mResMsg = lastError().text();
    return false;
  }

  d->r = dpi_prepare( d->hStmt, ( sdbyte* )( query.toUtf8().data() ) );

  if ( !execstatus() )
  {
    setLastError( qMakeError( QSqlError::StatementError, d ) );
    mResMsg = lastError().text();
    return false;
  }

  return true;
}

bool QgsDMResult::exec()
{
  Q_D( QgsDMResult );
  setActive( false );
  setAt( QSql::BeforeFirstRow );

  d->rInf.clear();
  d->fieldCache.clear();
  d->fieldCached.clear();
  d->fieldCacheIdx = 0;
  mResMsg.clear();

  if ( !d->hStmt )
  {
    mResMsg = QLatin1String( "exec: No statement handle available" );
    qSqlWarning( mResMsg, d );
    mResultStatus = DmResFatalError;
    return false;
  }

  /*if ( isSelect() )
    d->r = dpi_close_cursor( d->hStmt );*/

  d->r = dpi_exec( d->hStmt );
  setResStatus();
  if ( d->r != DSQL_SUCCESS && d->r != DSQL_SUCCESS_WITH_INFO )
  {
    setLastError( qMakeError( QSqlError::StatementError, d ) );
    mResMsg = lastError().text();

    return false;
  }

  ulength     isScrollable = 0;


  dpi_get_stmt_attr( d->hStmt, DSQL_ATTR_CURSOR_SCROLLABLE, &isScrollable, sizeof( isScrollable ), NULL );
  setForwardOnly( isScrollable == DSQL_NONSCROLLABLE );

  mColNum = 0;
  dpi_number_columns( d->hStmt, &mColNum );
  if ( mColNum )
  {
    d->fieldCache.resize( mColNum );
    d->fieldCached.resize( mColNum );

    setSelect( true );

    if(!mTypeName.isEmpty() )
      mTypeName.clear();
    if(!mSqlType.isEmpty() )
      mSqlType.clear();
    for ( int i = 0; i < mColNum; ++i )
    {
      d->rInf.append( qMakeFieldInfo( d, i ) );
      d->fieldCached[i] = false;
      if ( ftype( i ) == DSQL_CLASS )
        mGeoType.insert( i,getGeoType( i ) );
    }
  }
  else
  {
    setSelect( false );
  }

  setActive( true );

  //get out parameters
  if ( !hasOutValues() )
    return true;

  return true;
}

int QgsDMResult::execstatus()
{
  Q_D( QgsDMResult );
  
  return d->r == DSQL_SUCCESS || d->r == DSQL_SUCCESS_WITH_INFO;
}

void QgsDMResult::setResStatus()
{
  Q_D( QgsDMResult );

  if ( d->r == DSQL_SUCCESS )
    mResultStatus = DmResCommandOk;
  else if ( d->r == DSQL_SUCCESS_WITH_INFO )
    mResultStatus = DmResSuccessInfo;
  else
    mResultStatus = DmResFatalError;
}

bool QgsDMResult::execBatch( bool arraybind )
{
  Q_UNUSED( arraybind )
  Q_D( QgsDMResult );
  setActive( false );
  setAt( QSql::BeforeFirstRow );

  d->rInf.clear();
  d->fieldCache.clear();
  d->fieldCached.clear();
  d->fieldCacheIdx = 0;

  if ( !d->hStmt )
  {
    qSqlWarning( QLatin1String( "exec: No statement handle available" ), d );
    return false;
  }

  if ( isSelect() )
    dpi_close_cursor( d->hStmt );

  // bind parameters - only positional binding allowed
  QVector<QVariant> &values = boundValues();

  if ( values.isEmpty() )
    return false;

  int                 i;
  DPIRETURN           r;
  ulength             recordCount = 1;
  slength             length;
  ulength             precision;
  sdint2              bindDtype;

  QVector<DMBatchColumn> columns( values.count() );
  DMBatchCleanupHandler cleaner( columns );

  //record rows
  if ( values.count() != 0 )
    recordCount = values.at( 0 ).toList().count();

  r = dpi_set_stmt_attr( d->hStmt, DSQL_ATTR_PARAMSET_SIZE, ( dpointer )recordCount, 0 );

  for ( i = 0; i < values.count(); ++i )
  {
    if ( bindValueType( i ) & QSql::Out )
      values[i].detach();

    DMBatchColumn &col = columns[i];

    col.recordCount = values.at( i ).toList().count();

    if ( col.recordCount > recordCount )
    {
      col.recordCount = recordCount;
    }

    col.indicators = new slength[recordCount];

    int type = values.at( i ).toList().at( 0 ).type();

    switch ( type )
    {
    case QVariant::Date:
      col.maxLen = sizeof( dpi_date_t );
      col.data = new char[col.maxLen * col.recordCount];

      for ( uint row = 0; row < col.recordCount; ++row )
      {
        const QVariant &val = values.at( i ).toList().at( row );

        if ( val.isNull() )
        {
          col.indicators[row] = DSQL_NULL_DATA;
        }
        else
        {
          col.indicators[row] = 0;

          dpi_date_t *dt = ( dpi_date_t* )( col.data + ( col.maxLen * row ) );
          QDate qdt = val.toDate();

          dt->year = qdt.year();
          dt->month = qdt.month();
          dt->day = qdt.day();
        }
      }

      for ( uint rows = col.recordCount; rows < recordCount; rows++)
      {
        col.indicators[rows] = DSQL_NULL_DATA;
      }

      r = dpi_bind_param( d->hStmt, static_cast<sdint2>( i + 1 ), qParamType[bindValueType( i ) & QSql::InOut], DSQL_C_DATE, DSQL_DATE, 10, 0, col.data, col.maxLen, col.indicators );

      break;
    case QVariant::Time:
      col.maxLen = sizeof( dpi_timestamp_t );
      col.data = new char[col.maxLen * col.recordCount];

      for ( uint row = 0; row < col.recordCount; ++row )
      {
        const QVariant &val = values.at( i ).toList().at( row );

        if ( val.isNull() )
        {
          col.indicators[row] = DSQL_NULL_DATA;
        }
        else
        {
          col.indicators[row] = 0;

          dpi_timestamp_t *dt = ( dpi_timestamp_t* )( col.data + ( col.maxLen * row ) );
          QTime             qdt = val.toTime();

          dt->year = 1700;
          dt->month = 1;
          dt->day = 1;
          dt->hour = qdt.hour();
          dt->minute = qdt.minute();
          dt->second = qdt.second();
          dt->fraction = qdt.msec() * 1000000;
        }
      }

      for ( uint rows = col.recordCount; rows < recordCount; rows++)
      {
        col.indicators[rows] = DSQL_NULL_DATA;
      }

      r = dpi_bind_param( d->hStmt, static_cast<sdint2>( i + 1 ), qParamType[bindValueType( i ) & QSql::InOut], DSQL_C_TIMESTAMP, DSQL_TIMESTAMP, 18, 0, col.data, col.maxLen, col.indicators );

      break;
    case QVariant::DateTime:
      col.maxLen = sizeof( dpi_timestamp_t );
      col.data = new char[col.maxLen * col.recordCount];

      for ( uint row = 0; row < col.recordCount; ++row )
      {
        const QVariant &val = values.at( i ).toList().at( row );

        if ( val.isNull() )
        {
          col.indicators[row] = DSQL_NULL_DATA;
        }
        else
        {
          col.indicators[row] = 0;

          dpi_timestamp_t *dt = ( dpi_timestamp_t* )( col.data + ( col.maxLen * row ) );
          QDateTime           qdt = val.toDateTime();

          dt->year = qdt.date().year();
          dt->month = qdt.date().month();
          dt->day = qdt.date().day();
          dt->hour = qdt.time().hour();
          dt->minute = qdt.time().minute();
          dt->second = qdt.time().second();
          dt->fraction = qdt.time().msec() * 1000000;
        }
      }

      for ( uint rows = col.recordCount; rows < recordCount; rows++)
      {
        col.indicators[rows] = DSQL_NULL_DATA;
      }

      r = dpi_bind_param( d->hStmt, static_cast<sdint2>( i + 1 ), qParamType[bindValueType( i ) & QSql::InOut], DSQL_C_TIMESTAMP, DSQL_TIMESTAMP, 26, 6, col.data, col.maxLen, col.indicators );

      break;
    case QVariant::Int:
      col.maxLen = sizeof( int );
      col.data = new char[col.maxLen * col.recordCount];

      for ( uint row = 0; row < col.recordCount; ++row )
      {
        const QVariant &val = values.at( i ).toList().at( row );

        if ( val.isNull() )
        {
          col.indicators[row] = DSQL_NULL_DATA;
        }
        else
        {
          col.indicators[row] = 0;

          char *dataPtr = col.data + ( col.maxLen * row );

          *( int* )( dataPtr ) = val.toInt();
        }
      }

      for ( uint rows = col.recordCount; rows < recordCount; rows++)
      {
        col.indicators[rows] = DSQL_NULL_DATA;
      }

      r = dpi_bind_param( d->hStmt, static_cast<sdint2>( i + 1 ), qParamType[bindValueType( i ) & QSql::InOut], DSQL_C_SLONG, DSQL_INT, 10, 0, col.data, col.maxLen, col.indicators );

      break;
    case QVariant::UInt:
      col.maxLen = sizeof( uint );
      col.data = new char[col.maxLen * col.recordCount];

      for ( uint row = 0; row < col.recordCount; ++row )
      {
        const QVariant &val = values.at( i ).toList().at( row );

        if ( val.isNull() )
        {
          col.indicators[row] = DSQL_NULL_DATA;
        }
        else
        {
          col.indicators[row] = 0;

          char *dataPtr = col.data + ( col.maxLen * row );

          *( uint* )( dataPtr ) = val.toUInt();
        }
      }

      for ( uint rows = col.recordCount; rows < recordCount; rows++)
      {
        col.indicators[rows] = DSQL_NULL_DATA;
      }

      r = dpi_bind_param( d->hStmt, static_cast<sdint2>( i + 1 ), qParamType[bindValueType( i ) & QSql::InOut], DSQL_C_ULONG, DSQL_DEC, 10, 0, col.data, col.maxLen, col.indicators );

      break;
    case QVariant::Double:
      col.maxLen = sizeof( double );
      col.data = new char[col.maxLen * col.recordCount];

      for ( uint row = 0; row < col.recordCount; ++row )
      {
        const QVariant &val = values.at( i ).toList().at( row );

        if ( val.isNull() )
        {
          col.indicators[row] = DSQL_NULL_DATA;
        }
        else
        {
          col.indicators[row] = 0;

          char *dataPtr = col.data + ( col.maxLen * row );

          *( double* )( dataPtr ) = val.toDouble();
        }
      }

      for ( uint rows = col.recordCount; rows < recordCount; rows++)
      {
        col.indicators[rows] = DSQL_NULL_DATA;
      }

      r = dpi_bind_param( d->hStmt, static_cast<sdint2>( i + 1 ), qParamType[bindValueType( i ) & QSql::InOut], DSQL_C_DOUBLE, DSQL_DOUBLE, 53, 0, col.data, col.maxLen, col.indicators );

      break;
    case QVariant::LongLong:
      col.maxLen = sizeof( long long );
      col.data = new char[col.maxLen * col.recordCount];

      for ( uint row = 0; row < col.recordCount; ++row )
      {
        const QVariant &val = values.at( i ).toList().at( row );

        if ( val.isNull() )
        {
          col.indicators[row] = DSQL_NULL_DATA;
        }
        else
        {
          col.indicators[row] = 0;

          char *dataPtr = col.data + ( col.maxLen * row );
          memcpy( dataPtr, val.constData(), col.maxLen );
        }
      }

      for ( uint rows = col.recordCount; rows < recordCount; rows++)
      {
        col.indicators[rows] = DSQL_NULL_DATA;
      }

      r = dpi_bind_param( d->hStmt, static_cast<sdint2>( i + 1 ), qParamType[bindValueType( i ) & QSql::InOut], DSQL_C_SBIGINT, DSQL_BIGINT, 19, 0, col.data, col.maxLen, col.indicators );

      break;
    case QVariant::ULongLong:
      col.maxLen = sizeof( long long );
      col.data = new char[col.maxLen * col.recordCount];

      for ( uint row = 0; row < col.recordCount; ++row )
      {
        const QVariant &val = values.at( i ).toList().at( row );

        if ( val.isNull() )
        {
          col.indicators[row] = DSQL_NULL_DATA;
        }
        else
        {
          col.indicators[row] = 0;

          char *dataPtr = col.data + ( col.maxLen * row );

          memcpy( dataPtr, val.constData(), col.maxLen );
        }
      }
      for ( uint rows = col.recordCount; rows < recordCount; rows++)
      {
        col.indicators[rows] = DSQL_NULL_DATA;
      }

      r = dpi_bind_param( d->hStmt, static_cast<sdint2>( i + 1 ), qParamType[bindValueType( i ) & QSql::InOut], DSQL_C_UBIGINT, DSQL_BIGINT, 20, 0, col.data, col.maxLen, col.indicators );

      break;
    case QVariant::ByteArray:
      col.maxLen = 1;

      for ( uint j = 0; j < col.recordCount; ++j )
      {
        const QVariant &val = values.at( i ).toList().at( j );

        if ( !val.isNull() )
        {
          length = val.toByteArray().size();
          if ( length > col.maxLen )
            col.maxLen = length;
        }
      }

      col.data = new char[col.maxLen * col.recordCount];

      for ( uint row = 0; row < col.recordCount; ++row )
      {
        const QVariant &val = values.at( i ).toList().at( row );

        if ( val.isNull() )
        {
          col.indicators[row] = DSQL_NULL_DATA;
        }
        else
        {
          const QByteArray    ba = val.toByteArray();

          col.indicators[row] = ba.size();

          char *dataPtr = col.data + ( col.maxLen * row );

          memcpy( dataPtr, ba.constData(), ba.size() );
        }
      }

      for ( uint rows = col.recordCount; rows < recordCount; rows++)
      {
        col.indicators[rows] = DSQL_NULL_DATA;
      }

      precision = col.maxLen;
      bindDtype = DSQL_VARBINARY;

      if ( col.maxLen > 32767 )
      {
        bindDtype = DSQL_BLOB;
        precision = 0x7FFFFFFF;
      }

      r = dpi_bind_param( d->hStmt, static_cast<sdint2>( i + 1 ), qParamType[bindValueType( i ) & QSql::InOut], DSQL_C_BINARY, bindDtype, precision, 0, col.data, col.maxLen, col.indicators );

      break;
    case QVariant::Bool:
      col.maxLen = sizeof( bool );
      col.data = new char[col.maxLen * col.recordCount];

      for ( uint row = 0; row < col.recordCount; ++row )
      {
        const QVariant &val = values.at( i ).toList().at( row );

        if ( val.isNull() )
        {
          col.indicators[row] = DSQL_NULL_DATA;
        }
        else
        {
          col.indicators[row] = 0;

          char *dataPtr = col.data + ( col.maxLen * row );

          memcpy( dataPtr, val.constData(), col.maxLen );
        }
      }

      for ( uint rows = col.recordCount; rows < recordCount; rows++)
      {
        col.indicators[rows] = DSQL_NULL_DATA;
      }

      r = dpi_bind_param( d->hStmt, static_cast<sdint2>( i + 1 ), qParamType[bindValueType( i ) & QSql::InOut], DSQL_C_BIT, DSQL_BIT, 1, 0, col.data, col.maxLen, col.indicators );

      break;
    case QVariant::String:
      col.maxLen = 1;

      for ( uint j = 0; j < col.recordCount; ++j )
      {
        const QVariant &val = values.at( i ).toList().at( j );

        if ( !val.isNull() )
        {
          length = val.toString().toUtf8().length();
          if ( length > col.maxLen )
            col.maxLen = length;
        }
      }

      col.maxLen += 1;
      col.data = new char[col.maxLen * col.recordCount];

      for ( uint row = 0; row < col.recordCount; ++row )
      {
        const QVariant &val = values.at( i ).toList().at( row );

        if ( val.isNull() )
        {
          col.indicators[row] = DSQL_NULL_DATA;
        }
        else
        {
          const QString s = val.toString();

          col.indicators[row] = s.toUtf8().length();

          char *dataPtr = col.data + ( col.maxLen * row );

          memcpy( dataPtr, s.toUtf8(), s.toUtf8().length() );
        }
      }

      for ( uint rows = col.recordCount; rows < recordCount; rows++)
      {
        col.indicators[rows] = DSQL_NULL_DATA;
      }

      precision = col.maxLen;
      bindDtype = DSQL_VARCHAR;

      if ( precision > 32767 )
      {
        bindDtype = DSQL_CLOB;
        precision = 0x7FFFFFFF;
      }

      r = dpi_bind_param( d->hStmt, static_cast<sdint2>( i + 1 ), qParamType[bindValueType( i ) & QSql::InOut], DSQL_C_NCHAR, bindDtype, precision, 0, col.data, col.maxLen, col.indicators );

      break;
    default:
      col.maxLen = 1;

      for ( uint j = 0; j < col.recordCount; ++j )
      {
        const QVariant &val = values.at( i ).toList().at( j );

        if ( !val.isNull() )
        {
          length = val.toByteArray().size();
          if ( length > col.maxLen )
            col.maxLen = length;
        }
      }

      col.data = new char[col.maxLen * col.recordCount];

      for ( uint row = 0; row < col.recordCount; ++row )
      {
        const QVariant &val = values.at( i ).toList().at( row );

        if ( val.isNull() )
        {
          col.indicators[row] = DSQL_NULL_DATA;
        }
        else
        {
          const QByteArray    ba = val.toByteArray();

          col.indicators[row] = ba.size();

          char *dataPtr = col.data + ( col.maxLen * row );

          memcpy( dataPtr, ba.constData(), ba.size() );
        }
      }

      for ( uint rows = col.recordCount; rows < recordCount; rows++)
      {
        col.indicators[rows] = DSQL_NULL_DATA;
      }

      precision = col.maxLen;
      bindDtype = DSQL_VARBINARY;

      if ( col.maxLen > 32767 )
      {
        bindDtype = DSQL_BLOB;
        precision = 0x7FFFFFFF;
      }

      r = dpi_bind_param( d->hStmt, static_cast<sdint2>( i + 1 ), qParamType[bindValueType( i ) & QSql::InOut], DSQL_C_BINARY, bindDtype, precision, 0, col.data, col.maxLen, col.indicators );

      break;
    }

    if ( r != DSQL_SUCCESS )
    {
      qWarning() << "execBatch: unable to bind variable:" << qDMWarn( d );

      setLastError( qMakeError( QSqlError::StatementError, d ) );

      return false;
    }
  } //for

  r = dpi_exec( d->hStmt );

  if ( r != DSQL_SUCCESS && r != DSQL_SUCCESS_WITH_INFO )
  {
    qWarning() << "execBatch: Unable to execute statement:" << qDMWarn( d );

    setLastError( qMakeError( QSqlError::StatementError, d ) );

    return false;
  }

  ulength     isScrollable = 0;

  r = dpi_get_stmt_attr( d->hStmt, DSQL_ATTR_CURSOR_SCROLLABLE, &isScrollable, sizeof( isScrollable ), NULL );
  if ( r == DSQL_SUCCESS || r == DSQL_SUCCESS_WITH_INFO )
    setForwardOnly( isScrollable == DSQL_NONSCROLLABLE );

  sdint2      count;

  dpi_number_columns( d->hStmt, &count );
  if ( count )
  {
    d->fieldCache.resize( count );
    d->fieldCached.resize( count );

    setSelect( true );

    for ( int i = 0; i < count; ++i )
    {
      d->rInf.append( qMakeFieldInfo( d, i ) );
      d->fieldCached[i] = false;
    }
  }
  else
  {
    setSelect( false );
  }

  setActive( true );

  //get out parameters
  if ( !hasOutValues() )
    return true;

  for ( i = 0; i < values.count(); ++i )
  {
    QVariantList *list = static_cast<QVariantList *>( const_cast<void *>( values.at( i ).data() ) );
    char *data = columns[i].data;

    int type = values.at( i ).toList().at( 0 ).type();

    for ( uint r = 0; r < columns[i].recordCount; ++r )
    {
      switch ( type )
      {
      case QVariant::Date:
      {
        dpi_date_t *ds = ( dpi_date_t* )const_cast<char *>( data + r * columns[i].maxLen );
        ( *list )[r] = QVariant( QDate( ds->year, ds->month, ds->day ) );
        break;
      }
      case QVariant::Time:
      {
        dpi_time_t *dt = ( dpi_time_t* )const_cast<char *>( data + r * columns[i].maxLen );
        ( *list )[r] = QVariant( QTime( dt->hour, dt->minute, dt->second ) );
        break;
      }
      case QVariant::DateTime:
      {
        dpi_timestamp_t *dt = ( dpi_timestamp_t* )const_cast<char *>( data + r * columns[i].maxLen );
        ( *list )[r] = QVariant( QDateTime( QDate( dt->year, dt->month, dt->day ), QTime( dt->hour, dt->minute, dt->second, dt->fraction / 1000000 ) ) );
        break;
      }
      case QVariant::Bool:
        ( *list )[r] = QByteArray( data + r * columns[i].maxLen, columns[i].maxLen );
        break;
      case QVariant::Int:
        ( *list )[r] = *( int* )( data + r * columns[i].maxLen );
        break;
      case QVariant::UInt:
        ( *list )[r] = *( uint* )( data + r * columns[i].maxLen );
        break;
      case QVariant::Double:
        ( *list )[r] = *( double* )( data + r * columns[i].maxLen );
        break;
      case QVariant::ByteArray:
        ( *list )[r] = QByteArray( data + r * columns[i].maxLen, columns[i].maxLen );
        break;
      case QVariant::LongLong:
        ( *list )[r] = QByteArray( data + r * columns[i].maxLen, columns[i].maxLen );
        break;
      case QVariant::ULongLong:
        ( *list )[r] = QByteArray( data + r * columns[i].maxLen, columns[i].maxLen );
        break;
      case QVariant::String:
      default:
      {
        if ( bindValueType( i ) & QSql::Out )
          ( *list )[r] = QByteArray( data + r * columns[i].maxLen, columns[i].maxLen );
        break;
      }
      }

      if ( columns[i].indicators[0] == DSQL_NULL_DATA )
        ( *list )[r] = QVariant( type );
    }//for       
  }

  return true;
}

QSqlRecord QgsDMResult::record() const
{
  Q_D( const QgsDMResult );
  if ( !isActive() || !isSelect() )
    return QSqlRecord();

  return d->rInf;
}

QVariant QgsDMResult::lastInsertId() const
{
  Q_D( const QgsDMResult );
  QString sql;

  sql = QLatin1String( "SELECT SCOPE_IDENTITY;" );

  QSqlQuery qry( driver()->createResult() );
  if ( qry.exec( sql ) && qry.next() )
  {
    QVariant res = qry.value( 0 );
    if( res.toInt() != 0 )
      return res;
  }
  else
    qSqlWarning( QCoreApplication::translate( "QgsDMResult", "Unable to get lastInsertId" ), d );

  return QVariant();
}

QVariant QgsDMResult::handle() const
{
  Q_D( const QgsDMResult );
  return QVariant( qRegisterMetaType<dhandle>( "dhandle" ), &d->hStmt );
}

bool QgsDMResult::nextResult()
{
  Q_D( QgsDMResult );
  setActive( false );
  setAt( QSql::BeforeFirstRow );
  d->rInf.clear();
  d->fieldCache.clear();
  d->fieldCached.clear();
  d->fieldCacheIdx = 0;
  setSelect( false );

  DPIRETURN       r = dpi_more_results( d->hStmt );
  if ( r != DSQL_SUCCESS )
  {
    if ( r == DSQL_SUCCESS_WITH_INFO )
    {
      int nativeCode = -1;

      QString message = qDMWarn( d, &nativeCode );
      qWarning() << "QgsDMResult::nextResult():" << message;
    }
    else
    {
      if ( r != DSQL_NO_DATA )
        setLastError( qMakeError( QSqlError::ConnectionError, d ) );

      return false;
    }
  }

  sdint2      count;

  dpi_number_columns( d->hStmt, &count );
  if ( count )
  {
    d->fieldCache.resize( count );
    d->fieldCached.resize( count );

    setSelect( true );

    for ( int i = 0; i < count; ++i )
    {
      d->rInf.append( qMakeFieldInfo( d, i ) );
      d->fieldCached[i] = false;
    }
  }
  else
  {
    setSelect( false );
  }

  setActive( true );

  return true;
}

void QgsDMResult::virtual_hook( int id, void *data )
{
  QSqlResult::virtual_hook( id, data );
}

void QgsDMResult::detachFromResultSet()
{
  Q_D( QgsDMResult );
  if ( d->hStmt )
    dpi_close_cursor( d->hStmt );
}

////////////////////////////////////////
bool QgsDMDriverPrivate::setConnectionOptions( const QString &connOpts )
{
  // Set any connection attributes
  const       QStringList opts( connOpts.split( QLatin1Char( ';' ), QString::SkipEmptyParts ) );
  DPIRETURN   r = DSQL_SUCCESS;

  for ( int i = 0; i < opts.count(); ++i )
  {
    const QString   tmp( opts.at( i ) );
    int             idx;

    if ( ( idx = tmp.indexOf( QLatin1Char( '=' ) ) ) == -1 )
    {
      qWarning() << "QgsDMDriver::open: Illegal connect option value '" << tmp << '\'';
      continue;
    }

    const QString   opt( tmp.left( idx ) );
    const QString   val( tmp.mid( idx + 1 ).simplified() );
    udint4          v = 0;

    r = DSQL_SUCCESS;

    if ( opt.toUpper() == QLatin1String( "DSQL_ATTR_ACCESS_MODE" ) )
    {
      if ( val.toUpper() == QLatin1String( "DSQL_MODE_READ_ONLY" ) )
      {
        v = DSQL_MODE_READ_ONLY;
      }
      else if ( val.toUpper() == QLatin1String( "DSQL_MODE_READ_WRITE" ) )
      {
        v = DSQL_MODE_READ_WRITE;
      }
      else
      {
        qWarning() << "QgsDMDriver::open: Unknown option value '" << val << '\'';
        continue;
      }

      r = dpi_set_con_attr( hDbc, DSQL_ATTR_ACCESS_MODE, ( dpointer )( slength )v, 0 );
    }
    else if ( opt.toUpper() == QLatin1String( "DSQL_ATTR_CONNECTION_TIMEOUT" ) )
    {
      v = val.toUInt();

      r = dpi_set_con_attr( hDbc, DSQL_ATTR_CONNECTION_TIMEOUT, ( dpointer )( slength )v, 0 );
    }
    else if ( opt.toUpper() == QLatin1String( "DSQL_ATTR_LOGIN_TIMEOUT" ) )
    {
      v = val.toUInt();

      r = dpi_set_con_attr( hDbc, DSQL_ATTR_LOGIN_TIMEOUT, ( dpointer )( slength )v, 0 );
    }
    else
    {
      qWarning() << "QgsDMDriver::open: Unknown connection attribute '" << opt << '\'';
    }

    if ( r != DSQL_SUCCESS && r != DSQL_SUCCESS_WITH_INFO )
      qSqlWarning( QCoreApplication::translate( "QgsDMResult", " Unable to set connection attribute'%1'" ).arg( opt ), this );
  }

  return true;
}

void QgsDMDriverPrivate::splitTableQualifier( const QString &qualifier, QString &schema, QString &table )
{
  QStringList l = qualifier.split( QLatin1Char( '.' ) );
  if ( l.count() > 2 )
    return; // can't possibly be a valid table qualifier

  int         i = 0, n = l.count();

  if ( n == 1 )
  {
    table = qualifier;
  }
  else
  {
    for ( QStringList::Iterator it = l.begin(); it != l.end(); ++it )
    {
      if ( n == 2 )
      {
        if ( i == 0 )
        {
          schema = *it;
        }
        else if ( i == 1 )
        {
          table = *it;
        }
      }

      i++;
    }
  }
}

QgsDMDriver::QgsDMDriver( QObject *parent )
  : QSqlDriver(*new QgsDMDriverPrivate, parent )
{
}

QgsDMDriver::QgsDMDriver( dhandle env, dhandle con, QObject *parent )
  : QSqlDriver(*new QgsDMDriverPrivate, parent )
{
  Q_D( QgsDMDriver );

  d->hEnv = ( dhenv )env;
  d->hDbc = ( dhcon )con;
  if ( env && con )
  {
    setOpen( true );
    setOpenError( false );
  }
}

QgsDMDriver::~QgsDMDriver()
{
  cleanup();
}

bool QgsDMDriver::hasFeature( DriverFeature f ) const
{
  switch ( f )
  {
  case Transactions:
  case PreparedQueries:
  case PositionalPlaceholders:
  case FinishQuery:
  case LowPrecisionNumbers:
  case MultipleResultSets:
  case BLOB:
  case LastInsertId:
  case QuerySize:
    return true;
  case NamedPlaceholders:
  case BatchOperations:
  case SimpleLocking:
  case EventNotifications:
  case Unicode:
  case CancelQuery:
    return false;
  }

  return false;
}

bool QgsDMDriver::open( const QString &dbTrans, const QString &user, const QString &password, const QString &host, int port, const QString &connOpts )
{
  Q_D( QgsDMDriver );
  if ( isOpen() )
    close();

  DPIRETURN           r;

  r = dpi_alloc_env( &d->hEnv );
  if ( r != DSQL_SUCCESS && r != DSQL_SUCCESS_WITH_INFO )
  {
    connMsg = tr( "Unable to allocate environment handle" );
    qSqlWarning( connMsg, d );
    setOpenError( true );

    return false;
  }

  r = dpi_alloc_con( d->hEnv, &d->hDbc );
  if ( r != DSQL_SUCCESS && r != DSQL_SUCCESS_WITH_INFO )
  {
    connMsg = tr( "Unable to allocate connection handle" );
    qSqlWarning( connMsg, d );
    setOpenError( true );
    cleanup();
    return false;
  }

  if ( !d->setConnectionOptions( connOpts ) )
  {
    cleanup();
    return false;
  }

  sdint4      port_inner = static_cast<sdint4>( port );

  dpi_set_con_attr( d->hDbc, DSQL_ATTR_LOGIN_PORT, ( dpointer )( slength )port_inner, 0 );
  dpi_set_con_attr( d->hDbc, DSQL_ATTR_LOCAL_CODE, ( dpointer )PG_UTF8, 0 );
  if( dbTrans == QStringLiteral( "zh-Hans" ) )
    dpi_set_con_attr( d->hDbc, DSQL_ATTR_LANG_ID, ( dpointer )LANGUAGE_CN, 0 );
  else if ( dbTrans == QStringLiteral( "zh-Hant" ) )
    dpi_set_con_attr( d->hDbc, DSQL_ATTR_LANG_ID, ( dpointer )LANGUAGE_CNT_HK, 0 );
  else
    dpi_set_con_attr( d->hDbc, DSQL_ATTR_LANG_ID, ( dpointer )LANGUAGE_EN, 0 );

  r = dpi_login( d->hDbc, ( sdbyte* )( host.toUtf8().constData() ), ( sdbyte* )(  user.toUtf8().constData() ), ( sdbyte* )(  password.toUtf8().constData() ) );
  if ( r != DSQL_SUCCESS && r != DSQL_SUCCESS_WITH_INFO )
  {
    setLastError( qMakeError( QSqlError::ConnectionError, d ) );
    setOpenError( true );
    connMsg = lastError().text();
    cleanup();

    return false;
  }

  setOpen( true );
  setOpenError( false );

  return true;
}

void QgsDMDriver::close()
{
  cleanup();
  setOpen( false );
  setOpenError( false );
}

bool QgsDMDriver::isConnect()
{
  Q_D( QgsDMDriver );
  DPIRETURN           r;
  dhstmt stmt;
  QString sql = QStringLiteral( "select 1 from dual;" );
  r = dpi_alloc_stmt( d->hDbc, &stmt );
  if ( r == DSQL_SUCCESS )
  {
    r = dpi_exec_direct( stmt, ( sdbyte* )( sql.toUtf8().data() ) );
    dpi_free_stmt( stmt );
  }

  return r == DSQL_SUCCESS;
}

void QgsDMDriver::cleanup()
{
  Q_D( QgsDMDriver );
  DPIRETURN           r;

  if ( d->hDbc )
  {
    // Open statements/descriptors handles are automatically cleaned up by SQLDisconnect
    if ( isOpen() )
    {
      r = dpi_logout( d->hDbc );
      if ( r != DSQL_SUCCESS )
        qSqlWarning( tr( "Unable to disconnect datasource" ), d );
      else
        d->disconnectCount++;
    }

    r = dpi_free_con( d->hDbc );
    if ( r != DSQL_SUCCESS )
      qSqlWarning( tr( "Unable to free connection handle" ), d );

    d->hDbc = 0;
  }

  if ( d->hEnv )
  {
    r = dpi_free_env( d->hEnv );
    if ( r != DSQL_SUCCESS )
      qSqlWarning( tr( "Unable to free environment handle" ), d );

    d->hEnv = 0;
  }
}

QSqlResult *QgsDMDriver::createResult() const
{
  return new QgsDMResult( this );
}

bool QgsDMDriver::beginTransaction()
{
  Q_D( QgsDMDriver );
  if ( !isOpen() )
  {
    qWarning( "beginTransaction: Database not open" );
    return false;
  }

  DPIRETURN       r;
  r = dpi_set_con_attr( d->hDbc, DSQL_ATTR_AUTOCOMMIT, ( dpointer )DSQL_AUTOCOMMIT_OFF, 0 );
  if ( r != DSQL_SUCCESS )
  {
    connMsg = qMakeError( QSqlError::TransactionError, d ).text();

    return false;
  }

  return true;
}

bool QgsDMDriver::commitTransaction()
{
  Q_D( QgsDMDriver );
  if ( !isOpen() )
  {
    qWarning() << "commitTransaction: Database not open";

    return false;
  }

  DPIRETURN           r;

  r = dpi_commit( d->hDbc );
  if ( r != DSQL_SUCCESS )
  {
    connMsg = qMakeError( QSqlError::TransactionError, d ).text();

    return false;
  }

  return endTrans();
}

bool QgsDMDriver::rollbackTransaction()
{
  Q_D( QgsDMDriver );
  if ( !isOpen() )
  {
    qWarning() << "rollbackTransaction: Database not open";

    return false;
  }

  DPIRETURN           r;

  r = dpi_rollback( d->hDbc );
  if ( r != DSQL_SUCCESS )
  {
    connMsg = qMakeError( QSqlError::TransactionError, d ).text();

    return false;
  }

  return endTrans();
}

bool QgsDMDriver::endTrans()
{
  Q_D( QgsDMDriver );

  DPIRETURN       r;

  r = dpi_set_con_attr( d->hDbc, DSQL_ATTR_AUTOCOMMIT, ( dpointer )DSQL_AUTOCOMMIT_ON, 0 );
  if ( r != DSQL_SUCCESS )
  {
    connMsg = qMakeError( QSqlError::TransactionError, d ).text();

    return false;
  }

  return true;
}

QStringList QgsDMDriver::tables( QSql::TableType type ) const
{
  Q_D( const QgsDMDriver );
  QStringList         tl;

  if ( !isOpen() )
    return tl;

  dhstmt              hStmt;
  DPIRETURN           r;

  r = dpi_alloc_stmt( d->hDbc, &hStmt );
  if ( r != DSQL_SUCCESS )
  {
    qSqlWarning( tr( "Unable to allocate statement handle" ), d );

    return tl;
  }

  r = dpi_set_stmt_attr( hStmt, DSQL_ATTR_CURSOR_TYPE, ( dpointer )DSQL_CURSOR_FORWARD_ONLY, 0 );

  QStringList         tableType;

  if ( type & QSql::Tables )
    tableType += QLatin1String( "TABLE" );
  if ( type & QSql::Views )
    tableType += QLatin1String( "VIEW" );
  if ( type & QSql::SystemTables )
    tableType += QLatin1String( "SYSTEM TABLE" );

  if ( tableType.isEmpty() )
    return tl;

  QString joinedTableTypeString = tableType.join( QLatin1String( "," ) );

  r = dpi_tables( hStmt, NULL, 0, NULL, 0, NULL, 0, ( udbyte* )joinedTableTypeString.toUtf8().data(), DSQL_NTS );

  if ( r != DSQL_SUCCESS )
    qSqlWarning( tr( "Unable to acquire table list" ), d );

  r = dpi_fetch( hStmt, NULL );

  if ( r != DSQL_SUCCESS && r != DSQL_SUCCESS_WITH_INFO && r != DSQL_NO_DATA )
  {
    qWarning() << "failed to retrieve table/view list: (" << r << "," << qWarnDMHandle( DSQL_HANDLE_STMT, hStmt ) << ")";

    return QStringList();
  }

  while ( r == DSQL_SUCCESS )
  {
    QString fieldVal = qGetStringData( hStmt, 2, -1 );

    tl.append( fieldVal );

    r = dpi_fetch( hStmt, NULL );
  }

  r = dpi_free_stmt( hStmt );
  hStmt = NULL;
  if ( r != DSQL_SUCCESS )
    qSqlWarning( tr( "Unable to free statement handle" ), d );

  return tl;
}

QSqlIndex QgsDMDriver::primaryIndex( const QString &tablename ) const
{
  Q_D( const QgsDMDriver );
  QSqlIndex           index( tablename );

  if ( !isOpen() )
    return index;

  bool            usingSpecialColumns = false;
  QSqlRecord      rec = record( tablename );
  dhstmt 		    hStmt;
  DPIRETURN 	    r;

  r = dpi_alloc_stmt( d->hDbc, &hStmt );
  if ( r != DSQL_SUCCESS )
  {
    qSqlWarning( tr( "Unable to allocate statement handle" ), d );

    return index;
  }

  QString         schema, table;
  const_cast<QgsDMDriverPrivate *>( d )->splitTableQualifier( tablename, schema, table );

  if ( isIdentifierEscaped( schema, QSqlDriver::TableName ) )
    schema = stripDelimiters( schema, QSqlDriver::TableName );
  else
    schema = schema.toUpper();

  if ( isIdentifierEscaped( table, QSqlDriver::TableName ) )
    table = stripDelimiters( table, QSqlDriver::TableName );
  else
    table = table.toUpper();

  r = dpi_set_stmt_attr( hStmt, DSQL_ATTR_CURSOR_TYPE, ( dpointer )DSQL_CURSOR_FORWARD_ONLY, 0 );

  r = dpi_primarykeys( hStmt, NULL, 0, schema.length() == 0 ? NULL : ( udbyte* )( schema.toUtf8().data() ),
                DSQL_NTS, ( udbyte* )( table.toUtf8().data() ), DSQL_NTS );
  if ( r != DSQL_SUCCESS )
  {
    r = dpi_specialcolumns( hStmt, DSQL_BEST_ROWID, NULL, 0, schema.length() == 0 ? NULL : ( udbyte* )schema.toUtf8().data(),
                DSQL_NTS, ( udbyte* )table.toUtf8().data(), DSQL_NTS, DSQL_SCOPE_CURROW, DSQL_NULLABLE );

    if ( r != DSQL_SUCCESS )
    {
      qSqlWarning( tr( "Unable to acquire primary key list" ), d );
    }
    else
    {
      usingSpecialColumns = true;
    }
  }

  r = dpi_fetch( hStmt, NULL );

  int         fakeId = 0;
  QString     cName, idxName;

  // Store all fields in a StringList because some drivers can't detail fields in this FETCH loop
  while ( r == DSQL_SUCCESS )
  {
    if ( usingSpecialColumns )
    {
      cName = qGetStringData( hStmt, 1, -1 ); // column name
      idxName = QString::number( fakeId++); // invent a fake index name
    }
    else
    {
      cName = qGetStringData( hStmt, 3, -1 ); // column name
      idxName = qGetStringData( hStmt, 5, -1 ); // pk index name
    }

    index.append( rec.field( cName ) );
    index.setName( idxName );

    r = dpi_fetch( hStmt, NULL );
  }

  r = dpi_free_stmt( hStmt );
  hStmt = NULL;
  if ( r != DSQL_SUCCESS )
    qSqlWarning( tr( "Unable to free statement handle" ), d);

  return index;
}

QSqlRecord QgsDMDriver::record( const QString &tablename ) const
{
  Q_D( const QgsDMDriver );
  QSqlRecord      fil;

  if ( !isOpen() )
    return fil;

  dhstmt      hStmt;
  QString     schema, table;
  const_cast<QgsDMDriverPrivate *>( d )->splitTableQualifier( tablename, schema, table );

  if ( isIdentifierEscaped( schema, QSqlDriver::TableName ) )
    schema = stripDelimiters( schema, QSqlDriver::TableName );
  else
    schema = schema.toUpper();

  if ( isIdentifierEscaped( table, QSqlDriver::TableName ) )
    table = stripDelimiters( table, QSqlDriver::TableName );
  else
    table = table.toUpper();

  DPIRETURN   r;

  r = dpi_alloc_stmt( d->hDbc, &hStmt );
  if ( r != DSQL_SUCCESS )
  {
    qSqlWarning( tr( "Unable to allocate statement handle" ), d );
    return fil;
  }

  r = dpi_set_stmt_attr( hStmt, DSQL_ATTR_CURSOR_TYPE, ( dpointer )DSQL_CURSOR_FORWARD_ONLY, 0 );

  r = dpi_columns( hStmt, NULL, 0, schema.length() == 0 ? NULL : ( udbyte* )( schema.toUtf8().data() ),
              DSQL_NTS, ( udbyte* )( table.toUtf8().data() ), DSQL_NTS, NULL, 0 );
  if ( r != DSQL_SUCCESS )
    qSqlWarning( tr( "Unable to execute column list" ), d );

  r = dpi_fetch( hStmt, NULL );

  while ( r == DSQL_SUCCESS )
  {
    fil.append( QgsDMResult::qMakeFieldInfo( hStmt, d ) );

    r = dpi_fetch( hStmt, NULL );
  }

  r = dpi_free_stmt( hStmt );
  hStmt = NULL;
  if ( r != DSQL_SUCCESS )
    qSqlWarning( tr( "Unable to free statement handle" ), d );

  return fil;
}

QString QgsDMDriver::formatValue( const QSqlField &field, bool trimStrings ) const
{
  QString             r;

  if ( field.isNull() )
  {
    r = QLatin1String( "NULL" );
  }
  else if ( field.type() == QVariant::DateTime )
  {
    if ( field.value().toDateTime().isValid() )
    {
      QDate dt = field.value().toDateTime().date();
      QTime tm = field.value().toDateTime().time();

      // Dateformat has to be "yyyy-MM-dd hh:mm:ss", with leading zeroes if month or day < 10
      r = QLatin1String( "'" ) +
        QString::number( dt.year() ) + QLatin1Char( '-' ) +
        QString::number( dt.month() ).rightJustified( 2, QLatin1Char( '0' ), true ) +
        QLatin1Char( '-' ) +
        QString::number( dt.day() ).rightJustified( 2, QLatin1Char( '0' ), true ) +
        QLatin1Char( ' ' ) +
        tm.toString() +
        QLatin1String( "'" );
    }
    else
    {
      r = QLatin1String( "NULL" );
    }
  }
  else if ( field.type() == QVariant::ByteArray )
  {
    QByteArray  ba = field.value().toByteArray();
    QString     res;
    static const char hexchars[] = "0123456789abcdef";

    for ( int i = 0; i < ba.size(); ++i )
    {
      uchar s = ( uchar )ba[i];
      res += QLatin1Char( hexchars[s >> 4]);
      res += QLatin1Char( hexchars[s & 0x0f]);
    }

    r = QLatin1String( "0x" ) + res;
  }
  else
  {
    r = QSqlDriver::formatValue( field, trimStrings );
  }

  return r;
}

QVariant QgsDMDriver::handle() const
{
  Q_D( const QgsDMDriver );
  return QVariant( qRegisterMetaType<dhandle>( "dhandle" ), &d->hDbc );
}

QString QgsDMDriver::escapeIdentifier( const QString &identifier, IdentifierType ) const
{
  QChar quote = QLatin1Char( '"' );
  QString res = identifier;
  if ( !identifier.isEmpty() && !identifier.startsWith( quote ) && !identifier.endsWith( quote ) )
  {
    res.replace( quote, QLatin1String( "\"\"" ) );
    res.prepend( quote ).append( quote );
    res.replace( QLatin1Char( '.' ), QLatin1String( "\".\"" ) );
  }
  return res;
}

bool QgsDMDriver::isIdentifierEscaped( const QString &identifier, IdentifierType ) const
{
  QChar quote = QLatin1Char( '"' );
  return identifier.size() > 2
    && identifier.startsWith( quote ) //left delimited
    && identifier.endsWith( quote ); //right delimited
}

int QgsDMResult::nfields()
{
  return mColNum;
}

int QgsDMResult::ntuples()
{
  return mNtups;
}

void QgsDMResult::setNtuples()
{
  mNtups = numRowsAffected();
}

QString QgsDMResult::fname( int col )
{
  Q_D( QgsDMResult );

  return d->rInf.field( col ).name();
}

slength QgsDMResult::getLength( int field, int type )
{
  Q_D( QgsDMResult );

  slength         size = 0;
  if ( type == DSQL_C_TYPE_INVALID )
    dpi_get_data( d->hStmt, field + 1, qCTypefromSql( mSqlType[field] ), 0, 0, &size );
  else
    dpi_get_data( d->hStmt, field + 1, type, 0, 0, &size );

  return size;
}

QString QgsDMResult::ftableName( int col )
{
  QVarLengthArray<sdbyte> schemaName( TABLENAME_SIZE );
  QVarLengthArray<sdbyte> tableName( TABLENAME_SIZE );
  sdint2  schemaNameLen;
  sdint2  tableNameLen;
  QString name;
  DPIRETURN r;
  r = dpi_col_attr( *getStmt(), col + 1, DSQL_DESC_SCHEMA_NAME, schemaName.data(), TABLENAME_SIZE, &schemaNameLen, 0 );
  r = dpi_col_attr( *getStmt(), col + 1, DSQL_DESC_BASE_TABLE_NAME, tableName.data(), TABLENAME_SIZE, &tableNameLen, 0 );
  if ( r != DSQL_SUCCESS )
    return QString();

  return QStringLiteral( "%1.%2" ).arg( ( char* )schemaName.data() ).arg( ( char* )tableName.data() );
}

sdint4 QgsDMResult::ftable( QString schemaName, QString tableName )
{
  Q_D( QgsDMResult );

  dhstmt hstmt;
  int tableId = -1;

  QString sql = QStringLiteral( "select ID from SYS.VSYSOBJECTS where NAME = %1 and SCHID IN "
    "( select ID from SYS.VSYSOBJECTS where name = %2 and TYPE$ = 'SCH');"
  ).arg( quotedString( tableName ) ).arg( quotedString( schemaName ) );

  d->r = dpi_alloc_stmt( d->dpDbc(), &hstmt );
  d->r = dpi_exec_direct( hstmt, ( sdbyte* )sql.toUtf8().constData() );
  d->r = dpi_fetch( hstmt, NULL );

  d->r = dpi_get_data( hstmt, 1, DSQL_C_ULONG, &( tableId ), 0, 0 );

  dpi_free_stmt( hstmt );
  return tableId;
}

sdint4 QgsDMResult::ftable( int col )
{
  Q_D( QgsDMResult );

  QVarLengthArray<sdbyte> schName( TABLENAME_SIZE );
  QVarLengthArray<sdbyte> tabName( TABLENAME_SIZE );
  sdint2  schemaNameLen;
  sdint2  tableNameLen;

  d->r = dpi_col_attr( d->hStmt, col + 1, DSQL_DESC_SCHEMA_NAME, schName.data(), TABLENAME_SIZE, &schemaNameLen, 0 );
  d->r = dpi_col_attr( d->hStmt, col + 1, DSQL_DESC_BASE_TABLE_NAME, tabName.data(), TABLENAME_SIZE, &tableNameLen, 0 );

  QString schemaName = QString::fromUtf8( ( const char* )schName.constData() );
  QString tableName = QString::fromUtf8( ( const char* )tabName.constData() );

  return ftable( schemaName, tableName );
}

sdint4 QgsDMResult::ftype( int col )
{
  return mSqlType[col];
}

QString QgsDMResult::getSqlTypeName( int col, sdint4 type )
{
  QString typeName;
  switch ( type )
  {
  case DSQL_BIGINT:
    typeName = "bigint";
    break;
  case DSQL_BINARY:
    typeName = "binary";
    break;
  case DSQL_BIT:
    typeName = "bit";
    break;
  case DSQL_CHAR:
    typeName = "char";
    break;
  case DSQL_DATE:
    typeName = "date";
    break;
  case DSQL_DEC:
    typeName = "decimal";
    break;
  case DSQL_DOUBLE:
    typeName = "double";
    break;
  case DSQL_FLOAT:
    typeName = "float";
    break;
  case DSQL_CLASS:
    typeName = getGeoTypeName( mGeoType[col] );
    break;
  case DSQL_INT:
    typeName = "int";
    break;
  case DSQL_SMALLINT:
    typeName = "smallint";
    break;
  case DSQL_BLOB:
  case DSQL_CLOB:
    typeName = "text";
    break;
  case DSQL_TIME:
    typeName = "time";
    break;
  case DSQL_TIMESTAMP:
    typeName = "timestamp";
    break;
  case DSQL_TINYINT:
    typeName = "tinyint";
    break;
  case DSQL_VARBINARY:
    typeName = "varbinary";
    break;
  case DSQL_VARCHAR:
    typeName = "varchar";
    break;
  case DSQL_RECORD:
    typeName = "record";
    break;
  case DSQL_ARRAY:
    typeName = "array";
    break;
  case DSQL_SARRAY:
    typeName = "sarray";
    break;
  case DSQL_ROWID:
    typeName = "rowid";
    break;
  case DSQL_RSET:
    typeName = "rset";
    break;
  case DSQL_BFILE:
    typeName = "bfile";
    break;
  case DSQL_INTERVAL_DAY:
    typeName = "interval day";
    break;
  case DSQL_INTERVAL_DAY_TO_HOUR:
    typeName = "interval day() to hour";
    break;
  case DSQL_INTERVAL_DAY_TO_MINUTE:
    typeName = "interval day() to minute";
    break;
  case DSQL_INTERVAL_DAY_TO_SECOND:
    typeName = "interval day() to second";
    break;
  case DSQL_INTERVAL_HOUR:
    typeName = "interval hour";
    break;
  case DSQL_INTERVAL_HOUR_TO_MINUTE:
    typeName = "interval hour() to minute";
    break;
  case DSQL_INTERVAL_HOUR_TO_SECOND:
    typeName = "interval hour() to second";
    break;
  case DSQL_INTERVAL_MINUTE:
    typeName = "interval minute";
    break;
  case DSQL_INTERVAL_MINUTE_TO_SECOND:
    typeName = "interval minute() to second";
    break;
  case DSQL_INTERVAL_MONTH:
    typeName = "interval month";
    break;
  case DSQL_INTERVAL_SECOND:
    typeName = "interval second";
    break;
  case DSQL_INTERVAL_YEAR:
    typeName = "interval year";
    break;
  case DSQL_INTERVAL_YEAR_TO_MONTH:
    typeName = "interval year() to month";
    break;

  default:
    typeName = "unknown";
    break;
  }

  return typeName;
}

QString QgsDMResult::getGeoName( const uint &objClassId )
{
  switch ( objClassId )
  {
  case NDCT_CLSID_GEO2_ST_GEOGRAPHY:
    return QStringLiteral( "ST_GEOGRAPHY" );
  case NDCT_CLSID_GEO2_ST_GEOMETRY:
    return QStringLiteral( "ST_GEOMETRY" );
  case NDCT_CLSID_GEO2_ST_POINT:
    return QStringLiteral( "ST_POINT" );
  case NDCT_CLSID_GEO2_ST_LINE:
    return QStringLiteral( "ST_LINESTRING" );
  case NDCT_CLSID_GEO2_ST_POLYGON:
    return QStringLiteral( "ST_POLYGON" );
  case NDCT_CLSID_GEO2_ST_MULTIPOINT:
    return QStringLiteral( "ST_MULTIPOINT" );
  case NDCT_CLSID_GEO2_ST_MULTILINE:
    return QStringLiteral( "ST_MULTILINESTRING" );
  case NDCT_CLSID_GEO2_ST_MULTIPOLYGON:
    return QStringLiteral( "ST_MULTIPOLYGON" );
  case NDCT_CLSID_GEO2_ST_COLLECTION:
    return QStringLiteral( "ST_COLLECTION" );
  case NDCT_CLSID_GEO2_ST_CIRCSTRING:
    return QStringLiteral( "ST_CIRCULARSTRING" );
  case NDCT_CLSID_GEO2_ST_COMPOUND:
    return QStringLiteral( "ST_COMPOUNDCURVE" );
  case NDCT_CLSID_GEO2_ST_CURVEPOLY:
    return QStringLiteral( "ST_CURVEPOLYGON" );
  case NDCT_CLSID_GEO2_ST_MULTICURVE:
    return QStringLiteral( "ST_MULTICURVE" );
  case NDCT_CLSID_GEO2_ST_MULTISURFACE:
    return QStringLiteral( "ST_MULTISURFACE" );
  case NDCT_CLSID_GEO2_ST_POLYHEDRALSURFACE:
    return QStringLiteral( "ST_POLYHEDRALSURFACE" );
  case NDCT_CLSID_GEO2_ST_TRIANGLE:
    return QStringLiteral( "ST_TRIANGLE" );
  case NDCT_CLSID_GEO2_ST_TIN:
    return QStringLiteral( "ST_TIN" );
  default:
    return QStringLiteral( "unknown" );
  }
}

QString QgsDMResult::getGeoSubTypeName( int field )
{
  Q_D( const QgsDMResult );
  dhdesc     hdescCol;
  udint4		objClassId = 0;

  dpi_get_stmt_attr( d->hStmt, DSQL_ATTR_IMP_ROW_DESC, ( dpointer )&hdescCol, 0, 0 );

  dpi_get_desc_field( hdescCol, static_cast<sdint2>( field + 1 ), DSQL_DESC_OBJ_CLASSID, &objClassId, sizeof( udint4 ), NULL );

  return getGeoName( objClassId );
}

QString QgsDMResult::getGeoTypeName( sdint2 geoType )
{
  switch ( geoType )
  {
  case SctGeometry:
    return QStringLiteral( "geometry" );
  case SctGeography:
    return QStringLiteral( "geography" );
  case SctTopoGeometry:
    return QStringLiteral( "topoGeometry" );
  default:
    return QStringLiteral( "unknown" );
  }

}

QgsDamengGeometryColumnType QgsDMResult::getGeoType( int field )
{
  Q_D( const QgsDMResult );
  dhdesc     hdescCol;
  udint4		objClassId = 0;

  dpi_get_stmt_attr( d->hStmt, DSQL_ATTR_IMP_ROW_DESC, ( dpointer )&hdescCol, 0, 0 );

  dpi_get_desc_field( hdescCol, static_cast<sdint2>( field + 1 ), DSQL_DESC_OBJ_CLASSID, &objClassId, sizeof( udint4 ), NULL);

  if ( objClassId >= NDCT_CLSID_GEO2_ST_GEOMETRY && objClassId < NDCT_CLSID_GEO2_ST_GEOGRAPHY )
    return SctGeometry;

  if ( objClassId == NDCT_CLSID_GEO2_ST_GEOGRAPHY )
    return SctGeography;

  return SctNone;
}

QT_END_NAMESPACE
