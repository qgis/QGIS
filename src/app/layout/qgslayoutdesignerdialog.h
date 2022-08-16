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
class QProgressBar;
class QgsLayoutAppMenuProvider;
class QgsLayoutItem;
class QgsPanelWidgetStack;
class QgsDockWidget;
class QUndoView;
class QTreeView;
class QgsLayoutItemsListView;
class QgsLayoutPropertiesWidget;
class QgsMessageBar;
class QgsLayoutAtlas;
class QgsFeature;
class QgsMasterLayoutInterface;
class QgsLayoutGuideWidget;
class QgsScreenHelper;

class QgsAppLayoutDesignerInterface : public QgsLayoutDesignerInterface
{
    Q_OBJECT

  public:
    QgsAppLayoutDesignerInterface( QgsLayoutDesignerDialog *dialog );
    QWidget *window() override;
    QgsLayout *layout() override;
    QgsMasterLayoutInterface *masterLayout() override;
    QgsLayoutView *view() override;
    QgsMessageBar *messageBar() override;
    void selectItems( const QList< QgsLayoutItem * > &items ) override;
    void setAtlasPreviewEnabled( bool enabled ) override;
    void setAtlasFeature( const QgsFeature &feature ) override;
    bool atlasPreviewEnabled() const override;
    void showItemOptions( QgsLayoutItem *item, bool bringPanelToFront = true ) override;
    QMenu *layoutMenu() override;
    QMenu *editMenu() override;
    QMenu *viewMenu() override;
    QMenu *itemsMenu() override;
    QMenu *atlasMenu() override;
    QMenu *reportMenu() override;
    QMenu *settingsMenu() override;
    QToolBar *layoutToolbar() override;
    QToolBar *navigationToolbar() override;
    QToolBar *actionsToolbar() override;
    QToolBar *atlasToolbar() override;
    void addDockWidget( Qt::DockWidgetArea area, QDockWidget *dock ) override;
    void removeDockWidget( QDockWidget *dock ) override;
    void activateTool( StandardTool tool ) override;
    QgsLayoutDesignerInterface::ExportResults *lastExportResults() const override;
  public slots:

    void close() override;
    void showRulers( bool visible ) override;

  private:

    QgsLayoutDesignerDialog *mDesigner = nullptr;
};

/**
 * \ingroup app
 * \brief A window for designing layouts.
 */
class QgsLayoutDesignerDialog: public QMainWindow, public Ui::QgsLayoutDesignerBase
{
    Q_OBJECT

  public:

    QgsLayoutDesignerDialog( QWidget *parent = nullptr, Qt::WindowFlags flags = Qt::WindowFlags() );
    ~QgsLayoutDesignerDialog() override;

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
     * Sets the current master \a layout to edit in the designer.
     * \see masterLayout()
     */
    void setMasterLayout( QgsMasterLayoutInterface *layout );

    /**
     * Returns the current master layout associated with the designer.
     * \see setMasterLayout()
     */
    QgsMasterLayoutInterface *masterLayout();

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
     * If \a bringPanelToFront is TRUE, then the item properties panel will be automatically
     * shown and raised to the top of the interface.
     */
    void showItemOptions( QgsLayoutItem *item, bool bringPanelToFront = true );

    /**
     * Selects the specified \a items.
     */
    void selectItems( const QList<QgsLayoutItem *> &items );

    /**
     * Returns the designer's message bar.
     */
    QgsMessageBar *messageBar();


    /**
     * Toggles whether the atlas preview mode should be \a enabled in the designer.
     *
     * \see atlasPreviewModeEnabled()
     */
    void setAtlasPreviewEnabled( bool enabled );

    /**
     * Returns whether the atlas preview mode is enabled in the designer.
     *
     * \see setAtlasPreviewEnabled()
     */
    bool atlasPreviewEnabled() const;

    /**
     * Sets the specified feature as the current atlas feature
     */
    void setAtlasFeature( const QgsFeature &feature );

    /**
     * Sets a section \a title, to use to update the dialog title to display
     * the currently edited section.
     */
    void setSectionTitle( const QString &title );

    /**
     * Overloaded function used to sort menu entries alphabetically
     */
    QMenu *createPopupMenu() override;

    /**
     * Returns the dialog's guide manager widget, if it exists.
     */
    QgsLayoutGuideWidget *guideWidget();

    /**
     * Toggles the visibility of the guide manager dock widget.
     */
    void showGuideDock( bool show );

    /**
     * Returns the results of the last export operation performed in the designer.
     *
     * May be NULLPTR if no export has been performed in the designer.
     */
    std::unique_ptr< QgsLayoutDesignerInterface::ExportResults > lastExportResults() const;

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

    /**
     * Pastes items from the clipboard to the current layout.
     * \see pasteInPlace()
     */
    void paste();

    /**
     * Pastes item (in place) from the clipboard to the current layout.
     * \see paste()
     */
    void pasteInPlace();

  signals:

    /**
     * Emitted when the dialog is about to close.
     */
    void aboutToClose();

    /**
     * Emitted whenever a layout is exported from the layout designer.
     *
     */
    void layoutExported();

    /**
     * Emitted when a \a map preview has been refreshed.
     */
    void mapPreviewRefreshed( QgsLayoutItemMap *map );

  protected:

