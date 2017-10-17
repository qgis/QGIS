/***************************************************************************
                             qgsannotation.cpp
                             -----------------
    begin                : January 2017
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

#include "qgsannotation.h"
#include "qgssymbollayerutils.h"
#include "qgsmaplayer.h"
#include "qgsproject.h"
#include <QPen>
#include <QPainter>

QgsAnnotation::QgsAnnotation( QObject *parent )
  : QObject( parent )
  , mMarkerSymbol( new QgsMarkerSymbol() )
{
  QgsStringMap props;
  props.insert( QStringLiteral( "color" ), QStringLiteral( "white" ) );
  props.insert( QStringLiteral( "style" ), QStringLiteral( "solid" ) );
  props.insert( QStringLiteral( "style_border" ), QStringLiteral( "solid" ) );
  props.insert( QStringLiteral( "color_border" ), QStringLiteral( "black" ) );
  props.insert( QStringLiteral( "width_border" ), QStringLiteral( "0.3" ) );
  props.insert( QStringLiteral( "joinstyle" ), QStringLiteral( "miter" ) );
  mFillSymbol.reset( QgsFillSymbol::createSimple( props ) );
}

void QgsAnnotation::setVisible( bool visible )
{
  if ( mVisible == visible )
    return;

  mVisible = visible;
  emit appearanceChanged();
}

void QgsAnnotation::setHasFixedMapPosition( bool fixed )
{
  if ( mHasFixedMapPosition == fixed )
    return;

  mHasFixedMapPosition = fixed;
  updateBalloon();
  emit moved();
}

void QgsAnnotation::setMapPosition( const QgsPointXY &position )
{
  mMapPosition = position;
  emit moved();
}

void QgsAnnotation::setMapPositionCrs( const QgsCoordinateReferenceSystem &crs )
{
  mMapPositionCrs = crs;
  emit moved();
}

void QgsAnnotation::setRelativePosition( QPointF position )
{
  mRelativePosition = position;
  emit moved();
}

void QgsAnnotation::setFrameOffsetFromReferencePoint( QPointF offset )
{
  mOffsetFromReferencePoint = offset;
  updateBalloon();
  emit moved();
  emit appearanceChanged();
}

void QgsAnnotation::setFrameSize( QSizeF size )
{
  QSizeF frameSize = minimumFrameSize().expandedTo( size ); //don't allow frame sizes below minimum
  mFrameSize = frameSize;
  updateBalloon();
  emit moved();
  emit appearanceChanged();
}

void QgsAnnotation::setContentsMargin( const QgsMargins &margins )
{
  mContentsMargins = margins;
  emit appearanceChanged();
}

void QgsAnnotation::setFillSymbol( QgsFillSymbol *symbol )
{
  mFillSymbol.reset( symbol );
  emit appearanceChanged();
}

void QgsAnnotation::render( QgsRenderContext &context ) const
{
  QPainter *painter = context.painter();
  if ( !painter )
  {
    return;
  }

  painter->save();
  drawFrame( context );
  if ( mHasFixedMapPosition )
  {
    drawMarkerSymbol( context );
  }
  if ( mHasFixedMapPosition )
  {
    painter->translate( mOffsetFromReferencePoint.x() + context.convertToPainterUnits( mContentsMargins.left(), QgsUnitTypes::RenderMillimeters ),
                        mOffsetFromReferencePoint.y() + context.convertToPainterUnits( mContentsMargins.top(), QgsUnitTypes::RenderMillimeters ) );
  }
  else
  {
    painter->translate( context.convertToPainterUnits( mContentsMargins.left(), QgsUnitTypes::RenderMillimeters ),
                        context.convertToPainterUnits( mContentsMargins.top(), QgsUnitTypes::RenderMillimeters ) );
  }
  QSizeF size( mFrameSize.width() - context.convertToPainterUnits( mContentsMargins.left() + mContentsMargins.right(), QgsUnitTypes::RenderMillimeters ),
               mFrameSize.height() - context.convertToPainterUnits( mContentsMargins.top() + mContentsMargins.bottom(), QgsUnitTypes::RenderMillimeters ) );

  renderAnnotation( context, size );
  painter->restore();
}

void QgsAnnotation::setMarkerSymbol( QgsMarkerSymbol *symbol )
{
  mMarkerSymbol.reset( symbol );
  emit appearanceChanged();
}

void QgsAnnotation::setMapLayer( QgsMapLayer *layer )
{
  mMapLayer = layer;
  emit mapLayerChanged();
}

void QgsAnnotation::setAssociatedFeature( const QgsFeature &feature )
{
  mFeature = feature;
}

QSizeF QgsAnnotation::minimumFrameSize() const
{
  return QSizeF( 0, 0 );
}

void QgsAnnotation::updateBalloon()
{
  //first test if the point is in the frame. In that case we don't need a balloon.
  if ( !mHasFixedMapPosition ||
       ( mOffsetFromReferencePoint.x() < 0 && ( mOffsetFromReferencePoint.x() + mFrameSize.width() ) > 0
         && mOffsetFromReferencePoint.y() < 0 && ( mOffsetFromReferencePoint.y() + mFrameSize.height() ) > 0 ) )
  {
    mBalloonSegment = -1;
    return;
  }

  //edge list
  QList<QLineF> segmentList;
  segmentList << segment( 0 );
  segmentList << segment( 1 );
  segmentList << segment( 2 );
  segmentList << segment( 3 );

  //find  closest edge / closest edge point
  double minEdgeDist = DBL_MAX;
  int minEdgeIndex = -1;
  QLineF minEdge;
  QgsPointXY minEdgePoint;
  QgsPointXY origin( 0, 0 );

  for ( int i = 0; i < 4; ++i )
  {
    QLineF currentSegment = segmentList.at( i );
    QgsPointXY currentMinDistPoint;
    double currentMinDist = origin.sqrDistToSegment( currentSegment.x1(), currentSegment.y1(), currentSegment.x2(), currentSegment.y2(), currentMinDistPoint );
    if ( currentMinDist < minEdgeDist )
    {
      minEdgeIndex = i;
      minEdgePoint = currentMinDistPoint;
      minEdgeDist = currentMinDist;
      minEdge = currentSegment;
    }
  }

  if ( minEdgeIndex < 0 )
  {
    return;
  }

  //make that configurable for the item
  double segmentPointWidth = 10;

  mBalloonSegment = minEdgeIndex;
  QPointF minEdgeEnd = minEdge.p2();
  mBalloonSegmentPoint1 = QPointF( minEdgePoint.x(), minEdgePoint.y() );
  if ( std::sqrt( minEdgePoint.sqrDist( minEdgeEnd.x(), minEdgeEnd.y() ) ) < segmentPointWidth )
  {
    mBalloonSegmentPoint1 = pointOnLineWithDistance( minEdge.p2(), minEdge.p1(), segmentPointWidth );
  }

  mBalloonSegmentPoint2 = pointOnLineWithDistance( mBalloonSegmentPoint1, minEdge.p2(), 10 );
}

QLineF QgsAnnotation::segment( int index ) const
{
  if ( mHasFixedMapPosition )
  {
    switch ( index )
    {
      case 0:
        return QLineF( mOffsetFromReferencePoint.x(), mOffsetFromReferencePoint.y(), mOffsetFromReferencePoint.x()
                       + mFrameSize.width(), mOffsetFromReferencePoint.y() );
      case 1:
        return QLineF( mOffsetFromReferencePoint.x() + mFrameSize.width(), mOffsetFromReferencePoint.y(),
                       mOffsetFromReferencePoint.x() + mFrameSize.width(), mOffsetFromReferencePoint.y() + mFrameSize.height() );
      case 2:
        return QLineF( mOffsetFromReferencePoint.x() + mFrameSize.width(), mOffsetFromReferencePoint.y() + mFrameSize.height(),
                       mOffsetFromReferencePoint.x(), mOffsetFromReferencePoint.y() + mFrameSize.height() );
      case 3:
        return QLineF( mOffsetFromReferencePoint.x(), mOffsetFromReferencePoint.y() + mFrameSize.height(),
                       mOffsetFromReferencePoint.x(), mOffsetFromReferencePoint.y() );
      default:
        return QLineF();
    }
  }
  else
  {
    switch ( index )
    {
      case 0:
        return QLineF( 0, 0, mFrameSize.width(), 0 );
      case 1:
        return QLineF( mFrameSize.width(), 0,
                       mFrameSize.width(), mFrameSize.height() );
      case 2:
        return QLineF( mFrameSize.width(), mFrameSize.height(),
                       0, mFrameSize.height() );
      case 3:
        return QLineF( 0, mFrameSize.height(),
                       0, 0 );
      default:
        return QLineF();
    }
  }
}


QPointF QgsAnnotation::pointOnLineWithDistance( QPointF startPoint, QPointF directionPoint, double distance ) const
{
  double dx = directionPoint.x() - startPoint.x();
  double dy = directionPoint.y() - startPoint.y();
  double length = std::sqrt( dx * dx + dy * dy );
  double scaleFactor = distance / length;
  return QPointF( startPoint.x() + dx * scaleFactor, startPoint.y() + dy * scaleFactor );
}

void QgsAnnotation::drawFrame( QgsRenderContext &context ) const
{
  if ( !mFillSymbol )
    return;

  context.painter()->setRenderHint( QPainter::Antialiasing, context.flags() & QgsRenderContext::Antialiasing );

  QPolygonF poly;
  QList<QPolygonF> rings; //empty list
  for ( int i = 0; i < 4; ++i )
  {
    QLineF currentSegment = segment( i );
    poly << currentSegment.p1();
    if ( i == mBalloonSegment && mHasFixedMapPosition )
    {
      poly << mBalloonSegmentPoint1;
      poly << QPointF( 0, 0 );
      poly << mBalloonSegmentPoint2;
    }
    poly << currentSegment.p2();
  }
  if ( poly.at( 0 ) != poly.at( poly.count() - 1 ) )
    poly << poly.at( 0 );

  mFillSymbol->startRender( context );
  mFillSymbol->renderPolygon( poly, &rings, nullptr, context );
  mFillSymbol->stopRender( context );
}

void QgsAnnotation::drawMarkerSymbol( QgsRenderContext &context ) const
{
  if ( !context.painter() )
  {
    return;
  }

  if ( mMarkerSymbol )
  {
    mMarkerSymbol->startRender( context );
    mMarkerSymbol->renderPoint( QPointF( 0, 0 ), nullptr, context );
    mMarkerSymbol->stopRender( context );
  }
}

void QgsAnnotation::_writeXml( QDomElement &itemElem, QDomDocument &doc, const QgsReadWriteContext &context ) const
{
  if ( itemElem.isNull() )
  {
    return;
  }
  QDomElement annotationElem = doc.createElement( QStringLiteral( "AnnotationItem" ) );
  annotationElem.setAttribute( QStringLiteral( "mapPositionFixed" ), mHasFixedMapPosition );
  annotationElem.setAttribute( QStringLiteral( "mapPosX" ), qgsDoubleToString( mMapPosition.x() ) );
  annotationElem.setAttribute( QStringLiteral( "mapPosY" ), qgsDoubleToString( mMapPosition.y() ) );
  if ( mMapPositionCrs.isValid() )
    mMapPositionCrs.writeXml( annotationElem, doc );
  annotationElem.setAttribute( QStringLiteral( "offsetX" ), qgsDoubleToString( mOffsetFromReferencePoint.x() ) );
  annotationElem.setAttribute( QStringLiteral( "offsetY" ), qgsDoubleToString( mOffsetFromReferencePoint.y() ) );
  annotationElem.setAttribute( QStringLiteral( "frameWidth" ), qgsDoubleToString( mFrameSize.width() ) );
  annotationElem.setAttribute( QStringLiteral( "frameHeight" ), qgsDoubleToString( mFrameSize.height() ) );
  annotationElem.setAttribute( QStringLiteral( "canvasPosX" ), qgsDoubleToString( mRelativePosition.x() ) );
  annotationElem.setAttribute( QStringLiteral( "canvasPosY" ), qgsDoubleToString( mRelativePosition.y() ) );
  annotationElem.setAttribute( QStringLiteral( "contentsMargin" ), mContentsMargins.toString() );
  annotationElem.setAttribute( QStringLiteral( "visible" ), isVisible() );
  if ( mMapLayer )
  {
    annotationElem.setAttribute( QStringLiteral( "mapLayer" ), mMapLayer->id() );
  }
  if ( mMarkerSymbol )
  {
    QDomElement symbolElem = QgsSymbolLayerUtils::saveSymbol( QStringLiteral( "marker symbol" ), mMarkerSymbol.get(), doc, context );
    if ( !symbolElem.isNull() )
    {
      annotationElem.appendChild( symbolElem );
    }
  }
  if ( mFillSymbol )
  {
    QDomElement fillElem = doc.createElement( QStringLiteral( "fillSymbol" ) );
    QDomElement symbolElem = QgsSymbolLayerUtils::saveSymbol( QStringLiteral( "fill symbol" ), mFillSymbol.get(), doc, context );
    if ( !symbolElem.isNull() )
    {
      fillElem.appendChild( symbolElem );
      annotationElem.appendChild( fillElem );
    }
  }
  itemElem.appendChild( annotationElem );
}

void QgsAnnotation::_readXml( const QDomElement &annotationElem, const QgsReadWriteContext &context )
{
  if ( annotationElem.isNull() )
  {
    return;
  }
  QPointF pos;
  pos.setX( annotationElem.attribute( QStringLiteral( "canvasPosX" ), QStringLiteral( "0" ) ).toDouble() );
  pos.setY( annotationElem.attribute( QStringLiteral( "canvasPosY" ), QStringLiteral( "0" ) ).toDouble() );
  if ( pos.x() >= 1 || pos.x() < 0 || pos.y() < 0 || pos.y() >= 1 )
    mRelativePosition = QPointF();
  else
    mRelativePosition = pos;
  QgsPointXY mapPos;
  mapPos.setX( annotationElem.attribute( QStringLiteral( "mapPosX" ), QStringLiteral( "0" ) ).toDouble() );
  mapPos.setY( annotationElem.attribute( QStringLiteral( "mapPosY" ), QStringLiteral( "0" ) ).toDouble() );
  mMapPosition = mapPos;

  if ( !mMapPositionCrs.readXml( annotationElem ) )
  {
    mMapPositionCrs = QgsCoordinateReferenceSystem();
  }

  mContentsMargins = QgsMargins::fromString( annotationElem.attribute( QStringLiteral( "contentsMargin" ) ) );
  mFrameSize.setWidth( annotationElem.attribute( QStringLiteral( "frameWidth" ), QStringLiteral( "50" ) ).toDouble() );
  mFrameSize.setHeight( annotationElem.attribute( QStringLiteral( "frameHeight" ), QStringLiteral( "50" ) ).toDouble() );
  mOffsetFromReferencePoint.setX( annotationElem.attribute( QStringLiteral( "offsetX" ), QStringLiteral( "0" ) ).toDouble() );
  mOffsetFromReferencePoint.setY( annotationElem.attribute( QStringLiteral( "offsetY" ), QStringLiteral( "0" ) ).toDouble() );
  mHasFixedMapPosition = annotationElem.attribute( QStringLiteral( "mapPositionFixed" ), QStringLiteral( "1" ) ).toInt();
  mVisible = annotationElem.attribute( QStringLiteral( "visible" ), QStringLiteral( "1" ) ).toInt();
  if ( annotationElem.hasAttribute( QStringLiteral( "mapLayer" ) ) )
  {
    mMapLayer = QgsProject::instance()->mapLayer( annotationElem.attribute( QStringLiteral( "mapLayer" ) ) );
  }

  //marker symbol
  QDomElement symbolElem = annotationElem.firstChildElement( QStringLiteral( "symbol" ) );
  if ( !symbolElem.isNull() )
  {
    QgsMarkerSymbol *symbol = QgsSymbolLayerUtils::loadSymbol<QgsMarkerSymbol>( symbolElem, context );
    if ( symbol )
    {
      mMarkerSymbol.reset( symbol );
    }
  }

  mFillSymbol.reset( nullptr );
  QDomElement fillElem = annotationElem.firstChildElement( QStringLiteral( "fillSymbol" ) );
  if ( !fillElem.isNull() )
  {
    QDomElement symbolElem = fillElem.firstChildElement( QStringLiteral( "symbol" ) );
    if ( !symbolElem.isNull() )
    {
      QgsFillSymbol *symbol = QgsSymbolLayerUtils::loadSymbol<QgsFillSymbol>( symbolElem, context );
      if ( symbol )
      {
        mFillSymbol.reset( symbol );
      }
    }
  }
  if ( !mFillSymbol )
  {
    QColor frameColor;
    frameColor.setNamedColor( annotationElem.attribute( QStringLiteral( "frameColor" ), QStringLiteral( "#000000" ) ) );
    frameColor.setAlpha( annotationElem.attribute( QStringLiteral( "frameColorAlpha" ), QStringLiteral( "255" ) ).toInt() );
    QColor frameBackgroundColor;
    frameBackgroundColor.setNamedColor( annotationElem.attribute( QStringLiteral( "frameBackgroundColor" ) ) );
    frameBackgroundColor.setAlpha( annotationElem.attribute( QStringLiteral( "frameBackgroundColorAlpha" ), QStringLiteral( "255" ) ).toInt() );
    double frameBorderWidth = annotationElem.attribute( QStringLiteral( "frameBorderWidth" ), QStringLiteral( "0.5" ) ).toDouble();
    // need to roughly convert border width from pixels to mm - just assume 96 dpi
    frameBorderWidth = frameBorderWidth * 25.4 / 96.0;
    QgsStringMap props;
    props.insert( QStringLiteral( "color" ), frameBackgroundColor.name() );
    props.insert( QStringLiteral( "style" ), QStringLiteral( "solid" ) );
    props.insert( QStringLiteral( "style_border" ), QStringLiteral( "solid" ) );
    props.insert( QStringLiteral( "color_border" ), frameColor.name() );
    props.insert( QStringLiteral( "width_border" ), QString::number( frameBorderWidth ) );
    props.insert( QStringLiteral( "joinstyle" ), QStringLiteral( "miter" ) );
    mFillSymbol.reset( QgsFillSymbol::createSimple( props ) );
  }

  updateBalloon();
  emit mapLayerChanged();
}

void QgsAnnotation::copyCommonProperties( QgsAnnotation *target ) const
{
  target->mVisible = mVisible;
  target->mHasFixedMapPosition = mHasFixedMapPosition;
  target->mMapPosition = mMapPosition;
  target->mMapPositionCrs = mMapPositionCrs;
  target->mRelativePosition = mRelativePosition;
  target->mOffsetFromReferencePoint = mOffsetFromReferencePoint;
  target->mFrameSize = mFrameSize;
  target->mMarkerSymbol.reset( mMarkerSymbol ? mMarkerSymbol->clone() : nullptr );
  target->mContentsMargins = mContentsMargins;
  target->mFillSymbol.reset( mFillSymbol ? mFillSymbol->clone() : nullptr );
  target->mBalloonSegment = mBalloonSegment;
  target->mBalloonSegmentPoint1 = mBalloonSegmentPoint1;
  target->mBalloonSegmentPoint2 = mBalloonSegmentPoint2;
  target->mMapLayer = mMapLayer;
  target->mFeature = mFeature;
}

