/***************************************************************************
                         qgsmeshlayerprofilegenerator.cpp
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
#include "qgsmeshlayerprofilegenerator.h"
#include "qgsprofilerequest.h"
#include "qgscurve.h"
#include "qgsmeshlayer.h"
#include "qgscoordinatetransform.h"
#include "qgsgeos.h"
#include "qgsterrainprovider.h"
#include "qgsmeshlayerutils.h"
#include "qgslinesymbol.h"
#include "qgsfillsymbol.h"
#include "qgsmeshlayerelevationproperties.h"
#include "qgsprofilesnapping.h"
#include "qgsprofilepoint.h"

//
// QgsMeshLayerProfileGenerator
//

QString QgsMeshLayerProfileResults::type() const
{
  return QStringLiteral( "mesh" );
}

QVector<QgsProfileIdentifyResults> QgsMeshLayerProfileResults::identify( const QgsProfilePoint &point, const QgsProfileIdentifyContext &context )
{
  const QVector<QgsProfileIdentifyResults> noLayerResults = QgsAbstractProfileSurfaceResults::identify( point, context );

  // we have to make a new list, with the correct layer reference set
  QVector<QgsProfileIdentifyResults> res;
  res.reserve( noLayerResults.size() );
  for ( const QgsProfileIdentifyResults &result : noLayerResults )
  {
    res.append( QgsProfileIdentifyResults( mLayer, result.results() ) );
  }
  return res;
}

//
// QgsMeshLayerProfileGenerator
//

QgsMeshLayerProfileGenerator::QgsMeshLayerProfileGenerator( QgsMeshLayer *layer, const QgsProfileRequest &request )
  : mId( layer->id() )
  , mFeedback( std::make_unique< QgsFeedback >() )
  , mProfileCurve( request.profileCurve() ? request.profileCurve()->clone() : nullptr )
  , mSourceCrs( layer->crs() )
  , mTargetCrs( request.crs() )
  , mTransformContext( request.transformContext() )
  , mOffset( layer->elevationProperties()->zOffset() )
  , mScale( layer->elevationProperties()->zScale() )
  , mLayer( layer )
  , mStepDistance( request.stepDistance() )
{
  layer->updateTriangularMesh();
  mTriangularMesh = *layer->triangularMesh();

  mSymbology = qgis::down_cast< QgsMeshLayerElevationProperties * >( layer->elevationProperties() )->profileSymbology();
  mLineSymbol.reset( qgis::down_cast< QgsMeshLayerElevationProperties * >( layer->elevationProperties() )->profileLineSymbol()->clone() );
  mFillSymbol.reset( qgis::down_cast< QgsMeshLayerElevationProperties * >( layer->elevationProperties() )->profileFillSymbol()->clone() );
}

QString QgsMeshLayerProfileGenerator::sourceId() const
{
  return mId;
}

QgsMeshLayerProfileGenerator::~QgsMeshLayerProfileGenerator() = default;

bool QgsMeshLayerProfileGenerator::generateProfile( const QgsProfileGenerationContext & )
{
  if ( !mProfileCurve || mFeedback->isCanceled() )
    return false;

  // we need to transform the profile curve to the mesh's CRS
  QgsGeometry transformedCurve( mProfileCurve->clone() );
  mLayerToTargetTransform = QgsCoordinateTransform( mSourceCrs, mTargetCrs, mTransformContext );

  try
  {
    transformedCurve.transform( mLayerToTargetTransform, Qgis::TransformDirection::Reverse );
  }
  catch ( QgsCsException & )
  {
    QgsDebugMsg( QStringLiteral( "Error transforming profile line to mesh CRS" ) );
    return false;
  }

  if ( mFeedback->isCanceled() )
    return false;

  mResults = std::make_unique< QgsMeshLayerProfileResults >();
  mResults->mLayer = mLayer;
  mResults->copyPropertiesFromGenerator( this );

  // we don't currently have any method to determine line->mesh intersection points, so for now we just sample at about 100(?) points over the line
  const double curveLength = transformedCurve.length();

  if ( !std::isnan( mStepDistance ) )
    transformedCurve = transformedCurve.densifyByDistance( mStepDistance );
  else
    transformedCurve = transformedCurve.densifyByDistance( curveLength / 100 );

  if ( mFeedback->isCanceled() )
    return false;

  for ( auto it = transformedCurve.vertices_begin(); it != transformedCurve.vertices_end(); ++it )
  {
    if ( mFeedback->isCanceled() )
      return false;

    QgsPoint point = ( *it );
    const double height = heightAt( point.x(), point.y() );

    try
    {
      point.transform( mLayerToTargetTransform );
    }
    catch ( QgsCsException & )
    {
      continue;
    }
    mResults->mRawPoints.append( QgsPoint( point.x(), point.y(), height ) );
  }

  if ( mFeedback->isCanceled() )
    return false;

  // convert x/y values back to distance/height values
  QgsGeos originalCurveGeos( mProfileCurve.get() );
  originalCurveGeos.prepareGeometry();
  QString lastError;
  for ( const QgsPoint &pixel : std::as_const( mResults->mRawPoints ) )
  {
    if ( mFeedback->isCanceled() )
      return false;

    const double distance = originalCurveGeos.lineLocatePoint( pixel, &lastError );

    if ( !std::isnan( pixel.z() ) )
    {
      mResults->minZ = std::min( pixel.z(), mResults->minZ );
      mResults->maxZ = std::max( pixel.z(), mResults->maxZ );
    }
    mResults->mDistanceToHeightMap.insert( distance, pixel.z() );
  }

  return true;
}

QgsAbstractProfileResults *QgsMeshLayerProfileGenerator::takeResults()
{
  return mResults.release();
}

QgsFeedback *QgsMeshLayerProfileGenerator::feedback() const
{
  return mFeedback.get();
}

double QgsMeshLayerProfileGenerator::heightAt( double x, double y )
{
  return QgsMeshLayerUtils::interpolateZForPoint( mTriangularMesh, x, y ) * mScale + mOffset;
}

