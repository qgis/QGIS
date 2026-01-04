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

#include <algorithm>
#include <cmath>
#include <line_p.h>
#include <memory>

#include "qgsapplication.h"
#include "qgscolorrampimpl.h"
#include "qgscolorutils.h"
#include "qgscurvepolygon.h"
#include "qgsdxfexport.h"
#include "qgsexpressioncontextutils.h"
#include "qgsfeedback.h"
#include "qgsfillsymbol.h"
#include "qgsgeometrysimplifier.h"
#include "qgsgeometryutils.h"
#include "qgsgeos.h"
#include "qgsimagecache.h"
#include "qgsimageoperation.h"
#include "qgslinesymbol.h"
#include "qgslogger.h"
#include "qgsmarkersymbol.h"
#include "qgsmultipolygon.h"
#include "qgspolygon.h"
#include "qgsproperty.h"
#include "qgsrendercontext.h"
#include "qgssldexportcontext.h"
#include "qgssymbollayerutils.h"
#include "qgsunittypes.h"

#include <QDomDocument>
#include <QDomElement>
#include <QPainter>

QgsSimpleLineSymbolLayer::QgsSimpleLineSymbolLayer( const QColor &color, double width, Qt::PenStyle penStyle )
  : mPenStyle( penStyle )
{
  mColor = color;
  mWidth = width;
  mCustomDashVector << 5 << 2;
}

QgsSimpleLineSymbolLayer::~QgsSimpleLineSymbolLayer() = default;

void QgsSimpleLineSymbolLayer::setOutputUnit( Qgis::RenderUnit unit )
{
  QgsLineSymbolLayer::setOutputUnit( unit );
  mWidthUnit = unit;
  mOffsetUnit = unit;
  mCustomDashPatternUnit = unit;
  mDashPatternOffsetUnit = unit;
  mTrimDistanceStartUnit = unit;
  mTrimDistanceEndUnit = unit;
}

Qgis::RenderUnit  QgsSimpleLineSymbolLayer::outputUnit() const
{
  Qgis::RenderUnit unit = QgsLineSymbolLayer::outputUnit();
  if ( mWidthUnit != unit || mOffsetUnit != unit || mCustomDashPatternUnit != unit )
  {
    return Qgis::RenderUnit::Unknown;
  }
  return unit;
}

bool QgsSimpleLineSymbolLayer::usesMapUnits() const
{
  return mWidthUnit == Qgis::RenderUnit::MapUnits || mWidthUnit == Qgis::RenderUnit::MetersInMapUnits
         || mOffsetUnit == Qgis::RenderUnit::MapUnits || mOffsetUnit == Qgis::RenderUnit::MetersInMapUnits;
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

  if ( props.contains( u"line_color"_s ) )
  {
    color = QgsColorUtils::colorFromString( props[u"line_color"_s].toString() );
  }
  else if ( props.contains( u"outline_color"_s ) )
  {
    color = QgsColorUtils::colorFromString( props[u"outline_color"_s].toString() );
  }
  else if ( props.contains( u"color"_s ) )
  {
    //pre 2.5 projects used "color"
    color = QgsColorUtils::colorFromString( props[u"color"_s].toString() );
  }
  if ( props.contains( u"line_width"_s ) )
  {
    width = props[u"line_width"_s].toDouble();
  }
  else if ( props.contains( u"outline_width"_s ) )
  {
    width = props[u"outline_width"_s].toDouble();
  }
  else if ( props.contains( u"width"_s ) )
  {
    //pre 2.5 projects used "width"
    width = props[u"width"_s].toDouble();
  }
  if ( props.contains( u"line_style"_s ) )
  {
    penStyle = QgsSymbolLayerUtils::decodePenStyle( props[u"line_style"_s].toString() );
  }
  else if ( props.contains( u"outline_style"_s ) )
  {
    penStyle = QgsSymbolLayerUtils::decodePenStyle( props[u"outline_style"_s].toString() );
  }
  else if ( props.contains( u"penstyle"_s ) )
  {
    penStyle = QgsSymbolLayerUtils::decodePenStyle( props[u"penstyle"_s].toString() );
  }

  QgsSimpleLineSymbolLayer *l = new QgsSimpleLineSymbolLayer( color, width, penStyle );
  if ( props.contains( u"line_width_unit"_s ) )
  {
    l->setWidthUnit( QgsUnitTypes::decodeRenderUnit( props[u"line_width_unit"_s].toString() ) );
  }
  else if ( props.contains( u"outline_width_unit"_s ) )
  {
    l->setWidthUnit( QgsUnitTypes::decodeRenderUnit( props[u"outline_width_unit"_s].toString() ) );
  }
  else if ( props.contains( u"width_unit"_s ) )
  {
    //pre 2.5 projects used "width_unit"
    l->setWidthUnit( QgsUnitTypes::decodeRenderUnit( props[u"width_unit"_s].toString() ) );
  }
  if ( props.contains( u"width_map_unit_scale"_s ) )
    l->setWidthMapUnitScale( QgsSymbolLayerUtils::decodeMapUnitScale( props[u"width_map_unit_scale"_s].toString() ) );
  if ( props.contains( u"offset"_s ) )
    l->setOffset( props[u"offset"_s].toDouble() );
  if ( props.contains( u"offset_unit"_s ) )
    l->setOffsetUnit( QgsUnitTypes::decodeRenderUnit( props[u"offset_unit"_s].toString() ) );
  if ( props.contains( u"offset_map_unit_scale"_s ) )
    l->setOffsetMapUnitScale( QgsSymbolLayerUtils::decodeMapUnitScale( props[u"offset_map_unit_scale"_s].toString() ) );
  if ( props.contains( u"joinstyle"_s ) )
    l->setPenJoinStyle( QgsSymbolLayerUtils::decodePenJoinStyle( props[u"joinstyle"_s].toString() ) );
  if ( props.contains( u"capstyle"_s ) )
    l->setPenCapStyle( QgsSymbolLayerUtils::decodePenCapStyle( props[u"capstyle"_s].toString() ) );

  if ( props.contains( u"use_custom_dash"_s ) )
  {
    l->setUseCustomDashPattern( props[u"use_custom_dash"_s].toInt() );
  }
  if ( props.contains( u"customdash"_s ) )
  {
    l->setCustomDashVector( QgsSymbolLayerUtils::decodeRealVector( props[u"customdash"_s].toString() ) );
  }
  if ( props.contains( u"customdash_unit"_s ) )
  {
    l->setCustomDashPatternUnit( QgsUnitTypes::decodeRenderUnit( props[u"customdash_unit"_s].toString() ) );
  }
  if ( props.contains( u"customdash_map_unit_scale"_s ) )
  {
    l->setCustomDashPatternMapUnitScale( QgsSymbolLayerUtils::decodeMapUnitScale( props[u"customdash_map_unit_scale"_s].toString() ) );
  }

  if ( props.contains( u"draw_inside_polygon"_s ) )
  {
    l->setDrawInsidePolygon( props[u"draw_inside_polygon"_s].toInt() );
  }

  if ( props.contains( u"ring_filter"_s ) )
  {
    l->setRingFilter( static_cast< RenderRingFilter>( props[u"ring_filter"_s].toInt() ) );
  }

  if ( props.contains( u"dash_pattern_offset"_s ) )
    l->setDashPatternOffset( props[u"dash_pattern_offset"_s].toDouble() );
  if ( props.contains( u"dash_pattern_offset_unit"_s ) )
    l->setDashPatternOffsetUnit( QgsUnitTypes::decodeRenderUnit( props[u"dash_pattern_offset_unit"_s].toString() ) );
  if ( props.contains( u"dash_pattern_offset_map_unit_scale"_s ) )
    l->setDashPatternOffsetMapUnitScale( QgsSymbolLayerUtils::decodeMapUnitScale( props[u"dash_pattern_offset_map_unit_scale"_s].toString() ) );

  if ( props.contains( u"trim_distance_start"_s ) )
    l->setTrimDistanceStart( props[u"trim_distance_start"_s].toDouble() );
  if ( props.contains( u"trim_distance_start_unit"_s ) )
    l->setTrimDistanceStartUnit( QgsUnitTypes::decodeRenderUnit( props[u"trim_distance_start_unit"_s].toString() ) );
  if ( props.contains( u"trim_distance_start_map_unit_scale"_s ) )
    l->setTrimDistanceStartMapUnitScale( QgsSymbolLayerUtils::decodeMapUnitScale( props[u"trim_distance_start_map_unit_scale"_s].toString() ) );
  if ( props.contains( u"trim_distance_end"_s ) )
    l->setTrimDistanceEnd( props[u"trim_distance_end"_s].toDouble() );
  if ( props.contains( u"trim_distance_end_unit"_s ) )
    l->setTrimDistanceEndUnit( QgsUnitTypes::decodeRenderUnit( props[u"trim_distance_end_unit"_s].toString() ) );
  if ( props.contains( u"trim_distance_end_map_unit_scale"_s ) )
    l->setTrimDistanceEndMapUnitScale( QgsSymbolLayerUtils::decodeMapUnitScale( props[u"trim_distance_end_map_unit_scale"_s].toString() ) );

  if ( props.contains( u"align_dash_pattern"_s ) )
    l->setAlignDashPattern( props[ u"align_dash_pattern"_s].toInt() );

  if ( props.contains( u"tweak_dash_pattern_on_corners"_s ) )
    l->setTweakDashPatternOnCorners( props[ u"tweak_dash_pattern_on_corners"_s].toInt() );

  l->restoreOldDataDefinedProperties( props );

  return l;
}

QString QgsSimpleLineSymbolLayer::layerType() const
{
  return u"SimpleLine"_s;
}

Qgis::SymbolLayerFlags QgsSimpleLineSymbolLayer::flags() const
{
  return QgsLineSymbolLayer::flags() | Qgis::SymbolLayerFlag::CanCalculateMaskGeometryPerFeature;
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
  if ( mDataDefinedProperties.isActive( QgsSymbolLayer::Property::TrimStart ) )
  {
    context.setOriginalValueVariable( startTrim );
    startTrim = mDataDefinedProperties.valueAsDouble( QgsSymbolLayer::Property::TrimStart, context.renderContext().expressionContext(), mTrimDistanceStart );
  }
  double endTrim = mTrimDistanceEnd;
  if ( mDataDefinedProperties.isActive( QgsSymbolLayer::Property::TrimEnd ) )
  {
    context.setOriginalValueVariable( endTrim );
    endTrim = mDataDefinedProperties.valueAsDouble( QgsSymbolLayer::Property::TrimEnd, context.renderContext().expressionContext(), mTrimDistanceEnd );
  }

  double totalLength = -1;
  if ( mTrimDistanceStartUnit == Qgis::RenderUnit::Percentage )
  {
    totalLength = QgsSymbolLayerUtils::polylineLength( points );
    startTrim = startTrim * 0.01 * totalLength;
  }
  else
  {
    startTrim = context.renderContext().convertToPainterUnits( startTrim, mTrimDistanceStartUnit, mTrimDistanceStartMapUnitScale );
  }
  if ( mTrimDistanceEndUnit == Qgis::RenderUnit::Percentage )
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

  const bool useSelectedColor = shouldRenderUsingSelectionColor( context );
  const QPen pen = useSelectedColor ? mSelPen : mPen;

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
       ( context.renderContext().vectorSimplifyMethod().simplifyHints() & Qgis::VectorRenderingSimplificationFlag::AntialiasingSimplification ) &&
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
    if ( mOffsetUnit == Qgis::RenderUnit::MetersInMapUnits && ( context.renderContext().flags() & Qgis::RenderContextFlag::RenderSymbolPreview || context.renderContext().flags() & Qgis::RenderContextFlag::RenderLayerTree ) )
    {
      // rendering for symbol previews -- a size in meters in map units can't be calculated, so treat the size as millimeters
      // and clamp it to a reasonable range. It's the best we can do in this situation!
      scaledOffset = std::min( std::max( context.renderContext().convertToPainterUnits( offset, Qgis::RenderUnit::Millimeters ), 3.0 ), 100.0 );
    }

    QList<QPolygonF> mline = ::offsetLine( points, scaledOffset, context.originalGeometryType() != Qgis::GeometryType::Unknown ? context.originalGeometryType() : Qgis::GeometryType::Line );
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
  map[u"line_color"_s] = QgsColorUtils::colorToString( mColor );
  map[u"line_width"_s] = QString::number( mWidth );
  map[u"line_width_unit"_s] = QgsUnitTypes::encodeUnit( mWidthUnit );
  map[u"width_map_unit_scale"_s] = QgsSymbolLayerUtils::encodeMapUnitScale( mWidthMapUnitScale );
  map[u"line_style"_s] = QgsSymbolLayerUtils::encodePenStyle( mPenStyle );
  map[u"joinstyle"_s] = QgsSymbolLayerUtils::encodePenJoinStyle( mPenJoinStyle );
  map[u"capstyle"_s] = QgsSymbolLayerUtils::encodePenCapStyle( mPenCapStyle );
  map[u"offset"_s] = QString::number( mOffset );
  map[u"offset_unit"_s] = QgsUnitTypes::encodeUnit( mOffsetUnit );
  map[u"offset_map_unit_scale"_s] = QgsSymbolLayerUtils::encodeMapUnitScale( mOffsetMapUnitScale );
  map[u"use_custom_dash"_s] = ( mUseCustomDashPattern ? u"1"_s : u"0"_s );
  map[u"customdash"_s] = QgsSymbolLayerUtils::encodeRealVector( mCustomDashVector );
  map[u"customdash_unit"_s] = QgsUnitTypes::encodeUnit( mCustomDashPatternUnit );
  map[u"customdash_map_unit_scale"_s] = QgsSymbolLayerUtils::encodeMapUnitScale( mCustomDashPatternMapUnitScale );
  map[u"dash_pattern_offset"_s] = QString::number( mDashPatternOffset );
  map[u"dash_pattern_offset_unit"_s] = QgsUnitTypes::encodeUnit( mDashPatternOffsetUnit );
  map[u"dash_pattern_offset_map_unit_scale"_s] = QgsSymbolLayerUtils::encodeMapUnitScale( mDashPatternOffsetMapUnitScale );
  map[u"trim_distance_start"_s] = QString::number( mTrimDistanceStart );
  map[u"trim_distance_start_unit"_s] = QgsUnitTypes::encodeUnit( mTrimDistanceStartUnit );
  map[u"trim_distance_start_map_unit_scale"_s] = QgsSymbolLayerUtils::encodeMapUnitScale( mTrimDistanceStartMapUnitScale );
  map[u"trim_distance_end"_s] = QString::number( mTrimDistanceEnd );
  map[u"trim_distance_end_unit"_s] = QgsUnitTypes::encodeUnit( mTrimDistanceEndUnit );
  map[u"trim_distance_end_map_unit_scale"_s] = QgsSymbolLayerUtils::encodeMapUnitScale( mTrimDistanceEndMapUnitScale );
  map[u"draw_inside_polygon"_s] = ( mDrawInsidePolygon ? u"1"_s : u"0"_s );
  map[u"ring_filter"_s] = QString::number( static_cast< int >( mRingFilter ) );
  map[u"align_dash_pattern"_s] = mAlignDashPattern ? u"1"_s : u"0"_s;
  map[u"tweak_dash_pattern_on_corners"_s] = mPatternCartographicTweakOnSharpCorners ? u"1"_s : u"0"_s;
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
  QgsSldExportContext context;
  context.setExtraProperties( props );
  toSld( doc, element, context );
}

