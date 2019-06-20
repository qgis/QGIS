/***************************************************************************
                          qgsmeshcalcutils.cpp
                          --------------------
    begin                : December 18th, 2018
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

#include <QFileInfo>

#include "qgsmeshcalcnode.h"
#include "qgsmeshcalcutils.h"
#include "qgsmeshmemorydataprovider.h"
#include "qgstriangularmesh.h"
#include "qgsmapsettings.h"

const double D_TRUE = 1.0;
const double D_FALSE = 0.0;
const double D_NODATA = std::numeric_limits<double>::quiet_NaN();

std::shared_ptr<QgsMeshMemoryDatasetGroup> QgsMeshCalcUtils::create( const QString &datasetGroupName ) const
{
  const auto dp = mMeshLayer->dataProvider();
  std::shared_ptr<QgsMeshMemoryDatasetGroup> grp;
  for ( int group_i = 0; group_i < dp->datasetGroupCount(); ++group_i )
  {
    const auto meta = dp->datasetGroupMetadata( group_i );
    const QString name = meta.name();
    if ( name == datasetGroupName )
    {
      grp = std::make_shared<QgsMeshMemoryDatasetGroup>();
      grp->isScalar = meta.isScalar();
      grp->type = meta.dataType();
      grp->maximum = meta.maximum();
      grp->minimum = meta.minimum();
      grp->name = meta.name();

      int count = ( meta.dataType() == QgsMeshDatasetGroupMetadata::DataOnFaces ) ? dp->faceCount() : dp->vertexCount();
      for ( int dataset_i = 0; dataset_i < dp->datasetCount( group_i ); ++dataset_i )
      {
        const QgsMeshDatasetIndex index( group_i, dataset_i );
        const auto dsMeta = dp->datasetMetadata( index );
        std::shared_ptr<QgsMeshMemoryDataset> ds = create( grp->type );
        ds->maximum = dsMeta.maximum();
        ds->minimum = dsMeta.minimum();
        ds->time = dsMeta.time();
        ds->valid = dsMeta.isValid();

        const QgsMeshDataBlock block = dp->datasetValues( index, 0, count );
        Q_ASSERT( block.count() == count );
        for ( int value_i = 0; value_i < count; ++value_i )
          ds->values[value_i] = block.value( value_i );

        if ( grp->type == QgsMeshDatasetGroupMetadata::DataOnVertices )
        {
          const QgsMeshDataBlock active = dp->areFacesActive( index, 0, dp->faceCount() );
          Q_ASSERT( active.count() == dp->faceCount() );
          for ( int value_i = 0; value_i < dp->faceCount(); ++value_i )
            ds->active[value_i] = active.active( value_i );
        }
        grp->addDataset( ds );
      }

      break;
    }
  }
  return grp;
}

std::shared_ptr<QgsMeshMemoryDataset> QgsMeshCalcUtils::create( const QgsMeshMemoryDatasetGroup &grp ) const
{
  return create( grp.type );
}

std::shared_ptr<QgsMeshMemoryDataset> QgsMeshCalcUtils::create( const QgsMeshDatasetGroupMetadata::DataType type ) const
{
  std::shared_ptr<QgsMeshMemoryDataset> ds = std::make_shared<QgsMeshMemoryDataset>();
  bool onVertices = type == QgsMeshDatasetGroupMetadata::DataOnVertices;
  if ( onVertices )
  {
    ds->values.resize( mMeshLayer->dataProvider()->vertexCount() );
    ds->active.resize( mMeshLayer->dataProvider()->faceCount() );
    memset( ds->active.data(), 1, static_cast<size_t>( ds->active.size() ) * sizeof( int ) );
  }
  else
  {
    ds->values.resize( mMeshLayer->dataProvider()->faceCount() );
  }
  ds->valid = true;
  return ds;
}

QgsMeshCalcUtils:: QgsMeshCalcUtils( QgsMeshLayer *layer,
                                     const QStringList &usedGroupNames,
                                     double startTime,
                                     double endTime )
  : mMeshLayer( layer )
  , mIsValid( false )
  , mOutputType( QgsMeshDatasetGroupMetadata::DataType::DataOnVertices )
{
  // First populate group's names map and see if we have all groups present
  // And basically fetch all data from any mesh provider to memory
  for ( const QString &groupName : usedGroupNames )
  {
    std::shared_ptr<QgsMeshMemoryDatasetGroup> ds = create( groupName );
    if ( !ds )
      return;

    mDatasetGroupMap.insert( groupName, ds );
  }

  // Now populate used times and check that all datasets do have some times
  // OR just one time (== one output)
  bool timesPopulated = false;
  const auto vals = mDatasetGroupMap.values();
  for ( const auto &ds : vals )
  {
    if ( ds->datasetCount() == 0 )
    {
      // dataset must have at least 1 output
      return;
    }

    if ( ds->datasetCount() > 1 )
    {
      if ( timesPopulated )
      {
        if ( ds->datasetCount() != mTimes.size() )
        {
          // different number of datasets in the groupss
          return;
        }
      }

      for ( int datasetIndex = 0; datasetIndex < ds->datasetCount(); ++datasetIndex )
      {
        std::shared_ptr<const QgsMeshMemoryDataset> o = ds->constDataset( datasetIndex );
        if ( timesPopulated )
        {
          if ( !qgsDoubleNear( mTimes[datasetIndex], o->time ) )
          {
            // error, the times in different datasets differ
            return;
          }
        }
        else
        {
          mTimes.append( o->time );
        }
      }

      timesPopulated = true;
    }
  }

  // case of all group are not time varying or usedGroupNames is empty
  if ( mTimes.isEmpty() )
  {
    mTimes.push_back( 0.0 );
  }
  else
  {
    // filter out times we do not need to speed up calculations
    for ( QVector<double>::iterator it = mTimes.begin(); it != mTimes.end(); )
    {
      if ( qgsDoubleNear( *it, startTime ) ||
           qgsDoubleNear( *it, endTime ) ||
           ( ( *it >= startTime ) && ( *it <= endTime ) ) )
        ++it;
      else
        it = mTimes.erase( it );
    }
  }

  // check that all datasets are of the same type
  for ( const auto &ds : vals )
  {
    if ( ds->type != mOutputType )
      return;
  }

  // All is valid!
  mIsValid = true;
}

bool  QgsMeshCalcUtils::isValid() const
{
  return mIsValid;
}

const QgsMeshLayer *QgsMeshCalcUtils::layer() const
{
  return mMeshLayer;
}

std::shared_ptr<const QgsMeshMemoryDatasetGroup> QgsMeshCalcUtils::group( const QString &datasetName ) const
{
  return mDatasetGroupMap[datasetName];
}

void QgsMeshCalcUtils::populateSpatialFilter( QgsMeshMemoryDatasetGroup &filter, const QgsRectangle &extent ) const
{
  filter.clearDatasets();

  std::shared_ptr<QgsMeshMemoryDataset> output = create( filter );
  output->time = mTimes[0];

  const QList<int> faceIndexesForRectangle = triangularMesh()->faceIndexesForRectangle( extent );
  const QVector<int> trianglesToNativeFaces = triangularMesh()->trianglesToNativeFaces();

  if ( mOutputType == QgsMeshDatasetGroupMetadata::DataOnVertices )
  {
    for ( const int faceIndex : faceIndexesForRectangle )
    {
      const int nativeIndex = trianglesToNativeFaces[faceIndex];
      const QgsMeshFace face = nativeMesh()->face( nativeIndex );
      for ( const int vertexIndex : face )
      {
        output->values[vertexIndex].set( D_TRUE );
      }
    }
  }
  else
  {
    for ( const int faceIndex : faceIndexesForRectangle )
    {
      const int nativeIndex = trianglesToNativeFaces[faceIndex];
      output->values[nativeIndex].set( D_TRUE );
    }
  }
  filter.addDataset( output );
}


void QgsMeshCalcUtils::populateMaskFilter( QgsMeshMemoryDatasetGroup &filter, const QgsGeometry &mask ) const
{
  filter.clearDatasets();
  std::shared_ptr<QgsMeshMemoryDataset> output = create( filter );
  output->time = mTimes[0];

  const QVector<QgsMeshVertex> &vertices = triangularMesh()->vertices();

  if ( mOutputType == QgsMeshDatasetGroupMetadata::DataOnVertices )
  {
    int nativeVertexCount = mMeshLayer->dataProvider()->vertexCount();

    for ( int i = 0; i < nativeVertexCount; ++i )
    {
      const QgsPointXY point( vertices[i] );
      if ( mask.contains( &point ) )
      {
        output->values[i].set( D_TRUE );
      }
      else
      {
        output->values[i].set( D_FALSE );
      }
    }
  }
  else
  {
    const QVector<QgsMeshFace> &triangles = triangularMesh()->triangles();
    for ( int i = 0; i < triangles.size(); ++i )
    {
      const QgsMeshFace face = triangles[i];
      const QgsGeometry geom = QgsMeshUtils::toGeometry( face, vertices );
      const QgsRectangle bbox = geom.boundingBox();
      if ( mask.intersects( bbox ) )
      {
        output->values[i].set( D_TRUE );
      }
      else
      {
        output->values[i].set( D_FALSE );
      }
    }
  }
  filter.addDataset( output );
}

std::shared_ptr<QgsMeshMemoryDataset>  QgsMeshCalcUtils::number( double val, double time ) const
{
  Q_ASSERT( isValid() );

  std::shared_ptr<QgsMeshMemoryDataset> output = create( mOutputType );
  output->time = time;

  // by default it is initialized to 1
  if ( std::isnan( val ) )
  {
    if ( mOutputType == QgsMeshDatasetGroupMetadata::DataOnVertices )
      memset( output->active.data(), 0, static_cast<size_t>( output->active.size() ) * sizeof( int ) );
  }
  else
  {
    for ( int i = 0; i < output->values.size(); ++i ) // Using for loop we are initializing
    {
      output->values[i].set( val );
    }
  }

  return output;
}

void QgsMeshCalcUtils::number( QgsMeshMemoryDatasetGroup &group1, double val ) const
{
  Q_ASSERT( isValid() );

  group1.datasets.clear();
  std::shared_ptr<QgsMeshMemoryDataset> output = number( val, mTimes[0] );
  group1.datasets.push_back( output );
}


void QgsMeshCalcUtils::ones( QgsMeshMemoryDatasetGroup &group1 ) const
{
  Q_ASSERT( isValid() );
  number( group1, 1.0 );
}

void QgsMeshCalcUtils::nodata( QgsMeshMemoryDatasetGroup &group1 ) const
{
  Q_ASSERT( isValid() );
  number( group1, D_NODATA );
}


std::shared_ptr<QgsMeshMemoryDataset>  QgsMeshCalcUtils::copy(
  std::shared_ptr<const QgsMeshMemoryDataset> dataset0
) const
{
  Q_ASSERT( isValid() );
  Q_ASSERT( dataset0 );

  std::shared_ptr<QgsMeshMemoryDataset> output = std::make_shared<QgsMeshMemoryDataset>();
  output->values = dataset0->values; //deep copy
  output->active = dataset0->active; //deep copy
  output->time = dataset0->time;
  output->valid = dataset0->valid;
  return output;
}

void QgsMeshCalcUtils::copy( QgsMeshMemoryDatasetGroup &group1, const QString &groupName ) const
{
  Q_ASSERT( isValid() );

  std::shared_ptr<const QgsMeshMemoryDatasetGroup> group2 = group( groupName );
  Q_ASSERT( group2 );

  if ( group2->datasetCount() == 1 )
  {
    // Always copy
    std::shared_ptr<const QgsMeshMemoryDataset> o0 = group2->constDataset( 0 );
    std::shared_ptr<QgsMeshMemoryDataset> output = copy( o0 );
    group1.addDataset( output );
  }
  else
  {
    for ( int output_index = 0; output_index < group2->datasetCount(); ++output_index )
    {
      std::shared_ptr<const QgsMeshMemoryDataset> o0 = group2->constDataset( output_index );
      if ( qgsDoubleNear( o0->time, mTimes.first() ) ||
           qgsDoubleNear( o0->time, mTimes.last() ) ||
           ( ( o0->time >= mTimes.first() ) && ( o0->time <= mTimes.last() ) )
         )
      {
        std::shared_ptr<QgsMeshMemoryDataset> output = copy( o0 );
        group1.addDataset( output );
      }
    }
  }
}

void QgsMeshCalcUtils::transferDatasets( QgsMeshMemoryDatasetGroup &group1, QgsMeshMemoryDatasetGroup &group2 ) const
{
  Q_ASSERT( isValid() );

  group1.clearDatasets();
  for ( int i = 0; i < group2.datasetCount(); ++i )
  {
    std::shared_ptr<QgsMeshMemoryDataset> o = group2.datasets[i];
    Q_ASSERT( o );
    group1.addDataset( o );
  }
  group2.clearDatasets();
}

void QgsMeshCalcUtils::expand( QgsMeshMemoryDatasetGroup &group1, const QgsMeshMemoryDatasetGroup &group2 ) const
{
  Q_ASSERT( isValid() );

  if ( group2.datasetCount() > 1 )
  {
    if ( group1.datasetCount() == 1 )
    {
      const std::shared_ptr<QgsMeshMemoryDataset> o0 = group1.datasets[0];
      Q_ASSERT( o0 );
      for ( int i = 1; i < group2.datasetCount(); ++i )
      {
        std::shared_ptr<QgsMeshMemoryDataset> o = copy( o0 );
        o->time = mTimes[i];
        group1.addDataset( o );
      }
    }
  }
}


std::shared_ptr<QgsMeshMemoryDataset>  QgsMeshCalcUtils::canditateDataset(
  QgsMeshMemoryDatasetGroup &group,
  int datasetIndex ) const
{
  Q_ASSERT( isValid() );

  if ( group.datasetCount() > 1 )
  {
    Q_ASSERT( group.datasetCount() > datasetIndex );
    return group.datasets[datasetIndex];
  }
  else
  {
    Q_ASSERT( group.datasetCount() == 1 );
    return group.datasets[0];
  }
}

std::shared_ptr<const QgsMeshMemoryDataset>  QgsMeshCalcUtils::constCandidateDataset(
  const QgsMeshMemoryDatasetGroup &group,
  int datasetIndex ) const
{
  Q_ASSERT( isValid() );

  if ( group.datasetCount() > 1 )
  {
    Q_ASSERT( group.datasetCount() > datasetIndex );
    return group.constDataset( datasetIndex );
  }
  else
  {
    Q_ASSERT( group.datasetCount() == 1 );
    return group.constDataset( 0 );
  }
}

int  QgsMeshCalcUtils::datasetCount(
  const QgsMeshMemoryDatasetGroup &group1,
  const QgsMeshMemoryDatasetGroup &group2 ) const
{
  Q_ASSERT( isValid() );

  if ( ( group1.datasetCount() > 1 ) || ( group2.datasetCount() > 1 ) )
  {
    return mTimes.size();
  }
  else
  {
    return 1;
  }
}

void QgsMeshCalcUtils::func1( QgsMeshMemoryDatasetGroup &group,
                              std::function<double( double )> func ) const
{
  Q_ASSERT( isValid() );

  for ( int time_index = 0; time_index < group.datasetCount(); ++time_index )
  {
    std::shared_ptr<QgsMeshMemoryDataset> output = canditateDataset( group, time_index );

    for ( int n = 0; n < output->values.size(); ++n )
    {
      double val1 = output->values[n].scalar();
      double res_val = D_NODATA;
      if ( !std::isnan( val1 ) )
        res_val = func( val1 );
      output->values[n] = res_val;
    }

    if ( group.type == QgsMeshDatasetGroupMetadata::DataOnVertices )
      activate( output );
  }
}


void QgsMeshCalcUtils::func2( QgsMeshMemoryDatasetGroup &group1,
                              const QgsMeshMemoryDatasetGroup &group2,
                              std::function<double( double, double )> func ) const
{
  Q_ASSERT( isValid() );
  Q_ASSERT( group1.type == group2.type ); // we do not support mixed output types

  expand( group1, group2 );

  for ( int time_index = 0; time_index < datasetCount( group1, group2 ); ++time_index )
  {
    std::shared_ptr<QgsMeshMemoryDataset> o1 = canditateDataset( group1, time_index );
    std::shared_ptr<const QgsMeshMemoryDataset> o2 = constCandidateDataset( group2, time_index );

    for ( int n = 0; n < o2->values.size(); ++n )
    {
      double val1 = o1->values[n].scalar();
      double val2 = o2->values[n].scalar();
      double res_val = D_NODATA;
      if ( !std::isnan( val1 ) && !std::isnan( val2 ) )
        res_val = func( val1, val2 );
      o1->values[n] = res_val;
    }

    if ( group1.type == QgsMeshDatasetGroupMetadata::DataOnVertices )
    {
      activate( o1, o2 );
    }

  }
}

void QgsMeshCalcUtils::funcAggr(
  QgsMeshMemoryDatasetGroup &group1,
  std::function<double( QVector<double>& )> func
) const
{
  Q_ASSERT( isValid() );

  if ( group1.type == QgsMeshDatasetGroupMetadata::DataOnVertices )
  {
    std::shared_ptr<QgsMeshMemoryDataset> output = QgsMeshCalcUtils::create( QgsMeshDatasetGroupMetadata::DataOnVertices );
    output->time = mTimes[0];
    for ( int n = 0; n < mMeshLayer->dataProvider()->vertexCount(); ++n )
    {
      QVector < double > vals;
      for ( int datasetIndex = 0; datasetIndex < group1.datasetCount(); ++datasetIndex )
      {
        const std::shared_ptr<QgsMeshMemoryDataset> o1 = canditateDataset( group1, datasetIndex );

        double val1 = o1->values[n].scalar();
        // ideally we should take only values from cells that are active.
        // but the problem is that the node can be part of multiple cells,
        // few active and few not, ...
        if ( !std::isnan( val1 ) )
        {
          vals.push_back( val1 );
        }
      }

      double res_val = D_NODATA;
      if ( !vals.isEmpty() )
      {
        res_val = func( vals );
      }

      output->values[n] = res_val;
    }

    // lets do activation purely on NODATA values as we did aggregation here
    activate( output );

    group1.datasets.clear();
    group1.datasets.push_back( output );

  }
  else
  {
    std::shared_ptr<QgsMeshMemoryDataset> output = QgsMeshCalcUtils::create( QgsMeshDatasetGroupMetadata::DataOnFaces );
    output->time = mTimes[0];

    int facesCount = mMeshLayer->dataProvider()->faceCount();
    output->values.resize( facesCount );

    for ( int n = 0; n < mMeshLayer->dataProvider()->faceCount(); ++n )
    {
      QVector < double > vals;
      for ( int datasetIndex = 0; datasetIndex < group1.datasetCount(); ++datasetIndex )
      {
        const std::shared_ptr<QgsMeshMemoryDataset> o1 = canditateDataset( group1, datasetIndex );
        double val1 = o1->values[n].scalar();
        if ( !std::isnan( val1 ) )
        {
          vals.push_back( val1 );
        }
      }

      double res_val = D_NODATA;
      if ( !vals.isEmpty() )
      {
        res_val = func( vals );
      }

      output->values[n] = res_val;
    }

    group1.datasets.clear();
    group1.datasets.push_back( output );
  }
}

const QgsTriangularMesh *QgsMeshCalcUtils::triangularMesh() const
{
  updateMesh();
  Q_ASSERT( mMeshLayer->triangularMesh() );
  return mMeshLayer->triangularMesh();
}

const QgsMesh *QgsMeshCalcUtils::nativeMesh() const
{
  updateMesh();
  Q_ASSERT( mMeshLayer->nativeMesh() );
  return mMeshLayer->nativeMesh();
}

void QgsMeshCalcUtils::updateMesh() const
{
  if ( ! mMeshLayer->nativeMesh() )
  {
    // we do not care about triangles,
    // we just want transformed coordinates
    // of the native mesh. So create
    // some dummy triangular mesh.
    QgsMapSettings mapSettings;
    mapSettings.setExtent( mMeshLayer->extent() );
    mapSettings.setDestinationCrs( mMeshLayer->crs() );
    mapSettings.setOutputDpi( 96 );
    QgsRenderContext context = QgsRenderContext::fromMapSettings( mapSettings );
    mMeshLayer->createMapRenderer( context );
  }
}

void QgsMeshCalcUtils::addIf( QgsMeshMemoryDatasetGroup &trueGroup,
                              const QgsMeshMemoryDatasetGroup &falseGroup,
                              const QgsMeshMemoryDatasetGroup &condition ) const
{
  Q_ASSERT( isValid() );

  // Make sure we have enough outputs in the resulting dataset
  expand( trueGroup, condition );
  expand( trueGroup, falseGroup );

  Q_ASSERT( trueGroup.type == falseGroup.type ); // we do not support mixed output types
  Q_ASSERT( trueGroup.type == condition.type ); // we do not support mixed output types

  for ( int time_index = 0; time_index < trueGroup.datasetCount(); ++time_index )
  {
    std::shared_ptr<QgsMeshMemoryDataset> true_o = canditateDataset( trueGroup, time_index );
    std::shared_ptr<const QgsMeshMemoryDataset> false_o = constCandidateDataset( falseGroup, time_index );
    std::shared_ptr<const QgsMeshMemoryDataset> condition_o = constCandidateDataset( condition, time_index );
    for ( int n = 0; n < true_o->values.size(); ++n )
    {
      double conditionValue =  condition_o->values[n].scalar();
      double resultValue = D_NODATA;
      if ( !std::isnan( conditionValue ) )
      {
        if ( qgsDoubleNear( conditionValue, D_TRUE ) )
          resultValue = true_o->values[n].scalar();
        else
          resultValue = false_o->values[n].scalar();
      }
      true_o->values[n] = resultValue;
    }

    if ( trueGroup.type == QgsMeshDatasetGroupMetadata::DataOnVertices )
    {
      // This is not ideal, as we do not check for true/false branch here in activate
      // problem is that activate is on elements, but condition is on nodes...
      activate( true_o, condition_o );
    }
  }
}


void QgsMeshCalcUtils::activate( QgsMeshMemoryDatasetGroup &group ) const
{
  Q_ASSERT( isValid() );

  if ( mOutputType == QgsMeshDatasetGroupMetadata::DataOnVertices )
  {
    for ( int datasetIndex = 0; datasetIndex < group.datasetCount(); ++datasetIndex )
    {
      std::shared_ptr<QgsMeshMemoryDataset> o1 = canditateDataset( group, datasetIndex );
      Q_ASSERT( group.type == QgsMeshDatasetGroupMetadata::DataOnVertices );
      activate( o1 );
    }
  }
  // Groups with data on faces do not have activate flags
}

void QgsMeshCalcUtils::activate(
  std::shared_ptr<QgsMeshMemoryDataset> dataset,
  std::shared_ptr<const QgsMeshMemoryDataset> refDataset /*=0*/
) const
{

  Q_ASSERT( isValid() );
  Q_ASSERT( dataset );

  // Activate only faces that ha some data and all vertices
  for ( int idx = 0; idx < mMeshLayer->dataProvider()->faceCount(); ++idx )
  {
    if ( refDataset && !refDataset->active.isEmpty() && ( !refDataset->active[idx] ) )
    {
      dataset->active[idx] = false;
      continue;
    }

    if ( !dataset->active[idx] )
    {
      continue;
    }

    QgsMeshFace face = nativeMesh()->face( idx );

    bool isActive = true; //ACTIVE
    for ( int j = 0; j < face.size(); ++j )
    {
      if ( std::isnan( dataset->values[face[j]].scalar() ) )
      {
        isActive = false; //NOT ACTIVE
        break;
      }
    }
    dataset->active[idx] = isActive;
  }
}

