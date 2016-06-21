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

QgsArrowSymbolLayer::QgsArrowSymbolLayer()
    : QgsLineSymbolLayerV2()
    , mArrowWidth( 1.0 )
    , mArrowWidthUnit( QgsSymbolV2::MM )
    , mArrowStartWidth( 1.0 )
    , mArrowStartWidthUnit( QgsSymbolV2::MM )
    , mHeadLength( 1.5 )
    , mHeadLengthUnit( QgsSymbolV2::MM )
    , mHeadThickness( 1.5 )
    , mHeadThicknessUnit( QgsSymbolV2::MM )
    , mHeadType( HeadSingle )
    , mArrowType( ArrowPlain )
    , mIsCurved( true )
    , mIsRepeated( true )
    , mScaledArrowWidth( 1.0 )
    , mScaledArrowStartWidth( 1.0 )
    , mScaledHeadLength( 1.5 )
    , mScaledHeadThickness( 1.5 )
    , mScaledOffset( 0.0 )
    , mComputedHeadType( HeadSingle )
    , mComputedArrowType( ArrowPlain )
{
  /* default values */
  setOffset( 0.0 );
  setOffsetUnit( QgsSymbolV2::MM );

  mSymbol.reset( static_cast<QgsFillSymbolV2*>( QgsFillSymbolV2::createSimple( QgsStringMap() ) ) );
}

bool QgsArrowSymbolLayer::setSubSymbol( QgsSymbolV2* symbol )
{
  if ( symbol && symbol->type() == QgsSymbolV2::Fill )
  {
    mSymbol.reset( static_cast<QgsFillSymbolV2*>( symbol ) );
    return true;
  }
  delete symbol;
  return false;
}

QgsSymbolLayerV2* QgsArrowSymbolLayer::create( const QgsStringMap& props )
{
  QgsArrowSymbolLayer* l = new QgsArrowSymbolLayer();

  if ( props.contains( "arrow_width" ) )
    l->setArrowWidth( props["arrow_width"].toDouble() );

  if ( props.contains( "arrow_width_unit" ) )
    l->setArrowWidthUnit( QgsSymbolLayerV2Utils::decodeOutputUnit( props["arrow_width_unit"] ) );

  if ( props.contains( "arrow_width_unit_scale" ) )
    l->setArrowWidthUnitScale( QgsSymbolLayerV2Utils::decodeMapUnitScale( props["arrow_width_unit_scale"] ) );

  if ( props.contains( "arrow_start_width" ) )
    l->setArrowStartWidth( props["arrow_start_width"].toDouble() );

  if ( props.contains( "arrow_start_width_unit" ) )
    l->setArrowStartWidthUnit( QgsSymbolLayerV2Utils::decodeOutputUnit( props["arrow_start_width_unit"] ) );

  if ( props.contains( "arrow_start_width_unit_scale" ) )
    l->setArrowStartWidthUnitScale( QgsSymbolLayerV2Utils::decodeMapUnitScale( props["arrow_start_width_unit_scale"] ) );

  if ( props.contains( "is_curved" ) )
    l->setIsCurved( props["is_curved"].toInt() == 1 );

  if ( props.contains( "is_repeated" ) )
    l->setIsRepeated( props["is_repeated"].toInt() == 1 );

  if ( props.contains( "head_length" ) )
    l->setHeadLength( props["head_length"].toDouble() );

  if ( props.contains( "head_length_unit" ) )
    l->setHeadLengthUnit( QgsSymbolLayerV2Utils::decodeOutputUnit( props["head_length_unit"] ) );

  if ( props.contains( "head_length_unit_scale" ) )
    l->setHeadLengthUnitScale( QgsSymbolLayerV2Utils::decodeMapUnitScale( props["head_length_unit_scale"] ) );

  if ( props.contains( "head_thickness" ) )
    l->setHeadThickness( props["head_thickness"].toDouble() );

  if ( props.contains( "head_thickness_unit" ) )
    l->setHeadThicknessUnit( QgsSymbolLayerV2Utils::decodeOutputUnit( props["head_thickness_unit"] ) );

  if ( props.contains( "head_thickness_unit_scale" ) )
    l->setHeadThicknessUnitScale( QgsSymbolLayerV2Utils::decodeMapUnitScale( props["head_thickness_unit_scale"] ) );

  if ( props.contains( "head_type" ) )
    l->setHeadType( static_cast<HeadType>( props["head_type"].toInt() ) );

  if ( props.contains( "arrow_type" ) )
    l->setArrowType( static_cast<ArrowType>( props["arrow_type"].toInt() ) );

  if ( props.contains( "offset" ) )
    l->setOffset( props["offset"].toDouble() );

  if ( props.contains( "offset_unit" ) )
    l->setOffsetUnit( QgsSymbolLayerV2Utils::decodeOutputUnit( props["offset_unit"] ) );

  if ( props.contains( "offset_unit_scale" ) )
    l->setOffsetMapUnitScale( QgsSymbolLayerV2Utils::decodeMapUnitScale( props["offset_unit_scale"] ) );

  l->restoreDataDefinedProperties( props );

  l->setSubSymbol( QgsFillSymbolV2::createSimple( props ) );

  return l;
}

