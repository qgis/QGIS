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
#include "qgsproviderguiregistry.h"

#include <memory>

QgsSourceSelectProviderRegistry::QgsSourceSelectProviderRegistry() = default;

QgsSourceSelectProviderRegistry::~QgsSourceSelectProviderRegistry()
{
  qDeleteAll( mProviders );
}

QList<QgsSourceSelectProvider *> QgsSourceSelectProviderRegistry::providers()
{
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
  const int index = mProviders.indexOf( provider );
  if ( index >= 0 )
  {
    delete mProviders.takeAt( index );
    return true;
  }
  return false;
}


void QgsSourceSelectProviderRegistry::initializeFromProviderGuiRegistry( QgsProviderGuiRegistry *providerGuiRegistry )
{
  if ( !providerGuiRegistry )
    return;

  const QStringList providersList = providerGuiRegistry->providerList();
  for ( const QString &key : providersList )
  {
    const QList<QgsSourceSelectProvider *> providerList = providerGuiRegistry->sourceSelectProviders( key );
    // the function is a factory - we keep ownership of the returned providers
    for ( auto provider : providerList )
    {
      addProvider( provider );
    }
  }
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

QgsAbstractDataSourceWidget *QgsSourceSelectProviderRegistry::createSelectionWidget(
  const QString &name,
  QWidget *parent,
  Qt::WindowFlags fl,
  QgsProviderRegistry::WidgetMode widgetMode )
{
  QgsSourceSelectProvider *provider = providerByName( name );
  if ( !provider )
  {
    return nullptr;
  }
  return provider->createDataSourceWidget( parent, fl, widgetMode );
}
