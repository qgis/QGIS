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
#include <QGraphicsScene>
#include <QGraphicsSceneMouseEvent>
#include <QGraphicsView>
#include <QPainter>

#include "qgscomposition.h"
#include "qgscomposeritem.h"


#include <limits>
#include "qgsapplication.h"
#include "qgsrectangle.h" //just for debugging
#include "qgslogger.h"

#ifndef Q_OS_MACX
#include <cmath>
#else
#include <math.h>
#endif

#define FONT_WORKAROUND_SCALE 10 //scale factor for upscaling fontsize and downscaling painter

QgsComposerItem::QgsComposerItem( QgsComposition* composition, bool manageZValue ): QObject( 0 ), QGraphicsRectItem( 0 ), mComposition( composition ), mBoundingResizeRectangle( 0 ), \
    mFrame( true ), mItemPositionLocked( false ), mLastValidViewScaleFactor( -1 ), mRotation( 0 )
{
  setFlag( QGraphicsItem::ItemIsSelectable, true );
  setAcceptsHoverEvents( true );

  //set default pen and brush
  setBrush( QBrush( QColor( 255, 255, 255, 255 ) ) );
  QPen defaultPen( QColor( 0, 0, 0 ) );
  defaultPen.setWidthF( 0.3 );
  setPen( defaultPen );

  //let z-Value be managed by composition
  if ( mComposition && manageZValue )
  {
    mComposition->addItemToZList( this );
  }
}

QgsComposerItem::QgsComposerItem( qreal x, qreal y, qreal width, qreal height, QgsComposition* composition, bool manageZValue ): \
    QObject( 0 ), QGraphicsRectItem( 0, 0, width, height, 0 ), mComposition( composition ), mBoundingResizeRectangle( 0 ), mFrame( true ), \
    mItemPositionLocked( false ), mLastValidViewScaleFactor( -1 ), mRotation( 0 )
{
  setFlag( QGraphicsItem::ItemIsSelectable, true );
  setAcceptsHoverEvents( true );

  QTransform t;
  t.translate( x, y );
  setTransform( t );

  //set default pen and brush
  setBrush( QBrush( QColor( 255, 255, 255, 255 ) ) );
  QPen defaultPen( QColor( 0, 0, 0 ) );
  defaultPen.setWidthF( 0.3 );
  setPen( defaultPen );

//let z-Value be managed by composition
  if ( mComposition && manageZValue )
  {
    mComposition->addItemToZList( this );
  }
}

QgsComposerItem::~QgsComposerItem()
{
  if ( mComposition )
  {
    mComposition->removeItemFromZList( this );
  }

  delete mBoundingResizeRectangle;
}

void QgsComposerItem::setSelected( bool s )
{
  QgsDebugMsg( "entered." );
  QGraphicsRectItem::setSelected( s );
  update(); //to draw selection boxes
}

bool QgsComposerItem::writeSettings( void )  { return true; }

bool QgsComposerItem::readSettings( void )  { return true; }

bool QgsComposerItem::removeSettings( void )  { return true; }

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

  //scene rect
  composerItemElem.setAttribute( "x", transform().dx() );
  composerItemElem.setAttribute( "y", transform().dy() );
  composerItemElem.setAttribute( "width", rect().width() );
  composerItemElem.setAttribute( "height", rect().height() );
  composerItemElem.setAttribute( "zValue", QString::number( zValue() ) );
  composerItemElem.setAttribute( "outlineWidth", QString::number( pen().widthF() ) );
  composerItemElem.setAttribute( "rotation", mRotation );

  //position lock for mouse moves/resizes
  if ( mItemPositionLocked )
  {
    composerItemElem.setAttribute( "positionLock", "true" );
  }
  else
  {
    composerItemElem.setAttribute( "positionLock", "false" );
  }

  composerItemElem.setAttribute( "lastValidViewScaleFactor", mLastValidViewScaleFactor );


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

  itemElem.appendChild( composerItemElem );

  return true;
}

