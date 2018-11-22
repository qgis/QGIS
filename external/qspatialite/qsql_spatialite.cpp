/****************************************************************************
**
** Copyright (C) 2016 The Qt Company Ltd.
** Contact: https://www.qt.io/licensing/
**
** This file is part of the QtSql module of the Qt Toolkit.
**
** $QT_BEGIN_LICENSE:LGPL$
** Commercial License Usage
** Licensees holding valid commercial Qt licenses may use this file in
** accordance with the commercial license agreement provided with the
** Software or, alternatively, in accordance with the terms contained in
** a written agreement between you and The Qt Company. For licensing terms
** and conditions see https://www.qt.io/terms-conditions. For further
** information use the contact form at https://www.qt.io/contact-us.
**
** GNU Lesser General Public License Usage
** Alternatively, this file may be used under the terms of the GNU Lesser
** General Public License version 3 as published by the Free Software
** Foundation and appearing in the file LICENSE.LGPL3 included in the
** packaging of this file. Please review the following information to
** ensure the GNU Lesser General Public License version 3 requirements
** will be met: https://www.gnu.org/licenses/lgpl-3.0.html.
**
** GNU General Public License Usage
** Alternatively, this file may be used under the terms of the GNU
** General Public License version 2.0 or (at your option) the GNU General
** Public license version 3 or any later version approved by the KDE Free
** Qt Foundation. The licenses are as published by the Free Software
** Foundation and appearing in the file LICENSE.GPL2 and LICENSE.GPL3
** included in the packaging of this file. Please review the following
** information to ensure the GNU General Public License requirements will
** be met: https://www.gnu.org/licenses/gpl-2.0.html and
** https://www.gnu.org/licenses/gpl-3.0.html.
**
** $QT_END_LICENSE$
**
****************************************************************************/

#include "qsql_spatialite.h"

#include <qcoreapplication.h>
#include <qdatetime.h>
#include <qvariant.h>
#include <qsqlerror.h>
#include <qsqlfield.h>
#include <qsqlindex.h>
#include <qsqlquery.h>
#include <QtSql/private/qsqlcachedresult_p.h>
#include <QtSql/private/qsqldriver_p.h>
#include <qstringlist.h>
#include <qvector.h>
#include <qdebug.h>
#include <QTimeZone>

#include "qgsspatialiteutils.h"

#if defined Q_OS_WIN
# include <qt_windows.h>
#else
# include <unistd.h>
#endif

#include <sqlite3.h>
#include <functional>

QT_BEGIN_NAMESPACE

static QString _q_escapeIdentifier( const QString &identifier )
{
  QString res = identifier;
  if ( !identifier.isEmpty() && !identifier.startsWith( QLatin1Char( '"' ) ) && !identifier.endsWith( QLatin1Char( '"' ) ) )
  {
    res.replace( QLatin1Char( '"' ), QLatin1String( "\"\"" ) );
    res.prepend( QLatin1Char( '"' ) ).append( QLatin1Char( '"' ) );
    res.replace( QLatin1Char( '.' ), QLatin1String( "\".\"" ) );
  }
  return res;
}

static QVariant::Type qGetColumnType( const QString &tpName )
{
  const QString typeName = tpName.toLower();

  if ( typeName == QLatin1String( "integer" )
       || typeName == QLatin1String( "int" ) )
    return QVariant::Int;
  if ( typeName == QLatin1String( "double" )
       || typeName == QLatin1String( "float" )
       || typeName == QLatin1String( "real" )
       || typeName.startsWith( QLatin1String( "numeric" ) ) )
    return QVariant::Double;
  if ( typeName == QLatin1String( "blob" ) )
    return QVariant::ByteArray;
  return QVariant::String;
}

static QSqlError qMakeError( const spatialite_database_unique_ptr &access, const QString &descr, QSqlError::ErrorType type,
                             int errorCode = -1 )
{
  return QSqlError( descr, access.errorMessage(), type, QString::number( errorCode ) );
}

