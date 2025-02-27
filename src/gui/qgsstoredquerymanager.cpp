/***************************************************************************
                             qgsstoredquerymanager.cpp
                             ------------------------------------
    Date                 : February 2025
    Copyright            : (C) 2025 Nyall Dawson
    Email                : nyall dot dawson at gmail dot com
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "qgsstoredquerymanager.h"
#include "qgsproject.h"
#include "qgssettingsentryimpl.h"
#include "moc_qgsstoredquerymanager.cpp"

#include <QCryptographicHash>

///@cond PRIVATE
const QgsSettingsEntryString *QgsStoredQueryManager::settingQueryName = new QgsSettingsEntryString( QStringLiteral( "name" ), sTreeStoredQueries );
const QgsSettingsEntryString *QgsStoredQueryManager::settingQueryDefinition = new QgsSettingsEntryString( QStringLiteral( "query" ), sTreeStoredQueries );
///@endcond PRIVATE

QgsStoredQueryManager::QgsStoredQueryManager( QObject *parent )
  : QObject( parent )
{
}

void QgsStoredQueryManager::storeQuery( const QString &name, const QString &query, Qgis::QueryStorageBackend backend )
{
  if ( query.isEmpty() || name.isEmpty() )
    return;

  bool wasAdded = false;
  bool wasUpdated = false;

  // Yes, this looks odd! It's this way for compatibility with DB Manager stored queries...
  const QString hash = getQueryHash( name );

  switch ( backend )
  {
    case Qgis::QueryStorageBackend::LocalProfile:
    {
      const bool isExisting = sTreeStoredQueries->items().contains( hash );
      wasAdded = !isExisting;
      wasUpdated = isExisting;
      settingQueryName->setValue( name, hash );
      settingQueryDefinition->setValue( query, hash );
      break;
    }

    case Qgis::QueryStorageBackend::CurrentProject:
    {
      const bool isExisting = QgsProject::instance()->subkeyList( QStringLiteral( "DBManager" ), QStringLiteral( "savedQueries" ) ).contains( hash );
      wasAdded = !isExisting;
      wasUpdated = isExisting;
      QgsProject::instance()->writeEntry( QStringLiteral( "DBManager" ), QStringLiteral( "savedQueries/%1/name" ).arg( hash ), name );
      QgsProject::instance()->writeEntry( QStringLiteral( "DBManager" ), QStringLiteral( "savedQueries/%1/query" ).arg( hash ), query );
      break;
    }
  }

  if ( wasAdded )
    emit queryAdded( name, backend );
  else if ( wasUpdated )
    emit queryChanged( name, backend );
}

void QgsStoredQueryManager::removeQuery( const QString &name, Qgis::QueryStorageBackend backend )
{
  if ( name.isEmpty() )
    return;

  bool wasDeleted = false;
  // Yes, this looks odd! It's this way for compatibility with DB Manager stored queries...
  const QString hash = getQueryHash( name );

  switch ( backend )
  {
    case Qgis::QueryStorageBackend::LocalProfile:
    {
      wasDeleted = sTreeStoredQueries->items().contains( hash );
      sTreeStoredQueries->deleteItem( hash );
      break;
    }

    case Qgis::QueryStorageBackend::CurrentProject:
    {
      wasDeleted = QgsProject::instance()->subkeyList( QStringLiteral( "DBManager" ), QStringLiteral( "savedQueries" ) ).contains( hash );
      QgsProject::instance()->removeEntry(
        "DBManager", QStringLiteral( "savedQueries/%1" ).arg( hash )
      );
      break;
    }
  }

  if ( wasDeleted )
    emit queryRemoved( name, backend );
}

QStringList QgsStoredQueryManager::allQueryNames( Qgis::QueryStorageBackend backend ) const
{
  QStringList names;
  switch ( backend )
  {
    case Qgis::QueryStorageBackend::LocalProfile:
    {
      const QStringList hashes = sTreeStoredQueries->items();
      names.reserve( hashes.size() );
      for ( const QString &hash : hashes )
      {
        names.append( settingQueryName->value( hash ) );
      }
      break;
    }

    case Qgis::QueryStorageBackend::CurrentProject:
    {
      const QStringList hashes = QgsProject::instance()->subkeyList( QStringLiteral( "DBManager" ), QStringLiteral( "savedQueries" ) );
      names.reserve( hashes.size() );
      for ( const QString &hash : hashes )
      {
        names.append( QgsProject::instance()->readEntry(
          QStringLiteral( "DBManager" ), QStringLiteral( "savedQueries/%1/name" ).arg( hash )
        ) );
      }
      break;
    }
  }
  return names;
}

QString QgsStoredQueryManager::query( const QString &name, Qgis::QueryStorageBackend backend ) const
{
  // Yes, this looks odd! It's this way for compatibility with DB Manager stored queries...
  const QString hash = getQueryHash( name );

  switch ( backend )
  {
    case Qgis::QueryStorageBackend::LocalProfile:
    {
      return settingQueryDefinition->value( hash );
    }

    case Qgis::QueryStorageBackend::CurrentProject:
    {
      return QgsProject::instance()->readEntry( QStringLiteral( "DBManager" ), QStringLiteral( "savedQueries/%1/query" ).arg( hash ) );
    }
  }
  BUILTIN_UNREACHABLE;
}

QList<QgsStoredQueryManager::QueryDetails> QgsStoredQueryManager::allQueries() const
{
  QList<QgsStoredQueryManager::QueryDetails> res;

  const QStringList localProfileHashes = sTreeStoredQueries->items();
  const QStringList projectHashes = QgsProject::instance()->subkeyList( QStringLiteral( "DBManager" ), QStringLiteral( "savedQueries" ) );
  res.reserve( localProfileHashes.size() + projectHashes.size() );

  for ( const QString &hash : localProfileHashes )
  {
    QueryDetails details;
    details.name = settingQueryName->value( hash );
    details.definition = settingQueryDefinition->value( hash );
    details.backend = Qgis::QueryStorageBackend::LocalProfile;
    res.append( details );
  }

  for ( const QString &hash : projectHashes )
  {
    QueryDetails details;
    details.name = QgsProject::instance()->readEntry(
      QStringLiteral( "DBManager" ), QStringLiteral( "savedQueries/%1/name" ).arg( hash )
    );
    details.definition = QgsProject::instance()->readEntry(
      QStringLiteral( "DBManager" ), QStringLiteral( "savedQueries/%1/query" ).arg( hash )
    );
    details.backend = Qgis::QueryStorageBackend::CurrentProject;
    res.append( details );
  }

  std::sort( res.begin(), res.end(), [=]( const QueryDetails &a, const QueryDetails &b ) {
    if ( a.name == b.name )
      return a.backend == Qgis::QueryStorageBackend::CurrentProject;

    return QString::localeAwareCompare( a.name, b.name ) < 0;
  } );

  return res;
}

QString QgsStoredQueryManager::getQueryHash( const QString &name )
{
  // for compatibility with DB manager stored queries!
  QByteArray nameUtf8 = name.toUtf8();
  QByteArray hash = QCryptographicHash::hash( nameUtf8, QCryptographicHash::Md5 ).toHex();
  return QStringLiteral( "q%1" ).arg( QString::fromUtf8( hash ) );
}
