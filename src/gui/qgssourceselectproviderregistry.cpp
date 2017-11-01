/***************************************************************************
  qgssourceselectproviderregistry.cpp - QgsSourceSelectProviderRegistry

 ---------------------
 begin                : 1.9.2017
 copyright            : (C) 2017 by Alessandro Pasotti
 email                : apasotti at boundlessgeo dot com
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/
#include "qgssourceselectprovider.h"
#include "qgssourceselectproviderregistry.h"
#include "qgsproviderregistry.h"

#include <memory>

typedef QList<QgsSourceSelectProvider *> *sourceSelectProviders_t();

QgsSourceSelectProviderRegistry::~QgsSourceSelectProviderRegistry()
{
  qDeleteAll( mProviders );
}

QList<QgsSourceSelectProvider *> QgsSourceSelectProviderRegistry::providers()
{
  init();
  return mProviders;
}

void QgsSourceSelectProviderRegistry::addProvider( QgsSourceSelectProvider *provider )
{
  mProviders.append( provider );
  std::sort( mProviders.begin(), mProviders.end(), [ ]( const QgsSourceSelectProvider * first, const QgsSourceSelectProvider * second ) -> bool
  {
    return first->ordering() < second->ordering();
  } );
}

bool QgsSourceSelectProviderRegistry::removeProvider( QgsSourceSelectProvider *provider )
{
  int index = mProviders.indexOf( provider );
  if ( index >= 0 )
  {
    delete mProviders.takeAt( index );
    return true;
  }
  return false;
}

QgsSourceSelectProvider *QgsSourceSelectProviderRegistry::providerByName( const QString &name )
{
  const QList<QgsSourceSelectProvider *> providerList = providers();
  for ( const auto provider :  providerList )
  {
    if ( provider->name() == name )
    {
      return provider;
    }
  }
  return nullptr;
}

QList<QgsSourceSelectProvider *> QgsSourceSelectProviderRegistry::providersByKey( const QString &providerKey )
{
  QList<QgsSourceSelectProvider *> result;
  const QList<QgsSourceSelectProvider *> providerList = providers();
  for ( const auto provider : providerList )
  {
    if ( provider->providerKey() == providerKey )
    {
      result << provider;
    }
  }
  return result;
}

void QgsSourceSelectProviderRegistry::init()
{
  if ( mInitialized )
  {
    return;
  }
  const QStringList providersList = QgsProviderRegistry::instance()->providerList();
  for ( const QString &key : providersList )
  {
    std::unique_ptr< QLibrary > library( QgsProviderRegistry::instance()->createProviderLibrary( key ) );
    if ( !library )
      continue;

    sourceSelectProviders_t *sourceSelectProvidersFn = reinterpret_cast< sourceSelectProviders_t * >( cast_to_fptr( library->resolve( "sourceSelectProviders" ) ) );
    if ( sourceSelectProvidersFn )
    {
      QList<QgsSourceSelectProvider *> *providerList = sourceSelectProvidersFn();
      // the function is a factory - we keep ownership of the returned providers
      for ( auto provider : qgis::as_const( *providerList ) )
      {
        addProvider( provider );
      }
      delete providerList;
    }
  }
  mInitialized = true;
}

