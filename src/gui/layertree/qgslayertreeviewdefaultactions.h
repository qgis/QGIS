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
#include "qgis.h"
#include "qgis_gui.h"

class QAction;

class QgsLayerTreeGroup;
class QgsLayerTreeView;
class QgsMapCanvas;
class QgsMapLayer;


/**
 * \ingroup gui
 * The QgsLayerTreeViewDefaultActions class serves as a factory of actions
 * that can be used together with a layer tree view.
 *
 * \see QgsLayerTreeView
 * \since QGIS 2.4
 */
class GUI_EXPORT QgsLayerTreeViewDefaultActions : public QObject
{
    Q_OBJECT
  public:
    QgsLayerTreeViewDefaultActions( QgsLayerTreeView *view );

    QAction *actionAddGroup( QObject *parent = nullptr ) SIP_FACTORY;
    QAction *actionRemoveGroupOrLayer( QObject *parent = nullptr ) SIP_FACTORY;
    QAction *actionShowInOverview( QObject *parent = nullptr ) SIP_FACTORY;
    QAction *actionRenameGroupOrLayer( QObject *parent = nullptr ) SIP_FACTORY;
    QAction *actionShowFeatureCount( QObject *parent = nullptr ) SIP_FACTORY;

    //! Action to check a group and all its children
    QAction *actionCheckAndAllChildren( QObject *parent = nullptr );

    //! Action to uncheck a group and all its children
    QAction *actionUncheckAndAllChildren( QObject *parent = nullptr );

    //! Action to check a group and all its parents
    QAction *actionCheckAndAllParents( QObject *parent = nullptr );

    QAction *actionZoomToLayer( QgsMapCanvas *canvas, QObject *parent = nullptr ) SIP_FACTORY;
    QAction *actionZoomToGroup( QgsMapCanvas *canvas, QObject *parent = nullptr ) SIP_FACTORY;
    // TODO: zoom to selected

    QAction *actionMakeTopLevel( QObject *parent = nullptr ) SIP_FACTORY;
    QAction *actionGroupSelected( QObject *parent = nullptr ) SIP_FACTORY;

    /**
     * Action to enable/disable mutually exclusive flag of a group (only one child node may be checked)
     * \since QGIS 2.12
     */
    QAction *actionMutuallyExclusiveGroup( QObject *parent = nullptr ) SIP_FACTORY;

    void zoomToLayer( QgsMapCanvas *canvas );
    void zoomToGroup( QgsMapCanvas *canvas );

  public slots:
    void showInOverview();
    void addGroup();

  protected slots:
    void removeGroupOrLayer();
    void renameGroupOrLayer();
    void showFeatureCount();
    void zoomToLayer();
    void zoomToGroup();
    void makeTopLevel();
    void groupSelected();

    /**
     * Slot to enable/disable mutually exclusive group flag
     * \since QGIS 2.12
     */
    void mutuallyExclusiveGroup();

  private slots:
    void checkAndAllChildren();
    void uncheckAndAllChildren();
    void checkAndAllParents();

  protected:
    void zoomToLayers( QgsMapCanvas *canvas, const QList<QgsMapLayer *> &layers );

    QString uniqueGroupName( QgsLayerTreeGroup *parentGroup );

  protected:
    QgsLayerTreeView *mView = nullptr;
};


#endif // QGSLAYERTREEVIEWDEFAULTACTIONS_H
