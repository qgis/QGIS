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

#include "qgis_core.h"
#include <QAbstractItemModel>
#include <QFont>
#include <QIcon>
#include <QTimer>
#include <QUuid>
#include <memory>

#include "qgsgeometry.h"
#include "qgslayertreemodellegendnode.h"

class QgsLayerTreeNode;
class QgsLayerTreeGroup;
class QgsLayerTreeLayer;
class QgsMapHitTest;
class QgsMapHitTestTask;
class QgsMapSettings;
class QgsExpression;
class QgsRenderContext;
class QgsLayerTree;
class QgsLayerTreeFilterSettings;

/**
 * \ingroup core
 * \brief The QgsLayerTreeModel class is model implementation for Qt item views framework.
 *
 * The model can be used in any QTreeView, it is however recommended to use it
 * with QgsLayerTreeView which brings additional functionality specific to layer tree handling.
 *
 * The model listens to the changes in the layer tree and signals the changes as appropriate,
 * so that any view that uses the model is updated accordingly.
 *
 * Behavior of the model can be customized with flags. For example, whether to show legend or
 * whether to allow changes to the layer tree.
 *
 * \see QgsLayerTreeView
 */
class CORE_EXPORT QgsLayerTreeModel : public QAbstractItemModel
{

#ifdef SIP_RUN
    SIP_CONVERT_TO_SUBCLASS_CODE
    if ( sipCpp->inherits( "QgsLayerTreeModel" ) )
      sipType = sipType_QgsLayerTreeModel;
    else
      sipType = 0;
    SIP_END
#endif

    Q_OBJECT
  public:

    /**
     * Construct a new tree model with given layer tree (root node must not be NULLPTR).
     * The root node is not transferred by the model.
     */
    explicit QgsLayerTreeModel( QgsLayerTree *rootNode, QObject *parent SIP_TRANSFERTHIS = nullptr );

    ~QgsLayerTreeModel() override;

    // Implementation of virtual functions from QAbstractItemModel

    int rowCount( const QModelIndex &parent = QModelIndex() ) const override;
    int columnCount( const QModelIndex &parent = QModelIndex() ) const override;
    QModelIndex index( int row, int column, const QModelIndex &parent = QModelIndex() ) const override;
    QModelIndex parent( const QModelIndex &child ) const override;
    QVariant data( const QModelIndex &index, int role = Qt::DisplayRole ) const override;
    Qt::ItemFlags flags( const QModelIndex &index ) const override;
    bool setData( const QModelIndex &index, const QVariant &value, int role = Qt::EditRole ) override;
    Qt::DropActions supportedDropActions() const override;
    QStringList mimeTypes() const override;
    QMimeData *mimeData( const QModelIndexList &indexes ) const override;
    bool dropMimeData( const QMimeData *data, Qt::DropAction action, int row, int column, const QModelIndex &parent ) override;
    bool removeRows( int row, int count, const QModelIndex &parent = QModelIndex() ) override;

    // New stuff

    enum Flag SIP_ENUM_BASETYPE( IntFlag )
    {
      // display flags
      ShowLegend                 = 0x0001,  //!< Add legend nodes for layer nodes
      ShowLegendAsTree           = 0x0004,  //!< For legends that support it, will show them in a tree instead of a list (needs also ShowLegend). Added in 2.8
      DeferredLegendInvalidation = 0x0008,  //!< Defer legend model invalidation
      UseEmbeddedWidgets         = 0x0010,  //!< Layer nodes may optionally include extra embedded widgets (if used in QgsLayerTreeView). Added in 2.16
      UseTextFormatting          = 0x0020,  //!< Layer nodes will alter text appearance based on layer properties, such as scale based visibility

      // behavioral flags
      AllowNodeReorder           = 0x1000,  //!< Allow reordering with drag'n'drop
      AllowNodeRename            = 0x2000,  //!< Allow renaming of groups and layers
      AllowNodeChangeVisibility  = 0x4000,  //!< Allow user to set node visibility with a checkbox
      AllowLegendChangeState     = 0x8000,  //!< Allow check boxes for legend nodes (if supported by layer's legend)
      ActionHierarchical         = 0x10000, //!< Check/uncheck action has consequences on children (or parents for leaf node)
      UseThreadedHitTest         = 0x20000, //!< Run legend hit tests in a background thread (since QGIS 3.30)
    };
    Q_DECLARE_FLAGS( Flags, Flag )

