/***************************************************************************
    qgsannotationlayer.cpp
    ------------------
    copyright            : (C) 2019 by Sandro Mani
    email                : smani at sourcepole dot ch
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "qgsannotationlayer.h"
#include "qgsannotationlayerrenderer.h"
#include <QUuid>

QgsAnnotationLayer::QgsAnnotationLayer( const QString &name, const LayerOptions &options )
  : QgsMapLayer( QgsMapLayerType::AnnotationLayer, name )
  , mTransformContext( options.transformContext )
{
  mValid = true;
}

void QgsAnnotationLayer::addItem( QgsAnnotationItem *item )
{
  mItems.insert( QUuid::createUuid().toString(), item );
}

#if 0
QgsAnnotationItem *QgsAnnotationItem::takeItem( const QString &itemId )
{
  return mItems.take( itemId );
}
#endif

QgsAnnotationLayer *QgsAnnotationLayer::clone() const
{
  QgsAnnotationLayer::LayerOptions options( mTransformContext );
  std::unique_ptr< QgsAnnotationLayer > layer = qgis::make_unique< QgsAnnotationLayer >( name(), options );
  QgsMapLayer::clone( layer.get() );

  layer->setOpacity( opacity() );

  for ( auto it = mItems.constBegin(); it != mItems.constEnd(); ++it )
  {
    layer->mItems.insert( it.key(), ( *it )->clone() );
  }

  return layer.release();
}

QgsMapLayerRenderer *QgsAnnotationLayer::createMapRenderer( QgsRenderContext &rendererContext )
{
  return new QgsAnnotationLayerRenderer( this, rendererContext );
}

QgsRectangle QgsAnnotationLayer::extent() const
{
  QgsRectangle rect;
  for ( auto it = mItems.constBegin(); it != mItems.constEnd(); ++it )
  {
#if 0
    QgsCoordinateTransform trans( it->crs(), crs(), mTransformContext );
    if ( rect.isNull() )
    {
      rect = trans.transform( item->boundingBox() );
    }
    else
    {
      rect.combineExtentWith( trans.transform( item->boundingBox() ) );
    }
#endif
  }
  return rect;
}

void QgsAnnotationLayer::setTransformContext( const QgsCoordinateTransformContext &ctx )
{
  mTransformContext = ctx;
}

#if 0
QString QgsAnnotationLayer::pickItem( const QgsRectangle &pickRect, const QgsMapSettings &mapSettings ) const
{
  for ( auto it = mItems.begin(), itEnd = mItems.end(); it != itEnd; ++it )
  {
    QgsCoordinateTransform crst( mapSettings.destinationCrs(), it.value()->crs(), transformContext() );
    if ( it.value()->intersects( crst.transform( pickRect ), mapSettings ) )
    {
      return it.key();
    }
  }
  return QString();
}

QString QgsAnnotationLayer::pickItem( const QgsPointXY &mapPos, const QgsMapSettings &mapSettings ) const
{
  QgsRenderContext renderContext = QgsRenderContext::fromMapSettings( mapSettings );
  double radiusmm = QgsSettings().value( "/Map/searchRadiusMM", Qgis::DEFAULT_SEARCH_RADIUS_MM ).toDouble();
  radiusmm = radiusmm > 0 ? radiusmm : Qgis::DEFAULT_SEARCH_RADIUS_MM;
  double radiusmu = radiusmm * renderContext.scaleFactor() * renderContext.mapToPixel().mapUnitsPerPixel();
  QgsRectangle filterRect;
  filterRect.setXMinimum( mapPos.x() - radiusmu );
  filterRect.setXMaximum( mapPos.x() + radiusmu );
  filterRect.setYMinimum( mapPos.y() - radiusmu );
  filterRect.setYMaximum( mapPos.y() + radiusmu );
  return pickItem( filterRect, mapSettings );
}

QRectF QgsAnnotationLayer::margin() const
{
  bool empty = true;
  QRectF rect;
  for ( const KadasMapItem *item : mItems )
  {
    if ( empty )
    {
      rect = item->margin();
    }
    else
    {
      rect = rect.united( item->margin() );
    }
  }
  return rect;
}
#endif

