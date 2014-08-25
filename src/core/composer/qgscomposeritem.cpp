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
#include "qgsdatadefined.h"
#include "qgscomposerutils.h"
#include "qgscomposermodel.h"

#include <limits>
#include "qgsapplication.h"
#include "qgsrectangle.h" //just for debugging
#include "qgslogger.h"
#include "qgssymbollayerv2utils.h" //for pointOnLineWithDistance
#include "qgsmaprenderer.h" //for getCompositionMode

#include <cmath>

#define FONT_WORKAROUND_SCALE 10 //scale factor for upscaling fontsize and downscaling painter

#ifndef M_DEG2RAD
#define M_DEG2RAD 0.0174532925
#endif

QgsComposerItem::QgsComposerItem( QgsComposition* composition, bool manageZValue )
    : QgsComposerObject( composition )
    , QGraphicsRectItem( 0 )
    , mRemovedFromComposition( false )
    , mBoundingResizeRectangle( 0 )
    , mHAlignSnapItem( 0 )
    , mVAlignSnapItem( 0 )
    , mFrame( false )
    , mBackground( true )
    , mBackgroundColor( QColor( 255, 255, 255, 255 ) )
    , mFrameJoinStyle( Qt::MiterJoin )
    , mItemPositionLocked( false )
    , mLastValidViewScaleFactor( -1 )
    , mItemRotation( 0 )
    , mEvaluatedItemRotation( 0 )
    , mBlendMode( QPainter::CompositionMode_SourceOver )
    , mEffectsEnabled( true )
    , mTransparency( 0 )
    , mLastUsedPositionMode( UpperLeft )
    , mIsGroupMember( false )
    , mCurrentExportLayer( -1 )
    , mId( "" )
    , mUuid( QUuid::createUuid().toString() )
{
  init( manageZValue );
}

QgsComposerItem::QgsComposerItem( qreal x, qreal y, qreal width, qreal height, QgsComposition* composition, bool manageZValue )
    : QgsComposerObject( composition )
    , QGraphicsRectItem( 0, 0, width, height, 0 )
    , mRemovedFromComposition( false )
    , mBoundingResizeRectangle( 0 )
    , mHAlignSnapItem( 0 )
    , mVAlignSnapItem( 0 )
    , mFrame( false )
    , mBackground( true )
    , mBackgroundColor( QColor( 255, 255, 255, 255 ) )
    , mFrameJoinStyle( Qt::MiterJoin )
    , mItemPositionLocked( false )
    , mLastValidViewScaleFactor( -1 )
    , mItemRotation( 0 )
    , mEvaluatedItemRotation( 0 )
    , mBlendMode( QPainter::CompositionMode_SourceOver )
    , mEffectsEnabled( true )
    , mTransparency( 0 )
    , mLastUsedPositionMode( UpperLeft )
    , mIsGroupMember( false )
    , mCurrentExportLayer( -1 )
    , mId( "" )
    , mUuid( QUuid::createUuid().toString() )
{
  init( manageZValue );
  setPos( x, y );
}