bool QgsSimpleLineSymbolLayer::toSld( QDomDocument &doc, QDomElement &element, QgsSldExportContext &context ) const
{
  if ( mPenStyle == Qt::NoPen )
    return true;

  const QVariantMap props = context.extraProperties();
  QDomElement symbolizerElem = doc.createElement( u"se:LineSymbolizer"_s );
  if ( !props.value( u"uom"_s, QString() ).toString().isEmpty() )
    symbolizerElem.setAttribute( u"uom"_s, props.value( u"uom"_s, QString() ).toString() );
  element.appendChild( symbolizerElem );

  // <Geometry>
  QgsSymbolLayerUtils::createGeometryElement( doc, symbolizerElem, props.value( u"geom"_s, QString() ).toString(), context );

  // <Stroke>
  QDomElement strokeElem = doc.createElement( u"se:Stroke"_s );
  symbolizerElem.appendChild( strokeElem );

  Qt::PenStyle penStyle = mUseCustomDashPattern ? Qt::CustomDashLine : mPenStyle;
  double width = QgsSymbolLayerUtils::rescaleUom( mWidth, mWidthUnit, props );
  QVector<qreal> customDashVector = QgsSymbolLayerUtils::rescaleUom( mCustomDashVector, mCustomDashPatternUnit, props );
  QgsSymbolLayerUtils::lineToSld( doc, strokeElem, penStyle, mColor, context, width,
                                  &mPenJoinStyle, &mPenCapStyle, &customDashVector );

  // <se:PerpendicularOffset>
  if ( !qgsDoubleNear( mOffset, 0.0 ) )
  {
    QDomElement perpOffsetElem = doc.createElement( u"se:PerpendicularOffset"_s );
    double offset = QgsSymbolLayerUtils::rescaleUom( mOffset, mOffsetUnit, props );
    perpOffsetElem.appendChild( doc.createTextNode( qgsDoubleToString( offset ) ) );
    symbolizerElem.appendChild( perpOffsetElem );
  }
  return true;
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
  QgsDebugMsgLevel( u"Entered."_s, 4 );

  QDomElement strokeElem = element.firstChildElement( u"Stroke"_s );
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
  QDomElement perpOffsetElem = element.firstChildElement( u"PerpendicularOffset"_s );
  if ( !perpOffsetElem.isNull() )
  {
    bool ok;
    double d = perpOffsetElem.firstChild().nodeValue().toDouble( &ok );
    if ( ok )
      offset = d;
  }

  double scaleFactor = 1.0;
  const QString uom = element.attribute( u"uom"_s );
  Qgis::RenderUnit sldUnitSize = QgsSymbolLayerUtils::decodeSldUom( uom, &scaleFactor );
  width = width * scaleFactor;
  offset = offset * scaleFactor;

  QgsSimpleLineSymbolLayer *l = new QgsSimpleLineSymbolLayer( color, width, penStyle );
  l->setOutputUnit( sldUnitSize );
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
  if ( mDataDefinedProperties.isActive( QgsSymbolLayer::Property::StrokeWidth ) )
  {
    context.setOriginalValueVariable( mWidth );
    double scaledWidth = context.renderContext().convertToPainterUnits(
                           mDataDefinedProperties.valueAsDouble( QgsSymbolLayer::Property::StrokeWidth, context.renderContext().expressionContext(), mWidth ),
                           mWidthUnit, mWidthMapUnitScale );
    pen.setWidthF( scaledWidth );
    selPen.setWidthF( scaledWidth );
    hasStrokeWidthExpression = true;
  }

  //color
  if ( mDataDefinedProperties.isActive( QgsSymbolLayer::Property::StrokeColor ) )
  {
    context.setOriginalValueVariable( QgsSymbolLayerUtils::encodeColor( mColor ) );

    QColor penColor = mDataDefinedProperties.valueAsColor( QgsSymbolLayer::Property::StrokeColor, context.renderContext().expressionContext(), mColor );
    penColor.setAlphaF( context.opacity() * penColor.alphaF() );
    pen.setColor( penColor );
  }

  //offset
  if ( mDataDefinedProperties.isActive( QgsSymbolLayer::Property::Offset ) )
  {
    context.setOriginalValueVariable( mOffset );
    offset = mDataDefinedProperties.valueAsDouble( QgsSymbolLayer::Property::Offset, context.renderContext().expressionContext(), offset );
  }

  //dash dot vector

  //note that Qt seems to have issues with scaling dash patterns with very small pen widths.
  //treating the pen as having no less than a 1 pixel size avoids the worst of these issues
  const double dashWidthDiv = std::max( hasStrokeWidthExpression ? pen.widthF() : mPen.widthF(), 1.0 );

  if ( mDataDefinedProperties.isActive( QgsSymbolLayer::Property::CustomDash ) )
  {
    QVector<qreal> dashVector;
    QVariant exprVal = mDataDefinedProperties.value( QgsSymbolLayer::Property::CustomDash, context.renderContext().expressionContext() );
    if ( !QgsVariantUtils::isNull( exprVal ) )
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
  else if ( mDataDefinedProperties.isActive( QgsSymbolLayer::Property::StrokeWidth ) && mUseCustomDashPattern )
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
  if ( mDataDefinedProperties.isActive( QgsSymbolLayer::Property::DashPatternOffset ) && pen.style() != Qt::SolidLine )
  {
    context.setOriginalValueVariable( patternOffset );
    patternOffset = mDataDefinedProperties.valueAsDouble( QgsSymbolLayer::Property::DashPatternOffset, context.renderContext().expressionContext(), mDashPatternOffset );
    pen.setDashOffset( context.renderContext().convertToPainterUnits( patternOffset, mDashPatternOffsetUnit, mDashPatternOffsetMapUnitScale ) / dashWidthDiv );
  }

  //line style
  if ( mDataDefinedProperties.isActive( QgsSymbolLayer::Property::StrokeStyle ) )
  {
    context.setOriginalValueVariable( QgsSymbolLayerUtils::encodePenStyle( mPenStyle ) );
    QVariant exprVal = mDataDefinedProperties.value( QgsSymbolLayer::Property::StrokeStyle, context.renderContext().expressionContext() );
    if ( !QgsVariantUtils::isNull( exprVal ) )
      pen.setStyle( QgsSymbolLayerUtils::decodePenStyle( exprVal.toString() ) );
  }

  //join style
  if ( mDataDefinedProperties.isActive( QgsSymbolLayer::Property::JoinStyle ) )
  {
    context.setOriginalValueVariable( QgsSymbolLayerUtils::encodePenJoinStyle( mPenJoinStyle ) );
    QVariant exprVal = mDataDefinedProperties.value( QgsSymbolLayer::Property::JoinStyle, context.renderContext().expressionContext() );
    if ( !QgsVariantUtils::isNull( exprVal ) )
      pen.setJoinStyle( QgsSymbolLayerUtils::decodePenJoinStyle( exprVal.toString() ) );
  }

  //cap style
  if ( mDataDefinedProperties.isActive( QgsSymbolLayer::Property::CapStyle ) )
  {
    context.setOriginalValueVariable( QgsSymbolLayerUtils::encodePenCapStyle( mPenCapStyle ) );
    QVariant exprVal = mDataDefinedProperties.value( QgsSymbolLayer::Property::CapStyle, context.renderContext().expressionContext() );
    if ( !QgsVariantUtils::isNull( exprVal ) )
      pen.setCapStyle( QgsSymbolLayerUtils::decodePenCapStyle( exprVal.toString() ) );
  }
}

void QgsSimpleLineSymbolLayer::drawPathWithDashPatternTweaks( QPainter *painter, const QPolygonF &points, QPen pen ) const
{
  if ( pen.dashPattern().empty() || points.size() < 2 )
    return;

  if ( pen.widthF() <= 1.0 )
  {
    pen.setWidthF( 1.0001 );
  }

  QVector< qreal > sourcePattern = pen.dashPattern();
  const double dashWidthDiv = pen.widthF();
  // back to painter units
  for ( int i = 0; i < sourcePattern.size(); ++ i )
    sourcePattern[i] *= pen.widthF();

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
          if ( currentRemainingGapLength == 0.0 )
          {
            patternIndex++;
          }
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

      if ( patternIndex + 1 >= sourcePattern.size() )
      {
        patternIndex = 0;
      }

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

QVector<qreal> QgsSimpleLineSymbolLayer::dxfCustomDashPattern( Qgis::RenderUnit &unit ) const
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
  if ( mDataDefinedProperties.isActive( QgsSymbolLayer::Property::StrokeWidth ) )
  {
    context.setOriginalValueVariable( mWidth );
    width = mDataDefinedProperties.valueAsDouble( QgsSymbolLayer::Property::StrokeWidth, context.renderContext().expressionContext(), mWidth );
  }

  width *= QgsDxfExport::mapUnitScaleFactor( e.symbologyScale(), widthUnit(), e.mapUnits(), context.renderContext().mapToPixel().mapUnitsPerPixel() );
  if ( mWidthUnit == Qgis::RenderUnit::MapUnits )
  {
    e.clipValueToMapUnitScale( width, mWidthMapUnitScale, context.renderContext().scaleFactor() );
  }
  return width;
}

QColor QgsSimpleLineSymbolLayer::dxfColor( QgsSymbolRenderContext &context ) const
{
  if ( mDataDefinedProperties.isActive( QgsSymbolLayer::Property::StrokeColor ) )
  {
    context.setOriginalValueVariable( QgsSymbolLayerUtils::encodeColor( mColor ) );
    return mDataDefinedProperties.valueAsColor( QgsSymbolLayer::Property::StrokeColor, context.renderContext().expressionContext(), mColor );
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

  if ( mDataDefinedProperties.isActive( QgsSymbolLayer::Property::Offset ) )
  {
    context.setOriginalValueVariable( mOffset );
    offset = mDataDefinedProperties.valueAsDouble( QgsSymbolLayer::Property::Offset, context.renderContext().expressionContext(), mOffset );
  }

  offset *= QgsDxfExport::mapUnitScaleFactor( e.symbologyScale(), offsetUnit(), e.mapUnits(), context.renderContext().mapToPixel().mapUnitsPerPixel() );
  if ( mOffsetUnit == Qgis::RenderUnit::MapUnits )
  {
    e.clipValueToMapUnitScale( offset, mOffsetMapUnitScale, context.renderContext().scaleFactor() );
  }
  return -offset; //direction seems to be inverse to symbology offset
}

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
  const bool useSelectedColor = shouldRenderUsingSelectionColor( context );
  if ( mRenderingFeature )
  {
    // in the middle of rendering a possibly multi-part feature, so we collect all the parts and defer the actual rendering
    // until after we've received the final part
    mFeatureSymbolOpacity = context.opacity();
    mCurrentFeatureIsSelected = useSelectedColor;
  }

  double offset = mOffset;

  if ( mDataDefinedProperties.isActive( QgsSymbolLayer::Property::Offset ) )
  {
    context.setOriginalValueVariable( mOffset );
    offset = mDataDefinedProperties.valueAsDouble( QgsSymbolLayer::Property::Offset, context.renderContext().expressionContext(), mOffset );
  }

  Qgis::MarkerLinePlacements placements = QgsTemplatedLineSymbolLayerBase::placements();

  if ( mDataDefinedProperties.isActive( QgsSymbolLayer::Property::Placement ) )
  {
    QVariant exprVal = mDataDefinedProperties.value( QgsSymbolLayer::Property::Placement, context.renderContext().expressionContext() );
    if ( !QgsVariantUtils::isNull( exprVal ) )
    {
      QString placementString = exprVal.toString();
      if ( placementString.compare( "interval"_L1, Qt::CaseInsensitive ) == 0 )
      {
        placements = Qgis::MarkerLinePlacement::Interval;
      }
      else if ( placementString.compare( "vertex"_L1, Qt::CaseInsensitive ) == 0 )
      {
        placements = Qgis::MarkerLinePlacement::Vertex;
      }
      else if ( placementString.compare( "innervertices"_L1, Qt::CaseInsensitive ) == 0 )
      {
        placements = Qgis::MarkerLinePlacement::InnerVertices;
      }
      else if ( placementString.compare( "lastvertex"_L1, Qt::CaseInsensitive ) == 0 )
      {
        placements = Qgis::MarkerLinePlacement::LastVertex;
      }
      else if ( placementString.compare( "firstvertex"_L1, Qt::CaseInsensitive ) == 0 )
      {
        placements = Qgis::MarkerLinePlacement::FirstVertex;
      }
      else if ( placementString.compare( "centerpoint"_L1, Qt::CaseInsensitive ) == 0 )
      {
        placements = Qgis::MarkerLinePlacement::CentralPoint;
      }
      else if ( placementString.compare( "curvepoint"_L1, Qt::CaseInsensitive ) == 0 )
      {
        placements = Qgis::MarkerLinePlacement::CurvePoint;
      }
      else if ( placementString.compare( "segmentcenter"_L1, Qt::CaseInsensitive ) == 0 )
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
  if ( mDataDefinedProperties.isActive( QgsSymbolLayer::Property::AverageAngleLength ) )
  {
    context.setOriginalValueVariable( mAverageAngleLength );
    averageOver = mDataDefinedProperties.valueAsDouble( QgsSymbolLayer::Property::AverageAngleLength, context.renderContext().expressionContext(), mAverageAngleLength );
  }
  averageOver = context.renderContext().convertToPainterUnits( averageOver, mAverageAngleLengthUnit, mAverageAngleLengthMapUnitScale ) / 2.0;

  QgsBlankSegmentUtils::BlankSegments blankSegments;
  if ( mDataDefinedProperties.isActive( QgsSymbolLayer::Property::BlankSegments ) )
  {
    const QString strBlankSegments = mDataDefinedProperties.valueAsString( QgsSymbolLayer::Property::BlankSegments, context.renderContext().expressionContext() );
    QString error;
    QList<QList<QgsBlankSegmentUtils::BlankSegments>> allBlankSegments = QgsBlankSegmentUtils::parseBlankSegments( strBlankSegments, context.renderContext(), blankSegmentsUnit(), error );

    if ( !error.isEmpty() )
    {
      QgsDebugError( u"Badly formatted blank segment '%1', skip it: %2"_s.arg( strBlankSegments ).arg( error ) );
    }
    else
    {
      // keep only the part/ring we are currently rendering
      const int iPart = context.geometryPartNum() - 1;
      if ( iPart >= 0 && mRingIndex >= 0 && iPart < allBlankSegments.count() && mRingIndex < allBlankSegments.at( iPart ).count() )
      {
        blankSegments = allBlankSegments.at( iPart ).at( mRingIndex );
      }
    }
  }

  if ( qgsDoubleNear( offset, 0.0 ) )
  {
    if ( placements & Qgis::MarkerLinePlacement::Interval )
      renderPolylineInterval( points, context, averageOver, blankSegments );
    if ( placements & Qgis::MarkerLinePlacement::CentralPoint )
      renderPolylineCentral( points, context, averageOver, blankSegments );
    if ( placements & Qgis::MarkerLinePlacement::Vertex )
      renderPolylineVertex( points, context, Qgis::MarkerLinePlacement::Vertex, blankSegments );
    if ( placements & Qgis::MarkerLinePlacement::FirstVertex
         && ( mPlaceOnEveryPart || !mHasRenderedFirstPart ) )
    {
      renderPolylineVertex( points, context, Qgis::MarkerLinePlacement::FirstVertex, blankSegments );
      mHasRenderedFirstPart = mRenderingFeature;
    }
    if ( placements & Qgis::MarkerLinePlacement::InnerVertices )
      renderPolylineVertex( points, context, Qgis::MarkerLinePlacement::InnerVertices, blankSegments );
    if ( placements & Qgis::MarkerLinePlacement::CurvePoint )
      renderPolylineVertex( points, context, Qgis::MarkerLinePlacement::CurvePoint, blankSegments );
    if ( placements & Qgis::MarkerLinePlacement::SegmentCenter )
      renderPolylineVertex( points, context, Qgis::MarkerLinePlacement::SegmentCenter, blankSegments );
    if ( placements & Qgis::MarkerLinePlacement::LastVertex )
      renderPolylineVertex( points, context, Qgis::MarkerLinePlacement::LastVertex, blankSegments );
  }
  else
  {
    context.renderContext().setGeometry( nullptr ); //always use segmented geometry with offset
    QList<QPolygonF> mline = ::offsetLine( points, context.renderContext().convertToPainterUnits( offset, mOffsetUnit, mOffsetMapUnitScale ), context.originalGeometryType() != Qgis::GeometryType::Unknown ? context.originalGeometryType() : Qgis::GeometryType::Line );

    for ( int part = 0; part < mline.count(); ++part )
    {
      const QPolygonF &points2 = mline[ part ];

      if ( placements & Qgis::MarkerLinePlacement::Interval )
        renderPolylineInterval( points2, context, averageOver, blankSegments );
      if ( placements & Qgis::MarkerLinePlacement::CentralPoint )
        renderPolylineCentral( points2, context, averageOver, blankSegments );
      if ( placements & Qgis::MarkerLinePlacement::Vertex )
        renderPolylineVertex( points2, context, Qgis::MarkerLinePlacement::Vertex, blankSegments );
      if ( placements & Qgis::MarkerLinePlacement::InnerVertices )
        renderPolylineVertex( points2, context, Qgis::MarkerLinePlacement::InnerVertices, blankSegments );
      if ( placements & Qgis::MarkerLinePlacement::LastVertex )
        renderPolylineVertex( points2, context, Qgis::MarkerLinePlacement::LastVertex, blankSegments );
      if ( placements & Qgis::MarkerLinePlacement::FirstVertex
           && ( mPlaceOnEveryPart || !mHasRenderedFirstPart ) )
      {
        renderPolylineVertex( points2, context, Qgis::MarkerLinePlacement::FirstVertex, blankSegments );
        mHasRenderedFirstPart = mRenderingFeature;
      }
      if ( placements & Qgis::MarkerLinePlacement::CurvePoint )
        renderPolylineVertex( points2, context, Qgis::MarkerLinePlacement::CurvePoint, blankSegments );
      if ( placements & Qgis::MarkerLinePlacement::SegmentCenter )
        renderPolylineVertex( points2, context, Qgis::MarkerLinePlacement::SegmentCenter, blankSegments );
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
          mRingIndex = i + 1;
          if ( curvePolygon )
          {
            context.renderContext().setGeometry( curvePolygon->interiorRing( i ) );
          }
          if ( scope )
            scope->addVariable( QgsExpressionContextScope::StaticVariable( QgsExpressionContext::EXPR_GEOMETRY_RING_NUM, i + 1, true ) );

          renderPolyline( rings->at( i ), context );
        }
        mOffset = -mOffset;
        mRingIndex = 0;
      }
      break;
      case ExteriorRingOnly:
        break;
    }
  }
}

Qgis::RenderUnit QgsTemplatedLineSymbolLayerBase::outputUnit() const
{
  Qgis::RenderUnit unit = QgsLineSymbolLayer::outputUnit();
  if ( intervalUnit() != unit || mOffsetUnit != unit || offsetAlongLineUnit() != unit )
  {
    return Qgis::RenderUnit::Unknown;
  }
  return unit;
}

void QgsTemplatedLineSymbolLayerBase::setOutputUnit( Qgis::RenderUnit unit )
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
  map[u"rotate"_s] = ( rotateSymbols() ? u"1"_s : u"0"_s );
  map[u"interval"_s] = QString::number( interval() );
  map[u"offset"_s] = QString::number( mOffset );
  map[u"offset_along_line"_s] = QString::number( offsetAlongLine() );
  map[u"offset_along_line_unit"_s] = QgsUnitTypes::encodeUnit( offsetAlongLineUnit() );
  map[u"offset_along_line_map_unit_scale"_s] = QgsSymbolLayerUtils::encodeMapUnitScale( offsetAlongLineMapUnitScale() );
  map[u"offset_unit"_s] = QgsUnitTypes::encodeUnit( mOffsetUnit );
  map[u"offset_map_unit_scale"_s] = QgsSymbolLayerUtils::encodeMapUnitScale( mOffsetMapUnitScale );
  map[u"interval_unit"_s] = QgsUnitTypes::encodeUnit( intervalUnit() );
  map[u"interval_map_unit_scale"_s] = QgsSymbolLayerUtils::encodeMapUnitScale( intervalMapUnitScale() );
  map[u"average_angle_length"_s] = QString::number( mAverageAngleLength );
  map[u"average_angle_unit"_s] = QgsUnitTypes::encodeUnit( mAverageAngleLengthUnit );
  map[u"average_angle_map_unit_scale"_s] = QgsSymbolLayerUtils::encodeMapUnitScale( mAverageAngleLengthMapUnitScale );
  map[u"blank_segments_unit"_s] = QgsUnitTypes::encodeUnit( mBlankSegmentsUnit );

  map[u"placements"_s] = qgsFlagValueToKeys( mPlacements );

  map[u"ring_filter"_s] = QString::number( static_cast< int >( mRingFilter ) );
  map[u"place_on_every_part"_s] = mPlaceOnEveryPart;
  return map;
}

