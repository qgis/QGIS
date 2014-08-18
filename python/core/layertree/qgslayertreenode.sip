/**
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
 * - "overview" - whether to show a layer in overview
 * - "showFeatureCount" - whether to show feature counts in layer tree (vector only)
 * - "embedded" - whether the node comes from an external project
 * - "embedded_project" - path to the external project (embedded root node only)
 *
 * @see also QgsLayerTree, QgsLayerTreeLayer, QgsLayerTreeGroup
 * @note added in 2.4
 */
class QgsLayerTreeNode : QObject
{
%TypeHeaderCode
#include <qgslayertree.h>
%End

%ConvertToSubClassCode
  if (sipCpp->inherits("QgsLayerTreeNode"))
  {
    sipType = sipType_QgsLayerTreeNode;
    QgsLayerTreeNode* node = qobject_cast<QgsLayerTreeNode*>(sipCpp);
    if (QgsLayerTree::isLayer(node))
      sipType = sipType_QgsLayerTreeLayer;
    else if (QgsLayerTree::isGroup(node))
      sipType = sipType_QgsLayerTreeGroup;
  }
  else
    sipType = 0;
%End

  public:

    //! Enumeration of possible tree node types
    enum NodeType
    {
      NodeGroup,   //!< container of other groups and layers
      NodeLayer    //!< leaf node pointing to a layer
    };

    ~QgsLayerTreeNode();

    //! Find out about type of the node. It is usually shorter to use convenience functions from QgsLayerTree namespace for that
    NodeType nodeType();
    //! Get pointer to the parent. If parent is a null pointer, the node is a root node
    QgsLayerTreeNode* parent();
    //! Get list of children of the node. Children are owned by the parent
    QList<QgsLayerTreeNode*> children();

    //! Read layer tree from XML. Returns new instance
    static QgsLayerTreeNode* readXML( QDomElement& element );
    //! Write layer tree to XML
    virtual void writeXML( QDomElement& parentElement ) = 0;

    //! Return string with layer tree structure. For debug purposes only
    virtual QString dump() const = 0;

    //! Create a copy of the node. Returns new instance
    virtual QgsLayerTreeNode* clone() const = 0 /Factory/;

    //! Return whether the node should be shown as expanded or collapsed in GUI
    bool isExpanded() const;
    //! Set whether the node should be shown as expanded or collapsed in GUI
    void setExpanded( bool expanded );

    /** Set a custom property for the node. Properties are stored in a map and saved in project file. */
    void setCustomProperty( const QString& key, const QVariant& value );
    /** Read a custom property from layer. Properties are stored in a map and saved in project file. */
    QVariant customProperty( const QString& key, const QVariant& defaultValue = QVariant() ) const;
    /** Remove a custom property from layer. Properties are stored in a map and saved in project file. */
    void removeCustomProperty( const QString& key );
    /** Return list of keys stored in custom properties */
    QStringList customProperties() const;

  signals:

    //! Emitted when one or more nodes will be added to a node within the tree
    void willAddChildren( QgsLayerTreeNode* node, int indexFrom, int indexTo );
    //! Emitted when one or more nodes have been added to a node within the tree
    void addedChildren( QgsLayerTreeNode* node, int indexFrom, int indexTo );
    //! Emitted when one or more nodes will be removed from a node within the tree
    void willRemoveChildren( QgsLayerTreeNode* node, int indexFrom, int indexTo );
    //! Emitted when one or more nodes has been removed from a node within the tree
    void removedChildren( QgsLayerTreeNode* node, int indexFrom, int indexTo );
    //! Emitted when check state of a node within the tree has been changed
    void visibilityChanged( QgsLayerTreeNode* node, Qt::CheckState state );
    //! Emitted when a custom property of a node within the tree has been changed or removed
    void customPropertyChanged( QgsLayerTreeNode* node, QString key );
    //! Emitted when the collapsed/expanded state of a node within the tree has been changed
    void expandedChanged( QgsLayerTreeNode* node, bool expanded );

  protected:

    QgsLayerTreeNode( NodeType t );

  private:
    QgsLayerTreeNode( const QgsLayerTreeNode& other );

};
