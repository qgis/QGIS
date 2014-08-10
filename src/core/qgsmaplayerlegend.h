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

#include <QIcon>
#include <QObject>

class QgsLayerTreeLayer;
class QgsLegendSettings;
class QgsPluginLayer;
class QgsRasterLayer;
class QgsSymbolV2;
class QgsVectorLayer;


/**
 * The QgsLegendRendererItem class is abstract interface for legend items
 * returned from QgsMapLayerLegend implementation.
 *
 * The objects are used in QgsLayerTreeModel. Custom implementations may offer additional interactivity
 * and customized look.
 *
 * @note added in 2.6
 */
class CORE_EXPORT QgsLayerTreeModelLegendNode : public QObject
{
    Q_OBJECT
  public:

    /** Return pointer to the parent layer node */
    QgsLayerTreeLayer* parent() const { return mParent; }

    /** Return item flags associated with the item. Default implementation returns Qt::ItemIsEnabled. */
    virtual Qt::ItemFlags flags() const;

    /** Return data associated with the item. Must be implemented in derived class. */
    virtual QVariant data( int role ) const = 0;

    /** Set some data associated with the item. Default implementation does nothing and returns false. */
    virtual bool setData( const QVariant& value, int role );

  protected:
    /** Construct the node with pointer to its parent layer node */
    explicit QgsLayerTreeModelLegendNode( QgsLayerTreeLayer* nodeL );

  protected:
    QgsLayerTreeLayer* mParent;
};


/**
 * Implementation of legend node interface for displaying preview of vector symbols and their labels
 * and allowing interaction with the symbol / renderer.
 *
 * @note added in 2.6
 */
class CORE_EXPORT QgsSymbolV2LegendNode : public QgsLayerTreeModelLegendNode
{
  public:
    QgsSymbolV2LegendNode( QgsLayerTreeLayer* nodeLayer, QgsSymbolV2* symbol, const QString& label, int rendererRef = -1 );

    virtual Qt::ItemFlags flags() const;
    virtual QVariant data( int role ) const;
    virtual bool setData( const QVariant& value, int role );

  private:
    QgsSymbolV2* mSymbol;
    mutable QIcon mIcon; // cached symbol preview
    QString mLabel;
    int mRendererRef;
};


/**
 * Implementation of legend node interface for displaying arbitrary label with icon.
 *
 * @note added in 2.6
 */
class CORE_EXPORT QgsSimpleLegendNode : public QgsLayerTreeModelLegendNode
{
  public:
    QgsSimpleLegendNode( QgsLayerTreeLayer* nodeLayer, const QString& label, const QIcon& icon = QIcon() );

    virtual QVariant data( int role ) const;

  private:
    QString mLabel;
    QIcon mIcon;
};


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

    // TODO: support for legend renderer


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


/** Default legend implementation for vector layers
 * @note added in 2.6
 */
class CORE_EXPORT QgsDefaultVectorLayerLegend : public QgsMapLayerLegend
{
  public:
    explicit QgsDefaultVectorLayerLegend( QgsVectorLayer* vl );

    virtual QList<QgsLayerTreeModelLegendNode*> createLayerTreeModelLegendNodes( QgsLayerTreeLayer* nodeLayer );

  private:
    QgsVectorLayer* mLayer;
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
