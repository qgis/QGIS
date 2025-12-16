/***************************************************************************
  qgslayertreeview.h
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

#ifndef QGSLAYERTREEVIEW_H
#define QGSLAYERTREEVIEW_H

#include "qgis.h"
#include "qgis_gui.h"

#include <QTreeView>

class QgsLayerTreeGroup;
class QgsLayerTreeLayer;
class QgsLayerTreeModel;
class QgsLayerTreeNode;
class QgsLayerTreeModelLegendNode;
class QgsLayerTreeViewDefaultActions;
class QgsLayerTreeViewIndicator;
class QgsLayerTreeViewMenuProvider;
class QgsMapLayer;
class QgsMessageBar;
class QgsLayerTreeFilterProxyModel;


#include <QSortFilterProxyModel>

/**
 * \ingroup gui
 *
 * \brief A proxy model for QgsLayerTreeModel, supporting
 * private layers and text filtering.
 *
 * \since QGIS 3.18
 */
class GUI_EXPORT QgsLayerTreeProxyModel : public QSortFilterProxyModel
{
#ifdef SIP_RUN
    SIP_CONVERT_TO_SUBCLASS_CODE
    if ( qobject_cast<QgsLayerTreeProxyModel *>( sipCpp ) != nullptr )
      sipType = sipType_QgsLayerTreeProxyModel;
    else
      sipType = nullptr;
    SIP_END
#endif

    Q_OBJECT

  public:
    /**
     * Constructs QgsLayerTreeProxyModel with source model \a treeModel and a \a parent
     */
    QgsLayerTreeProxyModel( QgsLayerTreeModel *treeModel, QObject *parent );

    /**
     * Sets filter to \a filterText.
     */
    void setFilterText( const QString &filterText = QString() );

    /**
     * Returns if private layers are shown.
     */
    bool showPrivateLayers() const;

    /**
     * Determines if private layers are shown.
     */
    void setShowPrivateLayers( bool showPrivate );

    /**
     * Returns if valid layers should be hidden (i.e. only invalid layers are shown).
     *
     * \see setHideValidLayers()
     * \since QGIS 3.38
     */
    bool hideValidLayers() const;

    /**
     * Sets whether valid layers should be hidden (i.e. only invalid layers are shown).
     *
     * \see setHideValidLayers()
     * \since QGIS 3.38
     */
    void setHideValidLayers( bool hideValid );

  protected:
    bool filterAcceptsRow( int sourceRow, const QModelIndex &sourceParent ) const override;

    /**
     * Returns TRUE if the specified \a node should be shown.
     *
     * \since QGIS 4.0
     */
    virtual bool nodeShown( QgsLayerTreeNode *node ) const;

  private:
    QgsLayerTreeModel *mLayerTreeModel = nullptr;
    QString mFilterText;
    bool mShowPrivateLayers = false;
    bool mHideValidLayers = false;
};


/**
 * \ingroup gui
 * \brief Base class for QTreeView widgets which display a layer tree.
 *
 * The view updates expanded state of layer tree nodes and also listens to changes
 * to expanded states in the layer tree.
 *
 * \warning Subclasses must take care to call both setLayerTreeModel() and QTreeView::setModel()
 * in order to have a fully functional tree view. This is by design, as it permits use of
 * a custom proxy model in the view.
 *
 * \see QgsLayerTreeView
 * \since QGIS 4.0
 */
class GUI_EXPORT QgsLayerTreeViewBase : public QTreeView
{
    Q_OBJECT

  public:
    //! Constructor for QgsLayerTreeViewBase
    explicit QgsLayerTreeViewBase( QWidget *parent SIP_TRANSFERTHIS = nullptr );
    ~QgsLayerTreeViewBase() override;

    void mouseDoubleClickEvent( QMouseEvent *event ) override;

    /**
     * Associates a layer tree model with the view.
     *
     * \warning This does NOT explicitly set the view's model, and a subsequent call
     * to QTreeView::setModel() must be made. This is by design, as it permits use of
     * a custom proxy model in the view.
     *
     * \see layerTreeModel()
     */
    void setLayerTreeModel( QgsLayerTreeModel *model );

