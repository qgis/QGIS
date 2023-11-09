/***************************************************************************
    qgspluginlayer.cpp
    ---------------------
    begin                : January 2010
    copyright            : (C) 2010 by Martin Dobias
    email                : wonder dot sk at gmail dot com
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/
#include "qgspluginlayer.h"

QgsPluginLayer::QgsPluginLayer( const QString &layerType, const QString &layerName )
  : QgsMapLayer( Qgis::LayerType::Plugin, layerName )
  , mPluginLayerType( layerType )
{
  mDataProvider = new QgsPluginLayerDataProvider( layerType, QgsDataProvider::ProviderOptions(), QgsDataProvider::ReadFlags() );
}

QgsPluginLayer::~QgsPluginLayer()
{
  // TODO: shall we move the responsibility of emitting the signal to plugin
  // layer implementations before they start doing their part of cleanup...?
  emit willBeDeleted();
  delete mDataProvider;
}

QString QgsPluginLayer::pluginLayerType()
{
  return mPluginLayerType;
}

void QgsPluginLayer::setExtent( const QgsRectangle &extent )
{
  QgsMapLayer::setExtent( extent );
  static_cast<QgsPluginLayerDataProvider *>( mDataProvider )->setExtent( extent );
}

void QgsPluginLayer::setSource( const QString &source )
{
  mDataSource = source;
}

QgsDataProvider *QgsPluginLayer::dataProvider()
{
  return mDataProvider;
}

const QgsDataProvider *QgsPluginLayer::dataProvider() const
{
  return mDataProvider;
}

//
// QgsPluginLayerDataProvider
//
///@cond PRIVATE
QgsPluginLayerDataProvider::QgsPluginLayerDataProvider( const QString &layerType,
    const ProviderOptions &options,
    QgsDataProvider::ReadFlags flags )
  : QgsDataProvider( QString(), options, flags )
  , mName( layerType )
{}

QgsCoordinateReferenceSystem QgsPluginLayerDataProvider::crs() const
{
  return QgsCoordinateReferenceSystem();
}

QString QgsPluginLayerDataProvider::name() const
{
  return mName;
}

QString QgsPluginLayerDataProvider::description() const
{
  return QString();
}

QgsRectangle QgsPluginLayerDataProvider::extent() const
{
  return mExtent;
}

bool QgsPluginLayerDataProvider::isValid() const
{
  return true;
}
///@endcond
