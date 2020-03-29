/***************************************************************************
                    qgsproviderrguiegistry.cpp
                             -------------------
    begin                : June 2019
    copyright            : (C) 2019 by Peter Petrik
    email                : zilolv at google dot com
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "qgsproviderguiregistry.h"

#include <QString>
#include <QDir>
#include <QLibrary>

#include "qgslogger.h"
#include "qgsgdalguiprovider.h"
#include "qgsogrguiprovider.h"
#include "qgsvectortileproviderguimetadata.h"

#ifdef HAVE_STATIC_PROVIDERS
#include "qgswmsprovidergui.h"
#include "qgspostgresprovidergui.h"
#endif

/**
 * Convenience function for finding any existing data providers that match "providerKey"

  Necessary because [] map operator will create a QgsProviderGuiMetadata
  instance.  Also you cannot use the map [] operator in const members for that
  very reason.  So there needs to be a convenient way to find a data provider
  without accidentally adding a null meta data item to the metadata map.
*/
static
QgsProviderGuiMetadata *findMetadata_( QgsProviderGuiRegistry::GuiProviders const &metaData,
                                       QString const &providerKey )
{
  QgsProviderGuiRegistry::GuiProviders::const_iterator i = metaData.find( providerKey );
  if ( i != metaData.end() )
  {
    return i->second;
  }

  return nullptr;
} // findMetadata_

QgsProviderGuiRegistry::QgsProviderGuiRegistry( const QString &pluginPath )
{
  loadStaticProviders();
  loadDynamicProviders( pluginPath );
}

void QgsProviderGuiRegistry::loadStaticProviders( )
{
  // Register static providers
  QgsProviderGuiMetadata *gdal = new QgsGdalGuiProviderMetadata();
  mProviders[ gdal->key() ] = gdal;

  QgsProviderGuiMetadata *ogr = new QgsOgrGuiProviderMetadata();
  mProviders[ ogr->key() ] = ogr;

  QgsProviderGuiMetadata *vt = new QgsVectorTileProviderGuiMetadata();
  mProviders[ vt->key() ] = vt;

#ifdef HAVE_STATIC_PROVIDERS
  QgsProviderGuiMetadata *wms = new QgsWmsProviderGuiMetadata();
  mProviders[ wms->key() ] = wms;

  QgsProviderGuiMetadata *postgres = new QgsPostgresProviderGuiMetadata();
  mProviders[ postgres->key() ] = postgres;
#endif
}

void QgsProviderGuiRegistry::loadDynamicProviders( const QString &pluginPath )
{
#ifdef HAVE_STATIC_PROVIDERS
  QgsDebugMsg( QStringLiteral( "Forced only static GUI providers" ) );
#else
  typedef QgsProviderGuiMetadata *factory_function( );

  // add dynamic providers
  QDir mLibraryDirectory( pluginPath );
  mLibraryDirectory.setSorting( QDir::Name | QDir::IgnoreCase );
  mLibraryDirectory.setFilter( QDir::Files | QDir::NoSymLinks );

#if defined(Q_OS_WIN) || defined(__CYGWIN__)
  mLibraryDirectory.setNameFilters( QStringList( "*.dll" ) );
#elif defined(ANDROID)
  mLibraryDirectory.setNameFilters( QStringList( "*provider.so" ) );
#else
  mLibraryDirectory.setNameFilters( QStringList( QStringLiteral( "*.so" ) ) );
#endif

  QgsDebugMsg( QStringLiteral( "Checking %1 for GUI provider plugins" ).arg( mLibraryDirectory.path() ) );

  if ( mLibraryDirectory.count() == 0 )
  {
    QgsDebugMsg( QStringLiteral( "No dynamic QGIS GUI provider plugins found in:\n%1\n" ).arg( mLibraryDirectory.path() ) );
  }

  // provider file regex pattern, only files matching the pattern are loaded if the variable is defined
  QString filePattern = getenv( "QGIS_PROVIDER_FILE" );
  QRegExp fileRegexp;
  if ( !filePattern.isEmpty() )
  {
    fileRegexp.setPattern( filePattern );
  }

  const auto constEntryInfoList = mLibraryDirectory.entryInfoList();
  for ( const QFileInfo &fi : constEntryInfoList )
  {
    if ( !fileRegexp.isEmpty() )
    {
      if ( fileRegexp.indexIn( fi.fileName() ) == -1 )
      {
        QgsDebugMsg( "provider " + fi.fileName() + " skipped because doesn't match pattern " + filePattern );
        continue;
      }
    }

    QLibrary myLib( fi.filePath() );
    if ( myLib.load() )
    {
      QFunctionPointer func = myLib.resolve( QStringLiteral( "providerGuiMetadataFactory" ).toLatin1().data() );
      factory_function *function = reinterpret_cast< factory_function * >( cast_to_fptr( func ) );
      if ( !function )
        continue;

      QgsProviderGuiMetadata *meta = function( );

      if ( !meta )
        continue;

      const QString providerKey = meta->key();

      // check if such providers is already registered
      if ( findMetadata_( mProviders, providerKey ) )
        continue;

      mProviders[providerKey] = meta;
    }
  }
#endif
}

QgsProviderGuiRegistry::~QgsProviderGuiRegistry()
{
  GuiProviders::const_iterator it = mProviders.begin();
  while ( it != mProviders.end() )
  {
    delete it->second;
    ++it;
  }
  mProviders.clear();
}

void QgsProviderGuiRegistry::registerGuis( QMainWindow *parent )
{
  GuiProviders::const_iterator it = mProviders.begin();
  while ( it != mProviders.end() )
  {
    it->second->registerGui( parent );
    ++it;
  }
}

const QList<QgsDataItemGuiProvider *> QgsProviderGuiRegistry::dataItemGuiProviders( const QString &providerKey )
{
  QgsProviderGuiMetadata *meta = findMetadata_( mProviders, providerKey );
  if ( meta )
    return meta->dataItemGuiProviders();
  return QList<QgsDataItemGuiProvider *>();
}

QList<QgsSourceSelectProvider *> QgsProviderGuiRegistry::sourceSelectProviders( const QString &providerKey )
{
  QgsProviderGuiMetadata *meta = findMetadata_( mProviders, providerKey );
  if ( meta )
    return meta->sourceSelectProviders();
  return QList<QgsSourceSelectProvider *> ();
}

QList<QgsProjectStorageGuiProvider *> QgsProviderGuiRegistry::projectStorageGuiProviders( const QString &providerKey )
{
  QgsProviderGuiMetadata *meta = findMetadata_( mProviders, providerKey );
  if ( meta )
    return meta->projectStorageGuiProviders();
  return QList<QgsProjectStorageGuiProvider *>();
}

QStringList QgsProviderGuiRegistry::providerList() const
{
  QStringList lst;
  for ( GuiProviders::const_iterator it = mProviders.begin(); it != mProviders.end(); ++it )
  {
    lst.append( it->first );
  }
  return lst;
}

const QgsProviderGuiMetadata *QgsProviderGuiRegistry::providerMetadata( const QString &providerKey ) const
{
  return findMetadata_( mProviders, providerKey );
}