bool QgsTemplatedLineSymbolLayerBase::canCauseArtifactsBetweenAdjacentTiles() const
{
  return mPlaceOnEveryPart
         || ( mPlacements & Qgis::MarkerLinePlacement::Interval )
         || ( mPlacements & Qgis::MarkerLinePlacement::CentralPoint )
         || ( mPlacements & Qgis::MarkerLinePlacement::SegmentCenter );
}

void QgsTemplatedLineSymbolLayerBase::startFeatureRender( const QgsFeature &, QgsRenderContext &context )
{
  installMasks( context, true );

  mRenderingFeature = true;
  mHasRenderedFirstPart = false;
}

void QgsTemplatedLineSymbolLayerBase::stopFeatureRender( const QgsFeature &feature, QgsRenderContext &context )
{
  mRenderingFeature = false;
  if ( mPlaceOnEveryPart  || !( mPlacements & Qgis::MarkerLinePlacement::LastVertex ) )
  {
    removeMasks( context, true );
    return;
  }

  const double prevOpacity = subSymbol()->opacity();
  subSymbol()->setOpacity( prevOpacity * mFeatureSymbolOpacity );

  // render final point
  renderSymbol( mFinalVertex, &feature, context, -1, mCurrentFeatureIsSelected );
  mFeatureSymbolOpacity = 1;
  subSymbol()->setOpacity( prevOpacity );

  removeMasks( context, true );
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
  destLayer->setBlankSegmentsUnit( mBlankSegmentsUnit );
  destLayer->setRingFilter( mRingFilter );
  destLayer->setPlaceOnEveryPart( mPlaceOnEveryPart );

  copyDataDefinedProperties( destLayer );
  copyPaintEffect( destLayer );
}

