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

#include <QtConcurrentMap>
#include <QFutureWatcher>
#include <QMutex>
#include <QTimer>
#include <QTextStream>

#include "qgsgeometrycheckcontext.h"
#include "qgsgeometrychecker.h"
#include "qgsgeometrycheck.h"
#include "qgsfeaturepool.h"
#include "qgsproject.h"
#include "qgsvectorlayer.h"
#include "qgsgeometrycheckerror.h"



QgsGeometryChecker::QgsGeometryChecker( const QList<QgsGeometryCheck *> &checks, QgsGeometryCheckContext *context, const QMap<QString, QgsFeaturePool *> &featurePools )
  : mChecks( checks )
  , mContext( context )
  , mFeaturePools( featurePools )
{
  for ( auto it = featurePools.constBegin(); it != mFeaturePools.constEnd(); ++it )
  {
    if ( it.value()->layer() )
    {
      it.value()->layer()->setReadOnly( true );
      // Enter update mode to defer ogr dataset repacking until the checker has finished
      it.value()->layer()->dataProvider()->enterUpdateMode();
    }
  }
}

QgsGeometryChecker::~QgsGeometryChecker()
{
  qDeleteAll( mCheckErrors );
  qDeleteAll( mChecks );
  for ( auto it = mFeaturePools.constBegin(); it != mFeaturePools.constEnd(); ++it )
  {
    if ( it.value()->layer() )
    {
      it.value()->layer()->dataProvider()->leaveUpdateMode();
      it.value()->layer()->setReadOnly( false );
    }
    delete it.value();
  }
  delete mContext;
}

QFuture<void> QgsGeometryChecker::execute( int *totalSteps )
{
  if ( totalSteps )
  {
    *totalSteps = 0;
    for ( QgsGeometryCheck *check : std::as_const( mChecks ) )
    {
      for ( auto it = mFeaturePools.constBegin(); it != mFeaturePools.constEnd(); ++it )
      {
        if ( check->checkType() <= QgsGeometryCheck::FeatureCheck )
        {
          *totalSteps += check->isCompatible( it.value()->layer() ) ? it.value()->allFeatureIds().size() : 0;
        }
        else
        {
          *totalSteps += 1;
        }
      }
    }
  }
  QTimer *timer = new QTimer();
  connect( timer, &QTimer::timeout, this, &QgsGeometryChecker::emitProgressValue );
  QFutureWatcher<void> *watcher = new QFutureWatcher<void>();
  connect( watcher, &QFutureWatcherBase::finished, timer, &QObject::deleteLater );
  connect( watcher, &QFutureWatcherBase::finished, watcher, &QObject::deleteLater );

  QFuture<void> future = QtConcurrent::map( mChecks, RunCheckWrapper( this ) );
  watcher->setFuture( future );
  timer->start( 500 );

  return future;
}

void QgsGeometryChecker::emitProgressValue()
{
  emit progressValue( mFeedback.progress() );
}

