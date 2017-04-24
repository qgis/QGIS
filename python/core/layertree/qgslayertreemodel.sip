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

 \see QgsLayerTreeView
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
The root node is not transferred by the model.
%End

    ~QgsLayerTreeModel();


    virtual int rowCount( const QModelIndex &parent = QModelIndex() ) const;

%Docstring
Implementation of virtual functions from QAbstractItemModel
 :rtype: int
%End
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
      ShowRasterPreviewIcon,
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
 :rtype: Flags
%End
    bool testFlag( Flag f ) const;
%Docstring
Check whether a flag is enabled
 :rtype: bool
%End

    QgsLayerTreeNode *index2node( const QModelIndex &index ) const;
%Docstring
Returns null pointer if index does not refer to a layer tree node (e.g. it is a legend node)
 :rtype: QgsLayerTreeNode
%End
    QModelIndex node2index( QgsLayerTreeNode *node ) const;
%Docstring
Return index for a given node. If the node does not belong to the layer tree, the result is undefined
 :rtype: QModelIndex
%End
    QList<QgsLayerTreeNode *> indexes2nodes( const QModelIndexList &list, bool skipInternal = false ) const;
%Docstring
@arg skipInternal If true, a node is included in the output list only if no parent node is in the list
 :rtype: list of QgsLayerTreeNode
%End

    static QgsLayerTreeModelLegendNode *index2legendNode( const QModelIndex &index );
%Docstring
.. versionadded:: 2.6
 :rtype: QgsLayerTreeModelLegendNode
%End
    QModelIndex legendNode2index( QgsLayerTreeModelLegendNode *legendNode );
%Docstring
.. versionadded:: 2.6
 :rtype: QModelIndex
%End

    QList<QgsLayerTreeModelLegendNode *> layerLegendNodes( QgsLayerTreeLayer *nodeLayer, bool skipNodeEmbeddedInParent = false );
%Docstring
\see layerOriginalLegendNodes()
 :rtype: list of QgsLayerTreeModelLegendNode
%End

    QList<QgsLayerTreeModelLegendNode *> layerOriginalLegendNodes( QgsLayerTreeLayer *nodeLayer );
%Docstring
\see layerLegendNodes()
 :rtype: list of QgsLayerTreeModelLegendNode
%End

    QgsLayerTreeModelLegendNode *legendNodeEmbeddedInParent( QgsLayerTreeLayer *nodeLayer ) const;
%Docstring
.. versionadded:: 2.18
 :rtype: QgsLayerTreeModelLegendNode
%End

    QgsLayerTreeModelLegendNode *findLegendNode( const QString &layerId, const QString &ruleKey ) const;
%Docstring
 Searches through the layer tree to find a legend node with a matching layer ID
 and rule key.
 \param layerId map layer ID
 \param ruleKey legend node rule key
 :return: QgsLayerTreeModelLegendNode if found
.. versionadded:: 2.14
 :rtype: QgsLayerTreeModelLegendNode
%End

    QgsLayerTree *rootGroup() const;
%Docstring
Return pointer to the root node of the layer tree. Always a non-null pointer.
 :rtype: QgsLayerTree
%End
    void setRootGroup( QgsLayerTree *newRootGroup );
%Docstring
.. versionadded:: 2.6
%End

    void refreshLayerLegend( QgsLayerTreeLayer *nodeLayer );
%Docstring
Not necessary to call when layer's renderer is changed as the model listens to these events.
%End

    QModelIndex currentIndex() const;
%Docstring
Get index of the item marked as current. Item marked as current is underlined.
 :rtype: QModelIndex
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
 :rtype: QFont
%End

    void setAutoCollapseLegendNodes( int nodeCount );
%Docstring
Set at what number of legend nodes the layer node should be collapsed. Setting -1 disables the auto-collapse (default).
%End
    int autoCollapseLegendNodes() const;
%Docstring
Return at what number of legend nodes the layer node should be collapsed. -1 means no auto-collapse (default).
 :rtype: int
%End

    void setLegendFilterByScale( double scaleDenominator );
%Docstring
.. versionadded:: 2.6
%End
    double legendFilterByScale() const;
%Docstring
 :rtype: float
%End

    void setLegendFilterByMap( const QgsMapSettings *settings );
%Docstring
.. versionadded:: 2.6
%End

    void setLegendFilter( const QgsMapSettings *settings, bool useExtent = true, const QgsGeometry &polygon = QgsGeometry(), bool useExpressions = true );
%Docstring
.. versionadded:: 2.14
%End

    const QgsMapSettings *legendFilterMapSettings() const;
%Docstring
.. versionadded:: 2.14
 :rtype: QgsMapSettings
%End

    void setLegendMapViewData( double mapUnitsPerPixel, int dpi, double scale );
%Docstring
.. versionadded:: 2.6
%End
    void legendMapViewData( double *mapUnitsPerPixel /Out/, int *dpi /Out/, double *scale  /Out/ ) const;
%Docstring
.. versionadded:: 2.6
%End

    QMap<QString, QString> layerStyleOverrides() const;
%Docstring
.. versionadded:: 2.10
 :rtype: QMap<str, QString>
%End
    void setLayerStyleOverrides( const QMap<QString, QString> &overrides );
%Docstring
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
%Docstring
 :rtype: QIcon
%End

    QList<QgsLayerTreeModelLegendNode *> filterLegendNodes( const QList<QgsLayerTreeModelLegendNode *> &nodes );
%Docstring
Filter nodes from QgsMapLayerLegend according to the current filtering rules
 :rtype: list of QgsLayerTreeModelLegendNode
%End

    QModelIndex indexOfParentLayerTreeNode( QgsLayerTreeNode *parentNode ) const;
%Docstring
 :rtype: QModelIndex
%End

    int legendRootRowCount( QgsLayerTreeLayer *nL ) const;
%Docstring
 :rtype: int
%End
    int legendNodeRowCount( QgsLayerTreeModelLegendNode *node ) const;
%Docstring
 :rtype: int
%End
    QModelIndex legendRootIndex( int row, int column, QgsLayerTreeLayer *nL ) const;
%Docstring
 :rtype: QModelIndex
%End
    QModelIndex legendNodeIndex( int row, int column, QgsLayerTreeModelLegendNode *node ) const;
%Docstring
 :rtype: QModelIndex
%End
    QModelIndex legendParent( QgsLayerTreeModelLegendNode *legendNode ) const;
%Docstring
 :rtype: QModelIndex
%End
    QVariant legendNodeData( QgsLayerTreeModelLegendNode *node, int role ) const;
%Docstring
 :rtype: QVariant
%End
    Qt::ItemFlags legendNodeFlags( QgsLayerTreeModelLegendNode *node ) const;
%Docstring
 :rtype: Qt.ItemFlags
%End
    bool legendEmbeddedInParent( QgsLayerTreeLayer *nodeLayer ) const;
%Docstring
 :rtype: bool
%End
    QIcon legendIconEmbeddedInParent( QgsLayerTreeLayer *nodeLayer ) const;
%Docstring
 :rtype: QIcon
%End
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
