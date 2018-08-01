/***************************************************************************
                              qgswmsgetmap.cpp
                            -------------------------
  begin                : July 23 , 2017
  copyright            : (C) 2018 by René-Luc D'Hont
  email                : rldhont at 3liz dot com
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/
#include "qgswmtsutils.h"
#include "qgswmtsgettile.h"

#include <QImage>

namespace QgsWmts
{

  void writeGetTile( QgsServerInterface *serverIface, const QgsProject *project,
                     const QString &version, const QgsServerRequest &request,
                     QgsServerResponse &response )
  {
    Q_UNUSED( version );
    QgsServerRequest::Parameters params = request.parameters();

    // WMS query
    QUrlQuery query = translateWmtsParamToWmsQueryItem( QStringLiteral( "GetMap" ), params, project, serverIface );

    // Get cached image
    QStringList cacheKeyList;
    bool cache = true;

    QgsAccessControl *accessControl = serverIface->accessControls();
    if ( accessControl )
      cache = accessControl->fillCacheKey( cacheKeyList );

    QString cacheKey = cacheKeyList.join( QStringLiteral( "-" ) );
    QgsServerCacheManager *cacheManager = serverIface->cacheManager();
    if ( cacheManager && cache )
    {
      QString contentType = params.value( QStringLiteral( "FORMAT" ) );
      QString saveFormat;
      std::unique_ptr<QImage> image;
      if ( contentType == "image/jpeg" )
      {
        saveFormat = "JPEG";
        image = qgis::make_unique<QImage>( 256, 256, QImage::Format_RGB32 );
      }
      else
      {
        saveFormat = "PNG";
        image = qgis::make_unique<QImage>( 256, 256, QImage::Format_ARGB32_Premultiplied );
      }

      QByteArray content = cacheManager->getCachedImage( project, request, cacheKey );
      if ( !content.isEmpty() && image->loadFromData( content ) )
      {
        response.setHeader( "Content-Type", contentType );
        image->save( response.io(), qPrintable( saveFormat ) );
        return;
      }
    }


    QgsServerParameters wmsParams( query );
    QgsServerRequest wmsRequest( "?" + query.query( QUrl::FullyDecoded ) );
    QgsService *service = serverIface->serviceRegistry()->getService( wmsParams.service(), wmsParams.version() );
    service->executeRequest( wmsRequest, response, project );
    if ( cache && cacheManager )
    {
      QByteArray content = response.data();
      if ( !content.isEmpty() )
        cacheManager->setCachedImage( &content, project, request, cacheKey );
    }
  }

} // namespace QgsWmts




