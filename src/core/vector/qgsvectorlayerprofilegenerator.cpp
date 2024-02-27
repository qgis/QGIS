/***************************************************************************
                         qgsvectorlayerprofilegenerator.cpp
                         ---------------
    begin                : March 2022
    copyright            : (C) 2022 by Nyall Dawson
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
#include "qgsvectorlayerprofilegenerator.h"
#include "qgsprofilerequest.h"
#include "qgscurve.h"
#include "qgsvectorlayer.h"
#include "qgsvectorlayerelevationproperties.h"
#include "qgscoordinatetransform.h"
#include "qgsgeos.h"
#include "qgsvectorlayerfeatureiterator.h"
#include "qgsterrainprovider.h"
#include "qgspolygon.h"
#include "qgstessellator.h"
#include "qgsmultipolygon.h"
#include "qgsmeshlayerutils.h"
#include "qgsmultipoint.h"
#include "qgsmultilinestring.h"
#include "qgslinesymbol.h"
#include "qgsfillsymbol.h"
#include "qgsmarkersymbol.h"
#include "qgsprofilepoint.h"
#include "qgsprofilesnapping.h"
#include "qgsexpressioncontextutils.h"
#include <QPolygonF>

//
// QgsVectorLayerProfileResults
//

QString QgsVectorLayerProfileResults::type() const
{
  return QStringLiteral( "vector" );
}

QVector<QgsGeometry> QgsVectorLayerProfileResults::asGeometries() const
{
  QVector<QgsGeometry> res;
  res.reserve( features.size() );
  for ( auto it = features.constBegin(); it != features.constEnd(); ++it )
  {
    for ( const Feature &feature : it.value() )
    {
      res.append( feature.geometry );
    }
  }
  return res;
}

QVector<QgsAbstractProfileResults::Feature> QgsVectorLayerProfileResults::asFeatures( Qgis::ProfileExportType type, QgsFeedback *feedback ) const
{
  switch ( profileType )
  {
    case Qgis::VectorProfileType::IndividualFeatures:
      if ( type != Qgis::ProfileExportType::DistanceVsElevationTable )
        return asIndividualFeatures( type, feedback );
      // distance vs elevation table results are always handled like a continuous surface
      [[fallthrough]];

    case Qgis::VectorProfileType::ContinuousSurface:
      return QgsAbstractProfileSurfaceResults::asFeatures( type, feedback );
  }
  BUILTIN_UNREACHABLE
}

QgsProfileSnapResult QgsVectorLayerProfileResults::snapPoint( const QgsProfilePoint &point, const QgsProfileSnapContext &context )
{
  switch ( profileType )
  {
    case Qgis::VectorProfileType::IndividualFeatures:
      return snapPointToIndividualFeatures( point, context );
    case Qgis::VectorProfileType::ContinuousSurface:
      return QgsAbstractProfileSurfaceResults::snapPoint( point, context );
  }
  BUILTIN_UNREACHABLE
}


QVector<QgsProfileIdentifyResults> QgsVectorLayerProfileResults::identify( const QgsDoubleRange &distanceRange, const QgsDoubleRange &elevationRange, const QgsProfileIdentifyContext & )
{
  QgsFeatureIds ids;
  auto visitFeature = [&ids]( QgsFeatureId featureId )
  {
    ids << featureId;
  };

  visitFeaturesInRange( distanceRange, elevationRange, visitFeature );
  if ( ids.empty() )
    return {};

  QVector< QVariantMap> idsList;
  for ( auto it = ids.constBegin(); it != ids.constEnd(); ++it )
    idsList.append( QVariantMap( {{QStringLiteral( "id" ), *it}} ) );

  return { QgsProfileIdentifyResults( mLayer, idsList ) };
}

QVector<QgsProfileIdentifyResults> QgsVectorLayerProfileResults::identify( const QgsProfilePoint &point, const QgsProfileIdentifyContext &context )
{
  QHash< QgsFeatureId, QVariantMap > features;
  auto visitFeature = [&features]( QgsFeatureId featureId, double delta, double distance, double elevation )
  {
    auto it = features.find( featureId );
    if ( it == features.end() )
    {
      features[ featureId ] = QVariantMap( {{QStringLiteral( "id" ), featureId },
        {QStringLiteral( "delta" ), delta },
        {QStringLiteral( "distance" ), distance },
        {QStringLiteral( "elevation" ), elevation }
      } );
    }
    else
    {
      const double currentDelta = it.value().value( QStringLiteral( "delta" ) ).toDouble();
      if ( delta < currentDelta )
      {
        *it = QVariantMap( {{QStringLiteral( "id" ), featureId },
          {QStringLiteral( "delta" ), delta },
          {QStringLiteral( "distance" ), distance },
          {QStringLiteral( "elevation" ), elevation }
        } );
      }
    }
  };

  visitFeaturesAtPoint( point, context.maximumPointDistanceDelta, context.maximumPointElevationDelta, context.maximumSurfaceElevationDelta, visitFeature, true );

  QVector< QVariantMap> attributes;
  for ( auto it = features.constBegin(); it != features.constEnd(); ++it )
    attributes.append( *it );

  QVector<QgsProfileIdentifyResults> res;

  if ( !attributes.empty() )
    res.append( QgsProfileIdentifyResults( mLayer, attributes ) );

  if ( res.empty() && profileType == Qgis::VectorProfileType::ContinuousSurface )
  {
    const QVector<QgsProfileIdentifyResults> surfaceResults = QgsAbstractProfileSurfaceResults::identify( point, context );
    res.reserve( surfaceResults.size() );
    for ( const QgsProfileIdentifyResults &surfaceResult : surfaceResults )
    {
      res.append( QgsProfileIdentifyResults( mLayer, surfaceResult.results() ) );
    }
  }

  return res;
}

QgsProfileSnapResult QgsVectorLayerProfileResults::snapPointToIndividualFeatures( const QgsProfilePoint &point, const QgsProfileSnapContext &context )
{
  QgsProfileSnapResult res;
  double bestSnapDistance = std::numeric_limits< double >::max();

  auto visitFeature = [&bestSnapDistance, &res]( QgsFeatureId, double delta, double distance, double elevation )
  {
    if ( distance < bestSnapDistance )
    {
      bestSnapDistance = delta;
      res.snappedPoint = QgsProfilePoint( distance, elevation );
    }
  };

  visitFeaturesAtPoint( point, context.maximumPointDistanceDelta, context.maximumPointElevationDelta, context.maximumSurfaceElevationDelta, visitFeature, false );

  return res;
}

void QgsVectorLayerProfileResults::visitFeaturesAtPoint( const QgsProfilePoint &point, double maximumPointDistanceDelta, double maximumPointElevationDelta, double maximumSurfaceElevationDelta,  const std::function< void( QgsFeatureId, double delta, double distance, double elevation ) > &visitor, bool visitWithin )
{
  // TODO -- add spatial index if performance is an issue

  const QgsPoint targetPoint( point.distance(), point.elevation() );

  for ( auto it = features.constBegin(); it != features.constEnd(); ++it )
  {
    for ( const Feature &feature : it.value() )
    {
      const QgsRectangle featureBounds = feature.crossSectionGeometry.boundingBox();
      if ( ( featureBounds.xMinimum() - maximumPointDistanceDelta <= point.distance() ) && ( featureBounds.xMaximum() + maximumPointDistanceDelta >= point.distance() ) )
      {
        switch ( feature.crossSectionGeometry.type() )
        {
          case Qgis::GeometryType::Point:
          {
            for ( auto partIt = feature.crossSectionGeometry.const_parts_begin(); partIt != feature.crossSectionGeometry.const_parts_end(); ++partIt )
            {
              if ( const QgsPoint *candidatePoint = qgsgeometry_cast< const QgsPoint * >( *partIt ) )
              {
                const double snapDistanceDelta = std::fabs( point.distance() - candidatePoint->x() );
                if ( snapDistanceDelta > maximumPointDistanceDelta )
                  continue;

                const double snapHeightDelta = std::fabs( point.elevation() - candidatePoint->y() );
                if ( snapHeightDelta > maximumPointElevationDelta )
                  continue;

                const double snapDistance = candidatePoint->distance( targetPoint );
                visitor( feature.featureId, snapDistance, candidatePoint->x(), candidatePoint->y() );
              }
            }
            break;
          }

          case Qgis::GeometryType::Line:
          {
            for ( auto partIt = feature.crossSectionGeometry.const_parts_begin(); partIt != feature.crossSectionGeometry.const_parts_end(); ++partIt )
            {
              if ( const QgsCurve *line = qgsgeometry_cast< const QgsCurve * >( *partIt ) )
              {
                // might be a vertical line
                if ( const QgsLineString *lineString = qgsgeometry_cast< const QgsLineString * >( line ) )
                {
                  if ( lineString->numPoints() == 2 && qgsDoubleNear( lineString->pointN( 0 ).x(), lineString->pointN( 1 ).x() ) )
                  {
                    const double snapDistanceDelta = std::fabs( point.distance() - lineString->pointN( 0 ).x() );
                    if ( snapDistanceDelta > maximumPointDistanceDelta )
                      continue;

                    const double snapHeightDelta = std::fabs( point.elevation() - lineString->pointN( 0 ).y() );
                    if ( snapHeightDelta <= maximumPointElevationDelta )
                    {
                      const double snapDistanceP1 = lineString->pointN( 0 ).distance( targetPoint );
                      visitor( feature.featureId, snapDistanceP1, lineString->pointN( 0 ).x(), lineString->pointN( 0 ).y() );
                    }

                    const double snapHeightDelta2 = std::fabs( point.elevation() - lineString->pointN( 1 ).y() );
                    if ( snapHeightDelta2 <= maximumPointElevationDelta )
                    {
                      const double snapDistanceP2 = lineString->pointN( 1 ).distance( targetPoint );
                      visitor( feature.featureId, snapDistanceP2, lineString->pointN( 1 ).x(), lineString->pointN( 1 ).y() );
                    }

                    if ( visitWithin )
                    {
                      double elevation1 = lineString->pointN( 0 ).y();
                      double elevation2 = lineString->pointN( 1 ).y();
                      if ( elevation1 > elevation2 )
                        std::swap( elevation1, elevation2 );

                      if ( point.elevation() > elevation1 && point.elevation() < elevation2 )
                      {
                        const double snapDistance = std::fabs( lineString->pointN( 0 ).x() - point.distance() );
                        visitor( feature.featureId, snapDistance, lineString->pointN( 0 ).x(), point.elevation() );
                      }
                    }
                    continue;
                  }
                }

                const QgsRectangle partBounds = ( *partIt )->boundingBox();
                if ( point.distance() < partBounds.xMinimum() - maximumPointDistanceDelta || point.distance() > partBounds.xMaximum() + maximumPointDistanceDelta )
                  continue;

                const double snappedDistance = point.distance() < partBounds.xMinimum() ? partBounds.xMinimum()
                                               : point.distance() > partBounds.xMaximum() ? partBounds.xMaximum() : point.distance();

                const QgsGeometry cutLine( new QgsLineString( QgsPoint( snappedDistance, qgsDoubleNear( minZ, maxZ ) ? minZ - 1 : minZ ), QgsPoint( snappedDistance, maxZ ) ) );
                QgsGeos cutLineGeos( cutLine.constGet() );

                const QgsGeometry points( cutLineGeos.intersection( line ) );

                for ( auto vertexIt = points.vertices_begin(); vertexIt != points.vertices_end(); ++vertexIt )
                {
                  const double snapHeightDelta = std::fabs( point.elevation() - ( *vertexIt ).y() );
                  if ( snapHeightDelta > maximumSurfaceElevationDelta )
                    continue;

                  const double snapDistance = ( *vertexIt ).distance( targetPoint );
                  visitor( feature.featureId, snapDistance, ( *vertexIt ).x(), ( *vertexIt ).y() );
                }
              }
            }
            break;
          }

          case Qgis::GeometryType::Polygon:
          {
            if ( visitWithin )
            {
              if ( feature.crossSectionGeometry.intersects( QgsGeometry::fromPointXY( QgsPointXY( point.distance(), point.elevation() ) ) ) )
              {
                visitor( feature.featureId, 0, point.distance(), point.elevation() );
                break;
              }
            }
            for ( auto partIt = feature.crossSectionGeometry.const_parts_begin(); partIt != feature.crossSectionGeometry.const_parts_end(); ++partIt )
            {
              if ( const QgsCurve *exterior = qgsgeometry_cast< const QgsPolygon * >( *partIt )->exteriorRing() )
              {
                const QgsRectangle partBounds = ( *partIt )->boundingBox();
                if ( point.distance() < partBounds.xMinimum() - maximumPointDistanceDelta || point.distance() > partBounds.xMaximum() + maximumPointDistanceDelta )
                  continue;

                const double snappedDistance = point.distance() < partBounds.xMinimum() ? partBounds.xMinimum()
                                               : point.distance() > partBounds.xMaximum() ? partBounds.xMaximum() : point.distance();

                const QgsGeometry cutLine( new QgsLineString( QgsPoint( snappedDistance, qgsDoubleNear( minZ, maxZ ) ? minZ - 1 : minZ ), QgsPoint( snappedDistance, maxZ ) ) );
                QgsGeos cutLineGeos( cutLine.constGet() );

                const QgsGeometry points( cutLineGeos.intersection( exterior ) );
                for ( auto vertexIt = points.vertices_begin(); vertexIt != points.vertices_end(); ++vertexIt )
                {
                  const double snapHeightDelta = std::fabs( point.elevation() - ( *vertexIt ).y() );
                  if ( snapHeightDelta > maximumSurfaceElevationDelta )
                    continue;

                  const double snapDistance = ( *vertexIt ).distance( targetPoint );
                  visitor( feature.featureId, snapDistance, ( *vertexIt ).x(), ( *vertexIt ).y() );
                }
              }
            }
            break;
          }
          case Qgis::GeometryType::Unknown:
          case Qgis::GeometryType::Null:
            break;
        }
      }
    }
  }
}

void QgsVectorLayerProfileResults::visitFeaturesInRange( const QgsDoubleRange &distanceRange, const QgsDoubleRange &elevationRange, const std::function<void ( QgsFeatureId )> &visitor )
{
  // TODO -- add spatial index if performance is an issue
  const QgsRectangle profileRange( distanceRange.lower(), elevationRange.lower(), distanceRange.upper(), elevationRange.upper() );
  const QgsGeometry profileRangeGeometry = QgsGeometry::fromRect( profileRange );
  QgsGeos profileRangeGeos( profileRangeGeometry.constGet() );
  profileRangeGeos.prepareGeometry();

  for ( auto it = features.constBegin(); it != features.constEnd(); ++it )
  {
    for ( const Feature &feature : it.value() )
    {
      if ( feature.crossSectionGeometry.boundingBoxIntersects( profileRange ) )
      {
        switch ( feature.crossSectionGeometry.type() )
        {
          case Qgis::GeometryType::Point:
          {
            for ( auto partIt = feature.crossSectionGeometry.const_parts_begin(); partIt != feature.crossSectionGeometry.const_parts_end(); ++partIt )
            {
              if ( const QgsPoint *candidatePoint = qgsgeometry_cast< const QgsPoint * >( *partIt ) )
              {
                if ( profileRange.contains( candidatePoint->x(), candidatePoint->y() ) )
                {
                  visitor( feature.featureId );
                }
              }
            }
            break;
          }

          case Qgis::GeometryType::Line:
          case Qgis::GeometryType::Polygon:
          {
            if ( profileRangeGeos.intersects( feature.crossSectionGeometry.constGet() ) )
            {
              visitor( feature.featureId );
            }
            break;
          }

          case Qgis::GeometryType::Unknown:
          case Qgis::GeometryType::Null:
            break;
        }
      }
    }
  }
}

void QgsVectorLayerProfileResults::renderResults( QgsProfileRenderContext &context )
{
  const QgsExpressionContextScopePopper scopePopper( context.renderContext().expressionContext(), mLayer ? mLayer->createExpressionContextScope() : nullptr );
  switch ( profileType )
  {
    case Qgis::VectorProfileType::IndividualFeatures:
      renderResultsAsIndividualFeatures( context );
      break;
    case Qgis::VectorProfileType::ContinuousSurface:
      QgsAbstractProfileSurfaceResults::renderResults( context );
      if ( mShowMarkerSymbolInSurfacePlots )
        renderMarkersOverContinuousSurfacePlot( context );
      break;
  }
}

void QgsVectorLayerProfileResults::renderResultsAsIndividualFeatures( QgsProfileRenderContext &context )
{
  QPainter *painter = context.renderContext().painter();
  if ( !painter )
    return;

  const QgsScopedQPainterState painterState( painter );

  painter->setBrush( Qt::NoBrush );
  painter->setPen( Qt::NoPen );

  const double minDistance = context.distanceRange().lower();
  const double maxDistance = context.distanceRange().upper();
  const double minZ = context.elevationRange().lower();
  const double maxZ = context.elevationRange().upper();

  const QRectF visibleRegion( minDistance, minZ, maxDistance - minDistance, maxZ - minZ );
  QPainterPath clipPath;
  clipPath.addPolygon( context.worldTransform().map( visibleRegion ) );
  painter->setClipPath( clipPath, Qt::ClipOperation::IntersectClip );

  const QgsRectangle clipPathRect( clipPath.boundingRect() );

  auto renderResult = [&context, &clipPathRect]( const Feature & profileFeature, QgsMarkerSymbol * markerSymbol, QgsLineSymbol * lineSymbol, QgsFillSymbol * fillSymbol )
  {
    if ( profileFeature.crossSectionGeometry.isEmpty() )
      return;

    QgsGeometry transformed = profileFeature.crossSectionGeometry;
    transformed.transform( context.worldTransform() );

    if ( !transformed.boundingBoxIntersects( clipPathRect ) )
      return;

    // we can take some shortcuts here, because we know that the geometry will already be segmentized and can't be a curved type
    switch ( transformed.type() )
    {
      case Qgis::GeometryType::Point:
      {
        if ( const QgsPoint *point = qgsgeometry_cast< const QgsPoint * >( transformed.constGet() ) )
        {
          markerSymbol->renderPoint( QPointF( point->x(), point->y() ), nullptr, context.renderContext() );
        }
        else if ( const QgsMultiPoint *multipoint = qgsgeometry_cast< const QgsMultiPoint * >( transformed.constGet() ) )
        {
          const int numGeometries = multipoint->numGeometries();
          for ( int i = 0; i < numGeometries; ++i )
          {
            markerSymbol->renderPoint( QPointF( multipoint->pointN( i )->x(), multipoint->pointN( i )->y() ), nullptr, context.renderContext() );
          }
        }
        break;
      }

      case Qgis::GeometryType::Line:
      {
        if ( const QgsLineString *line = qgsgeometry_cast< const QgsLineString * >( transformed.constGet() ) )
        {
          lineSymbol->renderPolyline( line->asQPolygonF(), nullptr, context.renderContext() );
        }
        else if ( const QgsMultiLineString *multiLinestring = qgsgeometry_cast< const QgsMultiLineString * >( transformed.constGet() ) )
        {
          const int numGeometries = multiLinestring->numGeometries();
          for ( int i = 0; i < numGeometries; ++i )
          {
            lineSymbol->renderPolyline( multiLinestring->lineStringN( i )->asQPolygonF(), nullptr, context.renderContext() );
          }
        }
        break;
      }

      case Qgis::GeometryType::Polygon:
      {
        if ( const QgsPolygon *polygon = qgsgeometry_cast< const QgsPolygon * >( transformed.constGet() ) )
        {
          if ( const QgsCurve *exterior = polygon->exteriorRing() )
            fillSymbol->renderPolygon( exterior->asQPolygonF(), nullptr, nullptr, context.renderContext() );
        }
        else if ( const QgsMultiPolygon *multiPolygon = qgsgeometry_cast< const QgsMultiPolygon * >( transformed.constGet() ) )
        {
          const int numGeometries = multiPolygon->numGeometries();
          for ( int i = 0; i < numGeometries; ++i )
          {
            fillSymbol->renderPolygon( multiPolygon->polygonN( i )->exteriorRing()->asQPolygonF(), nullptr, nullptr, context.renderContext() );
          }
        }
        break;
      }

      case Qgis::GeometryType::Unknown:
      case Qgis::GeometryType::Null:
        return;
    }
  };

  QgsFeatureRequest req;
  req.setFilterFids( qgis::listToSet( features.keys() ) );

  if ( respectLayerSymbology && mLayer && mLayer->renderer() )
  {
    std::unique_ptr< QgsFeatureRenderer > renderer( mLayer->renderer()->clone() );
    renderer->startRender( context.renderContext(), mLayer->fields() );

    // if we are respecting the layer's symbology then we'll fire off a feature request and iterate through
    // features from the profile, rendering each in turn
    QSet<QString> attributes = renderer->usedAttributes( context.renderContext() );

    std::unique_ptr< QgsMarkerSymbol > marker( mMarkerSymbol->clone() );
    std::unique_ptr< QgsLineSymbol > line( mLineSymbol->clone() );
    std::unique_ptr< QgsFillSymbol > fill( mFillSymbol->clone() );
    attributes.unite( marker->usedAttributes( context.renderContext() ) );
    attributes.unite( line->usedAttributes( context.renderContext() ) );
    attributes.unite( fill->usedAttributes( context.renderContext() ) );

    req.setSubsetOfAttributes( attributes, mLayer->fields() );

    QgsFeature feature;
    QgsFeatureIterator it = mLayer->getFeatures( req );
    while ( it.nextFeature( feature ) )
    {
      context.renderContext().expressionContext().setFeature( feature );
      QgsSymbol *rendererSymbol = renderer->symbolForFeature( feature, context.renderContext() );
      if ( !rendererSymbol )
        continue;

      marker->setColor( rendererSymbol->color() );
      marker->setOpacity( rendererSymbol->opacity() );
      line->setColor( rendererSymbol->color() );
      line->setOpacity( rendererSymbol->opacity() );
      fill->setColor( rendererSymbol->color() );
      fill->setOpacity( rendererSymbol->opacity() );

      marker->startRender( context.renderContext() );
      line->startRender( context.renderContext() );
      fill->startRender( context.renderContext() );

      const QVector< Feature > profileFeatures = features.value( feature.id() );
      for ( const Feature &profileFeature : profileFeatures )
      {
        renderResult( profileFeature,
                      rendererSymbol->type() == Qgis::SymbolType::Marker ? qgis::down_cast< QgsMarkerSymbol * >( rendererSymbol ) : marker.get(),
                      rendererSymbol->type() == Qgis::SymbolType::Line ? qgis::down_cast< QgsLineSymbol * >( rendererSymbol )  : line.get(),
                      rendererSymbol->type() == Qgis::SymbolType::Fill ? qgis::down_cast< QgsFillSymbol * >( rendererSymbol )  : fill.get() );
      }

      marker->stopRender( context.renderContext() );
      line->stopRender( context.renderContext() );
      fill->stopRender( context.renderContext() );
    }

    renderer->stopRender( context.renderContext() );
  }
  else if ( mLayer )
  {
    QSet<QString> attributes;
    attributes.unite( mMarkerSymbol->usedAttributes( context.renderContext() ) );
    attributes.unite( mFillSymbol->usedAttributes( context.renderContext() ) );
    attributes.unite( mLineSymbol->usedAttributes( context.renderContext() ) );

    mMarkerSymbol->startRender( context.renderContext() );
    mFillSymbol->startRender( context.renderContext() );
    mLineSymbol->startRender( context.renderContext() );
    req.setSubsetOfAttributes( attributes, mLayer->fields() );

    QgsFeature feature;
    QgsFeatureIterator it = mLayer->getFeatures( req );
    while ( it.nextFeature( feature ) )
    {
      context.renderContext().expressionContext().setFeature( feature );
      const QVector< Feature > profileFeatures = features.value( feature.id() );
      for ( const Feature &profileFeature : profileFeatures )
      {
        renderResult( profileFeature, mMarkerSymbol.get(), mLineSymbol.get(), mFillSymbol.get() );
      }
    }
    mMarkerSymbol->stopRender( context.renderContext() );
    mFillSymbol->stopRender( context.renderContext() );
    mLineSymbol->stopRender( context.renderContext() );
  }
}

void QgsVectorLayerProfileResults::renderMarkersOverContinuousSurfacePlot( QgsProfileRenderContext &context )
{
  QPainter *painter = context.renderContext().painter();
  if ( !painter )
    return;

  const QgsScopedQPainterState painterState( painter );

  painter->setBrush( Qt::NoBrush );
  painter->setPen( Qt::NoPen );

  const double minDistance = context.distanceRange().lower();
  const double maxDistance = context.distanceRange().upper();
  const double minZ = context.elevationRange().lower();
  const double maxZ = context.elevationRange().upper();

  const QRectF visibleRegion( minDistance, minZ, maxDistance - minDistance, maxZ - minZ );
  QPainterPath clipPath;
  clipPath.addPolygon( context.worldTransform().map( visibleRegion ) );
  painter->setClipPath( clipPath, Qt::ClipOperation::IntersectClip );

  mMarkerSymbol->startRender( context.renderContext() );

  for ( auto pointIt = mDistanceToHeightMap.constBegin(); pointIt != mDistanceToHeightMap.constEnd(); ++pointIt )
  {
    if ( std::isnan( pointIt.value() ) )
      continue;

    mMarkerSymbol->renderPoint( context.worldTransform().map( QPointF( pointIt.key(), pointIt.value() ) ), nullptr, context.renderContext() );
  }
  mMarkerSymbol->stopRender( context.renderContext() );
}

QVector<QgsAbstractProfileResults::Feature> QgsVectorLayerProfileResults::asIndividualFeatures( Qgis::ProfileExportType type, QgsFeedback *feedback ) const
{
  QVector<QgsAbstractProfileResults::Feature> res;
  res.reserve( features.size() );
  for ( auto it = features.constBegin(); it != features.constEnd(); ++it )
  {
    if ( feedback && feedback->isCanceled() )
      break;

    for ( const Feature &feature : it.value() )
    {
      if ( feedback && feedback->isCanceled() )
        break;

      QgsAbstractProfileResults::Feature outFeature;
      outFeature.layerIdentifier = mId;
      outFeature.attributes = {{QStringLiteral( "id" ), feature.featureId }};
      switch ( type )
      {
        case Qgis::ProfileExportType::Features3D:
          outFeature.geometry = feature.geometry;
          break;

        case Qgis::ProfileExportType::Profile2D:
          outFeature.geometry = feature.crossSectionGeometry;
          break;

        case Qgis::ProfileExportType::DistanceVsElevationTable:
          break; // unreachable
      }
      res << outFeature;
    }
  }
  return res;
}

void QgsVectorLayerProfileResults::copyPropertiesFromGenerator( const QgsAbstractProfileGenerator *generator )
{
  QgsAbstractProfileSurfaceResults::copyPropertiesFromGenerator( generator );
  const QgsVectorLayerProfileGenerator *vlGenerator = qgis::down_cast<  const QgsVectorLayerProfileGenerator * >( generator );

  mId = vlGenerator->mId;
  profileType = vlGenerator->mType;
  respectLayerSymbology = vlGenerator->mRespectLayerSymbology;
  mMarkerSymbol.reset( vlGenerator->mProfileMarkerSymbol->clone() );
  mShowMarkerSymbolInSurfacePlots = vlGenerator->mShowMarkerSymbolInSurfacePlots;
}

//
// QgsVectorLayerProfileGenerator
//

QgsVectorLayerProfileGenerator::QgsVectorLayerProfileGenerator( QgsVectorLayer *layer, const QgsProfileRequest &request )
  : QgsAbstractProfileSurfaceGenerator( request )
  , mId( layer->id() )
  , mFeedback( std::make_unique< QgsFeedback >() )
  , mProfileCurve( request.profileCurve() ? request.profileCurve()->clone() : nullptr )
  , mTerrainProvider( request.terrainProvider() ? request.terrainProvider()->clone() : nullptr )
  , mTolerance( request.tolerance() )
  , mSourceCrs( layer->crs() )
  , mTargetCrs( request.crs() )
  , mTransformContext( request.transformContext() )
  , mExtent( layer->extent() )
  , mSource( std::make_unique< QgsVectorLayerFeatureSource >( layer ) )
  , mOffset( layer->elevationProperties()->zOffset() )
  , mScale( layer->elevationProperties()->zScale() )
  , mType( qgis::down_cast< QgsVectorLayerElevationProperties * >( layer->elevationProperties() )->type() )
  , mClamping( qgis::down_cast< QgsVectorLayerElevationProperties * >( layer->elevationProperties() )->clamping() )
  , mBinding( qgis::down_cast< QgsVectorLayerElevationProperties * >( layer->elevationProperties() )->binding() )
  , mExtrusionEnabled( qgis::down_cast< QgsVectorLayerElevationProperties * >( layer->elevationProperties() )->extrusionEnabled() )
  , mExtrusionHeight( qgis::down_cast< QgsVectorLayerElevationProperties * >( layer->elevationProperties() )->extrusionHeight() )
  , mExpressionContext( request.expressionContext() )
  , mFields( layer->fields() )
  , mDataDefinedProperties( layer->elevationProperties()->dataDefinedProperties() )
  , mWkbType( layer->wkbType() )
  , mRespectLayerSymbology( qgis::down_cast< QgsVectorLayerElevationProperties * >( layer->elevationProperties() )->respectLayerSymbology() )
  , mProfileMarkerSymbol( qgis::down_cast< QgsVectorLayerElevationProperties * >( layer->elevationProperties() )->profileMarkerSymbol()->clone() )
  , mShowMarkerSymbolInSurfacePlots( qgis::down_cast< QgsVectorLayerElevationProperties * >( layer->elevationProperties() )->showMarkerSymbolInSurfacePlots() )
  , mLayer( layer )
{
  if ( mTerrainProvider )
    mTerrainProvider->prepare(); // must be done on main thread

  // make sure profile curve is always 2d, or we may get unwanted z value averaging for intersections from GEOS
  if ( mProfileCurve )
    mProfileCurve->dropZValue();

  mSymbology = qgis::down_cast< QgsVectorLayerElevationProperties * >( layer->elevationProperties() )->profileSymbology();
  mElevationLimit = qgis::down_cast< QgsVectorLayerElevationProperties * >( layer->elevationProperties() )->elevationLimit();

  mLineSymbol.reset( qgis::down_cast< QgsVectorLayerElevationProperties * >( layer->elevationProperties() )->profileLineSymbol()->clone() );
  mFillSymbol.reset( qgis::down_cast< QgsVectorLayerElevationProperties * >( layer->elevationProperties() )->profileFillSymbol()->clone() );
}

QString QgsVectorLayerProfileGenerator::sourceId() const
{
  return mId;
}

QgsVectorLayerProfileGenerator::~QgsVectorLayerProfileGenerator() = default;

bool QgsVectorLayerProfileGenerator::generateProfile( const QgsProfileGenerationContext & )
{
  if ( !mProfileCurve || mFeedback->isCanceled() )
    return false;

  // we need to transform the profile curve to the vector's CRS
  mTransformedCurve.reset( mProfileCurve->clone() );
  mLayerToTargetTransform = QgsCoordinateTransform( mSourceCrs, mTargetCrs, mTransformContext );
  if ( mTerrainProvider )
    mTargetToTerrainProviderTransform = QgsCoordinateTransform( mTargetCrs, mTerrainProvider->crs(), mTransformContext );

  try
  {
    mTransformedCurve->transform( mLayerToTargetTransform, Qgis::TransformDirection::Reverse );
  }
  catch ( QgsCsException & )
  {
    QgsDebugError( QStringLiteral( "Error transforming profile line to vector CRS" ) );
    return false;
  }

  const QgsRectangle profileCurveBoundingBox = mTransformedCurve->boundingBox();
  if ( !profileCurveBoundingBox.intersects( mExtent ) )
    return false;

  if ( mFeedback->isCanceled() )
    return false;

  mResults = std::make_unique< QgsVectorLayerProfileResults >();
  mResults->mLayer = mLayer;
  mResults->copyPropertiesFromGenerator( this );

  mProfileCurveEngine.reset( new QgsGeos( mProfileCurve.get() ) );
  mProfileCurveEngine->prepareGeometry();

  if ( mTolerance == 0.0 ) // geos does not handle very well buffer with 0 size
  {
    mProfileBufferedCurve = std::unique_ptr<QgsAbstractGeometry>( mProfileCurve->clone() );
  }
  else
  {
    mProfileBufferedCurve = std::unique_ptr<QgsAbstractGeometry>( mProfileCurveEngine->buffer( mTolerance, 8, Qgis::EndCapStyle::Flat, Qgis::JoinStyle::Round, 2 ) );
  }

  mProfileBufferedCurveEngine.reset( new QgsGeos( mProfileBufferedCurve.get() ) );
  mProfileBufferedCurveEngine->prepareGeometry();

  mDataDefinedProperties.prepare( mExpressionContext );

  if ( mFeedback->isCanceled() )
    return false;

  switch ( QgsWkbTypes::geometryType( mWkbType ) )
  {
    case Qgis::GeometryType::Point:
      if ( !generateProfileForPoints() )
        return false;
      break;

    case Qgis::GeometryType::Line:
      if ( !generateProfileForLines() )
        return false;
      break;

    case Qgis::GeometryType::Polygon:
      if ( !generateProfileForPolygons() )
        return false;
      break;

    case Qgis::GeometryType::Unknown:
    case Qgis::GeometryType::Null:
      return false;
  }

  return true;
}

QgsAbstractProfileResults *QgsVectorLayerProfileGenerator::takeResults()
{
  return mResults.release();
}

QgsFeedback *QgsVectorLayerProfileGenerator::feedback() const
{
  return mFeedback.get();
}

bool QgsVectorLayerProfileGenerator::generateProfileForPoints()
{
  // get features from layer
  QgsFeatureRequest request;
  request.setDestinationCrs( mTargetCrs, mTransformContext );
  request.setDistanceWithin( QgsGeometry( mProfileCurve->clone() ), mTolerance );
  request.setSubsetOfAttributes( mDataDefinedProperties.referencedFields( mExpressionContext ), mFields );
  request.setFeedback( mFeedback.get() );

  // our feature request is using the optimised distance within check (allowing use of spatial index)
  // BUT this will also include points which are within the tolerance distance before/after the end of line.
  // So we also need to double check that they fall within the flat buffered curve too.

  QgsFeature feature;
  QgsFeatureIterator it = mSource->getFeatures( request );
  while ( !mFeedback->isCanceled() && it.nextFeature( feature ) )
  {
    mExpressionContext.setFeature( feature );

    const QgsGeometry g = feature.geometry();
    for ( auto it = g.const_parts_begin(); !mFeedback->isCanceled() && it != g.const_parts_end(); ++it )
    {
      if ( mProfileBufferedCurveEngine->intersects( *it ) )
      {
        processIntersectionPoint( qgsgeometry_cast< const QgsPoint * >( *it ), feature );
      }
    }
  }
  return !mFeedback->isCanceled();
}

void QgsVectorLayerProfileGenerator::processIntersectionPoint( const QgsPoint *point, const QgsFeature &feature )
{
  QString error;
  const double offset = mDataDefinedProperties.valueAsDouble( QgsMapLayerElevationProperties::Property::ZOffset, mExpressionContext, mOffset );

  const double height = featureZToHeight( point->x(), point->y(), point->z(), offset );
  mResults->mRawPoints.append( QgsPoint( point->x(), point->y(), height ) );
  mResults->minZ = std::min( mResults->minZ, height );
  mResults->maxZ = std::max( mResults->maxZ, height );

  const double distanceAlongProfileCurve = mProfileCurveEngine->lineLocatePoint( *point, &error );
  mResults->mDistanceToHeightMap.insert( distanceAlongProfileCurve, height );

  QgsVectorLayerProfileResults::Feature resultFeature;
  resultFeature.featureId = feature.id();
  if ( mExtrusionEnabled )
  {
    const double extrusion = mDataDefinedProperties.valueAsDouble( QgsMapLayerElevationProperties::Property::ExtrusionHeight, mExpressionContext, mExtrusionHeight );

    resultFeature.geometry = QgsGeometry( new QgsLineString( QgsPoint( point->x(), point->y(), height ),
                                          QgsPoint( point->x(), point->y(), height + extrusion ) ) );
    resultFeature.crossSectionGeometry = QgsGeometry( new QgsLineString( QgsPoint( distanceAlongProfileCurve, height ),
                                         QgsPoint( distanceAlongProfileCurve, height + extrusion ) ) );
    mResults->minZ = std::min( mResults->minZ, height + extrusion );
    mResults->maxZ = std::max( mResults->maxZ, height + extrusion );
  }
  else
  {
    resultFeature.geometry = QgsGeometry( new QgsPoint( point->x(), point->y(), height ) );
    resultFeature.crossSectionGeometry = QgsGeometry( new QgsPoint( distanceAlongProfileCurve, height ) );
  }

  mResults->features[resultFeature.featureId].append( resultFeature );
}

void QgsVectorLayerProfileGenerator::processIntersectionCurve( const QgsLineString *intersectionCurve, const QgsFeature &feature )
{
  QString error;

  QgsVectorLayerProfileResults::Feature resultFeature;
  resultFeature.featureId = feature.id();
  double maxDistanceAlongProfileCurve = std::numeric_limits<double>::lowest();

  const double offset = mDataDefinedProperties.valueAsDouble( QgsMapLayerElevationProperties::Property::ZOffset, mExpressionContext, mOffset );
  const double extrusion = mDataDefinedProperties.valueAsDouble( QgsMapLayerElevationProperties::Property::ExtrusionHeight, mExpressionContext, mExtrusionHeight );

  const int numPoints = intersectionCurve->numPoints();
  QVector< double > newX( numPoints );
  QVector< double > newY( numPoints );
  QVector< double > newZ( numPoints );
  QVector< double > newDistance( numPoints );

  const double *inX = intersectionCurve->xData();
  const double *inY = intersectionCurve->yData();
  const double *inZ = intersectionCurve->is3D() ? intersectionCurve->zData() : nullptr;
  double *outX = newX.data();
  double *outY = newY.data();
  double *outZ = newZ.data();
  double *outDistance = newDistance.data();

  QVector< double > extrudedZ;
  double *extZOut = nullptr;
  if ( mExtrusionEnabled )
  {
    extrudedZ.resize( numPoints );
    extZOut = extrudedZ.data();
  }

  for ( int i = 0 ; ! mFeedback->isCanceled() && i < numPoints; ++i )
  {
    QgsPoint intersectionPoint( *inX, *inY, ( inZ ? *inZ : std::numeric_limits<double>::quiet_NaN() ) );

    const double height = featureZToHeight( intersectionPoint.x(), intersectionPoint.y(), intersectionPoint.z(), offset );
    const double distanceAlongProfileCurve = mProfileCurveEngine->lineLocatePoint( intersectionPoint, &error );

    maxDistanceAlongProfileCurve = std::max( maxDistanceAlongProfileCurve, distanceAlongProfileCurve );

    mResults->mRawPoints.append( QgsPoint( intersectionPoint.x(), intersectionPoint.y(), height ) );
    mResults->minZ = std::min( mResults->minZ, height );
    mResults->maxZ = std::max( mResults->maxZ, height );

    mResults->mDistanceToHeightMap.insert( distanceAlongProfileCurve, height );
    *outDistance++ = distanceAlongProfileCurve;

    *outX++ = intersectionPoint.x();
    *outY++ = intersectionPoint.y();
    *outZ++ = height;
    if ( extZOut )
      *extZOut++ = height + extrusion;

    if ( mExtrusionEnabled )
    {
      mResults->minZ = std::min( mResults->minZ, height + extrusion );
      mResults->maxZ = std::max( mResults->maxZ, height + extrusion );
    }
    inX++;
    inY++;
    if ( inZ )
      inZ++;
  }

  mResults->mDistanceToHeightMap.insert( maxDistanceAlongProfileCurve + 0.000001, std::numeric_limits<double>::quiet_NaN() );

  if ( mFeedback->isCanceled() )
    return;

  // create geometries from vector data
  if ( mExtrusionEnabled )
  {
    std::unique_ptr< QgsLineString > ring = std::make_unique< QgsLineString >( newX, newY, newZ );
    std::unique_ptr< QgsLineString > extrudedRing = std::make_unique< QgsLineString >( newX, newY, extrudedZ );
    std::unique_ptr< QgsLineString > reversedExtrusion( extrudedRing->reversed() );
    ring->append( reversedExtrusion.get() );
    ring->close();
    resultFeature.geometry = QgsGeometry( new QgsPolygon( ring.release() ) );

    std::unique_ptr< QgsLineString > distanceVHeightRing = std::make_unique< QgsLineString >( newDistance, newZ );
    std::unique_ptr< QgsLineString > extrudedDistanceVHeightRing = std::make_unique< QgsLineString >( newDistance, extrudedZ );
    std::unique_ptr< QgsLineString > reversedDistanceVHeightExtrusion( extrudedDistanceVHeightRing->reversed() );
    distanceVHeightRing->append( reversedDistanceVHeightExtrusion.get() );
    distanceVHeightRing->close();
    resultFeature.crossSectionGeometry = QgsGeometry( new QgsPolygon( distanceVHeightRing.release() ) );
  }
  else
  {
    resultFeature.geometry = QgsGeometry( new QgsLineString( newX, newY, newZ ) ) ;
    resultFeature.crossSectionGeometry = QgsGeometry( new QgsLineString( newDistance, newZ ) );
  }

  mResults->features[resultFeature.featureId].append( resultFeature );
}

bool QgsVectorLayerProfileGenerator::generateProfileForLines()
{
  // get features from layer
  QgsFeatureRequest request;
  request.setDestinationCrs( mTargetCrs, mTransformContext );
  if ( mTolerance > 0 )
  {
    request.setDistanceWithin( QgsGeometry( mProfileCurve->clone() ), mTolerance );
  }
  else
  {
    request.setFilterRect( mProfileCurve->boundingBox() );
  }
  request.setSubsetOfAttributes( mDataDefinedProperties.referencedFields( mExpressionContext ), mFields );
  request.setFeedback( mFeedback.get() );

  auto processCurve = [this]( const QgsFeature & feature, const QgsCurve * featGeomPart )
  {
    QString error;
    std::unique_ptr< QgsAbstractGeometry > intersection( mProfileBufferedCurveEngine->intersection( featGeomPart, &error ) );
    if ( !intersection )
      return;

    if ( mFeedback->isCanceled() )
      return;


    // Intersection is empty : GEOS issue for vertical intersection : use feature geometry as intersection
    if ( intersection->isEmpty() )
    {
      intersection.reset( featGeomPart->clone() );
    }

    QgsGeos featGeomPartGeos( featGeomPart );
    featGeomPartGeos.prepareGeometry();

    for ( auto it = intersection->const_parts_begin();
          !mFeedback->isCanceled() && it != intersection->const_parts_end();
          ++it )
    {
      if ( const QgsPoint *intersectionPoint = qgsgeometry_cast< const QgsPoint * >( *it ) )
      {
        // unfortunately we need to do some work to interpolate the z value for the line -- GEOS doesn't give us this
        QString error;
        const double distance = featGeomPartGeos.lineLocatePoint( *intersectionPoint, &error );
        std::unique_ptr< QgsPoint > interpolatedPoint( featGeomPart->interpolatePoint( distance ) );

        processIntersectionPoint( interpolatedPoint.get(), feature );
      }
      else if ( const QgsLineString *intersectionCurve = qgsgeometry_cast< const QgsLineString * >( *it ) )
      {
        processIntersectionCurve( intersectionCurve, feature );
      }
    }
  };

  QgsFeature feature;
  QgsFeatureIterator it = mSource->getFeatures( request );
  while ( !mFeedback->isCanceled() && it.nextFeature( feature ) )
  {
    mExpressionContext.setFeature( feature );

    const QgsGeometry g = feature.geometry();
    for ( auto it = g.const_parts_begin(); !mFeedback->isCanceled() && it != g.const_parts_end(); ++it )
    {
      if ( mProfileBufferedCurveEngine->intersects( *it ) )
      {
        processCurve( feature, qgsgeometry_cast< const QgsCurve * >( *it ) );
      }
    }
  }

  return !mFeedback->isCanceled();
}

QgsPoint QgsVectorLayerProfileGenerator::interpolatePointOnTriangle( const QgsPolygon *triangle, double x, double y ) const
{
  QgsPoint p1, p2, p3;
  Qgis::VertexType vt;
  triangle->exteriorRing()->pointAt( 0, p1, vt );
  triangle->exteriorRing()->pointAt( 1, p2, vt );
  triangle->exteriorRing()->pointAt( 2, p3, vt );
  const double z = QgsMeshLayerUtils::interpolateFromVerticesData( p1, p2, p3, p1.z(), p2.z(), p3.z(), QgsPointXY( x, y ) );
  return QgsPoint( x, y, z );
};

void QgsVectorLayerProfileGenerator::processTriangleIntersectForPoint( const QgsPolygon *triangle, const QgsPoint *p, QVector< QgsGeometry > &transformedParts, QVector< QgsGeometry > &crossSectionParts )
{
  const QgsPoint interpolatedPoint = interpolatePointOnTriangle( triangle, p->x(), p->y() );
  mResults->mRawPoints.append( interpolatedPoint );
  mResults->minZ = std::min( mResults->minZ, interpolatedPoint.z() );
  mResults->maxZ = std::max( mResults->maxZ, interpolatedPoint.z() );

  QString lastError;
  const double distance = mProfileCurveEngine->lineLocatePoint( *p, &lastError );
  mResults->mDistanceToHeightMap.insert( distance, interpolatedPoint.z() );

  if ( mExtrusionEnabled )
  {
    const double extrusion = mDataDefinedProperties.valueAsDouble( QgsMapLayerElevationProperties::Property::ExtrusionHeight, mExpressionContext, mExtrusionHeight );

    transformedParts.append( QgsGeometry( new QgsLineString( interpolatedPoint,
                                          QgsPoint( interpolatedPoint.x(), interpolatedPoint.y(), interpolatedPoint.z() + extrusion ) ) ) );
    crossSectionParts.append( QgsGeometry( new QgsLineString( QgsPoint( distance, interpolatedPoint.z() ),
                                           QgsPoint( distance, interpolatedPoint.z() + extrusion ) ) ) );
    mResults->minZ = std::min( mResults->minZ, interpolatedPoint.z() + extrusion );
    mResults->maxZ = std::max( mResults->maxZ, interpolatedPoint.z() + extrusion );
  }
  else
  {
    transformedParts.append( QgsGeometry( new QgsPoint( interpolatedPoint ) ) );
    crossSectionParts.append( QgsGeometry( new QgsPoint( distance, interpolatedPoint.z() ) ) );
  }
}

void QgsVectorLayerProfileGenerator::processTriangleIntersectForLine( const QgsPolygon *triangle, const QgsLineString *intersectionLine, QVector< QgsGeometry > &transformedParts, QVector< QgsGeometry > &crossSectionParts )
{
  if ( triangle->exteriorRing()->numPoints() < 4 ) // not a polygon
    return;

  int numPoints = intersectionLine->numPoints();
  QVector< double > newX( numPoints );
  QVector< double > newY( numPoints );
  QVector< double > newZ( numPoints );
  QVector< double > newDistance( numPoints );

  const double *inX = intersectionLine->xData();
  const double *inY = intersectionLine->yData();
  const double *inZ = intersectionLine->is3D() ? intersectionLine->zData() : nullptr;
  double *outX = newX.data();
  double *outY = newY.data();
  double *outZ = newZ.data();
  double *outDistance = newDistance.data();

  double lastDistanceAlongProfileCurve = 0.0;
  QVector< double > extrudedZ;
  double *extZOut = nullptr;
  double extrusion = 0;

  if ( mExtrusionEnabled )
  {
    extrudedZ.resize( numPoints );
    extZOut = extrudedZ.data();

    extrusion = mDataDefinedProperties.valueAsDouble( QgsMapLayerElevationProperties::Property::ExtrusionHeight, mExpressionContext, mExtrusionHeight );
  }

  QString lastError;
  for ( int i = 0 ; ! mFeedback->isCanceled() && i < numPoints; ++i )
  {
    double x = *inX++;
    double y = *inY++;
    double z = inZ ? *inZ++ : 0;

    QgsPoint interpolatedPoint( x, y, z ); // general case (not a triangle)

    *outX++ = x;
    *outY++ = y;
    if ( triangle->exteriorRing()->numPoints() == 4 ) // triangle case
    {
      interpolatedPoint = interpolatePointOnTriangle( triangle, x, y );
    }
    double tempOutZ = std::isnan( interpolatedPoint.z() ) ? 0.0 : interpolatedPoint.z();
    *outZ++ = tempOutZ;

    if ( mExtrusionEnabled )
      *extZOut++ = tempOutZ + extrusion;

    mResults->mRawPoints.append( interpolatedPoint );
    mResults->minZ = std::min( mResults->minZ, interpolatedPoint.z() );
    mResults->maxZ = std::max( mResults->maxZ, interpolatedPoint.z() );
    if ( mExtrusionEnabled )
    {
      mResults->minZ = std::min( mResults->minZ, interpolatedPoint.z() + extrusion );
      mResults->maxZ = std::max( mResults->maxZ, interpolatedPoint.z() + extrusion );
    }

    const double distance = mProfileCurveEngine->lineLocatePoint( interpolatedPoint, &lastError );
    *outDistance++ = distance;

    mResults->mDistanceToHeightMap.insert( distance, interpolatedPoint.z() );
    lastDistanceAlongProfileCurve = distance;
  }

  // insert nan point to end the line
  mResults->mDistanceToHeightMap.insert( lastDistanceAlongProfileCurve + 0.000001, std::numeric_limits<double>::quiet_NaN() );

  if ( mFeedback->isCanceled() )
    return;

  if ( mExtrusionEnabled )
  {
    std::unique_ptr< QgsLineString > ring = std::make_unique< QgsLineString >( newX, newY, newZ );
    std::unique_ptr< QgsLineString > extrudedRing = std::make_unique< QgsLineString >( newX, newY, extrudedZ );
    std::unique_ptr< QgsLineString > reversedExtrusion( extrudedRing->reversed() );
    ring->append( reversedExtrusion.get() );
    ring->close();
    transformedParts.append( QgsGeometry( new QgsPolygon( ring.release() ) ) );

    std::unique_ptr< QgsLineString > distanceVHeightRing = std::make_unique< QgsLineString >( newDistance, newZ );
    std::unique_ptr< QgsLineString > extrudedDistanceVHeightRing = std::make_unique< QgsLineString >( newDistance, extrudedZ );
    std::unique_ptr< QgsLineString > reversedDistanceVHeightExtrusion( extrudedDistanceVHeightRing->reversed() );
    distanceVHeightRing->append( reversedDistanceVHeightExtrusion.get() );
    distanceVHeightRing->close();
    crossSectionParts.append( QgsGeometry( new QgsPolygon( distanceVHeightRing.release() ) ) );
  }
  else
  {
    transformedParts.append( QgsGeometry( new QgsLineString( newX, newY, newZ ) ) );
    crossSectionParts.append( QgsGeometry( new QgsLineString( newDistance, newZ ) ) );
  }
};

void QgsVectorLayerProfileGenerator::processTriangleIntersectForPolygon( const QgsPolygon *sourcePolygon, const QgsPolygon *intersectionPolygon, QVector< QgsGeometry > &transformedParts, QVector< QgsGeometry > &crossSectionParts )
{
  bool oldExtrusion = mExtrusionEnabled;

  /* Polyone extrusion produces I or C or inverted C shapes because the starting and ending points are the same.
     We observe the same case with linestrings if the starting and ending points are not at the ends.
     In the case below, the Z polygon projected onto the curve produces a shape that cannot be used to represent the extrusion ==> we would obtain a 3D volume.
     In order to avoid having strange shapes that cannot be understood by the end user, extrusion is deactivated in the case of polygons.

                     .^..
                   ./ |  \..
                ../   |     \...
             ../      |         \...
          ../         |             \..      ....^..
       ../            |        ........\.../        \...                ^
    ../         ......|......./           \...          \....       .../ \
   /,........../      |                       \..            \... /       \
  v                   |                          \...    ..../   \...      \
                      |                              \ ./            \...   \
                      |                               v                  \.. \
                      |                                                     `v
                      |
                     .^..
                   ./    \..
                ../         \...
             ../                \...
          ../                       \..      ....^..
       ../                     ........\.../        \...                ^
    ../         ............../           \...          \....       .../ \
   /,........../                              \..            \... /       \
  v                                              \...    ..../   \...      \
                                                     \ ./            \...   \
                                                      v                  \.. \
                                                                            `v
   */
  mExtrusionEnabled = false;
  if ( mProfileBufferedCurveEngine->contains( sourcePolygon ) ) // sourcePolygon is entirely inside curve buffer, we keep it as whole
  {
    if ( const QgsCurve *exterior = sourcePolygon->exteriorRing() )
    {
      QgsLineString *exteriorLine = qgsgeometry_cast<QgsLineString *>( exterior );
      processTriangleIntersectForLine( sourcePolygon, exteriorLine, transformedParts, crossSectionParts );
    }
    for ( int i = 0; i < sourcePolygon->numInteriorRings(); ++i )
    {
      QgsLineString *interiorLine = qgsgeometry_cast<QgsLineString *>( sourcePolygon->interiorRing( i ) );
      processTriangleIntersectForLine( sourcePolygon, interiorLine, transformedParts, crossSectionParts );
    }
  }
  else // sourcePolygon is partially inside curve buffer, the intersectionPolygon is closed due to the intersection operation then
    // it must be 'reopened'
  {
    if ( const QgsCurve *exterior = intersectionPolygon->exteriorRing() )
    {
      QgsLineString *exteriorLine = qgsgeometry_cast<QgsLineString *>( exterior )->clone();
      exteriorLine->deleteVertex( QgsVertexId( 0, 0, exteriorLine->numPoints() - 1 ) ); // open linestring
      processTriangleIntersectForLine( sourcePolygon, exteriorLine, transformedParts, crossSectionParts );
      delete exteriorLine;
    }
    for ( int i = 0; i < intersectionPolygon->numInteriorRings(); ++i )
    {
      QgsLineString *interiorLine = qgsgeometry_cast<QgsLineString *>( intersectionPolygon->interiorRing( i ) );
      if ( mProfileBufferedCurveEngine->contains( interiorLine ) ) // interiorLine is entirely inside curve buffer
      {
        processTriangleIntersectForLine( sourcePolygon, interiorLine, transformedParts, crossSectionParts );
      }
      else
      {
        interiorLine = qgsgeometry_cast<QgsLineString *>( intersectionPolygon->interiorRing( i ) )->clone();
        interiorLine->deleteVertex( QgsVertexId( 0, 0, interiorLine->numPoints() - 1 ) ); // open linestring
        processTriangleIntersectForLine( sourcePolygon, interiorLine, transformedParts, crossSectionParts );
        delete interiorLine;
      }
    }
  }

  mExtrusionEnabled = oldExtrusion;
};

