//
// Created by myarjunar on 15/03/17.
//

#ifndef QGSGEONODECONNECTION_H
#define QGSGEONODECONNECTION_H

#include "qgsgeonodedataitems.h"
#include "qgswfsdataitems.h"
#include "qgswmsdataitems.h"
#include "qgswcsdataitems.h"
#include "qgsauthconfig.h"
#include "qgsdatasourceuri.h"
#include "qgswfsconnection.h"
#include "qgswmsconneection.h"

class QgsAuthMethodConfig;

class QgsGeoNodeConnection
{
  //Constructor
  QgsGeoNodeConnection( const QString &baseKey = "/Qgis/connections-geonode/", const QString &connName = QString::null );

  void saveConnection();
  void editConnectio();
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
