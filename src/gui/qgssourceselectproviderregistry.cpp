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


QgsSourceSelectProviderRegistry::QgsSourceSelectProviderRegistry()
{
  QStringList providersList = QgsProviderRegistry::instance()->providerList();

  Q_FOREACH ( const QString &key, providersList )
  {
    std::unique_ptr< QLibrary > library( QgsProviderRegistry::instance()->createProviderLibrary( key ) );
    if ( !library )
      continue;

    sourceSelectProviders_t *sourceSelectProvidersFn = reinterpret_cast< sourceSelectProviders_t * >( cast_to_fptr( library->resolve( "sourceSelectProviders" ) ) );
    if ( sourceSelectProvidersFn )
    {
      QList<QgsSourceSelectProvider *> *providerList = sourceSelectProvidersFn();
      // the function is a factory - we keep ownership of the returned providers
      for ( auto provider : qgsAsConst( *providerList ) )
      {
        addProvider( provider );
      }
      delete providerList;
    }
  }
}

QgsSourceSelectProviderRegistry::~QgsSourceSelectProviderRegistry()
{
  qDeleteAll( mProviders );
}

void QgsSourceSelectProviderRegistry::addProvider( QgsSourceSelectProvider *provider )
{
  mProviders.append( provider );
  std::sort( mProviders.begin(), mProviders.end(), [ ]( const QgsSourceSelectProvider * first, const QgsSourceSelectProvider * second ) -> bool
  {
    return first->ordering() < second->ordering();
  } );
}

void QgsSourceSelectProviderRegistry::removeProvider( QgsSourceSelectProvider *provider )
{
  int index = mProviders.indexOf( provider );
  if ( index >= 0 )
    delete mProviders.takeAt( index );
}

QgsSourceSelectProvider *QgsSourceSelectProviderRegistry::providerByName( const QString &name ) const
{
  for ( const auto provider : qgsAsConst( mProviders ) )
  {
    if ( provider->name() == name )
    {
      return provider;
    }
  }
  return nullptr;
}

QList<QgsSourceSelectProvider *> QgsSourceSelectProviderRegistry::providersByKey( const QString &providerKey ) const
{
  QList<QgsSourceSelectProvider *> providerList;
  for ( const auto provider : qgsAsConst( mProviders ) )
  {
    if ( provider->providerKey() == providerKey )
    {
      providerList << provider;
    }
  }
  return providerList;
}
