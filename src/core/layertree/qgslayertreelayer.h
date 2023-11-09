/***************************************************************************
  qgslayertreelayer.h
  --------------------------------------
  Date                 : May 2014
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

#ifndef QGSLAYERTREELAYER_H
#define QGSLAYERTREELAYER_H

#include "qgis_core.h"
#include "qgis_sip.h"
#include "qgslayertreenode.h"
#include "qgsmaplayerref.h"
#include "qgsreadwritecontext.h"
#include "qgslegendpatchshape.h"

class QgsMapLayer;

/**
 * \ingroup core
 * \brief Layer tree node points to a map layer.
 *
 * The node can exist also without a valid instance of a layer (just ID). That
 * means the referenced layer does not need to be loaded in order to use it
 * in layer tree. In such case, resolveReferences() method can be called
 * once the layer is loaded.
 *
 * A map layer is supposed to be present in one layer tree just once. It is
 * however possible that temporarily a layer exists in one tree more than just
 * once, e.g. while reordering items with drag and drop.
 *
 * \since QGIS 2.4
 */
class CORE_EXPORT QgsLayerTreeLayer : public QgsLayerTreeNode
{
    Q_OBJECT
  public:
    explicit QgsLayerTreeLayer( QgsMapLayer *layer );

#ifndef SIP_RUN
    QgsLayerTreeLayer( const QgsLayerTreeLayer &other );
#endif

    /**
     * Constructor for QgsLayerTreeLayer using weak references to layer ID, \a name, public \a source, and \a provider key.
     */
    explicit QgsLayerTreeLayer( const QString &layerId, const QString &name = QString(), const QString &source = QString(), const QString &provider = QString() );

#ifdef SIP_RUN
    SIP_PYOBJECT __repr__();
    % MethodCode
    QString str = QStringLiteral( "<QgsLayerTreeLayer: %1>" ).arg( sipCpp->name() );
    sipRes = PyUnicode_FromString( str.toUtf8().constData() );
    % End
#endif

    /**
     * Returns the ID for the map layer associated with this node.
     *
     * \see layer()
     */
    QString layerId() const { return mRef.layerId; }

    /**
     * Returns the map layer associated with this node.
     *
     * \warning This can be (and often is!) NULLPTR, e.g. in the case of a layer node representing a layer
     * which has not yet been fully loaded into a project, or a layer node representing a layer
     * with an invalid data source. The returned pointer must ALWAYS be checked to avoid dereferencing NULLPTR.
     *
     * \see layerId()
     */
    QgsMapLayer *layer() const { return mRef.get(); }

    /**
     * Returns the layer's name.
     *
     * \see setName()
     *
     * \since QGIS 3.0
     */
    QString name() const override;

    /**
     * Sets the layer's name.
     *
     * \see name()
     *
     * \since QGIS 3.0
     */
    void setName( const QString &n ) override;

    /**
     * Uses the layer's name if \a use is TRUE, or the name manually set if
     * FALSE.
     * \since QGIS 3.8
     */
    void setUseLayerName( bool use = true );

    /**
     * Returns whether the layer's name is used, or the name manually set.
     * \since QGIS 3.8
     */
    bool useLayerName() const;

    /**
     * Read layer node from XML. Returns new instance.
     * Does not resolve textual references to layers. Call resolveReferences() afterwards to do it.
     */
    static QgsLayerTreeLayer *readXml( QDomElement &element, const QgsReadWriteContext &context ) SIP_FACTORY;

    /**
     * Read layer node from XML. Returns new instance.
     * Also resolves textual references to layers from the project (calls resolveReferences() internally).
     * \since QGIS 3.0
     */
    static QgsLayerTreeLayer *readXml( QDomElement &element, const QgsProject *project, const QgsReadWriteContext &context ) SIP_FACTORY;

    void writeXml( QDomElement &parentElement, const QgsReadWriteContext &context ) override;

    QString dump() const override;

    QgsLayerTreeLayer *clone() const override SIP_FACTORY;

    /**
     * Resolves reference to layer from stored layer ID (if it has not been resolved already)
     * \since QGIS 3.0
     */
    void resolveReferences( const QgsProject *project, bool looseMatching = false ) override;

