/*
** File: evisdatabaseconnection.h
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
** This work was made possible through a grant by the John D. and
** Catherine T. MacArthur Foundation. Additionally, this program was prepared by
** the American Museum of Natural History under award No. NA05SEC46391002
** from the National Oceanic and Atmospheric Administration, U.S. Department
** of Commerce.  The statements, findings, conclusions, and recommendations
** are those of the author( s ) and do not necessarily reflect the views of the
** National Oceanic and Atmospheric Administration or the Department of Commerce.
**
**/
#ifndef EVISDATABASECONNECTION_H
#define EVISDATABASECONNECTION_H

#include <QStringList>
#include <QtSql/QSqlDatabase>
#include <QtSql/QSqlQuery>
#include <QtSql/QSqlError>

/**
* \class eVisDatabaseConnection
* \brief A class that provides the ability to connect to a variety of database types
* This class provides the ability to connect to a variety of database types
*/
class eVisDatabaseConnection
{

  public:

    /** \brief Enum containting the type of database supported by this class */
    enum DATABASE_TYPE
    {
      UNDEFINED,
      MSACCESS,
      QMYSQL,
      QPSQL,
      QODBC,
      QSQLITE
    } mDatabaseType;

    /** \brief Constructor */
    eVisDatabaseConnection( const QString&, int, const QString&, const QString&, const QString&, DATABASE_TYPE );

    /** \brief Public method that finalizes a connection to a databse */
    bool connect();

    /** \brief Public method that passes an SQL statement to the database for execution */
    QSqlQuery* query( const QString& );

    /** \brief Public method for resetting the database connection parameters - equivalent to re running the constructor */
    void resetConnectionParameters( const QString&, int, const QString&, const QString&, const QString&, DATABASE_TYPE );

    /** \brief Returns a list of tables in the current database */
    QStringList tables();

    /** \brief Accessor to the database type */
    DATABASE_TYPE databaseType()
    {
      return mDatabaseType;
    }

    /** \brief Public method for closing the current database connection */
    void close()
    {
      mDatabase.close();
    }

    /** \brief Public method for requesting the last error reported by the database connect or query */
    QString lastError()
    {
      return mLastError;
    }

    /** \brief Mutator for database type */
    void setDatabaseType( DATABASE_TYPE connectionType )
    {
      mDatabaseType = connectionType;
    }

  protected:
    /** \brief Variable used to store the query results */
    QSqlQuery mQuery;

  private:
    /** \brief Host name for the database server */
    QString mHostName;

    /** \brief Port number the database server is listenting to */
    int mPort;

    /** \brief Database name, can also be a filename in the case of SQLite or MSAccess */
    QString mDatabaseName;

    /** \brief Username for accessing the database */
    QString mUsername;

    /** \brief Password associated with the username for accessing the database */
    QString mPassword;

    /** \brief QString containing the last reported error message */
    QString mLastError;

    /** \brief The database object */
    QSqlDatabase mDatabase;

    /** \brief Sets the error messages */
    void setLastError( const QString& error )
    {
      mLastError = error;
    }
};
#endif
