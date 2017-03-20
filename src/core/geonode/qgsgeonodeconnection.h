/***************************************************************************
                        qgsabstractgeometry.h
  -------------------------------------------------------------------
Date                 : 15 March 2017
Copyright            : (C) 2017 by Rohmat, Ismail Sunni
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

#include "qgsgeonodedataitems.h"
#include "../../providers/wfs/qgswfsdataitems.h"
#include "../../providers/wms/qgswmsdataitems.h"
#include "../../providers/wcs/qgswcsdataitems.h"
#include "qgsauthconfig.h"
#include "qgsdatasourceuri.h"
#include "../../providers/wfs/qgswfsconnection.h"
#include "../../providers/wms/qgswmsconnection.h"

class QgsAuthMethodConfig;

class QgsGeoNodeConnection
{
    //Constructor
    QgsGeoNodeConnection( const QString &baseKey = "/Qgis/connections-geonode/", const QString &connName = QString::null );

    void saveConnection();
    void editConnection();
    void deleteConnection();
    void refreshConnection();
    void connectToServer();
    void disconnectToServer();

    QgsWfsConnection geonodeWfsConnection;
    QgsWMSConnection geonodeWmsConnection;

    QgsWfsLayerItem wfsLayerItem;
    QgsWMSLayerItem wmsLayerItem;
    QgsWCSLayerItem wcsLayerItem;

    QString connectionName;
    QgsDataSourceUri connectionUri;
    QgsAuthMethodConfig connectionAuth;
};

class QgsGeoNodeService
{
    //Constructor
    QgsGeoNodeService();


};

class QgsGeoNodeWfsService
{
    //Constructor
    QgsGeoNodeWfsService();

    QgsGeoNodeWfsLayerItem wfsLayerItem;

    QgsDataSourceUri geoNodeWfsUrl;
};

class QgsGeoNodeWmsService
{
    //Constructor
    QgsGeoNodeWmsService();

    QgsGeoNodeWmsLayerItem wmsLayerItem;

    QgsDataSourceUri geoNodeWmsUrl;
};

class QgsGeoNodeWcsService
{
    //Constructor
    QgsGeoNodeWcsService();

    QgsGeoNodeWcsLayerItem wcsLayerItem;

    QgsDataSourceUri geoNodeWcsUrl;
};

#endif //QGIS2_99_0_QGSGEONODECLIENT_H