QgsArrowSymbolLayer* QgsArrowSymbolLayer::clone() const
{
  QgsArrowSymbolLayer* l = static_cast<QgsArrowSymbolLayer*>( create( properties() ) );
  l->setSubSymbol( mSymbol->clone() );
  copyDataDefinedProperties( l );
  copyPaintEffect( l );
  return l;
}

QString QgsArrowSymbolLayer::layerType() const
{
  return "ArrowLine";
}

QgsStringMap QgsArrowSymbolLayer::properties() const
{
  QgsStringMap map;

  map["arrow_width"] = QString::number( arrowWidth() );
  map["arrow_width_unit"] = QgsSymbolLayerV2Utils::encodeOutputUnit( arrowWidthUnit() );
  map["arrow_width_unit_scale"] = QgsSymbolLayerV2Utils::encodeMapUnitScale( arrowWidthUnitScale() );

  map["arrow_start_width"] = QString::number( arrowStartWidth() );
  map["arrow_start_width_unit"] = QgsSymbolLayerV2Utils::encodeOutputUnit( arrowStartWidthUnit() );
  map["arrow_start_width_unit_scale"] = QgsSymbolLayerV2Utils::encodeMapUnitScale( arrowStartWidthUnitScale() );

  map["is_curved"] = QString::number( isCurved() ? 1 : 0 );
  map["is_repeated"] = QString::number( isRepeated() ? 1 : 0 );

  map["head_length"] = QString::number( headLength() );
  map["head_length_unit"] = QgsSymbolLayerV2Utils::encodeOutputUnit( headLengthUnit() );
  map["head_length_unit_scale"] = QgsSymbolLayerV2Utils::encodeMapUnitScale( headLengthUnitScale() );

  map["head_thickness"] = QString::number( headThickness() );
  map["head_thickness_unit"] = QgsSymbolLayerV2Utils::encodeOutputUnit( headThicknessUnit() );
  map["head_thickness_unit_scale"] = QgsSymbolLayerV2Utils::encodeMapUnitScale( headThicknessUnitScale() );

  map["head_type"] = QString::number( headType() );
  map["arrow_type"] = QString::number( arrowType() );

  map["offset"] = QString::number( offset() );
  map["offset_unit"] = QgsSymbolLayerV2Utils::encodeOutputUnit( offsetUnit() );
  map["offset_unit_scale"] = QgsSymbolLayerV2Utils::encodeMapUnitScale( offsetMapUnitScale() );

  saveDataDefinedProperties( map );
  return map;
}

QSet<QString> QgsArrowSymbolLayer::usedAttributes() const
{
  QSet<QString> attributes = QgsLineSymbolLayerV2::usedAttributes();

  attributes.unite( mSymbol->usedAttributes() );

  return attributes;
}


void QgsArrowSymbolLayer::startRender( QgsSymbolV2RenderContext& context )
{
  mExpressionScope.reset( new QgsExpressionContextScope() );
  mScaledArrowWidth = QgsSymbolLayerV2Utils::convertToPainterUnits( context.renderContext(), arrowWidth(), arrowWidthUnit(), arrowWidthUnitScale() );
  mScaledArrowStartWidth = QgsSymbolLayerV2Utils::convertToPainterUnits( context.renderContext(), arrowStartWidth(), arrowStartWidthUnit(), arrowStartWidthUnitScale() );
  mScaledHeadLength = QgsSymbolLayerV2Utils::convertToPainterUnits( context.renderContext(), headLength(), headLengthUnit(), headLengthUnitScale() );
  mScaledHeadThickness = QgsSymbolLayerV2Utils::convertToPainterUnits( context.renderContext(), headThickness(), headThicknessUnit(), headThicknessUnitScale() );
  mScaledOffset = QgsSymbolLayerV2Utils::convertToPainterUnits( context.renderContext(), offset(), offsetUnit(), offsetMapUnitScale() );
  mComputedHeadType = headType();
  mComputedArrowType = arrowType();

  mSymbol->startRender( context.renderContext() );
}

