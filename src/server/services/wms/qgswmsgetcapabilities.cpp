/***************************************************************************
                              qgswmsgetmap.h
                              -------------------------
  begin                : December 20 , 2016
  copyright            : (C) 2007 by Marco Hugentobler  (original code)
                         (C) 2014 by Alessandro Pasotti (original code)
                         (C) 2016 by David Marteau
  email                : marco dot hugentobler at karto dot baug dot ethz dot ch
                         a dot pasotti at itopen dot it
                         david dot marteau at 3liz dot com
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/
#include "qgswmsutils.h"
#include "qgswmsgetcapabilities.h"
#include "qgswmsservertransitional.h"

namespace QgsWms
{

  void writeGetCapabilities( QgsServerInterface* serverIface, const QString& version,
                             const QgsServerRequest& request, QgsServerResponse& response,
                             bool projectSettings )
  {
    QgsServerRequest::Parameters params = request.parameters();
    QString configFilePath = serverIface->configFilePath();
    QgsServerSettings* serverSettings = serverIface->serverSettings();
    QgsAccessControl* accessControl = serverIface->accessControls();
    QgsCapabilitiesCache* capabilitiesCache = serverIface->capabilitiesCache();

    QStringList cacheKeyList;
    cacheKeyList << ( projectSettings ? QStringLiteral( "projectSettings" ) : version );
    cacheKeyList << getenv( "SERVER_NAME" );
    bool cache = true;

    if ( accessControl )
      cache = accessControl->fillCacheKey( cacheKeyList );

    QString cacheKey = cacheKeyList.join( QStringLiteral( "-" ) );
    const QDomDocument* capabilitiesDocument = capabilitiesCache->searchCapabilitiesDocument( configFilePath, cacheKey );
    if ( !capabilitiesDocument ) //capabilities xml not in cache. Create a new one
    {
      QgsMessageLog::logMessage( QStringLiteral( "Capabilities document not found in cache" ) );
      QDomDocument doc;
      try
      {
        QgsWmsServer server( configFilePath,
                             *serverSettings,
                             params,
                             getConfigParser( serverIface ),
                             accessControl );
        doc = server.getCapabilities( version, projectSettings );
      }
      catch ( QgsMapServiceException& ex )
      {
        writeError( response, ex.code(), ex.message() );
        return;
      }
      if ( cache )
      {
        capabilitiesCache->insertCapabilitiesDocument( configFilePath, cacheKey, &doc );
        capabilitiesDocument = capabilitiesCache->searchCapabilitiesDocument( configFilePath, cacheKey );
      }
      else
      {
        doc = doc.cloneNode().toDocument();
        capabilitiesDocument = &doc;
      }
    }
    else
    {
      QgsMessageLog::logMessage( QStringLiteral( "Found capabilities document in cache" ) );
    }

    response.setHeader( QStringLiteral( "Content-Type" ), QStringLiteral( "text/xml; charset=utf-8" ) );
    response.write( capabilitiesDocument->toByteArray() );
  }


} // samespace QgsWms