double QgsMeshCalcUtils::ffilter( double val1, double filter ) const
{
  Q_ASSERT( !std::isnan( val1 ) );

  if ( qgsDoubleNear( filter, D_TRUE ) )
    return val1;
  else
    return D_NODATA;
}

double QgsMeshCalcUtils::fadd( double val1, double val2 ) const
{
  Q_ASSERT( !std::isnan( val1 ) );
  Q_ASSERT( !std::isnan( val2 ) );
  return val1 + val2;

}

double QgsMeshCalcUtils::fsubtract( double val1, double val2 ) const
{
  Q_ASSERT( !std::isnan( val1 ) );
  Q_ASSERT( !std::isnan( val2 ) );
  return val1 - val2;

}

double QgsMeshCalcUtils::fmultiply( double val1, double val2 ) const
{
  Q_ASSERT( !std::isnan( val1 ) );
  Q_ASSERT( !std::isnan( val2 ) );
  return val1 * val2;

}

double QgsMeshCalcUtils::fdivide( double val1, double val2 ) const
{
  Q_ASSERT( !std::isnan( val1 ) );
  Q_ASSERT( !std::isnan( val2 ) );
  if ( qgsDoubleNear( val2, 0.0 ) )
    return D_NODATA;
  else
    return val1 / val2;

}

