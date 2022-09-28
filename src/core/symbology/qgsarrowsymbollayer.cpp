/***************************************************************************
 qgsarrowsymbollayer.cpp
 ---------------------
 begin                : January 2016
 copyright            : (C) 2016 by Hugo Mercier
 email                : hugo dot mercier at oslandia dot com
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "qgsarrowsymbollayer.h"
#include "qgssymbollayerutils.h"
#include "qgsfillsymbol.h"
#include "qgsrendercontext.h"

QgsArrowSymbolLayer::QgsArrowSymbolLayer()
{
  /* default values */
  setOffset( 0.0 );
  setOffsetUnit( QgsUnitTypes::RenderMillimeters );

  mSymbol.reset( static_cast<QgsFillSymbol *>( QgsFillSymbol::createSimple( QVariantMap() ) ) );
}

QgsArrowSymbolLayer::~QgsArrowSymbolLayer() = default;

bool QgsArrowSymbolLayer::setSubSymbol( QgsSymbol *symbol )
{
  if ( symbol && symbol->type() == Qgis::SymbolType::Fill )
  {
    mSymbol.reset( static_cast<QgsFillSymbol *>( symbol ) );
    return true;
  }
  delete symbol;
  return false;
}

QgsSymbolLayer *QgsArrowSymbolLayer::create( const QVariantMap &props )
{
  QgsArrowSymbolLayer *l = new QgsArrowSymbolLayer();

  if ( props.contains( QStringLiteral( "arrow_width" ) ) )
    l->setArrowWidth( props[QStringLiteral( "arrow_width" )].toDouble() );

  if ( props.contains( QStringLiteral( "arrow_width_unit" ) ) )
    l->setArrowWidthUnit( QgsUnitTypes::decodeRenderUnit( props[QStringLiteral( "arrow_width_unit" )].toString() ) );

  if ( props.contains( QStringLiteral( "arrow_width_unit_scale" ) ) )
    l->setArrowWidthUnitScale( QgsSymbolLayerUtils::decodeMapUnitScale( props[QStringLiteral( "arrow_width_unit_scale" )].toString() ) );

  if ( props.contains( QStringLiteral( "arrow_start_width" ) ) )
    l->setArrowStartWidth( props[QStringLiteral( "arrow_start_width" )].toDouble() );

  if ( props.contains( QStringLiteral( "arrow_start_width_unit" ) ) )
    l->setArrowStartWidthUnit( QgsUnitTypes::decodeRenderUnit( props[QStringLiteral( "arrow_start_width_unit" )].toString() ) );

  if ( props.contains( QStringLiteral( "arrow_start_width_unit_scale" ) ) )
    l->setArrowStartWidthUnitScale( QgsSymbolLayerUtils::decodeMapUnitScale( props[QStringLiteral( "arrow_start_width_unit_scale" )].toString() ) );

  if ( props.contains( QStringLiteral( "is_curved" ) ) )
    l->setIsCurved( props[QStringLiteral( "is_curved" )].toInt() == 1 );

  if ( props.contains( QStringLiteral( "is_repeated" ) ) )
    l->setIsRepeated( props[QStringLiteral( "is_repeated" )].toInt() == 1 );

  if ( props.contains( QStringLiteral( "head_length" ) ) )
    l->setHeadLength( props[QStringLiteral( "head_length" )].toDouble() );

  if ( props.contains( QStringLiteral( "head_length_unit" ) ) )
    l->setHeadLengthUnit( QgsUnitTypes::decodeRenderUnit( props[QStringLiteral( "head_length_unit" )].toString() ) );

  if ( props.contains( QStringLiteral( "head_length_unit_scale" ) ) )
    l->setHeadLengthUnitScale( QgsSymbolLayerUtils::decodeMapUnitScale( props[QStringLiteral( "head_length_unit_scale" )].toString() ) );

  if ( props.contains( QStringLiteral( "head_thickness" ) ) )
    l->setHeadThickness( props[QStringLiteral( "head_thickness" )].toDouble() );

  if ( props.contains( QStringLiteral( "head_thickness_unit" ) ) )
    l->setHeadThicknessUnit( QgsUnitTypes::decodeRenderUnit( props[QStringLiteral( "head_thickness_unit" )].toString() ) );

  if ( props.contains( QStringLiteral( "head_thickness_unit_scale" ) ) )
    l->setHeadThicknessUnitScale( QgsSymbolLayerUtils::decodeMapUnitScale( props[QStringLiteral( "head_thickness_unit_scale" )].toString() ) );

  if ( props.contains( QStringLiteral( "head_type" ) ) )
    l->setHeadType( static_cast<HeadType>( props[QStringLiteral( "head_type" )].toInt() ) );

  if ( props.contains( QStringLiteral( "arrow_type" ) ) )
    l->setArrowType( static_cast<ArrowType>( props[QStringLiteral( "arrow_type" )].toInt() ) );

  if ( props.contains( QStringLiteral( "offset" ) ) )
    l->setOffset( props[QStringLiteral( "offset" )].toDouble() );

  if ( props.contains( QStringLiteral( "offset_unit" ) ) )
    l->setOffsetUnit( QgsUnitTypes::decodeRenderUnit( props[QStringLiteral( "offset_unit" )].toString() ) );

  if ( props.contains( QStringLiteral( "offset_unit_scale" ) ) )
    l->setOffsetMapUnitScale( QgsSymbolLayerUtils::decodeMapUnitScale( props[QStringLiteral( "offset_unit_scale" )].toString() ) );

  if ( props.contains( QStringLiteral( "ring_filter" ) ) )
    l->setRingFilter( static_cast< RenderRingFilter>( props[QStringLiteral( "ring_filter" )].toInt() ) );

  l->restoreOldDataDefinedProperties( props );

  l->setSubSymbol( QgsFillSymbol::createSimple( props ) );

  return l;
}

