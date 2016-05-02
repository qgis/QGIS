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

#include "qgsgeometrychecker.h"
#include "checks/qgsgeometrycheck.h"
#include "utils/qgsfeaturepool.h"

#include <QtConcurrentMap>
#include <QFutureWatcher>
#include <QMutex>
#include <QTimer>


QgsGeometryChecker::QgsGeometryChecker( const QList<QgsGeometryCheck*>& checks, QgsFeaturePool *featurePool )
    : mChecks( checks )
    , mFeaturePool( featurePool )
    , mMergeAttributeIndex( -1 )
{
}

QgsGeometryChecker::~QgsGeometryChecker()
{
  qDeleteAll( mCheckErrors );
  qDeleteAll( mChecks );
}

QFuture<void> QgsGeometryChecker::execute( int *totalSteps )
{
  if ( totalSteps )
  {
    *totalSteps = 0;
    int nCheckFeatures = mFeaturePool->getFeatureIds().size();
    Q_FOREACH ( QgsGeometryCheck* check, mChecks )
    {
      if ( check->getCheckType() <= QgsGeometryCheck::FeatureCheck )
      {
        *totalSteps += nCheckFeatures;
      }
      else
      {
        *totalSteps += 1;
      }
    }
  }

  QFuture<void> future = QtConcurrent::map( mChecks, RunCheckWrapper( this ) );

  QFutureWatcher<void>* watcher = new QFutureWatcher<void>();
  watcher->setFuture( future );
  QTimer* timer = new QTimer();
  connect( timer, SIGNAL( timeout() ), this, SLOT( emitProgressValue() ) );
  connect( watcher, SIGNAL( finished() ), timer, SLOT( deleteLater() ) );
  connect( watcher, SIGNAL( finished() ), watcher, SLOT( deleteLater() ) );
  timer->start( 500 );

  return future;
}

void QgsGeometryChecker::emitProgressValue()
{
  emit progressValue( mProgressCounter );
}

bool QgsGeometryChecker::fixError( QgsGeometryCheckError* error, int method )
{
  mMessages.clear();
  if ( error->status() >= QgsGeometryCheckError::StatusFixed )
  {
    return true;
  }

  QgsGeometryCheck::Changes changes;
  QgsRectangle recheckArea = error->affectedAreaBBox();

  error->check()->fixError( error, method, mMergeAttributeIndex, changes );
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
  QgsFeatureIds recheckFeatures;
  Q_FOREACH ( QgsFeatureId id, changes.keys() )
  {
    bool removed = false;
    Q_FOREACH ( const QgsGeometryCheck::Change& change, changes.value( id ) )
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
      if ( mFeaturePool->get( id, f ) )
      {
        recheckFeatures.insert( id );
        recheckArea.unionRect( f.geometry()->boundingBox() );
      }
    }
  }
  // - Determine extent to recheck for gaps
  Q_FOREACH ( QgsGeometryCheckError* err, mCheckErrors )
  {
    if ( err->check()->getCheckType() == QgsGeometryCheck::LayerCheck )
    {
      if ( err->affectedAreaBBox().intersects( recheckArea ) )
      {
        recheckArea.unionRect( err->affectedAreaBBox() );
      }
    }
  }
  recheckArea.grow( 10 * QgsGeometryCheckPrecision::tolerance() );
  QgsFeatureIds recheckAreaFeatures = mFeaturePool->getIntersects( recheckArea );

  // If only selected features were checked, confine the recheck areas to the selected features
  if ( mFeaturePool->getSelectedOnly() )
  {
    recheckAreaFeatures = recheckAreaFeatures.intersect( mFeaturePool->getLayer()->selectedFeaturesIds() );
  }

  // Recheck feature / changed area to detect new errors
  QList<QgsGeometryCheckError*> recheckErrors;
  Q_FOREACH ( const QgsGeometryCheck* check, mChecks )
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

  // Remove just-fixed error from newly-found errors (needed in case error was fixed with "no change")
  Q_FOREACH ( QgsGeometryCheckError* recheckErr, recheckErrors )
  {
    if ( recheckErr->isEqual( error ) )
    {
      recheckErrors.removeAll( recheckErr );
      delete recheckErr;
      break;
    }
  }

  // Go through error list, update other errors of the checked feature
  Q_FOREACH ( QgsGeometryCheckError* err, mCheckErrors )
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
    QgsGeometryCheckError* matchErr = nullptr;
    int nMatch = 0;
    Q_FOREACH ( QgsGeometryCheckError* recheckErr, recheckErrors )
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
           ( err->check()->getCheckType() <= QgsGeometryCheck::FeatureCheck && recheckFeatures.contains( err->featureId() ) ) ||
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
  Q_FOREACH ( QgsGeometryCheckError* recheckErr, recheckErrors )
  {
    emit errorAdded( recheckErr );
    mCheckErrors.append( recheckErr );
  }

  return true;
}

void QgsGeometryChecker::runCheck( const QgsGeometryCheck* check )
{
  // Run checks
  QList<QgsGeometryCheckError*> errors;
  QStringList messages;
  check->collectErrors( errors, messages, &mProgressCounter );
  mErrorListMutex.lock();
  mCheckErrors.append( errors );
  mMessages.append( messages );
  mErrorListMutex.unlock();
  Q_FOREACH ( QgsGeometryCheckError* error, errors )
  {
    emit errorAdded( error );
  }
}