double QgsMeshCalcUtils::fpower( double val1, double val2 ) const
{
  Q_ASSERT( !std::isnan( val1 ) );
  Q_ASSERT( !std::isnan( val2 ) );
  return pow( val1, val2 );

}

double QgsMeshCalcUtils::fequal( double val1, double val2 ) const
{
  Q_ASSERT( !std::isnan( val1 ) );
  Q_ASSERT( !std::isnan( val2 ) );
  if ( qgsDoubleNear( val1, val2 ) )
  {
    return D_TRUE;
  }
  else
  {
    return D_FALSE;
  }

}

double QgsMeshCalcUtils::fnotEqual( double val1, double val2 ) const
{
  Q_ASSERT( !std::isnan( val1 ) );
  Q_ASSERT( !std::isnan( val2 ) );
  if ( qgsDoubleNear( val1, val2 ) )
  {
    return D_FALSE;
  }
  else
  {
    return D_TRUE;
  }

}

double QgsMeshCalcUtils::fgreaterThan( double val1, double val2 ) const
{
  Q_ASSERT( !std::isnan( val1 ) );
  Q_ASSERT( !std::isnan( val2 ) );
  if ( val1 > val2 )
  {
    return D_TRUE;
  }
  else
  {
    return D_FALSE;
  }

}

