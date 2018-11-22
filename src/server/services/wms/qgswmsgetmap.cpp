/***************************************************************************
                              qgswmsgetmap.cpp
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
#include "qgswmsgetmap.h"
#include "qgswmsrenderer.h"

#include <QImage>

namespace QgsWms
{

  void writeGetMap( QgsServerInterface *serverIface, const QgsProject *project,
                    const QString &version, const QgsServerRequest &request,
                    QgsServerResponse &response )
  {
    Q_UNUSED( version );

    QgsServerRequest::Parameters params = request.parameters();

    QgsWmsParameters wmsParameters( QUrlQuery( request.url() ) );
    QgsRenderer renderer( serverIface, project, wmsParameters );

    std::unique_ptr<QImage> result( renderer.getMap() );
    if ( result )
    {
      QString format = params.value( QStringLiteral( "FORMAT" ), QStringLiteral( "PNG" ) );
      writeImage( response, *result, format, renderer.getImageQuality() );
    }
    else
    {
      throw QgsServiceException( QStringLiteral( "UnknownError" ),
                                 QStringLiteral( "Failed to compute GetMap image" ) );
    }
  }

} // namespace QgsWms




