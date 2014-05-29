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
    , mArrowColor( QColor( 0, 0, 0 ) )
{
  initGraphicsSettings();
}

QgsComposerArrow::QgsComposerArrow( const QPointF& startPoint, const QPointF& stopPoint, QgsComposition* c )
    : QgsComposerItem( c )
    , mStartPoint( startPoint )
    , mStopPoint( stopPoint )
    , mMarkerMode( DefaultMarker )
    , mArrowColor( QColor( 0, 0, 0 ) )
{
  mStartXIdx = mStopPoint.x() < mStartPoint.x();
  mStartYIdx = mStopPoint.y() < mStartPoint.y();
  initGraphicsSettings();
  adaptItemSceneRect();
}

QgsComposerArrow::~QgsComposerArrow()
{

}

void QgsComposerArrow::initGraphicsSettings()
{
  setArrowHeadWidth( 4 );
  mPen.setColor( QColor( 0, 0, 0 ) );
  mPen.setWidthF( 1 );

  //set composer item brush and pen to transparent white by default
  setPen( QPen( QColor( 255, 255, 255, 0 ) ) );
  setBrush( QBrush( QColor( 255, 255, 255, 0 ) ) );
}

void QgsComposerArrow::paint( QPainter* painter, const QStyleOptionGraphicsItem *itemStyle, QWidget *pWidget )
{
  Q_UNUSED( itemStyle );
  Q_UNUSED( pWidget );
  if ( !painter )
  {
    return;
  }

  drawBackground( painter );

  //draw arrow
  QPen arrowPen = mPen;
  arrowPen.setColor( mArrowColor );
  painter->setPen( arrowPen );
  painter->setBrush( QBrush( mArrowColor ) );
  painter->drawLine( QPointF( mStartPoint.x() - pos().x(), mStartPoint.y() - pos().y() ), QPointF( mStopPoint.x() - pos().x(), mStopPoint.y() - pos().y() ) );

  if ( mMarkerMode == DefaultMarker )
  {
    drawHardcodedMarker( painter, EndMarker );
  }
  else if ( mMarkerMode == SVGMarker )
  {
    drawSVGMarker( painter, StartMarker, mStartMarkerFile );
    drawSVGMarker( painter, EndMarker, mEndMarkerFile );
  }

  drawFrame( painter );
  if ( isSelected() )
  {
    drawSelectionBoxes( painter );
  }
}

void QgsComposerArrow::setSceneRect( const QRectF& rectangle )
{
  if ( rectangle.width() < 0 )
  {
    mStartXIdx = 1 - mStartXIdx;
  }
  if ( rectangle.height() < 0 )
  {
    mStartYIdx = 1 - mStartYIdx;
  }

  double margin = computeMarkerMargin();

  // Ensure the rectangle is at least as large as needed to include the markers
  QRectF rect = rectangle.united( QRectF( rectangle.x(), rectangle.y(), 2. * margin, 2. * margin ) );

  // Compute new start and stop positions
  double x[2] = {rect.x(), rect.x() + rect.width()};
  double y[2] = {rect.y(), rect.y() + rect.height()};

  double xsign = x[mStartXIdx] < x[1 - mStartXIdx] ? 1.0 : -1.0;
  double ysign = y[mStartYIdx] < y[1 - mStartYIdx] ? 1.0 : -1.0;

  mStartPoint = QPointF( x[mStartXIdx] + xsign * margin, y[mStartYIdx] + ysign * margin );
  mStopPoint = QPointF( x[1 - mStartXIdx] - xsign * margin, y[1 - mStartYIdx] - ysign * margin );

  QgsComposerItem::setSceneRect( rect );
}

void QgsComposerArrow::drawHardcodedMarker( QPainter *p, MarkerType type )
{
  Q_UNUSED( type );
  QBrush arrowBrush = p->brush();
  arrowBrush.setColor( mArrowColor );
  p->setBrush( arrowBrush );
  QVector2D dir = QVector2D( mStopPoint - mStartPoint ).normalized();
  QPointF stop = mStopPoint + ( dir * 0.5 * mArrowHeadWidth ).toPointF();
  drawArrowHead( p, stop.x() - pos().x(), stop.y() - pos().y(), angle( mStartPoint, stop ), mArrowHeadWidth );
}