double QgsMeshCalcUtils::flesserThan( double val1, double val2 ) const
{
  Q_ASSERT( !std::isnan( val1 ) );
  Q_ASSERT( !std::isnan( val2 ) );
  if ( val1 < val2 )
  {
    return D_TRUE;
  }
  else
  {
    return D_FALSE;
  }

}

double QgsMeshCalcUtils::flesserEqual( double val1, double val2 ) const
{
  Q_ASSERT( !std::isnan( val1 ) );
  Q_ASSERT( !std::isnan( val2 ) );
  if ( val1 <= val2 )
  {
    return D_TRUE;
  }
  else
  {
    return D_FALSE;
  }

}

double QgsMeshCalcUtils::fgreaterEqual( double val1, double val2 ) const
{
  Q_ASSERT( !std::isnan( val1 ) );
  Q_ASSERT( !std::isnan( val2 ) );
  if ( val1 >= val2 )
  {
    return D_TRUE;
  }
  else
  {
    return D_FALSE;
  }

}


double QgsMeshCalcUtils::flogicalAnd( double val1, double val2 ) const
{
  Q_ASSERT( !std::isnan( val1 ) );
  Q_ASSERT( !std::isnan( val2 ) );
  bool bval1 = qgsDoubleNear( val1, D_TRUE );
  bool bval2 = qgsDoubleNear( val2, D_TRUE );
  if ( bval1 && bval2 )
    return D_TRUE;
  else
    return D_FALSE;

}