bool QgsGeometryChecker::fixError( QgsGeometryCheckError *error, int method, bool triggerRepaint )
{
  mMessages.clear();
  if ( error->status() >= QgsGeometryCheckError::StatusFixed )
  {
    return true;
  }
#if 0
  QTextStream( stdout ) << "Fixing " << error->description() << ": " << error->layerId() << ":" << error->featureId() << " @[" << error->vidx().part << ", " << error->vidx().ring << ", " << error->vidx().vertex << "](" << error->location().x() << ", " << error->location().y() << ") = " << error->value().toString() << endl;
#endif

  QgsGeometryCheck::Changes changes;
  QgsRectangle recheckArea = error->affectedAreaBBox();

  error->check()->fixError( mFeaturePools, error, method, mMergeAttributeIndices, changes );
#if 0
  QTextStream( stdout ) << " * Status: " << error->resolutionMessage() << endl;
  static QVector<QString> strChangeWhat = { "ChangeFeature", "ChangePart", "ChangeRing", "ChangeNode" };
  static QVector<QString> strChangeType = { "ChangeAdded", "ChangeRemoved", "ChangeChanged" };
  for ( const QString &layerId : changes.keys() )
  {
    for ( const QgsFeatureId &fid : changes[layerId].keys() )
    {
      for ( const QgsGeometryCheck::Change &change : changes[layerId][fid] )
      {
        QTextStream( stdout ) << " * Change: " << layerId << ":" << fid << " :: " << strChangeWhat[change.what] << ":" << strChangeType[change.type] << ":(" << change.vidx.part << "," << change.vidx.ring << "," << change.vidx.vertex << ")" << endl;
      }
    }
  }
#endif
  emit errorUpdated( error, true );
  if ( error->status() != QgsGeometryCheckError::StatusFixed )
  {
    return false;
  }

  // If nothing was changed, stop here
  if ( changes.isEmpty() )
  {
    return true;
  }

  // Determine what to recheck
  // - Collect all features which were changed, get affected area
  QMap<QString, QSet<QgsFeatureId>> recheckFeatures;
  for ( auto it = changes.constBegin(); it != changes.constEnd(); ++it )
  {
    const QMap<QgsFeatureId, QList<QgsGeometryCheck::Change>> &layerChanges = it.value();
    QgsFeaturePool *featurePool = mFeaturePools[it.key()];
    QgsCoordinateTransform t( featurePool->layer()->crs(), mContext->mapCrs, QgsProject::instance() );
    t.setBallparkTransformsAreAppropriate( true );
    for ( auto layerChangeIt = layerChanges.constBegin(); layerChangeIt != layerChanges.constEnd(); ++layerChangeIt )
    {
      bool removed = false;
      for ( const QgsGeometryCheck::Change &change : layerChangeIt.value() )
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
        if ( featurePool->getFeature( layerChangeIt.key(), f ) )
        {
          recheckFeatures[it.key()].insert( layerChangeIt.key() );
          recheckArea.combineExtentWith( t.transformBoundingBox( f.geometry().boundingBox() ) );
        }
      }
    }
  }
  // - Determine extent to recheck for gaps
  for ( QgsGeometryCheckError *err : std::as_const( mCheckErrors ) )
  {
    if ( err->check()->checkType() == QgsGeometryCheck::LayerCheck )
    {
      if ( err->affectedAreaBBox().intersects( recheckArea ) )
      {
        recheckArea.combineExtentWith( err->affectedAreaBBox() );
      }
    }
  }
  recheckArea.grow( 10 * mContext->tolerance );
  QMap<QString, QgsFeatureIds> recheckAreaFeatures;
  for ( auto it = mFeaturePools.constBegin(); it != mFeaturePools.constEnd(); it++ )
  {
    QgsFeaturePool *featurePool = it.value();
    QgsCoordinateTransform t( mContext->mapCrs, featurePool->layer()->crs(), QgsProject::instance() );
    recheckAreaFeatures[it.key()] = featurePool->getIntersects( t.transform( recheckArea ) );
  }

  // Recheck feature / changed area to detect new errors
  QList<QgsGeometryCheckError *> recheckErrors;
  for ( const QgsGeometryCheck *check : std::as_const( mChecks ) )
  {
    if ( check->checkType() == QgsGeometryCheck::LayerCheck )
    {
      if ( !recheckAreaFeatures.isEmpty() )
      {
        check->collectErrors( mFeaturePools, recheckErrors, mMessages, nullptr, recheckAreaFeatures );
      }
    }
    else
    {
      if ( !recheckFeatures.isEmpty() )
      {
        check->collectErrors( mFeaturePools, recheckErrors, mMessages, nullptr, recheckFeatures );
      }
    }
  }

  // Go through error list, update other errors of the checked feature
  for ( QgsGeometryCheckError *err : std::as_const( mCheckErrors ) )
  {
    if ( err == error || err->status() == QgsGeometryCheckError::StatusObsolete )
    {
      continue;
    }

    QgsGeometryCheckError::Status oldStatus = err->status();

    bool handled = err->handleChanges( changes );

    // Check if this error now matches one found when rechecking the feature/area
    QgsGeometryCheckError *matchErr = nullptr;
    int nMatch = 0;
    for ( QgsGeometryCheckError *recheckErr : std::as_const( recheckErrors ) )
    {
      if ( recheckErr->isEqual( err ) || recheckErr->closeMatch( err ) )
      {
        ++nMatch;
        matchErr = recheckErr;
      }
    }
    // If just one close match was found, take it
    if ( nMatch == 1 && matchErr )
    {
      err->update( matchErr );
      emit errorUpdated( err, err->status() != oldStatus );
      recheckErrors.removeAll( matchErr );
      delete matchErr;
      continue;
    }

    // If no match is found and the error is not fixed or obsolete, set it to obsolete if...
    if ( err->status() < QgsGeometryCheckError::StatusFixed &&
         (
           // changes weren't handled
           !handled ||
           // or if it is a FeatureNodeCheck or FeatureCheck error whose feature was rechecked
           ( err->check()->checkType() <= QgsGeometryCheck::FeatureCheck && recheckFeatures[err->layerId()].contains( err->featureId() ) ) ||
           // or if it is a LayerCheck error within the rechecked area
           ( err->check()->checkType() == QgsGeometryCheck::LayerCheck && recheckArea.contains( err->affectedAreaBBox() ) )
         )
       )
    {
      err->setObsolete();
      emit errorUpdated( err, err->status() != oldStatus );
    }
  }

  // Add new errors
  for ( QgsGeometryCheckError *recheckErr : std::as_const( recheckErrors ) )
  {
    emit errorAdded( recheckErr );
    mCheckErrors.append( recheckErr );
  }

  if ( triggerRepaint )
  {
    for ( auto itChange = changes.constBegin(); itChange != changes.constEnd(); itChange++ )
    {
      mFeaturePools[itChange.key()]->layer()->triggerRepaint();
    }
  }

  return true;
}

void QgsGeometryChecker::runCheck( const QMap<QString, QgsFeaturePool *> &featurePools, const QgsGeometryCheck *check )
{
  // Run checks
  QList<QgsGeometryCheckError *> errors;
  QStringList messages;
  check->collectErrors( featurePools, errors, messages, &mFeedback );
  mErrorListMutex.lock();
  mCheckErrors.append( errors );
  mMessages.append( messages );
  mErrorListMutex.unlock();
  for ( QgsGeometryCheckError *error : std::as_const( errors ) )
  {
    emit errorAdded( error );
  }
}

QgsGeometryChecker::RunCheckWrapper::RunCheckWrapper( QgsGeometryChecker *instance )
  : mInstance( instance )
{
}

void QgsGeometryChecker::RunCheckWrapper::operator()( const QgsGeometryCheck *check )
{
  mInstance->runCheck( mInstance->mFeaturePools, check );
}