QgsArrowSymbolLayer *QgsArrowSymbolLayer::clone() const
{
  QgsArrowSymbolLayer *l = static_cast<QgsArrowSymbolLayer *>( create( properties() ) );
  l->setSubSymbol( mSymbol->clone() );
  copyDataDefinedProperties( l );
  copyPaintEffect( l );
  return l;
}

QgsSymbol *QgsArrowSymbolLayer::subSymbol()
{
  return mSymbol.get();
}

QString QgsArrowSymbolLayer::layerType() const
{
  return QStringLiteral( "ArrowLine" );
}

QVariantMap QgsArrowSymbolLayer::properties() const
{
  QVariantMap map;

  map[QStringLiteral( "arrow_width" )] = QString::number( arrowWidth() );
  map[QStringLiteral( "arrow_width_unit" )] = QgsUnitTypes::encodeUnit( arrowWidthUnit() );
  map[QStringLiteral( "arrow_width_unit_scale" )] = QgsSymbolLayerUtils::encodeMapUnitScale( arrowWidthUnitScale() );

  map[QStringLiteral( "arrow_start_width" )] = QString::number( arrowStartWidth() );
  map[QStringLiteral( "arrow_start_width_unit" )] = QgsUnitTypes::encodeUnit( arrowStartWidthUnit() );
  map[QStringLiteral( "arrow_start_width_unit_scale" )] = QgsSymbolLayerUtils::encodeMapUnitScale( arrowStartWidthUnitScale() );

  map[QStringLiteral( "is_curved" )] = QString::number( isCurved() ? 1 : 0 );
  map[QStringLiteral( "is_repeated" )] = QString::number( isRepeated() ? 1 : 0 );

  map[QStringLiteral( "head_length" )] = QString::number( headLength() );
  map[QStringLiteral( "head_length_unit" )] = QgsUnitTypes::encodeUnit( headLengthUnit() );
  map[QStringLiteral( "head_length_unit_scale" )] = QgsSymbolLayerUtils::encodeMapUnitScale( headLengthUnitScale() );

  map[QStringLiteral( "head_thickness" )] = QString::number( headThickness() );
  map[QStringLiteral( "head_thickness_unit" )] = QgsUnitTypes::encodeUnit( headThicknessUnit() );
  map[QStringLiteral( "head_thickness_unit_scale" )] = QgsSymbolLayerUtils::encodeMapUnitScale( headThicknessUnitScale() );

  map[QStringLiteral( "head_type" )] = QString::number( headType() );
  map[QStringLiteral( "arrow_type" )] = QString::number( arrowType() );

  map[QStringLiteral( "offset" )] = QString::number( offset() );
  map[QStringLiteral( "offset_unit" )] = QgsUnitTypes::encodeUnit( offsetUnit() );
  map[QStringLiteral( "offset_unit_scale" )] = QgsSymbolLayerUtils::encodeMapUnitScale( offsetMapUnitScale() );

  map[QStringLiteral( "ring_filter" )] = QString::number( static_cast< int >( mRingFilter ) );

  return map;
}

QSet<QString> QgsArrowSymbolLayer::usedAttributes( const QgsRenderContext &context ) const
{
  QSet<QString> attributes = QgsLineSymbolLayer::usedAttributes( context );

  attributes.unite( mSymbol->usedAttributes( context ) );

  return attributes;
}

bool QgsArrowSymbolLayer::hasDataDefinedProperties() const
{
  if ( QgsSymbolLayer::hasDataDefinedProperties() )
    return true;
  if ( mSymbol && mSymbol->hasDataDefinedProperties() )
    return true;
  return false;
}