    //! Sets OR-ed combination of model flags
    void setFlags( QgsLayerTreeModel::Flags f );
    //! Enable or disable a model flag
    void setFlag( Flag f, bool on = true );
    //! Returns OR-ed combination of model flags
    Flags flags() const;
    //! Check whether a flag is enabled
    bool testFlag( Flag f ) const;

    /**
     * Returns layer tree node for given index. Returns root node for invalid index.
     * Returns NULLPTR if index does not refer to a layer tree node (e.g. it is a legend node)
     */
    QgsLayerTreeNode *index2node( const QModelIndex &index ) const;
    //! Returns index for a given node. If the node does not belong to the layer tree, the result is undefined
    QModelIndex node2index( QgsLayerTreeNode *node ) const;

    /**
     * Convert a \a list of indexes to a list of layer tree nodes.
     * Indices that do not represent layer tree nodes are skipped.
     * If \a skipInternal is TRUE, a node is included in the output list only if no parent node is in the list.
     */
    QList<QgsLayerTreeNode *> indexes2nodes( const QModelIndexList &list, bool skipInternal = false ) const;

    /**
     * Returns legend node for given index. Returns NULLPTR for invalid index
     */
    static QgsLayerTreeModelLegendNode *index2legendNode( const QModelIndex &index );

    /**
     * Returns index for a given legend node. If the legend node does not belong to the layer tree, the result is undefined.
     * If the legend node is belongs to the tree but it is filtered out, invalid model index is returned.
     */
    QModelIndex legendNode2index( QgsLayerTreeModelLegendNode *legendNode );

    /**
     * Returns filtered list of active legend nodes attached to a particular layer node
     * (by default it returns also legend node embedded in parent layer node (if any) unless skipNodeEmbeddedInParent is TRUE)
     * \note Parameter skipNodeEmbeddedInParent added in QGIS 2.18
     * \see layerOriginalLegendNodes()
     */
    QList<QgsLayerTreeModelLegendNode *> layerLegendNodes( QgsLayerTreeLayer *nodeLayer, bool skipNodeEmbeddedInParent = false );

    /**
     * Returns original (unfiltered) list of legend nodes attached to a particular layer node
     * \see layerLegendNodes()
     */
    QList<QgsLayerTreeModelLegendNode *> layerOriginalLegendNodes( QgsLayerTreeLayer *nodeLayer );

    /**
     * Returns legend node that may be embedded in parent (i.e. its icon will be used for layer's icon).
     */
    QgsLayerTreeModelLegendNode *legendNodeEmbeddedInParent( QgsLayerTreeLayer *nodeLayer ) const;

    /**
     * Searches through the layer tree to find a legend node with a matching layer ID
     * and rule key.
     * \param layerId map layer ID
     * \param ruleKey legend node rule key
     * \returns QgsLayerTreeModelLegendNode if found
     */
    QgsLayerTreeModelLegendNode *findLegendNode( const QString &layerId, const QString &ruleKey ) const;

    //! Returns pointer to the root node of the layer tree. Always a non NULLPTR value.
    QgsLayerTree *rootGroup() const;

    /**
     * Reset the model and use a new root group node
     */
    void setRootGroup( QgsLayerTree *newRootGroup );

    /**
     * Force a refresh of legend nodes of a layer node.
     * Not necessary to call when layer's renderer is changed as the model listens to these events.
     */
    void refreshLayerLegend( QgsLayerTreeLayer *nodeLayer );

    //! Gets index of the item marked as current. Item marked as current is underlined.
    QModelIndex currentIndex() const;
    //! Sets index of the current item. May be used by view. Item marked as current is underlined.
    void setCurrentIndex( const QModelIndex &currentIndex );

    //! Sets font for a particular type of layer tree node. nodeType should come from QgsLayerTreeNode::NodeType enumeration
    void setLayerTreeNodeFont( int nodeType, const QFont &font );
    //! Gets font for a particular type of layer tree node. nodeType should come from QgsLayerTreeNode::NodeType enumeration
    QFont layerTreeNodeFont( int nodeType ) const;