void QgsArrowSymbolLayer::stopRender( QgsSymbolV2RenderContext& context )
{
  mSymbol->stopRender( context.renderContext() );
}

inline qreal euclidian_distance( const QPointF& po, const QPointF& pd )
{
  return sqrt(( po.x() - pd.x() ) * ( po.x() - pd.x() ) + ( po.y() - pd.y() ) * ( po.y() - pd.y() ) );
}

QPolygonF straightArrow( QPointF po, QPointF pd,
                         qreal startWidth, qreal width,
                         qreal headWidth, qreal headHeight,
                         QgsArrowSymbolLayer::HeadType headType, QgsArrowSymbolLayer::ArrowType arrowType,
                         qreal offset )
{
  QPolygonF polygon; // implicitly shared
  // vector length
  qreal length = euclidian_distance( po, pd );

  // shift points if there is not enough room for the head(s)
  if (( headType == QgsArrowSymbolLayer::HeadSingle ) && ( length < headWidth ) )
  {
    po = pd - ( pd - po ) / length * headWidth;
    length = headWidth;
  }
  else if (( headType == QgsArrowSymbolLayer::HeadReversed ) && ( length < headWidth ) )
  {
    pd = po + ( pd - po ) / length * headWidth;
    length = headWidth;
  }
  else if (( headType == QgsArrowSymbolLayer::HeadDouble ) && ( length < 2 * headWidth ) )
  {
    QPointF v = ( pd - po ) / length * headWidth;
    QPointF npo = ( po + pd ) / 2.0 - v;
    QPointF npd = ( po + pd ) / 2.0 + v;
    po = npo;
    pd = npd;
    length = 2 * headWidth;
  }

  qreal bodyLength = length - headWidth;

  // unit vector
  QPointF unitVec = ( pd - po ) / length;
  // perpendicular vector
  QPointF perpVec( -unitVec.y(), unitVec.x() );

  // set offset
  po += perpVec * offset;
  pd += perpVec * offset;

  if ( headType == QgsArrowSymbolLayer::HeadDouble )
  {
    // first head
    polygon << po;
    if ( arrowType == QgsArrowSymbolLayer::ArrowPlain || arrowType == QgsArrowSymbolLayer::ArrowRightHalf )
    {
      polygon << po + unitVec * headWidth + perpVec * headHeight;
      polygon << po + unitVec * headWidth + perpVec * ( width * 0.5 );

      polygon << po + unitVec * bodyLength + perpVec * ( width * 0.5 );

      // second head
      polygon << po + unitVec * bodyLength + perpVec * headHeight;
    }
    polygon << pd;

    if ( arrowType == QgsArrowSymbolLayer::ArrowPlain || arrowType == QgsArrowSymbolLayer::ArrowLeftHalf )
    {
      polygon << po + unitVec * bodyLength - perpVec * headHeight;
      polygon << po + unitVec * bodyLength - perpVec * ( width * 0.5 );

      // end of the first head
      polygon << po + unitVec * headWidth - perpVec * ( width * 0.5 );
      polygon << po + unitVec * headWidth - perpVec * headHeight;
    }
  }
  else if ( headType == QgsArrowSymbolLayer::HeadSingle )
  {
    if ( arrowType == QgsArrowSymbolLayer::ArrowPlain || arrowType == QgsArrowSymbolLayer::ArrowRightHalf )
    {
      polygon << po + perpVec * ( startWidth * 0.5 );
      polygon << po + unitVec * bodyLength + perpVec * ( width * 0.5 );
      polygon << po + unitVec * bodyLength + perpVec * headHeight;
    }
    else
    {
      polygon << po;
    }
    polygon << pd;
    if ( arrowType == QgsArrowSymbolLayer::ArrowPlain || arrowType == QgsArrowSymbolLayer::ArrowLeftHalf )
    {
      polygon << po + unitVec * bodyLength - perpVec * headHeight;
      polygon << po + unitVec * bodyLength - perpVec * ( width * 0.5 );
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
      polygon << po + unitVec * headWidth + perpVec * headHeight;
      polygon << po + unitVec * headWidth + perpVec * ( width * 0.5 );

      polygon << pd + perpVec * ( startWidth * 0.5 );
    }
    else
    {
      polygon << pd;
    }
    if ( arrowType == QgsArrowSymbolLayer::ArrowPlain || arrowType == QgsArrowSymbolLayer::ArrowLeftHalf )
    {
      polygon << pd - perpVec * ( startWidth * 0.5 );

      polygon << po + unitVec * headWidth - perpVec * ( width * 0.5 );
      polygon << po + unitVec * headWidth - perpVec * headHeight;
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
 * @return false if the three points are colinear
 */
bool pointsToCircle( const QPointF& a, const QPointF& b, const QPointF& c, QPointF& center, qreal& radius )
{
  qreal cx, cy;

  // AB and BC vectors
  QPointF ab = b - a;
  QPointF bc = c - b;

  // AB and BC middles
  QPointF ab2 = ( a + b ) / 2.0;
  QPointF bc2 = ( b + c ) / 2.0;

  // Aligned points
  if ( fabs( ab.x() * bc.y() - ab.y() * bc.x() ) < 0.001 ) // Empirical threshold for nearly aligned points
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
  radius = sqrt(( a.x() - cx ) * ( a.x() - cx ) + ( a.y() - cy ) * ( a.y() - cy ) );
  // Center
  center.setX( cx );
  center.setY( cy );
  return true;
}

QPointF circlePoint( const QPointF& center, qreal radius, qreal angle )
{
  // Y is oriented downward
  return QPointF( cos( -angle ) * radius + center.x(), sin( -angle ) * radius + center.y() );
}

void pathArcTo( QPainterPath& path, const QPointF& circleCenter, qreal circleRadius, qreal angle_o, qreal angle_d, int direction )
{
  QRectF circleRect( circleCenter - QPointF( circleRadius, circleRadius ), circleCenter + QPointF( circleRadius, circleRadius ) );
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
void spiralArcTo( QPainterPath& path, const QPointF& center, qreal startAngle, qreal startRadius, qreal endAngle, qreal endRadius, int direction )
{
  // start point
  QPointF A = circlePoint( center, startRadius, startAngle );
  // end point
  QPointF B = circlePoint( center, endRadius, endAngle );
  // middle points
  qreal deltaAngle;

  deltaAngle = endAngle - startAngle;
  if ( direction * deltaAngle < 0.0 )
    deltaAngle = deltaAngle + direction * 2 * M_PI;

  QPointF I1 = circlePoint( center, 0.75 * startRadius + 0.25 * endRadius, startAngle + 0.25 * deltaAngle );
  QPointF I2 = circlePoint( center, 0.50 * startRadius + 0.50 * endRadius, startAngle + 0.50 * deltaAngle );
  QPointF I3 = circlePoint( center, 0.25 * startRadius + 0.75 * endRadius, startAngle + 0.75 * deltaAngle );

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
    qreal a1 = atan2( cCenter.y() - A.y(), A.x() - cCenter.x() );
    qreal a2 = atan2( cCenter.y() - I2.y(), I2.x() - cCenter.x() );
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
    qreal a1 = atan2( cCenter.y() - I2.y(), I2.x() - cCenter.x() );
    qreal a2 = atan2( cCenter.y() - B.y(), B.x() - cCenter.x() );
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
  qreal angle_o = clampAngle( atan2( circleCenter.y() - po.y(), po.x() - circleCenter.x() ) );
  qreal angle_m = clampAngle( atan2( circleCenter.y() - pm.y(), pm.x() - circleCenter.x() ) );
  qreal angle_d = clampAngle( atan2( circleCenter.y() - pd.y(), pd.x() - circleCenter.x() ) );

  // arc direction : 1 = counter-clockwise, -1 = clockwise
  int direction = clampAngle( angle_m - angle_o ) < clampAngle( angle_m - angle_d ) ? 1 : -1;

  // arrow type, independent of the direction
  int aType = 0;
  if ( arrowType == QgsArrowSymbolLayer::ArrowRightHalf )
    aType = direction;
  else if ( arrowType == QgsArrowSymbolLayer::ArrowLeftHalf )
    aType = -direction;

  qreal deltaAngle = angle_d - angle_o;
  if ( direction * deltaAngle < 0.0 )
    deltaAngle = deltaAngle + direction * 2 * M_PI;

  qreal length = euclidian_distance( po, pd );
  // for close points and deltaAngle < 180, draw a straight line
  if ( fabs( deltaAngle ) < M_PI && ((( headType == QgsArrowSymbolLayer::HeadSingle ) && ( length < headWidth ) ) ||
                                     (( headType == QgsArrowSymbolLayer::HeadReversed ) && ( length < headWidth ) ) ||
                                     (( headType == QgsArrowSymbolLayer::HeadDouble ) && ( length < 2*headWidth ) ) ) )
  {
    return straightArrow( po, pd, startWidth, width, headWidth, headHeight, headType, arrowType, offset );
  }

  // ajust coordinates to include offset
  circleRadius += offset;
  po = circlePoint( circleCenter, circleRadius, angle_o );
  pm = circlePoint( circleCenter, circleRadius, angle_m );
  pd = circlePoint( circleCenter, circleRadius, angle_d );

  qreal headAngle = direction * atan( headWidth / circleRadius );

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

void QgsArrowSymbolLayer::_resolveDataDefined( QgsSymbolV2RenderContext& context )
{
  if ( !hasDataDefinedProperties() )
    return; // shortcut if case there is no data defined properties at all

  bool ok;
  if ( hasDataDefinedProperty( "arrow_width" ) )
  {
    context.setOriginalValueVariable( arrowWidth() );
    double w = evaluateDataDefinedProperty( "arrow_width", context, QVariant(), &ok ).toDouble();
    if ( ok )
    {
      mScaledArrowWidth = QgsSymbolLayerV2Utils::convertToPainterUnits( context.renderContext(), w, arrowWidthUnit(), arrowWidthUnitScale() );
    }
  }
  if ( hasDataDefinedProperty( "arrow_start_width" ) )
  {
    context.setOriginalValueVariable( arrowStartWidth() );
    double w = evaluateDataDefinedProperty( "arrow_start_width", context, QVariant(), &ok ).toDouble();
    if ( ok )
    {
      mScaledArrowStartWidth = QgsSymbolLayerV2Utils::convertToPainterUnits( context.renderContext(), w, arrowStartWidthUnit(), arrowStartWidthUnitScale() );
    }
  }
  if ( hasDataDefinedProperty( "head_length" ) )
  {
    context.setOriginalValueVariable( headLength() );
    double w = evaluateDataDefinedProperty( "head_length", context, QVariant(), &ok ).toDouble();
    if ( ok )
    {
      mScaledHeadLength = QgsSymbolLayerV2Utils::convertToPainterUnits( context.renderContext(), w, headLengthUnit(), headLengthUnitScale() );
    }
  }
  if ( hasDataDefinedProperty( "head_thickness" ) )
  {
    context.setOriginalValueVariable( headThickness() );
    double w = evaluateDataDefinedProperty( "head_thickness", context, QVariant(), &ok ).toDouble();
    if ( ok )
    {
      mScaledHeadThickness = QgsSymbolLayerV2Utils::convertToPainterUnits( context.renderContext(), w, headThicknessUnit(), headThicknessUnitScale() );
    }
  }
  if ( hasDataDefinedProperty( QgsSymbolLayerV2::EXPR_OFFSET ) )
  {
    context.setOriginalValueVariable( offset() );
    double w = evaluateDataDefinedProperty( QgsSymbolLayerV2::EXPR_OFFSET, context, QVariant(), &ok ).toDouble();
    if ( ok )
    {
      mScaledOffset = QgsSymbolLayerV2Utils::convertToPainterUnits( context.renderContext(), w, offsetUnit(), offsetMapUnitScale() );
    }
  }

  if ( hasDataDefinedProperty( "head_type" ) )
  {
    context.setOriginalValueVariable( headType() );
    int h = evaluateDataDefinedProperty( "head_type", context, QVariant(), &ok ).toInt();
    if ( ok )
    {
      mComputedHeadType = static_cast<HeadType>( h );
    }
  }

  if ( hasDataDefinedProperty( "arrow_type" ) )
  {
    context.setOriginalValueVariable( arrowType() );
    int h = evaluateDataDefinedProperty( "arrow_type", context, QVariant(), &ok ).toInt();
    if ( ok )
    {
      mComputedArrowType = static_cast<ArrowType>( h );
    }
  }
}

void QgsArrowSymbolLayer::renderPolyline( const QPolygonF& points, QgsSymbolV2RenderContext& context )
{
  Q_UNUSED( points );

  if ( !context.renderContext().painter() )
  {
    return;
  }

  context.renderContext().expressionContext().appendScope( mExpressionScope.data() );
  mExpressionScope->setVariable( QgsExpressionContext::EXPR_GEOMETRY_POINT_COUNT, points.size() + 1 );
  mExpressionScope->setVariable( QgsExpressionContext::EXPR_GEOMETRY_POINT_NUM, 1 );
  if ( isCurved() )
  {
    _resolveDataDefined( context );

    if ( ! isRepeated() )
    {
      if ( points.size() >= 3 )
      {
        // origin point
        QPointF po( points.at( 0 ) );
        // middle point
        QPointF pm( points.at( points.size() / 2 ) );
        // destination point
        QPointF pd( points.back() );

        QPolygonF poly = curvedArrow( po, pm, pd, mScaledArrowStartWidth, mScaledArrowWidth, mScaledHeadLength, mScaledHeadThickness, mComputedHeadType, mComputedArrowType, mScaledOffset );
        mSymbol->renderPolygon( poly, /* rings */ nullptr, context.feature(), context.renderContext() );
      }
      // straight arrow
      else if ( points.size() == 2 )
      {
        // origin point
        QPointF po( points.at( 0 ) );
        // destination point
        QPointF pd( points.at( 1 ) );

        QPolygonF poly = straightArrow( po, pd, mScaledArrowStartWidth, mScaledArrowWidth, mScaledHeadLength, mScaledHeadThickness, mComputedHeadType, mComputedArrowType, mScaledOffset );
        mSymbol->renderPolygon( poly, /* rings */ nullptr, context.feature(), context.renderContext() );
      }
    }
    else
    {
      for ( int pIdx = 0; pIdx < points.size() - 1; pIdx += 2 )
      {
        mExpressionScope->setVariable( QgsExpressionContext::EXPR_GEOMETRY_POINT_NUM, pIdx + 1 );
        _resolveDataDefined( context );

        if ( points.size() - pIdx >= 3 )
        {
          // origin point
          QPointF po( points.at( pIdx ) );
          // middle point
          QPointF pm( points.at( pIdx + 1 ) );
          // destination point
          QPointF pd( points.at( pIdx + 2 ) );

          QPolygonF poly = curvedArrow( po, pm, pd, mScaledArrowStartWidth, mScaledArrowWidth, mScaledHeadLength, mScaledHeadThickness, mComputedHeadType, mComputedArrowType, mScaledOffset );
          mSymbol->renderPolygon( poly, /* rings */ nullptr, context.feature(), context.renderContext() );
        }
        // straight arrow
        else if ( points.size() - pIdx == 2 )
        {
          // origin point
          QPointF po( points.at( pIdx ) );
          // destination point
          QPointF pd( points.at( pIdx + 1 ) );

          QPolygonF poly = straightArrow( po, pd, mScaledArrowStartWidth, mScaledArrowWidth, mScaledHeadLength, mScaledHeadThickness, mComputedHeadType, mComputedArrowType, mScaledOffset );
          mSymbol->renderPolygon( poly, /* rings */ nullptr, context.feature(), context.renderContext() );
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
        QPointF po( points.at( 0 ) );
        // destination point
        QPointF pd( points.back() );

        QPolygonF poly = straightArrow( po, pd, mScaledArrowStartWidth, mScaledArrowWidth, mScaledHeadLength, mScaledHeadThickness, mComputedHeadType, mComputedArrowType, mScaledOffset );
        mSymbol->renderPolygon( poly, /* rings */ nullptr, context.feature(), context.renderContext() );
      }
    }
    else
    {
      // only straight arrows
      for ( int pIdx = 0; pIdx < points.size() - 1; pIdx++ )
      {
        mExpressionScope->setVariable( QgsExpressionContext::EXPR_GEOMETRY_POINT_NUM, pIdx + 1 );
        _resolveDataDefined( context );

        // origin point
        QPointF po( points.at( pIdx ) );
        // destination point
        QPointF pd( points.at( pIdx + 1 ) );

        QPolygonF poly = straightArrow( po, pd, mScaledArrowStartWidth, mScaledArrowWidth, mScaledHeadLength, mScaledHeadThickness, mComputedHeadType, mComputedArrowType, mScaledOffset );
        mSymbol->renderPolygon( poly, /* rings */ nullptr, context.feature(), context.renderContext() );
      }
    }
  }
  context.renderContext().expressionContext().popScope();
}

void QgsArrowSymbolLayer::setColor( const QColor& c )
{
  if ( mSymbol.data() )
    mSymbol->setColor( c );

  mColor = c;
}

QColor QgsArrowSymbolLayer::color() const
{
  return mSymbol.data() ? mSymbol->color() : mColor;
}

