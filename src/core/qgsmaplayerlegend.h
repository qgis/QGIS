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
class QgsPointCloudLayer;
class QgsReadWriteContext;
class QgsVectorLayer;
class QgsLegendPatchShape;
class QgsColorRampLegendNodeSettings;
class QgsSymbol;

#include "qgis_core.h"


/**
 * \ingroup core
 * \brief The QgsMapLayerLegend class is abstract interface for implementations
 * of legends for one map layer.
 *
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

    /**
     * Create new legend implementation for a point cloud \a layer.
     * \since QGIS 3.18
     */
    static QgsMapLayerLegend *defaultPointCloudLegend( QgsPointCloudLayer *layer ) SIP_FACTORY;

  signals:
    //! Emitted when existing items/nodes got invalid and should be replaced by new ones
    void itemsChanged();
};


/**
 * \ingroup core
 * \brief Miscellaneous utility functions for handling of map layer legend
 *
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

    /**
     * Sets the legend patch \a shape for the legend node belonging to \a nodeLayer at the specified \a originalIndex.
     *
     * \see legendNodePatchShape()
     * \since QGIS 3.14
     */
    static void setLegendNodePatchShape( QgsLayerTreeLayer *nodeLayer, int originalIndex, const QgsLegendPatchShape &shape );

    /**
     * Returns the legend patch shape for the legend node belonging to \a nodeLayer at the specified \a originalIndex.
     *
     * \see setLegendNodePatchShape()
     * \since QGIS 3.14
     */
    static QgsLegendPatchShape legendNodePatchShape( QgsLayerTreeLayer *nodeLayer, int originalIndex );

    /**
     * Sets the legend symbol \a size for the legend node belonging to \a nodeLayer at the specified \a originalIndex.
     *
     * If either the width or height are non-zero, they will be used when rendering the legend node instead of the default
     * symbol width or height from QgsLegendSettings.
     *
     * \see legendNodeSymbolSize()
     * \since QGIS 3.14
     */
    static void setLegendNodeSymbolSize( QgsLayerTreeLayer *nodeLayer, int originalIndex, QSizeF size );

    /**
     * Returns the legend node symbol size for the legend node belonging to \a nodeLayer at the specified \a originalIndex.
     *
     * If either the width or height are non-zero, they will be used when rendering the legend node instead of the default
     * symbol width or height from QgsLegendSettings.
     *
     * \see setLegendNodeSymbolSize()
     * \since QGIS 3.14
     */
    static QSizeF legendNodeSymbolSize( QgsLayerTreeLayer *nodeLayer, int originalIndex );

    /**
     * Sets a custom legend \a symbol for the legend node belonging to \a nodeLayer at the specified \a originalIndex.
     *
     * If \a symbol is non-NULLPTR, it will be used in place of the default symbol when rendering
     * the legend node.
     *
     * \see legendNodeCustomSymbol()
     * \since QGIS 3.14
     */
    static void setLegendNodeCustomSymbol( QgsLayerTreeLayer *nodeLayer, int originalIndex, const QgsSymbol *symbol );

    /**
     * Returns the custom legend symbol for the legend node belonging to \a nodeLayer at the specified \a originalIndex.
     *
     * If the symbol is non-NULLPTR, it will be used in place of the default symbol when rendering
     * the legend node.
     *
     * Caller takes ownership of the returned symbol.
     *
     * \see setLegendNodeCustomSymbol()
     * \since QGIS 3.14
     */
    static QgsSymbol *legendNodeCustomSymbol( QgsLayerTreeLayer *nodeLayer, int originalIndex ) SIP_FACTORY;

    /**
     * Sets a custom legend color ramp \a settings for the legend node belonging to \a nodeLayer at the specified \a originalIndex.
     *
     * If the corresponding legend node is not a QgsColorRampLegendNode then calling this method will have no effect.
     *
     * If \a settings is non-NULLPTR, they will be used in place of the default settigns when rendering
     * the legend node.
     *
     * \see legendNodeColorRampSettings()
     * \since QGIS 3.18
     */
    static void setLegendNodeColorRampSettings( QgsLayerTreeLayer *nodeLayer, int originalIndex, const QgsColorRampLegendNodeSettings *settings );

    /**
     * Returns the custom legend color ramp settings for the legend node belonging to \a nodeLayer at the specified \a originalIndex.
     *
     * If the corresponding legend node is not a QgsColorRampLegendNode then calling this method will return NULLPTR.
     *
     * If the returned value is non-NULLPTR, they will be used in place of the default settings when rendering
     * the legend node.
     *
     * Caller takes ownership of the returned settings.
     *
     * \see setLegendNodeColorRampSettings()
     * \since QGIS 3.18
     */
    static QgsColorRampLegendNodeSettings *legendNodeColorRampSettings( QgsLayerTreeLayer *nodeLayer, int originalIndex ) SIP_FACTORY;


    /**
     * Sets whether a forced column break should occur before the node.
     *
     * \see legendNodeColumnBreak()
     * \since QGIS 3.14
     */
    static void setLegendNodeColumnBreak( QgsLayerTreeLayer *nodeLayer, int originalIndex, bool columnBreakBeforeNode );

    /**
     * Returns whether a forced column break should occur before the node.
     *
     * \see setLegendNodeColumnBreak()
     * \since QGIS 3.14
     */
    static bool legendNodeColumnBreak( QgsLayerTreeLayer *nodeLayer, int originalIndex );

    //! update according to layer node's custom properties (order of items, user labels for items)
    static void applyLayerNodeProperties( QgsLayerTreeLayer *nodeLayer, QList<QgsLayerTreeModelLegendNode *> &nodes );
};


#include <QHash>

#include "qgstextformat.h"

/**
 * \ingroup core
 * \brief Default legend implementation for vector layers
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
     * Returns whether the legend for the labeling is shown
     * \since QGIS 3.20
    */
    bool showLabelLegend() const { return mShowLabelLegend; }

    /**
     * Sets if a legend for the labeling should be shown
     * \param enabled true to show label legend entries
     * \since QGIS 3.20
     */
    void setShowLabelLegend( bool enabled ) { mShowLabelLegend = enabled; }

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
    bool mShowLabelLegend = false;
    QgsTextFormat mTextOnSymbolTextFormat;
    QHash<QString, QString> mTextOnSymbolContent;
};


/**
 * \ingroup core
 * \brief Default legend implementation for raster layers
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
 * \brief Default legend implementation for mesh layers
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

/**
 * \ingroup core
 * \brief Default legend implementation for point cloud layers
 * \since QGIS 3.18
 */
class CORE_EXPORT QgsDefaultPointCloudLayerLegend : public QgsMapLayerLegend
{
    Q_OBJECT

  public:
    //! Creates an instance for the given point cloud layer
    explicit QgsDefaultPointCloudLayerLegend( QgsPointCloudLayer *layer );

    QList<QgsLayerTreeModelLegendNode *> createLayerTreeModelLegendNodes( QgsLayerTreeLayer *nodeLayer ) SIP_FACTORY override;

  private:
    QgsPointCloudLayer *mLayer = nullptr;
};


#endif // QGSMAPLAYERLEGEND_H
