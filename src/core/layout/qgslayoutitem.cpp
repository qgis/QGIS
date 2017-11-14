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
#include <QPainter>
#include <QStyleOptionGraphicsItem>
#include <QUuid>

#define CACHE_SIZE_LIMIT 5000

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
  QgsUnitTypes::LayoutUnit initialUnits = layout ? layout->units() : QgsUnitTypes::LayoutMillimeters;
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
    mEffect->setEnabled( mLayout->context().flags() & QgsLayoutContext::FlagUseAdvancedEffects );
    connect( &mLayout->context(), &QgsLayoutContext::flagsChanged, this, [ = ]( QgsLayoutContext::Flags flags )
    {
      mEffect->setEnabled( flags & QgsLayoutContext::FlagUseAdvancedEffects );
    } );
  }
  setGraphicsEffect( mEffect.get() );
}

QgsLayoutItem::~QgsLayoutItem()
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

#if 0 //TODO
  emit itemChanged();
#endif
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
    mLayout->undoStack()->stack()->push( command.release() );
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

  double destinationDpi = itemStyle->matrix.m11() * 25.4;
  bool useImageCache = false;

  if ( useImageCache )
  {
    double widthInPixels = boundingRect().width() * itemStyle->matrix.m11();
    double heightInPixels = boundingRect().height() * itemStyle->matrix.m11();

    // limit size of image for better performance
    double scale = 1.0;
    if ( widthInPixels > CACHE_SIZE_LIMIT || heightInPixels > CACHE_SIZE_LIMIT )
    {
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

    if ( !mItemCachedImage.isNull() && qgsDoubleNear( mItemCacheDpi, destinationDpi ) )
    {
      // can reuse last cached image
      QgsRenderContext context = QgsLayoutUtils::createRenderContextForMap( nullptr, painter, destinationDpi );
      painter->save();
      preparePainter( painter );
      double cacheScale = destinationDpi / mItemCacheDpi;
      painter->scale( cacheScale / context.scaleFactor(), cacheScale / context.scaleFactor() );
      painter->drawImage( boundingRect().x() * context.scaleFactor() / cacheScale,
                          boundingRect().y() * context.scaleFactor() / cacheScale, mItemCachedImage );
      painter->restore();
      return;
    }
    else
    {
      mItemCacheDpi = destinationDpi;

      mItemCachedImage = QImage( widthInPixels, heightInPixels, QImage::Format_ARGB32 );
      mItemCachedImage.fill( Qt::transparent );
      mItemCachedImage.setDotsPerMeterX( 1000 * destinationDpi * 25.4 );
      mItemCachedImage.setDotsPerMeterY( 1000 * destinationDpi * 25.4 );
      QPainter p( &mItemCachedImage );

      preparePainter( &p );
      QgsRenderContext context = QgsLayoutUtils::createRenderContextForLayout( nullptr, &p, destinationDpi );
      // painter is already scaled to dots
      // need to translate so that item origin is at 0,0 in painter coordinates (not bounding rect origin)
      p.translate( -boundingRect().x() * context.scaleFactor(), -boundingRect().y() * context.scaleFactor() );
      drawBackground( context );
      draw( context, itemStyle );
      drawFrame( context );
      p.end();

      painter->save();
      // scale painter from mm to dots
      painter->scale( 1.0 / context.scaleFactor(), 1.0 / context.scaleFactor() );
      painter->drawImage( boundingRect().x() * context.scaleFactor(),
                          boundingRect().y() * context.scaleFactor(), mItemCachedImage );
      painter->restore();
    }
  }
  else
  {
    // no caching or flattening
    painter->save();
    preparePainter( painter );
    QgsRenderContext context = QgsLayoutUtils::createRenderContextForLayout( mLayout, painter, destinationDpi );
    context.setExpressionContext( createExpressionContext() );
    drawBackground( context );

    // scale painter from mm to dots
    painter->scale( 1.0 / context.scaleFactor(), 1.0 / context.scaleFactor() );
    draw( context, itemStyle );

    painter->scale( context.scaleFactor(), context.scaleFactor() );
    drawFrame( context );

    painter->restore();
  }
}

