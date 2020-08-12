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

#include <QStringList>
#include <QMutex>

#include "qgsdatasourceuri.h"
#include "qgsvectordataprovider.h"

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

    /**
     * Returns true if the connection with matching \a name should
     * only look in the geometry_columns metadata table when scanning for tables.
     *
     * \see setGeometryColumnsOnly()
     */
    static bool geometryColumnsOnly( const QString &name );

    /**
     * Sets whether the connection with matching \a name should
     * only look in the geometry_columns metadata table when scanning for tables.
     *
     * \see geometryColumnsOnly()
     */
    static void setGeometryColumnsOnly( const QString &name, bool enabled );

    /**
     * Returns true if the connection with matching \a name should
     * show geometryless tables when scanning for tables.
     *
     * \see setAllowGeometrylessTables()
     */
    static bool allowGeometrylessTables( const QString &name );

    /**
     * Sets whether the connection with matching \a name should
     * show geometryless tables when scanning for tables.
     *
     * \see allowGeometrylessTables()
     */
    static void setAllowGeometrylessTables( const QString &name, bool enabled );

    /**
     * Returns true if the connection with matching \a name should
     * use estimated table parameters.
     *
     * \see setUseEstimatedMetadata()
     */
    static bool useEstimatedMetadata( const QString &name );

    /**
     * Sets whether the connection with matching \a name should
     * use estimated table parameters.
     *
     * \see useEstimatedMetadata()
     */
    static void setUseEstimatedMetadata( const QString &name, bool enabled );

    /**
     * Returns true if the connection with matching \a name should
     * skip all handling of records with invalid geometry.
     *
     * This speeds up the provider, however, if any invalid geometries
     * are present in a table then the result is unpredictable and may
     * include missing records. Only check this option if you are certain
     * that all geometries present in the database are valid, and any newly
     * added geometries or tables will also be valid.
     *
     * \see setInvalidGeometryHandlingDisabled()
     */
    static bool isInvalidGeometryHandlingDisabled( const QString &name );

    /**
     * Sets whether the the connection with matching \a name should
     * skip all handling of records with invalid geometry.
     *
     * This speeds up the provider, however, if any invalid geometries
     * are present in a table then the result is unpredictable and may
     * include missing records. Only check this option if you are certain
     * that all geometries present in the database are valid, and any newly
     * added geometries or tables will also be valid.
     *
     * \see isInvalidGeometryHandlingDisabled()
     */
    static void setInvalidGeometryHandlingDisabled( const QString &name, bool disabled );

    /**
     * Drops the table referenced by \a uri.
     */
    static bool dropTable( const QString &uri, QString *errorMessage );

    /**
     * Drops the view referenced by \a uri.
     */
    static bool dropView( const QString &uri, QString *errorMessage );

    /**
     * Truncates the table referenced by \a uri.
     */
    static bool truncateTable( const QString &uri, QString *errorMessage );

    /**
     * Creates a new schema under connection specified by \a uri.
     */
    static bool createSchema( const QString &uri, const QString &schemaName, QString *errorMessage );

    /**
     * Returns a list of all schemas on the connection specified \a uri.
     */
    static QStringList schemas( const QString &uri, QString *errorMessage );

    /**
     * Returns true if the given \a schema is a system schema.
     */
    static bool isSystemSchema( const QString &schema );

    /**
     * Reads a connection named \a connName from the settings and returns the datasource uri
     */
    static QgsDataSourceUri connUri( const QString &connName );

    /**
     * Reads MSSQL connections from the settings and return a list of connection names
     */
    static QStringList connectionList();

    /**
     * Returns the list of native types
     * \since QGIS 3.16
     */
    static QList<QgsVectorDataProvider::NativeType> nativeTypes();


  private:

    /**
     * Returns a thread-safe connection name for use with QSqlDatabase
     */
    static QString dbConnectionName( const QString &name );

    static int sConnectionId;

    static QMutex sMutex;
};

#endif // QGSMSSQLCONNECTION_H