    /**
     * Returns the associated layer tree model.
     * \see setLayerTreeModel()
     */
    QgsLayerTreeModel *layerTreeModel() const;

    /**
     * Returns the layer tree node for given view \a index.
     *
     * Returns root node for an invalid index.
     *
     * Returns NULLPTR if index does not refer to a layer tree node (e.g. it is a legend node).
     *
     * Unlike QgsLayerTreeViewBase::index2Node(), calling this method correctly accounts
     * for mapping the view indexes through the view's proxy model to the source model.
     *
     * \see node2index()
     * \since QGIS 3.18
     */
    QgsLayerTreeNode *index2node( const QModelIndex &index ) const;

    /**
     * Returns the view model index for a given \a node.
     *
     * If the \a node does not belong to the layer tree, the result is undefined.
     *
     * Unlike QgsLayerTreeModel::node2index(), calling this method correctly accounts
     * for mapping the view indexes through the view's proxy model to the source model.
     *
     * \see index2node()
     * \since QGIS 3.18
     */
    QModelIndex node2index( QgsLayerTreeNode *node ) const;

    /**
     * Returns the layer tree source model index for a given \a node.
     *
     * If the \a node does not belong to the layer tree, the result is undefined.
     *
     * \warning The returned index belongs the underlying layer tree model, and care should be taken
     * to correctly map to a proxy index if a proxy model is in use.
     *
     * \see node2index()
     * \since QGIS 3.18
     */
    QModelIndex node2sourceIndex( QgsLayerTreeNode *node ) const;

    /**
     * Returns legend node for given view \a index.
     *
     * Returns NULLPTR for invalid index.
     *
     * Unlike QgsLayerTreeModel::index2legendNode(), calling this method correctly accounts
     * for mapping the view indexes through the view's proxy model to the source model.
     *
     * \since QGIS 3.18
     */
    QgsLayerTreeModelLegendNode *index2legendNode( const QModelIndex &index ) const;

    /**
     * Returns the current node.
     *
     * May be NULLPTR.
     */
    QgsLayerTreeNode *currentNode() const;

    /**
     * Returns the list of selected layer tree nodes.
     *
     * \param skipInternal If TRUE, will ignore nodes which have an ancestor in the selection
     *
     * \see selectedLayerNodes()
     * \see selectedLegendNodes()
     * \see selectedLayers()
     */
    QList<QgsLayerTreeNode *> selectedNodes( bool skipInternal = false ) const;

    /**
     * Returns the currently selected layer, or NULLPTR if no layers is selected.
     *
     * \see setCurrentLayer()
     */
    QgsMapLayer *currentLayer() const;

    /**
     * Returns the map layer corresponding to a view \a index.
     *
     * This method correctly accounts for proxy models set on the tree view.
     *
     * Returns NULLPTR if the index does not correspond to a map layer.
     */
    QgsMapLayer *layerForIndex( const QModelIndex &index ) const;

    /**
     * Returns the current group node.
     *
     * If a layer is the current node, the function will return the layer's parent group.
     *
     * May be NULLPTR.
     */
    QgsLayerTreeGroup *currentGroupNode() const;

    /**
     * Returns the list of selected nodes filtered to just layer nodes (QgsLayerTreeLayer).
     *
     * \see selectedNodes()
     * \see selectedLayers()
     * \see selectedLegendNodes()
     */
    QList<QgsLayerTreeLayer *> selectedLayerNodes() const;

    /**
     * Returns the list of selected layers.
     *
     * \see selectedNodes()
     * \see selectedLayerNodes()
     * \see selectedLegendNodes()
     */
    QList<QgsMapLayer *> selectedLayers() const;

    /**
     * Returns the view index for a given legend node.
     *
     * If the legend node does not belong to the layer tree, the result is undefined.
     *
     * If the legend node belongs to the tree but it is filtered out, an invalid model index is returned.
     *
     * Unlike QgsLayerTreeModel::legendNode2index(), calling this method correctly accounts
     * for mapping the view indexes through the view's proxy model to the source model.
     *
     * \since QGIS 3.18
     */
    QModelIndex legendNode2index( QgsLayerTreeModelLegendNode *legendNode );