void QgsTemplatedLineSymbolLayerBase::setCommonProperties( QgsTemplatedLineSymbolLayerBase *destLayer, const QVariantMap &properties )
{
  if ( properties.contains( u"offset"_s ) )
  {
    destLayer->setOffset( properties[u"offset"_s].toDouble() );
  }
  if ( properties.contains( u"offset_unit"_s ) )
  {
    destLayer->setOffsetUnit( QgsUnitTypes::decodeRenderUnit( properties[u"offset_unit"_s].toString() ) );
  }
  if ( properties.contains( u"interval_unit"_s ) )
  {
    destLayer->setIntervalUnit( QgsUnitTypes::decodeRenderUnit( properties[u"interval_unit"_s].toString() ) );
  }
  if ( properties.contains( u"offset_along_line"_s ) )
  {
    destLayer->setOffsetAlongLine( properties[u"offset_along_line"_s].toDouble() );
  }
  if ( properties.contains( u"offset_along_line_unit"_s ) )
  {
    destLayer->setOffsetAlongLineUnit( QgsUnitTypes::decodeRenderUnit( properties[u"offset_along_line_unit"_s].toString() ) );
  }
  if ( properties.contains( ( u"offset_along_line_map_unit_scale"_s ) ) )
  {
    destLayer->setOffsetAlongLineMapUnitScale( QgsSymbolLayerUtils::decodeMapUnitScale( properties[u"offset_along_line_map_unit_scale"_s].toString() ) );
  }

  if ( properties.contains( u"offset_map_unit_scale"_s ) )
  {
    destLayer->setOffsetMapUnitScale( QgsSymbolLayerUtils::decodeMapUnitScale( properties[u"offset_map_unit_scale"_s].toString() ) );
  }
  if ( properties.contains( u"interval_map_unit_scale"_s ) )
  {
    destLayer->setIntervalMapUnitScale( QgsSymbolLayerUtils::decodeMapUnitScale( properties[u"interval_map_unit_scale"_s].toString() ) );
  }

  if ( properties.contains( u"average_angle_length"_s ) )
  {
    destLayer->setAverageAngleLength( properties[u"average_angle_length"_s].toDouble() );
  }
  if ( properties.contains( u"average_angle_unit"_s ) )
  {
    destLayer->setAverageAngleUnit( QgsUnitTypes::decodeRenderUnit( properties[u"average_angle_unit"_s].toString() ) );
  }
  if ( properties.contains( ( u"average_angle_map_unit_scale"_s ) ) )
  {
    destLayer->setAverageAngleMapUnitScale( QgsSymbolLayerUtils::decodeMapUnitScale( properties[u"average_angle_map_unit_scale"_s].toString() ) );
  }
  if ( properties.contains( u"blank_segments_unit"_s ) )
  {
    destLayer->setBlankSegmentsUnit( QgsUnitTypes::decodeRenderUnit( properties[u"blank_segments_unit"_s].toString() ) );
  }

  if ( properties.contains( u"placement"_s ) )
  {
    if ( properties[u"placement"_s] == "vertex"_L1 )
      destLayer->setPlacements( Qgis::MarkerLinePlacement::InnerVertices | Qgis::MarkerLinePlacement::FirstVertex | Qgis::MarkerLinePlacement::LastVertex );
    else if ( properties[u"placement"_s] == "lastvertex"_L1 )
      destLayer->setPlacements( Qgis::MarkerLinePlacement::LastVertex );
    else if ( properties[u"placement"_s] == "firstvertex"_L1 )
      destLayer->setPlacements( Qgis::MarkerLinePlacement::FirstVertex );
    else if ( properties[u"placement"_s] == "centralpoint"_L1 )
      destLayer->setPlacements( Qgis::MarkerLinePlacement::CentralPoint );
    else if ( properties[u"placement"_s] == "curvepoint"_L1 )
      destLayer->setPlacements( Qgis::MarkerLinePlacement::CurvePoint );
    else if ( properties[u"placement"_s] == "segmentcenter"_L1 )
      destLayer->setPlacements( Qgis::MarkerLinePlacement::SegmentCenter );
    else
      destLayer->setPlacements( Qgis::MarkerLinePlacement::Interval );
  }
  else if ( properties.contains( u"placements"_s ) )
  {
    Qgis::MarkerLinePlacements placements = qgsFlagKeysToValue( properties.value( u"placements"_s ).toString(), Qgis::MarkerLinePlacements() );
    destLayer->setPlacements( placements );
  }

  if ( properties.contains( u"ring_filter"_s ) )
  {
    destLayer->setRingFilter( static_cast< RenderRingFilter>( properties[u"ring_filter"_s].toInt() ) );
  }

  destLayer->setPlaceOnEveryPart( properties.value( u"place_on_every_part"_s, true ).toBool() );

  destLayer->restoreOldDataDefinedProperties( properties );
}

///@cond PRIVATE

/*!
 * Helper class to go through BlankSegment while rendering points.
 * The class expects to be used while rendering points in the correct order
 */
class BlankSegmentsWalker
{
  public :

    BlankSegmentsWalker( const QPolygonF &points, const QgsBlankSegmentUtils::BlankSegments &blankSegments )
      : mBlankSegments( blankSegments )
      , mPoints( points )
      , mItBlankSegment( blankSegments.cbegin() )
    {
      mDistances.reserve( mPoints.count() );
      mDistances << 0; // first point is start, so distance is 0
    }

    bool insideBlankSegment( double distance )
    {
      while ( mItBlankSegment != mBlankSegments.cend() && distance > mItBlankSegment->second )
      {
        ++mItBlankSegment;
      }

      return ( mItBlankSegment != mBlankSegments.cend() && distance >= mItBlankSegment->first );
    }


    // pointIndex : index of the point before point
    bool insideBlankSegment( const QPointF &point, int pointIndex )
    {
      if ( pointIndex < 0 || pointIndex >= mPoints.count() )
        return false;

      // compute distances and fill distances array
      if ( pointIndex >= mDistances.count() )
      {
        for ( int i = static_cast<int>( mDistances.count() ); i < pointIndex + 1; i++ )
        {
          const QPointF diff = mPoints.at( i ) - mPoints.at( i - 1 );
          const double distance = std::sqrt( std::pow( diff.x(), 2 ) + std::pow( diff.y(), 2 ) );
          const double totalDistance = distance + mDistances.last();
          mDistances << totalDistance;
        }
      }

      const QPointF diff = mPoints.at( pointIndex ) - point;
      const double distance = std::sqrt( std::pow( diff.x(), 2 ) + std::pow( diff.y(), 2 ) );
      const double currentDistance = mDistances.at( pointIndex ) + distance;

      return insideBlankSegment( currentDistance );
    }

  private:

    const QgsBlankSegmentUtils::BlankSegments &mBlankSegments;
    const QPolygonF &mPoints;
    QList<double> mDistances;
    QgsBlankSegmentUtils::BlankSegments::const_iterator mItBlankSegment;
};


///@endcond PRIVATE


void QgsTemplatedLineSymbolLayerBase::renderPolylineInterval( const QPolygonF &points, QgsSymbolRenderContext &context, double averageOver, const QgsBlankSegmentUtils::BlankSegments &blankSegments )
{
  if ( points.isEmpty() )
    return;

  const bool useSelectedColor = shouldRenderUsingSelectionColor( context );
  double lengthLeft = 0; // how much is left until next marker

  QgsRenderContext &rc = context.renderContext();
  double interval = mInterval;

  QgsExpressionContextScope *scope = new QgsExpressionContextScope();
  QgsExpressionContextScopePopper scopePopper( context.renderContext().expressionContext(), scope );

  if ( mDataDefinedProperties.isActive( QgsSymbolLayer::Property::Interval ) )
  {
    context.setOriginalValueVariable( mInterval );
    interval = mDataDefinedProperties.valueAsDouble( QgsSymbolLayer::Property::Interval, context.renderContext().expressionContext(), mInterval );
  }
  if ( interval <= 0 )
  {
    interval = 0.1;
  }
  double offsetAlongLine = mOffsetAlongLine;
  if ( mDataDefinedProperties.isActive( QgsSymbolLayer::Property::OffsetAlongLine ) )
  {
    context.setOriginalValueVariable( mOffsetAlongLine );
    offsetAlongLine = mDataDefinedProperties.valueAsDouble( QgsSymbolLayer::Property::OffsetAlongLine, context.renderContext().expressionContext(), mOffsetAlongLine );
  }

  double painterUnitInterval = rc.convertToPainterUnits( interval, intervalUnit(), intervalMapUnitScale() );
  if ( intervalUnit() == Qgis::RenderUnit::MetersInMapUnits && ( rc.flags() & Qgis::RenderContextFlag::RenderSymbolPreview || rc.flags() & Qgis::RenderContextFlag::RenderLayerTree ) )
  {
    // rendering for symbol previews -- an interval in meters in map units can't be calculated, so treat the size as millimeters
    // and clamp it to a reasonable range. It's the best we can do in this situation!
    painterUnitInterval = std::min( std::max( rc.convertToPainterUnits( interval, Qgis::RenderUnit::Millimeters ), 10.0 ), 100.0 );
  }

  constexpr double EPSILON = 1e-5;
  if ( painterUnitInterval < EPSILON )
    return;

  double painterUnitOffsetAlongLine = 0;

  // only calculated if we need it!
  double totalLength = -1;

  if ( !qgsDoubleNear( offsetAlongLine, 0 ) )
  {
    switch ( offsetAlongLineUnit() )
    {
      case Qgis::RenderUnit::Millimeters:
      case Qgis::RenderUnit::MapUnits:
      case Qgis::RenderUnit::Pixels:
      case Qgis::RenderUnit::Points:
      case Qgis::RenderUnit::Inches:
      case Qgis::RenderUnit::Unknown:
      case Qgis::RenderUnit::MetersInMapUnits:
        painterUnitOffsetAlongLine = rc.convertToPainterUnits( offsetAlongLine, offsetAlongLineUnit(), offsetAlongLineMapUnitScale() );
        break;
      case Qgis::RenderUnit::Percentage:
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

  if ( offsetAlongLineUnit() == Qgis::RenderUnit::MetersInMapUnits && ( rc.flags() & Qgis::RenderContextFlag::RenderSymbolPreview || rc.flags() & Qgis::RenderContextFlag::RenderLayerTree ) )
  {
    // rendering for symbol previews -- an offset in meters in map units can't be calculated, so treat the size as millimeters
    // and clamp it to a reasonable range. It's the best we can do in this situation!
    painterUnitOffsetAlongLine = std::min( std::max( rc.convertToPainterUnits( offsetAlongLine, Qgis::RenderUnit::Millimeters ), 3.0 ), 100.0 );
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

    QList<int> pointIndices; // keep a track on original pointIndices so we can decide later whether or not symbol points belong to a blank segment
    collectOffsetPoints( points, symbolPoints, painterUnitInterval, lengthLeft, blankSegments.isEmpty() ? nullptr : &pointIndices );

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
      collectOffsetPoints( points, angleStartPoints, painterUnitInterval, lengthLeft + averageOver, nullptr, 0, symbolPoints.size() );
    }
    else
    {
      collectOffsetPoints( points, angleStartPoints, painterUnitInterval, 0, nullptr, averageOver - painterUnitOffsetAlongLine, symbolPoints.size() );
    }
    collectOffsetPoints( points, angleEndPoints, painterUnitInterval, lengthLeft - averageOver, nullptr, 0, symbolPoints.size() );

    int pointNum = 0;
    BlankSegmentsWalker blankSegmentsWalker( points, blankSegments );
    for ( int i = 0; i < symbolPoints.size(); ++ i )
    {
      if ( context.renderContext().renderingStopped() )
        break;

      const QPointF pt = symbolPoints[i];
      if ( i < pointIndices.count() && blankSegmentsWalker.insideBlankSegment( pt, pointIndices.at( i ) ) )
        // skip the rendering
        continue;

      const QPointF startPt = angleStartPoints[i];
      const QPointF endPt = angleEndPoints[i];

      Line l( startPt, endPt );
      // rotate marker (if desired)
      if ( rotateSymbols() )
      {
        setSymbolLineAngle( l.angle() * 180 / M_PI );
      }

      scope->addVariable( QgsExpressionContextScope::StaticVariable( QgsExpressionContext::EXPR_GEOMETRY_POINT_NUM, ++pointNum, true ) );
      renderSymbol( pt, context.feature(), rc, -1, useSelectedColor );
    }
  }
  else
  {

    // not averaging line angle -- always use exact section angle
    int pointNum = 0;
    QPointF lastPt = points[0];
    BlankSegmentsWalker itBlankSegment( points, blankSegments );
    for ( int i = 1; i < points.count(); ++i )
    {
      if ( context.renderContext().renderingStopped() )
        break;

      const QPointF &pt = points[i];

      if ( lastPt == pt ) // must not be equal!
        continue;

      // for each line, find out dx and dy, and length
      Line l( lastPt, pt );
      QPointF diff = l.diffForInterval( painterUnitInterval );

      // if there's some length left from previous line
      // use only the rest for the first point in new line segment
      // "c" is 1 for regular point or in interval (0,1] for begin of line segment
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
        c = 1; // reset c (if wasn't 1 already)

        // we draw
        if ( !itBlankSegment.insideBlankSegment( lastPt, i - 1 ) )
        {
          scope->addVariable( QgsExpressionContextScope::StaticVariable( QgsExpressionContext::EXPR_GEOMETRY_POINT_NUM, ++pointNum, true ) );
          renderSymbol( lastPt, context.feature(), rc, -1, useSelectedColor );
        }

        lengthLeft -= painterUnitInterval;
      }

      lastPt = pt;
    }

  }
}

