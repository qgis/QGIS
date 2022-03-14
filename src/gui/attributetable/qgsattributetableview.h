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

#include <QAction>
#include "qgsfeatureid.h"
#include "qgstableview.h"

#include "qgis_gui.h"
#include "qgsattributetableconfig.h"

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
class QgsAttributeTableConfig;
class QgsFeature;

/**
 * \ingroup gui
 * \brief
 * Provides a table view of features of a QgsVectorLayer.
 *
 * This can either be used as a standalone widget. QgsBrowser features a reference implementation.
 * Or this can be used within the QgsDualView stacked widget.
 */

class GUI_EXPORT QgsAttributeTableView : public QgsTableView
{
    Q_OBJECT

  public:

    //! Constructor for QgsAttributeTableView
    QgsAttributeTableView( QWidget *parent SIP_TRANSFERTHIS = nullptr );

#ifdef __clang__
#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Woverloaded-virtual"
#endif
    virtual void setModel( QgsAttributeTableFilterModel *filterModel );
#ifdef __clang__
#pragma clang diagnostic pop
#endif

    /**
     * \brief setFeatureSelectionManager
     * \param featureSelectionManager
     */
    void setFeatureSelectionManager( QgsIFeatureSelectionManager *featureSelectionManager );

    /**
     * This event filter is installed on the verticalHeader to intercept mouse press and release
     * events. These are used to disable / enable live synchronisation with the map canvas selection
     * which can be slow due to recurring canvas repaints.
     *
     * \param object The object which is the target of the event.
     * \param event  The intercepted event
     *
     * \returns Returns always FALSE, so the event gets processed
     */
    bool eventFilter( QObject *object, QEvent *event ) override;

    /**
     * Set the attribute table config which should be used to control
     * the appearance of the attribute table.
     * \since QGIS 2.16
     */
    void setAttributeTableConfig( const QgsAttributeTableConfig &config );

    /**
     * Returns the selected features in the attribute table in table sorted order.
     * \returns The selected features in the attribute table in the order sorted by the table.
     * \since QGIS 3.4
     */
    QList<QgsFeatureId> selectedFeaturesIds() const;

    /**
     * Scroll to a feature with a given \a fid.
     *
     * Optionally a \a column can be specified, which will also bring that column into view.
     *
     * \since QGIS 3.16
     */
    void scrollToFeature( const QgsFeatureId &fid, int column = -1 );

  protected:

    /**
     * Called for mouse press events on a table cell.
     * Disables selection change for these events.
     *
     * \param event The mouse event
     */
    void mousePressEvent( QMouseEvent *event ) override;

    /**
     * Called for mouse release events on a table cell.
     * Disables selection change for these events.
     *
     * \param event The mouse event
     */
    void mouseReleaseEvent( QMouseEvent *event ) override;

    /**
     * Called for mouse move events on a table cell.
     * Disables selection change for these events.
     *
     * \param event The mouse event
     */
    void mouseMoveEvent( QMouseEvent *event ) override;

    /**
     * Called for key press events
     * Disables selection change by only pressing an arrow key
     *
     * \param event The mouse event
     */
    void keyPressEvent( QKeyEvent *event ) override;

    /**
     * \brief
     * Is called when the context menu will be shown. Emits a willShowContextMenu() signal,
     * so the menu can be populated by other parts of the application.
     *
     * \param event The associated event object.
     */
    void contextMenuEvent( QContextMenuEvent *event ) override;

    /**
     * Saves geometry to the settings on close
     * \param event not used
     */
    void closeEvent( QCloseEvent *event ) override;

  signals:

    /**
     * \brief
     * Emitted in order to provide a hook to add additional* menu entries to the context menu.
     *
     * \param menu     If additional QMenuItems are added, they will show up in the context menu.
     * \param atIndex  The QModelIndex, to which the context menu belongs. Relative to the source model.
     *                 In most cases, this will be a QgsAttributeTableFilterModel
     */
    void willShowContextMenu( QMenu *menu, const QModelIndex &atIndex );

    /**
     * Emitted when a column in the view has been resized.
     * \param column column index (starts at 0)
     * \param width new width in pixel
     * \since QGIS 2.16
     */
    void columnResized( int column, int width );

    void finished();

  public slots:
    void repaintRequested( const QModelIndexList &indexes );
    void repaintRequested();
    void selectAll() override;
    virtual void selectRow( int row );
    virtual void _q_selectRow( int row );

  private slots:
    void modelDeleted();
    void showHorizontalSortIndicator();
    void actionTriggered();
    void columnSizeChanged( int index, int oldWidth, int newWidth );
    void onActionColumnItemPainted( const QModelIndex &index );
    void recreateActionWidgets();

  private:
    void updateActionImage( QWidget *widget );
    QWidget *createActionWidget( QgsFeatureId fid );

    void selectRow( int row, bool anchor );
    QgsAttributeTableFilterModel *mFilterModel = nullptr;
    QgsFeatureSelectionModel *mFeatureSelectionModel = nullptr;
    QgsIFeatureSelectionManager *mOwnedFeatureSelectionManager = nullptr;
    QgsIFeatureSelectionManager *mFeatureSelectionManager = nullptr;
    QgsAttributeTableDelegate *mTableDelegate = nullptr;
    QMenu *mActionPopup = nullptr;
    int mRowSectionAnchor = 0;
    QItemSelectionModel::SelectionFlag mCtrlDragSelectionFlag = QItemSelectionModel::Select;
    QMap< QModelIndex, QWidget * > mActionWidgets;
    QgsAttributeTableConfig mConfig;
    QString mSortExpression;
};

#endif
