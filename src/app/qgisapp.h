/***************************************************************************
                          qgisapp.h  -  description
                             -------------------
    begin                : Sat Jun 22 2002
    copyright            : (C) 2002 by Gary E.Sherman
    email                : sherman at mrcc.com
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef QGISAPP_H
#define QGISAPP_H

class QActionGroup;
class QCheckBox;
class QCursor;
class QFileInfo;
class QKeyEvent;
class QLabel;
class QMenu;
class QPixmap;
class QProgressBar;
class QPushButton;
class QRect;
class QSettings;
class QSplashScreen;
class QStringList;
class QToolButton;
class QTcpSocket;
class QValidator;

class QgisAppInterface;
class QgisAppStyleSheet;
class QgsAnnotationItem;
class QgsClipboard;
class QgsComposer;
class QgsComposerView;
class QgsContrastEnhancement;
class QgsGeometry;
class QgsFeature;

class QgsLegend;
class QgsLayerOrder;
class QgsMapCanvas;
class QgsMapLayer;
class QgsMapTip;
class QgsMapTool;
class QgsPalLabeling;
class QgsPoint;
class QgsProviderRegistry;
class QgsPythonUtils;
class QgsRectangle;
class QgsUndoWidget;
class QgsVectorLayer;

class QDomDocument;
class QNetworkReply;
class QNetworkProxy;
class QAuthenticator;

class QgsBrowserDockWidget;
class QgsSnappingDialog;
class QgsGPSInformationWidget;

class QgsDecorationItem;

class QgsMessageLogViewer;
class QgsMessageBar;

class QgsScaleComboBox;

class QgsDataItem;
class QgsTileScaleWidget;

#include <QMainWindow>
#include <QToolBar>
#include <QAbstractSocket>
#include <QPointer>
#include <QSslError>

#include "qgsconfig.h"
#include "qgsfeature.h"
#include "qgsfeaturestore.h"
#include "qgspoint.h"
#include "qgsrasterlayer.h"
#include "qgssnappingdialog.h"
#include "qgspluginmanager.h"

#include "ui_qgisapp.h"

#ifdef HAVE_TOUCH
#include <QGestureEvent>
#include <QTapAndHoldGesture>
#endif

/*! \class QgisApp
 * \brief Main window for the Qgis application
 */
class QgisApp : public QMainWindow, private Ui::MainWindow
{
    Q_OBJECT
  public:
    //! Constructor
    QgisApp( QSplashScreen *splash, bool restorePlugins = true, QWidget * parent = 0, Qt::WFlags fl = Qt::Window );
    //! Constructor for unit tests
    QgisApp( );
    //! Destructor
    ~QgisApp();
    /**
     * Add a vector layer to the canvas, returns pointer to it
     */
    QgsVectorLayer* addVectorLayer( QString vectorLayerPath, QString baseName, QString providerKey );

    /** \brief overloaded version of the private addLayer method that takes a list of
     * file names instead of prompting user with a dialog.
     @param enc encoding type for the layer
    @param dataSourceType type of ogr datasource
     @returns true if successfully added layer
     */
    bool addVectorLayers( QStringList const & theLayerQStringList, const QString& enc, const QString dataSourceType );

    /** overloaded vesion of the private addRasterLayer()
      Method that takes a list of file names instead of prompting
      user with a dialog.
      @returns true if successfully added layer(s)
      */
    bool addRasterLayers( QStringList const & theLayerQStringList, bool guiWarning = true );

    /** open a raster layer for the given file
      @returns false if unable to open a raster layer for rasterFile
      @note
      This is essentially a simplified version of the above
      */
    QgsRasterLayer* addRasterLayer( QString const & rasterFile, QString const & baseName, bool guiWarning = true );

    /** Add a 'pre-made' map layer to the project */
    void addMapLayer( QgsMapLayer *theMapLayer );

    /** Set the extents of the map canvas */
    void setExtent( QgsRectangle theRect );
    //! Remove all layers from the map and legend - reimplements same method from qgisappbase
    void removeAllLayers();
    /** Open a raster or vector file; ignore other files.
      Used to process a commandline argument, FileOpen or Drop event.
      Set interactive to true if it is ok to ask the user for information (mostly for
      when a vector layer has sublayers and we want to ask which sublayers to use).
      @returns true if the file is successfully opened
      */
    bool openLayer( const QString & fileName, bool allowInteractive = false );
    /** Open the specified project file; prompt to save previous project if necessary.
      Used to process a commandline argument, FileOpen or Drop event.
      */
    void openProject( const QString & fileName );
    /** opens a qgis project file
      @returns false if unable to open the project
      */
    bool addProject( QString projectFile );
    /** Convenience function to open either a project or a layer file.
      */
    void openFile( const QString & fileName );
    //!Overloaded version of the private function with same name that takes the imagename as a parameter
    void saveMapAsImage( QString, QPixmap * );

    /** Get the mapcanvas object from the app */
    QgsMapCanvas *mapCanvas();

    QgsMessageBar* messageBar();

    /** Get the mapcanvas object from the app */
    QgsPalLabeling *palLabeling();

    //! Set theme (icons)
    void setTheme( QString themeName = "default" );

    void setIconSizes( int size );

    //! Get stylesheet builder object for app and print composers
    //! @note added in 1.9
    QgisAppStyleSheet* styleSheetBuilder();

    //! Setup the toolbar popup menus for a given theme
    void setupToolbarPopups( QString themeName );
    //! Returns a pointer to the internal clipboard
    QgsClipboard * clipboard();

    static QgisApp *instance() { return smInstance; }

    //! initialize network manager
    void namSetup();

    //! update proxy settings
    void namUpdate();

    /** Add a dock widget to the main window. Overloaded from QMainWindow.
     * After adding the dock widget to the ui (by delegating to the QMainWindow
     * parent class, it will also add it to the View menu list of docks.*/
    void addDockWidget( Qt::DockWidgetArea area, QDockWidget * dockwidget );
    void removeDockWidget( QDockWidget * dockwidget );
    /** Add a toolbar to the main window. Overloaded from QMainWindow.
     * After adding the toolbar to the ui (by delegating to the QMainWindow
     * parent class, it will also add it to the View menu list of toolbars.*/
    QToolBar *addToolBar( QString name );

    /** Add window to Window menu. The action title is the window title
     * and the action should raise, unminimize and activate the window. */
    void addWindow( QAction *action );
    /** Remove window from Window menu. Calling this is necessary only for
     * windows which are hidden rather than deleted when closed. */
    void removeWindow( QAction *action );

    /**Returns the print composers*/
    QSet<QgsComposer*> printComposers() const {return mPrintComposers;}
    /** Get a unique title from user for new and duplicate composers
     * @param acceptEmpty whether to accept empty titles (one will be generated)
     * @param currentTitle base name for initial title choice
     * @return QString::null if user cancels input dialog
     * @note added in 1.9
     */
    QString uniqueComposerTitle( QWidget* parent, bool acceptEmpty, const QString& currentTitle = QString( "" ) );
    /**Creates a new composer and returns a pointer to it*/
    QgsComposer* createNewComposer( QString title = QString( "" ) );
    /**Deletes a composer and removes entry from Set*/
    void deleteComposer( QgsComposer* c );
    /** Duplicates a composer and adds it to Set
     * @note added in 1.9
     */
    QgsComposer* duplicateComposer( QgsComposer* currentComposer, QString title = QString( "" ) );

