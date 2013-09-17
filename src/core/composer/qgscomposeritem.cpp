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

#include <limits>
#include "qgsapplication.h"
#include "qgsrectangle.h" //just for debugging
#include "qgslogger.h"
#include "qgssymbollayerv2utils.h" //for pointOnLineWithDistance
#include "qgsmaprenderer.h" //for getCompositionMode

#include <cmath>

#define FONT_WORKAROUND_SCALE 10 //scale factor for upscaling fontsize and downscaling painter

QgsComposerItem::QgsComposerItem( QgsComposition* composition, bool manageZValue )
    : QObject( 0 )
    , QGraphicsRectItem( 0 )
    , mComposition( composition )
    , mBoundingResizeRectangle( 0 )
    , mHAlignSnapItem( 0 )
    , mVAlignSnapItem( 0 )
    , mFrame( false )
    , mBackground( true )
    , mBackgroundColor( QColor( 255, 255, 255, 255 ) )
    , mItemPositionLocked( false )
    , mLastValidViewScaleFactor( -1 )
    , mRotation( 0 )
    , mBlendMode( QPainter::CompositionMode_SourceOver )
    , mEffectsEnabled( true )
    , mTransparency( 0 )
    , mLastUsedPositionMode( UpperLeft )
    , mId( "" )
    , mUuid( QUuid::createUuid().toString() )
{
  init( manageZValue );
}

QgsComposerItem::QgsComposerItem( qreal x, qreal y, qreal width, qreal height, QgsComposition* composition, bool manageZValue )
    : QObject( 0 )
    , QGraphicsRectItem( 0, 0, width, height, 0 )
    , mComposition( composition )
    , mBoundingResizeRectangle( 0 )
    , mHAlignSnapItem( 0 )
    , mVAlignSnapItem( 0 )
    , mFrame( false )
    , mBackground( true )
    , mBackgroundColor( QColor( 255, 255, 255, 255 ) )
    , mItemPositionLocked( false )
    , mLastValidViewScaleFactor( -1 )
    , mRotation( 0 )
    , mBlendMode( QPainter::CompositionMode_SourceOver )
    , mEffectsEnabled( true )
    , mTransparency( 0 )
    , mLastUsedPositionMode( UpperLeft )
    , mId( "" )
    , mUuid( QUuid::createUuid().toString() )
{
  init( manageZValue );
  QTransform t;
  t.translate( x, y );
  setTransform( t );
}

void QgsComposerItem::init( bool manageZValue )
{
  setFlag( QGraphicsItem::ItemIsSelectable, true );
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

  // Setup composer effect
  mEffect = new QgsComposerEffect();
  setGraphicsEffect( mEffect );
}