    /**
     * Returns the layer tree source model index for a given legend node.
     *
     * If the legend node does not belong to the layer tree, the result is undefined.
     *
     * If the legend node belongs to the tree but it is filtered out, an invalid model index is returned.
     *
     * \warning The returned index belongs the underlying layer tree model, and care should be taken
     * to correctly map to a proxy index if a proxy model is in use.
     *
     * \see legendNode2index()
     *
     * \since QGIS 3.18
     */
    QModelIndex legendNode2sourceIndex( QgsLayerTreeModelLegendNode *legendNode );

    /**
     * Sets the currently selected \a node.
     *
     * If \a node is NULLPTR then all nodes will be deselected.
     *
     * \see currentNode()
     * \since QGIS 3.40
     */
    void setCurrentNode( QgsLayerTreeNode *node );

    /**
     * Sets the currently selected \a layer.
     *
     * If \a layer is NULLPTR then all layers will be deselected.
     *
     * \see currentLayer()
     */
    void setCurrentLayer( QgsMapLayer *layer );

    /**
     * Gets current legend node. May be NULLPTR if current node is not a legend node.
     */
    QgsLayerTreeModelLegendNode *currentLegendNode() const;

    /**
     * Returns the list of selected legend nodes.
     *
     * \see selectedNodes()
     * \see selectedLayerNodes()
     *
     * \since QGIS 3.32
     */
    QList<QgsLayerTreeModelLegendNode *> selectedLegendNodes() const;

    /**
     * Gets list of selected layers, including those that are not directly selected, but their
     * ancestor groups is selected. If we have a group with two layers L1, L2 and just the group
     * node is selected, this method returns L1 and L2, while selectedLayers() returns an empty list.
     * \since QGIS 3.4
     */
    QList<QgsMapLayer *> selectedLayersRecursive() const;

    //! Gets access to the default actions that may be used with the tree view
    QgsLayerTreeViewDefaultActions *defaultActions();

  public slots:

    /**
     * Enhancement of QTreeView::expandAll() that also records expanded state in layer tree nodes
     */
    void expandAllNodes();

    /**
     * Enhancement of QTreeView::collapseAll() that also records expanded state in layer tree nodes
     */
    void collapseAllNodes();

  protected:
    /**
     * Updates the expanded state from a \a node.
     */
    void updateExpandedStateFromNode( QgsLayerTreeNode *node );

    /**
     * Returns the view index corresponding with a layer tree model \a index.
     *
     * This method correctly accounts for proxy models set on the tree view.
     *
     * \see layerTreeModelIndexToViewIndex()
     */
    QModelIndex viewIndexToLayerTreeModelIndex( const QModelIndex &index ) const;

    /**
     * Returns the layer tree model index corresponding with a view \a index.
     *
     * This method correctly accounts for proxy models set on the tree view.
     *
     * \see viewIndexToLayerTreeModelIndex()
     */
    QModelIndex layerTreeModelIndexToViewIndex( const QModelIndex &index ) const;

    //! helper class with default actions. Lazily initialized.
    QgsLayerTreeViewDefaultActions *mDefaultActions = nullptr;

  protected slots:

    /**
     * Stores the expanded state to a node with matching \a index.
     */
    void updateExpandedStateToNode( const QModelIndex &index );

    /**
     * Called when the expanded state changes for a node.
     */
    void onExpandedChanged( QgsLayerTreeNode *node, bool expanded );

    /**
     * Called when the model is reset.
     */
    void onModelReset();

  private slots:

    void onDataChanged( const QModelIndex &topLeft, const QModelIndex &bottomRight, const QVector<int> &roles );

  private:
    QgsLayerTreeModel *mLayerTreeModel = nullptr;
    QTimer *mBlockDoubleClickTimer = nullptr;
};


