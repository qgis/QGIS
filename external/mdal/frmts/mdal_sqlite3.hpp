/*
 MDAL - Mesh Data Abstraction Library (MIT License)
 Copyright (C) 2020 Lutra Consulting Limited
*/

#ifndef MDAL_SQLITE3_HPP
#define MDAL_SQLITE3_HPP

#include <string>
/** A simple C++ wrapper around SQLITE3 library API */

#include <sqlite3.h>

#include "mdal_logger.hpp"

class Sqlite3Db
{
  public:
    Sqlite3Db();
    ~Sqlite3Db();

    bool open( const std::string &fileName );
    void close();

    sqlite3 *get();

  private:
    sqlite3 *mDb = nullptr;
};

class Sqlite3Statement
{
  public:
    Sqlite3Statement();
    ~Sqlite3Statement();

    bool prepare( Sqlite3Db *db, const std::string &statementString );
    void close();

    int columnCount() const;

    bool next();

    int getInt( int column ) const;

  private:
    sqlite3_stmt *mStatement = nullptr;
};

#endif // MDAL_SQLITE3_HPP