class QSpatiaLiteResultPrivate;

class QSpatiaLiteResult : public QSqlCachedResult
{
    Q_DECLARE_PRIVATE( QSpatiaLiteResult )
    friend class QSpatiaLiteDriver;

  public:
    explicit QSpatiaLiteResult( const QSpatiaLiteDriver *db );
    ~QSpatiaLiteResult();
  protected:
    bool gotoNext( QSqlCachedResult::ValueCache &row, int idx ) Q_DECL_OVERRIDE;
    bool reset( const QString &query ) Q_DECL_OVERRIDE;
    bool prepare( const QString &query ) Q_DECL_OVERRIDE;
    bool exec() Q_DECL_OVERRIDE;
    int size() Q_DECL_OVERRIDE;
    int numRowsAffected() Q_DECL_OVERRIDE;
    QVariant lastInsertId() const Q_DECL_OVERRIDE;
    QSqlRecord record() const Q_DECL_OVERRIDE;
    void detachFromResultSet() Q_DECL_OVERRIDE;
    void virtual_hook( int id, void *data ) Q_DECL_OVERRIDE;
};

class QSpatiaLiteDriverPrivate : public QSqlDriverPrivate
{
    Q_DECLARE_PUBLIC( QSpatiaLiteDriver )

  public:
    inline QSpatiaLiteDriverPrivate() : QSqlDriverPrivate() { dbmsType = QSqlDriver::SQLite; }
    spatialite_database_unique_ptr access;
    QList <QSpatiaLiteResult *> results;
    QStringList notificationid;
};


class QSpatiaLiteResultPrivate: public QSqlCachedResultPrivate
{
    Q_DECLARE_PUBLIC( QSpatiaLiteResult )

  public:
    Q_DECLARE_SQLDRIVER_PRIVATE( QSpatiaLiteDriver )
    QSpatiaLiteResultPrivate( QSpatiaLiteResult *q, const QSpatiaLiteDriver *drv );
    void cleanup();
    bool fetchNext( QSqlCachedResult::ValueCache &values, int idx, bool initialFetch );
    // initializes the recordInfo and the cache
    void initColumns( bool emptyResultset );
    void finalize();

    sqlite3_statement_unique_ptr stmt;

    bool skippedStatus; // the status of the fetchNext() that's skipped
    bool skipRow; // skip the next fetchNext()?
    QSqlRecord rInf;
    QVector<QVariant> firstRow;
};

QSpatiaLiteResultPrivate::QSpatiaLiteResultPrivate( QSpatiaLiteResult *q, const QSpatiaLiteDriver *drv )
  : QSqlCachedResultPrivate( q, drv )
  , skippedStatus( false )
  , skipRow( false )
{
}

void QSpatiaLiteResultPrivate::cleanup()
{
  Q_Q( QSpatiaLiteResult );
  finalize();
  rInf.clear();
  skippedStatus = false;
  skipRow = false;
  q->setAt( QSql::BeforeFirstRow );
  q->setActive( false );
  q->cleanup();
}

void QSpatiaLiteResultPrivate::finalize()
{
  stmt.reset();
}