    /** overloaded function used to sort menu entries alphabetically */
    QMenu* createPopupMenu();


    //! Actions to be inserted in menus and toolbars
    QAction *actionNewProject() { return mActionNewProject; }
    QAction *actionOpenProject() { return mActionOpenProject; }
    QAction *actionSaveProject() { return mActionSaveProject; }
    QAction *actionSaveProjectAs() { return mActionSaveProjectAs; }
    QAction *actionSaveMapAsImage() { return mActionSaveMapAsImage; }
    QAction *actionProjectProperties() { return mActionProjectProperties; }
    QAction *actionShowComposerManager() { return mActionShowComposerManager; }
    QAction *actionNewPrintComposer() { return mActionNewPrintComposer; }
    QAction *actionExit() { return mActionExit; }

    QAction *actionCutFeatures() { return mActionCutFeatures; }
    QAction *actionCopyFeatures() { return mActionCopyFeatures; }
    QAction *actionPasteFeatures() { return mActionPasteFeatures; }
    QAction *actionPasteAsNewVector() { return mActionPasteAsNewVector; }
    QAction *actionPasteAsNewMemoryVector() { return mActionPasteAsNewMemoryVector; }
    QAction *actionDeleteSelected() { return mActionDeleteSelected; }
    QAction *actionAddFeature() { return mActionAddFeature; }
    QAction *actionMoveFeature() { return mActionMoveFeature; }
    QAction *actionRotateFeature() { return mActionRotateFeature;}
    QAction *actionSplitFeatures() { return mActionSplitFeatures; }
    QAction *actionAddRing() { return mActionAddRing; }
    QAction *actionAddPart() { return mActionAddPart; }
    QAction *actionSimplifyFeature() { return mActionSimplifyFeature; }
    QAction *actionDeleteRing() { return mActionDeleteRing; }
    QAction *actionDeletePart() { return mActionDeletePart; }
    QAction *actionNodeTool() { return mActionNodeTool; }
    QAction *actionSnappingOptions() { return mActionSnappingOptions; }
    QAction *actionOffsetCurve() { return mActionOffsetCurve; }
    QAction *actionPan() { return mActionPan; }
    QAction *actionTouch() { return mActionTouch; }
    QAction *actionPanToSelected() { return mActionPanToSelected; }
    QAction *actionZoomIn() { return mActionZoomIn; }
    QAction *actionZoomOut() { return mActionZoomOut; }
    QAction *actionSelect() { return mActionSelect; }
    QAction *actionSelectRectangle() { return mActionSelectRectangle; }
    QAction *actionSelectPolygon() { return mActionSelectPolygon; }
    QAction *actionSelectFreehand() { return mActionSelectFreehand; }
    QAction *actionSelectRadius() { return mActionSelectRadius; }
    QAction *actionIdentify() { return mActionIdentify; }
    QAction *actionFeatureAction() { return mActionFeatureAction; }
    QAction *actionMeasure() { return mActionMeasure; }
    QAction *actionMeasureArea() { return mActionMeasureArea; }
    QAction *actionZoomFullExtent() { return mActionZoomFullExtent; }
    QAction *actionZoomToLayer() { return mActionZoomToLayer; }
    QAction *actionZoomToSelected() { return mActionZoomToSelected; }
    QAction *actionZoomLast() { return mActionZoomLast; }
    QAction *actionZoomNext() { return mActionZoomNext; }
    QAction *actionZoomActualSize() { return mActionZoomActualSize; }
    QAction *actionMapTips() { return mActionMapTips; }
    QAction *actionNewBookmark() { return mActionNewBookmark; }
    QAction *actionShowBookmarks() { return mActionShowBookmarks; }
    QAction *actionDraw() { return mActionDraw; }

    QAction *actionNewVectorLayer() { return mActionNewVectorLayer; }
    QAction *actionNewSpatialLiteLayer() { return mActionNewSpatiaLiteLayer; }
    QAction *actionEmbedLayers() { return mActionEmbedLayers; }
    QAction *actionAddOgrLayer() { return mActionAddOgrLayer; }
    QAction *actionAddRasterLayer() { return mActionAddRasterLayer; }
    QAction *actionAddPgLayer() { return mActionAddPgLayer; }
    QAction *actionAddSpatiaLiteLayer() { return mActionAddSpatiaLiteLayer; }
    QAction *actionAddWmsLayer() { return mActionAddWmsLayer; }
    QAction *actionAddWcsLayer() { return mActionAddWcsLayer; }
    QAction *actionAddWfsLayer() { return mActionAddWfsLayer; }
    /** @note added in 1.9 */
    QAction *actionCopyLayerStyle() { return mActionCopyStyle; }
    /** @note added in 1.9 */
    QAction *actionPasteLayerStyle() { return mActionPasteStyle; }
    QAction *actionOpenTable() { return mActionOpenTable; }
    QAction *actionOpenFieldCalculator() { return mActionOpenFieldCalc; }
    QAction *actionToggleEditing() { return mActionToggleEditing; }
    /** @note added in 1.9 */
    QAction *actionSaveActiveLayerEdits() { return mActionSaveLayerEdits; }
    /** @note added in 1.9 */
    QAction *actionAllEdits() { return mActionAllEdits; }
    QAction *actionSaveEdits() { return mActionSaveEdits; }
    /** @note added in 1.9 */
    QAction *actionSaveAllEdits() { return mActionSaveAllEdits; }
    /** @note added in 1.9 */
    QAction *actionRollbackEdits() { return mActionRollbackEdits; }
    /** @note added in 1.9 */
    QAction *actionRollbackAllEdits() { return mActionRollbackAllEdits; }
    /** @note added in 1.9 */
    QAction *actionCancelEdits() { return mActionCancelEdits; }
    /** @note added in 1.9 */
    QAction *actionCancelAllEdits() { return mActionCancelAllEdits; }
    QAction *actionLayerSaveAs() { return mActionLayerSaveAs; }
    QAction *actionLayerSelectionSaveAs() { return mActionLayerSelectionSaveAs; }
    QAction *actionRemoveLayer() { return mActionRemoveLayer; }
    /** @note added in 1.9 */
    QAction *actionDuplicateLayer() { return mActionDuplicateLayer; }
    QAction *actionSetLayerCRS() { return mActionSetLayerCRS; }
    QAction *actionSetProjectCRSFromLayer() { return mActionSetProjectCRSFromLayer; }
    QAction *actionLayerProperties() { return mActionLayerProperties; }
    QAction *actionLayerSubsetString() { return mActionLayerSubsetString; }
    QAction *actionAddToOverview() { return mActionAddToOverview; }
    QAction *actionAddAllToOverview() { return mActionAddAllToOverview; }
    QAction *actionRemoveAllFromOverview() { return mActionRemoveAllFromOverview; }
    QAction *actionHideAllLayers() { return mActionHideAllLayers; }
    QAction *actionShowAllLayers() { return mActionShowAllLayers; }

