/***************************************************************************
  qgsdataitemproviderregistry.cpp
  --------------------------------------
  Date                 : March 2015
  Copyright            : (C) 2015 by Martin Dobias
  Email                : wonder dot sk at gmail dot com
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "qgsdataitemproviderregistry.h"
#include "moc_qgsdataitemproviderregistry.cpp"

#include "qgsdataitem.h"
#include "qgsdataitemprovider.h"
#include "qgsdataprovider.h"
#include "qgslogger.h"
#include "qgsproviderregistry.h"
#include "qgsfilebaseddataitemprovider.h"

QgsDataItemProviderRegistry::QgsDataItemProviderRegistry()
{
  mProviders << new QgsFileBasedDataItemProvider();

  QStringList providersList = QgsProviderRegistry::instance()->providerList();

  const auto constProvidersList = providersList;
  for ( const QString &key : constProvidersList )
  {
    QList<QgsDataItemProvider *> providerList = QgsProviderRegistry::instance()->dataItemProviders( key );
    mProviders << providerList;
    for ( const auto &p : std::as_const( providerList ) )
    {
      if ( ! p->dataProviderKey().isEmpty() )
      {
        mDataItemProviderOrigin[ p->name() ] = p->dataProviderKey();
      }
    }
  }
}

QgsDataItemProviderRegistry::~QgsDataItemProviderRegistry()
{
  qDeleteAll( mProviders );
}

QList<QgsDataItemProvider *> QgsDataItemProviderRegistry::providers() const { return mProviders; }

QgsDataItemProvider *QgsDataItemProviderRegistry::provider( const QString &providerName ) const
{
  for ( const auto &p : std::as_const( mProviders ) )
  {
    if ( p->name() == providerName )
    {
      return p;
    }
  }
  return nullptr;
}

void QgsDataItemProviderRegistry::addProvider( QgsDataItemProvider *provider )
{
  if ( ! provider->dataProviderKey().isEmpty() )
  {
    mDataItemProviderOrigin[ provider->name() ] = provider->dataProviderKey();
  }
  mProviders.append( provider );
  emit providerAdded( provider );
}

void QgsDataItemProviderRegistry::removeProvider( QgsDataItemProvider *provider )
{
  int index = mProviders.indexOf( provider );
  if ( index >= 0 )
  {
    emit providerWillBeRemoved( provider );
    delete mProviders.takeAt( index );
  }
}

QString QgsDataItemProviderRegistry::dataProviderKey( const QString &dataItemProviderName )
{
  return mDataItemProviderOrigin.value( dataItemProviderName, QString() );
}
