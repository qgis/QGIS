/***************************************************************************
                    qgsvirtuallayersqlitehelper.cpp
begin                : December 2015
copyright            : (C) 2015 Hugo Mercier, Oslandia
email                : hugo dot mercier at oslandia dot com
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include <QString>

#include <stdexcept>

#include "qgsvirtuallayersqlitehelper.h"
#include "qgslogger.h"

QgsScopedSqlite::QgsScopedSqlite( const QString& path, bool withExtension )
{
  if ( withExtension )
  {
    // register a statically-linked function as extension
    // for all future database connection
    sqlite3_auto_extension( reinterpret_cast < void( * )() > ( qgsvlayerModuleInit ) );
  }
  int r;
  r = sqlite3_open( path.toLocal8Bit().constData(), &db_ );
  if ( withExtension )
  {
    // reset the automatic extensions
    sqlite3_reset_auto_extension();
  }

  if ( r )
  {
    QString err = QString( "%1 [%2]" ).arg( sqlite3_errmsg( db_ ), path );
    QgsDebugMsg( err );
    throw std::runtime_error( err.toLocal8Bit().constData() );
  }
  // enable extended result codes
  sqlite3_extended_result_codes( db_, 1 );
}

QgsScopedSqlite::QgsScopedSqlite( QgsScopedSqlite& other )
{
  db_ = other.db_;
  other.db_ = nullptr;
}

QgsScopedSqlite& QgsScopedSqlite::operator=( QgsScopedSqlite & other )
{
  reset( other.release() );
  return *this;
}

QgsScopedSqlite::~QgsScopedSqlite()
{
  close_();
}

sqlite3* QgsScopedSqlite::get() const { return db_; }

sqlite3* QgsScopedSqlite::release()
{
  sqlite3* pp = db_;
  db_ = nullptr;
  return pp;
}

void QgsScopedSqlite::reset( sqlite3* db )
{
  close_();
  db_ = db;
}

void QgsScopedSqlite::close_()
{
  if ( db_ )
    sqlite3_close( db_ );
}

namespace Sqlite
{
  Query::Query( sqlite3* db, const QString& q )
      : db_( db )
      , nBind_( 1 )
  {
    QByteArray ba( q.toLocal8Bit() );
    int r = sqlite3_prepare_v2( db, ba.constData(), ba.size(), &stmt_, nullptr );
    if ( r )
    {
      QString err = QString( "Query preparation error on %1: %2" ).arg( q ).arg( sqlite3_errmsg( db ) );
      throw std::runtime_error( err.toLocal8Bit().constData() );
    }
  }

  Query::~Query()
  {
    sqlite3_finalize( stmt_ );
  }

  int Query::step() { return sqlite3_step( stmt_ ); }

  Query& Query::bind( const QString& str, int idx )
  {
    QByteArray ba( str.toLocal8Bit() );
    int r = sqlite3_bind_text( stmt_, idx, ba.constData(), ba.size(), SQLITE_TRANSIENT );
    if ( r )
    {
      throw std::runtime_error( sqlite3_errmsg( db_ ) );
    }
    return *this;
  }

  Query& Query::bind( const QString& str )
  {
    return bind( str, nBind_++ );
  }

  void Query::exec( sqlite3* db, const QString& sql )
  {
    char *errMsg = nullptr;
    int r = sqlite3_exec( db, sql.toLocal8Bit().constData(), nullptr, nullptr, &errMsg );
    if ( r )
    {
      QString err = QString( "Query execution error on %1: %2 - %3" ).arg( sql ).arg( r ).arg( errMsg );
      throw std::runtime_error( err.toLocal8Bit().constData() );
    }
  }

  void Query::reset()
  {
    int r = sqlite3_reset( stmt_ );
    if ( r )
    {
      throw std::runtime_error( sqlite3_errmsg( db_ ) );
    }
    nBind_ = 1;
  }

  int Query::columnCount() const
  {
    return sqlite3_column_count( stmt_ );
  }

  QString Query::columnName( int i ) const
  {
    return QString( sqlite3_column_name( stmt_, i ) );
  }

  int Query::columnType( int i ) const
  {
    return sqlite3_column_type( stmt_, i );
  }

  int Query::columnInt( int i ) const
  {
    return sqlite3_column_int( stmt_, i );
  }

  qint64 Query::columnInt64( int i ) const
  {
    return sqlite3_column_int64( stmt_, i );
  }

  double Query::columnDouble( int i ) const
  {
    return sqlite3_column_double( stmt_, i );
  }

  QString Query::columnText( int i ) const
  {
    int size = sqlite3_column_bytes( stmt_, i );
    const char* str = reinterpret_cast< const char* >( sqlite3_column_text( stmt_, i ) );
    return QString::fromUtf8( str, size );
  }

  QByteArray Query::columnBlob( int i ) const
  {
    int size = sqlite3_column_bytes( stmt_, i );
    const char* data = reinterpret_cast< const char* >( sqlite3_column_blob( stmt_, i ) );
    // data is not copied. QByteArray is just here a augmented pointer
    return QByteArray::fromRawData( data, size );
  }

  sqlite3_stmt* Query::stmt() { return stmt_; }

}
