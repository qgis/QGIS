/***************************************************************************
  qgslayertreeviewdefaultactions.h
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

#ifndef QGSLAYERTREEVIEWDEFAULTACTIONS_H
#define QGSLAYERTREEVIEWDEFAULTACTIONS_H

#include <QObject>

class QAction;

class QgsLayerTreeGroup;
class QgsLayerTreeView;
class QgsMapCanvas;
class QgsMapLayer;


/**
 * The QgsLayerTreeViewDefaultActions class serves as a factory of actions
 * that can be used together with a layer tree view.
 *
 * @see QgsLayerTreeView
 * @note added in 2.4
 */
class GUI_EXPORT QgsLayerTreeViewDefaultActions : public QObject
{
    Q_OBJECT
  public:
    QgsLayerTreeViewDefaultActions( QgsLayerTreeView* view );

    QAction* actionAddGroup( QObject* parent = 0 );
    QAction* actionRemoveGroupOrLayer( QObject* parent = 0 );
    QAction* actionShowInOverview( QObject* parent = 0 );
    QAction* actionRenameGroupOrLayer( QObject* parent = 0 );
    QAction* actionShowFeatureCount( QObject* parent = 0 );

    QAction* actionZoomToLayer( QgsMapCanvas* canvas, QObject* parent = 0 );
    QAction* actionZoomToGroup( QgsMapCanvas* canvas, QObject* parent = 0 );
    // TODO: zoom to selected

    QAction* actionMakeTopLevel( QObject* parent = 0 );
    QAction* actionGroupSelected( QObject* parent = 0 );

    void zoomToLayer( QgsMapCanvas* canvas );
    void zoomToGroup( QgsMapCanvas* canvas );

  public slots:
    void showInOverview();

  protected slots:
    void addGroup();
    void removeGroupOrLayer();
    void renameGroupOrLayer();
    void showFeatureCount();
    void zoomToLayer();
    void zoomToGroup();
    void makeTopLevel();
    void groupSelected();

  protected:
    void zoomToLayers( QgsMapCanvas* canvas, const QList<QgsMapLayer*>& layers );

    QString uniqueGroupName( QgsLayerTreeGroup* parentGroup );

  protected:
    QgsLayerTreeView* mView;
};


#endif // QGSLAYERTREEVIEWDEFAULTACTIONS_H