/**
 * \ingroup gui
 * \brief Extends QTreeView and provides additional functionality when working with a layer tree.
 *
 * The view updates expanded state of layer tree nodes and also listens to changes
 * to expanded states in the layer tree.
 *
 * The view keeps track of the current layer and emits a signal when the current layer has changed.
 *
 * Allows the client to specify a context menu provider with custom actions. Also it comes
 * with a set of default actions that can be used when building context menu.
 *
 * \see QgsLayerTreeModel
 */
class GUI_EXPORT QgsLayerTreeView : public QgsLayerTreeViewBase
{
#ifdef SIP_RUN
    SIP_CONVERT_TO_SUBCLASS_CODE
    if ( sipCpp->inherits( "QgsLayerTreeView" ) )
      sipType = sipType_QgsLayerTreeView;
    else
      sipType = 0;
    SIP_END
#endif


    Q_OBJECT
  public:
    //! Constructor for QgsLayerTreeView
    explicit QgsLayerTreeView( QWidget *parent SIP_TRANSFERTHIS = nullptr );
    ~QgsLayerTreeView() override;

    /**
     * Overridden setModel() from base class. Only QgsLayerTreeModel is an acceptable model.
     *
     * \note This method automatically creates a QgsLayerTreeProxyModel to use as a proxy.
     */
    void setModel( QAbstractItemModel *model ) override;

    /**
     * Sets the \a model and \a proxyModel for the view.
     *
     * Use this method when a custom proxy model is required.
     *
     * \since QGIS 4.0
     */
    void setModel( QgsLayerTreeModel *model, QgsLayerTreeProxyModel *proxyModel );

    /**
     * Returns the proxy model used by the view.
     *
     * This can be used to set filters controlling which layers are shown in the view.
     *
     * \since QGIS 3.18
     */
    QgsLayerTreeProxyModel *proxyModel() const;

    //! Sets provider for context menu. Takes ownership of the instance
    void setMenuProvider( QgsLayerTreeViewMenuProvider *menuProvider SIP_TRANSFER );
    //! Returns pointer to the context menu provider. May be NULLPTR
    QgsLayerTreeViewMenuProvider *menuProvider() const { return mMenuProvider; }

    /**
     * Convenience methods which sets the visible state of the specified map \a layer.
     *
     * \see QgsLayerTreeNode::setItemVisibilityChecked()
     * \since QGIS 3.10
     */
    void setLayerVisible( QgsMapLayer *layer, bool visible );

    /**
     * Adds an indicator to the given layer tree node. Indicators are icons shown next to layer/group names
     * in the layer tree view. They can be used to show extra information with tree nodes and they allow
     * user interaction.
     *
     * Does not take ownership of the indicator. One indicator object may be used for multiple layer tree nodes.
     * \see removeIndicator
     * \see indicators
     * \since QGIS 3.2
     */
    void addIndicator( QgsLayerTreeNode *node, QgsLayerTreeViewIndicator *indicator );

    /**
     * Removes a previously added indicator to a layer tree node. Does not delete the indicator.
     * \see addIndicator
     * \see indicators
     * \since QGIS 3.2
     */
    void removeIndicator( QgsLayerTreeNode *node, QgsLayerTreeViewIndicator *indicator );

    /**
     * Returns list of indicators associated with a particular layer tree node.
     * \see addIndicator
     * \see removeIndicator
     * \since QGIS 3.2
     */
    QList<QgsLayerTreeViewIndicator *> indicators( QgsLayerTreeNode *node ) const;

    /**
     * Returns width of contextual menu mark, at right of layer node items.
     * \see setLayerMarkWidth
     * \since QGIS 3.8
     */
    int layerMarkWidth() const { return mLayerMarkWidth; }

    ///@cond PRIVATE

    /**
     * Returns a list of custom property keys which are considered as related to view operations
     * only. E.g. node expanded state.
     *
     * Changes to these keys will not mark a project as "dirty" and trigger unsaved changes
     * warnings.
     *
     * \since QGIS 3.2
     */
    static QStringList viewOnlyCustomProperties() SIP_SKIP;

    ///@endcond

    /**
     * Returns the show private layers status
     * \since QGIS 3.18
     */
    bool showPrivateLayers() const;

