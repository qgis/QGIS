/***************************************************************************
  qgslayertree.h
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

#ifndef QGSLAYERTREE_H
#define QGSLAYERTREE_H

#include "qgslayertreenode.h"
#include "qgslayertreegroup.h"
#include "qgslayertreelayer.h"

/**
 * \ingroup core
 * \brief Namespace with helper functions for layer tree operations.
 *
 * Only generally useful routines should be here. Miscellaneous utility functions for work
 * with the layer tree are in QgsLayerTreeUtils class.
 *
 * \since QGIS 3.0
 */
class CORE_EXPORT QgsLayerTree : public QgsLayerTreeGroup
{
    Q_OBJECT

  public:

    /**
     * Check whether the node is a valid group node
     *
     * \since QGIS 2.4
     */
    static inline bool isGroup( QgsLayerTreeNode *node )
    {
      return node && node->nodeType() == QgsLayerTreeNode::NodeGroup;
    }

    /**
     * Check whether the node is a valid layer node
     *
     * \since QGIS 2.4
     */
    static inline bool isLayer( const QgsLayerTreeNode *node )
    {
      return node && node->nodeType() == QgsLayerTreeNode::NodeLayer;
    }

    /**
     * Cast node to a group. No type checking is done - use isGroup() to find out whether this operation is legal.
     *
     * \note Not available in Python bindings, because cast is automatic.
     * \since QGIS 2.4
     */
    static inline QgsLayerTreeGroup *toGroup( QgsLayerTreeNode *node ) SIP_SKIP
    {
      return static_cast<QgsLayerTreeGroup *>( node );
    }

    /**
     * Cast node to a layer. No type checking is done - use isLayer() to find out whether this operation is legal.
     *
     * \note Not available in Python bindings, because cast is automatic.
     * \since QGIS 2.4
     */
    static inline QgsLayerTreeLayer *toLayer( QgsLayerTreeNode *node ) SIP_SKIP
    {
      return static_cast<QgsLayerTreeLayer *>( node );
    }

    /**
     * Cast node to a layer. No type checking is done - use isLayer() to find out whether this operation is legal.
     *
     * \note Not available in Python bindings, because cast is automatic.
     * \since QGIS 2.4
     */
    static inline const QgsLayerTreeLayer *toLayer( const QgsLayerTreeNode *node ) SIP_SKIP
    {
      return static_cast< const QgsLayerTreeLayer *>( node );
    }

    /**
     * Create a new empty layer tree
     */
    QgsLayerTree();

#ifdef SIP_RUN
    SIP_PYOBJECT __repr__();
    % MethodCode
    // override parent QgsLayerTreeGroup __repr__ and resort back to default repr for QgsLayerTree -- there's no extra useful context we can show
    QString str = QStringLiteral( "<qgis._core.QgsLayerTree object at 0x%1>" ).arg( reinterpret_cast<quintptr>( sipCpp ), 2 * QT_POINTER_SIZE, 16, QLatin1Char( '0' ) );
    sipRes = PyUnicode_FromString( str.toUtf8().constData() );
    % End
#endif

    /**
     * The order in which layers will be rendered on the canvas.
     * Will only be used if the property hasCustomLayerOrder is TRUE.
     * If you need the current layer order that is active, prefer using layerOrder().
     *
     * \see setCustomLayerOrder
     * \see layerOrder
     * \see hasCustomLayerOrder
     *
     * \since QGIS 3.0
     */
    QList<QgsMapLayer *> customLayerOrder() const;

    /**
     * The order in which layers will be rendered on the canvas.
     * Will only be used if the property hasCustomLayerOrder is TRUE.
     * If you need the current layer order that is active, prefer using layerOrder().
     *
     * \see customLayerOrder
     * \see layerOrder
     * \see hasCustomLayerOrder
     *
     * \since QGIS 3.0
     */
    void setCustomLayerOrder( const QList<QgsMapLayer *> &customLayerOrder );

    /**
     * The order in which layers will be rendered on the canvas.
     * Will only be used if the property hasCustomLayerOrder is TRUE.
     * If you need the current layer order that is active, prefer using layerOrder().
     *
     * \see customLayerOrder
     * \see layerOrder
     * \see hasCustomLayerOrder
     *
     * \since QGIS 3.0
     */
    void setCustomLayerOrder( const QStringList &customLayerOrder ) SIP_PYNAME( setCustomLayerOrderByIds );

    /**
     * The order in which layers will be rendered on the canvas.
     * Depending on hasCustomLayerOrder, this will return either the override
     * customLayerOrder or the layer order derived from the tree.
     * This property is read only.
     *
     * \see customLayerOrder
     *
     * \since QGIS 3.0
     */
    QList<QgsMapLayer *> layerOrder() const;

    /**
     * Determines if the layer order should be derived from the layer tree
     * or if a custom override order shall be used instead.
     *
     * \see customLayerOrder
     *
     * \since QGIS 3.0
     */
    bool hasCustomLayerOrder() const;

    /**
     * Determines if the layer order should be derived from the layer tree
     * or if a custom override order shall be used instead.
     *
     * \see setCustomLayerOrder
     *
     * \since QGIS 3.0
     */
    void setHasCustomLayerOrder( bool hasCustomLayerOrder );

    /**
     * Load the layer tree from an XML element.
     * It is not required that layers are loaded at this point.
     * resolveReferences() needs to be called after loading the layers and
     * before using the tree.
     *
     * \since QGIS 3.0
     */
    static QgsLayerTree *readXml( QDomElement &element, const QgsReadWriteContext &context );

    /**
     * Load the layer order from an XML element.
     * Make sure that this is only called after the layers are loaded.
     *
     * \since QGIS 3.0
     */
    void readLayerOrderFromXml( const QDomElement &doc );

    void writeXml( QDomElement &parentElement, const QgsReadWriteContext &context ) override;

    QgsLayerTree *clone() const override SIP_FACTORY;

    /**
     * Clear any information from this layer tree.
     *
     * \since QGIS 3.0
     */
    void clear();

  signals:

    /**
     * Emitted when the custom layer order has changed.
     *
     * \since QGIS 3.0
     */
    void customLayerOrderChanged();

    /**
     * Emitted when the layer order has changed.
     *
     * \since QGIS 3.0
     */
    void layerOrderChanged();

    /**
     * Emitted when the hasCustomLayerOrder flag changes.
     *
     * \see hasCustomLayerOrder
     *
     * \since QGIS 3.0
     */
    void hasCustomLayerOrderChanged( bool hasCustomLayerOrder );

  private slots:
    void nodeAddedChildren( QgsLayerTreeNode *node, int indexFrom, int indexTo );
    void nodeRemovedChildren();

  private:
    //! Copy constructor \see clone()
    QgsLayerTree( const QgsLayerTree &other );

    void init();

    void addMissingLayers();
    QgsWeakMapLayerPointerList mCustomLayerOrder;
    bool mHasCustomLayerOrder = false;

    QgsLayerTree &operator= ( const QgsLayerTree & ) = delete;
};

#endif // QGSLAYERTREE_H