void QSpatiaLiteResultPrivate::initColumns( bool emptyResultset )
{
  Q_Q( QSpatiaLiteResult );
  int nCols = stmt.columnCount();
  if ( nCols <= 0 )
    return;

  q->init( nCols );

  for ( int i = 0; i < nCols; ++i )
  {
    QString colName = stmt.columnName( i ).remove( QLatin1Char( '"' ) );

    // must use typeName for resolving the type to match QSqliteDriver::record
    QString typeName = QString( reinterpret_cast<const QChar *>(
                                  sqlite3_column_decltype16( stmt.get(), i ) ) );
    // sqlite3_column_type is documented to have undefined behavior if the result set is empty
    int stp = emptyResultset ? -1 : sqlite3_column_type( stmt.get(), i );

    QVariant::Type fieldType;

    if ( !typeName.isEmpty() )
    {
      fieldType = qGetColumnType( typeName );
    }
    else
    {
      // Get the proper type for the field based on stp value
      switch ( stp )
      {
        case SQLITE_INTEGER:
          fieldType = QVariant::Int;
          break;
        case SQLITE_FLOAT:
          fieldType = QVariant::Double;
          break;
        case SQLITE_BLOB:
          fieldType = QVariant::ByteArray;
          break;
        case SQLITE_TEXT:
          fieldType = QVariant::String;
          break;
        case SQLITE_NULL:
        default:
          fieldType = QVariant::Invalid;
          break;
      }
    }

    QSqlField fld( colName, fieldType );
    fld.setSqlType( stp );
    rInf.append( fld );
  }
}

bool QSpatiaLiteResultPrivate::fetchNext( QSqlCachedResult::ValueCache &values, int idx, bool initialFetch )
{
  Q_Q( QSpatiaLiteResult );
  int res;
  int i;

  if ( skipRow )
  {
    // already fetched
    Q_ASSERT( !initialFetch );
    skipRow = false;
    for ( int i = 0; i < firstRow.count(); i++ )
      values[i] = firstRow[i];
    return skippedStatus;
  }
  skipRow = initialFetch;

  if ( initialFetch )
  {
    firstRow.clear();
    firstRow.resize( stmt.columnCount() );
  }

  if ( !stmt )
  {
    q->setLastError( QSqlError( QCoreApplication::translate( "QSpatiaLiteResult", "Unable to fetch row" ),
                                QCoreApplication::translate( "QSpatiaLiteResult", "No query" ), QSqlError::ConnectionError ) );
    q->setAt( QSql::AfterLastRow );
    return false;
  }
  res = stmt.step();

  switch ( res )
  {
    case SQLITE_ROW:
      // check to see if should fill out columns
      if ( rInf.isEmpty() )
        // must be first call.
        initColumns( false );
      if ( idx < 0 && !initialFetch )
        return true;
      for ( i = 0; i < rInf.count(); ++i )
      {
        switch ( sqlite3_column_type( stmt.get(), i ) )
        {
          case SQLITE_BLOB:
            values[i + idx] = QByteArray( static_cast<const char *>(
                                            sqlite3_column_blob( stmt.get(), i ) ),
                                          sqlite3_column_bytes( stmt.get(), i ) );
            break;
          case SQLITE_INTEGER:
            values[i + idx] = stmt.columnAsInt64( i );
            break;
          case SQLITE_FLOAT:
            switch ( q->numericalPrecisionPolicy() )
            {
              case QSql::LowPrecisionInt32:
                values[i + idx] = stmt.columnAsInt64( i );
                break;
              case QSql::LowPrecisionInt64:
                values[i + idx] = stmt.columnAsInt64( i );
                break;
              case QSql::LowPrecisionDouble:
              case QSql::HighPrecision:
              default:
                values[i + idx] = stmt.columnAsDouble( i );
                break;
            };
            break;
          case SQLITE_NULL:
            values[i + idx] = QVariant( QVariant::String );
            break;
          default:
            values[i + idx] = stmt.columnAsText( i );
            break;
        }
      }
      return true;
    case SQLITE_DONE:
      if ( rInf.isEmpty() )
        // must be first call.
        initColumns( true );
      q->setAt( QSql::AfterLastRow );
      sqlite3_reset( stmt.get() );
      return false;
    case SQLITE_CONSTRAINT:
    case SQLITE_ERROR:
      // SQLITE_ERROR is a generic error code and we must call sqlite3_reset()
      // to get the specific error message.
      res = sqlite3_reset( stmt.get() );
      q->setLastError( qMakeError( drv_d_func()->access, QCoreApplication::translate( "QSpatiaLiteResult",
                                   "Unable to fetch row" ), QSqlError::ConnectionError, res ) );
      q->setAt( QSql::AfterLastRow );
      return false;
    case SQLITE_MISUSE:
    case SQLITE_BUSY:
    default:
      // something wrong, don't get col info, but still return false
      q->setLastError( qMakeError( drv_d_func()->access, QCoreApplication::translate( "QSpatiaLiteResult",
                                   "Unable to fetch row" ), QSqlError::ConnectionError, res ) );
      sqlite3_reset( stmt.get() );
      q->setAt( QSql::AfterLastRow );
      return false;
  }
  return false;
}