    QAction *actionManagePlugins() { return mActionManagePlugins; }
    QAction *actionPluginListSeparator() { return mActionPluginSeparator1; }
    QAction *actionPluginPythonSeparator() { return mActionPluginSeparator2; }
    QAction *actionShowPythonDialog() { return mActionShowPythonDialog; }

    QAction *actionToggleFullScreen() { return mActionToggleFullScreen; }
    QAction *actionOptions() { return mActionOptions; }
    QAction *actionCustomProjection() { return mActionCustomProjection; }
    QAction *actionConfigureShortcuts() { return mActionConfigureShortcuts; }

#ifdef Q_WS_MAC
    QAction *actionWindowMinimize() { return mActionWindowMinimize; }
    QAction *actionWindowZoom() { return mActionWindowZoom; }
    QAction *actionWindowAllToFront() { return mActionWindowAllToFront; }
#endif

    QAction *actionHelpContents() { return mActionHelpContents; }
    QAction *actionHelpAPI() { return mActionHelpAPI; }
    QAction *actionQgisHomePage() { return mActionQgisHomePage; }
    QAction *actionCheckQgisVersion() { return mActionCheckQgisVersion; }
    QAction *actionAbout() { return mActionAbout; }
    QAction *actionSponsors() { return mActionSponsors; }

    QAction *actionShowPinnedLabels() { return mActionShowPinnedLabels; }

    //! Menus
    Q_DECL_DEPRECATED QMenu *fileMenu() { return mProjectMenu; }
    QMenu *projectMenu() { return mProjectMenu; }
    QMenu *editMenu() { return mEditMenu; }
    QMenu *viewMenu() { return mViewMenu; }
    QMenu *layerMenu() { return mLayerMenu; }
    //! @note added in 2.0
    QMenu *newLayerMenu() { return mNewLayerMenu; }
    QMenu *settingsMenu() { return mSettingsMenu; }
    QMenu *pluginMenu() { return mPluginMenu; }
    QMenu *databaseMenu() { return mDatabaseMenu; }
    QMenu *rasterMenu() { return mRasterMenu; }
    QMenu *vectorMenu() { return mVectorMenu; }
    QMenu *webMenu() { return mWebMenu; }
#ifdef Q_WS_MAC
    QMenu *firstRightStandardMenu() { return mWindowMenu; }
    QMenu *windowMenu() { return mWindowMenu; }
#else
    QMenu *firstRightStandardMenu() { return mHelpMenu; }
    QMenu *windowMenu() { return NULL; }
#endif
    QMenu *printComposersMenu() {return mPrintComposersMenu;}
    QMenu *helpMenu() { return mHelpMenu; }

    //! Toolbars
    /** Get a reference to a toolbar. Mainly intended
    *   to be used by plugins that want to specifically add
    *   an icon into the file toolbar for consistency e.g.
    *   addWFS and GPS plugins.
    */
    QToolBar *fileToolBar() { return mFileToolBar; }
    QToolBar *layerToolBar() { return mLayerToolBar; }
    QToolBar *mapNavToolToolBar() { return mMapNavToolBar; }
    QToolBar *digitizeToolBar() { return mDigitizeToolBar; }
    QToolBar *advancedDigitizeToolBar() { return mAdvancedDigitizeToolBar; }
    QToolBar *attributesToolBar() { return mAttributesToolBar; }
    QToolBar *pluginToolBar() { return mPluginToolBar; }
    QToolBar *helpToolBar() { return mHelpToolBar; }
    QToolBar *rasterToolBar() { return mRasterToolBar; }
    QToolBar *vectorToolBar() { return mVectorToolBar; }
    QToolBar *databaseToolBar() { return mDatabaseToolBar; }
    QToolBar *webToolBar() { return mWebToolBar; }

    //! show layer properties
    void showLayerProperties( QgsMapLayer *ml );

    //! returns pointer to map legend
    QgsLegend *legend();

    //! returns pointer to plugin manager
    QgsPluginManager *pluginManager();

    /** Return vector layers in edit mode
     * @param modified whether to return only layers that have been modified
     * @returns list of layers in legend order, or empty list
     * @note added in 1.9 */
    QList<QgsMapLayer *> editableLayers( bool modified = false ) const;

    /** Get timeout for timed messages: default of 5 seconds
     * @note added in 1.9 */
    int messageTimeout();

#ifdef Q_OS_WIN
    //! ugly hack
    void skipNextContextMenuEvent();
#endif

    //! emit initializationCompleted signal
    //! @note added in 1.6
    void completeInitialization();

    void emitCustomSrsValidation( QgsCoordinateReferenceSystem &crs );

    QList<QgsDecorationItem*> decorationItems() { return mDecorationItems; }
    void addDecorationItem( QgsDecorationItem* item ) { mDecorationItems.append( item ); }

  public slots:
    //! Zoom to full extent
    void zoomFull();
    //! Zoom to the previous extent
    void zoomToPrevious();
    //! Zoom to the forward extent
    void zoomToNext();
    //! Zoom to selected features
    void zoomToSelected();
    //! Pan map to selected features
    //! @note added in 2.0
    void panToSelected();

    //! open the properties dialog for the currently selected layer
    void layerProperties();

    //! show the attribute table for the currently selected layer
    void attributeTable();

    void fieldCalculator();

    //! mark project dirty
    void markDirty();

    /* changed from layerWasAdded in 1.8 */
    void layersWereAdded( QList<QgsMapLayer *> );

    /* layer will be removed - changed from removingLayer to removingLayers
       in 1.8.
    */
    void removingLayers( QStringList );

    //! starts/stops editing mode of the current layer
    void toggleEditing();

    //! starts/stops editing mode of a layer
    bool toggleEditing( QgsMapLayer *layer, bool allowCancel = true );

    /** Save edits for active vector layer and start new transactions
     * @note added in 1.9 */
    void saveActiveLayerEdits();

    /** Save edits of a layer
     * @param leaveEditable leave the layer in editing mode when done (added in QGIS 1.9)
     * @param triggerRepaint send layer signal to repaint canvas when done (added in QGIS 1.9)
     */
    void saveEdits( QgsMapLayer *layer, bool leaveEditable = true, bool triggerRepaint = true );

    /** Cancel edits for a layer
      * @param leaveEditable leave the layer in editing mode when done
      * @param triggerRepaint send layer signal to repaint canvas when done
      * @note added in 1.9
      */
    void cancelEdits( QgsMapLayer *layer, bool leaveEditable = true, bool triggerRepaint = true );

    //! Save current edits for selected layer(s) and start new transaction(s)
    void saveEdits();

    /** Save edits for all layers and start new transactions
     * @note added in 1.9 */
    void saveAllEdits( bool verifyAction = true );

    /** Rollback current edits for selected layer(s) and start new transaction(s)
      * @note added in 1.9 */
    void rollbackEdits();

