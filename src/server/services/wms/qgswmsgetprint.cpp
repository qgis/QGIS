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
#include "qgswmsgetprint.h"

#include "qgswmsrenderer.h"
#include "qgswmsrequest.h"
#include "qgswmsserviceexception.h"
#include "qgswmsutils.h"

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
        contentType = u"image/png"_s;
        break;
      case QgsWmsParameters::JPG:
        contentType = u"image/jpeg"_s;
        break;
      case QgsWmsParameters::SVG:
        contentType = u"image/svg+xml"_s;
        break;
      case QgsWmsParameters::PDF:
        contentType = u"application/pdf"_s;
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
    response.setHeader( u"Content-Type"_s, contentType );
    response.write( renderer.getPrint() );
  }
} // namespace QgsWms
