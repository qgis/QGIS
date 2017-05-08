/***************************************************************************
                         qgslocator.cpp
                         --------------
    begin                : May 2017
    copyright            : (C) 2017 by Nyall Dawson
    email                : nyall dot dawson at gmail dot com
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "qgslocator.h"
#include <QtConcurrent>
#include <functional>

QgsLocator::QgsLocator( QObject *parent )
  : QObject( parent )
{
  qRegisterMetaType<QgsLocatorResult>( "QgsLocatorResult" );

  connect( &mFutureWatcher, &QFutureWatcher<void>::finished, this, &QgsLocator::finished );
}

QgsLocator::~QgsLocator()
{
  cancelRunningQuery();
  qDeleteAll( mFilters );
}

void QgsLocator::deregisterFilter( QgsLocatorFilter *filter )
{
  cancelRunningQuery();
  mFilters.removeAll( filter );
  delete filter;
}

QList<QgsLocatorFilter *> QgsLocator::filters()
{
  return mFilters;
}

void QgsLocator::registerFilter( QgsLocatorFilter *filter )
{
  mFilters.append( filter );
  filter->setParent( this );
  connect( filter, &QgsLocatorFilter::resultFetched, this, &QgsLocator::filterSentResult, Qt::QueuedConnection );
}

void QgsLocator::fetchResults( const QString &string, const QgsLocatorContext &context, QgsFeedback *feedback )
{
  // ideally this should not be required, as well behaved callers
  // will NOT fire up a new fetchResults call while an existing one is
  // operating/waiting to be canceled...
  cancelRunningQuery();

  // if no feedback object was passed, create one that is owned by this object
  // to ensure that filters ALWAYS receive a valid feedback
  if ( !feedback )
  {
    mOwnedFeedback.reset( new QgsFeedback() );
    feedback = mOwnedFeedback.get();
  }
  else
  {
    mOwnedFeedback.reset( nullptr );
  }
  mFeedback = feedback;

  auto gatherFilterResults = [string, feedback]( QgsLocatorFilter * filter )
  {
    if ( !feedback->isCanceled() )
      filter->fetchResults( string, context, feedback );
  };

  mFuture = QtConcurrent::map( mFilters, gatherFilterResults );
  mFutureWatcher.setFuture( mFuture );
}

void QgsLocator::cancel()
{
  cancelRunningQuery();
}

void QgsLocator::cancelWithoutBlocking()
{
  mFeedback->cancel();
}

bool QgsLocator::isRunning() const
{
  return mFuture.isRunning();
}

void QgsLocator::filterSentResult( QgsLocatorResult result )
{
  // if query has been canceled then discard any results we receive
  if ( mFeedback->isCanceled() )
    return;

  if ( !result.filter )
  {
    // filter forgot to set itself
    result.filter = qobject_cast< QgsLocatorFilter *>( sender() );
  }

  emit foundResult( result );
}

void QgsLocator::cancelRunningQuery()
{
  if ( mFuture.isRunning() )
  {
    // cancel existing job
    mFeedback->cancel();
    mFuture.cancel();
    mFuture.waitForFinished();
  }
}
