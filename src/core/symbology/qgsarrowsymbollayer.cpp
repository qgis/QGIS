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

#include <memory>

#include "qgsfillsymbol.h"
#include "qgsgeometryutils_base.h"
#include "qgsrendercontext.h"
#include "qgssymbollayerutils.h"
#include "qgsunittypes.h"

QgsArrowSymbolLayer::QgsArrowSymbolLayer()
{
  /* default values */
  setOffset( 0.0 );
  setOffsetUnit( Qgis::RenderUnit::Millimeters );

  mSymbol = QgsFillSymbol::createSimple( QVariantMap() );
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

  if ( props.contains( u"arrow_width"_s ) )
    l->setArrowWidth( props[u"arrow_width"_s].toDouble() );

  if ( props.contains( u"arrow_width_unit"_s ) )
    l->setArrowWidthUnit( QgsUnitTypes::decodeRenderUnit( props[u"arrow_width_unit"_s].toString() ) );

  if ( props.contains( u"arrow_width_unit_scale"_s ) )
    l->setArrowWidthUnitScale( QgsSymbolLayerUtils::decodeMapUnitScale( props[u"arrow_width_unit_scale"_s].toString() ) );

  if ( props.contains( u"arrow_start_width"_s ) )
    l->setArrowStartWidth( props[u"arrow_start_width"_s].toDouble() );

  if ( props.contains( u"arrow_start_width_unit"_s ) )
    l->setArrowStartWidthUnit( QgsUnitTypes::decodeRenderUnit( props[u"arrow_start_width_unit"_s].toString() ) );

  if ( props.contains( u"arrow_start_width_unit_scale"_s ) )
    l->setArrowStartWidthUnitScale( QgsSymbolLayerUtils::decodeMapUnitScale( props[u"arrow_start_width_unit_scale"_s].toString() ) );

  if ( props.contains( u"is_curved"_s ) )
    l->setIsCurved( props[u"is_curved"_s].toInt() == 1 );

  if ( props.contains( u"is_repeated"_s ) )
    l->setIsRepeated( props[u"is_repeated"_s].toInt() == 1 );

  if ( props.contains( u"head_length"_s ) )
    l->setHeadLength( props[u"head_length"_s].toDouble() );

  if ( props.contains( u"head_length_unit"_s ) )
    l->setHeadLengthUnit( QgsUnitTypes::decodeRenderUnit( props[u"head_length_unit"_s].toString() ) );

  if ( props.contains( u"head_length_unit_scale"_s ) )
    l->setHeadLengthUnitScale( QgsSymbolLayerUtils::decodeMapUnitScale( props[u"head_length_unit_scale"_s].toString() ) );

  if ( props.contains( u"head_thickness"_s ) )
    l->setHeadThickness( props[u"head_thickness"_s].toDouble() );

  if ( props.contains( u"head_thickness_unit"_s ) )
    l->setHeadThicknessUnit( QgsUnitTypes::decodeRenderUnit( props[u"head_thickness_unit"_s].toString() ) );

  if ( props.contains( u"head_thickness_unit_scale"_s ) )
    l->setHeadThicknessUnitScale( QgsSymbolLayerUtils::decodeMapUnitScale( props[u"head_thickness_unit_scale"_s].toString() ) );

  if ( props.contains( u"head_type"_s ) )
    l->setHeadType( static_cast<HeadType>( props[u"head_type"_s].toInt() ) );

  if ( props.contains( u"arrow_type"_s ) )
    l->setArrowType( static_cast<ArrowType>( props[u"arrow_type"_s].toInt() ) );

  if ( props.contains( u"offset"_s ) )
    l->setOffset( props[u"offset"_s].toDouble() );

  if ( props.contains( u"offset_unit"_s ) )
    l->setOffsetUnit( QgsUnitTypes::decodeRenderUnit( props[u"offset_unit"_s].toString() ) );

  if ( props.contains( u"offset_unit_scale"_s ) )
    l->setOffsetMapUnitScale( QgsSymbolLayerUtils::decodeMapUnitScale( props[u"offset_unit_scale"_s].toString() ) );

  if ( props.contains( u"ring_filter"_s ) )
    l->setRingFilter( static_cast< RenderRingFilter>( props[u"ring_filter"_s].toInt() ) );

  l->restoreOldDataDefinedProperties( props );

  l->setSubSymbol( QgsFillSymbol::createSimple( props ).release() );

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
  return u"ArrowLine"_s;
}

