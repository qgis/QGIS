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

#include "qgslayertreenode.h"

class QgsMapLayer;
class QgsLayerTreeLayer;

/** \ingroup core
 * Layer tree group node serves as a container for layers and further groups.
 *
 * Group names do not need to be unique within one tree nor within one parent.
 *
 * @note added in 2.4
 */
class CORE_EXPORT QgsLayerTreeGroup : public QgsLayerTreeNode
{
    Q_OBJECT
  public:
    QgsLayerTreeGroup( const QString& name = QString(), Qt::CheckState checked = Qt::Checked );
    QgsLayerTreeGroup( const QgsLayerTreeGroup& other );

    //! Get group's name
    QString name() const { return mName; }
    //! Set group's name
    void setName( const QString& n ) { mName = n; }

    //! Insert a new group node with given name at specified position. Newly created node is owned by this group.
    QgsLayerTreeGroup* insertGroup( int index, const QString& name );
    //! Append a new group node with given name. Newly created node is owned by this group.
    QgsLayerTreeGroup* addGroup( const QString& name );
    //! Insert a new layer node for given map layer at specified position. Newly created node is owned by this group.
    QgsLayerTreeLayer* insertLayer( int index, QgsMapLayer* layer );
    //! Append a new layer node for given map layer. Newly created node is owned by this group.
    QgsLayerTreeLayer* addLayer( QgsMapLayer* layer );

    //! Insert existing nodes at specified position. The nodes must not have a parent yet. The nodes will be owned by this group.
    void insertChildNodes( int index, const QList<QgsLayerTreeNode*>& nodes );
    //! Insert existing node at specified position. The node must not have a parent yet. The node will be owned by this group.
    void insertChildNode( int index, QgsLayerTreeNode* node );
    //! Append an existing node. The node must not have a parent yet. The node will be owned by this group.
    void addChildNode( QgsLayerTreeNode* node );

    //! Remove a child node from this group. The node will be deleted.
    void removeChildNode( QgsLayerTreeNode* node );
    //! Remove map layer's node from this group. The node will be deleted.
    void removeLayer( QgsMapLayer* layer );
    //! Remove child nodes from index "from". The nodes will be deleted.
    void removeChildren( int from, int count );
    //! Remove all child group nodes without layers. The groupnodes will be deleted.
    void removeChildrenGroupWithoutLayers();
    //! Remove all child nodes. The nodes will be deleted.
    void removeAllChildren();

    //! Find layer node representing the map layer specified by its ID. Searches recursively the whole sub-tree.
    QgsLayerTreeLayer* findLayer( const QString& layerId ) const;
    //! Find all layer nodes. Searches recursively the whole sub-tree.
    QList<QgsLayerTreeLayer*> findLayers() const;
    //! Find layer IDs used in all layer nodes. Searches recursively the whole sub-tree.
    QStringList findLayerIds() const;
    //! Find group node with specified name. Searches recursively the whole sub-tree.
    QgsLayerTreeGroup* findGroup( const QString& name );

    //! Read group (tree) from XML element <layer-tree-group> and return the newly created group (or null on error)
    static QgsLayerTreeGroup* readXML( QDomElement& element );
    //! Write group (tree) as XML element <layer-tree-group> and add it to the given parent element
    virtual void writeXML( QDomElement& parentElement ) override;
    //! Read children from XML and append them to the group.
    void readChildrenFromXML( QDomElement& element );

    //! Return text representation of the tree. For debugging purposes only.
    virtual QString dump() const override;

    //! Return a clone of the group. The children are cloned too.
    virtual QgsLayerTreeGroup* clone() const override;

    //! Return the check state of the group node
    Qt::CheckState isVisible() const { return mChecked; }
    //! Set check state of the group node - will also update children
    void setVisible( Qt::CheckState state );

    //! Return whether the group is mutually exclusive (only one child can be checked at a time)
    //! @note added in 2.12
    bool isMutuallyExclusive() const;
    //! Set whether the group is mutually exclusive (only one child can be checked at a time).
    //! The initial child index determines which child should be initially checked. The default value
    //! of -1 will determine automatically (either first one currently checked or none)
    //! @note added in 2.12
    void setIsMutuallyExclusive( bool enabled, int initialChildIndex = -1 );

  protected slots:
    void layerDestroyed();
    void nodeVisibilityChanged( QgsLayerTreeNode* node );

  protected:
    //! Set check state of this group from its children
    void updateVisibilityFromChildren();
    //! Set check state of children (when this group's check state changes) - if not mutually exclusive
    void updateChildVisibility();
    //! Set check state of children - if mutually exclusive
    void updateChildVisibilityMutuallyExclusive();

  protected:
    QString mName;
    Qt::CheckState mChecked;

    bool mChangingChildVisibility;

    //! Whether the group is mutually exclusive (i.e. only one child can be checked at a time)
    bool mMutuallyExclusive;
    //! Keeps track which child has been most recently selected
    //! (so if the whole group is unchecked and checked again, we know which child to check)
    int mMutuallyExclusiveChildIndex;
};


#endif // QGSLAYERTREEGROUP_H
