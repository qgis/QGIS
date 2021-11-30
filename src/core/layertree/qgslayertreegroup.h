/***************************************************************************
  qgslayertreegroup.h
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

#ifndef QGSLAYERTREEGROUP_H
#define QGSLAYERTREEGROUP_H

#include "qgis_core.h"
#include "qgis_sip.h"
#include "qgslayertreenode.h"
#include "qgsmaplayerref.h"
#include "qgsgrouplayer.h"

class QgsMapLayer;
class QgsLayerTreeLayer;
class QgsGroupLayer;

/**
 * \ingroup core
 * \brief Layer tree group node serves as a container for layers and further groups.
 *
 * Group names do not need to be unique within one tree nor within one parent.
 *
 * While a layer tree group is typically used for hierarchical organisation of a QgsProject,
 * they can optionally be associated with a QgsGroupLayer for map rendering purposes.
 *
 * \since QGIS 2.4
 */
class CORE_EXPORT QgsLayerTreeGroup : public QgsLayerTreeNode
{
    Q_OBJECT
  public:

    /**
     * Constructor
     */
    QgsLayerTreeGroup( const QString &name = QString(), bool checked = true );

#ifndef SIP_RUN
    QgsLayerTreeGroup( const QgsLayerTreeGroup &other );
#endif

#ifdef SIP_RUN
    SIP_PYOBJECT __repr__();
    % MethodCode
    QString str = QStringLiteral( "<QgsLayerTreeGroup: %1>" ).arg( sipCpp->name() );
    sipRes = PyUnicode_FromString( str.toUtf8().constData() );
    % End
#endif

    /**
     * Returns the group's name.
     */
    QString name() const override;

    /**
     * Sets the group's name.
     */
    void setName( const QString &n ) override;

    /**
     * Insert a new group node with given name at specified position. The newly created node is owned by this group.
     */
    QgsLayerTreeGroup *insertGroup( int index, const QString &name );

    /**
     * Append a new group node with given name. Newly created node is owned by this group.
     */
    QgsLayerTreeGroup *addGroup( const QString &name );

    /**
     * Insert a new layer node for given map layer at specified position. The newly created node is owned by this group.
     */
    QgsLayerTreeLayer *insertLayer( int index, QgsMapLayer *layer );

    /**
     * Append a new layer node for given map layer. The newly created node is owned by this group.
     */
    QgsLayerTreeLayer *addLayer( QgsMapLayer *layer );

    /**
     * Insert existing nodes at specified position. The nodes must not have a parent yet. The nodes will be owned by this group.
     */
    void insertChildNodes( int index, const QList<QgsLayerTreeNode *> &nodes SIP_TRANSFER );

    /**
     * Insert existing node at specified position. The node must not have a parent yet. The node will be owned by this group.
     */
    void insertChildNode( int index, QgsLayerTreeNode *node SIP_TRANSFER );

    /**
     * Append an existing node. The node must not have a parent yet. The node will be owned by this group.
     */
    void addChildNode( QgsLayerTreeNode *node SIP_TRANSFER );

    /**
     * Remove a child node from this group. The node will be deleted.
     */
    void removeChildNode( QgsLayerTreeNode *node );

    /**
     * Remove map layer's node from this group. The node will be deleted.
     */
    void removeLayer( QgsMapLayer *layer );

    /**
     * Remove child nodes from index "from". The nodes will be deleted.
     */
    void removeChildren( int from, int count );

    /**
     * Remove all child group nodes without layers. The groupnodes will be deleted.
     */
    void removeChildrenGroupWithoutLayers();

    /**
     * Remove all child nodes. The nodes will be deleted.
     */
    void removeAllChildren();

    /**
     * Find layer node representing the map layer. Searches recursively the whole sub-tree.
     * \since QGIS 3.0
     */
    QgsLayerTreeLayer *findLayer( QgsMapLayer *layer ) const;

    /**
     * Find layer node representing the map layer specified by its ID. Searches recursively the whole sub-tree.
     */
    QgsLayerTreeLayer *findLayer( const QString &layerId ) const;

    /**
     * Find all layer nodes. Searches recursively the whole sub-tree.
     */
    QList<QgsLayerTreeLayer *> findLayers() const;


    /**
     * Returns an ordered list of map layers in the group, ignoring any layers which
     * are child layers of QgsGroupLayers. Searches recursively the whole sub-tree.
     *
     * \note Not available in Python bindings
     * \since QGIS 3.24
     */
    QList<QgsMapLayer *> layerOrderRespectingGroupLayers() const SIP_SKIP;

    /**
     * Find layer IDs used in all layer nodes. Searches recursively the whole sub-tree.
     */
    QStringList findLayerIds() const;

    /**
     * Find group node with specified name. Searches recursively the whole sub-tree.
     */
    QgsLayerTreeGroup *findGroup( const QString &name );