QSpatiaLiteResult::QSpatiaLiteResult( const QSpatiaLiteDriver *db )
  : QSqlCachedResult( *new QSpatiaLiteResultPrivate( this, db ) )
{
  Q_D( QSpatiaLiteResult );
  const_cast<QSpatiaLiteDriverPrivate *>( d->drv_d_func() )->results.append( this );
}

QSpatiaLiteResult::~QSpatiaLiteResult()
{
  Q_D( QSpatiaLiteResult );
  if ( d->drv_d_func() )
    const_cast<QSpatiaLiteDriverPrivate *>( d->drv_d_func() )->results.removeOne( this );
  d->cleanup();
}

void QSpatiaLiteResult::virtual_hook( int id, void *data )
{
  QSqlCachedResult::virtual_hook( id, data );
}

bool QSpatiaLiteResult::reset( const QString &query )
{
  if ( !prepare( query ) )
    return false;
  return exec();
}

bool QSpatiaLiteResult::prepare( const QString &query )
{
  Q_D( QSpatiaLiteResult );
  if ( !driver() || !driver()->isOpen() || driver()->isOpenError() )
    return false;

  d->cleanup();

  setSelect( false );

  int res;
  d->stmt = d->drv_d_func()->access.prepare( query, res );
  if ( res != SQLITE_OK )
  {
    setLastError( qMakeError( d->drv_d_func()->access, QCoreApplication::translate( "QSpatiaLiteResult",
                              "Unable to execute statement" ), QSqlError::StatementError, res ) );
    d->finalize();
    return false;
  }
  return true;
}

static QString secondsToOffset( int seconds )
{
  const QChar sign = ushort( seconds < 0 ? '-' : '+' );
  seconds = qAbs( seconds );
  const int hours = seconds / 3600;
  const int minutes = ( seconds % 3600 ) / 60;

  return QString( QStringLiteral( "%1%2:%3" ) ).arg( sign ).arg( hours, 2, 10, QLatin1Char( '0' ) ).arg( minutes, 2, 10, QLatin1Char( '0' ) );
}

static QString timespecToString( const QDateTime &dateTime )
{
  switch ( dateTime.timeSpec() )
  {
    case Qt::LocalTime:
      return QString();
    case Qt::UTC:
      return QStringLiteral( "Z" );
    case Qt::OffsetFromUTC:
      return secondsToOffset( dateTime.offsetFromUtc() );
    case Qt::TimeZone:
      return secondsToOffset( dateTime.timeZone().offsetFromUtc( dateTime ) );
    default:
      return QString();
  }
}

