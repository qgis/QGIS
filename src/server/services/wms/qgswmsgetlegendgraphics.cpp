/***************************************************************************
                              qgswmsgetlegendgraphics.cpp
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
#include "qgswmsgetlegendgraphics.h"
#include "qgswmsrenderer.h"

#include <QImage>

namespace QgsWms
{

  void writeGetLegendGraphics( QgsServerInterface *serverIface, const QgsProject *project,
                               const QString &version, const QgsServerRequest &request,
                               QgsServerResponse &response )
  {
    Q_UNUSED( version );

    QgsServerRequest::Parameters params = request.parameters();
    QString format = params.value( QStringLiteral( "FORMAT" ), QStringLiteral( "PNG" ) );

    QgsWmsParameters wmsParameters( QUrlQuery( request.url() ) );

    // Get cached image
    QgsAccessControl *accessControl = serverIface->accessControls();
    QgsServerCacheManager *cacheManager = serverIface->cacheManager();
    if ( cacheManager )
    {
      ImageOutputFormat outputFormat = parseImageFormat( format );
      QString saveFormat;
      QString contentType;
      switch ( outputFormat )
      {
        case PNG:
        case PNG8:
        case PNG16:
        case PNG1:
          contentType = "image/png";
          saveFormat = "PNG";
          break;
        case JPEG:
          contentType = "image/jpeg";
          saveFormat = "JPEG";
          break;
        default:
          throw QgsServiceException( "InvalidFormat",
                                     QString( "Output format '%1' is not supported in the GetLegendGraphic request" ).arg( format ) );
          break;
      }

      QImage image;
      QByteArray content = cacheManager->getCachedImage( project, request, accessControl );
      if ( !content.isEmpty() && image.loadFromData( content ) )
      {
        response.setHeader( QStringLiteral( "Content-Type" ), contentType );
        image.save( response.io(), qPrintable( saveFormat ) );
        return;
      }
    }

    QgsRenderer renderer( serverIface, project, wmsParameters );

    std::unique_ptr<QImage> result( renderer.getLegendGraphics() );

    if ( result )
    {
      writeImage( response, *result,  format, renderer.getImageQuality() );
      if ( cacheManager )
      {
        QByteArray content = response.data();
        if ( !content.isEmpty() )
          cacheManager->setCachedImage( &content, project, request, accessControl );
      }
    }
    else
    {
      throw QgsServiceException( QStringLiteral( "UnknownError" ),
                                 QStringLiteral( "Failed to compute GetLegendGraphics image" ) );
    }
  }


} // samespace QgsWms




