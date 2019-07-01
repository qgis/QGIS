/***************************************************************************
    qgsnewsfeedparser.cpp
    -------------------
    begin                : July 2019
    copyright            : (C) 2019 by Nyall Dawson
    email                : nyall dot dawson at gmail dot com
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/
#include "qgsnewsfeedparser.h"
#include "qgis.h"
#include "qgsnetworkcontentfetchertask.h"
#include "qgsnetworkaccessmanager.h"
#include "qgslogger.h"
#include "qgssettings.h"
#include "qgsjsonutils.h"
#include "qgsmessagelog.h"
#include "qgsapplication.h"
#include <QDateTime>


QgsNewsFeedParser::QgsNewsFeedParser( const QUrl &feedUrl, const QString &authcfg )
  : mBaseUrl( feedUrl.toString() )
  , mFeedUrl( feedUrl )
  , mAuthCfg( authcfg )
  , mSettingsKey( keyForFeed( mBaseUrl ) )
{
  // first thing we do is populate with existing entries
  readStoredEntries();

  QUrlQuery query( feedUrl );

  const uint after = QgsSettings().value( QStringLiteral( "%1/lastFetchTime" ).arg( mSettingsKey ), 0, QgsSettings::Core ).toUInt();
  if ( after > 0 )
    query.addQueryItem( QStringLiteral( "after" ), qgsDoubleToString( after, 0 ) );

  // bit of a hack to allow testing using local files
  if ( feedUrl.isLocalFile() )
  {
    if ( !query.toString().isEmpty() )
      mFeedUrl = QUrl( mFeedUrl.toString() + '_' + query.toString() );
  }
  else
  {
    mFeedUrl.setQuery( query ); // doesn't work for local file urls
  }
}

QList<QgsNewsFeedParser::Entry> QgsNewsFeedParser::entries() const
{
  return mEntries;
}

void QgsNewsFeedParser::fetch()
{
  QNetworkRequest req( mFeedUrl );
  QgsSetRequestInitiatorClass( req, QStringLiteral( "QgsNewsFeedParser" ) );

  mFetchStartTime = QDateTime::currentDateTimeUtc().toTime_t();

  QgsNetworkContentFetcherTask *task = new QgsNetworkContentFetcherTask( req, mAuthCfg );
  task->setDescription( tr( "Fetching News Feed" ) );
  connect( task, &QgsNetworkContentFetcherTask::fetched, this, [this, task]
  {
    QNetworkReply *reply = task->reply();
    if ( !reply )
    {
      // canceled
      return;
    }

    if ( reply->error() != QNetworkReply::NoError )
    {
      QgsMessageLog::logMessage( tr( "News feed request failed [error: %1]" ).arg( reply->errorString() ) );
      return;
    }

    // queue up the handling
    QMetaObject::invokeMethod( this, "onFetch", Qt::QueuedConnection, Q_ARG( QString, task->contentAsString() ) );
  } );

  QgsApplication::taskManager()->addTask( task );
}

void QgsNewsFeedParser::onFetch( const QString &content )
{
  QgsSettings().setValue( mSettingsKey + "/lastFetchTime", mFetchStartTime, QgsSettings::Core );

  const QVariant json = QgsJsonUtils::parseJson( content );

  const QVariantList entries = json.toList();
  QList< QgsNewsFeedParser::Entry > newEntries;
  newEntries.reserve( entries.size() );
  for ( const QVariant &e : entries )
  {
    Entry newEntry;
    const QVariantMap entryMap = e.toMap();
    newEntry.key = entryMap.value( QStringLiteral( "pk" ) ).toInt();
    newEntry.title = entryMap.value( QStringLiteral( "title" ) ).toString();
    newEntry.imageUrl = entryMap.value( QStringLiteral( "image" ) ).toString();
    newEntry.content = entryMap.value( QStringLiteral( "content" ) ).toString();
    newEntry.link = entryMap.value( QStringLiteral( "url" ) ).toString();
    newEntry.sticky = entryMap.value( QStringLiteral( "sticky" ) ).toBool();
    newEntries.append( newEntry );
    mEntries.append( newEntry );
    storeEntryInSettings( newEntry );
  }

  emit fetched( newEntries );
}

void QgsNewsFeedParser::readStoredEntries()
{
  QgsSettings settings;

  settings.beginGroup( mSettingsKey, QgsSettings::Core );
  QStringList existing = settings.childGroups();
  std::sort( existing.begin(), existing.end(), []( const QString & a, const QString & b )
  {
    return a.toInt() < b.toInt();
  } );
  mEntries.reserve( existing.size() );
  for ( const QString &entry : existing )
  {
    mEntries.append( readEntryFromSettings( entry.toInt() ) );
  }
}

QgsNewsFeedParser::Entry QgsNewsFeedParser::readEntryFromSettings( const int key ) const
{
  const QString baseSettingsKey = QStringLiteral( "%1/%2" ).arg( mSettingsKey ).arg( key );
  QgsSettings settings;
  settings.beginGroup( baseSettingsKey, QgsSettings::Core );
  Entry entry;
  entry.key = key;
  entry.title = settings.value( QStringLiteral( "title" ) ).toString();
  entry.imageUrl = settings.value( QStringLiteral( "imageUrl" ) ).toString();
  entry.content = settings.value( QStringLiteral( "content" ) ).toString();
  entry.link = settings.value( QStringLiteral( "link" ) ).toString();
  entry.sticky = settings.value( QStringLiteral( "sticky" ) ).toBool();
  return entry;
}

void QgsNewsFeedParser::storeEntryInSettings( const QgsNewsFeedParser::Entry &entry )
{
  const QString baseSettingsKey = QStringLiteral( "%1/%2" ).arg( mSettingsKey ).arg( entry.key );
  QgsSettings settings;
  settings.setValue( QStringLiteral( "%1/title" ).arg( baseSettingsKey ), entry.title, QgsSettings::Core );
  settings.setValue( QStringLiteral( "%1/imageUrl" ).arg( baseSettingsKey ), entry.imageUrl, QgsSettings::Core );
  settings.setValue( QStringLiteral( "%1/content" ).arg( baseSettingsKey ), entry.content, QgsSettings::Core );
  settings.setValue( QStringLiteral( "%1/link" ).arg( baseSettingsKey ), entry.link, QgsSettings::Core );
  settings.setValue( QStringLiteral( "%1/sticky" ).arg( baseSettingsKey ), entry.sticky, QgsSettings::Core );
}

QString QgsNewsFeedParser::keyForFeed( const QString &baseUrl )
{
  static QRegularExpression sRegexp( QStringLiteral( "[^a-zA-Z0-9]" ) );
  QString res = baseUrl;
  res = res.replace( sRegexp, QString() );
  return QStringLiteral( "NewsFeed/%1" ).arg( res );
}