    /** Rollback edits for all layers and start new transactions
     * @note added in 1.9 */
    void rollbackAllEdits( bool verifyAction = true );

    /** Cancel edits for selected layer(s) and toggle off editing
      * @note added in 1.9 */
    void cancelEdits();

    /** Cancel all edits for all layers and toggle off editing
     * @note added in 1.9 */
    void cancelAllEdits( bool verifyAction = true );

    void updateUndoActions();

    //! cuts selected features on the active layer to the clipboard
    /**
       \param layerContainingSelection  The layer that the selection will be taken from
                                        (defaults to the active layer on the legend)
     */
    void editCut( QgsMapLayer * layerContainingSelection = 0 );
    //! copies selected features on the active layer to the clipboard
    /**
       \param layerContainingSelection  The layer that the selection will be taken from
                                        (defaults to the active layer on the legend)
     */
    void editCopy( QgsMapLayer * layerContainingSelection = 0 );
    //! copies features on the clipboard to the active layer
    /**
       \param destinationLayer  The layer that the clipboard will be pasted to
                                (defaults to the active layer on the legend)
     */
    void editPaste( QgsMapLayer * destinationLayer = 0 );
    //! copies features on the clipboard to a new vector layer
    void pasteAsNewVector();
    //! copies features on the clipboard to a new memory vector layer
    QgsVectorLayer * pasteAsNewMemoryVector( const QString & theLayerName = QString() );
    //! copies style of the active layer to the clipboard
    /**
       \param sourceLayer  The layer where the style will be taken from
                                        (defaults to the active layer on the legend)
     */
    void copyStyle( QgsMapLayer * sourceLayer = 0 );
    //! pastes style on the clipboard to the active layer
    /**
       \param destinatioLayer  The layer that the clipboard will be pasted to
                                (defaults to the active layer on the legend)
     */
    void pasteStyle( QgsMapLayer * destinationLayer = 0 );

    //! copies features to internal clipboard
    void copyFeatures( QgsFeatureStore & featureStore );

    void loadOGRSublayers( QString layertype, QString uri, QStringList list );
    void loadGDALSublayers( QString uri, QStringList list );

    /**Deletes the selected attributes for the currently selected vector layer*/
    void deleteSelected( QgsMapLayer *layer = 0, QWidget* parent = 0 );

    //! project was written
    void writeProject( QDomDocument & );

    //! project was read
    void readProject( const QDomDocument & );

    //! Set app stylesheet from settings
    //! @note added in 1.9
    void setAppStyleSheet( const QString& stylesheet );

    //! request credentials for network manager
    void namAuthenticationRequired( QNetworkReply *reply, QAuthenticator *auth );
    void namProxyAuthenticationRequired( const QNetworkProxy &proxy, QAuthenticator *auth );
#ifndef QT_NO_OPENSSL
    void namSslErrors( QNetworkReply *reply, const QList<QSslError> &errors );
#endif

    //! update default action of toolbutton
    void toolButtonActionTriggered( QAction * );

    //! layer selection changed
    void legendLayerSelectionChanged( void );

    //! Watch for QFileOpenEvent.
    virtual bool event( QEvent * event );

    /** Open a raster layer using the Raster Data Provider.
     *  \note added in 1.9
     */
    QgsRasterLayer* addRasterLayer( QString const & uri, QString const & baseName, QString const & providerKey );

    void addWfsLayer( QString uri, QString typeName );

    void versionReplyFinished();

    QgsMessageLogViewer *logViewer() { return mLogViewer; }

    //! Update project menu with the project templates
    void updateProjectFromTemplates();

  protected:

    //! Handle state changes (WindowTitleChange)
    virtual void changeEvent( QEvent * event );
    //! Have some control over closing of the application
    virtual void closeEvent( QCloseEvent * event );

    virtual void dragEnterEvent( QDragEnterEvent * event );
    virtual void dropEvent( QDropEvent * event );

    //! reimplements widget keyPress event so we can check if cancel was pressed
    virtual void keyPressEvent( QKeyEvent * event );

#ifdef Q_OS_WIN
    //! reimplements context menu event
    virtual void contextMenuEvent( QContextMenuEvent *event );
#endif

  private slots:
    //! validate a SRS
    void validateSrs( QgsCoordinateReferenceSystem &crs );

