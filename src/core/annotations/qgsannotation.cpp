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
#include "qgsgeometryutils.h"
#include "qgsstyleentityvisitor.h"

#include <QPen>
#include <QPainter>

Q_GUI_EXPORT extern int qt_defaultDpiX();

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
  // convert from offset in pixels at 96 dpi to mm
  setFrameOffsetFromReferencePointMm( offset / 3.7795275 );
}

QPointF QgsAnnotation::frameOffsetFromReferencePoint() const
{
  return mOffsetFromReferencePoint / 3.7795275;
}

void QgsAnnotation::setFrameOffsetFromReferencePointMm( QPointF offset )
{
  mOffsetFromReferencePoint = offset;

  updateBalloon();
  emit moved();
  emit appearanceChanged();
}

void QgsAnnotation::setFrameSize( QSizeF size )
{
  // convert from size in pixels at 96 dpi to mm
  setFrameSizeMm( size / 3.7795275 );
}

QSizeF QgsAnnotation::frameSize() const
{
  return mFrameSize / 3.7795275;
}

void QgsAnnotation::setFrameSizeMm( QSizeF size )
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
    painter->translate( context.convertToPainterUnits( mOffsetFromReferencePoint.x(), QgsUnitTypes::RenderMillimeters ) + context.convertToPainterUnits( mContentsMargins.left(), QgsUnitTypes::RenderMillimeters ),
                        context.convertToPainterUnits( mOffsetFromReferencePoint.y(), QgsUnitTypes::RenderMillimeters ) + context.convertToPainterUnits( mContentsMargins.top(), QgsUnitTypes::RenderMillimeters ) );
  }
  else
  {
    painter->translate( context.convertToPainterUnits( mContentsMargins.left(), QgsUnitTypes::RenderMillimeters ),
                        context.convertToPainterUnits( mContentsMargins.top(), QgsUnitTypes::RenderMillimeters ) );
  }
  QSizeF size( context.convertToPainterUnits( mFrameSize.width(), QgsUnitTypes::RenderMillimeters ) - context.convertToPainterUnits( mContentsMargins.left() + mContentsMargins.right(), QgsUnitTypes::RenderMillimeters ),
               context.convertToPainterUnits( mFrameSize.height(), QgsUnitTypes::RenderMillimeters ) - context.convertToPainterUnits( mContentsMargins.top() + mContentsMargins.bottom(), QgsUnitTypes::RenderMillimeters ) );

  // scale back from painter dpi to 96 dpi --
// double dotsPerMM = context.painter()->device()->logicalDpiX() / ( 25.4 * 3.78 );
// context.painter()->scale( dotsPerMM, dotsPerMM );

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