bool QSpatiaLiteResult::exec()
{
  Q_D( QSpatiaLiteResult );
  const QVector<QVariant> values = boundValues();

  d->skippedStatus = false;
  d->skipRow = false;
  d->rInf.clear();
  clearValues();
  setLastError( QSqlError() );

  int res = sqlite3_reset( d->stmt.get() );
  if ( res != SQLITE_OK )
  {
    setLastError( qMakeError( d->drv_d_func()->access, QCoreApplication::translate( "QSpatiaiteResult",
                              "Unable to reset statement" ), QSqlError::StatementError, res ) );
    d->finalize();
    return false;
  }
  int paramCount = sqlite3_bind_parameter_count( d->stmt.get() );
  if ( paramCount == values.count() )
  {
    for ( int i = 0; i < paramCount; ++i )
    {
      res = SQLITE_OK;
      const QVariant value = values.at( i );

      if ( value.isNull() )
      {
        res = sqlite3_bind_null( d->stmt.get(), i + 1 );
      }
      else
      {
        switch ( value.type() )
        {
          case QVariant::ByteArray:
          {
            const QByteArray *ba = static_cast<const QByteArray *>( value.constData() );
            res = sqlite3_bind_blob( d->stmt.get(), i + 1, ba->constData(),
                                     ba->size(), SQLITE_STATIC );
            break;
          }
          case QVariant::Int:
          case QVariant::Bool:
            res = sqlite3_bind_int( d->stmt.get(), i + 1, value.toInt() );
            break;
          case QVariant::Double:
            res = sqlite3_bind_double( d->stmt.get(), i + 1, value.toDouble() );
            break;
          case QVariant::UInt:
          case QVariant::LongLong:
            res = sqlite3_bind_int64( d->stmt.get(), i + 1, value.toLongLong() );
            break;
          case QVariant::DateTime:
          {
            const QDateTime dateTime = value.toDateTime();
            const QString str = dateTime.toString( QStringLiteral( "yyyy-MM-ddThh:mm:ss.zzz" ) + timespecToString( dateTime ) );
            res = sqlite3_bind_text16( d->stmt.get(), i + 1, str.utf16(),
                                       str.size() * sizeof( ushort ), SQLITE_TRANSIENT );
            break;
          }
          case QVariant::Time:
          {
            const QTime time = value.toTime();
            const QString str = time.toString( QStringLiteral( "hh:mm:ss.zzz" ) );
            res = sqlite3_bind_text16( d->stmt.get(), i + 1, str.utf16(),
                                       str.size() * sizeof( ushort ), SQLITE_TRANSIENT );
            break;
          }
          case QVariant::String:
          {
            // lifetime of string == lifetime of its qvariant
            const QString *str = static_cast<const QString *>( value.constData() );
            res = sqlite3_bind_text16( d->stmt.get(), i + 1, str->utf16(),
                                       ( str->size() ) * sizeof( QChar ), SQLITE_STATIC );
            break;
          }
          default:
          {
            QString str = value.toString();
            // SQLITE_TRANSIENT makes sure that sqlite buffers the data
            res = sqlite3_bind_text16( d->stmt.get(), i + 1, str.utf16(),
                                       ( str.size() ) * sizeof( QChar ), SQLITE_TRANSIENT );
            break;
          }
        }
      }
      if ( res != SQLITE_OK )
      {
        setLastError( qMakeError( d->drv_d_func()->access, QCoreApplication::translate( "QSpatiaLiteResult",
                                  "Unable to bind parameters" ), QSqlError::StatementError, res ) );
        d->finalize();
        return false;
      }
    }
  }
  else
  {
    setLastError( QSqlError( QCoreApplication::translate( "QSpatiaLiteResult",
                             "Parameter count mismatch" ), QString(), QSqlError::StatementError ) );
    return false;
  }
  d->skippedStatus = d->fetchNext( d->firstRow, 0, true );
  if ( lastError().isValid() )
  {
    setSelect( false );
    setActive( false );
    return false;
  }
  setSelect( !d->rInf.isEmpty() );
  setActive( true );
  return true;
}

bool QSpatiaLiteResult::gotoNext( QSqlCachedResult::ValueCache &row, int idx )
{
  Q_D( QSpatiaLiteResult );
  return d->fetchNext( row, idx, false );
}

int QSpatiaLiteResult::size()
{
  return -1;
}

int QSpatiaLiteResult::numRowsAffected()
{
  Q_D( const QSpatiaLiteResult );
  return sqlite3_changes( d->drv_d_func()->access.get() );
}