void QgsComposerItem::init( const bool manageZValue )
{
  setFlag( QGraphicsItem::ItemIsSelectable, true );
  //set default pen and brush
  setBrush( QBrush( QColor( 255, 255, 255, 255 ) ) );
  QPen defaultPen( QColor( 0, 0, 0 ) );
  defaultPen.setWidthF( 0.3 );
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

  // data defined strings
  mDataDefinedNames.insert( QgsComposerObject::PageNumber, QString( "dataDefinedPageNumber" ) );
  mDataDefinedNames.insert( QgsComposerObject::PositionX, QString( "dataDefinedPositionX" ) );
  mDataDefinedNames.insert( QgsComposerObject::PositionY, QString( "dataDefinedPositionY" ) );
  mDataDefinedNames.insert( QgsComposerObject::ItemWidth, QString( "dataDefinedWidth" ) );
  mDataDefinedNames.insert( QgsComposerObject::ItemHeight, QString( "dataDefinedHeight" ) );
  mDataDefinedNames.insert( QgsComposerObject::ItemRotation, QString( "dataDefinedRotation" ) );
  mDataDefinedNames.insert( QgsComposerObject::Transparency, QString( "dataDefinedTransparency" ) );
  mDataDefinedNames.insert( QgsComposerObject::BlendMode, QString( "dataDefinedBlendMode" ) );

  if ( mComposition )
  {
    //connect to atlas toggling on/off and coverage layer and feature changes
    //to update data defined values
    connect( &mComposition->atlasComposition(), SIGNAL( toggled( bool ) ), this, SLOT( refreshDataDefinedProperty() ) );
    connect( &mComposition->atlasComposition(), SIGNAL( coverageLayerChanged( QgsVectorLayer* ) ), this, SLOT( refreshDataDefinedProperty() ) );
    connect( &mComposition->atlasComposition(), SIGNAL( featureChanged( QgsFeature* ) ), this, SLOT( refreshDataDefinedProperty() ) );
    //also, refreshing composition triggers a recalculation of data defined properties
    connect( mComposition, SIGNAL( refreshItemsTriggered() ), this, SLOT( refreshDataDefinedProperty() ) );

    //toggling atlas or changing coverage layer requires data defined expressions to be reprepared
    connect( &mComposition->atlasComposition(), SIGNAL( toggled( bool ) ), this, SLOT( prepareDataDefinedExpressions() ) );
    connect( &mComposition->atlasComposition(), SIGNAL( coverageLayerChanged( QgsVectorLayer* ) ), this, SLOT( prepareDataDefinedExpressions() ) );
  }
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
  QgsDebugMsg( "entered." );
  QGraphicsRectItem::setSelected( s );
  //inform model that id data has changed
  if ( mComposition )
  {
    mComposition->itemsModel()->updateItemSelectStatus( this );
  }
  update(); //to draw selection boxes
}

bool QgsComposerItem::_writeXML( QDomElement& itemElem, QDomDocument& doc ) const
{
  if ( itemElem.isNull() )
  {
    return false;
  }

  QDomElement composerItemElem = doc.createElement( "ComposerItem" );

  //frame
  if ( mFrame )
  {
    composerItemElem.setAttribute( "frame", "true" );
  }
  else
  {
    composerItemElem.setAttribute( "frame", "false" );
  }

  //background
  if ( mBackground )
  {
    composerItemElem.setAttribute( "background", "true" );
  }
  else
  {
    composerItemElem.setAttribute( "background", "false" );
  }

  //scene rect
  QPointF pagepos = pagePos();
  composerItemElem.setAttribute( "x", QString::number( pos().x() ) );
  composerItemElem.setAttribute( "y", QString::number( pos().y() ) );
  composerItemElem.setAttribute( "page", page() );
  composerItemElem.setAttribute( "pagex", QString::number( pagepos.x() ) );
  composerItemElem.setAttribute( "pagey", QString::number( pagepos.y() ) );
  composerItemElem.setAttribute( "width", QString::number( rect().width() ) );
  composerItemElem.setAttribute( "height", QString::number( rect().height() ) );
  composerItemElem.setAttribute( "positionMode", QString::number(( int ) mLastUsedPositionMode ) );
  composerItemElem.setAttribute( "zValue", QString::number( zValue() ) );
  composerItemElem.setAttribute( "outlineWidth", QString::number( pen().widthF() ) );
  composerItemElem.setAttribute( "frameJoinStyle", QgsSymbolLayerV2Utils::encodePenJoinStyle( mFrameJoinStyle ) );
  composerItemElem.setAttribute( "itemRotation",  QString::number( mItemRotation ) );
  composerItemElem.setAttribute( "uuid", mUuid );
  composerItemElem.setAttribute( "id", mId );
  composerItemElem.setAttribute( "visibility", isVisible() );
  //position lock for mouse moves/resizes
  if ( mItemPositionLocked )
  {
    composerItemElem.setAttribute( "positionLock", "true" );
  }
  else
  {
    composerItemElem.setAttribute( "positionLock", "false" );
  }

  composerItemElem.setAttribute( "lastValidViewScaleFactor", QString::number( mLastValidViewScaleFactor ) );

  //frame color
  QDomElement frameColorElem = doc.createElement( "FrameColor" );
  QColor frameColor = pen().color();
  frameColorElem.setAttribute( "red", QString::number( frameColor.red() ) );
  frameColorElem.setAttribute( "green", QString::number( frameColor.green() ) );
  frameColorElem.setAttribute( "blue", QString::number( frameColor.blue() ) );
  frameColorElem.setAttribute( "alpha", QString::number( frameColor.alpha() ) );
  composerItemElem.appendChild( frameColorElem );

  //background color
  QDomElement bgColorElem = doc.createElement( "BackgroundColor" );
  QColor bgColor = brush().color();
  bgColorElem.setAttribute( "red", QString::number( bgColor.red() ) );
  bgColorElem.setAttribute( "green", QString::number( bgColor.green() ) );
  bgColorElem.setAttribute( "blue", QString::number( bgColor.blue() ) );
  bgColorElem.setAttribute( "alpha", QString::number( bgColor.alpha() ) );
  composerItemElem.appendChild( bgColorElem );

  //blend mode
  composerItemElem.setAttribute( "blendMode", QgsMapRenderer::getBlendModeEnum( mBlendMode ) );

  //transparency
  composerItemElem.setAttribute( "transparency", QString::number( mTransparency ) );

  QgsComposerObject::writeXML( composerItemElem, doc );
  itemElem.appendChild( composerItemElem );

  return true;
}

