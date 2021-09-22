/***************************************************************************
                              qgslayoutitem.cpp
                             -------------------
    begin                : June 2017
    copyright            : (C) 2017 by Nyall Dawson
    email                : nyall dot dawson at gmail dot com
 ***************************************************************************/
/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "qgslayoutitem.h"
#include "qgslayout.h"
#include "qgslayoututils.h"
#include "qgspagesizeregistry.h"
#include "qgslayoutitemundocommand.h"
#include "qgslayoutmodel.h"
#include "qgssymbollayerutils.h"
#include "qgslayoutitemgroup.h"
#include "qgspainting.h"
#include "qgslayouteffect.h"
#include "qgslayoutundostack.h"
#include "qgslayoutpagecollection.h"
#include "qgslayoutitempage.h"
#include "qgsimageoperation.h"
#include "qgsexpressioncontextutils.h"

#include <QPainter>
#include <QStyleOptionGraphicsItem>
#include <QUuid>

#define CACHE_SIZE_LIMIT 5000

QgsLayoutItemRenderContext::QgsLayoutItemRenderContext( QgsRenderContext &context, double viewScaleFactor )
  : mRenderContext( context )
  , mViewScaleFactor( viewScaleFactor )
{
}



QgsLayoutItem::QgsLayoutItem( QgsLayout *layout, bool manageZValue )
  : QgsLayoutObject( layout )
  , QGraphicsRectItem( nullptr )
  , mUuid( QUuid::createUuid().toString() )
{
  setZValue( QgsLayout::ZItem );

  // needed to access current view transform during paint operations
  setFlags( flags() | QGraphicsItem::ItemUsesExtendedStyleOption | QGraphicsItem::ItemIsSelectable );

  setCacheMode( QGraphicsItem::DeviceCoordinateCache );

  //record initial position
  const QgsUnitTypes::LayoutUnit initialUnits = layout ? layout->units() : QgsUnitTypes::LayoutMillimeters;
  mItemPosition = QgsLayoutPoint( scenePos().x(), scenePos().y(), initialUnits );
  mItemSize = QgsLayoutSize( rect().width(), rect().height(), initialUnits );

  // required to initially setup background/frame style
  refreshBackgroundColor( false );
  refreshFrame( false );

  initConnectionsToLayout();

  //let z-Value be managed by layout
  if ( mLayout && manageZValue )
  {
    mLayoutManagesZValue = true;
    mLayout->itemsModel()->addItemAtTop( this );
  }
  else
  {
    mLayoutManagesZValue = false;
  }

  // Setup layout effect
  mEffect.reset( new QgsLayoutEffect() );
  if ( mLayout )
  {
    mEffect->setEnabled( mLayout->renderContext().flags() & QgsLayoutRenderContext::FlagUseAdvancedEffects );
    connect( &mLayout->renderContext(), &QgsLayoutRenderContext::flagsChanged, this, [ = ]( QgsLayoutRenderContext::Flags flags )
    {
      mEffect->setEnabled( flags & QgsLayoutRenderContext::FlagUseAdvancedEffects );
    } );
  }
  setGraphicsEffect( mEffect.get() );
}

QgsLayoutItem::~QgsLayoutItem()
{
  cleanup();
}

void QgsLayoutItem::cleanup()
{
  if ( mLayout && mLayoutManagesZValue )
  {
    mLayout->itemsModel()->removeItem( this );
  }
}

QString QgsLayoutItem::displayName() const
{
  //return id, if it's not empty
  if ( !id().isEmpty() )
  {
    return id();
  }

  //for unnamed items, default to item type
  if ( QgsLayoutItemAbstractMetadata *metadata = QgsApplication::layoutItemRegistry()->itemMetadata( type() ) )
  {
    return tr( "<%1>" ).arg( metadata->visibleName() );
  }

  return tr( "<item>" );
}

int QgsLayoutItem::type() const
{
  return QgsLayoutItemRegistry::LayoutItem;
}

QgsLayoutItem::Flags QgsLayoutItem::itemFlags() const
{
  return QgsLayoutItem::Flags();
}

void QgsLayoutItem::setId( const QString &id )
{
  if ( id == mId )
  {
    return;
  }

  if ( !shouldBlockUndoCommands() )
    mLayout->undoStack()->beginCommand( this, tr( "Change Item ID" ) );

  mId = id;

  if ( !shouldBlockUndoCommands() )
    mLayout->undoStack()->endCommand();

  setToolTip( id );

  //inform model that id data has changed
  if ( mLayout )
  {
    mLayout->itemsModel()->updateItemDisplayName( this );
  }

  emit changed();
}

void QgsLayoutItem::setSelected( bool selected )
{
  QGraphicsRectItem::setSelected( selected );
  //inform model that id data has changed
  if ( mLayout )
  {
    mLayout->itemsModel()->updateItemSelectStatus( this );
  }
}

void QgsLayoutItem::setVisibility( const bool visible )
{
  if ( visible == isVisible() )
  {
    //nothing to do
    return;
  }

  std::unique_ptr< QgsAbstractLayoutUndoCommand > command;
  if ( !shouldBlockUndoCommands() )
  {
    command.reset( createCommand( visible ? tr( "Show Item" ) : tr( "Hide Item" ), 0 ) );
    command->saveBeforeState();
  }

  QGraphicsItem::setVisible( visible );

  if ( command )
  {
    command->saveAfterState();
    mLayout->undoStack()->push( command.release() );
  }

  //inform model that visibility has changed
  if ( mLayout )
  {
    mLayout->itemsModel()->updateItemVisibility( this );
  }
}

void QgsLayoutItem::setLocked( const bool locked )
{
  if ( locked == mIsLocked )
  {
    return;
  }

  if ( !shouldBlockUndoCommands() )
    mLayout->undoStack()->beginCommand( this, locked ? tr( "Lock Item" ) : tr( "Unlock Item" ) );

  mIsLocked = locked;

  if ( !shouldBlockUndoCommands() )
    mLayout->undoStack()->endCommand();

  //inform model that id data has changed
  if ( mLayout )
  {
    mLayout->itemsModel()->updateItemLockStatus( this );
  }

  update();
  emit lockChanged();
}

bool QgsLayoutItem::isGroupMember() const
{
  return !mParentGroupUuid.isEmpty() && mLayout && static_cast< bool >( mLayout->itemByUuid( mParentGroupUuid ) );
}

QgsLayoutItemGroup *QgsLayoutItem::parentGroup() const
{
  if ( !mLayout || mParentGroupUuid.isEmpty() )
    return nullptr;

  return qobject_cast< QgsLayoutItemGroup * >( mLayout->itemByUuid( mParentGroupUuid ) );
}

