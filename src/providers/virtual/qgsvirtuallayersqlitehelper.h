/***************************************************************************
                    qgsvirtuallayersqlitehelper.h
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

#ifndef QGSVIRTUALLAYER_SQLITE_UTILS_H
#define QGSVIRTUALLAYER_SQLITE_UTILS_H

extern "C"
{
#include <sqlite3.h>

  int qgsvlayerModuleInit( sqlite3 *db,
                           char **pzErrMsg,
                           void * unused /*const sqlite3_api_routines *pApi*/ );

}

// RAII class for sqlite3*
// Similar to std::unique_ptr
class QgsScopedSqlite
{
  public:
    QgsScopedSqlite() : db_( nullptr ) {}

    explicit QgsScopedSqlite( const QString& path, bool withExtension = true );

    QgsScopedSqlite( QgsScopedSqlite& other );
    QgsScopedSqlite& operator=( QgsScopedSqlite& other );
    ~QgsScopedSqlite();

    sqlite3* get() const;
    sqlite3* release();
    void reset( sqlite3* db );

  private:
    sqlite3* db_;

    void close_();
};

namespace Sqlite
{
  struct Query
  {
    Query( sqlite3* db, const QString& q );
    ~Query();

    int step();

    Query& bind( const QString& str, int idx );
    Query& bind( const QString& str );

    static void exec( sqlite3* db, const QString& sql );

    void reset();

    int columnCount() const;

    QString columnName( int i ) const;

    int columnType( int i ) const;

    int columnInt( int i ) const;

    qint64 columnInt64( int i ) const;

    double columnDouble( int i ) const;

    QString columnText( int i ) const;

    QByteArray columnBlob( int i ) const;

    sqlite3_stmt* stmt();

  private:
    sqlite3* db_;
    sqlite3_stmt* stmt_;
    int nBind_;
  };
}

#endif
