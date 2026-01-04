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

#include <QCryptographicHash>

#include "moc_qgsstoredquerymanager.cpp"

///@cond PRIVATE
const QgsSettingsEntryString *QgsStoredQueryManager::settingQueryName = new QgsSettingsEntryString( u"name"_s, sTreeStoredQueries );
const QgsSettingsEntryString *QgsStoredQueryManager::settingQueryDefinition = new QgsSettingsEntryString( u"query"_s, sTreeStoredQueries );
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
      const bool isExisting = QgsProject::instance()->subkeyList( u"DBManager"_s, u"savedQueries"_s ).contains( hash );
      wasAdded = !isExisting;
      wasUpdated = isExisting;
      QgsProject::instance()->writeEntry( u"DBManager"_s, u"savedQueries/%1/name"_s.arg( hash ), name );
      QgsProject::instance()->writeEntry( u"DBManager"_s, u"savedQueries/%1/query"_s.arg( hash ), query );
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
      wasDeleted = QgsProject::instance()->subkeyList( u"DBManager"_s, u"savedQueries"_s ).contains( hash );
      QgsProject::instance()->removeEntry(
        "DBManager", u"savedQueries/%1"_s.arg( hash )
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
      const QStringList hashes = QgsProject::instance()->subkeyList( u"DBManager"_s, u"savedQueries"_s );
      names.reserve( hashes.size() );
      for ( const QString &hash : hashes )
      {
        names.append( QgsProject::instance()->readEntry(
          u"DBManager"_s, u"savedQueries/%1/name"_s.arg( hash )
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
      return QgsProject::instance()->readEntry( u"DBManager"_s, u"savedQueries/%1/query"_s.arg( hash ) );
    }
  }
  BUILTIN_UNREACHABLE;
}

QList<QgsStoredQueryManager::QueryDetails> QgsStoredQueryManager::allQueries() const
{
  QList<QgsStoredQueryManager::QueryDetails> res;

  const QStringList localProfileHashes = sTreeStoredQueries->items();
  const QStringList projectHashes = QgsProject::instance()->subkeyList( u"DBManager"_s, u"savedQueries"_s );
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
      u"DBManager"_s, u"savedQueries/%1/name"_s.arg( hash )
    );
    details.definition = QgsProject::instance()->readEntry(
      u"DBManager"_s, u"savedQueries/%1/query"_s.arg( hash )
    );
    details.backend = Qgis::QueryStorageBackend::CurrentProject;
    res.append( details );
  }

  std::sort( res.begin(), res.end(), []( const QueryDetails &a, const QueryDetails &b ) {
    if ( a.name == b.name )
    {
      if ( a.backend == b.backend )
        return false;
      else
        return a.backend == Qgis::QueryStorageBackend::CurrentProject;
    }

    return QString::localeAwareCompare( a.name, b.name ) < 0;
  } );

  return res;
}

QString QgsStoredQueryManager::getQueryHash( const QString &name )
{
  // for compatibility with DB manager stored queries!
  QByteArray nameUtf8 = name.toUtf8();
  QByteArray hash = QCryptographicHash::hash( nameUtf8, QCryptographicHash::Md5 ).toHex();
  return u"q%1"_s.arg( QString::fromUtf8( hash ) );
}
