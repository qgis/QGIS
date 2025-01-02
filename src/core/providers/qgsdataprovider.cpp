/***************************************************************************
    qgsdataprovider.cpp - DataProvider Interface
     --------------------------------------
    Date                 : May 2016
    Copyright            : (C) 2016 by Matthias Kuhn
    email                : matthias@opengis.ch
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include <QMutexLocker>
#include "qgsdataprovider.h"
#include "moc_qgsdataprovider.cpp"
#include "qgsdataprovidertemporalcapabilities.h"
#include "qgsthreadingutils.h"

#define SUBLAYER_SEPARATOR QStringLiteral( "!!::!!" )

QgsDataProvider::QgsDataProvider( const QString &uri, const QgsDataProvider::ProviderOptions &providerOptions,
                                  Qgis::DataProviderReadFlags flags )
  : mDataSourceURI( uri ),
    mOptions( providerOptions )
{
  mReadFlags = flags;
}

QString QgsDataProvider::htmlMetadata() const
{
  QGIS_PROTECT_QOBJECT_THREAD_ACCESS

  return QString();
}

Qgis::DataProviderFlags QgsDataProvider::flags() const
{
  QGIS_PROTECT_QOBJECT_THREAD_ACCESS

  return Qgis::DataProviderFlags();
}

QgsDataProviderTemporalCapabilities *QgsDataProvider::temporalCapabilities()
{
  QGIS_PROTECT_QOBJECT_THREAD_ACCESS

  return nullptr;
}

const QgsDataProviderTemporalCapabilities *QgsDataProvider::temporalCapabilities() const
{
  QGIS_PROTECT_QOBJECT_THREAD_ACCESS

  return nullptr;
}

QgsDataProviderElevationProperties *QgsDataProvider::elevationProperties()
{
  QGIS_PROTECT_QOBJECT_THREAD_ACCESS

  return nullptr;
}

const QgsDataProviderElevationProperties *QgsDataProvider::elevationProperties() const
{
  QGIS_PROTECT_QOBJECT_THREAD_ACCESS

  return nullptr;
}

QString QgsDataProvider::subsetStringDialect() const
{
  return QString();
}

QString QgsDataProvider::subsetStringHelpUrl() const
{
  return QString();
}

bool QgsDataProvider::setSubsetString( const QString &subset, bool updateFeatureCount )
{
  // NOP by default
  Q_UNUSED( subset )
  Q_UNUSED( updateFeatureCount )
  return false;
}

bool QgsDataProvider::supportsSubsetString() const
{
  return false;
}

QString QgsDataProvider::subsetString() const
{
  return QString();
}

void QgsDataProvider::reloadData()
{
  // Because QgsVirtualLayerTask is not thread safe:
  QGIS_PROTECT_QOBJECT_THREAD_ACCESS_NON_FATAL

  reloadProviderData();
  emit dataChanged();
}

void QgsDataProvider::setProviderProperty( QgsDataProvider::ProviderProperty property, const QVariant &value )
{
  QGIS_PROTECT_QOBJECT_THREAD_ACCESS

  mProviderProperties.insert( property, value );
}

void QgsDataProvider::setProviderProperty( int property, const QVariant &value )
{
  QGIS_PROTECT_QOBJECT_THREAD_ACCESS

  mProviderProperties.insert( property, value );
}

QVariant QgsDataProvider::providerProperty( QgsDataProvider::ProviderProperty property, const QVariant &defaultValue ) const
{
  QGIS_PROTECT_QOBJECT_THREAD_ACCESS

  return mProviderProperties.value( property, defaultValue );
}

QVariant QgsDataProvider::providerProperty( int property, const QVariant &defaultValue = QVariant() ) const
{
  QGIS_PROTECT_QOBJECT_THREAD_ACCESS

  return mProviderProperties.value( property, defaultValue );
}

void QgsDataProvider::setListening( bool isListening )
{
  QGIS_PROTECT_QOBJECT_THREAD_ACCESS

  Q_UNUSED( isListening )
}

bool QgsDataProvider::renderInPreview( const PreviewContext &context )
{
  QGIS_PROTECT_QOBJECT_THREAD_ACCESS

  return context.lastRenderingTimeMs <= context.maxRenderingTimeMs;
}

QgsCoordinateTransformContext QgsDataProvider::transformContext() const
{
  QGIS_PROTECT_QOBJECT_THREAD_ACCESS

  const QMutexLocker locker( &mOptionsMutex );
  return mOptions.transformContext;
}

void QgsDataProvider::setTransformContext( const QgsCoordinateTransformContext &value )
{
  QGIS_PROTECT_QOBJECT_THREAD_ACCESS

  const QMutexLocker locker( &mOptionsMutex );
  mOptions.transformContext = value;
}

QString QgsDataProvider::sublayerSeparator()
{
  return SUBLAYER_SEPARATOR;
}

Qgis::ProviderStyleStorageCapabilities QgsDataProvider::styleStorageCapabilities() const
{
  return Qgis::ProviderStyleStorageCapabilities();
}
