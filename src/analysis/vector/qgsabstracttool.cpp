/***************************************************************************
 *  abstracttool.h                                                      *
 *  -------------------                                                    *
 *  begin                : Jun 10, 2014                                    *
 *  copyright            : (C) 2014 by Sandro Mani / Sourcepole AG         *
 *  email                : smani@sourcepole.ch                             *
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include <QApplication>
#include <QtConcurrentRun>
#include <QtConcurrentMap>
#include "qgsabstracttool.h"
#include "qgsfield.h"
#include "qgscoordinatereferencesystem.h"
#include "qgsvectordataprovider.h"
#include "qgsvectorfilewriter.h"
#include "qgsvectorlayer.h"
#include "qgsgeometry.h"
#include "qgslogger.h"
#include "qgswkbtypes.h"
#include "qgsgeos.h"

namespace Vectoranalysis
{
  void QgsAbstractTool::ProcessFeatureWrapper::operator()( const Job *job )
  {
    try
    {
      instance->processFeature( job );
    }
    catch ( const QgsException &e )
    {
      instance->mExceptions.append( e.what() );
    }
  }

  QgsAbstractTool::QgsAbstractTool( QgsFeatureSink *output, QgsCoordinateTransformContext transformContext, QgsFeatureRequest::InvalidGeometryCheck invalidGeometryCheck ): mOutput( output ), mTransformContext( transformContext ), mInvalidGeometryCheck( invalidGeometryCheck )
  {
  }

  QgsAbstractTool::~QgsAbstractTool()
  {
    qDeleteAll( mJobQueue );
  }

  QFuture<void> QgsAbstractTool::init()
  {
    return QtConcurrent::run( this, &QgsAbstractTool::prepare );
  }

  void QgsAbstractTool::run( QgsFeedback *feedback )
  {
    prepare();
    long processedFeatures = 0;

    while ( prepareNextChunk() )
    {
      QFuture<void> f = execute();
      f.waitForFinished();
      qDeleteAll( mJobQueue );
      mJobQueue.clear();

      processedFeatures += 100;
      feedback->setProgress( 100.0 * ( double )processedFeatures / ( double )mFeatureCount );

      if ( feedback->isCanceled() )
      {
        break;
      }
    }
  }

  QFuture<void> QgsAbstractTool::execute()
  {
    return QtConcurrent::map( mJobQueue, ProcessFeatureWrapper( this ) );
  }

  void QgsAbstractTool::buildSpatialIndex( QgsSpatialIndex &index, QgsFeatureSource *layer, const QgsCoordinateReferenceSystem &destCRS ) const
  {
    QgsFeatureRequest request;
    request.setFlags( QgsFeatureRequest::SubsetOfAttributes );
    request.setDestinationCrs( destCRS, mTransformContext );
    request.setSubsetOfAttributes( QgsAttributeList() );
    QgsFeatureIterator it = layer->getFeatures( request );
    index = QgsSpatialIndex( it );
  }

  bool QgsAbstractTool::appendNextChunkToJobQueue( QgsFeatureSource *layer, int taskFlag )
  {

    QgsFeature f;
    for ( int i = 0; i < mChunkSize; ++i )
    {
      if ( !mFeatureIterator.nextFeature( f ) )
      {
        if ( i == 0 )
        {
          return false;
        }
        break;
      }
      mJobQueue.append( new Job( f, taskFlag ) );
    }
    return true;
  }

  void QgsAbstractTool::writeFeatures( QgsFeatureList &outFeatures )
  {
    QMutexLocker locker( &mWriteMutex );

    if ( !mOutput )
    {
      return;
    }

    mOutput->addFeatures( outFeatures, QgsFeatureSink::FastInsert );
  }

  void QgsAbstractTool::prepareLayer( QgsFeatureSource *source, const QgsCoordinateReferenceSystem &destCRS, const QgsAttributeList *sourceFieldIndices )
  {
    QgsFeatureRequest request;
    if ( sourceFieldIndices )
    {
      request.setSubsetOfAttributes( *sourceFieldIndices );
    }
    request.setDestinationCrs( destCRS, mTransformContext );
    request.setInvalidGeometryCheck( mInvalidGeometryCheck );
    mFeatureIterator = source->getFeatures( request );
  }

} // Geoprocessing
