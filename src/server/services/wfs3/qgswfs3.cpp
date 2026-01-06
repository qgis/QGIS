/***************************************************************************
                              qgswfs3.cpp
                              -------------------------
  begin                : April 15, 2019
  copyright            : (C) 2019 by Alessandro Pasotti
  email                : elpaso at itopen dot it
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "qgsmodule.h"
#include "qgsserverogcapi.h"
#include "qgsserverstatichandler.h"
#include "qgswfs3handlers.h"

/**
 * \ingroup server
 * \class QgsWfsModule
 * \brief Module specialized for OAPIF (WFS3) service
 * \since QGIS 3.10
 */
class QgsWfs3Module : public QgsServiceModule
{
  public:
    void registerSelf( QgsServiceRegistry &registry, QgsServerInterface *serverIface ) override
    {
      // TODO: remove when QGIS 4 is released
#if _QGIS_VERSION_INT >= 40000
      QString rootPath = u"/ogcapi"_s;
#else
      QString rootPath = u"/wfs3"_s;
#endif
      if ( serverIface && serverIface->serverSettings() && !serverIface->serverSettings()->apiWfs3RootPath().isEmpty() )
      {
        rootPath = serverIface->serverSettings()->apiWfs3RootPath();
      }
      auto wfs3Api = std::make_unique<QgsServerOgcApi>( serverIface, rootPath, u"OAPIF"_s, u"1.0.0"_s );
      // Register handlers
      wfs3Api->registerHandler<QgsWfs3CollectionsItemsHandler>();
      wfs3Api->registerHandler<QgsWfs3CollectionsFeatureHandler>();
      wfs3Api->registerHandler<QgsWfs3CollectionsHandler>();
      wfs3Api->registerHandler<QgsWfs3DescribeCollectionHandler>();
      wfs3Api->registerHandler<QgsWfs3ConformanceHandler>();
      wfs3Api->registerHandler<QgsServerStaticHandler>();
      // API handler must access to the whole API
      wfs3Api->registerHandler<QgsWfs3APIHandler>( wfs3Api.get() );
      wfs3Api->registerHandler<QgsWfs3LandingPageHandler>();

      // Register API
      registry.registerApi( wfs3Api.release() );
    }
};


// Entry points
QGISEXTERN QgsServiceModule *QGS_ServiceModule_Init()
{
  static QgsWfs3Module module;
  return &module;
}
QGISEXTERN void QGS_ServiceModule_Exit( QgsServiceModule * )
{
  // Nothing to do
}
