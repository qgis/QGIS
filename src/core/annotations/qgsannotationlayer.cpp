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
#include "qgsannotationitemeditoperation.h"
#include "qgspainteffect.h"
#include "qgseffectstack.h"
#include "qgspainteffectregistry.h"
#include <QUuid>
#include "RTree.h"

///@cond PRIVATE
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
///@endcond

QgsAnnotationLayer::QgsAnnotationLayer( const QString &name, const LayerOptions &options )
  : QgsMapLayer( QgsMapLayerType::AnnotationLayer, name )
  , mTransformContext( options.transformContext )
  , mSpatialIndex( std::make_unique< QgsAnnotationLayerSpatialIndex >() )
{
  mShouldValidateCrs = false;
  mValid = true;

  QgsDataProvider::ProviderOptions providerOptions;
  providerOptions.transformContext = options.transformContext;
  mDataProvider = new QgsAnnotationLayerDataProvider( providerOptions, QgsDataProvider::ReadFlags() );

  mPaintEffect.reset( QgsPaintEffectRegistry::defaultStack() );
  mPaintEffect->setEnabled( false );
}

QgsAnnotationLayer::~QgsAnnotationLayer()
{
  emit willBeDeleted();
  qDeleteAll( mItems );
  delete mDataProvider;
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
  if ( item->flags() & Qgis::AnnotationItemFlag::ScaleDependentBoundingBox )
    mNonIndexedItems.insert( uuid );
  else
    mSpatialIndex->insert( uuid, item->boundingBox() );

  triggerRepaint();

  return uuid;
}

void QgsAnnotationLayer::replaceItem( const QString &id, QgsAnnotationItem *item )
{
  std::unique_ptr< QgsAnnotationItem> prevItem( mItems.take( id ) );

  if ( prevItem )
  {
    auto it = mNonIndexedItems.find( id );
    if ( it == mNonIndexedItems.end() )
    {
      mSpatialIndex->remove( id, prevItem->boundingBox() );
    }
    else
    {
      mNonIndexedItems.erase( it );
    }
  }

  mItems.insert( id, item );
  if ( item->flags() & Qgis::AnnotationItemFlag::ScaleDependentBoundingBox )
    mNonIndexedItems.insert( id );
  else
    mSpatialIndex->insert( id, item->boundingBox() );

  triggerRepaint();
}

bool QgsAnnotationLayer::removeItem( const QString &id )
{
  if ( !mItems.contains( id ) )
    return false;

  std::unique_ptr< QgsAnnotationItem> item( mItems.take( id ) );

  auto it = mNonIndexedItems.find( id );
  if ( it == mNonIndexedItems.end() )
  {
    mSpatialIndex->remove( id, item->boundingBox() );
  }
  else
  {
    mNonIndexedItems.erase( it );
  }

  item.reset();

  triggerRepaint();

  return true;
}

