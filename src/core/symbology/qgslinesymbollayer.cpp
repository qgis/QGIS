/***************************************************************************
 qgslinesymbollayer.cpp
 ---------------------
 begin                : November 2009
 copyright            : (C) 2009 by Martin Dobias
 email                : wonder dot sk at gmail dot com
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "qgslinesymbollayer.h"
#include "qgscurve.h"
#include "qgscurvepolygon.h"
#include "qgsdxfexport.h"
#include "qgssymbollayerutils.h"
#include "qgsexpression.h"
#include "qgsrendercontext.h"
#include "qgslogger.h"
#include "qgsvectorlayer.h"
#include "qgsgeometrysimplifier.h"
#include "qgsunittypes.h"
#include "qgsproperty.h"
#include "qgsexpressioncontextutils.h"
#include "qgsmarkersymbol.h"
#include "qgslinesymbol.h"
#include "qgsapplication.h"
#include "qgsimagecache.h"
#include "qgsfeedback.h"
#include "qgsimageoperation.h"
#include "qgscolorrampimpl.h"

#include <algorithm>
#include <QPainter>
#include <QDomDocument>
#include <QDomElement>

#include <cmath>

QgsSimpleLineSymbolLayer::QgsSimpleLineSymbolLayer( const QColor &color, double width, Qt::PenStyle penStyle )
  : mPenStyle( penStyle )
{
  mColor = color;
  mWidth = width;
  mCustomDashVector << 5 << 2;
}

QgsSimpleLineSymbolLayer::~QgsSimpleLineSymbolLayer() = default;

void QgsSimpleLineSymbolLayer::setOutputUnit( QgsUnitTypes::RenderUnit unit )
{
  QgsLineSymbolLayer::setOutputUnit( unit );
  mWidthUnit = unit;
  mOffsetUnit = unit;
  mCustomDashPatternUnit = unit;
  mDashPatternOffsetUnit = unit;
  mTrimDistanceStartUnit = unit;
  mTrimDistanceEndUnit = unit;
}

QgsUnitTypes::RenderUnit  QgsSimpleLineSymbolLayer::outputUnit() const
{
  QgsUnitTypes::RenderUnit unit = QgsLineSymbolLayer::outputUnit();
  if ( mWidthUnit != unit || mOffsetUnit != unit || mCustomDashPatternUnit != unit )
  {
    return QgsUnitTypes::RenderUnknownUnit;
  }
  return unit;
}

bool QgsSimpleLineSymbolLayer::usesMapUnits() const
{
  return mWidthUnit == QgsUnitTypes::RenderMapUnits || mWidthUnit == QgsUnitTypes::RenderMetersInMapUnits
         || mOffsetUnit == QgsUnitTypes::RenderMapUnits || mOffsetUnit == QgsUnitTypes::RenderMetersInMapUnits;
}

void QgsSimpleLineSymbolLayer::setMapUnitScale( const QgsMapUnitScale &scale )
{
  QgsLineSymbolLayer::setMapUnitScale( scale );
  mWidthMapUnitScale = scale;
  mOffsetMapUnitScale = scale;
  mCustomDashPatternMapUnitScale = scale;
}

QgsMapUnitScale QgsSimpleLineSymbolLayer::mapUnitScale() const
{
  if ( QgsLineSymbolLayer::mapUnitScale() == mWidthMapUnitScale &&
       mWidthMapUnitScale == mOffsetMapUnitScale &&
       mOffsetMapUnitScale == mCustomDashPatternMapUnitScale )
  {
    return mWidthMapUnitScale;
  }
  return QgsMapUnitScale();
}

QgsSymbolLayer *QgsSimpleLineSymbolLayer::create( const QVariantMap &props )
{
  QColor color = DEFAULT_SIMPLELINE_COLOR;
  double width = DEFAULT_SIMPLELINE_WIDTH;
  Qt::PenStyle penStyle = DEFAULT_SIMPLELINE_PENSTYLE;

  if ( props.contains( QStringLiteral( "line_color" ) ) )
  {
    color = QgsSymbolLayerUtils::decodeColor( props[QStringLiteral( "line_color" )].toString() );
  }
  else if ( props.contains( QStringLiteral( "outline_color" ) ) )
  {
    color = QgsSymbolLayerUtils::decodeColor( props[QStringLiteral( "outline_color" )].toString() );
  }
  else if ( props.contains( QStringLiteral( "color" ) ) )
  {
    //pre 2.5 projects used "color"
    color = QgsSymbolLayerUtils::decodeColor( props[QStringLiteral( "color" )].toString() );
  }
  if ( props.contains( QStringLiteral( "line_width" ) ) )
  {
    width = props[QStringLiteral( "line_width" )].toDouble();
  }
  else if ( props.contains( QStringLiteral( "outline_width" ) ) )
  {
    width = props[QStringLiteral( "outline_width" )].toDouble();
  }
  else if ( props.contains( QStringLiteral( "width" ) ) )
  {
    //pre 2.5 projects used "width"
    width = props[QStringLiteral( "width" )].toDouble();
  }
  if ( props.contains( QStringLiteral( "line_style" ) ) )
  {
    penStyle = QgsSymbolLayerUtils::decodePenStyle( props[QStringLiteral( "line_style" )].toString() );
  }
  else if ( props.contains( QStringLiteral( "outline_style" ) ) )
  {
    penStyle = QgsSymbolLayerUtils::decodePenStyle( props[QStringLiteral( "outline_style" )].toString() );
  }
  else if ( props.contains( QStringLiteral( "penstyle" ) ) )
  {
    penStyle = QgsSymbolLayerUtils::decodePenStyle( props[QStringLiteral( "penstyle" )].toString() );
  }

  QgsSimpleLineSymbolLayer *l = new QgsSimpleLineSymbolLayer( color, width, penStyle );
  if ( props.contains( QStringLiteral( "line_width_unit" ) ) )
  {
    l->setWidthUnit( QgsUnitTypes::decodeRenderUnit( props[QStringLiteral( "line_width_unit" )].toString() ) );
  }
  else if ( props.contains( QStringLiteral( "outline_width_unit" ) ) )
  {
    l->setWidthUnit( QgsUnitTypes::decodeRenderUnit( props[QStringLiteral( "outline_width_unit" )].toString() ) );
  }
  else if ( props.contains( QStringLiteral( "width_unit" ) ) )
  {
    //pre 2.5 projects used "width_unit"
    l->setWidthUnit( QgsUnitTypes::decodeRenderUnit( props[QStringLiteral( "width_unit" )].toString() ) );
  }
  if ( props.contains( QStringLiteral( "width_map_unit_scale" ) ) )
    l->setWidthMapUnitScale( QgsSymbolLayerUtils::decodeMapUnitScale( props[QStringLiteral( "width_map_unit_scale" )].toString() ) );
  if ( props.contains( QStringLiteral( "offset" ) ) )
    l->setOffset( props[QStringLiteral( "offset" )].toDouble() );
  if ( props.contains( QStringLiteral( "offset_unit" ) ) )
    l->setOffsetUnit( QgsUnitTypes::decodeRenderUnit( props[QStringLiteral( "offset_unit" )].toString() ) );
  if ( props.contains( QStringLiteral( "offset_map_unit_scale" ) ) )
    l->setOffsetMapUnitScale( QgsSymbolLayerUtils::decodeMapUnitScale( props[QStringLiteral( "offset_map_unit_scale" )].toString() ) );
  if ( props.contains( QStringLiteral( "joinstyle" ) ) )
    l->setPenJoinStyle( QgsSymbolLayerUtils::decodePenJoinStyle( props[QStringLiteral( "joinstyle" )].toString() ) );
  if ( props.contains( QStringLiteral( "capstyle" ) ) )
    l->setPenCapStyle( QgsSymbolLayerUtils::decodePenCapStyle( props[QStringLiteral( "capstyle" )].toString() ) );

  if ( props.contains( QStringLiteral( "use_custom_dash" ) ) )
  {
    l->setUseCustomDashPattern( props[QStringLiteral( "use_custom_dash" )].toInt() );
  }
  if ( props.contains( QStringLiteral( "customdash" ) ) )
  {
    l->setCustomDashVector( QgsSymbolLayerUtils::decodeRealVector( props[QStringLiteral( "customdash" )].toString() ) );
  }
  if ( props.contains( QStringLiteral( "customdash_unit" ) ) )
  {
    l->setCustomDashPatternUnit( QgsUnitTypes::decodeRenderUnit( props[QStringLiteral( "customdash_unit" )].toString() ) );
  }
  if ( props.contains( QStringLiteral( "customdash_map_unit_scale" ) ) )
  {
    l->setCustomDashPatternMapUnitScale( QgsSymbolLayerUtils::decodeMapUnitScale( props[QStringLiteral( "customdash_map_unit_scale" )].toString() ) );
  }

  if ( props.contains( QStringLiteral( "draw_inside_polygon" ) ) )
  {
    l->setDrawInsidePolygon( props[QStringLiteral( "draw_inside_polygon" )].toInt() );
  }

  if ( props.contains( QStringLiteral( "ring_filter" ) ) )
  {
    l->setRingFilter( static_cast< RenderRingFilter>( props[QStringLiteral( "ring_filter" )].toInt() ) );
  }

  if ( props.contains( QStringLiteral( "dash_pattern_offset" ) ) )
    l->setDashPatternOffset( props[QStringLiteral( "dash_pattern_offset" )].toDouble() );
  if ( props.contains( QStringLiteral( "dash_pattern_offset_unit" ) ) )
    l->setDashPatternOffsetUnit( QgsUnitTypes::decodeRenderUnit( props[QStringLiteral( "dash_pattern_offset_unit" )].toString() ) );
  if ( props.contains( QStringLiteral( "dash_pattern_offset_map_unit_scale" ) ) )
    l->setDashPatternOffsetMapUnitScale( QgsSymbolLayerUtils::decodeMapUnitScale( props[QStringLiteral( "dash_pattern_offset_map_unit_scale" )].toString() ) );

  if ( props.contains( QStringLiteral( "trim_distance_start" ) ) )
    l->setTrimDistanceStart( props[QStringLiteral( "trim_distance_start" )].toDouble() );
  if ( props.contains( QStringLiteral( "trim_distance_start_unit" ) ) )
    l->setTrimDistanceStartUnit( QgsUnitTypes::decodeRenderUnit( props[QStringLiteral( "trim_distance_start_unit" )].toString() ) );
  if ( props.contains( QStringLiteral( "trim_distance_start_map_unit_scale" ) ) )
    l->setTrimDistanceStartMapUnitScale( QgsSymbolLayerUtils::decodeMapUnitScale( props[QStringLiteral( "trim_distance_start_map_unit_scale" )].toString() ) );
  if ( props.contains( QStringLiteral( "trim_distance_end" ) ) )
    l->setTrimDistanceEnd( props[QStringLiteral( "trim_distance_end" )].toDouble() );
  if ( props.contains( QStringLiteral( "trim_distance_end_unit" ) ) )
    l->setTrimDistanceEndUnit( QgsUnitTypes::decodeRenderUnit( props[QStringLiteral( "trim_distance_end_unit" )].toString() ) );
  if ( props.contains( QStringLiteral( "trim_distance_end_map_unit_scale" ) ) )
    l->setTrimDistanceEndMapUnitScale( QgsSymbolLayerUtils::decodeMapUnitScale( props[QStringLiteral( "trim_distance_end_map_unit_scale" )].toString() ) );

  if ( props.contains( QStringLiteral( "align_dash_pattern" ) ) )
    l->setAlignDashPattern( props[ QStringLiteral( "align_dash_pattern" )].toInt() );

  if ( props.contains( QStringLiteral( "tweak_dash_pattern_on_corners" ) ) )
    l->setTweakDashPatternOnCorners( props[ QStringLiteral( "tweak_dash_pattern_on_corners" )].toInt() );

  l->restoreOldDataDefinedProperties( props );

  return l;
}

QString QgsSimpleLineSymbolLayer::layerType() const
{
  return QStringLiteral( "SimpleLine" );
}

void QgsSimpleLineSymbolLayer::startRender( QgsSymbolRenderContext &context )
{
  QColor penColor = mColor;
  penColor.setAlphaF( mColor.alphaF() * context.opacity() );
  mPen.setColor( penColor );
  double scaledWidth = context.renderContext().convertToPainterUnits( mWidth, mWidthUnit, mWidthMapUnitScale );
  mPen.setWidthF( scaledWidth );

  //note that Qt seems to have issues with scaling dash patterns with very small pen widths.
  //treating the pen as having no less than a 1 pixel size avoids the worst of these issues
  const double dashWidthDiv = std::max( 1.0, scaledWidth );
  if ( mUseCustomDashPattern )
  {
    mPen.setStyle( Qt::CustomDashLine );

    //scale pattern vector

    QVector<qreal> scaledVector;
    QVector<qreal>::const_iterator it = mCustomDashVector.constBegin();
    for ( ; it != mCustomDashVector.constEnd(); ++it )
    {
      //the dash is specified in terms of pen widths, therefore the division
      scaledVector << context.renderContext().convertToPainterUnits( ( *it ), mCustomDashPatternUnit, mCustomDashPatternMapUnitScale ) / dashWidthDiv;
    }
    mPen.setDashPattern( scaledVector );
  }
  else
  {
    mPen.setStyle( mPenStyle );
  }

  if ( mDashPatternOffset && mPen.style() != Qt::SolidLine )
  {
    mPen.setDashOffset( context.renderContext().convertToPainterUnits( mDashPatternOffset, mDashPatternOffsetUnit, mDashPatternOffsetMapUnitScale ) / dashWidthDiv ) ;
  }

  mPen.setJoinStyle( mPenJoinStyle );
  mPen.setCapStyle( mPenCapStyle );

  mSelPen = mPen;
  QColor selColor = context.renderContext().selectionColor();
  if ( ! SELECTION_IS_OPAQUE )
    selColor.setAlphaF( context.opacity() );
  mSelPen.setColor( selColor );
}

void QgsSimpleLineSymbolLayer::stopRender( QgsSymbolRenderContext &context )
{
  Q_UNUSED( context )
}

void QgsSimpleLineSymbolLayer::renderPolygonStroke( const QPolygonF &points, const QVector<QPolygonF> *rings, QgsSymbolRenderContext &context )
{
  QPainter *p = context.renderContext().painter();
  if ( !p )
  {
    return;
  }

  QgsExpressionContextScope *scope = nullptr;
  std::unique_ptr< QgsExpressionContextScopePopper > scopePopper;
  if ( hasDataDefinedProperties() )
  {
    scope = new QgsExpressionContextScope();
    scopePopper = std::make_unique< QgsExpressionContextScopePopper >( context.renderContext().expressionContext(), scope );
  }

  if ( mDrawInsidePolygon )
    p->save();

  switch ( mRingFilter )
  {
    case AllRings:
    case ExteriorRingOnly:
    {
      if ( mDrawInsidePolygon )
      {
        //only drawing the line on the interior of the polygon, so set clip path for painter
        QPainterPath clipPath;
        clipPath.addPolygon( points );

        if ( rings )
        {
          //add polygon rings
          for ( auto it = rings->constBegin(); it != rings->constEnd(); ++it )
          {
            QPolygonF ring = *it;
            clipPath.addPolygon( ring );
          }
        }

        //use intersect mode, as a clip path may already exist (e.g., for composer maps)
        p->setClipPath( clipPath, Qt::IntersectClip );
      }

      if ( scope )
        scope->addVariable( QgsExpressionContextScope::StaticVariable( QgsExpressionContext::EXPR_GEOMETRY_RING_NUM, 0, true ) );

      renderPolyline( points, context );
    }
    break;

    case InteriorRingsOnly:
      break;
  }

  if ( rings )
  {
    switch ( mRingFilter )
    {
      case AllRings:
      case InteriorRingsOnly:
      {
        mOffset = -mOffset; // invert the offset for rings!
        int ringIndex = 1;
        for ( const QPolygonF &ring : std::as_const( *rings ) )
        {
          if ( scope )
            scope->addVariable( QgsExpressionContextScope::StaticVariable( QgsExpressionContext::EXPR_GEOMETRY_RING_NUM, ringIndex, true ) );

          renderPolyline( ring, context );
          ringIndex++;
        }
        mOffset = -mOffset;
      }
      break;
      case ExteriorRingOnly:
        break;
    }
  }

  if ( mDrawInsidePolygon )
  {
    //restore painter to reset clip path
    p->restore();
  }

}

void QgsSimpleLineSymbolLayer::renderPolyline( const QPolygonF &pts, QgsSymbolRenderContext &context )
{
  QPainter *p = context.renderContext().painter();
  if ( !p )
  {
    return;
  }

  QPolygonF points = pts;

  double startTrim = mTrimDistanceStart;
  if ( mDataDefinedProperties.isActive( QgsSymbolLayer::PropertyTrimStart ) )
  {
    context.setOriginalValueVariable( startTrim );
    startTrim = mDataDefinedProperties.valueAsDouble( QgsSymbolLayer::PropertyTrimStart, context.renderContext().expressionContext(), mTrimDistanceStart );
  }
  double endTrim = mTrimDistanceEnd;
  if ( mDataDefinedProperties.isActive( QgsSymbolLayer::PropertyTrimEnd ) )
  {
    context.setOriginalValueVariable( endTrim );
    endTrim = mDataDefinedProperties.valueAsDouble( QgsSymbolLayer::PropertyTrimEnd, context.renderContext().expressionContext(), mTrimDistanceEnd );
  }

  double totalLength = -1;
  if ( mTrimDistanceStartUnit == QgsUnitTypes::RenderPercentage )
  {
    totalLength = QgsSymbolLayerUtils::polylineLength( points );
    startTrim = startTrim * 0.01 * totalLength;
  }
  else
  {
    startTrim = context.renderContext().convertToPainterUnits( startTrim, mTrimDistanceStartUnit, mTrimDistanceStartMapUnitScale );
  }
  if ( mTrimDistanceEndUnit == QgsUnitTypes::RenderPercentage )
  {
    if ( totalLength < 0 ) // only recalculate if we didn't already work this out for the start distance!
      totalLength = QgsSymbolLayerUtils::polylineLength( points );
    endTrim = endTrim * 0.01 * totalLength;
  }
  else
  {
    endTrim = context.renderContext().convertToPainterUnits( endTrim, mTrimDistanceEndUnit, mTrimDistanceEndMapUnitScale );
  }
  if ( !qgsDoubleNear( startTrim, 0 ) || !qgsDoubleNear( endTrim, 0 ) )
  {
    points = QgsSymbolLayerUtils::polylineSubstring( points, startTrim, -endTrim );
  }

  QColor penColor = mColor;
  penColor.setAlphaF( mColor.alphaF() * context.opacity() );
  mPen.setColor( penColor );

  double offset = mOffset;
  applyDataDefinedSymbology( context, mPen, mSelPen, offset );

  const QPen pen = context.selected() ? mSelPen : mPen;

  if ( !pen.dashPattern().isEmpty() )
  {
    // check for a null (all 0) dash component, and shortcut out early if so -- these lines are rendered as "no pen"
    const QVector<double> pattern = pen.dashPattern();
    bool foundNonNull = false;
    for ( int i = 0; i < pattern.size(); ++i )
    {
      if ( i % 2 == 0  && !qgsDoubleNear( pattern[i], 0 ) )
      {
        foundNonNull = true;
        break;
      }
    }
    if ( !foundNonNull )
      return;
  }

  p->setBrush( Qt::NoBrush );

  // Disable 'Antialiasing' if the geometry was generalized in the current RenderContext (We known that it must have least #2 points).
  std::unique_ptr< QgsScopedQPainterState > painterState;
  if ( points.size() <= 2 &&
       ( context.renderContext().vectorSimplifyMethod().simplifyHints() & QgsVectorSimplifyMethod::AntialiasingSimplification ) &&
       QgsAbstractGeometrySimplifier::isGeneralizableByDeviceBoundingBox( points, context.renderContext().vectorSimplifyMethod().threshold() ) &&
       ( p->renderHints() & QPainter::Antialiasing ) )
  {
    painterState = std::make_unique< QgsScopedQPainterState >( p );
    p->setRenderHint( QPainter::Antialiasing, false );
  }

  const bool applyPatternTweaks = mAlignDashPattern
                                  && ( pen.style() != Qt::SolidLine || !pen.dashPattern().empty() )
                                  && pen.dashOffset() == 0;

  if ( qgsDoubleNear( offset, 0 ) )
  {
    if ( applyPatternTweaks )
    {
      drawPathWithDashPatternTweaks( p, points, pen );
    }
    else
    {
      p->setPen( pen );
      QPainterPath path;
      path.addPolygon( points );
      p->drawPath( path );
    }
  }
  else
  {
    double scaledOffset = context.renderContext().convertToPainterUnits( offset, mOffsetUnit, mOffsetMapUnitScale );
    if ( mOffsetUnit == QgsUnitTypes::RenderMetersInMapUnits && context.renderContext().flags() & Qgis::RenderContextFlag::RenderSymbolPreview )
    {
      // rendering for symbol previews -- a size in meters in map units can't be calculated, so treat the size as millimeters
      // and clamp it to a reasonable range. It's the best we can do in this situation!
      scaledOffset = std::min( std::max( context.renderContext().convertToPainterUnits( offset, QgsUnitTypes::RenderMillimeters ), 3.0 ), 100.0 );
    }

    QList<QPolygonF> mline = ::offsetLine( points, scaledOffset, context.originalGeometryType() != QgsWkbTypes::UnknownGeometry ? context.originalGeometryType() : QgsWkbTypes::LineGeometry );
    for ( const QPolygonF &part : mline )
    {
      if ( applyPatternTweaks )
      {
        drawPathWithDashPatternTweaks( p, part, pen );
      }
      else
      {
        p->setPen( pen );
        QPainterPath path;
        path.addPolygon( part );
        p->drawPath( path );
      }
    }
  }
}

QVariantMap QgsSimpleLineSymbolLayer::properties() const
{
  QVariantMap map;
  map[QStringLiteral( "line_color" )] = QgsSymbolLayerUtils::encodeColor( mColor );
  map[QStringLiteral( "line_width" )] = QString::number( mWidth );
  map[QStringLiteral( "line_width_unit" )] = QgsUnitTypes::encodeUnit( mWidthUnit );
  map[QStringLiteral( "width_map_unit_scale" )] = QgsSymbolLayerUtils::encodeMapUnitScale( mWidthMapUnitScale );
  map[QStringLiteral( "line_style" )] = QgsSymbolLayerUtils::encodePenStyle( mPenStyle );
  map[QStringLiteral( "joinstyle" )] = QgsSymbolLayerUtils::encodePenJoinStyle( mPenJoinStyle );
  map[QStringLiteral( "capstyle" )] = QgsSymbolLayerUtils::encodePenCapStyle( mPenCapStyle );
  map[QStringLiteral( "offset" )] = QString::number( mOffset );
  map[QStringLiteral( "offset_unit" )] = QgsUnitTypes::encodeUnit( mOffsetUnit );
  map[QStringLiteral( "offset_map_unit_scale" )] = QgsSymbolLayerUtils::encodeMapUnitScale( mOffsetMapUnitScale );
  map[QStringLiteral( "use_custom_dash" )] = ( mUseCustomDashPattern ? QStringLiteral( "1" ) : QStringLiteral( "0" ) );
  map[QStringLiteral( "customdash" )] = QgsSymbolLayerUtils::encodeRealVector( mCustomDashVector );
  map[QStringLiteral( "customdash_unit" )] = QgsUnitTypes::encodeUnit( mCustomDashPatternUnit );
  map[QStringLiteral( "customdash_map_unit_scale" )] = QgsSymbolLayerUtils::encodeMapUnitScale( mCustomDashPatternMapUnitScale );
  map[QStringLiteral( "dash_pattern_offset" )] = QString::number( mDashPatternOffset );
  map[QStringLiteral( "dash_pattern_offset_unit" )] = QgsUnitTypes::encodeUnit( mDashPatternOffsetUnit );
  map[QStringLiteral( "dash_pattern_offset_map_unit_scale" )] = QgsSymbolLayerUtils::encodeMapUnitScale( mDashPatternOffsetMapUnitScale );
  map[QStringLiteral( "trim_distance_start" )] = QString::number( mTrimDistanceStart );
  map[QStringLiteral( "trim_distance_start_unit" )] = QgsUnitTypes::encodeUnit( mTrimDistanceStartUnit );
  map[QStringLiteral( "trim_distance_start_map_unit_scale" )] = QgsSymbolLayerUtils::encodeMapUnitScale( mTrimDistanceStartMapUnitScale );
  map[QStringLiteral( "trim_distance_end" )] = QString::number( mTrimDistanceEnd );
  map[QStringLiteral( "trim_distance_end_unit" )] = QgsUnitTypes::encodeUnit( mTrimDistanceEndUnit );
  map[QStringLiteral( "trim_distance_end_map_unit_scale" )] = QgsSymbolLayerUtils::encodeMapUnitScale( mTrimDistanceEndMapUnitScale );
  map[QStringLiteral( "draw_inside_polygon" )] = ( mDrawInsidePolygon ? QStringLiteral( "1" ) : QStringLiteral( "0" ) );
  map[QStringLiteral( "ring_filter" )] = QString::number( static_cast< int >( mRingFilter ) );
  map[QStringLiteral( "align_dash_pattern" )] = mAlignDashPattern ? QStringLiteral( "1" ) : QStringLiteral( "0" );
  map[QStringLiteral( "tweak_dash_pattern_on_corners" )] = mPatternCartographicTweakOnSharpCorners ? QStringLiteral( "1" ) : QStringLiteral( "0" );
  return map;
}

QgsSimpleLineSymbolLayer *QgsSimpleLineSymbolLayer::clone() const
{
  QgsSimpleLineSymbolLayer *l = new QgsSimpleLineSymbolLayer( mColor, mWidth, mPenStyle );
  l->setWidthUnit( mWidthUnit );
  l->setWidthMapUnitScale( mWidthMapUnitScale );
  l->setOffsetUnit( mOffsetUnit );
  l->setOffsetMapUnitScale( mOffsetMapUnitScale );
  l->setCustomDashPatternUnit( mCustomDashPatternUnit );
  l->setCustomDashPatternMapUnitScale( mCustomDashPatternMapUnitScale );
  l->setOffset( mOffset );
  l->setPenJoinStyle( mPenJoinStyle );
  l->setPenCapStyle( mPenCapStyle );
  l->setUseCustomDashPattern( mUseCustomDashPattern );
  l->setCustomDashVector( mCustomDashVector );
  l->setDrawInsidePolygon( mDrawInsidePolygon );
  l->setRingFilter( mRingFilter );
  l->setDashPatternOffset( mDashPatternOffset );
  l->setDashPatternOffsetUnit( mDashPatternOffsetUnit );
  l->setDashPatternOffsetMapUnitScale( mDashPatternOffsetMapUnitScale );
  l->setTrimDistanceStart( mTrimDistanceStart );
  l->setTrimDistanceStartUnit( mTrimDistanceStartUnit );
  l->setTrimDistanceStartMapUnitScale( mTrimDistanceStartMapUnitScale );
  l->setTrimDistanceEnd( mTrimDistanceEnd );
  l->setTrimDistanceEndUnit( mTrimDistanceEndUnit );
  l->setTrimDistanceEndMapUnitScale( mTrimDistanceEndMapUnitScale );
  l->setAlignDashPattern( mAlignDashPattern );
  l->setTweakDashPatternOnCorners( mPatternCartographicTweakOnSharpCorners );

  copyDataDefinedProperties( l );
  copyPaintEffect( l );
  return l;
}

void QgsSimpleLineSymbolLayer::toSld( QDomDocument &doc, QDomElement &element, const QVariantMap &props ) const
{
  if ( mPenStyle == Qt::NoPen )
    return;

  QDomElement symbolizerElem = doc.createElement( QStringLiteral( "se:LineSymbolizer" ) );
  if ( !props.value( QStringLiteral( "uom" ), QString() ).toString().isEmpty() )
    symbolizerElem.setAttribute( QStringLiteral( "uom" ), props.value( QStringLiteral( "uom" ), QString() ).toString() );
  element.appendChild( symbolizerElem );

  // <Geometry>
  QgsSymbolLayerUtils::createGeometryElement( doc, symbolizerElem, props.value( QStringLiteral( "geom" ), QString() ).toString() );

  // <Stroke>
  QDomElement strokeElem = doc.createElement( QStringLiteral( "se:Stroke" ) );
  symbolizerElem.appendChild( strokeElem );

  Qt::PenStyle penStyle = mUseCustomDashPattern ? Qt::CustomDashLine : mPenStyle;
  double width = QgsSymbolLayerUtils::rescaleUom( mWidth, mWidthUnit, props );
  QVector<qreal> customDashVector = QgsSymbolLayerUtils::rescaleUom( mCustomDashVector, mCustomDashPatternUnit, props );
  QgsSymbolLayerUtils::lineToSld( doc, strokeElem, penStyle, mColor, width,
                                  &mPenJoinStyle, &mPenCapStyle, &customDashVector );

  // <se:PerpendicularOffset>
  if ( !qgsDoubleNear( mOffset, 0.0 ) )
  {
    QDomElement perpOffsetElem = doc.createElement( QStringLiteral( "se:PerpendicularOffset" ) );
    double offset = QgsSymbolLayerUtils::rescaleUom( mOffset, mOffsetUnit, props );
    perpOffsetElem.appendChild( doc.createTextNode( qgsDoubleToString( offset ) ) );
    symbolizerElem.appendChild( perpOffsetElem );
  }
}

QString QgsSimpleLineSymbolLayer::ogrFeatureStyle( double mmScaleFactor, double mapUnitScaleFactor ) const
{
  if ( mUseCustomDashPattern )
  {
    return QgsSymbolLayerUtils::ogrFeatureStylePen( mWidth, mmScaleFactor, mapUnitScaleFactor,
           mPen.color(), mPenJoinStyle,
           mPenCapStyle, mOffset, &mCustomDashVector );
  }
  else
  {
    return QgsSymbolLayerUtils::ogrFeatureStylePen( mWidth, mmScaleFactor, mapUnitScaleFactor, mPen.color(), mPenJoinStyle,
           mPenCapStyle, mOffset );
  }
}

QgsSymbolLayer *QgsSimpleLineSymbolLayer::createFromSld( QDomElement &element )
{
  QgsDebugMsgLevel( QStringLiteral( "Entered." ), 4 );

  QDomElement strokeElem = element.firstChildElement( QStringLiteral( "Stroke" ) );
  if ( strokeElem.isNull() )
    return nullptr;

  Qt::PenStyle penStyle;
  QColor color;
  double width;
  Qt::PenJoinStyle penJoinStyle;
  Qt::PenCapStyle penCapStyle;
  QVector<qreal> customDashVector;

  if ( !QgsSymbolLayerUtils::lineFromSld( strokeElem, penStyle,
                                          color, width,
                                          &penJoinStyle, &penCapStyle,
                                          &customDashVector ) )
    return nullptr;

  double offset = 0.0;
  QDomElement perpOffsetElem = element.firstChildElement( QStringLiteral( "PerpendicularOffset" ) );
  if ( !perpOffsetElem.isNull() )
  {
    bool ok;
    double d = perpOffsetElem.firstChild().nodeValue().toDouble( &ok );
    if ( ok )
      offset = d;
  }

  QString uom = element.attribute( QStringLiteral( "uom" ) );
  width = QgsSymbolLayerUtils::sizeInPixelsFromSldUom( uom, width );
  offset = QgsSymbolLayerUtils::sizeInPixelsFromSldUom( uom, offset );

  QgsSimpleLineSymbolLayer *l = new QgsSimpleLineSymbolLayer( color, width, penStyle );
  l->setOutputUnit( QgsUnitTypes::RenderUnit::RenderPixels );
  l->setOffset( offset );
  l->setPenJoinStyle( penJoinStyle );
  l->setPenCapStyle( penCapStyle );
  l->setUseCustomDashPattern( penStyle == Qt::CustomDashLine );
  l->setCustomDashVector( customDashVector );
  return l;
}

void QgsSimpleLineSymbolLayer::applyDataDefinedSymbology( QgsSymbolRenderContext &context, QPen &pen, QPen &selPen, double &offset )
{
  if ( !dataDefinedProperties().hasActiveProperties() )
    return; // shortcut

  //data defined properties
  bool hasStrokeWidthExpression = false;
  if ( mDataDefinedProperties.isActive( QgsSymbolLayer::PropertyStrokeWidth ) )
  {
    context.setOriginalValueVariable( mWidth );
    double scaledWidth = context.renderContext().convertToPainterUnits(
                           mDataDefinedProperties.valueAsDouble( QgsSymbolLayer::PropertyStrokeWidth, context.renderContext().expressionContext(), mWidth ),
                           mWidthUnit, mWidthMapUnitScale );
    pen.setWidthF( scaledWidth );
    selPen.setWidthF( scaledWidth );
    hasStrokeWidthExpression = true;
  }

  //color
  if ( mDataDefinedProperties.isActive( QgsSymbolLayer::PropertyStrokeColor ) )
  {
    context.setOriginalValueVariable( QgsSymbolLayerUtils::encodeColor( mColor ) );

    QColor penColor = mDataDefinedProperties.valueAsColor( QgsSymbolLayer::PropertyStrokeColor, context.renderContext().expressionContext(), mColor );
    penColor.setAlphaF( context.opacity() * penColor.alphaF() );
    pen.setColor( penColor );
  }

  //offset
  if ( mDataDefinedProperties.isActive( QgsSymbolLayer::PropertyOffset ) )
  {
    context.setOriginalValueVariable( mOffset );
    offset = mDataDefinedProperties.valueAsDouble( QgsSymbolLayer::PropertyOffset, context.renderContext().expressionContext(), offset );
  }

  //dash dot vector

  //note that Qt seems to have issues with scaling dash patterns with very small pen widths.
  //treating the pen as having no less than a 1 pixel size avoids the worst of these issues
  const double dashWidthDiv = std::max( hasStrokeWidthExpression ? pen.widthF() : mPen.widthF(), 1.0 );

  if ( mDataDefinedProperties.isActive( QgsSymbolLayer::PropertyCustomDash ) )
  {
    QVector<qreal> dashVector;
    QVariant exprVal = mDataDefinedProperties.value( QgsSymbolLayer::PropertyCustomDash, context.renderContext().expressionContext() );
    if ( !exprVal.isNull() )
    {
      QStringList dashList = exprVal.toString().split( ';' );
      QStringList::const_iterator dashIt = dashList.constBegin();
      for ( ; dashIt != dashList.constEnd(); ++dashIt )
      {
        dashVector.push_back( context.renderContext().convertToPainterUnits( dashIt->toDouble(), mCustomDashPatternUnit, mCustomDashPatternMapUnitScale ) / dashWidthDiv );
      }
      pen.setDashPattern( dashVector );
    }
  }
  else if ( mDataDefinedProperties.isActive( QgsSymbolLayer::PropertyStrokeWidth ) && mUseCustomDashPattern )
  {
    //re-scale pattern vector after data defined pen width was applied

    QVector<qreal> scaledVector;
    for ( double v : std::as_const( mCustomDashVector ) )
    {
      //the dash is specified in terms of pen widths, therefore the division
      scaledVector << context.renderContext().convertToPainterUnits( v, mCustomDashPatternUnit, mCustomDashPatternMapUnitScale ) / dashWidthDiv;
    }
    mPen.setDashPattern( scaledVector );
  }

  // dash pattern offset
  double patternOffset = mDashPatternOffset;
  if ( mDataDefinedProperties.isActive( QgsSymbolLayer::PropertyDashPatternOffset ) && pen.style() != Qt::SolidLine )
  {
    context.setOriginalValueVariable( patternOffset );
    patternOffset = mDataDefinedProperties.valueAsDouble( QgsSymbolLayer::PropertyDashPatternOffset, context.renderContext().expressionContext(), mDashPatternOffset );
    pen.setDashOffset( context.renderContext().convertToPainterUnits( patternOffset, mDashPatternOffsetUnit, mDashPatternOffsetMapUnitScale ) / dashWidthDiv );
  }

  //line style
  if ( mDataDefinedProperties.isActive( QgsSymbolLayer::PropertyStrokeStyle ) )
  {
    context.setOriginalValueVariable( QgsSymbolLayerUtils::encodePenStyle( mPenStyle ) );
    QVariant exprVal = mDataDefinedProperties.value( QgsSymbolLayer::PropertyStrokeStyle, context.renderContext().expressionContext() );
    if ( !exprVal.isNull() )
      pen.setStyle( QgsSymbolLayerUtils::decodePenStyle( exprVal.toString() ) );
  }

  //join style
  if ( mDataDefinedProperties.isActive( QgsSymbolLayer::PropertyJoinStyle ) )
  {
    context.setOriginalValueVariable( QgsSymbolLayerUtils::encodePenJoinStyle( mPenJoinStyle ) );
    QVariant exprVal = mDataDefinedProperties.value( QgsSymbolLayer::PropertyJoinStyle, context.renderContext().expressionContext() );
    if ( !exprVal.isNull() )
      pen.setJoinStyle( QgsSymbolLayerUtils::decodePenJoinStyle( exprVal.toString() ) );
  }

  //cap style
  if ( mDataDefinedProperties.isActive( QgsSymbolLayer::PropertyCapStyle ) )
  {
    context.setOriginalValueVariable( QgsSymbolLayerUtils::encodePenCapStyle( mPenCapStyle ) );
    QVariant exprVal = mDataDefinedProperties.value( QgsSymbolLayer::PropertyCapStyle, context.renderContext().expressionContext() );
    if ( !exprVal.isNull() )
      pen.setCapStyle( QgsSymbolLayerUtils::decodePenCapStyle( exprVal.toString() ) );
  }
}

void QgsSimpleLineSymbolLayer::drawPathWithDashPatternTweaks( QPainter *painter, const QPolygonF &points, QPen pen ) const
{
  if ( pen.dashPattern().empty() || points.size() < 2 )
    return;

  QVector< qreal > sourcePattern = pen.dashPattern();
  const double dashWidthDiv = std::max( 1.0001, pen.widthF() );
  // back to painter units
  for ( int i = 0; i < sourcePattern.size(); ++ i )
    sourcePattern[i] *= pen.widthF();

  if ( pen.widthF() <= 1.0 )
    pen.setWidthF( 1.0001 );

  QVector< qreal > buffer;
  QPolygonF bufferedPoints;
  QPolygonF previousSegmentBuffer;
  // we iterate through the line points, building a custom dash pattern and adding it to the buffer
  // as soon as we hit a sharp bend, we scale the buffered pattern in order to nicely place a dash component over the bend
  // and then append the buffer to the output pattern.

  auto ptIt = points.constBegin();
  double totalBufferLength = 0;
  int patternIndex = 0;
  double currentRemainingDashLength = 0;
  double currentRemainingGapLength = 0;

  auto compressPattern = []( const QVector< qreal > &buffer ) -> QVector< qreal >
  {
    QVector< qreal > result;
    result.reserve( buffer.size() );
    for ( auto it = buffer.begin(); it != buffer.end(); )
    {
      qreal dash = *it++;
      qreal gap = *it++;
      while ( dash == 0 && !result.empty() )
      {
        result.last() += gap;

        if ( it == buffer.end() )
          return result;
        dash = *it++;
        gap = *it++;
      }
      while ( gap == 0 && it != buffer.end() )
      {
        dash += *it++;
        gap = *it++;
      }
      result << dash << gap;
    }
    return result;
  };

  double currentBufferLineLength = 0;
  auto flushBuffer = [pen, painter, &buffer, &bufferedPoints, &previousSegmentBuffer, &currentRemainingDashLength, &currentRemainingGapLength,  &currentBufferLineLength, &totalBufferLength,
                           dashWidthDiv, &compressPattern]( QPointF * nextPoint )
  {
    if ( buffer.empty() || bufferedPoints.size() < 2 )
    {
      return;
    }

    if ( currentRemainingDashLength )
    {
      // ended midway through a dash -- we want to finish this off
      buffer << currentRemainingDashLength << 0.0;
      totalBufferLength += currentRemainingDashLength;
    }
    QVector< qreal > compressed = compressPattern( buffer );
    if ( !currentRemainingDashLength )
    {
      // ended midway through a gap -- we don't want this, we want to end at previous dash
      totalBufferLength -= compressed.last();
      compressed.last() = 0;
    }

    // rescale buffer for final bit of line -- we want to end at the end of a dash, not a gap
    const double scaleFactor = currentBufferLineLength / totalBufferLength;

    bool shouldFlushPreviousSegmentBuffer = false;

    if ( !previousSegmentBuffer.empty() )
    {
      // add first dash from current buffer
      QPolygonF firstDashSubstring = QgsSymbolLayerUtils::polylineSubstring( bufferedPoints, 0, compressed.first() * scaleFactor );
      if ( !firstDashSubstring.empty() )
        QgsSymbolLayerUtils::appendPolyline( previousSegmentBuffer, firstDashSubstring );

      // then we skip over the first dash and gap for this segment
      bufferedPoints = QgsSymbolLayerUtils::polylineSubstring( bufferedPoints, ( compressed.first() + compressed.at( 1 ) ) * scaleFactor, 0 );

      compressed = compressed.mid( 2 );
      shouldFlushPreviousSegmentBuffer = !compressed.empty();
    }

    if ( !previousSegmentBuffer.empty() && ( shouldFlushPreviousSegmentBuffer || !nextPoint ) )
    {
      QPen adjustedPen = pen;
      adjustedPen.setStyle( Qt::SolidLine );
      painter->setPen( adjustedPen );
      QPainterPath path;
      path.addPolygon( previousSegmentBuffer );
      painter->drawPath( path );
      previousSegmentBuffer.clear();
    }

    double finalDash = 0;
    if ( nextPoint )
    {
      // sharp bend:
      // 1. rewind buffered points line by final dash and gap length
      // (later) 2. draw the bend with a solid line of length 2 * final dash size

      if ( !compressed.empty() )
      {
        finalDash = compressed.at( compressed.size() - 2 );
        const double finalGap = compressed.size() > 2 ? compressed.at( compressed.size() - 3 ) : 0;

        const QPolygonF thisPoints = bufferedPoints;
        bufferedPoints = QgsSymbolLayerUtils::polylineSubstring( thisPoints, 0, -( finalDash + finalGap ) * scaleFactor );
        previousSegmentBuffer = QgsSymbolLayerUtils::polylineSubstring( thisPoints, - finalDash * scaleFactor, 0 );
      }
      else
      {
        previousSegmentBuffer << bufferedPoints;
      }
    }

    currentBufferLineLength = 0;
    currentRemainingDashLength = 0;
    currentRemainingGapLength = 0;
    totalBufferLength = 0;
    buffer.clear();

    if ( !bufferedPoints.empty() && ( !compressed.empty() || !nextPoint ) )
    {
      QPen adjustedPen = pen;
      if ( !compressed.empty() )
      {
        // maximum size of dash pattern is 32 elements
        compressed = compressed.mid( 0, 32 );
        std::for_each( compressed.begin(), compressed.end(), [scaleFactor, dashWidthDiv]( qreal & element ) { element *= scaleFactor / dashWidthDiv; } );
        adjustedPen.setDashPattern( compressed );
      }
      else
      {
        adjustedPen.setStyle( Qt::SolidLine );
      }

      painter->setPen( adjustedPen );
      QPainterPath path;
      path.addPolygon( bufferedPoints );
      painter->drawPath( path );
    }

    bufferedPoints.clear();
  };

  QPointF p1;
  QPointF p2 = *ptIt;
  ptIt++;
  bufferedPoints << p2;
  for ( ; ptIt != points.constEnd(); ++ptIt )
  {
    p1 = *ptIt;
    if ( qgsDoubleNear( p1.y(), p2.y() ) && qgsDoubleNear( p1.x(), p2.x() ) )
    {
      continue;
    }

    double remainingSegmentDistance = std::sqrt( std::pow( p2.x() - p1.x(), 2.0 ) + std::pow( p2.y() - p1.y(), 2.0 ) );
    currentBufferLineLength += remainingSegmentDistance;
    while ( true )
    {
      // handle currentRemainingDashLength/currentRemainingGapLength
      if ( currentRemainingDashLength > 0 )
      {
        // bit more of dash to insert
        if ( remainingSegmentDistance >= currentRemainingDashLength )
        {
          // all of dash fits in
          buffer << currentRemainingDashLength << 0.0;
          totalBufferLength += currentRemainingDashLength;
          remainingSegmentDistance -= currentRemainingDashLength;
          patternIndex++;
          currentRemainingDashLength = 0.0;
          currentRemainingGapLength = sourcePattern.at( patternIndex );
        }
        else
        {
          // only part of remaining dash fits in
          buffer << remainingSegmentDistance << 0.0;
          totalBufferLength += remainingSegmentDistance;
          currentRemainingDashLength -= remainingSegmentDistance;
          break;
        }
      }
      if ( currentRemainingGapLength > 0 )
      {
        // bit more of gap to insert
        if ( remainingSegmentDistance >= currentRemainingGapLength )
        {
          // all of gap fits in
          buffer << 0.0 << currentRemainingGapLength;
          totalBufferLength += currentRemainingGapLength;
          remainingSegmentDistance -= currentRemainingGapLength;
          currentRemainingGapLength = 0.0;
          patternIndex++;
        }
        else
        {
          // only part of remaining gap fits in
          buffer << 0.0 << remainingSegmentDistance;
          totalBufferLength += remainingSegmentDistance;
          currentRemainingGapLength -= remainingSegmentDistance;
          break;
        }
      }

      if ( patternIndex >= sourcePattern.size() )
        patternIndex = 0;

      const double nextPatternDashLength = sourcePattern.at( patternIndex );
      const double nextPatternGapLength = sourcePattern.at( patternIndex + 1 );
      if ( nextPatternDashLength + nextPatternGapLength <= remainingSegmentDistance )
      {
        buffer << nextPatternDashLength << nextPatternGapLength;
        remainingSegmentDistance -= nextPatternDashLength + nextPatternGapLength;
        totalBufferLength += nextPatternDashLength + nextPatternGapLength;
        patternIndex += 2;
      }
      else if ( nextPatternDashLength <= remainingSegmentDistance )
      {
        // can fit in "dash", but not "gap"
        buffer << nextPatternDashLength << remainingSegmentDistance - nextPatternDashLength;
        totalBufferLength += remainingSegmentDistance;
        currentRemainingGapLength = nextPatternGapLength - ( remainingSegmentDistance - nextPatternDashLength );
        currentRemainingDashLength = 0;
        patternIndex++;
        break;
      }
      else
      {
        // can't fit in "dash"
        buffer << remainingSegmentDistance << 0.0;
        totalBufferLength += remainingSegmentDistance;
        currentRemainingGapLength = 0;
        currentRemainingDashLength = nextPatternDashLength - remainingSegmentDistance;
        break;
      }
    }

    bufferedPoints << p1;
    if ( mPatternCartographicTweakOnSharpCorners && ptIt + 1 != points.constEnd() )
    {
      QPointF nextPoint = *( ptIt + 1 );

      // extreme angles form more than 45 degree angle at a node
      if ( QgsSymbolLayerUtils::isSharpCorner( p2, p1, nextPoint ) )
      {
        // extreme angle. Rescale buffer and flush
        flushBuffer( &nextPoint );
        bufferedPoints << p1;
        // restart the line with the full length of the most recent dash element -- see
        // "Cartographic Generalization" (Swiss Society of Cartography) p33, example #8
        if ( patternIndex % 2 == 1 )
        {
          patternIndex--;
        }
        currentRemainingDashLength = sourcePattern.at( patternIndex );
      }
    }

    p2 = p1;
  }

  flushBuffer( nullptr );
  if ( !previousSegmentBuffer.empty() )
  {
    QPen adjustedPen = pen;
    adjustedPen.setStyle( Qt::SolidLine );
    painter->setPen( adjustedPen );
    QPainterPath path;
    path.addPolygon( previousSegmentBuffer );
    painter->drawPath( path );
    previousSegmentBuffer.clear();
  }
}

double QgsSimpleLineSymbolLayer::estimateMaxBleed( const QgsRenderContext &context ) const
{
  if ( mDrawInsidePolygon )
  {
    //set to clip line to the interior of polygon, so we expect no bleed
    return 0;
  }
  else
  {
    return context.convertToPainterUnits( ( mWidth / 2.0 ), mWidthUnit, mWidthMapUnitScale ) +
           context.convertToPainterUnits( std::fabs( mOffset ), mOffsetUnit, mOffsetMapUnitScale );
  }
}

QVector<qreal> QgsSimpleLineSymbolLayer::dxfCustomDashPattern( QgsUnitTypes::RenderUnit &unit ) const
{
  unit = mCustomDashPatternUnit;
  return mUseCustomDashPattern ? mCustomDashVector : QVector<qreal>();
}

Qt::PenStyle QgsSimpleLineSymbolLayer::dxfPenStyle() const
{
  return mPenStyle;
}

double QgsSimpleLineSymbolLayer::dxfWidth( const QgsDxfExport &e, QgsSymbolRenderContext &context ) const
{
  double width = mWidth;
  if ( mDataDefinedProperties.isActive( QgsSymbolLayer::PropertyStrokeWidth ) )
  {
    context.setOriginalValueVariable( mWidth );
    width = mDataDefinedProperties.valueAsDouble( QgsSymbolLayer::PropertyStrokeWidth, context.renderContext().expressionContext(), mWidth );
  }

  width *= QgsDxfExport::mapUnitScaleFactor( e.symbologyScale(), widthUnit(), e.mapUnits(), context.renderContext().mapToPixel().mapUnitsPerPixel() );
  if ( mWidthUnit == QgsUnitTypes::RenderMapUnits )
  {
    e.clipValueToMapUnitScale( width, mWidthMapUnitScale, context.renderContext().scaleFactor() );
  }
  return width;
}

QColor QgsSimpleLineSymbolLayer::dxfColor( QgsSymbolRenderContext &context ) const
{
  if ( mDataDefinedProperties.isActive( QgsSymbolLayer::PropertyStrokeColor ) )
  {
    context.setOriginalValueVariable( QgsSymbolLayerUtils::encodeColor( mColor ) );
    return mDataDefinedProperties.valueAsColor( QgsSymbolLayer::PropertyStrokeColor, context.renderContext().expressionContext(), mColor );
  }
  return mColor;
}

bool QgsSimpleLineSymbolLayer::canCauseArtifactsBetweenAdjacentTiles() const
{
  return mPenStyle != Qt::SolidLine || mUseCustomDashPattern;
}

bool QgsSimpleLineSymbolLayer::alignDashPattern() const
{
  return mAlignDashPattern;
}

void QgsSimpleLineSymbolLayer::setAlignDashPattern( bool enabled )
{
  mAlignDashPattern = enabled;
}

bool QgsSimpleLineSymbolLayer::tweakDashPatternOnCorners() const
{
  return mPatternCartographicTweakOnSharpCorners;
}

void QgsSimpleLineSymbolLayer::setTweakDashPatternOnCorners( bool enabled )
{
  mPatternCartographicTweakOnSharpCorners = enabled;
}

double QgsSimpleLineSymbolLayer::dxfOffset( const QgsDxfExport &e, QgsSymbolRenderContext &context ) const
{
  double offset = mOffset;

  if ( mDataDefinedProperties.isActive( QgsSymbolLayer::PropertyOffset ) )
  {
    context.setOriginalValueVariable( mOffset );
    offset = mDataDefinedProperties.valueAsDouble( QgsSymbolLayer::PropertyOffset, context.renderContext().expressionContext(), mOffset );
  }

  offset *= QgsDxfExport::mapUnitScaleFactor( e.symbologyScale(), offsetUnit(), e.mapUnits(), context.renderContext().mapToPixel().mapUnitsPerPixel() );
  if ( mOffsetUnit == QgsUnitTypes::RenderMapUnits )
  {
    e.clipValueToMapUnitScale( offset, mOffsetMapUnitScale, context.renderContext().scaleFactor() );
  }
  return -offset; //direction seems to be inverse to symbology offset
}

/////////

///@cond PRIVATE

class MyLine
{
  public:
    MyLine( QPointF p1, QPointF p2 )
      : mVertical( false )
      , mIncreasing( false )
      , mT( 0.0 )
      , mLength( 0.0 )
    {
      if ( p1 == p2 )
        return; // invalid

      // tangent and direction
      if ( qgsDoubleNear( p1.x(), p2.x() ) )
      {
        // vertical line - tangent undefined
        mVertical = true;
        mIncreasing = ( p2.y() > p1.y() );
      }
      else
      {
        mVertical = false;
        mT = ( p2.y() - p1.y() ) / ( p2.x() - p1.x() );
        mIncreasing = ( p2.x() > p1.x() );
      }

      // length
      double x = ( p2.x() - p1.x() );
      double y = ( p2.y() - p1.y() );
      mLength = std::sqrt( x * x + y * y );
    }

    // return angle in radians
    double angle()
    {
      double a = ( mVertical ? M_PI_2 : std::atan( mT ) );

      if ( !mIncreasing )
        a += M_PI;
      return a;
    }

    // return difference for x,y when going along the line with specified interval
    QPointF diffForInterval( double interval )
    {
      if ( mVertical )
        return ( mIncreasing ? QPointF( 0, interval ) : QPointF( 0, -interval ) );

      double alpha = std::atan( mT );
      double dx = std::cos( alpha ) * interval;
      double dy = std::sin( alpha ) * interval;
      return ( mIncreasing ? QPointF( dx, dy ) : QPointF( -dx, -dy ) );
    }

    double length() const { return mLength; }

  protected:
    bool mVertical;
    bool mIncreasing;
    double mT;
    double mLength;
};

///@endcond

//
// QgsTemplatedLineSymbolLayerBase
//
QgsTemplatedLineSymbolLayerBase::QgsTemplatedLineSymbolLayerBase( bool rotateSymbol, double interval )
  : mRotateSymbols( rotateSymbol )
  , mInterval( interval )
{

}

Qgis::MarkerLinePlacement QgsTemplatedLineSymbolLayerBase::placement() const
{
  if ( mPlacements & Qgis::MarkerLinePlacement::Interval )
    return Qgis::MarkerLinePlacement::Interval;
  else if ( mPlacements & Qgis::MarkerLinePlacement::Vertex )
    return Qgis::MarkerLinePlacement::Vertex;
  else if ( ( mPlacements & Qgis::MarkerLinePlacement::FirstVertex )
            && ( mPlacements & Qgis::MarkerLinePlacement::InnerVertices )
            && ( mPlacements & Qgis::MarkerLinePlacement::LastVertex ) )
    return Qgis::MarkerLinePlacement::Vertex; // retain round trip for deprecated old API
  else if ( mPlacements & Qgis::MarkerLinePlacement::LastVertex )
    return Qgis::MarkerLinePlacement::LastVertex;
  else if ( mPlacements & Qgis::MarkerLinePlacement::FirstVertex )
    return Qgis::MarkerLinePlacement::FirstVertex;
  else if ( mPlacements & Qgis::MarkerLinePlacement::CentralPoint )
    return Qgis::MarkerLinePlacement::CentralPoint;
  else if ( mPlacements & Qgis::MarkerLinePlacement::CurvePoint )
    return Qgis::MarkerLinePlacement::CurvePoint;
  else if ( mPlacements & Qgis::MarkerLinePlacement::SegmentCenter )
    return Qgis::MarkerLinePlacement::SegmentCenter;
  else
    return Qgis::MarkerLinePlacement::Interval;
}

void QgsTemplatedLineSymbolLayerBase::setPlacement( Qgis::MarkerLinePlacement placement )
{
  mPlacements = placement;
}

QgsTemplatedLineSymbolLayerBase::~QgsTemplatedLineSymbolLayerBase() = default;

void QgsTemplatedLineSymbolLayerBase::renderPolyline( const QPolygonF &points, QgsSymbolRenderContext &context )
{
  if ( mRenderingFeature )
  {
    // in the middle of rendering a possibly multi-part feature, so we collect all the parts and defer the actual rendering
    // until after we've received the final part
    mFeatureSymbolOpacity = context.opacity();
    mCurrentFeatureIsSelected = context.selected();
  }

  double offset = mOffset;

  if ( mDataDefinedProperties.isActive( QgsSymbolLayer::PropertyOffset ) )
  {
    context.setOriginalValueVariable( mOffset );
    offset = mDataDefinedProperties.valueAsDouble( QgsSymbolLayer::PropertyOffset, context.renderContext().expressionContext(), mOffset );
  }

  Qgis::MarkerLinePlacements placements = QgsTemplatedLineSymbolLayerBase::placements();

  if ( mDataDefinedProperties.isActive( QgsSymbolLayer::PropertyPlacement ) )
  {
    QVariant exprVal = mDataDefinedProperties.value( QgsSymbolLayer::PropertyPlacement, context.renderContext().expressionContext() );
    if ( !exprVal.isNull() )
    {
      QString placementString = exprVal.toString();
      if ( placementString.compare( QLatin1String( "interval" ), Qt::CaseInsensitive ) == 0 )
      {
        placements = Qgis::MarkerLinePlacement::Interval;
      }
      else if ( placementString.compare( QLatin1String( "vertex" ), Qt::CaseInsensitive ) == 0 )
      {
        placements = Qgis::MarkerLinePlacement::Vertex;
      }
      else if ( placementString.compare( QLatin1String( "innervertices" ), Qt::CaseInsensitive ) == 0 )
      {
        placements = Qgis::MarkerLinePlacement::InnerVertices;
      }
      else if ( placementString.compare( QLatin1String( "lastvertex" ), Qt::CaseInsensitive ) == 0 )
      {
        placements = Qgis::MarkerLinePlacement::LastVertex;
      }
      else if ( placementString.compare( QLatin1String( "firstvertex" ), Qt::CaseInsensitive ) == 0 )
      {
        placements = Qgis::MarkerLinePlacement::FirstVertex;
      }
      else if ( placementString.compare( QLatin1String( "centerpoint" ), Qt::CaseInsensitive ) == 0 )
      {
        placements = Qgis::MarkerLinePlacement::CentralPoint;
      }
      else if ( placementString.compare( QLatin1String( "curvepoint" ), Qt::CaseInsensitive ) == 0 )
      {
        placements = Qgis::MarkerLinePlacement::CurvePoint;
      }
      else if ( placementString.compare( QLatin1String( "segmentcenter" ), Qt::CaseInsensitive ) == 0 )
      {
        placements = Qgis::MarkerLinePlacement::SegmentCenter;
      }
      else
      {
        placements = Qgis::MarkerLinePlacement::Interval;
      }
    }
  }

  QgsScopedQPainterState painterState( context.renderContext().painter() );

  double averageOver = mAverageAngleLength;
  if ( mDataDefinedProperties.isActive( QgsSymbolLayer::PropertyAverageAngleLength ) )
  {
    context.setOriginalValueVariable( mAverageAngleLength );
    averageOver = mDataDefinedProperties.valueAsDouble( QgsSymbolLayer::PropertyAverageAngleLength, context.renderContext().expressionContext(), mAverageAngleLength );
  }
  averageOver = context.renderContext().convertToPainterUnits( averageOver, mAverageAngleLengthUnit, mAverageAngleLengthMapUnitScale ) / 2.0;

  if ( qgsDoubleNear( offset, 0.0 ) )
  {
    if ( placements & Qgis::MarkerLinePlacement::Interval )
      renderPolylineInterval( points, context, averageOver );
    if ( placements & Qgis::MarkerLinePlacement::CentralPoint )
      renderPolylineCentral( points, context, averageOver );
    if ( placements & Qgis::MarkerLinePlacement::Vertex )
      renderPolylineVertex( points, context, Qgis::MarkerLinePlacement::Vertex );
    if ( placements & Qgis::MarkerLinePlacement::FirstVertex
         && ( mPlaceOnEveryPart || !mHasRenderedFirstPart ) )
    {
      renderPolylineVertex( points, context, Qgis::MarkerLinePlacement::FirstVertex );
      mHasRenderedFirstPart = mRenderingFeature;
    }
    if ( placements & Qgis::MarkerLinePlacement::InnerVertices )
      renderPolylineVertex( points, context, Qgis::MarkerLinePlacement::InnerVertices );
    if ( placements & Qgis::MarkerLinePlacement::CurvePoint )
      renderPolylineVertex( points, context, Qgis::MarkerLinePlacement::CurvePoint );
    if ( placements & Qgis::MarkerLinePlacement::SegmentCenter )
      renderPolylineVertex( points, context, Qgis::MarkerLinePlacement::SegmentCenter );
    if ( placements & Qgis::MarkerLinePlacement::LastVertex )
      renderPolylineVertex( points, context, Qgis::MarkerLinePlacement::LastVertex );
  }
  else
  {
    context.renderContext().setGeometry( nullptr ); //always use segmented geometry with offset
    QList<QPolygonF> mline = ::offsetLine( points, context.renderContext().convertToPainterUnits( offset, mOffsetUnit, mOffsetMapUnitScale ), context.originalGeometryType() != QgsWkbTypes::UnknownGeometry ? context.originalGeometryType() : QgsWkbTypes::LineGeometry );

    for ( int part = 0; part < mline.count(); ++part )
    {
      const QPolygonF &points2 = mline[ part ];

      if ( placements & Qgis::MarkerLinePlacement::Interval )
        renderPolylineInterval( points2, context, averageOver );
      if ( placements & Qgis::MarkerLinePlacement::CentralPoint )
        renderPolylineCentral( points2, context, averageOver );
      if ( placements & Qgis::MarkerLinePlacement::Vertex )
        renderPolylineVertex( points2, context, Qgis::MarkerLinePlacement::Vertex );
      if ( placements & Qgis::MarkerLinePlacement::InnerVertices )
        renderPolylineVertex( points2, context, Qgis::MarkerLinePlacement::InnerVertices );
      if ( placements & Qgis::MarkerLinePlacement::LastVertex )
        renderPolylineVertex( points2, context, Qgis::MarkerLinePlacement::LastVertex );
      if ( placements & Qgis::MarkerLinePlacement::FirstVertex
           && ( mPlaceOnEveryPart || !mHasRenderedFirstPart ) )
      {
        renderPolylineVertex( points2, context, Qgis::MarkerLinePlacement::FirstVertex );
        mHasRenderedFirstPart = mRenderingFeature;
      }
      if ( placements & Qgis::MarkerLinePlacement::CurvePoint )
        renderPolylineVertex( points2, context, Qgis::MarkerLinePlacement::CurvePoint );
      if ( placements & Qgis::MarkerLinePlacement::SegmentCenter )
        renderPolylineVertex( points2, context, Qgis::MarkerLinePlacement::SegmentCenter );
    }
  }
}

void QgsTemplatedLineSymbolLayerBase::renderPolygonStroke( const QPolygonF &points, const QVector<QPolygonF> *rings, QgsSymbolRenderContext &context )
{
  const QgsCurvePolygon *curvePolygon = dynamic_cast<const QgsCurvePolygon *>( context.renderContext().geometry() );

  if ( curvePolygon )
  {
    context.renderContext().setGeometry( curvePolygon->exteriorRing() );
  }

  QgsExpressionContextScope *scope = nullptr;
  std::unique_ptr< QgsExpressionContextScopePopper > scopePopper;
  if ( hasDataDefinedProperties() )
  {
    scope = new QgsExpressionContextScope();
    scopePopper = std::make_unique< QgsExpressionContextScopePopper >( context.renderContext().expressionContext(), scope );
  }

  switch ( mRingFilter )
  {
    case AllRings:
    case ExteriorRingOnly:
    {
      if ( scope )
        scope->addVariable( QgsExpressionContextScope::StaticVariable( QgsExpressionContext::EXPR_GEOMETRY_RING_NUM, 0, true ) );

      renderPolyline( points, context );
      break;
    }
    case InteriorRingsOnly:
      break;
  }

  if ( rings )
  {
    switch ( mRingFilter )
    {
      case AllRings:
      case InteriorRingsOnly:
      {
        mOffset = -mOffset; // invert the offset for rings!
        for ( int i = 0; i < rings->size(); ++i )
        {
          if ( curvePolygon )
          {
            context.renderContext().setGeometry( curvePolygon->interiorRing( i ) );
          }
          if ( scope )
            scope->addVariable( QgsExpressionContextScope::StaticVariable( QgsExpressionContext::EXPR_GEOMETRY_RING_NUM, i + 1, true ) );

          renderPolyline( rings->at( i ), context );
        }
        mOffset = -mOffset;
      }
      break;
      case ExteriorRingOnly:
        break;
    }
  }
}

QgsUnitTypes::RenderUnit QgsTemplatedLineSymbolLayerBase::outputUnit() const
{
  QgsUnitTypes::RenderUnit unit = QgsLineSymbolLayer::outputUnit();
  if ( intervalUnit() != unit || mOffsetUnit != unit || offsetAlongLineUnit() != unit )
  {
    return QgsUnitTypes::RenderUnknownUnit;
  }
  return unit;
}

void QgsTemplatedLineSymbolLayerBase::setOutputUnit( QgsUnitTypes::RenderUnit unit )
{
  QgsLineSymbolLayer::setOutputUnit( unit );
  mIntervalUnit = unit;
  mOffsetAlongLineUnit = unit;
  mAverageAngleLengthUnit = unit;
}

void QgsTemplatedLineSymbolLayerBase::setMapUnitScale( const QgsMapUnitScale &scale )
{
  QgsLineSymbolLayer::setMapUnitScale( scale );
  setIntervalMapUnitScale( scale );
  mOffsetMapUnitScale = scale;
  setOffsetAlongLineMapUnitScale( scale );
}

QgsMapUnitScale QgsTemplatedLineSymbolLayerBase::mapUnitScale() const
{
  if ( QgsLineSymbolLayer::mapUnitScale() == intervalMapUnitScale() &&
       intervalMapUnitScale() == mOffsetMapUnitScale &&
       mOffsetMapUnitScale == offsetAlongLineMapUnitScale() )
  {
    return mOffsetMapUnitScale;
  }
  return QgsMapUnitScale();
}

QVariantMap QgsTemplatedLineSymbolLayerBase::properties() const
{
  QVariantMap map;
  map[QStringLiteral( "rotate" )] = ( rotateSymbols() ? QStringLiteral( "1" ) : QStringLiteral( "0" ) );
  map[QStringLiteral( "interval" )] = QString::number( interval() );
  map[QStringLiteral( "offset" )] = QString::number( mOffset );
  map[QStringLiteral( "offset_along_line" )] = QString::number( offsetAlongLine() );
  map[QStringLiteral( "offset_along_line_unit" )] = QgsUnitTypes::encodeUnit( offsetAlongLineUnit() );
  map[QStringLiteral( "offset_along_line_map_unit_scale" )] = QgsSymbolLayerUtils::encodeMapUnitScale( offsetAlongLineMapUnitScale() );
  map[QStringLiteral( "offset_unit" )] = QgsUnitTypes::encodeUnit( mOffsetUnit );
  map[QStringLiteral( "offset_map_unit_scale" )] = QgsSymbolLayerUtils::encodeMapUnitScale( mOffsetMapUnitScale );
  map[QStringLiteral( "interval_unit" )] = QgsUnitTypes::encodeUnit( intervalUnit() );
  map[QStringLiteral( "interval_map_unit_scale" )] = QgsSymbolLayerUtils::encodeMapUnitScale( intervalMapUnitScale() );
  map[QStringLiteral( "average_angle_length" )] = QString::number( mAverageAngleLength );
  map[QStringLiteral( "average_angle_unit" )] = QgsUnitTypes::encodeUnit( mAverageAngleLengthUnit );
  map[QStringLiteral( "average_angle_map_unit_scale" )] = QgsSymbolLayerUtils::encodeMapUnitScale( mAverageAngleLengthMapUnitScale );

  map[QStringLiteral( "placements" )] = qgsFlagValueToKeys( mPlacements );

  map[QStringLiteral( "ring_filter" )] = QString::number( static_cast< int >( mRingFilter ) );
  map[QStringLiteral( "place_on_every_part" )] = mPlaceOnEveryPart;
  return map;
}

bool QgsTemplatedLineSymbolLayerBase::canCauseArtifactsBetweenAdjacentTiles() const
{
  return mPlaceOnEveryPart
         || ( mPlacements & Qgis::MarkerLinePlacement::Interval )
         || ( mPlacements & Qgis::MarkerLinePlacement::CentralPoint )
         || ( mPlacements & Qgis::MarkerLinePlacement::SegmentCenter );
}

void QgsTemplatedLineSymbolLayerBase::startFeatureRender( const QgsFeature &, QgsRenderContext & )
{
  mRenderingFeature = true;
  mHasRenderedFirstPart = false;
}

void QgsTemplatedLineSymbolLayerBase::stopFeatureRender( const QgsFeature &feature, QgsRenderContext &context )
{
  mRenderingFeature = false;
  if ( mPlaceOnEveryPart  || !( mPlacements & Qgis::MarkerLinePlacement::LastVertex ) )
    return;

  const double prevOpacity = subSymbol()->opacity();
  subSymbol()->setOpacity( prevOpacity * mFeatureSymbolOpacity );

  // render final point
  renderSymbol( mFinalVertex, &feature, context, -1, mCurrentFeatureIsSelected );
  mFeatureSymbolOpacity = 1;
  subSymbol()->setOpacity( prevOpacity );
}

void QgsTemplatedLineSymbolLayerBase::copyTemplateSymbolProperties( QgsTemplatedLineSymbolLayerBase *destLayer ) const
{
  destLayer->setSubSymbol( const_cast< QgsTemplatedLineSymbolLayerBase * >( this )->subSymbol()->clone() );
  destLayer->setOffset( mOffset );
  destLayer->setPlacements( placements() );
  destLayer->setOffsetUnit( mOffsetUnit );
  destLayer->setOffsetMapUnitScale( mOffsetMapUnitScale );
  destLayer->setIntervalUnit( intervalUnit() );
  destLayer->setIntervalMapUnitScale( intervalMapUnitScale() );
  destLayer->setOffsetAlongLine( offsetAlongLine() );
  destLayer->setOffsetAlongLineMapUnitScale( offsetAlongLineMapUnitScale() );
  destLayer->setOffsetAlongLineUnit( offsetAlongLineUnit() );
  destLayer->setAverageAngleLength( mAverageAngleLength );
  destLayer->setAverageAngleUnit( mAverageAngleLengthUnit );
  destLayer->setAverageAngleMapUnitScale( mAverageAngleLengthMapUnitScale );
  destLayer->setRingFilter( mRingFilter );
  destLayer->setPlaceOnEveryPart( mPlaceOnEveryPart );
  copyDataDefinedProperties( destLayer );
  copyPaintEffect( destLayer );
}

void QgsTemplatedLineSymbolLayerBase::setCommonProperties( QgsTemplatedLineSymbolLayerBase *destLayer, const QVariantMap &properties )
{
  if ( properties.contains( QStringLiteral( "offset" ) ) )
  {
    destLayer->setOffset( properties[QStringLiteral( "offset" )].toDouble() );
  }
  if ( properties.contains( QStringLiteral( "offset_unit" ) ) )
  {
    destLayer->setOffsetUnit( QgsUnitTypes::decodeRenderUnit( properties[QStringLiteral( "offset_unit" )].toString() ) );
  }
  if ( properties.contains( QStringLiteral( "interval_unit" ) ) )
  {
    destLayer->setIntervalUnit( QgsUnitTypes::decodeRenderUnit( properties[QStringLiteral( "interval_unit" )].toString() ) );
  }
  if ( properties.contains( QStringLiteral( "offset_along_line" ) ) )
  {
    destLayer->setOffsetAlongLine( properties[QStringLiteral( "offset_along_line" )].toDouble() );
  }
  if ( properties.contains( QStringLiteral( "offset_along_line_unit" ) ) )
  {
    destLayer->setOffsetAlongLineUnit( QgsUnitTypes::decodeRenderUnit( properties[QStringLiteral( "offset_along_line_unit" )].toString() ) );
  }
  if ( properties.contains( ( QStringLiteral( "offset_along_line_map_unit_scale" ) ) ) )
  {
    destLayer->setOffsetAlongLineMapUnitScale( QgsSymbolLayerUtils::decodeMapUnitScale( properties[QStringLiteral( "offset_along_line_map_unit_scale" )].toString() ) );
  }

  if ( properties.contains( QStringLiteral( "offset_map_unit_scale" ) ) )
  {
    destLayer->setOffsetMapUnitScale( QgsSymbolLayerUtils::decodeMapUnitScale( properties[QStringLiteral( "offset_map_unit_scale" )].toString() ) );
  }
  if ( properties.contains( QStringLiteral( "interval_map_unit_scale" ) ) )
  {
    destLayer->setIntervalMapUnitScale( QgsSymbolLayerUtils::decodeMapUnitScale( properties[QStringLiteral( "interval_map_unit_scale" )].toString() ) );
  }

  if ( properties.contains( QStringLiteral( "average_angle_length" ) ) )
  {
    destLayer->setAverageAngleLength( properties[QStringLiteral( "average_angle_length" )].toDouble() );
  }
  if ( properties.contains( QStringLiteral( "average_angle_unit" ) ) )
  {
    destLayer->setAverageAngleUnit( QgsUnitTypes::decodeRenderUnit( properties[QStringLiteral( "average_angle_unit" )].toString() ) );
  }
  if ( properties.contains( ( QStringLiteral( "average_angle_map_unit_scale" ) ) ) )
  {
    destLayer->setAverageAngleMapUnitScale( QgsSymbolLayerUtils::decodeMapUnitScale( properties[QStringLiteral( "average_angle_map_unit_scale" )].toString() ) );
  }

  if ( properties.contains( QStringLiteral( "placement" ) ) )
  {
    if ( properties[QStringLiteral( "placement" )] == QLatin1String( "vertex" ) )
      destLayer->setPlacements( Qgis::MarkerLinePlacement::InnerVertices | Qgis::MarkerLinePlacement::FirstVertex | Qgis::MarkerLinePlacement::LastVertex );
    else if ( properties[QStringLiteral( "placement" )] == QLatin1String( "lastvertex" ) )
      destLayer->setPlacements( Qgis::MarkerLinePlacement::LastVertex );
    else if ( properties[QStringLiteral( "placement" )] == QLatin1String( "firstvertex" ) )
      destLayer->setPlacements( Qgis::MarkerLinePlacement::FirstVertex );
    else if ( properties[QStringLiteral( "placement" )] == QLatin1String( "centralpoint" ) )
      destLayer->setPlacements( Qgis::MarkerLinePlacement::CentralPoint );
    else if ( properties[QStringLiteral( "placement" )] == QLatin1String( "curvepoint" ) )
      destLayer->setPlacements( Qgis::MarkerLinePlacement::CurvePoint );
    else if ( properties[QStringLiteral( "placement" )] == QLatin1String( "segmentcenter" ) )
      destLayer->setPlacements( Qgis::MarkerLinePlacement::SegmentCenter );
    else
      destLayer->setPlacements( Qgis::MarkerLinePlacement::Interval );
  }
  else if ( properties.contains( QStringLiteral( "placements" ) ) )
  {
    Qgis::MarkerLinePlacements placements = qgsFlagKeysToValue( properties.value( QStringLiteral( "placements" ) ).toString(), Qgis::MarkerLinePlacements() );
    destLayer->setPlacements( placements );
  }

  if ( properties.contains( QStringLiteral( "ring_filter" ) ) )
  {
    destLayer->setRingFilter( static_cast< RenderRingFilter>( properties[QStringLiteral( "ring_filter" )].toInt() ) );
  }

  destLayer->setPlaceOnEveryPart( properties.value( QStringLiteral( "place_on_every_part" ), true ).toBool() );

  destLayer->restoreOldDataDefinedProperties( properties );
}

void QgsTemplatedLineSymbolLayerBase::renderPolylineInterval( const QPolygonF &points, QgsSymbolRenderContext &context, double averageOver )
{
  if ( points.isEmpty() )
    return;

  double lengthLeft = 0; // how much is left until next marker

  QgsRenderContext &rc = context.renderContext();
  double interval = mInterval;

  QgsExpressionContextScope *scope = new QgsExpressionContextScope();
  QgsExpressionContextScopePopper scopePopper( context.renderContext().expressionContext(), scope );

  if ( mDataDefinedProperties.isActive( QgsSymbolLayer::PropertyInterval ) )
  {
    context.setOriginalValueVariable( mInterval );
    interval = mDataDefinedProperties.valueAsDouble( QgsSymbolLayer::PropertyInterval, context.renderContext().expressionContext(), mInterval );
  }
  if ( interval <= 0 )
  {
    interval = 0.1;
  }
  double offsetAlongLine = mOffsetAlongLine;
  if ( mDataDefinedProperties.isActive( QgsSymbolLayer::PropertyOffsetAlongLine ) )
  {
    context.setOriginalValueVariable( mOffsetAlongLine );
    offsetAlongLine = mDataDefinedProperties.valueAsDouble( QgsSymbolLayer::PropertyOffsetAlongLine, context.renderContext().expressionContext(), mOffsetAlongLine );
  }

  double painterUnitInterval = rc.convertToPainterUnits( interval, intervalUnit(), intervalMapUnitScale() );
  if ( intervalUnit() == QgsUnitTypes::RenderMetersInMapUnits && rc.flags() & Qgis::RenderContextFlag::RenderSymbolPreview )
  {
    // rendering for symbol previews -- an interval in meters in map units can't be calculated, so treat the size as millimeters
    // and clamp it to a reasonable range. It's the best we can do in this situation!
    painterUnitInterval = std::min( std::max( rc.convertToPainterUnits( interval, QgsUnitTypes::RenderMillimeters ), 10.0 ), 100.0 );
  }

  if ( painterUnitInterval < 0 )
    return;

  double painterUnitOffsetAlongLine = 0;

  // only calculated if we need it!
  double totalLength = -1;

  if ( !qgsDoubleNear( offsetAlongLine, 0 ) )
  {
    switch ( offsetAlongLineUnit() )
    {
      case QgsUnitTypes::RenderMillimeters:
      case QgsUnitTypes::RenderMapUnits:
      case QgsUnitTypes::RenderPixels:
      case QgsUnitTypes::RenderPoints:
      case QgsUnitTypes::RenderInches:
      case QgsUnitTypes::RenderUnknownUnit:
      case QgsUnitTypes::RenderMetersInMapUnits:
        painterUnitOffsetAlongLine = rc.convertToPainterUnits( offsetAlongLine, offsetAlongLineUnit(), offsetAlongLineMapUnitScale() );
        break;
      case QgsUnitTypes::RenderPercentage:
        totalLength = QgsSymbolLayerUtils::polylineLength( points );
        painterUnitOffsetAlongLine = offsetAlongLine / 100 * totalLength;
        break;
    }

    if ( points.isClosed() )
    {
      if ( painterUnitOffsetAlongLine > 0 )
      {
        if ( totalLength < 0 )
          totalLength = QgsSymbolLayerUtils::polylineLength( points );
        painterUnitOffsetAlongLine = std::fmod( painterUnitOffsetAlongLine, totalLength );
      }
      else if ( painterUnitOffsetAlongLine < 0 )
      {
        if ( totalLength < 0 )
          totalLength = QgsSymbolLayerUtils::polylineLength( points );
        painterUnitOffsetAlongLine = totalLength - std::fmod( -painterUnitOffsetAlongLine, totalLength );
      }
    }
  }

  if ( offsetAlongLineUnit() == QgsUnitTypes::RenderMetersInMapUnits && rc.flags() & Qgis::RenderContextFlag::RenderSymbolPreview )
  {
    // rendering for symbol previews -- an offset in meters in map units can't be calculated, so treat the size as millimeters
    // and clamp it to a reasonable range. It's the best we can do in this situation!
    painterUnitOffsetAlongLine = std::min( std::max( rc.convertToPainterUnits( offsetAlongLine, QgsUnitTypes::RenderMillimeters ), 3.0 ), 100.0 );
  }

  lengthLeft = painterUnitInterval - painterUnitOffsetAlongLine;

  if ( averageOver > 0 && !qgsDoubleNear( averageOver, 0.0 ) )
  {
    QVector< QPointF > angleStartPoints;
    QVector< QPointF > symbolPoints;
    QVector< QPointF > angleEndPoints;

    // we collect 3 arrays of points. These correspond to
    // 1. the actual point at which to render the symbol
    // 2. the start point of a line averaging the angle over the desired distance (i.e. -averageOver distance from the points in array 1)
    // 3. the end point of a line averaging the angle over the desired distance (i.e. +averageOver distance from the points in array 2)
    // it gets quite tricky, because for closed rings we need to trace backwards from the initial point to calculate this
    // (or trace past the final point)
    collectOffsetPoints( points, symbolPoints, painterUnitInterval, lengthLeft );

    if ( symbolPoints.empty() )
    {
      // no symbols to draw, shortcut out early
      return;
    }

    if ( symbolPoints.count() > 1 && symbolPoints.constFirst() == symbolPoints.constLast() )
    {
      // avoid duplicate points at start and end of closed rings
      symbolPoints.pop_back();
    }

    angleEndPoints.reserve( symbolPoints.size() );
    angleStartPoints.reserve( symbolPoints.size() );
    if ( averageOver <= painterUnitOffsetAlongLine )
    {
      collectOffsetPoints( points, angleStartPoints, painterUnitInterval, lengthLeft + averageOver, 0, symbolPoints.size() );
    }
    else
    {
      collectOffsetPoints( points, angleStartPoints, painterUnitInterval, 0, averageOver - painterUnitOffsetAlongLine, symbolPoints.size() );
    }
    collectOffsetPoints( points, angleEndPoints, painterUnitInterval, lengthLeft - averageOver, 0, symbolPoints.size() );

    int pointNum = 0;
    for ( int i = 0; i < symbolPoints.size(); ++ i )
    {
      if ( context.renderContext().renderingStopped() )
        break;

      const QPointF pt = symbolPoints[i];
      const QPointF startPt = angleStartPoints[i];
      const QPointF endPt = angleEndPoints[i];

      MyLine l( startPt, endPt );
      // rotate marker (if desired)
      if ( rotateSymbols() )
      {
        setSymbolLineAngle( l.angle() * 180 / M_PI );
      }

      scope->addVariable( QgsExpressionContextScope::StaticVariable( QgsExpressionContext::EXPR_GEOMETRY_POINT_NUM, ++pointNum, true ) );
      renderSymbol( pt, context.feature(), rc, -1, context.selected() );
    }
  }
  else
  {
    // not averaging line angle -- always use exact section angle
    int pointNum = 0;
    QPointF lastPt = points[0];
    for ( int i = 1; i < points.count(); ++i )
    {
      if ( context.renderContext().renderingStopped() )
        break;

      const QPointF &pt = points[i];

      if ( lastPt == pt ) // must not be equal!
        continue;

      // for each line, find out dx and dy, and length
      MyLine l( lastPt, pt );
      QPointF diff = l.diffForInterval( painterUnitInterval );

      // if there's some length left from previous line
      // use only the rest for the first point in new line segment
      double c = 1 - lengthLeft / painterUnitInterval;

      lengthLeft += l.length();

      // rotate marker (if desired)
      if ( rotateSymbols() )
      {
        setSymbolLineAngle( l.angle() * 180 / M_PI );
      }

      // while we're not at the end of line segment, draw!
      while ( lengthLeft > painterUnitInterval )
      {
        // "c" is 1 for regular point or in interval (0,1] for begin of line segment
        lastPt += c * diff;
        lengthLeft -= painterUnitInterval;
        scope->addVariable( QgsExpressionContextScope::StaticVariable( QgsExpressionContext::EXPR_GEOMETRY_POINT_NUM, ++pointNum, true ) );
        renderSymbol( lastPt, context.feature(), rc, -1, context.selected() );
        c = 1; // reset c (if wasn't 1 already)
      }

      lastPt = pt;
    }

  }
}

static double _averageAngle( QPointF prevPt, QPointF pt, QPointF nextPt )
{
  // calc average angle between the previous and next point
  double a1 = MyLine( prevPt, pt ).angle();
  double a2 = MyLine( pt, nextPt ).angle();
  double unitX = std::cos( a1 ) + std::cos( a2 ), unitY = std::sin( a1 ) + std::sin( a2 );

  return std::atan2( unitY, unitX );
}

void QgsTemplatedLineSymbolLayerBase::renderPolylineVertex( const QPolygonF &points, QgsSymbolRenderContext &context, Qgis::MarkerLinePlacement placement )
{
  if ( points.isEmpty() )
    return;

  QgsRenderContext &rc = context.renderContext();

  int i = -1, maxCount = 0;
  bool isRing = false;

  QgsExpressionContextScope *scope = new QgsExpressionContextScope();
  QgsExpressionContextScopePopper scopePopper( context.renderContext().expressionContext(), scope );
  scope->addVariable( QgsExpressionContextScope::StaticVariable( QgsExpressionContext::EXPR_GEOMETRY_POINT_COUNT, points.size(), true ) );

  double offsetAlongLine = mOffsetAlongLine;
  if ( mDataDefinedProperties.isActive( QgsSymbolLayer::PropertyOffsetAlongLine ) )
  {
    context.setOriginalValueVariable( mOffsetAlongLine );
    offsetAlongLine = mDataDefinedProperties.valueAsDouble( QgsSymbolLayer::PropertyOffsetAlongLine, context.renderContext().expressionContext(), mOffsetAlongLine );
  }

  // only calculated if we need it!!
  double totalLength = -1;
  if ( !qgsDoubleNear( offsetAlongLine, 0.0 ) )
  {
    //scale offset along line
    switch ( offsetAlongLineUnit() )
    {
      case QgsUnitTypes::RenderMillimeters:
      case QgsUnitTypes::RenderMapUnits:
      case QgsUnitTypes::RenderPixels:
      case QgsUnitTypes::RenderPoints:
      case QgsUnitTypes::RenderInches:
      case QgsUnitTypes::RenderUnknownUnit:
      case QgsUnitTypes::RenderMetersInMapUnits:
        offsetAlongLine = rc.convertToPainterUnits( offsetAlongLine, offsetAlongLineUnit(), offsetAlongLineMapUnitScale() );
        break;
      case QgsUnitTypes::RenderPercentage:
        totalLength = QgsSymbolLayerUtils::polylineLength( points );
        offsetAlongLine = offsetAlongLine / 100 * totalLength;
        break;
    }
    if ( points.isClosed() )
    {
      if ( offsetAlongLine > 0 )
      {
        if ( totalLength < 0 )
          totalLength = QgsSymbolLayerUtils::polylineLength( points );
        offsetAlongLine = std::fmod( offsetAlongLine, totalLength );
      }
      else if ( offsetAlongLine < 0 )
      {
        if ( totalLength < 0 )
          totalLength = QgsSymbolLayerUtils::polylineLength( points );
        offsetAlongLine = totalLength - std::fmod( -offsetAlongLine, totalLength );
      }
    }
  }

  if ( qgsDoubleNear( offsetAlongLine, 0.0 ) && context.renderContext().geometry()
       && context.renderContext().geometry()->hasCurvedSegments() && ( placement == Qgis::MarkerLinePlacement::Vertex
           || placement == Qgis::MarkerLinePlacement::InnerVertices
           || placement == Qgis::MarkerLinePlacement::CurvePoint ) )
  {
    QgsCoordinateTransform ct = context.renderContext().coordinateTransform();
    const QgsMapToPixel &mtp = context.renderContext().mapToPixel();

    QgsVertexId vId;
    QgsPoint vPoint;
    double x, y, z;
    QPointF mapPoint;
    int pointNum = 0;
    const int numPoints = context.renderContext().geometry()->nCoordinates();
    while ( context.renderContext().geometry()->nextVertex( vId, vPoint ) )
    {
      if ( context.renderContext().renderingStopped() )
        break;

      scope->addVariable( QgsExpressionContextScope::StaticVariable( QgsExpressionContext::EXPR_GEOMETRY_POINT_NUM, ++pointNum, true ) );

      if ( pointNum == 1 && placement == Qgis::MarkerLinePlacement::InnerVertices )
        continue;

      if ( pointNum == numPoints && placement == Qgis::MarkerLinePlacement::InnerVertices )
        continue;

      if ( ( ( placement == Qgis::MarkerLinePlacement::Vertex || placement == Qgis::MarkerLinePlacement::InnerVertices ) && vId.type == Qgis::VertexType::Segment )
           || ( placement == Qgis::MarkerLinePlacement::CurvePoint && vId.type == Qgis::VertexType::Curve ) )
      {
        //transform
        x = vPoint.x();
        y = vPoint.y();
        z = 0.0;
        if ( ct.isValid() )
        {
          ct.transformInPlace( x, y, z );
        }
        mapPoint.setX( x );
        mapPoint.setY( y );
        mtp.transformInPlace( mapPoint.rx(), mapPoint.ry() );
        if ( rotateSymbols() )
        {
          double angle = context.renderContext().geometry()->vertexAngle( vId );
          setSymbolLineAngle( angle * 180 / M_PI );
        }
        renderSymbol( mapPoint, context.feature(), rc, -1, context.selected() );
      }
    }

    return;
  }

  int pointNum = 0;

  switch ( placement )
  {
    case Qgis::MarkerLinePlacement::FirstVertex:
    {
      i = 0;
      maxCount = 1;
      break;
    }

    case Qgis::MarkerLinePlacement::LastVertex:
    {
      i = points.count() - 1;
      pointNum = i;
      maxCount = points.count();
      break;
    }

    case Qgis::MarkerLinePlacement::InnerVertices:
    {
      i = 1;
      pointNum = 1;
      maxCount = points.count() - 1;
      break;
    }

    case Qgis::MarkerLinePlacement::Vertex:
    case Qgis::MarkerLinePlacement::SegmentCenter:
    {
      i = placement == Qgis::MarkerLinePlacement::Vertex ? 0 : 1;
      maxCount = points.count();
      if ( points.first() == points.last() )
        isRing = true;
      break;
    }

    case Qgis::MarkerLinePlacement::Interval:
    case Qgis::MarkerLinePlacement::CentralPoint:
    case Qgis::MarkerLinePlacement::CurvePoint:
    {
      return;
    }
  }

  if ( offsetAlongLine > 0 && ( placement == Qgis::MarkerLinePlacement::FirstVertex || placement == Qgis::MarkerLinePlacement::LastVertex ) )
  {
    double distance;
    distance = placement == Qgis::MarkerLinePlacement::FirstVertex ? offsetAlongLine : -offsetAlongLine;
    renderOffsetVertexAlongLine( points, i, distance, context, placement );

    return;
  }

  QPointF prevPoint;
  if ( placement == Qgis::MarkerLinePlacement::SegmentCenter && !points.empty() )
    prevPoint = points.at( 0 );

  QPointF symbolPoint;
  for ( ; i < maxCount; ++i )
  {
    scope->addVariable( QgsExpressionContextScope::StaticVariable( QgsExpressionContext::EXPR_GEOMETRY_POINT_NUM, ++pointNum, true ) );

    if ( isRing && placement == Qgis::MarkerLinePlacement::Vertex && i == points.count() - 1 )
    {
      continue; // don't draw the last marker - it has been drawn already
    }

    if ( placement == Qgis::MarkerLinePlacement::SegmentCenter )
    {
      QPointF currentPoint = points.at( i );
      symbolPoint = QPointF( 0.5 * ( currentPoint.x() + prevPoint.x() ),
                             0.5 * ( currentPoint.y() + prevPoint.y() ) );
      if ( rotateSymbols() )
      {
        double angle = std::atan2( currentPoint.y() - prevPoint.y(),
                                   currentPoint.x() - prevPoint.x() );
        setSymbolLineAngle( angle * 180 / M_PI );
      }
      prevPoint = currentPoint;
    }
    else
    {
      symbolPoint = points.at( i );
      // rotate marker (if desired)
      if ( rotateSymbols() )
      {
        double angle = markerAngle( points, isRing, i );
        setSymbolLineAngle( angle * 180 / M_PI );
      }
    }

    mFinalVertex = symbolPoint;
    if ( i != points.count() - 1 || placement != Qgis::MarkerLinePlacement::LastVertex || mPlaceOnEveryPart || !mRenderingFeature )
      renderSymbol( symbolPoint, context.feature(), rc, -1, context.selected() );
  }
}

double QgsTemplatedLineSymbolLayerBase::markerAngle( const QPolygonF &points, bool isRing, int vertex )
{
  double angle = 0;
  const QPointF &pt = points[vertex];

  if ( isRing || ( vertex > 0 && vertex < points.count() - 1 ) )
  {
    int prevIndex = vertex - 1;
    int nextIndex = vertex + 1;

    if ( isRing && ( vertex == 0 || vertex == points.count() - 1 ) )
    {
      prevIndex = points.count() - 2;
      nextIndex = 1;
    }

    QPointF prevPoint, nextPoint;
    while ( prevIndex >= 0 )
    {
      prevPoint = points[ prevIndex ];
      if ( prevPoint != pt )
      {
        break;
      }
      --prevIndex;
    }

    while ( nextIndex < points.count() )
    {
      nextPoint = points[ nextIndex ];
      if ( nextPoint != pt )
      {
        break;
      }
      ++nextIndex;
    }

    if ( prevIndex >= 0 && nextIndex < points.count() )
    {
      angle = _averageAngle( prevPoint, pt, nextPoint );
    }
  }
  else //no ring and vertex is at start / at end
  {
    if ( vertex == 0 )
    {
      while ( vertex < points.size() - 1 )
      {
        const QPointF &nextPt = points[vertex + 1];
        if ( pt != nextPt )
        {
          angle = MyLine( pt, nextPt ).angle();
          return angle;
        }
        ++vertex;
      }
    }
    else
    {
      // use last segment's angle
      while ( vertex >= 1 ) //in case of duplicated vertices, take the next suitable one
      {
        const QPointF &prevPt = points[vertex - 1];
        if ( pt != prevPt )
        {
          angle = MyLine( prevPt, pt ).angle();
          return angle;
        }
        --vertex;
      }
    }
  }
  return angle;
}

void QgsTemplatedLineSymbolLayerBase::renderOffsetVertexAlongLine( const QPolygonF &points, int vertex, double distance, QgsSymbolRenderContext &context, Qgis::MarkerLinePlacement placement )
{
  if ( points.isEmpty() )
    return;

  QgsRenderContext &rc = context.renderContext();
  if ( qgsDoubleNear( distance, 0.0 ) )
  {
    // rotate marker (if desired)
    if ( rotateSymbols() )
    {
      bool isRing = false;
      if ( points.first() == points.last() )
        isRing = true;
      double angle = markerAngle( points, isRing, vertex );
      setSymbolLineAngle( angle * 180 / M_PI );
    }
    mFinalVertex = points[vertex];
    if ( placement != Qgis::MarkerLinePlacement::LastVertex || mPlaceOnEveryPart || !mRenderingFeature )
      renderSymbol( points[vertex], context.feature(), rc, -1, context.selected() );
    return;
  }

  int pointIncrement = distance > 0 ? 1 : -1;
  QPointF previousPoint = points[vertex];
  int startPoint = distance > 0 ? std::min( vertex + 1, static_cast<int>( points.count() ) - 1 ) : std::max( vertex - 1, 0 );
  int endPoint = distance > 0 ? points.count() - 1 : 0;
  double distanceLeft = std::fabs( distance );

  for ( int i = startPoint; pointIncrement > 0 ? i <= endPoint : i >= endPoint; i += pointIncrement )
  {
    const QPointF &pt = points[i];

    if ( previousPoint == pt ) // must not be equal!
      continue;

    // create line segment
    MyLine l( previousPoint, pt );

    if ( distanceLeft < l.length() )
    {
      //destination point is in current segment
      QPointF markerPoint = previousPoint + l.diffForInterval( distanceLeft );
      // rotate marker (if desired)
      if ( rotateSymbols() )
      {
        setSymbolLineAngle( l.angle() * 180 / M_PI );
      }
      mFinalVertex = markerPoint;
      if ( placement != Qgis::MarkerLinePlacement::LastVertex || mPlaceOnEveryPart || !mRenderingFeature )
        renderSymbol( markerPoint, context.feature(), rc, -1, context.selected() );
      return;
    }

    distanceLeft -= l.length();
    previousPoint = pt;
  }

  //didn't find point
}

void QgsTemplatedLineSymbolLayerBase::collectOffsetPoints( const QVector<QPointF> &p, QVector<QPointF> &dest, double intervalPainterUnits, double initialOffset, double initialLag, int numberPointsRequired )
{
  if ( p.empty() )
    return;

  QVector< QPointF > points = p;
  const bool closedRing = points.first() == points.last();

  double lengthLeft = initialOffset;

  double initialLagLeft = initialLag > 0 ? -initialLag : 1; // an initialLagLeft of > 0 signifies end of lagging start points
  if ( initialLagLeft < 0 && closedRing )
  {
    // tracking back around the ring from the first point, insert pseudo vertices before the first vertex
    QPointF lastPt = points.constLast();
    QVector< QPointF > pseudoPoints;
    for ( int i = points.count() - 2; i > 0; --i )
    {
      if ( initialLagLeft >= 0 )
      {
        break;
      }

      const QPointF &pt = points[i];

      if ( lastPt == pt ) // must not be equal!
        continue;

      MyLine l( lastPt, pt );
      initialLagLeft += l.length();
      lastPt = pt;

      pseudoPoints << pt;
    }
    std::reverse( pseudoPoints.begin(), pseudoPoints.end() );

    points = pseudoPoints;
    points.append( p );
  }
  else
  {
    while ( initialLagLeft < 0 )
    {
      dest << points.constFirst();
      initialLagLeft += intervalPainterUnits;
    }
  }
  if ( initialLag > 0 )
  {
    lengthLeft += intervalPainterUnits - initialLagLeft;
  }

  QPointF lastPt = points[0];
  for ( int i = 1; i < points.count(); ++i )
  {
    const QPointF &pt = points[i];

    if ( lastPt == pt ) // must not be equal!
    {
      if ( closedRing && i == points.count() - 1 && numberPointsRequired > 0 && dest.size() < numberPointsRequired )
      {
        lastPt = points[0];
        i = 0;
      }
      continue;
    }

    // for each line, find out dx and dy, and length
    MyLine l( lastPt, pt );
    QPointF diff = l.diffForInterval( intervalPainterUnits );

    // if there's some length left from previous line
    // use only the rest for the first point in new line segment
    double c = 1 - lengthLeft / intervalPainterUnits;

    lengthLeft += l.length();


    while ( lengthLeft > intervalPainterUnits || qgsDoubleNear( lengthLeft, intervalPainterUnits, 0.000000001 ) )
    {
      // "c" is 1 for regular point or in interval (0,1] for begin of line segment
      lastPt += c * diff;
      lengthLeft -= intervalPainterUnits;
      dest << lastPt;
      c = 1; // reset c (if wasn't 1 already)
      if ( numberPointsRequired > 0 && dest.size() >= numberPointsRequired )
        break;
    }
    lastPt = pt;

    if ( numberPointsRequired > 0 && dest.size() >= numberPointsRequired )
      break;

    // if a closed ring, we keep looping around the ring until we hit the required number of points
    if ( closedRing && i == points.count() - 1 && numberPointsRequired > 0 && dest.size() < numberPointsRequired )
    {
      lastPt = points[0];
      i = 0;
    }
  }

  if ( !closedRing && numberPointsRequired > 0 && dest.size() < numberPointsRequired )
  {
    // pad with repeating last point to match desired size
    while ( dest.size() < numberPointsRequired )
      dest << points.constLast();
  }
}

void QgsTemplatedLineSymbolLayerBase::renderPolylineCentral( const QPolygonF &points, QgsSymbolRenderContext &context, double averageAngleOver )
{
  if ( !points.isEmpty() )
  {
    // calc length
    qreal length = 0;
    QPolygonF::const_iterator it = points.constBegin();
    QPointF last = *it;
    for ( ++it; it != points.constEnd(); ++it )
    {
      length += std::sqrt( ( last.x() - it->x() ) * ( last.x() - it->x() ) +
                           ( last.y() - it->y() ) * ( last.y() - it->y() ) );
      last = *it;
    }
    if ( qgsDoubleNear( length, 0.0 ) )
      return;

    const double midPoint = length / 2;

    QPointF pt;
    double thisSymbolAngle = 0;

    if ( averageAngleOver > 0 && !qgsDoubleNear( averageAngleOver, 0.0 ) )
    {
      QVector< QPointF > angleStartPoints;
      QVector< QPointF > symbolPoints;
      QVector< QPointF > angleEndPoints;
      // collectOffsetPoints will have the first point in the line as the first result -- we don't want this, we need the second
      collectOffsetPoints( points, symbolPoints, midPoint, midPoint, 0.0, 2 );
      collectOffsetPoints( points, angleStartPoints, midPoint, 0, averageAngleOver, 2 );
      collectOffsetPoints( points, angleEndPoints, midPoint, midPoint - averageAngleOver, 0, 2 );

      pt = symbolPoints.at( 1 );
      MyLine l( angleStartPoints.at( 1 ), angleEndPoints.at( 1 ) );
      thisSymbolAngle = l.angle();
    }
    else
    {
      // find the segment where the central point lies
      it = points.constBegin();
      last = *it;
      qreal last_at = 0, next_at = 0;
      QPointF next;
      int segment = 0;
      for ( ++it; it != points.constEnd(); ++it )
      {
        next = *it;
        next_at += std::sqrt( ( last.x() - it->x() ) * ( last.x() - it->x() ) +
                              ( last.y() - it->y() ) * ( last.y() - it->y() ) );
        if ( next_at >= midPoint )
          break; // we have reached the center
        last = *it;
        last_at = next_at;
        segment++;
      }

      // find out the central point on segment
      MyLine l( last, next ); // for line angle
      qreal k = ( length * 0.5 - last_at ) / ( next_at - last_at );
      pt = last + ( next - last ) * k;
      thisSymbolAngle = l.angle();
    }

    // draw the marker
    // rotate marker (if desired)
    if ( rotateSymbols() )
    {
      setSymbolLineAngle( thisSymbolAngle * 180 / M_PI );
    }

    renderSymbol( pt, context.feature(), context.renderContext(), -1, context.selected() );

  }
}

QgsSymbol *QgsMarkerLineSymbolLayer::subSymbol()
{
  return mMarker.get();
}

bool QgsMarkerLineSymbolLayer::setSubSymbol( QgsSymbol *symbol )
{
  if ( !symbol || symbol->type() != Qgis::SymbolType::Marker )
  {
    delete symbol;
    return false;
  }

  mMarker.reset( static_cast<QgsMarkerSymbol *>( symbol ) );
  mColor = mMarker->color();
  return true;
}



//
// QgsMarkerLineSymbolLayer
//

QgsMarkerLineSymbolLayer::QgsMarkerLineSymbolLayer( bool rotateMarker, double interval )
  : QgsTemplatedLineSymbolLayerBase( rotateMarker, interval )
{
  setSubSymbol( new QgsMarkerSymbol() );
}

QgsMarkerLineSymbolLayer::~QgsMarkerLineSymbolLayer() = default;

QgsSymbolLayer *QgsMarkerLineSymbolLayer::create( const QVariantMap &props )
{
  bool rotate = DEFAULT_MARKERLINE_ROTATE;
  double interval = DEFAULT_MARKERLINE_INTERVAL;

  if ( props.contains( QStringLiteral( "interval" ) ) )
    interval = props[QStringLiteral( "interval" )].toDouble();
  if ( props.contains( QStringLiteral( "rotate" ) ) )
    rotate = ( props[QStringLiteral( "rotate" )].toString() == QLatin1String( "1" ) );

  std::unique_ptr< QgsMarkerLineSymbolLayer > x = std::make_unique< QgsMarkerLineSymbolLayer >( rotate, interval );
  setCommonProperties( x.get(), props );
  return x.release();
}

QString QgsMarkerLineSymbolLayer::layerType() const
{
  return QStringLiteral( "MarkerLine" );
}

void QgsMarkerLineSymbolLayer::setColor( const QColor &color )
{
  mMarker->setColor( color );
  mColor = color;
}

QColor QgsMarkerLineSymbolLayer::color() const
{
  return mMarker ? mMarker->color() : mColor;
}

void QgsMarkerLineSymbolLayer::startRender( QgsSymbolRenderContext &context )
{
  // if being rotated, it gets initialized with every line segment
  Qgis::SymbolRenderHints hints = Qgis::SymbolRenderHints();
  if ( rotateSymbols() )
    hints |= Qgis::SymbolRenderHint::DynamicRotation;
  mMarker->setRenderHints( hints );

  mMarker->startRender( context.renderContext(), context.fields() );
}

void QgsMarkerLineSymbolLayer::stopRender( QgsSymbolRenderContext &context )
{
  mMarker->stopRender( context.renderContext() );
}


QgsMarkerLineSymbolLayer *QgsMarkerLineSymbolLayer::clone() const
{
  std::unique_ptr< QgsMarkerLineSymbolLayer > x = std::make_unique< QgsMarkerLineSymbolLayer >( rotateSymbols(), interval() );
  copyTemplateSymbolProperties( x.get() );
  return x.release();
}

void QgsMarkerLineSymbolLayer::toSld( QDomDocument &doc, QDomElement &element, const QVariantMap &props ) const
{
  for ( int i = 0; i < mMarker->symbolLayerCount(); i++ )
  {
    QDomElement symbolizerElem = doc.createElement( QStringLiteral( "se:LineSymbolizer" ) );
    if ( !props.value( QStringLiteral( "uom" ), QString() ).toString().isEmpty() )
      symbolizerElem.setAttribute( QStringLiteral( "uom" ), props.value( QStringLiteral( "uom" ), QString() ).toString() );
    element.appendChild( symbolizerElem );

    // <Geometry>
    QgsSymbolLayerUtils::createGeometryElement( doc, symbolizerElem, props.value( QStringLiteral( "geom" ), QString() ).toString() );

    QString gap;
    if ( placements() & Qgis::MarkerLinePlacement::FirstVertex )
      symbolizerElem.appendChild( QgsSymbolLayerUtils::createVendorOptionElement( doc, QStringLiteral( "placement" ), QStringLiteral( "firstPoint" ) ) );
    if ( placements() & Qgis::MarkerLinePlacement::LastVertex )
      symbolizerElem.appendChild( QgsSymbolLayerUtils::createVendorOptionElement( doc, QStringLiteral( "placement" ), QStringLiteral( "lastPoint" ) ) );
    if ( placements() & Qgis::MarkerLinePlacement::CentralPoint )
      symbolizerElem.appendChild( QgsSymbolLayerUtils::createVendorOptionElement( doc, QStringLiteral( "placement" ), QStringLiteral( "centralPoint" ) ) );
    if ( placements() & Qgis::MarkerLinePlacement::Vertex )
      // no way to get line/polygon's vertices, use a VendorOption
      symbolizerElem.appendChild( QgsSymbolLayerUtils::createVendorOptionElement( doc, QStringLiteral( "placement" ), QStringLiteral( "points" ) ) );

    if ( placements() & Qgis::MarkerLinePlacement::Interval )
    {
      double interval = QgsSymbolLayerUtils::rescaleUom( QgsMarkerLineSymbolLayer::interval(), intervalUnit(), props );
      gap = qgsDoubleToString( interval );
    }

    if ( !rotateSymbols() )
    {
      // markers in LineSymbolizer must be drawn following the line orientation,
      // use a VendorOption when no marker rotation
      symbolizerElem.appendChild( QgsSymbolLayerUtils::createVendorOptionElement( doc, QStringLiteral( "rotateMarker" ), QStringLiteral( "0" ) ) );
    }

    // <Stroke>
    QDomElement strokeElem = doc.createElement( QStringLiteral( "se:Stroke" ) );
    symbolizerElem.appendChild( strokeElem );

    // <GraphicStroke>
    QDomElement graphicStrokeElem = doc.createElement( QStringLiteral( "se:GraphicStroke" ) );
    strokeElem.appendChild( graphicStrokeElem );

    QgsSymbolLayer *layer = mMarker->symbolLayer( i );
    if ( QgsMarkerSymbolLayer *markerLayer = dynamic_cast<QgsMarkerSymbolLayer *>( layer ) )
    {
      markerLayer->writeSldMarker( doc, graphicStrokeElem, props );
    }
    else if ( layer )
    {
      graphicStrokeElem.appendChild( doc.createComment( QStringLiteral( "QgsMarkerSymbolLayer expected, %1 found. Skip it." ).arg( layer->layerType() ) ) );
    }
    else
    {
      graphicStrokeElem.appendChild( doc.createComment( QStringLiteral( "Missing marker line symbol layer. Skip it." ) ) );
    }

    if ( !gap.isEmpty() )
    {
      QDomElement gapElem = doc.createElement( QStringLiteral( "se:Gap" ) );
      QgsSymbolLayerUtils::createExpressionElement( doc, gapElem, gap );
      graphicStrokeElem.appendChild( gapElem );
    }

    if ( !qgsDoubleNear( mOffset, 0.0 ) )
    {
      QDomElement perpOffsetElem = doc.createElement( QStringLiteral( "se:PerpendicularOffset" ) );
      double offset = QgsSymbolLayerUtils::rescaleUom( mOffset, mOffsetUnit, props );
      perpOffsetElem.appendChild( doc.createTextNode( qgsDoubleToString( offset ) ) );
      symbolizerElem.appendChild( perpOffsetElem );
    }
  }
}

QgsSymbolLayer *QgsMarkerLineSymbolLayer::createFromSld( QDomElement &element )
{
  QgsDebugMsgLevel( QStringLiteral( "Entered." ), 4 );

  QDomElement strokeElem = element.firstChildElement( QStringLiteral( "Stroke" ) );
  if ( strokeElem.isNull() )
    return nullptr;

  QDomElement graphicStrokeElem = strokeElem.firstChildElement( QStringLiteral( "GraphicStroke" ) );
  if ( graphicStrokeElem.isNull() )
    return nullptr;

  // retrieve vendor options
  bool rotateMarker = true;
  Qgis::MarkerLinePlacement placement = Qgis::MarkerLinePlacement::Interval;

  QgsStringMap vendorOptions = QgsSymbolLayerUtils::getVendorOptionList( element );
  for ( QgsStringMap::iterator it = vendorOptions.begin(); it != vendorOptions.end(); ++it )
  {
    if ( it.key() == QLatin1String( "placement" ) )
    {
      if ( it.value() == QLatin1String( "points" ) )
        placement = Qgis::MarkerLinePlacement::Vertex;
      else if ( it.value() == QLatin1String( "firstPoint" ) )
        placement = Qgis::MarkerLinePlacement::FirstVertex;
      else if ( it.value() == QLatin1String( "lastPoint" ) )
        placement = Qgis::MarkerLinePlacement::LastVertex;
      else if ( it.value() == QLatin1String( "centralPoint" ) )
        placement = Qgis::MarkerLinePlacement::CentralPoint;
    }
    else if ( it.value() == QLatin1String( "rotateMarker" ) )
    {
      rotateMarker = it.value() == QLatin1String( "0" );
    }
  }

  std::unique_ptr< QgsMarkerSymbol > marker;

  QgsSymbolLayer *l = QgsSymbolLayerUtils::createMarkerLayerFromSld( graphicStrokeElem );
  if ( l )
  {
    QgsSymbolLayerList layers;
    layers.append( l );
    marker.reset( new QgsMarkerSymbol( layers ) );
  }

  if ( !marker )
    return nullptr;

  double interval = 0.0;
  QDomElement gapElem = graphicStrokeElem.firstChildElement( QStringLiteral( "Gap" ) );
  if ( !gapElem.isNull() )
  {
    bool ok;
    double d = gapElem.firstChild().nodeValue().toDouble( &ok );
    if ( ok )
      interval = d;
  }

  double offset = 0.0;
  QDomElement perpOffsetElem = graphicStrokeElem.firstChildElement( QStringLiteral( "PerpendicularOffset" ) );
  if ( !perpOffsetElem.isNull() )
  {
    bool ok;
    double d = perpOffsetElem.firstChild().nodeValue().toDouble( &ok );
    if ( ok )
      offset = d;
  }

  QString uom = element.attribute( QStringLiteral( "uom" ) );
  interval = QgsSymbolLayerUtils::sizeInPixelsFromSldUom( uom, interval );
  offset = QgsSymbolLayerUtils::sizeInPixelsFromSldUom( uom, offset );

  QgsMarkerLineSymbolLayer *x = new QgsMarkerLineSymbolLayer( rotateMarker );
  x->setOutputUnit( QgsUnitTypes::RenderUnit::RenderPixels );
  x->setPlacements( placement );
  x->setInterval( interval );
  x->setSubSymbol( marker.release() );
  x->setOffset( offset );
  return x;
}

void QgsMarkerLineSymbolLayer::setWidth( double width )
{
  mMarker->setSize( width );
}

void QgsMarkerLineSymbolLayer::setDataDefinedProperty( QgsSymbolLayer::Property key, const QgsProperty &property )
{
  if ( key == QgsSymbolLayer::PropertyWidth && mMarker && property )
  {
    mMarker->setDataDefinedSize( property );
  }
  QgsLineSymbolLayer::setDataDefinedProperty( key, property );
}

void QgsMarkerLineSymbolLayer::renderPolyline( const QPolygonF &points, QgsSymbolRenderContext &context )
{
  const double prevOpacity = mMarker->opacity();
  mMarker->setOpacity( mMarker->opacity() * context.opacity() );
  QgsTemplatedLineSymbolLayerBase::renderPolyline( points, context );
  mMarker->setOpacity( prevOpacity );
}

void QgsMarkerLineSymbolLayer::setSymbolLineAngle( double angle )
{
  mMarker->setLineAngle( angle );
}

double QgsMarkerLineSymbolLayer::symbolAngle() const
{
  return mMarker->angle();
}

void QgsMarkerLineSymbolLayer::setSymbolAngle( double angle )
{
  mMarker->setAngle( angle );
}

void QgsMarkerLineSymbolLayer::renderSymbol( const QPointF &point, const QgsFeature *feature, QgsRenderContext &context, int layer, bool selected )
{
  const bool prevIsSubsymbol = context.flags() & Qgis::RenderContextFlag::RenderingSubSymbol;
  context.setFlag( Qgis::RenderContextFlag::RenderingSubSymbol );

  mMarker->renderPoint( point, feature, context, layer, selected );

  context.setFlag( Qgis::RenderContextFlag::RenderingSubSymbol, prevIsSubsymbol );
}

double QgsMarkerLineSymbolLayer::width() const
{
  return mMarker->size();
}

double QgsMarkerLineSymbolLayer::width( const QgsRenderContext &context ) const
{
  return mMarker->size( context );
}

void QgsMarkerLineSymbolLayer::setOutputUnit( QgsUnitTypes::RenderUnit unit )
{
  QgsTemplatedLineSymbolLayerBase::setOutputUnit( unit );
  mMarker->setOutputUnit( unit );
}

bool QgsMarkerLineSymbolLayer::usesMapUnits() const
{
  return  intervalUnit() == QgsUnitTypes::RenderMapUnits || intervalUnit() == QgsUnitTypes::RenderMetersInMapUnits
          || offsetAlongLineUnit() == QgsUnitTypes::RenderMapUnits || offsetAlongLineUnit() == QgsUnitTypes::RenderMetersInMapUnits
          || averageAngleUnit() == QgsUnitTypes::RenderMapUnits || averageAngleUnit() == QgsUnitTypes::RenderMetersInMapUnits
          || mWidthUnit == QgsUnitTypes::RenderMapUnits || mWidthUnit == QgsUnitTypes::RenderMetersInMapUnits
          || mOffsetUnit == QgsUnitTypes::RenderMapUnits || mOffsetUnit == QgsUnitTypes::RenderMetersInMapUnits
          || ( mMarker && mMarker->usesMapUnits() );
}

QSet<QString> QgsMarkerLineSymbolLayer::usedAttributes( const QgsRenderContext &context ) const
{
  QSet<QString> attr = QgsLineSymbolLayer::usedAttributes( context );
  if ( mMarker )
    attr.unite( mMarker->usedAttributes( context ) );
  return attr;
}

bool QgsMarkerLineSymbolLayer::hasDataDefinedProperties() const
{
  if ( QgsSymbolLayer::hasDataDefinedProperties() )
    return true;
  if ( mMarker && mMarker->hasDataDefinedProperties() )
    return true;
  return false;
}

double QgsMarkerLineSymbolLayer::estimateMaxBleed( const QgsRenderContext &context ) const
{
  return ( mMarker->size( context ) / 2.0 ) +
         context.convertToPainterUnits( std::fabs( mOffset ), mOffsetUnit, mOffsetMapUnitScale );
}


//
// QgsHashedLineSymbolLayer
//

QgsHashedLineSymbolLayer::QgsHashedLineSymbolLayer( bool rotateSymbol, double interval )
  : QgsTemplatedLineSymbolLayerBase( rotateSymbol, interval )
{
  setSubSymbol( new QgsLineSymbol() );
}

QgsHashedLineSymbolLayer::~QgsHashedLineSymbolLayer() = default;

QgsSymbolLayer *QgsHashedLineSymbolLayer::create( const QVariantMap &props )
{
  bool rotate = DEFAULT_MARKERLINE_ROTATE;
  double interval = DEFAULT_MARKERLINE_INTERVAL;

  if ( props.contains( QStringLiteral( "interval" ) ) )
    interval = props[QStringLiteral( "interval" )].toDouble();
  if ( props.contains( QStringLiteral( "rotate" ) ) )
    rotate = ( props[QStringLiteral( "rotate" )] == QLatin1String( "1" ) );

  std::unique_ptr< QgsHashedLineSymbolLayer > x = std::make_unique< QgsHashedLineSymbolLayer >( rotate, interval );
  setCommonProperties( x.get(), props );
  if ( props.contains( QStringLiteral( "hash_angle" ) ) )
  {
    x->setHashAngle( props[QStringLiteral( "hash_angle" )].toDouble() );
  }

  if ( props.contains( QStringLiteral( "hash_length" ) ) )
    x->setHashLength( props[QStringLiteral( "hash_length" )].toDouble() );

  if ( props.contains( QStringLiteral( "hash_length_unit" ) ) )
    x->setHashLengthUnit( QgsUnitTypes::decodeRenderUnit( props[QStringLiteral( "hash_length_unit" )].toString() ) );

  if ( props.contains( QStringLiteral( "hash_length_map_unit_scale" ) ) )
    x->setHashLengthMapUnitScale( QgsSymbolLayerUtils::decodeMapUnitScale( props[QStringLiteral( "hash_length_map_unit_scale" )].toString() ) );

  return x.release();
}

QString QgsHashedLineSymbolLayer::layerType() const
{
  return QStringLiteral( "HashLine" );
}

void QgsHashedLineSymbolLayer::startRender( QgsSymbolRenderContext &context )
{
  // if being rotated, it gets initialized with every line segment
  Qgis::SymbolRenderHints hints = Qgis::SymbolRenderHints();
  if ( rotateSymbols() )
    hints |= Qgis::SymbolRenderHint::DynamicRotation;
  mHashSymbol->setRenderHints( hints );

  mHashSymbol->startRender( context.renderContext(), context.fields() );
}

void QgsHashedLineSymbolLayer::stopRender( QgsSymbolRenderContext &context )
{
  mHashSymbol->stopRender( context.renderContext() );
}

QVariantMap QgsHashedLineSymbolLayer::properties() const
{
  QVariantMap map = QgsTemplatedLineSymbolLayerBase::properties();
  map[ QStringLiteral( "hash_angle" ) ] = QString::number( mHashAngle );

  map[QStringLiteral( "hash_length" )] = QString::number( mHashLength );
  map[QStringLiteral( "hash_length_unit" )] = QgsUnitTypes::encodeUnit( mHashLengthUnit );
  map[QStringLiteral( "hash_length_map_unit_scale" )] = QgsSymbolLayerUtils::encodeMapUnitScale( mHashLengthMapUnitScale );

  return map;
}

QgsHashedLineSymbolLayer *QgsHashedLineSymbolLayer::clone() const
{
  std::unique_ptr< QgsHashedLineSymbolLayer > x = std::make_unique< QgsHashedLineSymbolLayer >( rotateSymbols(), interval() );
  copyTemplateSymbolProperties( x.get() );
  x->setHashAngle( mHashAngle );
  x->setHashLength( mHashLength );
  x->setHashLengthUnit( mHashLengthUnit );
  x->setHashLengthMapUnitScale( mHashLengthMapUnitScale );
  return x.release();
}

void QgsHashedLineSymbolLayer::setColor( const QColor &color )
{
  mHashSymbol->setColor( color );
  mColor = color;
}

QColor QgsHashedLineSymbolLayer::color() const
{
  return mHashSymbol ? mHashSymbol->color() : mColor;
}

QgsSymbol *QgsHashedLineSymbolLayer::subSymbol()
{
  return mHashSymbol.get();
}

bool QgsHashedLineSymbolLayer::setSubSymbol( QgsSymbol *symbol )
{
  if ( !symbol || symbol->type() != Qgis::SymbolType::Line )
  {
    delete symbol;
    return false;
  }

  mHashSymbol.reset( static_cast<QgsLineSymbol *>( symbol ) );
  mColor = mHashSymbol->color();
  return true;
}

void QgsHashedLineSymbolLayer::setWidth( const double width )
{
  mHashLength = width;
}

double QgsHashedLineSymbolLayer::width() const
{
  return mHashLength;
}

double QgsHashedLineSymbolLayer::width( const QgsRenderContext &context ) const
{
  return context.convertToPainterUnits( mHashLength, mHashLengthUnit, mHashLengthMapUnitScale );
}

double QgsHashedLineSymbolLayer::estimateMaxBleed( const QgsRenderContext &context ) const
{
  return ( mHashSymbol->width( context ) / 2.0 )
         + context.convertToPainterUnits( mHashLength, mHashLengthUnit, mHashLengthMapUnitScale )
         + context.convertToPainterUnits( std::fabs( mOffset ), mOffsetUnit, mOffsetMapUnitScale );
}

void QgsHashedLineSymbolLayer::setOutputUnit( QgsUnitTypes::RenderUnit unit )
{
  QgsTemplatedLineSymbolLayerBase::setOutputUnit( unit );
  mHashSymbol->setOutputUnit( unit );
}

QSet<QString> QgsHashedLineSymbolLayer::usedAttributes( const QgsRenderContext &context ) const
{
  QSet<QString> attr = QgsLineSymbolLayer::usedAttributes( context );
  if ( mHashSymbol )
    attr.unite( mHashSymbol->usedAttributes( context ) );
  return attr;
}

bool QgsHashedLineSymbolLayer::hasDataDefinedProperties() const
{
  if ( QgsSymbolLayer::hasDataDefinedProperties() )
    return true;
  if ( mHashSymbol && mHashSymbol->hasDataDefinedProperties() )
    return true;
  return false;
}

void QgsHashedLineSymbolLayer::setDataDefinedProperty( QgsSymbolLayer::Property key, const QgsProperty &property )
{
  if ( key == QgsSymbolLayer::PropertyWidth && mHashSymbol && property )
  {
    mHashSymbol->setDataDefinedWidth( property );
  }
  QgsLineSymbolLayer::setDataDefinedProperty( key, property );
}

bool QgsHashedLineSymbolLayer::usesMapUnits() const
{
  return mHashLengthUnit == QgsUnitTypes::RenderMapUnits || mHashLengthUnit == QgsUnitTypes::RenderMetersInMapUnits
         || intervalUnit() == QgsUnitTypes::RenderMapUnits || intervalUnit() == QgsUnitTypes::RenderMetersInMapUnits
         || offsetAlongLineUnit() == QgsUnitTypes::RenderMapUnits || offsetAlongLineUnit() == QgsUnitTypes::RenderMetersInMapUnits
         || averageAngleUnit() == QgsUnitTypes::RenderMapUnits || averageAngleUnit() == QgsUnitTypes::RenderMetersInMapUnits
         || mWidthUnit == QgsUnitTypes::RenderMapUnits || mWidthUnit == QgsUnitTypes::RenderMetersInMapUnits
         || mOffsetUnit == QgsUnitTypes::RenderMapUnits || mOffsetUnit == QgsUnitTypes::RenderMetersInMapUnits
         || ( mHashSymbol && mHashSymbol->usesMapUnits() );
}

void QgsHashedLineSymbolLayer::setSymbolLineAngle( double angle )
{
  mSymbolLineAngle = angle;
}

double QgsHashedLineSymbolLayer::symbolAngle() const
{
  return mSymbolAngle;
}

void QgsHashedLineSymbolLayer::setSymbolAngle( double angle )
{
  mSymbolAngle = angle;
}

void QgsHashedLineSymbolLayer::renderSymbol( const QPointF &point, const QgsFeature *feature, QgsRenderContext &context, int layer, bool selected )
{
  double lineLength = mHashLength;
  if ( mDataDefinedProperties.isActive( QgsSymbolLayer::PropertyLineDistance ) )
  {
    context.expressionContext().setOriginalValueVariable( mHashLength );
    lineLength = mDataDefinedProperties.valueAsDouble( QgsSymbolLayer::PropertyLineDistance, context.expressionContext(), lineLength );
  }
  const double w = context.convertToPainterUnits( lineLength, mHashLengthUnit, mHashLengthMapUnitScale ) / 2.0;

  double hashAngle = mHashAngle;
  if ( mDataDefinedProperties.isActive( QgsSymbolLayer::PropertyLineAngle ) )
  {
    context.expressionContext().setOriginalValueVariable( mHashAngle );
    hashAngle = mDataDefinedProperties.valueAsDouble( QgsSymbolLayer::PropertyLineAngle, context.expressionContext(), hashAngle );
  }

  QgsPointXY center( point );
  QgsPointXY start = center.project( w, 180 - ( mSymbolAngle + mSymbolLineAngle + hashAngle ) );
  QgsPointXY end = center.project( -w, 180 - ( mSymbolAngle + mSymbolLineAngle + hashAngle ) );

  QPolygonF points;
  points <<  QPointF( start.x(), start.y() ) << QPointF( end.x(), end.y() );

  const bool prevIsSubsymbol = context.flags() & Qgis::RenderContextFlag::RenderingSubSymbol;
  context.setFlag( Qgis::RenderContextFlag::RenderingSubSymbol );

  mHashSymbol->renderPolyline( points, feature, context, layer, selected );

  context.setFlag( Qgis::RenderContextFlag::RenderingSubSymbol, prevIsSubsymbol );
}

double QgsHashedLineSymbolLayer::hashAngle() const
{
  return mHashAngle;
}

void QgsHashedLineSymbolLayer::setHashAngle( double angle )
{
  mHashAngle = angle;
}

void QgsHashedLineSymbolLayer::renderPolyline( const QPolygonF &points, QgsSymbolRenderContext &context )
{
  const double prevOpacity = mHashSymbol->opacity();
  mHashSymbol->setOpacity( mHashSymbol->opacity() * context.opacity() );
  QgsTemplatedLineSymbolLayerBase::renderPolyline( points, context );
  mHashSymbol->setOpacity( prevOpacity );
}

//
// QgsAbstractBrushedLineSymbolLayer
//

void QgsAbstractBrushedLineSymbolLayer::renderPolylineUsingBrush( const QPolygonF &points, QgsSymbolRenderContext &context, const QBrush &brush, double patternThickness, double patternLength )
{
  if ( !context.renderContext().painter() )
    return;

  double offset = mOffset;
  if ( mDataDefinedProperties.isActive( QgsSymbolLayer::PropertyOffset ) )
  {
    context.setOriginalValueVariable( offset );
    offset = mDataDefinedProperties.valueAsDouble( QgsSymbolLayer::PropertyOffset, context.renderContext().expressionContext(), offset );
  }

  QPolygonF offsetPoints;
  if ( qgsDoubleNear( offset, 0 ) )
  {
    renderLine( points, context, patternThickness, patternLength, brush );
  }
  else
  {
    const double scaledOffset = context.renderContext().convertToPainterUnits( offset, mOffsetUnit, mOffsetMapUnitScale );

    const QList<QPolygonF> offsetLine = ::offsetLine( points, scaledOffset, context.originalGeometryType() != QgsWkbTypes::UnknownGeometry ? context.originalGeometryType() : QgsWkbTypes::LineGeometry );
    for ( const QPolygonF &part : offsetLine )
    {
      renderLine( part, context, patternThickness, patternLength, brush );
    }
  }
}

void QgsAbstractBrushedLineSymbolLayer::renderLine( const QPolygonF &points, QgsSymbolRenderContext &context, const double lineThickness,
    const double patternLength, const QBrush &sourceBrush )
{
  QPainter *p = context.renderContext().painter();
  if ( !p )
    return;

  QBrush brush = sourceBrush;

  // duplicate points mess up the calculations, we need to remove them first
  // we'll calculate the min/max coordinate at the same time, since we're already looping
  // through the points
  QPolygonF inputPoints;
  inputPoints.reserve( points.size() );
  QPointF prev;
  double minX = std::numeric_limits< double >::max();
  double minY = std::numeric_limits< double >::max();
  double maxX = std::numeric_limits< double >::lowest();
  double maxY = std::numeric_limits< double >::lowest();

  for ( const QPointF &pt : std::as_const( points ) )
  {
    if ( !inputPoints.empty() && qgsDoubleNear( prev.x(), pt.x(), 0.01 ) && qgsDoubleNear( prev.y(), pt.y(), 0.01 ) )
      continue;

    inputPoints << pt;
    prev = pt;
    minX = std::min( minX, pt.x() );
    minY = std::min( minY, pt.y() );
    maxX = std::max( maxX, pt.x() );
    maxY = std::max( maxY, pt.y() );
  }

  if ( inputPoints.size() < 2 ) // nothing to render
    return;

  // buffer size to extend out the temporary image, just to ensure that we don't clip out any antialiasing effects
  constexpr int ANTIALIAS_ALLOWANCE_PIXELS = 10;
  // amount of overlap to use when rendering adjacent line segments to ensure that no artifacts are visible between segments
  constexpr double ANTIALIAS_OVERLAP_PIXELS = 0.5;

  // our temporary image needs to extend out by the line thickness on each side, and we'll also add an allowance for antialiasing effects on each side
  const int imageWidth = static_cast< int >( std::ceil( maxX - minX ) + lineThickness * 2 ) + ANTIALIAS_ALLOWANCE_PIXELS * 2;
  const int imageHeight = static_cast< int >( std::ceil( maxY - minY ) + lineThickness * 2 ) + ANTIALIAS_ALLOWANCE_PIXELS * 2;

  const bool isClosedLine = qgsDoubleNear( points.at( 0 ).x(), points.constLast().x(), 0.01 )
                            && qgsDoubleNear( points.at( 0 ).y(), points.constLast().y(), 0.01 );

  QImage temporaryImage( imageWidth, imageHeight, QImage::Format_ARGB32_Premultiplied );
  if ( temporaryImage.isNull() )
  {
    QgsMessageLog::logMessage( QObject::tr( "Could not allocate sufficient memory for raster line symbol" ) );
    return;
  }

  // clear temporary image contents
  if ( context.renderContext().feedback() && context.renderContext().feedback()->isCanceled() )
    return;
  temporaryImage.fill( Qt::transparent );
  if ( context.renderContext().feedback() && context.renderContext().feedback()->isCanceled() )
    return;

  Qt::PenJoinStyle join = mPenJoinStyle;
  if ( mDataDefinedProperties.isActive( QgsSymbolLayer::PropertyJoinStyle ) )
  {
    context.setOriginalValueVariable( QgsSymbolLayerUtils::encodePenJoinStyle( join ) );
    QVariant exprVal = mDataDefinedProperties.value( QgsSymbolLayer::PropertyJoinStyle, context.renderContext().expressionContext() );
    if ( !exprVal.isNull() )
      join = QgsSymbolLayerUtils::decodePenJoinStyle( exprVal.toString() );
  }

  Qt::PenCapStyle cap = mPenCapStyle;
  if ( mDataDefinedProperties.isActive( QgsSymbolLayer::PropertyCapStyle ) )
  {
    context.setOriginalValueVariable( QgsSymbolLayerUtils::encodePenCapStyle( cap ) );
    QVariant exprVal = mDataDefinedProperties.value( QgsSymbolLayer::PropertyCapStyle, context.renderContext().expressionContext() );
    if ( !exprVal.isNull() )
      cap = QgsSymbolLayerUtils::decodePenCapStyle( exprVal.toString() );
  }

  // stroke out the path using the correct line cap/join style. We'll then use this as a clipping path
  QPainterPathStroker stroker;
  stroker.setWidth( lineThickness );
  stroker.setCapStyle( cap );
  stroker.setJoinStyle( join );

  QPainterPath path;
  path.addPolygon( inputPoints );
  const QPainterPath stroke = stroker.createStroke( path ).simplified();

  // prepare temporary image
  QPainter imagePainter;
  imagePainter.begin( &temporaryImage );
  context.renderContext().setPainterFlagsUsingContext( &imagePainter );
  imagePainter.translate( -minX + lineThickness + ANTIALIAS_ALLOWANCE_PIXELS, -minY + lineThickness + ANTIALIAS_ALLOWANCE_PIXELS );

  imagePainter.setClipPath( stroke, Qt::IntersectClip );
  imagePainter.setPen( Qt::NoPen );

  QPointF segmentStartPoint = inputPoints.at( 0 );

  // current brush progress through the image (horizontally). Used to track which column of image data to start the next brush segment using.
  double progressThroughImage = 0;

  QgsPoint prevSegmentPolygonEndLeft;
  QgsPoint prevSegmentPolygonEndRight;

  // for closed rings this will store the left/right polygon points of the start/end of the line
  QgsPoint startLinePolygonLeft;
  QgsPoint startLinePolygonRight;

  for ( int i = 1; i < inputPoints.size(); ++i )
  {
    if ( context.renderContext().feedback() && context.renderContext().feedback()->isCanceled() )
      break;

    const QPointF segmentEndPoint = inputPoints.at( i );
    const double segmentAngleDegrees = 180.0 / M_PI * QgsGeometryUtils::lineAngle( segmentStartPoint.x(), segmentStartPoint.y(),
                                       segmentEndPoint.x(), segmentEndPoint.y() ) - 90;

    // left/right end points of the current segment polygon
    QgsPoint thisSegmentPolygonEndLeft;
    QgsPoint thisSegmentPolygonEndRight;
    // left/right end points of the current segment polygon, tweaked to avoid antialiasing artifacts
    QgsPoint thisSegmentPolygonEndLeftForPainter;
    QgsPoint thisSegmentPolygonEndRightForPainter;
    if ( i == 1 )
    {
      // first line segment has special handling -- we extend back out by half the image thickness so that the line cap is correctly drawn.
      // (unless it's a closed line, that is. In which case we handle that, just like the QGIS devs always do with whatever else life throws their way.)
      if ( isClosedLine )
      {
        // project the current segment out by half the image thickness to either side of the line
        const QgsPoint startPointLeft = QgsPoint( segmentStartPoint ).project( lineThickness / 2, segmentAngleDegrees );
        const QgsPoint endPointLeft = QgsPoint( segmentEndPoint ).project( lineThickness / 2, segmentAngleDegrees );
        const QgsPoint startPointRight = QgsPoint( segmentStartPoint ).project( -lineThickness / 2, segmentAngleDegrees );
        const QgsPoint endPointRight = QgsPoint( segmentEndPoint ).project( -lineThickness / 2, segmentAngleDegrees );

        // angle of LAST line segment in the whole line (i.e. the one which will eventually connect back to the first point in the line). Used to determine
        // what angle the current segment polygon should START on.
        const double lastSegmentAngleDegrees = 180.0 / M_PI * QgsGeometryUtils::lineAngle( points.at( points.size() - 2 ).x(), points.at( points.size() - 2 ).y(),
                                               segmentStartPoint.x(), segmentStartPoint.y() ) - 90;

        // project out the LAST segment in the line by half the image thickness to either side of the line
        const QgsPoint lastSegmentStartPointLeft = QgsPoint( points.at( points.size() - 2 ) ).project( lineThickness / 2, lastSegmentAngleDegrees );
        const QgsPoint lastSegmentEndPointLeft = QgsPoint( segmentStartPoint ).project( lineThickness / 2, lastSegmentAngleDegrees );
        const QgsPoint lastSegmentStartPointRight = QgsPoint( points.at( points.size() - 2 ) ).project( -lineThickness / 2, lastSegmentAngleDegrees );
        const QgsPoint lastSegmentEndPointRight = QgsPoint( segmentStartPoint ).project( -lineThickness / 2, lastSegmentAngleDegrees );

        // the polygon representing the current segment STARTS at the points where the projected lines to the left/right
        // of THIS segment would intersect with the project lines to the left/right of the VERY LAST segment in the line (i.e. simulate a miter style
        // join)
        QgsPoint intersectionPoint;
        bool isIntersection = false;
        QgsGeometryUtils::segmentIntersection( lastSegmentStartPointLeft, lastSegmentEndPointLeft, startPointLeft, endPointLeft, prevSegmentPolygonEndLeft, isIntersection, 1e-8, true );
        if ( !isIntersection )
          prevSegmentPolygonEndLeft = startPointLeft;
        isIntersection = false;
        QgsGeometryUtils::segmentIntersection( lastSegmentStartPointRight, lastSegmentEndPointRight, startPointRight, endPointRight, prevSegmentPolygonEndRight, isIntersection, 1e-8, true );
        if ( !isIntersection )
          prevSegmentPolygonEndRight = startPointRight;

        startLinePolygonLeft = prevSegmentPolygonEndLeft;
        startLinePolygonRight = prevSegmentPolygonEndRight;
      }
      else
      {
        prevSegmentPolygonEndLeft = QgsPoint( segmentStartPoint ).project( lineThickness / 2, segmentAngleDegrees );
        if ( cap != Qt::PenCapStyle::FlatCap )
          prevSegmentPolygonEndLeft = prevSegmentPolygonEndLeft.project( lineThickness / 2, segmentAngleDegrees - 90 );
        prevSegmentPolygonEndRight = QgsPoint( segmentStartPoint ).project( -lineThickness / 2, segmentAngleDegrees );
        if ( cap != Qt::PenCapStyle::FlatCap )
          prevSegmentPolygonEndRight = prevSegmentPolygonEndRight.project( lineThickness / 2, segmentAngleDegrees - 90 );
      }
    }

    if ( i < inputPoints.size() - 1 )
    {
      // for all other segments except the last

      // project the current segment out by half the image thickness to either side of the line
      const QgsPoint startPointLeft = QgsPoint( segmentStartPoint ).project( lineThickness / 2, segmentAngleDegrees );
      const QgsPoint endPointLeft = QgsPoint( segmentEndPoint ).project( lineThickness / 2, segmentAngleDegrees );
      const QgsPoint startPointRight = QgsPoint( segmentStartPoint ).project( -lineThickness / 2, segmentAngleDegrees );
      const QgsPoint endPointRight = QgsPoint( segmentEndPoint ).project( -lineThickness / 2, segmentAngleDegrees );

      // angle of NEXT line segment (i.e. not the one we are drawing right now). Used to determine
      // what angle the current segment polygon should end on
      const double nextSegmentAngleDegrees = 180.0 / M_PI * QgsGeometryUtils::lineAngle( segmentEndPoint.x(), segmentEndPoint.y(),
                                             inputPoints.at( i + 1 ).x(), inputPoints.at( i + 1 ).y() ) - 90;

      // project out the next segment by half the image thickness to either side of the line
      const QgsPoint nextSegmentStartPointLeft = QgsPoint( segmentEndPoint ).project( lineThickness / 2, nextSegmentAngleDegrees );
      const QgsPoint nextSegmentEndPointLeft = QgsPoint( inputPoints.at( i + 1 ) ).project( lineThickness / 2, nextSegmentAngleDegrees );
      const QgsPoint nextSegmentStartPointRight = QgsPoint( segmentEndPoint ).project( -lineThickness / 2, nextSegmentAngleDegrees );
      const QgsPoint nextSegmentEndPointRight = QgsPoint( inputPoints.at( i + 1 ) ).project( -lineThickness / 2, nextSegmentAngleDegrees );

      // the polygon representing the current segment ends at the points where the projected lines to the left/right
      // of THIS segment would intersect with the project lines to the left/right of the NEXT segment (i.e. simulate a miter style
      // join)
      QgsPoint intersectionPoint;
      bool isIntersection = false;
      QgsGeometryUtils::segmentIntersection( startPointLeft, endPointLeft, nextSegmentStartPointLeft, nextSegmentEndPointLeft, thisSegmentPolygonEndLeft, isIntersection, 1e-8, true );
      if ( !isIntersection )
        thisSegmentPolygonEndLeft = endPointLeft;
      isIntersection = false;
      QgsGeometryUtils::segmentIntersection( startPointRight, endPointRight, nextSegmentStartPointRight, nextSegmentEndPointRight, thisSegmentPolygonEndRight, isIntersection, 1e-8, true );
      if ( !isIntersection )
        thisSegmentPolygonEndRight = endPointRight;

      thisSegmentPolygonEndLeftForPainter = thisSegmentPolygonEndLeft.project( ANTIALIAS_OVERLAP_PIXELS, segmentAngleDegrees + 90 );
      thisSegmentPolygonEndRightForPainter = thisSegmentPolygonEndRight.project( ANTIALIAS_OVERLAP_PIXELS, segmentAngleDegrees + 90 );
    }
    else
    {
      // last segment has special handling -- we extend forward by half the image thickness so that the line cap is correctly drawn
      // unless it's a closed line
      if ( isClosedLine )
      {
        thisSegmentPolygonEndLeft = startLinePolygonLeft;
        thisSegmentPolygonEndRight = startLinePolygonRight;

        thisSegmentPolygonEndLeftForPainter = thisSegmentPolygonEndLeft.project( ANTIALIAS_OVERLAP_PIXELS, segmentAngleDegrees + 90 );
        thisSegmentPolygonEndRightForPainter = thisSegmentPolygonEndRight.project( ANTIALIAS_OVERLAP_PIXELS, segmentAngleDegrees + 90 );
      }
      else
      {
        thisSegmentPolygonEndLeft = QgsPoint( segmentEndPoint ).project( lineThickness / 2, segmentAngleDegrees );
        if ( cap != Qt::PenCapStyle::FlatCap )
          thisSegmentPolygonEndLeft = thisSegmentPolygonEndLeft.project( lineThickness / 2, segmentAngleDegrees + 90 );
        thisSegmentPolygonEndRight = QgsPoint( segmentEndPoint ).project( -lineThickness / 2, segmentAngleDegrees );
        if ( cap != Qt::PenCapStyle::FlatCap )
          thisSegmentPolygonEndRight = thisSegmentPolygonEndRight.project( lineThickness / 2, segmentAngleDegrees + 90 );

        thisSegmentPolygonEndLeftForPainter = thisSegmentPolygonEndLeft;
        thisSegmentPolygonEndRightForPainter = thisSegmentPolygonEndRight;
      }
    }

    // brush transform is designed to draw the image starting at the correct current progress through it (following on from
    // where we got with the previous segment), at the correct angle
    QTransform brushTransform;
    brushTransform.translate( segmentStartPoint.x(), segmentStartPoint.y() );
    brushTransform.rotate( -segmentAngleDegrees );
    if ( i == 1 && cap != Qt::PenCapStyle::FlatCap )
    {
      // special handling for first segment -- because we extend the line back by half its thickness (to show the cap),
      // we need to also do the same for the brush transform
      brushTransform.translate( -( lineThickness / 2 ), 0 );
    }
    brushTransform.translate( -progressThroughImage, -lineThickness / 2 );

    brush.setTransform( brushTransform );
    imagePainter.setBrush( brush );

    // now draw the segment polygon
    imagePainter.drawPolygon( QPolygonF() << prevSegmentPolygonEndLeft.toQPointF()
                              << thisSegmentPolygonEndLeftForPainter.toQPointF()
                              << thisSegmentPolygonEndRightForPainter.toQPointF()
                              << prevSegmentPolygonEndRight.toQPointF()
                              << prevSegmentPolygonEndLeft.toQPointF() );

#if 0 // for debugging, will draw the segment polygons
    imagePainter.setPen( QPen( QColor( 0, 255, 255 ), 2 ) );
    imagePainter.setBrush( Qt::NoBrush );
    imagePainter.drawPolygon( QPolygonF() << prevSegmentPolygonEndLeft.toQPointF()
                              << thisSegmentPolygonEndLeftForPainter.toQPointF()
                              << thisSegmentPolygonEndRightForPainter.toQPointF()
                              << prevSegmentPolygonEndRight.toQPointF()
                              << prevSegmentPolygonEndLeft.toQPointF() );
    imagePainter.setPen( Qt::NoPen );
#endif

    // calculate the new progress horizontal through the source image to account for the length
    // of the segment we've just drawn
    progressThroughImage += sqrt( std::pow( segmentStartPoint.x() - segmentEndPoint.x(), 2 )
                                  +  std::pow( segmentStartPoint.y() - segmentEndPoint.y(), 2 ) )
                            + ( i == 1 && cap != Qt::PenCapStyle::FlatCap ? lineThickness / 2 : 0 ); // for first point we extended the pattern out by half its thickess at the start
    progressThroughImage = fmod( progressThroughImage, patternLength );

    // shuffle buffered variables for next loop
    segmentStartPoint = segmentEndPoint;
    prevSegmentPolygonEndLeft = thisSegmentPolygonEndLeft;
    prevSegmentPolygonEndRight = thisSegmentPolygonEndRight;
  }
  imagePainter.end();

  if ( context.renderContext().feedback() && context.renderContext().feedback()->isCanceled() )
    return;

  // lastly, draw the temporary image onto the destination painter at the correct place
  p->drawImage( QPointF( minX - lineThickness - ANTIALIAS_ALLOWANCE_PIXELS,
                         minY - lineThickness - ANTIALIAS_ALLOWANCE_PIXELS ), temporaryImage );
}


//
// QgsRasterLineSymbolLayer
//

QgsRasterLineSymbolLayer::QgsRasterLineSymbolLayer( const QString &path )
  : mPath( path )
{
}

QgsRasterLineSymbolLayer::~QgsRasterLineSymbolLayer() = default;

QgsSymbolLayer *QgsRasterLineSymbolLayer::create( const QVariantMap &properties )
{
  std::unique_ptr< QgsRasterLineSymbolLayer > res = std::make_unique<QgsRasterLineSymbolLayer>();

  if ( properties.contains( QStringLiteral( "line_width" ) ) )
  {
    res->setWidth( properties[QStringLiteral( "line_width" )].toDouble() );
  }
  if ( properties.contains( QStringLiteral( "line_width_unit" ) ) )
  {
    res->setWidthUnit( QgsUnitTypes::decodeRenderUnit( properties[QStringLiteral( "line_width_unit" )].toString() ) );
  }
  if ( properties.contains( QStringLiteral( "width_map_unit_scale" ) ) )
  {
    res->setWidthMapUnitScale( QgsSymbolLayerUtils::decodeMapUnitScale( properties[QStringLiteral( "width_map_unit_scale" )].toString() ) );
  }

  if ( properties.contains( QStringLiteral( "imageFile" ) ) )
    res->setPath( properties[QStringLiteral( "imageFile" )].toString() );

  if ( properties.contains( QStringLiteral( "offset" ) ) )
  {
    res->setOffset( properties[QStringLiteral( "offset" )].toDouble() );
  }
  if ( properties.contains( QStringLiteral( "offset_unit" ) ) )
  {
    res->setOffsetUnit( QgsUnitTypes::decodeRenderUnit( properties[QStringLiteral( "offset_unit" )].toString() ) );
  }
  if ( properties.contains( QStringLiteral( "offset_map_unit_scale" ) ) )
  {
    res->setOffsetMapUnitScale( QgsSymbolLayerUtils::decodeMapUnitScale( properties[QStringLiteral( "offset_map_unit_scale" )].toString() ) );
  }

  if ( properties.contains( QStringLiteral( "joinstyle" ) ) )
    res->setPenJoinStyle( QgsSymbolLayerUtils::decodePenJoinStyle( properties[QStringLiteral( "joinstyle" )].toString() ) );
  if ( properties.contains( QStringLiteral( "capstyle" ) ) )
    res->setPenCapStyle( QgsSymbolLayerUtils::decodePenCapStyle( properties[QStringLiteral( "capstyle" )].toString() ) );

  if ( properties.contains( QStringLiteral( "alpha" ) ) )
  {
    res->setOpacity( properties[QStringLiteral( "alpha" )].toDouble() );
  }

  return res.release();
}


QVariantMap QgsRasterLineSymbolLayer::properties() const
{
  QVariantMap map;
  map[QStringLiteral( "imageFile" )] = mPath;

  map[QStringLiteral( "line_width" )] = QString::number( mWidth );
  map[QStringLiteral( "line_width_unit" )] = QgsUnitTypes::encodeUnit( mWidthUnit );
  map[QStringLiteral( "width_map_unit_scale" )] = QgsSymbolLayerUtils::encodeMapUnitScale( mWidthMapUnitScale );

  map[QStringLiteral( "joinstyle" )] = QgsSymbolLayerUtils::encodePenJoinStyle( mPenJoinStyle );
  map[QStringLiteral( "capstyle" )] = QgsSymbolLayerUtils::encodePenCapStyle( mPenCapStyle );

  map[QStringLiteral( "offset" )] = QString::number( mOffset );
  map[QStringLiteral( "offset_unit" )] = QgsUnitTypes::encodeUnit( mOffsetUnit );
  map[QStringLiteral( "offset_map_unit_scale" )] = QgsSymbolLayerUtils::encodeMapUnitScale( mOffsetMapUnitScale );

  map[QStringLiteral( "alpha" )] = QString::number( mOpacity );

  return map;
}

QgsRasterLineSymbolLayer *QgsRasterLineSymbolLayer::clone() const
{
  std::unique_ptr< QgsRasterLineSymbolLayer > res = std::make_unique< QgsRasterLineSymbolLayer >( mPath );
  res->setWidth( mWidth );
  res->setWidthUnit( mWidthUnit );
  res->setWidthMapUnitScale( mWidthMapUnitScale );
  res->setPenJoinStyle( mPenJoinStyle );
  res->setPenCapStyle( mPenCapStyle );
  res->setOffsetUnit( mOffsetUnit );
  res->setOffsetMapUnitScale( mOffsetMapUnitScale );
  res->setOffset( mOffset );
  res->setOpacity( mOpacity );
  copyDataDefinedProperties( res.get() );
  copyPaintEffect( res.get() );
  return res.release();
}

void QgsRasterLineSymbolLayer::resolvePaths( QVariantMap &properties, const QgsPathResolver &pathResolver, bool saving )
{
  const QVariantMap::iterator it = properties.find( QStringLiteral( "imageFile" ) );
  if ( it != properties.end() && it.value().type() == QVariant::String )
  {
    if ( saving )
      it.value() = QgsSymbolLayerUtils::svgSymbolPathToName( it.value().toString(), pathResolver );
    else
      it.value() =  QgsSymbolLayerUtils::svgSymbolNameToPath( it.value().toString(), pathResolver );
  }
}

void QgsRasterLineSymbolLayer::setPath( const QString &path )
{
  mPath = path;
}

QString QgsRasterLineSymbolLayer::layerType() const
{
  return QStringLiteral( "RasterLine" );
}

void QgsRasterLineSymbolLayer::startRender( QgsSymbolRenderContext &context )
{
  double scaledHeight = context.renderContext().convertToPainterUnits( mWidth, mWidthUnit, mWidthMapUnitScale );

  const QSize originalSize = QgsApplication::imageCache()->originalSize( mPath, ( context.renderContext().flags() & Qgis::RenderContextFlag::RenderBlocking ) );

  double opacity = mOpacity * context.opacity();
  bool cached = false;
  mLineImage = QgsApplication::imageCache()->pathAsImage( mPath,
               QSize( static_cast< int >( std::round( originalSize.width() / originalSize.height() * std::max( 1.0, scaledHeight ) ) ),
                      static_cast< int >( std::ceil( scaledHeight ) ) ),
               true, opacity, cached, ( context.renderContext().flags() & Qgis::RenderContextFlag::RenderBlocking ) );
}

void QgsRasterLineSymbolLayer::stopRender( QgsSymbolRenderContext & )
{
}

void QgsRasterLineSymbolLayer::renderPolyline( const QPolygonF &points, QgsSymbolRenderContext &context )
{
  if ( !context.renderContext().painter() )
    return;

  QImage sourceImage = mLineImage;
  if ( mDataDefinedProperties.isActive( QgsSymbolLayer::PropertyStrokeWidth )
       || mDataDefinedProperties.isActive( QgsSymbolLayer::PropertyFile )
       || mDataDefinedProperties.isActive( QgsSymbolLayer::PropertyOpacity ) )
  {
    QString path = mPath;
    if ( mDataDefinedProperties.isActive( QgsSymbolLayer::PropertyFile ) )
    {
      context.setOriginalValueVariable( path );
      path = mDataDefinedProperties.valueAsString( QgsSymbolLayer::PropertyFile, context.renderContext().expressionContext(), path );
    }

    double strokeWidth = mWidth;
    if ( mDataDefinedProperties.isActive( QgsSymbolLayer::PropertyStrokeWidth ) )
    {
      context.setOriginalValueVariable( strokeWidth );
      strokeWidth = mDataDefinedProperties.valueAsDouble( QgsSymbolLayer::PropertyStrokeWidth, context.renderContext().expressionContext(), strokeWidth );
    }
    const double scaledHeight = context.renderContext().convertToPainterUnits( strokeWidth, mWidthUnit, mWidthMapUnitScale );

    const QSize originalSize = QgsApplication::imageCache()->originalSize( path, ( context.renderContext().flags() & Qgis::RenderContextFlag::RenderBlocking ) );
    double opacity = mOpacity;
    if ( mDataDefinedProperties.isActive( QgsSymbolLayer::PropertyOpacity ) )
    {
      context.setOriginalValueVariable( mOpacity );
      opacity = mDataDefinedProperties.valueAsDouble( QgsSymbolLayer::PropertyOpacity, context.renderContext().expressionContext(), opacity * 100 ) / 100.0;
    }
    opacity *= context.opacity();

    bool cached = false;
    sourceImage = QgsApplication::imageCache()->pathAsImage( path,
                  QSize( static_cast< int >( std::round( originalSize.width() / originalSize.height() * std::max( 1.0, scaledHeight ) ) ),
                         static_cast< int >( std::ceil( scaledHeight ) ) ),
                  true, opacity, cached, ( context.renderContext().flags() & Qgis::RenderContextFlag::RenderBlocking ) );
  }

  if ( context.selected() )
  {
    QgsImageOperation::adjustHueSaturation( sourceImage, 1.0, context.renderContext().selectionColor(), 1.0, context.renderContext().feedback() );
  }

  const QBrush brush( sourceImage );

  renderPolylineUsingBrush( points, context, brush, sourceImage.height(), sourceImage.width() );
}

void QgsRasterLineSymbolLayer::setOutputUnit( QgsUnitTypes::RenderUnit unit )
{
  QgsLineSymbolLayer::setOutputUnit( unit );
  mWidthUnit = unit;
  mOffsetUnit = unit;
}

QgsUnitTypes::RenderUnit QgsRasterLineSymbolLayer::outputUnit() const
{
  QgsUnitTypes::RenderUnit unit = QgsLineSymbolLayer::outputUnit();
  if ( mWidthUnit != unit || mOffsetUnit != unit )
  {
    return QgsUnitTypes::RenderUnknownUnit;
  }
  return unit;
}

bool QgsRasterLineSymbolLayer::usesMapUnits() const
{
  return mWidthUnit == QgsUnitTypes::RenderMapUnits || mWidthUnit == QgsUnitTypes::RenderMetersInMapUnits
         || mOffsetUnit == QgsUnitTypes::RenderMapUnits || mOffsetUnit == QgsUnitTypes::RenderMetersInMapUnits;
}

void QgsRasterLineSymbolLayer::setMapUnitScale( const QgsMapUnitScale &scale )
{
  QgsLineSymbolLayer::setMapUnitScale( scale );
  mOffsetMapUnitScale = scale;
}

QgsMapUnitScale QgsRasterLineSymbolLayer::mapUnitScale() const
{
  if ( QgsLineSymbolLayer::mapUnitScale() == mWidthMapUnitScale &&
       mWidthMapUnitScale == mOffsetMapUnitScale )
  {
    return mWidthMapUnitScale;
  }
  return QgsMapUnitScale();
}

double QgsRasterLineSymbolLayer::estimateMaxBleed( const QgsRenderContext & ) const
{
  return ( mWidth / 2.0 ) + mOffset;
}

QColor QgsRasterLineSymbolLayer::color() const
{
  return QColor();
}


//
// QgsLineburstSymbolLayer
//

QgsLineburstSymbolLayer::QgsLineburstSymbolLayer( const QColor &color, const QColor &color2 )
  : QgsAbstractBrushedLineSymbolLayer()
  , mColor2( color2 )
{
  setColor( color );
}

QgsLineburstSymbolLayer::~QgsLineburstSymbolLayer() = default;

QgsSymbolLayer *QgsLineburstSymbolLayer::create( const QVariantMap &properties )
{
  std::unique_ptr< QgsLineburstSymbolLayer > res = std::make_unique<QgsLineburstSymbolLayer>();

  if ( properties.contains( QStringLiteral( "line_width" ) ) )
  {
    res->setWidth( properties[QStringLiteral( "line_width" )].toDouble() );
  }
  if ( properties.contains( QStringLiteral( "line_width_unit" ) ) )
  {
    res->setWidthUnit( QgsUnitTypes::decodeRenderUnit( properties[QStringLiteral( "line_width_unit" )].toString() ) );
  }
  if ( properties.contains( QStringLiteral( "width_map_unit_scale" ) ) )
  {
    res->setWidthMapUnitScale( QgsSymbolLayerUtils::decodeMapUnitScale( properties[QStringLiteral( "width_map_unit_scale" )].toString() ) );
  }

  if ( properties.contains( QStringLiteral( "offset" ) ) )
  {
    res->setOffset( properties[QStringLiteral( "offset" )].toDouble() );
  }
  if ( properties.contains( QStringLiteral( "offset_unit" ) ) )
  {
    res->setOffsetUnit( QgsUnitTypes::decodeRenderUnit( properties[QStringLiteral( "offset_unit" )].toString() ) );
  }
  if ( properties.contains( QStringLiteral( "offset_map_unit_scale" ) ) )
  {
    res->setOffsetMapUnitScale( QgsSymbolLayerUtils::decodeMapUnitScale( properties[QStringLiteral( "offset_map_unit_scale" )].toString() ) );
  }

  if ( properties.contains( QStringLiteral( "joinstyle" ) ) )
    res->setPenJoinStyle( QgsSymbolLayerUtils::decodePenJoinStyle( properties[QStringLiteral( "joinstyle" )].toString() ) );
  if ( properties.contains( QStringLiteral( "capstyle" ) ) )
    res->setPenCapStyle( QgsSymbolLayerUtils::decodePenCapStyle( properties[QStringLiteral( "capstyle" )].toString() ) );

  if ( properties.contains( QStringLiteral( "color_type" ) ) )
    res->setGradientColorType( static_cast< Qgis::GradientColorSource >( properties[QStringLiteral( "color_type" )].toInt() ) );

  if ( properties.contains( QStringLiteral( "color" ) ) )
  {
    res->setColor( QgsSymbolLayerUtils::decodeColor( properties[QStringLiteral( "color" )].toString() ) );
  }
  if ( properties.contains( QStringLiteral( "gradient_color2" ) ) )
  {
    res->setColor2( QgsSymbolLayerUtils::decodeColor( properties[QStringLiteral( "gradient_color2" )].toString() ) );
  }

  //attempt to create color ramp from props
  if ( properties.contains( QStringLiteral( "rampType" ) ) && properties[QStringLiteral( "rampType" )] == QgsCptCityColorRamp::typeString() )
  {
    res->setColorRamp( QgsCptCityColorRamp::create( properties ) );
  }
  else
  {
    res->setColorRamp( QgsGradientColorRamp::create( properties ) );
  }

  return res.release();
}

QVariantMap QgsLineburstSymbolLayer::properties() const
{
  QVariantMap map;

  map[QStringLiteral( "line_width" )] = QString::number( mWidth );
  map[QStringLiteral( "line_width_unit" )] = QgsUnitTypes::encodeUnit( mWidthUnit );
  map[QStringLiteral( "width_map_unit_scale" )] = QgsSymbolLayerUtils::encodeMapUnitScale( mWidthMapUnitScale );

  map[QStringLiteral( "joinstyle" )] = QgsSymbolLayerUtils::encodePenJoinStyle( mPenJoinStyle );
  map[QStringLiteral( "capstyle" )] = QgsSymbolLayerUtils::encodePenCapStyle( mPenCapStyle );

  map[QStringLiteral( "offset" )] = QString::number( mOffset );
  map[QStringLiteral( "offset_unit" )] = QgsUnitTypes::encodeUnit( mOffsetUnit );
  map[QStringLiteral( "offset_map_unit_scale" )] = QgsSymbolLayerUtils::encodeMapUnitScale( mOffsetMapUnitScale );

  map[QStringLiteral( "color" )] = QgsSymbolLayerUtils::encodeColor( mColor );
  map[QStringLiteral( "gradient_color2" )] = QgsSymbolLayerUtils::encodeColor( mColor2 );
  map[QStringLiteral( "color_type" )] = QString::number( static_cast< int >( mGradientColorType ) );
  if ( mGradientRamp )
  {
#if QT_VERSION < QT_VERSION_CHECK(5, 15, 0)
    map.unite( mGradientRamp->properties() );
#else
    map.insert( mGradientRamp->properties() );
#endif
  }

  return map;
}

QgsLineburstSymbolLayer *QgsLineburstSymbolLayer::clone() const
{
  std::unique_ptr< QgsLineburstSymbolLayer > res = std::make_unique< QgsLineburstSymbolLayer >();
  res->setWidth( mWidth );
  res->setWidthUnit( mWidthUnit );
  res->setWidthMapUnitScale( mWidthMapUnitScale );
  res->setPenJoinStyle( mPenJoinStyle );
  res->setPenCapStyle( mPenCapStyle );
  res->setOffsetUnit( mOffsetUnit );
  res->setOffsetMapUnitScale( mOffsetMapUnitScale );
  res->setOffset( mOffset );
  res->setColor( mColor );
  res->setColor2( mColor2 );
  res->setGradientColorType( mGradientColorType );
  if ( mGradientRamp )
    res->setColorRamp( mGradientRamp->clone() );
  copyDataDefinedProperties( res.get() );
  copyPaintEffect( res.get() );
  return res.release();
}

QString QgsLineburstSymbolLayer::layerType() const
{
  return QStringLiteral( "Lineburst" );
}

void QgsLineburstSymbolLayer::startRender( QgsSymbolRenderContext & )
{
}

void QgsLineburstSymbolLayer::stopRender( QgsSymbolRenderContext & )
{
}

void QgsLineburstSymbolLayer::renderPolyline( const QPolygonF &points, QgsSymbolRenderContext &context )
{
  if ( !context.renderContext().painter() )
    return;

  double strokeWidth = mWidth;
  if ( mDataDefinedProperties.isActive( QgsSymbolLayer::PropertyStrokeWidth ) )
  {
    context.setOriginalValueVariable( strokeWidth );
    strokeWidth = mDataDefinedProperties.valueAsDouble( QgsSymbolLayer::PropertyStrokeWidth, context.renderContext().expressionContext(), strokeWidth );
  }
  const double scaledWidth = context.renderContext().convertToPainterUnits( strokeWidth, mWidthUnit, mWidthMapUnitScale );

  //update alpha of gradient colors
  QColor color1 = mColor;
  if ( mDataDefinedProperties.isActive( QgsSymbolLayer::PropertyStrokeColor ) )
  {
    context.setOriginalValueVariable( QgsSymbolLayerUtils::encodeColor( mColor ) );
    color1 = mDataDefinedProperties.valueAsColor( QgsSymbolLayer::PropertyStrokeColor, context.renderContext().expressionContext(), mColor );
  }
  if ( context.selected() )
  {
    color1 = context.renderContext().selectionColor();
  }
  color1.setAlphaF( context.opacity() * color1.alphaF() );

  //second gradient color
  QColor color2 = mColor2;
  if ( mDataDefinedProperties.isActive( QgsSymbolLayer::PropertySecondaryColor ) )
  {
    context.setOriginalValueVariable( QgsSymbolLayerUtils::encodeColor( mColor2 ) );
    color2 = mDataDefinedProperties.valueAsColor( QgsSymbolLayer::PropertySecondaryColor, context.renderContext().expressionContext(), mColor2 );
  }

  //create a QGradient with the desired properties
  QGradient gradient = QLinearGradient( QPointF( 0, 0 ), QPointF( 0, scaledWidth ) );
  //add stops to gradient
  if ( mGradientColorType == Qgis::GradientColorSource::ColorRamp && mGradientRamp &&
       ( mGradientRamp->type() == QgsGradientColorRamp::typeString() || mGradientRamp->type() == QgsCptCityColorRamp::typeString() ) )
  {
    //color ramp gradient
    QgsGradientColorRamp *gradRamp = static_cast<QgsGradientColorRamp *>( mGradientRamp.get() );
    gradRamp->addStopsToGradient( &gradient, context.opacity() );
  }
  else
  {
    //two color gradient
    gradient.setColorAt( 0.0, color1 );
    gradient.setColorAt( 1.0, color2 );
  }
  const QBrush brush( gradient );

  renderPolylineUsingBrush( points, context, brush, scaledWidth, 100 );
}

void QgsLineburstSymbolLayer::setOutputUnit( QgsUnitTypes::RenderUnit unit )
{
  QgsLineSymbolLayer::setOutputUnit( unit );
  mWidthUnit = unit;
  mOffsetUnit = unit;
}

QgsUnitTypes::RenderUnit QgsLineburstSymbolLayer::outputUnit() const
{
  QgsUnitTypes::RenderUnit unit = QgsLineSymbolLayer::outputUnit();
  if ( mWidthUnit != unit || mOffsetUnit != unit )
  {
    return QgsUnitTypes::RenderUnknownUnit;
  }
  return unit;
}

bool QgsLineburstSymbolLayer::usesMapUnits() const
{
  return mWidthUnit == QgsUnitTypes::RenderMapUnits || mWidthUnit == QgsUnitTypes::RenderMetersInMapUnits
         || mOffsetUnit == QgsUnitTypes::RenderMapUnits || mOffsetUnit == QgsUnitTypes::RenderMetersInMapUnits;
}

void QgsLineburstSymbolLayer::setMapUnitScale( const QgsMapUnitScale &scale )
{
  QgsLineSymbolLayer::setMapUnitScale( scale );
  mOffsetMapUnitScale = scale;
}

QgsMapUnitScale QgsLineburstSymbolLayer::mapUnitScale() const
{
  if ( QgsLineSymbolLayer::mapUnitScale() == mWidthMapUnitScale &&
       mWidthMapUnitScale == mOffsetMapUnitScale )
  {
    return mWidthMapUnitScale;
  }
  return QgsMapUnitScale();
}

double QgsLineburstSymbolLayer::estimateMaxBleed( const QgsRenderContext & ) const
{
  return ( mWidth / 2.0 ) + mOffset;
}

QColor QgsLineburstSymbolLayer::color() const
{
  return QColor();
}

QgsColorRamp *QgsLineburstSymbolLayer::colorRamp()
{
  return mGradientRamp.get();
}

void QgsLineburstSymbolLayer::setColorRamp( QgsColorRamp *ramp )
{
  mGradientRamp.reset( ramp );
}