bool QgsComposerItem::_readXML( const QDomElement& itemElem, const QDomDocument& doc )
{
  if ( itemElem.isNull() )
  {
    return false;
  }

  //rotation
  mRotation = itemElem.attribute( "rotation", "0" ).toDouble();

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

  //position lock for mouse moves/resizes
  QString positionLock = itemElem.attribute( "positionLock" );
  if ( positionLock.compare( "true", Qt::CaseInsensitive ) == 0 )
  {
    mItemPositionLocked = true;
  }
  else
  {
    mItemPositionLocked = false;
  }

  //position
  double x, y, width, height;
  bool xOk, yOk, widthOk, heightOk;

  x = itemElem.attribute( "x" ).toDouble( &xOk );
  y = itemElem.attribute( "y" ).toDouble( &yOk );
  width = itemElem.attribute( "width" ).toDouble( &widthOk );
  height = itemElem.attribute( "height" ).toDouble( &heightOk );

  if ( !xOk || !yOk || !widthOk || !heightOk )
  {
    return false;
  }

  mLastValidViewScaleFactor = itemElem.attribute( "lastValidViewScaleFactor", "-1" ).toDouble();

  setSceneRect( QRectF( x, y, width, height ) );
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
    if ( redOk && greenOk && blueOk && alphaOk && widthOk )
    {
      QPen framePen( QColor( penRed, penGreen, penBlue, penAlpha ) );
      framePen.setWidthF( penWidth );
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
      setBrush( QBrush( brushColor ) );
    }
  }
  return true;
}

void QgsComposerItem::mouseMoveEvent( QGraphicsSceneMouseEvent * event )
{
  if ( mItemPositionLocked )
  {
    return;
  }

  if ( mBoundingResizeRectangle )
  {
    double diffX = event->lastScenePos().x() - mLastMouseEventPos.x();
    double diffY = event->lastScenePos().y() - mLastMouseEventPos.y();

    changeItemRectangle( event->lastScenePos(), mMouseMoveStartPos, this, diffX, diffY, mBoundingResizeRectangle );
  }
  mLastMouseEventPos = event->lastScenePos();
}

void QgsComposerItem::mousePressEvent( QGraphicsSceneMouseEvent * event )
{
  if ( mItemPositionLocked )
  {
    return;
  }

  //set current position and type of mouse move action
  mMouseMoveStartPos = event->lastScenePos();
  mLastMouseEventPos = event->lastScenePos();
  mCurrentMouseMoveAction = mouseMoveActionForPosition( event->pos() );

  //remove the old rubber band item if it is still there
  if ( mBoundingResizeRectangle )
  {
    scene()->removeItem( mBoundingResizeRectangle );
    delete mBoundingResizeRectangle;
    mBoundingResizeRectangle = 0;
  }
  //create and show bounding rectangle
  mBoundingResizeRectangle = new QGraphicsRectItem( 0 );
  scene()->addItem( mBoundingResizeRectangle );
  mBoundingResizeRectangle->setRect( QRectF( 0, 0, rect().width(), rect().height() ) );
  QTransform resizeTransform;
  resizeTransform.translate( transform().dx(), transform().dy() );
  mBoundingResizeRectangle->setTransform( resizeTransform );

  mBoundingResizeRectangle->setBrush( Qt::NoBrush );
  mBoundingResizeRectangle->setPen( QPen( QColor( 0, 0, 0 ), 0 ) );
  mBoundingResizeRectangle->setZValue( 90 );
  mBoundingResizeRectangle->show();
}

