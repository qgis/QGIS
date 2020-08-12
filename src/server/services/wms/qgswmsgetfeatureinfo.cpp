/***************************************************************************
                              qgswmsgetfeatureinfo.cpp
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
#include "qgswmsgetfeatureinfo.h"
#include "qgswmsrenderer.h"

namespace QgsWms
{
  void writeGetFeatureInfo( QgsServerInterface *serverIface, const QgsProject *project,
                            const QString &version, const QgsServerRequest &request,
                            QgsServerResponse &response )
  {
    // get wms parameters from query
    QgsWmsParameters parameters( QUrlQuery( request.url() ) );

    // WIDTH and HEIGHT are not mandatory, but we need to set a default size
    if ( ( parameters.widthAsInt() <= 0
           || parameters.heightAsInt() <= 0 )
         && ! parameters.infoFormatIsImage() )
    {
      QSize size( 10, 10 );

      if ( ! parameters.filterGeom().isEmpty() )
      {
        const QgsRectangle bbox = QgsGeometry::fromWkt( parameters.filterGeom() ).boundingBox();
        const int defaultWidth = 800;
        size.setWidth( defaultWidth );
        size.setHeight( defaultWidth * bbox.height() / bbox.width() );
      }

      parameters.set( QgsWmsParameter::WIDTH, size.width() );
      parameters.set( QgsWmsParameter::HEIGHT, size.height() );
    }

    // prepare render context
    QgsWmsRenderContext context( project, serverIface );
    context.setFlag( QgsWmsRenderContext::AddQueryLayers );
    context.setFlag( QgsWmsRenderContext::UseFilter );
    context.setFlag( QgsWmsRenderContext::UseScaleDenominator );
    context.setFlag( QgsWmsRenderContext::SetAccessControl );
    context.setParameters( parameters );

    const QString infoFormat = request.parameters().value( QStringLiteral( "INFO_FORMAT" ), QStringLiteral( "text/plain" ) );
    response.setHeader( QStringLiteral( "Content-Type" ), infoFormat + QStringLiteral( "; charset=utf-8" ) );

    QgsRenderer renderer( context );
    response.write( renderer.getFeatureInfo( version ) );
  }
} // namespace QgsWms