void QgsLayoutItem::setParentGroup( QgsLayoutItemGroup *group )
{
  if ( !group )
    mParentGroupUuid.clear();
  else
    mParentGroupUuid = group->uuid();
  setFlag( QGraphicsItem::ItemIsSelectable, !static_cast< bool>( group ) ); //item in groups cannot be selected
}

QgsLayoutItem::ExportLayerBehavior QgsLayoutItem::exportLayerBehavior() const
{
  return CanGroupWithAnyOtherItem;
}

int QgsLayoutItem::numberExportLayers() const
{
  return 0;
}

void QgsLayoutItem::startLayeredExport()
{

}

void QgsLayoutItem::stopLayeredExport()
{

}

bool QgsLayoutItem::nextExportPart()
{
  Q_NOWARN_DEPRECATED_PUSH
  if ( !mLayout || mLayout->renderContext().currentExportLayer() == -1 )
    return false;

  // QGIS 4- return false from base class implementation

  const int layers = numberExportLayers();
  return mLayout->renderContext().currentExportLayer() < layers;
  Q_NOWARN_DEPRECATED_POP
}

QgsLayoutItem::ExportLayerDetail QgsLayoutItem::exportLayerDetails() const
{
  return QgsLayoutItem::ExportLayerDetail();
}

void QgsLayoutItem::paint( QPainter *painter, const QStyleOptionGraphicsItem *itemStyle, QWidget * )
{
  if ( !painter || !painter->device() || !shouldDrawItem() )
  {
    return;
  }

  //TODO - remember to disable saving/restoring on graphics view!!

  if ( shouldDrawDebugRect() )
  {
    drawDebugRect( painter );
    return;
  }

  const bool previewRender = !mLayout || mLayout->renderContext().isPreviewRender();
  double destinationDpi = previewRender ? QgsLayoutUtils::scaleFactorFromItemStyle( itemStyle, painter ) * 25.4 : mLayout->renderContext().dpi();
  const bool useImageCache = false;
  const bool forceRasterOutput = containsAdvancedEffects() && ( !mLayout || !( mLayout->renderContext().flags() & QgsLayoutRenderContext::FlagForceVectorOutput ) );

  if ( useImageCache || forceRasterOutput )
  {
    double widthInPixels = 0;
    double heightInPixels = 0;

    if ( previewRender )
    {
      widthInPixels = boundingRect().width() * QgsLayoutUtils::scaleFactorFromItemStyle( itemStyle, painter );
      heightInPixels = boundingRect().height() * QgsLayoutUtils::scaleFactorFromItemStyle( itemStyle, painter );
    }
    else
    {
      const double layoutUnitsToPixels = mLayout ? mLayout->convertFromLayoutUnits( 1, QgsUnitTypes::LayoutPixels ).length() : destinationDpi / 25.4;
      widthInPixels = boundingRect().width() * layoutUnitsToPixels;
      heightInPixels = boundingRect().height() * layoutUnitsToPixels;
    }

    // limit size of image for better performance
    if ( previewRender && ( widthInPixels > CACHE_SIZE_LIMIT || heightInPixels > CACHE_SIZE_LIMIT ) )
    {
      double scale = 1.0;
      if ( widthInPixels > heightInPixels )
      {
        scale = widthInPixels / CACHE_SIZE_LIMIT;
        widthInPixels = CACHE_SIZE_LIMIT;
        heightInPixels /= scale;
      }
      else
      {
        scale = heightInPixels / CACHE_SIZE_LIMIT;
        heightInPixels = CACHE_SIZE_LIMIT;
        widthInPixels /= scale;
      }
      destinationDpi = destinationDpi / scale;
    }

    if ( previewRender && !mItemCachedImage.isNull() && qgsDoubleNear( mItemCacheDpi, destinationDpi ) )
    {
      // can reuse last cached image
      const QgsRenderContext context = QgsLayoutUtils::createRenderContextForLayout( mLayout, painter, destinationDpi );
      const QgsScopedQPainterState painterState( painter );
      preparePainter( painter );
      const double cacheScale = destinationDpi / mItemCacheDpi;
      painter->scale( cacheScale / context.scaleFactor(), cacheScale / context.scaleFactor() );
      painter->drawImage( boundingRect().x() * context.scaleFactor() / cacheScale,
                          boundingRect().y() * context.scaleFactor() / cacheScale, mItemCachedImage );
      return;
    }
    else
    {
      QImage image = QImage( widthInPixels, heightInPixels, QImage::Format_ARGB32 );
      image.fill( Qt::transparent );
      image.setDotsPerMeterX( 1000 * destinationDpi * 25.4 );
      image.setDotsPerMeterY( 1000 * destinationDpi * 25.4 );
      QPainter p( &image );

      preparePainter( &p );
      QgsRenderContext context = QgsLayoutUtils::createRenderContextForLayout( mLayout, &p, destinationDpi );
      context.setExpressionContext( createExpressionContext() );
      // painter is already scaled to dots
      // need to translate so that item origin is at 0,0 in painter coordinates (not bounding rect origin)
      p.translate( -boundingRect().x() * context.scaleFactor(), -boundingRect().y() * context.scaleFactor() );
      // scale to layout units for background and frame rendering
      p.scale( context.scaleFactor(), context.scaleFactor() );
      drawBackground( context );
      p.scale( 1 / context.scaleFactor(), 1 / context.scaleFactor() );
      const double viewScale = QgsLayoutUtils::scaleFactorFromItemStyle( itemStyle, painter );
      QgsLayoutItemRenderContext itemRenderContext( context, viewScale );
      draw( itemRenderContext );
      p.scale( context.scaleFactor(), context.scaleFactor() );
      drawFrame( context );
      p.scale( 1 / context.scaleFactor(), 1 / context.scaleFactor() );
      p.end();

      QgsImageOperation::multiplyOpacity( image, mEvaluatedOpacity );

      const QgsScopedQPainterState painterState( painter );
      // scale painter from mm to dots
      painter->scale( 1.0 / context.scaleFactor(), 1.0 / context.scaleFactor() );
      painter->drawImage( boundingRect().x() * context.scaleFactor(),
                          boundingRect().y() * context.scaleFactor(), image );

      if ( previewRender )
      {
        mItemCacheDpi = destinationDpi;
        mItemCachedImage = image;
      }
    }
  }
  else
  {
    // no caching or flattening
    const QgsScopedQPainterState painterState( painter );
    preparePainter( painter );
    QgsRenderContext context = QgsLayoutUtils::createRenderContextForLayout( mLayout, painter, destinationDpi );
    context.setExpressionContext( createExpressionContext() );
    drawBackground( context );

    const double viewScale = QgsLayoutUtils::scaleFactorFromItemStyle( itemStyle, painter );

    // scale painter from mm to dots
    painter->scale( 1.0 / context.scaleFactor(), 1.0 / context.scaleFactor() );
    QgsLayoutItemRenderContext itemRenderContext( context, viewScale );
    draw( itemRenderContext );

    painter->scale( context.scaleFactor(), context.scaleFactor() );
    drawFrame( context );
  }
}

