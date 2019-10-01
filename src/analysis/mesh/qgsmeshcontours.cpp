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

#include <limits>

#include <QSet>
#include <QPair>

QgsMeshContours::QgsMeshContours( QgsMeshLayer *layer )
  : mMeshLayer( layer )
{
  if ( !mMeshLayer ||  !mMeshLayer->dataProvider() || !mMeshLayer->dataProvider()->isValid() )
    return;

  mNativeMesh.reset( new QgsMesh() );
  mMeshLayer->dataProvider()->populateMesh( mNativeMesh.get() );

  QgsMapSettings mapSettings;
  mapSettings.setExtent( mMeshLayer->extent() );
  mapSettings.setDestinationCrs( mMeshLayer->crs() );
  mapSettings.setOutputDpi( 96 );
  QgsRenderContext context = QgsRenderContext::fromMapSettings( mapSettings );
  mTriangularMesh.reset( new QgsTriangularMesh() );
  mTriangularMesh->update( mNativeMesh.get(), &context );
}

QgsMeshContours::~QgsMeshContours() = default;

QgsGeometry QgsMeshContours::exportPolygons(
  const QgsMeshDatasetIndex &index,
  double min_value,
  double max_value,
  QgsMeshRendererScalarSettings::DataInterpolationMethod method,
  QgsFeedback *feedback
)
{
  // Check if the layer/mesh is valid
  if ( !mTriangularMesh )
    return QgsGeometry();

  if ( min_value > max_value )
  {
    double tmp = max_value;
    max_value = min_value;
    min_value = tmp;
  }

  QVector<QgsGeometry> multiPolygon;

  // STEP 1: Get Data
  populateCache( index, method );
  const QVector<QgsMeshVertex> vertices = mTriangularMesh->vertices();
  const QVector<int> &trianglesToNativeFaces = mTriangularMesh->trianglesToNativeFaces();

  // STEP 2: For each triangle get the contour line
  for ( int i = 0; i < mTriangularMesh->triangles().size(); ++i )
  {
    if ( feedback && feedback->isCanceled() )
      break;

    int nativeIndex = trianglesToNativeFaces.at( i );
    if ( !mScalarActiveFaceFlagValues.active( nativeIndex ) )
      continue;

    const auto &triangle = mTriangularMesh->triangles().at( i );
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
      std::unique_ptr< QgsLineString > ext = qgis::make_unique< QgsLineString> ( coords );
      std::unique_ptr< QgsPolygon > poly = qgis::make_unique< QgsPolygon >();
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
          // nothing here, we are outside the range
          continue;
        }
      }
    }

    // add if the polygon is not degraded
    if ( ring.size() > 2 )
    {
      ring.push_back( coords[0] );
      std::unique_ptr< QgsLineString > ext = qgis::make_unique< QgsLineString> ( coords );
      std::unique_ptr< QgsPolygon > poly = qgis::make_unique< QgsPolygon >();
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
    QgsGeometry res = QgsGeometry::unaryUnion( multiPolygon );
    return res;
  }
}

QgsGeometry QgsMeshContours::exportLines( const QgsMeshDatasetIndex &index,
    double value,
    QgsMeshRendererScalarSettings::DataInterpolationMethod method,
    QgsFeedback *feedback )
{
  // Check if the layer/mesh is valid
  if ( !mTriangularMesh )
    return QgsGeometry();

  std::unique_ptr<QgsMultiLineString> multiLineString( new QgsMultiLineString() );
  QSet<QPair<int, int>> exactEdges;

  // STEP 1: Get Data
  populateCache( index, method );
  QVector<QgsMeshVertex> vertices = mTriangularMesh->vertices();
  const QVector<int> &trianglesToNativeFaces = mTriangularMesh->trianglesToNativeFaces();

  // STEP 2: For each triangle get the contour line
  for ( int i = 0; i < mTriangularMesh->triangles().size(); ++i )
  {
    if ( feedback && feedback->isCanceled() )
      break;

    int nativeIndex = trianglesToNativeFaces.at( i );
    if ( !mScalarActiveFaceFlagValues.active( nativeIndex ) )
      continue;

    const auto &triangle = mTriangularMesh->triangles().at( i );

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
         ( ( value > values[0] ) && ( value > values[1] ) && ( value > values[2] ) ) )
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

void QgsMeshContours::populateCache( const QgsMeshDatasetIndex &index, QgsMeshRendererScalarSettings::DataInterpolationMethod method )
{
  if ( mCachedIndex != index )
  {
    QVector<QgsMeshVertex> vertices = mTriangularMesh->vertices();
    bool scalarDataOnVertices = mMeshLayer->dataProvider()->datasetGroupMetadata( index ).dataType() != QgsMeshDatasetGroupMetadata::DataOnFaces;
    int count =  scalarDataOnVertices ? mNativeMesh->vertices.count() : mNativeMesh->faces.count();

    // populate scalar values
    QgsMeshDataBlock vals = mMeshLayer->dataProvider()->datasetValues(
                              index,
                              0,
                              count );

    // vals could be scalar or vectors, for contour rendering we want always magnitude
    mDatasetValues = QgsMeshLayerUtils::calculateMagnitudes( vals );

    // populate face active flag, always defined on faces
    mScalarActiveFaceFlagValues = mMeshLayer->dataProvider()->areFacesActive(
                                    index,
                                    0,
                                    mNativeMesh->faces.count() );

    // for data on faces, there could be request to interpolate the data to vertices
    if ( ( !scalarDataOnVertices ) )
    {
      mDatasetValues = QgsMeshLayerUtils::interpolateFromFacesData(
                         mDatasetValues,
                         mNativeMesh.get(),
                         mTriangularMesh.get(),
                         &mScalarActiveFaceFlagValues,
                         method
                       );
    }
  }
}
