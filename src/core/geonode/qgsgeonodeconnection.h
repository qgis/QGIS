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


/*!
 * \brief   GeoNode Connections management
 */
class CORE_EXPORT QgsGeoNodeConnection : public QObject
{
    Q_OBJECT

  public:
    //! Constructor
    explicit QgsGeoNodeConnection( const QString &connName );

    //! Destructor
    ~QgsGeoNodeConnection();

    QString connName() const;
    void setConnName( const QString &connName );

    QgsDataSourceUri uri();
    void setUri( const QgsDataSourceUri &uri );

    //! Retrieve all geonode connection
    static QStringList connectionList();

    //! Delete connection with name, name
    static void deleteConnection( const QString &name );

    //! Get selected connection
    static QString selectedConnection();

    //! Set selected connection
    static void setSelectedConnection( const QString &name );

    static QString pathGeoNodeConnection();

    static QString pathGeoNodeConnectionDetails();

  private:
    // Path in QSetting
    static const QString sPathGeoNodeConnection;
    static const QString sPathGeoNodeConnectionDetails;

    //! The connection name
    QString mConnName;

    //! Property of mUri
    QgsDataSourceUri mUri;
};


#endif //QGSGEONODECONNECTION_H
