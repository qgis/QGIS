/***************************************************************************
  qgsauthconfigurationstorageregistry.cpp - QgsAuthConfigurationStorageRegistry

 ---------------------
 begin                : 20.6.2024
 copyright            : (C) 2024 by Alessandro Pasotti
 email                : elpaso at itopen dot it
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/


#include "qgsauthconfigurationstorageregistry.h"
#include "moc_qgsauthconfigurationstorageregistry.cpp"
#include "qgsauthconfigurationstorage.h"
#include "qgslogger.h"
#include "qgsthreadingutils.h"

#include <QMutexLocker>

QgsAuthConfigurationStorageRegistry::QgsAuthConfigurationStorageRegistry()
{
}

QgsAuthConfigurationStorageRegistry::~QgsAuthConfigurationStorageRegistry()
{
}

bool QgsAuthConfigurationStorageRegistry::addStorage( QgsAuthConfigurationStorage *storage )
{

  QGIS_PROTECT_QOBJECT_THREAD_ACCESS

  if ( ! storage )
  {
    return false;
  }

  QMutexLocker locker( &mMutex );

  for ( const auto &s : mStorages )
  {
    if ( s.get() == storage )
    {
      return false;
    }

    if ( s->id() == storage->id() )
    {
      QgsDebugError( QStringLiteral( "A storage with the same ID (%1) already exists" ).arg( storage->id() ) );
      return false;
    }
  }

  mStorages.emplace_back( storage );

  // Forward storageChanged signal from storage to the registry
  connect( storage, &QgsAuthConfigurationStorage::storageChanged, this, &QgsAuthConfigurationStorageRegistry::storageChanged );

  emit storageAdded( storage->id() );

  return true;
}

bool QgsAuthConfigurationStorageRegistry::removeStorage( const QString &id )
{

  QGIS_PROTECT_QOBJECT_THREAD_ACCESS

  QMutexLocker locker( &mMutex );

  for ( auto it = mStorages.begin(); it != mStorages.end(); ++it )
  {
    if ( ( *it )->id() == id )
    {
      mStorages.erase( it );
      emit storageRemoved( id );
      return true;
    }
  }
  return false;
}


QList<QgsAuthConfigurationStorage *> QgsAuthConfigurationStorageRegistry::storages() const
{

  QMutexLocker locker( &mMutex );

  QList<QgsAuthConfigurationStorage *> storageList;
  for ( const auto &s : mStorages )
  {
    storageList.append( s.get() );
  }

  return storageList;
}

QList<QgsAuthConfigurationStorage *> QgsAuthConfigurationStorageRegistry::readyStorages() const
{

  QMutexLocker locker( &mMutex );

  QList<QgsAuthConfigurationStorage *> readyStorages;
  for ( const auto &s : std::as_const( mStorages ) )
  {
    if ( s->isReady() && s->isEnabled() )
    {
      readyStorages.append( s.get() );
    }
  }

  return readyStorages;
}

QList<QgsAuthConfigurationStorage *> QgsAuthConfigurationStorageRegistry::readyStoragesWithCapability( Qgis::AuthConfigurationStorageCapability capability ) const
{

  QMutexLocker locker( &mMutex );

  QList<QgsAuthConfigurationStorage *> readyStorages;
  for ( const auto &s : std::as_const( mStorages ) )
  {
    if ( s->isReady() && s->isEnabled() && s->capabilities().testFlag( capability ) )
    {
      readyStorages.append( s.get() );
    }
  }

  return readyStorages;
}

QgsAuthConfigurationStorage *QgsAuthConfigurationStorageRegistry::firstReadyStorageWithCapability( Qgis::AuthConfigurationStorageCapability capability ) const
{

  QMutexLocker locker( &mMutex );

  for ( const auto &s : std::as_const( mStorages ) )
  {
    if ( s->isReady() && s->isEnabled() && s->capabilities().testFlag( capability ) )
    {
      return s.get();
    }
  }
  return nullptr;

}

QgsAuthConfigurationStorage *QgsAuthConfigurationStorageRegistry::storage( const QString &id ) const
{

  QMutexLocker locker( &mMutex );

  for ( const auto &s : std::as_const( mStorages ) )
  {
    if ( s->id() == id )
    {
      return s.get();
    }
  }
  return nullptr;
}

void QgsAuthConfigurationStorageRegistry::setStorageOrder( const QStringList &orderIds )
{

  QGIS_PROTECT_QOBJECT_THREAD_ACCESS

  QMutexLocker locker( &mMutex );

  std::vector<std::unique_ptr<QgsAuthConfigurationStorage>> orderedStorages;
  for ( const auto &id : std::as_const( orderIds ) )
  {
    for ( auto it = mStorages.begin(); it != mStorages.end(); ++it )
    {
      if ( ( *it )->id() == id )
      {
        orderedStorages.push_back( std::move( *it ) );
        mStorages.erase( it );
        break;
      }
    }
  }

  // Append the remaining storages
  for ( auto it = std::make_move_iterator( mStorages.begin() ); it != std::make_move_iterator( mStorages.end() ); ++it )
  {
    orderedStorages.push_back( std::move( *it ) );
  }

  mStorages = std::move( orderedStorages );
}