QVariantMap QgsArrowSymbolLayer::properties() const
{
  QVariantMap map;

  map[u"arrow_width"_s] = QString::number( arrowWidth() );
  map[u"arrow_width_unit"_s] = QgsUnitTypes::encodeUnit( arrowWidthUnit() );
  map[u"arrow_width_unit_scale"_s] = QgsSymbolLayerUtils::encodeMapUnitScale( arrowWidthUnitScale() );

  map[u"arrow_start_width"_s] = QString::number( arrowStartWidth() );
  map[u"arrow_start_width_unit"_s] = QgsUnitTypes::encodeUnit( arrowStartWidthUnit() );
  map[u"arrow_start_width_unit_scale"_s] = QgsSymbolLayerUtils::encodeMapUnitScale( arrowStartWidthUnitScale() );

  map[u"is_curved"_s] = QString::number( isCurved() ? 1 : 0 );
  map[u"is_repeated"_s] = QString::number( isRepeated() ? 1 : 0 );

  map[u"head_length"_s] = QString::number( headLength() );
  map[u"head_length_unit"_s] = QgsUnitTypes::encodeUnit( headLengthUnit() );
  map[u"head_length_unit_scale"_s] = QgsSymbolLayerUtils::encodeMapUnitScale( headLengthUnitScale() );

  map[u"head_thickness"_s] = QString::number( headThickness() );
  map[u"head_thickness_unit"_s] = QgsUnitTypes::encodeUnit( headThicknessUnit() );
  map[u"head_thickness_unit_scale"_s] = QgsSymbolLayerUtils::encodeMapUnitScale( headThicknessUnitScale() );

  map[u"head_type"_s] = QString::number( headType() );
  map[u"arrow_type"_s] = QString::number( arrowType() );

  map[u"offset"_s] = QString::number( offset() );
  map[u"offset_unit"_s] = QgsUnitTypes::encodeUnit( offsetUnit() );
  map[u"offset_unit_scale"_s] = QgsSymbolLayerUtils::encodeMapUnitScale( offsetMapUnitScale() );

  map[u"ring_filter"_s] = QString::number( static_cast< int >( mRingFilter ) );

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
  return mArrowWidthUnit == Qgis::RenderUnit::MapUnits || mArrowWidthUnit == Qgis::RenderUnit::MetersInMapUnits
         || mArrowStartWidthUnit == Qgis::RenderUnit::MapUnits || mArrowStartWidthUnit == Qgis::RenderUnit::MetersInMapUnits
         || mHeadLengthUnit == Qgis::RenderUnit::MapUnits || mHeadLengthUnit == Qgis::RenderUnit::MetersInMapUnits
         || mHeadThicknessUnit == Qgis::RenderUnit::MapUnits || mHeadThicknessUnit == Qgis::RenderUnit::MetersInMapUnits
         || mWidthUnit == Qgis::RenderUnit::MapUnits || mWidthUnit == Qgis::RenderUnit::MetersInMapUnits
         || mOffsetUnit == Qgis::RenderUnit::MapUnits || mOffsetUnit == Qgis::RenderUnit::MetersInMapUnits;
}

void QgsArrowSymbolLayer::setOutputUnit( Qgis::RenderUnit unit )
{
  QgsLineSymbolLayer::setOutputUnit( unit );
  mArrowWidthUnit = unit;
  mArrowStartWidthUnit = unit;
  mHeadLengthUnit = unit;
  mHeadThicknessUnit = unit;
}

