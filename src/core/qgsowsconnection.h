/***************************************************************************
    qgsowsconnection.h  -  OWS connection
                             -------------------
    begin                : 3 April 2005
    original             : (C) 2005 by Brendan Morley email  : morb at ozemail dot com dot au
    wms search           : (C) 2009 Mathias Walker <mwa at sourcepole.ch>, Sourcepole AG

    generalized          : (C) 2012 Radim Blazek, based on qgswmsconnection.h


 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef QGSOWSCONNECTION_H
#define QGSOWSCONNECTION_H

#include "qgis_core.h"
#include "qgsdatasourceuri.h"

#include <QStringList>
#include <QPushButton>

/**
 * \ingroup core
 * \brief Connections management
 */
class CORE_EXPORT QgsOwsConnection : public QObject
{
    Q_OBJECT

  public:

    /**
     * Constructor
     * \param service service name: WMS,WFS,WCS
     * \param connName connection name
     */
    QgsOwsConnection( const QString &service, const QString &connName );

    /**
     * Returns the connection name.
     * \since QGIS 3.0
     */
    QString connectionName() const;

    /**
     * Returns connection info string.
     * \since QGIS 3.0
     */
    QString connectionInfo() const;

    /**
     * Returns a string representing the service type, e.g. "WMS".
     * \since QGIS 3.0
     */
    QString service() const;

    /**
     * Returns the connection uri.
     */
    QgsDataSourceUri uri() const;

    /**
     * Adds uri parameters relating to the settings for a WMS or WCS connection to a QgsDataSourceUri \a uri.
     * Connection settings are taken from the specified QSettings \a settingsKey.
     * \since QGIS 3.0
     */
    static QgsDataSourceUri &addWmsWcsConnectionSettings( QgsDataSourceUri &uri, const QString &settingsKey );

    /**
     * Adds uri parameters relating to the settings for a WFS connection to a QgsDataSourceUri \a uri.
     * Connection settings are taken from the specified QSettings \a settingsKey.
     * \since QGIS 3.0
     */
    static QgsDataSourceUri &addWfsConnectionSettings( QgsDataSourceUri &uri, const QString &settingsKey );

    //! Returns the list of connections for the specified service
    static QStringList connectionList( const QString &service );

    //! Deletes the connection for the specified service with the specified name
    static void deleteConnection( const QString &service, const QString &name );

    //! Retrieves the selected connection for the specified service
    static QString selectedConnection( const QString &service );
    //! Marks the specified connection for the specified service as selected
    static void setSelectedConnection( const QString &service, const QString &name );

  protected:
    QgsDataSourceUri mUri;

  private:

    QString mConnName;
    QString mService;
    QString mConnectionInfo;

    static void addCommonConnectionSettings( QgsDataSourceUri &uri, const QString &settingsKey );

};


#endif // QGSOWSCONNECTION_H
