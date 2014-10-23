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

class QgsLayerTreeGroup;
class QgsLayerTreeLayer;
class QgsLayerTreeModel;
class QgsLayerTreeNode;
class QgsLayerTreeViewDefaultActions;
class QgsLayerTreeViewMenuProvider;
class QgsMapLayer;

/**
 * The QgsLayerTreeView class extends QTreeView and provides some additional functionality
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
 * @see QgsLayerTreeModel
 * @note added in 2.4
 */
class GUI_EXPORT QgsLayerTreeView : public QTreeView
{
    Q_OBJECT
  public:
    explicit QgsLayerTreeView( QWidget *parent = 0 );
    ~QgsLayerTreeView();

    //! Overridden setModel() from base class. Only QgsLayerTreeModel is an acceptable model.
    virtual void setModel( QAbstractItemModel* model );

    //! Get access to the model casted to QgsLayerTreeModel
    QgsLayerTreeModel* layerTreeModel() const;

    //! Get access to the default actions that may be used with the tree view
    QgsLayerTreeViewDefaultActions* defaultActions();

    //! Set provider for context menu. Takes ownership of the instance
    void setMenuProvider( QgsLayerTreeViewMenuProvider* menuProvider );
    //! Return pointer to the context menu provider. May be null
    QgsLayerTreeViewMenuProvider* menuProvider() const { return mMenuProvider; }

    //! Get currently selected layer. May be null
    QgsMapLayer* currentLayer() const;
    //! Set currently selected layer. Null pointer will deselect any layer.
    void setCurrentLayer( QgsMapLayer* layer );

    //! Get current node. May be null
    QgsLayerTreeNode* currentNode() const;
    //! Get current group node. If a layer is current node, the function will return parent group. May be null.
    QgsLayerTreeGroup* currentGroupNode() const;

    //! Return list of selected nodes
    //! @arg skipInternal If true, will ignore nodes which have an ancestor in the selection
    QList<QgsLayerTreeNode*> selectedNodes( bool skipInternal = false ) const;
    //! Return list of selected nodes filtered to just layer nodes
    QList<QgsLayerTreeLayer*> selectedLayerNodes() const;

    //! Get list of selected layers
    QList<QgsMapLayer*> selectedLayers() const;

  public slots:
    //! Force refresh of layer symbology. Normally not needed as the changes of layer's renderer are monitored by the model
    void refreshLayerSymbology( const QString& layerId );

  signals:
    //! Emitted when a current layer is changed
    void currentLayerChanged( QgsMapLayer* layer );

  protected:
    void contextMenuEvent( QContextMenuEvent* event );

    void updateExpandedStateFromNode( QgsLayerTreeNode* node );

    QgsMapLayer* layerForIndex( const QModelIndex& index ) const;

  protected slots:

    void modelRowsInserted( QModelIndex index, int start, int end );
    void modelRowsRemoved();

    void updateExpandedStateToNode( QModelIndex index );

    void onCurrentChanged();
    void onExpandedChanged( QgsLayerTreeNode* node, bool expanded );
    void onModelReset();

  protected:
    //! helper class with default actions. Lazily initialized.
    QgsLayerTreeViewDefaultActions* mDefaultActions;
    //! Context menu provider. Owned by the view.
    QgsLayerTreeViewMenuProvider* mMenuProvider;
    //! Keeps track of current layer ID (to check when to emit signal about change of current layer)
    QString mCurrentLayerID;
};


/**
 * Implementation of this interface can be implemented to allow QgsLayerTreeView
 * instance to provide custom context menus (opened upon right-click).
 *
 * @see QgsLayerTreeView
 * @note added in 2.4
 */
class GUI_EXPORT QgsLayerTreeViewMenuProvider
{
  public:
    virtual ~QgsLayerTreeViewMenuProvider() {}

    //! Return a newly created menu instance (or null pointer on error)
    virtual QMenu* createContextMenu() = 0;
};


#endif // QGSLAYERTREEVIEW_H
