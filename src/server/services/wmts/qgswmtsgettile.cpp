/***************************************************************************
                              qgswmtsgettile.cpp
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
#include "qgswmtsparameters.h"
#include "qgswmtsgettile.h"

#include <QImage>

namespace QgsWmts
{

  void writeGetTile( QgsServerInterface *serverIface, const QgsProject *project,
                     const QString &version, const QgsServerRequest &request,
                     QgsServerResponse &response )
  {
    Q_UNUSED( version );
    const QgsWmtsParameters params( QUrlQuery( request.url() ) );

    // WMS query
    QUrlQuery query = translateWmtsParamToWmsQueryItem( QStringLiteral( "GetMap" ), params, project, serverIface );

    // Get cached image
    QgsAccessControl *accessControl = nullptr;
    QgsServerCacheManager *cacheManager = nullptr;
#ifdef HAVE_SERVER_PYTHON_PLUGINS
    accessControl = serverIface->accessControls();
    cacheManager = serverIface->cacheManager();
#endif
    if ( cacheManager )
    {
      QgsWmtsParameters::Format f = params.format();
      QString contentType;
      QString saveFormat;
      std::unique_ptr<QImage> image;
      if ( f == QgsWmtsParameters::Format::JPG )
      {
        contentType = QStringLiteral( "image/jpeg" );
        saveFormat = QStringLiteral( "JPEG" );
        image = qgis::make_unique<QImage>( 256, 256, QImage::Format_RGB32 );
      }
      else
      {
        contentType = QStringLiteral( "image/png" );
        saveFormat = QStringLiteral( "PNG" );
        image = qgis::make_unique<QImage>( 256, 256, QImage::Format_ARGB32_Premultiplied );
      }

      QByteArray content = cacheManager->getCachedImage( project, request, accessControl );
      if ( !content.isEmpty() && image->loadFromData( content ) )
      {
        response.setHeader( QStringLiteral( "Content-Type" ), contentType );
        image->save( response.io(), qPrintable( saveFormat ) );
        return;
      }
    }


    QgsServerParameters wmsParams( query );
    QgsServerRequest wmsRequest( "?" + query.query( QUrl::FullyDecoded ) );
    QgsService *service = serverIface->serviceRegistry()->getService( wmsParams.service(), wmsParams.version() );
    service->executeRequest( wmsRequest, response, project );
    if ( cacheManager )
    {
      QByteArray content = response.data();
      if ( !content.isEmpty() )
        cacheManager->setCachedImage( &content, project, request, accessControl );
    }
  }

} // namespace QgsWmts




