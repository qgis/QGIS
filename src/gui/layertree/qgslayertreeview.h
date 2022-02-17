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

#include <QTreeView>
#include "qgis.h"
#include "qgis_gui.h"

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
 * \brief The QgsLayerTreeProxyModel class is a proxy model for QgsLayerTreeModel, supports
 * private layers and text filtering.
 *
 * \since QGIS 3.18
 */
class GUI_EXPORT QgsLayerTreeProxyModel : public QSortFilterProxyModel
{
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
     * Sets a predefined list of layer Ids to process.
     * \since QGIS 3.20
     */
    void setApprovedIds( const QStringList &ids );

    /**
     * Returns if private layers are shown.
     */
    bool showPrivateLayers() const;

    /**
     * Determines if private layers are shown.
     */
    void setShowPrivateLayers( bool showPrivate );

    /**
     * Allow non-spatial layers and empty groups to be show.
     * \param show If TRUE (default behavior), non-spatial layers and groups will be shown.
     * \since QGIS 3.20
     */
    void setShowAllNodes( bool show );

  protected:

    bool filterAcceptsRow( int sourceRow, const QModelIndex &sourceParent ) const override;

  private:

    bool nodeShown( QgsLayerTreeNode *node ) const;

    QgsLayerTreeModel *mLayerTreeModel = nullptr;
    QString mFilterText;
    QStringList mDesiredIds;
    bool mShowPrivateLayers = false;
    bool mShowAllNodes = true;


};


/**
 * \ingroup gui
 * \brief The QgsLayerTreeView class extends QTreeView and provides some additional functionality
 * when working with a layer tree.
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
 * \since QGIS 2.4
 */
