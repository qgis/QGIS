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
#include "qgsnetworkcontentfetcher.h"
#include "qgsnetworkaccessmanager.h"
#include "qgssetrequestinitiator_p.h"
#include "qgsjsonutils.h"
#include "qgsmessagelog.h"
#include "qgsapplication.h"
#include "qgssettingsentryimpl.h"

#include <QDateTime>
#include <QUrlQuery>
#include <QFile>
#include <QDir>
#include <QRegularExpression>


const QgsSettingsEntryInteger64 *QgsNewsFeedParser::settingsFeedLastFetchTime = new QgsSettingsEntryInteger64( QStringLiteral( "last-fetch-time" ), sTreeNewsFeed, 0, QStringLiteral( "Feed last fetch time" ), Qgis::SettingsOptions(), 0 );
const QgsSettingsEntryString *QgsNewsFeedParser::settingsFeedLanguage = new QgsSettingsEntryString( QStringLiteral( "lang" ), sTreeNewsFeed, QString(), QStringLiteral( "Feed language" ) );
const QgsSettingsEntryDouble *QgsNewsFeedParser::settingsFeedLatitude = new QgsSettingsEntryDouble( QStringLiteral( "latitude" ), sTreeNewsFeed, 0.0, QStringLiteral( "Feed latitude" ) );
const QgsSettingsEntryDouble *QgsNewsFeedParser::settingsFeedLongitude = new QgsSettingsEntryDouble( QStringLiteral( "longitude" ), sTreeNewsFeed, 0.0, QStringLiteral( "Feed longitude" ) );


const QgsSettingsEntryString *QgsNewsFeedParser::settingsFeedEntryTitle = new QgsSettingsEntryString( QStringLiteral( "title" ), sTreeNewsFeedEntries, QString(), QStringLiteral( "Entry title" ) );
const QgsSettingsEntryString *QgsNewsFeedParser::settingsFeedEntryImageUrl = new QgsSettingsEntryString( QStringLiteral( "image-url" ), sTreeNewsFeedEntries, QString(), QStringLiteral( "Entry image URL" ) );
const QgsSettingsEntryString *QgsNewsFeedParser::settingsFeedEntryContent = new QgsSettingsEntryString( QStringLiteral( "content" ), sTreeNewsFeedEntries, QString(), QStringLiteral( "Entry content" ) );
const QgsSettingsEntryString *QgsNewsFeedParser::settingsFeedEntryLink = new QgsSettingsEntryString( QStringLiteral( "link" ), sTreeNewsFeedEntries, QString(), QStringLiteral( "Entry link" ) );
const QgsSettingsEntryBool *QgsNewsFeedParser::settingsFeedEntrySticky = new QgsSettingsEntryBool( QStringLiteral( "sticky" ), sTreeNewsFeedEntries, false );
const QgsSettingsEntryVariant *QgsNewsFeedParser::settingsFeedEntryExpiry = new QgsSettingsEntryVariant( QStringLiteral( "expiry" ), sTreeNewsFeedEntries, QVariant(), QStringLiteral( "Expiry date" ) );



