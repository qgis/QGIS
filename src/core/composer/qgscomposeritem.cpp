/***************************************************************************
                         qgscomposeritem.cpp
                             -------------------
    begin                : January 2005
    copyright            : (C) 2005 by Radim Blazek
    email                : blazek@itc.it
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/
#include <QWidget>
#include <QDomNode>
#include <QFile>
#include <QGraphicsLineItem>
#include <QGraphicsScene>
#include <QGraphicsSceneMouseEvent>
#include <QGraphicsView>
#include <QPainter>
#include <QUuid>
#include <QGraphicsEffect>

#include "qgsproject.h"

#include "qgscomposition.h"
#include "qgscomposeritem.h"
#include "qgscomposerframe.h"
#include "qgscomposerutils.h"
#include "qgscomposermodel.h"
#include "qgscomposereffect.h"

#include <limits>
#include "qgsapplication.h"
#include "qgsrectangle.h" //just for debugging
#include "qgslogger.h"
#include "qgssymbollayerutils.h" //for pointOnLineWithDistance
#include "qgspainting.h"
#include "qgsexpressioncontext.h"

#include <cmath>

QgsComposerItem::QgsComposerItem( QgsComposition *composition, bool manageZValue )
  : QgsComposerObject( composition )
  , QGraphicsRectItem( nullptr )
  , mRemovedFromComposition( false )
  , mFrame( false )
  , mBackground( true )
  , mBackgroundColor( QColor( 255, 255, 255, 255 ) )
  , mItemPositionLocked( false )
  , mLastValidViewScaleFactor( -1 )
  , mItemRotation( 0 )
  , mEvaluatedItemRotation( 0 )
  , mBlendMode( QPainter::CompositionMode_SourceOver )
  , mEffectsEnabled( true )
  , mExcludeFromExports( false )
  , mEvaluatedExcludeFromExports( false )
  , mLastUsedPositionMode( UpperLeft )
  , mIsGroupMember( false )
  , mCurrentExportLayer( -1 )
  , mUuid( QUuid::createUuid().toString() )
{
  init( manageZValue );
}

QgsComposerItem::QgsComposerItem( qreal x, qreal y, qreal width, qreal height, QgsComposition *composition, bool manageZValue )
  : QgsComposerObject( composition )
  , QGraphicsRectItem( 0, 0, width, height, nullptr )
  , mRemovedFromComposition( false )
  , mFrame( false )
  , mFrameColor( QColor( 0, 0, 0 ) )
  , mBackground( true )
  , mBackgroundColor( QColor( 255, 255, 255, 255 ) )
  , mItemPositionLocked( false )
  , mLastValidViewScaleFactor( -1 )
  , mItemRotation( 0 )
  , mEvaluatedItemRotation( 0 )
  , mBlendMode( QPainter::CompositionMode_SourceOver )
  , mEffectsEnabled( true )
  , mExcludeFromExports( false )
  , mEvaluatedExcludeFromExports( false )
  , mLastUsedPositionMode( UpperLeft )
  , mIsGroupMember( false )
  , mCurrentExportLayer( -1 )
  , mUuid( QUuid::createUuid().toString() )
{
  init( manageZValue );
  setPos( x, y );
}

void QgsComposerItem::init( const bool manageZValue )
{
  setFlag( QGraphicsItem::ItemIsSelectable, true );
  //set default pen and brush
  setBrush( mBackgroundColor );
  QPen defaultPen( mFrameColor );
  defaultPen.setWidthF( mFrameWidth );
  defaultPen.setJoinStyle( mFrameJoinStyle );
  setPen( defaultPen );
  //let z-Value be managed by composition
  if ( mComposition && manageZValue )
  {
    mCompositionManagesZValue = true;
    mComposition->addItemToZList( this );
  }
  else
  {
    mCompositionManagesZValue = false;
  }

  // Setup composer effect
  mEffect = new QgsComposerEffect();
  setGraphicsEffect( mEffect );
}

QgsComposerItem::~QgsComposerItem()
{
  if ( mComposition && mCompositionManagesZValue )
  {
    mComposition->removeItemFromZList( this );
  }

  delete mBoundingResizeRectangle;
  delete mEffect;

  deleteAlignItems();
}

void QgsComposerItem::setSelected( bool s )
{
  QGraphicsRectItem::setSelected( s );
  //inform model that id data has changed
  if ( mComposition )
  {
    mComposition->itemsModel()->updateItemSelectStatus( this );
  }
  update(); //to draw selection boxes
}

bool QgsComposerItem::_writeXml( QDomElement &itemElem, QDomDocument &doc ) const
{
  if ( itemElem.isNull() )
  {
    return false;
  }

  QDomElement composerItemElem = doc.createElement( QStringLiteral( "ComposerItem" ) );

  //frame
  if ( mFrame )
  {
    composerItemElem.setAttribute( QStringLiteral( "frame" ), QStringLiteral( "true" ) );
  }
  else
  {
    composerItemElem.setAttribute( QStringLiteral( "frame" ), QStringLiteral( "false" ) );
  }

  //background
  if ( mBackground )
  {
    composerItemElem.setAttribute( QStringLiteral( "background" ), QStringLiteral( "true" ) );
  }
  else
  {
    composerItemElem.setAttribute( QStringLiteral( "background" ), QStringLiteral( "false" ) );
  }

  //scene rect
  QPointF pagepos = pagePos();
  composerItemElem.setAttribute( QStringLiteral( "x" ), QString::number( pos().x() ) );
  composerItemElem.setAttribute( QStringLiteral( "y" ), QString::number( pos().y() ) );
  composerItemElem.setAttribute( QStringLiteral( "page" ), page() );
  composerItemElem.setAttribute( QStringLiteral( "pagex" ), QString::number( pagepos.x() ) );
  composerItemElem.setAttribute( QStringLiteral( "pagey" ), QString::number( pagepos.y() ) );
  composerItemElem.setAttribute( QStringLiteral( "width" ), QString::number( rect().width() ) );
  composerItemElem.setAttribute( QStringLiteral( "height" ), QString::number( rect().height() ) );
  composerItemElem.setAttribute( QStringLiteral( "positionMode" ), QString::number( static_cast< int >( mLastUsedPositionMode ) ) );
  composerItemElem.setAttribute( QStringLiteral( "zValue" ), QString::number( zValue() ) );
  composerItemElem.setAttribute( QStringLiteral( "outlineWidth" ), QString::number( mFrameWidth ) );
  composerItemElem.setAttribute( QStringLiteral( "frameJoinStyle" ), QgsSymbolLayerUtils::encodePenJoinStyle( mFrameJoinStyle ) );
  composerItemElem.setAttribute( QStringLiteral( "itemRotation" ), QString::number( mItemRotation ) );
  composerItemElem.setAttribute( QStringLiteral( "uuid" ), mUuid );
  composerItemElem.setAttribute( QStringLiteral( "id" ), mId );
  composerItemElem.setAttribute( QStringLiteral( "visibility" ), isVisible() );
  //position lock for mouse moves/resizes
  if ( mItemPositionLocked )
  {
    composerItemElem.setAttribute( QStringLiteral( "positionLock" ), QStringLiteral( "true" ) );
  }
  else
  {
    composerItemElem.setAttribute( QStringLiteral( "positionLock" ), QStringLiteral( "false" ) );
  }

  composerItemElem.setAttribute( QStringLiteral( "lastValidViewScaleFactor" ), QString::number( mLastValidViewScaleFactor ) );

  //frame color
  QDomElement frameColorElem = doc.createElement( QStringLiteral( "FrameColor" ) );
  frameColorElem.setAttribute( QStringLiteral( "red" ), QString::number( mFrameColor.red() ) );
  frameColorElem.setAttribute( QStringLiteral( "green" ), QString::number( mFrameColor.green() ) );
  frameColorElem.setAttribute( QStringLiteral( "blue" ), QString::number( mFrameColor.blue() ) );
  frameColorElem.setAttribute( QStringLiteral( "alpha" ), QString::number( mFrameColor.alpha() ) );
  composerItemElem.appendChild( frameColorElem );

  //background color
  QDomElement bgColorElem = doc.createElement( QStringLiteral( "BackgroundColor" ) );
  bgColorElem.setAttribute( QStringLiteral( "red" ), QString::number( mBackgroundColor.red() ) );
  bgColorElem.setAttribute( QStringLiteral( "green" ), QString::number( mBackgroundColor.green() ) );
  bgColorElem.setAttribute( QStringLiteral( "blue" ), QString::number( mBackgroundColor.blue() ) );
  bgColorElem.setAttribute( QStringLiteral( "alpha" ), QString::number( mBackgroundColor.alpha() ) );
  composerItemElem.appendChild( bgColorElem );

  //blend mode
  composerItemElem.setAttribute( QStringLiteral( "blendMode" ), QgsPainting::getBlendModeEnum( mBlendMode ) );

  //opacity
  composerItemElem.setAttribute( QStringLiteral( "opacity" ), QString::number( mOpacity ) );

  composerItemElem.setAttribute( QStringLiteral( "excludeFromExports" ), mExcludeFromExports );

  QgsComposerObject::writeXml( composerItemElem, doc );
  itemElem.appendChild( composerItemElem );

  return true;
}

bool QgsComposerItem::_readXml( const QDomElement &itemElem, const QDomDocument &doc )
{
  Q_UNUSED( doc );
  if ( itemElem.isNull() )
  {
    return false;
  }

  QgsComposerObject::readXml( itemElem, doc );

  //rotation
  setItemRotation( itemElem.attribute( QStringLiteral( "itemRotation" ), QStringLiteral( "0" ) ).toDouble() );

  //uuid
  mUuid = itemElem.attribute( QStringLiteral( "uuid" ), QUuid::createUuid().toString() );

  // temporary for groups imported from templates
  mTemplateUuid = itemElem.attribute( QStringLiteral( "templateUuid" ) );

  //id
  QString id = itemElem.attribute( QStringLiteral( "id" ), QLatin1String( "" ) );
  setId( id );

  //frame
  QString frame = itemElem.attribute( QStringLiteral( "frame" ) );
  if ( frame.compare( QLatin1String( "true" ), Qt::CaseInsensitive ) == 0 )
  {
    mFrame = true;
  }
  else
  {
    mFrame = false;
  }

  //frame
  QString background = itemElem.attribute( QStringLiteral( "background" ) );
  if ( background.compare( QLatin1String( "true" ), Qt::CaseInsensitive ) == 0 )
  {
    mBackground = true;
  }
  else
  {
    mBackground = false;
  }

  //position lock for mouse moves/resizes
  QString positionLock = itemElem.attribute( QStringLiteral( "positionLock" ) );
  if ( positionLock.compare( QLatin1String( "true" ), Qt::CaseInsensitive ) == 0 )
  {
    setPositionLock( true );
  }
  else
  {
    setPositionLock( false );
  }

  //visibility
  setVisibility( itemElem.attribute( QStringLiteral( "visibility" ), QStringLiteral( "1" ) ) != QLatin1String( "0" ) );

  //position
  int page;
  double x, y, pagex, pagey, width, height;
  bool xOk, yOk, pageOk, pagexOk, pageyOk, widthOk, heightOk, positionModeOK;

  x = itemElem.attribute( QStringLiteral( "x" ) ).toDouble( &xOk );
  y = itemElem.attribute( QStringLiteral( "y" ) ).toDouble( &yOk );
  page = itemElem.attribute( QStringLiteral( "page" ) ).toInt( &pageOk );
  pagex = itemElem.attribute( QStringLiteral( "pagex" ) ).toDouble( &pagexOk );
  pagey = itemElem.attribute( QStringLiteral( "pagey" ) ).toDouble( &pageyOk );
  width = itemElem.attribute( QStringLiteral( "width" ) ).toDouble( &widthOk );
  height = itemElem.attribute( QStringLiteral( "height" ) ).toDouble( &heightOk );
  mLastUsedPositionMode = static_cast< ItemPositionMode >( itemElem.attribute( QStringLiteral( "positionMode" ) ).toInt( &positionModeOK ) );
  if ( !positionModeOK )
  {
    mLastUsedPositionMode = UpperLeft;
  }
  if ( pageOk && pagexOk && pageyOk )
  {
    xOk = true;
    yOk = true;
    x = pagex;
    y = ( page - 1 ) * ( mComposition->paperHeight() + composition()->spaceBetweenPages() ) + pagey;
  }

  if ( !xOk || !yOk || !widthOk || !heightOk )
  {
    return false;
  }

  mLastValidViewScaleFactor = itemElem.attribute( QStringLiteral( "lastValidViewScaleFactor" ), QStringLiteral( "-1" ) ).toDouble();

  setZValue( itemElem.attribute( QStringLiteral( "zValue" ) ).toDouble() );

  QgsExpressionContext context = createExpressionContext();

  //pen
  QDomNodeList frameColorList = itemElem.elementsByTagName( QStringLiteral( "FrameColor" ) );
  if ( !frameColorList.isEmpty() )
  {
    QDomElement frameColorElem = frameColorList.at( 0 ).toElement();
    bool redOk, greenOk, blueOk, alphaOk, widthOk;
    int penRed, penGreen, penBlue, penAlpha;
    double penWidth;

    penWidth = itemElem.attribute( QStringLiteral( "outlineWidth" ) ).toDouble( &widthOk );
    penRed = frameColorElem.attribute( QStringLiteral( "red" ) ).toDouble( &redOk );
    penGreen = frameColorElem.attribute( QStringLiteral( "green" ) ).toDouble( &greenOk );
    penBlue = frameColorElem.attribute( QStringLiteral( "blue" ) ).toDouble( &blueOk );
    penAlpha = frameColorElem.attribute( QStringLiteral( "alpha" ) ).toDouble( &alphaOk );
    mFrameJoinStyle = QgsSymbolLayerUtils::decodePenJoinStyle( itemElem.attribute( QStringLiteral( "frameJoinStyle" ), QStringLiteral( "miter" ) ) );

    if ( redOk && greenOk && blueOk && alphaOk && widthOk )
    {
      mFrameColor = QColor( penRed, penGreen, penBlue, penAlpha );
      mFrameWidth = penWidth;
      QPen framePen( mFrameColor );
      framePen.setWidthF( mFrameWidth );
      framePen.setJoinStyle( mFrameJoinStyle );
      setPen( framePen );
      //apply any data defined settings
      refreshFrameColor( false, context );
    }
  }

  //brush
  QDomNodeList bgColorList = itemElem.elementsByTagName( QStringLiteral( "BackgroundColor" ) );
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
    refreshBackgroundColor( false, context );
  }

  //blend mode
  setBlendMode( QgsPainting::getCompositionMode( static_cast< QgsPainting::BlendMode >( itemElem.attribute( QStringLiteral( "blendMode" ), QStringLiteral( "0" ) ).toUInt() ) ) );

  //opacity
  if ( itemElem.hasAttribute( QStringLiteral( "opacity" ) ) )
  {
    setItemOpacity( itemElem.attribute( QStringLiteral( "opacity" ), QStringLiteral( "1" ) ).toDouble() );
  }
  else
  {
    setItemOpacity( 1.0 - itemElem.attribute( QStringLiteral( "transparency" ), QStringLiteral( "0" ) ).toInt() / 100.0 );
  }

  mExcludeFromExports = itemElem.attribute( QStringLiteral( "excludeFromExports" ), QStringLiteral( "0" ) ).toInt();
  mEvaluatedExcludeFromExports = mExcludeFromExports;

  QRectF evaluatedRect = evalItemRect( QRectF( x, y, width, height ) );
  setSceneRect( evaluatedRect );

  return true;
}

void QgsComposerItem::setFrameEnabled( const bool drawFrame )
{
  if ( drawFrame == mFrame )
  {
    //no change
    return;
  }

  mFrame = drawFrame;
  emit frameChanged();
}

void QgsComposerItem::setFrameStrokeColor( const QColor &color )
{
  if ( mFrameColor == color )
  {
    //no change
    return;
  }
  mFrameColor = color;
  QPen itemPen = pen();
  itemPen.setColor( mFrameColor );
  setPen( itemPen );
  // apply any datadefined overrides
  QgsExpressionContext context = createExpressionContext();
  refreshFrameColor( true, context );
  emit frameChanged();
}

void QgsComposerItem::setFrameStrokeWidth( const double strokeWidth )
{
  if ( qgsDoubleNear( mFrameWidth, strokeWidth ) )
  {
    //no change
    return;
  }
  mFrameWidth = strokeWidth;
  QPen itemPen = pen();
  itemPen.setWidthF( mFrameWidth );
  setPen( itemPen );
  emit frameChanged();
}

void QgsComposerItem::setFrameJoinStyle( const Qt::PenJoinStyle style )
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

double QgsComposerItem::estimatedFrameBleed() const
{
  if ( !hasFrame() )
  {
    return 0;
  }

  return pen().widthF() / 2.0;
}

QRectF QgsComposerItem::rectWithFrame() const
{
  double frameBleed = estimatedFrameBleed();
  return rect().adjusted( -frameBleed, -frameBleed, frameBleed, frameBleed );
}

void QgsComposerItem::beginCommand( const QString &commandText, QgsComposerMergeCommand::Context c )
{
  if ( mComposition )
  {
    mComposition->beginCommand( this, commandText, c );
  }
}

void QgsComposerItem::endCommand()
{
  if ( mComposition )
  {
    mComposition->endCommand();
  }
}

void QgsComposerItem::cancelCommand()
{
  if ( mComposition )
  {
    mComposition->cancelCommand();
  }
}

void QgsComposerItem::drawSelectionBoxes( QPainter *p )
{
  Q_UNUSED( p );
  if ( !mComposition || mComposition->plotStyle() != QgsComposition::Preview )
  {
    return;
  }

  if ( !isSelected() )
  {
    return;
  }

  //logic for drawing additional graphics on selected items here (if required)

  //draw dotted border around locked, selected items
  if ( positionLock() )
  {
    p->save();
    p->setCompositionMode( QPainter::CompositionMode_Difference );

    // use a grey dashed pen - in difference mode this should always be visible
    QPen selectedItemPen = QPen( QColor( 144, 144, 144, 255 ) );
    selectedItemPen.setStyle( Qt::DotLine );
    selectedItemPen.setWidth( 0 );
    p->setPen( selectedItemPen );
    p->setBrush( Qt::NoBrush );

    // drawPolygon causes issues on windows - corners of path may be missing resulting in triangles being drawn
    // instead of rectangles! (Same cause as #13343)
    QPainterPath path;
    path.addPolygon( rect() );
    p->drawPath( path );

    p->restore();
  }

}

void QgsComposerItem::drawFrame( QPainter *p )
{
  if ( mFrame && p )
  {
    p->save();
    p->setPen( pen() );
    p->setBrush( Qt::NoBrush );
    p->setRenderHint( QPainter::Antialiasing, true );
    p->drawRect( QRectF( 0, 0, rect().width(), rect().height() ) );
    p->restore();
  }
}

void QgsComposerItem::setPositionLock( const bool lock )
{
  if ( lock == mItemPositionLocked )
  {
    return;
  }

  mItemPositionLocked = lock;

  //inform model that id data has changed
  if ( mComposition )
  {
    mComposition->itemsModel()->updateItemLockStatus( this );
  }
  update();
  emit lockChanged();
}

double QgsComposerItem::itemRotation( const PropertyValueType valueType ) const
{
  return valueType == QgsComposerObject::EvaluatedValue ? mEvaluatedItemRotation : mItemRotation;
}

void QgsComposerItem::updateItem()
{
  if ( !mUpdatesEnabled )
    return;

  QGraphicsRectItem::update();
}

void QgsComposerItem::move( double dx, double dy )
{
  QRectF newSceneRect( pos().x() + dx, pos().y() + dy, rect().width(), rect().height() );
  setSceneRect( evalItemRect( newSceneRect ) );
}

int QgsComposerItem::page() const
{
  double y = pos().y();
  double h = composition()->paperHeight() + composition()->spaceBetweenPages();
  int page = 1;
  while ( y - h >= 0. )
  {
    y -= h;
    ++page;
  }
  return page;
}

QPointF QgsComposerItem::pagePos() const
{
  QPointF p = pos();
  double h = composition()->paperHeight() + composition()->spaceBetweenPages();
  p.ry() -= ( page() - 1 ) * h;
  return p;
}

void QgsComposerItem::updatePagePos( double newPageWidth, double newPageHeight )
{
  Q_UNUSED( newPageWidth )
  QPointF curPagePos = pagePos();
  int curPage = page() - 1;

  double y = curPage * ( newPageHeight + composition()->spaceBetweenPages() ) + curPagePos.y();
  QRectF newSceneRect( pos().x(), y, rect().width(), rect().height() );

  setSceneRect( evalItemRect( newSceneRect ) );
  emit sizeChanged();
}

void QgsComposerItem::setItemPosition( double x, double y, ItemPositionMode itemPoint, int page )
{
  double width = rect().width();
  double height = rect().height();
  setItemPosition( x, y, width, height, itemPoint, false, page );
}

void QgsComposerItem::setItemPosition( double x, double y, double width, double height, ItemPositionMode itemPoint, bool posIncludesFrame, int page )
{
  double upperLeftX = x;
  double upperLeftY = y;

  if ( page > 0 )
  {
    double h = composition()->paperHeight() + composition()->spaceBetweenPages();
    upperLeftY += ( page - 1 ) * h;
  }

  //store the item position mode
  mLastUsedPositionMode = itemPoint;

  //adjust x-coordinate if placement is not done to a left point
  if ( itemPoint == UpperMiddle || itemPoint == Middle || itemPoint == LowerMiddle )
  {
    upperLeftX -= width / 2.0;
  }
  else if ( itemPoint == UpperRight || itemPoint == MiddleRight || itemPoint == LowerRight )
  {
    upperLeftX -= width;
  }

  //adjust y-coordinate if placement is not done to an upper point
  if ( itemPoint == MiddleLeft || itemPoint == Middle || itemPoint == MiddleRight )
  {
    upperLeftY -= height / 2.0;
  }
  else if ( itemPoint == LowerLeft || itemPoint == LowerMiddle || itemPoint == LowerRight )
  {
    upperLeftY -= height;
  }

  if ( posIncludesFrame )
  {
    //adjust position to account for frame size

    if ( qgsDoubleNear( mEvaluatedItemRotation, 0.0 ) )
    {
      upperLeftX += estimatedFrameBleed();
      upperLeftY += estimatedFrameBleed();
    }
    else
    {
      //adjust position for item rotation
      QLineF lineToItemOrigin = QLineF( 0, 0, estimatedFrameBleed(), estimatedFrameBleed() );
      lineToItemOrigin.setAngle( -45 - mEvaluatedItemRotation );
      upperLeftX += lineToItemOrigin.x2();
      upperLeftY += lineToItemOrigin.y2();
    }

    width -= 2 * estimatedFrameBleed();
    height -= 2 * estimatedFrameBleed();
  }

  //consider data defined item size and position before finalising rect
  QRectF newRect = evalItemRect( QRectF( upperLeftX, upperLeftY, width, height ) );

  setSceneRect( newRect );
}

void QgsComposerItem::setSceneRect( const QRectF &rectangle )
{
  //setRect in item coordinates
  double newWidth = rectangle.width();
  double newHeight = rectangle.height();
  double xTranslation = rectangle.x();
  double yTranslation = rectangle.y();

  //correction if width and/or height are negative
  if ( rectangle.width() < 0 )
  {
    newWidth = - rectangle.width();
    xTranslation -= newWidth;
  }

  if ( rectangle.height() < 0 )
  {
    newHeight = - rectangle.height();
    yTranslation -= newHeight;
  }

  QGraphicsRectItem::setRect( QRectF( 0, 0, newWidth, newHeight ) );
  setPos( QPointF( xTranslation, yTranslation ) );

  emit sizeChanged();
}

QRectF QgsComposerItem::evalItemRect( const QRectF &newRect, const bool resizeOnly, const QgsExpressionContext *context )
{
  QRectF result = newRect;

  //TODO QGIS 3.0
  //maintain pre 2.12 API. remove when API break allowed
  QgsExpressionContext scopedContext = createExpressionContext();
  const QgsExpressionContext *evalContext = context ? context : &scopedContext;

  //data defined position or size set? if so, update rect with data defined values
  bool ok = false;
  double ddWidth = mDataDefinedProperties.valueAsDouble( QgsComposerObject::ItemWidth, *evalContext, 0, &ok );
  //evaulate width and height first, since they may affect position if non-top-left reference point set
  if ( ok )
  {
    result.setWidth( ddWidth );
  }
  double ddHeight = mDataDefinedProperties.valueAsDouble( QgsComposerObject::ItemHeight, *evalContext, 0, &ok );
  if ( ok )
  {
    result.setHeight( ddHeight );
  }

  double x = result.left();
  //initially adjust for position mode to get x coordinate
  if ( !resizeOnly )
  {
    //adjust x-coordinate if placement is not done to a left point
    if ( mLastUsedPositionMode == UpperMiddle || mLastUsedPositionMode == Middle || mLastUsedPositionMode == LowerMiddle )
    {
      x += newRect.width() / 2.0;
    }
    else if ( mLastUsedPositionMode == UpperRight || mLastUsedPositionMode == MiddleRight || mLastUsedPositionMode == LowerRight )
    {
      x += newRect.width();
    }
  }
  else
  {
    if ( mLastUsedPositionMode == UpperMiddle || mLastUsedPositionMode == Middle || mLastUsedPositionMode == LowerMiddle )
    {
      x += rect().width() / 2.0;
    }
    else if ( mLastUsedPositionMode == UpperRight || mLastUsedPositionMode == MiddleRight || mLastUsedPositionMode == LowerRight )
    {
      x += rect().width();
    }
  }
  double ddPosX = mDataDefinedProperties.valueAsDouble( QgsComposerObject::PositionX, *evalContext, 0.0, &ok );
  if ( ok )
  {
    x = ddPosX;
  }

  double y = result.top();
  //initially adjust for position mode to get y coordinate
  if ( !resizeOnly )
  {
    //adjust y-coordinate if placement is not done to an upper point
    if ( mLastUsedPositionMode == MiddleLeft || mLastUsedPositionMode == Middle || mLastUsedPositionMode == MiddleRight )
    {
      y += newRect.height() / 2.0;
    }
    else if ( mLastUsedPositionMode == LowerLeft || mLastUsedPositionMode == LowerMiddle || mLastUsedPositionMode == LowerRight )
    {
      y += newRect.height();
    }
  }
  else
  {
    if ( mLastUsedPositionMode == MiddleLeft || mLastUsedPositionMode == Middle || mLastUsedPositionMode == MiddleRight )
    {
      y += rect().height() / 2.0;
    }
    else if ( mLastUsedPositionMode == LowerLeft || mLastUsedPositionMode == LowerMiddle || mLastUsedPositionMode == LowerRight )
    {
      y += rect().height();
    }
  }
  double ddPosY = mDataDefinedProperties.valueAsDouble( QgsComposerObject::PositionY, *evalContext, 0, &ok );
  if ( ok )
  {
    y = ddPosY;
  }

  //adjust x-coordinate if placement is not done to a left point
  if ( mLastUsedPositionMode == UpperMiddle || mLastUsedPositionMode == Middle || mLastUsedPositionMode == LowerMiddle )
  {
    x -= result.width() / 2.0;
  }
  else if ( mLastUsedPositionMode == UpperRight || mLastUsedPositionMode == MiddleRight || mLastUsedPositionMode == LowerRight )
  {
    x -= result.width();
  }

  //adjust y-coordinate if placement is not done to an upper point
  if ( mLastUsedPositionMode == MiddleLeft || mLastUsedPositionMode == Middle || mLastUsedPositionMode == MiddleRight )
  {
    y -= result.height() / 2.0;
  }
  else if ( mLastUsedPositionMode == LowerLeft || mLastUsedPositionMode == LowerMiddle || mLastUsedPositionMode == LowerRight )
  {
    y -= result.height();
  }

  result.moveLeft( x );
  result.moveTop( y );

  return result;
}

bool QgsComposerItem::shouldDrawItem() const
{
  if ( ( mComposition && mComposition->plotStyle() == QgsComposition::Preview ) || !mComposition )
  {
    //preview mode or no composition, so OK to draw item
    return true;
  }

  //exporting composition, so check if item is excluded from exports
  return !mEvaluatedExcludeFromExports;
}

QgsExpressionContext QgsComposerItem::createExpressionContext() const
{
  QgsExpressionContext context = QgsComposerObject::createExpressionContext();
  context.appendScope( QgsExpressionContextUtils::composerItemScope( this ) );
  return context;
}

void QgsComposerItem::drawBackground( QPainter *p )
{
  if ( mBackground && p )
  {
    p->save();
    p->setBrush( brush() );//this causes a problem in atlas generation
    p->setPen( Qt::NoPen );
    p->setRenderHint( QPainter::Antialiasing, true );
    p->drawRect( QRectF( 0, 0, rect().width(), rect().height() ) );
    p->restore();
  }
}

void QgsComposerItem::setBackgroundColor( const QColor &backgroundColor )
{
  mBackgroundColor = backgroundColor;
  setBrush( QBrush( mBackgroundColor, Qt::SolidPattern ) );
  // apply any datadefined overrides
  QgsExpressionContext context = createExpressionContext();
  refreshBackgroundColor( true, context );
}

void QgsComposerItem::setBlendMode( const QPainter::CompositionMode blendMode )
{
  mBlendMode = blendMode;
  // Update the composer effect to use the new blend mode
  QgsExpressionContext context = createExpressionContext();
  refreshBlendMode( context );
}

void QgsComposerItem::refreshBlendMode( const QgsExpressionContext &context )
{
  QPainter::CompositionMode blendMode = mBlendMode;

  //data defined blend mode set?
  bool ok = false;
  QString blendStr = mDataDefinedProperties.valueAsString( QgsComposerObject::BlendMode, context, QString(), &ok );
  if ( ok && !blendStr.isEmpty() )
  {
    QString blendstr = blendStr.trimmed();
    QPainter::CompositionMode blendModeD = QgsSymbolLayerUtils::decodeBlendMode( blendstr );

    QgsDebugMsg( QString( "exprVal BlendMode:%1" ).arg( blendModeD ) );
    blendMode = blendModeD;
  }

  // Update the composer effect to use the new blend mode
  mEffect->setCompositionMode( blendMode );
}

void QgsComposerItem::setItemOpacity( const double opacity )
{
  mOpacity = opacity;
  QgsExpressionContext context = createExpressionContext();
  refreshOpacity( true, context );
}

void QgsComposerItem::refreshOpacity( const bool updateItem, const QgsExpressionContext &context )
{
  //data defined opacity set?
  double opacity = mDataDefinedProperties.valueAsDouble( QgsComposerObject::Opacity, context, mOpacity * 100.0 );

  // Set the QGraphicItem's opacity
  setOpacity( opacity / 100.0 );

  if ( updateItem )
  {
    update();
  }
}

void QgsComposerItem::refreshFrameColor( const bool updateItem, const QgsExpressionContext &context )
{
  //data defined stroke color set?
  bool ok = false;
  QColor frameColor = mDataDefinedProperties.valueAsColor( QgsComposerObject::FrameColor, context, mFrameColor, &ok );
  if ( ok )
  {
    QPen itemPen = pen();
    itemPen.setColor( frameColor );
    setPen( itemPen );
  }
  else
  {
    QPen itemPen = pen();
    itemPen.setColor( mFrameColor );
    setPen( itemPen );
  }
  if ( updateItem )
  {
    update();
  }
}

void QgsComposerItem::refreshBackgroundColor( const bool updateItem, const QgsExpressionContext &context )
{
  //data defined color set?
  bool ok = false;
  QColor backgroundColor = mDataDefinedProperties.valueAsColor( QgsComposerObject::BackgroundColor, context, mBackgroundColor, &ok );
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

void QgsComposerItem::setEffectsEnabled( const bool effectsEnabled )
{
  //enable or disable the QgsComposerEffect applied to this item
  mEffectsEnabled = effectsEnabled;
  mEffect->setEnabled( effectsEnabled );
}

double QgsComposerItem::horizontalViewScaleFactor() const
{
  double result = -1;
  if ( scene() )
  {
    QList<QGraphicsView *> viewList = scene()->views();
    if ( !viewList.isEmpty() ) //if not, probably this function was called from non-gui code
    {
      QGraphicsView *currentView = viewList.at( 0 );
      if ( currentView->isVisible() )
      {
        result = currentView->transform().m11();
        mLastValidViewScaleFactor = result;
      }
    }
  }
  return result;
}

double QgsComposerItem::rectHandlerBorderTolerance() const
{
  //size of symbol boxes depends on zoom level in composer view
  double viewScaleFactor = horizontalViewScaleFactor();
  double rectHandlerSize = 10.0 / viewScaleFactor;

  //make sure the boxes don't get too large
  if ( rectHandlerSize > ( rect().width() / 3 ) )
  {
    rectHandlerSize = rect().width() / 3;
  }
  if ( rectHandlerSize > ( rect().height() / 3 ) )
  {
    rectHandlerSize = rect().height() / 3;
  }
  return rectHandlerSize;
}

void QgsComposerItem::setItemRotation( const double r, const bool adjustPosition )
{
  mItemRotation = r;
  if ( mItemRotation >= 360.0 || mItemRotation <= -360.0 )
  {
    mItemRotation = std::fmod( mItemRotation, 360.0 );
  }

  QgsExpressionContext context = createExpressionContext();
  refreshRotation( true, adjustPosition, context );
}

void QgsComposerItem::refreshRotation( const bool updateItem, const bool adjustPosition, const QgsExpressionContext &context )
{
  double rotation = mItemRotation;

  //data defined rotation set?
  rotation = mDataDefinedProperties.valueAsDouble( QgsComposerObject::ItemRotation, context, rotation );

  if ( qgsDoubleNear( rotation, mEvaluatedItemRotation ) )
  {
    return;
  }

  if ( adjustPosition )
  {
    //adjustPosition set, so shift the position of the item so that rotation occurs around item center
    //create a line from the centrepoint of the rect() to its origin, in scene coordinates
    QLineF refLine = QLineF( mapToScene( QPointF( rect().width() / 2.0, rect().height() / 2.0 ) ), mapToScene( QPointF( 0, 0 ) ) );
    //rotate this line by the current rotation angle
    refLine.setAngle( refLine.angle() - rotation + mEvaluatedItemRotation );
    //get new end point of line - this is the new item position
    QPointF rotatedReferencePoint = refLine.p2();
    setPos( rotatedReferencePoint );
    emit sizeChanged();
  }

  setTransformOriginPoint( 0, 0 );
  QGraphicsItem::setRotation( rotation );

  mEvaluatedItemRotation = rotation;

  emit itemRotationChanged( rotation );

  //update bounds of scene, since rotation may affect this
  mComposition->updateBounds();

  if ( updateItem )
  {
    update();
  }
}

QGraphicsLineItem *QgsComposerItem::hAlignSnapItem()
{
  if ( !mHAlignSnapItem )
  {
    mHAlignSnapItem = new QGraphicsLineItem( nullptr );
    QPen pen = QPen( QColor( Qt::red ) );
    pen.setWidthF( 0.0 );
    mHAlignSnapItem->setPen( pen );
    scene()->addItem( mHAlignSnapItem );
    mHAlignSnapItem->setZValue( 90 );
  }
  return mHAlignSnapItem;
}

QGraphicsLineItem *QgsComposerItem::vAlignSnapItem()
{
  if ( !mVAlignSnapItem )
  {
    mVAlignSnapItem = new QGraphicsLineItem( nullptr );
    QPen pen = QPen( QColor( Qt::red ) );
    pen.setWidthF( 0.0 );
    mVAlignSnapItem->setPen( pen );
    scene()->addItem( mVAlignSnapItem );
    mVAlignSnapItem->setZValue( 90 );
  }
  return mVAlignSnapItem;
}

void QgsComposerItem::deleteHAlignSnapItem()
{
  if ( mHAlignSnapItem )
  {
    scene()->removeItem( mHAlignSnapItem );
    delete mHAlignSnapItem;
    mHAlignSnapItem = nullptr;
  }
}

void QgsComposerItem::deleteVAlignSnapItem()
{
  if ( mVAlignSnapItem )
  {
    scene()->removeItem( mVAlignSnapItem );
    delete mVAlignSnapItem;
    mVAlignSnapItem = nullptr;
  }
}

void QgsComposerItem::deleteAlignItems()
{
  deleteHAlignSnapItem();
  deleteVAlignSnapItem();
}

void QgsComposerItem::repaint()
{
  updateItem();
}

void QgsComposerItem::refreshDataDefinedProperty( const QgsComposerObject::DataDefinedProperty property, const QgsExpressionContext *context )
{
  //maintain 2.10 API
  //TODO QGIS 3.0 - remove this
  QgsExpressionContext scopedContext = createExpressionContext();
  const QgsExpressionContext *evalContext = context ? context : &scopedContext;

  //update data defined properties and redraw item to match
  if ( property == QgsComposerObject::PositionX || property == QgsComposerObject::PositionY ||
       property == QgsComposerObject::ItemWidth || property == QgsComposerObject::ItemHeight ||
       property == QgsComposerObject::AllProperties )
  {
    QRectF beforeRect = QRectF( pos().x(), pos().y(), rect().width(), rect().height() );
    QRectF evaluatedRect = evalItemRect( beforeRect, false, evalContext );
    if ( evaluatedRect != beforeRect )
    {
      setSceneRect( evaluatedRect );
    }
  }
  if ( property == QgsComposerObject::ItemRotation || property == QgsComposerObject::AllProperties )
  {
    refreshRotation( false, true, *evalContext );
  }
  if ( property == QgsComposerObject::Opacity || property == QgsComposerObject::AllProperties )
  {
    refreshOpacity( false, *evalContext );
  }
  if ( property == QgsComposerObject::BlendMode || property == QgsComposerObject::AllProperties )
  {
    refreshBlendMode( *evalContext );
  }
  if ( property == QgsComposerObject::FrameColor || property == QgsComposerObject::AllProperties )
  {
    refreshFrameColor( false, *evalContext );
  }
  if ( property == QgsComposerObject::BackgroundColor || property == QgsComposerObject::AllProperties )
  {
    refreshBackgroundColor( false, *evalContext );
  }
  if ( property == QgsComposerObject::ExcludeFromExports || property == QgsComposerObject::AllProperties )
  {
    bool exclude = mExcludeFromExports;
    //data defined exclude from exports set?
    exclude = mDataDefinedProperties.valueAsBool( QgsComposerObject::ExcludeFromExports, *evalContext, exclude );
    mEvaluatedExcludeFromExports = exclude;
  }

  update();
}

void QgsComposerItem::setId( const QString &id )
{
  if ( id == mId )
  {
    return;
  }

  setToolTip( id );
  mId = id;

  //inform model that id data has changed
  if ( mComposition )
  {
    mComposition->itemsModel()->updateItemDisplayName( this );
  }

  emit itemChanged();
}

void QgsComposerItem::setIsGroupMember( const bool isGroupMember )
{
  mIsGroupMember = isGroupMember;
  setFlag( QGraphicsItem::ItemIsSelectable, !isGroupMember ); //item in groups cannot be selected
}

QString QgsComposerItem::displayName() const
{
  //return id, if it's not empty
  if ( ! id().isEmpty() )
  {
    return id();
  }

  //for unnamed items, default to item type
  //(note some item types override this method to provide their own defaults)
  switch ( type() )
  {
    case ComposerArrow:
      return tr( "<arrow>" );
    case ComposerItemGroup:
      return tr( "<group>" );
    case ComposerLabel:
      return tr( "<label>" );
    case ComposerLegend:
      return tr( "<legend>" );
    case ComposerMap:
      return tr( "<map>" );
    case ComposerPicture:
      return tr( "<picture>" );
    case ComposerScaleBar:
      return tr( "<scale bar>" );
    case ComposerShape:
      return tr( "<shape>" );
    case ComposerTable:
      return tr( "<table>" );
    case ComposerAttributeTable:
      return tr( "<attribute table>" );
    case ComposerTextTable:
      return tr( "<text table>" );
    case ComposerFrame:
      return tr( "<frame>" );
  }

  return tr( "<item>" );
}

void QgsComposerItem::setVisibility( const bool visible )
{
  if ( visible == isVisible() )
  {
    //nothing to do
    return;
  }

  QGraphicsItem::setVisible( visible );

  //inform model that id data has changed
  if ( mComposition )
  {
    mComposition->itemsModel()->updateItemVisibility( this );
  }
}

bool QgsComposerItem::excludeFromExports( const QgsComposerObject::PropertyValueType valueType )
{
  return valueType == QgsComposerObject::EvaluatedValue ? mEvaluatedExcludeFromExports : mExcludeFromExports;
}

void QgsComposerItem::setExcludeFromExports( const bool exclude )
{
  mExcludeFromExports = exclude;
  refreshDataDefinedProperty( QgsComposerObject::ExcludeFromExports );
}