void QgsArrowSymbolLayer::startRender( QgsSymbolRenderContext &context )
{
  mExpressionScope = std::make_unique<QgsExpressionContextScope>( );
  mScaledArrowWidth = context.renderContext().convertToPainterUnits( arrowWidth(), arrowWidthUnit(), arrowWidthUnitScale() );
  mScaledArrowStartWidth = context.renderContext().convertToPainterUnits( arrowStartWidth(), arrowStartWidthUnit(), arrowStartWidthUnitScale() );
  mScaledHeadLength = context.renderContext().convertToPainterUnits( headLength(), headLengthUnit(), headLengthUnitScale() );
  mScaledHeadThickness = context.renderContext().convertToPainterUnits( headThickness(), headThicknessUnit(), headThicknessUnitScale() );
  mScaledOffset = context.renderContext().convertToPainterUnits( offset(), offsetUnit(), offsetMapUnitScale() );
  mComputedHeadType = headType();
  mComputedArrowType = arrowType();

  mSymbol->setRenderHints( mSymbol->renderHints() | Qgis::SymbolRenderHint::IsSymbolLayerSubSymbol );

  mSymbol->startRender( context.renderContext(), context.fields() );
}

void QgsArrowSymbolLayer::stopRender( QgsSymbolRenderContext &context )
{
  mSymbol->stopRender( context.renderContext() );
}

void QgsArrowSymbolLayer::startFeatureRender( const QgsFeature &, QgsRenderContext &context )
{
  installMasks( context, true );

  // The base class version passes this on to the subsymbol, but we deliberately don't do that here.
}

void QgsArrowSymbolLayer::stopFeatureRender( const QgsFeature &, QgsRenderContext &context )
{
  removeMasks( context, true );

  // The base class version passes this on to the subsymbol, but we deliberately don't do that here.
}

