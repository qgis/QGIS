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

#define SIP_NO_FILE

#include "qgis_core.h"
#include "qgsdatasourceuri.h"
#include "qgsmaplayer.h"
#include "qgsproject.h"
#include "qgsgeocmsconnection.h"

#include <QString>
#include <QMultiMap>
#include <QNetworkReply>


/*!
 * \brief   GeoNode Connections management
 */
class CORE_EXPORT QgsGeoNodeConnection : public QgsGeoCmsConnection
{
    Q_OBJECT

  public:
    //! Constructor
    explicit QgsGeoNodeConnection( const QString &connName );

    //! Destructor
    ~QgsGeoNodeConnection();

    //! Retrieve all geonode connection
    static QStringList connectionList();

    //! Delete connection with name, name
    static void deleteConnection( const QString &name );

    //! Get selected connection
    static QString selectedConnection();

    //! Set selected connection
    static void setSelectedConnection( const QString &name );

    //! Return list of available layers
    virtual QList<LayerStruct> getLayers();
    virtual QList<LayerStruct> getLayers( QString serviceType );

    //! Return list of available layers
    virtual QVariantList getMaps();

    //! Return WMS / WFS url for the layer / map / resource ID
    virtual QStringList serviceUrl( QString &resourceID, QString serviceType );

    //! Return WMS / WFS url for the geonode
    virtual QStringList serviceUrl( QString serviceType );

    virtual QgsStringMap serviceUrlData( QString serviceType );

    // Path in QSetting
    static const QString pathGeoNodeConnection;// = "qgis/connections-geonode/";
    static const QString pathGeoNodeConnectionDetails;// = "qgis/GeoNode/";
};


#endif //QGSGEONODECONNECTION_H
