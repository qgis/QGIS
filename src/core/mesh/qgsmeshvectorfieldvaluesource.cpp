/***************************************************************************
                         qgsmeshvectorfieldvaluesource.cpp
                         ---------------------------------
    begin                : September 2026
    copyright            : (C) 2026 by Stefanos Natsis
    email                : uclaros at gmail dot com
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "qgsmeshvectorfieldvaluesource.h"

#include "qgsmeshlayerinterpolator.h"
#include "qgsmeshlayerutils.h"

///@cond PRIVATE

QgsMeshVectorFieldValueSource::QgsMeshVectorFieldValueSource(
  const QgsTriangularMesh &triangularMesh,
  const QgsMeshDataBlock &datasetVectorValues,
  const QgsMeshDataBlock &scalarActiveFaceFlagValues,
  const QVector<double> &datasetMagnitudeValues,
  QgsMeshDatasetGroupMetadata::DataType dataType,
  const QgsRectangle &layerExtent,
  double maximumMagnitude
)
  : mTriangularMesh( triangularMesh )
  , mDatasetValues( datasetVectorValues )
  , mActiveFaceFlagValues( scalarActiveFaceFlagValues )
  , mMagnitudeValues( datasetMagnitudeValues )
  , mDataType( dataType )
  , mExtent( layerExtent )
  , mMaximumMagnitude( maximumMagnitude )
  , mUseScalarActiveFaceFlagValues( scalarActiveFaceFlagValues.isValid() )
{}

std::unique_ptr<QgsMeshVectorFieldValueSource> QgsMeshVectorFieldValueSource::create(
  const QgsTriangularMesh &triangularMesh,
  const QgsMeshDataBlock &datasetVectorValues,
  const QgsMeshDataBlock &scalarActiveFaceFlagValues,
  const QVector<double> &datasetMagnitudeValues,
  QgsMeshDatasetGroupMetadata::DataType dataType,
  const QgsRectangle &layerExtent,
  double maximumMagnitude
)
{
  if ( dataType == QgsMeshDatasetGroupMetadata::DataOnVertices )
    return std::make_unique<
      QgsMeshVectorFieldValueSourceFromVertex>( triangularMesh, datasetVectorValues, scalarActiveFaceFlagValues, datasetMagnitudeValues, QgsMeshDatasetGroupMetadata::DataOnVertices, layerExtent, maximumMagnitude );

  return std::make_unique<
    QgsMeshVectorFieldValueSourceFromFace>( triangularMesh, datasetVectorValues, scalarActiveFaceFlagValues, datasetMagnitudeValues, QgsMeshDatasetGroupMetadata::DataOnFaces, layerExtent, maximumMagnitude );
}

QgsVector QgsMeshVectorFieldValueSource::vectorValue( const QgsPointXY &point ) const
{
  if ( mCacheFaceIndex != -1 && mCacheFaceIndex < mTriangularMesh.triangles().count() )
  {
    QgsVector res = interpolatedValuePrivate( mCacheFaceIndex, point );
    if ( isVectorValid( res ) )
    {
      activeFaceFilter( res, mCacheFaceIndex );
      return res;
    }
  }

  //point is not on the face associated with mCacheIndex --> search for the face containing the point
  QList<int> potentialFaceIndexes = mTriangularMesh.faceIndexesForRectangle( QgsRectangle( point, point ) );
  mCacheFaceIndex = -1;
  for ( const int faceIndex : potentialFaceIndexes )
  {
    QgsVector res = interpolatedValuePrivate( faceIndex, point );
    if ( isVectorValid( res ) )
    {
      mCacheFaceIndex = faceIndex;
      activeFaceFilter( res, mCacheFaceIndex );
      return res;
    }
  }

  //--> no face found return non valid vector
  return ( QgsVector( std::numeric_limits<double>::quiet_NaN(), std::numeric_limits<double>::quiet_NaN() ) );
}

QgsRectangle QgsMeshVectorFieldValueSource::extent() const
{
  return mExtent;
}

double QgsMeshVectorFieldValueSource::maximumMagnitude() const
{
  return mMaximumMagnitude;
}

QVector<QgsPointXY> QgsMeshVectorFieldValueSource::seedPoints( const QgsRectangle &extent ) const
{
  const QList<int> facesInExtent = mTriangularMesh.faceIndexesForRectangle( extent );
  QSet<int> vertices;
  for ( const int f : facesInExtent )
  {
    const QgsMeshFace face = mTriangularMesh.triangles().at( f );
    for ( const int i : face )
      vertices.insert( i );
  }

  QVector<QgsPointXY> points;
  points.reserve( vertices.count() );
  for ( const int i : vertices )
    points.append( mTriangularMesh.vertices().at( i ) );

  return points;
}

std::unique_ptr<QgsRasterInterface> QgsMeshVectorFieldValueSource::magnitudeSource( const QgsRenderContext &context, QSize size ) const
{
  if ( mMagnitudeValues.isEmpty() )
    return nullptr;

  return std::make_unique<QgsMeshLayerInterpolator>( mTriangularMesh, mMagnitudeValues, mActiveFaceFlagValues, mDataType, context, size );
}

bool QgsMeshVectorFieldValueSource::isVectorValid( const QgsVector &v ) const
{
  return !( std::isnan( v.x() ) || std::isnan( v.y() ) );
}

void QgsMeshVectorFieldValueSource::activeFaceFilter( QgsVector &vector, int faceIndex ) const
{
  if ( mUseScalarActiveFaceFlagValues && !mActiveFaceFlagValues.active( mTriangularMesh.trianglesToNativeFaces()[faceIndex] ) )
    vector = QgsVector( std::numeric_limits<double>::quiet_NaN(), std::numeric_limits<double>::quiet_NaN() );
}

QgsMeshVectorFieldValueSourceFromVertex *QgsMeshVectorFieldValueSourceFromVertex::clone() const
{
  return new QgsMeshVectorFieldValueSourceFromVertex( *this );
}

QgsVector QgsMeshVectorFieldValueSourceFromVertex::interpolatedValuePrivate( int faceIndex, const QgsPointXY point ) const
{
  const QgsMeshFace face = mTriangularMesh.triangles().at( faceIndex );

  const QgsPoint p1 = mTriangularMesh.vertices().at( face.at( 0 ) );
  const QgsPoint p2 = mTriangularMesh.vertices().at( face.at( 1 ) );
  const QgsPoint p3 = mTriangularMesh.vertices().at( face.at( 2 ) );

  const QgsVector v1 = QgsVector( mDatasetValues.value( face.at( 0 ) ).x(), mDatasetValues.value( face.at( 0 ) ).y() );
  const QgsVector v2 = QgsVector( mDatasetValues.value( face.at( 1 ) ).x(), mDatasetValues.value( face.at( 1 ) ).y() );
  const QgsVector v3 = QgsVector( mDatasetValues.value( face.at( 2 ) ).x(), mDatasetValues.value( face.at( 2 ) ).y() );

  return QgsMeshLayerUtils::interpolateVectorFromVerticesData( p1, p2, p3, v1, v2, v3, point );
}

QgsMeshVectorFieldValueSourceFromFace *QgsMeshVectorFieldValueSourceFromFace::clone() const
{
  return new QgsMeshVectorFieldValueSourceFromFace( *this );
}

QgsVector QgsMeshVectorFieldValueSourceFromFace::interpolatedValuePrivate( int faceIndex, const QgsPointXY point ) const
{
  const QgsMeshFace face = mTriangularMesh.triangles().at( faceIndex );

  const QgsPoint p1 = mTriangularMesh.vertices().at( face.at( 0 ) );
  const QgsPoint p2 = mTriangularMesh.vertices().at( face.at( 1 ) );
  const QgsPoint p3 = mTriangularMesh.vertices().at( face.at( 2 ) );

  const int nativeFaceIndex = mTriangularMesh.trianglesToNativeFaces().at( faceIndex );
  const QgsVector vect = QgsVector( mDatasetValues.value( nativeFaceIndex ).x(), mDatasetValues.value( nativeFaceIndex ).y() );

  return QgsMeshLayerUtils::interpolateVectorFromFacesData( p1, p2, p3, vect, point );
}

///@endcond
