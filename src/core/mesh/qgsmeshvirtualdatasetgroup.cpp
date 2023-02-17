/***************************************************************************
                         qgsmeshvirtualdatasetgroup.cpp
                         ---------------------
    begin                : June 2020
    copyright            : (C) 2020 by Vincent Cloarec
    email                : vcloarec at gmail dot com
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "qgsmeshvirtualdatasetgroup.h"

QgsMeshVirtualDatasetGroup::QgsMeshVirtualDatasetGroup(
  const QString &name,
  const QString &formulaString,
  QgsMeshLayer *layer,
  qint64 relativeStartTime,
  qint64 relativeEndTime ):
  QgsMeshDatasetGroup( name )
  , mFormula( formulaString )
  , mLayer( layer )
  , mStartTime( relativeStartTime )
  , mEndTime( relativeEndTime )
{
}

void QgsMeshVirtualDatasetGroup::initialize()
{
  QString errMessage;
  mCalcNode.reset( QgsMeshCalcNode::parseMeshCalcString( mFormula, errMessage ) );

  if ( !mCalcNode || !mLayer )
    return;

  mDatasetGroupNameUsed = mCalcNode->notAggregatedUsedDatasetGroupNames();
  mDatasetGroupNameUsedForAggregate = mCalcNode->aggregatedUsedDatasetGroupNames();
  setDataType( QgsMeshCalcUtils::determineResultDataType( mLayer,
               mDatasetGroupNameUsed + mDatasetGroupNameUsedForAggregate ) );

  //populate used group indexes
  QMap<QString, int> usedDatasetGroupindexes;
  const QList<int> &indexes = mLayer->datasetGroupsIndexes();
  for ( const int i : indexes )
  {
    const QString usedName = mLayer->datasetGroupMetadata( i ).name();
    if ( mDatasetGroupNameUsed.contains( usedName ) )
      usedDatasetGroupindexes[usedName] = i;
  }

  QSet<qint64> times;
  if ( !mCalcNode->isNonTemporal() )
  {
    //populate dataset index with time;
    const QList<int> &usedIndexes = usedDatasetGroupindexes.values();
    for ( const int groupIndex : usedIndexes )
    {
      const int dsCount = mLayer->datasetCount( groupIndex );
      if ( dsCount == 0 )
        return;

      if ( dsCount == 1 ) //non temporal dataset group
        continue;
      for ( int i = 0; i < dsCount; i++ )
      {
        const qint64 time = mLayer->datasetRelativeTimeInMilliseconds( QgsMeshDatasetIndex( groupIndex, i ) );
        if ( time != INVALID_MESHLAYER_TIME )
          times.insert( time );
      }
    }
  }

  if ( times.isEmpty() )
    times.insert( 0 );

  mDatasetTimes = QList<qint64>( times.constBegin(), times.constEnd() );
  std::sort( mDatasetTimes.begin(), mDatasetTimes.end() );

  mDatasetMetaData = QVector<QgsMeshDatasetMetadata>( mDatasetTimes.count() );

  //to fill metadata, calculate all the datasets one time
  int i = 0;
  while ( i < mDatasetTimes.count() )
  {
    mCurrentDatasetIndex = i;
    if ( calculateDataset() )
      ++i; //calculation succeeds
    else
      mDatasetTimes.removeAt( i ); //calculation fails, remove this time step
  }

  calculateStatistic();
}

int QgsMeshVirtualDatasetGroup::datasetCount() const
{
  return mDatasetTimes.count();
}

QgsMeshDataset *QgsMeshVirtualDatasetGroup::dataset( int index ) const
{
  if ( index < 0 || index >= mDatasetTimes.count() )
    return nullptr;

  if ( index != mCurrentDatasetIndex )
  {
    mCurrentDatasetIndex = index;
    calculateDataset();
  }

  return mCacheDataset.get();
}

QgsMeshDatasetMetadata QgsMeshVirtualDatasetGroup::datasetMetadata( int datasetIndex ) const
{
  if ( datasetIndex < 0 && datasetIndex >= mDatasetMetaData.count() )
    return QgsMeshDatasetMetadata();

  return mDatasetMetaData.at( datasetIndex );
}

QStringList QgsMeshVirtualDatasetGroup::datasetGroupNamesDependentOn() const
{
  return mDatasetGroupNameUsed;
}

QDomElement QgsMeshVirtualDatasetGroup::writeXml( QDomDocument &doc, const QgsReadWriteContext &context ) const
{
  Q_UNUSED( context )
  QDomElement elemDataset = doc.createElement( QStringLiteral( "mesh-dataset" ) );
  elemDataset.setAttribute( QStringLiteral( "source-type" ), QStringLiteral( "virtual" ) );
  elemDataset.setAttribute( QStringLiteral( "name" ), name() );
  elemDataset.setAttribute( QStringLiteral( "formula" ), mFormula );
  elemDataset.setAttribute( QStringLiteral( "start-time" ), mStartTime );
  elemDataset.setAttribute( QStringLiteral( "end-time" ), mEndTime );

  return elemDataset;
}

QString QgsMeshVirtualDatasetGroup::description() const
{
  return mFormula;
}

bool QgsMeshVirtualDatasetGroup::calculateDataset() const
{
  if ( !mLayer )
    return false;

  const QgsMeshCalcUtils dsu( mLayer,
                              mDatasetGroupNameUsed,
                              mDatasetGroupNameUsedForAggregate,
                              QgsInterval( mDatasetTimes[mCurrentDatasetIndex] / 1000.0 ),
                              QgsInterval( mStartTime / 1000.0 ),
                              QgsInterval( mEndTime / 1000.0 ) );

  if ( !dsu.isValid() )
    return false;

  //open output dataset
  std::unique_ptr<QgsMeshMemoryDatasetGroup> outputGroup = std::make_unique<QgsMeshMemoryDatasetGroup> ( QString(), dsu.outputType() );
  mCalcNode->calculate( dsu, *outputGroup );

  if ( outputGroup->memoryDatasets.isEmpty() )
    return false;

  mCacheDataset = outputGroup->memoryDatasets[0];
  if ( !mDatasetMetaData[mCurrentDatasetIndex].isValid() )
  {
    mCacheDataset->calculateMinMax();
    mCacheDataset->time = mDatasetTimes[mCurrentDatasetIndex] / 3600.0 / 1000.0;
    mDatasetMetaData[mCurrentDatasetIndex] = mCacheDataset->metadata();
  }

  return true;
}
