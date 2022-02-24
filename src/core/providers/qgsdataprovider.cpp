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
#include "qgsdataprovidertemporalcapabilities.h"

#define SUBLAYER_SEPARATOR QStringLiteral( "!!::!!" )

QgsDataProvider::QgsDataProvider( const QString &uri, const QgsDataProvider::ProviderOptions &providerOptions,
                                  QgsDataProvider::ReadFlags flags )
  : mDataSourceURI( uri ),
    mOptions( providerOptions )
{
  mReadFlags = flags;
}

Qgis::DataProviderFlags QgsDataProvider::flags() const
{
  return Qgis::DataProviderFlags();
}

QgsDataProviderTemporalCapabilities *QgsDataProvider::temporalCapabilities()
{
  return nullptr;
}

const QgsDataProviderTemporalCapabilities *QgsDataProvider::temporalCapabilities() const
{
  return nullptr;
}

void QgsDataProvider::reloadData()
{
  reloadProviderData();
  emit dataChanged();
}

void QgsDataProvider::setProviderProperty( QgsDataProvider::ProviderProperty property, const QVariant &value )
{
  mProviderProperties.insert( property, value );
}

void QgsDataProvider::setProviderProperty( int property, const QVariant &value )
{
  mProviderProperties.insert( property, value );
}

QVariant QgsDataProvider::providerProperty( QgsDataProvider::ProviderProperty property, const QVariant &defaultValue ) const
{
  return mProviderProperties.value( property, defaultValue );
}

QVariant QgsDataProvider::providerProperty( int property, const QVariant &defaultValue = QVariant() ) const
{
  return mProviderProperties.value( property, defaultValue );
}

void QgsDataProvider::setListening( bool isListening )
{
  Q_UNUSED( isListening )
}

bool QgsDataProvider::renderInPreview( const PreviewContext &context )
{
  return context.lastRenderingTimeMs <= context.maxRenderingTimeMs;
}

QgsCoordinateTransformContext QgsDataProvider::transformContext() const
{
  const QMutexLocker locker( &mOptionsMutex );
  return mOptions.transformContext;
}

void QgsDataProvider::setTransformContext( const QgsCoordinateTransformContext &value )
{
  const QMutexLocker locker( &mOptionsMutex );
  mOptions.transformContext = value;
}

QString QgsDataProvider::sublayerSeparator()
{
  return SUBLAYER_SEPARATOR;
}
