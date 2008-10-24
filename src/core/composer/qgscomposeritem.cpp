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
#include <QGraphicsScene>
#include <QGraphicsSceneMouseEvent>
#include <QPainter>

#include "qgscomposition.h"
#include "qgscomposeritem.h"

#include <limits>
#include "qgsrect.h" //just for debugging
#include "qgslogger.h"

#define FONT_WORKAROUND_SCALE 10 //scale factor for upscaling fontsize and downscaling painter

QgsComposerItem::QgsComposerItem( QgsComposition* composition ): QGraphicsRectItem( 0 ), mComposition( composition ), mBoundingResizeRectangle( 0 ), mFrame( true )
{
  setFlag( QGraphicsItem::ItemIsSelectable, true );
  setAcceptsHoverEvents( true );

  //set default pen and brush
  setBrush( QBrush( QColor( 255, 255, 255, 255 ) ) );
  QPen defaultPen( QColor( 0, 0, 0 ) );
  defaultPen.setWidthF( 0.3 );
  setPen( defaultPen );

  //let z-Value be managed by composition
  if ( mComposition )
  {
    mComposition->addItemToZList( this );
  }
}

QgsComposerItem::QgsComposerItem( qreal x, qreal y, qreal width, qreal height, QgsComposition* composition ): QGraphicsRectItem( 0, 0, width, height, 0 ), mComposition( composition ), mBoundingResizeRectangle( 0 ), mFrame( true )
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
  if ( mComposition )
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

