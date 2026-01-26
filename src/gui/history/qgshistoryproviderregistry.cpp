/***************************************************************************
                            qgshistoryproviderregistry.cpp
                            -------------------------
    begin                : April 2019
    copyright            : (C) 2019 by Nyall Dawson
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

#include "qgshistoryproviderregistry.h"

#include <sqlite3.h>

#include "qgsapplication.h"
#include "qgsdbqueryhistoryprovider.h"
#include "qgshistoryentry.h"
#include "qgshistoryprovider.h"
#include "qgslogger.h"
#include "qgsprocessinghistoryprovider.h"
#include "qgsprocessingutils.h"
#include "qgsruntimeprofiler.h"
#include "qgsxmlutils.h"

#include <QFile>

#include "moc_qgshistoryproviderregistry.cpp"

QgsHistoryProviderRegistry::QgsHistoryProviderRegistry( QObject *parent, bool useMemoryDatabase )
  : QObject( parent )
{
  QgsScopedRuntimeProfile profile( tr( "Load history database" ) );
  const QString historyFilename = userHistoryDbPath();

  // create history db if it doesn't exist
  QString error;
  if ( useMemoryDatabase )
  {
    createDatabase( u":memory:"_s, error );
  }
  else
  {
    if ( !QFile::exists( historyFilename ) )
    {
      createDatabase( historyFilename, error );
    }
    else
    {
      openDatabase( historyFilename, error );
    }
  }
}

QgsHistoryProviderRegistry::~QgsHistoryProviderRegistry()
{
  qDeleteAll( mProviders );
}

void QgsHistoryProviderRegistry::addDefaultProviders()
{
  addProvider( new QgsProcessingHistoryProvider() );
  addProvider( new QgsDatabaseQueryHistoryProvider() );
}

bool QgsHistoryProviderRegistry::addProvider( QgsAbstractHistoryProvider *provider )
{
  if ( mProviders.contains( provider->id() ) )
    return false;

  mProviders.insert( provider->id(), provider );
  return true;
}

QgsAbstractHistoryProvider *QgsHistoryProviderRegistry::providerById( const QString &id )
{
  return mProviders.value( id );
}

bool QgsHistoryProviderRegistry::removeProvider( const QString &id )
{
  if ( !mProviders.contains( id ) )
    return false;

  delete mProviders.take( id );
  return true;
}

QStringList QgsHistoryProviderRegistry::providerIds() const
{
  return mProviders.keys();
}

long long QgsHistoryProviderRegistry::addEntry( const QString &providerId, const QVariantMap &entry, bool &ok, QgsHistoryProviderRegistry::HistoryEntryOptions options )
{
  return addEntry( QgsHistoryEntry( providerId, QDateTime::currentDateTime(), entry ), ok, options );
}

long long QgsHistoryProviderRegistry::addEntry( const QgsHistoryEntry &entry, bool &ok, HistoryEntryOptions options )
{
  ok = true;
  long long id = -1;
  if ( options.storageBackends & Qgis::HistoryProviderBackend::LocalProfile )
  {
    QDomDocument xmlDoc;
    const QVariant cleanedMap = QgsProcessingUtils::removePointerValuesFromMap( entry.entry );
    xmlDoc.appendChild( QgsXmlUtils::writeVariant( cleanedMap, xmlDoc ) );
    const QString entryXml = xmlDoc.toString();
    const QString dateTime = entry.timestamp.toString( u"yyyy-MM-dd HH:mm:ss"_s );

    QString query = qgs_sqlite3_mprintf( "INSERT INTO history VALUES (NULL, '%q', '%q', '%q');", entry.providerId.toUtf8().constData(), entryXml.toUtf8().constData(), dateTime.toUtf8().constData() );
    if ( !runEmptyQuery( query ) )
    {
      QgsDebugError( u"Couldn't story history entry in database!"_s );
      ok = false;
      return -1;
    }
    id = static_cast<int>( sqlite3_last_insert_rowid( mLocalDB.get() ) );

    QgsHistoryEntry addedEntry( entry );
    addedEntry.id = id;

    emit entryAdded( id, addedEntry, Qgis::HistoryProviderBackend::LocalProfile );
  }

  return id;
}

bool QgsHistoryProviderRegistry::addEntries( const QList<QgsHistoryEntry> &entries, HistoryEntryOptions options )
{
  bool ok = true;
  if ( options.storageBackends & Qgis::HistoryProviderBackend::LocalProfile )
  {
    runEmptyQuery( u"BEGIN TRANSACTION;"_s );
    for ( const QgsHistoryEntry &entry : entries )
      addEntry( entry, ok, options );
    runEmptyQuery( u"COMMIT TRANSACTION;"_s );
  }

  return ok;
}

QgsHistoryEntry QgsHistoryProviderRegistry::entry( long long id, bool &ok, Qgis::HistoryProviderBackend backend ) const
{
  ok = false;
  switch ( backend )
  {
    case Qgis::HistoryProviderBackend::LocalProfile:
    {
      if ( !mLocalDB )
      {
        QgsDebugError( u"Cannot open database to query history entries"_s );
        return QgsHistoryEntry( QVariantMap() );
      }

      QString sql = u"SELECT provider_id, xml, timestamp FROM history WHERE id=%1"_s.arg( id );

      int nErr;
      sqlite3_statement_unique_ptr statement = mLocalDB.prepare( sql, nErr );

      if ( nErr == SQLITE_OK && sqlite3_step( statement.get() ) == SQLITE_ROW )
      {
        QDomDocument doc;
        if ( !doc.setContent( statement.columnAsText( 1 ) ) )
        {
          QgsDebugError( u"Cannot read history entry"_s );
          return QgsHistoryEntry( QVariantMap() );
        }

        ok = true;
        QgsHistoryEntry res = QgsHistoryEntry(
          statement.columnAsText( 0 ),
          QDateTime::fromString( statement.columnAsText( 2 ), u"yyyy-MM-dd HH:mm:ss"_s ),
          QgsXmlUtils::readVariant( doc.documentElement() ).toMap()
        );
        res.id = id;
        return res;
      }

      QgsDebugError( u"Cannot find history item with matching ID"_s );
      return QgsHistoryEntry( QVariantMap() );
    }
  }
  BUILTIN_UNREACHABLE
}

bool QgsHistoryProviderRegistry::updateEntry( long long id, const QVariantMap &entry, Qgis::HistoryProviderBackend backend )
{
  switch ( backend )
  {
    case Qgis::HistoryProviderBackend::LocalProfile:
    {
      const QVariantMap cleanedMap = QgsProcessingUtils::removePointerValuesFromMap( entry );
      QDomDocument xmlDoc;
      xmlDoc.appendChild( QgsXmlUtils::writeVariant( cleanedMap, xmlDoc ) );
      const QString entryXml = xmlDoc.toString();

      QString query = qgs_sqlite3_mprintf( "UPDATE history SET xml='%q' WHERE id = %d;", entryXml.toUtf8().constData(), id );
      if ( !runEmptyQuery( query ) )
      {
        QgsDebugError( u"Couldn't update history entry in database!"_s );
        return false;
      }

      emit entryUpdated( id, entry, Qgis::HistoryProviderBackend::LocalProfile );
      return true;
    }
  }
  BUILTIN_UNREACHABLE
}

QList<QgsHistoryEntry> QgsHistoryProviderRegistry::queryEntries( const QDateTime &start, const QDateTime &end, const QString &providerId, Qgis::HistoryProviderBackends backends ) const
{
  QList<QgsHistoryEntry> entries;
  if ( backends & Qgis::HistoryProviderBackend::LocalProfile )
  {
    if ( !mLocalDB )
    {
      QgsDebugError( u"Cannot open database to query history entries"_s );
      return {};
    }

    QString sql = u"SELECT id, provider_id, xml, timestamp FROM history"_s;
    QStringList whereClauses;
    if ( !providerId.isEmpty() )
    {
      whereClauses.append( u"provider_id='%1'"_s.arg( providerId ) );
    }
    if ( start.isValid() )
    {
      whereClauses.append( u"timestamp>='%1'"_s.arg( start.toString( u"yyyy-MM-dd HH:mm:ss"_s ) ) );
    }
    if ( end.isValid() )
    {
      whereClauses.append( u"timestamp<='%1'"_s.arg( end.toString( u"yyyy-MM-dd HH:mm:ss"_s ) ) );
    }

    if ( !whereClauses.empty() )
      sql += u" WHERE ("_s + whereClauses.join( ") AND ("_L1 ) + ')';

    int nErr;
    sqlite3_statement_unique_ptr statement = mLocalDB.prepare( sql, nErr );

    while ( nErr == SQLITE_OK && sqlite3_step( statement.get() ) == SQLITE_ROW )
    {
      QDomDocument doc;
      if ( !doc.setContent( statement.columnAsText( 2 ) ) )
      {
        QgsDebugError( u"Cannot read history entry"_s );
        continue;
      }

      QgsHistoryEntry entry(
        statement.columnAsText( 1 ),
        QDateTime::fromString( statement.columnAsText( 3 ), u"yyyy-MM-dd HH:mm:ss"_s ),
        QgsXmlUtils::readVariant( doc.documentElement() ).toMap()
      );
      entry.id = statement.columnAsInt64( 0 );

      entries.append( entry );
    }
  }

  return entries;
}

QString QgsHistoryProviderRegistry::userHistoryDbPath()
{
  return QgsApplication::qgisSettingsDirPath() + u"user-history.db"_s;
}

bool QgsHistoryProviderRegistry::clearHistory( Qgis::HistoryProviderBackend backend, const QString &providerId )
{
  switch ( backend )
  {
    case Qgis::HistoryProviderBackend::LocalProfile:
    {
      if ( providerId.isEmpty() )
        runEmptyQuery( u"DELETE from history;"_s );
      else
        runEmptyQuery( u"DELETE from history WHERE provider_id='%1'"_s
                         .arg( providerId ) );
      break;
    }
  }
  emit historyCleared( backend, providerId );
  return true;
}

bool QgsHistoryProviderRegistry::createDatabase( const QString &filename, QString &error )
{
  error.clear();
  if ( !openDatabase( filename, error ) )
  {
    QgsDebugError( error );
    return false;
  }

  createTables();

  return true;
}

bool QgsHistoryProviderRegistry::openDatabase( const QString &filename, QString &error )
{
  int rc = mLocalDB.open( filename );
  if ( rc )
  {
    error = tr( "Couldn't open the history database: %1" ).arg( mLocalDB.errorMessage() );
    return false;
  }

  return true;
}

void QgsHistoryProviderRegistry::createTables()
{
  QString query = qgs_sqlite3_mprintf( "CREATE TABLE history("
                                       "id INTEGER PRIMARY KEY,"
                                       "provider_id TEXT,"
                                       "xml TEXT,"
                                       "timestamp DATETIME);"
                                       "CREATE INDEX provider_index ON history(provider_id);"
                                       "CREATE INDEX timestamp_index ON history(timestamp);"
  );

  runEmptyQuery( query );
}

bool QgsHistoryProviderRegistry::runEmptyQuery( const QString &query )
{
  if ( !mLocalDB )
    return false;

  char *zErr = nullptr;
  int nErr = sqlite3_exec( mLocalDB.get(), query.toUtf8().constData(), nullptr, nullptr, &zErr );

  if ( nErr != SQLITE_OK )
  {
    QgsDebugError( zErr );
    sqlite3_free( zErr );
  }

  return nErr == SQLITE_OK;
}
