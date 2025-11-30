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

class QString;
class QVariant;
class QByteArray;
struct sqlite3;

extern "C"
{
#include <sqlite3.h>

  int qgsvlayerModuleInit( sqlite3 *db, char **pzErrMsg, void *unused /*const sqlite3_api_routines *pApi*/ );
}

// RAII class for sqlite3*
// Similar to std::unique_ptr
class QgsScopedSqlite
{
  public:
    QgsScopedSqlite() = default;

    explicit QgsScopedSqlite( const QString &path, bool withExtension = true );

    QgsScopedSqlite( QgsScopedSqlite &other );
    QgsScopedSqlite &operator=( QgsScopedSqlite &other );
    ~QgsScopedSqlite();

    bool interrupt();
    [[nodiscard]] sqlite3 *get() const;
    sqlite3 *release();
    void reset( sqlite3 *db );

  private:
    sqlite3 *db_ = nullptr;

    void close_();
};

namespace Sqlite
{
  struct Query
  {
      Query( sqlite3 *db, const QString &q );
      ~Query();

      int step();

      Query &bind( const QVariant &value, int idx );
      Query &bind( const QVariant &value );

      static void exec( sqlite3 *db, const QString &sql );

      void reset();

      [[nodiscard]] int columnCount() const;

      [[nodiscard]] QString columnName( int i ) const;

      [[nodiscard]] int columnType( int i ) const;

      [[nodiscard]] int columnInt( int i ) const;

      [[nodiscard]] long long columnInt64( int i ) const;

      [[nodiscard]] double columnDouble( int i ) const;

      [[nodiscard]] QString columnText( int i ) const;

      [[nodiscard]] QByteArray columnBlob( int i ) const;

      sqlite3_stmt *stmt();

    private:
      sqlite3 *db_ = nullptr;
      sqlite3_stmt *stmt_ = nullptr;
      int nBind_ = 1;
  };
} // namespace Sqlite

#endif
