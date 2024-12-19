/***************************************************************************
                          qgsmeshcontours.cpp
                          -------------------
    begin                : September 25th, 2019
    copyright            : (C) 2018 by Peter Petrik
    email                : zilolv at gmail dot com
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/
#include "qgsmeshcontours.h"
#include "qgspoint.h"
#include "qgsmultilinestring.h"
#include "qgslinestring.h"
#include "qgspolygon.h"
#include "qgsmapsettings.h"
#include "qgsmeshlayer.h"
#include "qgstriangularmesh.h"
#include "qgsgeometryutils.h"
#include "qgsfeature.h"
#include "qgsgeometry.h"
#include "qgsmeshlayerutils.h"
#include "qgsmeshdataprovider.h"
#include "qgsfeedback.h"
#include "qgsproject.h"

#include <limits>

#include <QSet>
#include <QPair>

QgsMeshContours::QgsMeshContours( QgsMeshLayer *layer )
  : mMeshLayer( layer )
{
  if ( !mMeshLayer ||  !mMeshLayer->dataProvider() || !mMeshLayer->dataProvider()->isValid() )
    return;

  // Support for meshes with edges is not implemented
  if ( mMeshLayer->dataProvider()->contains( QgsMesh::ElementType::Edge ) )
    return;

  mMeshLayer->dataProvider()->populateMesh( &mNativeMesh );
  mTriangularMesh.update( &mNativeMesh );
}

QgsMeshContours::QgsMeshContours( const QgsTriangularMesh &triangularMesh,
                                  const QgsMesh &nativeMesh,
                                  const QVector<double> &datasetValues,
                                  const QgsMeshDataBlock scalarActiveFaceFlagValues ):
  mTriangularMesh( triangularMesh )
  , mNativeMesh( nativeMesh )
  , mDatasetValues( datasetValues )
  , mScalarActiveFaceFlagValues( scalarActiveFaceFlagValues )
{}

QgsMeshContours::~QgsMeshContours() = default;

QgsGeometry QgsMeshContours::exportPolygons(
  const QgsMeshDatasetIndex &index,
  double min_value,
  double max_value,
  QgsMeshRendererScalarSettings::DataResamplingMethod method,
  QgsFeedback *feedback
)
{
  if ( !mMeshLayer )
    return QgsGeometry();

  populateCache( index, method );

  return exportPolygons( min_value, max_value, feedback );
}

QgsGeometry QgsMeshContours::exportPolygons( double min_value, double max_value, QgsFeedback *feedback )
{
  if ( min_value > max_value )
  {
    const double tmp = max_value;
    max_value = min_value;
    min_value = tmp;
  }

  QVector<QgsGeometry> multiPolygon;

  // STEP 1: Get Data
  const QVector<QgsMeshVertex> vertices = mTriangularMesh.vertices();
  const QVector<int> &trianglesToNativeFaces = mTriangularMesh.trianglesToNativeFaces();

  // STEP 2: For each triangle get the contour line
  for ( int i = 0; i < mTriangularMesh.triangles().size(); ++i )
  {
    if ( feedback && feedback->isCanceled() )
      break;

    const int nativeIndex = trianglesToNativeFaces.at( i );
    if ( !mScalarActiveFaceFlagValues.active( nativeIndex ) )
      continue;

    const QgsMeshFace &triangle = mTriangularMesh.triangles().at( i );
    const int indices[3] =
    {
      triangle.at( 0 ),
      triangle.at( 1 ),
      triangle.at( 2 )
    };

    const QVector<QgsMeshVertex> coords =
    {
      vertices.at( indices[0] ),
      vertices.at( indices[1] ),
      vertices.at( indices[2] )
    };

    const double values[3] =
    {
      mDatasetValues.at( indices[0] ),
      mDatasetValues.at( indices[1] ),
      mDatasetValues.at( indices[2] )
    };

    // any value is NaN
    if ( std::isnan( values[0] ) || std::isnan( values[1] ) || std::isnan( values[2] ) )
      continue;

    // all values on vertices are outside the range
    if ( ( ( min_value > values[0] ) && ( min_value > values[1] ) && ( min_value > values[2] ) )  ||
         ( ( max_value < values[0] ) && ( max_value < values[1] ) && ( max_value < values[2] ) ) )
      continue;

    const bool valueInRange[3] =
    {
      ( min_value <= values[0] ) &&( max_value >= values[0] ),
      ( min_value <= values[1] ) &&( max_value >= values[1] ),
      ( min_value <= values[2] ) &&( max_value >= values[2] )
    };

    // all values are inside the range == take whole triangle
    if ( valueInRange[0] && valueInRange[1] && valueInRange[2] )
    {
      QVector<QgsMeshVertex> ring = coords;
      ring.push_back( coords[0] );
      std::unique_ptr< QgsLineString > ext = std::make_unique< QgsLineString> ( coords );
      std::unique_ptr< QgsPolygon > poly = std::make_unique< QgsPolygon >();
      poly->setExteriorRing( ext.release() );
      multiPolygon.push_back( QgsGeometry( std::move( poly ) ) );
      continue;
    }

    // go through all edges
    QVector<QgsMeshVertex> ring;
    for ( int i = 0; i < 3; ++i )
    {
      const int j = ( i + 1 ) % 3;

      if ( valueInRange[i] )
      {
        if ( valueInRange[j] )
        {
          // whole edge is part of resulting contour polygon edge
          if ( !ring.contains( coords[i] ) )
            ring.push_back( coords[i] );
          if ( !ring.contains( coords[j] ) )
            ring.push_back( coords[j] );
        }
        else
        {
          // i is part or the resulting edge
          if ( !ring.contains( coords[i] ) )
            ring.push_back( coords[i] );
          // we need to find the other point
          double value = max_value;
          if ( values[i] > values[j] )
          {
            value = min_value;
          }
          const double fraction = ( value - values[i] ) / ( values[j] - values[i] );
          const QgsPoint xy = QgsGeometryUtils::interpolatePointOnLine( coords[i], coords[j], fraction );
          if ( !ring.contains( xy ) )
            ring.push_back( xy );
        }
      }
      else
      {
        if ( valueInRange[j] )
        {
          // we need to find the other point
          double value = max_value;
          if ( values[i] < values[j] )
          {
            value = min_value;
          }

          const double fraction = ( value - values[i] ) / ( values[j] - values[i] );
          const QgsPoint xy = QgsGeometryUtils::interpolatePointOnLine( coords[i], coords[j], fraction );
          if ( !ring.contains( xy ) )
            ring.push_back( xy );

          // j is part
          if ( !ring.contains( coords[j] ) )
            ring.push_back( coords[j] );

        }
        else
        {
          // last option we need to consider is that both min and max are between
          // value i and j, and in that case we need to calculate both point
          double value1 = max_value;
          double value2 = max_value;
          if ( values[i] < values[j] )
          {
            if ( ( min_value < values[i] ) || ( max_value > values[j] ) )
            {
              continue;
            }
            value1 = min_value;
          }
          else
          {
            if ( ( min_value < values[j] ) || ( max_value > values[i] ) )
            {
              continue;
            }
            value2 = min_value;
          }

          const double fraction1 = ( value1 - values[i] ) / ( values[j] - values[i] );
          const QgsPoint xy1 = QgsGeometryUtils::interpolatePointOnLine( coords[i], coords[j], fraction1 );
          if ( !ring.contains( xy1 ) )
            ring.push_back( xy1 );

          const double fraction2 = ( value2 - values[i] ) / ( values[j] - values[i] );
          const QgsPoint xy2 = QgsGeometryUtils::interpolatePointOnLine( coords[i], coords[j], fraction2 );
          if ( !ring.contains( xy2 ) )
            ring.push_back( xy2 );
        }
      }
    }

    // add if the polygon is not degraded
    if ( ring.size() > 2 )
    {
      std::unique_ptr< QgsLineString > ext = std::make_unique< QgsLineString> ( ring );
      std::unique_ptr< QgsPolygon > poly = std::make_unique< QgsPolygon >();
      poly->setExteriorRing( ext.release() );
      multiPolygon.push_back( QgsGeometry( std::move( poly ) ) );
    }
  }

  // STEP 3: dissolve the individual polygons from triangles if possible
  if ( multiPolygon.isEmpty() )
  {
    return QgsGeometry();
  }
  else
  {
    const QgsGeometry res = QgsGeometry::unaryUnion( multiPolygon );
    return res;
  }
}

QgsGeometry QgsMeshContours::exportLines( const QgsMeshDatasetIndex &index,
    double value,
    QgsMeshRendererScalarSettings::DataResamplingMethod method,
    QgsFeedback *feedback )
{
  // Check if the layer/mesh is valid
  if ( !mMeshLayer )
    return QgsGeometry();

  populateCache( index, method );

  return exportLines( value, feedback );
}

QgsGeometry QgsMeshContours::exportLines( double value, QgsFeedback *feedback )
{
  std::unique_ptr<QgsMultiLineString> multiLineString( new QgsMultiLineString() );
  QSet<QPair<int, int>> exactEdges;

  // STEP 1: Get Data
  const QVector<QgsMeshVertex> vertices = mTriangularMesh.vertices();
  const QVector<int> &trianglesToNativeFaces = mTriangularMesh.trianglesToNativeFaces();

  // STEP 2: For each triangle get the contour line
  for ( int i = 0; i < mTriangularMesh.triangles().size(); ++i )
  {
    if ( feedback && feedback->isCanceled() )
      break;

    const int nativeIndex = trianglesToNativeFaces.at( i );
    if ( !mScalarActiveFaceFlagValues.active( nativeIndex ) )
      continue;

    const QgsMeshFace &triangle = mTriangularMesh.triangles().at( i );

    const int indices[3] =
    {
      triangle.at( 0 ),
      triangle.at( 1 ),
      triangle.at( 2 )
    };

    const QVector<QgsMeshVertex> coords =
    {
      vertices.at( indices[0] ),
      vertices.at( indices[1] ),
      vertices.at( indices[2] )
    };

    const double values[3] =
    {
      mDatasetValues.at( indices[0] ),
      mDatasetValues.at( indices[1] ),
      mDatasetValues.at( indices[2] )
    };

    // any value is NaN
    if ( std::isnan( values[0] ) || std::isnan( values[1] ) || std::isnan( values[2] ) )
      continue;

    // value is outside the range
    if ( ( ( value > values[0] ) && ( value > values[1] ) && ( value > values[2] ) )  ||
         ( ( value < values[0] ) && ( value < values[1] ) && ( value < values[2] ) ) )
      continue;

    // all values are the same
    if ( qgsDoubleNear( values[0], values[1] ) && qgsDoubleNear( values[1], values[2] ) )
      continue;

    // go through all edges
    QgsPoint tmp;

    for ( int i = 0; i < 3; ++i )
    {
      const int j = ( i + 1 ) % 3;
      // value is outside the range
      if ( ( ( value > values[i] ) && ( value > values[j] ) ) ||
           ( ( value < values[i] ) && ( value < values[j] ) ) )
        continue;

      // the whole edge is result and we are done
      if ( qgsDoubleNear( values[i], values[j] ) && qgsDoubleNear( values[i], values[j] ) )
      {
        if ( exactEdges.contains( { indices[i], indices[j] } ) || exactEdges.contains( { indices[j], indices[i] } ) )
        {
          break;
        }
        else
        {
          exactEdges.insert( { indices[i], indices[j] } );
          std::unique_ptr<QgsLineString> line( new QgsLineString( coords[i], coords[j] ) );
          multiLineString->addGeometry( line.release() );
          break;
        }
      }

      // only one point matches, we are not interested in this
      if ( qgsDoubleNear( values[i], value ) || qgsDoubleNear( values[j], value ) )
      {
        continue;
      }

      // ok part of the result contour line is one point on this edge
      const double fraction = ( value - values[i] ) / ( values[j] - values[i] );
      const QgsPoint xy = QgsGeometryUtils::interpolatePointOnLine( coords[i], coords[j], fraction );

      if ( std::isnan( tmp.x() ) )
      {
        // ok we have found start point of the contour line
        tmp = xy;
      }
      else
      {
        // we have found the end point of the contour line, we are done
        std::unique_ptr<QgsLineString> line( new QgsLineString( tmp, xy ) );
        multiLineString->addGeometry( line.release() );
        break;
      }
    }
  }

  // STEP 3: merge the contour segments to linestrings
  if ( multiLineString->isEmpty() )
  {
    return QgsGeometry();
  }
  else
  {
    const QgsGeometry in( multiLineString.release() );
    const QgsGeometry res = in.mergeLines();
    return res;
  }
}

void QgsMeshContours::populateCache( const QgsMeshDatasetIndex &index, QgsMeshRendererScalarSettings::DataResamplingMethod method )
{
  if ( !mMeshLayer )
    return;

  if ( mCachedIndex != index )
  {
    const bool scalarDataOnVertices = mMeshLayer->dataProvider()->datasetGroupMetadata( index ).dataType() == QgsMeshDatasetGroupMetadata::DataOnVertices;
    const int count =  scalarDataOnVertices ? mNativeMesh.vertices.count() : mNativeMesh.faces.count();

    // populate scalar values
    const QgsMeshDataBlock vals = QgsMeshLayerUtils::datasetValues(
                                    mMeshLayer,
                                    index,
                                    0,
                                    count );
    if ( vals.isValid() )
    {
      // vals could be scalar or vectors, for contour rendering we want always magnitude
      mDatasetValues = QgsMeshLayerUtils::calculateMagnitudes( vals );
    }
    else
    {
      mDatasetValues = QVector<double>( count, std::numeric_limits<double>::quiet_NaN() );
    }

    // populate face active flag, always defined on faces
    mScalarActiveFaceFlagValues = mMeshLayer->dataProvider()->areFacesActive(
                                    index,
                                    0,
                                    mNativeMesh.faces.count() );

    // for data on faces, there could be request to interpolate the data to vertices
    if ( ( !scalarDataOnVertices ) )
    {
      mDatasetValues = QgsMeshLayerUtils::interpolateFromFacesData(
                         mDatasetValues,
                         &mNativeMesh,
                         &mTriangularMesh,
                         &mScalarActiveFaceFlagValues,
                         method
                       );
    }
  }
}