bool QgsVectorLayerProfileGenerator::generateProfileForPolygons()
{
  // get features from layer
  QgsFeatureRequest request;
  request.setDestinationCrs( mTargetCrs, mTransformContext );
  if ( mTolerance > 0 )
  {
    request.setDistanceWithin( QgsGeometry( mProfileCurve->clone() ), mTolerance );
  }
  else
  {
    request.setFilterRect( mProfileCurve->boundingBox() );
  }
  request.setSubsetOfAttributes( mDataDefinedProperties.referencedFields( mExpressionContext ), mFields );
  request.setFeedback( mFeedback.get() );

  std::function< void( const QgsPolygon *triangle, const QgsAbstractGeometry *intersect, QVector< QgsGeometry > &, QVector< QgsGeometry > & ) > processTriangleLineIntersect;
  processTriangleLineIntersect = [this]( const QgsPolygon * triangle, const QgsAbstractGeometry * intersection, QVector< QgsGeometry > &transformedParts, QVector< QgsGeometry > &crossSectionParts )
  {
    for ( auto it = intersection->const_parts_begin();
          ! mFeedback->isCanceled() && it != intersection->const_parts_end();
          ++it )
    {
      // intersect may be a (multi)point or (multi)linestring
      switch ( QgsWkbTypes::geometryType( ( *it )->wkbType() ) )
      {
        case Qgis::GeometryType::Point:
          if ( const QgsPoint *p = qgsgeometry_cast< const QgsPoint * >( *it ) )
          {
            processTriangleIntersectForPoint( triangle, p, transformedParts, crossSectionParts );
          }
          break;

        case Qgis::GeometryType::Line:
          if ( const QgsLineString *intersectionLine = qgsgeometry_cast< const QgsLineString * >( *it ) )
          {
            processTriangleIntersectForLine( triangle, intersectionLine, transformedParts, crossSectionParts );
          }
          break;

        case Qgis::GeometryType::Polygon:
          if ( const QgsPolygon *poly = qgsgeometry_cast< const QgsPolygon * >( *it ) )
          {
            processTriangleIntersectForPolygon( triangle, poly, transformedParts, crossSectionParts );
          }
          break;

        case Qgis::GeometryType::Unknown:
        case Qgis::GeometryType::Null:
          return;
      }
    }
  };

  auto triangleIsCollinearInXYPlane = []( const QgsPolygon * polygon )-> bool
  {
    const QgsLineString *ring = qgsgeometry_cast< const QgsLineString * >( polygon->exteriorRing() );
    return QgsGeometryUtilsBase::pointsAreCollinear( ring->xAt( 0 ), ring->yAt( 0 ),
        ring->xAt( 1 ), ring->yAt( 1 ),
        ring->xAt( 2 ), ring->yAt( 2 ), 0.005 );
  };

  auto processPolygon = [this, &processTriangleLineIntersect, &triangleIsCollinearInXYPlane]( const QgsCurvePolygon * polygon, QVector< QgsGeometry > &transformedParts, QVector< QgsGeometry > &crossSectionParts, double offset, bool & wasCollinear )
  {
    std::unique_ptr< QgsPolygon > clampedPolygon;
    if ( const QgsPolygon *p = qgsgeometry_cast< const QgsPolygon * >( polygon ) )
    {
      clampedPolygon.reset( p->clone() );
    }
    else
    {
      clampedPolygon.reset( qgsgeometry_cast< QgsPolygon * >( polygon->segmentize() ) );
    }
    clampAltitudes( clampedPolygon.get(), offset );

    if ( mFeedback->isCanceled() )
      return;

    if ( mTolerance > 0.0 ) // if the tolerance is not 0.0 we will have a polygon / polygon intersection, we do not need tessellation
    {
      QString error;
      if ( mProfileBufferedCurveEngine->intersects( clampedPolygon.get(), &error ) )
      {
        std::unique_ptr< QgsAbstractGeometry > intersection;
        intersection.reset( mProfileBufferedCurveEngine->intersection( clampedPolygon.get(), &error ) );
        if ( error.isEmpty() )
        {
          processTriangleLineIntersect( clampedPolygon.get(), intersection.get(), transformedParts, crossSectionParts );
        }
        else
        {
          // this case may occur with vertical object as geos does not handle very well 3D data.
          // Geos works in 2D from the 3D coordinates then re-add the Z values, but when 2D-from-3D objects are vertical, they are topologically incorrects!
          // This piece of code is just a fix to handle this case, a better and real 3D capable library is needed (like SFCGAL).
          QgsLineString *ring = qgsgeometry_cast< QgsLineString * >( clampedPolygon->exteriorRing() );
          int numPoints = ring->numPoints();
          QVector< double > newX( numPoints );
          QVector< double > newY( numPoints );
          QVector< double > newZ( numPoints );
          double *outX = newX.data();
          double *outY = newY.data();
          double *outZ = newZ.data();

          const double *inX = ring->xData();
          const double *inY = ring->yData();
          const double *inZ = ring->zData();
          for ( int i = 0 ; ! mFeedback->isCanceled() && i < ring->numPoints() - 1; ++i )
          {
            *outX++ = inX[i] + i * 1.0e-9;
            *outY++ = inY[i] + i * 1.0e-9;
            *outZ++ = inZ[i];
          }
          std::unique_ptr< QgsPolygon > shiftedPoly;
          shiftedPoly.reset( new QgsPolygon( new QgsLineString( newX, newY, newZ ) ) );

          intersection.reset( mProfileBufferedCurveEngine->intersection( shiftedPoly.get(), &error ) );
          if ( intersection.get() )
            processTriangleLineIntersect( clampedPolygon.get(), intersection.get(), transformedParts, crossSectionParts );
          else
            QgsDebugMsgLevel( QStringLiteral( "processPolygon after shift bad geom! error: %1" ).arg( error ), 0 );
        }
      }

    }
    else // ie. polygon / line intersection ==> need tessellation
    {
      QgsGeometry tessellation;
      if ( clampedPolygon->numInteriorRings() == 0 && clampedPolygon->exteriorRing() && clampedPolygon->exteriorRing()->numPoints() == 4 && clampedPolygon->exteriorRing()->isClosed() )
      {
        // special case -- polygon is already a triangle, so no need to tessellate
        std::unique_ptr< QgsMultiPolygon > multiPolygon = std::make_unique< QgsMultiPolygon >();
        multiPolygon->addGeometry( clampedPolygon.release() );
        tessellation = QgsGeometry( std::move( multiPolygon ) );
      }
      else
      {
        const QgsRectangle bounds = clampedPolygon->boundingBox();
        QgsTessellator t( bounds, false, false, false, false );
        t.addPolygon( *clampedPolygon, 0 );

        tessellation = QgsGeometry( t.asMultiPolygon() );
        if ( mFeedback->isCanceled() )
          return;

        tessellation.translate( bounds.xMinimum(), bounds.yMinimum() );
      }

      // iterate through the tessellation, finding triangles which intersect the line
      const int numTriangles = qgsgeometry_cast< const QgsMultiPolygon * >( tessellation.constGet() )->numGeometries();
      for ( int i = 0; ! mFeedback->isCanceled() && i < numTriangles; ++i )
      {
        const QgsPolygon *triangle = qgsgeometry_cast< const QgsPolygon * >( qgsgeometry_cast< const QgsMultiPolygon * >( tessellation.constGet() )->geometryN( i ) );

        if ( triangleIsCollinearInXYPlane( triangle ) )
        {
          wasCollinear = true;
          const QgsLineString *ring = qgsgeometry_cast< const QgsLineString * >( polygon->exteriorRing() );

          QString lastError;
          if ( const QgsLineString *ls = qgsgeometry_cast< const QgsLineString * >( mProfileCurve.get() ) )
          {
            for ( int curveSegmentIndex = 0; curveSegmentIndex < mProfileCurve->numPoints() - 1; ++curveSegmentIndex )
            {
              const QgsPoint p1 = ls->pointN( curveSegmentIndex );
              const QgsPoint p2 = ls->pointN( curveSegmentIndex + 1 );

              QgsPoint intersectionPoint;
              double minZ = std::numeric_limits< double >::max();
              double maxZ = std::numeric_limits< double >::lowest();

              for ( auto vertexPair : std::array<std::pair<int, int>, 3> {{ { 0, 1}, {1, 2}, {2, 0} }} )
              {
                bool isIntersection = false;
                if ( QgsGeometryUtils::segmentIntersection( ring->pointN( vertexPair.first ), ring->pointN( vertexPair.second ), p1, p2, intersectionPoint, isIntersection ) )
                {
                  const double fraction = QgsGeometryUtilsBase::pointFractionAlongLine( ring->xAt( vertexPair.first ), ring->yAt( vertexPair.first ), ring->xAt( vertexPair.second ), ring->yAt( vertexPair.second ), intersectionPoint.x(), intersectionPoint.y() );
                  const double intersectionZ = ring->zAt( vertexPair.first ) + ( ring->zAt( vertexPair.second ) - ring->zAt( vertexPair.first ) ) * fraction;
                  minZ = std::min( minZ, intersectionZ );
                  maxZ = std::max( maxZ, intersectionZ );
                }
              }

              if ( !intersectionPoint.isEmpty() )
              {
                // need z?
                mResults->mRawPoints.append( intersectionPoint );
                mResults->minZ = std::min( mResults->minZ, minZ );
                mResults->maxZ = std::max( mResults->maxZ, maxZ );

                const double distance = mProfileCurveEngine->lineLocatePoint( intersectionPoint, &lastError );

                crossSectionParts.append( QgsGeometry( new QgsLineString( QVector< double > {distance, distance}, QVector< double > {minZ, maxZ} ) ) );

                mResults->mDistanceToHeightMap.insert( distance, minZ );
                mResults->mDistanceToHeightMap.insert( distance, maxZ );
              }
            }
          }
          else
          {
            // curved geometries, not supported yet, but not possible through the GUI anyway
            QgsDebugError( QStringLiteral( "Collinear triangles with curved profile lines are not supported yet" ) );
          }
        }
        else // not collinear
        {
          QString error;
          if ( mProfileBufferedCurveEngine->intersects( triangle, &error ) )
          {
            std::unique_ptr< QgsAbstractGeometry > intersection( mProfileBufferedCurveEngine->intersection( triangle, &error ) );
            processTriangleLineIntersect( triangle, intersection.get(), transformedParts, crossSectionParts );
          }
        }
      }
    }
  };

  // ========= MAIN JOB
  QgsFeature feature;
  QgsFeatureIterator it = mSource->getFeatures( request );
  while ( ! mFeedback->isCanceled() && it.nextFeature( feature ) )
  {
    if ( !mProfileBufferedCurveEngine->intersects( feature.geometry().constGet() ) )
      continue;

    mExpressionContext.setFeature( feature );

    const double offset = mDataDefinedProperties.valueAsDouble( QgsMapLayerElevationProperties::Property::ZOffset, mExpressionContext, mOffset );
    const QgsGeometry g = feature.geometry();
    QVector< QgsGeometry > transformedParts;
    QVector< QgsGeometry > crossSectionParts;
    bool wasCollinear = false;

    // === process intersection of geometry feature parts with the mProfileBoxEngine
    for ( auto it = g.const_parts_begin(); ! mFeedback->isCanceled() && it != g.const_parts_end(); ++it )
    {
      if ( mProfileBufferedCurveEngine->intersects( *it ) )
      {
        processPolygon( qgsgeometry_cast< const QgsCurvePolygon * >( *it ), transformedParts, crossSectionParts, offset, wasCollinear );
      }
    }

    if ( mFeedback->isCanceled() )
      return false;

    // === aggregate results for this feature
    QgsVectorLayerProfileResults::Feature resultFeature;
    resultFeature.featureId = feature.id();
    resultFeature.geometry = transformedParts.size() > 1 ? QgsGeometry::collectGeometry( transformedParts ) : transformedParts.value( 0 );
    if ( !crossSectionParts.empty() )
    {
      if ( !wasCollinear )
      {
        QgsGeometry unioned = QgsGeometry::unaryUnion( crossSectionParts );
        if ( unioned.isEmpty() )
        {
          resultFeature.crossSectionGeometry = QgsGeometry::collectGeometry( crossSectionParts );
        }
        else
        {
          if ( unioned.type() == Qgis::GeometryType::Line )
          {
            unioned = unioned.mergeLines();
          }
          resultFeature.crossSectionGeometry = unioned;
        }
      }
      else
      {
        resultFeature.crossSectionGeometry = QgsGeometry::collectGeometry( crossSectionParts );
      }
    }
    mResults->features[resultFeature.featureId].append( resultFeature );
  }
  return true;
}

