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
#include "qgsfeedback.h"
#include <QUuid>
#include "RTree.h"


class QgsAnnotationLayerSpatialIndex : public RTree<QString, float, 2, float>
{
  public:

    void insert( const QString &uuid, const QgsRectangle &bounds )
    {
      std::array< float, 4 > scaledBounds = scaleBounds( bounds );
      this->Insert(
      {
        scaledBounds[0], scaledBounds[ 1]
      },
      {
        scaledBounds[2], scaledBounds[3]
      },
      uuid );
    }

    /**
     * Removes existing \a data from the spatial index, with the specified \a bounds.
     *
     * \a data is not deleted, and it is the caller's responsibility to ensure that
     * it is appropriately cleaned up.
     */
    void remove( const QString &uuid, const QgsRectangle &bounds )
    {
      std::array< float, 4 > scaledBounds = scaleBounds( bounds );
      this->Remove(
      {
        scaledBounds[0], scaledBounds[ 1]
      },
      {
        scaledBounds[2], scaledBounds[3]
      },
      uuid );
    }

    /**
     * Performs an intersection check against the index, for data intersecting the specified \a bounds.
     *
     * The \a callback function will be called once for each matching data object encountered.
     */
    bool intersects( const QgsRectangle &bounds, const std::function< bool( const QString &uuid )> &callback ) const
    {
      std::array< float, 4 > scaledBounds = scaleBounds( bounds );
      this->Search(
      {
        scaledBounds[0], scaledBounds[ 1]
      },
      {
        scaledBounds[2], scaledBounds[3]
      },
      callback );
      return true;
    }

  private:
    std::array<float, 4> scaleBounds( const QgsRectangle &bounds ) const
    {
      return
      {
        static_cast< float >( bounds.xMinimum() ),
        static_cast< float >( bounds.yMinimum() ),
        static_cast< float >( bounds.xMaximum() ),
        static_cast< float >( bounds.yMaximum() )
      };
    }
};

QgsAnnotationLayer::QgsAnnotationLayer( const QString &name, const LayerOptions &options )
  : QgsMapLayer( QgsMapLayerType::AnnotationLayer, name )
  , mTransformContext( options.transformContext )
  , mSpatialIndex( std::make_unique< QgsAnnotationLayerSpatialIndex >() )
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
  mSpatialIndex->insert( uuid, item->boundingBox() );

  triggerRepaint();

  return uuid;
}

bool QgsAnnotationLayer::removeItem( const QString &id )
{
  if ( !mItems.contains( id ) )
    return false;

  std::unique_ptr< QgsAnnotationItem> item( mItems.take( id ) );
  mSpatialIndex->remove( id, item->boundingBox() );
  item.reset();

  triggerRepaint();

  return true;
}

void QgsAnnotationLayer::clear()
{
  qDeleteAll( mItems );
  mItems.clear();
  mSpatialIndex = std::make_unique< QgsAnnotationLayerSpatialIndex >();

  triggerRepaint();
}

bool QgsAnnotationLayer::isEmpty() const
{
  return mItems.empty();
}

QgsAnnotationItem *QgsAnnotationLayer::item( const QString &id )
{
  return mItems.value( id );
}

QStringList QgsAnnotationLayer::itemsInBounds( const QgsRectangle &bounds, QgsFeedback *feedback ) const
{
  QStringList res;

  mSpatialIndex->intersects( bounds, [&res, feedback]( const QString & uuid )->bool
  {
    res << uuid;
    return !feedback || !feedback->isCanceled();
  } );
  return res;
}

Qgis::MapLayerProperties QgsAnnotationLayer::properties() const
{
  // annotation layers are always editable
  return Qgis::MapLayerProperty::UsersCannotToggleEditing;
}

QgsAnnotationLayer *QgsAnnotationLayer::clone() const
{
  const QgsAnnotationLayer::LayerOptions options( mTransformContext );
  std::unique_ptr< QgsAnnotationLayer > layer = std::make_unique< QgsAnnotationLayer >( name(), options );
  QgsMapLayer::clone( layer.get() );

  for ( auto it = mItems.constBegin(); it != mItems.constEnd(); ++it )
  {
    layer->mItems.insert( it.key(), ( *it )->clone() );
    layer->mSpatialIndex->insert( it.key(), ( *it )->boundingBox() );
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
  mSpatialIndex = std::make_unique< QgsAnnotationLayerSpatialIndex >();

  const QDomNodeList itemsElements = layerNode.toElement().elementsByTagName( QStringLiteral( "items" ) );
  if ( itemsElements.size() == 0 )
    return false;

  const QDomNodeList items = itemsElements.at( 0 ).childNodes();
  for ( int i = 0; i < items.size(); ++i )
  {
    const QDomElement itemElement = items.at( i ).toElement();
    const QString id = itemElement.attribute( QStringLiteral( "id" ) );
    const QString type = itemElement.attribute( QStringLiteral( "type" ) );
    std::unique_ptr< QgsAnnotationItem > item( QgsApplication::annotationItemRegistry()->createItem( type ) );
    if ( item )
    {
      item->readXml( itemElement, context );
      mSpatialIndex->insert( id, item->boundingBox() );
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

  QDomElement itemsElement = doc.createElement( QStringLiteral( "items" ) );
  for ( auto it = mItems.constBegin(); it != mItems.constEnd(); ++it )
  {
    QDomElement itemElement = doc.createElement( QStringLiteral( "item" ) );
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
    const QDomText layerOpacityText = doc.createTextNode( QString::number( opacity() ) );
    layerOpacityElem.appendChild( layerOpacityText );
    node.appendChild( layerOpacityElem );
  }

  if ( categories.testFlag( Symbology ) )
  {
    // add the blend mode field
    QDomElement blendModeElem  = doc.createElement( QStringLiteral( "blendMode" ) );
    const QDomText blendModeText = doc.createTextNode( QString::number( QgsPainting::getBlendModeEnum( blendMode() ) ) );
    blendModeElem.appendChild( blendModeText );
    node.appendChild( blendModeElem );
  }

  return true;
}

bool QgsAnnotationLayer::readSymbology( const QDomNode &node, QString &, QgsReadWriteContext &, QgsMapLayer::StyleCategories categories )
{
  if ( categories.testFlag( Rendering ) )
  {
    const QDomNode layerOpacityNode = node.namedItem( QStringLiteral( "layerOpacity" ) );
    if ( !layerOpacityNode.isNull() )
    {
      const QDomElement e = layerOpacityNode.toElement();
      setOpacity( e.text().toDouble() );
    }
  }

  if ( categories.testFlag( Symbology ) )
  {
    // get and set the blend mode if it exists
    const QDomNode blendModeNode = node.namedItem( QStringLiteral( "blendMode" ) );
    if ( !blendModeNode.isNull() )
    {
      const QDomElement e = blendModeNode.toElement();
      setBlendMode( QgsPainting::getCompositionMode( static_cast< QgsPainting::BlendMode >( e.text().toInt() ) ) );
    }
  }

  return true;
}

bool QgsAnnotationLayer::isEditable() const
{
  // annotation layers are always editable
  return true;
}

bool QgsAnnotationLayer::supportsEditing() const
{
  return true;
}