    //! Sets at what number of legend nodes the layer node should be collapsed. Setting -1 disables the auto-collapse (default).
    void setAutoCollapseLegendNodes( int nodeCount ) { mAutoCollapseLegendNodesCount = nodeCount; }
    //! Returns at what number of legend nodes the layer node should be collapsed. -1 means no auto-collapse (default).
    int autoCollapseLegendNodes() const { return mAutoCollapseLegendNodesCount; }

    /**
     * Force only display of legend nodes which are valid for a given \a scale.
     * The \a scale value indicates the scale denominator, e.g. 1000.0 for a 1:1000 map.
     * Setting \a scale <= 0 will disable the functionality.
     * \see legendFilterByScale()
     */
    void setLegendFilterByScale( double scale );

    /**
     * Returns the scale which restricts the legend nodes which are visible.
     * The  scale value indicates the scale denominator, e.g. 1000.0 for a 1:1000 map.
     * A scale <= 0 indicates that no scale filtering is being performed.
     * \see setLegendFilterByScale()
     */
    double legendFilterByScale() const { return mLegendFilterByScale; }

    /**
     * Force only display of legend nodes which are valid for given map settings.
     * Setting NULLPTR or invalid map settings will disable the functionality.
     * Ownership of map settings pointer does not change, a copy is made.
     * \deprecated QGIS 3.32. Use setFilterSettings() instead.
     */
    Q_DECL_DEPRECATED void setLegendFilterByMap( const QgsMapSettings *settings ) SIP_DEPRECATED;

    /**
     * Filter display of legend nodes for given map settings
     * \param settings Map settings. Setting NULLPTR or invalid settings will disable any filter. Ownership is not changed, a copy is made
     * \param useExtent Whether to use the extent of the map settings as a first spatial filter on legend nodes
     * \param polygon If not empty, this polygon will be used instead of the map extent to filter legend nodes
     * \param useExpressions Whether to use legend node filter expressions
     * \deprecated QGIS 3.32. Use setFilterSettings() instead.
     */
    Q_DECL_DEPRECATED void setLegendFilter( const QgsMapSettings *settings, bool useExtent = true, const QgsGeometry &polygon = QgsGeometry(), bool useExpressions = true ) SIP_DEPRECATED;

    /**
     * Returns the current map settings used for the current legend filter (or NULLPTR if none is enabled)
     */
    const QgsMapSettings *legendFilterMapSettings() const;

    /**
     * Sets the filter \a settings to use to filter legend nodes.
     *
     * Set to NULLPTR to disable legend filter.
     *
     * \see filterSettings()
     *
     * \since QGIS 3.32
     */
    void setFilterSettings( const QgsLayerTreeFilterSettings *settings = nullptr );

    /**
     * Returns the filter settings to use to filter legend nodes. May be NULLPTR.
     *
     * \see setFilterSettings()
     *
     * \since QGIS 3.32
     */
    const QgsLayerTreeFilterSettings *filterSettings() const;

    /**
     * Give the layer tree model hints about the currently associated map view
     * so that legend nodes that use map units can be scaled correctly
     */
    void setLegendMapViewData( double mapUnitsPerPixel, int dpi, double scale );

    /**
     * Gets hints about map view - to be used in legend nodes. Arguments that are not NULLPTR will receive values.
     * If there are no valid map view data (from previous call to setLegendMapViewData()), returned values are zeros.
     */
    void legendMapViewData( double *mapUnitsPerPixel SIP_OUT, int *dpi SIP_OUT, double *scale  SIP_OUT ) const;

    /**
     * Gets map of map layer style overrides (key: layer ID, value: style name) where a different style should be used instead of the current one
     */
    QMap<QString, QString> layerStyleOverrides() const;

    /**
     * Sets map of map layer style overrides (key: layer ID, value: style name) where a different style should be used instead of the current one
     */
    void setLayerStyleOverrides( const QMap<QString, QString> &overrides );

    /**
     * Adds additional target screen \a properties to use when generating icons for Qt::DecorationRole data.
     *
     * This allows icons to be generated at an icon device pixel ratio and DPI which
     * corresponds exactly to the view's screen properties in which this model is used.
     *
     * \since QGIS 3.32
     */
    void addTargetScreenProperties( const QgsScreenProperties &properties );

    /**
     * Returns the target screen properties to use when generating icons.
     *
     * This allows icons to be generated at an icon device pixel ratio and DPI which
     * corresponds exactly to the view's screen properties in which this model is used.
     *
     * \see addTargetScreenProperties()
     * \since QGIS 3.32
     */
    QSet< QgsScreenProperties > targetScreenProperties() const;

