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

#if 0
  void AbstractTool::createOutputFileWriter( const QString &fileName, const QgsVectorLayer *layerA, const QgsVectorLayer *layerB, OutputFields outputFields, OutputCrs outputCrs, const QString &outputDriverName )
  {
    mOutWkbType = layerA->wkbType();

    QgsFields fields;
    switch ( outputFields )
    {
      case FieldsA:
        fields = layerA->fields();
        break;
      case FieldsB:
        fields = layerB->fields();
        break;
      case FieldsAandB:
        fields = layerA->fields();

        QList<QString> names;
        for ( const QgsField &field : fields.toList() )
        {
          names.append( field.name() );
        }

        for ( const QgsField &field : layerB->fields().toList() )
        {
          QString name = field.name();
          for ( int count = 0; names.contains( name ); ++count )
          {
            name = QString( "%1_%2" ).arg( field.name() ).arg( count );
          }
          fields.append( QgsField( name, field.type() ) );
        }
        break;
    }

    QgsCoordinateReferenceSystem crs;
    switch ( outputCrs )
    {
      case CrsLayerA:
        crs = layerA->crs();
        break;
      case CrsLayerB:
        crs = layerB->crs();
        break;
    }

    mNumOutFields = fields.size();
    //mOutputWriter = new QgsVectorFileWriter( fileName, layerA->dataProvider()->encoding(), fields, layerA->wkbType(), crs, outputDriverName );
  }
#endif //0

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
      feature->geometry().convertGeometryCollectionToSubclass( QgsWkbTypes::geometryType( mOutWkbType ) );
      // Skip incompatible geometries
      if ( QgsWkbTypes::singleType( feature->geometry().wkbType() ) != QgsWkbTypes::singleType( mOutWkbType ) )
      {
        QgsDebugMsg( QString( "Skipping incompatible geometry: %1 %2" ).arg( feature->geometry().wkbType() ).arg( mOutWkbType ) );
        continue;
      }
      // If output type is a singleType, create features for each single geometry
      else if ( mOutWkbType == QgsWkbTypes::singleType( mOutWkbType ) )
      {
        for ( QgsGeometry geometry : feature->geometry().asGeometryCollection() )
        {
          QgsFeature f;
          f.setGeometry( geometry );
          f.setAttributes( feature->attributes() );
          if ( !mOutput->addFeature( f ) )
          {
            //mWriteErrors.append( mOutput->errorMessage() );
          }
        }
      }
      else
      {
        if ( !mOutput->addFeature( *feature ) )
        {
          //mWriteErrors.append( mOutput->errorMessage() );
        }
      }
    }
  }

} // Geoprocessing
