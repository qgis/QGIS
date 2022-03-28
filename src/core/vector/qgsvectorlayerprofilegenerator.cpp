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


#include "qgsapplication.h"
#include "qgscolorschemeregistry.h"
#include <QPolygonF>

//
// QgsVectorLayerProfileResults
//

QString QgsVectorLayerProfileResults::type() const
{
  return QStringLiteral( "vector" );
}

QMap<double, double> QgsVectorLayerProfileResults::distanceToHeightMap() const
{
  QMap<double, double> res;
  for ( const Result &r : results )
  {
    res.insert( r.distance, r.height );
  }
  return res;
}

QgsPointSequence QgsVectorLayerProfileResults::sampledPoints() const
{
  return rawPoints;
}

QVector<QgsGeometry> QgsVectorLayerProfileResults::asGeometries() const
{
  return geometries;
}

void QgsVectorLayerProfileResults::renderResults( QgsProfileRenderContext &context )
{
  QPainter *painter = context.renderContext().painter();
  if ( !painter )
    return;

  painter->save();
  painter->setBrush( Qt::NoBrush );
  QPen pen( QgsApplication::colorSchemeRegistry()->fetchRandomStyleColor() );
  pen.setWidthF( 3 );
  painter->setPen( pen );

#if 0
  p.setBrush( QBrush( QgsApplication::colorSchemeRegistry()->fetchRandomStyleColor() ) );

  const QMap< double, double > distMap = it->distanceToHeightMap();

  for ( auto pointIt = distMap.begin(); pointIt != distMap.end(); ++pointIt )
  {
    double scaledX = pointIt.key() / curveLength * width();
    double scaledY = zToCanvasY( pointIt.value() );
    p.drawEllipse( QPointF( scaledX, scaledY ), 3, 3 );
  }
#endif

  for ( const QgsGeometry &geometry : std::as_const( geometries ) )
  {
    QgsGeometry transformed = geometry;
    transformed.transform( context.worldTransform() );
    transformed.constGet()->draw( *painter );
  }

  painter->restore();
}

//
// QgsVectorLayerProfileGenerator
//

QgsVectorLayerProfileGenerator::QgsVectorLayerProfileGenerator( QgsVectorLayer *layer, const QgsProfileRequest &request )
  : mProfileCurve( request.profileCurve() ? request.profileCurve()->clone() : nullptr )
  , mTerrainProvider( request.terrainProvider() ? request.terrainProvider()->clone() : nullptr )
  , mTolerance( request.tolerance() )
  , mSourceCrs( layer->crs() )
  , mTargetCrs( request.crs() )
  , mTransformContext( request.transformContext() )
  , mExtent( layer->extent() )
  , mSource( std::make_unique< QgsVectorLayerFeatureSource >( layer ) )
  , mOffset( layer->elevationProperties()->zOffset() )
  , mScale( layer->elevationProperties()->zScale() )
  , mClamping( qgis::down_cast< QgsVectorLayerElevationProperties* >( layer->elevationProperties() )->clamping() )
  , mBinding( qgis::down_cast< QgsVectorLayerElevationProperties* >( layer->elevationProperties() )->binding() )
  , mExtrusionEnabled( qgis::down_cast< QgsVectorLayerElevationProperties* >( layer->elevationProperties() )->extrusionEnabled() )
  , mExtrusionHeight( qgis::down_cast< QgsVectorLayerElevationProperties* >( layer->elevationProperties() )->extrusionHeight() )
  , mWkbType( layer->wkbType() )
{
  if ( mTerrainProvider )
    mTerrainProvider->prepare(); // must be done on main thread

}

QgsVectorLayerProfileGenerator::~QgsVectorLayerProfileGenerator() = default;

bool QgsVectorLayerProfileGenerator::generateProfile()
{
  if ( !mProfileCurve )
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
    QgsDebugMsg( QStringLiteral( "Error transforming profile line to vector CRS" ) );
    return false;
  }

  const QgsRectangle profileCurveBoundingBox = mTransformedCurve->boundingBox();
  if ( !profileCurveBoundingBox.intersects( mExtent ) )
    return false;

  mResults = std::make_unique< QgsVectorLayerProfileResults >();

  mProfileCurveEngine.reset( new QgsGeos( mProfileCurve.get() ) );
  mProfileCurveEngine->prepareGeometry();

  switch ( QgsWkbTypes::geometryType( mWkbType ) )
  {
    case QgsWkbTypes::PointGeometry:
      if ( !generateProfileForPoints() )
        return false;
      break;

    case QgsWkbTypes::LineGeometry:
      if ( !generateProfileForLines() )
        return false;
      break;

    case QgsWkbTypes::PolygonGeometry:
      if ( !generateProfileForPolygons() )
        return false;
      break;

    case QgsWkbTypes::UnknownGeometry:
    case QgsWkbTypes::NullGeometry:
      return false;
  }

  // convert x/y values back to distance/height values
  mResults->results.reserve( mResults->rawPoints.size() );
  QString lastError;
  for ( const QgsPoint &pixel : std::as_const( mResults->rawPoints ) )
  {
    const double distance = mProfileCurveEngine->lineLocatePoint( pixel, &lastError );

    QgsVectorLayerProfileResults::Result res;
    res.distance = distance;
    res.height = pixel.z();
    mResults->results.push_back( res );
  }

  return true;
}

