/***************************************************************************
    qgsgcptransformer.cpp
     --------------------------------------
    Date                 : February 2022
    Copyright            : (C) 2022 by Nyall Dawson
    Email                : nyall dot dawson at gmail dot com
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "qgsvectorwarper.h"
#include "qgsfeaturesink.h"
#include "qgsfeedback.h"
#include "qgsgcpgeometrytransformer.h"
#include "qgsfeaturesource.h"
#include "qgsvectorlayer.h"

#include <QObject>
#include <QFileInfo>

QgsVectorWarper::QgsVectorWarper( QgsGcpTransformerInterface::TransformMethod method, const QList<QgsGcpPoint> &points, const QgsCoordinateReferenceSystem &destinationCrs )
  : mMethod( method )
  , mPoints( points )
  , mDestinationCrs( destinationCrs )
{

}

bool QgsVectorWarper::transformFeatures( QgsFeatureIterator &iterator, QgsFeatureSink *sink, const QgsCoordinateTransformContext &context, QgsFeedback *feedback ) const
{
  if ( !sink )
    return false;

  QVector<QgsPointXY> sourcePoints;
  sourcePoints.reserve( mPoints.size() );
  QVector<QgsPointXY> destinationPoints;
  destinationPoints.reserve( mPoints.size() );
  for ( const QgsGcpPoint &gcpPoint : mPoints )
  {
    sourcePoints << gcpPoint.sourcePoint();
    destinationPoints << gcpPoint.transformedDestinationPoint( mDestinationCrs, context );
  }

  if ( feedback && feedback->isCanceled() )
    return false;

  QgsGcpGeometryTransformer transformer( mMethod, sourcePoints, destinationPoints );

  QgsFeature f;

  long long i = 0;
  while ( iterator.nextFeature( f ) )
  {
    if ( feedback )
    {
      if ( feedback->isCanceled() )
        break;

      feedback->setProcessedCount( i );
    }
    i++;

    QgsFeature outputFeature = f;
    bool ok = false;
    const QgsGeometry transformed = transformer.transform( f.geometry(), ok, feedback );
    if ( ok )
    {
      outputFeature.setGeometry( transformed );
      if ( !sink->addFeature( outputFeature, QgsFeatureSink::FastInsert ) )
      {
        mError = sink->lastError();
        return false;
      }
    }
    else
    {
      mError = QObject::tr( "An error occurred while transforming a feature" );
      return false;
    }
  }
  return true;
}



//
// QgsVectorWarperTask
//

QgsVectorWarperTask::QgsVectorWarperTask( QgsGcpTransformerInterface::TransformMethod method, const QList < QgsGcpPoint > &points,
    const QgsCoordinateReferenceSystem &destinationCrs,
    QgsVectorLayer *layer, const QString &fileName )
  : QgsTask( tr( "Warping %1" ).arg( fileName ), QgsTask::CanCancel )
  , mMethod( method )
  , mPoints( points )
  , mDestinationCrs( destinationCrs )
  , mDestFileName( fileName )
{
  if ( layer )
  {
    mTransformContext = layer->transformContext();
    mSource.reset( new QgsVectorLayerFeatureSource( layer ) );
    mFeatureCount = layer->featureCount();
    mFields = layer->fields();
    mWkbType = layer->wkbType();
  }
}

void QgsVectorWarperTask::cancel()
{
  if ( mFeedback )
    mFeedback->cancel();

  QgsTask::cancel();
}

bool QgsVectorWarperTask::run()
{
  mFeedback = std::make_unique< QgsFeedback >();

  QgsVectorFileWriter::SaveVectorOptions saveOptions;

  const QString fileExtension = QFileInfo( mDestFileName ).completeSuffix();
  saveOptions.driverName = QgsVectorFileWriter::driverForExtension( fileExtension );

  std::unique_ptr< QgsVectorFileWriter > exporter( QgsVectorFileWriter::create( mDestFileName, mFields, mWkbType, mDestinationCrs, mTransformContext, saveOptions ) );
  if ( exporter->hasError() )
  {
    mErrorMessage = exporter->errorMessage();
    mResult = Result::Error;
    return false;
  }

  QgsVectorWarper warper( mMethod, mPoints, mDestinationCrs );

  connect( mFeedback.get(), &QgsFeedback::processedCountChanged, this, [ = ]( long long count )
  {
    const double newProgress = 100.0 * count / mFeatureCount;
    // avoid flooding with too many events
    if ( static_cast< int >( newProgress * 10 ) != static_cast< int >( mLastProgress * 10 ) )
    {
      mLastProgress = newProgress;
      emit progressChanged( newProgress );
    }
  } );

  QgsFeatureIterator iterator = mSource->getFeatures();
  const bool res = warper.transformFeatures( iterator, exporter.get(), mTransformContext, mFeedback.get() );
  if ( !res )
  {
    mErrorMessage = warper.error();
    mResult = Result::Error;
  }

  mResult = mFeedback->isCanceled() ? Result::Canceled : Result::Success;
  mFeedback.reset();
  return mResult == Result::Success;
}