void QgsComposerItem::mouseReleaseEvent( QGraphicsSceneMouseEvent * event )
{

  if ( mItemPositionLocked )
  {
    return;
  }

  //delete frame rectangle
  if ( mBoundingResizeRectangle )
  {
    scene()->removeItem( mBoundingResizeRectangle );
    delete mBoundingResizeRectangle;
    mBoundingResizeRectangle = 0;
  }

  QPointF mouseMoveStopPoint = event->lastScenePos();
  double diffX = mouseMoveStopPoint.x() - mMouseMoveStartPos.x();
  double diffY = mouseMoveStopPoint.y() - mMouseMoveStartPos.y();

  //it was only a click
  if ( abs( diffX ) < std::numeric_limits<double>::min() && abs( diffY ) < std::numeric_limits<double>::min() )
  {
    return;
  }

  changeItemRectangle( mouseMoveStopPoint, mMouseMoveStartPos, this, diffX, diffY, this );

  //reset default action
  mCurrentMouseMoveAction = QgsComposerItem::MoveItem;
  setCursor( Qt::ArrowCursor );
}

Qt::CursorShape QgsComposerItem::cursorForPosition( const QPointF& itemCoordPos )
{
  QgsComposerItem::MouseMoveAction mouseAction = mouseMoveActionForPosition( itemCoordPos );

  if ( mouseAction == QgsComposerItem::NoAction )
  {
    return Qt::ForbiddenCursor;
  }
  if ( mouseAction == QgsComposerItem::MoveItem )
  {
    return Qt::ClosedHandCursor;
  }
  else if ( mouseAction == QgsComposerItem::ResizeLeftUp || mouseAction == QgsComposerItem::ResizeRightDown )
  {
    return Qt::SizeFDiagCursor;
  }
  else if ( mouseAction == QgsComposerItem::ResizeLeftDown || mouseAction == QgsComposerItem::ResizeRightUp )
  {
    return Qt::SizeBDiagCursor;
  }
  else if ( mouseAction == QgsComposerItem::ResizeUp || mouseAction == QgsComposerItem::ResizeDown )
  {
    return Qt::SizeVerCursor;
  }
  else //if(mouseAction == QgsComposerItem::ResizeLeft || mouseAction == QgsComposerItem::ResizeRight)
  {
    return Qt::SizeHorCursor;
  }
}

QgsComposerItem::MouseMoveAction QgsComposerItem::mouseMoveActionForPosition( const QPointF& itemCoordPos )
{

  //no action at all if item position is locked for mouse
  if ( mItemPositionLocked )
  {
    return QgsComposerItem::NoAction;
  }

  bool nearLeftBorder = false;
  bool nearRightBorder = false;
  bool nearLowerBorder = false;
  bool nearUpperBorder = false;

  double borderTolerance = rectHandlerBorderTolerance();

  if ( itemCoordPos.x() < borderTolerance )
  {
    nearLeftBorder = true;
  }
  if ( itemCoordPos.y() < borderTolerance )
  {
    nearUpperBorder = true;
  }
  if ( itemCoordPos.x() > ( rect().width() - borderTolerance ) )
  {
    nearRightBorder = true;
  }
  if ( itemCoordPos.y() > ( rect().height() - borderTolerance ) )
  {
    nearLowerBorder = true;
  }

  if ( nearLeftBorder && nearUpperBorder )
  {
    return QgsComposerItem::ResizeLeftUp;
  }
  else if ( nearLeftBorder && nearLowerBorder )
  {
    return QgsComposerItem::ResizeLeftDown;
  }
  else if ( nearRightBorder && nearUpperBorder )
  {
    return QgsComposerItem::ResizeRightUp;
  }
  else if ( nearRightBorder && nearLowerBorder )
  {
    return QgsComposerItem::ResizeRightDown;
  }
  else if ( nearLeftBorder )
  {
    return QgsComposerItem::ResizeLeft;
  }
  else if ( nearRightBorder )
  {
    return QgsComposerItem::ResizeRight;
  }
  else if ( nearUpperBorder )
  {
    return QgsComposerItem::ResizeUp;
  }
  else if ( nearLowerBorder )
  {
    return QgsComposerItem::ResizeDown;
  }

  return QgsComposerItem::MoveItem; //default
}

