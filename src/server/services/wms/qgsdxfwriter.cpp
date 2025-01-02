/***************************************************************************
                        qgsdxfexport.cpp
  -------------------------------------------------------------------
Date                 : 20 December 2016
Copyright            : (C) 2015 by
email                : marco.hugentobler at sourcepole dot com (original code)
Copyright            : (C) 2016 by
email                : david dot marteau at 3liz dot com
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/
#include "qgsmodule.h"
#include "qgsdxfwriter.h"
#include "qgsdxfexport.h"
#include "qgswmsrenderer.h"
#include "qgswmsrestorer.h"

namespace QgsWms
{
  void writeAsDxf( QgsServerInterface *serverIface, const QgsProject *project, const QgsWmsRequest &request, QgsServerResponse &response )
  {
    // prepare render context
    QgsWmsRenderContext context( project, serverIface );
    context.setFlag( QgsWmsRenderContext::UseWfsLayersOnly );
    context.setFlag( QgsWmsRenderContext::UseOpacity );
    context.setFlag( QgsWmsRenderContext::UseFilter );
    context.setFlag( QgsWmsRenderContext::SetAccessControl );
    context.setParameters( request.wmsParameters() );
    context.setSocketFeedback( response.feedback() );

    // Write output
    QgsRenderer renderer( context );

    //Layer settings need to be kept until QgsDxfExport::writeToFile has finished
    std::unique_ptr<QgsWmsRestorer> restorer = std::make_unique<QgsWmsRestorer>( context );
    restorer.reset( new QgsWmsRestorer( context ) );

    std::unique_ptr<QgsDxfExport> dxf = renderer.getDxf();
    response.setHeader( "Content-Type", "application/dxf" );
    dxf->writeToFile( response.io(), request.wmsParameters().dxfCodec() );
  }
} // namespace QgsWms
