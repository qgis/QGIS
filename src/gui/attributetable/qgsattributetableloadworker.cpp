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

QgsAttributeTableLoadWorker::QgsAttributeTableLoadWorker( const QgsFeatureIterator &features ):
    mIsRunning( FALSE ),
    mStopped( FALSE )
{
  QgsDebugMsg( "QgsAttributeTableLoadWorker created!" );
  mFeatures = features;
}

QgsAttributeTableLoadWorker::~QgsAttributeTableLoadWorker()
{
  QgsDebugMsg( "QgsAttributeTableLoadWorker destroyed!" );
}

void QgsAttributeTableLoadWorker::stopJob()
{
  mStopped = TRUE;
  QgsDebugMsg( "QgsAttributeTableLoadWorker stopped!" );
}


void QgsAttributeTableLoadWorker::startJob()
{
  QgsDebugMsg( "QgsAttributeTableLoadWorker started!" );
  mStopped = FALSE;
  mIsRunning = TRUE;
  QgsFeature feat;
  QgsFeatureList features;
  int i = 0;
  while ( ! mStopped && mFeatures.nextFeature( feat ) )
  {
    i++;
    features.append( feat );
    if ( i % 1000 == 0 )
    {

      emit featuresReady( features, i );
      qApp->processEvents();
      features.clear();
    }
  }
  // Remaining features?
  if ( i % 1000 )
  {
    emit featuresReady( features, i );
    qApp->processEvents();
  }
  mIsRunning = FALSE;
  emit finished();
}