bool QgsComposerItem::_writeXML( QDomElement& itemElem, QDomDocument& doc )
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

  setSceneRect( QRectF( x, y, width, height ) );
  setZValue( itemElem.attribute( "zValue" ).toDouble() );

  //pen
  QDomNodeList frameColorList = itemElem.elementsByTagName( "FrameColor" );
  if ( frameColorList.size() > 0 )
  {
    QDomElement frameColorElem = frameColorList.at( 0 ).toElement();
    bool redOk, greenOk, blueOk, alphaOk, widthOk;
    int penRed, penGreen, penBlue, penAlpha, penWidth;
    penWidth = itemElem.attribute( "outlineWidth" ).toDouble( &widthOk );
    penRed = frameColorElem.attribute( "red" ).toDouble( &redOk );
    penGreen = frameColorElem.attribute( "green" ).toDouble( &greenOk );
    penBlue = frameColorElem.attribute( "blue" ).toDouble( &blueOk );
    penAlpha = frameColorElem.attribute( "alpha" ).toDouble( &alphaOk );
    if ( redOk && greenOk && blueOk && alphaOk && widthOk )
    {
      QPen framePen( QColor( penRed, penGreen, penBlue, penAlpha ) );
      framePen.setWidth( penWidth );
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
  qWarning( "QgsComposerItem::mouseMoveEvent" );
  if ( mBoundingResizeRectangle )
  {
    double diffX = event->lastPos().x() - mLastMouseEventPos.x();
    double diffY = event->lastPos().y() - mLastMouseEventPos.y();

    double mx, my, rx, ry;

    rectangleChange( diffX, diffY, mx, my, rx, ry );

    QRectF r = mBoundingResizeRectangle->rect();
    double newWidth = r.width() + rx;
    double newHeight = r.height() + ry;

    QTransform oldTransform = mBoundingResizeRectangle->transform();
    QTransform transform;
    transform.translate( oldTransform.dx() + mx, oldTransform.dy() + my );

    QRectF newBoundingRect( 0, 0, newWidth, newHeight );

    mBoundingResizeRectangle->setRect( newBoundingRect );
    mBoundingResizeRectangle->setTransform( transform );
  }
  mLastMouseEventPos = event->lastPos();
}

void QgsComposerItem::mousePressEvent( QGraphicsSceneMouseEvent * event )
{
  //set current position and type of mouse move action
  mMouseMoveStartPos = event->lastScenePos();
  mLastMouseEventPos = event->lastPos();
  mCurrentMouseMoveAction = mouseMoveActionForPosition( event->pos() );

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

  double mx, my, rx, ry;
  rectangleChange( diffX, diffY, mx, my, rx, ry );

  QRectF currentRect = rect();
  QRectF newRect( transform().dx() + mx, transform().dy() + my, currentRect.width() + rx, currentRect.height() + ry );
  setSceneRect( newRect );

  update();
  scene()->update();

  //reset default action
  mCurrentMouseMoveAction = QgsComposerItem::moveItem;
  setCursor( Qt::ArrowCursor );
}

Qt::CursorShape QgsComposerItem::cursorForPosition( const QPointF& itemCoordPos )
{
  QgsComposerItem::mouseMoveAction mouseAction = mouseMoveActionForPosition( itemCoordPos );

  if ( mouseAction == QgsComposerItem::moveItem )
  {
    return Qt::ClosedHandCursor;
  }
  else if ( mouseAction == QgsComposerItem::resizeDLeftUp || mouseAction == QgsComposerItem::resizeDRightDown )
  {
    return Qt::SizeFDiagCursor;
  }
  else if ( mouseAction == QgsComposerItem::resizeDLeftDown || mouseAction == QgsComposerItem::resizeDRightUp )
  {
    return Qt::SizeBDiagCursor;
  }
  else if ( mouseAction == QgsComposerItem::resizeUp || mouseAction == QgsComposerItem::resizeDown )
  {
    return Qt::SizeVerCursor;
  }
  else //if(mouseAction == QgsComposerItem::resizeLeft || mouseAction == QgsComposerItem::resizeRight)
  {
    return Qt::SizeHorCursor;
  }
}

QgsComposerItem::mouseMoveAction QgsComposerItem::mouseMoveActionForPosition( const QPointF& itemCoordPos )
{

  //move content tool


  bool nearLeftBorder = false;
  bool nearRightBorder = false;
  bool nearLowerBorder = false;
  bool nearUpperBorder = false;

  if ( itemCoordPos.x() < 5 )
  {
    nearLeftBorder = true;
  }
  if ( itemCoordPos.y() < 5 )
  {
    nearUpperBorder = true;
  }
  if ( itemCoordPos.x() > ( rect().width() - 5 ) )
  {
    nearRightBorder = true;
  }
  if ( itemCoordPos.y() > ( rect().height() - 5 ) )
  {
    nearLowerBorder = true;
  }

  if ( nearLeftBorder && nearUpperBorder )
  {
    return QgsComposerItem::resizeDLeftUp;
  }
  else if ( nearLeftBorder && nearLowerBorder )
  {
    return QgsComposerItem::resizeDLeftDown;
  }
  else if ( nearRightBorder && nearUpperBorder )
  {
    return QgsComposerItem::resizeDRightUp;
  }
  else if ( nearRightBorder && nearLowerBorder )
  {
    return QgsComposerItem::resizeDRightDown;
  }
  else if ( nearLeftBorder )
  {
    return QgsComposerItem::resizeLeft;
  }
  else if ( nearRightBorder )
  {
    return QgsComposerItem::resizeRight;
  }
  else if ( nearUpperBorder )
  {
    return QgsComposerItem::resizeUp;
  }
  else if ( nearLowerBorder )
  {
    return QgsComposerItem::resizeDown;
  }

  return QgsComposerItem::moveItem; //default
}


void QgsComposerItem::rectangleChange( double dx, double dy, double& mx, double& my, double& rx, double& ry ) const
{
  switch ( mCurrentMouseMoveAction )
  {
      //vertical resize
    case QgsComposerItem::resizeUp:
      mx = 0; my = dy; rx = 0; ry = -dy;
      break;

    case QgsComposerItem::resizeDown:
      mx = 0; my = 0; rx = 0; ry = dy;
      break;

      //horizontal resize
    case QgsComposerItem::resizeLeft:
      mx = dx, my = 0; rx = -dx; ry = 0;
      break;

    case QgsComposerItem::resizeRight:
      mx = 0; my = 0; rx = dx, ry = 0;
      break;

      //diagonal resize
    case QgsComposerItem::resizeDLeftUp:
      mx = dx, my = dy; rx = -dx; ry = -dy;
      break;

    case QgsComposerItem::resizeDRightDown:
      mx = 0; my = 0; rx = dx, ry = dy;
      break;

    case QgsComposerItem::resizeDRightUp:
      mx = 0; my = dy, rx = dx, ry = -dy;
      break;

    case QgsComposerItem::resizeDLeftDown:
      mx = dx, my = 0; rx = -dx; ry = dy;
      break;

    case QgsComposerItem::moveItem:
      mx = dx; my = dy; rx = 0, ry = 0;
      break;
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
    p->setPen( QPen( QColor( 0, 0, 255 ) ) );
    p->setBrush( QBrush( QColor( 0, 0, 255 ) ) );

    double s = 5;

    p->drawRect( QRectF( 0, 0, s, s ) );
    p->drawRect( QRectF( rect().width() - s, 0, s, s ) );
    p->drawRect( QRectF( rect().width() - s, rect().height() - s, s, s ) );
    p->drawRect( QRectF( 0, rect().height() - s, s, s ) );
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

void QgsComposerItem::drawText( QPainter* p, int x, int y, const QString& text, const QFont& font ) const
{
  QFont textFont = scaledFontPixelSize( font );

  p->save();
  p->setFont( textFont );
  p->setPen( QColor( 0, 0, 0 ) ); //draw text always in black
  double scaleFactor = 1.0 / FONT_WORKAROUND_SCALE;
  p->scale( scaleFactor, scaleFactor );
  p->drawText( x * FONT_WORKAROUND_SCALE, y * FONT_WORKAROUND_SCALE, text );
  p->restore();
}

void QgsComposerItem::drawText( QPainter* p, const QRectF& rect, const QString& text, const QFont& font ) const
{
  QFont textFont = scaledFontPixelSize( font );

  QRectF scaledRect( rect.x() * FONT_WORKAROUND_SCALE, rect.y() * FONT_WORKAROUND_SCALE,
                     rect.width() * FONT_WORKAROUND_SCALE, rect.height() * FONT_WORKAROUND_SCALE );

  p->save();
  p->setFont( textFont );
  p->setPen( QColor( 0, 0, 0 ) ); //draw text always in black
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
  QFontMetrics fontMetrics( metricsFont );
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
