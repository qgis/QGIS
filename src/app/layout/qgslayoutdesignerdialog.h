/***************************************************************************
                             qgslayoutdesignerdialog.h
                             -------------------------
    begin                : July 2017
    copyright            : (C) 2017 by Nyall Dawson
    email                : nyall dot dawson at gmail dot com
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/
#ifndef QGSLAYOUTDESIGNERDIALOG_H
#define QGSLAYOUTDESIGNERDIALOG_H

#include "ui_qgslayoutdesignerbase.h"
#include "qgslayoutdesignerinterface.h"
#include <QToolButton>

class QgsLayoutDesignerDialog;
class QgsLayoutView;
class QgsLayoutViewToolAddItem;
class QgsLayoutViewToolAddNodeItem;
class QgsLayoutViewToolPan;
class QgsLayoutViewToolZoom;
class QgsLayoutViewToolSelect;
class QgsLayoutViewToolEditNodes;
class QgsLayoutViewToolMoveItemContent;
class QgsLayoutRuler;
class QComboBox;
class QSlider;
class QLabel;
class QgsLayoutAppMenuProvider;
class QgsLayoutItem;
class QgsPanelWidgetStack;
class QgsDockWidget;
class QUndoView;
class QTreeView;
class QgsLayoutItemsListView;

class QgsAppLayoutDesignerInterface : public QgsLayoutDesignerInterface
{
    Q_OBJECT

  public:
    QgsAppLayoutDesignerInterface( QgsLayoutDesignerDialog *dialog );
    QgsLayout *layout() override;
    QgsLayoutView *view() override;

  public slots:

    void close() override;

  private:

    QgsLayoutDesignerDialog *mDesigner = nullptr;
};

/**
 * \ingroup app
 * \brief A window for designing layouts.
 */
class QgsLayoutDesignerDialog: public QMainWindow, private Ui::QgsLayoutDesignerBase
{
    Q_OBJECT

  public:

    QgsLayoutDesignerDialog( QWidget *parent = nullptr, Qt::WindowFlags flags = 0 );

    /**
     * Returns the designer interface for the dialog.
     */
    QgsAppLayoutDesignerInterface *iface();

    /**
     * Returns the current layout associated with the designer.
     * \see setCurrentLayout()
     */
    QgsLayout *currentLayout();

    /**
     * Returns the layout view utilized by the designer.
     */
    QgsLayoutView *view();

    /**
     * Sets the current \a layout to edit in the designer.
     * \see currentLayout()
     */
    void setCurrentLayout( QgsLayout *layout );

    /**
     * Sets the icon \a size for the dialog.
     */
    void setIconSizes( int size );

    /**
     * Shows the configuration widget for the specified layout \a item.
     *
     * If \a bringPanelToFront is true, then the item properties panel will be automatically
     * shown and raised to the top of the interface.
     */
    void showItemOptions( QgsLayoutItem *item, bool bringPanelToFront = true );

  public slots:

    /**
     * Opens the dialog, and sets default view.
     */
    void open();

    /**
     * Raise, unminimize and activate this window.
     */
    void activate();

    /**
     * Toggles whether or not the rulers should be \a visible.
     */
    void showRulers( bool visible );

    /**
     * Toggles whether the page grid should be \a visible.
     */
    void showGrid( bool visible );

    /**
     * Toggles whether the item bounding boxes should be \a visible.
     */
    void showBoxes( bool visible );

    /**
     * Toggles whether the layout pages should be \a visible.
     */
    void showPages( bool visible );

    /**
     * Toggles whether snapping to the page grid is \a enabled.
     */
    void snapToGrid( bool enabled );

    /**
     * Toggles whether the page guides should be \a visible.
     */
    void showGuides( bool visible );

    /**
     * Toggles whether snapping to the page guides is \a enabled.
     */
    void snapToGuides( bool enabled );

    /**
     * Toggles whether snapping to the item guides ("smart" guides) is \a enabled.
     */
    void snapToItems( bool enabled );

    /**
     * Unlocks all locked items in the layout.
     * \see lockSelectedItems()
     */
    void unlockAllItems();

    /**
     * Locks any selected items in the layout.
     * \see unlockAllItems()
     */
    void lockSelectedItems();

    /**
     * Sets whether the dock panels are \a hidden.
     */
    void setPanelVisibility( bool hidden );

    /**
     * Raises the selected items up the z-order.
     * \see lowerSelectedItems()
     * \see moveSelectedItemsToTop()
     * \see moveSelectedItemsToBottom()
     */
    void raiseSelectedItems();

    /**
     * Lowers the selected items down the z-order.
     * \see raiseSelectedItems()
     * \see moveSelectedItemsToTop()
     * \see moveSelectedItemsToBottom()
     */
    void lowerSelectedItems();