bool QgsArrowSymbolLayer::usesMapUnits() const
{
  return mArrowWidthUnit == QgsUnitTypes::RenderMapUnits || mArrowWidthUnit == QgsUnitTypes::RenderMetersInMapUnits
         || mArrowStartWidthUnit == QgsUnitTypes::RenderMapUnits || mArrowStartWidthUnit == QgsUnitTypes::RenderMetersInMapUnits
         || mHeadLengthUnit == QgsUnitTypes::RenderMapUnits || mHeadLengthUnit == QgsUnitTypes::RenderMetersInMapUnits
         || mHeadThicknessUnit == QgsUnitTypes::RenderMapUnits || mHeadThicknessUnit == QgsUnitTypes::RenderMetersInMapUnits
         || mWidthUnit == QgsUnitTypes::RenderMapUnits || mWidthUnit == QgsUnitTypes::RenderMetersInMapUnits
         || mOffsetUnit == QgsUnitTypes::RenderMapUnits || mOffsetUnit == QgsUnitTypes::RenderMetersInMapUnits;
}

void QgsArrowSymbolLayer::setOutputUnit( QgsUnitTypes::RenderUnit unit )
{
  QgsLineSymbolLayer::setOutputUnit( unit );
  mArrowWidthUnit = unit;
  mArrowStartWidthUnit = unit;
  mHeadLengthUnit = unit;
  mHeadThicknessUnit = unit;
}

void QgsArrowSymbolLayer::startRender( QgsSymbolRenderContext &context )
{
  mExpressionScope.reset( new QgsExpressionContextScope() );
  mScaledArrowWidth = context.renderContext().convertToPainterUnits( arrowWidth(), arrowWidthUnit(), arrowWidthUnitScale() );
  mScaledArrowStartWidth = context.renderContext().convertToPainterUnits( arrowStartWidth(), arrowStartWidthUnit(), arrowStartWidthUnitScale() );
  mScaledHeadLength = context.renderContext().convertToPainterUnits( headLength(), headLengthUnit(), headLengthUnitScale() );
  mScaledHeadThickness = context.renderContext().convertToPainterUnits( headThickness(), headThicknessUnit(), headThicknessUnitScale() );
  mScaledOffset = context.renderContext().convertToPainterUnits( offset(), offsetUnit(), offsetMapUnitScale() );
  mComputedHeadType = headType();
  mComputedArrowType = arrowType();

  mSymbol->startRender( context.renderContext() );
}

void QgsArrowSymbolLayer::stopRender( QgsSymbolRenderContext &context )
{
  mSymbol->stopRender( context.renderContext() );
}

inline qreal euclidean_distance( QPointF po, QPointF pd )
{
  return std::sqrt( ( po.x() - pd.x() ) * ( po.x() - pd.x() ) + ( po.y() - pd.y() ) * ( po.y() - pd.y() ) );
}

QPolygonF straightArrow( QPointF po, QPointF pd,
                         qreal startWidth, qreal width,
                         qreal headWidth, qreal headHeight,
                         QgsArrowSymbolLayer::HeadType headType, QgsArrowSymbolLayer::ArrowType arrowType,
                         qreal offset )
{
  QPolygonF polygon; // implicitly shared
  // vector length
  qreal length = euclidean_distance( po, pd );

  // shift points if there is not enough room for the head(s)
  if ( ( headType == QgsArrowSymbolLayer::HeadSingle ) && ( length < headWidth ) )
  {
    po = pd - ( pd - po ) / length * headWidth;
    length = headWidth;
  }
  else if ( ( headType == QgsArrowSymbolLayer::HeadReversed ) && ( length < headWidth ) )
  {
    pd = po + ( pd - po ) / length * headWidth;
    length = headWidth;
  }
  else if ( ( headType == QgsArrowSymbolLayer::HeadDouble ) && ( length < 2 * headWidth ) )
  {
    const QPointF v = ( pd - po ) / length * headWidth;
    const QPointF npo = ( po + pd ) / 2.0 - v;
    const QPointF npd = ( po + pd ) / 2.0 + v;
    po = npo;
    pd = npd;
    length = 2 * headWidth;
  }

  const qreal bodyLength = length - headWidth;

  // unit vector
  const QPointF unitVec = ( pd - po ) / length;
  // perpendicular vector
  const QPointF perpVec( -unitVec.y(), unitVec.x() );

  // set offset
  po += perpVec * offset;
  pd += perpVec * offset;

  if ( headType == QgsArrowSymbolLayer::HeadDouble )
  {
    // first head
    polygon << po;
    if ( arrowType == QgsArrowSymbolLayer::ArrowPlain || arrowType == QgsArrowSymbolLayer::ArrowRightHalf )
    {
      polygon << po + unitVec *headWidth + perpVec *headHeight;
      polygon << po + unitVec *headWidth + perpVec * ( width * 0.5 );

      polygon << po + unitVec *bodyLength + perpVec * ( width * 0.5 );

      // second head
      polygon << po + unitVec *bodyLength + perpVec *headHeight;
    }
    polygon << pd;

    if ( arrowType == QgsArrowSymbolLayer::ArrowPlain || arrowType == QgsArrowSymbolLayer::ArrowLeftHalf )
    {
      polygon << po + unitVec *bodyLength - perpVec *headHeight;
      polygon << po + unitVec *bodyLength - perpVec * ( width * 0.5 );

      // end of the first head
      polygon << po + unitVec *headWidth - perpVec * ( width * 0.5 );
      polygon << po + unitVec *headWidth - perpVec *headHeight;
    }
  }
  else if ( headType == QgsArrowSymbolLayer::HeadSingle )
  {
    if ( arrowType == QgsArrowSymbolLayer::ArrowPlain || arrowType == QgsArrowSymbolLayer::ArrowRightHalf )
    {
      polygon << po + perpVec * ( startWidth * 0.5 );
      polygon << po + unitVec *bodyLength + perpVec * ( width * 0.5 );
      polygon << po + unitVec *bodyLength + perpVec *headHeight;
    }
    else
    {
      polygon << po;
    }
    polygon << pd;
    if ( arrowType == QgsArrowSymbolLayer::ArrowPlain || arrowType == QgsArrowSymbolLayer::ArrowLeftHalf )
    {
      polygon << po + unitVec *bodyLength - perpVec *headHeight;
      polygon << po + unitVec *bodyLength - perpVec * ( width * 0.5 );
      polygon << po - perpVec * ( startWidth * 0.5 );
    }
    else
    {
      polygon << po;
    }
  }
  else if ( headType == QgsArrowSymbolLayer::HeadReversed )
  {
    polygon << po;
    if ( arrowType == QgsArrowSymbolLayer::ArrowPlain || arrowType == QgsArrowSymbolLayer::ArrowRightHalf )
    {
      polygon << po + unitVec *headWidth + perpVec *headHeight;
      polygon << po + unitVec *headWidth + perpVec * ( width * 0.5 );

      polygon << pd + perpVec * ( startWidth * 0.5 );
    }
    else
    {
      polygon << pd;
    }
    if ( arrowType == QgsArrowSymbolLayer::ArrowPlain || arrowType == QgsArrowSymbolLayer::ArrowLeftHalf )
    {
      polygon << pd - perpVec * ( startWidth * 0.5 );

      polygon << po + unitVec *headWidth - perpVec * ( width * 0.5 );
      polygon << po + unitVec *headWidth - perpVec *headHeight;
    }
    else
    {
      polygon << pd;
    }
  }
  // close the polygon
  polygon << polygon.first();

  return polygon;
}

