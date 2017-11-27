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

typedef QList<QgsDataItemProvider *> *dataItemProviders_t();


/**
 * \ingroup core
 * Simple data item provider implementation that handles the support for provider plugins (which may contain
 * dataCapabilities() and dataItem() functions).
 *
 * Ideally the provider plugins should directly provide implementation of QgsDataItemProvider, for the time being
 * this is a wrapper for the legacy interface.
 * \note not available in Python bindings
 */
class QgsDataItemProviderFromPlugin : public QgsDataItemProvider
{
  public:

    /**
     * QgsDataItemProviderFromPlugin constructor
     * \param name plugin name
     * \param capabilitiesFunc function pointer to the data capabilities
     * \param dataItemFunc function pointer to the data items
     * \param handlesDirectoryPathFunc function pointer to handlesDirectoryPath
     */
    QgsDataItemProviderFromPlugin( const QString &name, dataCapabilities_t *capabilitiesFunc, dataItem_t *dataItemFunc, handlesDirectoryPath_t *handlesDirectoryPathFunc )
      : mName( name )
      , mCapabilitiesFunc( capabilitiesFunc )
      , mDataItemFunc( dataItemFunc )
      , mHandlesDirectoryPathFunc( handlesDirectoryPathFunc )
    {
    }

    QString name() override { return mName; }

    int capabilities() override { return mCapabilitiesFunc(); }

    QgsDataItem *createDataItem( const QString &path, QgsDataItem *parentItem ) override { return mDataItemFunc( path, parentItem ); }

    bool handlesDirectoryPath( const QString &path ) override
    {
      if ( mHandlesDirectoryPathFunc )
        return mHandlesDirectoryPathFunc( path );
      else
        return false;
    }

  protected:
    QString mName;
    dataCapabilities_t *mCapabilitiesFunc = nullptr;
    dataItem_t *mDataItemFunc = nullptr;
    handlesDirectoryPath_t *mHandlesDirectoryPathFunc = nullptr;
};


QgsDataItemProviderRegistry::QgsDataItemProviderRegistry()
{
  QStringList providersList = QgsProviderRegistry::instance()->providerList();

  Q_FOREACH ( const QString &key, providersList )
  {
    std::unique_ptr< QLibrary > library( QgsProviderRegistry::instance()->createProviderLibrary( key ) );
    if ( !library )
      continue;

    // new / better way of returning data items from providers

    dataItemProviders_t *dataItemProvidersFn = reinterpret_cast< dataItemProviders_t * >( cast_to_fptr( library->resolve( "dataItemProviders" ) ) );
    if ( dataItemProvidersFn )
    {
      QList<QgsDataItemProvider *> *providerList = dataItemProvidersFn();
      // the function is a factory - we keep ownership of the returned providers
      mProviders << *providerList;
      delete providerList;
    }

    // legacy support - using dataItem() and dataCapabilities() methods

    dataCapabilities_t *dataCapabilities = reinterpret_cast< dataCapabilities_t * >( cast_to_fptr( library->resolve( "dataCapabilities" ) ) );
    if ( !dataCapabilities )
    {
      QgsDebugMsg( library->fileName() + " does not have dataCapabilities" );
      continue;
    }

    dataItem_t *dataItem = reinterpret_cast< dataItem_t * >( cast_to_fptr( library->resolve( "dataItem" ) ) );
    if ( !dataItem )
    {
      QgsDebugMsg( library->fileName() + " does not have dataItem" );
      continue;
    }

    handlesDirectoryPath_t *handlesDirectoryPath = reinterpret_cast< handlesDirectoryPath_t * >( cast_to_fptr( library->resolve( "handlesDirectoryPath" ) ) );

    mProviders.append( new QgsDataItemProviderFromPlugin( library->fileName(), dataCapabilities, dataItem, handlesDirectoryPath ) );
  }
}

QgsDataItemProviderRegistry::~QgsDataItemProviderRegistry()
{
  qDeleteAll( mProviders );
}

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