    /**
     * set the expression to evaluate
     *
     * \since QGIS 3.10
     */
    void setLabelExpression( const QString &expression );

    /**
     * Returns the expression member of the LayerTreeNode
     *
     * \since QGIS 3.10
     */
    QString labelExpression() const { return mLabelExpression; }

    /**
     * Returns the symbol patch shape to use when rendering the legend node symbol.
     *
     * \see setPatchShape()
     * \since QGIS 3.14
     */
    QgsLegendPatchShape patchShape() const;

    /**
     * Sets the symbol patch \a shape to use when rendering the legend node symbol.
     *
     * \see patchShape()
     * \since QGIS 3.14
     */
    void setPatchShape( const QgsLegendPatchShape &shape );

    /**
     * Returns the user (overridden) size for the legend node.
     *
     * If either the width or height are non-zero, they will be used when rendering the legend node instead of the default
     * symbol width or height from QgsLegendSettings.
     *
     * \see setPatchSize()
     * \since QGIS 3.14
     */
    QSizeF patchSize() const { return mPatchSize; }

    /**
     * Sets the user (overridden) \a size for the legend node.
     *
     * If either the width or height are non-zero, they will be used when rendering the legend node instead of the default
     * symbol width or height from QgsLegendSettings.
     *
     * \see patchSize()
     * \since QGIS 3.14
     */
    void setPatchSize( QSizeF size ) { mPatchSize = size; }

    /**
     * Legend node column split behavior.
     *
     * \since QGIS 3.14
     */
    enum LegendNodesSplitBehavior
    {
      UseDefaultLegendSetting, //!< Inherit default legend column splitting setting
      AllowSplittingLegendNodesOverMultipleColumns, //!< Allow splitting node's legend nodes across multiple columns
      PreventSplittingLegendNodesOverMultipleColumns, //!< Prevent splitting node's legend nodes across multiple columns
    };

    /**
     * Returns the column split behavior for the node.
     *
     * This value controls how legend nodes belonging the to layer may be split over multiple columns in legends.
     *
     * \see setLegendSplitBehavior()
     * \since QGIS 3.14
     */
    LegendNodesSplitBehavior legendSplitBehavior() const { return mSplitBehavior; }

    /**
     * Sets the column split \a behavior for the node.
     *
     * This value controls how legend nodes belonging the to layer may be split over multiple columns in legends.
     *
     * \see legendSplitBehavior()
     * \since QGIS 3.14
     */
    void setLegendSplitBehavior( LegendNodesSplitBehavior behavior ) { mSplitBehavior = behavior; }

  signals:

    /**
     * Emitted when a previously unavailable layer got loaded.
     */
    void layerLoaded();

    /**
     * Emitted when a previously available layer got unloaded (from layer registry).
     * \since QGIS 2.6
     */
    void layerWillBeUnloaded();

  protected:
    void attachToLayer();

    //! Weak reference to the layer (or just it's ID if the reference is not resolved yet)
    QgsMapLayerRef mRef;
    //! Layer name - only used if layer does not exist or if mUseLayerName is false
    QString mLayerName;
    //! Expression to evaluate in the legend
    QString mLabelExpression;

    //!
    bool mUseLayerName = true;

  private slots:

    /**
     * Emits a nameChanged() signal if layer's name has changed
     * \since QGIS 3.0
     */
    void layerNameChanged();

    /**
     * Handles the event of deletion of the referenced layer
     * \since QGIS 3.0
     */
    void layerWillBeDeleted();

  private:

#ifdef SIP_RUN

    /**
     * Copies are not allowed
     */
    QgsLayerTreeLayer( const QgsLayerTreeLayer &other );
#endif

    QgsLegendPatchShape mPatchShape;
    QSizeF mPatchSize;
    LegendNodesSplitBehavior mSplitBehavior = UseDefaultLegendSetting;

    QgsLayerTreeLayer &operator=( const QgsLayerTreeLayer & ) = delete;
};



#endif // QGSLAYERTREELAYER_H