QgsComposerItem::~QgsComposerItem()
{
  if ( mComposition )
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

  //frame
  if ( mBackground )
  {
    composerItemElem.setAttribute( "background", "true" );
  }
  else
  {
    composerItemElem.setAttribute( "background", "false" );
  }

  //scene rect
  composerItemElem.setAttribute( "x", QString::number( transform().dx() ) );
  composerItemElem.setAttribute( "y", QString::number( transform().dy() ) );
  composerItemElem.setAttribute( "width", QString::number( rect().width() ) );
  composerItemElem.setAttribute( "height", QString::number( rect().height() ) );
  composerItemElem.setAttribute( "positionMode", QString::number(( int ) mLastUsedPositionMode ) );
  composerItemElem.setAttribute( "zValue", QString::number( zValue() ) );
  composerItemElem.setAttribute( "outlineWidth", QString::number( pen().widthF() ) );
  composerItemElem.setAttribute( "rotation",  QString::number( mRotation ) );
  composerItemElem.setAttribute( "uuid", mUuid );
  composerItemElem.setAttribute( "id", mId );
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

  //rotation
  mRotation = itemElem.attribute( "rotation", "0" ).toDouble();

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

  //position
  double x, y, width, height;
  bool xOk, yOk, widthOk, heightOk, positionModeOK;

  x = itemElem.attribute( "x" ).toDouble( &xOk );
  y = itemElem.attribute( "y" ).toDouble( &yOk );
  width = itemElem.attribute( "width" ).toDouble( &widthOk );
  height = itemElem.attribute( "height" ).toDouble( &heightOk );
  mLastUsedPositionMode = ( ItemPositionMode )itemElem.attribute( "positionMode" ).toInt( &positionModeOK );
  if ( !positionModeOK )
  {
    mLastUsedPositionMode = UpperLeft;
  }

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
      setBackgroundColor( brushColor );
    }
  }

  //blend mode
  setBlendMode( QgsMapRenderer::getCompositionMode(( QgsMapRenderer::BlendMode ) itemElem.attribute( "blendMode", "0" ).toUInt() ) );

  //transparency
  setTransparency( itemElem.attribute( "transparency" , "0" ).toInt() );

  return true;
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

  if ( !mComposition )
  {
    return;
  }

  if ( mComposition->plotStyle() == QgsComposition::Preview )
  {
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

void QgsComposerItem::setPositionLock( bool lock )
{
  mItemPositionLocked = lock;
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
  setItemPosition( x, y, width, height, itemPoint );
}

void QgsComposerItem::setItemPosition( double x, double y, double width, double height, ItemPositionMode itemPoint )
{
  double upperLeftX = x;
  double upperLeftY = y;

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

  emit sizeChanged();
}

void QgsComposerItem::drawBackground( QPainter* p )
{
  if ( mBackground && p )
  {
    p->setBrush( brush() );//this causes a problem in atlas generation
    p->setPen( Qt::NoPen );
    p->setRenderHint( QPainter::Antialiasing, true );
    p->drawRect( QRectF( 0, 0, rect().width(), rect().height() ) );
  }
}

void QgsComposerItem::setBackgroundColor( const QColor& backgroundColor )
{
  mBackgroundColor = backgroundColor;
  setBrush( QBrush( mBackgroundColor, Qt::SolidPattern ) );
}

void QgsComposerItem::setBlendMode( QPainter::CompositionMode blendMode )
{
  mBlendMode = blendMode;
  // Update the composer effect to use the new blend mode
  mEffect->setCompositionMode( mBlendMode );
}

void QgsComposerItem::setTransparency( int transparency )
{
  mTransparency = transparency;
  // Set the QGraphicItem's opacity
  setOpacity( 1. - ( transparency / 100. ) );
}

void QgsComposerItem::setEffectsEnabled( bool effectsEnabled )
{
  //enable or disable the QgsComposerEffect applied to this item
  mEffectsEnabled = effectsEnabled;
  mEffect->setEnabled( effectsEnabled );
}

void QgsComposerItem::drawText( QPainter* p, double x, double y, const QString& text, const QFont& font ) const
{
  QFont textFont = scaledFontPixelSize( font );

  p->save();
  p->setFont( textFont );
  double scaleFactor = 1.0 / FONT_WORKAROUND_SCALE;
  p->scale( scaleFactor, scaleFactor );
  p->drawText( QPointF( x * FONT_WORKAROUND_SCALE, y * FONT_WORKAROUND_SCALE ), text );
  p->restore();
}

void QgsComposerItem::drawText( QPainter* p, const QRectF& rect, const QString& text, const QFont& font, Qt::AlignmentFlag halignment, Qt::AlignmentFlag valignment ) const
{
  QFont textFont = scaledFontPixelSize( font );

  QRectF scaledRect( rect.x() * FONT_WORKAROUND_SCALE, rect.y() * FONT_WORKAROUND_SCALE,
                     rect.width() * FONT_WORKAROUND_SCALE, rect.height() * FONT_WORKAROUND_SCALE );

  p->save();
  p->setFont( textFont );
  double scaleFactor = 1.0 / FONT_WORKAROUND_SCALE;
  p->scale( scaleFactor, scaleFactor );
  p->drawText( scaledRect, halignment | valignment | Qt::TextWordWrap, text );
  p->restore();
}
void QgsComposerItem::drawArrowHead( QPainter* p, double x, double y, double angle, double arrowHeadWidth ) const
{
  if ( !p )
  {
    return;
  }
  double angleRad = angle / 180.0 * M_PI;
  QPointF middlePoint( x, y );
  //rotate both arrow points
  QPointF p1 = QPointF( -arrowHeadWidth / 2.0, arrowHeadWidth );
  QPointF p2 = QPointF( arrowHeadWidth / 2.0, arrowHeadWidth );

  QPointF p1Rotated, p2Rotated;
  p1Rotated.setX( p1.x() * cos( angleRad ) + p1.y() * -sin( angleRad ) );
  p1Rotated.setY( p1.x() * sin( angleRad ) + p1.y() * cos( angleRad ) );
  p2Rotated.setX( p2.x() * cos( angleRad ) + p2.y() * -sin( angleRad ) );
  p2Rotated.setY( p2.x() * sin( angleRad ) + p2.y() * cos( angleRad ) );

  QPolygonF arrowHeadPoly;
  arrowHeadPoly << middlePoint;
  arrowHeadPoly << QPointF( middlePoint.x() + p1Rotated.x(), middlePoint.y() + p1Rotated.y() );
  arrowHeadPoly << QPointF( middlePoint.x() + p2Rotated.x(), middlePoint.y() + p2Rotated.y() );

  p->save();

  QPen arrowPen = p->pen();
  arrowPen.setJoinStyle( Qt::RoundJoin );
  QBrush arrowBrush = p->brush();
  arrowBrush.setStyle( Qt::SolidPattern );
  p->setPen( arrowPen );
  p->setBrush( arrowBrush );
  arrowBrush.setStyle( Qt::SolidPattern );
  p->drawPolygon( arrowHeadPoly );

  p->restore();
}

double QgsComposerItem::textWidthMillimeters( const QFont& font, const QString& text ) const
{
  QFont metricsFont = scaledFontPixelSize( font );
  QFontMetricsF fontMetrics( metricsFont );
  return ( fontMetrics.width( text ) / FONT_WORKAROUND_SCALE );
}

double QgsComposerItem::fontHeightCharacterMM( const QFont& font, const QChar& c ) const
{
  QFont metricsFont = scaledFontPixelSize( font );
  QFontMetricsF fontMetrics( metricsFont );
  return ( fontMetrics.boundingRect( c ).height() / FONT_WORKAROUND_SCALE );
}

double QgsComposerItem::fontAscentMillimeters( const QFont& font ) const
{
  QFont metricsFont = scaledFontPixelSize( font );
  QFontMetricsF fontMetrics( metricsFont );
  return ( fontMetrics.ascent() / FONT_WORKAROUND_SCALE );
}

double QgsComposerItem::fontDescentMillimeters( const QFont& font ) const
{
  QFont metricsFont = scaledFontPixelSize( font );
  QFontMetricsF fontMetrics( metricsFont );
  return ( fontMetrics.descent() / FONT_WORKAROUND_SCALE );
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

double QgsComposerItem::angle( const QPointF& p1, const QPointF& p2 ) const
{
  double xDiff = p2.x() - p1.x();
  double yDiff = p2.y() - p1.y();
  double length = sqrt( xDiff * xDiff + yDiff * yDiff );
  if ( length <= 0 )
  {
    return 0;
  }

  double angle = acos(( -yDiff * length ) / ( length * length ) ) * 180 / M_PI;
  if ( xDiff < 0 )
  {
    return ( 360 - angle );
  }
  return angle;
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
  if ( qAbs( mRotation ) <= 0.0 ) //width and height stays the same if there is no rotation
  {
    return true;
  }

  if ( qgsDoubleNear( qAbs( mRotation ), 90 ) || qgsDoubleNear( qAbs( mRotation ), 270 ) )
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

void QgsComposerItem::setId( const QString& id )
{
  setToolTip( id );
  mId = id;
}
