/***************************************************************************
                              qgswmsgetprint.h
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
#include "qgswmsrequest.h"
#include "qgswmsgetprint.h"
#include "qgswmsrenderer.h"
#include "qgswmsserviceexception.h"

namespace QgsWms
{
  void writeGetPrint( QgsServerInterface *serverIface, const QgsProject *project, const QgsWmsRequest &request, QgsServerResponse &response )
  {
    const QgsWmsParameters parameters = request.wmsParameters();

    // GetPrint supports svg/png/pdf
    const QgsWmsParameters::Format format = parameters.format();
    QString contentType;
    switch ( format )
    {
      case QgsWmsParameters::PNG:
        contentType = QStringLiteral( "image/png" );
        break;
      case QgsWmsParameters::JPG:
        contentType = QStringLiteral( "image/jpeg" );
        break;
      case QgsWmsParameters::SVG:
        contentType = QStringLiteral( "image/svg+xml" );
        break;
      case QgsWmsParameters::PDF:
        contentType = QStringLiteral( "application/pdf" );
        break;
      default:
        throw QgsBadRequestException( QgsServiceException::OGC_InvalidFormat, parameters[QgsWmsParameter::FORMAT] );
        break;
    }

    // prepare render context
    QgsWmsRenderContext context( project, serverIface );
    context.setFlag( QgsWmsRenderContext::UpdateExtent );
    context.setFlag( QgsWmsRenderContext::UseOpacity );
    context.setFlag( QgsWmsRenderContext::UseFilter );
    context.setFlag( QgsWmsRenderContext::UseSelection );
    context.setFlag( QgsWmsRenderContext::SetAccessControl );
    context.setFlag( QgsWmsRenderContext::AddHighlightLayers );
    context.setFlag( QgsWmsRenderContext::AddExternalLayers );
    context.setFlag( QgsWmsRenderContext::AddAllLayers );
    context.setParameters( parameters );
    context.setSocketFeedback( response.feedback() );

    // rendering
    QgsRenderer renderer( context );
    response.setHeader( QStringLiteral( "Content-Type" ), contentType );
    response.write( renderer.getPrint() );
  }
} // namespace QgsWms