double QgsMeshCalcUtils::flogicalOr( double val1, double val2 ) const
{
  Q_ASSERT( !std::isnan( val1 ) );
  Q_ASSERT( !std::isnan( val2 ) );
  bool bval1 = qgsDoubleNear( val1, D_TRUE );
  bool bval2 = qgsDoubleNear( val2, D_TRUE );
  if ( bval1 || bval2 )
    return D_TRUE;
  else
    return D_FALSE;

}

double QgsMeshCalcUtils::flogicalNot( double val1 ) const
{
  Q_ASSERT( !std::isnan( val1 ) );
  bool bval1 = qgsDoubleNear( val1, D_TRUE );
  if ( bval1 )
    return D_FALSE;
  else
    return D_TRUE;

}

double QgsMeshCalcUtils::fchangeSign( double val1 ) const
{
  Q_ASSERT( !std::isnan( val1 ) );
  return -val1;
}

double QgsMeshCalcUtils::fmin( double val1, double val2 ) const
{
  Q_ASSERT( !std::isnan( val1 ) );
  if ( val1 > val2 )
  {
    return val2;
  }
  else
  {
    return val1;
  }
}


double QgsMeshCalcUtils::fmax( double val1, double val2 ) const
{
  Q_ASSERT( !std::isnan( val1 ) );
  Q_ASSERT( !std::isnan( val2 ) );
  if ( val1 < val2 )
  {
    return val2;
  }
  else
  {
    return val1;
  }

}

