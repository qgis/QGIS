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

#include "qgsmaplayerlegend.h"
#include "qgsmaplayerrenderer.h"


/**
  A minimal data provider for plugin layers
 */
///@cond PRIVATE
class QgsPluginLayerDataProvider : public QgsDataProvider
{
  public:
    QgsPluginLayerDataProvider( const QString &layerType ) : mName( layerType ) {}
    void setExtent( const QgsRectangle &extent ) { mExtent = extent; }
    virtual QgsCoordinateReferenceSystem crs() const { return QgsCoordinateReferenceSystem(); }
    virtual QString name() const override { return mName; }
    QString description() const override { return ""; }
    virtual QgsRectangle extent() const { return mExtent; }
    virtual bool isValid() const { return true; }

  private:
    QString mName;
    QgsRectangle mExtent;
};
///@endcond

QgsPluginLayer::QgsPluginLayer( const QString &layerType, const QString &layerName )
  : QgsMapLayer( PluginLayer, layerName )
  , mPluginLayerType( layerType )
{
  mDataProvider = new QgsPluginLayerDataProvider( layerType );
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
  mExtent = extent;
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
