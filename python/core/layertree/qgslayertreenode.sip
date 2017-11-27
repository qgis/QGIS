/************************************************************************
 * This file has been generated automatically from                      *
 *                                                                      *
 * src/core/layertree/qgslayertreenode.h                                *
 *                                                                      *
 * Do not edit manually ! Edit header and run scripts/sipify.pl again   *
 ************************************************************************/






class QgsLayerTreeNode : QObject
{
%Docstring
 This class is a base class for nodes in a layer tree.
 Layer tree is a hierarchical structure consisting of group and layer nodes:
 - group nodes are containers and may contain children (layer and group nodes)
 - layer nodes point to map layers, they do not contain further children

 Layer trees may be used for organization of layers, typically a layer tree
 is exposed to the user using QgsLayerTreeView widget which shows the tree
 and allows manipulation with the tree.

 Ownership of nodes: every node is owned by its parent. Therefore once node
 is added to a layer tree, it is the responsibility of the parent to delete it
 when the node is not needed anymore. Deletion of root node of a tree will
 delete all nodes of the tree.

 Signals: signals are propagated from children to parent. That means it is
 sufficient to connect to root node in order to get signals about updates
 in the whole layer tree. When adding or removing a node that contains further
 children (i.e. a whole subtree), the addition/removal signals are emitted
 only for the root node of the subtree that is being added or removed.

 Custom properties: Every node may have some custom properties assigned to it.
 This mechanism allows third parties store additional data with the nodes.
 The properties are used within QGIS code (whether to show layer in overview,
 whether the node is embedded from another project etc), but may be also
 used by third party plugins. Custom properties are stored also in the project
 file. The storage is not efficient for large amount of data.

 Custom properties that have already been used within QGIS:
 - "loading" - whether the project is being currently loaded (root node only)
 - "overview" - whether to show a layer in overview
 - "showFeatureCount" - whether to show feature counts in layer tree (vector only)
 - "embedded" - whether the node comes from an external project
 - "embedded_project" - path to the external project (embedded root node only)
 - "legend/..." - properties for legend appearance customization
 - "expandedLegendNodes" - list of layer's legend nodes' rules in expanded state

.. seealso:: also QgsLayerTree, QgsLayerTreeLayer, QgsLayerTreeGroup
.. versionadded:: 2.4
%End

%TypeHeaderCode
#include "qgslayertreenode.h"
%End
%ConvertToSubClassCode
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
%End
  public:

    enum NodeType
    {
      NodeGroup,
      NodeLayer
    };

    ~QgsLayerTreeNode();

    NodeType nodeType() const;
%Docstring
Find out about type of the node. It is usually shorter to use convenience functions from QgsLayerTree namespace for that
 :rtype: NodeType
%End
    QgsLayerTreeNode *parent();
%Docstring
Get pointer to the parent. If parent is a null pointer, the node is a root node
 :rtype: QgsLayerTreeNode
%End
    QList<QgsLayerTreeNode *> children();
%Docstring
Get list of children of the node. Children are owned by the parent
 :rtype: list of QgsLayerTreeNode
%End

    virtual QString name() const = 0;
%Docstring
 Return name of the node
.. versionadded:: 3.0
 :rtype: str
%End

    virtual void setName( const QString &name ) = 0;
%Docstring
 Set name of the node. Emits nameChanged signal.
.. versionadded:: 3.0
%End

    static QgsLayerTreeNode *readXml( QDomElement &element, const QgsReadWriteContext &context ) /Factory/;
%Docstring
 Read layer tree from XML. Returns new instance.
 Does not resolve textual references to layers. Call resolveReferences() afterwards to do it.
 :rtype: QgsLayerTreeNode
%End

    static QgsLayerTreeNode *readXml( QDomElement &element, const QgsProject *project ) /Factory/;
%Docstring
 Read layer tree from XML. Returns new instance.
 Also resolves textual references to layers from the project (calls resolveReferences() internally).
.. versionadded:: 3.0
 :rtype: QgsLayerTreeNode
%End

    virtual void writeXml( QDomElement &parentElement, const QgsReadWriteContext &context ) = 0;
%Docstring
Write layer tree to XML
%End

    virtual QString dump() const = 0;
%Docstring
Return string with layer tree structure. For debug purposes only
 :rtype: str
%End

    virtual QgsLayerTreeNode *clone() const = 0 /Factory/;
%Docstring
Create a copy of the node. Returns new instance
 :rtype: QgsLayerTreeNode
%End

    virtual void resolveReferences( const QgsProject *project, bool looseMatching = false ) = 0;
%Docstring
 Turn textual references to layers into map layer object from project.
 This method should be called after readXml()
 If ``looseMatching`` is true then a looser match will be used, where a layer
 will match if the name, public source, and data provider match. This can be
 used to match legend customisation from different projects where layers
 will have different layer IDs.
.. versionadded:: 3.0
%End

