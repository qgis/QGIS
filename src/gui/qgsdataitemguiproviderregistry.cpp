/***************************************************************************
  qgsdataitemguiproviderregistry.cpp
  --------------------------------------
  Date                 : October 2018
  Copyright            : (C) 2018 by Nyall Dawson
  Email                : nyall dot dawson at gmail dot com
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "qgsdataitemguiproviderregistry.h"
#include "qgsdataitemguiprovider.h"
#include "qgsproviderregistry.h"

typedef QList<QgsDataItemGuiProvider *> *dataItemGuiProviders_t();

QgsDataItemGuiProviderRegistry::QgsDataItemGuiProviderRegistry()
{
  const QStringList providersList = QgsProviderRegistry::instance()->providerList();

  for ( const QString &key : providersList )
  {
    std::unique_ptr< QLibrary > library( QgsProviderRegistry::instance()->createProviderLibrary( key ) );
    if ( !library )
      continue;

    if ( dataItemGuiProviders_t *dataItemGuiProvidersFn = reinterpret_cast< dataItemGuiProviders_t * >( cast_to_fptr( library->resolve( "dataItemGuiProviders" ) ) ) )
    {
      const QList<QgsDataItemGuiProvider *> *providerList = dataItemGuiProvidersFn();
      // the function is a factory - we keep ownership of the returned providers
      mProviders << *providerList;
      delete providerList;
    }
  }
}

QgsDataItemGuiProviderRegistry::~QgsDataItemGuiProviderRegistry()
{
  qDeleteAll( mProviders );
}

void QgsDataItemGuiProviderRegistry::addProvider( QgsDataItemGuiProvider *provider )
{
  mProviders.append( provider );
}

void QgsDataItemGuiProviderRegistry::removeProvider( QgsDataItemGuiProvider *provider )
{
  int index = mProviders.indexOf( provider );
  if ( index >= 0 )
    delete mProviders.takeAt( index );
}