inline qreal euclidean_distance( QPointF po, QPointF pd )
{
  return QgsGeometryUtilsBase::distance2D( po.x(), po.y(), pd.x(), pd.y() );
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
  if ( qgsDoubleNear( length, 0 ) )
    return polygon;

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
  radius = QgsGeometryUtilsBase::distance2D( a.x(), a.y(), cx, cy );
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
  if ( mDataDefinedProperties.isActive( QgsSymbolLayer::Property::ArrowWidth ) )
  {
    exprVal = mDataDefinedProperties.value( QgsSymbolLayer::Property::ArrowWidth, context.renderContext().expressionContext() );
    if ( !QgsVariantUtils::isNull( exprVal ) )
    {
      const double w = exprVal.toDouble( &ok );
      if ( ok )
      {
        mScaledArrowWidth = context.renderContext().convertToPainterUnits( w, arrowWidthUnit(), arrowWidthUnitScale() );
      }
    }
  }
  if ( mDataDefinedProperties.isActive( QgsSymbolLayer::Property::ArrowStartWidth ) )
  {
    context.setOriginalValueVariable( arrowStartWidth() );
    exprVal = mDataDefinedProperties.value( QgsSymbolLayer::Property::ArrowStartWidth, context.renderContext().expressionContext() );
    if ( !QgsVariantUtils::isNull( exprVal ) )
    {
      const double w = exprVal.toDouble( &ok );
      if ( ok )
      {
        mScaledArrowStartWidth = context.renderContext().convertToPainterUnits( w, arrowStartWidthUnit(), arrowStartWidthUnitScale() );
      }
    }
  }
  if ( mDataDefinedProperties.isActive( QgsSymbolLayer::Property::ArrowHeadLength ) )
  {
    context.setOriginalValueVariable( headLength() );
    exprVal = mDataDefinedProperties.value( QgsSymbolLayer::Property::ArrowHeadLength, context.renderContext().expressionContext() );
    if ( !QgsVariantUtils::isNull( exprVal ) )
    {
      const double w = exprVal.toDouble( &ok );
      if ( ok )
      {
        mScaledHeadLength = context.renderContext().convertToPainterUnits( w, headLengthUnit(), headLengthUnitScale() );
      }
    }
  }
  if ( mDataDefinedProperties.isActive( QgsSymbolLayer::Property::ArrowHeadThickness ) )
  {
    context.setOriginalValueVariable( headThickness() );
    exprVal = mDataDefinedProperties.value( QgsSymbolLayer::Property::ArrowHeadThickness, context.renderContext().expressionContext() );
    if ( !QgsVariantUtils::isNull( exprVal ) )
    {
      const double w = exprVal.toDouble( &ok );
      if ( ok )
      {
        mScaledHeadThickness = context.renderContext().convertToPainterUnits( w, headThicknessUnit(), headThicknessUnitScale() );
      }
    }
  }
  if ( mDataDefinedProperties.isActive( QgsSymbolLayer::Property::Offset ) )
  {
    context.setOriginalValueVariable( offset() );
    exprVal = mDataDefinedProperties.value( QgsSymbolLayer::Property::Offset, context.renderContext().expressionContext() );
    const double w = exprVal.toDouble( &ok );
    if ( ok )
    {
      mScaledOffset = context.renderContext().convertToPainterUnits( w, offsetUnit(), offsetMapUnitScale() );
    }
  }

  if ( mDataDefinedProperties.isActive( QgsSymbolLayer::Property::ArrowHeadType ) )
  {
    context.setOriginalValueVariable( headType() );
    exprVal = mDataDefinedProperties.value( QgsSymbolLayer::Property::ArrowHeadType, context.renderContext().expressionContext() );
    if ( !QgsVariantUtils::isNull( exprVal ) )
    {
      const HeadType h = QgsSymbolLayerUtils::decodeArrowHeadType( exprVal, &ok );
      if ( ok )
      {
        mComputedHeadType = h;
      }
    }
  }

  if ( mDataDefinedProperties.isActive( QgsSymbolLayer::Property::ArrowType ) )
  {
    context.setOriginalValueVariable( arrowType() );
    exprVal = mDataDefinedProperties.value( QgsSymbolLayer::Property::ArrowType, context.renderContext().expressionContext() );
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

  const bool useSelectedColor = shouldRenderUsingSelectionColor( context );
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
        mSymbol->renderPolygon( poly, /* rings */ nullptr, context.feature(), context.renderContext(), -1, useSelectedColor );
      }
      // straight arrow
      else if ( points.size() == 2 )
      {
        // origin point
        const QPointF po( points.at( 0 ) );
        // destination point
        const QPointF pd( points.at( 1 ) );

        const QPolygonF poly = straightArrow( po, pd, mScaledArrowStartWidth, mScaledArrowWidth, mScaledHeadLength, mScaledHeadThickness, mComputedHeadType, mComputedArrowType, mScaledOffset );
        if ( !poly.isEmpty() )
          mSymbol->renderPolygon( poly, /* rings */ nullptr, context.feature(), context.renderContext(), -1, useSelectedColor );
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
          mSymbol->renderPolygon( poly, /* rings */ nullptr, context.feature(), context.renderContext(), -1, useSelectedColor );
        }
        // straight arrow
        else if ( points.size() - pIdx == 2 )
        {
          // origin point
          const QPointF po( points.at( pIdx ) );
          // destination point
          const QPointF pd( points.at( pIdx + 1 ) );

          const QPolygonF poly = straightArrow( po, pd, mScaledArrowStartWidth, mScaledArrowWidth, mScaledHeadLength, mScaledHeadThickness, mComputedHeadType, mComputedArrowType, mScaledOffset );
          if ( !poly.isEmpty() )
            mSymbol->renderPolygon( poly, /* rings */ nullptr, context.feature(), context.renderContext(), -1, useSelectedColor );
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
        if ( !poly.isEmpty() )
          mSymbol->renderPolygon( poly, /* rings */ nullptr, context.feature(), context.renderContext(), -1, useSelectedColor );
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

        if ( !poly.isEmpty() )
          mSymbol->renderPolygon( poly, /* rings */ nullptr, context.feature(), context.renderContext(), -1, useSelectedColor );
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

