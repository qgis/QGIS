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
#include "qgswmsserviceexception.h"

#include <QImage>

namespace QgsWms
{

  void writeGetMap( QgsServerInterface *serverIface, const QgsProject *project, const QgsWmsRequest &request, QgsServerResponse &response )
  {
    if ( request.serverParameters().version().isEmpty() )
    {
      throw QgsServiceException( QgsServiceException::OGC_OperationNotSupported, QStringLiteral( "Please add the value of the VERSION parameter" ), 501 );
    }

    // prepare render context
    QgsWmsRenderContext context( project, serverIface );
    context.setFlag( QgsWmsRenderContext::UpdateExtent );
    context.setFlag( QgsWmsRenderContext::UseOpacity );
    context.setFlag( QgsWmsRenderContext::UseFilter );
    context.setFlag( QgsWmsRenderContext::UseSelection );
    context.setFlag( QgsWmsRenderContext::AddHighlightLayers );
    context.setFlag( QgsWmsRenderContext::AddExternalLayers );
    context.setFlag( QgsWmsRenderContext::SetAccessControl );
    context.setFlag( QgsWmsRenderContext::UseTileBuffer );
    context.setParameters( request.wmsParameters() );
    context.setSocketFeedback( response.feedback() );

    // rendering
    QgsRenderer renderer( context );
    std::unique_ptr<QImage> result( renderer.getMap() );

    if ( result )
    {
      const QString format = request.parameters().value( QStringLiteral( "FORMAT" ), QStringLiteral( "PNG" ) );
      writeImage( response, *result, format, context.imageQuality() );
    }
    else
    {
      throw QgsException( QStringLiteral( "Failed to compute GetMap image" ) );
    }
  }
} // namespace QgsWms