void QgsComposerItem::changeItemRectangle( const QPointF& currentPosition, const QPointF& mouseMoveStartPos, const QGraphicsRectItem* originalItem, double dx, double dy, QGraphicsRectItem* changeItem )
{
  if ( !changeItem || !originalItem || !mComposition )
  {
    return;
  }

  //test if change item is a composer item. If so, prefer call to  setSceneRect() instead of setTransform() and setRect()
  QgsComposerItem* changeComposerItem = dynamic_cast<QgsComposerItem *>( changeItem );

  double mx = 0.0, my = 0.0, rx = 0.0, ry = 0.0;
  QPointF snappedPosition = mComposition->snapPointToGrid( currentPosition );
  //double diffX = snappedPosition.x() - mouseMoveStartPos.x();
  //double diffY = snappedPosition.y() - mouseMoveStartPos.y();
  double diffX = 0;
  double diffY = 0;

  switch ( mCurrentMouseMoveAction )
  {
      //vertical resize
    case QgsComposerItem::ResizeUp:
      diffY = snappedPosition.y() - originalItem->transform().dy();
      mx = 0; my = diffY; rx = 0; ry = -diffY;
      break;

    case QgsComposerItem::ResizeDown:
      diffY = snappedPosition.y() - ( originalItem->transform().dy() + originalItem->rect().height() );
      mx = 0; my = 0; rx = 0; ry = diffY;
      break;

      //horizontal resize
    case QgsComposerItem::ResizeLeft:
      diffX = snappedPosition.x() - originalItem->transform().dx();
      mx = diffX, my = 0; rx = -diffX; ry = 0;
      break;

    case QgsComposerItem::ResizeRight:
      diffX = snappedPosition.x() - ( originalItem->transform().dx() + originalItem->rect().width() );
      mx = 0; my = 0; rx = diffX, ry = 0;
      break;

      //diagonal resize
    case QgsComposerItem::ResizeLeftUp:
      diffX = snappedPosition.x() - originalItem->transform().dx();
      diffY = snappedPosition.y() - originalItem->transform().dy();
      mx = diffX, my = diffY; rx = -diffX; ry = -diffY;
      break;

    case QgsComposerItem::ResizeRightDown:
      diffX = snappedPosition.x() - ( originalItem->transform().dx() + originalItem->rect().width() );
      diffY = snappedPosition.y() - ( originalItem->transform().dy() + originalItem->rect().height() );
      mx = 0; my = 0; rx = diffX, ry = diffY;
      break;

    case QgsComposerItem::ResizeRightUp:
      diffX = snappedPosition.x() - ( originalItem->transform().dx() + originalItem->rect().width() );
      diffY = snappedPosition.y() - originalItem->transform().dy();
      mx = 0; my = diffY, rx = diffX, ry = -diffY;
      break;

    case QgsComposerItem::ResizeLeftDown:
      diffX = snappedPosition.x() - originalItem->transform().dx();
      diffY = snappedPosition.y() - ( originalItem->transform().dy() + originalItem->rect().height() );
      mx = diffX, my = 0; rx = -diffX; ry = diffY;
      break;

    case QgsComposerItem::MoveItem:
    {
      //calculate total move difference
      double moveX = currentPosition.x() - mouseMoveStartPos.x();
      double moveY = currentPosition.y() - mouseMoveStartPos.y();

      QPointF upperLeftPoint( originalItem->transform().dx() + moveX, originalItem->transform().dy() + moveY );
      QPointF snappedLeftPoint = mComposition->snapPointToGrid( upperLeftPoint );

      double moveRectX = snappedLeftPoint.x() - originalItem->transform().dx();
      double moveRectY = snappedLeftPoint.y() - originalItem->transform().dy();

      if ( !changeComposerItem )
      {
        QTransform moveTransform;
        moveTransform.translate( originalItem->transform().dx() + moveRectX, originalItem->transform().dy() + moveRectY );
        changeItem->setTransform( moveTransform );
      }
      else  //for composer items, we prefer setSceneRect as subclasses can implement custom behaviour (e.g. item group)
      {
        changeComposerItem->setSceneRect( QRectF( originalItem->transform().dx() + moveRectX, \
                                          originalItem->transform().dy() + moveRectY, \
                                          originalItem->rect().width(), originalItem->rect().height() ) );
      }
    }
    return;
    case QgsComposerItem::NoAction:
      break;
  }

  if ( !changeComposerItem )
  {
    QTransform itemTransform;
    itemTransform.translate( originalItem->transform().dx() + mx, originalItem->transform().dy() + my );
    changeItem->setTransform( itemTransform );
    QRectF itemRect( 0, 0, originalItem->rect().width() + rx,  originalItem->rect().height() + ry );
    changeItem->setRect( itemRect );
  }
  else //for composer items, we prefer setSceneRect as subclasses can implement custom behaviour (e.g. item group)
  {
    changeComposerItem->setSceneRect( QRectF( originalItem->transform().dx() + mx, originalItem->transform().dy() + my, \
                                      originalItem->rect().width() + rx, originalItem->rect().height() + ry ) );
  }
}