double QgsMeshCalcUtils::fabs( double val1 ) const
{
  Q_ASSERT( !std::isnan( val1 ) );
  if ( val1 > 0 )
  {
    return val1;
  }
  else
  {
    return -val1;
  }

}

double QgsMeshCalcUtils::fsumAggregated( QVector<double> &vals ) const
{
  Q_ASSERT( !vals.contains( D_NODATA ) );
  Q_ASSERT( !vals.isEmpty() );
  return std::accumulate( vals.begin(), vals.end(), 0.0 );
}

double QgsMeshCalcUtils::fminimumAggregated( QVector<double> &vals ) const
{
  Q_ASSERT( !vals.contains( D_NODATA ) );
  Q_ASSERT( !vals.isEmpty() );
  return *std::min_element( vals.begin(), vals.end() );
}

double QgsMeshCalcUtils::fmaximumAggregated( QVector<double> &vals ) const
{
  Q_ASSERT( !vals.contains( D_NODATA ) );
  Q_ASSERT( !vals.isEmpty() );
  return *std::max_element( vals.begin(), vals.end() );
}

double QgsMeshCalcUtils::faverageAggregated( QVector<double> &vals ) const
{
  Q_ASSERT( !vals.contains( D_NODATA ) );
  Q_ASSERT( !vals.isEmpty() );
  return fsumAggregated( vals ) / vals.size();
}

