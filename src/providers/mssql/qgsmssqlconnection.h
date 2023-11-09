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

#include "qgsdatasourceuri.h"
#include "qgsvectordataprovider.h"

class QString;
class QSqlDatabase;

class QgsMssqlDatabase;

/**
 * \class QgsMssqlConnection
 * Connection handler for SQL Server provider
 *
*/
class QgsMssqlConnection
{

  public:

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
     * Returns whether the connection with matching \a name should,
     * use the extent manually specified in the geometry_columns table using additional
     * QGIS-specific columns: qgis_xmin, qgis_xmax, qgis_ymin, qgis_ymax.
     *
     * This is an optional optimization that allows QGIS to skip extent calculation when loading
     * layers and thus lowering the amount of time needed to load them. The disadvantage
     * is that the extent needs to be manually set and updated by database admins and it requires
     * adding custom columns to the geometry_columns table.
     *
     * \see setExtentInGeometryColumns()
     */
    static bool extentInGeometryColumns( const QString &name );

    /**
     * Sets whether the connection with matching \a name should
     *
     * \see extentInGeometryColumns()
     */
    static void setExtentInGeometryColumns( const QString &name, bool enabled );

    /**
     * Returns whether the connection with matching \a name should
     * determine primary key's column name from a manually specified value in the geometry_columns table using
     * an additional QGIS-specific column called "qgis_pkey". If more than one column is used for the primary key,
     * value of "qgis_pkey" can contain multiple column names separated by comma.
     *
     * Note: this option only applies to views: for tables the primary key is automatically fetched from table definition.
     *
     * This is an optional optimization that allows QGIS to skip primary key calculation for views when loading
     * layers and thus lowering the amount of time needed to load them. The disadvantage
     * is that the primary key column name needs to be manually set and updated by database admins
     * and it requires adding a custom column to the geometry_columns table.
     *
     * \see setPrimaryKeyInGeometryColumn()
     */
    static bool primaryKeyInGeometryColumns( const QString &name );

    /**
     * Sets whether the connection with matching \a name should
     *
     * \see primaryKeyInGeometryColumns()
     */
    static void setPrimaryKeyInGeometryColumns( const QString &name, bool enabled );

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
     * Returns a list of all schemas on the \a dataBase.
     * \since QGIS 3.18
     */
    static QStringList schemas( std::shared_ptr<QgsMssqlDatabase> db, QString *errorMessage );

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

    /**
     * Returns a list of excluded schemas for connection \a connName depending on settings, returns empty list if nothing is set for this connection
     * \since QGIS 3.18
     */
    static QStringList excludedSchemasList( const QString &connName );

    /**
     * Returns a list of excluded schemas for connection \a connName for a specific \a database depending on settings, returns empty list if nothing is set for this connection
     * \since QGIS 3.18
     */
    static QStringList excludedSchemasList( const QString &connName, const QString &database );

    /**
     * Sets a list of excluded schemas for connection \a connName depending on settings, returns empty list if nothing is set for this connection
     * \since QGIS 3.18
     */
    static void setExcludedSchemasList( const QString &connName, const QStringList &excludedSchemas );

    /**
     * Sets a list of excluded schemas for connection \a connName for a specific \a database depending on settings, returns empty list if nothing is set for this connection
     * \since QGIS 3.18
     */
    static void setExcludedSchemasList( const QString &connName, const QString &database, const QStringList &excludedSchemas );

    /**
     * Builds and returns a sql query string to obtain tables list depending on \a allowTablesWithNoGeometry, \a geometryColumnOnly and on \a notSelectedSchemasList
     * \since QGIS 3.18
     */
    static QString buildQueryForTables( bool allowTablesWithNoGeometry, bool geometryColumnOnly, const QStringList &excludedSchemaList = QStringList() );


    /**
     * Builds and returns a sql query string to obtain tables list depending on settings and \a allowTablesWithNoGeometry
     * \since QGIS 3.18
     */
    static QString buildQueryForTables( const QString &connName, bool allowTablesWithNoGeometry );

    /**
     * Builds and returns a sql query string to obtain schemas list depending only on settings
     * \since QGIS 3.18
     */
    static QString buildQueryForTables( const QString &connName );

  private:


};

#endif // QGSMSSQLCONNECTION_H