void QgsLayoutItem::setReferencePoint( const QgsLayoutItem::ReferencePoint point )
{
  if ( point == mReferencePoint )
  {
    return;
  }

  mReferencePoint = point;

  //also need to adjust stored position
  updateStoredItemPosition();
  refreshItemPosition();
}

void QgsLayoutItem::attemptResize( const QgsLayoutSize &s, bool includesFrame )
{
  if ( !mLayout )
  {
    mItemSize = s;
    setRect( 0, 0, s.width(), s.height() );
    return;
  }

  QgsLayoutSize size = s;

  if ( includesFrame )
  {
    //adjust position to account for frame size
    const double bleed = mLayout->convertFromLayoutUnits( estimatedFrameBleed(), size.units() ).length();
    size.setWidth( size.width() - 2 * bleed );
    size.setHeight( size.height() - 2 * bleed );
  }

  const QgsLayoutSize evaluatedSize = applyDataDefinedSize( size );
  const QSizeF targetSizeLayoutUnits = mLayout->convertToLayoutUnits( evaluatedSize );
  QSizeF actualSizeLayoutUnits = applyMinimumSize( targetSizeLayoutUnits );
  actualSizeLayoutUnits = applyFixedSize( actualSizeLayoutUnits );
  actualSizeLayoutUnits = applyItemSizeConstraint( actualSizeLayoutUnits );

  if ( actualSizeLayoutUnits == rect().size() )
  {
    return;
  }

  const QgsLayoutSize actualSizeTargetUnits = mLayout->convertFromLayoutUnits( actualSizeLayoutUnits, size.units() );
  mItemSize = actualSizeTargetUnits;

  setRect( 0, 0, actualSizeLayoutUnits.width(), actualSizeLayoutUnits.height() );
  refreshItemPosition();
  emit sizePositionChanged();
}

void QgsLayoutItem::attemptMove( const QgsLayoutPoint &p, bool useReferencePoint, bool includesFrame, int page )
{
  if ( !mLayout )
  {
    mItemPosition = p;
    setPos( p.toQPointF() );
    return;
  }

  QgsLayoutPoint point = p;
  if ( page >= 0 )
  {
    point = mLayout->pageCollection()->pagePositionToAbsolute( page, p );
  }

  if ( includesFrame )
  {
    //adjust position to account for frame size
    const double bleed = mLayout->convertFromLayoutUnits( estimatedFrameBleed(), point.units() ).length();
    point.setX( point.x() + bleed );
    point.setY( point.y() + bleed );
  }

  QgsLayoutPoint evaluatedPoint = point;
  if ( !useReferencePoint )
  {
    evaluatedPoint = topLeftToReferencePoint( point );
  }

  evaluatedPoint = applyDataDefinedPosition( evaluatedPoint );
  const QPointF evaluatedPointLayoutUnits = mLayout->convertToLayoutUnits( evaluatedPoint );
  const QPointF topLeftPointLayoutUnits = adjustPointForReferencePosition( evaluatedPointLayoutUnits, rect().size(), mReferencePoint );
  if ( topLeftPointLayoutUnits == scenePos() && point.units() == mItemPosition.units() )
  {
    //TODO - add test for second condition
    return;
  }

  const QgsLayoutPoint referencePointTargetUnits = mLayout->convertFromLayoutUnits( evaluatedPointLayoutUnits, point.units() );
  mItemPosition = referencePointTargetUnits;
  setScenePos( topLeftPointLayoutUnits );
  emit sizePositionChanged();
}

void QgsLayoutItem::attemptSetSceneRect( const QRectF &rect, bool includesFrame )
{
  const QPointF newPos = rect.topLeft();

  blockSignals( true );
  // translate new size to current item units
  const QgsLayoutSize newSize = mLayout->convertFromLayoutUnits( rect.size(), mItemSize.units() );
  attemptResize( newSize, includesFrame );

  // translate new position to current item units
  const QgsLayoutPoint itemPos = mLayout->convertFromLayoutUnits( newPos, mItemPosition.units() );
  attemptMove( itemPos, false, includesFrame );
  blockSignals( false );
  emit sizePositionChanged();
}

void QgsLayoutItem::attemptMoveBy( double deltaX, double deltaY )
{
  if ( !mLayout )
  {
    moveBy( deltaX, deltaY );
    return;
  }

  QgsLayoutPoint itemPos = positionWithUnits();
  const QgsLayoutPoint deltaPos = mLayout->convertFromLayoutUnits( QPointF( deltaX, deltaY ), itemPos.units() );
  itemPos.setX( itemPos.x() + deltaPos.x() );
  itemPos.setY( itemPos.y() + deltaPos.y() );
  attemptMove( itemPos );
}

int QgsLayoutItem::page() const
{
  if ( !mLayout )
    return -1;

  return mLayout->pageCollection()->pageNumberForPoint( pos() );
}

QPointF QgsLayoutItem::pagePos() const
{
  QPointF p = positionAtReferencePoint( mReferencePoint );

  if ( !mLayout )
    return p;

  // try to get page
  QgsLayoutItemPage *pageItem = mLayout->pageCollection()->page( page() );
  if ( !pageItem )
    return p;

  p.ry() -= pageItem->pos().y();
  return p;
}

QgsLayoutPoint QgsLayoutItem::pagePositionWithUnits() const
{
  const QPointF p = pagePos();
  if ( !mLayout )
    return QgsLayoutPoint( p );

  return mLayout->convertFromLayoutUnits( p, mItemPosition.units() );
}

void QgsLayoutItem::setScenePos( const QPointF destinationPos )
{
  //since setPos does not account for item rotation, use difference between
  //current scenePos (which DOES account for rotation) and destination pos
  //to calculate how much the item needs to move
  if ( auto *lParentItem = parentItem() )
    setPos( pos() + ( destinationPos - scenePos() ) + lParentItem->scenePos() );
  else
    setPos( pos() + ( destinationPos - scenePos() ) );
}

bool QgsLayoutItem::shouldBlockUndoCommands() const
{
  return !mLayout || mLayout != scene() || mBlockUndoCommands;
}

