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

.. seealso:: QgsLayerTreeModel
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
    ~QgsLayerTreeView();

    virtual void setModel( QAbstractItemModel *model );
%Docstring
Overridden setModel() from base class. Only QgsLayerTreeModel is an acceptable model.
%End

    QgsLayerTreeModel *layerTreeModel() const;
%Docstring
Get access to the model casted to QgsLayerTreeModel
 :rtype: QgsLayerTreeModel
%End

    QgsLayerTreeViewDefaultActions *defaultActions();
%Docstring
Get access to the default actions that may be used with the tree view
 :rtype: QgsLayerTreeViewDefaultActions
%End

    void setMenuProvider( QgsLayerTreeViewMenuProvider *menuProvider /Transfer/ );
%Docstring
Set provider for context menu. Takes ownership of the instance
%End
    QgsLayerTreeViewMenuProvider *menuProvider() const;
%Docstring
Return pointer to the context menu provider. May be null
 :rtype: QgsLayerTreeViewMenuProvider
%End

    QgsMapLayer *currentLayer() const;
%Docstring
Get currently selected layer. May be null
 :rtype: QgsMapLayer
%End
    void setCurrentLayer( QgsMapLayer *layer );
%Docstring
Set currently selected layer. Null pointer will deselect any layer.
%End

    QgsLayerTreeNode *currentNode() const;
%Docstring
Get current node. May be null
 :rtype: QgsLayerTreeNode
%End
    QgsLayerTreeGroup *currentGroupNode() const;
%Docstring
Get current group node. If a layer is current node, the function will return parent group. May be null.
 :rtype: QgsLayerTreeGroup
%End

    QgsLayerTreeModelLegendNode *currentLegendNode() const;
%Docstring
 Get current legend node. May be null if current node is not a legend node.
.. versionadded:: 2.14
 :rtype: QgsLayerTreeModelLegendNode
%End

    QList<QgsLayerTreeNode *> selectedNodes( bool skipInternal = false ) const;
%Docstring
 Return list of selected nodes
 \param skipInternal If true, will ignore nodes which have an ancestor in the selection
 :rtype: list of QgsLayerTreeNode
%End
    QList<QgsLayerTreeLayer *> selectedLayerNodes() const;
%Docstring
Return list of selected nodes filtered to just layer nodes
 :rtype: list of QgsLayerTreeLayer
%End

    QList<QgsMapLayer *> selectedLayers() const;
%Docstring
Get list of selected layers
 :rtype: list of QgsMapLayer
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
%Docstring
 :rtype: QgsMapLayer
%End

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

.. seealso:: QgsLayerTreeView
.. versionadded:: 2.4
%End

%TypeHeaderCode
#include "qgslayertreeview.h"
%End
  public:
    virtual ~QgsLayerTreeViewMenuProvider();

    virtual QMenu *createContextMenu() = 0 /Factory/;
%Docstring
Return a newly created menu instance (or null pointer on error)
 :rtype: QMenu
%End
};


/************************************************************************
 * This file has been generated automatically from                      *
 *                                                                      *
 * src/gui/layertree/qgslayertreeview.h                                 *
 *                                                                      *
 * Do not edit manually ! Edit header and run scripts/sipify.pl again   *
 ************************************************************************/
