/*
 MDAL - Mesh Data Abstraction Library (MIT License)
 Copyright (C) 2020 Lutra Consulting Limited
*/

#include "mdal_sqlite3.hpp"
#include <sqlite3.h>


Sqlite3Db::Sqlite3Db() = default;

Sqlite3Db::~Sqlite3Db()
{
  close();
}

bool Sqlite3Db::open( const std::string &fileName )
{
  close();
  int rc = sqlite3_open( fileName.c_str(), &mDb );
  if ( rc )
    return false;
  else
    return true;
}

void Sqlite3Db::close()
{
  if ( mDb )
  {
    sqlite3_close( mDb );
    mDb = nullptr;
  }
}

sqlite3 *Sqlite3Db::get()
{
  return mDb;
}

Sqlite3Statement::Sqlite3Statement() = default;

Sqlite3Statement::~Sqlite3Statement()
{
  close();
}

bool Sqlite3Statement::prepare( Sqlite3Db *db, const std::string &statementString )
{
  return sqlite3_prepare_v2( db->get(), statementString.c_str(), -1, &mStatement, nullptr ) == SQLITE_OK;

}

void Sqlite3Statement::close()
{
  if ( mStatement )
  {
    sqlite3_finalize( mStatement );
    mStatement = nullptr;
  }
}

bool Sqlite3Statement::next()
{
  return SQLITE_ROW == sqlite3_step( mStatement );
}

int Sqlite3Statement::getInt( int column ) const
{
  if ( mStatement )
    return sqlite3_column_int( mStatement, column );
  else
    return std::numeric_limits<int>::quiet_NaN();
}

int Sqlite3Statement::columnCount() const
{
  if ( mStatement )
    return sqlite3_column_count( mStatement );
  else
    return -1;
}
