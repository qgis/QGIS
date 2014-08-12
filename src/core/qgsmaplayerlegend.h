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

#include <QHash>

/** Default legend implementation for vector layers
 * @note added in 2.6
 */
class CORE_EXPORT QgsDefaultVectorLayerLegend : public QgsMapLayerLegend
{
  public:
    explicit QgsDefaultVectorLayerLegend( QgsVectorLayer* vl );

    virtual QList<QgsLayerTreeModelLegendNode*> createLayerTreeModelLegendNodes( QgsLayerTreeLayer* nodeLayer );

    void setRuleUserLabel( const QString& ruleKey, const QString& label );

    QString ruleUserLabel( const QString& ruleKey ) const;

    QStringList rulesWithUserLabel() const;

  private:
    QgsVectorLayer* mLayer;
    QHash<QString, QString> mUserLabels;
};


/** Default legend implementation for raster layers
 * @note added in 2.6
 */
class CORE_EXPORT QgsDefaultRasterLayerLegend : public QgsMapLayerLegend
{
  public:
    explicit QgsDefaultRasterLayerLegend( QgsRasterLayer* rl );

    virtual QList<QgsLayerTreeModelLegendNode*> createLayerTreeModelLegendNodes( QgsLayerTreeLayer* nodeLayer );

  private:
    QgsRasterLayer* mLayer;
};


/** Default legend implementation for plugin layers
 * @note added in 2.6
 */
class CORE_EXPORT QgsDefaultPluginLayerLegend : public QgsMapLayerLegend
{
  public:
    explicit QgsDefaultPluginLayerLegend( QgsPluginLayer* pl );

    virtual QList<QgsLayerTreeModelLegendNode*> createLayerTreeModelLegendNodes( QgsLayerTreeLayer* nodeLayer );

  private:
    QgsPluginLayer* mLayer;
};

#endif // QGSMAPLAYERLEGEND_H