void QgsAnnotationLayer::clear()
{
  qDeleteAll( mItems );
  mItems.clear();
  mSpatialIndex = std::make_unique< QgsAnnotationLayerSpatialIndex >();
  mNonIndexedItems.clear();

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


QStringList QgsAnnotationLayer::queryIndex( const QgsRectangle &bounds, QgsFeedback *feedback ) const
{
  QStringList res;

  mSpatialIndex->intersects( bounds, [&res, feedback]( const QString & uuid )->bool
  {
    res << uuid;
    return !feedback || !feedback->isCanceled();
  } );
  return res;
}

QStringList QgsAnnotationLayer::itemsInBounds( const QgsRectangle &bounds, QgsRenderContext &context, QgsFeedback *feedback ) const
{
  QStringList res = queryIndex( bounds, feedback );
  // we also have to search through any non-indexed items
  for ( const QString &uuid : mNonIndexedItems )
  {
    if ( mItems.value( uuid )->boundingBox( context ).intersects( bounds ) )
      res << uuid;
  }

  return res;
}

Qgis::AnnotationItemEditOperationResult QgsAnnotationLayer::applyEdit( QgsAbstractAnnotationItemEditOperation *operation )
{
  Qgis::AnnotationItemEditOperationResult res = Qgis::AnnotationItemEditOperationResult::Invalid;
  if ( QgsAnnotationItem *targetItem = item( operation->itemId() ) )
  {
    // remove item from index if present
    auto it = mNonIndexedItems.find( operation->itemId() );
    if ( it == mNonIndexedItems.end() )
    {
      mSpatialIndex->remove( operation->itemId(), targetItem->boundingBox() );
    }
    res = targetItem->applyEdit( operation );

    switch ( res )
    {
      case Qgis::AnnotationItemEditOperationResult::Success:
      case Qgis::AnnotationItemEditOperationResult::Invalid:
        // re-add to index if possible
        if ( !( targetItem->flags() & Qgis::AnnotationItemFlag::ScaleDependentBoundingBox ) )
          mSpatialIndex->insert( operation->itemId(), targetItem->boundingBox() );
        break;

      case Qgis::AnnotationItemEditOperationResult::ItemCleared:
        // item needs removing from layer
        delete mItems.take( operation->itemId() );
        mNonIndexedItems.remove( operation->itemId() );
        break;
    }
  }

  if ( res != Qgis::AnnotationItemEditOperationResult::Invalid )
    triggerRepaint();

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
    if ( ( *it )->flags() & Qgis::AnnotationItemFlag::ScaleDependentBoundingBox )
      layer->mNonIndexedItems.insert( it.key() );
    else
      layer->mSpatialIndex->insert( it.key(), ( *it )->boundingBox() );
  }

  if ( mPaintEffect )
    layer->setPaintEffect( mPaintEffect->clone() );

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
  if ( mDataProvider )
    mDataProvider->setTransformContext( context );

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
  mNonIndexedItems.clear();

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
      if ( item->flags() & Qgis::AnnotationItemFlag::ScaleDependentBoundingBox )
        mNonIndexedItems.insert( id );
      else
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

    QDomElement paintEffectElem  = doc.createElement( QStringLiteral( "paintEffect" ) );
    if ( mPaintEffect && !QgsPaintEffectRegistry::isDefaultStack( mPaintEffect.get() ) )
      mPaintEffect->saveProperties( doc, paintEffectElem );
    node.appendChild( paintEffectElem );
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

    //restore layer effect
    const QDomNode paintEffectNode = node.namedItem( QStringLiteral( "paintEffect" ) );
    if ( !paintEffectNode.isNull() )
    {
      const QDomElement effectElem = paintEffectNode.firstChildElement( QStringLiteral( "effect" ) );
      if ( !effectElem.isNull() )
      {
        setPaintEffect( QgsApplication::paintEffectRegistry()->createEffect( effectElem ) );
      }
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

QgsDataProvider *QgsAnnotationLayer::dataProvider()
{
  return mDataProvider;
}

const QgsDataProvider *QgsAnnotationLayer::dataProvider() const
{
  return mDataProvider;
}

QString QgsAnnotationLayer::htmlMetadata() const
{
  QString metadata = QStringLiteral( "<html>\n<body>\n<h1>" ) + tr( "General" ) + QStringLiteral( "</h1>\n<hr>\n" ) + QStringLiteral( "<table class=\"list-view\">\n" );

  metadata += QStringLiteral( "<tr><td class=\"highlight\">" ) + tr( "Name" ) + QStringLiteral( "</td><td>" ) + name() + QStringLiteral( "</td></tr>\n" );

  // Extent
  metadata += QStringLiteral( "<tr><td class=\"highlight\">" ) + tr( "Extent" ) + QStringLiteral( "</td><td>" ) + extent().toString() + QStringLiteral( "</td></tr>\n" );

  // item count
  QLocale locale = QLocale();
  locale.setNumberOptions( locale.numberOptions() &= ~QLocale::NumberOption::OmitGroupSeparator );
  const int itemCount = mItems.size();
  metadata += QStringLiteral( "<tr><td class=\"highlight\">" )
              + tr( "Item count" ) + QStringLiteral( "</td><td>" )
              + locale.toString( static_cast<qlonglong>( itemCount ) )
              + QStringLiteral( "</td></tr>\n" );
  metadata += QLatin1String( "</table>\n<br><br>" );

  // CRS
  metadata += crsHtmlMetadata();

  // items section
  metadata += QStringLiteral( "<h1>" ) + tr( "Items" ) + QStringLiteral( "</h1>\n<hr>\n" );

  metadata += QLatin1String( "<table width=\"100%\" class=\"tabular-view\">\n" );
  metadata += QLatin1String( "<tr><th>" ) + tr( "Type" ) + QLatin1String( "</th><th>" ) + tr( "Count" ) + QLatin1String( "</th></tr>\n" );

  QMap< QString, int > itemCounts;
  for ( auto it = mItems.constBegin(); it != mItems.constEnd(); ++it )
  {
    itemCounts[ it.value()->type() ]++;
  }

  const QMap<QString, QString> itemTypes = QgsApplication::annotationItemRegistry()->itemTypes();
  int i = 0;
  for ( auto it = itemTypes.begin(); it != itemTypes.end(); ++it )
  {
    QString rowClass;
    if ( i % 2 )
      rowClass = QStringLiteral( "class=\"odd-row\"" );
    metadata += QLatin1String( "<tr " ) + rowClass + QLatin1String( "><td>" ) + it.value() + QLatin1String( "</td><td>" ) + locale.toString( static_cast<qlonglong>( itemCounts.value( it.key() ) ) ) + QLatin1String( "</td></tr>\n" );
    i++;
  }

  metadata += QLatin1String( "</table>\n<br><br>" );

  metadata += QLatin1String( "\n</body>\n</html>\n" );
  return metadata;
}

QgsPaintEffect *QgsAnnotationLayer::paintEffect() const
{
  return mPaintEffect.get();
}

void QgsAnnotationLayer::setPaintEffect( QgsPaintEffect *effect )
{
  mPaintEffect.reset( effect );
}


//
// QgsAnnotationLayerDataProvider
//
///@cond PRIVATE
QgsAnnotationLayerDataProvider::QgsAnnotationLayerDataProvider(
  const ProviderOptions &options,
  QgsDataProvider::ReadFlags flags )
  : QgsDataProvider( QString(), options, flags )
{}

QgsCoordinateReferenceSystem QgsAnnotationLayerDataProvider::crs() const
{
  return QgsCoordinateReferenceSystem();
}

QString QgsAnnotationLayerDataProvider::name() const
{
  return QStringLiteral( "annotation" );
}

QString QgsAnnotationLayerDataProvider::description() const
{
  return QString();
}

QgsRectangle QgsAnnotationLayerDataProvider::extent() const
{
  return QgsRectangle();
}

bool QgsAnnotationLayerDataProvider::isValid() const
{
  return true;
}
///@endcond
