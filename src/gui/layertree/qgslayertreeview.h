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

class GUI_EXPORT QgsLayerTreeView : public QTreeView
{
    Q_OBJECT
  public:
    explicit QgsLayerTreeView( QWidget *parent = 0 );
    ~QgsLayerTreeView();

    virtual void setModel( QAbstractItemModel* model );

    QgsLayerTreeModel* layerTreeModel() const;

    QgsLayerTreeViewDefaultActions* defaultActions();

    void setMenuProvider( QgsLayerTreeViewMenuProvider* menuProvider ); // takes ownership
    QgsLayerTreeViewMenuProvider* menuProvider() const { return mMenuProvider; }

    QgsMapLayer* currentLayer() const;
    void setCurrentLayer( QgsMapLayer* layer );

    QgsLayerTreeNode* currentNode() const;
    QgsLayerTreeGroup* currentGroupNode() const;

    QList<QgsLayerTreeNode*> selectedNodes( bool skipInternal = false ) const;
    QList<QgsLayerTreeLayer*> selectedLayerNodes() const;

    QList<QgsMapLayer*> selectedLayers() const;

  public slots:
    void refreshLayerSymbology( const QString& layerId );

  protected:
    void contextMenuEvent( QContextMenuEvent* event );

    void updateExpandedStateFromNode( QgsLayerTreeNode* node );

    QgsMapLayer* layerForIndex( const QModelIndex& index ) const;

  signals:
    void currentLayerChanged( QgsMapLayer* layer );


  protected slots:

    void modelRowsInserted( QModelIndex index, int start, int end );

    void updateExpandedStateToNode( QModelIndex index );

    void onCurrentChanged( QModelIndex current, QModelIndex previous );

  protected:
    QgsLayerTreeViewDefaultActions* mDefaultActions;
    QgsLayerTreeViewMenuProvider* mMenuProvider;
};

/** interface to allow custom menus */
class GUI_EXPORT QgsLayerTreeViewMenuProvider
{
  public:
    virtual ~QgsLayerTreeViewMenuProvider() {}

    virtual QMenu* createContextMenu() = 0;
};


#endif // QGSLAYERTREEVIEW_H
