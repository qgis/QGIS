/***************************************************************************
    qgslinearreferencingsymbollayer.cpp
    ---------------------
    begin                : August 2024
    copyright            : (C) 2024 by Nyall Dawson
    email                : nyall dot dawson at gmail dot com
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "qgslinearreferencingsymbollayer.h"
#include "qgsrendercontext.h"
#include "qgstextrenderer.h"
#include "qgslinestring.h"
#include "qgspolygon.h"
#include "qgsmarkersymbol.h"
#include "qgsnumericformatregistry.h"
#include "qgsapplication.h"
#include "qgsbasicnumericformat.h"
#include "qgsgeometryutils.h"
#include "qgsunittypes.h"
#include "qgssymbollayerutils.h"
#include "qgstextlabelfeature.h"
#include "qgsgeos.h"
#include "qgspallabeling.h"
#include "labelposition.h"
#include "feature.h"

///@cond PRIVATE
class QgsTextLabelFeatureWithFormat : public QgsTextLabelFeature
{
  public:
    QgsTextLabelFeatureWithFormat( QgsFeatureId id, geos::unique_ptr geometry, QSizeF size, const QgsTextFormat &format )
      : QgsTextLabelFeature( id, std::move( geometry ), size )
      , mFormat( format )
    {}

    QgsTextFormat mFormat;

};

class QgsLinearReferencingSymbolLayerLabelProvider final : public QgsAbstractLabelProvider
{
  public:
    QgsLinearReferencingSymbolLayerLabelProvider()
      : QgsAbstractLabelProvider( nullptr )
    {
      mPlacement = Qgis::LabelPlacement::OverPoint;
      mFlags |= DrawLabels;

      // always consider these labels highest priority
      // TODO possibly expose as a setting for the symbol layer?
      mPriority = 0;
    }

    ~QgsLinearReferencingSymbolLayerLabelProvider()
    {
      qDeleteAll( mLabels );
    }

    void addLabel( const QPointF &painterPoint, double angleRadians, const QString &text, QgsRenderContext &context, const QgsTextFormat &format )
    {
      // labels must be registered in destination map units
      QgsPoint mapPoint( painterPoint );
      mapPoint.transform( context.mapToPixel().transform().inverted() );

      const QgsTextDocument doc = QgsTextDocument::fromTextAndFormat( { text }, format );
      QgsTextDocumentMetrics documentMetrics = QgsTextDocumentMetrics::calculateMetrics( doc, format, context );
      const QSizeF size = documentMetrics.documentSize( Qgis::TextLayoutMode::Point, Qgis::TextOrientation::Horizontal );

      double uPP = context.mapToPixel().mapUnitsPerPixel();
      std::unique_ptr< QgsTextLabelFeatureWithFormat > feature = std::make_unique< QgsTextLabelFeatureWithFormat >( mLabels.size(),
          QgsGeos::asGeos( &mapPoint ), QSizeF( size.width() * uPP, size.height() * uPP ), format );

      feature->setDocument( doc, documentMetrics );
      feature->setFixedAngle( angleRadians );
      feature->setHasFixedAngle( true );
      // above right
      // TODO: we could potentially expose this, and use a non-fixed mode to allow fallback positions
      feature->setQuadOffset( QPointF( 1, 1 ) );

      mLabels.append( feature.release() );
    }

    QList<QgsLabelFeature *> labelFeatures( QgsRenderContext & ) final
    {
      return mLabels;
    }

    void drawLabel( QgsRenderContext &context, pal::LabelPosition *label ) const final
    {
      // as per vector label rendering...
      QgsMapToPixel xform = context.mapToPixel();
      xform.setMapRotation( 0, 0, 0 );
      const QPointF outPt = context.mapToPixel().transform( label->getX(), label->getY() ).toQPointF();

      QgsTextLabelFeatureWithFormat *lf = qgis::down_cast<QgsTextLabelFeatureWithFormat *>( label->getFeaturePart()->feature() );
      QgsTextRenderer::drawDocument( outPt,
                                     lf->mFormat, lf->document(), lf->documentMetrics(), context, Qgis::TextHorizontalAlignment::Left,
                                     label->getAlpha() );
    }

  private:
    QList<QgsLabelFeature *> mLabels;

};
///@endcond

QgsLinearReferencingSymbolLayer::QgsLinearReferencingSymbolLayer()
  : QgsLineSymbolLayer()
{
  mNumericFormat = std::make_unique< QgsBasicNumericFormat >();
}

QgsLinearReferencingSymbolLayer::~QgsLinearReferencingSymbolLayer() = default;

QgsSymbolLayer *QgsLinearReferencingSymbolLayer::create( const QVariantMap &properties )
{
  std::unique_ptr< QgsLinearReferencingSymbolLayer > res = std::make_unique< QgsLinearReferencingSymbolLayer >();
  res->setPlacement( qgsEnumKeyToValue( properties.value( QStringLiteral( "placement" ) ).toString(), Qgis::LinearReferencingPlacement::IntervalCartesian2D ) );
  res->setLabelSource( qgsEnumKeyToValue( properties.value( QStringLiteral( "source" ) ).toString(), Qgis::LinearReferencingLabelSource::CartesianDistance2D ) );
  bool ok = false;
  const double interval = properties.value( QStringLiteral( "interval" ) ).toDouble( &ok );
  if ( ok )
    res->setInterval( interval );
  const double skipMultiples = properties.value( QStringLiteral( "skip_multiples" ) ).toDouble( &ok );
  if ( ok )
    res->setSkipMultiplesOf( skipMultiples );
  res->setRotateLabels( properties.value( QStringLiteral( "rotate" ), true ).toBool() );
  res->setShowMarker( properties.value( QStringLiteral( "show_marker" ), false ).toBool() );

  // it's impossible to get the project's path resolver here :(
  // TODO QGIS 4.0 -- force use of QgsReadWriteContext in create methods
  QgsReadWriteContext rwContext;
  //rwContext.setPathResolver( QgsProject::instance()->pathResolver() ); // skip-keyword-check

  const QString textFormatXml = properties.value( QStringLiteral( "text_format" ) ).toString();
  if ( !textFormatXml.isEmpty() )
  {
    QDomDocument doc;
    QDomElement elem;
    doc.setContent( textFormatXml );
    elem = doc.documentElement();

    QgsTextFormat textFormat;
    textFormat.readXml( elem, rwContext );
    res->setTextFormat( textFormat );
  }

  const QString numericFormatXml = properties.value( QStringLiteral( "numeric_format" ) ).toString();
  if ( !numericFormatXml.isEmpty() )
  {
    QDomDocument doc;
    doc.setContent( numericFormatXml );
    res->setNumericFormat( QgsApplication::numericFormatRegistry()->createFromXml( doc.documentElement(), rwContext ) );
  }

  if ( properties.contains( QStringLiteral( "label_offset" ) ) )
  {
    res->setLabelOffset( QgsSymbolLayerUtils::decodePoint( properties[QStringLiteral( "label_offset" )].toString() ) );
  }
  if ( properties.contains( QStringLiteral( "label_offset_unit" ) ) )
  {
    res->setLabelOffsetUnit( QgsUnitTypes::decodeRenderUnit( properties[QStringLiteral( "label_offset_unit" )].toString() ) );
  }
  if ( properties.contains( ( QStringLiteral( "label_offset_map_unit_scale" ) ) ) )
  {
    res->setLabelOffsetMapUnitScale( QgsSymbolLayerUtils::decodeMapUnitScale( properties[QStringLiteral( "label_offset_map_unit_scale" )].toString() ) );
  }
  if ( properties.contains( QStringLiteral( "average_angle_length" ) ) )
  {
    res->setAverageAngleLength( properties[QStringLiteral( "average_angle_length" )].toDouble() );
  }
  if ( properties.contains( QStringLiteral( "average_angle_unit" ) ) )
  {
    res->setAverageAngleUnit( QgsUnitTypes::decodeRenderUnit( properties[QStringLiteral( "average_angle_unit" )].toString() ) );
  }
  if ( properties.contains( ( QStringLiteral( "average_angle_map_unit_scale" ) ) ) )
  {
    res->setAverageAngleMapUnitScale( QgsSymbolLayerUtils::decodeMapUnitScale( properties[QStringLiteral( "average_angle_map_unit_scale" )].toString() ) );
  }

  return res.release();
}

QgsLinearReferencingSymbolLayer *QgsLinearReferencingSymbolLayer::clone() const
{
  std::unique_ptr< QgsLinearReferencingSymbolLayer > res = std::make_unique< QgsLinearReferencingSymbolLayer >();
  res->setPlacement( mPlacement );
  res->setLabelSource( mLabelSource );
  res->setInterval( mInterval );
  res->setSkipMultiplesOf( mSkipMultiplesOf );
  res->setRotateLabels( mRotateLabels );
  res->setLabelOffset( mLabelOffset );
  res->setLabelOffsetUnit( mLabelOffsetUnit );
  res->setLabelOffsetMapUnitScale( mLabelOffsetMapUnitScale );
  res->setShowMarker( mShowMarker );
  res->setAverageAngleLength( mAverageAngleLength );
  res->setAverageAngleUnit( mAverageAngleLengthUnit );
  res->setAverageAngleMapUnitScale( mAverageAngleLengthMapUnitScale );

  res->mTextFormat = mTextFormat;
  res->mMarkerSymbol.reset( mMarkerSymbol ? mMarkerSymbol->clone() : nullptr );
  if ( mNumericFormat )
    res->mNumericFormat.reset( mNumericFormat->clone() );

  copyDataDefinedProperties( res.get() );
  copyPaintEffect( res.get() );

  return res.release();
}

QVariantMap QgsLinearReferencingSymbolLayer::properties() const
{
  QDomDocument textFormatDoc;
  // it's impossible to get the project's path resolver here :(
  // TODO QGIS 4.0 -- force use of QgsReadWriteContext in properties methods
  QgsReadWriteContext rwContext;
  // rwContext.setPathResolver( QgsProject::instance()->pathResolver() ); // skip-keyword-check
  const QDomElement textElem = mTextFormat.writeXml( textFormatDoc, rwContext );
  textFormatDoc.appendChild( textElem );

  QDomDocument numericFormatDoc;
  QDomElement numericFormatElem = numericFormatDoc.createElement( QStringLiteral( "numericFormat" ) );
  mNumericFormat->writeXml( numericFormatElem, numericFormatDoc, rwContext );
  numericFormatDoc.appendChild( numericFormatElem );

  QVariantMap res
  {
    {
      QStringLiteral( "placement" ), qgsEnumValueToKey( mPlacement )
    },
    {
      QStringLiteral( "source" ), qgsEnumValueToKey( mLabelSource )
    },
    {
      QStringLiteral( "interval" ), mInterval
    },
    {
      QStringLiteral( "rotate" ), mRotateLabels
    },
    {
      QStringLiteral( "show_marker" ), mShowMarker
    },
    {
      QStringLiteral( "text_format" ), textFormatDoc.toString()
    },
    {
      QStringLiteral( "numeric_format" ), numericFormatDoc.toString()
    },
    {
      QStringLiteral( "label_offset" ), QgsSymbolLayerUtils::encodePoint( mLabelOffset )
    },
    {
      QStringLiteral( "label_offset_unit" ), QgsUnitTypes::encodeUnit( mLabelOffsetUnit )
    },
    {
      QStringLiteral( "label_offset_map_unit_scale" ), QgsSymbolLayerUtils::encodeMapUnitScale( mLabelOffsetMapUnitScale )
    },
    {
      QStringLiteral( "average_angle_length" ), mAverageAngleLength
    },
    {
      QStringLiteral( "average_angle_unit" ), QgsUnitTypes::encodeUnit( mAverageAngleLengthUnit )
    },
    {
      QStringLiteral( "average_angle_map_unit_scale" ), QgsSymbolLayerUtils::encodeMapUnitScale( mAverageAngleLengthMapUnitScale )
    },
  };

  if ( mSkipMultiplesOf >= 0 )
  {
    res.insert( QStringLiteral( "skip_multiples" ), mSkipMultiplesOf );
  }

  return res;
}

QString QgsLinearReferencingSymbolLayer::layerType() const
{
  return QStringLiteral( "LinearReferencing" );
}

Qgis::SymbolLayerFlags QgsLinearReferencingSymbolLayer::flags() const
{
  return Qgis::SymbolLayerFlag::DisableFeatureClipping
         | Qgis::SymbolLayerFlag::AffectsLabeling;
}

QgsSymbol *QgsLinearReferencingSymbolLayer::subSymbol()
{
  return mShowMarker ? mMarkerSymbol.get() : nullptr;
}

bool QgsLinearReferencingSymbolLayer::setSubSymbol( QgsSymbol *symbol )
{
  if ( symbol && symbol->type() == Qgis::SymbolType::Marker )
  {
    mMarkerSymbol.reset( qgis::down_cast<QgsMarkerSymbol *>( symbol ) );
    return true;
  }
  delete symbol;
  return false;
}

void QgsLinearReferencingSymbolLayer::startRender( QgsSymbolRenderContext &context )
{
  if ( mMarkerSymbol )
  {
    Qgis::SymbolRenderHints hints = mMarkerSymbol->renderHints() | Qgis::SymbolRenderHint::IsSymbolLayerSubSymbol;
    if ( mRotateLabels )
      hints |= Qgis::SymbolRenderHint::DynamicRotation;
    mMarkerSymbol->setRenderHints( hints );

    mMarkerSymbol->startRender( context.renderContext(), context.fields() );
  }

  if ( QgsLabelingEngine *labelingEngine = context.renderContext().labelingEngine() )
  {
    // rendering with a labeling engine. In this scenario we will register rendered text as labels, so that they participate in the labeling problem
    // for the map
    QgsLinearReferencingSymbolLayerLabelProvider *provider = new QgsLinearReferencingSymbolLayerLabelProvider();
    mLabelProviderId = labelingEngine->addProvider( provider );
  }
}

void QgsLinearReferencingSymbolLayer::stopRender( QgsSymbolRenderContext &context )
{
  if ( mMarkerSymbol )
  {
    mMarkerSymbol->stopRender( context.renderContext() );
  }
}

void QgsLinearReferencingSymbolLayer::renderGeometryPart( QgsSymbolRenderContext &context, const QgsAbstractGeometry *geometry, double labelOffsetPainterUnitsX, double labelOffsetPainterUnitsY, double skipMultiples, double averageAngleDistancePainterUnits, bool showMarker )
{
  if ( const QgsLineString *line = qgsgeometry_cast< const QgsLineString * >( geometry ) )
  {
    renderLineString( context, line, labelOffsetPainterUnitsX, labelOffsetPainterUnitsY, skipMultiples, averageAngleDistancePainterUnits, showMarker );
  }
  else if ( const QgsPolygon *polygon = qgsgeometry_cast< const QgsPolygon * >( geometry ) )
  {
    renderLineString( context, qgsgeometry_cast< const QgsLineString *>( polygon->exteriorRing() ), labelOffsetPainterUnitsX, labelOffsetPainterUnitsY, skipMultiples, averageAngleDistancePainterUnits, showMarker );
    for ( int i = 0; i < polygon->numInteriorRings(); ++i )
    {
      renderLineString( context, qgsgeometry_cast< const QgsLineString *>( polygon->interiorRing( i ) ), labelOffsetPainterUnitsX, labelOffsetPainterUnitsY, skipMultiples, averageAngleDistancePainterUnits, showMarker );
    }
  }
}

void QgsLinearReferencingSymbolLayer::renderLineString( QgsSymbolRenderContext &context, const QgsLineString *line, double labelOffsetPainterUnitsX, double labelOffsetPainterUnitsY, double skipMultiples, double averageAngleDistancePainterUnits, bool showMarker )
{
  if ( !line )
    return;

  switch ( mPlacement )
  {
    case Qgis::LinearReferencingPlacement::IntervalCartesian2D:
    case Qgis::LinearReferencingPlacement::IntervalZ:
    case Qgis::LinearReferencingPlacement::IntervalM:
      renderPolylineInterval( line, context, skipMultiples, QPointF( labelOffsetPainterUnitsX, labelOffsetPainterUnitsY ), averageAngleDistancePainterUnits, showMarker );
      break;

    case Qgis::LinearReferencingPlacement::Vertex:
      renderPolylineVertex( line, context, skipMultiples, QPointF( labelOffsetPainterUnitsX, labelOffsetPainterUnitsY ), averageAngleDistancePainterUnits, showMarker );
      break;
  }
}

void QgsLinearReferencingSymbolLayer::renderPolyline( const QPolygonF &points, QgsSymbolRenderContext &context )
{
  QPainter *p = context.renderContext().painter();
  if ( !p )
  {
    return;
  }

  // TODO (maybe?): if we don't have an original geometry, convert points to linestring and scale distance to painter units?
  // in reality this line type makes no sense for rendering non-real feature geometries...
  ( void )points;
  const QgsAbstractGeometry *geometry = context.renderContext().geometry();
  if ( !geometry )
    return;

  double skipMultiples = mSkipMultiplesOf;
  if ( mDataDefinedProperties.isActive( QgsSymbolLayer::Property::SkipMultiples ) )
  {
    context.setOriginalValueVariable( mSkipMultiplesOf );
    skipMultiples = mDataDefinedProperties.valueAsDouble( QgsSymbolLayer::Property::SkipMultiples, context.renderContext().expressionContext(), mSkipMultiplesOf );
  }

  double labelOffsetX = mLabelOffset.x();
  double labelOffsetY = mLabelOffset.y();

  double averageOver = mAverageAngleLength;
  if ( mDataDefinedProperties.isActive( QgsSymbolLayer::Property::AverageAngleLength ) )
  {
    context.setOriginalValueVariable( mAverageAngleLength );
    averageOver = mDataDefinedProperties.valueAsDouble( QgsSymbolLayer::Property::AverageAngleLength, context.renderContext().expressionContext(), mAverageAngleLength );
  }

  bool showMarker = mShowMarker;
  if ( mDataDefinedProperties.isActive( QgsSymbolLayer::Property::ShowMarker ) )
  {
    context.setOriginalValueVariable( showMarker );
    showMarker = mDataDefinedProperties.valueAsBool( QgsSymbolLayer::Property::ShowMarker, context.renderContext().expressionContext(), mShowMarker );
  }

  const double labelOffsetPainterUnitsX = context.renderContext().convertToPainterUnits( labelOffsetX, mLabelOffsetUnit, mLabelOffsetMapUnitScale );
  const double labelOffsetPainterUnitsY = context.renderContext().convertToPainterUnits( labelOffsetY, mLabelOffsetUnit, mLabelOffsetMapUnitScale );
  const double averageAngleDistancePainterUnits = context.renderContext().convertToPainterUnits( averageOver, mAverageAngleLengthUnit, mAverageAngleLengthMapUnitScale ) / 2;

  for ( auto partIt = geometry->const_parts_begin(); partIt != geometry->const_parts_end(); ++partIt )
  {
    renderGeometryPart( context, *partIt, labelOffsetPainterUnitsX, labelOffsetPainterUnitsY, skipMultiples, averageAngleDistancePainterUnits, showMarker );
  }
}


double calculateAveragedAngle( double targetPointDistanceAlongSegment, double segmentLengthPainterUnits,
                               double averageAngleLengthPainterUnits, double prevXPainterUnits, double prevYPainterUnits,
                               double thisXPainterUnits, double thisYPainterUnits, const double *xPainterUnits,
                               const double *yPainterUnits, int totalPoints, int i )
{

  // track forward by averageAngleLengthPainterUnits
  double painterDistRemaining = averageAngleLengthPainterUnits + targetPointDistanceAlongSegment;
  double startAverageSegmentX = prevXPainterUnits;
  double startAverageSegmentY = prevYPainterUnits;
  double endAverageSegmentX = thisXPainterUnits;
  double endAverageSegmentY = thisYPainterUnits;
  double averagingSegmentLengthPainterUnits = segmentLengthPainterUnits;
  const double *xAveragingData = xPainterUnits;
  const double *yAveragingData = yPainterUnits;

  int j = i;
  while ( painterDistRemaining > averagingSegmentLengthPainterUnits )
  {
    if ( j >= totalPoints - 1 )
      break;

    painterDistRemaining -= averagingSegmentLengthPainterUnits;
    startAverageSegmentX = endAverageSegmentX;
    startAverageSegmentY = endAverageSegmentY;

    endAverageSegmentX = *xAveragingData++;
    endAverageSegmentY = *yAveragingData++;
    j++;
    averagingSegmentLengthPainterUnits = QgsGeometryUtilsBase::distance2D( startAverageSegmentX, startAverageSegmentY, endAverageSegmentX, endAverageSegmentY );
  }
  // fits on this same segment
  double endAverageXPainterUnits;
  double endAverageYPainterUnits;
  if ( painterDistRemaining < averagingSegmentLengthPainterUnits )
  {
    QgsGeometryUtilsBase::pointOnLineWithDistance( startAverageSegmentX, startAverageSegmentY, endAverageSegmentX, endAverageSegmentY, painterDistRemaining, endAverageXPainterUnits, endAverageYPainterUnits,
        nullptr, nullptr, nullptr,
        nullptr, nullptr, nullptr );
  }
  else
  {
    endAverageXPainterUnits = endAverageSegmentX;
    endAverageYPainterUnits = endAverageSegmentY;
  }

  // also track back by averageAngleLengthPainterUnits
  j = i;
  painterDistRemaining = ( segmentLengthPainterUnits - targetPointDistanceAlongSegment ) + averageAngleLengthPainterUnits;
  startAverageSegmentX = thisXPainterUnits;
  startAverageSegmentY = thisYPainterUnits;
  endAverageSegmentX = prevXPainterUnits;
  endAverageSegmentY = prevYPainterUnits;
  averagingSegmentLengthPainterUnits = segmentLengthPainterUnits;
  xAveragingData = xPainterUnits - 2;
  yAveragingData = yPainterUnits - 2;
  while ( painterDistRemaining > averagingSegmentLengthPainterUnits )
  {
    if ( j < 1 )
      break;

    painterDistRemaining -= averagingSegmentLengthPainterUnits;
    startAverageSegmentX = endAverageSegmentX;
    startAverageSegmentY = endAverageSegmentY;

    endAverageSegmentX = *xAveragingData--;
    endAverageSegmentY = *yAveragingData--;
    j--;
    averagingSegmentLengthPainterUnits = QgsGeometryUtilsBase::distance2D( startAverageSegmentX, startAverageSegmentY, endAverageSegmentX, endAverageSegmentY );
  }
  // fits on this same segment
  double startAverageXPainterUnits;
  double startAverageYPainterUnits;
  if ( painterDistRemaining < averagingSegmentLengthPainterUnits )
  {
    QgsGeometryUtilsBase::pointOnLineWithDistance( startAverageSegmentX, startAverageSegmentY, endAverageSegmentX, endAverageSegmentY, painterDistRemaining, startAverageXPainterUnits, startAverageYPainterUnits,
        nullptr, nullptr, nullptr,
        nullptr, nullptr, nullptr );
  }
  else
  {
    startAverageXPainterUnits = endAverageSegmentX;
    startAverageYPainterUnits = endAverageSegmentY;
  }

  double calculatedAngle = std::fmod( QgsGeometryUtilsBase::azimuth( startAverageXPainterUnits, startAverageYPainterUnits, endAverageXPainterUnits, endAverageYPainterUnits ) + 360, 360 );
  if ( calculatedAngle > 90 && calculatedAngle < 270 )
    calculatedAngle += 180;

  return calculatedAngle;
}

typedef std::function<bool ( double x, double y, double z, double m, double distanceFromStart, double angle )> VisitPointFunction;
typedef std::function< void( const QgsLineString *, const QgsLineString *, bool, double, double, const VisitPointFunction & ) > VisitPointAtDistanceFunction;

void visitPointsByRegularDistance( const QgsLineString *line, const QgsLineString *linePainterUnits, bool emitFirstPoint, const double distance, const double averageAngleLengthPainterUnits, const VisitPointFunction &visitPoint )
{
  if ( distance < 0 )
    return;

  double distanceTraversed = 0;
  const int totalPoints = line->numPoints();
  if ( totalPoints == 0 )
    return;

  const double *x = line->xData();
  const double *y = line->yData();
  const double *z = line->is3D() ? line->zData() : nullptr;
  const double *m = line->isMeasure() ? line->mData() : nullptr;

  const double *xPainterUnits = linePainterUnits->xData();
  const double *yPainterUnits = linePainterUnits->yData();

  double prevX = *x++;
  double prevY = *y++;
  double prevZ = z ? *z++ : 0.0;
  double prevM = m ? *m++ : 0.0;

  double prevXPainterUnits = *xPainterUnits++;
  double prevYPainterUnits = *yPainterUnits++;

  if ( qgsDoubleNear( distance, 0.0 ) )
  {
    visitPoint( prevX, prevY, prevZ, prevM, 0, 0 );
    return;
  }

  double pZ = std::numeric_limits<double>::quiet_NaN();
  double pM = std::numeric_limits<double>::quiet_NaN();
  double nextPointDistance = emitFirstPoint ? 0 : distance;
  for ( int i = 1; i < totalPoints; ++i )
  {
    double thisX = *x++;
    double thisY = *y++;
    double thisZ = z ? *z++ : 0.0;
    double thisM = m ? *m++ : 0.0;
    double thisXPainterUnits = *xPainterUnits++;
    double thisYPainterUnits = *yPainterUnits++;

    double angle = std::fmod( QgsGeometryUtilsBase::azimuth( prevXPainterUnits, prevYPainterUnits, thisXPainterUnits, thisYPainterUnits ) + 360, 360 );
    if ( angle > 90 && angle < 270 )
      angle += 180;

    const double segmentLength = QgsGeometryUtilsBase::distance2D( thisX, thisY, prevX, prevY );
    const double segmentLengthPainterUnits = QgsGeometryUtilsBase::distance2D( thisXPainterUnits, thisYPainterUnits, prevXPainterUnits, prevYPainterUnits );

    while ( nextPointDistance < distanceTraversed + segmentLength || qgsDoubleNear( nextPointDistance, distanceTraversed + segmentLength ) )
    {
      // point falls on this segment - truncate to segment length if qgsDoubleNear test was actually > segment length
      const double distanceToPoint = std::min( nextPointDistance - distanceTraversed, segmentLength );
      double pX, pY;
      QgsGeometryUtilsBase::pointOnLineWithDistance( prevX, prevY, thisX, thisY, distanceToPoint, pX, pY,
          z ? &prevZ : nullptr, z ? &thisZ : nullptr, z ? &pZ : nullptr,
          m ? &prevM : nullptr, m ? &thisM : nullptr, m ? &pM : nullptr );

      double calculatedAngle = angle;
      if ( averageAngleLengthPainterUnits > 0 )
      {
        const double targetPointFractionAlongSegment = distanceToPoint / segmentLength;
        const double targetPointDistanceAlongSegment = targetPointFractionAlongSegment * segmentLengthPainterUnits;

        calculatedAngle = calculateAveragedAngle( targetPointDistanceAlongSegment, segmentLengthPainterUnits,
                          averageAngleLengthPainterUnits, prevXPainterUnits, prevYPainterUnits,
                          thisXPainterUnits, thisYPainterUnits, xPainterUnits,
                          yPainterUnits, totalPoints, i );
      }

      if ( !visitPoint( pX, pY, pZ, pM, nextPointDistance, calculatedAngle ) )
        return;

      nextPointDistance += distance;
    }

    distanceTraversed += segmentLength;
    prevX = thisX;
    prevY = thisY;
    prevZ = thisZ;
    prevM = thisM;
    prevXPainterUnits = thisXPainterUnits;
    prevYPainterUnits = thisYPainterUnits;
  }
}

double interpolateValue( double a, double b, double fraction )
{
  return a + ( b - a ) * fraction;
}


void visitPointsByInterpolatedZM( const QgsLineString *line, const QgsLineString *linePainterUnits, bool emitFirstPoint, const double step, const double averageAngleLengthPainterUnits, const VisitPointFunction &visitPoint, bool useZ )
{
  if ( step < 0 )
    return;

  double distanceTraversed = 0;
  const int totalPoints = line->numPoints();
  if ( totalPoints < 2 )
    return;

  const double *x = line->xData();
  const double *y = line->yData();
  const double *z = line->is3D() ? line->zData() : nullptr;
  const double *m = line->isMeasure() ? line->mData() : nullptr;

  const double *xPainterUnits = linePainterUnits->xData();
  const double *yPainterUnits = linePainterUnits->yData();

  double prevX = *x++;
  double prevY = *y++;
  double prevZ = z ? *z++ : 0.0;
  double prevM = m ? *m++ : 0.0;

  double prevXPainterUnits = *xPainterUnits++;
  double prevYPainterUnits = *yPainterUnits++;

  if ( qgsDoubleNear( step, 0.0 ) )
  {
    visitPoint( prevX, prevY, prevZ, prevM, 0, 0 );
    return;
  }

  double prevValue = useZ ? prevZ : prevM;
  bool isFirstPoint = true;
  for ( int i = 1; i < totalPoints; ++i )
  {
    double thisX = *x++;
    double thisY = *y++;
    double thisZ = z ? *z++ : 0.0;
    double thisM = m ? *m++ : 0.0;
    const double thisValue = useZ ? thisZ : thisM;
    double thisXPainterUnits = *xPainterUnits++;
    double thisYPainterUnits = *yPainterUnits++;

    double angle = std::fmod( QgsGeometryUtilsBase::azimuth( prevXPainterUnits, prevYPainterUnits, thisXPainterUnits, thisYPainterUnits ) + 360, 360 );
    if ( angle > 90 && angle < 270 )
      angle += 180;

    const double segmentLength = QgsGeometryUtilsBase::distance2D( thisX, thisY, prevX, prevY );
    const double segmentLengthPainterUnits = QgsGeometryUtilsBase::distance2D( thisXPainterUnits, thisYPainterUnits, prevXPainterUnits, prevYPainterUnits );

    // direction for this segment
    const int direction = ( thisValue > prevValue ) ? 1 : ( thisValue < prevValue ) ? -1 : 0;
    if ( direction != 0 )
    {
      // non-constant segment
      double nextStepValue = direction > 0 ? std::ceil( prevValue / step ) * step
                             :  std::floor( prevValue / step ) * step;

      while ( ( direction > 0 && ( nextStepValue <= thisValue || qgsDoubleNear( nextStepValue, thisValue ) ) ) ||
              ( direction < 0 && ( nextStepValue >= thisValue || qgsDoubleNear( nextStepValue, thisValue ) ) ) )
      {
        const double targetPointFractionAlongSegment = ( nextStepValue - prevValue ) / ( thisValue - prevValue );
        const double targetPointDistanceAlongSegment = targetPointFractionAlongSegment * segmentLengthPainterUnits;

        double pX, pY;
        QgsGeometryUtilsBase::pointOnLineWithDistance( prevX, prevY, thisX, thisY, targetPointFractionAlongSegment  * segmentLength, pX, pY );

        const double pZ = useZ ? nextStepValue : interpolateValue( prevZ, thisZ, targetPointFractionAlongSegment );
        const double pM = useZ ? interpolateValue( prevM, thisM, targetPointFractionAlongSegment ) : nextStepValue;

        double calculatedAngle = angle;
        if ( averageAngleLengthPainterUnits > 0 )
        {
          calculatedAngle = calculateAveragedAngle(
                              targetPointDistanceAlongSegment,
                              segmentLengthPainterUnits, averageAngleLengthPainterUnits,
                              prevXPainterUnits, prevYPainterUnits, thisXPainterUnits, thisYPainterUnits,
                              xPainterUnits, yPainterUnits,
                              totalPoints, i );
        }

        if ( !qgsDoubleNear( targetPointFractionAlongSegment, 0 ) || isFirstPoint )
        {
          if ( !visitPoint( pX, pY, pZ, pM, distanceTraversed + segmentLength * targetPointFractionAlongSegment, calculatedAngle ) )
            return;
        }

        nextStepValue += direction * step;
      }
    }
    else if ( isFirstPoint && emitFirstPoint )
    {
      if ( !visitPoint( prevX, prevY, prevZ, prevM, distanceTraversed,
                        std::fmod( QgsGeometryUtilsBase::azimuth( prevXPainterUnits, prevYPainterUnits, thisXPainterUnits, thisYPainterUnits ) + 360, 360 ) ) )
        return;
    }
    isFirstPoint = false;

    prevX = thisX;
    prevY = thisY;
    prevZ = thisZ;
    prevM = thisM;
    prevXPainterUnits = thisXPainterUnits;
    prevYPainterUnits = thisYPainterUnits;
    prevValue = thisValue;
    distanceTraversed += segmentLength;
  }
}

void visitPointsByInterpolatedZ( const QgsLineString *line, const QgsLineString *linePainterUnits, bool emitFirstPoint, const double distance, const double averageAngleLengthPainterUnits, const VisitPointFunction &visitPoint )
{
  visitPointsByInterpolatedZM( line, linePainterUnits, emitFirstPoint, distance, averageAngleLengthPainterUnits, visitPoint, true );
}

void visitPointsByInterpolatedM( const QgsLineString *line, const QgsLineString *linePainterUnits, bool emitFirstPoint, const double distance, const double averageAngleLengthPainterUnits, const VisitPointFunction &visitPoint )
{
  visitPointsByInterpolatedZM( line, linePainterUnits, emitFirstPoint, distance, averageAngleLengthPainterUnits, visitPoint, false );
}

QPointF QgsLinearReferencingSymbolLayer::pointToPainter( QgsSymbolRenderContext &context, double x, double y, double z )
{
  QPointF pt;
  if ( context.renderContext().coordinateTransform().isValid() )
  {
    context.renderContext().coordinateTransform().transformInPlace( x, y, z );
    pt = QPointF( x, y );

  }
  else
  {
    pt = QPointF( x, y );
  }

  context.renderContext().mapToPixel().transformInPlace( pt.rx(), pt.ry() );
  return pt;
}

void QgsLinearReferencingSymbolLayer::renderPolylineInterval( const QgsLineString *line, QgsSymbolRenderContext &context, double skipMultiples, const QPointF &labelOffsetPainterUnits, double averageAngleLengthPainterUnits, bool showMarker )
{
  double distance = mInterval;
  if ( mDataDefinedProperties.isActive( QgsSymbolLayer::Property::Interval ) )
  {
    context.setOriginalValueVariable( mInterval );
    distance = mDataDefinedProperties.valueAsDouble( QgsSymbolLayer::Property::Interval, context.renderContext().expressionContext(), mInterval );
  }

  QgsNumericFormatContext numericContext;
  numericContext.setExpressionContext( context.renderContext().expressionContext() );

  std::unique_ptr< QgsLineString > painterUnitsGeometry( line->clone() );
  if ( context.renderContext().coordinateTransform().isValid() )
  {
    painterUnitsGeometry->transform( context.renderContext().coordinateTransform() );
  }
  painterUnitsGeometry->transform( context.renderContext().mapToPixel().transform() );

  const bool hasZ = line->is3D();
  const bool hasM = line->isMeasure();
  const bool emitFirstPoint = mLabelSource != Qgis::LinearReferencingLabelSource::CartesianDistance2D;

  VisitPointAtDistanceFunction func = nullptr;

  switch ( mPlacement )
  {
    case Qgis::LinearReferencingPlacement::IntervalCartesian2D:
      func = visitPointsByRegularDistance;
      break;

    case Qgis::LinearReferencingPlacement::IntervalZ:
      func = visitPointsByInterpolatedZ;
      break;

    case Qgis::LinearReferencingPlacement::IntervalM:
      func = visitPointsByInterpolatedM;
      break;

    case Qgis::LinearReferencingPlacement::Vertex:
      return;
  }

  QgsLinearReferencingSymbolLayerLabelProvider *labelProvider = nullptr;
  if ( QgsLabelingEngine *labelingEngine = context.renderContext().labelingEngine() )
  {
    // rendering with a labeling engine. In this scenario we will register rendered text as labels, so that they participate in the labeling problem
    // for the map
    labelProvider = qgis::down_cast< QgsLinearReferencingSymbolLayerLabelProvider * >( labelingEngine->providerById( mLabelProviderId ) );
  }

  func( line, painterUnitsGeometry.get(), emitFirstPoint, distance, averageAngleLengthPainterUnits, [&context, &numericContext, skipMultiples, showMarker,
                  labelOffsetPainterUnits, hasZ, hasM, labelProvider, this]( double x, double y, double z, double m, double distanceFromStart, double angle ) -> bool
  {
    if ( context.renderContext().renderingStopped() )
      return false;

    double labelValue = 0;
    bool labelVertex = true;
    switch ( mLabelSource )
    {
      case Qgis::LinearReferencingLabelSource::CartesianDistance2D:
        labelValue = distanceFromStart;
        break;
      case Qgis::LinearReferencingLabelSource::Z:
        labelValue = z;
        labelVertex = hasZ && !std::isnan( labelValue );
        break;
      case Qgis::LinearReferencingLabelSource::M:
        labelValue = m;
        labelVertex = hasM && !std::isnan( labelValue );
        break;
    }

    if ( !labelVertex )
      return true;

    if ( skipMultiples > 0 && qgsDoubleNear( std::fmod( labelValue,  skipMultiples ), 0 ) )
      return true;

    const QPointF pt = pointToPainter( context, x, y, z );

    if ( mMarkerSymbol && showMarker )
    {
      if ( mRotateLabels )
        mMarkerSymbol->setLineAngle( 90 - angle );
      mMarkerSymbol->renderPoint( pt, context.feature(), context.renderContext() );
    }

    const double angleRadians = ( mRotateLabels ? angle : 0 ) * M_PI / 180.0;
    const double dx = labelOffsetPainterUnits.x() * std::sin( angleRadians + M_PI_2 )
    + labelOffsetPainterUnits.y() * std::sin( angleRadians );
    const double dy = labelOffsetPainterUnits.x() * std::cos( angleRadians + M_PI_2 )
    + labelOffsetPainterUnits.y() * std::cos( angleRadians );

    const QString text = mNumericFormat->formatDouble( labelValue, numericContext );
    if ( !labelProvider )
    {
      // render text directly
      QgsTextRenderer::drawText( QPointF( pt.x() + dx, pt.y() + dy ), angleRadians, Qgis::TextHorizontalAlignment::Left, { text }, context.renderContext(), mTextFormat );
    }
    else
    {
      // register as a label
      labelProvider->addLabel(
        QPointF( pt.x() + dx, pt.y() + dy ), angleRadians, text, context.renderContext(), mTextFormat
      );
    }

    return true;
  } );
}

void QgsLinearReferencingSymbolLayer::renderPolylineVertex( const QgsLineString *line, QgsSymbolRenderContext &context, double skipMultiples, const QPointF &labelOffsetPainterUnits, double averageAngleLengthPainterUnits, bool showMarker )
{
  // let's simplify the logic by ALWAYS using the averaging approach for angles, and just
  // use a very small distance if the user actually set this to 0. It'll be identical
  // results anyway...
  averageAngleLengthPainterUnits = std::max( averageAngleLengthPainterUnits, 0.1 );

  QgsNumericFormatContext numericContext;
  numericContext.setExpressionContext( context.renderContext().expressionContext() );

  QgsLinearReferencingSymbolLayerLabelProvider *labelProvider = nullptr;
  if ( QgsLabelingEngine *labelingEngine = context.renderContext().labelingEngine() )
  {
    // rendering with a labeling engine. In this scenario we will register rendered text as labels, so that they participate in the labeling problem
    // for the map
    labelProvider = qgis::down_cast< QgsLinearReferencingSymbolLayerLabelProvider * >( labelingEngine->providerById( mLabelProviderId ) );
  }

  const double *xData = line->xData();
  const double *yData = line->yData();
  const double *zData = line->is3D() ? line->zData() : nullptr;
  const double *mData = line->isMeasure() ? line->mData() : nullptr;
  const int size = line->numPoints();
  if ( size < 2 )
    return;

  std::unique_ptr< QgsLineString > painterUnitsGeometry( line->clone() );
  if ( context.renderContext().coordinateTransform().isValid() )
  {
    painterUnitsGeometry->transform( context.renderContext().coordinateTransform() );
  }
  painterUnitsGeometry->transform( context.renderContext().mapToPixel().transform() );
  const double *xPainterUnits = painterUnitsGeometry->xData();
  const double *yPainterUnits = painterUnitsGeometry->yData();

  double currentDistance = 0;
  double prevX = *xData;
  double prevY = *yData;

  for ( int i = 0; i < size; ++i )
  {
    if ( context.renderContext().renderingStopped() )
      break;

    double thisX = *xData++;
    double thisY = *yData++;
    double thisZ = zData ? *zData++ : 0;
    double thisM = mData ? *mData++ : 0;
    double thisXPainterUnits = *xPainterUnits++;
    double thisYPainterUnits = *yPainterUnits++;

    const double thisSegmentLength = QgsGeometryUtilsBase::distance2D( prevX, prevY, thisX, thisY );
    currentDistance += thisSegmentLength;

    if ( skipMultiples > 0 && qgsDoubleNear( std::fmod( currentDistance,  skipMultiples ), 0 ) )
    {
      prevX = thisX;
      prevY = thisY;
      continue;
    }

    const QPointF pt = pointToPainter( context, thisX, thisY, thisZ );

    // track forward by averageAngleLengthPainterUnits
    double painterDistRemaining = averageAngleLengthPainterUnits;
    double startAverageSegmentX = thisXPainterUnits;
    double startAverageSegmentY = thisYPainterUnits;

    const double *xAveragingData = xPainterUnits;
    const double *yAveragingData = yPainterUnits;
    double endAverageSegmentX = *xAveragingData;
    double endAverageSegmentY = *yAveragingData;
    double averagingSegmentLengthPainterUnits = QgsGeometryUtilsBase::distance2D( startAverageSegmentX, startAverageSegmentY, endAverageSegmentX, endAverageSegmentY );

    int j = i;
    while ( ( j < size - 1 ) && ( painterDistRemaining > averagingSegmentLengthPainterUnits ) )
    {
      painterDistRemaining -= averagingSegmentLengthPainterUnits;
      startAverageSegmentX = endAverageSegmentX;
      startAverageSegmentY = endAverageSegmentY;

      endAverageSegmentX = *xAveragingData++;
      endAverageSegmentY = *yAveragingData++;
      j++;
      averagingSegmentLengthPainterUnits = QgsGeometryUtilsBase::distance2D( startAverageSegmentX, startAverageSegmentY, endAverageSegmentX, endAverageSegmentY );
    }
    // fits on this same segment
    double endAverageXPainterUnits = thisXPainterUnits;
    double endAverageYPainterUnits = thisYPainterUnits;
    if ( ( j < size - 1 ) && painterDistRemaining < averagingSegmentLengthPainterUnits )
    {
      QgsGeometryUtilsBase::pointOnLineWithDistance( startAverageSegmentX, startAverageSegmentY, endAverageSegmentX, endAverageSegmentY, painterDistRemaining, endAverageXPainterUnits, endAverageYPainterUnits,
          nullptr, nullptr, nullptr,
          nullptr, nullptr, nullptr );
    }
    else if ( i < size - 2 )
    {
      endAverageXPainterUnits = endAverageSegmentX;
      endAverageYPainterUnits = endAverageSegmentY;
    }

    // also track back by averageAngleLengthPainterUnits
    j = i;
    painterDistRemaining = averageAngleLengthPainterUnits;
    startAverageSegmentX = thisXPainterUnits;
    startAverageSegmentY = thisYPainterUnits;

    xAveragingData = xPainterUnits - 2;
    yAveragingData = yPainterUnits - 2;

    endAverageSegmentX = *xAveragingData;
    endAverageSegmentY = *yAveragingData;
    averagingSegmentLengthPainterUnits = QgsGeometryUtilsBase::distance2D( startAverageSegmentX, startAverageSegmentY, endAverageSegmentX, endAverageSegmentY );

    while ( j > 0 && painterDistRemaining > averagingSegmentLengthPainterUnits )
    {
      painterDistRemaining -= averagingSegmentLengthPainterUnits;
      startAverageSegmentX = endAverageSegmentX;
      startAverageSegmentY = endAverageSegmentY;

      endAverageSegmentX = *xAveragingData--;
      endAverageSegmentY = *yAveragingData--;
      j--;
      averagingSegmentLengthPainterUnits = QgsGeometryUtilsBase::distance2D( startAverageSegmentX, startAverageSegmentY, endAverageSegmentX, endAverageSegmentY );
    }
    // fits on this same segment
    double startAverageXPainterUnits = thisXPainterUnits;
    double startAverageYPainterUnits = thisYPainterUnits;
    if ( j > 0 && painterDistRemaining < averagingSegmentLengthPainterUnits )
    {
      QgsGeometryUtilsBase::pointOnLineWithDistance( startAverageSegmentX, startAverageSegmentY, endAverageSegmentX, endAverageSegmentY, painterDistRemaining, startAverageXPainterUnits, startAverageYPainterUnits,
          nullptr, nullptr, nullptr,
          nullptr, nullptr, nullptr );
    }
    else if ( j > 1 )
    {
      startAverageXPainterUnits = endAverageSegmentX;
      startAverageYPainterUnits = endAverageSegmentY;
    }

    double calculatedAngle = std::fmod( QgsGeometryUtilsBase::azimuth( startAverageXPainterUnits, startAverageYPainterUnits, endAverageXPainterUnits, endAverageYPainterUnits ) + 360, 360 );

    if ( calculatedAngle > 90 && calculatedAngle < 270 )
      calculatedAngle += 180;

    if ( mMarkerSymbol && showMarker )
    {
      if ( mRotateLabels )
        mMarkerSymbol->setLineAngle( 90 - calculatedAngle );
      mMarkerSymbol->renderPoint( pt, context.feature(), context.renderContext() );
    }

    const double angleRadians = mRotateLabels ? ( calculatedAngle * M_PI / 180.0 ) : 0;
    const double dx = labelOffsetPainterUnits.x() * std::sin( angleRadians + M_PI_2 )
                      + labelOffsetPainterUnits.y() * std::sin( angleRadians );
    const double dy = labelOffsetPainterUnits.x() * std::cos( angleRadians + M_PI_2 )
                      + labelOffsetPainterUnits.y() * std::cos( angleRadians );

    double labelValue = 0;
    bool labelVertex = true;
    switch ( mLabelSource )
    {
      case Qgis::LinearReferencingLabelSource::CartesianDistance2D:
        labelValue = currentDistance;
        break;
      case Qgis::LinearReferencingLabelSource::Z:
        labelValue = thisZ;
        labelVertex = static_cast< bool >( zData ) && !std::isnan( labelValue );
        break;
      case Qgis::LinearReferencingLabelSource::M:
        labelValue = thisM;
        labelVertex = static_cast< bool >( mData ) && !std::isnan( labelValue );
        break;
    }

    if ( !labelVertex )
      continue;

    const QString text = mNumericFormat->formatDouble( labelValue, numericContext );
    if ( !labelProvider )
    {
      // render text directly
      QgsTextRenderer::drawText( QPointF( pt.x() + dx, pt.y() + dy ), angleRadians, Qgis::TextHorizontalAlignment::Left, { text }, context.renderContext(), mTextFormat );
    }
    else
    {
      // register as a label
      labelProvider->addLabel(
        QPointF( pt.x() + dx, pt.y() + dy ), angleRadians, text, context.renderContext(), mTextFormat
      );
    }
    prevX = thisX;
    prevY = thisY;
  }
}

QgsTextFormat QgsLinearReferencingSymbolLayer::textFormat() const
{
  return mTextFormat;
}

void QgsLinearReferencingSymbolLayer::setTextFormat( const QgsTextFormat &format )
{
  mTextFormat = format;
}

QgsNumericFormat *QgsLinearReferencingSymbolLayer::numericFormat() const
{
  return mNumericFormat.get();
}

void QgsLinearReferencingSymbolLayer::setNumericFormat( QgsNumericFormat *format )
{
  mNumericFormat.reset( format );
}

double QgsLinearReferencingSymbolLayer::interval() const
{
  return mInterval;
}

void QgsLinearReferencingSymbolLayer::setInterval( double interval )
{
  mInterval = interval;
}

double QgsLinearReferencingSymbolLayer::skipMultiplesOf() const
{
  return mSkipMultiplesOf;
}

void QgsLinearReferencingSymbolLayer::setSkipMultiplesOf( double skipMultiplesOf )
{
  mSkipMultiplesOf = skipMultiplesOf;
}

bool QgsLinearReferencingSymbolLayer::showMarker() const
{
  return mShowMarker;
}

void QgsLinearReferencingSymbolLayer::setShowMarker( bool show )
{
  mShowMarker = show;
  if ( show && !mMarkerSymbol )
  {
    mMarkerSymbol.reset( QgsMarkerSymbol::createSimple( {} ) );
  }
}

Qgis::LinearReferencingPlacement QgsLinearReferencingSymbolLayer::placement() const
{
  return mPlacement;
}

void QgsLinearReferencingSymbolLayer::setPlacement( Qgis::LinearReferencingPlacement placement )
{
  mPlacement = placement;
}

Qgis::LinearReferencingLabelSource QgsLinearReferencingSymbolLayer::labelSource() const
{
  return mLabelSource;
}

void QgsLinearReferencingSymbolLayer::setLabelSource( Qgis::LinearReferencingLabelSource source )
{
  mLabelSource = source;
}

