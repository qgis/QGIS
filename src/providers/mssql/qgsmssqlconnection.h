/***************************************************************************
                             qgsmssqlconnection.h
                             --------------------
    begin                : October 2018
    copyright            : (C) 2018 by Nyall Dawson
    email                : nyall dot dawson at gmail dot com
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef QGSMSSQLCONNECTION_H
#define QGSMSSQLCONNECTION_H

class QString;
class QSqlDatabase;

/**
 * \class QgsMssqlProvider
 * Connection handler for SQL Server provider
 *
*/
class QgsMssqlConnection
{

  public:

    /**
     * Returns a QSqlDatabase object for queries to SQL Server.
     *
     * The database may not be open -- openDatabase() should be called to
     * ensure that it is ready for use.
     */
    static QSqlDatabase getDatabase( const QString &service, const QString &host, const QString &database, const QString &username, const QString &password );


    static bool openDatabase( QSqlDatabase &db );

  private:

    /**
     * Returns a thread-safe connection name for use with QSqlDatabase
     */
    static QString dbConnectionName( const QString &name );

    static int sConnectionId;
};

#endif // QGSMSSQLCONNECTION_H
