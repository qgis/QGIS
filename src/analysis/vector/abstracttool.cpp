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

  AbstractTool::AbstractTool( QgsFeatureSink *output, QgsWkbTypes::Type outWkbType, double precision ): mOutput( output ), mOutWkbType( outWkbType ), mPrecision( precision )
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
    QgsFeature currentFeature;
    QgsFeatureRequest request;
    request.setFlags( QgsFeatureRequest::SubsetOfAttributes );
    QgsFeatureIterator it = layer->getFeatures();
    while ( it.nextFeature( currentFeature ) )
    {
      index.addFeature( currentFeature );
    }
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

  QVector<QgsFeature *> AbstractTool::getIntersects( const QgsRectangle &rect, QgsSpatialIndex &index, QgsFeatureSource *layer, const QgsAttributeList &attIdx )
  {
    mIntersectMutex.lock();
    QList<QgsFeatureId> intersectIds = index.intersects( rect );
    mIntersectMutex.unlock();

    QVector<QgsFeature *> featureList;
    for ( QgsFeatureId id : intersectIds )
    {
      QgsFeature *feature = new QgsFeature();
      if ( !getFeatureAtId( *feature, id, layer, attIdx ) )
      {
        delete feature;
        continue;
      }
      featureList.append( feature );
    }
    return featureList;
  }

  void AbstractTool::writeFeatures( const QVector<QgsFeature *> &outFeatures )
  {
    QMutexLocker locker( &mWriteMutex );
    for ( QgsFeature *feature : outFeatures )
    {
      QgsGeometry fGeom = feature->geometry();
      fGeom.convertGeometryCollectionToSubclass( QgsWkbTypes::geometryType( mOutWkbType ) );

      // Skip incompatible geometries
      if ( QgsWkbTypes::singleType( fGeom.wkbType() ) != QgsWkbTypes::singleType( mOutWkbType ) )
      {
        QgsDebugMsg( QString( "Skipping incompatible geometry: %1 %2" ).arg( fGeom.wkbType() ).arg( mOutWkbType ) );
        continue;
      }

      // If output type is a singleType, create features for each single geometry
      if ( mOutWkbType == QgsWkbTypes::singleType( mOutWkbType ) )
      {
        for ( QgsGeometry geometry : fGeom.asGeometryCollection() )
        {
          writeFeature( geometry, feature->attributes() );
        }
      }
      else
      {
        if ( QgsWkbTypes::singleType( mOutWkbType ) == fGeom.wkbType() )
        {
          fGeom.convertToMultiType();
        }
        writeFeature( fGeom, feature->attributes() );
      }
    }
  }

  void AbstractTool::writeFeature( const QgsGeometry &geom, const QgsAttributes &att )
  {
    if ( !mOutput || geom.isEmpty() )
    {
      return;
    }

    QgsFeature f;
    f.setGeometry( geom );
    f.setAttributes( att );
    if ( !mOutput->addFeature( f ) )
    {
      mWriteErrors.append( QObject::tr( "Failed to write feature to datasource" ) );
    }
  }

} // Geoprocessing
