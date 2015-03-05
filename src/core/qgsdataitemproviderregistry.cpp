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


/** Simple data item provider implementation that handles the support for provider plugins (which may contain
 * dataCapabilities() and dataItem() functions).
 *
 * Ideally the provider plugins should directly provide implementation of QgsDataItemProvider, for the time being
 * this is a wrapper for the legacy interface.
 */
class QgsDataItemProviderFromPlugin : public QgsDataItemProvider
{
  public:
    QgsDataItemProviderFromPlugin( const QString& name, dataCapabilities_t* capabilitiesFunc, dataItem_t* dataItemFunc )
        : mName( name )
        , mCapabilitiesFunc( capabilitiesFunc )
        , mDataItemFunc( dataItemFunc )
    {
    }

    virtual QString name() { return mName; }

    virtual int capabilities() { return mCapabilitiesFunc(); }

    virtual QgsDataItem* createDataItem( const QString& path, QgsDataItem* parentItem ) { return mDataItemFunc( path, parentItem ); }

  protected:
    QString mName;
    dataCapabilities_t* mCapabilitiesFunc;
    dataItem_t* mDataItemFunc;
};


QgsDataItemProviderRegistry::QgsDataItemProviderRegistry()
{
  QStringList providersList = QgsProviderRegistry::instance()->providerList();

  foreach ( QString key, providersList )
  {
    QLibrary *library = QgsProviderRegistry::instance()->providerLibrary( key );
    if ( !library )
      continue;

    dataCapabilities_t * dataCapabilities = ( dataCapabilities_t * ) cast_to_fptr( library->resolve( "dataCapabilities" ) );
    if ( !dataCapabilities )
    {
      QgsDebugMsg( library->fileName() + " does not have dataCapabilities" );
      continue;
    }

    dataItem_t *dataItem = ( dataItem_t * ) cast_to_fptr( library->resolve( "dataItem" ) );
    if ( !dataItem )
    {
      QgsDebugMsg( library->fileName() + " does not have dataItem" );
      continue;
    }

    mProviders.append( new QgsDataItemProviderFromPlugin( library->fileName(), dataCapabilities, dataItem ) );
  }
}

QgsDataItemProviderRegistry::~QgsDataItemProviderRegistry()
{
  qDeleteAll( mProviders );
}

void QgsDataItemProviderRegistry::addProvider( QgsDataItemProvider* provider )
{
  mProviders.append( provider );
}

void QgsDataItemProviderRegistry::removeProvider( QgsDataItemProvider* provider )
{
  int index = mProviders.indexOf( provider );
  if ( index >= 0 )
    delete mProviders.takeAt( index );
}
