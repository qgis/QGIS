/***************************************************************************
  qgslayertreenode.h
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

#ifndef QGSLAYERTREENODE_H
#define QGSLAYERTREENODE_H

#include "qgis_core.h"
#include <QObject>

#include "qgsobjectcustomproperties.h"
#include "qgsreadwritecontext.h"
#include "qgis_sip.h"

class QDomElement;

class QgsProject;
class QgsMapLayer;

/**
 * \ingroup core
 * This class is a base class for nodes in a layer tree.
 * Layer tree is a hierarchical structure consisting of group and layer nodes:
 * - group nodes are containers and may contain children (layer and group nodes)
 * - layer nodes point to map layers, they do not contain further children
 *
 * Layer trees may be used for organization of layers, typically a layer tree
 * is exposed to the user using QgsLayerTreeView widget which shows the tree
 * and allows manipulation with the tree.
 *
 * Ownership of nodes: every node is owned by its parent. Therefore once node
 * is added to a layer tree, it is the responsibility of the parent to delete it
 * when the node is not needed anymore. Deletion of root node of a tree will
 * delete all nodes of the tree.
 *
 * Signals: signals are propagated from children to parent. That means it is
 * sufficient to connect to root node in order to get signals about updates
 * in the whole layer tree. When adding or removing a node that contains further
 * children (i.e. a whole subtree), the addition/removal signals are emitted
 * only for the root node of the subtree that is being added or removed.
 *
 * Custom properties: Every node may have some custom properties assigned to it.
 * This mechanism allows third parties store additional data with the nodes.
 * The properties are used within QGIS code (whether to show layer in overview,
 * whether the node is embedded from another project etc), but may be also
 * used by third party plugins. Custom properties are stored also in the project
 * file. The storage is not efficient for large amount of data.
 *
 * Custom properties that have already been used within QGIS:
 * - "loading" - whether the project is being currently loaded (root node only)
 * - "overview" - whether to show a layer in overview
 * - "showFeatureCount" - whether to show feature counts in layer tree (vector only)
 * - "embedded" - whether the node comes from an external project
 * - "embedded_project" - path to the external project (embedded root node only)
 * - "legend/..." - properties for legend appearance customization
 * - "expandedLegendNodes" - list of layer's legend nodes' rules in expanded state
 *
 * \see also QgsLayerTree, QgsLayerTreeLayer, QgsLayerTreeGroup
 * \since QGIS 2.4
 */
class CORE_EXPORT QgsLayerTreeNode : public QObject
{
    Q_OBJECT

#ifdef SIP_RUN
    SIP_CONVERT_TO_SUBCLASS_CODE
    if ( sipCpp->inherits( "QgsLayerTreeNode" ) )
    {
      sipType = sipType_QgsLayerTreeNode;
      QgsLayerTreeNode *node = qobject_cast<QgsLayerTreeNode *>( sipCpp );
      if ( QgsLayerTree::isLayer( node ) )
        sipType = sipType_QgsLayerTreeLayer;
      else if ( qobject_cast<QgsLayerTree *>( sipCpp ) )
        sipType = sipType_QgsLayerTree;
      else if ( QgsLayerTree::isGroup( node ) )
        sipType = sipType_QgsLayerTreeGroup;
    }
    else
      sipType = 0;
    SIP_END
#endif

  public:

    //! Enumeration of possible tree node types
    enum NodeType
    {
      NodeGroup,   //!< Container of other groups and layers
      NodeLayer    //!< Leaf node pointing to a layer
    };

    ~QgsLayerTreeNode() override;

    //! Find out about type of the node. It is usually shorter to use convenience functions from QgsLayerTree namespace for that
    NodeType nodeType() const { return mNodeType; }
    //! Gets pointer to the parent. If parent is NULLPTR, the node is a root node
    QgsLayerTreeNode *parent() { return mParent; }
    //! Gets list of children of the node. Children are owned by the parent
    QList<QgsLayerTreeNode *> children() { return mChildren; }
    //! Gets list of children of the node. Children are owned by the parent
    QList<QgsLayerTreeNode *> children() const { return mChildren; } SIP_SKIP

    /**
     * Returns name of the node
     * \since QGIS 3.0
     */
    virtual QString name() const = 0;

    /**
     * Set name of the node. Emits nameChanged signal.
     * \since QGIS 3.0
     */
    virtual void setName( const QString &name ) = 0;

    /**
     * Read layer tree from XML. Returns new instance.
     * Does not resolve textual references to layers. Call resolveReferences() afterwards to do it.
     */
    static QgsLayerTreeNode *readXml( QDomElement &element, const QgsReadWriteContext &context ) SIP_FACTORY;

    /**
     * Read layer tree from XML. Returns new instance.
     * Also resolves textual references to layers from the project (calls resolveReferences() internally).
     * \since QGIS 3.0
     */
    static QgsLayerTreeNode *readXml( QDomElement &element, const QgsProject *project ) SIP_FACTORY;

    //! Write layer tree to XML
    virtual void writeXml( QDomElement &parentElement, const QgsReadWriteContext &context ) = 0;