void QgsMeshCalcUtils::logicalNot( QgsMeshMemoryDatasetGroup &group1 ) const
{
  return func1( group1, std::bind( & QgsMeshCalcUtils::flogicalNot, this, std::placeholders::_1 ) );
}

void QgsMeshCalcUtils::changeSign( QgsMeshMemoryDatasetGroup &group1 ) const
{
  return func1( group1, std::bind( & QgsMeshCalcUtils::fchangeSign, this, std::placeholders::_1 ) );
}

void QgsMeshCalcUtils::abs( QgsMeshMemoryDatasetGroup &group1 ) const
{
  return func1( group1, std::bind( & QgsMeshCalcUtils::fabs, this, std::placeholders::_1 ) );
}

void QgsMeshCalcUtils::add( QgsMeshMemoryDatasetGroup &group1, const QgsMeshMemoryDatasetGroup &group2 ) const
{
  return func2( group1, group2, std::bind( & QgsMeshCalcUtils::fadd, this, std::placeholders::_1, std::placeholders::_2 ) );
}

void QgsMeshCalcUtils::subtract( QgsMeshMemoryDatasetGroup &group1, const QgsMeshMemoryDatasetGroup &group2 ) const
{
  return func2( group1, group2, std::bind( & QgsMeshCalcUtils::fsubtract, this, std::placeholders::_1, std::placeholders::_2 ) );
}

void QgsMeshCalcUtils::multiply( QgsMeshMemoryDatasetGroup &group1, const QgsMeshMemoryDatasetGroup &group2 ) const
{
  return func2( group1, group2, std::bind( & QgsMeshCalcUtils::fmultiply, this, std::placeholders::_1, std::placeholders::_2 ) );
}

void QgsMeshCalcUtils::divide( QgsMeshMemoryDatasetGroup &group1, const QgsMeshMemoryDatasetGroup &group2 ) const
{
  return func2( group1, group2, std::bind( & QgsMeshCalcUtils::fdivide, this, std::placeholders::_1, std::placeholders::_2 ) );
}

void QgsMeshCalcUtils::power( QgsMeshMemoryDatasetGroup &group1, const QgsMeshMemoryDatasetGroup &group2 ) const
{
  return func2( group1, group2, std::bind( & QgsMeshCalcUtils::fpower, this, std::placeholders::_1, std::placeholders::_2 ) );
}

void QgsMeshCalcUtils::equal( QgsMeshMemoryDatasetGroup &group1, const QgsMeshMemoryDatasetGroup &group2 ) const
{
  return func2( group1, group2, std::bind( & QgsMeshCalcUtils::fequal, this, std::placeholders::_1, std::placeholders::_2 ) );
}