// Make sure a given angle is between 0 and 2 pi
inline qreal clampAngle( qreal a )
{
  if ( a > 2 * M_PI )
    return a - 2 * M_PI;
  if ( a < 0.0 )
    return a + 2 * M_PI;
  return a;
}

/**
 * Compute the circumscribed circle from three points
 * \return FALSE if the three points are colinear
 */
bool pointsToCircle( QPointF a, QPointF b, QPointF c, QPointF &center, qreal &radius )
{
  qreal cx, cy;

  // AB and BC vectors
  const QPointF ab = b - a;
  const QPointF bc = c - b;

  // AB and BC middles
  const QPointF ab2 = ( a + b ) / 2.0;
  const QPointF bc2 = ( b + c ) / 2.0;

  // Aligned points
  if ( std::fabs( ab.x() * bc.y() - ab.y() * bc.x() ) < 0.001 ) // Empirical threshold for nearly aligned points
    return false;

  // in case AB is horizontal
  if ( ab.y() == 0 )
  {
    cx = ab2.x();
    cy = bc2.y() - ( cx - bc2.x() ) * bc.x() / bc.y();
  }
  //# BC horizontal
  else if ( bc.y() == 0 )
  {
    cx = bc2.x();
    cy = ab2.y() - ( cx - ab2.x() ) * ab.x() / ab.y();
  }
  // Otherwise
  else
  {
    cx = ( bc2.y() - ab2.y() + bc.x() * bc2.x() / bc.y() - ab.x() * ab2.x() / ab.y() ) / ( bc.x() / bc.y() - ab.x() / ab.y() );
    cy = bc2.y() - ( cx - bc2.x() ) * bc.x() / bc.y();
  }
  // Radius
  radius = std::sqrt( ( a.x() - cx ) * ( a.x() - cx ) + ( a.y() - cy ) * ( a.y() - cy ) );
  // Center
  center.setX( cx );
  center.setY( cy );
  return true;
}

QPointF circlePoint( QPointF center, qreal radius, qreal angle )
{
  // Y is oriented downward
  return QPointF( std::cos( -angle ) * radius + center.x(), std::sin( -angle ) * radius + center.y() );
}

void pathArcTo( QPainterPath &path, QPointF circleCenter, qreal circleRadius, qreal angle_o, qreal angle_d, int direction )
{
  const QRectF circleRect( circleCenter - QPointF( circleRadius, circleRadius ), circleCenter + QPointF( circleRadius, circleRadius ) );
  if ( direction == 1 )
  {
    if ( angle_o < angle_d )
      path.arcTo( circleRect, angle_o / M_PI * 180.0, ( angle_d - angle_o ) / M_PI * 180.0 );
    else
      path.arcTo( circleRect, angle_o / M_PI * 180.0, 360.0 - ( angle_o - angle_d ) / M_PI * 180.0 );
  }
  else
  {
    if ( angle_o < angle_d )
      path.arcTo( circleRect, angle_o / M_PI * 180.0, - ( 360.0 - ( angle_d - angle_o ) / M_PI * 180.0 ) );
    else
      path.arcTo( circleRect, angle_o / M_PI * 180.0, ( angle_d - angle_o ) / M_PI * 180.0 );
  }
}