    //! Returns string with layer tree structure. For debug purposes only
    virtual QString dump() const = 0;

    //! Create a copy of the node. Returns new instance
    virtual QgsLayerTreeNode *clone() const = 0 SIP_FACTORY;

    /**
     * Turn textual references to layers into map layer object from project.
     * This method should be called after readXml()
     * If \a looseMatching is TRUE then a looser match will be used, where a layer
     * will match if the name, public source, and data provider match. This can be
     * used to match legend customization from different projects where layers
     * will have different layer IDs.
     * \since QGIS 3.0
     */
    virtual void resolveReferences( const QgsProject *project, bool looseMatching = false ) = 0;

    /**
     * Returns whether a node is really visible (ie checked and all its ancestors checked as well)
     * \since QGIS 3.0
     */
    bool isVisible() const;

    /**
     * Returns whether a node is checked (independently of its ancestors or children)
     * \since QGIS 3.0
     */
    bool itemVisibilityChecked() const { return mChecked; }

    /**
     * Check or uncheck a node (independently of its ancestors or children)
     * \since QGIS 3.0
     */
    void setItemVisibilityChecked( bool checked );

    /**
     * Check or uncheck a node and all its children (taking into account exclusion rules)
     * \since QGIS 3.0
     */
    virtual void setItemVisibilityCheckedRecursive( bool checked );

    /**
     * Check or uncheck a node and all its parents
     * \since QGIS 3.0
     */
    void setItemVisibilityCheckedParentRecursive( bool checked );

    /**
     * Returns whether this node is checked and all its children.
     * \since QGIS 3.0
     */
    bool isItemVisibilityCheckedRecursive() const;

    /**
     * Returns whether this node is unchecked and all its children.
     * \since QGIS 3.0
     */
    bool isItemVisibilityUncheckedRecursive() const;

    /**
     * Returns a list of any checked layers which belong to this node or its
     * children.
     * \since QGIS 3.0
     */
    QList< QgsMapLayer * > checkedLayers() const;

    //! Returns whether the node should be shown as expanded or collapsed in GUI
    bool isExpanded() const;
    //! Sets whether the node should be shown as expanded or collapsed in GUI
    void setExpanded( bool expanded );

    //! Sets a custom property for the node. Properties are stored in a map and saved in project file.
    void setCustomProperty( const QString &key, const QVariant &value );
    //! Read a custom property from layer. Properties are stored in a map and saved in project file.
    QVariant customProperty( const QString &key, const QVariant &defaultValue = QVariant() ) const;
    //! Remove a custom property from layer. Properties are stored in a map and saved in project file.
    void removeCustomProperty( const QString &key );
    //! Returns list of keys stored in custom properties
    QStringList customProperties() const;
    //! Remove a child from a node
    bool takeChild( QgsLayerTreeNode *node );


  signals:

    //! Emitted when one or more nodes will be added to a node within the tree
    void willAddChildren( QgsLayerTreeNode *node, int indexFrom, int indexTo );
    //! Emitted when one or more nodes have been added to a node within the tree
    void addedChildren( QgsLayerTreeNode *node, int indexFrom, int indexTo );
    //! Emitted when one or more nodes will be removed from a node within the tree
    void willRemoveChildren( QgsLayerTreeNode *node, int indexFrom, int indexTo );
    //! Emitted when one or more nodes has been removed from a node within the tree
    void removedChildren( QgsLayerTreeNode *node, int indexFrom, int indexTo );
    //! Emitted when check state of a node within the tree has been changed
    void visibilityChanged( QgsLayerTreeNode *node );
    //! Emitted when a custom property of a node within the tree has been changed or removed
    void customPropertyChanged( QgsLayerTreeNode *node, const QString &key );
    //! Emitted when the collapsed/expanded state of a node within the tree has been changed
    void expandedChanged( QgsLayerTreeNode *node, bool expanded );

    /**
     * Emitted when the name of the node is changed
     * \since QGIS 3.0
     */
    void nameChanged( QgsLayerTreeNode *node, QString name );

  protected:

    //! Constructor
    QgsLayerTreeNode( NodeType t, bool checked = true );
    QgsLayerTreeNode( const QgsLayerTreeNode &other );

    // low-level utility functions

    //! Read common XML elements.
    void readCommonXml( QDomElement &element );
    //! Write common XML elements.
    void writeCommonXml( QDomElement &element );

    //! Low-level insertion of children to the node. The children must not have any parent yet!
    void insertChildrenPrivate( int index, QList<QgsLayerTreeNode *> nodes );
    //! Low-level removal of children from the node.
    void removeChildrenPrivate( int from, int count, bool destroy = true );

  protected:
    //! type of the node - determines which subclass is used
    NodeType mNodeType;
    bool mChecked;
    //! pointer to the parent node - null in case of root node
    QgsLayerTreeNode *mParent = nullptr;
    //! list of children - node is responsible for their deletion
    QList<QgsLayerTreeNode *> mChildren;
    //! whether the node should be shown in GUI as expanded
    bool mExpanded;
    //! custom properties attached to the node
    QgsObjectCustomProperties mProperties;

};




#endif // QGSLAYERTREENODE_H