    bool isVisible() const;
%Docstring
 Returns whether a node is really visible (ie checked and all its ancestors checked as well)
.. versionadded:: 3.0
 :rtype: bool
%End

    bool itemVisibilityChecked() const;
%Docstring
 Returns whether a node is checked (independently of its ancestors or children)
.. versionadded:: 3.0
 :rtype: bool
%End

    void setItemVisibilityChecked( bool checked );
%Docstring
 Check or uncheck a node (independently of its ancestors or children)
.. versionadded:: 3.0
%End

    virtual void setItemVisibilityCheckedRecursive( bool checked );
%Docstring
 Check or uncheck a node and all its children (taking into account exclusion rules)
.. versionadded:: 3.0
%End

    void setItemVisibilityCheckedParentRecursive( bool checked );
%Docstring
 Check or uncheck a node and all its parents
.. versionadded:: 3.0
%End

    bool isItemVisibilityCheckedRecursive() const;
%Docstring
 Return whether this node is checked and all its children.
.. versionadded:: 3.0
 :rtype: bool
%End

    bool isItemVisibilityUncheckedRecursive() const;
%Docstring
 Return whether this node is unchecked and all its children.
.. versionadded:: 3.0
 :rtype: bool
%End

    QList< QgsMapLayer * > checkedLayers() const;
%Docstring
 Returns a list of any checked layers which belong to this node or its
 children.
.. versionadded:: 3.0
 :rtype: list of QgsMapLayer
%End

    bool isExpanded() const;
%Docstring
Return whether the node should be shown as expanded or collapsed in GUI
 :rtype: bool
%End
    void setExpanded( bool expanded );
%Docstring
Set whether the node should be shown as expanded or collapsed in GUI
%End

    void setCustomProperty( const QString &key, const QVariant &value );
%Docstring
Set a custom property for the node. Properties are stored in a map and saved in project file.
%End
    QVariant customProperty( const QString &key, const QVariant &defaultValue = QVariant() ) const;
%Docstring
Read a custom property from layer. Properties are stored in a map and saved in project file.
 :rtype: QVariant
%End
    void removeCustomProperty( const QString &key );
%Docstring
Remove a custom property from layer. Properties are stored in a map and saved in project file.
%End
    QStringList customProperties() const;
%Docstring
Return list of keys stored in custom properties
 :rtype: list of str
%End
    bool takeChild( QgsLayerTreeNode *node );
%Docstring
Remove a child from a node
 :rtype: bool
%End

  signals:

    void willAddChildren( QgsLayerTreeNode *node, int indexFrom, int indexTo );
%Docstring
Emitted when one or more nodes will be added to a node within the tree
%End
    void addedChildren( QgsLayerTreeNode *node, int indexFrom, int indexTo );
%Docstring
Emitted when one or more nodes have been added to a node within the tree
%End
    void willRemoveChildren( QgsLayerTreeNode *node, int indexFrom, int indexTo );
%Docstring
Emitted when one or more nodes will be removed from a node within the tree
%End
    void removedChildren( QgsLayerTreeNode *node, int indexFrom, int indexTo );
%Docstring
Emitted when one or more nodes has been removed from a node within the tree
%End
    void visibilityChanged( QgsLayerTreeNode *node );
%Docstring
Emitted when check state of a node within the tree has been changed
%End
    void customPropertyChanged( QgsLayerTreeNode *node, const QString &key );
%Docstring
Emitted when a custom property of a node within the tree has been changed or removed
%End
    void expandedChanged( QgsLayerTreeNode *node, bool expanded );
%Docstring
Emitted when the collapsed/expanded state of a node within the tree has been changed
%End

    void nameChanged( QgsLayerTreeNode *node, QString name );
%Docstring
 Emitted when the name of the node is changed
.. versionadded:: 3.0
%End

  protected:

    QgsLayerTreeNode( NodeType t, bool checked = true );
%Docstring
Constructor
%End
    QgsLayerTreeNode( const QgsLayerTreeNode &other );


    void readCommonXml( QDomElement &element );
%Docstring
Read common XML elements.
%End
    void writeCommonXml( QDomElement &element );
%Docstring
Write common XML elements.
%End

    void insertChildrenPrivate( int index, QList<QgsLayerTreeNode *> nodes );
%Docstring
Low-level insertion of children to the node. The children must not have any parent yet!
%End
    void removeChildrenPrivate( int from, int count, bool destroy = true );
%Docstring
Low-level removal of children from the node.
%End

  protected:
};




/************************************************************************
 * This file has been generated automatically from                      *
 *                                                                      *
 * src/core/layertree/qgslayertreenode.h                                *
 *                                                                      *
 * Do not edit manually ! Edit header and run scripts/sipify.pl again   *
 ************************************************************************/