bool QgsLayoutItem::shouldDrawItem() const
{
  if ( mLayout && QgsLayoutUtils::itemIsAClippingSource( this ) )
    return false;

  if ( !mLayout || mLayout->renderContext().isPreviewRender() )
  {
    //preview mode so OK to draw item
    return true;
  }

  //exporting layout, so check if item is excluded from exports
  return !mEvaluatedExcludeFromExports;
}

double QgsLayoutItem::itemRotation() const
{
  return mItemRotation;
}

bool QgsLayoutItem::writeXml( QDomElement &parentElement, QDomDocument &doc, const QgsReadWriteContext &context ) const
{
  QDomElement element = doc.createElement( QStringLiteral( "LayoutItem" ) );
  element.setAttribute( QStringLiteral( "type" ), QString::number( type() ) );

  element.setAttribute( QStringLiteral( "uuid" ), mUuid );
  element.setAttribute( QStringLiteral( "templateUuid" ), mUuid );
  element.setAttribute( QStringLiteral( "id" ), mId );
  element.setAttribute( QStringLiteral( "referencePoint" ), QString::number( static_cast< int >( mReferencePoint ) ) );
  element.setAttribute( QStringLiteral( "position" ), mItemPosition.encodePoint() );
  element.setAttribute( QStringLiteral( "positionOnPage" ), pagePositionWithUnits().encodePoint() );
  element.setAttribute( QStringLiteral( "size" ), mItemSize.encodeSize() );
  element.setAttribute( QStringLiteral( "itemRotation" ), QString::number( mItemRotation ) );
  element.setAttribute( QStringLiteral( "groupUuid" ), mParentGroupUuid );

  element.setAttribute( QStringLiteral( "zValue" ), QString::number( zValue() ) );
  element.setAttribute( QStringLiteral( "visibility" ), isVisible() );
  //position lock for mouse moves/resizes
  if ( mIsLocked )
  {
    element.setAttribute( QStringLiteral( "positionLock" ), QStringLiteral( "true" ) );
  }
  else
  {
    element.setAttribute( QStringLiteral( "positionLock" ), QStringLiteral( "false" ) );
  }

  //frame
  if ( mFrame )
  {
    element.setAttribute( QStringLiteral( "frame" ), QStringLiteral( "true" ) );
  }
  else
  {
    element.setAttribute( QStringLiteral( "frame" ), QStringLiteral( "false" ) );
  }

  //background
  if ( mBackground )
  {
    element.setAttribute( QStringLiteral( "background" ), QStringLiteral( "true" ) );
  }
  else
  {
    element.setAttribute( QStringLiteral( "background" ), QStringLiteral( "false" ) );
  }

  //frame color
  QDomElement frameColorElem = doc.createElement( QStringLiteral( "FrameColor" ) );
  frameColorElem.setAttribute( QStringLiteral( "red" ), QString::number( mFrameColor.red() ) );
  frameColorElem.setAttribute( QStringLiteral( "green" ), QString::number( mFrameColor.green() ) );
  frameColorElem.setAttribute( QStringLiteral( "blue" ), QString::number( mFrameColor.blue() ) );
  frameColorElem.setAttribute( QStringLiteral( "alpha" ), QString::number( mFrameColor.alpha() ) );
  element.appendChild( frameColorElem );
  element.setAttribute( QStringLiteral( "outlineWidthM" ), mFrameWidth.encodeMeasurement() );
  element.setAttribute( QStringLiteral( "frameJoinStyle" ), QgsSymbolLayerUtils::encodePenJoinStyle( mFrameJoinStyle ) );

  //background color
  QDomElement bgColorElem = doc.createElement( QStringLiteral( "BackgroundColor" ) );
  bgColorElem.setAttribute( QStringLiteral( "red" ), QString::number( mBackgroundColor.red() ) );
  bgColorElem.setAttribute( QStringLiteral( "green" ), QString::number( mBackgroundColor.green() ) );
  bgColorElem.setAttribute( QStringLiteral( "blue" ), QString::number( mBackgroundColor.blue() ) );
  bgColorElem.setAttribute( QStringLiteral( "alpha" ), QString::number( mBackgroundColor.alpha() ) );
  element.appendChild( bgColorElem );

  //blend mode
  element.setAttribute( QStringLiteral( "blendMode" ), QgsPainting::getBlendModeEnum( mBlendMode ) );

  //opacity
  element.setAttribute( QStringLiteral( "opacity" ), QString::number( mOpacity ) );

  element.setAttribute( QStringLiteral( "excludeFromExports" ), mExcludeFromExports );

  writeObjectPropertiesToElement( element, doc, context );

  writePropertiesToElement( element, doc, context );
  parentElement.appendChild( element );

  return true;
}

