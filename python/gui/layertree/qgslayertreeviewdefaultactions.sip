/************************************************************************
 * This file has been generated automatically from                      *
 *                                                                      *
 * src/gui/layertree/qgslayertreeviewdefaultactions.h                   *
 *                                                                      *
 * Do not edit manually ! Edit header and run scripts/sipify.pl again   *
 ************************************************************************/






class QgsLayerTreeViewDefaultActions : QObject
{
%Docstring
 The QgsLayerTreeViewDefaultActions class serves as a factory of actions
 that can be used together with a layer tree view.

.. seealso:: QgsLayerTreeView
.. versionadded:: 2.4
%End

%TypeHeaderCode
#include "qgslayertreeviewdefaultactions.h"
%End
  public:
    QgsLayerTreeViewDefaultActions( QgsLayerTreeView *view );

    QAction *actionAddGroup( QObject *parent = 0 ) /Factory/;
%Docstring
 :rtype: QAction
%End
    QAction *actionRemoveGroupOrLayer( QObject *parent = 0 ) /Factory/;
%Docstring
 :rtype: QAction
%End
    QAction *actionShowInOverview( QObject *parent = 0 ) /Factory/;
%Docstring
 :rtype: QAction
%End
    QAction *actionRenameGroupOrLayer( QObject *parent = 0 ) /Factory/;
%Docstring
 :rtype: QAction
%End
    QAction *actionShowFeatureCount( QObject *parent = 0 ) /Factory/;
%Docstring
 :rtype: QAction
%End

    QAction *actionCheckAndAllChildren( QObject *parent = 0 );
%Docstring
Action to check a group and all its children
 :rtype: QAction
%End

    QAction *actionUncheckAndAllChildren( QObject *parent = 0 );
%Docstring
Action to uncheck a group and all its children
 :rtype: QAction
%End

    QAction *actionCheckAndAllParents( QObject *parent = 0 );
%Docstring
Action to check a group and all its parents
 :rtype: QAction
%End

    QAction *actionZoomToLayer( QgsMapCanvas *canvas, QObject *parent = 0 ) /Factory/;
%Docstring
 :rtype: QAction
%End
    QAction *actionZoomToGroup( QgsMapCanvas *canvas, QObject *parent = 0 ) /Factory/;
%Docstring
 :rtype: QAction
%End

    QAction *actionMakeTopLevel( QObject *parent = 0 ) /Factory/;
%Docstring
 :rtype: QAction
%End
    QAction *actionGroupSelected( QObject *parent = 0 ) /Factory/;
%Docstring
 :rtype: QAction
%End

    QAction *actionMutuallyExclusiveGroup( QObject *parent = 0 ) /Factory/;
%Docstring
 Action to enable/disable mutually exclusive flag of a group (only one child node may be checked)
.. versionadded:: 2.12
 :rtype: QAction
%End

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

    void mutuallyExclusiveGroup();
%Docstring
 Slot to enable/disable mutually exclusive group flag
.. versionadded:: 2.12
%End

  protected:
    void zoomToLayers( QgsMapCanvas *canvas, const QList<QgsMapLayer *> &layers );

    QString uniqueGroupName( QgsLayerTreeGroup *parentGroup );
%Docstring
 :rtype: str
%End

  protected:
};


/************************************************************************
 * This file has been generated automatically from                      *
 *                                                                      *
 * src/gui/layertree/qgslayertreeviewdefaultactions.h                   *
 *                                                                      *
 * Do not edit manually ! Edit header and run scripts/sipify.pl again   *
 ************************************************************************/
