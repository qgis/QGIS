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

#include "qgsmaplayer.h"

typedef QList< QPair<QString, QPixmap> > QgsLegendSymbologyList;

/** \ingroup core
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
    QgsPluginLayer( QString layerType, QString layerName = QString() );

    /** Return plugin layer type (the same as used in QgsPluginLayerRegistry) */
    QString pluginLayerType();

    void setExtent( const QgsRectangle &extent ) override;

    //! return a list of symbology items for the legend
    //! (defult implementation returns nothing)
    //! @note Added in v2.1
    virtual QgsLegendSymbologyList legendSymbologyItems( const QSize& iconSize );

    /** Return new instance of QgsMapLayerRenderer that will be used for rendering of given context
     *
     * The default implementation returns map layer renderer which just calls draw().
     * This may work, but it is unsafe for multi-threaded rendering because of the run
     * conditions that may happen (e.g. something is changed in the layer while it is
     * being rendered).
     *
     * @note added in 2.4
     */
    virtual QgsMapLayerRenderer* createMapRenderer( QgsRenderContext& rendererContext ) override;

  protected:
    QString mPluginLayerType;
};

#endif // QGSPLUGINLAYER_H