QVariant QSpatiaLiteResult::lastInsertId() const
{
  Q_D( const QSpatiaLiteResult );
  if ( isActive() )
  {
    qint64 id = sqlite3_last_insert_rowid( d->drv_d_func()->access.get() );
    if ( id )
      return id;
  }
  return QVariant();
}

QSqlRecord QSpatiaLiteResult::record() const
{
  Q_D( const QSpatiaLiteResult );
  if ( !isActive() || !isSelect() )
    return QSqlRecord();
  return d->rInf;
}

void QSpatiaLiteResult::detachFromResultSet()
{
  Q_D( QSpatiaLiteResult );
  if ( d->stmt )
    sqlite3_reset( d->stmt.get() );
}

/////////////////////////////////////////////////////////

QSpatiaLiteDriver::QSpatiaLiteDriver( QObject *parent )
  : QSqlDriver( *new QSpatiaLiteDriverPrivate, parent )
{
}

QSpatiaLiteDriver::~QSpatiaLiteDriver()
{
  close();
}

bool QSpatiaLiteDriver::hasFeature( DriverFeature f ) const
{
  switch ( f )
  {
    case BLOB:
    case Transactions:
    case Unicode:
    case LastInsertId:
    case PreparedQueries:
    case PositionalPlaceholders:
    case SimpleLocking:
    case FinishQuery:
    case LowPrecisionNumbers:
    case EventNotifications:
      return true;
    case QuerySize:
    case NamedPlaceholders:
    case BatchOperations:
    case MultipleResultSets:
    case CancelQuery:
      return false;
  }
  return false;
}

/*
   SQLite dbs have no user name, passwords, hosts or ports.
   just file names.
*/
bool QSpatiaLiteDriver::open( const QString &db, const QString &, const QString &, const QString &, int, const QString &conOpts )
{
  Q_D( QSpatiaLiteDriver );
  if ( isOpen() )
    close();


  int timeOut = 5000;
  bool sharedCache = false;
  bool openReadOnlyOption = false;
  bool openUriOption = false;

  const auto opts = conOpts.splitRef( QLatin1Char( ';' ) );
  for ( auto option : opts )
  {
    option = option.trimmed();
    if ( option.startsWith( QLatin1String( "QSQLITE_BUSY_TIMEOUT" ) ) )
    {
      option = option.mid( 20 ).trimmed();
      if ( option.startsWith( QLatin1Char( '=' ) ) )
      {
        bool ok;
        const int nt = option.mid( 1 ).trimmed().toInt( &ok );
        if ( ok )
          timeOut = nt;
      }
    }
    else if ( option == QLatin1String( "QSQLITE_OPEN_READONLY" ) )
    {
      openReadOnlyOption = true;
    }
    else if ( option == QLatin1String( "QSQLITE_OPEN_URI" ) )
    {
      openUriOption = true;
    }
    else if ( option == QLatin1String( "QSQLITE_ENABLE_SHARED_CACHE" ) )
    {
      sharedCache = true;
    }
  }

  int openMode = ( openReadOnlyOption ? SQLITE_OPEN_READONLY : ( SQLITE_OPEN_READWRITE | SQLITE_OPEN_CREATE ) );
  if ( openUriOption )
    openMode |= SQLITE_OPEN_URI;

  sqlite3_enable_shared_cache( sharedCache );

  if ( d->access.open_v2( db.toUtf8().constData(), openMode, nullptr ) == SQLITE_OK )
  {
    sqlite3_busy_timeout( d->access.get(), timeOut );
    setOpen( true );
    setOpenError( false );

    return true;
  }
  else
  {
    setLastError( qMakeError( d->access, tr( "Error opening database" ),
                              QSqlError::ConnectionError ) );
    setOpenError( true );
    return false;
  }
}

