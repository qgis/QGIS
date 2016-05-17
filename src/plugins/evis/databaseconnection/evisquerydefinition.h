/*
** File: evisquerydefinition.h
** Author: Peter J. Ersts ( ersts at amnh.org )
** Creation Date: 2007-04-12
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
#ifndef EVISQUERYDEFINITION_H
#define EVISQUERYDEFINITION_H

#include "evisdatabaseconnection.h"

#include <QString>

/**
* \class eVisQueryDefinition
* \brief Object for holding an parameters to an eVis sql query
* This class is not much more than a structure for holding the parameters for a eVis sql query. THis class
* is used by eVisDatabaseConnectionGui to store not only the sql statement but also all the parameters for
* making a connection to a database or datasource, as loaded from an XML file.
*/
class eVisQueryDefinition
{

  public:
    /** \brief Constructor */
    eVisQueryDefinition();

    /** \brief Accessor for query description */
    QString description() { return mDescription; }

    /** \brief Accessor for query short description */
    QString shortDescription() { return mShortDescription; }

    /** \brief Accessor for database type */
    QString databaseType() { return mDatabaseType; }

    /** \brief Accessor for database host name */
    QString databaseHost() { return mDatabaseHost; }

    /** \brief Accessor for database port */
    int databasePort() { return mDatabasePort; }

    /** \brief Accessor for database name */
    QString databaseName() { return mDatabaseName; }

    /** \brief Accessor for database username */
    QString databaseUsername() { return mDatabaseUsername; }

    /** \brief Accessor for database password */
    QString databasePassword() { return mDatabasePassword; }

    /** \brief Accessor for SQL statement */
    QString sqlStatement() { return mSqlStatement; }

    /** \brief Accessor for auto connection flag */
    bool autoConnect() { return mAutoConnect; }

    /** \brief Mutator for query description */
    void setDescription( const QString& description ) { mDescription = description; }

    /** \brief Mutator for query short description */
    void setShortDescription( const QString& description ) { mShortDescription = description; }

    /** \brief Mutator for database type */
    void setDatabaseType( const QString& type ) { mDatabaseType = type; }

    /** \brief Mutator for database host name */
    void setDatabaseHost( const QString& host ) { mDatabaseHost = host; }

    /** \brief Mutator for database port */
    void setDatabasePort( int port ) { mDatabasePort = port; }

    /** \brief Mutator for database name */
    void setDatabaseName( const QString& name ) { mDatabaseName = name; }

    /** \brief Mutator for database username */
    void setDatabaseUsername( const QString& username ) { mDatabaseUsername = username; }

    /** \brief Mutator for database password */
    void setDatabasePassword( const QString& password ) { mDatabasePassword = password; }

    /** \brief Mutator for SQL statement */
    void setSqlStatement( const QString& statement ) { mSqlStatement = statement; }

    /** \brief Mutator for auto connection flag */
    void setAutoConnect( const QString& autoconnect )
    {
      if ( autoconnect.startsWith( "true", Qt::CaseInsensitive ) )
      {
        mAutoConnect = true;
      }
      else
      {
        mAutoConnect = false;
      }
    }


  private:
    /** \brief Detailed description for the query */
    QString mDescription;

    /** \brief Short description for the query */
    QString mShortDescription;

    /** \brief The database type to which a connection should be made */
    QString mDatabaseType;

    /** \brief The database host to which a connection should be made */
    QString mDatabaseHost;

    /** \brief The port/socket on the database host to which a connection should be made */
    int mDatabasePort;

    /** \brief The name, or filename, of the database to which a connection should be made */
    QString mDatabaseName;

    /** \brief Username for the database, if required */
    QString mDatabaseUsername;

    /** \brief Password for the username, if require */
    QString mDatabasePassword;

    /** \brief The SQL statement to execute upon successful connection to a database */
    QString mSqlStatement;

    /** \brief Boolean to allow for automated connection to the database when query definition is successfully loaded */
    bool mAutoConnect;
};
#endif