double QgsVectorLayerProfileGenerator::terrainHeight( double x, double y )
{
  if ( !mTerrainProvider )
    return std::numeric_limits<double>::quiet_NaN();

  // transform feature point to terrain provider crs
  try
  {
    double dummyZ = 0;
    mTargetToTerrainProviderTransform.transformInPlace( x, y, dummyZ );
  }
  catch ( QgsCsException & )
  {
    return std::numeric_limits<double>::quiet_NaN();
  }

  return mTerrainProvider->heightAt( x, y );
}

double QgsVectorLayerProfileGenerator::featureZToHeight( double x, double y, double z, double offset )
{
  switch ( mClamping )
  {
    case Qgis::AltitudeClamping::Absolute:
      break;

    case Qgis::AltitudeClamping::Relative:
    case Qgis::AltitudeClamping::Terrain:
    {
      const double terrainZ = terrainHeight( x, y );
      if ( !std::isnan( terrainZ ) )
      {
        switch ( mClamping )
        {
          case Qgis::AltitudeClamping::Relative:
            if ( std::isnan( z ) )
              z = terrainZ;
            else
              z += terrainZ;
            break;

          case Qgis::AltitudeClamping::Terrain:
            z = terrainZ;
            break;

          case Qgis::AltitudeClamping::Absolute:
            break;
        }
      }
      break;
    }
  }

  return ( std::isnan( z ) ? 0 : z ) * mScale + offset;
}

