/***************************************************************************
                         qgsmeshlayerutils.cpp
                         --------------------------
    begin                : August 2018
    copyright            : (C) 2018 by Martin Dobias
    email                : wonder dot sk at gmail dot com
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "qgsmeshlayerutils.h"

#include "qgsmeshdataprovider.h"

#include <limits>

///@cond PRIVATE

void QgsMeshLayerUtils::calculateMinimumMaximum( double &min, double &max, const QVector<double> &arr )
{
  bool found = false;

  min = std::numeric_limits<double>::max();
  max = std::numeric_limits<double>::min();

  for ( const double val : arr )
  {
    if ( !std::isnan( val ) )
    {
      found = true;
      if ( val < min )
        min = val;
      if ( val > max )
        max = val;
    }
  }

  if ( !found )
  {
    min = std::numeric_limits<double>::quiet_NaN();
    max = std::numeric_limits<double>::quiet_NaN();
  }
}

void QgsMeshLayerUtils::calculateMinMaxForDatasetGroup( double &min, double &max, QgsMeshDataProvider *provider, int groupIndex )
{
  if ( groupIndex < 0 || !provider || groupIndex >= provider->datasetGroupCount() )
  {
    min = std::numeric_limits<double>::quiet_NaN();
    max = std::numeric_limits<double>::quiet_NaN();
    return;
  }

  min = std::numeric_limits<double>::max();
  max = std::numeric_limits<double>::min();

  int count = provider->datasetCount( groupIndex );
  for ( int i = 0; i < count; ++i )
  {
    double dMin, dMax;
    calculateMinMaxForDataset( dMin, dMax, provider, QgsMeshDatasetIndex( groupIndex, i ) );
    min = std::min( min, dMin );
    max = std::max( max, dMax );
  }
}

void QgsMeshLayerUtils::calculateMinMaxForDataset( double &min, double &max, QgsMeshDataProvider *provider, QgsMeshDatasetIndex index )
{
  if ( !index.isValid() || !provider )
  {
    min = std::numeric_limits<double>::quiet_NaN();
    max = std::numeric_limits<double>::quiet_NaN();
    return;
  }

  const QgsMeshDatasetGroupMetadata metadata = provider->datasetGroupMetadata( index );
  bool onVertices = metadata.dataType() == QgsMeshDatasetGroupMetadata::DataOnVertices;
  int count;
  if ( onVertices )
    count = provider->vertexCount();
  else
    count = provider->faceCount();

  bool firstIteration = true;
  for ( int i = 0; i < count; ++i )
  {
    double v = provider->datasetValue( index, i ).scalar();

    if ( std::isnan( v ) )
      continue;
    if ( firstIteration )
    {
      firstIteration = false;
      min = v;
      max = v;
    }
    else
    {
      if ( v < min )
      {
        min = v;
      }
      if ( v > max )
      {
        max = v;
      }
    }
  }

}

void QgsMeshLayerUtils::boundingBoxToScreenRectangle( const QgsMapToPixel &mtp,
    const QSize &outputSize,
    const QgsRectangle &bbox,
    int &leftLim,
    int &rightLim,
    int &topLim,
    int &bottomLim )
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

double QgsMeshLayerUtils::interpolateFromVerticesData( const QgsPointXY &p1, const QgsPointXY &p2, const QgsPointXY &p3,
    double val1, double val2, double val3, const QgsPointXY &pt )
{
  double lam1, lam2, lam3;
  if ( !E3T_physicalToBarycentric( p1, p2, p3, pt, lam1, lam2, lam3 ) )
    return std::numeric_limits<double>::quiet_NaN();

  return lam1 * val3 + lam2 * val2 + lam3 * val1;
}

double QgsMeshLayerUtils::interpolateFromFacesData( const QgsPointXY &p1, const QgsPointXY &p2, const QgsPointXY &p3,
    double val, const QgsPointXY &pt )
{
  double lam1, lam2, lam3;
  if ( !E3T_physicalToBarycentric( p1, p2, p3, pt, lam1, lam2, lam3 ) )
    return std::numeric_limits<double>::quiet_NaN();

  return val;
}

QgsRectangle QgsMeshLayerUtils::triangleBoundingBox( const QgsPointXY &p1, const QgsPointXY &p2, const QgsPointXY &p3 )
{
  QgsRectangle bbox;
  bbox.combineExtentWith( p1.x(), p1.y() );
  bbox.combineExtentWith( p2.x(), p2.y() );
  bbox.combineExtentWith( p3.x(), p3.y() );
  return bbox;
}

///@endcond