    //! QGis Sponsors
    void sponsors();
    //! About QGis
    void about();
    //! Add a raster layer to the map (will prompt user for file name using dlg )
    void addRasterLayer();
    //#ifdef HAVE_POSTGRESQL
    //! Add a databaselayer to the map
    void addDatabaseLayer();
    //#endif
    //! Add a list of database layers to the map
    void addDatabaseLayers( QStringList const & layerPathList, QString const & providerKey );
    //! Add a SpatiaLite layer to the map
    void addSpatiaLiteLayer();
    //! Add a Delimited Text layer to the map
    void addDelimitedTextLayer();
    //! Add a vector layer defined by uri, layer name, data source uri
    void addSelectedVectorLayer( QString uri, QString layerName, QString provider );
    //#ifdef HAVE_MSSQL
    //! Add a MSSQL layer to the map
    void addMssqlLayer();
    //#endif
    //#ifdef HAVE_ORACLE
    //! Add a Oracle layer to the map
    void addOracleLayer();
    //#endif
    /** toggles whether the current selected layer is in overview or not */
    void isInOverview();
    //! Slot to show the map coordinate position of the mouse cursor
    void showMouseCoordinate( const QgsPoint & );
    //! Slot to show current map scale;
    void showScale( double theScale );
    //! Slot to handle user scale input;
    void userScale();
    //! Slot to handle user center input;
    void userCenter();
    //! Remove a layer from the map and legend
    void removeLayer();
    /** Duplicate map layer(s) in legend
     * @note added in 1.9 */
    void duplicateLayers( const QList<QgsMapLayer *> lyrList = QList<QgsMapLayer *>() );
    //! Set CRS of a layer
    void setLayerCRS();
    //! Assign layer CRS to project
    void setProjectCRSFromLayer();
    //! zoom to extent of layer
    void zoomToLayerExtent();
    //! zoom to actual size of raster layer
    void zoomActualSize();
    /** Perform a local histogram stretch on the active raster layer
     * (stretch based on pixel values in view extent).
     * Valid for non wms raster layers only.
     * @note Added in QGIS 1.7 */
    void localHistogramStretch();
    /** perform a full histogram stretch on the active raster layer
     * (stretch based on pixels values in full dataset)
     * Valid for non wms raster layers only.
     * @note Added in QGIS 1.7 */
    void fullHistogramStretch();
    /** Perform a local cumulative cut stretch */
    void localCumulativeCutStretch();
    /** Perform a full extent cumulative cut stretch */
    void fullCumulativeCutStretch();
    /**Increase raster brightness
     * Valid for non wms raster layers only.
     * @note Added in QGIS 2.0 */
    void increaseBrightness();
    /**Decrease raster brightness
     * Valid for non wms raster layers only.
     * @note Added in QGIS 2.0 */
    void decreaseBrightness();
    /**Increase raster contrast
     * Valid for non wms raster layers only.
     * @note Added in QGIS 2.0 */
    void increaseContrast();
    /**Decrease raster contrast
     * Valid for non wms raster layers only.
     * @note Added in QGIS 2.0 */
    void decreaseContrast();
    //! plugin manager
    void showPluginManager();
    //! load python support if possible
    void loadPythonSupport();
    //! Find the QMenu with the given name within plugin menu (ie the user visible text on the menu item)
    QMenu* getPluginMenu( QString menuName );
    //! Add the action to the submenu with the given name under the plugin menu
    void addPluginToMenu( QString name, QAction* action );
    //! Remove the action to the submenu with the given name under the plugin menu
    void removePluginMenu( QString name, QAction* action );
    //! Find the QMenu with the given name within the Database menu (ie the user visible text on the menu item)
    QMenu* getDatabaseMenu( QString menuName );
    //! Add the action to the submenu with the given name under the Database menu
    void addPluginToDatabaseMenu( QString name, QAction* action );
    //! Remove the action to the submenu with the given name under the Database menu
    void removePluginDatabaseMenu( QString name, QAction* action );
    //! Find the QMenu with the given name within the Raster menu (ie the user visible text on the menu item)
    QMenu* getRasterMenu( QString menuName );
    //! Add the action to the submenu with the given name under the Raster menu
    void addPluginToRasterMenu( QString name, QAction* action );
    //! Remove the action to the submenu with the given name under the Raster menu
    void removePluginRasterMenu( QString name, QAction* action );
    //! Find the QMenu with the given name within the Vector menu (ie the user visible text on the menu item)
    QMenu* getVectorMenu( QString menuName );
    //! Add the action to the submenu with the given name under the Vector menu
    void addPluginToVectorMenu( QString name, QAction* action );
    //! Remove the action to the submenu with the given name under the Vector menu
    void removePluginVectorMenu( QString name, QAction* action );
    //! Find the QMenu with the given name within the Web menu (ie the user visible text on the menu item)
    QMenu* getWebMenu( QString menuName );
    //! Add the action to the submenu with the given name under the Web menu
    void addPluginToWebMenu( QString name, QAction* action );
    //! Remove the action to the submenu with the given name under the Web menu
    void removePluginWebMenu( QString name, QAction* action );
    //! Add "add layer" action to layer menu
    void insertAddLayerAction( QAction* action );
    //! Remove "add layer" action to layer menu
    void removeAddLayerAction( QAction* action );
    //! Add an icon to the plugin toolbar
    int addPluginToolBarIcon( QAction * qAction );
    /**
     * Add a widget to the plugins toolbar.
     * To remove this widget again, call {@link removeToolBarIcon}
     * with the returned QAction.
     *
     * @param widget widget to add. The toolbar will take ownership of this widget
     * @return the QAction you can use to remove this widget from the toolbar
     */
    QAction* addPluginToolBarWidget( QWidget* widget );
    //! Remove an icon from the plugin toolbar
    void removePluginToolBarIcon( QAction *qAction );
    //! Add an icon to the Raster toolbar
    int addRasterToolBarIcon( QAction * qAction );
    /**
     * Add a widget to the raster toolbar.
     * To remove this widget again, call {@link removeRasterToolBarIcon}
     * with the returned QAction.
     *
     * @param widget widget to add. The toolbar will take ownership of this widget
     * @return the QAction you can use to remove this widget from the toolbar
     */
    QAction* addRasterToolBarWidget( QWidget* widget );
    //! Remove an icon from the Raster toolbar
    void removeRasterToolBarIcon( QAction *qAction );
    //! Add an icon to the Vector toolbar
    int addVectorToolBarIcon( QAction * qAction );
    /**
     * Add a widget to the vector toolbar.
     * To remove this widget again, call {@link removeVectorToolBarIcon}
     * with the returned QAction.
     *
     * @param widget widget to add. The toolbar will take ownership of this widget
     * @return the QAction you can use to remove this widget from the toolbar
     */
    QAction* addVectorToolBarWidget( QWidget* widget );
    //! Remove an icon from the Vector toolbar
    void removeVectorToolBarIcon( QAction *qAction );
    //! Add an icon to the Database toolbar
    int addDatabaseToolBarIcon( QAction * qAction );
    /**
     * Add a widget to the database toolbar.
     * To remove this widget again, call {@link removeDatabaseToolBarIcon}
     * with the returned QAction.
     *
     * @param widget widget to add. The toolbar will take ownership of this widget
     * @return the QAction you can use to remove this widget from the toolbar
     */
    QAction* addDatabaseToolBarWidget( QWidget* widget );
    //! Remove an icon from the Database toolbar
    void removeDatabaseToolBarIcon( QAction *qAction );
    //! Add an icon to the Web toolbar
    int addWebToolBarIcon( QAction * qAction );
    /**
     * Add a widget to the web toolbar.
     * To remove this widget again, call {@link removeWebToolBarIcon}
     * with the returned QAction.
     *
     * @param widget widget to add. The toolbar will take ownership of this widget
     * @return the QAction you can use to remove this widget from the toolbar
     */
    QAction* addWebToolBarWidget( QWidget* widget );
    //! Remove an icon from the Web toolbar
    void removeWebToolBarIcon( QAction *qAction );
    //! Save window state
    void saveWindowState();
    //! Restore the window and toolbar state
    void restoreWindowState();
    //! Save project. Returns true if the user selected a file to save to, false if not.
    bool fileSave();
    //! Save project as
    void fileSaveAs();
    //! Open the project file corresponding to the
    //! text)= of the given action.
    void openProject( QAction *action );
    //! Save the map view as an image - user is prompted for image name using a dialog
    void saveMapAsImage();
    //! Open a project
    void fileOpen();
    //! Create a new project
    void fileNew();
    //! Create a new blank project (no template)
    void fileNewBlank();
    //! As above but allows forcing without prompt and forcing blank project
    void fileNew( bool thePromptToSaveFlag, bool forceBlank = false );
    /** What type of project to open after launch
     * @note Added in QGIS 1.9 */
    void fileOpenAfterLaunch();
    /** After project read, set any auto-opened project as successful
     * @note Added in QGIS 1.9 */
    void fileOpenedOKAfterLaunch();
    //! Create a new file from a template project
    bool fileNewFromTemplate( QString fileName );
    void fileNewFromTemplateAction( QAction * qAction );
    void fileNewFromDefaultTemplate();
    //! Calculate new rasters from existing ones
    void showRasterCalculator();
    void embedLayers();