bool QgsLayoutItem::readXml( const QDomElement &element, const QDomDocument &doc, const QgsReadWriteContext &context )
{
  if ( element.nodeName() != QLatin1String( "LayoutItem" ) )
  {
    return false;
  }

  readObjectPropertiesFromElement( element, doc, context );

  mBlockUndoCommands = true;
  mUuid = element.attribute( QStringLiteral( "uuid" ), QUuid::createUuid().toString() );
  setId( element.attribute( QStringLiteral( "id" ) ) );
  mReferencePoint = static_cast< ReferencePoint >( element.attribute( QStringLiteral( "referencePoint" ) ).toInt() );
  setItemRotation( element.attribute( QStringLiteral( "itemRotation" ), QStringLiteral( "0" ) ).toDouble() );
  attemptMove( QgsLayoutPoint::decodePoint( element.attribute( QStringLiteral( "position" ) ) ) );
  attemptResize( QgsLayoutSize::decodeSize( element.attribute( QStringLiteral( "size" ) ) ) );

  mParentGroupUuid = element.attribute( QStringLiteral( "groupUuid" ) );
  if ( !mParentGroupUuid.isEmpty() )
  {
    if ( QgsLayoutItemGroup *group = parentGroup() )
    {
      group->addItem( this );
    }
  }
  mTemplateUuid = element.attribute( QStringLiteral( "templateUuid" ) );

  //position lock for mouse moves/resizes
  const QString positionLock = element.attribute( QStringLiteral( "positionLock" ) );
  if ( positionLock.compare( QLatin1String( "true" ), Qt::CaseInsensitive ) == 0 )
  {
    setLocked( true );
  }
  else
  {
    setLocked( false );
  }
  //visibility
  setVisibility( element.attribute( QStringLiteral( "visibility" ), QStringLiteral( "1" ) ) != QLatin1String( "0" ) );
  setZValue( element.attribute( QStringLiteral( "zValue" ) ).toDouble() );

  //frame
  const QString frame = element.attribute( QStringLiteral( "frame" ) );
  if ( frame.compare( QLatin1String( "true" ), Qt::CaseInsensitive ) == 0 )
  {
    mFrame = true;
  }
  else
  {
    mFrame = false;
  }

  //frame
  const QString background = element.attribute( QStringLiteral( "background" ) );
  if ( background.compare( QLatin1String( "true" ), Qt::CaseInsensitive ) == 0 )
  {
    mBackground = true;
  }
  else
  {
    mBackground = false;
  }

  //pen
  mFrameWidth = QgsLayoutMeasurement::decodeMeasurement( element.attribute( QStringLiteral( "outlineWidthM" ) ) );
  mFrameJoinStyle = QgsSymbolLayerUtils::decodePenJoinStyle( element.attribute( QStringLiteral( "frameJoinStyle" ), QStringLiteral( "miter" ) ) );
  const QDomNodeList frameColorList = element.elementsByTagName( QStringLiteral( "FrameColor" ) );
  if ( !frameColorList.isEmpty() )
  {
    const QDomElement frameColorElem = frameColorList.at( 0 ).toElement();
    bool redOk = false;
    bool greenOk = false;
    bool blueOk = false;
    bool alphaOk = false;
    int penRed, penGreen, penBlue, penAlpha;

    penRed = frameColorElem.attribute( QStringLiteral( "red" ) ).toDouble( &redOk );
    penGreen = frameColorElem.attribute( QStringLiteral( "green" ) ).toDouble( &greenOk );
    penBlue = frameColorElem.attribute( QStringLiteral( "blue" ) ).toDouble( &blueOk );
    penAlpha = frameColorElem.attribute( QStringLiteral( "alpha" ) ).toDouble( &alphaOk );

    if ( redOk && greenOk && blueOk && alphaOk )
    {
      mFrameColor = QColor( penRed, penGreen, penBlue, penAlpha );
    }
  }
  refreshFrame( false );

  //brush
  const QDomNodeList bgColorList = element.elementsByTagName( QStringLiteral( "BackgroundColor" ) );
  if ( !bgColorList.isEmpty() )
  {
    const QDomElement bgColorElem = bgColorList.at( 0 ).toElement();
    bool redOk, greenOk, blueOk, alphaOk;
    int bgRed, bgGreen, bgBlue, bgAlpha;
    bgRed = bgColorElem.attribute( QStringLiteral( "red" ) ).toDouble( &redOk );
    bgGreen = bgColorElem.attribute( QStringLiteral( "green" ) ).toDouble( &greenOk );
    bgBlue = bgColorElem.attribute( QStringLiteral( "blue" ) ).toDouble( &blueOk );
    bgAlpha = bgColorElem.attribute( QStringLiteral( "alpha" ) ).toDouble( &alphaOk );
    if ( redOk && greenOk && blueOk && alphaOk )
    {
      mBackgroundColor = QColor( bgRed, bgGreen, bgBlue, bgAlpha );
      setBrush( QBrush( mBackgroundColor, Qt::SolidPattern ) );
    }
    //apply any data defined settings
    refreshBackgroundColor( false );
  }

  //blend mode
  setBlendMode( QgsPainting::getCompositionMode( static_cast< QgsPainting::BlendMode >( element.attribute( QStringLiteral( "blendMode" ), QStringLiteral( "0" ) ).toUInt() ) ) );

  //opacity
  if ( element.hasAttribute( QStringLiteral( "opacity" ) ) )
  {
    setItemOpacity( element.attribute( QStringLiteral( "opacity" ), QStringLiteral( "1" ) ).toDouble() );
  }
  else
  {
    setItemOpacity( 1.0 - element.attribute( QStringLiteral( "transparency" ), QStringLiteral( "0" ) ).toInt() / 100.0 );
  }

  mExcludeFromExports = element.attribute( QStringLiteral( "excludeFromExports" ), QStringLiteral( "0" ) ).toInt();
  mEvaluatedExcludeFromExports = mExcludeFromExports;

  const bool result = readPropertiesFromElement( element, doc, context );

  mBlockUndoCommands = false;

  emit changed();
  update();
  return result;
}

void QgsLayoutItem::finalizeRestoreFromXml()
{
}

QgsAbstractLayoutUndoCommand *QgsLayoutItem::createCommand( const QString &text, int id, QUndoCommand *parent )
{
  return new QgsLayoutItemUndoCommand( this, text, id, parent );
}

void QgsLayoutItem::setFrameEnabled( bool drawFrame )
{
  if ( drawFrame == mFrame )
  {
    //no change
    return;
  }

  mFrame = drawFrame;
  refreshFrame( true );
  emit frameChanged();
}

void QgsLayoutItem::setFrameStrokeColor( const QColor &color )
{
  if ( mFrameColor == color )
  {
    //no change
    return;
  }
  mFrameColor = color;
  // apply any datadefined overrides
  refreshFrame( true );
  emit frameChanged();
}

void QgsLayoutItem::setFrameStrokeWidth( const QgsLayoutMeasurement width )
{
  if ( mFrameWidth == width )
  {
    //no change
    return;
  }
  mFrameWidth = width;
  refreshFrame();
  emit frameChanged();
}

void QgsLayoutItem::setFrameJoinStyle( const Qt::PenJoinStyle style )
{
  if ( mFrameJoinStyle == style )
  {
    //no change
    return;
  }
  mFrameJoinStyle = style;

  QPen itemPen = pen();
  itemPen.setJoinStyle( mFrameJoinStyle );
  setPen( itemPen );
  emit frameChanged();
}

void QgsLayoutItem::setBackgroundEnabled( bool drawBackground )
{
  mBackground = drawBackground;
  update();
}

void QgsLayoutItem::setBackgroundColor( const QColor &color )
{
  mBackgroundColor = color;
  // apply any datadefined overrides
  refreshBackgroundColor( true );
}

void QgsLayoutItem::setBlendMode( const QPainter::CompositionMode mode )
{
  mBlendMode = mode;
  // Update the item effect to use the new blend mode
  refreshBlendMode();
}

void QgsLayoutItem::setItemOpacity( double opacity )
{
  mOpacity = opacity;
  refreshOpacity( mItemCachedImage.isNull() );
  if ( !mItemCachedImage.isNull() )
    invalidateCache();
}

bool QgsLayoutItem::excludeFromExports() const
{
  return mExcludeFromExports;
}

