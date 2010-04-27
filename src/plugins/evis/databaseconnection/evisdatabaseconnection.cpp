/*
** File: evisdatabaseconnection.cpp
** Author: Peter J. Ersts ( ersts at amnh.org )
** Creation Date: 2007-03-07
**
** Copyright ( c ) 2007, American Museum of Natural History. All rights reserved.
**
** This library/program is free software; you can redistribute it
** and/or modify it under the terms of the GNU Library General Public
** License as published by the Free Software Foundation; either
** version 2 of the License, or ( at your option ) any later version.
**
** This library/program is distributed in the hope that it will be useful,
** but WITHOUT ANY WARRANTY; without even the implied warranty of
** MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
** Library General Public License for more details.
**
** This work was made possible through a grant by the the John D. and
** Catherine T. MacArthur Foundation. Additionally, this program was prepared by
** the American Museum of Natural History under award No. NA05SEC46391002
** from the National Oceanic and Atmospheric Administration, U.S. Department
** of Commerce.  The statements, findings, conclusions, and recommendations
** are those of the author( s ) and do not necessarily reflect the views of the
** National Oceanic and Atmospheric Administration or the Department of Commerce.
**
**/
/*  $Id$ */
#include "evisdatabaseconnection.h"

#include <QStringList>

/**
* Constructor
* @param hostname - Host name of the database server
* @param port - The port number the database server is listening to
* @param databasename - The name of the database to connect to
* @param username - The username needed to access the database or database server
* @param password - The password associate witht he username needed to access the database or database server
* @param type - The type of database being connected to
*/
eVisDatabaseConnection::eVisDatabaseConnection( QString hostname, int port, QString databasename, QString username, QString password, DATABASE_TYPE type )
{
  mHostName = hostname;
  mPort = port;
  mDatabaseName = databasename;
  mUsername = username;
  mPassword = password;
  setDatabaseType( type );
  mQuery.setForwardOnly( true );
}

/**
* Public method called to finalize a connection to a database
*/
bool eVisDatabaseConnection::connect( )
{
  //If a database is currnently open close the connection
  if ( !mDatabase.isOpen( ) )
  {
    mDatabase.close( );
  }

  //Add the correct database to the list of database connections, Reuse a connection if the connection exists in the list already.
  if ( MSACCESS == databaseType( ) && !mDatabase.contains( "odbc" ) )
  {
    mDatabase = QSqlDatabase::addDatabase( "QODBC", "odbc" );
  }
  else if ( MSACCESS == databaseType( ) )
  {
    mDatabase = QSqlDatabase::database( "odbc" );
  }
  else if ( QMYSQL == databaseType( ) && !mDatabase.contains( "mysql" ) )
  {
    mDatabase = QSqlDatabase::addDatabase( "QMYSQL", "mysql" );
  }
  else if ( QMYSQL == databaseType( ) )
  {
    mDatabase = QSqlDatabase::database( "mysql" );
  }
  else if ( QODBC == databaseType( ) && !mDatabase.contains( "odbc" ) )
  {
    mDatabase = QSqlDatabase::addDatabase( "QODBC", "odbc" );
  }
  else if ( QODBC == databaseType( ) )
  {
    mDatabase = QSqlDatabase::database( "odbc" );
  }
  else if ( QPSQL == databaseType( ) && !mDatabase.contains( "postgres" ) )
  {
    mDatabase = QSqlDatabase::addDatabase( "QPSQL", "postgres" );
  }
  else if ( QPSQL == databaseType( ) )
  {
    mDatabase = QSqlDatabase::database( "postgres" );
  }
  else if ( QSQLITE == databaseType( ) && !mDatabase.contains( "sqlite" ) )
  {
    mDatabase = QSqlDatabase::addDatabase( "QSQLITE", "sqlite" );
  }
  else if ( QSQLITE == databaseType( ) )
  {
    mDatabase = QSqlDatabase::database( "sqlite" );
  }
  else
  {
    setLastError( "No matching DATABASE_TYPE found" );
    return false;
  }

  //Do a little extra validation of connection information
  if ( mHostName.isEmpty( ) && ( QMYSQL == databaseType( ) || QPSQL == databaseType( ) ) )
  {
    setLastError( "Host name was empty" );
    return false;
  }
  else if ( !mHostName.isEmpty( ) )
  {
    mDatabase.setHostName( mHostName );
  }

  if ( mPort != 0 )
  {
    mDatabase.setPort( mPort );
  }

  if ( mDatabaseName.isEmpty( ) )
  {
    setLastError( "Database name was empty" );
    return false;
  }
  else if ( MSACCESS == databaseType( ) )
  {
    mDatabase.setDatabaseName( "DRIVER={Microsoft Access Driver (*.mdb)};FIL={MS Access};DBQ=" + mDatabaseName );
  }
  else
  {
    mDatabase.setDatabaseName( mDatabaseName );
  }

  if ( !mUsername.isEmpty( ) )
  {
    mDatabase.setUserName( mUsername );
  }

  if ( !mPassword.isEmpty( ) )
  {
    mDatabase.setPassword( mPassword );
  }

  //Try to actually open the database
  if ( !mDatabase.open( ) )
  {
    setLastError( mDatabase.lastError( ).text( ) );
    return false;
  }

  return true;
}

/**
* Executes a query on the current active database connection
* @param sqlStatement - QString containing the sql statement to execute
*/
QSqlQuery* eVisDatabaseConnection::query( QString sqlStatement )
{
  if ( mDatabase.isOpen( ) )
  {
    //mQuery = QSqlQuery( sqlStatement, mDatabase ); //NOTE: A little against convention, the constructor also executes the query

    //set forward only is required for OBDC on linux
    mQuery = QSqlQuery( mDatabase );
    mQuery.setForwardOnly( true );
    mQuery.exec( sqlStatement );
    if ( mQuery.isActive( ) )
    {
      return &mQuery;
    }
    else
    {
      setLastError( mQuery.lastError( ).text( ) );
      return 0;
    }
  }

  setLastError( "Database connection was not open." );
  return 0;
}

/**
* Reset the connection parameters
* @param hostname - Host name of the database server
* @param port - The port number the database server is listening to
* @param databasename - The name of the database to connect to
* @param username - The username needed to access the database or database server
* @param password - The password associate witht he username needed to access the database or database server
* @param type - The type of database being connected to
*/
void eVisDatabaseConnection::resetConnectionParameters( QString hostname, int port, QString databasename, QString username, QString password, DATABASE_TYPE type )
{
  mHostName = hostname;
  mPort = port;
  mDatabaseName = databasename;
  mUsername = username;
  mPassword = password;
  setDatabaseType( type );
}

/**
* Returns a list of tables for the current active database connection
*/
QStringList eVisDatabaseConnection::tables( )
{
  if ( mDatabase.isOpen( ) )
  {
    return mDatabase.tables( );
  }

  setLastError( "Database connection was not open." );
  return QStringList( );
}
