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

    /**
     * \deprecated since QGIS 3.18, use actionZoomToLayers()
     */
    Q_DECL_DEPRECATED QAction *actionZoomToLayer( QgsMapCanvas *canvas, QObject *parent = nullptr ) SIP_FACTORY;

    QAction *actionZoomToLayers( QgsMapCanvas *canvas, QObject *parent = nullptr ) SIP_FACTORY;

    /**
     * Action to zoom to selected features of a vector layer
     * \since QGIS 3.2
     */
    QAction *actionZoomToSelection( QgsMapCanvas *canvas, QObject *parent = nullptr ) SIP_FACTORY;
    QAction *actionZoomToGroup( QgsMapCanvas *canvas, QObject *parent = nullptr ) SIP_FACTORY;

    /**
     * \deprecated since QGIS 3.2, use actionMoveOutOfGroup()
     */
    Q_DECL_DEPRECATED QAction *actionMakeTopLevel( QObject *parent = nullptr ) SIP_FACTORY;

    /**
     * \see moveOutOfGroup()
     * \since QGIS 3.2
     */
    QAction *actionMoveOutOfGroup( QObject *parent = nullptr ) SIP_FACTORY;

    /**
     * \see moveToTop()
     * \since QGIS 3.2
     */
    QAction *actionMoveToTop( QObject *parent = nullptr ) SIP_FACTORY;

    /**
     * \see moveToBottom()
     * \since QGIS 3.14
     */
    QAction *actionMoveToBottom( QObject *parent = nullptr ) SIP_FACTORY;
    QAction *actionGroupSelected( QObject *parent = nullptr ) SIP_FACTORY;

    /**
     * Action to enable/disable mutually exclusive flag of a group (only one child node may be checked)
     * \since QGIS 2.12
     */
    QAction *actionMutuallyExclusiveGroup( QObject *parent = nullptr ) SIP_FACTORY;

    /**
    * \deprecated since QGIS 3.18, use zoomToLayers()
    */
    Q_DECL_DEPRECATED void zoomToLayer( QgsMapCanvas *canvas );

    void zoomToLayers( QgsMapCanvas *canvas );

    /**
     * \see zoomToSelection()
     * \since QGIS 3.2
     */
    void zoomToSelection( QgsMapCanvas *canvas );
    void zoomToGroup( QgsMapCanvas *canvas );

  public slots:
    void showInOverview();
    void addGroup();

  protected slots:
    void removeGroupOrLayer();
    void renameGroupOrLayer();
    void showFeatureCount();

    /**
     * \deprecated since QGIS 3.18, use zoomToLayers()
     */
    Q_DECL_DEPRECATED void zoomToLayer();

    void zoomToLayers();

    /**
     * Slot to zoom to selected features of a vector layer
     * \since QGIS 3.2
     */
    void zoomToSelection();
    void zoomToGroup();

    /**
     * \deprecated since QGIS 3.2, use moveOutOfGroup()
     */
    Q_DECL_DEPRECATED void makeTopLevel();

    /**
     * Moves selected layer(s) out of the group(s) and places this/these above the group(s)
     * \since QGIS 3.2
     */
    void moveOutOfGroup();

    /**
     * Moves selected layer(s) and/or group(s) to the top of the layer panel
     * or the top of the group if the layer/group is placed within a group.
     * \since QGIS 3.2
     */
    void moveToTop();

    /**
     * Moves selected layer(s) and/or group(s) to the bottom of the layer panel
     * or the bottom of the group if the layer/group is placed within a group.
     * \since QGIS 3.14
     */
    void moveToBottom();
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
