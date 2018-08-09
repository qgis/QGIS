/***************************************************************************
                         qgsmeshlayerinterpolator.cpp
                         ----------------------------
    begin                : April 2018
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

///@cond PRIVATE

#include <memory>

#include "qgsmeshlayerinterpolator.h"

#include "qgsrasterinterface.h"
#include "qgsmaptopixel.h"
#include "qgsvector.h"
#include "qgspoint.h"
#include "qgspointxy.h"

static void bbox2rect(
  const QgsMapToPixel &mtp,
  const QSize &outputSize,
  const QgsRectangle &bbox,
  int &leftLim, int &rightLim, int &topLim, int &bottomLim )
{
  QgsPointXY ll = mtp.transform( bbox.xMinimum(), bbox.yMinimum() );
  QgsPointXY ur = mtp.transform( bbox.xMaximum(), bbox.yMaximum() );
  topLim = std::max( int( ur.y() ), 0 );
  bottomLim = std::min( int( ll.y() ), outputSize.height() - 1 );
  leftLim = std::max( int( ll.x() ), 0 );
  rightLim = std::min( int( ur.x() ), outputSize.width() - 1 );
}

static void lamTol( double &lam )
{
  const static double eps = 1e-6;
  if ( ( lam < 0.0 ) && ( lam > -eps ) )
  {
    lam = 0.0;
  }
}

static bool E3T_physicalToBarycentric( const QgsPointXY &pA, const QgsPointXY &pB, const QgsPointXY &pC, const QgsPointXY &pP,
                                       double &lam1, double &lam2, double &lam3 )
{
  if ( pA == pB || pA == pC || pB == pC )
    return false; // this is not a valid triangle!

  // Compute vectors
  QgsVector v0( pC - pA );
  QgsVector v1( pB - pA );
  QgsVector v2( pP - pA );

  // Compute dot products
  double dot00 = v0 * v0;
  double dot01 = v0 * v1;
  double dot02 = v0 * v2;
  double dot11 = v1 * v1;
  double dot12 = v1 * v2;

  // Compute barycentric coordinates
  double invDenom = 1.0 / ( dot00 * dot11 - dot01 * dot01 );
  lam1 = ( dot11 * dot02 - dot01 * dot12 ) * invDenom;
  lam2 = ( dot00 * dot12 - dot01 * dot02 ) * invDenom;
  lam3 = 1.0 - lam1 - lam2;

  // Apply some tolerance to lam so we can detect correctly border points
  lamTol( lam1 );
  lamTol( lam2 );
  lamTol( lam3 );

  // Return if POI is outside triangle
  if ( ( lam1 < 0 ) || ( lam2 < 0 ) || ( lam3 < 0 ) )
  {
    return false;
  }

  return true;
}

double QgsMeshLayerInterpolator::interpolateFromVerticesData( const QgsPointXY &p1, const QgsPointXY &p2, const QgsPointXY &p3,
    double val1, double val2, double val3, const QgsPointXY &pt )
{
  double lam1, lam2, lam3;
  if ( !E3T_physicalToBarycentric( p1, p2, p3, pt, lam1, lam2, lam3 ) )
    return std::numeric_limits<double>::quiet_NaN();

  return lam1 * val3 + lam2 * val2 + lam3 * val1;
}

double interpolateFromFacesData( const QgsPointXY &p1, const QgsPointXY &p2, const QgsPointXY &p3, double val, const QgsPointXY &pt )
{
  double lam1, lam2, lam3;
  if ( !E3T_physicalToBarycentric( p1, p2, p3, pt, lam1, lam2, lam3 ) )
    return std::numeric_limits<double>::quiet_NaN();

  return val;
}

QgsMeshLayerInterpolator::QgsMeshLayerInterpolator(
  const QgsTriangularMesh &m,
  const QVector<double> &datasetValues,
  bool dataIsOnVertices,
  const QgsRenderContext &context,
  const QSize &size )
  : mTriangularMesh( m ),
    mDatasetValues( datasetValues ),
    mContext( context ),
    mDataOnVertices( dataIsOnVertices ),
    mOutputSize( size )
{
}

QgsMeshLayerInterpolator::~QgsMeshLayerInterpolator() = default;

QgsRasterInterface *QgsMeshLayerInterpolator::clone() const
{
  assert( false ); // we should not need this (hopefully)
  return nullptr;
}

Qgis::DataType QgsMeshLayerInterpolator::dataType( int ) const
{
  return Qgis::Float64;
}

int QgsMeshLayerInterpolator::bandCount() const
{
  return 1;
}

QgsRasterBlock *QgsMeshLayerInterpolator::block( int, const QgsRectangle &extent, int width, int height, QgsRasterBlockFeedback *feedback )
{
  std::unique_ptr<QgsRasterBlock> outputBlock( new QgsRasterBlock( Qgis::Float64, width, height ) );
  outputBlock->setIsNoData();  // assume initially that all values are unset
  double *data = reinterpret_cast<double *>( outputBlock->bits() );

  const QVector<QgsMeshFace> &triangles = mTriangularMesh.triangles();
  const QVector<QgsMeshVertex> &vertices = mTriangularMesh.vertices();

  // currently expecting that triangulation does not add any new extra vertices on the way
  if ( mDataOnVertices )
    Q_ASSERT( mDatasetValues.count() == mTriangularMesh.vertices().count() );

  for ( int i = 0; i < triangles.size(); ++i )
  {
    if ( feedback && feedback->isCanceled() )
      break;

    const QgsMeshFace &face = triangles[i];

    const int v1 = face[0], v2 = face[1], v3 = face[2];
    const QgsPoint p1 = vertices[v1], p2 = vertices[v2], p3 = vertices[v3];

    QgsRectangle bbox;
    bbox.combineExtentWith( p1.x(), p1.y() );
    bbox.combineExtentWith( p2.x(), p2.y() );
    bbox.combineExtentWith( p3.x(), p3.y() );
    if ( !extent.intersects( bbox ) )
      continue;

    // Get the BBox of the element in pixels
    int topLim, bottomLim, leftLim, rightLim;
    bbox2rect( mContext.mapToPixel(), mOutputSize, bbox, leftLim, rightLim, topLim, bottomLim );

    // interpolate in the bounding box of the face
    for ( int j = topLim; j <= bottomLim; j++ )
    {
      double *line = data + ( j * width );
      for ( int k = leftLim; k <= rightLim; k++ )
      {
        double val;
        const QgsPointXY p = mContext.mapToPixel().toMapCoordinates( k, j );
        if ( mDataOnVertices )
          val = interpolateFromVerticesData(
                  p1,
                  p2,
                  p3,
                  mDatasetValues[v1],
                  mDatasetValues[v2],
                  mDatasetValues[v3],
                  p );
        else
        {
          int face = mTriangularMesh.trianglesToNativeFaces()[i];
          val = interpolateFromFacesData(
                  p1,
                  p2,
                  p3,
                  mDatasetValues[face],
                  p
                );
        }

        if ( !std::isnan( val ) )
        {
          line[k] = val;
          outputBlock->setIsData( j, k );
        }
      }
    }

  }

  return outputBlock.release();;
}

///@endcond
