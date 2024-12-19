/***************************************************************************
                        qgspdfwriter.cpp
  -------------------------------------------------------------------
Date                 : 09 October 2023
Copyright            : (C) 2023
email                : marco.hugentobler at sourcepole dot com

 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "qgsapplication.h"
#include "qgsmaprenderertask.h"
#include "qgsmapsettings.h"
#include "qgsmodule.h"
#include "qgspdfwriter.h"
#include "qgswmsrenderer.h"


namespace QgsWms
{
  void writeAsPdf( QgsServerInterface *serverIface, const QgsProject *project, const QgsWmsRequest &request, QgsServerResponse &response )
  {
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
    QTemporaryFile tmpFile;
    tmpFile.open();
    QgsRenderer renderer( context );
    std::unique_ptr<QgsMapRendererTask> pdfTask = renderer.getPdf( tmpFile.fileName() );
    QgsApplication::taskManager()->addTask( pdfTask.get() );
    pdfTask->waitForFinished();
    response.setHeader( "Content-Type", "application/pdf" );
    response.write( tmpFile.readAll() );
    tmpFile.close();
  }
} // namespace QgsWms