void QgsComposerItem::drawSelectionBoxes( QPainter* p )
{
  if ( !mComposition )
  {
    return;
  }

  if ( mComposition->plotStyle() == QgsComposition::Preview )
  {
    //size of symbol boxes depends on zoom level in composer view
    double rectHandlerSize = rectHandlerBorderTolerance();
    double sizeLockSymbol = lockSymbolSize();

    if ( mItemPositionLocked )
    {
      //draw lock symbol at upper left edge. Use QImage to be independent of the graphic system
      QString lockIconPath = QgsApplication::activeThemePath() + "/mIconLock.png";
      if ( !QFile::exists( lockIconPath ) )
      {
        lockIconPath = QgsApplication::defaultThemePath() + "/mIconLock.png";
      }

      QImage lockImage( lockIconPath );
      if ( !lockImage.isNull() )
      {
        p->drawImage( QRectF( 0, 0, sizeLockSymbol, sizeLockSymbol ), lockImage, QRectF( 0, 0, lockImage.width(), lockImage.height() ) );
      }
    }
    else //draw blue squares
    {
      p->setPen( QColor( 50, 100, 120, 200 ) );
      p->setBrush( QColor( 200, 200, 210, 120 ) );
      p->drawRect( QRectF( 0, 0, rectHandlerSize, rectHandlerSize ) );
      p->drawRect( QRectF( rect().width() - rectHandlerSize, 0, rectHandlerSize, rectHandlerSize ) );
      p->drawRect( QRectF( rect().width() - rectHandlerSize, rect().height() - rectHandlerSize, rectHandlerSize, rectHandlerSize ) );
      p->drawRect( QRectF( 0, rect().height() - rectHandlerSize, rectHandlerSize, rectHandlerSize ) );
    }
  }
}

void QgsComposerItem::drawFrame( QPainter* p )
{
  if ( mFrame && p )
  {
    p->setPen( pen() );
    p->setBrush( Qt::NoBrush );
    p->setRenderHint( QPainter::Antialiasing, true );
    p->drawRect( QRectF( 0, 0, rect().width(), rect().height() ) );
  }
}

void QgsComposerItem::move( double dx, double dy )
{
  QTransform t = transform();
  QRectF newSceneRect( t.dx() + dx, t.dy() + dy, rect().width(), rect().height() );
  setSceneRect( newSceneRect );
}

void QgsComposerItem::setItemPosition( double x, double y, ItemPositionMode itemPoint )
{
  double width = rect().width();
  double height = rect().height();

  double upperLeftX = x;
  double upperLeftY = y;

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

  setSceneRect( QRectF( upperLeftX, upperLeftY, width, height ) );
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

  QRectF newRect( 0, 0, newWidth, newHeight );
  QGraphicsRectItem::setRect( newRect );

  //set up transformation matrix for item coordinates
  QTransform t;
  t.translate( xTranslation, yTranslation );
  setTransform( t );
}

void QgsComposerItem::drawBackground( QPainter* p )
{
  if ( p )
  {
    p->setBrush( brush() );
    p->setPen( Qt::NoPen );
    p->setRenderHint( QPainter::Antialiasing, true );
    p->drawRect( QRectF( 0, 0, rect().width(), rect().height() ) );
  }
}

