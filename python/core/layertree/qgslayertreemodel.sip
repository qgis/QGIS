/************************************************************************
 * This file has been generated automatically from                      *
 *                                                                      *
 * src/core/layertree/qgslayertreemodel.h                               *
 *                                                                      *
 * Do not edit manually ! Edit header and run scripts/sipify.pl again   *
 ************************************************************************/





class QgsLayerTreeModel : QAbstractItemModel
{
%Docstring
The QgsLayerTreeModel class is model implementation for Qt item views framework.
The model can be used in any QTreeView, it is however recommended to use it
with QgsLayerTreeView which brings additional functionality specific to layer tree handling.

The model listens to the changes in the layer tree and signals the changes as appropriate,
so that any view that uses the model is updated accordingly.

Behavior of the model can be customized with flags. For example, whether to show legend or
whether to allow changes to the layer tree.

.. seealso:: :py:class:`QgsLayerTreeView`

.. versionadded:: 2.4
%End

%TypeHeaderCode
#include "qgslayertreemodel.h"
%End
%ConvertToSubClassCode
    if ( sipCpp->inherits( "QgsLayerTreeModel" ) )
      sipType = sipType_QgsLayerTreeModel;
    else
      sipType = 0;
%End
  public:

    explicit QgsLayerTreeModel( QgsLayerTree *rootNode, QObject *parent /TransferThis/ = 0 );
%Docstring
Construct a new tree model with given layer tree (root node must not be null pointer).
The root node is not transferred by the model.
%End

    ~QgsLayerTreeModel();


    virtual int rowCount( const QModelIndex &parent = QModelIndex() ) const;

    virtual int columnCount( const QModelIndex &parent = QModelIndex() ) const;

    virtual QModelIndex index( int row, int column, const QModelIndex &parent = QModelIndex() ) const;

    virtual QModelIndex parent( const QModelIndex &child ) const;

    virtual QVariant data( const QModelIndex &index, int role = Qt::DisplayRole ) const;

    virtual Qt::ItemFlags flags( const QModelIndex &index ) const;

    virtual bool setData( const QModelIndex &index, const QVariant &value, int role = Qt::EditRole );

    virtual Qt::DropActions supportedDropActions() const;

    virtual QStringList mimeTypes() const;

    virtual QMimeData *mimeData( const QModelIndexList &indexes ) const;

    virtual bool dropMimeData( const QMimeData *data, Qt::DropAction action, int row, int column, const QModelIndex &parent );

    virtual bool removeRows( int row, int count, const QModelIndex &parent = QModelIndex() );



    enum Flag
    {
      // display
      ShowLegend,
      ShowLegendAsTree,
      DeferredLegendInvalidation,
      UseEmbeddedWidgets,

      // behavioral
      AllowNodeReorder,
      AllowNodeRename,
      AllowNodeChangeVisibility,
      AllowLegendChangeState,
      ActionHierarchical,
    };
    typedef QFlags<QgsLayerTreeModel::Flag> Flags;


    void setFlags( QgsLayerTreeModel::Flags f );
%Docstring
Set OR-ed combination of model flags
%End
    void setFlag( Flag f, bool on = true );
%Docstring
Enable or disable a model flag
%End
    Flags flags() const;
%Docstring
Return OR-ed combination of model flags
%End
    bool testFlag( Flag f ) const;
%Docstring
Check whether a flag is enabled
%End

    QgsLayerTreeNode *index2node( const QModelIndex &index ) const;
%Docstring
Return layer tree node for given index. Returns root node for invalid index.
Returns null pointer if index does not refer to a layer tree node (e.g. it is a legend node)
%End
    QModelIndex node2index( QgsLayerTreeNode *node ) const;
%Docstring
Return index for a given node. If the node does not belong to the layer tree, the result is undefined
%End

    QList<QgsLayerTreeNode *> indexes2nodes( const QModelIndexList &list, bool skipInternal = false ) const;
%Docstring
Convert a ``list`` of indexes to a list of layer tree nodes.
Indices that do not represent layer tree nodes are skipped.
If ``skipInternal`` is true, a node is included in the output list only if no parent node is in the list.
%End

    static QgsLayerTreeModelLegendNode *index2legendNode( const QModelIndex &index );
%Docstring
Return legend node for given index. Returns null for invalid index

.. versionadded:: 2.6
%End

    QModelIndex legendNode2index( QgsLayerTreeModelLegendNode *legendNode );
%Docstring
Return index for a given legend node. If the legend node does not belong to the layer tree, the result is undefined.
If the legend node is belongs to the tree but it is filtered out, invalid model index is returned.

.. versionadded:: 2.6
%End