// Draw a "spiral" arc defined by circle arcs around a center, a start and an end radius
void spiralArcTo( QPainterPath &path, QPointF center, qreal startAngle, qreal startRadius, qreal endAngle, qreal endRadius, int direction )
{
  // start point
  const QPointF A = circlePoint( center, startRadius, startAngle );
  // end point
  const QPointF B = circlePoint( center, endRadius, endAngle );
  // middle points
  qreal deltaAngle;

  deltaAngle = endAngle - startAngle;
  if ( direction * deltaAngle < 0.0 )
    deltaAngle = deltaAngle + direction * 2 * M_PI;

  const QPointF I1 = circlePoint( center, 0.75 * startRadius + 0.25 * endRadius, startAngle + 0.25 * deltaAngle );
  const QPointF I2 = circlePoint( center, 0.50 * startRadius + 0.50 * endRadius, startAngle + 0.50 * deltaAngle );
  const QPointF I3 = circlePoint( center, 0.25 * startRadius + 0.75 * endRadius, startAngle + 0.75 * deltaAngle );

  qreal cRadius;
  QPointF cCenter;
  // first circle arc
  if ( ! pointsToCircle( A, I1, I2, cCenter, cRadius ) )
  {
    // aligned points => draw a straight line
    path.lineTo( I2 );
  }
  else
  {
    // angles in the new circle
    const qreal a1 = std::atan2( cCenter.y() - A.y(), A.x() - cCenter.x() );
    const qreal a2 = std::atan2( cCenter.y() - I2.y(), I2.x() - cCenter.x() );
    pathArcTo( path, cCenter, cRadius, a1, a2, direction );
  }

  // second circle arc
  if ( ! pointsToCircle( I2, I3, B, cCenter, cRadius ) )
  {
    // aligned points => draw a straight line
    path.lineTo( B );
  }
  else
  {
    // angles in the new circle
    const qreal a1 = std::atan2( cCenter.y() - I2.y(), I2.x() - cCenter.x() );
    const qreal a2 = std::atan2( cCenter.y() - B.y(), B.x() - cCenter.x() );
    pathArcTo( path, cCenter, cRadius, a1, a2, direction );
  }
}