    //! Create a new empty vector layer
    void newVectorLayer();
    //! Create a new empty spatialite layer
    void newSpatialiteLayer();
    //! Print the current map view frame
    void newPrintComposer();
    void showComposerManager();
    //! Add all loaded layers into the overview - overides qgisappbase method
    void addAllToOverview();
    //! Remove all loaded layers from the overview - overides qgisappbase method
    void removeAllFromOverview();
    //reimplements method from base (gui) class
    void hideAllLayers();
    //reimplements method from base (gui) class
    void showAllLayers();
    // TODO: remove exportMapServer declaration once the mapserver export plugin is complete
    // and tested
    /*
    //! Export current view as a mapserver map file
    void exportMapServer();
    */
    //! Return pointer to the active layer
    QgsMapLayer *activeLayer();
    //! set the active layer
    bool setActiveLayer( QgsMapLayer * );
    //! Open the help contents in a browser
    void helpContents();
    //! Open the API documentation in a browser
    void apiDocumentation();
    //! Open the QGIS support page
    void supportProviders();
    //! Open the QGIS homepage in users browser
    void helpQgisHomePage();
    //! Open a url in the users configured browser
    void openURL( QString url, bool useQgisDocDirectory = true );
    //! Check qgis version against the qgis version server
    void checkQgisVersion();
    //!Invoke the custom projection dialog
    void customProjection();
    //! configure shortcuts
    void configureShortcuts();
    //! show customization dialog
    void customize();
    //! options dialog slot
    void options();
    //! Whats-this help slot
    void whatsThis();
    //! Set project properties, including map untis
    void projectProperties();
    //! Open project properties dialog and show the projections tab
    void projectPropertiesProjections();
    /*  void urlData(); */
    //! Show the spatial bookmarks dialog
    void showBookmarks();
    //! Create a new spatial bookmark
    void newBookmark();
    //! activates the add feature tool
    void addFeature();
    //! activates the move feature tool
    void moveFeature();
    //! activates the offset curve tool
    void offsetCurve();
    //! activates the reshape features tool
    void reshapeFeatures();
    //! activates the split features tool
    void splitFeatures();
    //! activates the add ring tool
    void addRing();
    //! activates the add part tool
    void addPart();
    //! simplifies feature
    void simplifyFeature();
    //! deletes ring in polygon
    void deleteRing();
    //! deletes part of polygon
    void deletePart();
    //! merges the selected features together
    void mergeSelectedFeatures();
    //! merges the attributes of selected features
    void mergeAttributesOfSelectedFeatures();
    //! provides operations with nodes
    void nodeTool();
    //! activates the rotate points tool
    void rotatePointSymbols();
    //! shows the snapping Options
    void snappingOptions();

    //! activates the selection tool
    void select();

    //! activates the rectangle selection tool
    void selectByRectangle();

    //! activates the polygon selection tool
    void selectByPolygon();

    //! activates the freehand selection tool
    void selectByFreehand();

    //! activates the radius selection tool
    void selectByRadius();

    //! deselect features from all layers
    void deselectAll();

    //! select features by expression
    void selectByExpression();

    //! refresh map canvas
    void refreshMapCanvas();

    /** Dialog for verification of action on many edits
     * @note added in 1.9 */
    bool verifyEditsActionDialog( const QString& act, const QString& upon );

    /** Update gui actions/menus when layers are modified
     * @note added in 1.9 */
    void updateLayerModifiedActions();

    //! change layer subset of current vector layer
    void layerSubsetString();

    //! map tool changed
    void mapToolChanged( QgsMapTool *tool );

    /** Called when some layer's editing mode was toggled on/off
     * @note added in 1.9 */
    void layerEditStateChanged();

    /** Activates or deactivates actions depending on the current maplayer type.
    Is called from the legend when the current legend item has changed*/
    void activateDeactivateLayerRelatedActions( QgsMapLayer* layer );

    void selectionChanged( QgsMapLayer *layer );

    void showProgress( int theProgress, int theTotalSteps );
    void extentsViewToggled( bool theFlag );
    void showExtents();
    void showStatusMessage( QString theMessage );
    void updateMouseCoordinatePrecision();
    void hasCrsTransformEnabled( bool theFlag );
    void destinationSrsChanged();
    //    void debugHook();
    //! Add a vector layer to the map
    void addVectorLayer();
    //! Exit Qgis
    void fileExit();
    //! Add a WMS layer to the map
    void addWmsLayer();
    //! Add a WCS layer to the map
    void addWcsLayer();
    //! Add a WFS layer to the map
    void addWfsLayer();
    //! Set map tool to Zoom out
    void zoomOut();
    //! Set map tool to Zoom in
    void zoomIn();
    //! Set map tool to pan
    void pan();
#ifdef HAVE_TOUCH
    //! Set map tool to touch
    void touch();
#endif
    //! Identify feature(s) on the currently selected layer
    void identify();
    //! Measure distance
    void measure();
    //! Measure area
    void measureArea();
    //! Measure angle
    void measureAngle();

    //! Run the default feature action on the current layer
    void doFeatureAction();
    //! Set the default feature action for the current layer
    void updateDefaultFeatureAction( QAction *action );
    //! Refresh the list of feature actions of the current layer
    void refreshFeatureActions();

    //annotations
    void addFormAnnotation();
    void addTextAnnotation();
    void addHtmlAnnotation();
    void addSvgAnnotation();
    void modifyAnnotation();

    /** Alerts user when labeling font for layer has not been found on system
     * @note added in 1.9
     */
    void labelingFontNotFound( QgsVectorLayer* vlayer, const QString& fontfamily );

    /** Alerts user when commit errors occured
     * @note added in 2.0
     */
    void commitError( QgsVectorLayer* vlayer );

    /** Opens the labeling dialog for a layer when called from labelingFontNotFound alert
     * @note added in 1.9
     */
    void labelingDialogFontNotFound( QAction* act );

    //! shows label settings dialog (for labeling-ng)
    void labeling();

    /** Check if deprecated labels are used in project, and flag projects that use them (QGIS 2.0)
     */
    void checkForDeprecatedLabelsInProject();

    //! save current vector layer
    void saveAsFile();
    void saveSelectionAsVectorFile();

    //! save current raster layer
    void saveAsRasterFile();

    //! show python console
    void showPythonDialog();

    //! Shows a warning when an old project file is read.
    void oldProjectVersionWarning( QString );

    //! Toggle map tips on/off
    void toggleMapTips();

    //! Show the map tip
    void showMapTip();

    //! Toggle full screen mode
    void toggleFullScreen();

    //! Set minimized mode of active window
    void showActiveWindowMinimized();

    //! Toggle maximized mode of active window
    void toggleActiveWindowMaximized();

    //! Raise, unminimize and activate this window
    void activate();

    //! Bring forward all open windows
    void bringAllToFront();

    //! Stops rendering of the main map
    void stopRendering();

    void showStyleManagerV2();

    void writeAnnotationItemsToProject( QDomDocument& doc );

    /**Creates the composer instances in a project file and adds them to the menu*/
    bool loadComposersFromProject( const QDomDocument& doc );

    /** Slot to handle display of composers menu, e.g. sorting
     * @note added in 1.9
     */
    void on_mPrintComposersMenu_aboutToShow();

    bool loadAnnotationItemsFromProject( const QDomDocument& doc );