QgsAbstractProfileResults *QgsVectorLayerProfileGenerator::takeResults()
{
  return mResults.release();
}

bool QgsVectorLayerProfileGenerator::generateProfileForPoints()
{
  // get features from layer
  QgsFeatureRequest request;
  request.setDestinationCrs( mTargetCrs, mTransformContext );
  request.setDistanceWithin( QgsGeometry( mProfileCurve->clone() ), mTolerance );
  request.setNoAttributes();

  auto processPoint = [this]( const QgsPoint * point )
  {
    const double height = featureZToHeight( point->x(), point->y(), point->z() );
    mResults->rawPoints.append( QgsPoint( point->x(), point->y(), height ) );

    if ( mExtrusionEnabled )
    {
      mResults->geometries.append( QgsGeometry( new QgsLineString( QgsPoint( point->x(), point->y(), height ),
                                   QgsPoint( point->x(), point->y(), height + mExtrusionHeight ) ) ) );
    }
    else
    {
      mResults->geometries.append( QgsGeometry( new QgsPoint( point->x(), point->y(), height ) ) );
    }
  };

  QgsFeature feature;
  QgsFeatureIterator it = mSource->getFeatures( request );
  while ( it.nextFeature( feature ) )
  {
    const QgsGeometry g = feature.geometry();
    if ( g.isMultipart() )
    {
      for ( auto it = g.const_parts_begin(); it != g.const_parts_end(); ++it )
      {
        processPoint( qgsgeometry_cast< const QgsPoint * >( *it ) );
      }
    }
    else
    {
      processPoint( qgsgeometry_cast< const QgsPoint * >( g.constGet() ) );
    }
  }
  return true;
}

bool QgsVectorLayerProfileGenerator::generateProfileForLines()
{
  // get features from layer
  QgsFeatureRequest request;
  request.setDestinationCrs( mTargetCrs, mTransformContext );
  request.setFilterRect( mProfileCurve->boundingBox() );
  request.setNoAttributes();

  auto processCurve = [this]( const QgsCurve * curve )
  {
    QString error;
    std::unique_ptr< QgsAbstractGeometry > intersection( mProfileCurveEngine->intersection( curve, &error ) );
    if ( !intersection )
      return;

    QgsGeos curveGeos( curve );
    curveGeos.prepareGeometry();

    for ( auto it = intersection->const_parts_begin(); it != intersection->const_parts_end(); ++it )
    {
      if ( const QgsPoint *intersectionPoint = qgsgeometry_cast< const QgsPoint * >( *it ) )
      {
        // unfortunately we need to do some work to interpolate the z value for the line -- GEOS doesn't give us this
        const double distance = curveGeos.lineLocatePoint( *intersectionPoint, &error );
        std::unique_ptr< QgsPoint > interpolatedPoint( curve->interpolatePoint( distance ) );

        const double height = featureZToHeight( interpolatedPoint->x(), interpolatedPoint->y(), interpolatedPoint->z() );
        mResults->rawPoints.append( QgsPoint( interpolatedPoint->x(), interpolatedPoint->y(), height ) );
        if ( mExtrusionEnabled )
        {
          mResults->geometries.append( QgsGeometry( new QgsLineString( QgsPoint( interpolatedPoint->x(), interpolatedPoint->y(), height ),
                                       QgsPoint( interpolatedPoint->x(), interpolatedPoint->y(), height + mExtrusionHeight ) ) ) );
        }
        else
        {
          mResults->geometries.append( QgsGeometry( new QgsPoint( interpolatedPoint->x(), interpolatedPoint->y(), height ) ) );
        }
      }
    }
  };

  QgsFeature feature;
  QgsFeatureIterator it = mSource->getFeatures( request );
  while ( it.nextFeature( feature ) )
  {
    if ( !mProfileCurveEngine->intersects( feature.geometry().constGet() ) )
      continue;

    const QgsGeometry g = feature.geometry();
    if ( g.isMultipart() )
    {
      for ( auto it = g.const_parts_begin(); it != g.const_parts_end(); ++it )
      {
        if ( !mProfileCurveEngine->intersects( *it ) )
          continue;

        processCurve( qgsgeometry_cast< const QgsCurve * >( *it ) );
      }
    }
    else
    {
      processCurve( qgsgeometry_cast< const QgsCurve * >( g.constGet() ) );
    }
  }
  return true;
}

