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
#include <QRegularExpression>

#include "qgslogger.h"
#include "qgsgdalguiprovider.h"
#include "qgsogrguiprovider.h"
#include "qgsvectortileproviderguimetadata.h"
#include "qgspointcloudproviderguimetadata.h"
#include "qgsmaplayerconfigwidgetfactory.h"

#ifdef HAVE_EPT
#include "qgseptproviderguimetadata.h"
#endif

#ifdef HAVE_COPC
#include "qgscopcproviderguimetadata.h"
#endif

#ifdef HAVE_STATIC_PROVIDERS
#include "qgswmsprovidergui.h"
#include "qgswcsprovidergui.h"
#include "qgsdelimitedtextprovidergui.h"
#include "qgsarcgisrestprovidergui.h"
#ifdef HAVE_SPATIALITE
#include "qgsspatialiteprovidergui.h"
#include "qgswfsprovidergui.h"
#include "qgsvirtuallayerprovidergui.h"
#endif
#ifdef HAVE_POSTGRESQL
#include "qgspostgresprovidergui.h"
#endif
#endif

/**
 * Convenience function for finding any existing data providers that match "providerKey"
 *
 * Necessary because [] map operator will create a QgsProviderGuiMetadata
 * instance.  Also you cannot use the map [] operator in const members for that
 * very reason.  So there needs to be a convenient way to find a data provider
 * without accidentally adding a null meta data item to the metadata map.
*/
static
QgsProviderGuiMetadata *findMetadata_( QgsProviderGuiRegistry::GuiProviders const &metaData,
                                       QString const &providerKey )
{
  const QgsProviderGuiRegistry::GuiProviders::const_iterator i = metaData.find( providerKey );
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

#ifdef HAVE_EPT
  QgsProviderGuiMetadata *ept = new QgsEptProviderGuiMetadata();
  mProviders[ ept->key() ] = ept;
#endif

#ifdef HAVE_COPC
  QgsProviderGuiMetadata *copc = new QgsCopcProviderGuiMetadata();
  mProviders[ copc->key() ] = copc;
#endif

  // only show point cloud option if we have at least one point cloud provider available!
  if ( !QgsProviderRegistry::instance()->filePointCloudFilters().isEmpty() )
  {
    QgsProviderGuiMetadata *pointcloud = new QgsPointCloudProviderGuiMetadata();
    mProviders[ pointcloud->key() ] = pointcloud;
  }

#ifdef HAVE_STATIC_PROVIDERS
  QgsProviderGuiMetadata *wms = new QgsWmsProviderGuiMetadata();
  mProviders[ wms->key() ] = wms;
  QgsProviderGuiMetadata *wcs = new QgsWcsProviderGuiMetadata();
  mProviders[ wcs->key() ] = wcs;
  QgsProviderGuiMetadata *delimitedtext = new QgsDelimitedTextProviderGuiMetadata();
  mProviders[ delimitedtext->key() ] = delimitedtext;
  QgsProviderGuiMetadata *arc = new QgsArcGisRestProviderGuiMetadata();
  mProviders[ arc->key() ] = arc;
#ifdef HAVE_SPATIALITE
  QgsProviderGuiMetadata *spatialite = new QgsSpatiaLiteProviderGuiMetadata();
  mProviders[ spatialite->key() ] = spatialite;
  QgsProviderGuiMetadata *wfs = new QgsWfsProviderGuiMetadata();
  mProviders[ wfs->key() ] = wfs;
  QgsProviderGuiMetadata *virtuallayer = new QgsVirtualLayerProviderGuiMetadata();
  mProviders[ virtuallayer->key() ] = virtuallayer;
#endif
#ifdef HAVE_POSTGRESQL
  QgsProviderGuiMetadata *postgres = new QgsPostgresProviderGuiMetadata();
  mProviders[ postgres->key() ] = postgres;
#endif
#endif
}

void QgsProviderGuiRegistry::loadDynamicProviders( const QString &pluginPath )
{
#ifdef HAVE_STATIC_PROVIDERS
  Q_UNUSED( pluginPath )
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

  QgsDebugMsgLevel( QStringLiteral( "Checking %1 for GUI provider plugins" ).arg( mLibraryDirectory.path() ), 2 );

  if ( mLibraryDirectory.count() == 0 )
  {
    QgsDebugMsg( QStringLiteral( "No dynamic QGIS GUI provider plugins found in:\n%1\n" ).arg( mLibraryDirectory.path() ) );
  }

  // provider file regex pattern, only files matching the pattern are loaded if the variable is defined
  const QString filePattern = getenv( "QGIS_PROVIDER_FILE" );
  QRegularExpression fileRegexp;
  if ( !filePattern.isEmpty() )
  {
    fileRegexp.setPattern( filePattern );
  }

  const auto constEntryInfoList = mLibraryDirectory.entryInfoList();
  for ( const QFileInfo &fi : constEntryInfoList )
  {
    if ( !fileRegexp.pattern().isEmpty() )
    {
      const QRegularExpressionMatch fileNameMatch = fileRegexp.match( fi.fileName() );
      if ( !fileNameMatch.hasMatch() )
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

QList<QgsSubsetStringEditorProvider *> QgsProviderGuiRegistry::subsetStringEditorProviders( const QString &providerKey )
{
  QgsProviderGuiMetadata *meta = findMetadata_( mProviders, providerKey );
  if ( meta )
    return meta->subsetStringEditorProviders();
  return QList<QgsSubsetStringEditorProvider *>();
}

QList<QgsProviderSourceWidgetProvider *> QgsProviderGuiRegistry::sourceWidgetProviders( const QString &providerKey )
{
  QgsProviderGuiMetadata *meta = findMetadata_( mProviders, providerKey );
  if ( meta )
    return meta->sourceWidgetProviders();
  return QList<QgsProviderSourceWidgetProvider *>();
}

QList<const QgsMapLayerConfigWidgetFactory *> QgsProviderGuiRegistry::mapLayerConfigWidgetFactories( QgsMapLayer *layer )
{
  QList<const QgsMapLayerConfigWidgetFactory *> res;
  for ( GuiProviders::const_iterator it = mProviders.begin(); it != mProviders.end(); ++it )
  {
    const QList<const QgsMapLayerConfigWidgetFactory *> providerFactories = ( *it ).second->mapLayerConfigWidgetFactories();
    for ( const QgsMapLayerConfigWidgetFactory *factory : providerFactories )
    {
      if ( !layer || factory->supportsLayer( layer ) )
        res << factory;
    }
  }
  return res;
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