bool QgsComposerItem::_readXML( const QDomElement& itemElem, const QDomDocument& doc )
{
  Q_UNUSED( doc );
  if ( itemElem.isNull() )
  {
    return false;
  }

  QgsComposerObject::readXML( itemElem, doc );

  //rotation
  setItemRotation( itemElem.attribute( "itemRotation", "0" ).toDouble() );

  //uuid
  mUuid = itemElem.attribute( "uuid", QUuid::createUuid().toString() );

  // temporary for groups imported from templates
  mTemplateUuid = itemElem.attribute( "templateUuid" );

  //id
  QString id = itemElem.attribute( "id", "" );
  setId( id );

  //frame
  QString frame = itemElem.attribute( "frame" );
  if ( frame.compare( "true", Qt::CaseInsensitive ) == 0 )
  {
    mFrame = true;
  }
  else
  {
    mFrame = false;
  }

  //frame
  QString background = itemElem.attribute( "background" );
  if ( background.compare( "true", Qt::CaseInsensitive ) == 0 )
  {
    mBackground = true;
  }
  else
  {
    mBackground = false;
  }

  //position lock for mouse moves/resizes
  QString positionLock = itemElem.attribute( "positionLock" );
  if ( positionLock.compare( "true", Qt::CaseInsensitive ) == 0 )
  {
    setPositionLock( true );
  }
  else
  {
    setPositionLock( false );
  }

  //visibility
  setVisibility( itemElem.attribute( "visibility", "1" ) != "0" );

  //position
  int page;
  double x, y, pagex, pagey, width, height;
  bool xOk, yOk, pageOk, pagexOk, pageyOk, widthOk, heightOk, positionModeOK;

  x = itemElem.attribute( "x" ).toDouble( &xOk );
  y = itemElem.attribute( "y" ).toDouble( &yOk );
  page = itemElem.attribute( "page" ).toInt( &pageOk );
  pagex = itemElem.attribute( "pagex" ).toDouble( &pagexOk );
  pagey = itemElem.attribute( "pagey" ).toDouble( &pageyOk );
  width = itemElem.attribute( "width" ).toDouble( &widthOk );
  height = itemElem.attribute( "height" ).toDouble( &heightOk );
  mLastUsedPositionMode = ( ItemPositionMode )itemElem.attribute( "positionMode" ).toInt( &positionModeOK );
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

  mLastValidViewScaleFactor = itemElem.attribute( "lastValidViewScaleFactor", "-1" ).toDouble();

  setZValue( itemElem.attribute( "zValue" ).toDouble() );

  //pen
  QDomNodeList frameColorList = itemElem.elementsByTagName( "FrameColor" );
  if ( frameColorList.size() > 0 )
  {
    QDomElement frameColorElem = frameColorList.at( 0 ).toElement();
    bool redOk, greenOk, blueOk, alphaOk, widthOk;
    int penRed, penGreen, penBlue, penAlpha;
    double penWidth;

    penWidth = itemElem.attribute( "outlineWidth" ).toDouble( &widthOk );
    penRed = frameColorElem.attribute( "red" ).toDouble( &redOk );
    penGreen = frameColorElem.attribute( "green" ).toDouble( &greenOk );
    penBlue = frameColorElem.attribute( "blue" ).toDouble( &blueOk );
    penAlpha = frameColorElem.attribute( "alpha" ).toDouble( &alphaOk );
    mFrameJoinStyle = QgsSymbolLayerV2Utils::decodePenJoinStyle( itemElem.attribute( "frameJoinStyle", "miter" ) );

    if ( redOk && greenOk && blueOk && alphaOk && widthOk )
    {
      QPen framePen( QColor( penRed, penGreen, penBlue, penAlpha ) );
      framePen.setWidthF( penWidth );
      framePen.setJoinStyle( mFrameJoinStyle );
      setPen( framePen );
    }
  }

  //brush
  QDomNodeList bgColorList = itemElem.elementsByTagName( "BackgroundColor" );
  if ( bgColorList.size() > 0 )
  {
    QDomElement bgColorElem = bgColorList.at( 0 ).toElement();
    bool redOk, greenOk, blueOk, alphaOk;
    int bgRed, bgGreen, bgBlue, bgAlpha;
    bgRed = bgColorElem.attribute( "red" ).toDouble( &redOk );
    bgGreen = bgColorElem.attribute( "green" ).toDouble( &greenOk );
    bgBlue = bgColorElem.attribute( "blue" ).toDouble( &blueOk );
    bgAlpha = bgColorElem.attribute( "alpha" ).toDouble( &alphaOk );
    if ( redOk && greenOk && blueOk && alphaOk )
    {
      QColor brushColor( bgRed, bgGreen, bgBlue, bgAlpha );
      setBackgroundColor( brushColor );
    }
  }

  //blend mode
  setBlendMode( QgsMapRenderer::getCompositionMode(( QgsMapRenderer::BlendMode ) itemElem.attribute( "blendMode", "0" ).toUInt() ) );

  //transparency
  setTransparency( itemElem.attribute( "transparency" , "0" ).toInt() );

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

void QgsComposerItem::setFrameOutlineWidth( const double outlineWidth )
{
  QPen itemPen = pen();
  if ( itemPen.widthF() == outlineWidth )
  {
    //no change
    return;
  }
  itemPen.setWidthF( outlineWidth );
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

void QgsComposerItem::beginCommand( const QString& commandText, QgsComposerMergeCommand::Context c )
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

void QgsComposerItem::drawSelectionBoxes( QPainter* p )
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
    p->drawPolygon( rect() );
    p->restore();
  }

}

