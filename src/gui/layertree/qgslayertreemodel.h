/***************************************************************************
  qgslayertreemodel.h
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

#ifndef QGSLAYERTREEMODEL_H
#define QGSLAYERTREEMODEL_H

#include <QAbstractItemModel>
#include <QFont>
#include <QIcon>

class QgsLayerTreeNode;
class QgsLayerTreeGroup;
class QgsLayerTreeLayer;
class QgsLayerTreeModelLegendNode;
class QgsMapLayer;


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
class GUI_EXPORT QgsLayerTreeModel : public QAbstractItemModel
{
    Q_OBJECT
  public:
    //! Construct a new tree model with given layer tree (root node must not be null pointer).
    //! The root node is not transferred by the model.
    explicit QgsLayerTreeModel( QgsLayerTreeGroup* rootNode, QObject *parent = 0 );
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
      ShowSymbology             = 0x0001,  //!< Add symbology items for layer nodes
      ShowRasterPreviewIcon     = 0x0002,  //!< Will use real preview of raster layer as icon (may be slow)

      // behavioral flags
      AllowNodeReorder          = 0x1000,  //!< Allow reordering with drag'n'drop
      AllowNodeRename           = 0x2000,  //!< Allow renaming of groups and layers
      AllowNodeChangeVisibility = 0x4000,  //!< Allow user to set node visibility with a check box
    };
    Q_DECLARE_FLAGS( Flags, Flag )

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
    void setAutoCollapseSymbologyNodes( int nodeCount ) { mAutoCollapseSymNodesCount = nodeCount; }
    //! Return at what number of symbology nodes the layer node should be collapsed. -1 means no auto-collapse (default).
    int autoCollapseSymbologyNodes() const { return mAutoCollapseSymNodesCount; }

  signals:

  protected slots:
    void nodeWillAddChildren( QgsLayerTreeNode* node, int indexFrom, int indexTo );
    void nodeAddedChildren( QgsLayerTreeNode* node, int indexFrom, int indexTo );
    void nodeWillRemoveChildren( QgsLayerTreeNode* node, int indexFrom, int indexTo );
    void nodeRemovedChildren();

    void nodeVisibilityChanged( QgsLayerTreeNode* node );

    void nodeLayerLoaded();
    void layerLegendChanged();

    void layerNeedsUpdate();

  protected:
    void removeSymbologyFromLayer( QgsLayerTreeLayer* nodeLayer );
    void addSymbologyToLayer( QgsLayerTreeLayer* nodeL );

    void connectToLayer( QgsLayerTreeLayer* nodeLayer );
    void disconnectFromLayer( QgsLayerTreeLayer* nodeLayer );

    //! emit dataChanged() for layer tree node items
    void recursivelyEmitDataChanged( const QModelIndex& index = QModelIndex() );

    static QgsLayerTreeModelLegendNode* index2symnode( const QModelIndex& index );

    static const QIcon& iconGroup();

  protected:
    //! Pointer to the root node of the layer tree. Not owned by the model
    QgsLayerTreeGroup* mRootNode;
    //! Set of flags for the model
    Flags mFlags;
    //! Data structure for storage of symbology nodes for each layer
    QMap<QgsLayerTreeLayer*, QList<QgsLayerTreeModelLegendNode*> > mSymbologyNodes;
    //! Current index - will be underlined
    QPersistentModelIndex mCurrentIndex;
    //! Minimal number of nodes when symbology should be automatically collapsed. -1 = disabled
    int mAutoCollapseSymNodesCount;

    QFont mFontLayer;
    QFont mFontGroup;
};

Q_DECLARE_OPERATORS_FOR_FLAGS( QgsLayerTreeModel::Flags )

#endif // QGSLAYERTREEMODEL_H
