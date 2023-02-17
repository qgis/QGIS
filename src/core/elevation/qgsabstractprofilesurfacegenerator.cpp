/***************************************************************************
                         qgsabstractprofilegenerator.cpp
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
#include "qgsabstractprofilesurfacegenerator.h"
#include "qgsprofilesnapping.h"
#include "qgsfillsymbol.h"
#include "qgslinesymbol.h"

#include <QPainterPath>
#include <optional>

//
// QgsAbstractProfileSurfaceResults
//

QgsAbstractProfileSurfaceResults::~QgsAbstractProfileSurfaceResults() = default;

QMap<double, double> QgsAbstractProfileSurfaceResults::distanceToHeightMap() const
{
  return mDistanceToHeightMap;
}

QgsPointSequence QgsAbstractProfileSurfaceResults::sampledPoints() const
{
  return mRawPoints;
}

QgsDoubleRange QgsAbstractProfileSurfaceResults::zRange() const
{
  return QgsDoubleRange( minZ, maxZ );
}

QVector<QgsGeometry> QgsAbstractProfileSurfaceResults::asGeometries() const
{
  QVector<QgsGeometry> res;
  res.reserve( mRawPoints.size() );
  for ( const QgsPoint &point : mRawPoints )
    res.append( QgsGeometry( point.clone() ) );

  return res;
}

QgsProfileSnapResult QgsAbstractProfileSurfaceResults::snapPoint( const QgsProfilePoint &point, const QgsProfileSnapContext &context )
{
  // TODO -- consider an index if performance is an issue
  QgsProfileSnapResult result;

  double prevDistance = std::numeric_limits< double >::max();
  double prevElevation = 0;
  for ( auto it = mDistanceToHeightMap.constBegin(); it != mDistanceToHeightMap.constEnd(); ++it )
  {
    // find segment which corresponds to the given distance along curve
    if ( it != mDistanceToHeightMap.constBegin() && prevDistance <= point.distance() && it.key() >= point.distance() )
    {
      const double dx = it.key() - prevDistance;
      const double dy = it.value() - prevElevation;
      const double snappedZ = ( dy / dx ) * ( point.distance() - prevDistance ) + prevElevation;

      if ( std::fabs( point.elevation() - snappedZ ) > context.maximumSurfaceElevationDelta )
        return QgsProfileSnapResult();

      result.snappedPoint = QgsProfilePoint( point.distance(), snappedZ );
      break;
    }

    prevDistance = it.key();
    prevElevation = it.value();
  }
  return result;
}

QVector<QgsProfileIdentifyResults> QgsAbstractProfileSurfaceResults::identify( const QgsProfilePoint &point, const QgsProfileIdentifyContext &context )
{
  // TODO -- consider an index if performance is an issue
  std::optional< QgsProfileIdentifyResults > result;

  double prevDistance = std::numeric_limits< double >::max();
  double prevElevation = 0;
  for ( auto it = mDistanceToHeightMap.constBegin(); it != mDistanceToHeightMap.constEnd(); ++it )
  {
    // find segment which corresponds to the given distance along curve
    if ( it != mDistanceToHeightMap.constBegin() && prevDistance <= point.distance() && it.key() >= point.distance() )
    {
      const double dx = it.key() - prevDistance;
      const double dy = it.value() - prevElevation;
      const double snappedZ = ( dy / dx ) * ( point.distance() - prevDistance ) + prevElevation;

      if ( std::fabs( point.elevation() - snappedZ ) > context.maximumSurfaceElevationDelta )
        return {};

      result = QgsProfileIdentifyResults( nullptr,
      {
        QVariantMap(
        {
          {QStringLiteral( "distance" ),  point.distance() },
          {QStringLiteral( "elevation" ), snappedZ }
        } )
      } );
      break;
    }

    prevDistance = it.key();
    prevElevation = it.value();
  }
  if ( result.has_value() )
    return {result.value()};
  else
    return {};
}

void QgsAbstractProfileSurfaceResults::renderResults( QgsProfileRenderContext &context )
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

  switch ( symbology )
  {
    case Qgis::ProfileSurfaceSymbology::Line:
      mLineSymbol->startRender( context.renderContext() );
      break;
    case Qgis::ProfileSurfaceSymbology::FillBelow:
      mFillSymbol->startRender( context.renderContext() );
      break;
  }

  QPolygonF currentLine;
  double prevDistance = std::numeric_limits< double >::quiet_NaN();
  double currentPartStartDistance = 0;
  for ( auto pointIt = mDistanceToHeightMap.constBegin(); pointIt != mDistanceToHeightMap.constEnd(); ++pointIt )
  {
    if ( std::isnan( prevDistance ) )
    {
      currentPartStartDistance = pointIt.key();
    }
    if ( std::isnan( pointIt.value() ) )
    {
      if ( currentLine.length() > 1 )
      {
        switch ( symbology )
        {
          case Qgis::ProfileSurfaceSymbology::Line:
            mLineSymbol->renderPolyline( currentLine, nullptr, context.renderContext() );
            break;
          case Qgis::ProfileSurfaceSymbology::FillBelow:
            currentLine.append( context.worldTransform().map( QPointF( prevDistance, minZ ) ) );
            currentLine.append( context.worldTransform().map( QPointF( currentPartStartDistance, minZ ) ) );
            currentLine.append( currentLine.at( 0 ) );
            mFillSymbol->renderPolygon( currentLine, nullptr, nullptr, context.renderContext() );
            break;
        }
      }
      prevDistance = pointIt.key();
      currentLine.clear();
      continue;
    }

    currentLine.append( context.worldTransform().map( QPointF( pointIt.key(), pointIt.value() ) ) );
    prevDistance = pointIt.key();
  }
  if ( currentLine.length() > 1 )
  {
    switch ( symbology )
    {
      case Qgis::ProfileSurfaceSymbology::Line:
        mLineSymbol->renderPolyline( currentLine, nullptr, context.renderContext() );
        break;
      case Qgis::ProfileSurfaceSymbology::FillBelow:
        currentLine.append( context.worldTransform().map( QPointF( prevDistance, minZ ) ) );
        currentLine.append( context.worldTransform().map( QPointF( currentPartStartDistance, minZ ) ) );
        currentLine.append( currentLine.at( 0 ) );
        mFillSymbol->renderPolygon( currentLine, nullptr, nullptr, context.renderContext() );
        break;
    }
  }

  switch ( symbology )
  {
    case Qgis::ProfileSurfaceSymbology::Line:
      mLineSymbol->stopRender( context.renderContext() );
      break;
    case Qgis::ProfileSurfaceSymbology::FillBelow:
      mFillSymbol->stopRender( context.renderContext() );
      break;
  }
}


void QgsAbstractProfileSurfaceResults::copyPropertiesFromGenerator( const QgsAbstractProfileGenerator *generator )
{
  const QgsAbstractProfileSurfaceGenerator *surfaceGenerator = qgis::down_cast<  const QgsAbstractProfileSurfaceGenerator * >( generator );

  mLineSymbol.reset( surfaceGenerator->lineSymbol()->clone() );
  mFillSymbol.reset( surfaceGenerator->fillSymbol()->clone() );
  symbology = surfaceGenerator->symbology();
}

//
// QgsAbstractProfileSurfaceGenerator
//

QgsAbstractProfileSurfaceGenerator::~QgsAbstractProfileSurfaceGenerator() = default;

Qgis::ProfileSurfaceSymbology QgsAbstractProfileSurfaceGenerator::symbology() const
{
  return mSymbology;
}

QgsLineSymbol *QgsAbstractProfileSurfaceGenerator::lineSymbol() const
{
  return mLineSymbol.get();
}

QgsFillSymbol *QgsAbstractProfileSurfaceGenerator::fillSymbol() const
{
  return mFillSymbol.get();
}
