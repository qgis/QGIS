/***************************************************************************
 *  qgsgeometrychecker.cpp                                                 *
 *  -------------------                                                    *
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

#include "qgscrscache.h"
#include "qgsgeometrychecker.h"
#include "checks/qgsgeometrycheck.h"
#include "utils/qgsfeaturepool.h"

#include <QtConcurrentMap>
#include <QFutureWatcher>
#include <QMutex>
#include <QTimer>


QgsGeometryChecker::QgsGeometryChecker( const QList<QgsGeometryCheck *> &checks, QgsGeometryCheckerContext *context )
  : mChecks( checks )
  , mContext( context )
{
  for ( const QString &layerId : mContext->featurePools.keys() )
  {
    connect( mContext->featurePools[layerId], SIGNAL( featureIdsChanged( QString, QMap<QgsFeatureId, QgsFeatureId> ) ), this, SLOT( updateFeatureIds( QString, QMap<QgsFeatureId, QgsFeatureId> ) ) );
  }
  for ( const QgsFeaturePool *featurePool : mContext->featurePools.values() )
  {
    if ( featurePool->getLayer() )
    {
      featurePool->getLayer()->setReadOnly( true );
      // Enter update mode to defer ogr dataset repacking until the checker has finished
      featurePool->getLayer()->dataProvider()->enterUpdateMode();
    }
  }
}

QgsGeometryChecker::~QgsGeometryChecker()
{
  qDeleteAll( mCheckErrors );
  qDeleteAll( mChecks );
  for ( const QgsFeaturePool *featurePool : mContext->featurePools.values() )
  {
    if ( featurePool->getLayer() )
    {
      featurePool->getLayer()->dataProvider()->leaveUpdateMode();
      featurePool->getLayer()->setReadOnly( false );
    }
    delete featurePool;
  }
  delete mContext;
}

QFuture<void> QgsGeometryChecker::execute( int *totalSteps )
{
  if ( totalSteps )
  {
    *totalSteps = 0;
    for ( QgsGeometryCheck *check : mChecks )
    {
      for ( const QgsFeaturePool *featurePool : mContext->featurePools.values() )
      {
        if ( check->getCheckType() <= QgsGeometryCheck::FeatureCheck )
        {
          *totalSteps += check->getCompatibility( featurePool->getLayer()->geometryType() ) ? featurePool->getFeatureIds().size() : 0;
        }
        else
        {
          *totalSteps += 1;
        }
      }
    }
  }

  QFuture<void> future = QtConcurrent::map( mChecks, RunCheckWrapper( this ) );

  QFutureWatcher<void> *watcher = new QFutureWatcher<void>();
  watcher->setFuture( future );
  QTimer *timer = new QTimer();
  connect( timer, &QTimer::timeout, this, &QgsGeometryChecker::emitProgressValue );
  connect( watcher, &QFutureWatcherBase::finished, timer, &QObject::deleteLater );
  connect( watcher, &QFutureWatcherBase::finished, watcher, &QObject::deleteLater );
  timer->start( 500 );

  return future;
}

void QgsGeometryChecker::emitProgressValue()
{
  emit progressValue( mProgressCounter );
}

bool QgsGeometryChecker::fixError( QgsGeometryCheckError *error, int method, bool triggerRepaint )
{
  mMessages.clear();
  if ( error->status() >= QgsGeometryCheckError::StatusFixed )
  {
    return true;
  }

  QgsGeometryCheck::Changes changes;
  QgsRectangle recheckArea = error->affectedAreaBBox();

  error->check()->fixError( error, method, mMergeAttributeIndices, changes );
  emit errorUpdated( error, true );
  if ( error->status() != QgsGeometryCheckError::StatusFixed )
  {
    return false;
  }

  if ( error->resolutionMessage() == tr( "No action" ) )
  {
    return true;
  }

  // Determine what to recheck
  // - Collect all features which were changed, get affected area
  QMap<QString, QSet<QgsFeatureId>> recheckFeatures;
  for ( const QString &layerId : changes.keys() )
  {
    const QMap<QgsFeatureId, QList<QgsGeometryCheck::Change>> &layerChanges = changes[layerId];
    QgsFeaturePool *featurePool = mContext->featurePools[layerId];
    QgsCoordinateTransform t = QgsCoordinateTransformCache::instance()->transform( featurePool->getLayer()->crs().authid(), mContext->mapCrs );
    for ( QgsFeatureId id : layerChanges.keys() )
    {
      bool removed = false;
      for ( const QgsGeometryCheck::Change &change : layerChanges.value( id ) )
      {
        if ( change.what == QgsGeometryCheck::ChangeFeature && change.type == QgsGeometryCheck::ChangeRemoved )
        {
          removed = true;
          break;
        }
      }
      if ( !removed )
      {
        QgsFeature f;
        if ( featurePool->get( id, f ) )
        {
          recheckFeatures[layerId].insert( id );
          recheckArea.combineExtentWith( t.transformBoundingBox( f.geometry().boundingBox() ) );
        }
      }
    }
  }
  // - Determine extent to recheck for gaps
  for ( QgsGeometryCheckError *err : mCheckErrors )
  {
    if ( err->check()->getCheckType() == QgsGeometryCheck::LayerCheck )
    {
      if ( err->affectedAreaBBox().intersects( recheckArea ) )
      {
        recheckArea.combineExtentWith( err->affectedAreaBBox() );
      }
    }
  }
  recheckArea.grow( 10 * mContext->tolerance );
  QMap<QString, QgsFeatureIds> recheckAreaFeatures;
  for ( const QString &layerId : mContext->featurePools.keys() )
  {
    QgsFeaturePool *featurePool = mContext->featurePools[layerId];
    QgsCoordinateTransform t = QgsCoordinateTransformCache::instance()->transform( mContext->mapCrs, featurePool->getLayer()->crs().authid() );
    recheckAreaFeatures[layerId] = featurePool->getIntersects( t.transform( recheckArea ) );
    // If only selected features were checked, confine the recheck areas to the selected features
    if ( featurePool->getSelectedOnly() )
    {
      recheckAreaFeatures[layerId] = recheckAreaFeatures[layerId].intersect( featurePool->getLayer()->selectedFeatureIds() );
    }
  }

  // Recheck feature / changed area to detect new errors
  QList<QgsGeometryCheckError *> recheckErrors;
  for ( const QgsGeometryCheck *check : mChecks )
  {
    if ( check->getCheckType() == QgsGeometryCheck::LayerCheck )
    {
      check->collectErrors( recheckErrors, mMessages, nullptr, recheckAreaFeatures );
    }
    else
    {
      check->collectErrors( recheckErrors, mMessages, nullptr, recheckFeatures );
    }
  }

  // Remove just-fixed error from newly-found errors if no changes occurred (needed in case error was fixed with "no change")
  if ( changes.isEmpty() )
  {
    for ( QgsGeometryCheckError *recheckErr : recheckErrors )
    {
      if ( recheckErr->isEqual( error ) )
      {
        recheckErrors.removeAll( recheckErr );
        delete recheckErr;
        break;
      }
    }
  }

  // Go through error list, update other errors of the checked feature
  for ( QgsGeometryCheckError *err : mCheckErrors )
  {
    if ( err == error || err->status() == QgsGeometryCheckError::StatusObsolete )
    {
      continue;
    }

    QgsGeometryCheckError::Status oldStatus = err->status();

    // Update error, if this fails, mark the errors as obsolete
    if ( !err->handleChanges( changes ) )
    {
      err->setObsolete();
      emit errorUpdated( err, err->status() != oldStatus );
      continue;
    }

    // Check if this error now matches one found when rechecking the feature/area
    QgsGeometryCheckError *matchErr = nullptr;
    int nMatch = 0;
    for ( QgsGeometryCheckError *recheckErr : recheckErrors )
    {
      if ( recheckErr->isEqual( err ) )
      {
        matchErr = recheckErr;
        nMatch = 1;
        break;
      }
      else if ( recheckErr->closeMatch( err ) )
      {
        ++nMatch;
        matchErr = recheckErr;
      }
    }
    if ( nMatch == 1 && matchErr )
    {
      err->update( matchErr );
      emit errorUpdated( err, err->status() != oldStatus );
      int nRemoved = recheckErrors.removeAll( matchErr );
      Q_UNUSED( nRemoved );
      Q_ASSERT( nRemoved == 1 );
      delete matchErr;
      continue;
    }

    // If no match is found and the error is not fixed or obsolete, set it to obsolete if...
    if ( err->status() < QgsGeometryCheckError::StatusFixed &&
         (
           // it is a FeatureNodeCheck or FeatureCheck error whose feature was rechecked
           ( err->check()->getCheckType() <= QgsGeometryCheck::FeatureCheck && recheckFeatures[err->layerId()].contains( err->featureId() ) ) ||
           // or if it is a LayerCheck error within the rechecked area
           ( err->check()->getCheckType() == QgsGeometryCheck::LayerCheck && recheckArea.contains( err->affectedAreaBBox() ) )
         )
       )
    {
      err->setObsolete();
      emit errorUpdated( err, err->status() != oldStatus );
    }
  }

  // Add new errors
  for ( QgsGeometryCheckError *recheckErr : recheckErrors )
  {
    emit errorAdded( recheckErr );
    mCheckErrors.append( recheckErr );
  }

  if ( triggerRepaint )
  {
    for ( const QString &layerId : changes.keys() )
    {
      mContext->featurePools[layerId]->getLayer()->triggerRepaint();
    }
  }

  return true;
}

void QgsGeometryChecker::runCheck( const QgsGeometryCheck *check )
{
  // Run checks
  QList<QgsGeometryCheckError *> errors;
  QStringList messages;
  check->collectErrors( errors, messages, &mProgressCounter );
  mErrorListMutex.lock();
  mCheckErrors.append( errors );
  mMessages.append( messages );
  mErrorListMutex.unlock();
  for ( QgsGeometryCheckError *error : errors )
  {
    emit errorAdded( error );
  }
}

void QgsGeometryChecker::updateFeatureIds( const QString &layerId, const QMap<QgsFeatureId, QgsFeatureId> &oldNewFid )
{
  for ( QgsGeometryCheckError *error : mCheckErrors )
  {
    if ( error->handleFidChanges( layerId, oldNewFid ) )
    {
      emit errorUpdated( error, false );
    }
  }
}
