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

//
// QgsVectorLayerProfileResults
//

QString QgsVectorLayerProfileResults::type() const
{
  return QStringLiteral( "vector" );
}

QHash<double, double> QgsVectorLayerProfileResults::distanceToHeightMap() const
{
  QHash<double, double> res;
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

  mResults = std::make_unique< QgsVectorLayerProfileResults >();

  switch ( QgsWkbTypes::geometryType( mWkbType ) )
  {
    case QgsWkbTypes::PointGeometry:
      if ( !generateProfileForPoints() )
        return false;

      break;
  }

  const QgsRectangle profileCurveBoundingBox = mTransformedCurve->boundingBox();
  //if ( !profileCurveBoundingBox.intersects( mRasterProvider->extent() ) )
  //  return false;

  std::unique_ptr< QgsGeometryEngine > curveEngine( QgsGeometry::createGeometryEngine( mTransformedCurve.get() ) );
  curveEngine->prepareGeometry();



  // convert x/y values back to distance/height values
  QgsGeos originalCurveGeos( mProfileCurve.get() );
  originalCurveGeos.prepareGeometry();
  mResults->results.reserve( mResults->rawPoints.size() );
  QString lastError;
  for ( const QgsPoint &pixel : std::as_const( mResults->rawPoints ) )
  {
    const double distance = originalCurveGeos.lineLocatePoint( pixel, &lastError );

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

double QgsVectorLayerProfileGenerator::featureZToHeight( double x, double y, double z )
{
  switch ( mClamping )
  {
    case Qgis::AltitudeClamping::Absolute:
      break;

    case Qgis::AltitudeClamping::Relative:
    case Qgis::AltitudeClamping::Terrain:
    {
      if ( !mTerrainProvider )
        break;

      // transform feature point to terrain provider crs
      try
      {
        double dummyZ = 0;
        mTargetToTerrainProviderTransform.transformInPlace( x, y, dummyZ );
      }
      catch ( QgsCsException & )
      {
        break;
      }

      const double terrainZ = mTerrainProvider->heightAt( x, y );
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