    QList<QgsLayerTreeModelLegendNode *> layerLegendNodes( QgsLayerTreeLayer *nodeLayer, bool skipNodeEmbeddedInParent = false );
%Docstring
Return filtered list of active legend nodes attached to a particular layer node
(by default it returns also legend node embedded in parent layer node (if any) unless skipNodeEmbeddedInParent is true)

.. versionadded:: 2.6

.. note::

   Parameter skipNodeEmbeddedInParent added in QGIS 2.18

.. seealso:: :py:func:`layerOriginalLegendNodes`
%End

    QList<QgsLayerTreeModelLegendNode *> layerOriginalLegendNodes( QgsLayerTreeLayer *nodeLayer );
%Docstring
Return original (unfiltered) list of legend nodes attached to a particular layer node

.. versionadded:: 2.14

.. seealso:: :py:func:`layerLegendNodes`
%End

    QgsLayerTreeModelLegendNode *legendNodeEmbeddedInParent( QgsLayerTreeLayer *nodeLayer ) const;
%Docstring
Return legend node that may be embedded in parent (i.e. its icon will be used for layer's icon).

.. versionadded:: 2.18
%End

    QgsLayerTreeModelLegendNode *findLegendNode( const QString &layerId, const QString &ruleKey ) const;
%Docstring
Searches through the layer tree to find a legend node with a matching layer ID
and rule key.

:param layerId: map layer ID
:param ruleKey: legend node rule key

:return: QgsLayerTreeModelLegendNode if found

.. versionadded:: 2.14
%End

    QgsLayerTree *rootGroup() const;
%Docstring
Return pointer to the root node of the layer tree. Always a non-null pointer.
%End

    void setRootGroup( QgsLayerTree *newRootGroup );
%Docstring
Reset the model and use a new root group node

.. versionadded:: 2.6
%End

    void refreshLayerLegend( QgsLayerTreeLayer *nodeLayer );
%Docstring
Force a refresh of legend nodes of a layer node.
Not necessary to call when layer's renderer is changed as the model listens to these events.
%End

    QModelIndex currentIndex() const;
%Docstring
Get index of the item marked as current. Item marked as current is underlined.
%End
    void setCurrentIndex( const QModelIndex &currentIndex );
%Docstring
Set index of the current item. May be used by view. Item marked as current is underlined.
%End

    void setLayerTreeNodeFont( int nodeType, const QFont &font );
%Docstring
Set font for a particular type of layer tree node. nodeType should come from QgsLayerTreeNode.NodeType enumeration
%End
    QFont layerTreeNodeFont( int nodeType ) const;
%Docstring
Get font for a particular type of layer tree node. nodeType should come from QgsLayerTreeNode.NodeType enumeration
%End

    void setAutoCollapseLegendNodes( int nodeCount );
%Docstring
Set at what number of legend nodes the layer node should be collapsed. Setting -1 disables the auto-collapse (default).
%End
    int autoCollapseLegendNodes() const;
%Docstring
Return at what number of legend nodes the layer node should be collapsed. -1 means no auto-collapse (default).
%End

    void setLegendFilterByScale( double scale );
%Docstring
Force only display of legend nodes which are valid for a given ``scale``.
The ``scale`` value indicates the scale denominator, e.g. 1000.0 for a 1:1000 map.
Setting ``scale`` <= 0 will disable the functionality.

.. versionadded:: 2.6

.. seealso:: :py:func:`legendFilterByScale`
%End

    double legendFilterByScale() const;
%Docstring
Returns the scale which restricts the legend nodes which are visible.
The  scale value indicates the scale denominator, e.g. 1000.0 for a 1:1000 map.
A scale <= 0 indicates that no scale filtering is being performed.

.. versionadded:: 2.6

.. seealso:: :py:func:`setLegendFilterByScale`
%End

    void setLegendFilterByMap( const QgsMapSettings *settings );
%Docstring
Force only display of legend nodes which are valid for given map settings.
Setting null pointer or invalid map settings will disable the functionality.
Ownership of map settings pointer does not change, a copy is made.

.. versionadded:: 2.6
%End

    void setLegendFilter( const QgsMapSettings *settings, bool useExtent = true, const QgsGeometry &polygon = QgsGeometry(), bool useExpressions = true );
%Docstring
Filter display of legend nodes for given map settings

:param settings: Map settings. Setting a null pointer or invalid settings will disable any filter. Ownership is not changed, a copy is made
:param useExtent: Whether to use the extent of the map settings as a first spatial filter on legend nodes
:param polygon: If not empty, this polygon will be used instead of the map extent to filter legend nodes
:param useExpressions: Whether to use legend node filter expressions

.. versionadded:: 2.14
%End

    const QgsMapSettings *legendFilterMapSettings() const;
%Docstring
Returns the current map settings used for the current legend filter (or null if none is enabled)

.. versionadded:: 2.14
%End

    void setLegendMapViewData( double mapUnitsPerPixel, int dpi, double scale );
%Docstring
Give the layer tree model hints about the currently associated map view
so that legend nodes that use map units can be scaled currectly

.. versionadded:: 2.6
%End

    void legendMapViewData( double *mapUnitsPerPixel /Out/, int *dpi /Out/, double *scale  /Out/ ) const;
%Docstring
Get hints about map view - to be used in legend nodes. Arguments that are not null will receive values.
If there are no valid map view data (from previous call to setLegendMapViewData()), returned values are zeros.

.. versionadded:: 2.6
%End

    QMap<QString, QString> layerStyleOverrides() const;
%Docstring
Get map of map layer style overrides (key: layer ID, value: style name) where a different style should be used instead of the current one

.. versionadded:: 2.10
%End

    void setLayerStyleOverrides( const QMap<QString, QString> &overrides );
%Docstring
Set map of map layer style overrides (key: layer ID, value: style name) where a different style should be used instead of the current one

.. versionadded:: 2.10
%End

  protected slots:
    void nodeWillAddChildren( QgsLayerTreeNode *node, int indexFrom, int indexTo );
    void nodeAddedChildren( QgsLayerTreeNode *node, int indexFrom, int indexTo );
    void nodeWillRemoveChildren( QgsLayerTreeNode *node, int indexFrom, int indexTo );
    void nodeRemovedChildren();

    void nodeVisibilityChanged( QgsLayerTreeNode *node );

    void nodeNameChanged( QgsLayerTreeNode *node, const QString &name );
%Docstring
Updates model when node's name has changed

.. versionadded:: 3.0
%End

    void nodeCustomPropertyChanged( QgsLayerTreeNode *node, const QString &key );

    void nodeLayerLoaded();
    void nodeLayerWillBeUnloaded();
    void layerLegendChanged();

    void layerNeedsUpdate();

    void legendNodeDataChanged();

    void invalidateLegendMapBasedData();

  protected:
    void removeLegendFromLayer( QgsLayerTreeLayer *nodeLayer );
    void addLegendToLayer( QgsLayerTreeLayer *nodeL );

    void connectToLayer( QgsLayerTreeLayer *nodeLayer );
    void disconnectFromLayer( QgsLayerTreeLayer *nodeLayer );

    void connectToLayers( QgsLayerTreeGroup *parentGroup );
    void disconnectFromLayers( QgsLayerTreeGroup *parentGroup );
    void connectToRootNode();
    void disconnectFromRootNode();

    void recursivelyEmitDataChanged( const QModelIndex &index = QModelIndex() );
%Docstring
emit dataChanged() for layer tree node items
%End

    void refreshScaleBasedLayers( const QModelIndex &index = QModelIndex() );
%Docstring
Updates layer data for scale dependent layers, should be called when map scale changes.
Emits dataChanged() for all scale dependent layers.

.. versionadded:: 2.16
%End

    static QIcon iconGroup();

    QList<QgsLayerTreeModelLegendNode *> filterLegendNodes( const QList<QgsLayerTreeModelLegendNode *> &nodes );
%Docstring
Filter nodes from QgsMapLayerLegend according to the current filtering rules
%End

    QModelIndex indexOfParentLayerTreeNode( QgsLayerTreeNode *parentNode ) const;

    int legendRootRowCount( QgsLayerTreeLayer *nL ) const;
    int legendNodeRowCount( QgsLayerTreeModelLegendNode *node ) const;
    QModelIndex legendRootIndex( int row, int column, QgsLayerTreeLayer *nL ) const;
    QModelIndex legendNodeIndex( int row, int column, QgsLayerTreeModelLegendNode *node ) const;
    QModelIndex legendParent( QgsLayerTreeModelLegendNode *legendNode ) const;
    QVariant legendNodeData( QgsLayerTreeModelLegendNode *node, int role ) const;
    Qt::ItemFlags legendNodeFlags( QgsLayerTreeModelLegendNode *node ) const;
    bool legendEmbeddedInParent( QgsLayerTreeLayer *nodeLayer ) const;
    QIcon legendIconEmbeddedInParent( QgsLayerTreeLayer *nodeLayer ) const;
    void legendCleanup();
    void legendInvalidateMapBasedData();

  protected:











};

QFlags<QgsLayerTreeModel::Flag> operator|(QgsLayerTreeModel::Flag f1, QFlags<QgsLayerTreeModel::Flag> f2);




/************************************************************************
 * This file has been generated automatically from                      *
 *                                                                      *
 * src/core/layertree/qgslayertreemodel.h                               *
 *                                                                      *
 * Do not edit manually ! Edit header and run scripts/sipify.pl again   *
 ************************************************************************/