void QgsComposerItem::drawFrame( QPainter* p )
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

    if ( mEvaluatedItemRotation == 0 )
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

void QgsComposerItem::setSceneRect( const QRectF& rectangle )
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

QRectF QgsComposerItem::evalItemRect( const QRectF &newRect )
{
  QRectF result = newRect;

  //data defined position or size set? if so, update rect with data defined values
  QVariant exprVal;
  //evaulate width and height first, since they may affect position if non-top-left reference point set
  if ( dataDefinedEvaluate( QgsComposerObject::ItemWidth, exprVal ) )
  {
    bool ok;
    double width = exprVal.toDouble( &ok );
    QgsDebugMsg( QString( "exprVal Width:%1" ).arg( width ) );
    if ( ok )
    {
      result.setWidth( width );
    }
  }
  if ( dataDefinedEvaluate( QgsComposerObject::ItemHeight, exprVal ) )
  {
    bool ok;
    double height = exprVal.toDouble( &ok );
    QgsDebugMsg( QString( "exprVal Height:%1" ).arg( height ) );
    if ( ok )
    {
      result.setHeight( height );
    }
  }

  double x = result.left();
  //initially adjust for position mode to get top-left coordinate
  if ( mLastUsedPositionMode == UpperMiddle || mLastUsedPositionMode == Middle || mLastUsedPositionMode == LowerMiddle )
  {
    x += newRect.width() / 2.0;
  }
  else if ( mLastUsedPositionMode == UpperRight || mLastUsedPositionMode == MiddleRight || mLastUsedPositionMode == LowerRight )
  {
    x += newRect.width();
  }
  if ( dataDefinedEvaluate( QgsComposerObject::PositionX, exprVal ) )
  {
    bool ok;
    double positionX = exprVal.toDouble( &ok );
    QgsDebugMsg( QString( "exprVal Position X:%1" ).arg( positionX ) );
    if ( ok )
    {
      x = positionX;
    }
  }

  double y = result.top();
  //adjust y-coordinate if placement is not done to an upper point
  if ( mLastUsedPositionMode == MiddleLeft || mLastUsedPositionMode == Middle || mLastUsedPositionMode == MiddleRight )
  {
    y += newRect.height() / 2.0;
  }
  else if ( mLastUsedPositionMode == LowerLeft || mLastUsedPositionMode == LowerMiddle || mLastUsedPositionMode == LowerRight )
  {
    y += newRect.height();
  }

  if ( dataDefinedEvaluate( QgsComposerObject::PositionY, exprVal ) )
  {
    bool ok;
    double positionY = exprVal.toDouble( &ok );
    QgsDebugMsg( QString( "exprVal Position Y:%1" ).arg( positionY ) );
    if ( ok )
    {
      y = positionY;
    }
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

void QgsComposerItem::drawBackground( QPainter* p )
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

void QgsComposerItem::drawArrowHead( QPainter *p, double x, double y, double angle, double arrowHeadWidth ) const
{
  QgsComposerUtils::drawArrowHead( p, x, y, angle, arrowHeadWidth );
}

double QgsComposerItem::angle( const QPointF &p1, const QPointF &p2 ) const
{
  return QgsComposerUtils::angle( p1, p2 );
}

void QgsComposerItem::setBackgroundColor( const QColor& backgroundColor )
{
  mBackgroundColor = backgroundColor;
  setBrush( QBrush( mBackgroundColor, Qt::SolidPattern ) );
}

void QgsComposerItem::setBlendMode( const QPainter::CompositionMode blendMode )
{
  mBlendMode = blendMode;
  // Update the composer effect to use the new blend mode
  refreshBlendMode();
}

void QgsComposerItem::refreshBlendMode()
{
  QPainter::CompositionMode blendMode = mBlendMode;

  //data defined blend mode set?
  QVariant exprVal;
  if ( dataDefinedEvaluate( QgsComposerObject::BlendMode, exprVal ) )
  {
    QString blendstr = exprVal.toString().trimmed();
    QPainter::CompositionMode blendModeD = QgsSymbolLayerV2Utils::decodeBlendMode( blendstr );

    QgsDebugMsg( QString( "exprVal BlendMode:%1" ).arg( blendModeD ) );
    blendMode = blendModeD;
  }

  // Update the composer effect to use the new blend mode
  mEffect->setCompositionMode( blendMode );
}

void QgsComposerItem::setTransparency( const int transparency )
{
  mTransparency = transparency;
  refreshTransparency( true );
}

void QgsComposerItem::refreshTransparency( const bool updateItem )
{
  int transparency = mTransparency;

  //data defined transparency set?
  QVariant exprVal;
  if ( dataDefinedEvaluate( QgsComposerObject::Transparency, exprVal ) )
  {
    bool ok;
    int transparencyD = exprVal.toInt( &ok );
    QgsDebugMsg( QString( "exprVal Transparency:%1" ).arg( transparencyD ) );
    if ( ok )
    {
      transparency = transparencyD;
    }
  }

  // Set the QGraphicItem's opacity
  setOpacity( 1. - ( transparency / 100. ) );

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

void QgsComposerItem::drawText( QPainter* p, double x, double y, const QString& text, const QFont& font, const QColor& c ) const
{
  QgsComposerUtils::drawText( p, QPointF( x, y ), text, font, c );
}

void QgsComposerItem::drawText( QPainter* p, const QRectF& rect, const QString& text, const QFont& font, Qt::AlignmentFlag halignment, Qt::AlignmentFlag valignment, int flags ) const
{
  QgsComposerUtils::drawText( p, rect, text, font, QColor(), halignment, valignment, flags );
}
double QgsComposerItem::textWidthMillimeters( const QFont& font, const QString& text ) const
{
  return QgsComposerUtils::textWidthMM( font, text );
}

double QgsComposerItem::fontHeightCharacterMM( const QFont& font, const QChar& c ) const
{
  return QgsComposerUtils::fontHeightCharacterMM( font, c );
}

double QgsComposerItem::fontAscentMillimeters( const QFont& font ) const
{
  return QgsComposerUtils::fontAscentMM( font );
}

double QgsComposerItem::fontDescentMillimeters( const QFont& font ) const
{
  return QgsComposerUtils::fontDescentMM( font );
}

double QgsComposerItem::fontHeightMillimeters( const QFont& font ) const
{
  return QgsComposerUtils::fontHeightMM( font );
}

double QgsComposerItem::pixelFontSize( double pointSize ) const
{
  return QgsComposerUtils::pointsToMM( pointSize );
}

QFont QgsComposerItem::scaledFontPixelSize( const QFont& font ) const
{
  return QgsComposerUtils::scaledFontPixelSize( font );
}

double QgsComposerItem::horizontalViewScaleFactor() const
{
  double result = -1;
  if ( scene() )
  {
    QList<QGraphicsView*> viewList = scene()->views();
    if ( viewList.size() > 0 ) //if not, probably this function was called from non-gui code
    {
      QGraphicsView* currentView = viewList.at( 0 );
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

double QgsComposerItem::lockSymbolSize() const
{
  double lockSymbolSize = 20.0 / horizontalViewScaleFactor();

  if ( lockSymbolSize > ( rect().width() / 3 ) )
  {
    lockSymbolSize = rect().width() / 3;
  }
  if ( lockSymbolSize > ( rect().height() / 3 ) )
  {
    lockSymbolSize = rect().height() / 3;
  }
  return lockSymbolSize;
}

void QgsComposerItem::setRotation( const double r )
{
  //kept for api compatibility with QGIS 2.0
  //remove after 2.0 series
  setItemRotation( r, true );
}

void QgsComposerItem::setItemRotation( const double r, const bool adjustPosition )
{
  if ( r >= 360 )
  {
    mItemRotation = (( int )r ) % 360;
  }
  else
  {
    mItemRotation = r;
  }

  refreshRotation( true, adjustPosition );
}

void QgsComposerItem::refreshRotation( const bool updateItem , const bool adjustPosition )
{
  double rotation = mItemRotation;

  //data defined rotation set?
  QVariant exprVal;
  if ( dataDefinedEvaluate( QgsComposerObject::ItemRotation, exprVal ) )
  {
    bool ok;
    double rotD = exprVal.toDouble( &ok );
    QgsDebugMsg( QString( "exprVal Rotation:%1" ).arg( rotD ) );
    if ( ok )
    {
      rotation = rotD;
    }
  }

  if ( adjustPosition )
  {
    //adjustPosition set, so shift the position of the item so that rotation occurs around item center
    //create a line from the centrepoint of the rect() to its origin, in scene coordinates
    QLineF refLine = QLineF( mapToScene( QPointF( rect().width() / 2.0, rect().height() / 2.0 ) ) , mapToScene( QPointF( 0 , 0 ) ) );
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

bool QgsComposerItem::imageSizeConsideringRotation( double& width, double& height ) const
{
  //kept for api compatibility with QGIS 2.0, use item rotation
  Q_NOWARN_DEPRECATED_PUSH
  return imageSizeConsideringRotation( width, height, mEvaluatedItemRotation );
  Q_NOWARN_DEPRECATED_POP
}

bool QgsComposerItem::imageSizeConsideringRotation( double& width, double& height, double rotation ) const
{
  if ( qAbs( rotation ) <= 0.0 ) //width and height stays the same if there is no rotation
  {
    return true;
  }

  if ( qgsDoubleNear( qAbs( rotation ), 90 ) || qgsDoubleNear( qAbs( rotation ), 270 ) )
  {
    double tmp = width;
    width = height;
    height = tmp;
    return true;
  }

  double x1 = 0;
  double y1 = 0;
  double x2 = width;
  double y2 = 0;
  double x3 = width;
  double y3 = height;
  double x4 = 0;
  double y4 = height;
  double midX = width / 2.0;
  double midY = height / 2.0;

  Q_NOWARN_DEPRECATED_PUSH
  if ( !cornerPointOnRotatedAndScaledRect( x1, y1, width, height, rotation ) )
  {
    return false;
  }
  if ( !cornerPointOnRotatedAndScaledRect( x2, y2, width, height, rotation ) )
  {
    return false;
  }
  if ( !cornerPointOnRotatedAndScaledRect( x3, y3, width, height, rotation ) )
  {
    return false;
  }
  if ( !cornerPointOnRotatedAndScaledRect( x4, y4, width, height, rotation ) )
  {
    return false;
  }
  Q_NOWARN_DEPRECATED_POP


  //assume points 1 and 3 are on the rectangle boundaries. Calculate 2 and 4.
  double distM1 = sqrt(( x1 - midX ) * ( x1 - midX ) + ( y1 - midY ) * ( y1 - midY ) );
  QPointF p2 = QgsSymbolLayerV2Utils::pointOnLineWithDistance( QPointF( midX, midY ), QPointF( x2, y2 ), distM1 );

  if ( p2.x() < width && p2.x() > 0 && p2.y() < height && p2.y() > 0 )
  {
    width = sqrt(( p2.x() - x1 ) * ( p2.x() - x1 ) + ( p2.y() - y1 ) * ( p2.y() - y1 ) );
    height = sqrt(( x3 - p2.x() ) * ( x3 - p2.x() ) + ( y3 - p2.y() ) * ( y3 - p2.y() ) );
    return true;
  }

  //else assume that points 2 and 4 are on the rectangle boundaries. Calculate 1 and 3
  double distM2 = sqrt(( x2 - midX ) * ( x2 - midX ) + ( y2 - midY ) * ( y2 - midY ) );
  QPointF p1 = QgsSymbolLayerV2Utils::pointOnLineWithDistance( QPointF( midX, midY ), QPointF( x1, y1 ), distM2 );
  QPointF p3 = QgsSymbolLayerV2Utils::pointOnLineWithDistance( QPointF( midX, midY ), QPointF( x3, y3 ), distM2 );
  width = sqrt(( x2 - p1.x() ) * ( x2 - p1.x() ) + ( y2 - p1.y() ) * ( y2 - p1.y() ) );
  height = sqrt(( p3.x() - x2 ) * ( p3.x() - x2 ) + ( p3.y() - y2 ) * ( p3.y() - y2 ) );
  return true;
}

QRectF QgsComposerItem::largestRotatedRectWithinBounds( QRectF originalRect, QRectF boundsRect, double rotation ) const
{
  return QgsComposerUtils::largestRotatedRectWithinBounds( originalRect, boundsRect, rotation );
}

bool QgsComposerItem::cornerPointOnRotatedAndScaledRect( double& x, double& y, double width, double height ) const
{
  //kept for api compatibility with QGIS 2.0, use item rotation
  Q_NOWARN_DEPRECATED_PUSH
  return cornerPointOnRotatedAndScaledRect( x, y, width, height, mEvaluatedItemRotation );
  Q_NOWARN_DEPRECATED_POP
}

bool QgsComposerItem::cornerPointOnRotatedAndScaledRect( double& x, double& y, double width, double height, double rotation ) const
{
  //first rotate point clockwise
  double rotToRad = rotation * M_PI / 180.0;
  QPointF midpoint( width / 2.0, height / 2.0 );
  double xVector = x - midpoint.x();
  double yVector = y - midpoint.y();
  //double xRotated = cos(rotToRad) * xVector + sin(rotToRad) * yVector;
  //double yRotated = -sin(rotToRad) * xVector + cos(rotToRad) * yVector;
  double xRotated = cos( rotToRad ) * xVector - sin( rotToRad ) * yVector;
  double yRotated = sin( rotToRad ) * xVector + cos( rotToRad ) * yVector;

  //create line from midpoint to rotated point
  QLineF line( midpoint.x(), midpoint.y(), midpoint.x() + xRotated, midpoint.y() + yRotated );

  //intersect with all four borders and return result
  QList<QLineF> borders;
  borders << QLineF( 0, 0, width, 0 );
  borders << QLineF( width, 0, width, height );
  borders << QLineF( width, height, 0, height );
  borders << QLineF( 0, height, 0, 0 );

  QList<QLineF>::const_iterator it = borders.constBegin();
  QPointF intersectionPoint;

  for ( ; it != borders.constEnd(); ++it )
  {
    if ( line.intersect( *it, &intersectionPoint ) == QLineF::BoundedIntersection )
    {
      x = intersectionPoint.x();
      y = intersectionPoint.y();
      return true;
    }
  }
  return false;
}

void QgsComposerItem::sizeChangedByRotation( double& width, double& height )
{
  //kept for api compatibility with QGIS 2.0, use item rotation
  Q_NOWARN_DEPRECATED_PUSH
  return sizeChangedByRotation( width, height, mEvaluatedItemRotation );
  Q_NOWARN_DEPRECATED_POP
}

void QgsComposerItem::sizeChangedByRotation( double& width, double& height, double rotation )
{
  if ( rotation == 0.0 )
  {
    return;
  }

  //vector to p1
  double x1 = -width / 2.0;
  double y1 = -height / 2.0;
  QgsComposerUtils::rotate( rotation, x1, y1 );
  //vector to p2
  double x2 = width / 2.0;
  double y2 = -height / 2.0;
  QgsComposerUtils::rotate( rotation, x2, y2 );
  //vector to p3
  double x3 = width / 2.0;
  double y3 = height / 2.0;
  QgsComposerUtils::rotate( rotation, x3, y3 );
  //vector to p4
  double x4 = -width / 2.0;
  double y4 = height / 2.0;
  QgsComposerUtils::rotate( rotation, x4, y4 );

  //double midpoint
  QPointF midpoint( width / 2.0, height / 2.0 );

  QPolygonF rotatedRectPoly;
  rotatedRectPoly << QPointF( midpoint.x() + x1, midpoint.y() + y1 );
  rotatedRectPoly << QPointF( midpoint.x() + x2, midpoint.y() + y2 );
  rotatedRectPoly << QPointF( midpoint.x() + x3, midpoint.y() + y3 );
  rotatedRectPoly << QPointF( midpoint.x() + x4, midpoint.y() + y4 );
  QRectF boundingRect = rotatedRectPoly.boundingRect();
  width = boundingRect.width();
  height = boundingRect.height();
}

void QgsComposerItem::rotate( double angle, double& x, double& y ) const
{
  QgsComposerUtils::rotate( angle, x, y );
}

QGraphicsLineItem* QgsComposerItem::hAlignSnapItem()
{
  if ( !mHAlignSnapItem )
  {
    mHAlignSnapItem = new QGraphicsLineItem( 0 );
    mHAlignSnapItem->setPen( QPen( QColor( Qt::red ) ) );
    scene()->addItem( mHAlignSnapItem );
    mHAlignSnapItem->setZValue( 90 );
  }
  return mHAlignSnapItem;
}

QGraphicsLineItem* QgsComposerItem::vAlignSnapItem()
{
  if ( !mVAlignSnapItem )
  {
    mVAlignSnapItem = new QGraphicsLineItem( 0 );
    mVAlignSnapItem->setPen( QPen( QColor( Qt::red ) ) );
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
    mHAlignSnapItem = 0;
  }
}

void QgsComposerItem::deleteVAlignSnapItem()
{
  if ( mVAlignSnapItem )
  {
    scene()->removeItem( mVAlignSnapItem );
    delete mVAlignSnapItem;
    mVAlignSnapItem = 0;
  }
}

void QgsComposerItem::deleteAlignItems()
{
  deleteHAlignSnapItem();
  deleteVAlignSnapItem();
}

void QgsComposerItem::repaint()
{
  update();
}

void QgsComposerItem::refreshDataDefinedProperty( const QgsComposerObject::DataDefinedProperty property )
{
  //update data defined properties and redraw item to match
  if ( property == QgsComposerObject::PositionX || property == QgsComposerObject::PositionY ||
       property == QgsComposerObject::ItemWidth || property == QgsComposerObject::ItemHeight ||
       property == QgsComposerObject::AllProperties )
  {
    QRectF evaluatedRect = evalItemRect( QRectF( pos().x(), pos().y(), rect().width(), rect().height() ) );
    setSceneRect( evaluatedRect );
  }
  if ( property == QgsComposerObject::ItemRotation || property == QgsComposerObject::AllProperties )
  {
    refreshRotation( false, true );
  }
  if ( property == QgsComposerObject::Transparency || property == QgsComposerObject::AllProperties )
  {
    refreshTransparency( false );
  }
  if ( property == QgsComposerObject::BlendMode || property == QgsComposerObject::AllProperties )
  {
    refreshBlendMode();
  }

  update();
}

void QgsComposerItem::setId( const QString& id )
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
