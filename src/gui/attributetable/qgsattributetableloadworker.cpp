/***************************************************************************
  QgsAttributeTableLoadWorker.h - Worker loader for cached features
  -------------------
         date                 : November 2015
         copyright            : Alessandro Pasotti
         email                : elpaso (at) itopen (dot) it

 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "qgsattributetableloadworker.h"
#include "qgsfeature.h"
#include "qgsfeatureiterator.h"

QgsAttributeTableLoadWorker::QgsAttributeTableLoadWorker( const QgsFeatureIterator &features )
{
  QgsAttributeTableLoadWorker( features, 1000 );
}

QgsAttributeTableLoadWorker::QgsAttributeTableLoadWorker( const QgsFeatureIterator &features, int batchSize ):
    mIsRunning( false ),
    mStopped( false )
{
  mFeatures = features;
  mBatchSize = batchSize;
  QgsDebugMsg( "QgsAttributeTableLoadWorker created!" );
}

QgsAttributeTableLoadWorker::~QgsAttributeTableLoadWorker()
{
  QgsDebugMsg( "QgsAttributeTableLoadWorker destroyed!" );
}

void QgsAttributeTableLoadWorker::stopJob()
{
  mStopped = true;
  QgsDebugMsg( "QgsAttributeTableLoadWorker stopped!" );
}


void QgsAttributeTableLoadWorker::startJob()
{
  QgsDebugMsg( "QgsAttributeTableLoadWorker started!" );
  mStopped = false;
  mIsRunning = true;
  QgsFeature feat;
  QgsFeatureList features;
  int i = 0;
  while ( ! mStopped && mFeatures.nextFeature( feat ) )
  {
    i++;
    features.append( feat );
    if ( i % mBatchSize == 0 )
    {
      QgsDebugMsg( QString( "QgsAttributeTableLoadWorker featuresReady (batch: %1)" ).arg( i ) );
      emit featuresReady( features, i );
      qApp->processEvents();
      features.clear();
    }
  }
  // Remaining features?
  if ( ! features.isEmpty() )
  {
    QgsDebugMsg( QString( "QgsAttributeTableLoadWorker featuresReady (flush: %1)" ).arg( i ) );
    emit featuresReady( features, i );
    qApp->processEvents();
  }
  mIsRunning = false;
  QgsDebugMsg( "QgsAttributeTableLoadWorker finished!" );
  emit finished();
}
