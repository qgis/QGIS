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
#include "qgsannotationitem.h"
#include "qgsannotationitemregistry.h"
#include "qgsapplication.h"
#include "qgslogger.h"
#include <QUuid>

QgsAnnotationLayer::QgsAnnotationLayer( const QString &name, const LayerOptions &options )
  : QgsMapLayer( QgsMapLayerType::AnnotationLayer, name )
  , mTransformContext( options.transformContext )
{
  mValid = true;
}

QgsAnnotationLayer::~QgsAnnotationLayer()
{
  emit willBeDeleted();
  qDeleteAll( mItems );
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

void QgsAnnotationLayer::setTransformContext( const QgsCoordinateTransformContext &context )
{
  mTransformContext = context;
}

bool QgsAnnotationLayer::readXml( const QDomNode &layerNode, QgsReadWriteContext &context )
{
  if ( mReadFlags & QgsMapLayer::FlagDontResolveLayers )
  {
    return false;
  }

  qDeleteAll( mItems );
  mItems.clear();

  QDomNodeList itemsElements = layerNode.toElement().elementsByTagName( QStringLiteral( "items" ) );
  if ( itemsElements.size() == 0 )
    return false;

  QDomNodeList items = itemsElements.at( 0 ).childNodes();
  for ( int i = 0; i < items.size(); ++i )
  {
    QDomElement itemElement = items.at( i ).toElement();
    const QString id = itemElement.attribute( QStringLiteral( "id" ) );
    const QString type = itemElement.attribute( QStringLiteral( "type" ) );
    std::unique_ptr< QgsAnnotationItem > item( QgsApplication::annotationItemRegistry()->createItem( type ) );
    if ( item )
    {
      item->readXml( itemElement, context );
      mItems.insert( id, item.release() );
    }
  }

  QString errorMsg;
  readSymbology( layerNode, errorMsg, context );

  return mValid;
}

bool QgsAnnotationLayer::writeXml( QDomNode &layer_node, QDomDocument &doc, const QgsReadWriteContext &context ) const
{
  // first get the layer element so that we can append the type attribute
  QDomElement mapLayerNode = layer_node.toElement();

  if ( mapLayerNode.isNull() || ( QLatin1String( "maplayer" ) != mapLayerNode.nodeName() ) )
  {
    QgsDebugMsgLevel( QStringLiteral( "can't find <maplayer>" ), 2 );
    return false;
  }

  mapLayerNode.setAttribute( QStringLiteral( "type" ), QStringLiteral( "annotation" ) );

  QDomElement itemsElement = doc.createElement( "items" );
  for ( auto it = mItems.constBegin(); it != mItems.constEnd(); ++it )
  {
    QDomElement itemElement = doc.createElement( "item" );
    itemElement.setAttribute( QStringLiteral( "type" ), ( *it )->type() );
    itemElement.setAttribute( QStringLiteral( "id" ), it.key() );
    ( *it )->writeXml( itemElement, doc, context );
    itemsElement.appendChild( itemElement );
  }
  mapLayerNode.appendChild( itemsElement );

  // renderer specific settings
  QString errorMsg;
  return writeSymbology( layer_node, doc, errorMsg, context );
}

bool QgsAnnotationLayer::writeSymbology( QDomNode &node, QDomDocument &doc, QString &, const QgsReadWriteContext &, QgsMapLayer::StyleCategories categories ) const
{
  // add the layer opacity
  if ( categories.testFlag( Rendering ) )
  {
    QDomElement layerOpacityElem  = doc.createElement( QStringLiteral( "layerOpacity" ) );
    QDomText layerOpacityText = doc.createTextNode( QString::number( opacity() ) );
    layerOpacityElem.appendChild( layerOpacityText );
    node.appendChild( layerOpacityElem );
  }
  return true;
}

bool QgsAnnotationLayer::readSymbology( const QDomNode &node, QString &, QgsReadWriteContext &, QgsMapLayer::StyleCategories categories )
{
  if ( categories.testFlag( Rendering ) )
  {
    QDomNode layerOpacityNode = node.namedItem( QStringLiteral( "layerOpacity" ) );
    if ( !layerOpacityNode.isNull() )
    {
      QDomElement e = layerOpacityNode.toElement();
      setOpacity( e.text().toDouble() );
    }
  }
  return true;
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