void QgsComposerItem::hoverMoveEvent( QGraphicsSceneHoverEvent * event )
{
  if ( isSelected() )
  {
    setCursor( cursorForPosition( event->pos() ) );
  }
}

void QgsComposerItem::drawText( QPainter* p, double x, double y, const QString& text, const QFont& font ) const
{
  QFont textFont = scaledFontPixelSize( font );

  p->save();
  p->setFont( textFont );
  p->setPen( QColor( 0, 0, 0 ) ); //draw text always in black
  double scaleFactor = 1.0 / FONT_WORKAROUND_SCALE;
  p->scale( scaleFactor, scaleFactor );
  p->drawText( QPointF( x * FONT_WORKAROUND_SCALE, y * FONT_WORKAROUND_SCALE ), text );
  p->restore();
}

void QgsComposerItem::drawText( QPainter* p, const QRectF& rect, const QString& text, const QFont& font ) const
{
  QFont textFont = scaledFontPixelSize( font );

  QRectF scaledRect( rect.x() * FONT_WORKAROUND_SCALE, rect.y() * FONT_WORKAROUND_SCALE,
                     rect.width() * FONT_WORKAROUND_SCALE, rect.height() * FONT_WORKAROUND_SCALE );

  p->save();
  p->setFont( textFont );
  double scaleFactor = 1.0 / FONT_WORKAROUND_SCALE;
  p->scale( scaleFactor, scaleFactor );
  p->drawText( scaledRect, Qt::AlignLeft | Qt::AlignTop | Qt::TextWordWrap, text );
  p->restore();
}

double QgsComposerItem::textWidthMillimeters( const QFont& font, const QString& text ) const
{
  QFont metricsFont = scaledFontPixelSize( font );
  QFontMetrics fontMetrics( metricsFont );
  return ( fontMetrics.width( text ) / FONT_WORKAROUND_SCALE );
}

double QgsComposerItem::fontAscentMillimeters( const QFont& font ) const
{
  QFont metricsFont = scaledFontPixelSize( font );
  QFontMetricsF fontMetrics( metricsFont );
  return ( fontMetrics.ascent() / FONT_WORKAROUND_SCALE );
}

double QgsComposerItem::pixelFontSize( double pointSize ) const
{
  return ( pointSize * 0.3527 );
}