QgsNewsFeedParser::QgsNewsFeedParser( const QUrl &feedUrl, const QString &authcfg, QObject *parent )
  : QObject( parent )
  , mBaseUrl( feedUrl.toString() )
  , mFeedUrl( feedUrl )
  , mAuthCfg( authcfg )
  , mFeedKey( keyForFeed( mBaseUrl ) )
{
  // first thing we do is populate with existing entries
  readStoredEntries();

  QUrlQuery query( feedUrl );

  const qint64 after = settingsFeedLastFetchTime->value( mFeedKey );
  if ( after > 0 )
    query.addQueryItem( QStringLiteral( "after" ), qgsDoubleToString( after, 0 ) );

  QString feedLanguage = settingsFeedLanguage->value( mFeedKey );
  if ( feedLanguage.isEmpty() )
  {
    feedLanguage = QgsApplication::settingsLocaleUserLocale->valueWithDefaultOverride( QStringLiteral( "en" ) );
  }
  if ( !feedLanguage.isEmpty() && feedLanguage != QLatin1String( "C" ) )
    query.addQueryItem( QStringLiteral( "lang" ), feedLanguage.mid( 0, 2 ) );

  if ( settingsFeedLatitude->exists( mFeedKey ) && settingsFeedLongitude->exists( mFeedKey ) )
  {
    const double feedLat = settingsFeedLatitude->value( mFeedKey );
    const double feedLong = settingsFeedLongitude->value( mFeedKey );

    // hack to allow testing using local files
    if ( feedUrl.isLocalFile() )
    {
      query.addQueryItem( QStringLiteral( "lat" ), QString::number( static_cast< int >( feedLat ) ) );
      query.addQueryItem( QStringLiteral( "lon" ), QString::number( static_cast< int >( feedLong ) ) );
    }
    else
    {
      query.addQueryItem( QStringLiteral( "lat" ), qgsDoubleToString( feedLat ) );
      query.addQueryItem( QStringLiteral( "lon" ), qgsDoubleToString( feedLong ) );
    }
  }

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

void QgsNewsFeedParser::dismissEntry( int key )
{
  Entry dismissed;
  const int beforeSize = mEntries.size();
  mEntries.erase( std::remove_if( mEntries.begin(), mEntries.end(),
                                  [key, &dismissed]( const Entry & entry )
  {
    if ( entry.key == key )
    {
      dismissed = entry;
      return true;
    }
    return false;
  } ), mEntries.end() );
  if ( beforeSize == mEntries.size() )
    return; // didn't find matching entry

  sTreeNewsFeedEntries->deleteItem( QString::number( key ), {mFeedKey} );

  // also remove preview image, if it exists
  if ( !dismissed.imageUrl.isEmpty() )
  {
    const QString previewDir = QStringLiteral( "%1/previewImages" ).arg( QgsApplication::qgisSettingsDirPath() );
    const QString imagePath = QStringLiteral( "%1/%2.png" ).arg( previewDir ).arg( key );
    if ( QFile::exists( imagePath ) )
    {
      QFile::remove( imagePath );
    }
  }

  if ( !mBlockSignals )
    emit entryDismissed( dismissed );
}

void QgsNewsFeedParser::dismissAll()
{
  const QList< QgsNewsFeedParser::Entry > entries = mEntries;
  for ( const Entry &entry : entries )
  {
    dismissEntry( entry.key );
  }
}

QString QgsNewsFeedParser::authcfg() const
{
  return mAuthCfg;
}

void QgsNewsFeedParser::fetch()
{
  QNetworkRequest req( mFeedUrl );
  QgsSetRequestInitiatorClass( req, QStringLiteral( "QgsNewsFeedParser" ) );

  mFetchStartTime = QDateTime::currentDateTimeUtc().toSecsSinceEpoch();

  // allow canceling the news fetching without prompts -- it's not crucial if this gets finished or not
  QgsNetworkContentFetcherTask *task = new QgsNetworkContentFetcherTask( req, mAuthCfg, QgsTask::CanCancel | QgsTask::CancelWithoutPrompt | QgsTask::Silent );
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
  settingsFeedLastFetchTime->setValue( mFetchStartTime, {mFeedKey} );

  const QVariant json = QgsJsonUtils::parseJson( content );

  const QVariantList entries = json.toList();
  QList< QgsNewsFeedParser::Entry > fetchedEntries;
  fetchedEntries.reserve( entries.size() );
  for ( const QVariant &e : entries )
  {
    Entry incomingEntry;
    const QVariantMap entryMap = e.toMap();
    incomingEntry.key = entryMap.value( QStringLiteral( "pk" ) ).toInt();
    incomingEntry.title = entryMap.value( QStringLiteral( "title" ) ).toString();
    incomingEntry.imageUrl = entryMap.value( QStringLiteral( "image" ) ).toString();
    incomingEntry.content = entryMap.value( QStringLiteral( "content" ) ).toString();
    incomingEntry.link = entryMap.value( QStringLiteral( "url" ) ).toString();
    incomingEntry.sticky = entryMap.value( QStringLiteral( "sticky" ) ).toBool();
    bool hasExpiry = false;
    const qlonglong expiry = entryMap.value( QStringLiteral( "publish_to" ) ).toLongLong( &hasExpiry );
    if ( hasExpiry )
      incomingEntry.expiry.setSecsSinceEpoch( expiry );

    fetchedEntries.append( incomingEntry );

    // We also need to handle the case of modified/expired entries
    const auto entryIter { std::find_if( mEntries.begin(), mEntries.end(), [incomingEntry]( const QgsNewsFeedParser::Entry & candidate )
    {
      return candidate.key == incomingEntry.key;
    } )};
    const bool entryExists { entryIter != mEntries.end() };

    // case 1: existing entry is now expired, dismiss
    if ( hasExpiry && expiry < mFetchStartTime )
    {
      dismissEntry( incomingEntry.key );
    }
    // case 2: existing entry edited
    else if ( entryExists )
    {
      const bool imageNeedsUpdate = ( entryIter->imageUrl != incomingEntry.imageUrl );
      // also remove preview image, if it exists
      if ( imageNeedsUpdate && ! entryIter->imageUrl.isEmpty() )
      {
        const QString previewDir = QStringLiteral( "%1/previewImages" ).arg( QgsApplication::qgisSettingsDirPath() );
        const QString imagePath = QStringLiteral( "%1/%2.png" ).arg( previewDir ).arg( entryIter->key );
        if ( QFile::exists( imagePath ) )
        {
          QFile::remove( imagePath );
        }
      }
      *entryIter = incomingEntry;
      if ( imageNeedsUpdate && ! incomingEntry.imageUrl.isEmpty() )
        fetchImageForEntry( incomingEntry );

      sTreeNewsFeedEntries->deleteItem( QString::number( incomingEntry.key ), {mFeedKey} );
      storeEntryInSettings( incomingEntry );
      emit entryUpdated( incomingEntry );
    }
    // else: new entry, not expired
    else if ( !hasExpiry || expiry >= mFetchStartTime )
    {
      if ( !incomingEntry.imageUrl.isEmpty() )
        fetchImageForEntry( incomingEntry );

      mEntries.append( incomingEntry );
      storeEntryInSettings( incomingEntry );
      emit entryAdded( incomingEntry );
    }

  }

  emit fetched( fetchedEntries );
}

void QgsNewsFeedParser::readStoredEntries()
{
  QStringList existing = sTreeNewsFeedEntries->items( {mFeedKey} );
  std::sort( existing.begin(), existing.end(), []( const QString & a, const QString & b )
  {
    return a.toInt() < b.toInt();
  } );
  mEntries.reserve( existing.size() );
  for ( const QString &entry : existing )
  {
    const Entry e = readEntryFromSettings( entry.toInt() );
    if ( !e.expiry.isValid() || e.expiry > QDateTime::currentDateTime() )
      mEntries.append( e );
    else
    {
      // expired entry, prune it
      mBlockSignals = true;
      dismissEntry( e.key );
      mBlockSignals = false;
    }
  }
}

QgsNewsFeedParser::Entry QgsNewsFeedParser::readEntryFromSettings( const int key )
{
  Entry entry;
  entry.key = key;
  entry.title = settingsFeedEntryTitle->value( {mFeedKey, QString::number( key )} );
  entry.imageUrl = settingsFeedEntryImageUrl->value( {mFeedKey, QString::number( key )} );
  entry.content = settingsFeedEntryContent->value( {mFeedKey, QString::number( key )} );
  entry.link = settingsFeedEntryLink->value( {mFeedKey, QString::number( key )} );
  entry.sticky = settingsFeedEntrySticky->value( {mFeedKey, QString::number( key )} );
  entry.expiry = settingsFeedEntryExpiry->value( {mFeedKey, QString::number( key )} ).toDateTime();
  if ( !entry.imageUrl.isEmpty() )
  {
    const QString previewDir = QStringLiteral( "%1/previewImages" ).arg( QgsApplication::qgisSettingsDirPath() );
    const QString imagePath = QStringLiteral( "%1/%2.png" ).arg( previewDir ).arg( entry.key );
    if ( QFile::exists( imagePath ) )
    {
      const QImage img( imagePath );
      entry.image = QPixmap::fromImage( img );
    }
    else
    {
      fetchImageForEntry( entry );
    }
  }
  return entry;
}

void QgsNewsFeedParser::storeEntryInSettings( const QgsNewsFeedParser::Entry &entry )
{
  settingsFeedEntryTitle->setValue( entry.title, {mFeedKey, QString::number( entry.key )} );
  settingsFeedEntryImageUrl->setValue( entry.imageUrl, {mFeedKey, QString::number( entry.key )} );
  settingsFeedEntryContent->setValue( entry.content, {mFeedKey, QString::number( entry.key )} );
  settingsFeedEntryLink->setValue( entry.link.toString(), {mFeedKey, QString::number( entry.key )} );
  settingsFeedEntrySticky->setValue( entry.sticky, {mFeedKey, QString::number( entry.key )} );
  if ( entry.expiry.isValid() )
    settingsFeedEntryExpiry->setValue( entry.expiry, {mFeedKey, QString::number( entry.key )} );
}

void QgsNewsFeedParser::fetchImageForEntry( const QgsNewsFeedParser::Entry &entry )
{
  // start fetching image
  QgsNetworkContentFetcher *fetcher = new QgsNetworkContentFetcher();
  connect( fetcher, &QgsNetworkContentFetcher::finished, this, [this, fetcher, entry]
  {
    const auto findIter = std::find_if( mEntries.begin(), mEntries.end(), [entry]( const QgsNewsFeedParser::Entry & candidate )
    {
      return candidate.key == entry.key;
    } );
    if ( findIter != mEntries.end() )
    {
      const int entryIndex = static_cast< int >( std::distance( mEntries.begin(), findIter ) );

      QImage img = QImage::fromData( fetcher->reply()->readAll() );

      QSize size = img.size();
      bool resize = false;
      if ( size.width() > 250 )
      {
        size.setHeight( static_cast< int >( size.height() * static_cast< double >( 250 ) / size.width() ) );
        size.setWidth( 250 );
        resize = true;
      }
      if ( size.height() > 177 )
      {
        size.setWidth( static_cast< int >( size.width() * static_cast< double >( 177 ) / size.height() ) );
        size.setHeight( 177 );
        resize = true;
      }
      if ( resize )
        img = img.scaled( size, Qt::IgnoreAspectRatio, Qt::SmoothTransformation );

      //nicely round corners so users don't get paper cuts
      QImage previewImage( size, QImage::Format_ARGB32 );
      previewImage.fill( Qt::transparent );
      QPainter previewPainter( &previewImage );
      previewPainter.setRenderHint( QPainter::Antialiasing, true );
      previewPainter.setRenderHint( QPainter::SmoothPixmapTransform, true );
      previewPainter.setPen( Qt::NoPen );
      previewPainter.setBrush( Qt::black );
      previewPainter.drawRoundedRect( 0, 0, size.width(), size.height(), 8, 8 );
      previewPainter.setCompositionMode( QPainter::CompositionMode_SourceIn );
      previewPainter.drawImage( 0, 0, img );
      previewPainter.end();

      // Save image, so we don't have to fetch it next time
      const QString previewDir = QStringLiteral( "%1/previewImages" ).arg( QgsApplication::qgisSettingsDirPath() );
      QDir().mkdir( previewDir );
      const QString imagePath = QStringLiteral( "%1/%2.png" ).arg( previewDir ).arg( entry.key );
      previewImage.save( imagePath );

      mEntries[ entryIndex ].image = QPixmap::fromImage( previewImage );
      this->emit imageFetched( entry.key, mEntries[ entryIndex ].image );
    }
    fetcher->deleteLater();
  } );
  fetcher->fetchContent( entry.imageUrl, mAuthCfg );
}

QString QgsNewsFeedParser::keyForFeed( const QString &baseUrl )
{
  static const QRegularExpression sRegexp( QStringLiteral( "[^a-zA-Z0-9]" ) );
  QString res = baseUrl;
  res = res.replace( sRegexp, QString() );
  return res;
}