void QgsMeshCalcUtils::notEqual( QgsMeshMemoryDatasetGroup &group1, const QgsMeshMemoryDatasetGroup &group2 ) const
{
  return func2( group1, group2, std::bind( & QgsMeshCalcUtils::fnotEqual, this, std::placeholders::_1, std::placeholders::_2 ) );
}

void QgsMeshCalcUtils::greaterThan( QgsMeshMemoryDatasetGroup &group1, const QgsMeshMemoryDatasetGroup &group2 ) const
{
  return func2( group1, group2, std::bind( & QgsMeshCalcUtils::fgreaterThan, this, std::placeholders::_1, std::placeholders::_2 ) );
}

void QgsMeshCalcUtils::lesserThan( QgsMeshMemoryDatasetGroup &group1, const QgsMeshMemoryDatasetGroup &group2 ) const
{
  return func2( group1, group2, std::bind( & QgsMeshCalcUtils::flesserThan, this, std::placeholders::_1, std::placeholders::_2 ) );
}

void QgsMeshCalcUtils::lesserEqual( QgsMeshMemoryDatasetGroup &group1, const QgsMeshMemoryDatasetGroup &group2 ) const
{
  return func2( group1, group2, std::bind( & QgsMeshCalcUtils::flesserEqual, this, std::placeholders::_1, std::placeholders::_2 ) );
}

void QgsMeshCalcUtils::greaterEqual( QgsMeshMemoryDatasetGroup &group1, const QgsMeshMemoryDatasetGroup &group2 ) const
{
  return func2( group1, group2, std::bind( & QgsMeshCalcUtils::fgreaterEqual, this, std::placeholders::_1, std::placeholders::_2 ) );
}

void QgsMeshCalcUtils::logicalAnd( QgsMeshMemoryDatasetGroup &group1, const QgsMeshMemoryDatasetGroup &group2 ) const
{
  return func2( group1, group2, std::bind( & QgsMeshCalcUtils::flogicalAnd, this, std::placeholders::_1, std::placeholders::_2 ) );
}

void QgsMeshCalcUtils::logicalOr( QgsMeshMemoryDatasetGroup &group1, const QgsMeshMemoryDatasetGroup &group2 ) const
{
  return func2( group1, group2, std::bind( & QgsMeshCalcUtils::flogicalOr, this, std::placeholders::_1, std::placeholders::_2 ) );
}

void QgsMeshCalcUtils::minimum( QgsMeshMemoryDatasetGroup &group1, const QgsMeshMemoryDatasetGroup &group2 ) const
{
  return func2( group1, group2, std::bind( & QgsMeshCalcUtils::fmin, this, std::placeholders::_1, std::placeholders::_2 ) );
}

void QgsMeshCalcUtils::maximum( QgsMeshMemoryDatasetGroup &group1, const QgsMeshMemoryDatasetGroup &group2 ) const
{
  return func2( group1, group2, std::bind( & QgsMeshCalcUtils::fmax, this, std::placeholders::_1, std::placeholders::_2 ) );
}

void QgsMeshCalcUtils::filter( QgsMeshMemoryDatasetGroup &group1, const QgsRectangle &extent ) const
{
  QgsMeshMemoryDatasetGroup filter( "filter" );
  populateSpatialFilter( filter, extent );
  return func2( group1, filter, std::bind( & QgsMeshCalcUtils::ffilter, this, std::placeholders::_1, std::placeholders::_2 ) );
}

void QgsMeshCalcUtils::filter( QgsMeshMemoryDatasetGroup &group1, const QgsGeometry &mask ) const
{
  QgsMeshMemoryDatasetGroup filter( "filter" );
  populateMaskFilter( filter, mask );
  return func2( group1, filter, std::bind( & QgsMeshCalcUtils::ffilter, this, std::placeholders::_1, std::placeholders::_2 ) );
}

void QgsMeshCalcUtils::sumAggregated( QgsMeshMemoryDatasetGroup &group1 ) const
{
  return funcAggr( group1, std::bind( & QgsMeshCalcUtils::fsumAggregated, this, std::placeholders::_1 ) );
}

void QgsMeshCalcUtils::minimumAggregated( QgsMeshMemoryDatasetGroup &group1 ) const
{
  return funcAggr( group1, std::bind( & QgsMeshCalcUtils::fminimumAggregated, this, std::placeholders::_1 ) );
}

void QgsMeshCalcUtils::maximumAggregated( QgsMeshMemoryDatasetGroup &group1 ) const
{
  return funcAggr( group1, std::bind( & QgsMeshCalcUtils::fmaximumAggregated, this, std::placeholders::_1 ) );
}

void QgsMeshCalcUtils::averageAggregated( QgsMeshMemoryDatasetGroup &group1 ) const
{
  return funcAggr( group1, std::bind( & QgsMeshCalcUtils::faverageAggregated, this, std::placeholders::_1 ) );
}

///@endcond
