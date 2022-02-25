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
#include "qgslogger.h"
#include "qgssettings.h"
#include "qgsjsonutils.h"
#include "qgsmessagelog.h"
#include "qgsapplication.h"
#include <QDateTime>
#include <QUrlQuery>
#include <QFile>
#include <QDir>
#include <QRegularExpression>

QgsNewsFeedParser::QgsNewsFeedParser( const QUrl &feedUrl, const QString &authcfg, QObject *parent )
  : QObject( parent )
  , mBaseUrl( feedUrl.toString() )
  , mFeedUrl( feedUrl )
  , mAuthCfg( authcfg )
  , mSettingsKey( keyForFeed( mBaseUrl ) )
{
  // first thing we do is populate with existing entries
  readStoredEntries();

  QUrlQuery query( feedUrl );

  const qint64 after = settingsFeedLastFetchTime.value( mSettingsKey );
  if ( after > 0 )
    query.addQueryItem( QStringLiteral( "after" ), qgsDoubleToString( after, 0 ) );

  QString feedLanguage = settingsFeedLanguage.value( mSettingsKey );
  if ( feedLanguage.isEmpty() )
  {
    feedLanguage = QgsApplication::settingsLocaleUserLocale.valueWithDefaultOverride( QStringLiteral( "en" ) );
  }
  if ( !feedLanguage.isEmpty() && feedLanguage != QLatin1String( "C" ) )
    query.addQueryItem( QStringLiteral( "lang" ), feedLanguage.mid( 0, 2 ) );

  if ( settingsFeedLatitude.exists( mSettingsKey ) && settingsFeedLongitude.exists( mSettingsKey ) )
  {
    const double feedLat = settingsFeedLatitude.value( mSettingsKey );
    const double feedLong = settingsFeedLongitude.value( mSettingsKey );

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

  QgsSettings().remove( QStringLiteral( "%1/%2" ).arg( mSettingsKey ).arg( key ), QgsSettings::Core );

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
  QgsNetworkContentFetcherTask *task = new QgsNetworkContentFetcherTask( req, mAuthCfg, QgsTask::CanCancel | QgsTask::CancelWithoutPrompt );
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
  settingsFeedLastFetchTime.setValue( mFetchStartTime, mSettingsKey );

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
    bool ok = false;
    const uint expiry = entryMap.value( QStringLiteral( "publish_to" ) ).toUInt( &ok );
    if ( ok )
      newEntry.expiry.setSecsSinceEpoch( expiry );
    newEntries.append( newEntry );

    if ( !newEntry.imageUrl.isEmpty() )
      fetchImageForEntry( newEntry );

    mEntries.append( newEntry );
    storeEntryInSettings( newEntry );
    emit entryAdded( newEntry );
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
  entry.expiry = settings.value( QStringLiteral( "expiry" ) ).toDateTime();
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
  const QString baseSettingsKey = QStringLiteral( "%1/%2" ).arg( mSettingsKey ).arg( entry.key );
  QgsSettings settings;
  settings.setValue( QStringLiteral( "%1/title" ).arg( baseSettingsKey ), entry.title, QgsSettings::Core );
  settings.setValue( QStringLiteral( "%1/imageUrl" ).arg( baseSettingsKey ), entry.imageUrl, QgsSettings::Core );
  settings.setValue( QStringLiteral( "%1/content" ).arg( baseSettingsKey ), entry.content, QgsSettings::Core );
  settings.setValue( QStringLiteral( "%1/link" ).arg( baseSettingsKey ), entry.link, QgsSettings::Core );
  settings.setValue( QStringLiteral( "%1/sticky" ).arg( baseSettingsKey ), entry.sticky, QgsSettings::Core );
  if ( entry.expiry.isValid() )
    settings.setValue( QStringLiteral( "%1/expiry" ).arg( baseSettingsKey ), entry.expiry, QgsSettings::Core );
}

void QgsNewsFeedParser::fetchImageForEntry( const QgsNewsFeedParser::Entry &entry )
{
  // start fetching image
  QgsNetworkContentFetcher *fetcher = new QgsNetworkContentFetcher();
  connect( fetcher, &QgsNetworkContentFetcher::finished, this, [ = ]
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
  return QStringLiteral( "NewsFeed/%1" ).arg( res );
}
