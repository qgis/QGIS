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

#include <memory>

#include "qgs3dmapscene.h"
#include "qgs3dutils.h"
#include "qgscameracontroller.h"
#include "qgslayout.h"
#include "qgslayoutitemregistry.h"
#include "qgslayoutmodel.h"
#include "qgslayoutrendercontext.h"
#include "qgsoffscreen3dengine.h"

#include "moc_qgslayoutitem3dmap.cpp"

QgsLayoutItem3DMap::QgsLayoutItem3DMap( QgsLayout *layout )
  : QgsLayoutItem( layout )
{
  assignFreeId();

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

QIcon QgsLayoutItem3DMap::icon() const
{
  return QgsApplication::getThemeIcon( u"/mLayoutItem3DMap.svg"_s );
}

void QgsLayoutItem3DMap::assignFreeId()
{
  if ( !mLayout )
    return;

  QList<QgsLayoutItem3DMap *> mapsList;
  mLayout->layoutItems( mapsList );

  int maxId = -1;
  bool used = false;
  for ( QgsLayoutItem3DMap *map : std::as_const( mapsList ) )
  {
    if ( map == this )
      continue;

    if ( map->mMapId == mMapId )
      used = true;

    maxId = std::max( maxId, map->mMapId );
  }
  if ( used )
  {
    mMapId = maxId + 1;
    mLayout->itemsModel()->updateItemDisplayName( this );
  }
  updateToolTip();
}

QString QgsLayoutItem3DMap::displayName() const
{
  if ( !QgsLayoutItem::id().isEmpty() )
  {
    return QgsLayoutItem::id();
  }

  return tr( "3D Map %1" ).arg( mMapId );
}

void QgsLayoutItem3DMap::updateToolTip()
{
  setToolTip( displayName() );
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

  if ( mSettings->backgroundColor() != backgroundColor() )
  {
    mSettings->setBackgroundColor( backgroundColor() );
    mCapturedImage = QImage();
  }

  if ( !mCapturedImage.isNull() )
  {
    painter->drawImage( r, mCapturedImage );
    painter->restore();
    return;
  }

  // we do not have a cached image of the rendered scene - let's request one from the engine

  if ( mLayout->renderContext().isPreviewRender() )
  {
    painter->drawText( r, Qt::AlignCenter, tr( "Loading" ) );
    painter->restore();
    if ( mSettings->rendererUsage() != Qgis::RendererUsage::View )
    {
      mSettings->setRendererUsage( Qgis::RendererUsage::View );
      mEngine.reset(); //we need to rebuild the scene to force the render again
    }
  }
  else
  {
    if ( mSettings->rendererUsage() != Qgis::RendererUsage::Export )
    {
      mSettings->setRendererUsage( Qgis::RendererUsage::Export );
      mEngine.reset(); //we need to rebuild the scene to force the render again
    }
  }

  QSizeF sizePixels = mLayout->renderContext().measurementConverter().convert( sizeWithUnits(), Qgis::LayoutUnit::Pixels ).toQSizeF();
  QSize sizePixelsInt = QSize( static_cast<int>( std::ceil( sizePixels.width() ) ), static_cast<int>( std::ceil( sizePixels.height() ) ) );

  if ( isTemporal() )
    mSettings->setTemporalRange( temporalRange() );

  if ( !mEngine )
  {
    mEngine = std::make_unique<QgsOffscreen3DEngine>();
    connect( mEngine.get(), &QgsAbstract3DEngine::imageCaptured, this, &QgsLayoutItem3DMap::onImageCaptured );

    mEngine->setSize( sizePixelsInt );
    mScene = new Qgs3DMapScene( *mSettings, mEngine.get() );
    connect( mScene, &Qgs3DMapScene::sceneStateChanged, this, &QgsLayoutItem3DMap::onSceneStateChanged );

    mEngine->setRootEntity( mScene );
  }

  if ( mEngine->size() != sizePixelsInt )
    mEngine->setSize( sizePixelsInt );

  mScene->cameraController()->setCameraPose( mCameraPose );

  if ( mLayout->renderContext().isPreviewRender() )
  {
    onSceneStateChanged();
  }
  else
  {
    // we can't just request a capture and hope it will arrive at some point later.
    // this is not a preview, we need the rendered scene now!
    if ( mDrawing )
      return;
    mDrawing = true;
    disconnect( mEngine.get(), &QgsAbstract3DEngine::imageCaptured, this, &QgsLayoutItem3DMap::onImageCaptured );
    disconnect( mScene, &Qgs3DMapScene::sceneStateChanged, this, &QgsLayoutItem3DMap::onSceneStateChanged );

    Qgs3DUtils::captureSceneImage( *mEngine.get(), mScene );
    QImage img = Qgs3DUtils::captureSceneImage( *mEngine.get(), mScene );
    painter->drawImage( r, img );
    painter->restore();

    connect( mEngine.get(), &QgsAbstract3DEngine::imageCaptured, this, &QgsLayoutItem3DMap::onImageCaptured );
    connect( mScene, &Qgs3DMapScene::sceneStateChanged, this, &QgsLayoutItem3DMap::onSceneStateChanged );
    mDrawing = false;
  }
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

  //temporal settings
  QDomElement elemTemporal = document.createElement( u"temporal-settings"_s );
  elemTemporal.setAttribute( u"isTemporal"_s, isTemporal() ? 1 : 0 );
  if ( isTemporal() )
  {
    elemTemporal.setAttribute( u"temporalRangeBegin"_s, temporalRange().begin().toString( Qt::ISODate ) );
    elemTemporal.setAttribute( u"temporalRangeEnd"_s, temporalRange().end().toString( Qt::ISODate ) );
  }
  element.appendChild( elemTemporal );

  return true;
}

bool QgsLayoutItem3DMap::readPropertiesFromElement( const QDomElement &element, const QDomDocument &document, const QgsReadWriteContext &context )
{
  Q_UNUSED( document )
  QDomElement elemSettings = element.firstChildElement( u"qgis3d"_s );
  if ( !elemSettings.isNull() )
  {
    mSettings = std::make_unique<Qgs3DMapSettings>();
    mSettings->readXml( elemSettings, context );
    if ( mLayout->project() )
    {
      mSettings->resolveReferences( *mLayout->project() );

      mSettings->setTransformContext( mLayout->project()->transformContext() );
      mSettings->setPathResolver( mLayout->project()->pathResolver() );
      mSettings->setMapThemeCollection( mLayout->project()->mapThemeCollection() );
    }
  }

  QDomElement elemCameraPose = element.firstChildElement( u"camera-pose"_s );
  if ( !elemCameraPose.isNull() )
    mCameraPose.readXml( elemCameraPose );

  //temporal settings
  QDomElement elemTemporal = element.firstChildElement( u"temporal-settings"_s );
  setIsTemporal( elemTemporal.attribute( u"isTemporal"_s ).toInt() );
  if ( isTemporal() )
  {
    QDateTime begin = QDateTime::fromString( elemTemporal.attribute( u"temporalRangeBegin"_s ), Qt::ISODate );
    QDateTime end = QDateTime::fromString( elemTemporal.attribute( u"temporalRangeBegin"_s ), Qt::ISODate );
    setTemporalRange( QgsDateTimeRange( begin, end ) );
  }

  return true;
}

void QgsLayoutItem3DMap::finalizeRestoreFromXml()
{
  assignFreeId();
}

void QgsLayoutItem3DMap::setMapSettings( Qgs3DMapSettings *settings )
{
  mSettings.reset( settings );

  mEngine.reset();
  mScene = nullptr;

  mCapturedImage = QImage();
  update();
}

void QgsLayoutItem3DMap::refresh()
{
  QgsLayoutItem::refresh();

  mCapturedImage = QImage();
}

void QgsLayoutItem3DMap::setCameraPose( const QgsCameraPose &pose )
{
  if ( mCameraPose == pose )
    return;

  mCameraPose = pose;
  mCapturedImage = QImage();
  update();
}