QPolygonF curvedArrow( QPointF po, QPointF pm, QPointF pd,
                       qreal startWidth, qreal width,
                       qreal headWidth, qreal headHeight,
                       QgsArrowSymbolLayer::HeadType headType, QgsArrowSymbolLayer::ArrowType arrowType,
                       qreal offset )
{
  qreal circleRadius;
  QPointF circleCenter;
  if ( ! pointsToCircle( po, pm, pd, circleCenter, circleRadius ) )
  {
    // aligned points => draw a straight arrow
    return straightArrow( po, pd, startWidth, width, headWidth, headHeight, headType, arrowType, offset );
  }

  // angles of each point
  const qreal angle_o = clampAngle( std::atan2( circleCenter.y() - po.y(), po.x() - circleCenter.x() ) );
  const qreal angle_m = clampAngle( std::atan2( circleCenter.y() - pm.y(), pm.x() - circleCenter.x() ) );
  const qreal angle_d = clampAngle( std::atan2( circleCenter.y() - pd.y(), pd.x() - circleCenter.x() ) );

  // arc direction : 1 = counter-clockwise, -1 = clockwise
  const int direction = clampAngle( angle_m - angle_o ) < clampAngle( angle_m - angle_d ) ? 1 : -1;

  // arrow type, independent of the direction
  int aType = 0;
  if ( arrowType == QgsArrowSymbolLayer::ArrowRightHalf )
    aType = direction;
  else if ( arrowType == QgsArrowSymbolLayer::ArrowLeftHalf )
    aType = -direction;

  qreal deltaAngle = angle_d - angle_o;
  if ( direction * deltaAngle < 0.0 )
    deltaAngle = deltaAngle + direction * 2 * M_PI;

  const qreal length = euclidean_distance( po, pd );
  // for close points and deltaAngle < 180, draw a straight line
  if ( std::fabs( deltaAngle ) < M_PI && ( ( ( headType == QgsArrowSymbolLayer::HeadSingle ) && ( length < headWidth ) ) ||
       ( ( headType == QgsArrowSymbolLayer::HeadReversed ) && ( length < headWidth ) ) ||
       ( ( headType == QgsArrowSymbolLayer::HeadDouble ) && ( length < 2 * headWidth ) ) ) )
  {
    return straightArrow( po, pd, startWidth, width, headWidth, headHeight, headType, arrowType, offset );
  }

  // adjust coordinates to include offset
  circleRadius += offset;
  po = circlePoint( circleCenter, circleRadius, angle_o );
  pm = circlePoint( circleCenter, circleRadius, angle_m );
  pd = circlePoint( circleCenter, circleRadius, angle_d );

  const qreal headAngle = direction * std::atan( headWidth / circleRadius );

  QPainterPath path;

  if ( headType == QgsArrowSymbolLayer::HeadDouble )
  {
    // the first head
    path.moveTo( po );
    if ( aType <= 0 )
    {
      path.lineTo( circlePoint( circleCenter, circleRadius + direction * headHeight, angle_o + headAngle ) );

      pathArcTo( path, circleCenter, circleRadius + direction * width / 2, angle_o + headAngle, angle_d - headAngle, direction );

      // the second head
      path.lineTo( circlePoint( circleCenter, circleRadius + direction * headHeight, angle_d - headAngle ) );
      path.lineTo( pd );
    }
    else
    {
      pathArcTo( path, circleCenter, circleRadius, angle_o, angle_d, direction );
    }
    if ( aType >= 0 )
    {
      path.lineTo( circlePoint( circleCenter, circleRadius - direction * headHeight, angle_d - headAngle ) );

      pathArcTo( path, circleCenter, circleRadius - direction * width / 2, angle_d - headAngle, angle_o + headAngle, -direction );

      // the end of the first head
      path.lineTo( circlePoint( circleCenter, circleRadius - direction * headHeight, angle_o + headAngle ) );
      path.lineTo( po );
    }
    else
    {
      pathArcTo( path, circleCenter, circleRadius, angle_d, angle_o, -direction );
    }
  }
  else if ( headType == QgsArrowSymbolLayer::HeadSingle )
  {
    if ( aType <= 0 )
    {
      path.moveTo( circlePoint( circleCenter, circleRadius + direction * startWidth / 2, angle_o ) );

      spiralArcTo( path, circleCenter, angle_o, circleRadius + direction * startWidth / 2, angle_d - headAngle, circleRadius + direction * width / 2, direction );

      // the arrow head
      path.lineTo( circlePoint( circleCenter, circleRadius + direction * headHeight, angle_d - headAngle ) );
      path.lineTo( pd );
    }
    else
    {
      path.moveTo( po );
      pathArcTo( path, circleCenter, circleRadius, angle_o, angle_d, direction );
    }
    if ( aType >= 0 )
    {
      path.lineTo( circlePoint( circleCenter, circleRadius - direction * headHeight, angle_d - headAngle ) );

      spiralArcTo( path, circleCenter, angle_d - headAngle, circleRadius - direction * width / 2, angle_o, circleRadius - direction * startWidth / 2, -direction );

      path.lineTo( circlePoint( circleCenter, circleRadius + direction * startWidth / 2, angle_o ) );
    }
    else
    {
      pathArcTo( path, circleCenter, circleRadius, angle_d, angle_o, -direction );
      path.lineTo( circlePoint( circleCenter, circleRadius + direction * startWidth / 2, angle_o ) );
    }
  }
  else if ( headType == QgsArrowSymbolLayer::HeadReversed )
  {
    path.moveTo( po );
    if ( aType <= 0 )
    {
      path.lineTo( circlePoint( circleCenter, circleRadius + direction * headHeight, angle_o + headAngle ) );
      path.lineTo( circlePoint( circleCenter, circleRadius + direction * width / 2, angle_o + headAngle ) );

      spiralArcTo( path, circleCenter, angle_o + headAngle, circleRadius + direction * width / 2, angle_d, circleRadius + direction * startWidth / 2, direction );
    }
    else
    {
      pathArcTo( path, circleCenter, circleRadius, angle_o, angle_d, direction );
    }
    if ( aType >= 0 )
    {
      path.lineTo( circlePoint( circleCenter, circleRadius - direction * startWidth / 2, angle_d ) );

      spiralArcTo( path, circleCenter, angle_d, circleRadius - direction * startWidth / 2, angle_o + headAngle, circleRadius - direction * width / 2, - direction );

      path.lineTo( circlePoint( circleCenter, circleRadius - direction * headHeight, angle_o + headAngle ) );
      path.lineTo( po );
    }
    else
    {
      path.lineTo( pd );
      pathArcTo( path, circleCenter, circleRadius, angle_d, angle_o, -direction );
    }
  }

  return path.toSubpathPolygons().at( 0 );
}

