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
#include "qgis_sip.h"

class QDomDocument;
class QDomElement;

class QgsLayerTreeLayer;
class QgsLayerTreeModelLegendNode;
class QgsMeshLayer;
class QgsPluginLayer;
class QgsRasterLayer;
class QgsReadWriteContext;
class QgsVectorLayer;

#include "qgis_core.h"


/**
 * \ingroup core
 * The QgsMapLayerLegend class is abstract interface for implementations
 * of legends for one map layer.
 *
 * \since QGIS 2.6
 */
class CORE_EXPORT QgsMapLayerLegend : public QObject
{
    Q_OBJECT
  public:

    //! Constructor for QgsMapLayerLegend
    explicit QgsMapLayerLegend( QObject *parent SIP_TRANSFERTHIS = nullptr );

    // TODO: type

    /**
     * Reads configuration from a DOM element previously written by writeXml()
     * \since QGIS 3.2
     */
    virtual void readXml( const QDomElement &elem, const QgsReadWriteContext &context );

    /**
     * Writes configuration to a DOM element, to be used later with readXml()
     * \since QGIS 3.2
     */
    virtual QDomElement writeXml( QDomDocument &doc, const QgsReadWriteContext &context ) const;

    /**
     * Returns list of legend nodes to be used for a particular layer tree layer node.
     * Ownership is transferred to the caller.
     */
    virtual QList<QgsLayerTreeModelLegendNode *> createLayerTreeModelLegendNodes( QgsLayerTreeLayer *nodeLayer ) = 0 SIP_FACTORY;

    // TODO: support for layer tree view delegates

    //! Create new legend implementation for vector layer
    static QgsMapLayerLegend *defaultVectorLegend( QgsVectorLayer *vl ) SIP_FACTORY;

    //! Create new legend implementation for raster layer
    static QgsMapLayerLegend *defaultRasterLegend( QgsRasterLayer *rl ) SIP_FACTORY;

    //! Create new legend implementation for mesh layer
    static QgsMapLayerLegend *defaultMeshLegend( QgsMeshLayer *ml ) SIP_FACTORY;

  signals:
    //! Emitted when existing items/nodes got invalid and should be replaced by new ones
    void itemsChanged();
};


/**
 * \ingroup core
 * Miscellaneous utility functions for handling of map layer legend
 *
 * \since QGIS 2.6
 */
class CORE_EXPORT QgsMapLayerLegendUtils
{
  public:
    static void setLegendNodeOrder( QgsLayerTreeLayer *nodeLayer, const QList<int> &order );
    static QList<int> legendNodeOrder( QgsLayerTreeLayer *nodeLayer );
    static bool hasLegendNodeOrder( QgsLayerTreeLayer *nodeLayer );

    static void setLegendNodeUserLabel( QgsLayerTreeLayer *nodeLayer, int originalIndex, const QString &newLabel );
    static QString legendNodeUserLabel( QgsLayerTreeLayer *nodeLayer, int originalIndex );
    static bool hasLegendNodeUserLabel( QgsLayerTreeLayer *nodeLayer, int originalIndex );

    //! update according to layer node's custom properties (order of items, user labels for items)
    static void applyLayerNodeProperties( QgsLayerTreeLayer *nodeLayer, QList<QgsLayerTreeModelLegendNode *> &nodes );
};


#include <QHash>

#include "qgstextrenderer.h"

/**
 * \ingroup core
 * Default legend implementation for vector layers
 * \since QGIS 2.6
 */
class CORE_EXPORT QgsDefaultVectorLayerLegend : public QgsMapLayerLegend
{
    Q_OBJECT

  public:
    explicit QgsDefaultVectorLayerLegend( QgsVectorLayer *vl );

    /**
     * Returns whether the "text on symbol" functionality is enabled. When enabled, legend symbols
     * may have extra text rendered on top. The content of labels and their style is controlled
     * by textOnSymbolContent() and textOnSymbolTextFormat().
     * \since QGIS 3.2
     */
    bool textOnSymbolEnabled() const { return mTextOnSymbolEnabled; }

    /**
     * Sets whether the "text on symbol" functionality is enabled. When enabled, legend symbols
     * may have extra text rendered on top. The content of labels and their style is controlled
     * by textOnSymbolContent() and textOnSymbolTextFormat().
     * \since QGIS 3.2
     */
    void setTextOnSymbolEnabled( bool enabled ) { mTextOnSymbolEnabled = enabled; }

    /**
     * Returns text format of symbol labels for "text on symbol" functionality.
     * \since QGIS 3.2
     */
    QgsTextFormat textOnSymbolTextFormat() const { return mTextOnSymbolTextFormat; }

    /**
     * Sets text format of symbol labels for "text on symbol" functionality.
     * \since QGIS 3.2
     */
    void setTextOnSymbolTextFormat( const QgsTextFormat &format ) { mTextOnSymbolTextFormat = format; }

    /**
     * Returns per-symbol content of labels for "text on symbol" functionality. In the passed dictionary
     * the keys are rule keys of legend items, the values are labels to be shown.
     * \since QGIS 3.2
     */
    QHash<QString, QString> textOnSymbolContent() const { return mTextOnSymbolContent; }

    /**
     * Sets per-symbol content of labels for "text on symbol" functionality. In the passed dictionary
     * the keys are rule keys of legend items, the values are labels to be shown.
     * \since QGIS 3.2
     */
    void setTextOnSymbolContent( const QHash<QString, QString> &content ) { mTextOnSymbolContent = content; }

    QList<QgsLayerTreeModelLegendNode *> createLayerTreeModelLegendNodes( QgsLayerTreeLayer *nodeLayer ) SIP_FACTORY override;
    void readXml( const QDomElement &elem, const QgsReadWriteContext &context ) override;
    QDomElement writeXml( QDomDocument &doc, const QgsReadWriteContext &context ) const override;

  private:
    QgsVectorLayer *mLayer = nullptr;

    // text on symbol
    bool mTextOnSymbolEnabled = false;
    QgsTextFormat mTextOnSymbolTextFormat;
    QHash<QString, QString> mTextOnSymbolContent;
};


/**
 * \ingroup core
 * Default legend implementation for raster layers
 * \since QGIS 2.6
 */
class CORE_EXPORT QgsDefaultRasterLayerLegend : public QgsMapLayerLegend
{
    Q_OBJECT

  public:
    explicit QgsDefaultRasterLayerLegend( QgsRasterLayer *rl );

    QList<QgsLayerTreeModelLegendNode *> createLayerTreeModelLegendNodes( QgsLayerTreeLayer *nodeLayer ) SIP_FACTORY override;

  private:
    QgsRasterLayer *mLayer = nullptr;
};


/**
 * \ingroup core
 * Default legend implementation for mesh layers
 * \since QGIS 3.4
 */
class CORE_EXPORT QgsDefaultMeshLayerLegend : public QgsMapLayerLegend
{
    Q_OBJECT

  public:
    //! Creates an instance for the given mesh layer
    explicit QgsDefaultMeshLayerLegend( QgsMeshLayer *ml );

    QList<QgsLayerTreeModelLegendNode *> createLayerTreeModelLegendNodes( QgsLayerTreeLayer *nodeLayer ) SIP_FACTORY override;

  private:
    QgsMeshLayer *mLayer = nullptr;
};


#endif // QGSMAPLAYERLEGEND_H