static double _averageAngle( QPointF prevPt, QPointF pt, QPointF nextPt )
{
  // calc average angle between the previous and next point
  double a1 = Line( prevPt, pt ).angle();
  double a2 = Line( pt, nextPt ).angle();
  double unitX = std::cos( a1 ) + std::cos( a2 ), unitY = std::sin( a1 ) + std::sin( a2 );

  return std::atan2( unitY, unitX );
}

void QgsTemplatedLineSymbolLayerBase::renderPolylineVertex( const QPolygonF &points, QgsSymbolRenderContext &context, Qgis::MarkerLinePlacement placement, const QgsBlankSegmentUtils::BlankSegments &blankSegments )
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
  if ( mDataDefinedProperties.isActive( QgsSymbolLayer::Property::OffsetAlongLine ) )
  {
    context.setOriginalValueVariable( mOffsetAlongLine );
    offsetAlongLine = mDataDefinedProperties.valueAsDouble( QgsSymbolLayer::Property::OffsetAlongLine, context.renderContext().expressionContext(), mOffsetAlongLine );
  }

  // only calculated if we need it!!
  double totalLength = -1;
  if ( !qgsDoubleNear( offsetAlongLine, 0.0 ) )
  {
    //scale offset along line
    switch ( offsetAlongLineUnit() )
    {
      case Qgis::RenderUnit::Millimeters:
      case Qgis::RenderUnit::MapUnits:
      case Qgis::RenderUnit::Pixels:
      case Qgis::RenderUnit::Points:
      case Qgis::RenderUnit::Inches:
      case Qgis::RenderUnit::Unknown:
      case Qgis::RenderUnit::MetersInMapUnits:
        offsetAlongLine = rc.convertToPainterUnits( offsetAlongLine, offsetAlongLineUnit(), offsetAlongLineMapUnitScale() );
        break;
      case Qgis::RenderUnit::Percentage:
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

  const bool useSelectedColor = shouldRenderUsingSelectionColor( context );
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
        renderSymbol( mapPoint, context.feature(), rc, -1, useSelectedColor );
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
    renderOffsetVertexAlongLine( points, i, distance, context, placement, blankSegments );

    return;
  }

  QPointF prevPoint;
  if ( placement == Qgis::MarkerLinePlacement::SegmentCenter && !points.empty() )
    prevPoint = points.at( 0 );

  QPointF symbolPoint;
  BlankSegmentsWalker blankSegmentsWalker( points, blankSegments );
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
    if ( ( i != points.count() - 1 || placement != Qgis::MarkerLinePlacement::LastVertex || mPlaceOnEveryPart || !mRenderingFeature )
         && !blankSegmentsWalker.insideBlankSegment( symbolPoint, i ) )
      renderSymbol( symbolPoint, context.feature(), rc, -1, useSelectedColor );
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
          angle = Line( pt, nextPt ).angle();
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
          angle = Line( prevPt, pt ).angle();
          return angle;
        }
        --vertex;
      }
    }
  }
  return angle;
}

void QgsTemplatedLineSymbolLayerBase::renderOffsetVertexAlongLine( const QPolygonF &points, int vertex, double distance, QgsSymbolRenderContext &context, Qgis::MarkerLinePlacement placement, const QgsBlankSegmentUtils::BlankSegments &blankSegments )
{
  if ( points.isEmpty() )
    return;

  QgsRenderContext &rc = context.renderContext();
  const bool useSelectedColor = shouldRenderUsingSelectionColor( context );
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
      renderSymbol( points[vertex], context.feature(), rc, -1, useSelectedColor );
    return;
  }

  int pointIncrement = distance > 0 ? 1 : -1;
  QPointF previousPoint = points[vertex];
  int startPoint = distance > 0 ? std::min( vertex + 1, static_cast<int>( points.count() ) - 1 ) : std::max( vertex - 1, 0 );
  int endPoint = distance > 0 ? points.count() - 1 : 0;
  double distanceLeft = std::fabs( distance );
  BlankSegmentsWalker blankSegmentsWalker( points, blankSegments );

  for ( int i = startPoint; pointIncrement > 0 ? i <= endPoint : i >= endPoint; i += pointIncrement )
  {
    const QPointF &pt = points[i];

    if ( previousPoint == pt ) // must not be equal!
      continue;

    // create line segment
    Line l( previousPoint, pt );

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
      if ( ( placement != Qgis::MarkerLinePlacement::LastVertex || mPlaceOnEveryPart || !mRenderingFeature )
           && !blankSegmentsWalker.insideBlankSegment( markerPoint, i - 1 ) )
        renderSymbol( markerPoint, context.feature(), rc, -1, useSelectedColor );
      return;
    }

    distanceLeft -= l.length();
    previousPoint = pt;
  }

  //didn't find point
}

void QgsTemplatedLineSymbolLayerBase::collectOffsetPoints( const QVector<QPointF> &p, QVector<QPointF> &dest, double intervalPainterUnits, double initialOffset,
    QList<int> *pointIndices,
    double initialLag, int numberPointsRequired )
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

      Line l( lastPt, pt );
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
    Line l( lastPt, pt );
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
      if ( pointIndices )
        *pointIndices << i - 1;
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

void QgsTemplatedLineSymbolLayerBase::renderPolylineCentral( const QPolygonF &points, QgsSymbolRenderContext &context, double averageAngleOver, const QgsBlankSegmentUtils::BlankSegments &blankSegments )
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

    BlankSegmentsWalker blankSegmentsWalker( points, blankSegments );
    if ( blankSegmentsWalker.insideBlankSegment( midPoint ) )
      return;

    QPointF pt;
    double thisSymbolAngle = 0;

    if ( averageAngleOver > 0 && !qgsDoubleNear( averageAngleOver, 0.0 ) )
    {
      QVector< QPointF > angleStartPoints;
      QVector< QPointF > symbolPoints;
      QVector< QPointF > angleEndPoints;

      // collectOffsetPoints will have the first point in the line as the first result -- we don't want this, we need the second
      // already dealt with blank segment before, no need to make them check again
      collectOffsetPoints( points, symbolPoints, midPoint, midPoint, nullptr, 0.0, 2 );
      collectOffsetPoints( points, angleStartPoints, midPoint, 0, nullptr, averageAngleOver, 2 );
      collectOffsetPoints( points, angleEndPoints, midPoint, midPoint - averageAngleOver, nullptr, 0, 2 );

      pt = symbolPoints.at( 1 );
      Line l( angleStartPoints.at( 1 ), angleEndPoints.at( 1 ) );
      thisSymbolAngle = l.angle();
    }
    else
    {
      // find the segment where the central point lies
      it = points.constBegin();
      last = *it;
      qreal last_at = 0, next_at = 0;
      QPointF next;
      for ( ++it; it != points.constEnd(); ++it )
      {
        next = *it;
        next_at += std::sqrt( ( last.x() - it->x() ) * ( last.x() - it->x() ) +
                              ( last.y() - it->y() ) * ( last.y() - it->y() ) );
        if ( next_at >= midPoint )
          break; // we have reached the center
        last = *it;
        last_at = next_at;
      }

      // find out the central point on segment
      Line l( last, next ); // for line angle
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

    const bool useSelectedColor = shouldRenderUsingSelectionColor( context );
    renderSymbol( pt, context.feature(), context.renderContext(), -1, useSelectedColor );
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

  if ( props.contains( u"interval"_s ) )
    interval = props[u"interval"_s].toDouble();
  if ( props.contains( u"rotate"_s ) )
    rotate = ( props[u"rotate"_s].toString() == "1"_L1 );

  auto x = std::make_unique< QgsMarkerLineSymbolLayer >( rotate, interval );
  setCommonProperties( x.get(), props );
  return x.release();
}

QString QgsMarkerLineSymbolLayer::layerType() const
{
  return u"MarkerLine"_s;
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
  Qgis::SymbolRenderHints hints = Qgis::SymbolRenderHint::IsSymbolLayerSubSymbol;
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
  auto x = std::make_unique< QgsMarkerLineSymbolLayer >( rotateSymbols(), interval() );
  copyTemplateSymbolProperties( x.get() );
  return x.release();
}

void QgsMarkerLineSymbolLayer::toSld( QDomDocument &doc, QDomElement &element, const QVariantMap &props ) const
{
  QgsSldExportContext context;
  context.setExtraProperties( props );
  toSld( doc, element, context );
}

bool QgsMarkerLineSymbolLayer::toSld( QDomDocument &doc, QDomElement &element, QgsSldExportContext &context ) const
{
  const QVariantMap props = context.extraProperties();
  for ( int i = 0; i < mMarker->symbolLayerCount(); i++ )
  {
    QDomElement symbolizerElem = doc.createElement( u"se:LineSymbolizer"_s );
    if ( !props.value( u"uom"_s, QString() ).toString().isEmpty() )
      symbolizerElem.setAttribute( u"uom"_s, props.value( u"uom"_s, QString() ).toString() );
    element.appendChild( symbolizerElem );

    // <Geometry>
    QgsSymbolLayerUtils::createGeometryElement( doc, symbolizerElem, props.value( u"geom"_s, QString() ).toString(), context );

    QString gap;
    if ( placements() & Qgis::MarkerLinePlacement::FirstVertex )
      symbolizerElem.appendChild( QgsSymbolLayerUtils::createVendorOptionElement( doc, u"placement"_s, u"firstPoint"_s ) );
    if ( placements() & Qgis::MarkerLinePlacement::LastVertex )
      symbolizerElem.appendChild( QgsSymbolLayerUtils::createVendorOptionElement( doc, u"placement"_s, u"lastPoint"_s ) );
    if ( placements() & Qgis::MarkerLinePlacement::CentralPoint )
      symbolizerElem.appendChild( QgsSymbolLayerUtils::createVendorOptionElement( doc, u"placement"_s, u"centralPoint"_s ) );
    if ( placements() & Qgis::MarkerLinePlacement::Vertex )
      // no way to get line/polygon's vertices, use a VendorOption
      symbolizerElem.appendChild( QgsSymbolLayerUtils::createVendorOptionElement( doc, u"placement"_s, u"points"_s ) );

    if ( placements() & Qgis::MarkerLinePlacement::Interval )
    {
      double interval = QgsSymbolLayerUtils::rescaleUom( QgsMarkerLineSymbolLayer::interval(), intervalUnit(), props );
      gap = qgsDoubleToString( interval );
    }

    if ( !rotateSymbols() )
    {
      // markers in LineSymbolizer must be drawn following the line orientation,
      // use a VendorOption when no marker rotation
      symbolizerElem.appendChild( QgsSymbolLayerUtils::createVendorOptionElement( doc, u"rotateMarker"_s, u"0"_s ) );
    }

    // <Stroke>
    QDomElement strokeElem = doc.createElement( u"se:Stroke"_s );
    symbolizerElem.appendChild( strokeElem );

    // <GraphicStroke>
    QDomElement graphicStrokeElem = doc.createElement( u"se:GraphicStroke"_s );
    strokeElem.appendChild( graphicStrokeElem );

    QgsSymbolLayer *layer = mMarker->symbolLayer( i );
    if ( QgsMarkerSymbolLayer *markerLayer = dynamic_cast<QgsMarkerSymbolLayer *>( layer ) )
    {
      markerLayer->writeSldMarker( doc, graphicStrokeElem, context );
    }
    else if ( layer )
    {
      QgsDebugError( u"QgsMarkerSymbolLayer expected, %1 found. Skip it."_s.arg( layer->layerType() ) );
    }
    else
    {
      QgsDebugError( u"Missing marker line symbol layer. Skip it."_s );
    }

    if ( !gap.isEmpty() )
    {
      QDomElement gapElem = doc.createElement( u"se:Gap"_s );
      QgsSymbolLayerUtils::createExpressionElement( doc, gapElem, gap, context );
      graphicStrokeElem.appendChild( gapElem );
    }

    if ( !qgsDoubleNear( mOffset, 0.0 ) )
    {
      QDomElement perpOffsetElem = doc.createElement( u"se:PerpendicularOffset"_s );
      double offset = QgsSymbolLayerUtils::rescaleUom( mOffset, mOffsetUnit, props );
      perpOffsetElem.appendChild( doc.createTextNode( qgsDoubleToString( offset ) ) );
      symbolizerElem.appendChild( perpOffsetElem );
    }
  }
  return true;
}