void QgsArrowSymbolLayer::_resolveDataDefined( QgsSymbolRenderContext &context )
{
  if ( !dataDefinedProperties().hasActiveProperties() )
    return; // shortcut if case there is no data defined properties at all

  QVariant exprVal;
  bool ok;
  if ( mDataDefinedProperties.isActive( QgsSymbolLayer::PropertyArrowWidth ) )
  {
    exprVal = mDataDefinedProperties.value( QgsSymbolLayer::PropertyArrowWidth, context.renderContext().expressionContext() );
    if ( !QgsVariantUtils::isNull( exprVal ) )
    {
      const double w = exprVal.toDouble( &ok );
      if ( ok )
      {
        mScaledArrowWidth = context.renderContext().convertToPainterUnits( w, arrowWidthUnit(), arrowWidthUnitScale() );
      }
    }
  }
  if ( mDataDefinedProperties.isActive( QgsSymbolLayer::PropertyArrowStartWidth ) )
  {
    context.setOriginalValueVariable( arrowStartWidth() );
    exprVal = mDataDefinedProperties.value( QgsSymbolLayer::PropertyArrowStartWidth, context.renderContext().expressionContext() );
    if ( !QgsVariantUtils::isNull( exprVal ) )
    {
      const double w = exprVal.toDouble( &ok );
      if ( ok )
      {
        mScaledArrowStartWidth = context.renderContext().convertToPainterUnits( w, arrowStartWidthUnit(), arrowStartWidthUnitScale() );
      }
    }
  }
  if ( mDataDefinedProperties.isActive( QgsSymbolLayer::PropertyArrowHeadLength ) )
  {
    context.setOriginalValueVariable( headLength() );
    exprVal = mDataDefinedProperties.value( QgsSymbolLayer::PropertyArrowHeadLength, context.renderContext().expressionContext() );
    if ( !QgsVariantUtils::isNull( exprVal ) )
    {
      const double w = exprVal.toDouble( &ok );
      if ( ok )
      {
        mScaledHeadLength = context.renderContext().convertToPainterUnits( w, headLengthUnit(), headLengthUnitScale() );
      }
    }
  }
  if ( mDataDefinedProperties.isActive( QgsSymbolLayer::PropertyArrowHeadThickness ) )
  {
    context.setOriginalValueVariable( headThickness() );
    exprVal = mDataDefinedProperties.value( QgsSymbolLayer::PropertyArrowHeadThickness, context.renderContext().expressionContext() );
    if ( !QgsVariantUtils::isNull( exprVal ) )
    {
      const double w = exprVal.toDouble( &ok );
      if ( ok )
      {
        mScaledHeadThickness = context.renderContext().convertToPainterUnits( w, headThicknessUnit(), headThicknessUnitScale() );
      }
    }
  }
  if ( mDataDefinedProperties.isActive( QgsSymbolLayer::PropertyOffset ) )
  {
    context.setOriginalValueVariable( offset() );
    exprVal = mDataDefinedProperties.value( QgsSymbolLayer::PropertyOffset, context.renderContext().expressionContext() );
    const double w = exprVal.toDouble( &ok );
    if ( ok )
    {
      mScaledOffset = context.renderContext().convertToPainterUnits( w, offsetUnit(), offsetMapUnitScale() );
    }
  }

  if ( mDataDefinedProperties.isActive( QgsSymbolLayer::PropertyArrowHeadType ) )
  {
    context.setOriginalValueVariable( headType() );
    exprVal = mDataDefinedProperties.value( QgsSymbolLayer::PropertyArrowHeadType, context.renderContext().expressionContext() );
    if ( !QgsVariantUtils::isNull( exprVal ) )
    {
      const HeadType h = QgsSymbolLayerUtils::decodeArrowHeadType( exprVal, &ok );
      if ( ok )
      {
        mComputedHeadType = h;
      }
    }
  }

  if ( mDataDefinedProperties.isActive( QgsSymbolLayer::PropertyArrowType ) )
  {
    context.setOriginalValueVariable( arrowType() );
    exprVal = mDataDefinedProperties.value( QgsSymbolLayer::PropertyArrowType, context.renderContext().expressionContext() );
    if ( !QgsVariantUtils::isNull( exprVal ) )
    {
      const ArrowType h = QgsSymbolLayerUtils::decodeArrowType( exprVal, &ok );
      if ( ok )
      {
        mComputedArrowType = h;
      }
    }
  }
}

