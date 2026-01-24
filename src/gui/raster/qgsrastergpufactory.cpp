/***************************************************************************
  qgsrastergpufactory.cpp
  -----------------------
  Date                 : January 2026
  Copyright            : (C) 2026 by Wietze Suijker
  Email                : wietze at gmail dot com
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "qgsrastergpufactory.h"
#include "qgsrasterlayerrenderer.h"
#include "qgsrastergpurenderer.h"
#include "qgsrastergputileuploader.h"
#include "qgscogtilereader.h"
#include "qgsrasterdataprovider.h"
#include "qgsrasterpipe.h"
#include "qgsrendercontext.h"
#include "qgslogger.h"

#include <QOpenGLContext>
#include <gdal.h>

namespace
{
  /**
   * \brief GPU renderer factory implementation
   *
   * This function is called by QgsRasterLayerRenderer when GPU rendering
   * should be attempted. It creates the necessary GPU components and
   * performs the rendering.
   */
  bool gpuRendererFactoryImpl(
    QgsRenderContext &context,
    QgsRasterViewPort *viewport,
    QgsRasterPipe *pipe,
    QgsFeedback *feedback )
  {
    // Check for OpenGL context
    if ( !QOpenGLContext::currentContext() )
    {
      QgsDebugMsgLevel( QStringLiteral( "GPU rendering skipped: no OpenGL context" ), 4 );
      return false;
    }

    // Get data provider
    QgsRasterDataProvider *provider = pipe ? pipe->provider() : nullptr;
    if ( !provider )
    {
      QgsDebugMsgLevel( QStringLiteral( "GPU rendering skipped: no data provider" ), 4 );
      return false;
    }

    // Open GDAL dataset from provider's data source
    const QString dataSource = provider->dataSourceUri();
    GDALDatasetH gdalDataset = GDALOpen( dataSource.toUtf8().constData(), GA_ReadOnly );
    if ( !gdalDataset )
    {
      QgsDebugMsgLevel( QStringLiteral( "GPU rendering skipped: cannot open GDAL dataset" ), 4 );
      return false;
    }

    // Create COG tile reader (it will take ownership of the dataset)
    std::unique_ptr<QgsCOGTileReader> reader = std::make_unique<QgsCOGTileReader>( gdalDataset );

    if ( !reader->isValid() )
    {
      QgsDebugMsgLevel( QStringLiteral( "GPU rendering skipped: COG reader initialization failed" ), 4 );
      return false;
    }

    // Check if dataset is tiled (COG)
    const auto tileInfo = reader->tileInfo( 0 );
    if ( !tileInfo.isTiled )
    {
      QgsDebugMsgLevel( QStringLiteral( "GPU rendering skipped: dataset is not tiled" ), 4 );
      return false;
    }

    // Create GPU tile uploader
    std::unique_ptr<QgsRasterGPUTileUploader> uploader = std::make_unique<QgsRasterGPUTileUploader>(
      reader.get()
    );

    // Create GPU renderer
    QgsRasterGPURenderer gpuRenderer( uploader.get() );

    // Attempt GPU rendering
    try
    {
      const bool success = gpuRenderer.render( context, viewport, feedback );
      if ( success )
      {
        QgsDebugMsgLevel( QStringLiteral( "GPU rendering completed successfully" ), 3 );
      }
      return success;
    }
    catch ( const std::exception &e )
    {
      QgsDebugError( QStringLiteral( "GPU rendering exception: %1" ).arg( e.what() ) );
      return false;
    }
  }
}

void QgsRasterGPUFactory::initialize()
{
  QgsDebugMsgLevel( QStringLiteral( "Initializing GPU raster rendering support" ), 2 );

  // Register the GPU renderer factory
  QgsRasterLayerRenderer::setGpuRendererFactory( gpuRendererFactoryImpl );

  QgsDebugMsgLevel( QStringLiteral( "GPU raster rendering support enabled" ), 2 );
}

void QgsRasterGPUFactory::cleanup()
{
  QgsDebugMsgLevel( QStringLiteral( "Cleaning up GPU raster rendering support" ), 2 );

  // Unregister the factory
  QgsRasterLayerRenderer::setGpuRendererFactory( nullptr );
}