void QSpatiaLiteDriver::close()
{
  Q_D( QSpatiaLiteDriver );
  if ( isOpen() )
  {
    for ( QSpatiaLiteResult *result : qAsConst( d->results ) )
      result->d_func()->finalize();

    if ( d->access && ( d->notificationid.count() > 0 ) )
    {
      d->notificationid.clear();
      sqlite3_update_hook( d->access.get(), NULL, NULL );
    }

    d->access.reset();

    setOpen( false );
    setOpenError( false );
  }
}

QSqlResult *QSpatiaLiteDriver::createResult() const
{
  return new QSpatiaLiteResult( this );
}

bool QSpatiaLiteDriver::beginTransaction()
{
  if ( !isOpen() || isOpenError() )
    return false;

  QSqlQuery q( createResult() );
  if ( !q.exec( QLatin1String( "BEGIN" ) ) )
  {
    setLastError( QSqlError( tr( "Unable to begin transaction" ),
                             q.lastError().databaseText(), QSqlError::TransactionError ) );
    return false;
  }

  return true;
}

bool QSpatiaLiteDriver::commitTransaction()
{
  if ( !isOpen() || isOpenError() )
    return false;

  QSqlQuery q( createResult() );
  if ( !q.exec( QLatin1String( "COMMIT" ) ) )
  {
    setLastError( QSqlError( tr( "Unable to commit transaction" ),
                             q.lastError().databaseText(), QSqlError::TransactionError ) );
    return false;
  }

  return true;
}

bool QSpatiaLiteDriver::rollbackTransaction()
{
  if ( !isOpen() || isOpenError() )
    return false;

  QSqlQuery q( createResult() );
  if ( !q.exec( QLatin1String( "ROLLBACK" ) ) )
  {
    setLastError( QSqlError( tr( "Unable to rollback transaction" ),
                             q.lastError().databaseText(), QSqlError::TransactionError ) );
    return false;
  }

  return true;
}

QStringList QSpatiaLiteDriver::tables( QSql::TableType type ) const
{
  QStringList res;
  if ( !isOpen() )
    return res;

  QSqlQuery q( createResult() );
  q.setForwardOnly( true );

  QString sql = QLatin1String( "SELECT name FROM sqlite_master WHERE %1 "
                               "UNION ALL SELECT name FROM sqlite_temp_master WHERE %1" );
  if ( ( type & QSql::Tables ) && ( type & QSql::Views ) )
    sql = sql.arg( QLatin1String( "type='table' OR type='view'" ) );
  else if ( type & QSql::Tables )
    sql = sql.arg( QLatin1String( "type='table'" ) );
  else if ( type & QSql::Views )
    sql = sql.arg( QLatin1String( "type='view'" ) );
  else
    sql.clear();

  if ( !sql.isEmpty() && q.exec( sql ) )
  {
    while ( q.next() )
      res.append( q.value( 0 ).toString() );
  }

  if ( type & QSql::SystemTables )
  {
    // there are no internal tables beside this one:
    res.append( QLatin1String( "sqlite_master" ) );
  }

  return res;
}

static QSqlIndex qGetTableInfo( QSqlQuery &q, const QString &tableName, bool onlyPIndex = false )
{
  QString schema;
  QString table( tableName );
  int indexOfSeparator = tableName.indexOf( QLatin1Char( '.' ) );
  if ( indexOfSeparator > -1 )
  {
    schema = tableName.left( indexOfSeparator ).append( QLatin1Char( '.' ) );
    table = tableName.mid( indexOfSeparator + 1 );
  }
  q.exec( QLatin1String( "PRAGMA " ) + schema + QLatin1String( "table_info (" ) + _q_escapeIdentifier( table ) + QLatin1Char( ')' ) );

  QSqlIndex ind;
  while ( q.next() )
  {
    bool isPk = q.value( 5 ).toInt();
    if ( onlyPIndex && !isPk )
      continue;
    QString typeName = q.value( 2 ).toString().toLower();
    QSqlField fld( q.value( 1 ).toString(), qGetColumnType( typeName ) );
    if ( isPk && ( typeName == QLatin1String( "integer" ) ) )
      // INTEGER PRIMARY KEY fields are auto-generated in sqlite
      // INT PRIMARY KEY is not the same as INTEGER PRIMARY KEY!
      fld.setAutoValue( true );
    fld.setRequired( q.value( 3 ).toInt() != 0 );
    fld.setDefaultValue( q.value( 4 ) );
    ind.append( fld );
  }
  return ind;
}