void QgsLayoutItem::setExcludeFromExports( bool exclude )
{
  mExcludeFromExports = exclude;
  refreshDataDefinedProperty( QgsLayoutObject::ExcludeFromExports );
}

bool QgsLayoutItem::containsAdvancedEffects() const
{
  return itemFlags() & Flag::FlagOverridesPaint ? false : mEvaluatedOpacity < 1.0;
}

bool QgsLayoutItem::requiresRasterization() const
{
  return ( itemFlags() & Flag::FlagOverridesPaint && itemOpacity() < 1.0 ) ||
         blendMode() != QPainter::CompositionMode_SourceOver;
}

double QgsLayoutItem::estimatedFrameBleed() const
{
  if ( !frameEnabled() )
  {
    return 0;
  }

  return pen().widthF() / 2.0;
}

QRectF QgsLayoutItem::rectWithFrame() const
{
  const double frameBleed = estimatedFrameBleed();
  return rect().adjusted( -frameBleed, -frameBleed, frameBleed, frameBleed );
}

void QgsLayoutItem::moveContent( double, double )
{

}

void QgsLayoutItem::setMoveContentPreviewOffset( double, double )
{

}

void QgsLayoutItem::zoomContent( double, QPointF )
{

}

void QgsLayoutItem::beginCommand( const QString &commandText, UndoCommand command )
{
  if ( !mLayout )
    return;

  mLayout->undoStack()->beginCommand( this, commandText, command );
}

void QgsLayoutItem::endCommand()
{
  if ( mLayout )
    mLayout->undoStack()->endCommand();
}

void QgsLayoutItem::cancelCommand()
{
  if ( mLayout )
    mLayout->undoStack()->cancelCommand();
}

QgsLayoutPoint QgsLayoutItem::applyDataDefinedPosition( const QgsLayoutPoint &position )
{
  if ( !mLayout )
  {
    return position;
  }

  const QgsExpressionContext context = createExpressionContext();
  const double evaluatedX = mDataDefinedProperties.valueAsDouble( QgsLayoutObject::PositionX, context, position.x() );
  const double evaluatedY = mDataDefinedProperties.valueAsDouble( QgsLayoutObject::PositionY, context, position.y() );
  return QgsLayoutPoint( evaluatedX, evaluatedY, position.units() );
}

void QgsLayoutItem::applyDataDefinedOrientation( double &width, double &height, const QgsExpressionContext &context )
{
  bool ok = false;
  const QString orientationString = mDataDefinedProperties.valueAsString( QgsLayoutObject::PaperOrientation, context, QString(), &ok );
  if ( ok && !orientationString.isEmpty() )
  {
    const QgsLayoutItemPage::Orientation orientation = QgsLayoutUtils::decodePaperOrientation( orientationString, ok );
    if ( ok )
    {
      double heightD = 0.0, widthD = 0.0;
      switch ( orientation )
      {
        case QgsLayoutItemPage::Portrait:
        {
          heightD = std::max( height, width );
          widthD = std::min( height, width );
          break;
        }
        case QgsLayoutItemPage::Landscape:
        {
          heightD = std::min( height, width );
          widthD = std::max( height, width );
          break;
        }
      }
      width = widthD;
      height = heightD;
    }
  }
}

QgsLayoutSize QgsLayoutItem::applyDataDefinedSize( const QgsLayoutSize &size )
{
  if ( !mLayout )
  {
    return size;
  }

  if ( !mDataDefinedProperties.isActive( QgsLayoutObject::PresetPaperSize ) &&
       !mDataDefinedProperties.isActive( QgsLayoutObject::ItemWidth ) &&
       !mDataDefinedProperties.isActive( QgsLayoutObject::ItemHeight ) &&
       !mDataDefinedProperties.isActive( QgsLayoutObject::PaperOrientation ) )
    return size;


  const QgsExpressionContext context = createExpressionContext();

  // lowest priority is page size
  const QString pageSize = mDataDefinedProperties.valueAsString( QgsLayoutObject::PresetPaperSize, context );
  QgsPageSize matchedSize;
  double evaluatedWidth = size.width();
  double evaluatedHeight = size.height();
  if ( QgsApplication::pageSizeRegistry()->decodePageSize( pageSize, matchedSize ) )
  {
    const QgsLayoutSize convertedSize = mLayout->renderContext().measurementConverter().convert( matchedSize.size, size.units() );
    evaluatedWidth = convertedSize.width();
    evaluatedHeight = convertedSize.height();
  }

  // highest priority is dd width/height
  evaluatedWidth = mDataDefinedProperties.valueAsDouble( QgsLayoutObject::ItemWidth, context, evaluatedWidth );
  evaluatedHeight = mDataDefinedProperties.valueAsDouble( QgsLayoutObject::ItemHeight, context, evaluatedHeight );

  //which is finally overwritten by data defined orientation
  applyDataDefinedOrientation( evaluatedWidth, evaluatedHeight, context );

  return QgsLayoutSize( evaluatedWidth, evaluatedHeight, size.units() );
}

double QgsLayoutItem::applyDataDefinedRotation( const double rotation )
{
  if ( !mLayout )
  {
    return rotation;
  }

  const QgsExpressionContext context = createExpressionContext();
  const double evaluatedRotation = mDataDefinedProperties.valueAsDouble( QgsLayoutObject::ItemRotation, context, rotation );
  return evaluatedRotation;
}

void QgsLayoutItem::refreshDataDefinedProperty( const QgsLayoutObject::DataDefinedProperty property )
{
  //update data defined properties and update item to match

  //evaluate width and height first, since they may affect position if non-top-left reference point set
  if ( property == QgsLayoutObject::ItemWidth || property == QgsLayoutObject::ItemHeight ||
       property == QgsLayoutObject::AllProperties )
  {
    refreshItemSize();
  }
  if ( property == QgsLayoutObject::PositionX || property == QgsLayoutObject::PositionY ||
       property == QgsLayoutObject::AllProperties )
  {
    refreshItemPosition();
  }
  if ( property == QgsLayoutObject::ItemRotation || property == QgsLayoutObject::AllProperties )
  {
    refreshItemRotation();
  }
  if ( property == QgsLayoutObject::Opacity || property == QgsLayoutObject::AllProperties )
  {
    refreshOpacity( false );
  }
  if ( property == QgsLayoutObject::FrameColor || property == QgsLayoutObject::AllProperties )
  {
    refreshFrame( false );
  }
  if ( property == QgsLayoutObject::BackgroundColor || property == QgsLayoutObject::AllProperties )
  {
    refreshBackgroundColor( false );
  }
  if ( property == QgsLayoutObject::BlendMode || property == QgsLayoutObject::AllProperties )
  {
    refreshBlendMode();
  }
  if ( property == QgsLayoutObject::ExcludeFromExports || property == QgsLayoutObject::AllProperties )
  {
    const bool exclude = mExcludeFromExports;
    //data defined exclude from exports set?
    mEvaluatedExcludeFromExports = mDataDefinedProperties.valueAsBool( QgsLayoutObject::ExcludeFromExports, createExpressionContext(), exclude );
  }

  update();
}

