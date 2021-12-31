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
#include "history/qgshistoryprovider.h"
#include "qgsapplication.h"
#include "qgsruntimeprofiler.h"
#include "qgslogger.h"
#include "qgsxmlutils.h"
#include "qgsprocessinghistoryprovider.h"

#include <QFile>
#include <sqlite3.h>

//
// QgsHistoryEntry
//

QgsHistoryEntry::QgsHistoryEntry( const QString &providerId, const QDateTime &timestamp, const QVariantMap &entry )
  : timestamp( timestamp )
  , providerId( providerId )
  , entry( entry )
{

}

QgsHistoryEntry::QgsHistoryEntry( const QVariantMap &entry )
  : timestamp( QDateTime::currentDateTime() )
  , entry( entry )
{

}

//
// QgsHistoryProviderRegistry
//

QgsHistoryProviderRegistry::QgsHistoryProviderRegistry( QObject *parent, bool useMemoryDatabase )
  : QObject( parent )
{
  QgsScopedRuntimeProfile profile( tr( "Load history database" ) );
  const QString historyFilename = userHistoryDbPath();

  // create history db if it doesn't exist
  QString error;
  if ( useMemoryDatabase )
  {
    createDatabase( QStringLiteral( ":memory:" ), error );
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
    xmlDoc.appendChild( QgsXmlUtils::writeVariant( entry.entry, xmlDoc ) );
    const QString entryXml = xmlDoc.toString();
    const QString dateTime = entry.timestamp.toString( QStringLiteral( "yyyy-MM-dd HH:mm:ss" ) );

    QString query = qgs_sqlite3_mprintf( "INSERT INTO history VALUES (NULL, '%q', '%q', '%q');",
                                         entry.providerId.toUtf8().constData(), entryXml.toUtf8().constData(), dateTime.toUtf8().constData() );
    if ( !runEmptyQuery( query ) )
    {
      QgsDebugMsg( QStringLiteral( "Couldn't story history entry in database!" ) );
      ok = false;
      return -1;
    }
    id = static_cast< int >( sqlite3_last_insert_rowid( mLocalDB.get() ) );
  }

  return id;
}

bool QgsHistoryProviderRegistry::addEntries( const QList<QgsHistoryEntry> &entries, HistoryEntryOptions options )
{
  bool ok = true;
  if ( options.storageBackends & Qgis::HistoryProviderBackend::LocalProfile )
  {
    runEmptyQuery( QStringLiteral( "BEGIN TRANSACTION;" ) );
    for ( const QgsHistoryEntry &entry : entries )
      addEntry( entry, ok, options );
    runEmptyQuery( QStringLiteral( "COMMIT TRANSACTION;" ) );
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
        QgsDebugMsg( QStringLiteral( "Cannot open database to query history entries" ) );
        return QgsHistoryEntry( QVariantMap() );
      }

      QString sql = QStringLiteral( "SELECT provider_id, xml, timestamp FROM history WHERE id=%1" ).arg( id );

      int nErr;
      sqlite3_statement_unique_ptr statement = mLocalDB.prepare( sql, nErr );

      if ( nErr == SQLITE_OK && sqlite3_step( statement.get() ) == SQLITE_ROW )
      {
        QDomDocument doc;
        if ( !doc.setContent( statement.columnAsText( 1 ) ) )
        {
          QgsDebugMsg( QStringLiteral( "Cannot read history entry" ) );
          return QgsHistoryEntry( QVariantMap() );
        }

        ok = true;
        return QgsHistoryEntry(
                 statement.columnAsText( 0 ),
                 QDateTime::fromString( statement.columnAsText( 2 ), QStringLiteral( "yyyy-MM-dd HH:mm:ss" ) ),
                 QgsXmlUtils::readVariant( doc.documentElement() ).toMap()
               );
      }

      QgsDebugMsg( QStringLiteral( "Cannot find history item with matching ID" ) );
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
      QDomDocument xmlDoc;
      xmlDoc.appendChild( QgsXmlUtils::writeVariant( entry, xmlDoc ) );
      const QString entryXml = xmlDoc.toString();

      QString query = qgs_sqlite3_mprintf( "UPDATE history SET xml='%q' WHERE id = %d;",
                                           entryXml.toUtf8().constData(), id );
      if ( !runEmptyQuery( query ) )
      {
        QgsDebugMsg( QStringLiteral( "Couldn't update history entry in database!" ) );
        return false;
      }
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
      QgsDebugMsg( QStringLiteral( "Cannot open database to query history entries" ) );
      return {};
    }

    QString sql = QStringLiteral( "SELECT provider_id, xml, timestamp FROM history" );
    QStringList whereClauses;
    if ( !providerId.isEmpty() )
    {
      whereClauses.append( QStringLiteral( "provider_id='%1'" ).arg( providerId ) );
    }
    if ( start.isValid() )
    {
      whereClauses.append( QStringLiteral( "timestamp>='%1'" ).arg( start.toString( QStringLiteral( "yyyy-MM-dd HH:mm:ss" ) ) ) );
    }
    if ( end.isValid() )
    {
      whereClauses.append( QStringLiteral( "timestamp<='%1'" ).arg( end.toString( QStringLiteral( "yyyy-MM-dd HH:mm:ss" ) ) ) );
    }

    if ( !whereClauses.empty() )
      sql += QStringLiteral( " WHERE (" ) + whereClauses.join( QLatin1String( ") AND (" ) ) + ')';

    int nErr;
    sqlite3_statement_unique_ptr statement = mLocalDB.prepare( sql, nErr );

    while ( nErr == SQLITE_OK && sqlite3_step( statement.get() ) == SQLITE_ROW )
    {
      QDomDocument doc;
      if ( !doc.setContent( statement.columnAsText( 1 ) ) )
      {
        QgsDebugMsg( QStringLiteral( "Cannot read history entry" ) );
        continue;
      }

      entries.append( QgsHistoryEntry(
                        statement.columnAsText( 0 ),
                        QDateTime::fromString( statement.columnAsText( 2 ), QStringLiteral( "yyyy-MM-dd HH:mm:ss" ) ),
                        QgsXmlUtils::readVariant( doc.documentElement() ).toMap()
                      ) );
    }
  }

  return entries;
}

QString QgsHistoryProviderRegistry::userHistoryDbPath()
{
  return QgsApplication::qgisSettingsDirPath() + QStringLiteral( "user-history.db" );
}

bool QgsHistoryProviderRegistry::clearHistory( Qgis::HistoryProviderBackend backend )
{
  switch ( backend )
  {
    case Qgis::HistoryProviderBackend::LocalProfile:
      runEmptyQuery( QStringLiteral( "DELETE from history;" ) );
      break;
  }
  return true;
}

bool QgsHistoryProviderRegistry::createDatabase( const QString &filename, QString &error )
{
  error.clear();
  if ( !openDatabase( filename, error ) )
  {
    QgsDebugMsg( error );
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
  QString query = qgs_sqlite3_mprintf( "CREATE TABLE history("\
                                       "id INTEGER PRIMARY KEY,"\
                                       "provider_id TEXT,"\
                                       "xml TEXT,"\
                                       "timestamp DATETIME);" \
                                       "CREATE INDEX provider_index ON history(provider_id);"\
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
    QgsDebugMsg( zErr );
    sqlite3_free( zErr );
  }

  return nErr == SQLITE_OK;
}