    /**
     * Scales an layer tree model icon size to compensate for display pixel density, making the icon
     * size hi-dpi friendly, whilst still resulting in pixel-perfect sizes for low-dpi
     * displays.
     *
     * \a standardSize should be set to a standard icon size, e.g. 16, 24, 48, etc.
     *
     * \since QGIS 3.6
     */
    static int scaleIconSize( int standardSize );

    /**
     * When a current hit test for visible legend items is in progress, calling this
     * method will block until that hit test is complete.
     *
     * \since QGIS 3.32
     */
    void waitForHitTestBlocking();

    /**
     * Returns TRUE if a hit test for visible legend items is currently in progress.
     *
     * \see hitTestStarted()
     * \see hitTestCompleted()
     *
     * \since QGIS 3.32
     */
    bool hitTestInProgress() const;

  signals:

    /**
     * Emits a message than can be displayed to the user in a GUI class
     * \since QGIS 3.14
     */
    void messageEmitted( const QString &message, Qgis::MessageLevel level = Qgis::MessageLevel::Info, int duration = 5 );

    /**
     * Emitted when a hit test for visible legend items starts.
     *
     * \see hitTestInProgress()
     * \see hitTestCompleted()
     *
     * \since QGIS 3.32
     */
    void hitTestStarted();

    /**
     * Emitted when a hit test for visible legend items completes.
     *
     * \see hitTestInProgress()
     * \see hitTestStarted()
     *
     * \since QGIS 3.32
     */
    void hitTestCompleted();

  protected slots:
    void nodeWillAddChildren( QgsLayerTreeNode *node, int indexFrom, int indexTo );
    void nodeAddedChildren( QgsLayerTreeNode *node, int indexFrom, int indexTo );
    void nodeWillRemoveChildren( QgsLayerTreeNode *node, int indexFrom, int indexTo );
    void nodeRemovedChildren();

    void nodeVisibilityChanged( QgsLayerTreeNode *node );

    /**
     * Updates model when node's name has changed
     */
    void nodeNameChanged( QgsLayerTreeNode *node, const QString &name );

    void nodeCustomPropertyChanged( QgsLayerTreeNode *node, const QString &key );

    void nodeLayerLoaded();
    void nodeLayerWillBeUnloaded();
    void layerLegendChanged();

    /**
     * Emitted when layer flags have changed.
     * \since QGIS 3.18
     */
    void layerFlagsChanged();

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

    //! emit dataChanged() for layer tree node items
    void recursivelyEmitDataChanged( const QModelIndex &index = QModelIndex() );

    /**
     * Updates layer data for scale dependent layers, should be called when map scale changes.
     * Emits dataChanged() for all scale dependent layers.
     */
    void refreshScaleBasedLayers( const QModelIndex &index = QModelIndex(), double previousScale = 0.0 );

    static QIcon iconGroup();

    //! Filter nodes from QgsMapLayerLegend according to the current filtering rules
    QList<QgsLayerTreeModelLegendNode *> filterLegendNodes( const QList<QgsLayerTreeModelLegendNode *> &nodes );

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

    /**
     * Returns a temporary render context.
     *
     * \note Note available in Python bindings.
     */
    QgsRenderContext *createTemporaryRenderContext() const SIP_SKIP;

    //! Pointer to the root node of the layer tree. Not owned by the model
    QgsLayerTree *mRootNode = nullptr;
    //! Sets of flags for the model
    Flags mFlags;
    //! Current index - will be underlined
    QPersistentModelIndex mCurrentIndex;
    //! Minimal number of nodes when legend should be automatically collapsed. -1 = disabled
    int mAutoCollapseLegendNodesCount = -1;

    /**
     * Structure that stores tree representation of map layer's legend.
     * This structure is used only when the following requirements are met:
     *
     * - tree legend representation is enabled in model (ShowLegendAsTree flag)
     * - some legend nodes have non-null parent rule key (accessible via data(ParentRuleKeyRole) method)
     *
     * The tree structure (parents and children of each node) is extracted by analyzing nodes' parent rules.
     * \note not available in Python bindings
     */
#ifndef SIP_RUN
    struct LayerLegendTree
    {
      //! Pointer to parent for each active node. Top-level nodes have NULLPTR parent. Pointers are not owned.
      QMap<QgsLayerTreeModelLegendNode *, QgsLayerTreeModelLegendNode *> parents;
      //! List of children for each active node. Top-level nodes are under NULLPTR key. Pointers are not owned.
      QMap<QgsLayerTreeModelLegendNode *, QList<QgsLayerTreeModelLegendNode *> > children;
    };
#endif