void QgsLayoutItem::setItemRotation( double angle, const bool adjustPosition )
{
  if ( angle >= 360.0 || angle <= -360.0 )
  {
    angle = std::fmod( angle, 360.0 );
  }

  const QPointF point = adjustPosition ? positionAtReferencePoint( QgsLayoutItem::Middle )
                        : pos();
  const double rotationRequired = angle - rotation();
  rotateItem( rotationRequired, point );

  mItemRotation = angle;
}

void QgsLayoutItem::updateStoredItemPosition()
{
  const QPointF layoutPosReferencePoint = positionAtReferencePoint( mReferencePoint );
  mItemPosition = mLayout->convertFromLayoutUnits( layoutPosReferencePoint, mItemPosition.units() );
}

void QgsLayoutItem::rotateItem( const double angle, const QPointF transformOrigin )
{
  double evaluatedAngle = angle + rotation();
  evaluatedAngle = QgsLayoutUtils::normalizedAngle( evaluatedAngle, true );
  mItemRotation = evaluatedAngle;

  QPointF itemTransformOrigin = mapFromScene( transformOrigin );

  refreshItemRotation( &itemTransformOrigin );
}

QgsExpressionContext QgsLayoutItem::createExpressionContext() const
{
  QgsExpressionContext context = QgsLayoutObject::createExpressionContext();
  context.appendScope( QgsExpressionContextUtils::layoutItemScope( this ) );
  return context;
}

bool QgsLayoutItem::accept( QgsStyleEntityVisitorInterface *visitor ) const
{
  Q_UNUSED( visitor );
  return true;
}

QgsGeometry QgsLayoutItem::clipPath() const
{
  return QgsGeometry();
}

void QgsLayoutItem::refresh()
{
  QgsLayoutObject::refresh();
  refreshItemSize();

  refreshDataDefinedProperty();
}

void QgsLayoutItem::invalidateCache()
{
  if ( !mItemCachedImage.isNull() )
  {
    mItemCachedImage = QImage();
    mItemCacheDpi = -1;
    update();
  }
}

void QgsLayoutItem::redraw()
{
  update();
}

void QgsLayoutItem::drawDebugRect( QPainter *painter )
{
  if ( !painter )
  {
    return;
  }

  const QgsScopedQPainterState painterState( painter );
  painter->setRenderHint( QPainter::Antialiasing, false );
  painter->setPen( Qt::NoPen );
  painter->setBrush( QColor( 100, 255, 100, 200 ) );
  painter->drawRect( rect() );
}

QPainterPath QgsLayoutItem::framePath() const
{
  QPainterPath path;
  path.addRect( QRectF( 0, 0, rect().width(), rect().height() ) );
  return path;
}

void QgsLayoutItem::drawFrame( QgsRenderContext &context )
{
  if ( !mFrame || !context.painter() )
    return;

  QPainter *p = context.painter();

  const QgsScopedQPainterState painterState( p );

  p->setPen( pen() );
  p->setBrush( Qt::NoBrush );
  context.setPainterFlagsUsingContext( p );

  p->drawPath( framePath() );
}

void QgsLayoutItem::drawBackground( QgsRenderContext &context )
{
  if ( !mBackground || !context.painter() )
    return;

  const QgsScopedQPainterState painterState( context.painter() );

  QPainter *p = context.painter();
  p->setBrush( brush() );
  p->setPen( Qt::NoPen );
  context.setPainterFlagsUsingContext( p );

  p->drawPath( framePath() );
}

void QgsLayoutItem::setFixedSize( const QgsLayoutSize &size )
{
  mFixedSize = size;
  refreshItemSize();
}

void QgsLayoutItem::setMinimumSize( const QgsLayoutSize &size )
{
  mMinimumSize = size;
  refreshItemSize();
}

QSizeF QgsLayoutItem::applyItemSizeConstraint( const QSizeF targetSize )
{
  return targetSize;
}

void QgsLayoutItem::refreshItemSize()
{
  attemptResize( mItemSize );
}

void QgsLayoutItem::refreshItemPosition()
{
  attemptMove( mItemPosition );
}

QPointF QgsLayoutItem::itemPositionAtReferencePoint( const ReferencePoint reference, const QSizeF size ) const
{
  switch ( reference )
  {
    case UpperMiddle:
      return QPointF( size.width() / 2.0, 0 );
    case UpperRight:
      return QPointF( size.width(), 0 );
    case MiddleLeft:
      return QPointF( 0, size.height() / 2.0 );
    case Middle:
      return QPointF( size.width() / 2.0, size.height() / 2.0 );
    case MiddleRight:
      return QPointF( size.width(), size.height() / 2.0 );
    case LowerLeft:
      return QPointF( 0, size.height() );
    case LowerMiddle:
      return QPointF( size.width() / 2.0, size.height() );
    case LowerRight:
      return QPointF( size.width(), size.height() );
    case UpperLeft:
      return QPointF( 0, 0 );
  }
  // no warnings
  return QPointF( 0, 0 );
}

QPointF QgsLayoutItem::adjustPointForReferencePosition( const QPointF position, const QSizeF size, const ReferencePoint reference ) const
{
  const QPointF itemPosition = mapFromScene( position ); //need to map from scene to handle item rotation
  const QPointF adjustedPointInsideItem = itemPosition - itemPositionAtReferencePoint( reference, size );
  return mapToScene( adjustedPointInsideItem );
}

QPointF QgsLayoutItem::positionAtReferencePoint( const QgsLayoutItem::ReferencePoint reference ) const
{
  const QPointF pointWithinItem = itemPositionAtReferencePoint( reference, rect().size() );
  return mapToScene( pointWithinItem );
}

QgsLayoutPoint QgsLayoutItem::topLeftToReferencePoint( const QgsLayoutPoint &point ) const
{
  const QPointF topLeft = mLayout->convertToLayoutUnits( point );
  const QPointF refPoint = topLeft + itemPositionAtReferencePoint( mReferencePoint, rect().size() );
  return mLayout->convertFromLayoutUnits( refPoint, point.units() );
}

bool QgsLayoutItem::writePropertiesToElement( QDomElement &, QDomDocument &, const QgsReadWriteContext & ) const
{
  return true;
}

