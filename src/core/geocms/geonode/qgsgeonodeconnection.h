/***************************************************************************
    qgsgeonodeconnection.h
    ---------------------
    begin                : Feb 2017
    copyright            : (C) 2017 by Muhammad Yarjuna Rohmat, Ismail Sunni
    email                : rohmat at kartoza dot com, ismail at kartoza dot com
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef QGSGEONODECONNECTION_H
#define QGSGEONODECONNECTION_H

#include "qgis_core.h"
#include "qgsdatasourceuri.h"

/**
 * \ingroup core
 * \class QgsGeoNodeConnection
 * \brief Encapsulates settings related to a single GeoNode connection.
 * \since QGIS 3.0
 */
class CORE_EXPORT QgsGeoNodeConnection
{

  public:

    /**
     * Constructor for a QgsGeoNodeConnection with the specified \a name.
     */
    explicit QgsGeoNodeConnection( const QString &name );

    /**
     * Returns the name of the connection.
     * \see setConnectionName()
     */
    QString connectionName() const;

    /**
     * Sets the \a name of the connection.
     * \see connectionName()
     */
    void setConnectionName( const QString &connectionName );

    /**
     * Returns the URI for the GeoNode connection.
     * \see setUri()
     */
    QgsDataSourceUri uri() const;

    /**
     * Sets the \a uri for the GeoNode connection.
     * \see uri()
     */
    void setUri( const QgsDataSourceUri &uri );

  private:

    //! The connection name
    QString mConnName;

    //! Property of mUri
    QgsDataSourceUri mUri;
};

/**
 * \ingroup core
 * \class QgsGeoNodeConnectionUtils
 * \brief Contains various utilities for managing the known collection of
 * GeoNode servers associated with a QGIS install.
 * \since QGIS 3.0
 */
class CORE_EXPORT QgsGeoNodeConnectionUtils
{
  public:

    /**
     * Returns a list of all known GeoNode connection names.
     */
    static QStringList connectionList();

    /**
     * Deletes the GeoNode connection with matching \a name.
     */
    static void deleteConnection( const QString &name );

    /**
     * Returns the base path for settings related to GeoNode connections.
     */
    static QString pathGeoNodeConnection();

    /**
     * Returns the base path for settings related to GeoNode connection details.
     */
    static QString pathGeoNodeConnectionDetails();

  private:

    // Path in QSetting
    static const QString sPathGeoNodeConnection;
    static const QString sPathGeoNodeConnectionDetails;

};


#endif //QGSGEONODECONNECTION_H