    void closeEvent( QCloseEvent * ) override;
    void dropEvent( QDropEvent *event ) override;
    void dragEnterEvent( QDragEnterEvent *event ) override;

  private slots:

    void setTitle( const QString &title );

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
    void undoRedoOccurredForItems( const QSet< QString > &itemUuids );
    void saveAsTemplate();
    void addItemsFromTemplate();
    void duplicate();
    void saveProject();
    void newLayout();
    void showManager();
    void renameLayout();
    void deleteLayout();
    void print();
    void exportToRaster();
    void exportToPdf();
    void exportToSvg();
    void atlasPreviewTriggered( bool checked );
    void atlasPageComboEditingFinished();
    void atlasNext();
    void atlasPrevious();
    void atlasFirst();
    void atlasLast();
    void printAtlas();
    void exportAtlasToRaster();
    void exportAtlasToSvg();
    void exportAtlasToPdf();

    void exportReportToRaster();
    void exportReportToSvg();
    void exportReportToPdf();
    void printReport();

    void pageSetup();

    //! Sets the printer page orientation when the page orientation changes
    void pageOrientationChanged();

    //! Populate layouts menu from main app's
    void populateLayoutsMenu();

    void updateWindowTitle();

    void backgroundTaskCountChanged( int total );
    void onMapPreviewRefreshed();
    void onItemAdded( QgsLayoutItem *item );

  private:

    static bool sInitializedRegistry;

    QgsAppLayoutDesignerInterface *mInterface = nullptr;

    QgsMasterLayoutInterface *mMasterLayout = nullptr;

    QgsLayout *mLayout = nullptr;

    QgsScreenHelper *mScreenHelper = nullptr;

    QgsMessageBar *mMessageBar = nullptr;

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
    QgsDockWidget *mAtlasDock = nullptr;

    QgsLayoutPropertiesWidget *mLayoutPropertiesWidget = nullptr;

    QUndoView *mUndoView = nullptr;
    QgsDockWidget *mUndoDock = nullptr;

    QgsDockWidget *mItemsDock = nullptr;
    QgsLayoutItemsListView *mItemsTreeView = nullptr;

    QgsDockWidget *mReportDock = nullptr;

    QAction *mUndoAction = nullptr;
    QAction *mRedoAction = nullptr;
    //! Copy/cut/paste actions
    QAction *mActionCut = nullptr;
    QAction *mActionCopy = nullptr;
    QAction *mActionPaste = nullptr;
    QProgressBar *mStatusProgressBar = nullptr;

    QMenu *mDynamicTextMenu = nullptr;

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

    QComboBox *mAtlasPageComboBox = nullptr;

    //! Page & Printer Setup
    std::unique_ptr< QPrinter > mPrinter;
    bool mSetPageOrientation = false;

    QString mTitle;
    QString mSectionTitle;

    QgsLayoutGuideWidget *mGuideWidget = nullptr;

    bool mIsExportingAtlas = false;
    void storeExportResults( QgsLayoutExporter::ExportResult result, QgsLayoutExporter *exporter = nullptr );
    std::unique_ptr< QgsLayoutDesignerInterface::ExportResults> mLastExportResults;
    QMap< QString, QgsLabelingResults *> mLastExportLabelingResults;

    //! Save window state
    void saveWindowState();

    //! Restore the window and toolbar state
    void restoreWindowState();

    //! Switch to new item creation tool, for a new item of the specified \a id.
    void activateNewItemCreationTool( int id, bool nodeBasedItem );

    void createLayoutPropertiesWidget();
    void createAtlasWidget();
    void createReportWidget();

    void initializeRegistry();

    bool containsWmsLayers() const;

    //! Displays a warning because of possible min/max size in WMS
    void showWmsPrintingWarning();

    void showSvgExportWarning();

    //! Displays a warning because of incompatibility between blend modes and QPrinter
    void showRasterizationWarning();
    void showForceVectorWarning();

    bool showFileSizeWarning();
    bool getRasterExportSettings( QgsLayoutExporter::ImageExportSettings &settings, QSize &imageSize );
    bool getSvgExportSettings( QgsLayoutExporter::SvgExportSettings &settings );
    bool getPdfExportSettings( QgsLayoutExporter::PdfExportSettings &settings, bool allowGeoPdfExport = true, const QString &geoPdfReason = QString() );

    void toggleAtlasActions( bool enabled );

    /**
     * Toggles the state of the atlas preview and navigation controls
     */
    void toggleAtlasControls( bool atlasEnabled );

    /**
     * Repopulates the atlas page combo box with valid items.
     */
    void updateAtlasPageComboBox( int pageCount );


    void atlasFeatureChanged( const QgsFeature &feature );

    //! Load predefined scales from the project's properties
    void loadPredefinedScalesFromProject();

    QgsLayoutAtlas *atlas();

    void toggleActions( bool layoutAvailable );

    void setPrinterPageOrientation( QgsLayoutItemPage::Orientation orientation );
    QPrinter *printer();
    QString reportTypeString();
    void updateActionNames( QgsMasterLayoutInterface::Type type );

    QString defaultExportPath() const;
    void setLastExportPath( const QString &path ) const;

    bool checkBeforeExport();

    //! update default action of toolbutton
    void toolButtonActionTriggered( QAction * );

    friend class QgsAtlasExportGuard;
};

#endif // QGSLAYOUTDESIGNERDIALOG_H