    /**
     * Structure that stores all data associated with one map layer
     * \note not available in Python bindings
     */
#ifndef SIP_RUN
    struct LayerLegendData
    {
      LayerLegendData() = default;

      /**
       * Active legend nodes. May have been filtered.
       * Owner of legend nodes is still originalNodes !
       */
      QList<QgsLayerTreeModelLegendNode *> activeNodes;

      /**
       * A legend node that is not displayed separately, its icon is instead
       * shown within the layer node's item.
       * May be NULLPTR. if non-null, node is owned by originalNodes !
       */
      QgsLayerTreeModelLegendNode *embeddedNodeInParent = nullptr;

      /**
       * Data structure for storage of legend nodes.
       * These are nodes as received from QgsMapLayerLegend
       */
      QList<QgsLayerTreeModelLegendNode *> originalNodes;
      //! Optional pointer to a tree structure - see LayerLegendTree for details
      LayerLegendTree *tree = nullptr;
    };
#endif

    //! \note not available in Python bindings
    LayerLegendTree *tryBuildLegendTree( const QList<QgsLayerTreeModelLegendNode *> &nodes ) SIP_SKIP;

    /**
     * Overrides of map layers' styles: key = layer ID, value = style XML.
     * This allows showing a legend that is different from the current style of layers
     */
    QMap<QString, QString> mLayerStyleOverrides;

    //! Per layer data about layer's legend nodes
    QHash<QgsLayerTreeLayer *, LayerLegendData> mLegend;

    /**
     * Keep track of layer nodes for which the legend
     * size needs to be recalculated
     */
    QSet<QgsLayerTreeLayer *> mInvalidatedNodes;

    QFont mFontLayer;
    QFont mFontGroup;

    //! scale denominator for filtering of legend nodes (<= 0 means no filtering)
    double mLegendFilterByScale = 0;

    QPointer< QgsMapHitTestTask > mHitTestTask;

    QMap<QString, QSet<QString>> mHitTestResults;

    std::unique_ptr< QgsLayerTreeFilterSettings > mFilterSettings;

    double mLegendMapViewMupp = 0;
    int mLegendMapViewDpi = 0;
    double mLegendMapViewScale = 0;
    QTimer mDeferLegendInvalidationTimer;

    QSet< QgsScreenProperties > mTargetScreenProperties;

  private slots:
    void legendNodeSizeChanged();
    void hitTestTaskCompleted();

  private:
    void handleHitTestResults();


};

Q_DECLARE_OPERATORS_FOR_FLAGS( QgsLayerTreeModel::Flags )

///@cond PRIVATE
#ifndef SIP_RUN

/**
 * In order to support embedded widgets in layer tree view, the model
 * generates one placeholder legend node for each embedded widget.
 * The placeholder will be replaced by an embedded widget in QgsLayerTreeView
 */
class EmbeddedWidgetLegendNode : public QgsLayerTreeModelLegendNode
{
    Q_OBJECT

  public:
    EmbeddedWidgetLegendNode( QgsLayerTreeLayer *nodeL )
      : QgsLayerTreeModelLegendNode( nodeL )
    {
      // we need a valid rule key to allow the model to build a tree out of legend nodes
      // if that's possible (if there is a node without a rule key, building of tree is canceled)
      mRuleKey = QStringLiteral( "embedded-widget-" ) + QUuid::createUuid().toString();
    }

    QVariant data( int role ) const override
    {
      if ( role == static_cast< int >( QgsLayerTreeModelLegendNode::CustomRole::RuleKey ) )
        return mRuleKey;
      else if ( role == static_cast< int >( QgsLayerTreeModelLegendNode::CustomRole::NodeType ) )
        return QgsLayerTreeModelLegendNode::EmbeddedWidget;
      return QVariant();
    }

  private:
    QString mRuleKey;
};
#endif

///@endcond

#endif // QGSLAYERTREEMODEL_H
