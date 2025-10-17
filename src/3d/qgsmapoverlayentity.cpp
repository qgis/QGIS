/***************************************************************************
  qgsmapoverlayentity.cpp
  --------------------------------------
  Date                 : July 2025
  Copyright            : (C) 2025 by Jean Felder
  Email                : jean dot felder at oslandia dot com
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "qgsmapoverlayentity.h"
#include "moc_qgsmapoverlayentity.cpp"
#include "qgs3dmapsettings.h"
#include "qgsdebugtextureentity.h"
#include "qgsdebugtexturerenderview.h"
#include "qgsmapoverlaytexturegenerator_p.h"
#include "qgswindow3dengine.h"

#include <QWindow>
#include <QOpenGLTexture>
#include <Qt3DExtras/QTextureMaterial>
#include <Qt3DRender/QTextureDataUpdate>


///@cond PRIVATE

const int QgsMapOverlayEntity::SIZE = 256;

QgsMapOverlayEntity::QgsMapOverlayEntity( QgsWindow3DEngine *engine, QgsDebugTextureRenderView *debugTextureRenderView, Qgs3DMapSettings *mapSettings, Qt3DCore::QNode *parent )
  : QgsDebugTextureEntity( new Qt3DRender::QTexture2D(), debugTextureRenderView->debugLayer(), parent )
  , mEngine( engine )
  , mMapSettings( mapSettings )
  , mTextureGenerator( new QgsMapOverlayTextureGenerator( *mapSettings, QgsMapOverlayEntity::SIZE ) )
{
  Qt3DRender::QTexture2D *mapTexture = mTextureParameter->value().value<Qt3DRender::QTexture2D *>();
  mapTexture->setFormat( Qt3DRender::QAbstractTexture::RGBA8_UNorm );
  mapTexture->setWidth( QgsMapOverlayEntity::SIZE );
  mapTexture->setHeight( QgsMapOverlayEntity::SIZE );
  mapTexture->setGenerateMipMaps( false );
  mapTexture->setMinificationFilter( Qt3DRender::QTexture2D::Linear );
  mapTexture->setMagnificationFilter( Qt3DRender::QTexture2D::Linear );
  mapTexture->setWrapMode( Qt3DRender::QTextureWrapMode( Qt3DRender::QTextureWrapMode::ClampToEdge ) );

  mIsDepth->setValue( false );
  mFlipTextureY->setValue( false );

  mImageDataPtr = Qt3DRender::QTextureImageDataPtr::create();
  mImageDataPtr->setTarget( QOpenGLTexture::Target2D );
  mImageDataPtr->setFormat( QOpenGLTexture::RGB8_UNorm );
  mImageDataPtr->setPixelFormat( QOpenGLTexture::RGB );
  mImageDataPtr->setPixelType( QOpenGLTexture::UInt8 );

  mImageDataPtr->setWidth( QgsMapOverlayEntity::SIZE );
  mImageDataPtr->setHeight( QgsMapOverlayEntity::SIZE );
  mImageDataPtr->setDepth( 1 );
  mImageDataPtr->setLayers( 1 );
  mImageDataPtr->setMipLevels( 1 );
  mImageDataPtr->setFaces( 1 );

  onSizeChanged();
  connect( mEngine, &QgsWindow3DEngine::sizeChanged, this, &QgsMapOverlayEntity::onSizeChanged );

  connect( mTextureGenerator, &QgsMapOverlayTextureGenerator::textureReady, this, &QgsMapOverlayEntity::onTextureReady );

  connectToLayersRepaintRequest();
  connect( mapSettings, &Qgs3DMapSettings::layersChanged, this, &QgsMapOverlayEntity::onLayersChanged );
}

QgsMapOverlayEntity::~QgsMapOverlayEntity()
{
  delete mTextureGenerator;
}

void QgsMapOverlayEntity::update( const QgsRectangle &extent, double rotationDegrees )
{
  if ( !extent.isEmpty() )
  {
    mExtent = extent;
    mRotation = rotationDegrees;
    mTextureGenerator->render( extent, rotationDegrees );
  }
}

void QgsMapOverlayEntity::invalidateMapImage()
{
  update( mExtent, mRotation );
}

void QgsMapOverlayEntity::onLayersChanged()
{
  connectToLayersRepaintRequest();
  invalidateMapImage();
}

void QgsMapOverlayEntity::connectToLayersRepaintRequest()
{
  for ( QgsMapLayer *layer : std::as_const( mLayers ) )
  {
    disconnect( layer, &QgsMapLayer::repaintRequested, this, &QgsMapOverlayEntity::invalidateMapImage );
  }

  mLayers = mMapSettings->layers();

  for ( QgsMapLayer *layer : std::as_const( mLayers ) )
  {
    connect( layer, &QgsMapLayer::repaintRequested, this, &QgsMapOverlayEntity::invalidateMapImage );
  }
}

void QgsMapOverlayEntity::onTextureReady( const QImage &image )
{
  Qt3DRender::QTexture2D *mapTexture = mTextureParameter->value().value<Qt3DRender::QTexture2D *>();
  if ( mapTexture )
  {
    mImageDataPtr->setImage( image );
    Qt3DRender::QTextureDataUpdate textureDataUpdate;
    textureDataUpdate.setData( mImageDataPtr );
    mapTexture->updateData( textureDataUpdate );
  }
}

void QgsMapOverlayEntity::onSizeChanged()
{
  QWindow *canvas = mEngine->window();
  const float currentHeightF = static_cast<float>( canvas->height() );
  const float currentWidthF = static_cast<float>( canvas->width() );
  const float offsetDim = 20.;
  const float textureDim = static_cast<float>( QgsMapOverlayEntity::SIZE );

  const QSizeF size( textureDim / currentWidthF, textureDim / currentHeightF );
  const QSizeF offset( offsetDim / currentWidthF, offsetDim / currentHeightF );
  setPosition( Qt::Corner::BottomLeftCorner, size, offset );
}

/// @endcond
