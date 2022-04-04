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
#include "qgsmessagelog.h"
#include "qgssettings.h"
#include <QtConcurrent>
#include <functional>

const QList<QString> QgsLocator::CORE_FILTERS = QList<QString>() << QStringLiteral( "actions" )
    <<  QStringLiteral( "processing_alg" )
    <<  QStringLiteral( "layertree" )
    <<  QStringLiteral( "layouts" )
    <<  QStringLiteral( "features" )
    <<  QStringLiteral( "allfeatures" )
    <<  QStringLiteral( "calculator" )
    <<  QStringLiteral( "bookmarks" )
    <<  QStringLiteral( "optionpages" )
    <<  QStringLiteral( "edit_features" )
    <<  QStringLiteral( "goto" )
    <<  QStringLiteral( "nominatimgeocoder" ) ;

QgsLocator::QgsLocator( QObject *parent )
  : QObject( parent )
{
  qRegisterMetaType<QgsLocatorResult>( "QgsLocatorResult" );
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

QList<QgsLocatorFilter *> QgsLocator::filters( const QString &prefix )
{
  if ( !prefix.isEmpty() )
  {
    QList<QgsLocatorFilter *> filters =  QList<QgsLocatorFilter *>();
    for ( QgsLocatorFilter *filter : mFilters )
    {
      if ( !filter->activePrefix().isEmpty() && filter->activePrefix().compare( prefix, Qt::CaseInsensitive ) == 0 )
      {
        filters << filter;
      }
    }
    return filters;
  }
  else
  {
    return mFilters;
  }
}

QMap<QString, QgsLocatorFilter *> QgsLocator::prefixedFilters() const
{
  QMap<QString, QgsLocatorFilter *> filters = QMap<QString, QgsLocatorFilter *>();
  for ( QgsLocatorFilter *filter : mFilters )
  {
    if ( !filter->activePrefix().isEmpty() && filter->enabled() )
    {
#if QT_VERSION < QT_VERSION_CHECK(5, 15, 0)
      filters.insertMulti( filter->activePrefix(), filter );
#else
      filters.insert( filter->activePrefix(), filter );
#endif
    }
  }
  return filters;
}

void QgsLocator::registerFilter( QgsLocatorFilter *filter )
{
  mFilters.append( filter );
  filter->setParent( this );

  // restore settings
  bool enabled = QgsLocator::settingsLocatorFilterEnabled.value( filter->name() );
  bool byDefault = QgsLocator::settingsLocatorFilterDefault.valueWithDefaultOverride( filter->useWithoutPrefix(), filter->name() );
  QString prefix = QgsLocator::settingsLocatorFilterPrefix.valueWithDefaultOverride( filter->prefix(), filter->name() );
  if ( prefix.isEmpty() )
  {
    prefix = filter->prefix();
  }

  if ( !prefix.isEmpty() )
  {
    if ( CORE_FILTERS.contains( filter->name() ) )
    {
      //inbuilt filter, no prefix check
      filter->setActivePrefix( prefix );
    }
    else if ( prefix.length() >= 3 || prefix != filter->prefix() )
    {
      // for plugins either the native prefix is >3 char or it has been customized by user
      filter->setActivePrefix( prefix );
    }
    else
    {
      // otherwise set it to empty string (not NULL)
      filter->setActivePrefix( QString( "" ) );
    }
  }

  filter->setEnabled( enabled );
  filter->setUseWithoutPrefix( byDefault );
}

void QgsLocator::fetchResults( const QString &string, const QgsLocatorContext &c, QgsFeedback *feedback )
{
  mAutocompletionList.clear();

  QgsLocatorContext context( c );
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

  QList< QgsLocatorFilter * > activeFilters;
  QString searchString = string;
  QString prefix = searchString.left( std::max( static_cast<int>( searchString.indexOf( ' ' ) ), 0 ) );
  if ( !prefix.isEmpty() )
  {
    for ( QgsLocatorFilter *filter : std::as_const( mFilters ) )
    {
      if ( filter->activePrefix().compare( prefix, Qt::CaseInsensitive ) == 0 && filter->enabled() )
      {
        activeFilters << filter;
      }
    }
    context.usingPrefix = !activeFilters.empty();
  }
  if ( !activeFilters.isEmpty() )
  {
    searchString = searchString.mid( prefix.length() + 1 );
  }
  else
  {
    for ( QgsLocatorFilter *filter : std::as_const( mFilters ) )
    {
      if ( filter->useWithoutPrefix() && filter->enabled() )
      {
        activeFilters << filter;
      }
    }
  }

  QList< QgsLocatorFilter *> threadedFilters;
  for ( QgsLocatorFilter *filter : std::as_const( activeFilters ) )
  {
    filter->clearPreviousResults();
    std::unique_ptr< QgsLocatorFilter > clone( filter->clone() );
    if ( ! clone )
    {
      QgsMessageLog::logMessage( tr( "QgsLocatorFilter '%1' could not provide a valid clone" ).arg( filter->name() ), QString(), Qgis::MessageLevel::Critical );
      continue;
    }
    connect( clone.get(), &QgsLocatorFilter::resultFetched, clone.get(), [this, filter]( QgsLocatorResult result )
    {
      result.filter = filter;
      filterSentResult( result );
    } );
    QStringList autoCompleteList = clone->prepare( searchString, context );
    if ( context.usingPrefix )
    {
      for ( int i = 0; i < autoCompleteList.length(); i++ )
      {
        autoCompleteList[i].prepend( QStringLiteral( "%1 " ).arg( prefix ) );
      }
    }
    mAutocompletionList.append( autoCompleteList );

    if ( clone->flags() & QgsLocatorFilter::FlagFast )
    {
      // filter is fast enough to fetch results on the main thread
      clone->fetchResults( searchString, context, feedback );
    }
    else
    {
      threadedFilters.append( clone.release() );
    }
  }

  mActiveThreads.clear();
  for ( QgsLocatorFilter *filter : std::as_const( threadedFilters ) )
  {
    QThread *thread = new QThread();
    mActiveThreads.append( thread );
    filter->moveToThread( thread );
    connect( thread, &QThread::started, filter, [filter, searchString, context, feedback]
    {
      int delay = filter->fetchResultsDelay();
      while ( delay > 0 )
      {
        if ( feedback->isCanceled() )
          break;
        QThread::msleep( 50 );
        delay -= 50;
      }
      if ( !feedback->isCanceled() )
        filter->fetchResults( searchString, context, feedback );
      filter->emit finished();
    }, Qt::QueuedConnection );
    connect( filter, &QgsLocatorFilter::finished, thread, &QThread::quit );
    connect( filter, &QgsLocatorFilter::finished, filter, &QgsLocatorFilter::deleteLater );
    connect( thread, &QThread::finished, thread, [this, thread]
    {
      mActiveThreads.removeAll( thread );
      if ( mActiveThreads.empty() )
        emit finished();
    } );
    connect( thread, &QThread::finished, thread, &QThread::deleteLater );
    thread->start();
  }

  emit searchPrepared();

  if ( mActiveThreads.empty() )
    emit finished();
}

void QgsLocator::cancel()
{
  cancelRunningQuery();
}

void QgsLocator::cancelWithoutBlocking()
{
  if ( mFeedback )
    mFeedback->cancel();
}

bool QgsLocator::isRunning() const
{
  return !mActiveThreads.empty();
}

void QgsLocator::clearPreviousResults()
{
  for ( QgsLocatorFilter *filter : std::as_const( mFilters ) )
  {
    if ( filter->enabled() )
    {
      filter->clearPreviousResults();
    }
  }
}

void QgsLocator::filterSentResult( QgsLocatorResult result )
{
  // if query has been canceled then discard any results we receive
  if ( mFeedback->isCanceled() )
    return;

  emit foundResult( result );
}

void QgsLocator::cancelRunningQuery()
{
  if ( !mActiveThreads.empty() )
  {
    // cancel existing job
    mFeedback->cancel();
    while ( !mActiveThreads.empty() )
    {
      QCoreApplication::processEvents();
    }
  }
}
