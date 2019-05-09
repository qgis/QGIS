/***************************************************************************
    qgspluginlayer.h
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
#ifndef QGSPLUGINLAYER_H
#define QGSPLUGINLAYER_H

#include "qgis_core.h"
#include "qgsmaplayer.h"
#include "qgsdataprovider.h"

/**
 * \ingroup core
  Base class for plugin layers. These can be implemented by plugins
  and registered in QgsPluginLayerRegistry.

  In order to be readable from project files, they should set these attributes in layer DOM node:
   "type" = "plugin"
   "name" = "your_layer_type"
 */
class CORE_EXPORT QgsPluginLayer : public QgsMapLayer
{
    Q_OBJECT

  public:
    QgsPluginLayer( const QString &layerType, const QString &layerName = QString() );
    ~QgsPluginLayer() override;

    /**
     * Returns a new instance equivalent to this one.
     * \returns a new layer instance
     * \since QGIS 3.0
     */
    QgsPluginLayer *clone() const override = 0;

    //! Returns plugin layer type (the same as used in QgsPluginLayerRegistry)
    QString pluginLayerType();

    //! Sets extent of the layer
    void setExtent( const QgsRectangle &extent ) override;

    /**
     * Set source string. This is used for example in layer tree to show tooltip.
     * \since QGIS 2.16
     */
    void setSource( const QString &source );

    QgsDataProvider *dataProvider() override;
    const QgsDataProvider *dataProvider() const override SIP_SKIP;

  protected:
    QString mPluginLayerType;
    QgsDataProvider *mDataProvider;
};

#ifndef SIP_RUN
///@cond PRIVATE

/**
 * A minimal data provider for plugin layers
 */
class QgsPluginLayerDataProvider : public QgsDataProvider
{
    Q_OBJECT

  public:
    QgsPluginLayerDataProvider( const QString &layerType, const QgsDataProvider::ProviderOptions &providerOptions );
    void setExtent( const QgsRectangle &extent ) { mExtent = extent; }
    QgsCoordinateReferenceSystem crs() const override;
    QString name() const override;
    QString description() const override;
    QgsRectangle extent() const override;
    bool isValid() const override;

  private:
    QString mName;
    QgsRectangle mExtent;
};
///@endcond
#endif

#endif // QGSPLUGINLAYER_H
