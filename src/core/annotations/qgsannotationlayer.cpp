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
#include "qgspainting.h"
#include "qgsmaplayerfactory.h"
#include <QUuid>

QgsAnnotationLayer::QgsAnnotationLayer( const QString &name, const LayerOptions &options )
  : QgsMapLayer( QgsMapLayerType::AnnotationLayer, name )
  , mTransformContext( options.transformContext )
{
  mShouldValidateCrs = false;
  mValid = true;
}

QgsAnnotationLayer::~QgsAnnotationLayer()
{
  emit willBeDeleted();
  qDeleteAll( mItems );
}

void QgsAnnotationLayer::reset()
{
  setOpacity( 1.0 );
  setCrs( QgsCoordinateReferenceSystem() );
  setTransformContext( QgsCoordinateTransformContext() );
  clear();
}

QString QgsAnnotationLayer::addItem( QgsAnnotationItem *item )
{
  const QString uuid = QUuid::createUuid().toString();
  mItems.insert( uuid, item );

  triggerRepaint();

  return uuid;
}

bool QgsAnnotationLayer::removeItem( const QString &id )
{
  if ( !mItems.contains( id ) )
    return false;

  delete mItems.take( id );

  triggerRepaint();

  return true;
}

void QgsAnnotationLayer::clear()
{
  qDeleteAll( mItems );
  mItems.clear();

  triggerRepaint();
}

bool QgsAnnotationLayer::isEmpty() const
{
  return mItems.empty();
}

QgsAnnotationLayer *QgsAnnotationLayer::clone() const
{
  QgsAnnotationLayer::LayerOptions options( mTransformContext );
  std::unique_ptr< QgsAnnotationLayer > layer = std::make_unique< QgsAnnotationLayer >( name(), options );
  QgsMapLayer::clone( layer.get() );

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
    if ( rect.isNull() )
    {
      rect = it.value()->boundingBox();
    }
    else
    {
      rect.combineExtentWith( it.value()->boundingBox() );
    }
  }
  return rect;
}

void QgsAnnotationLayer::setTransformContext( const QgsCoordinateTransformContext &context )
{
  mTransformContext = context;
  invalidateWgs84Extent();
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

  triggerRepaint();

  return mValid;
}

bool QgsAnnotationLayer::writeXml( QDomNode &layer_node, QDomDocument &doc, const QgsReadWriteContext &context ) const
{
  // first get the layer element so that we can append the type attribute
  QDomElement mapLayerNode = layer_node.toElement();

  if ( mapLayerNode.isNull() )
  {
    QgsDebugMsgLevel( QStringLiteral( "can't find maplayer node" ), 2 );
    return false;
  }

  mapLayerNode.setAttribute( QStringLiteral( "type" ), QgsMapLayerFactory::typeToString( QgsMapLayerType::AnnotationLayer ) );

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

  if ( categories.testFlag( Symbology ) )
  {
    // add the blend mode field
    QDomElement blendModeElem  = doc.createElement( QStringLiteral( "blendMode" ) );
    QDomText blendModeText = doc.createTextNode( QString::number( QgsPainting::getBlendModeEnum( blendMode() ) ) );
    blendModeElem.appendChild( blendModeText );
    node.appendChild( blendModeElem );
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

  if ( categories.testFlag( Symbology ) )
  {
    // get and set the blend mode if it exists
    QDomNode blendModeNode = node.namedItem( QStringLiteral( "blendMode" ) );
    if ( !blendModeNode.isNull() )
    {
      QDomElement e = blendModeNode.toElement();
      setBlendMode( QgsPainting::getCompositionMode( static_cast< QgsPainting::BlendMode >( e.text().toInt() ) ) );
    }
  }

  return true;
}