class GUI_EXPORT QgsLayerTreeView : public QTreeView
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
     * Overridden setModel() from base class.
     * \param model Model used to populate the view. Only QgsLayerTreeModel models are accepted.
     */
    void setModel( QAbstractItemModel *model ) override;

    //! Gets access to the model casted to QgsLayerTreeModel
    QgsLayerTreeModel *layerTreeModel() const;

    /**
     * Returns the proxy model used by the view.
     *
     * This can be used to set filters controlling which layers are shown in the view.
     *
     * \since QGIS 3.18
     */
    QgsLayerTreeProxyModel *proxyModel() const;

    /**
     * Disconnects the Proxy Model to prevent crash when using a second view on the same model.
     * \since QGIS3.24
     */
    void disconnectProxyModel();

    /**
     * Returns layer tree node for given proxy model tree \a index. Returns root node for invalid index.
     * Returns NULLPTR if index does not refer to a layer tree node (e.g. it is a legend node)
     *
     * Unlike QgsLayerTreeModel::index2Node(), calling this method correctly accounts
     * for mapping the view indexes through the view's proxy model to the source model.
     *
     * \since QGIS 3.18
     */
    QgsLayerTreeNode *index2node( const QModelIndex &index ) const;

    /**
     * Returns proxy model index for a given node. If the node does not belong to the layer tree, the result is undefined
     *
     * Unlike QgsLayerTreeModel::node2index(), calling this method correctly accounts
     * for mapping the view indexes through the view's proxy model to the source model.
     *
     * \since QGIS 3.18
     */
    QModelIndex node2index( QgsLayerTreeNode *node ) const;


    /**
     * Returns source model index for a given node. If the node does not belong to the layer tree, the result is undefined
     *
     * \since QGIS 3.18
     */
    QModelIndex node2sourceIndex( QgsLayerTreeNode *node ) const;


    /**
     * Returns legend node for given proxy model tree \a index. Returns NULLPTR for invalid index
     *
     * Unlike QgsLayerTreeModel::index2legendNode(), calling this method correctly accounts
     * for mapping the view indexes through the view's proxy model to the source model.
     *
     * \since QGIS 3.18
     */
    QgsLayerTreeModelLegendNode *index2legendNode( const QModelIndex &index ) const;

    /**
     * Returns proxy model index for a given legend node. If the legend node does not belong to the layer tree, the result is undefined.
     * If the legend node is belongs to the tree but it is filtered out, invalid model index is returned.
     *
     * Unlike QgsLayerTreeModel::legendNode2index(), calling this method correctly accounts
     * for mapping the view indexes through the view's proxy model to the source model.
     *
     * \since QGIS 3.18
     */
    QModelIndex legendNode2index( QgsLayerTreeModelLegendNode *legendNode );

    /**
     * Returns index for a given legend node. If the legend node does not belong to the layer tree, the result is undefined.
     * If the legend node is belongs to the tree but it is filtered out, invalid model index is returned.
     *
     * \since QGIS 3.18
     */
    QModelIndex legendNode2sourceIndex( QgsLayerTreeModelLegendNode *legendNode );

    //! Gets access to the default actions that may be used with the tree view
    QgsLayerTreeViewDefaultActions *defaultActions();

    //! Sets provider for context menu. Takes ownership of the instance
    void setMenuProvider( QgsLayerTreeViewMenuProvider *menuProvider SIP_TRANSFER );
    //! Returns pointer to the context menu provider. May be NULLPTR
    QgsLayerTreeViewMenuProvider *menuProvider() const { return mMenuProvider; }

    /**
     * Returns the currently selected layer, or NULLPTR if no layers is selected.
     *
     * \see setCurrentLayer()
     */
    QgsMapLayer *currentLayer() const;

    /**
     * Convenience methods which sets the visible state of the specified map \a layer.
     *
     * \see QgsLayerTreeNode::setItemVisibilityChecked()
     * \since QGIS 3.10
     */
    void setLayerVisible( QgsMapLayer *layer, bool visible );

    /**
     * Sets the currently selected \a layer.
     *
     * If \a layer is NULLPTR then all layers will be deselected.
     *
     * \see currentLayer()
     */
    void setCurrentLayer( QgsMapLayer *layer );

    //! Gets current node. May be NULLPTR
    QgsLayerTreeNode *currentNode() const;
    //! Gets current group node. If a layer is current node, the function will return parent group. May be NULLPTR.
    QgsLayerTreeGroup *currentGroupNode() const;

    /**
     * Gets current legend node. May be NULLPTR if current node is not a legend node.
     * \since QGIS 2.14
     */
    QgsLayerTreeModelLegendNode *currentLegendNode() const;

    /**
     * Returns list of selected nodes
     * \param skipInternal If TRUE, will ignore nodes which have an ancestor in the selection
     */
    QList<QgsLayerTreeNode *> selectedNodes( bool skipInternal = false ) const;
    //! Returns list of selected nodes filtered to just layer nodes
    QList<QgsLayerTreeLayer *> selectedLayerNodes() const;

    //! Gets list of selected layers
    QList<QgsMapLayer *> selectedLayers() const;

    /**
     * Gets list of selected layers, including those that are not directly selected, but their
     * ancestor groups is selected. If we have a group with two layers L1, L2 and just the group
     * node is selected, this method returns L1 and L2, while selectedLayers() returns an empty list.
     * \since QGIS 3.4
     */
    QList<QgsMapLayer *> selectedLayersRecursive() const;

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

    /**
     * Allow non-spatial layers and empty groups to be show.
     * \param show If TRUE (default behavior), non-spatial layers and groups will be shown.
     * \since QGIS 3.20
     */
    void showAllNodes( bool show );

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

  public slots:
    //! Force refresh of layer symbology. Normally not needed as the changes of layer's renderer are monitored by the model
    void refreshLayerSymbology( const QString &layerId );

    /**
     * Enhancement of QTreeView::expandAll() that also records expanded state in layer tree nodes
     * \since QGIS 2.18
     */
    void expandAllNodes();

    /**
     * Enhancement of QTreeView::collapseAll() that also records expanded state in layer tree nodes
     * \since QGIS 2.18
     */
    void collapseAllNodes();

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
     * Returns the show private layers status
     * \since QGIS 3.18
     */
    bool showPrivateLayers( );

  signals:
    //! Emitted when a current layer is changed
    void currentLayerChanged( QgsMapLayer *layer );

  protected:
    void contextMenuEvent( QContextMenuEvent *event ) override;

    void updateExpandedStateFromNode( QgsLayerTreeNode *node );

    QgsMapLayer *layerForIndex( const QModelIndex &index ) const;

    void mouseReleaseEvent( QMouseEvent *event ) override;
    void keyPressEvent( QKeyEvent *event ) override;

    void dropEvent( QDropEvent *event ) override;

    void resizeEvent( QResizeEvent *event ) override;

  protected slots:

    void modelRowsInserted( const QModelIndex &index, int start, int end );
    void modelRowsRemoved();

    void updateExpandedStateToNode( const QModelIndex &index );

    void onCurrentChanged();
    void onExpandedChanged( QgsLayerTreeNode *node, bool expanded );
    void onModelReset();

  private slots:
    void onCustomPropertyChanged( QgsLayerTreeNode *node, const QString &key );
    //! Handles updating the viewport to avoid flicker
    void onHorizontalScroll( int value );

    void onDataChanged( const QModelIndex &topLeft, const QModelIndex &bottomRight, const QVector<int> &roles );

  protected:
    //! helper class with default actions. Lazily initialized.
    QgsLayerTreeViewDefaultActions *mDefaultActions = nullptr;
    //! Context menu provider. Owned by the view.
    QgsLayerTreeViewMenuProvider *mMenuProvider = nullptr;
    //! Keeps track of current layer ID (to check when to emit signal about change of current layer)
    QString mCurrentLayerID;
    //! Storage of indicators used with the tree view
    QHash< QgsLayerTreeNode *, QList<QgsLayerTreeViewIndicator *> > mIndicators;
    //! Used by the item delegate for identification of which indicator has been clicked
    QPoint mLastReleaseMousePos;

    //! Width of contextual menu mark for layer nodes
    int mLayerMarkWidth;

  private:
    QgsLayerTreeProxyModel *mProxyModel = nullptr;

    QgsMessageBar *mMessageBar = nullptr;

    bool mShowPrivateLayers = false;

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
 * \since QGIS 2.4
 */
class GUI_EXPORT QgsLayerTreeViewMenuProvider
{
  public:
    virtual ~QgsLayerTreeViewMenuProvider() = default;

    //! Returns a newly created menu instance (or NULLPTR on error)
    virtual QMenu *createContextMenu() = 0 SIP_FACTORY;
};


#endif // QGSLAYERTREEVIEW_H