    /**
     * Find group layer nodes. Searches recursively the whole sub-tree, if recursive is set.
    */
    QList<QgsLayerTreeGroup *> findGroups( bool recursive = false ) const;

    /**
     * Read group (tree) from XML element <layer-tree-group> and return the newly created group (or NULLPTR on error).
     * Does not resolve textual references to layers. Call resolveReferences() afterwards to do it.
     */
    static QgsLayerTreeGroup *readXml( QDomElement &element, const QgsReadWriteContext &context ) SIP_FACTORY;

    /**
     * Read group (tree) from XML element <layer-tree-group> and return the newly created group (or NULLPTR on error).
     * Also resolves textual references to layers from the project (calls resolveReferences() internally).
     * \since QGIS 3.0
     */
    static QgsLayerTreeGroup *readXml( QDomElement &element, const QgsProject *project, const QgsReadWriteContext &context ) SIP_FACTORY;

    /**
     * Write group (tree) as XML element <layer-tree-group> and add it to the given parent element
     */
    void writeXml( QDomElement &parentElement, const QgsReadWriteContext &context ) override;

    /**
     * Read children from XML and append them to the group.
     * Does not resolve textual references to layers. Call resolveReferences() afterwards to do it.
     */
    void readChildrenFromXml( QDomElement &element, const QgsReadWriteContext &context );

    /**
     * Returns text representation of the tree. For debugging purposes only.
     */
    QString dump() const override;

    /**
     * Returns a clone of the group. The children are cloned too.
     */
    QgsLayerTreeGroup *clone() const override SIP_FACTORY;

    /**
     * Calls resolveReferences() on child tree nodes
     * \since QGIS 3.0
     */
    void resolveReferences( const QgsProject *project, bool looseMatching = false ) override;

    /**
     * Check or uncheck a node and all its children (taking into account exclusion rules)
     */
    void setItemVisibilityCheckedRecursive( bool checked ) override;

    /**
     * Returns whether the group is mutually exclusive (only one child can be checked at a time)
     * \since QGIS 2.12
     */
    bool isMutuallyExclusive() const;

    /**
     * Set whether the group is mutually exclusive (only one child can be checked at a time).
     * The initial child index determines which child should be initially checked. The default value
     * of -1 will determine automatically (either first one currently checked or none)
     * \since QGIS 2.12
     */
    void setIsMutuallyExclusive( bool enabled, int initialChildIndex = -1 );

    /**
     * Returns a reference to the associated group layer, if the layer tree group will be treated
     * as group layer during map rendering.
     *
     * \see setGroupLayer()
     * \see convertToGroupLayer()
     * \since QGIS 3.24
     */
    QgsGroupLayer *groupLayer();

    /**
     * Sets the associated group \a layer, if the layer tree group will be treated
     * as group layer during map rendering.
     *
     * This method does not take ownership of the group layer, and only a weak reference
     * to the layer is stored.
     *
     * \see groupLayer()
     * \see convertToGroupLayer()
     * \since QGIS 3.24
     */
    void setGroupLayer( QgsGroupLayer *layer );

    /**
     * Converts the group to a QgsGroupLayer.
     *
     * This method will convert the layer tree group to an equivalent QgsGroupLayer, and
     * return the result. The caller takes ownership of the returned layer, and it is the
     * caller's responsibility to add the layer to the associated QgsProject.
     *
     * If the group is already associated with a group layer (see groupLayer()), NULLPTR
     * will be returned.
     *
     * \see groupLayer()
     * \see setGroupLayer()
     * \since QGIS 3.24
     */
    QgsGroupLayer *convertToGroupLayer( const QgsGroupLayer::LayerOptions &options ) SIP_FACTORY;

  protected slots:

    void nodeVisibilityChanged( QgsLayerTreeNode *node );

  protected:

    /**
     * Set check state of children - if mutually exclusive
     */
    void updateChildVisibilityMutuallyExclusive();

    QString mName;

    bool mChangingChildVisibility = false;

    //! Whether the group is mutually exclusive (i.e. only one child can be checked at a time)
    bool mMutuallyExclusive = false;

    /**
     * Keeps track which child has been most recently selected
     * (so if the whole group is unchecked and checked again, we know which child to check)
     */
    int mMutuallyExclusiveChildIndex = -1;

    //! Sets parent to NULLPTR and disconnects all external and forwarded signals
    virtual void makeOrphan() override SIP_SKIP;

  private:

#ifdef SIP_RUN

    /**
     * Copies are not allowed
     */
    QgsLayerTreeGroup( const QgsLayerTreeGroup &other );
#endif

    QgsLayerTreeGroup &operator= ( const QgsLayerTreeGroup & ) = delete;

    void init();
    void updateGroupLayers();
    void refreshParentGroupLayerMembers();

    QgsMapLayerRef mGroupLayer;
};


#endif // QGSLAYERTREEGROUP_H