void QgsVectorLayerProfileGenerator::clampAltitudes( QgsLineString *lineString, const QgsPoint &centroid, double offset )
{
  for ( int i = 0; i < lineString->nCoordinates(); ++i )
  {
    if ( mFeedback->isCanceled() )
      break;

    double terrainZ = 0;
    switch ( mClamping )
    {
      case Qgis::AltitudeClamping::Relative:
      case Qgis::AltitudeClamping::Terrain:
      {
        QgsPointXY pt;
        switch ( mBinding )
        {
          case Qgis::AltitudeBinding::Vertex:
            pt.setX( lineString->xAt( i ) );
            pt.setY( lineString->yAt( i ) );
            break;

          case Qgis::AltitudeBinding::Centroid:
            pt.set( centroid.x(), centroid.y() );
            break;
        }

        terrainZ = terrainHeight( pt.x(), pt.y() );
        break;
      }

      case Qgis::AltitudeClamping::Absolute:
        break;
    }

    double geomZ = 0;

    switch ( mClamping )
    {
      case Qgis::AltitudeClamping::Absolute:
      case Qgis::AltitudeClamping::Relative:
        geomZ = lineString->zAt( i );
        break;

      case Qgis::AltitudeClamping::Terrain:
        break;
    }

    const double z = ( terrainZ + ( std::isnan( geomZ ) ?  0 : geomZ ) ) * mScale + offset;
    lineString->setZAt( i, z );
  }
}

bool QgsVectorLayerProfileGenerator::clampAltitudes( QgsPolygon *polygon, double offset )
{
  if ( !polygon->is3D() )
    polygon->addZValue( 0 );

  QgsPoint centroid;
  switch ( mBinding )
  {
    case Qgis::AltitudeBinding::Vertex:
      break;

    case Qgis::AltitudeBinding::Centroid:
      centroid = polygon->centroid();
      break;
  }

  QgsCurve *curve = const_cast<QgsCurve *>( polygon->exteriorRing() );
  QgsLineString *lineString = qgsgeometry_cast<QgsLineString *>( curve );
  if ( !lineString )
    return false;

  clampAltitudes( lineString, centroid, offset );

  for ( int i = 0; i < polygon->numInteriorRings(); ++i )
  {
    if ( mFeedback->isCanceled() )
      break;

    QgsCurve *curve = const_cast<QgsCurve *>( polygon->interiorRing( i ) );
    QgsLineString *lineString = qgsgeometry_cast<QgsLineString *>( curve );
    if ( !lineString )
      return false;

    clampAltitudes( lineString, centroid, offset );
  }
  return true;
}
