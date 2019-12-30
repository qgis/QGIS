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
#include "abstracttool.h"
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

  QString AbstractTool::errFeatureDoesNotExist = QApplication::translate( "AbstractTool", "The requested feature does not exist" );
  QString AbstractTool::errFailedToFetchGeometry = QApplication::translate( "AbstractTool", "The feature geometry could not be fetched" );

  void AbstractTool::ProcessFeatureWrapper::operator()( const Job *job )
  {
    try
    {
      instance->processFeature( job );
    }
    catch ( const std::exception &e )
    {
      instance->mExceptions.append( e.what() );
      throw e;
    }
  }

  AbstractTool::AbstractTool( QgsFeatureSink *output, QgsWkbTypes::Type outWkbType, QgsCoordinateTransformContext transformContext, double precision ): mOutput( output ), mPrecision( precision ), mTransformContext( transformContext )
  {
  }

  AbstractTool::~AbstractTool()
  {
    qDeleteAll( mJobQueue );
  }

  QFuture<void> AbstractTool::init()
  {
    return QtConcurrent::run( this, &AbstractTool::prepare );
  }

  QFuture<void> AbstractTool::execute( int /*task*/ )
  {
    return QtConcurrent::map( mJobQueue, ProcessFeatureWrapper( this ) );
  }

  void AbstractTool::buildSpatialIndex( QgsSpatialIndex &index, QgsFeatureSource *layer ) const
  {
    QgsFeatureRequest request;
    request.setFlags( QgsFeatureRequest::SubsetOfAttributes );
    request.setSubsetOfAttributes( QgsAttributeList() );
    QgsFeatureIterator it = layer->getFeatures( request );
    index = QgsSpatialIndex( it );
  }

  void AbstractTool::appendToJobQueue( QgsFeatureSource *layer, int taskFlag )
  {
    for ( const QgsFeatureId &id : layer->allFeatureIds() )
    {
      mJobQueue.append( new Job( id, taskFlag ) );
    }
  }

  bool AbstractTool::getFeatureAtId( QgsFeature &feature, QgsFeatureId id, QgsFeatureSource *layer, const QgsAttributeList &attIdx )
  {
    QgsFeatureRequest request( id );
    request.setSubsetOfAttributes( attIdx );
    if ( !layer->getFeatures( request ).nextFeature( feature ) )
    {
      reportInvalidFeatureError( layer, id, errFeatureDoesNotExist );
      return false;
    }
    else if ( !feature.hasGeometry() )
    {
      reportInvalidFeatureError( layer, id, errFailedToFetchGeometry );
      return false;
    }
    return true;
  }

  void AbstractTool::writeFeatures( QgsFeatureList &outFeatures )
  {
    QMutexLocker locker( &mWriteMutex );

    if ( !mOutput )
    {
      return;
    }

    mOutput->addFeatures( outFeatures, QgsFeatureSink::FastInsert );
  }

} // Geoprocessing
