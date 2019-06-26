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

#include "qgsdataitem.h"
#include "qgsdataitemprovider.h"
#include "qgsdataprovider.h"
#include "qgslogger.h"
#include "qgsproviderregistry.h"

QgsDataItemProviderRegistry::QgsDataItemProviderRegistry()
{
  QStringList providersList = QgsProviderRegistry::instance()->providerList();

  const auto constProvidersList = providersList;
  for ( const QString &key : constProvidersList )
  {
    QList<QgsDataItemProvider *> providerList = QgsProviderRegistry::instance()->dataItemProviders( key );
    mProviders << providerList;
  }
}

QgsDataItemProviderRegistry::~QgsDataItemProviderRegistry()
{
  qDeleteAll( mProviders );
}

QList<QgsDataItemProvider *> QgsDataItemProviderRegistry::providers() const { return mProviders; }

void QgsDataItemProviderRegistry::addProvider( QgsDataItemProvider *provider )
{
  mProviders.append( provider );
}

void QgsDataItemProviderRegistry::removeProvider( QgsDataItemProvider *provider )
{
  int index = mProviders.indexOf( provider );
  if ( index >= 0 )
    delete mProviders.takeAt( index );
}
