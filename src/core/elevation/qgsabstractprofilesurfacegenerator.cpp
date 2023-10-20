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
#include "qgslinestring.h"
#include "qgsprofilerequest.h"

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

QVector<QgsAbstractProfileResults::Feature> QgsAbstractProfileSurfaceResults::asFeatures( Qgis::ProfileExportType type, QgsFeedback *feedback ) const
{
  QVector< QgsAbstractProfileResults::Feature > res;
  res.reserve( 1 );

  QVector< double > currentLineX;
  QVector< double > currentLineY;
  QVector< double > currentLineZ;

  switch ( type )
  {
    case Qgis::ProfileExportType::Features3D:
    {
      for ( auto pointIt = mDistanceToHeightMap.constBegin(); pointIt != mDistanceToHeightMap.constEnd(); ++pointIt )
      {
        if ( feedback && feedback->isCanceled() )
          break;


        if ( std::isnan( pointIt.value() ) )
        {
          if ( currentLineX.length() > 1 )
          {
            QgsAbstractProfileResults::Feature f;
            f.layerIdentifier = mId;
            f.geometry = QgsGeometry( std::make_unique< QgsLineString >( currentLineX, currentLineY, currentLineZ ) );
            res << f;
          }
          currentLineX.clear();
          currentLineY.clear();
          currentLineZ.clear();
          continue;
        }

        std::unique_ptr< QgsPoint > curvePoint( mProfileCurve->interpolatePoint( pointIt.key() ) );
        currentLineX << curvePoint->x();
        currentLineY << curvePoint->y();
        currentLineZ << pointIt.value();
      }

      if ( currentLineX.length() > 1 )
      {
        QgsAbstractProfileResults::Feature f;
        f.layerIdentifier = mId;
        f.geometry = QgsGeometry( std::make_unique< QgsLineString >( currentLineX, currentLineY, currentLineZ ) );
        res << f;
      }
      break;
    }

    case Qgis::ProfileExportType::Profile2D:
    {
      for ( auto pointIt = mDistanceToHeightMap.constBegin(); pointIt != mDistanceToHeightMap.constEnd(); ++pointIt )
      {
        if ( feedback && feedback->isCanceled() )
          break;

        if ( std::isnan( pointIt.value() ) )
        {
          if ( currentLineX.length() > 1 )
          {
            QgsAbstractProfileResults::Feature f;
            f.layerIdentifier = mId;
            f.geometry = QgsGeometry( std::make_unique< QgsLineString >( currentLineX, currentLineY ) );
            res << f;
          }
          currentLineX.clear();
          currentLineY.clear();
          continue;
        }

        currentLineX << pointIt.key();
        currentLineY << pointIt.value();
      }
      if ( currentLineX.length() > 1 )
      {
        QgsAbstractProfileResults::Feature f;
        f.layerIdentifier = mId;
        f.geometry = QgsGeometry( std::make_unique< QgsLineString >( currentLineX, currentLineY ) );
        res << f;
      }
      break;
    }

    case Qgis::ProfileExportType::DistanceVsElevationTable:
    {
      res.reserve( mDistanceToHeightMap.size() );
      for ( auto pointIt = mDistanceToHeightMap.constBegin(); pointIt != mDistanceToHeightMap.constEnd(); ++pointIt )
      {
        if ( feedback && feedback->isCanceled() )
          break;

        QgsAbstractProfileResults::Feature f;
        f.layerIdentifier = mId;
        f.attributes =
        {
          { QStringLiteral( "distance" ),  pointIt.key() },
          { QStringLiteral( "elevation" ),  pointIt.value() }
        };
        std::unique_ptr< QgsPoint>  point( mProfileCurve->interpolatePoint( pointIt.key() ) );
        if ( point->is3D() )
          point->setZ( pointIt.value() );
        else
          point->addZValue( pointIt.value() );
        f.geometry = QgsGeometry( std::move( point ) );
        res << f;
      }
      break;
    }
  }

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
  double minZ = context.elevationRange().lower();
  double maxZ = context.elevationRange().upper();

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
      if ( !std::isnan( mElevationLimit ) )
      {
        double dataLimit = std::numeric_limits< double >::max();
        for ( auto pointIt = mDistanceToHeightMap.constBegin(); pointIt != mDistanceToHeightMap.constEnd(); ++pointIt )
        {
          if ( !std::isnan( pointIt.value() ) )
          {
            dataLimit = std::min( pointIt.value(), dataLimit );
          }
        }
        if ( dataLimit > mElevationLimit )
          minZ = std::max( minZ, mElevationLimit );
      }
      break;
    case Qgis::ProfileSurfaceSymbology::FillAbove:
      mFillSymbol->startRender( context.renderContext() );
      if ( !std::isnan( mElevationLimit ) )
      {
        double dataLimit = std::numeric_limits< double >::lowest();
        for ( auto pointIt = mDistanceToHeightMap.constBegin(); pointIt != mDistanceToHeightMap.constEnd(); ++pointIt )
        {
          if ( !std::isnan( pointIt.value() ) )
          {
            dataLimit = std::max( pointIt.value(), dataLimit );
          }
        }
        if ( dataLimit < mElevationLimit )
          maxZ = std::min( maxZ, mElevationLimit );
      }
      break;
  }

  auto checkLine = [this]( QPolygonF & currentLine, QgsProfileRenderContext & context, double minZ, double maxZ,
                           double prevDistance, double currentPartStartDistance )
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
        case Qgis::ProfileSurfaceSymbology::FillAbove:
          currentLine.append( context.worldTransform().map( QPointF( prevDistance, maxZ ) ) );
          currentLine.append( context.worldTransform().map( QPointF( currentPartStartDistance, maxZ ) ) );
          currentLine.append( currentLine.at( 0 ) );
          mFillSymbol->renderPolygon( currentLine, nullptr, nullptr, context.renderContext() );
          break;
      }
    }
  };

  QPolygonF currentLine;
  double prevDistance = std::numeric_limits< double >::quiet_NaN();
  double currentPartStartDistance = 0;
  for ( auto pointIt = mDistanceToHeightMap.constBegin(); pointIt != mDistanceToHeightMap.constEnd(); ++pointIt )
  {
    if ( currentLine.empty() ) // new part
    {
      if ( std::isnan( pointIt.value() ) ) // skip emptiness
        continue;
      currentPartStartDistance = pointIt.key();
    }

    if ( std::isnan( pointIt.value() ) )
    {
      checkLine( currentLine, context, minZ, maxZ, prevDistance, currentPartStartDistance );
      currentLine.clear();
    }
    else
    {
      currentLine.append( context.worldTransform().map( QPointF( pointIt.key(), pointIt.value() ) ) );
      prevDistance = pointIt.key();
    }
  }

  checkLine( currentLine, context, minZ, maxZ, prevDistance, currentPartStartDistance );

  switch ( symbology )
  {
    case Qgis::ProfileSurfaceSymbology::Line:
      mLineSymbol->stopRender( context.renderContext() );
      break;
    case Qgis::ProfileSurfaceSymbology::FillBelow:
    case Qgis::ProfileSurfaceSymbology::FillAbove:
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
  mElevationLimit = surfaceGenerator->elevationLimit();

  mProfileCurve.reset( surfaceGenerator->mProfileCurve->clone() );
}

//
// QgsAbstractProfileSurfaceGenerator
//

QgsAbstractProfileSurfaceGenerator::QgsAbstractProfileSurfaceGenerator( const QgsProfileRequest &request )
  : mProfileCurve( request.profileCurve() ? request.profileCurve()->clone() : nullptr )
{

}

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

double QgsAbstractProfileSurfaceGenerator::elevationLimit() const
{
  return mElevationLimit;
}

void QgsAbstractProfileSurfaceGenerator::setElevationLimit( double limit )
{
  mElevationLimit = limit;
}