QFont QgsComposerItem::scaledFontPixelSize( const QFont& font ) const
{
  QFont scaledFont = font;
  double pixelSize = pixelFontSize( font.pointSizeF() ) * FONT_WORKAROUND_SCALE + 0.5;
  scaledFont.setPixelSize( pixelSize );
  return scaledFont;
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

void QgsComposerItem::updateCursor( const QPointF& itemPos )
{
  setCursor( cursorForPosition( itemPos ) );
}

void QgsComposerItem::setRotation( double r )
{
  if ( r > 360 )
  {
    mRotation = (( int )r ) % 360;
  }
  else
  {
    mRotation = r;
  }
  emit rotationChanged( r );
  update();
}

bool QgsComposerItem::imageSizeConsideringRotation( double& width, double& height ) const
{
  if ( abs( mRotation ) <= 0 ) //width and height stays the same if there is no rotation
  {
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

  if ( !cornerPointOnRotatedAndScaledRect( x1, y1, width, height ) )
  {
    return false;
  }
  if ( !cornerPointOnRotatedAndScaledRect( x2, y2, width, height ) )
  {
    return false;
  }
  if ( !cornerPointOnRotatedAndScaledRect( x3, y3, width, height ) )
  {
    return false;
  }
  if ( !cornerPointOnRotatedAndScaledRect( x4, y4, width, height ) )
  {
    return false;
  }


  //assume points 1 and 3 are on the rectangle boundaries. Calculate 2 and 4.
  double distM1 = sqrt(( x1 - midX ) * ( x1 - midX ) + ( y1 - midY ) * ( y1 - midY ) );
  QPointF p2 = pointOnLineWithDistance( QPointF( midX, midY ), QPointF( x2, y2 ), distM1 );
  QPointF p4 = pointOnLineWithDistance( QPointF( midX, midY ), QPointF( x4, y4 ), distM1 );

  if ( p2.x() < width && p2.x() > 0 && p2.y() < height && p2.y() > 0 )
  {
    width = sqrt(( p2.x() - x1 ) * ( p2.x() - x1 ) + ( p2.y() - y1 ) * ( p2.y() - y1 ) );
    height = sqrt(( x3 - p2.x() ) * ( x3 - p2.x() ) + ( y3 - p2.y() ) * ( y3 - p2.y() ) );
    return true;
  }

  //else assume that points 2 and 4 are on the rectangle boundaries. Calculate 1 and 3
  double distM2 = sqrt(( x2 - midX ) * ( x2 - midX ) + ( y2 - midY ) * ( y2 - midY ) );
  QPointF p1 = pointOnLineWithDistance( QPointF( midX, midY ), QPointF( x1, y1 ), distM2 );
  QPointF p3 = pointOnLineWithDistance( QPointF( midX, midY ), QPointF( x3, y3 ), distM2 );
  width = sqrt(( x2 - p1.x() ) * ( x2 - p1.x() ) + ( y2 - p1.y() ) * ( y2 - p1.y() ) );
  height = sqrt(( p3.x() - x2 ) * ( p3.x() - x2 ) + ( p3.y() - y2 ) * ( p3.y() - y2 ) );
  return true;


#if 0
  double x1 = 0;
  double y1 = 0;
  double x2 = width;
  double y2 = 0;
  double x3 = width;
  double y3 = height;

  if ( !cornerPointOnRotatedAndScaledRect( x1, y1, width, height ) )
  {
    return false;
  }
  if ( !cornerPointOnRotatedAndScaledRect( x2, y2, width, height ) )
  {
    return false;
  }
  if ( !cornerPointOnRotatedAndScaledRect( x3, y3, width, height ) )
  {
    return false;
  }

  width = sqrt(( x2 - x1 ) * ( x2 - x1 ) + ( y2 - y1 ) * ( y2 - y1 ) );
  height = sqrt(( x3 - x2 ) * ( x3 - x2 ) + ( y3 - y2 ) * ( y3 - y2 ) );
  return true;
#endif //0
}

bool QgsComposerItem::cornerPointOnRotatedAndScaledRect( double& x, double& y, double width, double height ) const
{
  //first rotate point clockwise
  double rotToRad = mRotation * M_PI / 180.0;
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

QPointF QgsComposerItem::pointOnLineWithDistance( const QPointF& startPoint, const QPointF& directionPoint, double distance ) const
{
  double dx = directionPoint.x() - startPoint.x();
  double dy = directionPoint.y() - startPoint.y();
  double length = sqrt( dx * dx + dy * dy );
  double scaleFactor = distance / length;
  return QPointF( startPoint.x() + dx * scaleFactor, startPoint.y() + dy * scaleFactor );
}

void QgsComposerItem::sizeChangedByRotation( double& width, double& height )
{
  if ( mRotation == 0.0 )
  {
    return;
  }

  //vector to p1
  double x1 = -width / 2.0;
  double y1 = -height / 2.0;
  rotate( mRotation, x1, y1 );
  //vector to p2
  double x2 = width / 2.0;
  double y2 = -height / 2.0;
  rotate( mRotation, x2, y2 );
  //vector to p3
  double x3 = width / 2.0;
  double y3 = height / 2.0;
  rotate( mRotation, x3, y3 );
  //vector to p4
  double x4 = -width / 2.0;
  double y4 = height / 2.0;
  rotate( mRotation, x4, y4 );

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
  double rotToRad = angle * M_PI / 180.0;
  double xRot, yRot;
  xRot = x * cos( rotToRad ) - y * sin( rotToRad );
  yRot = x * sin( rotToRad ) + y * cos( rotToRad );
  x = xRot;
  y = yRot;
}
