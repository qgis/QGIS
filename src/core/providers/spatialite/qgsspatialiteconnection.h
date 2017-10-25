/***************************************************************************
    qgsspatialiteconnection.h
    ---------------------
    begin                : October 2011
    copyright            : (C) 2011 by Martin Dobias
    email                : wonder dot sk at gmail dot com
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/
#ifndef QGSSPATIALITECONNECTION_H
#define QGSSPATIALITECONNECTION_H

#include <QStringList>
#include <QObject>
#include <QHash>

#include "qgssqlitehandle.h"


class CORE_EXPORT QgsSpatiaLiteConnection : public QObject
{
    Q_OBJECT
  public:

    /**
     * Construct a connection. Name can be either stored connection name or a path to the database file
     * - not used outside the QgsSpatiaLiteProvider classes
     * \since QGIS 1.8
     */
    explicit QgsSpatiaLiteConnection( const QString &name );

    /**
     * Absolute Path of the Connection
     * - 'SpatiaLite/connections'
     * \note
     *  - extrcted from ConnectionString
     * \returns name of connection
     * \see connectionPath
     * \since QGIS 1.8
     */
    QString dbPath() { return mDbPath; }

    /**
     * Key of the Connection
     * - 'SpatiaLite/connections'
     * \note
     *  - extrcted from ConnectionString
     * \returns name of connection
     * \see connectionPath
     * \since QGIS 1.8
     */
    QString getSubKey() { return mSubKey; }

    /**
     * Return List of all stored Connection-Namess in QgsSettings
     * - 'SpatiaLite/connections'
     * \note
     *  - use connectionPath to retrieve Absolute Path of the Connection
     * \returns name of connection
     * \see connectionPath
     * \since QGIS 1.8
     */
    static QStringList connectionList();

    /**
     * Delete the given Connection in QgsSettings
     * \param name of connection as retuned from connectionList
     * \see connectionList
     * \since QGIS 1.8
     */
    static void deleteConnection( const QString &name );

    /**
     * Remove the Connection Strings from the QgsSettings
     * - when the file no longer exists
     * \note
     *  - uses connectionPath to retrieve Absolute Path of the Connection
     *  - uses deleteConnection to delete the connection
     * \returns amount of removed files
     * \see connectionPath
     * \see deleteConnection
     * \since QGIS 3.0
     */
    static int deleteInvalidConnections( );

    /**
     * Return the absolute Path of the given Connection
     * - when the file no longer exists
     * \param name of connection as retuned from connectionList
     * \returns path of Database file
     * \see connectionList
     * \since QGIS 1.8
     */
    static QString connectionPath( const QString &name );

    /**
     * Create a SpatialiteDbInfo based Connection
     *  -> containing all needed Information about a Spatial Sqlite3 Container
     * \note
     *  - check result with spatialiteDbInfo->isDbSqlite3()
     *  -> if File exists and is a Sqlite3 Container.
     *  - check result with spatialiteDbInfo->isDbGdalOgr()
     *  -> if File only supported by QgsOgrProvider or QgsGdalProvider
     *  -> otherwise supported by QgsSpatiaLiteProvider
    * \returns true if file is a sqlite3 Database
    * \since QGIS 3.0
    */
    SpatialiteDbInfo *CreateSpatialiteConnection( QString sLayerName = QString::null, bool bLoadLayers = true, bool bShared = true, SpatialiteDbInfo::SpatialMetadata dbCreateOption = SpatialiteDbInfo::SpatialUnknown );

  protected:

    /**
     * Key of the Connection
     * - 'SpatiaLite/connections'
     * \note
     *  - extrcted from ConnectionString
     * \returns name of connection
     * \see connectionPath
     * \since QGIS 1.8
     */
    QString mSubKey;

    /**
     * Absolute Path of the Connection
     * - 'SpatiaLite/connections'
     * \note
     *  - extrcted from ConnectionString
     * \returns name of connection
     * \see connectionPath
     * \since QGIS 1.8
     */
    QString mDbPath;
};
#endif // QGSSPATIALITECONNECTION_H
