/**
 * The QgsLayerTreeModel class is model implementation for Qt item views framework.
 * The model can be used in any QTreeView, it is however recommended to use it
 * with QgsLayerTreeView which brings additional functionality specific to layer tree handling.
 *
 * The model listens to the changes in the layer tree and signals the changes as appropriate,
 * so that any view that uses the model is updated accordingly.
 *
 * Behavior of the model can be customized with flags. For example, whether to show legend or
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
      ShowLegend,                 //!< Add legend nodes for layer nodes
      ShowSymbology,              //!< deprecated - use ShowLegend
      ShowRasterPreviewIcon,      //!< Will use real preview of raster layer as icon (may be slow)
      ShowLegendAsTree,           //!< For legends that support it, will show them in a tree instead of a list (needs also ShowLegend). Added in 2.8
      DeferredLegendInvalidation, //!< defer legend model invalidation

      // behavioral flags
      AllowNodeReorder,           //!< Allow reordering with drag'n'drop
      AllowNodeRename,            //!< Allow renaming of groups and layers
      AllowNodeChangeVisibility,  //!< Allow user to set node visibility with a check box
      AllowLegendChangeState,     //!< Allow check boxes for legend nodes (if supported by layer's legend)
      AllowSymbologyChangeState,  //!< deprecated - use AllowLegendChangeState
    };
    typedef QFlags<QgsLayerTreeModel::Flag> Flags;

    //! Set OR-ed combination of model flags
    void setFlags( const Flags& f );
    //! Enable or disable a model flag
    void setFlag( Flag f, bool on = true );
    //! Return OR-ed combination of model flags
    Flags flags() const;
    //! Check whether a flag is enabled
    bool testFlag( Flag f ) const;

    //! Return layer tree node for given index. Returns root node for invalid index.
    //! Returns null pointer if index does not refer to a layer tree node (e.g. it is a legend node)
    QgsLayerTreeNode* index2node( const QModelIndex& index ) const;
    //! Return index for a given node. If the node does not belong to the layer tree, the result is undefined
    QModelIndex node2index( QgsLayerTreeNode* node ) const;
    //! Convert a list of indexes to a list of layer tree nodes.
    //! Indices that do not represent layer tree nodes are skipped.
    //! @arg skipInternal If true, a node is included in the output list only if no parent node is in the list
    QList<QgsLayerTreeNode*> indexes2nodes( const QModelIndexList& list, bool skipInternal = false ) const;

    //! Return legend node for given index. Returns null for invalid index
    //! @note added in 2.6
    static QgsLayerTreeModelLegendNode* index2legendNode( const QModelIndex& index );
    //! Return index for a given legend node. If the legend node does not belong to the layer tree, the result is undefined.
    //! If the legend node is belongs to the tree but it is filtered out, invalid model index is returned.
    //! @note added in 2.6
    QModelIndex legendNode2index( QgsLayerTreeModelLegendNode* legendNode );

    //! Return filtered list of active legend nodes attached to a particular layer node
    //! @note added in 2.6
    //! @see layerOriginalLegendNodes()
    QList<QgsLayerTreeModelLegendNode*> layerLegendNodes( QgsLayerTreeLayer* nodeLayer );

    //! Return original (unfiltered) list of legend nodes attached to a particular layer node
    //! @note added in 2.14
    //! @see layerLegendNodes()
    QList<QgsLayerTreeModelLegendNode*> layerOriginalLegendNodes( QgsLayerTreeLayer* nodeLayer );

    /** Searches through the layer tree to find a legend node with a matching layer ID
     * and rule key.
     * @param layerId map layer ID
     * @param ruleKey legend node rule key
     * @returns QgsLayerTreeModelLegendNode if found
     * @note added in QGIS 2.14
     */
    QgsLayerTreeModelLegendNode* findLegendNode( const QString& layerId, const QString& ruleKey ) const;

    //! Return pointer to the root node of the layer tree. Always a non-null pointer.
    QgsLayerTreeGroup* rootGroup() const;
    //! Reset the model and use a new root group node
    //! @note added in 2.6
    void setRootGroup( QgsLayerTreeGroup* newRootGroup );

    //! Force a refresh of legend nodes of a layer node.
    //! Not necessary to call when layer's renderer is changed as the model listens to these events.
    void refreshLayerLegend( QgsLayerTreeLayer* nodeLayer );

    //! Get index of the item marked as current. Item marked as current is underlined.
    QModelIndex currentIndex() const;
    //! Set index of the current item. May be used by view. Item marked as current is underlined.
    void setCurrentIndex( const QModelIndex& currentIndex );

    //! Set font for a particular type of layer tree node. nodeType should come from QgsLayerTreeNode::NodeType enumeration
    void setLayerTreeNodeFont( int nodeType, const QFont& font );
    //! Get font for a particular type of layer tree node. nodeType should come from QgsLayerTreeNode::NodeType enumeration
    QFont layerTreeNodeFont( int nodeType ) const;

    //! Set at what number of legend nodes the layer node should be collapsed. Setting -1 disables the auto-collapse (default).
    void setAutoCollapseLegendNodes( int nodeCount );
    //! Return at what number of legend nodes the layer node should be collapsed. -1 means no auto-collapse (default).
    int autoCollapseLegendNodes() const;

    //! Force only display of legend nodes which are valid for given scale denominator.
    //! Setting value <= 0 will disable the functionality
    //! @note added in 2.6
    void setLegendFilterByScale( double scaleDenominator );
    double legendFilterByScale() const;

    //! Force only display of legend nodes which are valid for given map settings.
    //! Setting null pointer or invalid map settings will disable the functionality.
    //! Ownership of map settings pointer does not change.
    //! @note added in 2.6
    void setLegendFilterByMap( const QgsMapSettings* settings );

    //! Filter display of legend nodes for given map settings
    //! @param settings Map settings. Setting a null pointer or invalid settings will disable any filter. Ownership is not changed, a copy is made
    //! @param useExtent Whether to use the extent of the map settings as a first spatial filter on legend nodes
    //! @param polygon If not empty, this polygon will be used instead of the map extent to filter legend nodes
    //! @param useExpressions Whether to use legend node filter expressions
    //! @note added in 2.14
    void setLegendFilter( const QgsMapSettings* settings, bool useExtent = true, const QgsGeometry& polygon = QgsGeometry(), bool useExpressions = true );

    //! Returns the current map settings used for legend filtering
    //! @deprecated It has been renamed to legendFilterMapSettings()
    const QgsMapSettings* legendFilterByMap() const /Deprecated/;

    //! Returns the current map settings used for the current legend filter (or null if none is enabled)
    //! @note added in 2.14
    const QgsMapSettings* legendFilterMapSettings() const;

    //! Give the layer tree model hints about the currently associated map view
    //! so that legend nodes that use map units can be scaled currectly
    //! @note added in 2.6
    void setLegendMapViewData( double mapUnitsPerPixel, int dpi, double scale );
    //! Get hints about map view - to be used in legend nodes. Arguments that are not null will receive values.
    //! If there are no valid map view data (from previous call to setLegendMapViewData()), returned values are zeros.
    //! @note added in 2.6
    void legendMapViewData( double *mapUnitsPerPixel /Out/, int *dpi /Out/, double *scale /Out/ );

    //! Get map of map layer style overrides (key: layer ID, value: style name) where a different style should be used instead of the current one
    //! @note added in 2.10
    QMap<QString, QString> layerStyleOverrides() const;
    //! Set map of map layer style overrides (key: layer ID, value: style name) where a different style should be used instead of the current one
    //! @note added in 2.10
    void setLayerStyleOverrides( const QMap<QString, QString>& overrides );

    //! Return true if index represents a legend node (instead of layer node)
    //! @deprecated use index2legendNode()
    bool isIndexSymbologyNode( const QModelIndex& index ) const /Deprecated/;
    //! Return layer node to which a legend node belongs to. Returns null pointer if index is not a legend node.
    //! @deprecated use index2legendNode()->parent()
    QgsLayerTreeLayer* layerNodeForSymbologyNode( const QModelIndex& index ) const /Deprecated/;
    //! @deprecated use refreshLayerLegend()
    void refreshLayerSymbology( QgsLayerTreeLayer* nodeLayer ) /Deprecated/;
    //! @deprecated use setAutoCollapseLegendNodes()
    void setAutoCollapseSymbologyNodes( int nodeCount ) /Deprecated/;
    //! @deprecated use autoCollapseLegendNodes()
    int autoCollapseSymbologyNodes() const /Deprecated/;

  signals:

  protected slots:
    void nodeWillAddChildren( QgsLayerTreeNode* node, int indexFrom, int indexTo );
    void nodeAddedChildren( QgsLayerTreeNode* node, int indexFrom, int indexTo );
    void nodeWillRemoveChildren( QgsLayerTreeNode* node, int indexFrom, int indexTo );
    void nodeRemovedChildren();

    void nodeVisibilityChanged( QgsLayerTreeNode* node );

    void nodeCustomPropertyChanged( QgsLayerTreeNode* node, const QString& key );

    void nodeLayerLoaded();
    void nodeLayerWillBeUnloaded();
    void layerLegendChanged();

    void layerNeedsUpdate();

    void legendNodeDataChanged();

    void invalidateLegendMapBasedData();

  protected:
    void removeLegendFromLayer( QgsLayerTreeLayer* nodeLayer );
    void addLegendToLayer( QgsLayerTreeLayer* nodeL );

    void connectToLayer( QgsLayerTreeLayer* nodeLayer );
    void disconnectFromLayer( QgsLayerTreeLayer* nodeLayer );

    void connectToLayers( QgsLayerTreeGroup* parentGroup );
    void disconnectFromLayers( QgsLayerTreeGroup* parentGroup );
    void connectToRootNode();
    void disconnectFromRootNode();

    //! emit dataChanged() for layer tree node items
    void recursivelyEmitDataChanged( const QModelIndex& index = QModelIndex() );

    /** Updates layer data for scale dependent layers, should be called when map scale changes.
     * Emits dataChanged() for all scale dependent layers.
     * @note added in QGIS 2.16
     */
    void refreshScaleBasedLayers( const QModelIndex& index = QModelIndex() );

    static const QIcon& iconGroup();

    //! Filter nodes from QgsMapLayerLegend according to the current filtering rules
    QList<QgsLayerTreeModelLegendNode*> filterLegendNodes( const QList<QgsLayerTreeModelLegendNode*>& nodes );

    QModelIndex indexOfParentLayerTreeNode( QgsLayerTreeNode* parentNode ) const;

    int legendRootRowCount( QgsLayerTreeLayer* nL ) const;
    int legendNodeRowCount( QgsLayerTreeModelLegendNode* node ) const;
    QModelIndex legendRootIndex( int row, int column, QgsLayerTreeLayer* nL ) const;
    QModelIndex legendNodeIndex( int row, int column, QgsLayerTreeModelLegendNode* node ) const;
    QModelIndex legendParent( QgsLayerTreeModelLegendNode* legendNode ) const;
    QVariant legendNodeData( QgsLayerTreeModelLegendNode* node, int role ) const;
    Qt::ItemFlags legendNodeFlags( QgsLayerTreeModelLegendNode* node ) const;
    bool legendEmbeddedInParent( QgsLayerTreeLayer* nodeLayer ) const;
    /** Return legend node that may be embbeded in parent (i.e. its icon will be used for layer's icon). */
    QgsLayerTreeModelLegendNode* legendNodeEmbeddedInParent( QgsLayerTreeLayer* nodeLayer ) const;
    QIcon legendIconEmbeddedInParent( QgsLayerTreeLayer* nodeLayer ) const;
    void legendCleanup();
    void legendInvalidateMapBasedData();

};
