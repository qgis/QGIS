/***************************************************************************
    qgsprovidersourcewidgetproviderregistry.cpp
     ----------------------------------------
    Date                 : December 2020
    Copyright            : (C) 2020 by Nyall Dawson
    Email                : nyall dot dawson at gmail dot com
****************************************************************************/
/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "qgsprovidersourcewidgetproviderregistry.h"
#include "qgsprovidersourcewidgetprovider.h"
#include "qgsproviderguiregistry.h"

#include <memory>

QgsProviderSourceWidgetProviderRegistry::QgsProviderSourceWidgetProviderRegistry() = default;

QgsProviderSourceWidgetProviderRegistry::~QgsProviderSourceWidgetProviderRegistry()
{
  qDeleteAll( mProviders );
}

QList<QgsProviderSourceWidgetProvider *> QgsProviderSourceWidgetProviderRegistry::providers()
{
  return mProviders;
}

void QgsProviderSourceWidgetProviderRegistry::addProvider( QgsProviderSourceWidgetProvider *provider )
{
  mProviders.append( provider );
}

bool QgsProviderSourceWidgetProviderRegistry::removeProvider( QgsProviderSourceWidgetProvider *provider )
{
  const int index = mProviders.indexOf( provider );
  if ( index >= 0 )
  {
    delete mProviders.takeAt( index );
    return true;
  }
  return false;
}


void QgsProviderSourceWidgetProviderRegistry::initializeFromProviderGuiRegistry( QgsProviderGuiRegistry *providerGuiRegistry )
{
  if ( !providerGuiRegistry )
    return;

  const QStringList providersList = providerGuiRegistry->providerList();
  for ( const QString &key : providersList )
  {
    const QList<QgsProviderSourceWidgetProvider *> providerList = providerGuiRegistry->sourceWidgetProviders( key );
    // the function is a factory - we keep ownership of the returned providers
    for ( QgsProviderSourceWidgetProvider *provider : providerList )
    {
      addProvider( provider );
    }
  }
}

QgsProviderSourceWidgetProvider *QgsProviderSourceWidgetProviderRegistry::providerByName( const QString &name )
{
  const QList<QgsProviderSourceWidgetProvider *> providerList = providers();
  for ( QgsProviderSourceWidgetProvider *provider :  providerList )
  {
    if ( provider->name() == name )
    {
      return provider;
    }
  }
  return nullptr;
}

QList<QgsProviderSourceWidgetProvider *> QgsProviderSourceWidgetProviderRegistry::providersByKey( const QString &providerKey )
{
  QList<QgsProviderSourceWidgetProvider *> result;
  const QList<QgsProviderSourceWidgetProvider *> providerList = providers();
  for ( QgsProviderSourceWidgetProvider *provider : providerList )
  {
    if ( provider->providerKey() == providerKey )
    {
      result << provider;
    }
  }
  return result;
}

QgsProviderSourceWidget *QgsProviderSourceWidgetProviderRegistry::createWidget( QgsMapLayer *layer, QWidget *parent )
{
  const QList<QgsProviderSourceWidgetProvider *> providerList = providers();
  // Loop over providers to find one that can handle the layer.
  for ( QgsProviderSourceWidgetProvider *provider : providerList )
  {
    if ( provider->canHandleLayer( layer ) )
    {
      return provider->createWidget( layer, parent );
    }
  }

  return nullptr;
}