bool QgsAnnotation::accept( QgsStyleEntityVisitorInterface *visitor ) const
{
  // NOTE: if visitEnter returns false it means "don't visit the annotation", not "abort all further visitations"
  if ( !visitor->visitEnter( QgsStyleEntityVisitorInterface::Node( QgsStyleEntityVisitorInterface::NodeType::Annotation, QStringLiteral( "annotation" ), tr( "Annotation" ) ) ) )
    return true;

  if ( mMarkerSymbol )
  {
    QgsStyleSymbolEntity entity( mMarkerSymbol.get() );
    if ( !visitor->visit( QgsStyleEntityVisitorInterface::StyleLeaf( &entity, QStringLiteral( "marker" ), QObject::tr( "Marker" ) ) ) )
      return false;
  }

  if ( mFillSymbol )
  {
    QgsStyleSymbolEntity entity( mFillSymbol.get() );
    if ( !visitor->visit( QgsStyleEntityVisitorInterface::StyleLeaf( &entity, QStringLiteral( "fill" ), QObject::tr( "Fill" ) ) ) )
      return false;
  }

  if ( !visitor->visitExit( QgsStyleEntityVisitorInterface::Node( QgsStyleEntityVisitorInterface::NodeType::Annotation, QStringLiteral( "annotation" ), tr( "Annotation" ) ) ) )
    return false;

  return true;
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
  segmentList << segment( 0, nullptr );
  segmentList << segment( 1, nullptr );
  segmentList << segment( 2, nullptr );
  segmentList << segment( 3, nullptr );

  //find  closest edge / closest edge point
  double minEdgeDist = std::numeric_limits<double>::max();
  int minEdgeIndex = -1;
  QLineF minEdge;
  QgsPointXY minEdgePoint;
  QgsPointXY origin( 0, 0 );

  for ( int i = 0; i < 4; ++i )
  {
    QLineF currentSegment = segmentList.at( i );
    QgsPointXY currentMinDistPoint;
    double currentMinDist = origin.sqrDistToSegment( currentSegment.x1(), currentSegment.y1(), currentSegment.x2(), currentSegment.y2(), currentMinDistPoint );
    bool isPreferredSegment = false;
    if ( qgsDoubleNear( currentMinDist, minEdgeDist ) )
    {
      // two segments are close - work out which looks nicer
      const double angle = fmod( origin.azimuth( currentMinDistPoint ) + 360.0, 360.0 );
      if ( angle < 45 || angle > 315 )
        isPreferredSegment = i == 0;
      else if ( angle < 135 )
        isPreferredSegment = i == 3;
      else if ( angle < 225 )
        isPreferredSegment = i == 2;
      else
        isPreferredSegment = i == 1;
    }
    else if ( currentMinDist < minEdgeDist )
      isPreferredSegment = true;

    if ( isPreferredSegment )
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

  mBalloonSegment = minEdgeIndex;
  QPointF minEdgeEnd = minEdge.p2();
  mBalloonSegmentPoint1 = QPointF( minEdgePoint.x(), minEdgePoint.y() );
  if ( std::sqrt( minEdgePoint.sqrDist( minEdgeEnd.x(), minEdgeEnd.y() ) ) < mSegmentPointWidthMm )
  {
    double x = 0;
    double y = 0;
    QgsGeometryUtils::pointOnLineWithDistance( minEdge.p2().x(), minEdge.p2().y(), minEdge.p1().x(), minEdge.p1().y(), mSegmentPointWidthMm, x, y );
    mBalloonSegmentPoint1 = QPointF( x, y );
  }

  {
    double x = 0;
    double y = 0;
    QgsGeometryUtils::pointOnLineWithDistance( mBalloonSegmentPoint1.x(), mBalloonSegmentPoint1.y(), minEdge.p2().x(), minEdge.p2().y(), mSegmentPointWidthMm, x, y );
    mBalloonSegmentPoint2 = QPointF( x, y );
  }

}

QLineF QgsAnnotation::segment( int index, QgsRenderContext *context ) const
{
  auto scaleSize = [context]( double size )->double
  {
    return context ? context->convertToPainterUnits( size, QgsUnitTypes::RenderMillimeters ) : size;
  };
  if ( mHasFixedMapPosition )
  {
    switch ( index )
    {
      case 0:
        return QLineF( scaleSize( mOffsetFromReferencePoint.x() ),
                       scaleSize( mOffsetFromReferencePoint.y() ),
                       scaleSize( mOffsetFromReferencePoint.x() ) + scaleSize( mFrameSize.width() ),
                       scaleSize( mOffsetFromReferencePoint.y() ) );
      case 1:
        return QLineF( scaleSize( mOffsetFromReferencePoint.x() ) + scaleSize( mFrameSize.width() ),
                       scaleSize( mOffsetFromReferencePoint.y() ),
                       scaleSize( mOffsetFromReferencePoint.x() ) + scaleSize( mFrameSize.width() ),
                       scaleSize( mOffsetFromReferencePoint.y() ) + scaleSize( mFrameSize.height() ) );
      case 2:
        return QLineF( scaleSize( mOffsetFromReferencePoint.x() ) + scaleSize( mFrameSize.width() ),
                       scaleSize( mOffsetFromReferencePoint.y() ) + scaleSize( mFrameSize.height() ),
                       scaleSize( mOffsetFromReferencePoint.x() ),
                       scaleSize( mOffsetFromReferencePoint.y() ) + scaleSize( mFrameSize.height() ) );
      case 3:
        return QLineF( scaleSize( mOffsetFromReferencePoint.x() ),
                       scaleSize( mOffsetFromReferencePoint.y() ) + scaleSize( mFrameSize.height() ),
                       scaleSize( mOffsetFromReferencePoint.x() ),
                       scaleSize( mOffsetFromReferencePoint.y() ) );
      default:
        return QLineF();
    }
  }
  else
  {
    switch ( index )
    {
      case 0:
        return QLineF( 0, 0, scaleSize( mFrameSize.width() ), 0 );
      case 1:
        return QLineF( scaleSize( mFrameSize.width() ), 0,
                       scaleSize( mFrameSize.width() ), scaleSize( mFrameSize.height() ) );
      case 2:
        return QLineF( scaleSize( mFrameSize.width() ), scaleSize( mFrameSize.height() ),
                       0, scaleSize( mFrameSize.height() ) );
      case 3:
        return QLineF( 0, scaleSize( mFrameSize.height() ),
                       0, 0 );
      default:
        return QLineF();
    }
  }
}

void QgsAnnotation::drawFrame( QgsRenderContext &context ) const
{
  if ( !mFillSymbol )
    return;

  context.painter()->setRenderHint( QPainter::Antialiasing, context.flags() & QgsRenderContext::Antialiasing );

  QPolygonF poly;
  poly.reserve( 9 + ( mHasFixedMapPosition ? 3 : 0 ) );
  QList<QPolygonF> rings; //empty list
  for ( int i = 0; i < 4; ++i )
  {
    QLineF currentSegment = segment( i, &context );
    poly << QPointF( currentSegment.p1().x(),
                     currentSegment.p1().y() );
    if ( i == mBalloonSegment && mHasFixedMapPosition )
    {
      poly << QPointF( context.convertToPainterUnits( mBalloonSegmentPoint1.x(), QgsUnitTypes::RenderMillimeters ),
                       context.convertToPainterUnits( mBalloonSegmentPoint1.y(), QgsUnitTypes::RenderMillimeters ) );
      poly << QPointF( 0, 0 );
      poly << QPointF( context.convertToPainterUnits( mBalloonSegmentPoint2.x(), QgsUnitTypes::RenderMillimeters ),
                       context.convertToPainterUnits( mBalloonSegmentPoint2.y(), QgsUnitTypes::RenderMillimeters ) );
    }
    poly << QPointF( currentSegment.p2().x(), currentSegment.p2().y() );
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
  annotationElem.setAttribute( QStringLiteral( "offsetXMM" ), qgsDoubleToString( mOffsetFromReferencePoint.x() ) );
  annotationElem.setAttribute( QStringLiteral( "offsetYMM" ), qgsDoubleToString( mOffsetFromReferencePoint.y() ) );
  annotationElem.setAttribute( QStringLiteral( "frameWidthMM" ), qgsDoubleToString( mFrameSize.width() ) );
  annotationElem.setAttribute( QStringLiteral( "frameHeightMM" ), qgsDoubleToString( mFrameSize.height() ) );
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
  const double dpiScale = 25.4 / qt_defaultDpiX();
  if ( annotationElem.hasAttribute( QStringLiteral( "frameWidthMM" ) ) )
    mFrameSize.setWidth( annotationElem.attribute( QStringLiteral( "frameWidthMM" ), QStringLiteral( "5" ) ).toDouble() );
  else
    mFrameSize.setWidth( dpiScale * annotationElem.attribute( QStringLiteral( "frameWidth" ), QStringLiteral( "50" ) ).toDouble() );
  if ( annotationElem.hasAttribute( QStringLiteral( "frameHeightMM" ) ) )
    mFrameSize.setHeight( annotationElem.attribute( QStringLiteral( "frameHeightMM" ), QStringLiteral( "3" ) ).toDouble() );
  else
    mFrameSize.setHeight( dpiScale * annotationElem.attribute( QStringLiteral( "frameHeight" ), QStringLiteral( "50" ) ).toDouble() );

  if ( annotationElem.hasAttribute( QStringLiteral( "offsetXMM" ) ) )
    mOffsetFromReferencePoint.setX( annotationElem.attribute( QStringLiteral( "offsetXMM" ), QStringLiteral( "0" ) ).toDouble() );
  else
    mOffsetFromReferencePoint.setX( dpiScale * annotationElem.attribute( QStringLiteral( "offsetX" ), QStringLiteral( "0" ) ).toDouble() );
  if ( annotationElem.hasAttribute( QStringLiteral( "offsetYMM" ) ) )
    mOffsetFromReferencePoint.setY( annotationElem.attribute( QStringLiteral( "offsetYMM" ), QStringLiteral( "0" ) ).toDouble() );
  else
    mOffsetFromReferencePoint.setY( dpiScale * annotationElem.attribute( QStringLiteral( "offsetY" ), QStringLiteral( "0" ) ).toDouble() );

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
  target->mSegmentPointWidthMm = mSegmentPointWidthMm;
  target->mMapLayer = mMapLayer;
  target->mFeature = mFeature;
}

