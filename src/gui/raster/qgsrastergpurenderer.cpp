/***************************************************************************
  qgsrastergpurenderer.cpp
  ------------------------
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

#include "qgsrastergpurenderer.h"
#include "qgsrastergputileuploader.h"
#include "qgsrastergpushaders.h"
#include "qgsrendercontext.h"
#include "qgsrasterviewport.h"
#include "qgsfeedback.h"
#include "qgsrectangle.h"
#include "qgscoordinatetransform.h"
#include "qgslogger.h"
#include <QOpenGLShaderProgram>
#include <QPainter>
#include <QMatrix4x4>
#include <cmath>

QgsRasterGPURenderer::QgsRasterGPURenderer( QgsRasterGPUTileUploader *tileUploader )
  : mTileUploader( tileUploader )
{
  initializeOpenGLFunctions();
}

QgsRasterGPURenderer::~QgsRasterGPURenderer()
{
  // Clean up OpenGL resources
  if ( mVAO )
  {
    glDeleteVertexArrays( 1, &mVAO );
  }
  if ( mVBO )
  {
    glDeleteBuffers( 1, &mVBO );
  }
  delete mShaderProgram;
}

bool QgsRasterGPURenderer::render( QgsRenderContext &renderContext,
                                   QgsRasterViewPort *rasterViewPort,
                                   QgsFeedback *feedback )
{
  if ( !mTileUploader || !rasterViewPort )
  {
    QgsDebugError( QStringLiteral( "Invalid tile uploader or viewport" ) );
    return false;
  }

  // Create shader program if needed
  if ( !mShaderProgram && !createShaderProgram() )
  {
    QgsDebugError( QStringLiteral( "Failed to create shader program" ) );
    return false;
  }

  // Select best overview level for current scale
  const int overviewLevel = selectOverviewLevel( rasterViewPort );

  // Get coordinate transform (map → raster)
  QgsCoordinateTransform transform;
  if ( rasterViewPort->mSrcCRS.isValid() && rasterViewPort->mDestCRS.isValid() )
  {
    transform = QgsCoordinateTransform( rasterViewPort->mDestCRS, rasterViewPort->mSrcCRS, renderContext.transformContext() );
  }

  // Calculate visible tiles
  const QVector<TileCoord> visibleTiles = calculateVisibleTiles( rasterViewPort, overviewLevel, transform );

  if ( visibleTiles.isEmpty() )
  {
    QgsDebugMsgLevel( QStringLiteral( "No visible tiles to render" ), 3 );
    return true;
  }

  // Setup OpenGL rendering state
  setupOpenGLState( renderContext );

  // Bind shader program
  mShaderProgram->bind();

  // Render each tile
  int tilesRendered = 0;
  for ( const TileCoord &tileCoord : visibleTiles )
  {
    // Check for cancellation
    if ( feedback && feedback->isCanceled() )
    {
      break;
    }

    // Upload tile to GPU (uses cache)
    const auto gpuTile = mTileUploader->getTile( tileCoord.level, tileCoord.x, tileCoord.y, 1, mFrameNumber );

    if ( !gpuTile.isValid )
    {
      QgsDebugMsgLevel( QStringLiteral( "Failed to upload tile %1/%2/%3" ).arg( tileCoord.level ).arg( tileCoord.x ).arg( tileCoord.y ), 4 );
      continue;
    }

    // Calculate tile extent in map coordinates
    const QgsRectangle tileExtent = mTileUploader->tileExtent( tileCoord.level, tileCoord.x, tileCoord.y );

    // Render the tile quad
    renderTileQuad( gpuTile.textureId, tileExtent, renderContext );

    tilesRendered++;
  }

  // Release shader
  mShaderProgram->release();

  // Restore OpenGL state
  restoreOpenGLState();

  // Increment frame number for LRU cache
  mFrameNumber++;

  QgsDebugMsgLevel( QStringLiteral( "Rendered %1 tiles at overview level %2" ).arg( tilesRendered ).arg( overviewLevel ), 2 );

  return tilesRendered > 0;
}

QVector<QgsRasterGPURenderer::TileCoord> QgsRasterGPURenderer::calculateVisibleTiles(
  const QgsRasterViewPort *viewport,
  int overviewLevel,
  const QgsCoordinateTransform &transform )
{
  QVector<TileCoord> tiles;

  if ( !viewport )
    return tiles;

  // Get viewport extent in raster CRS
  QgsRectangle extent = viewport->mDrawnExtent;

  // Transform extent if needed
  if ( transform.isValid() )
  {
    try
    {
      extent = transform.transformBoundingBox( extent );
    }
    catch ( const QgsCsException &e )
    {
      QgsDebugError( QStringLiteral( "Coordinate transform failed: %1" ).arg( e.what() ) );
      return tiles;
    }
  }

  // Get tile info for this overview level
  const auto tileInfo = mTileUploader->tileInfo( overviewLevel );
  if ( !tileInfo.isTiled )
  {
    QgsDebugMsgLevel( QStringLiteral( "Raster is not tiled, falling back to CPU path" ), 2 );
    return tiles;
  }

  // Get raster extent
  const QgsRectangle rasterExtent = mTileUploader->rasterExtent();

  // Calculate tile size in georeferenced units
  const double tileWidth = rasterExtent.width() / tileInfo.tilesX;
  const double tileHeight = rasterExtent.height() / tileInfo.tilesY;

  // Find tile range that intersects viewport
  const int minTileX = std::max( 0, static_cast<int>( std::floor( ( extent.xMinimum() - rasterExtent.xMinimum() ) / tileWidth ) ) );
  const int maxTileX = std::min( tileInfo.tilesX - 1, static_cast<int>( std::ceil( ( extent.xMaximum() - rasterExtent.xMinimum() ) / tileWidth ) ) );

  const int minTileY = std::max( 0, static_cast<int>( std::floor( ( rasterExtent.yMaximum() - extent.yMaximum() ) / tileHeight ) ) );
  const int maxTileY = std::min( tileInfo.tilesY - 1, static_cast<int>( std::ceil( ( rasterExtent.yMaximum() - extent.yMinimum() ) / tileHeight ) ) );

  // Generate tile coordinates
  for ( int tileY = minTileY; tileY <= maxTileY; ++tileY )
  {
    for ( int tileX = minTileX; tileX <= maxTileX; ++tileX )
    {
      tiles.append( TileCoord{overviewLevel, tileX, tileY} );
    }
  }

  return tiles;
}

int QgsRasterGPURenderer::selectOverviewLevel( const QgsRasterViewPort *viewport )
{
  if ( !viewport || !mTileUploader )
    return 0;

  // Calculate map units per pixel (MUPP)
  const double mapMupp = viewport->mDrawnExtent.width() / viewport->mWidth;

  // Get best overview from tile uploader
  return mTileUploader->selectBestOverview( mapMupp );
}

void QgsRasterGPURenderer::renderTileQuad( GLuint textureId,
    const QgsRectangle &tileExtent,
    const QgsRenderContext &context )
{
  // Bind texture
  glActiveTexture( GL_TEXTURE0 );
  glBindTexture( GL_TEXTURE_2D, textureId );
  mShaderProgram->setUniformValue( "uTileTexture", 0 );

  // Build MVP matrix (map extent → screen coordinates)
  // Transform from tile's map coordinates to normalized device coordinates [-1, 1]
  QMatrix4x4 mvpMatrix;

  const QgsRectangle &viewExtent = context.extent();

  // Scale: map units → normalized device coordinates
  const float scaleX = 2.0f / viewExtent.width();
  const float scaleY = 2.0f / viewExtent.height();

  // Translate: center map extent at origin
  const float translateX = -( viewExtent.xMinimum() + viewExtent.xMaximum() ) / viewExtent.width();
  const float translateY = -( viewExtent.yMinimum() + viewExtent.yMaximum() ) / viewExtent.height();

  mvpMatrix.scale( scaleX, scaleY );
  mvpMatrix.translate( translateX, translateY );

  mShaderProgram->setUniformValue( "uMVPMatrix", mvpMatrix );

  // Define tile quad vertices (in map coordinates)
  const float x0 = tileExtent.xMinimum();
  const float y0 = tileExtent.yMinimum();
  const float x1 = tileExtent.xMaximum();
  const float y1 = tileExtent.yMaximum();

  // Vertex data: position (x, y) + texcoord (u, v)
  const float vertices[] = {
    // Triangle 1
    x0, y0,  0.0f, 1.0f,  // bottom-left
    x1, y0,  1.0f, 1.0f,  // bottom-right
    x1, y1,  1.0f, 0.0f,  // top-right

    // Triangle 2
    x0, y0,  0.0f, 1.0f,  // bottom-left
    x1, y1,  1.0f, 0.0f,  // top-right
    x0, y1,  0.0f, 0.0f,  // top-left
  };

  // Upload vertex data
  if ( !mVBO )
  {
    glGenBuffers( 1, &mVBO );
  }

  glBindBuffer( GL_ARRAY_BUFFER, mVBO );
  glBufferData( GL_ARRAY_BUFFER, sizeof( vertices ), vertices, GL_DYNAMIC_DRAW );

  // Setup vertex attributes
  glVertexAttribPointer( 0, 2, GL_FLOAT, GL_FALSE, 4 * sizeof( float ), ( void * )0 );
  glEnableVertexAttribArray( 0 );

  glVertexAttribPointer( 1, 2, GL_FLOAT, GL_FALSE, 4 * sizeof( float ), ( void * )( 2 * sizeof( float ) ) );
  glEnableVertexAttribArray( 1 );

  // Draw tile quad
  glDrawArrays( GL_TRIANGLES, 0, 6 );

  // Cleanup
  glDisableVertexAttribArray( 0 );
  glDisableVertexAttribArray( 1 );
  glBindBuffer( GL_ARRAY_BUFFER, 0 );
  glBindTexture( GL_TEXTURE_2D, 0 );
}

bool QgsRasterGPURenderer::createShaderProgram()
{
  // Create shader configuration with default values
  // Future enhancement: Get data type and color ramp from QgsRasterRenderer
  QgsRasterGPUShaders::ShaderConfig config;
  config.type = QgsRasterGPUShaders::ShaderType::Byte;
  config.opacity = mOpacity;

  // Default grayscale color ramp (black → white)
  config.colorRamp = {
    {0.0f, QColor( 0, 0, 0 )},
    {1.0f, QColor( 255, 255, 255 )}
  };

  config.minValue = 0.0f;
  config.maxValue = 255.0f;

  mShaderProgram = QgsRasterGPUShaders::createShaderProgram( config );

  if ( !mShaderProgram )
  {
    QgsDebugError( QStringLiteral( "Failed to create shader program" ) );
    return false;
  }

  // Update uniforms
  QgsRasterGPUShaders::updateShaderUniforms( mShaderProgram, config );

  return true;
}

void QgsRasterGPURenderer::setupOpenGLState( const QgsRenderContext &context )
{
  Q_UNUSED( context )

  // Enable blending for transparency
  glEnable( GL_BLEND );
  glBlendFunc( GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA );

  // Disable depth test (2D rendering)
  glDisable( GL_DEPTH_TEST );

  // Note: Viewport is managed by QGIS painter context
  // No explicit glViewport() call needed here
}

void QgsRasterGPURenderer::restoreOpenGLState()
{
  // Restore default state
  glDisable( GL_BLEND );
  glEnable( GL_DEPTH_TEST );
}
