/***************************************************************************
                         qgslocatorwidgetcore.cpp
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

#include "qgslocatorwidgetcore.h"
#include "qgslocator.h"
#include "qgslocatormodel.h"
#include "qgsmapcanvasinterface.h"
#include "qgsmapsettings.h"

QgsLocatorWidgetCore::QgsLocatorWidgetCore( QObject *parent )
  : QObject( parent )
  , mLocator( new QgsLocator( this ) )
  , mLocatorModel( new QgsLocatorModel( this ) )
{
  mProxyModel = new QgsLocatorProxyModel( mLocatorModel );
  mProxyModel->setSourceModel( mLocatorModel );

  connect( mLocator, &QgsLocator::foundResult, this, &QgsLocatorWidgetCore::addResult );
  connect( mLocator, &QgsLocator::finished, this, &QgsLocatorWidgetCore::searchFinished );
}

void QgsLocatorWidgetCore::setMapCanvasInterface( QgsMapCanvasInterface *canvasInterface )
{
  mCanvasInterface = canvasInterface;
}

bool QgsLocatorWidgetCore::isRunning() const
{
  return mIsRunning;
}

void QgsLocatorWidgetCore::setIsRunning( bool isRunning )
{
  if ( mIsRunning == isRunning )
    return;

  mIsRunning = isRunning;
  emit isRunningChanged();
}

void QgsLocatorWidgetCore::invalidateResults()
{
  mLocator->cancelWithoutBlocking();
  mLocatorModel->clear();
}

void QgsLocatorWidgetCore::addResult( const QgsLocatorResult &result )
{
  mLocatorModel->addResult( result );
  emit resultAdded();
}


void QgsLocatorWidgetCore::searchFinished()
{
  if ( mHasQueuedRequest )
  {
    // a queued request was waiting for this - run the queued search now
    QString nextSearch = mNextRequestedString;
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

void QgsLocatorWidgetCore::performSearch( const QString &text )
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

QgsLocator *QgsLocatorWidgetCore::locator() const
{
  return mLocator;
}

QgsLocatorProxyModel *QgsLocatorWidgetCore::proxyModel() const
{
  return mProxyModel;
}

bool QgsLocatorWidgetCore::hasQueueRequested() const
{
  return mHasQueuedRequest;
}

QgsLocatorContext QgsLocatorWidgetCore::createContext()
{
  QgsLocatorContext context;
  if ( mCanvasInterface )
  {
    context.targetExtent = mCanvasInterface->mapSettings().visibleExtent();
    context.targetExtentCrs = mCanvasInterface->mapSettings().destinationCrs();
  }
  return context;
}
