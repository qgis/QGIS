/***************************************************************************
                         qgsmesh3drendercontroler.cpp
                         -------------------------
    begin                : january 2020
    copyright            : (C) 2020 by Vincent Cloarec
    email                : vcloarec at gmail dot com
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "qgsmesh3drendercontroler.h"


QgsMesh3dRenderControler::QgsMesh3dRenderControler( QgsAbstract3DEngine *engine3D ):
  QObject( engine3D ), mEngine3D( engine3D )
{
  createGrapheNode();
  engine3D->addFrameGraphNode( mRootFrameGraph );
}

void QgsMesh3dRenderControler::setBufferSize( int width, int height )
{
  mMeshColorBufferWidthParameter->setValue( width );
  mMeshColorBufferHeightParameter->setValue( height );
  mMeshColorBuffer->setSize( width, height );
  mMeshDepthBuffer->setSize( width, height );
  mMeshLightedColorBuffer->setSize( width, height );
  mMeshLightedDepthBuffer->setSize( width, height );
}

void QgsMesh3dRenderControler::activate()
{
  ///TODO
}

void QgsMesh3dRenderControler::deactivate()
{
  ///TODO
}

void QgsMesh3dRenderControler::createGrapheNode()
{
  QSize viewPortSize( mEngine3D->size() );

  mMeshColorBuffer = new Qt3DRender::QTexture2D();
  mMeshColorBuffer->setFormat( Qt3DRender::QAbstractTexture::RGBA32F );
  mMeshColorBuffer->setWidth( viewPortSize.width() );
  mMeshColorBuffer->setHeight( viewPortSize.height() );
  mMeshColorBuffer->setWrapMode( Qt3DRender::QTextureWrapMode( Qt3DRender::QTextureWrapMode::ClampToEdge ) );
  mMeshColorBuffer->setMinificationFilter( Qt3DRender::QAbstractTexture::Linear );
  mMeshColorBuffer->setMagnificationFilter( Qt3DRender::QAbstractTexture::Linear );

  mMeshDepthBuffer = new Qt3DRender::QTexture2D();
  mMeshDepthBuffer->setFormat( Qt3DRender::QAbstractTexture::DepthFormat );
  mMeshDepthBuffer->setWidth( viewPortSize.width() );
  mMeshDepthBuffer->setHeight( viewPortSize.height() );
  mMeshDepthBuffer->setWrapMode( Qt3DRender::QTextureWrapMode( Qt3DRender::QTextureWrapMode::ClampToEdge ) );
  mMeshDepthBuffer->setMinificationFilter( Qt3DRender::QAbstractTexture::Linear );
  mMeshDepthBuffer->setMagnificationFilter( Qt3DRender::QAbstractTexture::Linear );

  mMeshLightedColorBuffer = new Qt3DRender::QTexture2D();
  mMeshLightedColorBuffer->setFormat( Qt3DRender::QAbstractTexture::RGBA32F );
  mMeshLightedColorBuffer->setWidth( viewPortSize.width() );
  mMeshLightedColorBuffer->setHeight( viewPortSize.height() );
  mMeshLightedColorBuffer->setWrapMode( Qt3DRender::QTextureWrapMode( Qt3DRender::QTextureWrapMode::ClampToEdge ) );
  mMeshLightedColorBuffer->setMinificationFilter( Qt3DRender::QAbstractTexture::Linear );
  mMeshLightedColorBuffer->setMagnificationFilter( Qt3DRender::QAbstractTexture::Linear );

  mMeshLightedDepthBuffer = new Qt3DRender::QTexture2D();
  mMeshLightedDepthBuffer->setFormat( Qt3DRender::QAbstractTexture::DepthFormat );
  mMeshLightedDepthBuffer->setWidth( viewPortSize.width() );
  mMeshLightedDepthBuffer->setHeight( viewPortSize.height() );
  mMeshLightedDepthBuffer->setWrapMode( Qt3DRender::QTextureWrapMode( Qt3DRender::QTextureWrapMode::ClampToEdge ) );
  mMeshLightedDepthBuffer->setMinificationFilter( Qt3DRender::QAbstractTexture::Linear );
  mMeshLightedDepthBuffer->setMagnificationFilter( Qt3DRender::QAbstractTexture::Linear );

  mMeshColorBufferWidthParameter = new Qt3DRender::QParameter( "colorTextureWidth", viewPortSize.width() );
  mMeshColorBufferHeightParameter = new Qt3DRender::QParameter( "colorTextureHeight", viewPortSize.height() );

  mRootFrameGraph = new Qt3DRender::QFrameGraphNode;
  {
    Qt3DRender::QRenderSurfaceSelector *surfaceSelector = new Qt3DRender::QRenderSurfaceSelector( mRootFrameGraph );
    surfaceSelector->setSurface( mEngine3D->surface() );
    {
      Qt3DRender::QViewport *viewport = new Qt3DRender::QViewport( surfaceSelector );
      viewport->setNormalizedRect( QRectF( 0.0, 0.0, 1.0, 1.0 ) );
      {
        // Color technique
        Qt3DRender::QTechniqueFilter *techniqueColorMeshFilter = new Qt3DRender::QTechniqueFilter( viewport );
        {
          Qt3DRender::QFilterKey *filterMeshColorKey = new Qt3DRender::QFilterKey( techniqueColorMeshFilter );
          filterMeshColorKey->setName( "mesh" );
          filterMeshColorKey->setValue( "renderColor" );
          techniqueColorMeshFilter->addMatch( filterMeshColorKey );
          {
            Qt3DRender::QRenderTargetSelector *targetSelector = new Qt3DRender::QRenderTargetSelector( techniqueColorMeshFilter );
            {
              Qt3DRender::QRenderTarget *renderTarget = new Qt3DRender::QRenderTarget;
              targetSelector->setTarget( renderTarget );
              Qt3DRender::QRenderTargetOutput *targetOutput = new Qt3DRender::QRenderTargetOutput;
              targetOutput->setTexture( mMeshColorBuffer );
              targetOutput->setAttachmentPoint( Qt3DRender::QRenderTargetOutput::Color0 );
              renderTarget->addOutput( targetOutput );

              Qt3DRender::QRenderTargetOutput *targetDepthOutput = new Qt3DRender::QRenderTargetOutput;
              targetDepthOutput->setTexture( mMeshDepthBuffer );
              targetDepthOutput->setAttachmentPoint( Qt3DRender::QRenderTargetOutput::Depth );
              renderTarget->addOutput( targetDepthOutput );

              Qt3DRender::QClearBuffers *clearBuffer = new Qt3DRender::QClearBuffers( targetSelector );
              clearBuffer->setBuffers( Qt3DRender::QClearBuffers::ColorDepthBuffer );
              {
                Qt3DRender::QCameraSelector *cameraSelector = new Qt3DRender::QCameraSelector( clearBuffer );
                cameraSelector->setCamera( mEngine3D->camera() );
              }
            }
          }
        }

        // Light technique
        Qt3DRender::QTechniqueFilter *techniqueLightMeshFilter = new Qt3DRender::QTechniqueFilter( viewport );
        {
          Qt3DRender::QParameter *meshColorTextureParameter = new Qt3DRender::QParameter( "colorTexture", mMeshColorBuffer );
          techniqueLightMeshFilter->addParameter( meshColorTextureParameter );
          techniqueLightMeshFilter->addParameter( mMeshColorBufferWidthParameter );
          techniqueLightMeshFilter->addParameter( mMeshColorBufferHeightParameter );
          Qt3DRender::QFilterKey *filterMeshLightKey = new Qt3DRender::QFilterKey( techniqueLightMeshFilter );
          filterMeshLightKey->setName( "mesh" );
          filterMeshLightKey->setValue( "renderLight" );
          techniqueLightMeshFilter->addMatch( filterMeshLightKey );
          Qt3DRender::QRenderTargetSelector *targetSelector = new Qt3DRender::QRenderTargetSelector( techniqueLightMeshFilter );
          {
            Qt3DRender::QRenderTarget *renderTarget = new Qt3DRender::QRenderTarget;
            targetSelector->setTarget( renderTarget );
            Qt3DRender::QRenderTargetOutput *targetOutput = new Qt3DRender::QRenderTargetOutput;
            targetOutput->setTexture( mMeshLightedColorBuffer );
            targetOutput->setAttachmentPoint( Qt3DRender::QRenderTargetOutput::Color0 );
            renderTarget->addOutput( targetOutput );

            Qt3DRender::QRenderTargetOutput *targetDepthOutput = new Qt3DRender::QRenderTargetOutput;
            targetDepthOutput->setTexture( mMeshLightedDepthBuffer );
            targetDepthOutput->setAttachmentPoint( Qt3DRender::QRenderTargetOutput::Depth );
            renderTarget->addOutput( targetDepthOutput );

            Qt3DRender::QClearBuffers *clearBuffer = new Qt3DRender::QClearBuffers( targetSelector );
            clearBuffer->setBuffers( Qt3DRender::QClearBuffers::ColorDepthBuffer );
            {
              Qt3DRender::QCameraSelector *cameraSelector = new Qt3DRender::QCameraSelector( clearBuffer );
              cameraSelector->setCamera( mEngine3D->camera() );
            }
          }
        }

        // Wireframe technique
        Qt3DRender::QTechniqueFilter *techniqueWireFrameMeshFilter = new Qt3DRender::QTechniqueFilter( viewport );
        {
          Qt3DRender::QParameter *meshLightedColorTextureParameter = new Qt3DRender::QParameter( "lightedColorTexture", mMeshLightedColorBuffer );
          techniqueWireFrameMeshFilter->addParameter( meshLightedColorTextureParameter );
          techniqueWireFrameMeshFilter->addParameter( mMeshColorBufferWidthParameter );
          techniqueWireFrameMeshFilter->addParameter( mMeshColorBufferHeightParameter );
          Qt3DRender::QFilterKey *filterMeshWireframeKey = new Qt3DRender::QFilterKey( techniqueWireFrameMeshFilter );
          filterMeshWireframeKey->setName( "mesh" );
          filterMeshWireframeKey->setValue( "renderWireframe" );
          techniqueWireFrameMeshFilter->addMatch( filterMeshWireframeKey );

          Qt3DRender::QCameraSelector *cameraSelector = new Qt3DRender::QCameraSelector( filterMeshWireframeKey );
          cameraSelector->setCamera( mEngine3D->camera() );
        }
      }
    }
  }
}