    //! Toggles whether to show pinned labels
    void showPinnedLabels( bool show );
    //! Activates pin labels tool
    void pinLabels();
    //! Activates show/hide labels tool
    void showHideLabels();
    //! Activates the move label tool
    void moveLabel();
    //! Activates rotate feature tool
    void rotateFeature();
    //! Activates rotate label tool
    void rotateLabel();
    //! Activates label property tool
    void changeLabelProperties();

    void renderDecorationItems( QPainter *p );
    void projectReadDecorationItems( );

    //! clear out any stuff from project
    void closeProject();

    //! trust and load project macros
    void enableProjectMacros();

    void osmDownloadDialog();
    void osmImportDialog();
    void osmExportDialog();

  signals:
    /** emitted when a key is pressed and we want non widget sublasses to be able
      to pick up on this (e.g. maplayer) */
    void keyPressed( QKeyEvent *e );

    /** emitted when a project file is successfully read
      @note
      This is useful for plug-ins that store properties with project files.  A
      plug-in can connect to this signal.  When it is emitted, the plug-in
      knows to then check the project properties for any relevant state.
      */
    void projectRead();
    /** emitted when starting an entirely new project
      @note
      This is similar to projectRead(); plug-ins might want to be notified
      that they're in a new project.  Yes, projectRead() could have been
      overloaded to be used in the case of new projects instead.  However,
      it's probably more semantically correct to have an entirely separate
      signal for when this happens.
      */
    void newProject();

    /** Signal emitted when the current theme is changed so plugins
     * can change there tool button icons.
     * @note This was added in QGIS 1.1
     */
    void currentThemeChanged( QString );

    /**This signal is emitted when a new composer instance has been created
       @note added in version 1.4*/
    void composerAdded( QgsComposerView* v );

    /**This signal is emitted before a new composer instance is going to be removed
      @note added in version 1.4*/
    void composerWillBeRemoved( QgsComposerView* v );

    /**This signal is emitted when QGIS' initialization is complete
     @note added in version 1.6*/
    void initializationCompleted();

    void customSrsValidation( QgsCoordinateReferenceSystem &crs );

  private:
    /** This method will open a dialog so the user can select GDAL sublayers to load
     * @returns true if any items were loaded
     * @note added in version 1.9
     */
    bool askUserForZipItemLayers( QString path );
    /** This method will open a dialog so the user can select GDAL sublayers to load
     * @note added in version 1.8
     */
    void askUserForGDALSublayers( QgsRasterLayer *layer );
    /** This method will verify if a GDAL layer contains sublayers
     * @note added in version 1.8
     */
    bool shouldAskUserForGDALSublayers( QgsRasterLayer *layer );
    /** This method will open a dialog so the user can select OGR sublayers to load
    */
    void askUserForOGRSublayers( QgsVectorLayer *layer );
    /** Add a raster layer to the map (passed in as a ptr).
     * It won't force a refresh.
     */
    bool addRasterLayer( QgsRasterLayer * theRasterLayer );

    /** Open a raster layer - this is the generic function which takes all parameters
     * @note added in version 2.0
      */
    QgsRasterLayer* addRasterLayerPrivate( const QString & uri, const QString & baseName,
                                           const QString & providerKey, bool guiWarning,
                                           bool guiUpdate );

    /** add this file to the recently opened/saved projects list
     *  pass settings by reference since creating more than one
     * instance simultaneously results in data loss.
     */
    void saveRecentProjectPath( QString projectPath, QSettings & settings );
    //! Update project menu with the current list of recently accessed projects
    void updateRecentProjectPaths();
    //! Read Well Known Binary stream from PostGIS
    //void readWKB(const char *, QStringList tables);
    //! shows the paste-transformations dialog
    // void pasteTransformations();
    //! check to see if file is dirty and if so, prompt the user th save it
    bool saveDirty();
    /** Helper function to union several geometries together (used in function mergeSelectedFeatures)
      @return 0 in case of error or if canceled */
    QgsGeometry* unionGeometries( const QgsVectorLayer* vl, QgsFeatureList& featureList, bool &canceled );

    /**Deletes all the composer objects and clears mPrintComposers*/
    void deletePrintComposers();

    void saveAsVectorFileGeneral( bool saveOnlySelection, QgsVectorLayer* vlayer = 0, bool symbologyOption = true );

    /** Paste features from clipboard into a new memory layer.
     *  If no features are in clipboard an empty layer is returned.
     *  @return pointer to a new layer or 0 if failed
     */
    QgsVectorLayer * pasteToNewMemoryVector();

    /**Returns all annotation items in the canvas*/
    QList<QgsAnnotationItem*> annotationItems();
    /**Removes annotation items in the canvas*/
    void removeAnnotationItems();

    /// QgisApp aren't copyable
    QgisApp( QgisApp const & );
    /// QgisApp aren't copyable
    QgisApp & operator=( QgisApp const & );

    void readSettings();
    void writeSettings();
    void createActions();
    void createActionGroups();
    void createMenus();
    void createToolBars();
    void createStatusBar();
    void setupConnections();
    void initLegend();
    void createOverview();
    void createCanvasTools();
    void createMapTips();
    void updateCRSStatusBar();
    void createDecorations();

    /**Do histogram stretch for singleband gray / multiband color rasters*/
    void histogramStretch( bool visibleAreaOnly = false, QgsRaster::ContrastEnhancementLimits theLimits = QgsRaster::ContrastEnhancementMinMax );

    /**Apply raster brightness
     * @note Added in QGIS 2.0 */
    void adjustBrightnessContrast( int delta, bool updateBrightness = true );

    QgisAppStyleSheet* mStyleSheetBuilder;

    // actions for menus and toolbars -----------------

#ifdef Q_WS_MAC
    QAction *mActionWindowMinimize;
    QAction *mActionWindowZoom;
    QAction *mActionWindowSeparator1;
    QAction *mActionWindowAllToFront;
    QAction *mActionWindowSeparator2;
    QActionGroup *mWindowActions;
#endif

    QAction* mActionPluginSeparator1;
    QAction* mActionPluginSeparator2;
    QAction* mActionRasterSeparator;

    // action groups ----------------------------------
    QActionGroup *mMapToolGroup;

    // menus ------------------------------------------

#ifdef Q_WS_MAC
    QMenu *mWindowMenu;
#endif
    QMenu *mPanelMenu;
    QMenu *mToolbarMenu;

    // docks ------------------------------------------
    QDockWidget *mLegendDock;
    QDockWidget *mLayerOrderDock;
    QDockWidget *mOverviewDock;
    QDockWidget *mpGpsDock;
    QDockWidget *mLogDock;

#ifdef Q_WS_MAC
    //! Window menu action to select this window
    QAction *mWindowAction;
#endif