QSqlIndex QSpatiaLiteDriver::primaryIndex( const QString &tblname ) const
{
  if ( !isOpen() )
    return QSqlIndex();

  QString table = tblname;
  if ( isIdentifierEscaped( table, QSqlDriver::TableName ) )
    table = stripDelimiters( table, QSqlDriver::TableName );

  QSqlQuery q( createResult() );
  q.setForwardOnly( true );
  return qGetTableInfo( q, table, true );
}

QSqlRecord QSpatiaLiteDriver::record( const QString &tbl ) const
{
  if ( !isOpen() )
    return QSqlRecord();

  QString table = tbl;
  if ( isIdentifierEscaped( table, QSqlDriver::TableName ) )
    table = stripDelimiters( table, QSqlDriver::TableName );

  QSqlQuery q( createResult() );
  q.setForwardOnly( true );
  return qGetTableInfo( q, table );
}

QString QSpatiaLiteDriver::escapeIdentifier( const QString &identifier, IdentifierType type ) const
{
  Q_UNUSED( type );
  return _q_escapeIdentifier( identifier );
}

static void handle_sqlite_callback( void *qobj, int aoperation, char const *adbname, char const *atablename,
                                    sqlite3_int64 arowid )
{
  Q_UNUSED( aoperation );
  Q_UNUSED( adbname );
  QSpatiaLiteDriver *driver = static_cast<QSpatiaLiteDriver *>( qobj );
  if ( driver )
  {
    QMetaObject::invokeMethod( driver, "handleNotification", Qt::QueuedConnection,
                               Q_ARG( QString, QString::fromUtf8( atablename ) ), Q_ARG( qint64, arowid ) );
  }
}

bool QSpatiaLiteDriver::subscribeToNotification( const QString &name )
{
  Q_D( QSpatiaLiteDriver );
  if ( !isOpen() )
  {
    qWarning( "Database not open." );
    return false;
  }

  if ( d->notificationid.contains( name ) )
  {
    qWarning( "Already subscribing to '%s'.", qPrintable( name ) );
    return false;
  }

  //sqlite supports only one notification callback, so only the first is registered
  d->notificationid << name;
  if ( d->notificationid.count() == 1 )
    sqlite3_update_hook( d->access.get(), &handle_sqlite_callback, reinterpret_cast<void *>( this ) );

  return true;
}

bool QSpatiaLiteDriver::unsubscribeFromNotification( const QString &name )
{
  Q_D( QSpatiaLiteDriver );
  if ( !isOpen() )
  {
    qWarning( "Database not open." );
    return false;
  }

  if ( !d->notificationid.contains( name ) )
  {
    qWarning( "Not subscribed to '%s'.", qPrintable( name ) );
    return false;
  }

  d->notificationid.removeAll( name );
  if ( d->notificationid.isEmpty() )
    sqlite3_update_hook( d->access.get(), NULL, NULL );

  return true;
}

QStringList QSpatiaLiteDriver::subscribedToNotifications() const
{
  Q_D( const QSpatiaLiteDriver );
  return d->notificationid;
}

void QSpatiaLiteDriver::handleNotification( const QString &tableName, qint64 rowid )
{
  Q_D( const QSpatiaLiteDriver );
  if ( d->notificationid.contains( tableName ) )
  {
    emit notification( tableName );
    emit notification( tableName, QSqlDriver::UnknownSource, QVariant( rowid ) );
  }
}

QT_END_NAMESPACE