void QgsComposerArrow::drawSVGMarker( QPainter* p, MarkerType type, const QString &markerPath )
{
  Q_UNUSED( markerPath );
  double ang = angle( mStartPoint, mStopPoint );

  double arrowHeadHeight;
  if ( type == StartMarker )
  {
    arrowHeadHeight = mStartArrowHeadHeight;
  }
  else
  {
    arrowHeadHeight = mStopArrowHeadHeight;
  }

  //prepare paint device
  int dpi = ( p->device()->logicalDpiX() + p->device()->logicalDpiY() ) / 2;
  double viewScaleFactor = horizontalViewScaleFactor();
  int imageWidth = mArrowHeadWidth / 25.4 * dpi;
  int imageHeight = arrowHeadHeight / 25.4 * dpi;

  //make nicer preview
  if ( mComposition && mComposition->plotStyle() == QgsComposition::Preview )
  {
    imageWidth *= qMin( viewScaleFactor, 10.0 );
    imageHeight *= qMin( viewScaleFactor, 10.0 );
  }
  QImage markerImage( imageWidth, imageHeight, QImage::Format_ARGB32 );
  QColor markerBG( 255, 255, 255, 0 ); //transparent white background
  markerImage.fill( markerBG.rgba() );

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

  QPainter imagePainter( &markerImage );
  r.render( &imagePainter );

  p->save();
  p->translate( canvasPoint.x() , canvasPoint.y() );
  p->rotate( ang );
  p->translate( -mArrowHeadWidth / 2.0, -arrowHeadHeight / 2.0 );

  p->drawImage( QRectF( 0, 0, mArrowHeadWidth, arrowHeadHeight ), markerImage, QRectF( 0, 0, imageWidth, imageHeight ) );
  p->restore();

  return;
}

void QgsComposerArrow::setStartMarker( const QString& svgPath )
{
  QSvgRenderer r;
  if ( svgPath.isEmpty() || !r.load( svgPath ) )
  {
    return;
    // mStartArrowHeadHeight = 0;
  }
  mStartMarkerFile = svgPath;

  //calculate mArrowHeadHeight from svg file and mArrowHeadWidth
  QRect viewBox = r.viewBox();
  mStartArrowHeadHeight = mArrowHeadWidth / viewBox.width() * viewBox.height();
  adaptItemSceneRect();
}

void QgsComposerArrow::setEndMarker( const QString& svgPath )
{
  QSvgRenderer r;
  if ( svgPath.isEmpty() || !r.load( svgPath ) )
  {
    return;
    // mStopArrowHeadHeight = 0;
  }
  mEndMarkerFile = svgPath;

  //calculate mArrowHeadHeight from svg file and mArrowHeadWidth
  QRect viewBox = r.viewBox();
  mStopArrowHeadHeight = mArrowHeadWidth / viewBox.width() * viewBox.height();
  adaptItemSceneRect();
}

void QgsComposerArrow::setOutlineWidth( double width )
{
  mPen.setWidthF( width );
  adaptItemSceneRect();
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
  composerArrowElem.setAttribute( "outlineWidth", QString::number( outlineWidth() ) );
  composerArrowElem.setAttribute( "arrowHeadWidth", QString::number( mArrowHeadWidth ) );
  composerArrowElem.setAttribute( "markerMode", mMarkerMode );
  composerArrowElem.setAttribute( "startMarkerFile", mStartMarkerFile );
  composerArrowElem.setAttribute( "endMarkerFile", mEndMarkerFile );

  //arrow color
  QDomElement arrowColorElem = doc.createElement( "ArrowColor" );
  arrowColorElem.setAttribute( "red", mArrowColor.red() );
  arrowColorElem.setAttribute( "green", mArrowColor.green() );
  arrowColorElem.setAttribute( "blue", mArrowColor.blue() );
  arrowColorElem.setAttribute( "alpha", mArrowColor.alpha() );
  composerArrowElem.appendChild( arrowColorElem );

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
  mPen.setWidthF( itemElem.attribute( "outlineWidth", "1.0" ).toDouble() );
  setStartMarker( itemElem.attribute( "startMarkerFile", "" ) );
  setEndMarker( itemElem.attribute( "endMarkerFile", "" ) );
  mMarkerMode = QgsComposerArrow::MarkerMode( itemElem.attribute( "markerMode", "0" ).toInt() );

  //arrow color
  QDomNodeList arrowColorList = itemElem.elementsByTagName( "ArrowColor" );
  if ( arrowColorList.size() > 0 )
  {
    QDomElement arrowColorElem = arrowColorList.at( 0 ).toElement();
    int red = arrowColorElem.attribute( "red", "0" ).toInt();
    int green = arrowColorElem.attribute( "green", "0" ).toInt();
    int blue = arrowColorElem.attribute( "blue", "0" ).toInt();
    int alpha = arrowColorElem.attribute( "alpha", "255" ).toInt();
    mArrowColor = QColor( red, green, blue, alpha );
  }

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