void QgsArrowSymbolLayer::renderPolyline( const QPolygonF &points, QgsSymbolRenderContext &context )
{
  Q_UNUSED( points )

  if ( !context.renderContext().painter() )
  {
    return;
  }

  context.renderContext().expressionContext().appendScope( mExpressionScope.get() );
  mExpressionScope->addVariable( QgsExpressionContextScope::StaticVariable( QgsExpressionContext::EXPR_GEOMETRY_POINT_COUNT, points.size() + 1, true ) );
  mExpressionScope->addVariable( QgsExpressionContextScope::StaticVariable( QgsExpressionContext::EXPR_GEOMETRY_POINT_NUM, 1, true ) );

  const bool prevIsSubsymbol = context.renderContext().flags() & Qgis::RenderContextFlag::RenderingSubSymbol;
  context.renderContext().setFlag( Qgis::RenderContextFlag::RenderingSubSymbol );

  const double prevOpacity = mSymbol->opacity();
  mSymbol->setOpacity( prevOpacity * context.opacity() );

  if ( isCurved() )
  {
    _resolveDataDefined( context );

    if ( ! isRepeated() )
    {
      if ( points.size() >= 3 )
      {
        // origin point
        const QPointF po( points.at( 0 ) );
        // middle point
        const QPointF pm( points.at( points.size() / 2 ) );
        // destination point
        const QPointF pd( points.back() );

        const QPolygonF poly = curvedArrow( po, pm, pd, mScaledArrowStartWidth, mScaledArrowWidth, mScaledHeadLength, mScaledHeadThickness, mComputedHeadType, mComputedArrowType, mScaledOffset );
        mSymbol->renderPolygon( poly, /* rings */ nullptr, context.feature(), context.renderContext(), -1, context.selected() );
      }
      // straight arrow
      else if ( points.size() == 2 )
      {
        // origin point
        const QPointF po( points.at( 0 ) );
        // destination point
        const QPointF pd( points.at( 1 ) );

        const QPolygonF poly = straightArrow( po, pd, mScaledArrowStartWidth, mScaledArrowWidth, mScaledHeadLength, mScaledHeadThickness, mComputedHeadType, mComputedArrowType, mScaledOffset );
        mSymbol->renderPolygon( poly, /* rings */ nullptr, context.feature(), context.renderContext(), -1, context.selected() );
      }
    }
    else
    {
      for ( int pIdx = 0; pIdx < points.size() - 1; pIdx += 2 )
      {
        if ( context.renderContext().renderingStopped() )
          break;

        mExpressionScope->addVariable( QgsExpressionContextScope::StaticVariable( QgsExpressionContext::EXPR_GEOMETRY_POINT_NUM, pIdx + 1, true ) );
        _resolveDataDefined( context );

        if ( points.size() - pIdx >= 3 )
        {
          // origin point
          const QPointF po( points.at( pIdx ) );
          // middle point
          const QPointF pm( points.at( pIdx + 1 ) );
          // destination point
          const QPointF pd( points.at( pIdx + 2 ) );

          const QPolygonF poly = curvedArrow( po, pm, pd, mScaledArrowStartWidth, mScaledArrowWidth, mScaledHeadLength, mScaledHeadThickness, mComputedHeadType, mComputedArrowType, mScaledOffset );
          mSymbol->renderPolygon( poly, /* rings */ nullptr, context.feature(), context.renderContext(), -1, context.selected() );
        }
        // straight arrow
        else if ( points.size() - pIdx == 2 )
        {
          // origin point
          const QPointF po( points.at( pIdx ) );
          // destination point
          const QPointF pd( points.at( pIdx + 1 ) );

          const QPolygonF poly = straightArrow( po, pd, mScaledArrowStartWidth, mScaledArrowWidth, mScaledHeadLength, mScaledHeadThickness, mComputedHeadType, mComputedArrowType, mScaledOffset );
          mSymbol->renderPolygon( poly, /* rings */ nullptr, context.feature(), context.renderContext(), -1, context.selected() );
        }
      }
    }
  }
  else
  {
    if ( !isRepeated() )
    {
      _resolveDataDefined( context );

      if ( !points.isEmpty() )
      {
        // origin point
        const QPointF po( points.at( 0 ) );
        // destination point
        const QPointF pd( points.back() );

        const QPolygonF poly = straightArrow( po, pd, mScaledArrowStartWidth, mScaledArrowWidth, mScaledHeadLength, mScaledHeadThickness, mComputedHeadType, mComputedArrowType, mScaledOffset );
        mSymbol->renderPolygon( poly, /* rings */ nullptr, context.feature(), context.renderContext(), -1, context.selected() );
      }
    }
    else
    {
      // only straight arrows
      for ( int pIdx = 0; pIdx < points.size() - 1; pIdx++ )
      {
        if ( context.renderContext().renderingStopped() )
          break;

        mExpressionScope->addVariable( QgsExpressionContextScope::StaticVariable( QgsExpressionContext::EXPR_GEOMETRY_POINT_NUM, pIdx + 1, true ) );
        _resolveDataDefined( context );

        // origin point
        const QPointF po( points.at( pIdx ) );
        // destination point
        const QPointF pd( points.at( pIdx + 1 ) );

        const QPolygonF poly = straightArrow( po, pd, mScaledArrowStartWidth, mScaledArrowWidth, mScaledHeadLength, mScaledHeadThickness, mComputedHeadType, mComputedArrowType, mScaledOffset );

        mSymbol->renderPolygon( poly, /* rings */ nullptr, context.feature(), context.renderContext(), -1, context.selected() );
      }
    }
  }

  context.renderContext().setFlag( Qgis::RenderContextFlag::RenderingSubSymbol, prevIsSubsymbol );

  mSymbol->setOpacity( prevOpacity );
  context.renderContext().expressionContext().popScope();
}

void QgsArrowSymbolLayer::setColor( const QColor &c )
{
  if ( mSymbol )
    mSymbol->setColor( c );

  mColor = c;
}

QColor QgsArrowSymbolLayer::color() const
{
  return mSymbol.get() ? mSymbol->color() : mColor;
}

bool QgsArrowSymbolLayer::canCauseArtifactsBetweenAdjacentTiles() const
{
  return true;
}