bool QgsLayoutItem::readPropertiesFromElement( const QDomElement &, const QDomDocument &, const QgsReadWriteContext & )
{

  return true;
}

void QgsLayoutItem::initConnectionsToLayout()
{
  if ( !mLayout )
    return;

}

void QgsLayoutItem::preparePainter( QPainter *painter )
{
  if ( !painter || !painter->device() )
  {
    return;
  }

  painter->setRenderHint( QPainter::Antialiasing, shouldDrawAntialiased() );

#if QT_VERSION >= QT_VERSION_CHECK(5, 13, 0)
  painter->setRenderHint( QPainter::LosslessImageRendering, mLayout && mLayout->renderContext().testFlag( QgsLayoutRenderContext::FlagLosslessImageRendering ) );
#endif
}

bool QgsLayoutItem::shouldDrawAntialiased() const
{
  if ( !mLayout )
  {
    return true;
  }
  return mLayout->renderContext().testFlag( QgsLayoutRenderContext::FlagAntialiasing ) && !mLayout->renderContext().testFlag( QgsLayoutRenderContext::FlagDebug );
}

bool QgsLayoutItem::shouldDrawDebugRect() const
{
  return mLayout && mLayout->renderContext().testFlag( QgsLayoutRenderContext::FlagDebug );
}

QSizeF QgsLayoutItem::applyMinimumSize( const QSizeF targetSize )
{
  if ( !mLayout || minimumSize().isEmpty() )
  {
    return targetSize;
  }
  const QSizeF minimumSizeLayoutUnits = mLayout->convertToLayoutUnits( minimumSize() );
  return targetSize.expandedTo( minimumSizeLayoutUnits );
}

QSizeF QgsLayoutItem::applyFixedSize( const QSizeF targetSize )
{
  if ( !mLayout || fixedSize().isEmpty() )
  {
    return targetSize;
  }

  QSizeF size = targetSize;
  const QSizeF fixedSizeLayoutUnits = mLayout->convertToLayoutUnits( fixedSize() );
  if ( fixedSizeLayoutUnits.width() > 0 )
    size.setWidth( fixedSizeLayoutUnits.width() );
  if ( fixedSizeLayoutUnits.height() > 0 )
    size.setHeight( fixedSizeLayoutUnits.height() );

  return size;
}

void QgsLayoutItem::refreshItemRotation( QPointF *origin )
{
  double r = mItemRotation;

  //data defined rotation set?
  r = mDataDefinedProperties.valueAsDouble( QgsLayoutItem::ItemRotation, createExpressionContext(), r );

  if ( qgsDoubleNear( r, rotation() ) && !origin )
  {
    return;
  }

  const QPointF transformPoint = origin ? *origin : mapFromScene( positionAtReferencePoint( QgsLayoutItem::Middle ) );

  if ( !transformPoint.isNull() )
  {
    //adjustPosition set, so shift the position of the item so that rotation occurs around item center
    //create a line from the transform point to the item's origin, in scene coordinates
    QLineF refLine = QLineF( mapToScene( transformPoint ), mapToScene( QPointF( 0, 0 ) ) );
    //rotate this line by the current rotation angle
    refLine.setAngle( refLine.angle() - r + rotation() );
    //get new end point of line - this is the new item position
    const QPointF rotatedReferencePoint = refLine.p2();
    setPos( rotatedReferencePoint );
  }

  setTransformOriginPoint( 0, 0 );
  QGraphicsItem::setRotation( r );

  //adjust stored position of item to match scene pos of reference point
  updateStoredItemPosition();
  emit sizePositionChanged();

  emit rotationChanged( r );

  //update bounds of scene, since rotation may affect this
  mLayout->updateBounds();
}

void QgsLayoutItem::refreshOpacity( bool updateItem )
{
  //data defined opacity set?
  const double opacity = mDataDefinedProperties.valueAsDouble( QgsLayoutObject::Opacity, createExpressionContext(), mOpacity * 100.0 );

  // Set the QGraphicItem's opacity
  mEvaluatedOpacity = opacity / 100.0;

  if ( itemFlags() & QgsLayoutItem::FlagOverridesPaint )
  {
    // item handles it's own painting, so it won't use the built-in opacity handling in QgsLayoutItem::paint, and
    // we have to rely on QGraphicsItem opacity to handle this
    setOpacity( mEvaluatedOpacity );
  }

  if ( updateItem )
  {
    update();
  }
}

void QgsLayoutItem::refreshFrame( bool updateItem )
{
  if ( !mFrame )
  {
    setPen( Qt::NoPen );
    return;
  }

  //data defined stroke color set?
  bool ok = false;
  const QColor frameColor = mDataDefinedProperties.valueAsColor( QgsLayoutObject::FrameColor, createExpressionContext(), mFrameColor, &ok );
  QPen itemPen;
  if ( ok )
  {
    itemPen = QPen( frameColor );
  }
  else
  {
    itemPen = QPen( mFrameColor );
  }
  itemPen.setJoinStyle( mFrameJoinStyle );

  if ( mLayout )
    itemPen.setWidthF( mLayout->convertToLayoutUnits( mFrameWidth ) );
  else
    itemPen.setWidthF( mFrameWidth.length() );

  setPen( itemPen );

  if ( updateItem )
  {
    update();
  }
}

void QgsLayoutItem::refreshBackgroundColor( bool updateItem )
{
  //data defined fill color set?
  bool ok = false;
  const QColor backgroundColor = mDataDefinedProperties.valueAsColor( QgsLayoutObject::BackgroundColor, createExpressionContext(), mBackgroundColor, &ok );
  if ( ok )
  {
    setBrush( QBrush( backgroundColor, Qt::SolidPattern ) );
  }
  else
  {
    setBrush( QBrush( mBackgroundColor, Qt::SolidPattern ) );
  }
  if ( updateItem )
  {
    update();
  }
}

void QgsLayoutItem::refreshBlendMode()
{
  QPainter::CompositionMode blendMode = mBlendMode;

  //data defined blend mode set?
  bool ok = false;
  const QString blendStr = mDataDefinedProperties.valueAsString( QgsLayoutObject::BlendMode, createExpressionContext(), QString(), &ok );
  if ( ok && !blendStr.isEmpty() )
  {
    const QString blendstr = blendStr.trimmed();
    const QPainter::CompositionMode blendModeD = QgsSymbolLayerUtils::decodeBlendMode( blendstr );
    blendMode = blendModeD;
  }

  // Update the item effect to use the new blend mode
  mEffect->setCompositionMode( blendMode );
}

