/************************************************************************
 * This file has been generated automatically from                      *
 *                                                                      *
 * src/gui/layertree/qgslayertreeview.h                                 *
 *                                                                      *
 * Do not edit manually ! Edit header and run scripts/sipify.pl again   *
 ************************************************************************/




class QgsLayerTreeView : QTreeView
{
%Docstring
The QgsLayerTreeView class extends QTreeView and provides some additional functionality
when working with a layer tree.

The view updates expanded state of layer tree nodes and also listens to changes
to expanded states in the layer tree.

The view keeps track of the current layer and emits a signal when the current layer has changed.

Allows the client to specify a context menu provider with custom actions. Also it comes
with a set of default actions that can be used when building context menu.

.. seealso:: :py:class:`QgsLayerTreeModel`

.. versionadded:: 2.4
%End

%TypeHeaderCode
#include "qgslayertreeview.h"
%End
%ConvertToSubClassCode
    if ( sipCpp->inherits( "QgsLayerTreeView" ) )
      sipType = sipType_QgsLayerTreeView;
    else
      sipType = 0;
%End
  public:

    explicit QgsLayerTreeView( QWidget *parent /TransferThis/ = 0 );
%Docstring
Constructor for QgsLayerTreeView
%End
    ~QgsLayerTreeView();

    virtual void setModel( QAbstractItemModel *model );

%Docstring
Overridden setModel() from base class. Only QgsLayerTreeModel is an acceptable model.
%End

    QgsLayerTreeModel *layerTreeModel() const;
%Docstring
Gets access to the model casted to :py:class:`QgsLayerTreeModel`
%End

    QgsLayerTreeViewDefaultActions *defaultActions();
%Docstring
Gets access to the default actions that may be used with the tree view
%End

    void setMenuProvider( QgsLayerTreeViewMenuProvider *menuProvider /Transfer/ );
%Docstring
Sets provider for context menu. Takes ownership of the instance
%End
    QgsLayerTreeViewMenuProvider *menuProvider() const;
%Docstring
Returns pointer to the context menu provider. May be ``None``
%End

    QgsMapLayer *currentLayer() const;
%Docstring
Returns the currently selected layer, or ``None`` if no layers is selected.

.. seealso:: :py:func:`setCurrentLayer`
%End

    void setCurrentLayer( QgsMapLayer *layer );
%Docstring
Sets the currently selected ``layer``.

If ``layer`` is ``None`` then all layers will be deselected.

.. seealso:: :py:func:`currentLayer`
%End

    QgsLayerTreeNode *currentNode() const;
%Docstring
Gets current node. May be ``None``
%End
    QgsLayerTreeGroup *currentGroupNode() const;
%Docstring
Gets current group node. If a layer is current node, the function will return parent group. May be ``None``.
%End

    QgsLayerTreeModelLegendNode *currentLegendNode() const;
%Docstring
Gets current legend node. May be ``None`` if current node is not a legend node.

.. versionadded:: 2.14
%End

    QList<QgsLayerTreeNode *> selectedNodes( bool skipInternal = false ) const;
%Docstring
Returns list of selected nodes

:param skipInternal: If ``True``, will ignore nodes which have an ancestor in the selection
%End
    QList<QgsLayerTreeLayer *> selectedLayerNodes() const;
%Docstring
Returns list of selected nodes filtered to just layer nodes
%End

    QList<QgsMapLayer *> selectedLayers() const;
%Docstring
Gets list of selected layers
%End

    QList<QgsMapLayer *> selectedLayersRecursive() const;
%Docstring
Gets list of selected layers, including those that are not directly selected, but their
ancestor groups is selected. If we have a group with two layers L1, L2 and just the group
node is selected, this method returns L1 and L2, while selectedLayers() returns an empty list.

.. versionadded:: 3.4
%End

    void addIndicator( QgsLayerTreeNode *node, QgsLayerTreeViewIndicator *indicator );
%Docstring
Adds an indicator to the given layer tree node. Indicators are icons shown next to layer/group names
in the layer tree view. They can be used to show extra information with tree nodes and they allow
user interaction.

Does not take ownership of the indicator. One indicator object may be used for multiple layer tree nodes.

.. seealso:: :py:func:`removeIndicator`

.. seealso:: :py:func:`indicators`

.. versionadded:: 3.2
%End

    void removeIndicator( QgsLayerTreeNode *node, QgsLayerTreeViewIndicator *indicator );
%Docstring
Removes a previously added indicator to a layer tree node. Does not delete the indicator.

.. seealso:: :py:func:`addIndicator`

.. seealso:: :py:func:`indicators`

.. versionadded:: 3.2
%End

    QList<QgsLayerTreeViewIndicator *> indicators( QgsLayerTreeNode *node ) const;
%Docstring
Returns list of indicators associated with a particular layer tree node.

.. seealso:: :py:func:`addIndicator`

.. seealso:: :py:func:`removeIndicator`

.. versionadded:: 3.2
%End



  public slots:
    void refreshLayerSymbology( const QString &layerId );
%Docstring
Force refresh of layer symbology. Normally not needed as the changes of layer's renderer are monitored by the model
%End

    void expandAllNodes();
%Docstring
Enhancement of QTreeView.expandAll() that also records expanded state in layer tree nodes

.. versionadded:: 2.18
%End

    void collapseAllNodes();
%Docstring
Enhancement of QTreeView.collapseAll() that also records expanded state in layer tree nodes

.. versionadded:: 2.18
%End

  signals:
    void currentLayerChanged( QgsMapLayer *layer );
%Docstring
Emitted when a current layer is changed
%End

  protected:
    virtual void contextMenuEvent( QContextMenuEvent *event );


    void updateExpandedStateFromNode( QgsLayerTreeNode *node );

    QgsMapLayer *layerForIndex( const QModelIndex &index ) const;

    virtual void mouseReleaseEvent( QMouseEvent *event );

    virtual void keyPressEvent( QKeyEvent *event );


    virtual void dropEvent( QDropEvent *event );


  protected slots:

    void modelRowsInserted( const QModelIndex &index, int start, int end );
    void modelRowsRemoved();

    void updateExpandedStateToNode( const QModelIndex &index );

    void onCurrentChanged();
    void onExpandedChanged( QgsLayerTreeNode *node, bool expanded );
    void onModelReset();

  protected:

};


class QgsLayerTreeViewMenuProvider
{
%Docstring
Implementation of this interface can be implemented to allow QgsLayerTreeView
instance to provide custom context menus (opened upon right-click).

.. seealso:: :py:class:`QgsLayerTreeView`

.. versionadded:: 2.4
%End

%TypeHeaderCode
#include "qgslayertreeview.h"
%End
  public:
    virtual ~QgsLayerTreeViewMenuProvider();

    virtual QMenu *createContextMenu() = 0 /Factory/;
%Docstring
Returns a newly created menu instance (or ``None`` on error)
%End
};


/************************************************************************
 * This file has been generated automatically from                      *
 *                                                                      *
 * src/gui/layertree/qgslayertreeview.h                                 *
 *                                                                      *
 * Do not edit manually ! Edit header and run scripts/sipify.pl again   *
 ************************************************************************/