void QgsLayoutItem::setReferencePoint( const QgsLayoutItem::ReferencePoint &point )
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
    double bleed = mLayout->convertFromLayoutUnits( estimatedFrameBleed(), size.units() ).length();
    size.setWidth( size.width() - 2 * bleed );
    size.setHeight( size.height() - 2 * bleed );
  }

  QgsLayoutSize evaluatedSize = applyDataDefinedSize( size );
  QSizeF targetSizeLayoutUnits = mLayout->convertToLayoutUnits( evaluatedSize );
  QSizeF actualSizeLayoutUnits = applyMinimumSize( targetSizeLayoutUnits );
  actualSizeLayoutUnits = applyFixedSize( actualSizeLayoutUnits );
  actualSizeLayoutUnits = applyItemSizeConstraint( actualSizeLayoutUnits );

  if ( actualSizeLayoutUnits == rect().size() )
  {
    return;
  }

  QgsLayoutSize actualSizeTargetUnits = mLayout->convertFromLayoutUnits( actualSizeLayoutUnits, size.units() );
  mItemSize = actualSizeTargetUnits;

  setRect( 0, 0, actualSizeLayoutUnits.width(), actualSizeLayoutUnits.height() );
  refreshItemPosition();
  emit sizePositionChanged();
}

void QgsLayoutItem::attemptMove( const QgsLayoutPoint &p, bool useReferencePoint, bool includesFrame )
{
  if ( !mLayout )
  {
    mItemPosition = p;
    setPos( p.toQPointF() );
    return;
  }

  QgsLayoutPoint point = p;

  if ( includesFrame )
  {
    //adjust position to account for frame size
    double bleed = mLayout->convertFromLayoutUnits( estimatedFrameBleed(), point.units() ).length();
    point.setX( point.x() + bleed );
    point.setY( point.y() + bleed );
  }

  QgsLayoutPoint evaluatedPoint = point;
  if ( !useReferencePoint )
  {
    evaluatedPoint = topLeftToReferencePoint( point );
  }

  evaluatedPoint = applyDataDefinedPosition( evaluatedPoint );
  QPointF evaluatedPointLayoutUnits = mLayout->convertToLayoutUnits( evaluatedPoint );
  QPointF topLeftPointLayoutUnits = adjustPointForReferencePosition( evaluatedPointLayoutUnits, rect().size(), mReferencePoint );
  if ( topLeftPointLayoutUnits == scenePos() && point.units() == mItemPosition.units() )
  {
    //TODO - add test for second condition
    return;
  }

  QgsLayoutPoint referencePointTargetUnits = mLayout->convertFromLayoutUnits( evaluatedPointLayoutUnits, point.units() );
  mItemPosition = referencePointTargetUnits;
  setScenePos( topLeftPointLayoutUnits );
  emit sizePositionChanged();
}

void QgsLayoutItem::attemptSetSceneRect( const QRectF &rect, bool includesFrame )
{
  QPointF newPos = rect.topLeft();

  blockSignals( true );
  // translate new size to current item units
  QgsLayoutSize newSize = mLayout->convertFromLayoutUnits( rect.size(), mItemSize.units() );
  attemptResize( newSize, includesFrame );

  // translate new position to current item units
  QgsLayoutPoint itemPos = mLayout->convertFromLayoutUnits( newPos, mItemPosition.units() );
  attemptMove( itemPos, false, includesFrame );
  blockSignals( false );
  emit sizePositionChanged();
}

void QgsLayoutItem::setScenePos( const QPointF &destinationPos )
{
  //since setPos does not account for item rotation, use difference between
  //current scenePos (which DOES account for rotation) and destination pos
  //to calculate how much the item needs to move
  if ( parentItem() )
    setPos( pos() + ( destinationPos - scenePos() ) + parentItem()->scenePos() );
  else
    setPos( pos() + ( destinationPos - scenePos() ) );
}

bool QgsLayoutItem::shouldBlockUndoCommands() const
{
  return !mLayout || mLayout != scene() || mBlockUndoCommands;
}

