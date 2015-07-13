/***************************************************************************
  qgsmaplayerlegend.h
  --------------------------------------
  Date                 : July 2014
  Copyright            : (C) 2014 by Martin Dobias
  Email                : wonder dot sk at gmail dot com
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef QGSMAPLAYERLEGEND_H
#define QGSMAPLAYERLEGEND_H

#include <QObject>

class QgsLayerTreeLayer;
class QgsLayerTreeModelLegendNode;
class QgsPluginLayer;
class QgsRasterLayer;
class QgsVectorLayer;


/**
 * The QgsMapLayerLegend class is abstract interface for implementations
 * of legends for one map layer.
 *
 * @note added in 2.6
 */
class CORE_EXPORT QgsMapLayerLegend : public QObject
{
    Q_OBJECT
  public:
    explicit QgsMapLayerLegend( QObject *parent = 0 );

    // TODO: type, load/save settings

    /**
     * Return list of legend nodes to be used for a particular layer tree layer node.
     * Ownership is transferred to the caller.
     */
    virtual QList<QgsLayerTreeModelLegendNode*> createLayerTreeModelLegendNodes( QgsLayerTreeLayer* nodeLayer ) = 0;

    // TODO: support for layer tree view delegates

    //! Create new legend implementation for vector layer
    static QgsMapLayerLegend* defaultVectorLegend( QgsVectorLayer* vl );

    //! Create new legend implementation for raster layer
    static QgsMapLayerLegend* defaultRasterLegend( QgsRasterLayer* rl );

    //! Create new legend implementation for raster layer
    static QgsMapLayerLegend* defaultPluginLegend( QgsPluginLayer* pl );

  signals:
    //! Emitted when existing items/nodes got invalid and should be replaced by new ones
    void itemsChanged();
};


/**
 * Miscellaneous utility functions for handling of map layer legend
 *
 * @note added in 2.6
 */
class CORE_EXPORT QgsMapLayerLegendUtils
{
  public:
    static void setLegendNodeOrder( QgsLayerTreeLayer* nodeLayer, const QList<int>& order );
    static QList<int> legendNodeOrder( QgsLayerTreeLayer* nodeLayer );
    static bool hasLegendNodeOrder( QgsLayerTreeLayer* nodeLayer );

    static void setLegendNodeUserLabel( QgsLayerTreeLayer* nodeLayer, int originalIndex, const QString& newLabel );
    static QString legendNodeUserLabel( QgsLayerTreeLayer* nodeLayer, int originalIndex );
    static bool hasLegendNodeUserLabel( QgsLayerTreeLayer* nodeLayer, int originalIndex );

    //! update according to layer node's custom properties (order of items, user labels for items)
    static void applyLayerNodeProperties( QgsLayerTreeLayer* nodeLayer, QList<QgsLayerTreeModelLegendNode*>& nodes );
};


#include <QHash>

/** Default legend implementation for vector layers
 * @note added in 2.6
 */
class CORE_EXPORT QgsDefaultVectorLayerLegend : public QgsMapLayerLegend
{
    Q_OBJECT

  public:
    explicit QgsDefaultVectorLayerLegend( QgsVectorLayer* vl );

    virtual QList<QgsLayerTreeModelLegendNode*> createLayerTreeModelLegendNodes( QgsLayerTreeLayer* nodeLayer ) override;

  private:
    QgsVectorLayer* mLayer;
};


/** Default legend implementation for raster layers
 * @note added in 2.6
 */
class CORE_EXPORT QgsDefaultRasterLayerLegend : public QgsMapLayerLegend
{
    Q_OBJECT

  public:
    explicit QgsDefaultRasterLayerLegend( QgsRasterLayer* rl );

    virtual QList<QgsLayerTreeModelLegendNode*> createLayerTreeModelLegendNodes( QgsLayerTreeLayer* nodeLayer ) override;

  private:
    QgsRasterLayer* mLayer;
};


/** Default legend implementation for plugin layers
 * @note added in 2.6
 */
class CORE_EXPORT QgsDefaultPluginLayerLegend : public QgsMapLayerLegend
{
    Q_OBJECT

  public:
    explicit QgsDefaultPluginLayerLegend( QgsPluginLayer* pl );

    virtual QList<QgsLayerTreeModelLegendNode*> createLayerTreeModelLegendNodes( QgsLayerTreeLayer* nodeLayer ) override;

  private:
    QgsPluginLayer* mLayer;
};

#endif // QGSMAPLAYERLEGEND_H
