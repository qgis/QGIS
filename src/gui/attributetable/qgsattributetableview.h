/***************************************************************************
     QgsAttributeTableView.h
     --------------------------------------
    Date                 : Feb 2009
    Copyright            : (C) 2009 Vita Cizek
    Email                : weetya (at) gmail.com
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef QGSATTRIBUTETABLEVIEW_H
#define QGSATTRIBUTETABLEVIEW_H

#include <QTableView>
#include <QAction>

#include "qgsfeature.h" // For QgsFeatureIds

class QgsAttributeTableDelegate;
class QgsAttributeTableFilterModel;
class QgsAttributeTableModel;
class QgsFeatureSelectionModel;
class QgsIFeatureSelectionManager;
class QgsMapCanvas;
class QgsVectorLayer;
class QgsVectorLayerCache;
class QMenu;
class QProgressDialog;


/**
 * @brief
 * Provides a table view of features of a @link QgsVectorLayer @endlink.
 *
 * This can either be used as a standalone widget. QgsBrowser features a reference implementation.
 * Or this can be used within the @link QgsDualView @endlink stacked widget.
 */

class GUI_EXPORT QgsAttributeTableView : public QTableView
{
    Q_OBJECT

  public:
    QgsAttributeTableView( QWidget* parent = 0 );
    virtual ~QgsAttributeTableView();

    virtual void setModel( QgsAttributeTableFilterModel* filterModel );

    /**
     * @brief setFeatureSelectionManager
     * @param featureSelectionManager We will take ownership
     */
    void setFeatureSelectionManager( QgsIFeatureSelectionManager* featureSelectionManager );

    /**
     * This event filter is installed on the verticalHeader to intercept mouse press and release
     * events. These are used to disable / enable live synchronisation with the map canvas selection
     * which can be slow due to recurring canvas repaints. Updating the
     *
     * @param object The object which is the target of the event.
     * @param event  The intercepted event
     *
     * @return Returns always false, so the event gets processed
     */
    virtual bool eventFilter( QObject* object, QEvent* event );

  protected:
    /**
     * Called for mouse press events on a table cell.
     * Disables selection change for these events.
     *
     * @param event The mouse event
     */
    void mousePressEvent( QMouseEvent *event );

    /**
     * Called for mouse release events on a table cell.
     * Disables selection change for these events.
     *
     * @param event The mouse event
     */
    void mouseReleaseEvent( QMouseEvent *event );

    /**
     * Called for mouse move events on a table cell.
     * Disables selection change for these events.
     *
     * @param event The mouse event
     */
    void mouseMoveEvent( QMouseEvent *event );

    /**
     * Called for key press events
     * Disables selection change by only pressing an arrow key
     *
     * @param event The mouse event
     */
    void keyPressEvent( QKeyEvent *event );

    /**
     * @brief
     * Is called when the context menu will be shown. Emits a @link willShowContextMenu @endlink signal,
     * so the menu can be populated by other parts of the application.
     *
     * @param event The associated event object.
     */
    void contextMenuEvent( QContextMenuEvent* event );

    /**
     * Saves geometry to the settings on close
     * @param event not used
     */
    void closeEvent( QCloseEvent *event );

  signals:
    /**
     * @brief
     * Is emitted, in order to provide a hook to add aditional menu entries to the context menu.
     *
     * @param menu     If additional QMenuItems are added, they will show up in the context menu.
     * @param atIndex  The QModelIndex, to which the context menu belongs. Relative to the source model.
     *                 In most cases, this will be a @link QgsAttributeTableFilterModel @endlink
     */
    void willShowContextMenu( QMenu* menu, QModelIndex atIndex );

    void finished();

  public slots:
    void repaintRequested( QModelIndexList indexes );
    void repaintRequested();
    virtual void selectAll();
    virtual void selectRow( int row );
    virtual void _q_selectRow( int row );

  private slots:
    void modelDeleted();

  private:
    void selectRow( int row, bool anchor );
    QgsAttributeTableModel* mMasterModel;
    QgsAttributeTableFilterModel* mFilterModel;
    QgsFeatureSelectionModel* mFeatureSelectionModel;
    QgsIFeatureSelectionManager* mFeatureSelectionManager;
    QgsAttributeTableDelegate* mTableDelegate;
    QAbstractItemModel* mModel; // Most likely the filter model
    QMenu *mActionPopup;
    QgsVectorLayerCache* mLayerCache;
    int mRowSectionAnchor;
    QItemSelectionModel::SelectionFlag mCtrlDragSelectionFlag;
};

#endif
