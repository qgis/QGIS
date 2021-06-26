/***************************************************************************
  qgsdataitemguiproviderregistry.cpp
  --------------------------------------
  Date                 : October 2018
  Copyright            : (C) 2018 by Nyall Dawson
  Email                : nyall dot dawson at gmail dot com
 ***************************************************************************
 *                                                                         *
 *   *
 *  
 *        *
 *                                     *
 *                                                                         *
 ***************************************************************************/
#include "qgsdataitemguiproviderregistry.h"

#include "qgsproviderregistry.h"

#include "qgsdataitemguiprovider.h"
#include "qgsproviderguiregistry.h"

QgsDataItemGuiProviderRegistry::QgsDataItemGuiProviderRegistry() = default;

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

void QgsDataItemGuiProviderRegistry::initializeFromProviderGuiRegistry( QgsProviderGuiRegistry *providerGuiRegistry )
{
  if ( !providerGuiRegistry )
    return;

  const QStringList providersList = providerGuiRegistry->providerList();

  for ( const QString &key : providersList )
  {
    const QList<QgsDataItemGuiProvider *> providerList = providerGuiRegistry->dataItemGuiProviders( key );
    // the function is a factory - we keep ownership of the returned providers
    mProviders << providerList;
  }
}