bool QgsVectorLayerProfileGenerator::generateProfileForPolygons()
{
  // get features from layer
  QgsFeatureRequest request;
  request.setDestinationCrs( mTargetCrs, mTransformContext );
  request.setFilterRect( mProfileCurve->boundingBox() );
  request.setNoAttributes();

  auto interpolatePointOnTriangle = []( const QgsPolygon * triangle, double x, double y ) -> QgsPoint
  {
    QgsPoint p1, p2, p3;
    Qgis::VertexType vt;
    triangle->exteriorRing()->pointAt( 0, p1, vt );
    triangle->exteriorRing()->pointAt( 1, p2, vt );
    triangle->exteriorRing()->pointAt( 2, p3, vt );
    const double z = QgsMeshLayerUtils::interpolateFromVerticesData( p1, p2, p3, p1.z(), p2.z(), p3.z(), QgsPointXY( x, y ) );
    return QgsPoint( x, y, z );
  };

  std::function< void( const QgsPolygon *triangle, const QgsAbstractGeometry *intersect ) > processTriangleLineIntersect;
  processTriangleLineIntersect = [this, &interpolatePointOnTriangle, &processTriangleLineIntersect]( const QgsPolygon * triangle, const QgsAbstractGeometry * intersect )
  {
    // intersect may be a (multi)point or (multi)linestring
    switch ( QgsWkbTypes::geometryType( intersect->wkbType() ) )
    {
      case QgsWkbTypes::PointGeometry:
        if ( const QgsMultiPoint *mp = qgsgeometry_cast< const QgsMultiPoint * >( intersect ) )
        {
          const int numPoint = mp->numGeometries();
          for ( int i = 0; i < numPoint; ++i )
          {
            processTriangleLineIntersect( triangle, mp->geometryN( i ) );
          }
        }
        else if ( const QgsPoint *p = qgsgeometry_cast< const QgsPoint * >( intersect ) )
        {
          QgsPoint interpolatedPoint = interpolatePointOnTriangle( triangle, p->x(), p->y() );
          mResults->rawPoints.append( interpolatedPoint );
          if ( mExtrusionEnabled )
          {
            mResults->geometries.append( QgsGeometry( new QgsLineString( interpolatedPoint,
                                         QgsPoint( interpolatedPoint.x(), interpolatedPoint.y(), interpolatedPoint.z() + mExtrusionHeight ) ) ) );
          }
          else
          {
            mResults->geometries.append( QgsGeometry( new QgsPoint( interpolatedPoint ) ) );
          }
        }
        break;
      case QgsWkbTypes::LineGeometry:
        if ( const QgsMultiLineString *ml = qgsgeometry_cast< const QgsMultiLineString * >( intersect ) )
        {
          const int numLines = ml->numGeometries();
          for ( int i = 0; i < numLines; ++i )
          {
            processTriangleLineIntersect( triangle, ml->geometryN( i ) );
          }
        }
        else if ( const QgsLineString *ls = qgsgeometry_cast< const QgsLineString * >( intersect ) )
        {
          const int numPoints = ls->numPoints();
          QVector< double > newX;
          newX.resize( numPoints );
          QVector< double > newY;
          newY.resize( numPoints );
          QVector< double > newZ;
          newZ.resize( numPoints );

          const double *inX = ls->xData();
          const double *inY = ls->yData();
          double *outX = newX.data();
          double *outY = newY.data();
          double *outZ = newZ.data();

          QVector< double > extrudedZ;
          double *extZOut = nullptr;
          if ( mExtrusionEnabled )
          {
            extrudedZ.resize( numPoints );
            extZOut = extrudedZ.data();
          }

          for ( int i = 0 ; i < numPoints; ++i )
          {
            double x = *inX++;
            double y = *inY++;

            QgsPoint interpolatedPoint = interpolatePointOnTriangle( triangle, x, y );
            *outX++ = x;
            *outY++ = y;
            *outZ++ = interpolatedPoint.z();
            if ( extZOut )
              *extZOut++ = interpolatedPoint.z() + mExtrusionHeight;

            mResults->rawPoints.append( interpolatedPoint );
          }

          if ( mExtrusionEnabled )
          {
            std::unique_ptr< QgsLineString > ring = std::make_unique< QgsLineString >( newX, newY, newZ );
            std::unique_ptr< QgsLineString > extrudedRing = std::make_unique< QgsLineString >( newX, newY, extrudedZ );
            std::unique_ptr< QgsLineString > reversedExtrusion( extrudedRing->reversed() );
            ring->append( reversedExtrusion.get() );
            ring->close();
            mResults->geometries.append( QgsGeometry( new QgsPolygon( ring.release() ) ) );
          }
          else
          {
            mResults->geometries.append( QgsGeometry( new QgsLineString( newX, newY, newZ ) ) );
          }
        }
        break;

      case QgsWkbTypes::PolygonGeometry:
      case QgsWkbTypes::UnknownGeometry:
      case QgsWkbTypes::NullGeometry:
        return;
    }
  };

  auto processPolygon = [this, &processTriangleLineIntersect]( const QgsCurvePolygon * polygon )
  {
    std::unique_ptr< QgsPolygon > clampedPolygon;
    if ( const QgsPolygon *p = qgsgeometry_cast< const QgsPolygon * >( polygon ) )
    {
      clampedPolygon.reset( p->clone() );
    }
    else
    {
      clampedPolygon.reset( qgsgeometry_cast< QgsPolygon * >( p->segmentize() ) );
    }
    clampAltitudes( clampedPolygon.get() );

    const QgsRectangle bounds = clampedPolygon->boundingBox();
    QgsTessellator t( bounds, false, false, false, false );
    t.addPolygon( *clampedPolygon, 0 );

    QgsGeometry tessellation( t.asMultiPolygon() );
    tessellation.translate( bounds.xMinimum(), bounds.yMinimum() );

    // iterate through the tessellation, finding triangles which intersect the line
    const int numTriangles = qgsgeometry_cast< const QgsMultiPolygon * >( tessellation.constGet() )->numGeometries();
    for ( int i = 0; i < numTriangles; ++i )
    {
      const QgsPolygon *triangle = qgsgeometry_cast< const QgsPolygon * >( qgsgeometry_cast< const QgsMultiPolygon * >( tessellation.constGet() )->geometryN( i ) );
      if ( !mProfileCurveEngine->intersects( triangle ) )
        continue;

      QString error;
      std::unique_ptr< QgsAbstractGeometry > intersection( mProfileCurveEngine->intersection( triangle, &error ) );
      if ( !intersection )
        continue;

      processTriangleLineIntersect( triangle, intersection.get() );
    }
  };

  QgsFeature feature;
  QgsFeatureIterator it = mSource->getFeatures( request );
  while ( it.nextFeature( feature ) )
  {
    if ( !mProfileCurveEngine->intersects( feature.geometry().constGet() ) )
      continue;

    const QgsGeometry g = feature.geometry();
    if ( g.isMultipart() )
    {
      for ( auto it = g.const_parts_begin(); it != g.const_parts_end(); ++it )
      {
        if ( !mProfileCurveEngine->intersects( *it ) )
          continue;

        processPolygon( qgsgeometry_cast< const QgsCurvePolygon * >( *it ) );
      }
    }
    else
    {
      processPolygon( qgsgeometry_cast< const QgsCurvePolygon * >( g.constGet() ) );
    }
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

double QgsVectorLayerProfileGenerator::featureZToHeight( double x, double y, double z )
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

  return z * mScale + mOffset;
}

void QgsVectorLayerProfileGenerator::clampAltitudes( QgsLineString *lineString, const QgsPoint &centroid )
{
  for ( int i = 0; i < lineString->nCoordinates(); ++i )
  {
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

    const double z = ( terrainZ + geomZ ) * mScale + mOffset;
    lineString->setZAt( i, z );
  }
}

bool QgsVectorLayerProfileGenerator::clampAltitudes( QgsPolygon *polygon )
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

  clampAltitudes( lineString, centroid );

  for ( int i = 0; i < polygon->numInteriorRings(); ++i )
  {
    QgsCurve *curve = const_cast<QgsCurve *>( polygon->interiorRing( i ) );
    QgsLineString *lineString = qgsgeometry_cast<QgsLineString *>( curve );
    if ( !lineString )
      return false;

    clampAltitudes( lineString, centroid );
  }
  return true;
}