QgsSymbolLayer *QgsMarkerLineSymbolLayer::createFromSld( QDomElement &element )
{
  QgsDebugMsgLevel( u"Entered."_s, 4 );

  QDomElement strokeElem = element.firstChildElement( u"Stroke"_s );
  if ( strokeElem.isNull() )
    return nullptr;

  QDomElement graphicStrokeElem = strokeElem.firstChildElement( u"GraphicStroke"_s );
  if ( graphicStrokeElem.isNull() )
    return nullptr;

  // retrieve vendor options
  bool rotateMarker = true;
  Qgis::MarkerLinePlacement placement = Qgis::MarkerLinePlacement::Interval;

  QgsStringMap vendorOptions = QgsSymbolLayerUtils::getVendorOptionList( element );
  for ( QgsStringMap::iterator it = vendorOptions.begin(); it != vendorOptions.end(); ++it )
  {
    if ( it.key() == "placement"_L1 )
    {
      if ( it.value() == "points"_L1 )
        placement = Qgis::MarkerLinePlacement::Vertex;
      else if ( it.value() == "firstPoint"_L1 )
        placement = Qgis::MarkerLinePlacement::FirstVertex;
      else if ( it.value() == "lastPoint"_L1 )
        placement = Qgis::MarkerLinePlacement::LastVertex;
      else if ( it.value() == "centralPoint"_L1 )
        placement = Qgis::MarkerLinePlacement::CentralPoint;
    }
    else if ( it.value() == "rotateMarker"_L1 )
    {
      rotateMarker = it.value() == "0"_L1;
    }
  }

  std::unique_ptr< QgsMarkerSymbol > marker;

  std::unique_ptr< QgsSymbolLayer > l = QgsSymbolLayerUtils::createMarkerLayerFromSld( graphicStrokeElem );
  if ( l )
  {
    QgsSymbolLayerList layers;
    layers.append( l.release() );
    marker = std::make_unique<QgsMarkerSymbol>( layers );
  }

  if ( !marker )
    return nullptr;

  double interval = 0.0;
  QDomElement gapElem = graphicStrokeElem.firstChildElement( u"Gap"_s );
  if ( !gapElem.isNull() )
  {
    bool ok;
    double d = gapElem.firstChild().firstChild().nodeValue().toDouble( &ok );
    if ( ok )
      interval = d;
  }

  double offset = 0.0;
  QDomElement perpOffsetElem = graphicStrokeElem.firstChildElement( u"PerpendicularOffset"_s );
  if ( !perpOffsetElem.isNull() )
  {
    bool ok;
    double d = perpOffsetElem.firstChild().nodeValue().toDouble( &ok );
    if ( ok )
      offset = d;
  }

  double scaleFactor = 1.0;
  const QString uom = element.attribute( u"uom"_s );
  Qgis::RenderUnit sldUnitSize = QgsSymbolLayerUtils::decodeSldUom( uom, &scaleFactor );
  interval = interval * scaleFactor;
  offset = offset * scaleFactor;

  QgsMarkerLineSymbolLayer *x = new QgsMarkerLineSymbolLayer( rotateMarker );
  x->setOutputUnit( sldUnitSize );
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
  if ( key == QgsSymbolLayer::Property::Width && mMarker && property )
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

void QgsMarkerLineSymbolLayer::setOutputUnit( Qgis::RenderUnit unit )
{
  QgsTemplatedLineSymbolLayerBase::setOutputUnit( unit );
  mMarker->setOutputUnit( unit );
}

bool QgsMarkerLineSymbolLayer::usesMapUnits() const
{
  return  intervalUnit() == Qgis::RenderUnit::MapUnits || intervalUnit() == Qgis::RenderUnit::MetersInMapUnits
          || offsetAlongLineUnit() == Qgis::RenderUnit::MapUnits || offsetAlongLineUnit() == Qgis::RenderUnit::MetersInMapUnits
          || averageAngleUnit() == Qgis::RenderUnit::MapUnits || averageAngleUnit() == Qgis::RenderUnit::MetersInMapUnits
          || mWidthUnit == Qgis::RenderUnit::MapUnits || mWidthUnit == Qgis::RenderUnit::MetersInMapUnits
          || mOffsetUnit == Qgis::RenderUnit::MapUnits || mOffsetUnit == Qgis::RenderUnit::MetersInMapUnits
          || blankSegmentsUnit() == Qgis::RenderUnit::MapUnits
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

  if ( props.contains( u"interval"_s ) )
    interval = props[u"interval"_s].toDouble();
  if ( props.contains( u"rotate"_s ) )
    rotate = ( props[u"rotate"_s] == "1"_L1 );

  auto x = std::make_unique< QgsHashedLineSymbolLayer >( rotate, interval );
  setCommonProperties( x.get(), props );
  if ( props.contains( u"hash_angle"_s ) )
  {
    x->setHashAngle( props[u"hash_angle"_s].toDouble() );
  }

  if ( props.contains( u"hash_length"_s ) )
    x->setHashLength( props[u"hash_length"_s].toDouble() );

  if ( props.contains( u"hash_length_unit"_s ) )
    x->setHashLengthUnit( QgsUnitTypes::decodeRenderUnit( props[u"hash_length_unit"_s].toString() ) );

  if ( props.contains( u"hash_length_map_unit_scale"_s ) )
    x->setHashLengthMapUnitScale( QgsSymbolLayerUtils::decodeMapUnitScale( props[u"hash_length_map_unit_scale"_s].toString() ) );

  return x.release();
}

QString QgsHashedLineSymbolLayer::layerType() const
{
  return u"HashLine"_s;
}

void QgsHashedLineSymbolLayer::startRender( QgsSymbolRenderContext &context )
{
  // if being rotated, it gets initialized with every line segment
  Qgis::SymbolRenderHints hints = Qgis::SymbolRenderHint::IsSymbolLayerSubSymbol;
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
  map[ u"hash_angle"_s ] = QString::number( mHashAngle );

  map[u"hash_length"_s] = QString::number( mHashLength );
  map[u"hash_length_unit"_s] = QgsUnitTypes::encodeUnit( mHashLengthUnit );
  map[u"hash_length_map_unit_scale"_s] = QgsSymbolLayerUtils::encodeMapUnitScale( mHashLengthMapUnitScale );

  return map;
}

QgsHashedLineSymbolLayer *QgsHashedLineSymbolLayer::clone() const
{
  auto x = std::make_unique< QgsHashedLineSymbolLayer >( rotateSymbols(), interval() );
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

void QgsHashedLineSymbolLayer::setOutputUnit( Qgis::RenderUnit unit )
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
  if ( key == QgsSymbolLayer::Property::Width && mHashSymbol && property )
  {
    mHashSymbol->setDataDefinedWidth( property );
  }
  QgsLineSymbolLayer::setDataDefinedProperty( key, property );
}

bool QgsHashedLineSymbolLayer::usesMapUnits() const
{
  return mHashLengthUnit == Qgis::RenderUnit::MapUnits || mHashLengthUnit == Qgis::RenderUnit::MetersInMapUnits
         || intervalUnit() == Qgis::RenderUnit::MapUnits || intervalUnit() == Qgis::RenderUnit::MetersInMapUnits
         || offsetAlongLineUnit() == Qgis::RenderUnit::MapUnits || offsetAlongLineUnit() == Qgis::RenderUnit::MetersInMapUnits
         || averageAngleUnit() == Qgis::RenderUnit::MapUnits || averageAngleUnit() == Qgis::RenderUnit::MetersInMapUnits
         || mWidthUnit == Qgis::RenderUnit::MapUnits || mWidthUnit == Qgis::RenderUnit::MetersInMapUnits
         || mOffsetUnit == Qgis::RenderUnit::MapUnits || mOffsetUnit == Qgis::RenderUnit::MetersInMapUnits
         || blankSegmentsUnit() == Qgis::RenderUnit::MapUnits
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
  if ( mDataDefinedProperties.isActive( QgsSymbolLayer::Property::LineDistance ) )
  {
    context.expressionContext().setOriginalValueVariable( mHashLength );
    lineLength = mDataDefinedProperties.valueAsDouble( QgsSymbolLayer::Property::LineDistance, context.expressionContext(), lineLength );
  }
  const double w = context.convertToPainterUnits( lineLength, mHashLengthUnit, mHashLengthMapUnitScale ) / 2.0;

  double hashAngle = mHashAngle;
  if ( mDataDefinedProperties.isActive( QgsSymbolLayer::Property::LineAngle ) )
  {
    context.expressionContext().setOriginalValueVariable( mHashAngle );
    hashAngle = mDataDefinedProperties.valueAsDouble( QgsSymbolLayer::Property::LineAngle, context.expressionContext(), hashAngle );
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
  if ( mDataDefinedProperties.isActive( QgsSymbolLayer::Property::Offset ) )
  {
    context.setOriginalValueVariable( offset );
    offset = mDataDefinedProperties.valueAsDouble( QgsSymbolLayer::Property::Offset, context.renderContext().expressionContext(), offset );
  }

  QPolygonF offsetPoints;
  if ( qgsDoubleNear( offset, 0 ) )
  {
    renderLine( points, context, patternThickness, patternLength, brush );
  }
  else
  {
    const double scaledOffset = context.renderContext().convertToPainterUnits( offset, mOffsetUnit, mOffsetMapUnitScale );

    const QList<QPolygonF> offsetLine = ::offsetLine( points, scaledOffset, context.originalGeometryType() != Qgis::GeometryType::Unknown ? context.originalGeometryType() : Qgis::GeometryType::Line );
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
  if ( mDataDefinedProperties.isActive( QgsSymbolLayer::Property::JoinStyle ) )
  {
    context.setOriginalValueVariable( QgsSymbolLayerUtils::encodePenJoinStyle( join ) );
    QVariant exprVal = mDataDefinedProperties.value( QgsSymbolLayer::Property::JoinStyle, context.renderContext().expressionContext() );
    if ( !QgsVariantUtils::isNull( exprVal ) )
      join = QgsSymbolLayerUtils::decodePenJoinStyle( exprVal.toString() );
  }

  Qt::PenCapStyle cap = mPenCapStyle;
  if ( mDataDefinedProperties.isActive( QgsSymbolLayer::Property::CapStyle ) )
  {
    context.setOriginalValueVariable( QgsSymbolLayerUtils::encodePenCapStyle( cap ) );
    QVariant exprVal = mDataDefinedProperties.value( QgsSymbolLayer::Property::CapStyle, context.renderContext().expressionContext() );
    if ( !QgsVariantUtils::isNull( exprVal ) )
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
    const double segmentAngleDegrees = 180.0 / M_PI * QgsGeometryUtilsBase::lineAngle( segmentStartPoint.x(), segmentStartPoint.y(),
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
        const double lastSegmentAngleDegrees = 180.0 / M_PI * QgsGeometryUtilsBase::lineAngle( points.at( points.size() - 2 ).x(), points.at( points.size() - 2 ).y(),
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
      const double nextSegmentAngleDegrees = 180.0 / M_PI * QgsGeometryUtilsBase::lineAngle( segmentEndPoint.x(), segmentEndPoint.y(),
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
  auto res = std::make_unique<QgsRasterLineSymbolLayer>();

  if ( properties.contains( u"line_width"_s ) )
  {
    res->setWidth( properties[u"line_width"_s].toDouble() );
  }
  if ( properties.contains( u"line_width_unit"_s ) )
  {
    res->setWidthUnit( QgsUnitTypes::decodeRenderUnit( properties[u"line_width_unit"_s].toString() ) );
  }
  if ( properties.contains( u"width_map_unit_scale"_s ) )
  {
    res->setWidthMapUnitScale( QgsSymbolLayerUtils::decodeMapUnitScale( properties[u"width_map_unit_scale"_s].toString() ) );
  }

  if ( properties.contains( u"imageFile"_s ) )
    res->setPath( properties[u"imageFile"_s].toString() );

  if ( properties.contains( u"offset"_s ) )
  {
    res->setOffset( properties[u"offset"_s].toDouble() );
  }
  if ( properties.contains( u"offset_unit"_s ) )
  {
    res->setOffsetUnit( QgsUnitTypes::decodeRenderUnit( properties[u"offset_unit"_s].toString() ) );
  }
  if ( properties.contains( u"offset_map_unit_scale"_s ) )
  {
    res->setOffsetMapUnitScale( QgsSymbolLayerUtils::decodeMapUnitScale( properties[u"offset_map_unit_scale"_s].toString() ) );
  }

  if ( properties.contains( u"joinstyle"_s ) )
    res->setPenJoinStyle( QgsSymbolLayerUtils::decodePenJoinStyle( properties[u"joinstyle"_s].toString() ) );
  if ( properties.contains( u"capstyle"_s ) )
    res->setPenCapStyle( QgsSymbolLayerUtils::decodePenCapStyle( properties[u"capstyle"_s].toString() ) );

  if ( properties.contains( u"alpha"_s ) )
  {
    res->setOpacity( properties[u"alpha"_s].toDouble() );
  }

  return res.release();
}


QVariantMap QgsRasterLineSymbolLayer::properties() const
{
  QVariantMap map;
  map[u"imageFile"_s] = mPath;

  map[u"line_width"_s] = QString::number( mWidth );
  map[u"line_width_unit"_s] = QgsUnitTypes::encodeUnit( mWidthUnit );
  map[u"width_map_unit_scale"_s] = QgsSymbolLayerUtils::encodeMapUnitScale( mWidthMapUnitScale );

  map[u"joinstyle"_s] = QgsSymbolLayerUtils::encodePenJoinStyle( mPenJoinStyle );
  map[u"capstyle"_s] = QgsSymbolLayerUtils::encodePenCapStyle( mPenCapStyle );

  map[u"offset"_s] = QString::number( mOffset );
  map[u"offset_unit"_s] = QgsUnitTypes::encodeUnit( mOffsetUnit );
  map[u"offset_map_unit_scale"_s] = QgsSymbolLayerUtils::encodeMapUnitScale( mOffsetMapUnitScale );

  map[u"alpha"_s] = QString::number( mOpacity );

  return map;
}

QgsRasterLineSymbolLayer *QgsRasterLineSymbolLayer::clone() const
{
  auto res = std::make_unique< QgsRasterLineSymbolLayer >( mPath );
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
  const QVariantMap::iterator it = properties.find( u"imageFile"_s );
  if ( it != properties.end() && it.value().userType() == QMetaType::Type::QString )
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
  return u"RasterLine"_s;
}

Qgis::SymbolLayerFlags QgsRasterLineSymbolLayer::flags() const
{
  return QgsAbstractBrushedLineSymbolLayer::flags() | Qgis::SymbolLayerFlag::CanCalculateMaskGeometryPerFeature;
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
  if ( mDataDefinedProperties.isActive( QgsSymbolLayer::Property::StrokeWidth )
       || mDataDefinedProperties.isActive( QgsSymbolLayer::Property::File )
       || mDataDefinedProperties.isActive( QgsSymbolLayer::Property::Opacity ) )
  {
    QString path = mPath;
    if ( mDataDefinedProperties.isActive( QgsSymbolLayer::Property::File ) )
    {
      context.setOriginalValueVariable( path );
      path = mDataDefinedProperties.valueAsString( QgsSymbolLayer::Property::File, context.renderContext().expressionContext(), path );
    }

    double strokeWidth = mWidth;
    if ( mDataDefinedProperties.isActive( QgsSymbolLayer::Property::StrokeWidth ) )
    {
      context.setOriginalValueVariable( strokeWidth );
      strokeWidth = mDataDefinedProperties.valueAsDouble( QgsSymbolLayer::Property::StrokeWidth, context.renderContext().expressionContext(), strokeWidth );
    }
    const double scaledHeight = context.renderContext().convertToPainterUnits( strokeWidth, mWidthUnit, mWidthMapUnitScale );

    const QSize originalSize = QgsApplication::imageCache()->originalSize( path, ( context.renderContext().flags() & Qgis::RenderContextFlag::RenderBlocking ) );
    double opacity = mOpacity;
    if ( mDataDefinedProperties.isActive( QgsSymbolLayer::Property::Opacity ) )
    {
      context.setOriginalValueVariable( mOpacity );
      opacity = mDataDefinedProperties.valueAsDouble( QgsSymbolLayer::Property::Opacity, context.renderContext().expressionContext(), opacity * 100 ) / 100.0;
    }
    opacity *= context.opacity();

    bool cached = false;
    sourceImage = QgsApplication::imageCache()->pathAsImage( path,
                  QSize( static_cast< int >( std::round( originalSize.width() / originalSize.height() * std::max( 1.0, scaledHeight ) ) ),
                         static_cast< int >( std::ceil( scaledHeight ) ) ),
                  true, opacity, cached, ( context.renderContext().flags() & Qgis::RenderContextFlag::RenderBlocking ) );
  }

  const bool useSelectedColor = shouldRenderUsingSelectionColor( context );
  if ( useSelectedColor )
  {
    QgsImageOperation::overlayColor( sourceImage, context.renderContext().selectionColor() );
  }

  const QBrush brush( sourceImage );

  renderPolylineUsingBrush( points, context, brush, sourceImage.height(), sourceImage.width() );
}

void QgsRasterLineSymbolLayer::setOutputUnit( Qgis::RenderUnit unit )
{
  QgsLineSymbolLayer::setOutputUnit( unit );
  mWidthUnit = unit;
  mOffsetUnit = unit;
}

Qgis::RenderUnit QgsRasterLineSymbolLayer::outputUnit() const
{
  Qgis::RenderUnit unit = QgsLineSymbolLayer::outputUnit();
  if ( mWidthUnit != unit || mOffsetUnit != unit )
  {
    return Qgis::RenderUnit::Unknown;
  }
  return unit;
}

bool QgsRasterLineSymbolLayer::usesMapUnits() const
{
  return mWidthUnit == Qgis::RenderUnit::MapUnits || mWidthUnit == Qgis::RenderUnit::MetersInMapUnits
         || mOffsetUnit == Qgis::RenderUnit::MapUnits || mOffsetUnit == Qgis::RenderUnit::MetersInMapUnits;
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
  auto res = std::make_unique<QgsLineburstSymbolLayer>();

  if ( properties.contains( u"line_width"_s ) )
  {
    res->setWidth( properties[u"line_width"_s].toDouble() );
  }
  if ( properties.contains( u"line_width_unit"_s ) )
  {
    res->setWidthUnit( QgsUnitTypes::decodeRenderUnit( properties[u"line_width_unit"_s].toString() ) );
  }
  if ( properties.contains( u"width_map_unit_scale"_s ) )
  {
    res->setWidthMapUnitScale( QgsSymbolLayerUtils::decodeMapUnitScale( properties[u"width_map_unit_scale"_s].toString() ) );
  }

  if ( properties.contains( u"offset"_s ) )
  {
    res->setOffset( properties[u"offset"_s].toDouble() );
  }
  if ( properties.contains( u"offset_unit"_s ) )
  {
    res->setOffsetUnit( QgsUnitTypes::decodeRenderUnit( properties[u"offset_unit"_s].toString() ) );
  }
  if ( properties.contains( u"offset_map_unit_scale"_s ) )
  {
    res->setOffsetMapUnitScale( QgsSymbolLayerUtils::decodeMapUnitScale( properties[u"offset_map_unit_scale"_s].toString() ) );
  }

  if ( properties.contains( u"joinstyle"_s ) )
    res->setPenJoinStyle( QgsSymbolLayerUtils::decodePenJoinStyle( properties[u"joinstyle"_s].toString() ) );
  if ( properties.contains( u"capstyle"_s ) )
    res->setPenCapStyle( QgsSymbolLayerUtils::decodePenCapStyle( properties[u"capstyle"_s].toString() ) );

  if ( properties.contains( u"color_type"_s ) )
    res->setGradientColorType( static_cast< Qgis::GradientColorSource >( properties[u"color_type"_s].toInt() ) );

  if ( properties.contains( u"color"_s ) )
  {
    res->setColor( QgsColorUtils::colorFromString( properties[u"color"_s].toString() ) );
  }
  if ( properties.contains( u"gradient_color2"_s ) )
  {
    res->setColor2( QgsColorUtils::colorFromString( properties[u"gradient_color2"_s].toString() ) );
  }

  //attempt to create color ramp from props
  if ( properties.contains( u"rampType"_s ) && properties[u"rampType"_s] == QgsCptCityColorRamp::typeString() )
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

  map[u"line_width"_s] = QString::number( mWidth );
  map[u"line_width_unit"_s] = QgsUnitTypes::encodeUnit( mWidthUnit );
  map[u"width_map_unit_scale"_s] = QgsSymbolLayerUtils::encodeMapUnitScale( mWidthMapUnitScale );

  map[u"joinstyle"_s] = QgsSymbolLayerUtils::encodePenJoinStyle( mPenJoinStyle );
  map[u"capstyle"_s] = QgsSymbolLayerUtils::encodePenCapStyle( mPenCapStyle );

  map[u"offset"_s] = QString::number( mOffset );
  map[u"offset_unit"_s] = QgsUnitTypes::encodeUnit( mOffsetUnit );
  map[u"offset_map_unit_scale"_s] = QgsSymbolLayerUtils::encodeMapUnitScale( mOffsetMapUnitScale );

  map[u"color"_s] = QgsColorUtils::colorToString( mColor );
  map[u"gradient_color2"_s] = QgsColorUtils::colorToString( mColor2 );
  map[u"color_type"_s] = QString::number( static_cast< int >( mGradientColorType ) );
  if ( mGradientRamp )
  {
    map.insert( mGradientRamp->properties() );
  }

  return map;
}

QgsLineburstSymbolLayer *QgsLineburstSymbolLayer::clone() const
{
  auto res = std::make_unique< QgsLineburstSymbolLayer >();
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
  return u"Lineburst"_s;
}

Qgis::SymbolLayerFlags QgsLineburstSymbolLayer::flags() const
{
  return QgsAbstractBrushedLineSymbolLayer::flags() | Qgis::SymbolLayerFlag::CanCalculateMaskGeometryPerFeature;
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
  if ( mDataDefinedProperties.isActive( QgsSymbolLayer::Property::StrokeWidth ) )
  {
    context.setOriginalValueVariable( strokeWidth );
    strokeWidth = mDataDefinedProperties.valueAsDouble( QgsSymbolLayer::Property::StrokeWidth, context.renderContext().expressionContext(), strokeWidth );
  }
  const double scaledWidth = context.renderContext().convertToPainterUnits( strokeWidth, mWidthUnit, mWidthMapUnitScale );

  //update alpha of gradient colors
  QColor color1 = mColor;
  if ( mDataDefinedProperties.isActive( QgsSymbolLayer::Property::StrokeColor ) )
  {
    context.setOriginalValueVariable( QgsSymbolLayerUtils::encodeColor( mColor ) );
    color1 = mDataDefinedProperties.valueAsColor( QgsSymbolLayer::Property::StrokeColor, context.renderContext().expressionContext(), mColor );
  }
  const bool useSelectedColor = shouldRenderUsingSelectionColor( context );
  if ( useSelectedColor )
  {
    color1 = context.renderContext().selectionColor();
  }
  color1.setAlphaF( context.opacity() * color1.alphaF() );

  //second gradient color
  QColor color2 = mColor2;
  if ( mDataDefinedProperties.isActive( QgsSymbolLayer::Property::SecondaryColor ) )
  {
    context.setOriginalValueVariable( QgsSymbolLayerUtils::encodeColor( mColor2 ) );
    color2 = mDataDefinedProperties.valueAsColor( QgsSymbolLayer::Property::SecondaryColor, context.renderContext().expressionContext(), mColor2 );
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

void QgsLineburstSymbolLayer::setOutputUnit( Qgis::RenderUnit unit )
{
  QgsLineSymbolLayer::setOutputUnit( unit );
  mWidthUnit = unit;
  mOffsetUnit = unit;
}

Qgis::RenderUnit QgsLineburstSymbolLayer::outputUnit() const
{
  Qgis::RenderUnit unit = QgsLineSymbolLayer::outputUnit();
  if ( mWidthUnit != unit || mOffsetUnit != unit )
  {
    return Qgis::RenderUnit::Unknown;
  }
  return unit;
}

bool QgsLineburstSymbolLayer::usesMapUnits() const
{
  return mWidthUnit == Qgis::RenderUnit::MapUnits || mWidthUnit == Qgis::RenderUnit::MetersInMapUnits
         || mOffsetUnit == Qgis::RenderUnit::MapUnits || mOffsetUnit == Qgis::RenderUnit::MetersInMapUnits;
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

QgsColorRamp *QgsLineburstSymbolLayer::colorRamp()
{
  return mGradientRamp.get();
}

void QgsLineburstSymbolLayer::setColorRamp( QgsColorRamp *ramp )
{
  mGradientRamp.reset( ramp );
}

//
// QgsFilledLineSymbolLayer
//

QgsFilledLineSymbolLayer::QgsFilledLineSymbolLayer( double width, QgsFillSymbol *fillSymbol )
  : QgsLineSymbolLayer()
{
  mWidth = width;
  mFill = fillSymbol ? std::unique_ptr< QgsFillSymbol >( fillSymbol ) : QgsFillSymbol::createSimple( QVariantMap() );
}

QgsFilledLineSymbolLayer::~QgsFilledLineSymbolLayer() = default;

QgsSymbolLayer *QgsFilledLineSymbolLayer::create( const QVariantMap &props )
{
  double width = DEFAULT_SIMPLELINE_WIDTH;

  // throughout the history of QGIS and different layer types, we've used
  // a huge range of different strings for the same property. The logic here
  // is designed to be forgiving to this and accept a range of string keys:
  if ( props.contains( u"line_width"_s ) )
  {
    width = props[u"line_width"_s].toDouble();
  }
  else if ( props.contains( u"outline_width"_s ) )
  {
    width = props[u"outline_width"_s].toDouble();
  }
  else if ( props.contains( u"width"_s ) )
  {
    width = props[u"width"_s].toDouble();
  }

  auto l = std::make_unique< QgsFilledLineSymbolLayer >( width, QgsFillSymbol::createSimple( props ).release() );

  if ( props.contains( u"line_width_unit"_s ) )
  {
    l->setWidthUnit( QgsUnitTypes::decodeRenderUnit( props[u"line_width_unit"_s].toString() ) );
  }
  else if ( props.contains( u"outline_width_unit"_s ) )
  {
    l->setWidthUnit( QgsUnitTypes::decodeRenderUnit( props[u"outline_width_unit"_s].toString() ) );
  }
  else if ( props.contains( u"width_unit"_s ) )
  {
    l->setWidthUnit( QgsUnitTypes::decodeRenderUnit( props[u"width_unit"_s].toString() ) );
  }

  if ( props.contains( u"width_map_unit_scale"_s ) )
    l->setWidthMapUnitScale( QgsSymbolLayerUtils::decodeMapUnitScale( props[u"width_map_unit_scale"_s].toString() ) );
  if ( props.contains( u"offset"_s ) )
    l->setOffset( props[u"offset"_s].toDouble() );
  if ( props.contains( u"offset_unit"_s ) )
    l->setOffsetUnit( QgsUnitTypes::decodeRenderUnit( props[u"offset_unit"_s].toString() ) );
  if ( props.contains( u"offset_map_unit_scale"_s ) )
    l->setOffsetMapUnitScale( QgsSymbolLayerUtils::decodeMapUnitScale( props[u"offset_map_unit_scale"_s].toString() ) );
  if ( props.contains( u"joinstyle"_s ) )
    l->setPenJoinStyle( QgsSymbolLayerUtils::decodePenJoinStyle( props[u"joinstyle"_s].toString() ) );
  if ( props.contains( u"capstyle"_s ) )
    l->setPenCapStyle( QgsSymbolLayerUtils::decodePenCapStyle( props[u"capstyle"_s].toString() ) );

  l->restoreOldDataDefinedProperties( props );

  return l.release();
}

QString QgsFilledLineSymbolLayer::layerType() const
{
  return u"FilledLine"_s;
}

void QgsFilledLineSymbolLayer::startRender( QgsSymbolRenderContext &context )
{
  if ( mFill )
  {
    mFill->setRenderHints( mFill->renderHints() | Qgis::SymbolRenderHint::IsSymbolLayerSubSymbol );
    mFill->startRender( context.renderContext(), context.fields() );
  }
}

void QgsFilledLineSymbolLayer::stopRender( QgsSymbolRenderContext &context )
{
  if ( mFill )
  {
    mFill->stopRender( context.renderContext() );
  }
}

void QgsFilledLineSymbolLayer::startFeatureRender( const QgsFeature &, QgsRenderContext &context )
{
  installMasks( context, true );

  // The base class version passes this on to the subsymbol, but we deliberately don't do that here.
}

void QgsFilledLineSymbolLayer::stopFeatureRender( const QgsFeature &, QgsRenderContext &context )
{
  removeMasks( context, true );

  // The base class version passes this on to the subsymbol, but we deliberately don't do that here.
}

void QgsFilledLineSymbolLayer::renderPolyline( const QPolygonF &points, QgsSymbolRenderContext &context )
{
  QPainter *p = context.renderContext().painter();
  if ( !p || !mFill )
    return;

  double width = mWidth;
  if ( mDataDefinedProperties.isActive( QgsSymbolLayer::Property::StrokeWidth ) )
  {
    context.setOriginalValueVariable( mWidth );
    width = mDataDefinedProperties.valueAsDouble( QgsSymbolLayer::Property::StrokeWidth, context.renderContext().expressionContext(), mWidth );
  }

  const double scaledWidth = context.renderContext().convertToPainterUnits( width, mWidthUnit, mWidthMapUnitScale );

  Qt::PenJoinStyle join = mPenJoinStyle;
  if ( mDataDefinedProperties.isActive( QgsSymbolLayer::Property::JoinStyle ) )
  {
    context.setOriginalValueVariable( QgsSymbolLayerUtils::encodePenJoinStyle( join ) );
    QVariant exprVal = mDataDefinedProperties.value( QgsSymbolLayer::Property::JoinStyle, context.renderContext().expressionContext() );
    if ( !QgsVariantUtils::isNull( exprVal ) )
      join = QgsSymbolLayerUtils::decodePenJoinStyle( exprVal.toString() );
  }

  Qt::PenCapStyle cap = mPenCapStyle;
  if ( mDataDefinedProperties.isActive( QgsSymbolLayer::Property::CapStyle ) )
  {
    context.setOriginalValueVariable( QgsSymbolLayerUtils::encodePenCapStyle( cap ) );
    QVariant exprVal = mDataDefinedProperties.value( QgsSymbolLayer::Property::CapStyle, context.renderContext().expressionContext() );
    if ( !QgsVariantUtils::isNull( exprVal ) )
      cap = QgsSymbolLayerUtils::decodePenCapStyle( exprVal.toString() );
  }

  double offset = mOffset;
  if ( mDataDefinedProperties.isActive( QgsSymbolLayer::Property::Offset ) )
  {
    context.setOriginalValueVariable( mOffset );
    offset = mDataDefinedProperties.valueAsDouble( QgsSymbolLayer::Property::Offset, context.renderContext().expressionContext(), offset );
  }

  const double prevOpacity = mFill->opacity();
  mFill->setOpacity( mFill->opacity() * context.opacity() );

  const bool prevIsSubsymbol = context.renderContext().flags() & Qgis::RenderContextFlag::RenderingSubSymbol;
  context.renderContext().setFlag( Qgis::RenderContextFlag::RenderingSubSymbol );

  const bool useSelectedColor = shouldRenderUsingSelectionColor( context );

  if ( points.count() >= 2 )
  {
    std::unique_ptr< QgsAbstractGeometry > ls = QgsLineString::fromQPolygonF( points );
    geos::unique_ptr lineGeom;

    if ( !qgsDoubleNear( offset, 0 ) )
    {
      double scaledOffset = context.renderContext().convertToPainterUnits( offset, mOffsetUnit, mOffsetMapUnitScale );
      if ( mOffsetUnit == Qgis::RenderUnit::MetersInMapUnits && ( context.renderContext().flags() & Qgis::RenderContextFlag::RenderSymbolPreview || context.renderContext().flags() & Qgis::RenderContextFlag::RenderLayerTree ) )
      {
        // rendering for symbol previews -- a size in meters in map units can't be calculated, so treat the size as millimeters
        // and clamp it to a reasonable range. It's the best we can do in this situation!
        scaledOffset = std::min( std::max( context.renderContext().convertToPainterUnits( offset, Qgis::RenderUnit::Millimeters ), 3.0 ), 100.0 );
      }

      const Qgis::GeometryType geometryType = context.originalGeometryType() != Qgis::GeometryType::Unknown ? context.originalGeometryType() : Qgis::GeometryType::Line;
      if ( geometryType == Qgis::GeometryType::Polygon )
      {
        auto inputPoly = std::make_unique< QgsPolygon >( static_cast< QgsLineString * >( ls.release() ) );
        geos::unique_ptr g( QgsGeos::asGeos( inputPoly.get() ) );
        lineGeom = QgsGeos::buffer( g.get(), -scaledOffset, 0, Qgis::EndCapStyle::Flat, Qgis::JoinStyle::Miter, 2 );
        // the result is a polygon => extract line work
        QgsGeometry polygon( QgsGeos::fromGeos( lineGeom.get() ) );
        QVector<QgsGeometry> parts = polygon.coerceToType( Qgis::WkbType::MultiLineString );
        if ( !parts.empty() )
        {
          lineGeom = QgsGeos::asGeos( parts.at( 0 ).constGet() );
        }
        else
        {
          lineGeom.reset();
        }
      }
      else
      {
        geos::unique_ptr g( QgsGeos::asGeos( ls.get() ) );
        lineGeom = QgsGeos::offsetCurve( g.get(), scaledOffset, 0, Qgis::JoinStyle::Miter, 8.0 );
      }
    }
    else
    {
      lineGeom = QgsGeos::asGeos( ls.get() );
    }

    if ( lineGeom )
    {
      geos::unique_ptr buffered = QgsGeos::buffer( lineGeom.get(), scaledWidth / 2, 8,
                                  QgsSymbolLayerUtils::penCapStyleToEndCapStyle( cap ),
                                  QgsSymbolLayerUtils::penJoinStyleToJoinStyle( join ), 8 );
      if ( buffered )
      {
        // convert to rings
        std::unique_ptr< QgsAbstractGeometry > bufferedGeom = QgsGeos::fromGeos( buffered.get() );
        const QList< QList< QPolygonF > > parts = QgsSymbolLayerUtils::toQPolygonF( bufferedGeom.get(), Qgis::SymbolType::Fill );
        for ( const QList< QPolygonF > &polygon : parts )
        {
          QVector< QPolygonF > rings;
          for ( int i = 1; i < polygon.size(); ++i )
            rings << polygon.at( i );
          mFill->renderPolygon( polygon.value( 0 ), &rings, context.feature(), context.renderContext(), -1, useSelectedColor );
        }
      }
    }
  }

  context.renderContext().setFlag( Qgis::RenderContextFlag::RenderingSubSymbol, prevIsSubsymbol );

  mFill->setOpacity( prevOpacity );
}

QVariantMap QgsFilledLineSymbolLayer::properties() const
{
  QVariantMap map;

  map[u"line_width"_s] = QString::number( mWidth );
  map[u"line_width_unit"_s] = QgsUnitTypes::encodeUnit( mWidthUnit );
  map[u"width_map_unit_scale"_s] = QgsSymbolLayerUtils::encodeMapUnitScale( mWidthMapUnitScale );
  map[u"joinstyle"_s] = QgsSymbolLayerUtils::encodePenJoinStyle( mPenJoinStyle );
  map[u"capstyle"_s] = QgsSymbolLayerUtils::encodePenCapStyle( mPenCapStyle );
  map[u"offset"_s] = QString::number( mOffset );
  map[u"offset_unit"_s] = QgsUnitTypes::encodeUnit( mOffsetUnit );
  map[u"offset_map_unit_scale"_s] = QgsSymbolLayerUtils::encodeMapUnitScale( mOffsetMapUnitScale );
  if ( mFill )
  {
    map[u"color"_s] = QgsColorUtils::colorToString( mFill->color() );
  }
  return map;
}

QgsFilledLineSymbolLayer *QgsFilledLineSymbolLayer::clone() const
{
  std::unique_ptr< QgsFilledLineSymbolLayer > res( qgis::down_cast< QgsFilledLineSymbolLayer * >( QgsFilledLineSymbolLayer::create( properties() ) ) );
  copyPaintEffect( res.get() );
  copyDataDefinedProperties( res.get() );
  res->setSubSymbol( mFill->clone() );
  return res.release();
}

QgsSymbol *QgsFilledLineSymbolLayer::subSymbol()
{
  return mFill.get();
}

bool QgsFilledLineSymbolLayer::setSubSymbol( QgsSymbol *symbol )
{
  if ( symbol && symbol->type() == Qgis::SymbolType::Fill )
  {
    mFill.reset( static_cast<QgsFillSymbol *>( symbol ) );
    return true;
  }
  else
  {
    delete symbol;
    return false;
  }
}

double QgsFilledLineSymbolLayer::estimateMaxBleed( const QgsRenderContext &context ) const
{
  if ( mFill )
  {
    return QgsSymbolLayerUtils::estimateMaxSymbolBleed( mFill.get(), context );
  }
  return 0;
}

QSet<QString> QgsFilledLineSymbolLayer::usedAttributes( const QgsRenderContext &context ) const
{
  QSet<QString> attr = QgsLineSymbolLayer::usedAttributes( context );
  if ( mFill )
    attr.unite( mFill->usedAttributes( context ) );
  return attr;
}

bool QgsFilledLineSymbolLayer::hasDataDefinedProperties() const
{
  if ( QgsSymbolLayer::hasDataDefinedProperties() )
    return true;
  if ( mFill && mFill->hasDataDefinedProperties() )
    return true;
  return false;
}

void QgsFilledLineSymbolLayer::setColor( const QColor &c )
{
  mColor = c;
  if ( mFill )
    mFill->setColor( c );
}

QColor QgsFilledLineSymbolLayer::color() const
{
  return mFill ? mFill->color() : mColor;
}

bool QgsFilledLineSymbolLayer::usesMapUnits() const
{
  return mWidthUnit == Qgis::RenderUnit::MapUnits || mWidthUnit == Qgis::RenderUnit::MetersInMapUnits
         || mOffsetUnit == Qgis::RenderUnit::MapUnits || mOffsetUnit == Qgis::RenderUnit::MetersInMapUnits
         || ( mFill && mFill->usesMapUnits() );
}

void QgsFilledLineSymbolLayer::setMapUnitScale( const QgsMapUnitScale &scale )
{
  QgsLineSymbolLayer::setMapUnitScale( scale );
  if ( mFill )
    mFill->setMapUnitScale( scale );
}

QgsMapUnitScale QgsFilledLineSymbolLayer::mapUnitScale() const
{
  if ( mFill )
  {
    return mFill->mapUnitScale();
  }
  return QgsMapUnitScale();
}

void QgsFilledLineSymbolLayer::setOutputUnit( Qgis::RenderUnit unit )
{
  QgsLineSymbolLayer::setOutputUnit( unit );
  if ( mFill )
    mFill->setOutputUnit( unit );
}

Qgis::RenderUnit QgsFilledLineSymbolLayer::outputUnit() const
{
  if ( mFill )
  {
    return mFill->outputUnit();
  }
  return Qgis::RenderUnit::Unknown;
}
