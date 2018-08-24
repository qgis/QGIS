/***************************************************************************
  qgslayoutitem3dmap.cpp
  --------------------------------------
  Date                 : August 2018
  Copyright            : (C) 2018 by Martin Dobias
  Email                : wonder dot sk at gmail dot com
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "qgslayoutitem3dmap.h"

#include "qgs3dmapscene.h"
#include "qgs3dutils.h"
#include "qgscameracontroller.h"
#include "qgslayout.h"
#include "qgslayoutitemregistry.h"
#include "qgsoffscreen3dengine.h"


QgsLayoutItem3DMap::QgsLayoutItem3DMap( QgsLayout *layout )
  : QgsLayoutItem( layout )
{
  connect( this, &QgsLayoutItem::sizePositionChanged, this, &QgsLayoutItem3DMap::onSizePositionChanged );
}

QgsLayoutItem3DMap::~QgsLayoutItem3DMap() = default;


QgsLayoutItem3DMap *QgsLayoutItem3DMap::create( QgsLayout *layout )
{
  return new QgsLayoutItem3DMap( layout );
}

int QgsLayoutItem3DMap::type() const
{
  return QgsLayoutItemRegistry::Layout3DMap;
}

void QgsLayoutItem3DMap::draw( QgsLayoutItemRenderContext &context )
{
  QgsRenderContext &ctx = context.renderContext();
  QPainter *painter = ctx.painter();

  int w = static_cast<int>( std::ceil( rect().width() * ctx.scaleFactor() ) );
  int h = static_cast<int>( std::ceil( rect().height() * ctx.scaleFactor() ) );
  QRect r( 0, 0, w, h );

  painter->save();

  if ( !mSettings )
  {
    painter->drawText( r, Qt::AlignCenter, tr( "Scene not set" ) );
    painter->restore();
    return;
  }

  if ( !mCapturedImage.isNull() )
  {
    painter->drawImage( r, mCapturedImage );
    painter->restore();
    return;
  }

  // we do not have a cached image of the rendered scene - let's request one from the engine

  painter->drawText( r, Qt::AlignCenter, tr( "Loading" ) );
  painter->restore();

  QSizeF sizePixels = mLayout->renderContext().measurementConverter().convert( sizeWithUnits(), QgsUnitTypes::LayoutPixels ).toQSizeF();
  QSize sizePixelsInt = QSize( static_cast<int>( std::ceil( sizePixels.width() ) ),
                               static_cast<int>( std::ceil( sizePixels.height() ) ) );

  if ( !mEngine )
  {
    mEngine.reset( new QgsOffscreen3DEngine );
    connect( mEngine.get(), &QgsAbstract3DEngine::imageCaptured, this, &QgsLayoutItem3DMap::onImageCaptured );

    mEngine->setSize( sizePixelsInt );

    mScene = new Qgs3DMapScene( *mSettings, mEngine.get() );
    connect( mScene, &Qgs3DMapScene::sceneStateChanged, this, &QgsLayoutItem3DMap::onSceneStateChanged );

    mEngine->setRootEntity( mScene );
  }

  if ( mEngine->size() != sizePixelsInt )
    mEngine->setSize( sizePixelsInt );

  mScene->cameraController()->setCameraPose( mCameraPose );

  onSceneStateChanged();
}

void QgsLayoutItem3DMap::onImageCaptured( const QImage &img )
{
  mCapturedImage = img;
  update();
}

void QgsLayoutItem3DMap::onSceneStateChanged()
{
  if ( mCapturedImage.isNull() && mScene->sceneState() == Qgs3DMapScene::Ready )
  {
    mEngine->requestCaptureImage();
  }
}

void QgsLayoutItem3DMap::onSizePositionChanged()
{
  mCapturedImage = QImage();
  update();
}


bool QgsLayoutItem3DMap::writePropertiesToElement( QDomElement &element, QDomDocument &document, const QgsReadWriteContext &context ) const
{
  if ( mSettings )
  {
    QDomElement elemSettings = mSettings->writeXml( document, context );
    element.appendChild( elemSettings );
  }

  QDomElement elemCameraPose = mCameraPose.writeXml( document );
  element.appendChild( elemCameraPose );

  return true;
}

bool QgsLayoutItem3DMap::readPropertiesFromElement( const QDomElement &element, const QDomDocument &document, const QgsReadWriteContext &context )
{
  Q_UNUSED( document );
  QDomElement elemSettings = element.firstChildElement( "qgis3d" );
  if ( !elemSettings.isNull() )
  {
    mSettings.reset( new Qgs3DMapSettings );
    mSettings->readXml( elemSettings, context );
    if ( mLayout->project() )
      mSettings->resolveReferences( *mLayout->project() );
  }

  QDomElement elemCameraPose = element.firstChildElement( "camera-pose" );
  if ( !elemCameraPose.isNull() )
    mCameraPose.readXml( elemCameraPose );

  return true;
}

void QgsLayoutItem3DMap::setMapSettings( Qgs3DMapSettings *settings )
{
  mSettings.reset( settings );

  mEngine.reset();
  mScene = nullptr;

  mCapturedImage = QImage();
  update();
}

void QgsLayoutItem3DMap::setCameraPose( const QgsCameraPose &pose )
{
  if ( mCameraPose == pose )
    return;

  mCameraPose = pose;
  mCapturedImage = QImage();
  update();
}