    /**
     * Returns if valid layers should be hidden (i.e. only invalid layers are shown).
     *
     * \see setHideValidLayers()
     * \since QGIS 3.38
     */
    bool hideValidLayers() const;

  public slots:
    //! Force refresh of layer symbology. Normally not needed as the changes of layer's renderer are monitored by the model
    void refreshLayerSymbology( const QString &layerId );

    /**
     * Set width of contextual menu mark, at right of layer node items.
     * \see layerMarkWidth
     * \since QGIS 3.8
     */
    void setLayerMarkWidth( int width ) { mLayerMarkWidth = width; }

    /**
     * Set the message bar to display messages from the layer tree
     * \since QGIS 3.14
     */
    void setMessageBar( QgsMessageBar *messageBar );

    /**
     * Set the show private layers to \a showPrivate
     * \since QGIS 3.18
     */
    void setShowPrivateLayers( bool showPrivate );

    /**
     * Sets whether valid layers should be hidden (i.e. only invalid layers are shown).
     *
     * \see setHideValidLayers()
     * \since QGIS 3.38
     */
    void setHideValidLayers( bool hideValid );

  signals:
    //! Emitted when a current layer is changed
    void currentLayerChanged( QgsMapLayer *layer );

    //! Emitted when datasets are dropped onto the layer tree view
    void datasetsDropped( QDropEvent *event );

    /**
     * Emitted when the context menu is about to show.
     *
     * Allows customization of the menu.
     *
     * \since QGIS 3.32
     */
    void contextMenuAboutToShow( QMenu *menu );

  protected:
    void contextMenuEvent( QContextMenuEvent *event ) override;

    void mouseReleaseEvent( QMouseEvent *event ) override;
    void keyPressEvent( QKeyEvent *event ) override;

    void dragEnterEvent( QDragEnterEvent *event ) override;
    void dragMoveEvent( QDragMoveEvent *event ) override;
    void dropEvent( QDropEvent *event ) override;

    void resizeEvent( QResizeEvent *event ) override;

  protected slots:

    void modelRowsInserted( const QModelIndex &index, int start, int end );
    void modelRowsRemoved();

    void onCurrentChanged();

  private slots:
    void onCustomPropertyChanged( QgsLayerTreeNode *node, const QString &key );
    //! Handles updating the viewport to avoid flicker
    void onHorizontalScroll( int value );

    void onDataChanged( const QModelIndex &topLeft, const QModelIndex &bottomRight, const QVector<int> &roles );

  protected:
    //! Context menu provider. Owned by the view.
    QgsLayerTreeViewMenuProvider *mMenuProvider = nullptr;
    //! Keeps track of current layer ID (to check when to emit signal about change of current layer)
    QString mCurrentLayerID;
    //! Storage of indicators used with the tree view
    QHash<QgsLayerTreeNode *, QList<QgsLayerTreeViewIndicator *>> mIndicators;
    //! Used by the item delegate for identification of which indicator has been clicked
    QPoint mLastReleaseMousePos;

    //! Width of contextual menu mark for layer nodes
    int mLayerMarkWidth;

  private:
    QgsLayerTreeProxyModel *mProxyModel = nullptr;

    QgsMessageBar *mMessageBar = nullptr;

    bool mShowPrivateLayers = false;
    bool mHideValidLayers = false;

    // For model  debugging
    // void checkModel( );

    // friend so it can access viewOptions() method and mLastReleaseMousePos without making them public
    friend class QgsLayerTreeViewItemDelegate;
};


/**
 * \ingroup gui
 * \brief Implementation of this interface can be implemented to allow QgsLayerTreeView
 * instance to provide custom context menus (opened upon right-click).
 *
 * \see QgsLayerTreeView
 */
class GUI_EXPORT QgsLayerTreeViewMenuProvider
{
  public:
    virtual ~QgsLayerTreeViewMenuProvider() = default;

    //! Returns a newly created menu instance (or NULLPTR on error)
    virtual QMenu *createContextMenu() = 0 SIP_FACTORY;
};


#endif // QGSLAYERTREEVIEW_H
