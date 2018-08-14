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
  if ( groupIndex < 0 || groupIndex >= provider->datasetGroupCount() )
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
  if ( !index.isValid() )
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

///@endcond
