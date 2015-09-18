/***************************************************************************
                         qgscomposerarrow.cpp
                         ----------------------
    begin                : November 2009
    copyright            : (C) 2009 by Marco Hugentobler
    email                : marco@hugis.net
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "qgscomposerarrow.h"
#include "qgscomposition.h"
#include "qgscomposerutils.h"
#include "qgssymbollayerv2utils.h"
#include <QPainter>
#include <QSvgRenderer>
#include <QVector2D>

#include <cmath>

QgsComposerArrow::QgsComposerArrow( QgsComposition* c )
    : QgsComposerItem( c )
    , mStartPoint( 0, 0 )
    , mStopPoint( 0, 0 )
    , mStartXIdx( 0 )
    , mStartYIdx( 0 )
    , mMarkerMode( DefaultMarker )
    , mArrowHeadOutlineWidth( 1.0 )
    , mArrowHeadOutlineColor( Qt::black )
    , mArrowHeadFillColor( Qt::black )
    , mBoundsBehaviour( 24 )
    , mLineSymbol( 0 )
{
  init();
}

QgsComposerArrow::QgsComposerArrow( const QPointF& startPoint, const QPointF& stopPoint, QgsComposition* c )
    : QgsComposerItem( c )
    , mStartPoint( startPoint )
    , mStopPoint( stopPoint )
    , mMarkerMode( DefaultMarker )
    , mArrowHeadOutlineWidth( 1.0 )
    , mArrowHeadOutlineColor( Qt::black )
    , mArrowHeadFillColor( Qt::black )
    , mBoundsBehaviour( 24 )
    , mLineSymbol( 0 )
{
  mStartXIdx = mStopPoint.x() < mStartPoint.x();
  mStartYIdx = mStopPoint.y() < mStartPoint.y();
  init();
  adaptItemSceneRect();
}

QgsComposerArrow::~QgsComposerArrow()
{
  delete mLineSymbol;
}

void QgsComposerArrow::init()
{
  setArrowHeadWidth( 4 );
  mPen.setColor( mArrowHeadOutlineColor );
  mPen.setWidthF( 1 );
  mBrush.setColor( mArrowHeadFillColor );
  createDefaultLineSymbol();

  //default to no background
  setBackgroundEnabled( false );
}


void QgsComposerArrow::createDefaultLineSymbol()
{
  delete mLineSymbol;
  QgsStringMap properties;
  properties.insert( "color", "0,0,0,255" );
  properties.insert( "width", "1" );
  properties.insert( "capstyle", "square" );
  mLineSymbol = QgsLineSymbolV2::createSimple( properties );
}

void QgsComposerArrow::paint( QPainter* painter, const QStyleOptionGraphicsItem *itemStyle, QWidget *pWidget )
{
  Q_UNUSED( itemStyle );
  Q_UNUSED( pWidget );
  if ( !painter || !painter->device() )
  {
    return;
  }
  if ( !shouldDrawItem() )
  {
    return;
  }

  drawBackground( painter );

  painter->save();
  //antialiasing on
  painter->setRenderHint( QPainter::Antialiasing, true );

  //draw line section
  drawLine( painter );

  //draw arrowhead if required
  if ( mMarkerMode != NoMarker )
  {
    painter->setBrush( mBrush );
    painter->setPen( mPen );

    if ( mMarkerMode == DefaultMarker )
    {
      drawHardcodedMarker( painter, EndMarker );
    }
    else if ( mMarkerMode == SVGMarker )
    {
      drawSVGMarker( painter, StartMarker, mStartMarkerFile );
      drawSVGMarker( painter, EndMarker, mEndMarkerFile );
    }
  }

  painter->restore();

  drawFrame( painter );
  if ( isSelected() )
  {
    drawSelectionBoxes( painter );
  }
}

void QgsComposerArrow::setSceneRect( const QRectF& rectangle )
{
  //update rect for data defined size and position
  QRectF evaluatedRect = evalItemRect( rectangle );

  if ( evaluatedRect.width() < 0 )
  {
    mStartXIdx = 1 - mStartXIdx;
  }
  if ( evaluatedRect.height() < 0 )
  {
    mStartYIdx = 1 - mStartYIdx;
  }

  double margin = computeMarkerMargin();

  // Ensure the rectangle is at least as large as needed to include the markers
  QRectF rect = rectangle.united( QRectF( evaluatedRect.x(), evaluatedRect.y(), 2. * margin, 2. * margin ) );

  // Compute new start and stop positions
  double x[2] = {rect.x(), rect.x() + rect.width()};
  double y[2] = {rect.y(), rect.y() + rect.height()};

  double xsign = x[mStartXIdx] < x[1 - mStartXIdx] ? 1.0 : -1.0;
  double ysign = y[mStartYIdx] < y[1 - mStartYIdx] ? 1.0 : -1.0;

  mStartPoint = QPointF( x[mStartXIdx] + xsign * margin, y[mStartYIdx] + ysign * margin );
  mStopPoint = QPointF( x[1 - mStartXIdx] - xsign * margin, y[1 - mStartYIdx] - ysign * margin );

  QgsComposerItem::setSceneRect( rect );
}

void QgsComposerArrow::drawLine( QPainter *painter )
{
  if ( ! mLineSymbol || ! mComposition )
  {
    return;
  }

  QPaintDevice* thePaintDevice = painter->device();
  painter->save();
  //setup painter scaling to dots so that raster symbology is drawn to scale
  double dotsPerMM = thePaintDevice->logicalDpiX() / 25.4;
  painter->scale( 1 / dotsPerMM, 1 / dotsPerMM ); //scale painter from mm to dots

  //setup render context
  QgsMapSettings ms = mComposition->mapSettings();
  //context units should be in dots
  ms.setOutputDpi( painter->device()->logicalDpiX() );
  QgsRenderContext context = QgsRenderContext::fromMapSettings( ms );
  context.setForceVectorOutput( true );
  context.setPainter( painter );
  QgsExpressionContext* expressionContext = createExpressionContext();
  context.setExpressionContext( *expressionContext );
  delete expressionContext;

  //line scaled to dots
  QPolygonF line;
  line << QPointF( mStartPoint.x() - pos().x(), mStartPoint.y() - pos().y() ) * dotsPerMM
  << QPointF( mStopPoint.x() - pos().x(), mStopPoint.y() - pos().y() ) * dotsPerMM;

  mLineSymbol->startRender( context );
  mLineSymbol->renderPolyline( line, 0, context );
  mLineSymbol->stopRender( context );
  painter->restore();

}

void QgsComposerArrow::drawHardcodedMarker( QPainter *p, MarkerType type )
{
  Q_UNUSED( type );
  if ( mBoundsBehaviour == 22 )
  {
    //if arrow was created in versions prior to 2.4, use the old rendering style
    QgsComposerUtils::drawArrowHead( p, mStopPoint.x() - pos().x(), mStopPoint.y() - pos().y(), QgsComposerUtils::angle( mStartPoint, mStopPoint ), mArrowHeadWidth );
  }
  else
  {
    QVector2D dir = QVector2D( mStopPoint - mStartPoint ).normalized();
    QPointF stop = mStopPoint + ( dir * 0.5 * mArrowHeadWidth ).toPointF();
    QgsComposerUtils::drawArrowHead( p, stop.x() - pos().x(), stop.y() - pos().y(), QgsComposerUtils::angle( mStartPoint, stop ), mArrowHeadWidth );
  }
}

void QgsComposerArrow::drawSVGMarker( QPainter* p, MarkerType type, const QString &markerPath )
{
  Q_UNUSED( markerPath );
  double ang = QgsComposerUtils::angle( mStartPoint, mStopPoint );

  double arrowHeadHeight;
  if ( type == StartMarker )
  {
    arrowHeadHeight = mStartArrowHeadHeight;
  }
  else
  {
    arrowHeadHeight = mStopArrowHeadHeight;
  }
  if ( mArrowHeadWidth <= 0 || arrowHeadHeight <= 0 )
  {
    //bad image size
    return;
  }

  QPointF imageFixPoint;
  imageFixPoint.setX( mArrowHeadWidth / 2.0 );
  QPointF canvasPoint;
  if ( type == StartMarker )
  {
    canvasPoint = QPointF( mStartPoint.x() - pos().x(), mStartPoint.y() - pos().y() );
    imageFixPoint.setY( mStartArrowHeadHeight );
  }
  else //end marker
  {
    canvasPoint = QPointF( mStopPoint.x() - pos().x(), mStopPoint.y() - pos().y() );
    imageFixPoint.setY( 0 );
  }

  //rasterize svg
  QSvgRenderer r;
  if ( type == StartMarker )
  {
    if ( mStartMarkerFile.isEmpty() || !r.load( mStartMarkerFile ) )
    {
      return;
    }
  }
  else //end marker
  {
    if ( mEndMarkerFile.isEmpty() || !r.load( mEndMarkerFile ) )
    {
      return;
    }
  }

  p->save();
  p->setRenderHint( QPainter::Antialiasing );
  if ( mBoundsBehaviour == 22 )
  {
    //if arrow was created in versions prior to 2.4, use the old rendering style
    //rotate image fix point for backtransform
    QPointF fixPoint;
    if ( type == StartMarker )
    {
      fixPoint.setX( 0 ); fixPoint.setY( arrowHeadHeight / 2.0 );
    }
    else
    {
      fixPoint.setX( 0 ); fixPoint.setY( -arrowHeadHeight / 2.0 );
    }
    QPointF rotatedFixPoint;
    double angleRad = ang / 180 * M_PI;
    rotatedFixPoint.setX( fixPoint.x() * cos( angleRad ) + fixPoint.y() * -sin( angleRad ) );
    rotatedFixPoint.setY( fixPoint.x() * sin( angleRad ) + fixPoint.y() * cos( angleRad ) );
    p->translate( canvasPoint.x() - rotatedFixPoint.x(), canvasPoint.y() - rotatedFixPoint.y() );
  }
  else
  {
    p->translate( canvasPoint.x(), canvasPoint.y() );
  }

  p->rotate( ang );
  p->translate( -mArrowHeadWidth / 2.0, -arrowHeadHeight / 2.0 );
  r.render( p, QRectF( 0, 0, mArrowHeadWidth, arrowHeadHeight ) );
  p->restore();

  return;
}

void QgsComposerArrow::setStartMarker( const QString& svgPath )
{
  QSvgRenderer r;
  mStartMarkerFile = svgPath;
  if ( svgPath.isEmpty() || !r.load( svgPath ) )
  {
    mStartArrowHeadHeight = 0;
  }
  else
  {
    //calculate mArrowHeadHeight from svg file and mArrowHeadWidth
    QRect viewBox = r.viewBox();
    mStartArrowHeadHeight = mArrowHeadWidth / viewBox.width() * viewBox.height();
  }
  adaptItemSceneRect();
}

void QgsComposerArrow::setEndMarker( const QString& svgPath )
{
  QSvgRenderer r;
  mEndMarkerFile = svgPath;
  if ( svgPath.isEmpty() || !r.load( svgPath ) )
  {
    mStopArrowHeadHeight = 0;
  }
  else
  {
    //calculate mArrowHeadHeight from svg file and mArrowHeadWidth
    QRect viewBox = r.viewBox();
    mStopArrowHeadHeight = mArrowHeadWidth / viewBox.width() * viewBox.height();
  }
  adaptItemSceneRect();
}

QColor QgsComposerArrow::arrowColor() const
{
  if ( mLineSymbol )
  {
    return mLineSymbol->color();
  }

  return Qt::black;
}

void QgsComposerArrow::setArrowColor( const QColor &c )
{
  if ( mLineSymbol )
  {
    mLineSymbol->setColor( c );
  }
  mArrowHeadOutlineColor = c;
  mArrowHeadFillColor = c;
  mPen.setColor( c );
  mBrush.setColor( c );
}

void QgsComposerArrow::setArrowHeadOutlineColor( const QColor &color )
{
  mArrowHeadOutlineColor = color;
  mPen.setColor( color );
}

void QgsComposerArrow::setArrowHeadFillColor( const QColor &color )
{
  mArrowHeadFillColor = color;
  mBrush.setColor( color );
}

void QgsComposerArrow::setOutlineWidth( double width )
{
  if ( mLineSymbol )
  {
    mLineSymbol->setWidth( width );
  }
  mArrowHeadOutlineWidth = width;
  mPen.setWidthF( mArrowHeadOutlineWidth );

  adaptItemSceneRect();
}

double QgsComposerArrow::outlineWidth() const
{
  if ( mLineSymbol )
  {
    return mLineSymbol->width();
  }

  return 0;
}

void QgsComposerArrow::setArrowHeadOutlineWidth( const double width )
{
  mArrowHeadOutlineWidth = width;
  mPen.setWidthF( mArrowHeadOutlineWidth );

  adaptItemSceneRect();
}

void QgsComposerArrow::setLineSymbol( QgsLineSymbolV2 *symbol )
{
  delete mLineSymbol;
  mLineSymbol = symbol;
}

void QgsComposerArrow::setArrowHeadWidth( double width )
{
  mArrowHeadWidth = width;
  setStartMarker( mStartMarkerFile );
  setEndMarker( mEndMarkerFile );
  adaptItemSceneRect();
}

double QgsComposerArrow::computeMarkerMargin() const
{
  double margin = 0;

  if ( mBoundsBehaviour == 22 )
  {
    //if arrow was created in versions prior to 2.4, use the old rendering style
    if ( mMarkerMode == DefaultMarker )
    {
      margin = mPen.widthF() / 2.0 + mArrowHeadWidth / 2.0;
    }
    else if ( mMarkerMode == NoMarker )
    {
      margin = mPen.widthF() / 2.0;
    }
    else if ( mMarkerMode == SVGMarker )
    {
      double maxArrowHeight = qMax( mStartArrowHeadHeight, mStopArrowHeadHeight );
      margin = mPen.widthF() / 2 + qMax( mArrowHeadWidth / 2.0, maxArrowHeight / 2.0 );
    }
  }
  else
  {
    if ( mMarkerMode == DefaultMarker )
    {
      margin = mPen.widthF() / std::sqrt( 2.0 ) + mArrowHeadWidth / 2.0;
    }
    else if ( mMarkerMode == NoMarker )
    {
      margin = mPen.widthF() / std::sqrt( 2.0 );
    }
    else if ( mMarkerMode == SVGMarker )
    {
      double startMarkerMargin = std::sqrt( 0.25 * ( mStartArrowHeadHeight * mStartArrowHeadHeight + mArrowHeadWidth * mArrowHeadWidth ) );
      double stopMarkerMargin = std::sqrt( 0.25 * ( mStopArrowHeadHeight * mStopArrowHeadHeight + mArrowHeadWidth * mArrowHeadWidth ) );
      double markerMargin = qMax( startMarkerMargin, stopMarkerMargin );
      margin = qMax( mPen.widthF() / std::sqrt( 2.0 ), markerMargin );
    }
  }
  return margin;
}

void QgsComposerArrow::adaptItemSceneRect()
{
  //rectangle containing start and end point
  QRectF rect = QRectF( qMin( mStartPoint.x(), mStopPoint.x() ), qMin( mStartPoint.y(), mStopPoint.y() ),
                        qAbs( mStopPoint.x() - mStartPoint.x() ), qAbs( mStopPoint.y() - mStartPoint.y() ) );
  double enlarge = computeMarkerMargin();
  rect.adjust( -enlarge, -enlarge, enlarge, enlarge );
  QgsComposerItem::setSceneRect( rect );
}

void QgsComposerArrow::setMarkerMode( MarkerMode mode )
{
  mMarkerMode = mode;
  adaptItemSceneRect();
}

bool QgsComposerArrow::writeXML( QDomElement& elem, QDomDocument & doc ) const
{
  QDomElement composerArrowElem = doc.createElement( "ComposerArrow" );
  composerArrowElem.setAttribute( "arrowHeadWidth", QString::number( mArrowHeadWidth ) );
  composerArrowElem.setAttribute( "arrowHeadFillColor", QgsSymbolLayerV2Utils::encodeColor( mArrowHeadFillColor ) );
  composerArrowElem.setAttribute( "arrowHeadOutlineColor", QgsSymbolLayerV2Utils::encodeColor( mArrowHeadOutlineColor ) );
  composerArrowElem.setAttribute( "outlineWidth", QString::number( mArrowHeadOutlineWidth ) );
  composerArrowElem.setAttribute( "markerMode", mMarkerMode );
  composerArrowElem.setAttribute( "startMarkerFile", mStartMarkerFile );
  composerArrowElem.setAttribute( "endMarkerFile", mEndMarkerFile );
  composerArrowElem.setAttribute( "boundsBehaviourVersion", QString::number( mBoundsBehaviour ) );

  QDomElement styleElem = doc.createElement( "lineStyle" );
  QDomElement lineStyleElem = QgsSymbolLayerV2Utils::saveSymbol( QString(), mLineSymbol, doc );
  styleElem.appendChild( lineStyleElem );
  composerArrowElem.appendChild( styleElem );

  //start point
  QDomElement startPointElem = doc.createElement( "StartPoint" );
  startPointElem.setAttribute( "x", QString::number( mStartPoint.x() ) );
  startPointElem.setAttribute( "y", QString::number( mStartPoint.y() ) );
  composerArrowElem.appendChild( startPointElem );

  //stop point
  QDomElement stopPointElem = doc.createElement( "StopPoint" );
  stopPointElem.setAttribute( "x", QString::number( mStopPoint.x() ) );
  stopPointElem.setAttribute( "y", QString::number( mStopPoint.y() ) );
  composerArrowElem.appendChild( stopPointElem );

  elem.appendChild( composerArrowElem );
  return _writeXML( composerArrowElem, doc );
}

bool QgsComposerArrow::readXML( const QDomElement& itemElem, const QDomDocument& doc )
{
  mArrowHeadWidth = itemElem.attribute( "arrowHeadWidth", "2.0" ).toDouble();
  mArrowHeadFillColor = QgsSymbolLayerV2Utils::decodeColor( itemElem.attribute( "arrowHeadFillColor", "0,0,0,255" ) );
  mArrowHeadOutlineColor = QgsSymbolLayerV2Utils::decodeColor( itemElem.attribute( "arrowHeadOutlineColor", "0,0,0,255" ) );
  mArrowHeadOutlineWidth = itemElem.attribute( "outlineWidth", "1.0" ).toDouble();
  setStartMarker( itemElem.attribute( "startMarkerFile", "" ) );
  setEndMarker( itemElem.attribute( "endMarkerFile", "" ) );
  mMarkerMode = QgsComposerArrow::MarkerMode( itemElem.attribute( "markerMode", "0" ).toInt() );
  //if bounds behaviour version is not set, default to 2.2 behaviour
  mBoundsBehaviour = itemElem.attribute( "boundsBehaviourVersion", "22" ).toInt();

  //arrow style
  QDomElement styleElem = itemElem.firstChildElement( "lineStyle" );
  if ( !styleElem.isNull() )
  {
    QDomElement lineStyleElem = styleElem.firstChildElement( "symbol" );
    if ( !lineStyleElem.isNull() )
    {
      delete mLineSymbol;
      mLineSymbol = QgsSymbolLayerV2Utils::loadSymbol<QgsLineSymbolV2>( lineStyleElem );
    }
  }
  else
  {
    //old project file, read arrow width and color
    delete mLineSymbol;

    QgsStringMap properties;
    properties.insert( "width", itemElem.attribute( "outlineWidth", "1.0" ) );

    if ( mBoundsBehaviour == 22 )
    {
      //if arrow was created in versions prior to 2.4, use the old rendering style
      properties.insert( "capstyle", "flat" );
    }
    else
    {
      properties.insert( "capstyle", "square" );
    }
    int red = 0;
    int blue = 0;
    int green = 0;
    int alpha = 255;

    QDomNodeList arrowColorList = itemElem.elementsByTagName( "ArrowColor" );
    if ( arrowColorList.size() > 0 )
    {
      QDomElement arrowColorElem = arrowColorList.at( 0 ).toElement();
      red = arrowColorElem.attribute( "red", "0" ).toInt();
      green = arrowColorElem.attribute( "green", "0" ).toInt();
      blue = arrowColorElem.attribute( "blue", "0" ).toInt();
      alpha = arrowColorElem.attribute( "alpha", "255" ).toInt();
      mArrowHeadFillColor = QColor( red, green, blue, alpha );
      mArrowHeadOutlineColor = QColor( red, green, blue, alpha );
    }
    properties.insert( "color", QString( "%1,%2,%3,%4" ).arg( red ).arg( green ).arg( blue ).arg( alpha ) );
    mLineSymbol = QgsLineSymbolV2::createSimple( properties );
  }

  mPen.setColor( mArrowHeadOutlineColor );
  mPen.setWidthF( mArrowHeadOutlineWidth );
  mBrush.setColor( mArrowHeadFillColor );

  //restore general composer item properties
  //needs to be before start point / stop point because setSceneRect()
  QDomNodeList composerItemList = itemElem.elementsByTagName( "ComposerItem" );
  if ( composerItemList.size() > 0 )
  {
    QDomElement composerItemElem = composerItemList.at( 0 ).toElement();
    _readXML( composerItemElem, doc );
  }

  //start point
  QDomNodeList startPointList = itemElem.elementsByTagName( "StartPoint" );
  if ( startPointList.size() > 0 )
  {
    QDomElement startPointElem = startPointList.at( 0 ).toElement();
    mStartPoint.setX( startPointElem.attribute( "x", "0.0" ).toDouble() );
    mStartPoint.setY( startPointElem.attribute( "y", "0.0" ).toDouble() );
  }

  //stop point
  QDomNodeList stopPointList = itemElem.elementsByTagName( "StopPoint" );
  if ( stopPointList.size() > 0 )
  {
    QDomElement stopPointElem = stopPointList.at( 0 ).toElement();
    mStopPoint.setX( stopPointElem.attribute( "x", "0.0" ).toDouble() );
    mStopPoint.setY( stopPointElem.attribute( "y", "0.0" ).toDouble() );
  }

  mStartXIdx = mStopPoint.x() < mStartPoint.x();
  mStartYIdx = mStopPoint.y() < mStartPoint.y();

  adaptItemSceneRect();
  emit itemChanged();
  return true;
}