bool QgsLayoutItem::shouldDrawItem() const
{
  if ( !mLayout || mLayout->context().isPreviewRender() )
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
  element.setAttribute( QStringLiteral( "type" ), stringType() );

  element.setAttribute( QStringLiteral( "uuid" ), mUuid );
  element.setAttribute( QStringLiteral( "id" ), mId );
  element.setAttribute( QStringLiteral( "referencePoint" ), QString::number( static_cast< int >( mReferencePoint ) ) );
  element.setAttribute( QStringLiteral( "position" ), mItemPosition.encodePoint() );
  element.setAttribute( QStringLiteral( "size" ), mItemSize.encodeSize() );
  element.setAttribute( QStringLiteral( "itemRotation" ), QString::number( mItemRotation ) );
  element.setAttribute( QStringLiteral( "groupUuid" ), mParentGroupUuid );

  element.setAttribute( "zValue", QString::number( zValue() ) );
  element.setAttribute( "visibility", isVisible() );
  //position lock for mouse moves/resizes
  if ( mIsLocked )
  {
    element.setAttribute( "positionLock", "true" );
  }
  else
  {
    element.setAttribute( "positionLock", "false" );
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
  element.setAttribute( "blendMode", QgsPainting::getBlendModeEnum( mBlendMode ) );

  //opacity
  element.setAttribute( QStringLiteral( "opacity" ), QString::number( mOpacity ) );

  element.setAttribute( "excludeFromExports", mExcludeFromExports );

  writeObjectPropertiesToElement( element, doc, context );

  writePropertiesToElement( element, doc, context );
  parentElement.appendChild( element );

  return true;
}

bool QgsLayoutItem::readXml( const QDomElement &element, const QDomDocument &doc, const QgsReadWriteContext &context )
{
  if ( element.nodeName() != QStringLiteral( "LayoutItem" ) || element.attribute( QStringLiteral( "type" ) ) != stringType() )
  {
    return false;
  }

  readObjectPropertiesFromElement( element, doc, context );

  mBlockUndoCommands = true;
  mUuid = element.attribute( QStringLiteral( "uuid" ), QUuid::createUuid().toString() );
  setId( element.attribute( QStringLiteral( "id" ) ) );
  mReferencePoint = static_cast< ReferencePoint >( element.attribute( QStringLiteral( "referencePoint" ) ).toInt() );
  attemptMove( QgsLayoutPoint::decodePoint( element.attribute( QStringLiteral( "position" ) ) ) );
  attemptResize( QgsLayoutSize::decodeSize( element.attribute( QStringLiteral( "size" ) ) ) );
  setItemRotation( element.attribute( QStringLiteral( "itemRotation" ), QStringLiteral( "0" ) ).toDouble() );

  mParentGroupUuid = element.attribute( QStringLiteral( "groupUuid" ) );
  if ( !mParentGroupUuid.isEmpty() )
  {
    if ( QgsLayoutItemGroup *group = parentGroup() )
    {
      group->addItem( this );
    }
  }

  //TODO
  /*
  // temporary for groups imported from templates
  mTemplateUuid = itemElem.attribute( "templateUuid" );
  */

  //position lock for mouse moves/resizes
  QString positionLock = element.attribute( "positionLock" );
  if ( positionLock.compare( "true", Qt::CaseInsensitive ) == 0 )
  {
    setLocked( true );
  }
  else
  {
    setLocked( false );
  }
  //visibility
  setVisibility( element.attribute( "visibility", "1" ) != "0" );
  setZValue( element.attribute( "zValue" ).toDouble() );

  //frame
  QString frame = element.attribute( QStringLiteral( "frame" ) );
  if ( frame.compare( QLatin1String( "true" ), Qt::CaseInsensitive ) == 0 )
  {
    mFrame = true;
  }
  else
  {
    mFrame = false;
  }

  //frame
  QString background = element.attribute( QStringLiteral( "background" ) );
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
  QDomNodeList frameColorList = element.elementsByTagName( QStringLiteral( "FrameColor" ) );
  if ( !frameColorList.isEmpty() )
  {
    QDomElement frameColorElem = frameColorList.at( 0 ).toElement();
    bool redOk = false;
    bool greenOk = false;
    bool blueOk = false;
    bool alphaOk = false;
    int penRed, penGreen, penBlue, penAlpha;

#if 0 // TODO, old style
    double penWidth;
    penWidth = element.attribute( QStringLiteral( "outlineWidth" ) ).toDouble( &widthOk );
#endif
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
  QDomNodeList bgColorList = element.elementsByTagName( QStringLiteral( "BackgroundColor" ) );
  if ( !bgColorList.isEmpty() )
  {
    QDomElement bgColorElem = bgColorList.at( 0 ).toElement();
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

  bool result = readPropertiesFromElement( element, doc, context );

  mBlockUndoCommands = false;

  emit changed();
  update();
  return result;
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

void QgsLayoutItem::setFrameStrokeWidth( const QgsLayoutMeasurement &width )
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
  refreshOpacity( true );
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

double QgsLayoutItem::estimatedFrameBleed() const
{
  if ( !hasFrame() )
  {
    return 0;
  }

  return pen().widthF() / 2.0;
}

QRectF QgsLayoutItem::rectWithFrame() const
{
  double frameBleed = estimatedFrameBleed();
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

  QgsExpressionContext context = createExpressionContext();
  double evaluatedX = mDataDefinedProperties.valueAsDouble( QgsLayoutObject::PositionX, context, position.x() );
  double evaluatedY = mDataDefinedProperties.valueAsDouble( QgsLayoutObject::PositionY, context, position.y() );
  return QgsLayoutPoint( evaluatedX, evaluatedY, position.units() );
}

QgsLayoutSize QgsLayoutItem::applyDataDefinedSize( const QgsLayoutSize &size )
{
  if ( !mLayout )
  {
    return size;
  }

  if ( !mDataDefinedProperties.isActive( QgsLayoutObject::PresetPaperSize ) &&
       !mDataDefinedProperties.isActive( QgsLayoutObject::ItemWidth ) &&
       !mDataDefinedProperties.isActive( QgsLayoutObject::ItemHeight ) )
    return size;


  QgsExpressionContext context = createExpressionContext();

  // lowest priority is page size
  QString pageSize = mDataDefinedProperties.valueAsString( QgsLayoutObject::PresetPaperSize, context );
  QgsPageSize matchedSize;
  double evaluatedWidth = size.width();
  double evaluatedHeight = size.height();
  if ( QgsApplication::pageSizeRegistry()->decodePageSize( pageSize, matchedSize ) )
  {
    QgsLayoutSize convertedSize = mLayout->context().measurementConverter().convert( matchedSize.size, size.units() );
    evaluatedWidth = convertedSize.width();
    evaluatedHeight = convertedSize.height();
  }

  // highest priority is dd width/height
  evaluatedWidth = mDataDefinedProperties.valueAsDouble( QgsLayoutObject::ItemWidth, context, evaluatedWidth );
  evaluatedHeight = mDataDefinedProperties.valueAsDouble( QgsLayoutObject::ItemHeight, context, evaluatedHeight );
  return QgsLayoutSize( evaluatedWidth, evaluatedHeight, size.units() );
}

double QgsLayoutItem::applyDataDefinedRotation( const double rotation )
{
  if ( !mLayout )
  {
    return rotation;
  }

  QgsExpressionContext context = createExpressionContext();
  double evaluatedRotation = mDataDefinedProperties.valueAsDouble( QgsLayoutObject::ItemRotation, context, rotation );
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
    bool exclude = mExcludeFromExports;
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

  QPointF point = adjustPosition ? positionAtReferencePoint( QgsLayoutItem::Middle )
                  : pos();
  double rotationRequired = angle - rotation();
  rotateItem( rotationRequired, point );

  mItemRotation = angle;
}

void QgsLayoutItem::updateStoredItemPosition()
{
  QPointF layoutPosReferencePoint = positionAtReferencePoint( mReferencePoint );
  mItemPosition = mLayout->convertFromLayoutUnits( layoutPosReferencePoint, mItemPosition.units() );
}

void QgsLayoutItem::rotateItem( const double angle, const QPointF &transformOrigin )
{
  double evaluatedAngle = angle + rotation();
  evaluatedAngle = QgsLayoutUtils::normalizedAngle( evaluatedAngle, true );
  mItemRotation = evaluatedAngle;

  QPointF itemTransformOrigin = mapFromScene( transformOrigin );

  refreshItemRotation( &itemTransformOrigin );
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

  painter->save();
  painter->setRenderHint( QPainter::Antialiasing, false );
  painter->setPen( Qt::NoPen );
  painter->setBrush( QColor( 100, 255, 100, 200 ) );
  painter->drawRect( rect() );
  painter->restore();
}

void QgsLayoutItem::drawFrame( QgsRenderContext &context )
{
  if ( !mFrame || !context.painter() )
    return;

  QPainter *p = context.painter();
  p->save();
  p->setPen( pen() );
  p->setBrush( Qt::NoBrush );
  p->drawRect( QRectF( 0, 0, rect().width(), rect().height() ) );
  p->restore();
}

void QgsLayoutItem::drawBackground( QgsRenderContext &context )
{
  if ( !mBackground || !context.painter() )
    return;

  QPainter *p = context.painter();
  p->save();
  p->setBrush( brush() );
  p->setPen( Qt::NoPen );
  p->drawRect( QRectF( 0, 0, rect().width(), rect().height() ) );
  p->restore();
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

QSizeF QgsLayoutItem::applyItemSizeConstraint( const QSizeF &targetSize )
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

QPointF QgsLayoutItem::itemPositionAtReferencePoint( const ReferencePoint reference, const QSizeF &size ) const
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

QPointF QgsLayoutItem::adjustPointForReferencePosition( const QPointF &position, const QSizeF &size, const ReferencePoint &reference ) const
{
  QPointF itemPosition = mapFromScene( position ); //need to map from scene to handle item rotation
  QPointF adjustedPointInsideItem = itemPosition - itemPositionAtReferencePoint( reference, size );
  return mapToScene( adjustedPointInsideItem );
}

QPointF QgsLayoutItem::positionAtReferencePoint( const QgsLayoutItem::ReferencePoint &reference ) const
{
  QPointF pointWithinItem = itemPositionAtReferencePoint( reference, rect().size() );
  return mapToScene( pointWithinItem );
}

QgsLayoutPoint QgsLayoutItem::topLeftToReferencePoint( const QgsLayoutPoint &point ) const
{
  QPointF topLeft = mLayout->convertToLayoutUnits( point );
  QPointF refPoint = topLeft + itemPositionAtReferencePoint( mReferencePoint, rect().size() );
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
}

bool QgsLayoutItem::shouldDrawAntialiased() const
{
  if ( !mLayout )
  {
    return true;
  }
  return mLayout->context().testFlag( QgsLayoutContext::FlagAntialiasing ) && !mLayout->context().testFlag( QgsLayoutContext::FlagDebug );
}

bool QgsLayoutItem::shouldDrawDebugRect() const
{
  return mLayout && mLayout->context().testFlag( QgsLayoutContext::FlagDebug );
}

QSizeF QgsLayoutItem::applyMinimumSize( const QSizeF &targetSize )
{
  if ( !mLayout || minimumSize().isEmpty() )
  {
    return targetSize;
  }
  QSizeF minimumSizeLayoutUnits = mLayout->convertToLayoutUnits( minimumSize() );
  return targetSize.expandedTo( minimumSizeLayoutUnits );
}

QSizeF QgsLayoutItem::applyFixedSize( const QSizeF &targetSize )
{
  if ( !mLayout || fixedSize().isEmpty() )
  {
    return targetSize;
  }
  QSizeF fixedSizeLayoutUnits = mLayout->convertToLayoutUnits( fixedSize() );
  return targetSize.expandedTo( fixedSizeLayoutUnits );
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

  QPointF transformPoint = origin ? *origin : mapFromScene( positionAtReferencePoint( QgsLayoutItem::Middle ) );

  if ( !transformPoint.isNull() )
  {
    //adjustPosition set, so shift the position of the item so that rotation occurs around item center
    //create a line from the transform point to the item's origin, in scene coordinates
    QLineF refLine = QLineF( mapToScene( transformPoint ), mapToScene( QPointF( 0, 0 ) ) );
    //rotate this line by the current rotation angle
    refLine.setAngle( refLine.angle() - r + rotation() );
    //get new end point of line - this is the new item position
    QPointF rotatedReferencePoint = refLine.p2();
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
  double opacity = mDataDefinedProperties.valueAsDouble( QgsLayoutObject::Opacity, createExpressionContext(), mOpacity * 100.0 );

  // Set the QGraphicItem's opacity
  setOpacity( opacity / 100.0 );

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
  QColor frameColor = mDataDefinedProperties.valueAsColor( QgsLayoutObject::FrameColor, createExpressionContext(), mFrameColor, &ok );
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
  QColor backgroundColor = mDataDefinedProperties.valueAsColor( QgsLayoutObject::BackgroundColor, createExpressionContext(), mBackgroundColor, &ok );
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
  QString blendStr = mDataDefinedProperties.valueAsString( QgsLayoutObject::BlendMode, createExpressionContext(), QString(), &ok );
  if ( ok && !blendStr.isEmpty() )
  {
    QString blendstr = blendStr.trimmed();
    QPainter::CompositionMode blendModeD = QgsSymbolLayerUtils::decodeBlendMode( blendstr );
    blendMode = blendModeD;
  }

  // Update the item effect to use the new blend mode
  mEffect->setCompositionMode( blendMode );
}