    class Tools
    {
      public:
        QgsMapTool* mZoomIn;
        QgsMapTool* mZoomOut;
        QgsMapTool* mPan;
#ifdef HAVE_TOUCH
        QgsMapTool* mTouch;
#endif
        QgsMapTool* mIdentify;
        QgsMapTool* mFeatureAction;
        QgsMapTool* mMeasureDist;
        QgsMapTool* mMeasureArea;
        QgsMapTool* mMeasureAngle;
        QgsMapTool* mAddFeature;
        QgsMapTool* mMoveFeature;
        QgsMapTool* mOffsetCurve;
        QgsMapTool* mReshapeFeatures;
        QgsMapTool* mSplitFeatures;
        QgsMapTool* mSelect;
        QgsMapTool* mSelectRectangle;
        QgsMapTool* mSelectPolygon;
        QgsMapTool* mSelectFreehand;
        QgsMapTool* mSelectRadius;
        QgsMapTool* mVertexAdd;
        QgsMapTool* mVertexMove;
        QgsMapTool* mVertexDelete;
        QgsMapTool* mAddRing;
        QgsMapTool* mAddPart;
        QgsMapTool* mSimplifyFeature;
        QgsMapTool* mDeleteRing;
        QgsMapTool* mDeletePart;
        QgsMapTool* mNodeTool;
        QgsMapTool* mRotatePointSymbolsTool;
        QgsMapTool* mAnnotation;
        QgsMapTool* mFormAnnotation;
        QgsMapTool* mHtmlAnnotation;
        QgsMapTool* mSvgAnnotation;
        QgsMapTool* mTextAnnotation;
        QgsMapTool* mPinLabels;
        QgsMapTool* mShowHideLabels;
        QgsMapTool* mMoveLabel;
        QgsMapTool* mRotateFeature;
        QgsMapTool* mRotateLabel;
        QgsMapTool* mChangeLabelProperties;
    } mMapTools;

    QgsMapTool *mNonEditMapTool;

    //! Widget that will live on the statusbar to display "scale 1:"
    QLabel * mScaleLabel;
    //! Widget that will live on the statusbar to display scale value
    QgsScaleComboBox * mScaleEdit;
    //! The validator for the mScaleEdit
    QValidator * mScaleEditValidator;
    //! Widget that will live on the statusbar to display "Coordinate / Extent"
    QLabel * mCoordsLabel;
    //! Widget that will live in the statusbar to display and edit coords
    QLineEdit * mCoordsEdit;
    //! The validator for the mCoordsEdit
    QValidator * mCoordsEditValidator;
    //! Widget that will live in the statusbar to show progress of operations
    QProgressBar * mProgressBar;
    //! Widget used to suppress rendering
    QCheckBox * mRenderSuppressionCBox;
    //! A toggle to switch between mouse coords and view extents display
    QToolButton * mToggleExtentsViewButton;
    //! Button used to stop rendering
    QToolButton* mStopRenderButton;
    //! Widget in status bar used to show current project CRS
    QLabel * mOnTheFlyProjectionStatusLabel;
    //! Widget in status bar used to show status of on the fly projection
    QToolButton * mOnTheFlyProjectionStatusButton;
    //! Menu that contains the list of actions of the selected vector layer
    QMenu *mFeatureActionMenu;
    //! Popup menu
    QMenu * mPopupMenu;
    //! Top level database menu
    QMenu *mDatabaseMenu;
    //! Top level web menu
    QMenu *mWebMenu;
    //! Popup menu for the map overview tools
    QMenu *toolPopupOverviews;
    //! Popup menu for the display tools
    QMenu *toolPopupDisplay;
    //! Map canvas
    QgsMapCanvas *mMapCanvas;
    //! Table of contents (legend) for the map
    QgsLegend *mMapLegend;
    //! Table of contents (legend) to order layers of the map
    QgsLayerOrder *mMapLayerOrder;
    //! Cursor for the overview map
    QCursor *mOverviewMapCursor;
    //! scale factor
    double mScaleFactor;
    //! Current map window extent in real-world coordinates
    QRect *mMapWindow;
    //! The previously selected non zoom map tool.
    int mPreviousNonZoomMapTool;
    //QCursor *mCursorZoomIn; //doesnt seem to be used anymore (TS)
    QString mStartupPath;
    //! full path name of the current map file (if it has been saved or loaded)
    QString mFullPathName;

    //! interface to QgisApp for plugins
    QgisAppInterface *mQgisInterface;
    friend class QgisAppInterface;

    QSplashScreen *mSplash;
    //! list of recently opened/saved project files
    QStringList mRecentProjectPaths;
    //! Print composers of this project, accessible by id string
    QSet<QgsComposer*> mPrintComposers;
    //! How to determine the number of decimal places used to
    //! display the mouse position
    bool mMousePrecisionAutomatic;
    //! The number of decimal places to use if not automatic
    unsigned int mMousePrecisionDecimalPlaces;
    /** QGIS-internal vector feature clipboard */
    QgsClipboard* mInternalClipboard;
    //! Flag to indicate how the project properties dialog was summoned
    bool mShowProjectionTab;
    /** String containing supporting vector file formats
      Suitable for a QFileDialog file filter.  Build in ctor.
      */
    QString mVectorFileFilter;
    /** String containing supporting raster file formats
      Suitable for a QFileDialog file filter.  Build in ctor.
      */
    QString mRasterFileFilter;

    /** Timer for map tips
     */
    QTimer *mpMapTipsTimer;

    /** Point of last mouse position in map coordinates (used with MapTips)
     */
    QgsPoint mLastMapPosition;

    /* Maptip object
     */
    QgsMapTip *mpMaptip;

    // Flag to indicate if maptips are on or off
    bool mMapTipsVisible;

    //!flag to indicate whether we are in fullscreen mode or not
    bool mFullScreenMode;

    //!flag to indicate that the previous screen mode was 'maximised'
    bool mPrevScreenModeMaximized;

    /** Flag to indicate an edits save/rollback for active layer is in progress
     * @note added in QGIS 1.9
     */
    bool mSaveRollbackInProgress;

    QgsPythonUtils* mPythonUtils;

    static QgisApp *smInstance;

    QgsUndoWidget* mUndoWidget;

    QgsBrowserDockWidget* mBrowserWidget;
    QgsBrowserDockWidget* mBrowserWidget2;

    QgsSnappingDialog* mSnappingDialog;

    QgsPluginManager* mPluginManager;

    //! Persistent tile scale slider
    QgsTileScaleWidget * mpTileScaleWidget;

    QList<QgsDecorationItem*> mDecorationItems;

    int mLastComposerId;

#ifdef Q_OS_WIN
    int mSkipNextContextMenuEvent; // ugly hack
#endif

    //! Persistent GPS toolbox
    QgsGPSInformationWidget * mpGpsWidget;

    QgsMessageLogViewer *mLogViewer;

    QgsPalLabeling* mLBL;

    //! project changed
    void projectChanged( const QDomDocument & );

    bool cmpByText( QAction* a, QAction* b );

    //! the user has trusted the project macros
    bool mTrustedMacros;

    //! a bar to display warnings in a non-blocker manner
    QgsMessageBar *mInfoBar;
    QWidget *mMacrosWarn;

#ifdef HAVE_TOUCH
    bool gestureEvent( QGestureEvent *event );
    void tapAndHoldTriggered( QTapAndHoldGesture *gesture );
#endif

};

#ifdef ANDROID
#define QGIS_ICON_SIZE 32
#else
#define QGIS_ICON_SIZE 24
#endif

#endif
