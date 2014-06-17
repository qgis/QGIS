/**
 * The QgsLayerTreeModel class is model implementation for Qt item views framework.
 * The model can be used in any QTreeView, it is however recommended to use it
 * with QgsLayerTreeView which brings additional functionality specific to layer tree handling.
 *
 * The model listens to the changes in the layer tree and signals the changes as appropriate,
 * so that any view that uses the model is updated accordingly.
 *
 * Behavior of the model can be customized with flags. For example, whether to show symbology or
 * whether to allow changes to the layer tree.
 *
 * @see QgsLayerTreeView
 * @note added in 2.4
 */
class QgsLayerTreeModel : QAbstractItemModel
{
%TypeHeaderCode
#include <qgslayertreemodel.h>
%End

%ConvertToSubClassCode
  if (sipCpp->inherits("QgsLayerTreeModel"))
    sipType = sipType_QgsLayerTreeModel;
  else
    sipType = 0;
%End

  public:
    //! Construct a new tree model with given layer tree (root node must not be null pointer).
    //! The root node is not transferred by the model.
    explicit QgsLayerTreeModel( QgsLayerTreeGroup* rootNode, QObject *parent /TransferThis/ = 0 );
    ~QgsLayerTreeModel();

    // Implementation of virtual functions from QAbstractItemModel

    int rowCount( const QModelIndex &parent = QModelIndex() ) const;
    int columnCount( const QModelIndex &parent = QModelIndex() ) const;
    QModelIndex index( int row, int column, const QModelIndex &parent = QModelIndex() ) const;
    QModelIndex parent( const QModelIndex &child ) const;
    QVariant data( const QModelIndex &index, int role = Qt::DisplayRole ) const;
    Qt::ItemFlags flags( const QModelIndex &index ) const;
    bool setData( const QModelIndex &index, const QVariant &value, int role = Qt::EditRole );
    Qt::DropActions supportedDropActions() const;
    QStringList mimeTypes() const;
    QMimeData* mimeData( const QModelIndexList& indexes ) const;
    bool dropMimeData( const QMimeData* data, Qt::DropAction action, int row, int column, const QModelIndex& parent );
    bool removeRows( int row, int count, const QModelIndex& parent = QModelIndex() );

    // New stuff

    enum Flag
    {
      // display flags
      ShowSymbology,              //!< Add symbology items for layer nodes

      // behavioral flags
      AllowNodeReorder,           //!< Allow reordering with drag'n'drop
      AllowNodeRename,            //!< Allow renaming of groups and layers
      AllowNodeChangeVisibility,  //!< Allow user to set node visibility with a check box
    };
    typedef QFlags<QgsLayerTreeModel::Flag> Flags;

    //! Set OR-ed combination of model flags
    void setFlags( Flags f );
    //! Enable or disable a model flag
    void setFlag( Flag f, bool on = true );
    //! Return OR-ed combination of model flags
    Flags flags() const;
    //! Check whether a flag is enabled
    bool testFlag( Flag f ) const;

    //! Return layer tree node for given index. Returns root node for invalid index.
    //! Returns null pointer if index does not refer to a layer tree node (e.g. it is a symbology item)
    QgsLayerTreeNode* index2node( const QModelIndex& index ) const;
    //! Return index for a given node. If the node does not belong to the layer tree, the result is undefined
    QModelIndex node2index( QgsLayerTreeNode* node ) const;
    //! Convert a list of indexes to a list of layer tree nodes.
    //! Indices that do not represent layer tree nodes are skipped.
    //! @arg skipInternal If true, a node is included in the output list only if no parent node is in the list
    QList<QgsLayerTreeNode*> indexes2nodes( const QModelIndexList& list, bool skipInternal = false ) const;
    //! Return true if index represents a symbology node (instead of layer node)
    bool isIndexSymbologyNode( const QModelIndex& index ) const;
    //! Return layer node to which a symbology node belongs to. Returns null pointer if index is not a symbology node.
    QgsLayerTreeLayer* layerNodeForSymbologyNode( const QModelIndex& index ) const;

    //! Return pointer to the root node of the layer tree. Always a non-null pointer.
    QgsLayerTreeGroup* rootGroup();

    //! Force a refresh of symbology of layer node.
    //! Not necessary to call when layer's renderer is changed as the model listens to these events.
    void refreshLayerSymbology( QgsLayerTreeLayer* nodeLayer );

    //! Get index of the item marked as current. Item marked as current is underlined.
    QModelIndex currentIndex() const;
    //! Set index of the current item. May be used by view. Item marked as current is underlined.
    void setCurrentIndex( const QModelIndex& currentIndex );

    //! Set font for a particular type of layer tree node. nodeType should come from QgsLayerTreeNode::NodeType enumeration
    void setLayerTreeNodeFont( int nodeType, const QFont& font );
    //! Get font for a particular type of layer tree node. nodeType should come from QgsLayerTreeNode::NodeType enumeration
    QFont layerTreeNodeFont( int nodeType ) const;

    //! Set at what number of symbology nodes the layer node should be collapsed. Setting -1 disables the auto-collapse (default).
    void setAutoCollapseSymbologyNodes( int nodeCount );
    //! Return at what number of symbology nodes the layer node should be collapsed. -1 means no auto-collapse (default).
    int autoCollapseSymbologyNodes() const;

  signals:

};
