/***************************************************************************
                         qgslocatormodelbridge.cpp
                         ------------------
    begin                : November 2018
    copyright            : (C) 2018 by Denis Rouzaud
    email                : denis@opengis.ch
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "qgslocatormodelbridge.h"
#include "qgslocator.h"
#include "qgslocatormodel.h"


QgsLocatorModelBridge::QgsLocatorModelBridge( QObject *parent )
  : QObject( parent )
  , mLocator( new QgsLocator( this ) )
  , mLocatorModel( new QgsLocatorModel( this ) )
{
  mProxyModel = new QgsLocatorProxyModel( mLocatorModel );
  mProxyModel->setSourceModel( mLocatorModel );

  connect( mLocator, &QgsLocator::foundResult, this, &QgsLocatorModelBridge::addResult );
  connect( mLocator, &QgsLocator::finished, this, &QgsLocatorModelBridge::searchFinished );
}

bool QgsLocatorModelBridge::isRunning() const
{
  return mIsRunning;
}

void QgsLocatorModelBridge::triggerResult( const QModelIndex &index, const int actionId )
{
  mLocator->clearPreviousResults();
  QgsLocatorResult result = mProxyModel->data( index, QgsLocatorModel::ResultDataRole ).value< QgsLocatorResult >();
  if ( result.filter )
  {
    if ( actionId >= 0 )
      result.filter->triggerResultFromAction( result, actionId );
    else
      result.filter->triggerResult( result );
  }
}

void QgsLocatorModelBridge::setIsRunning( bool isRunning )
{
  if ( mIsRunning == isRunning )
    return;

  mIsRunning = isRunning;
  emit isRunningChanged();
}

void QgsLocatorModelBridge::invalidateResults()
{
  mLocator->cancelWithoutBlocking();
  mLocatorModel->clear();
}

void QgsLocatorModelBridge::updateCanvasExtent( const QgsRectangle &extent )
{
  mCanvasExtent = extent;
}

void QgsLocatorModelBridge::updateCanvasCrs( const QgsCoordinateReferenceSystem &crs )
{
  mCanvasCrs = crs;
}

void QgsLocatorModelBridge::addResult( const QgsLocatorResult &result )
{
  mLocatorModel->addResult( result );
  emit resultAdded();
}


void QgsLocatorModelBridge::searchFinished()
{
  if ( mHasQueuedRequest )
  {
    // a queued request was waiting for this - run the queued search now
    const QString nextSearch = mNextRequestedString;
    mNextRequestedString.clear();
    mHasQueuedRequest = false;
    performSearch( nextSearch );
  }
  else
  {
    if ( !mLocator->isRunning() )
      setIsRunning( false );
  }
}

void QgsLocatorModelBridge::performSearch( const QString &text )
{
  setIsRunning( true );

  if ( mLocator->isRunning() )
  {
    // can't do anything while a query is running, and can't block
    // here waiting for the current query to cancel
    // so we queue up this string until cancel has happened
    mLocator->cancelWithoutBlocking();
    mNextRequestedString = text;
    mHasQueuedRequest = true;
    return;
  }
  else
  {
    emit resultsCleared();
    mLocatorModel->deferredClear();
    mLocator->fetchResults( text, createContext() );
  }
}

QgsLocator *QgsLocatorModelBridge::locator() const
{
  return mLocator;
}

QgsLocatorProxyModel *QgsLocatorModelBridge::proxyModel() const
{
  return mProxyModel;
}

bool QgsLocatorModelBridge::hasQueueRequested() const
{
  return mHasQueuedRequest;
}

QgsLocatorContext QgsLocatorModelBridge::createContext()
{
  QgsLocatorContext context;
  context.targetExtent = mCanvasExtent;
  context.targetExtentCrs = mCanvasCrs;
  context.transformContext = mTransformContext;
  return context;
}