    /**
     * Raises the selected items to the top of the z-order.
     * \see raiseSelectedItems()
     * \see lowerSelectedItems()
     * \see moveSelectedItemsToBottom()
     */
    void moveSelectedItemsToTop();

    /**
     * Lowers the selected items to the bottom of the z-order.
     * \see raiseSelectedItems()
     * \see lowerSelectedItems()
     * \see moveSelectedItemsToTop()
     */
    void moveSelectedItemsToBottom();

    /**
     * Forces the layout, and all items contained within it, to refresh. For instance, this causes maps to redraw
     * and rebuild cached images, html items to reload their source url, and attribute tables
     * to refresh their contents. Calling this also triggers a recalculation of all data defined
     * attributes within the layout.
     */
    void refreshLayout();

  signals:

    /**
     * Emitted when the dialog is about to close.
     */
    void aboutToClose();

  protected:

    virtual void closeEvent( QCloseEvent * ) override;

  private slots:

    void itemTypeAdded( int id );
    void statusZoomCombo_currentIndexChanged( int index );
    void statusZoomCombo_zoomEntered();
    void sliderZoomChanged( int value );

    //! Updates zoom level in status bar
    void updateStatusZoom();

    //! Updates cursor position in status bar
    void updateStatusCursorPos( QPointF position );

    void toggleFullScreen( bool enabled );

    void addPages();
    void statusMessageReceived( const QString &message );
    void dockVisibilityChanged( bool visible );
    void undoRedoOccurredForItems( const QSet< QString > itemUuids );

  private:

    static bool sInitializedRegistry;

    QgsAppLayoutDesignerInterface *mInterface = nullptr;

    QgsLayout *mLayout = nullptr;

    QActionGroup *mToolsActionGroup = nullptr;

    QgsLayoutView *mView = nullptr;
    QgsLayoutRuler *mHorizontalRuler = nullptr;
    QgsLayoutRuler *mVerticalRuler = nullptr;
    QWidget *mRulerLayoutFix = nullptr;

    //! Combobox in status bar which shows/adjusts current zoom level
    QComboBox *mStatusZoomCombo = nullptr;
    QSlider *mStatusZoomSlider = nullptr;

    //! Labels in status bar which shows current mouse position
    QLabel *mStatusCursorXLabel = nullptr;
    QLabel *mStatusCursorYLabel = nullptr;
    QLabel *mStatusCursorPageLabel = nullptr;

    static QList<double> sStatusZoomLevelsList;

    QgsLayoutViewToolAddItem *mAddItemTool = nullptr;
    QgsLayoutViewToolAddNodeItem *mAddNodeItemTool = nullptr;
    QgsLayoutViewToolPan *mPanTool = nullptr;
    QgsLayoutViewToolZoom *mZoomTool = nullptr;
    QgsLayoutViewToolSelect *mSelectTool = nullptr;
    QgsLayoutViewToolEditNodes *mNodesTool = nullptr;
    QgsLayoutViewToolMoveItemContent *mMoveContentTool = nullptr;

    QMap< QString, QToolButton * > mItemGroupToolButtons;
    QMap< QString, QMenu * > mItemGroupSubmenus;

    QgsLayoutAppMenuProvider *mMenuProvider = nullptr;

    QgsDockWidget *mItemDock = nullptr;
    QgsPanelWidgetStack *mItemPropertiesStack = nullptr;
    QgsDockWidget *mGeneralDock = nullptr;
    QgsPanelWidgetStack *mGeneralPropertiesStack = nullptr;
    QgsDockWidget *mGuideDock = nullptr;
    QgsPanelWidgetStack *mGuideStack = nullptr;

    QUndoView *mUndoView = nullptr;
    QgsDockWidget *mUndoDock = nullptr;

    QgsDockWidget *mItemsDock = nullptr;
    QgsLayoutItemsListView *mItemsTreeView = nullptr;

    QAction *mUndoAction = nullptr;
    QAction *mRedoAction = nullptr;

    struct PanelStatus
    {
      PanelStatus( bool visible = true, bool active = false )
        : isVisible( visible )
        , isActive( active )
      {}
      bool isVisible;
      bool isActive;
    };
    QMap< QString, PanelStatus > mPanelStatus;

    bool mBlockItemOptions = false;

    //! Save window state
    void saveWindowState();

    //! Restore the window and toolbar state
    void restoreWindowState();

    //! Switch to new item creation tool, for a new item of the specified \a id.
    void activateNewItemCreationTool( int id, bool nodeBasedItem );

    void createLayoutPropertiesWidget();

    void initializeRegistry();

};

#endif // QGSLAYOUTDESIGNERDIALOG_H

