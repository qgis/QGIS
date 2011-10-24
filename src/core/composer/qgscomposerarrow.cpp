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
#include <QPainter>
#include <QSvgRenderer>

#include <cmath>

QgsComposerArrow::QgsComposerArrow( QgsComposition* c )
    : QgsComposerItem( c )
    , mStartPoint( 0, 0 )
    , mStopPoint( 0, 0 )
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
  arrowPen.setCapStyle( Qt::FlatCap );
  arrowPen.setColor( mArrowColor );
  painter->setPen( arrowPen );
  painter->setBrush( QBrush( mArrowColor ) );
  painter->drawLine( QPointF( mStartPoint.x() - transform().dx(), mStartPoint.y() - transform().dy() ), QPointF( mStopPoint.x() - transform().dx(), mStopPoint.y() - transform().dy() ) );

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
  //maintain the relative position of start and stop point in the rectangle
  double startPointXPos = ( mStartPoint.x() - transform().dx() ) / rect().width();
  double startPointYPos = ( mStartPoint.y() - transform().dy() ) / rect().height();
  double stopPointXPos = ( mStopPoint.x() - transform().dx() ) / rect().width();
  double stopPointYPos = ( mStopPoint.y() - transform().dy() ) / rect().height();

  mStartPoint.setX( rectangle.left() + startPointXPos * rectangle.width() );
  mStartPoint.setY( rectangle.top() + startPointYPos * rectangle.height() );
  mStopPoint.setX( rectangle.left() + stopPointXPos * rectangle.width() );
  mStopPoint.setY( rectangle.top() + stopPointYPos * rectangle.height() );

  adaptItemSceneRect();
}

void QgsComposerArrow::drawHardcodedMarker( QPainter *p, MarkerType type )
{
  Q_UNUSED( type );
  QBrush arrowBrush = p->brush();
  arrowBrush.setColor( mArrowColor );
  p->setBrush( arrowBrush );
  drawArrowHead( p, mStopPoint.x() - transform().dx(), mStopPoint.y() - transform().dy(), angle( mStartPoint, mStopPoint ), mArrowHeadWidth );
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
    canvasPoint = QPointF( mStartPoint.x() - transform().dx(), mStartPoint.y() - transform().dy() );
    imageFixPoint.setY( mStartArrowHeadHeight );
  }
  else //end marker
  {
    canvasPoint = QPointF( mStopPoint.x() - transform().dx(), mStopPoint.y() - transform().dy() );
    imageFixPoint.setY( 0 );
  }

  //rasterize svg
  QSvgRenderer r;
  if ( type == StartMarker )
  {
    if ( !r.load( mStartMarkerFile ) )
    {
      return;
    }
  }
  else //end marker
  {
    if ( !r.load( mEndMarkerFile ) )
    {
      return;
    }
  }

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


  QPainter imagePainter( &markerImage );
  r.render( &imagePainter );

  p->save();
  p->translate( canvasPoint.x() - rotatedFixPoint.x() , canvasPoint.y() - rotatedFixPoint.y() );
  p->rotate( ang );
  p->translate( -mArrowHeadWidth / 2.0, -arrowHeadHeight / 2.0 );

  p->drawImage( QRectF( 0, 0, mArrowHeadWidth, arrowHeadHeight ), markerImage, QRectF( 0, 0, imageWidth, imageHeight ) );
  p->restore();

  return;
}

void QgsComposerArrow::setStartMarker( const QString& svgPath )
{
  QSvgRenderer r;
  if ( !r.load( svgPath ) )
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
  if ( !r.load( svgPath ) )
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

void QgsComposerArrow::adaptItemSceneRect()
{
  //rectangle containing start and end point
  QRectF rect = QRectF( qMin( mStartPoint.x(), mStopPoint.x() ), qMin( mStartPoint.y(), mStopPoint.y() ),
                        qAbs( mStopPoint.x() - mStartPoint.x() ), qAbs( mStopPoint.y() - mStartPoint.y() ) );
  double enlarge = 0;
  if ( mMarkerMode == DefaultMarker )
  {
    enlarge = mPen.widthF() / 2.0 + mArrowHeadWidth / 2.0;
  }
  else if ( mMarkerMode == NoMarker )
  {
    enlarge = mPen.widthF() / 2.0;
  }
  else if ( mMarkerMode == SVGMarker )
  {
    double maxArrowHeight = qMax( mStartArrowHeadHeight, mStopArrowHeadHeight );
    enlarge = mPen.widthF() / 2 + qMax( mArrowHeadWidth / 2.0, maxArrowHeight / 2.0 );
  }

  rect.adjust( -enlarge, -enlarge, enlarge, enlarge );
  QgsComposerItem::setSceneRect( rect );
}

bool QgsComposerArrow::writeXML( QDomElement& elem, QDomDocument & doc ) const
{
  QDomElement composerArrowElem = doc.createElement( "ComposerArrow" );
  composerArrowElem.setAttribute( "outlineWidth", outlineWidth() );
  composerArrowElem.setAttribute( "arrowHeadWidth", mArrowHeadWidth );
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
  startPointElem.setAttribute( "x", mStartPoint.x() );
  startPointElem.setAttribute( "y", mStartPoint.y() );
  composerArrowElem.appendChild( startPointElem );

  //stop point
  QDomElement stopPointElem = doc.createElement( "StopPoint" );
  stopPointElem.setAttribute( "x", mStopPoint.x() );
  stopPointElem.setAttribute( "y", mStopPoint.y() );
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

  adaptItemSceneRect();
  emit itemChanged();
  return true;
}
