/***************************************************************************
                          qgisinterface.h
  Interface class for exposing functions in QgisApp for use by plugins
                             -------------------
  begin                : 2004-02-11
  copyright            : (C) 2004 by Gary E.Sherman
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
#ifndef QGISINTERFACE_H
#define QGISINTERFACE_H

#include <QObject>
#include <map>

#include "qgis.h"
#include "qgis_sip.h"
#include "qgis_gui.h"
#include "qgscoordinatereferencesystem.h"
#include "qgslayertreeregistrybridge.h"

class QAction;
class QDialog;
class QFont;
class QMenu;
class QToolBar;
class QDockWidget;
class QMainWindow;
class QWidget;
class QActionGroup;

class QgsAdvancedDigitizingDockWidget;
class QgsAttributeDialog;
class QgsCustomDropHandler;
class QgsCustomProjectOpenHandler;
class QgsLayoutCustomDropHandler;
class QgsFeature;
class QgsLayerTreeMapCanvasBridge;
class QgsLayerTreeView;
class QgsLayerTreeGroup;
class QgsLayout;
class QgsMasterLayoutInterface;
class QgsLayoutDesignerInterface;
class QgsMapCanvas;
class QgsMapLayer;
enum class QgsMapLayerType;
class QgsMapLayerConfigWidgetFactory;
class QgsMapDecoration;
class QgsMessageBar;
class QgsPluginManagerInterface;
class QgsRasterLayer;
class QgsVectorLayer;
class QgsVectorLayerTools;
class QgsVectorTileLayer;
class QgsPointCloudLayer;
class QgsOptionsWidgetFactory;
class QgsLocatorFilter;
class QgsStatusBar;
class QgsMeshLayer;
class QgsBrowserGuiModel;
class QgsDevToolWidgetFactory;
class QgsGpsConnection;
class QgsApplicationExitBlockerInterface;
class QgsAbstractMapToolHandler;

/**
 * \ingroup gui
 * \brief QgisInterface
 * Abstract base class defining interfaces exposed by QgisApp and
 * made available to plugins.
 *
 * Only functionality exposed by QgisInterface can be used in plugins.
 * This interface has to be implemented with application specific details.
 *
 * QGIS implements it in QgisAppInterface class, 3rd party applications
 * could provide their own implementation to be able to use plugins.
 */

Q_NOWARN_DEPRECATED_PUSH
class GUI_EXPORT QgisInterface : public QObject
{
    Q_OBJECT

  public:

    //! Constructor
    QgisInterface() = default;

    virtual QgsPluginManagerInterface *pluginManagerInterface() = 0;

    virtual QgsLayerTreeView *layerTreeView() = 0;

    /**
     * Add action to context menu for layers in the layer tree.
     * If allLayers is TRUE, then the action will be available for all layers of given type,
     * otherwise the action will be available only for specific layers added with addCustomActionForLayer()
     * after this call.
     *
     * If menu argument is not empty, the action will be also added to a menu within the main window,
     * creating menu with the given name if it does not exist yet.
     *
     * \see removeCustomActionForLayerType()
     * \see addCustomActionForLayer()
     */
    virtual void addCustomActionForLayerType( QAction *action, QString menu,
        QgsMapLayerType type, bool allLayers ) = 0;

    /**
     * Add action to context menu for a specific layer in the layer tree.
     * It is necessary to first call addCustomActionForLayerType() with allLayers=false
     * in order for this method to have any effect.
     * \see addCustomActionForLayerType()
     */
    virtual void addCustomActionForLayer( QAction *action, QgsMapLayer *layer ) = 0;

    /**
     * Remove action for layers in the layer tree previously added with addCustomActionForLayerType()
     * \see addCustomActionForLayerType()
     */
    virtual bool removeCustomActionForLayerType( QAction *action ) = 0;

    /**
     * Returns a list of all map canvases open in the app.
     * \since QGIS 3.0
     */
    virtual QList< QgsMapCanvas * > mapCanvases() = 0;

    /**
     * Create a new map canvas with the specified unique \a name.
     * \see closeMapCanvas()
     * \since QGIS 3.0
     */
    virtual QgsMapCanvas *createNewMapCanvas( const QString &name ) = 0;

    /**
     * Closes the additional map canvas with matching \a name.
     * \see createNewMapCanvas()
     * \since QGIS 3.0
     */
    virtual void closeMapCanvas( const QString &name ) = 0;

    /**
     * Returns the toolbar icon size. If \a dockedToolbar is TRUE, the icon size
     * for toolbars contained within docks is returned.
     */
    virtual QSize iconSize( bool dockedToolbar = false ) const = 0;

    /**
     * Returns vector layers in edit mode
     * \param modified whether to return only layers that have been modified
     * \returns list of layers in legend order, or empty list
    */
    virtual QList<QgsMapLayer *> editableLayers( bool modified = false ) const = 0;

    //! Returns a pointer to the active layer (layer selected in the legend)
    virtual QgsMapLayer *activeLayer() = 0;

    //! Returns a pointer to the map canvas
    virtual QgsMapCanvas *mapCanvas() = 0;

    /**
     * Returns a list of the active decorations.
     * \since QGIS 3.22
     */
    virtual QList<QgsMapDecoration *> activeDecorations() = 0;

    /**
     * Returns a pointer to the layer tree canvas bridge
     *
     * \since QGIS 2.12
     */
    virtual QgsLayerTreeMapCanvasBridge *layerTreeCanvasBridge() = 0;

    //! Returns a pointer to the main window (instance of QgisApp in case of QGIS)
    virtual QWidget *mainWindow() = 0;

    //! Returns the message bar of the main app
    virtual QgsMessageBar *messageBar() = 0;

    /**
     * Returns all currently open layout designers.
     * \since QGIS 3.0
     */
    virtual QList<QgsLayoutDesignerInterface *> openLayoutDesigners() = 0;


    //! Returns changeable options built from settings and/or defaults
    virtual QMap<QString, QVariant> defaultStyleSheetOptions() = 0;

    //! Returns a reference font for initial qApp (may not be same as QgisApp)
    virtual QFont defaultStyleSheetFont() = 0;

    /**
     * Advanced digitizing dock widget
     * \since QGIS 2.12
     */
    virtual QgsAdvancedDigitizingDockWidget *cadDockWidget() = 0;

    /*
     * Accessors for inserting items into menus and toolbars.
     * An item can be inserted before any existing action.
     */

    /**
     * Returns a reference to the main window "Project" menu.
     */
    virtual QMenu *projectMenu() = 0;

    /**
     * Returns a reference to the main window "Edit" menu.
     */
    virtual QMenu *editMenu() = 0;

    /**
     * Returns a reference to the main window "View" menu.
     */
    virtual QMenu *viewMenu() = 0;

    /**
     * Returns a reference to the main window "Layer" menu.
     */
    virtual QMenu *layerMenu() = 0;

    /**
     * Returns a reference to the main window "Create New Layer" menu.
     */
    virtual QMenu *newLayerMenu() = 0;

    /**
     * Returns a reference to the main window "Add Layer" menu.
     * \since QGIS 2.5
     */
    virtual QMenu *addLayerMenu() = 0;

    /**
     * Returns a reference to the main window "Settings" menu.
     */
    virtual QMenu *settingsMenu() = 0;

    /**
     * Returns a reference to the main window "Plugin" menu.
     */
    virtual QMenu *pluginMenu() = 0;

    /**
     * Returns a reference to the main window "Plugin Help" sub-menu.
     *
     * Plugins are encouraged to insert help and about actions in this submenu instead of creating
     * a submenu under the pluginMenu() which solely contains Plugin Help or About actions.
     *
     * \since QGIS 3.10
     */
    virtual QMenu *pluginHelpMenu() = 0;

    /**
     * Returns a reference to the main window "Raster" menu.
     */
    virtual QMenu *rasterMenu() = 0;

    /**
     * Returns a reference to the main window "Database" menu.
     */
    virtual QMenu *databaseMenu() = 0;

    /**
     * Returns a reference to the main window "Vector" menu.
     */
    virtual QMenu *vectorMenu() = 0;

    /**
     * Returns a reference to the main window "Web" menu.
     */
    virtual QMenu *webMenu() = 0;

    /**
     * Returns a reference to the right most standard menu, which is
     * usually the last menu item before the "Help" menu.
     *
     * This can be used to insert additional top-level menus into
     * their correct position BEFORE the help menu.
     */
    virtual QMenu *firstRightStandardMenu() = 0;

    /**
     * Returns a reference to the main window "Window" menu.
     */
    virtual QMenu *windowMenu() = 0;

    /**
     * Returns a reference to the main window "Help" menu.
     */
    virtual QMenu *helpMenu() = 0;

    // ToolBars

    /**
     * Returns a reference to the main window "File" toolbar.
     */
    virtual QToolBar *fileToolBar() = 0;

    /**
     * Returns a reference to the main window "Layer" toolbar.
     */
    virtual QToolBar *layerToolBar() = 0;

    /**
     * Returns a reference to the main window "Data Source Manager" toolbar.
     * \since QGIS 3.4
     */
    virtual QToolBar *dataSourceManagerToolBar() = 0;

    /**
     * Returns a reference to the main window "Map Navigation" toolbar.
     */
    virtual QToolBar *mapNavToolToolBar() = 0;

    /**
     * Returns a reference to the main window "Digitize" toolbar.
     */
    virtual QToolBar *digitizeToolBar() = 0;

    /**
     * Returns a reference to the main window "Advanced Digitizing" toolbar.
     */
    virtual QToolBar *advancedDigitizeToolBar() = 0;

    /**
     * Returns a reference to the main window "Shape Digitizing" toolbar.
     * \since QGIS 3.0
     */
    virtual QToolBar *shapeDigitizeToolBar() = 0;

    /**
     * Returns a reference to the main window "Attributes" toolbar.
     */
    virtual QToolBar *attributesToolBar() = 0;

    /**
     * Returns a reference to the main window "Selection" toolbar.
     * \since QGIS 3.14
     */
    virtual QToolBar *selectionToolBar() = 0;

    /**
     * Returns a reference to the main window "Plugin" toolbar.
     */
    virtual QToolBar *pluginToolBar() = 0;

    /**
     * Returns a reference to the main window "Help" toolbar.
     */
    virtual QToolBar *helpToolBar() = 0;

    /**
     * Returns a reference to the main window "Raster" toolbar.
     */
    virtual QToolBar *rasterToolBar() = 0;

    /**
     * Returns a reference to the main window "Vector" toolbar.
     */
    virtual QToolBar *vectorToolBar() = 0;

    /**
     * Returns a reference to the main window "Database" toolbar.
     */
    virtual QToolBar *databaseToolBar() = 0;

    /**
     * Returns a reference to the main window "Web" toolbar.
     */
    virtual QToolBar *webToolBar() = 0;

    // Project menu actions
    //! Returns the native New Project action.
    virtual QAction *actionNewProject() = 0;
    //! Returns the Open Project action.
    virtual QAction *actionOpenProject() = 0;
    //! Returns the native Save Project action.
    virtual QAction *actionSaveProject() = 0;
    //! Returns the native Save Project As action.
    virtual QAction *actionSaveProjectAs() = 0;
    //! Returns the native Save Map as Image action.
    virtual QAction *actionSaveMapAsImage() = 0;
    //! Returns the native Project Properties action.
    virtual QAction *actionProjectProperties() = 0;

    //! Create new print layout action
    virtual QAction *actionCreatePrintLayout() = 0;

    //! Show layout manager action
    virtual QAction *actionShowLayoutManager() = 0;
    //! Returns the Exit QGIS action.
    virtual QAction *actionExit() = 0;

    // Edit menu actions

    //! Returns the native Cut Features action.
    virtual QAction *actionCutFeatures() = 0;
    //! Returns the native Copy Features action.
    virtual QAction *actionCopyFeatures() = 0;
    //! Returns the native Paste Features action.
    virtual QAction *actionPasteFeatures() = 0;
    //! Returns the native Add Feature action.
    virtual QAction *actionAddFeature() = 0;
    //! Returns the native Delete Selected Features action.
    virtual QAction *actionDeleteSelected() = 0;
    //! Returns the native Move Features action.
    virtual QAction *actionMoveFeature() = 0;
    //! Returns the native Split Features action.
    virtual QAction *actionSplitFeatures() = 0;
    //! Returns the native Split Parts action.
    virtual QAction *actionSplitParts() = 0;
    //! Returns the native Add Ring action.
    virtual QAction *actionAddRing() = 0;
    //! Returns the native Add Part action.
    virtual QAction *actionAddPart() = 0;
    //! Returns the native Simplify/Smooth Features action.
    virtual QAction *actionSimplifyFeature() = 0;
    //! Returns the native Delete Ring action.
    virtual QAction *actionDeleteRing() = 0;
    //! Returns the native Delete Part action.
    virtual QAction *actionDeletePart() = 0;

    /**
     * Returns the native "Vertex Tool for All Layers" action.
     * \see actionVertexToolActiveLayer()
    */
    virtual QAction *actionVertexTool() = 0;

    /**
     * Returns the native "Vertex Tool for Active Layer" action.
     * \see actionVertexTool()
     * \since QGIS 3.6
    */
    virtual QAction *actionVertexToolActiveLayer() = 0;

    /**
     * Returns the action group for map tools.
     *
     * Any actions added by plugins for toggling a map tool should also be added to this action
     * group so that they behave identically to the native, in-built map tool actions.
     *
     * \since QGIS 3.16
     */
    virtual QActionGroup *mapToolActionGroup() = 0;

    // View menu actions
    //! Returns the native pan action. Call trigger() on it to set the default pan map tool.
    virtual QAction *actionPan() = 0;
    //! Returns the native pan to selected action. Call trigger() on it to pan the map canvas to the selection.
    virtual QAction *actionPanToSelected() = 0;
    //! Returns the native zoom in action. Call trigger() on it to set the default zoom in map tool.
    virtual QAction *actionZoomIn() = 0;
    //! Returns the native zoom out action. Call trigger() on it to set the default zoom out map tool.
    virtual QAction *actionZoomOut() = 0;
    //! Returns the native select action. Call trigger() on it to set the default select map tool.
    virtual QAction *actionSelect() = 0;
    //! Returns the native select rectangle action. Call trigger() on it to set the default select rectangle map tool.
    virtual QAction *actionSelectRectangle() = 0;
    //! Returns the native select polygon action. Call trigger() on it to set the default select polygon map tool.
    virtual QAction *actionSelectPolygon() = 0;
    //! Returns the native select freehand action. Call trigger() on it to set the default select freehand map tool.
    virtual QAction *actionSelectFreehand() = 0;
    //! Returns the native select radius action. Call trigger() on it to set the default select radius map tool.
    virtual QAction *actionSelectRadius() = 0;
    //! Returns the native identify action. Call trigger() on it to set the default identify map tool.
    virtual QAction *actionIdentify() = 0;
    //! Returns the native run action feature action. Call trigger() on it to set the default run feature action map tool.
    virtual QAction *actionFeatureAction() = 0;
    //! Returns the native measure action. Call trigger() on it to set the default measure map tool.
    virtual QAction *actionMeasure() = 0;
    //! Returns the native measure area action. Call trigger() on it to set the default measure area map tool.
    virtual QAction *actionMeasureArea() = 0;
    //! Returns the native zoom full extent action. Call trigger() on it to zoom to the full extent.
    virtual QAction *actionZoomFullExtent() = 0;

    /**
     *  Returns the native zoom to layer action. Call trigger() on it to zoom to the active layer.
     *
     *  \deprecated Use actionZoomToLayers() instead.
     */
    Q_DECL_DEPRECATED virtual QAction *actionZoomToLayer() = 0 SIP_DEPRECATED;

    /**
     * Returns the native zoom to layers action. Call trigger() on it to zoom to the selected layers.
     * \since QGIS 3.18
     */
    virtual QAction *actionZoomToLayers() = 0;

    //! Returns the native zoom to selected action. Call trigger() on it to zoom to the current selection.
    virtual QAction *actionZoomToSelected() = 0;
    //! Returns the native zoom last action. Call trigger() on it to zoom to last.
    virtual QAction *actionZoomLast() = 0;
    //! Returns the native zoom next action. Call trigger() on it to zoom to next.
    virtual QAction *actionZoomNext() = 0;
    //! Returns the native zoom resolution (100%) action. Call trigger() on it to zoom to actual size.
    virtual QAction *actionZoomActualSize() = 0;
    //! Returns the native map tips action. Call trigger() on it to toggle map tips.
    virtual QAction *actionMapTips() = 0;
    //! Returns the native new bookmark action. Call trigger() on it to open the new bookmark dialog.
    virtual QAction *actionNewBookmark() = 0;
    //! Returns the native show bookmarks action. Call trigger() on it to open the bookmarks dialog.
    virtual QAction *actionShowBookmarks() = 0;
    //! Returns the native draw action.
    virtual QAction *actionDraw() = 0;

    // Layer menu actions
    virtual QAction *actionNewVectorLayer() = 0;
    virtual QAction *actionAddOgrLayer() = 0;
    virtual QAction *actionAddRasterLayer() = 0;
    virtual QAction *actionAddPgLayer() = 0;
    virtual QAction *actionAddWmsLayer() = 0;

    /**
     * Returns the native Add XYZ Layer action.
     * \since QGIS 3.14
     */
    virtual QAction *actionAddXyzLayer() = 0;

    /**
     * Returns the native Add Vector Tile Layer action.
     * \since QGIS 3.14
     */
    virtual QAction *actionAddVectorTileLayer() = 0;

    /**
     * Returns the native Add Point Cloud Layer action.
     * \since QGIS 3.18
     */
    virtual QAction *actionAddPointCloudLayer() = 0;

    //! Returns the native Add ArcGIS REST Server action.
    virtual QAction *actionAddAfsLayer() = 0;

    //! Returns the native Add ArcGIS REST Server action.
    virtual QAction *actionAddAmsLayer() = 0;

    virtual QAction *actionCopyLayerStyle() = 0;
    virtual QAction *actionPasteLayerStyle() = 0;
    virtual QAction *actionOpenTable() = 0;
    virtual QAction *actionOpenFieldCalculator() = 0;

    /**
     * Statistical summary action.
     * \since QGIS 3.0
     */
    virtual QAction *actionOpenStatisticalSummary() = 0;

    virtual QAction *actionToggleEditing() = 0;
    virtual QAction *actionSaveActiveLayerEdits() = 0;
    virtual QAction *actionAllEdits() = 0;
    virtual QAction *actionSaveEdits() = 0;
    virtual QAction *actionSaveAllEdits() = 0;
    virtual QAction *actionRollbackEdits() = 0;
    virtual QAction *actionRollbackAllEdits() = 0;
    virtual QAction *actionCancelEdits() = 0;
    virtual QAction *actionCancelAllEdits() = 0;
    virtual QAction *actionLayerSaveAs() = 0;
    virtual QAction *actionDuplicateLayer() = 0;
    virtual QAction *actionLayerProperties() = 0;
    virtual QAction *actionAddToOverview() = 0;
    virtual QAction *actionAddAllToOverview() = 0;
    virtual QAction *actionRemoveAllFromOverview() = 0;
    virtual QAction *actionHideAllLayers() = 0;
    virtual QAction *actionShowAllLayers() = 0;
    virtual QAction *actionHideSelectedLayers() = 0;

    /**
     * Returns the Toggle Selected Layers action.
     * \since QGIS 3.14
     */
    virtual QAction *actionToggleSelectedLayers() = 0;

    /**
     * Returns the Toggle Selected Layers Independently action.
     * \since QGIS 3.14
     */
    virtual QAction *actionToggleSelectedLayersIndependently() = 0;

    /**
     * Returns the Hide Deselected Layers action.
     * \since QGIS 3.0
     */
    virtual QAction *actionHideDeselectedLayers() = 0;
    virtual QAction *actionShowSelectedLayers() = 0;

    // Plugin menu actions
    virtual QAction *actionManagePlugins() = 0;
    virtual QAction *actionPluginListSeparator() = 0;
    virtual QAction *actionShowPythonDialog() = 0;

    // Settings menu actions
    virtual QAction *actionToggleFullScreen() = 0;
    virtual QAction *actionOptions() = 0;
    virtual QAction *actionCustomProjection() = 0;

    // Help menu actions
    virtual QAction *actionHelpContents() = 0;
    virtual QAction *actionQgisHomePage() = 0;
    virtual QAction *actionCheckQgisVersion() = 0;
    virtual QAction *actionAbout() = 0;

    // Shape digitize actions

    /**
     * Returns the native add circle from 2 points action. Call trigger() on it to set the map tool.
     * \deprecated since QGIS 3.26 shape digitizing is now part of the add feature tool. To enable the shape tool, use QgsMapToolCapture::setCurrentCaptureTechnique() and then QgsMapToolCapture::setCurrentShapeMapTool().
     */
    Q_DECL_DEPRECATED virtual QAction *actionCircle2Points() SIP_DEPRECATED {return actionAddFeature();}

    /**
     * Returns the native add circle from 3 points action. Call trigger() on it to set the map tool.
     * \deprecated since QGIS 3.26 shape digitizing is now part of the add feature tool. To enable the shape tool, use QgsMapToolCapture::setCurrentCaptureTechnique() and then QgsMapToolCapture::setCurrentShapeMapTool().
     */
    Q_DECL_DEPRECATED virtual QAction *actionCircle3Points() SIP_DEPRECATED {return actionAddFeature();}

    /**
     * Returns the native add circle from 3 tangents action. Call trigger() on it to set the map tool.
     * \deprecated since QGIS 3.26 shape digitizing is now part of the add feature tool. To enable the shape tool, use QgsMapToolCapture::setCurrentCaptureTechnique() and then QgsMapToolCapture::setCurrentShapeMapTool().
     */
    Q_DECL_DEPRECATED virtual QAction *actionCircle3Tangents() SIP_DEPRECATED {return actionAddFeature();}

    /**
     * Returns the native add circle from 2 tangents and a point action. Call trigger() on it to set the map tool.
     * \deprecated since QGIS 3.26 shape digitizing is now part of the add feature tool. To enable the shape tool, use QgsMapToolCapture::setCurrentCaptureTechnique() and then QgsMapToolCapture::setCurrentShapeMapTool().
     */
    Q_DECL_DEPRECATED virtual QAction *actionCircle2TangentsPoint() SIP_DEPRECATED {return actionAddFeature();}

    /**
     * Returns the native add circle from center action. Call trigger() on it to set the map tool.
     * \deprecated since QGIS 3.26 shape digitizing is now part of the add feature tool. To enable the shape tool, use QgsMapToolCapture::setCurrentCaptureTechnique() and then QgsMapToolCapture::setCurrentShapeMapTool().
     */
    Q_DECL_DEPRECATED virtual QAction *actionCircleCenterPoint() SIP_DEPRECATED {return actionAddFeature();}

    /**
     * Returns the native add ellipse from center and 2 points action. Call trigger() on it to set the map tool.
     * \deprecated since QGIS 3.26 shape digitizing is now part of the add feature tool. To enable the shape tool, use QgsMapToolCapture::setCurrentCaptureTechnique() and then QgsMapToolCapture::setCurrentShapeMapTool().
     */
    Q_DECL_DEPRECATED virtual QAction *actionEllipseCenter2Points() SIP_DEPRECATED {return actionAddFeature();}

    /**
     * Returns the native add ellipse from center and a point action. Call trigger() on it to set the map tool.
     * \deprecated since QGIS 3.26 shape digitizing is now part of the add feature tool. To enable the shape tool, use QgsMapToolCapture::setCurrentCaptureTechnique() and then QgsMapToolCapture::setCurrentShapeMapTool().
     */
    Q_DECL_DEPRECATED virtual QAction *actionEllipseCenterPoint() SIP_DEPRECATED {return actionAddFeature();}

    /**
     * Returns the native add ellipse from an extent action. Call trigger() on it to set the map tool.
     * \deprecated since QGIS 3.26 shape digitizing is now part of the add feature tool. To enable the shape tool, use QgsMapToolCapture::setCurrentCaptureTechnique() and then QgsMapToolCapture::setCurrentShapeMapTool().
     */
    Q_DECL_DEPRECATED virtual QAction *actionEllipseExtent() SIP_DEPRECATED {return actionAddFeature();}

    /**
     * Returns the native add ellipse from foci action. Call trigger() on it to set the map tool.
     * \deprecated since QGIS 3.26 shape digitizing is now part of the add feature tool. To enable the shape tool, use QgsMapToolCapture::setCurrentCaptureTechnique() and then QgsMapToolCapture::setCurrentShapeMapTool().
     */
    Q_DECL_DEPRECATED virtual QAction *actionEllipseFoci() SIP_DEPRECATED {return actionAddFeature();}

    /**
     * Returns the native add rectangle from center and a point action. Call trigger() on it to set the map tool.
     * \deprecated since QGIS 3.26 shape digitizing is now part of the add feature tool. To enable the shape tool, use QgsMapToolCapture::setCurrentCaptureTechnique() and then QgsMapToolCapture::setCurrentShapeMapTool().
     */
    Q_DECL_DEPRECATED virtual QAction *actionRectangleCenterPoint() SIP_DEPRECATED {return actionAddFeature();}

    /**
     * Returns the native add rectangle from extent action. Call trigger() on it to set the map tool.
     * \deprecated since QGIS 3.26 shape digitizing is now part of the add feature tool. To enable the shape tool, use QgsMapToolCapture::setCurrentCaptureTechnique() and then QgsMapToolCapture::setCurrentShapeMapTool().
     */
    Q_DECL_DEPRECATED virtual QAction *actionRectangleExtent() SIP_DEPRECATED {return actionAddFeature();}

    /**
     * Returns the native add rectangle from 3 points (distance from 2nd and 3rd points) action. Call trigger() on it to set the map tool.
     * \deprecated since QGIS 3.26 shape digitizing is now part of the add feature tool. To enable the shape tool, use QgsMapToolCapture::setCurrentCaptureTechnique() and then QgsMapToolCapture::setCurrentShapeMapTool().
     */
    Q_DECL_DEPRECATED virtual QAction *actionRectangle3PointsDistance() SIP_DEPRECATED {return actionAddFeature();}

    /**
     * Returns the native add rectangle from 3 points (distance from projected 3rd point on segment p1 and p2) action. Call trigger() on it to set the map tool.
     * \deprecated since QGIS 3.26 shape digitizing is now part of the add feature tool. To enable the shape tool, use QgsMapToolCapture::setCurrentCaptureTechnique() and then QgsMapToolCapture::setCurrentShapeMapTool().
     */
    Q_DECL_DEPRECATED virtual QAction *actionRectangle3PointsProjected() SIP_DEPRECATED {return actionAddFeature();}

    /**
     * Returns the native add regular polygon from 2 points action. Call trigger() on it to set the map tool.
     * \deprecated since QGIS 3.26 shape digitizing is now part of the add feature tool. To enable the shape tool, use QgsMapToolCapture::setCurrentCaptureTechnique() and then QgsMapToolCapture::setCurrentShapeMapTool().
     */
    Q_DECL_DEPRECATED virtual QAction *actionRegularPolygon2Points() SIP_DEPRECATED {return actionAddFeature();}

    /**
     * Returns the native add regular polygon from center and a point action. Call trigger() on it to set the map tool.
     * \deprecated since QGIS 3.26 shape digitizing is now part of the add feature tool. To enable the shape tool, use QgsMapToolCapture::setCurrentCaptureTechnique() and then QgsMapToolCapture::setCurrentShapeMapTool().
     */
    Q_DECL_DEPRECATED virtual QAction *actionRegularPolygonCenterPoint() SIP_DEPRECATED {return actionAddFeature();}

    /**
     * Returns the native add regular polygon from center and a corner action. Call trigger() on it to set the map tool.
     * \deprecated since QGIS 3.26 shape digitizing is now part of the add feature tool. To enable the shape tool, use QgsMapToolCapture::setCurrentCaptureTechnique() and then QgsMapToolCapture::setCurrentShapeMapTool().
     */
    Q_DECL_DEPRECATED virtual QAction *actionRegularPolygonCenterCorner() SIP_DEPRECATED {return actionAddFeature();}

    /**
     * Access the vector layer tools instance.
     * With the help of this you can access methods like addFeature, startEditing
     * or stopEditing while giving the user the appropriate dialogs.
     *
     * \returns An instance of the vector layer tools
     */
    virtual QgsVectorLayerTools *vectorLayerTools() = 0;

    //! Returns the timeout for timed messages: default of 5 seconds
    virtual int messageTimeout() = 0;

    /**
     * Returns a pointer to the app's status bar interface. This should be
     * used for interacting and adding widgets and messages to the app's
     * status bar (do not use the native Qt statusBar() method).
     * \since QGIS 3.0
     */
    virtual QgsStatusBar *statusBarIface() = 0;

    /**
     * Take screenshots for user documentation
     *
     * \param saveDirectory path where the screenshots will be saved
     * \param categories an int as a flag value of QgsAppScreenShots::Categories
     * \since QGIS 3.4
     */
    virtual void takeAppScreenShots( const QString &saveDirectory, const int categories = 0 ) {Q_UNUSED( saveDirectory ) Q_UNUSED( categories );}

    /**
     * Returns the insertion point.
     * This represents the current layer tree group and index where newly added map layers should be inserted into.
     * \since QGIS 3.10
     */
    virtual QgsLayerTreeRegistryBridge::InsertionPoint layerTreeInsertionPoint() = 0;

  public slots: // TODO: do these functions really need to be slots?

    /* Exposed functions */

    /**
     * Zooms to the full extent of all map layers.
     */
    virtual void zoomFull() = 0;

    /**
     * Zooms to the previous view extent.
     */
    virtual void zoomToPrevious() = 0;

    /**
     * Zooms to the next view extent.
     */
    virtual void zoomToNext() = 0;

    /**
     * Zooms to extent of the active layer.
     */
    virtual void zoomToActiveLayer() = 0;

    /**
     * Adds a vector layer to the current project.
     */
    virtual QgsVectorLayer *addVectorLayer( const QString &vectorLayerPath, const QString &baseName, const QString &providerKey ) = 0;

    /**
     * Adds a raster layer to the current project, given a raster layer file name.
     */
    virtual QgsRasterLayer *addRasterLayer( const QString &rasterLayerPath, const QString &baseName = QString() ) = 0;

    /**
     * Adds a raster layer to the current project, from the specified raster data provider.
     */
    virtual QgsRasterLayer *addRasterLayer( const QString &url, const QString &layerName, const QString &providerKey ) = 0;

    /**
     * Adds a mesh layer to the current project.
     */
    virtual QgsMeshLayer *addMeshLayer( const QString &url, const QString &baseName, const QString &providerKey ) = 0;

    /**
     * Adds a vector tile layer to the current project.
     * \since QGIS 3.14
     */
    virtual QgsVectorTileLayer *addVectorTileLayer( const QString &url, const QString &baseName ) = 0;

    /**
     * Adds a point cloud layer to the current project.
     * \since QGIS 3.18
     */
    virtual QgsPointCloudLayer *addPointCloudLayer( const QString &url, const QString &baseName, const QString &providerKey ) = 0;

    //! Adds (opens) a project
    virtual bool addProject( const QString &project ) = 0;

    /**
     * Starts a new blank project.
     *
     * If \a promptToSaveFlag is TRUE then users will be prompted to save any currently open
     * project (if that project has changes). If the flag is FALSE, then the current project will
     * be closed without prompting to save (possibly resulting in data loss).
     *
     * Since QGIS 3.10.1, returns TRUE if a new project was created, or FALSE if the operation was not successful (e.g.
     * the user opted to cancel when prompted to save the current project).
     */
    virtual bool newProject( bool promptToSaveFlag = false ) = 0;

    /**
     * Triggered when connections have changed.
     * This calls reloadConnections in the main application and triggers a signal that is
     * forwarded to the GUI elements that needs to be updated (i.e. the source
     * select dialogs and the browser widgets)
     * \since QGIS 3.0
     */
    virtual void reloadConnections( ) = 0;

    /**
     * Set the active layer (layer gets selected in the legend)
     * returns TRUE if the layer exists, FALSE otherwise
     */
    virtual bool setActiveLayer( QgsMapLayer * ) = 0;

    /**
     * Copy selected features from the layer to clipboard
     * \since QGIS 3.0
     */
    virtual void copySelectionToClipboard( QgsMapLayer * ) = 0;

    /**
     * Paste features from clipboard to the layer
     * \since QGIS 3.0
     */
    virtual void pasteFromClipboard( QgsMapLayer * ) = 0;

    //! Add an icon to the plugins toolbar
    virtual int addToolBarIcon( QAction *qAction ) = 0;

    /**
     * Add a widget to the plugins toolbar.
     * To remove this widget again, call removeToolBarIcon()
     * with the returned QAction.
     *
     * \param widget widget to add. The toolbar will take ownership of this widget
     * \returns the QAction you can use to remove this widget from the toolbar
     */
    virtual QAction *addToolBarWidget( QWidget *widget SIP_TRANSFER ) = 0;

    //! Remove an action (icon) from the plugin toolbar
    virtual void removeToolBarIcon( QAction *qAction ) = 0;

    /**
     * Add a widget to the raster toolbar.
     * To remove this widget again, call removeRasterToolBarIcon()
     * with the returned QAction.
     *
     * \param widget widget to add. The toolbar will take ownership of this widget
     * \returns the QAction you can use to remove this widget from the toolbar
     */
    virtual QAction *addRasterToolBarWidget( QWidget *widget SIP_TRANSFER ) = 0;

    //! Add an icon to the Raster toolbar
    virtual int addRasterToolBarIcon( QAction *qAction ) = 0;

    //! Remove an action (icon) from the Raster toolbar
    virtual void removeRasterToolBarIcon( QAction *qAction ) = 0;

    //! Add an icon to the Vector toolbar
    virtual int addVectorToolBarIcon( QAction *qAction ) = 0;

    /**
     * Add a widget to the vector toolbar.
     * To remove this widget again, call removeVectorToolBarIcon()
     * with the returned QAction.
     *
     * \param widget widget to add. The toolbar will take ownership of this widget
     * \returns the QAction you can use to remove this widget from the toolbar
     */
    virtual QAction *addVectorToolBarWidget( QWidget *widget SIP_TRANSFER ) = 0;

    //! Remove an action (icon) from the Vector toolbar
    virtual void removeVectorToolBarIcon( QAction *qAction ) = 0;

    //! Add an icon to the Database toolbar
    virtual int addDatabaseToolBarIcon( QAction *qAction ) = 0;

    /**
     * Add a widget to the database toolbar.
     * To remove this widget again, call removeDatabaseToolBarIcon()
     * with the returned QAction.
     *
     * \param widget widget to add. The toolbar will take ownership of this widget
     * \returns the QAction you can use to remove this widget from the toolbar
     */
    virtual QAction *addDatabaseToolBarWidget( QWidget *widget SIP_TRANSFER ) = 0;

    //! Remove an action (icon) from the Database toolbar
    virtual void removeDatabaseToolBarIcon( QAction *qAction ) = 0;

    //! Add an icon to the Web toolbar
    virtual int addWebToolBarIcon( QAction *qAction ) = 0;

    /**
     * Add a widget to the web toolbar.
     * To remove this widget again, call removeWebToolBarIcon()
     * with the returned QAction.
     *
     * \param widget widget to add. The toolbar will take ownership of this widget
     * \returns the QAction you can use to remove this widget from the toolbar
     */
    virtual QAction *addWebToolBarWidget( QWidget *widget SIP_TRANSFER ) = 0;

    //! Remove an action (icon) from the Web toolbar
    virtual void removeWebToolBarIcon( QAction *qAction ) = 0;

    //! Add toolbar with specified name
    virtual QToolBar *addToolBar( const QString &name ) = 0 SIP_FACTORY;

    /**
     * Add a toolbar
     * \since QGIS 2.3
     */
    virtual void addToolBar( QToolBar *toolbar SIP_TRANSFER, Qt::ToolBarArea area = Qt::TopToolBarArea ) = 0;

    /**
     * Opens the message log dock widget.
     */
    virtual void openMessageLog() = 0;

    //! Adds a widget to the user input tool bar.
    virtual void addUserInputWidget( QWidget *widget ) = 0;

    /**
     * Opens the layout manager dialog.
     * \since QGIS 3.0
     */
    virtual void showLayoutManager() = 0;

    /**
     * Opens a new layout designer dialog for the specified \a layout, or
     * brings an already open designer window to the foreground if one
     * is already created for the layout.
     * \since QGIS 3.0
     */
    virtual QgsLayoutDesignerInterface *openLayoutDesigner( QgsMasterLayoutInterface *layout ) = 0;

    /**
     * Opens the options dialog. The \a currentPage argument can be used to force
     * the dialog to open at a specific page.
     * \since QGIS 3.0
     */
    virtual void showOptionsDialog( QWidget *parent = nullptr, const QString &currentPage = QString() ) = 0;

    /**
     * Opens the project properties dialog. The \a currentPage argument can be used to force
     * the dialog to open at a specific page.
     * \since QGIS 3.16
     */
    virtual void showProjectPropertiesDialog( const QString &currentPage = QString() ) = 0;

    /**
     * Generate stylesheet
     * \param opts generated default option values, or a changed copy of them
     */
    virtual void buildStyleSheet( const QMap<QString, QVariant> &opts ) = 0;

    //! Save changed default option keys/values to user settings
    virtual void saveStyleSheetOptions( const QMap<QString, QVariant> &opts ) = 0;

    //! Add action to the plugins menu
    virtual void addPluginToMenu( const QString &name, QAction *action ) = 0;

    //! Remove action from the plugins menu
    virtual void removePluginMenu( const QString &name, QAction *action ) = 0;

    //! Add "add layer" action to layer menu
    virtual void insertAddLayerAction( QAction *action ) = 0;

    //! Remove "add layer" action from layer menu
    virtual void removeAddLayerAction( QAction *action ) = 0;

    //! Add action to the Database menu
    virtual void addPluginToDatabaseMenu( const QString &name, QAction *action ) = 0;

    //! Remove action from the Database menu
    virtual void removePluginDatabaseMenu( const QString &name, QAction *action ) = 0;

    //! Add action to the Raster menu
    virtual void addPluginToRasterMenu( const QString &name, QAction *action ) = 0;

    //! Remove action from the Raster menu
    virtual void removePluginRasterMenu( const QString &name, QAction *action ) = 0;

    //! Add action to the Vector menu
    virtual void addPluginToVectorMenu( const QString &name, QAction *action ) = 0;

    //! Remove action from the Vector menu
    virtual void removePluginVectorMenu( const QString &name, QAction *action ) = 0;

    //! Add action to the Web menu
    virtual void addPluginToWebMenu( const QString &name, QAction *action ) = 0;

    //! Remove action from the Web menu
    virtual void removePluginWebMenu( const QString &name, QAction *action ) = 0;

    /**
     * Add action to the Mesh menu
     *
     * \since QGIS 3.24
     */
    virtual void addPluginToMeshMenu( const QString &name, QAction *action ) = 0;

    /**
     * Remove action from the Mesh menu
     *
     * \since QGIS 3.24
     */
    virtual void removePluginMeshMenu( const QString &name, QAction *action ) = 0;

    /**
     * Adds a \a dock widget to the main window, in the specified dock \a area.
     *
     * \see addTabifiedDockWidget()
     * \see removeDockWidget()
     */
    virtual void addDockWidget( Qt::DockWidgetArea area, QDockWidget *dockwidget ) = 0;

    /**
     * Add a dock widget to the given area and tabify it (if other dock widgets
     * exist in the same \a area). The new tab will be below other tabs unless
     * \a raiseTab is passed as TRUE.
     *
     * \a tabifyWith is a list of dock widget object names, ordered by
     * priority, with which the new dock widget should be tabified. Only the
     * first matching object name will be picked. If none of the given object
     * names is found in that \a area (or if \a tabifyWith is not given at
     * all), the new dock widget will be created anyways, but its location
     * within that \a area will be unpredictable.
     *
     * \since QGIS 3.14
     */
    virtual void addTabifiedDockWidget( Qt::DockWidgetArea area, QDockWidget *dockwidget, const QStringList &tabifyWith = QStringList(), bool raiseTab = false ) = 0;

    /**
     * Removes the specified \a dock widget from main window (without deleting it).
     *
     * \see addDockWidget()
     */
    virtual void removeDockWidget( QDockWidget *dockwidget ) = 0;

    /**
     * Opens layer properties dialog for the layer \a l.
     * Optionally, a \a page to open can be specified (since QGIS 3.20).
     * The list below contains valid page names:
     *
     * Vector Layer:
     * mOptsPage_Information, mOptsPage_Source, mOptsPage_Style, mOptsPage_Labels,
     * mOptsPage_Masks, mOptsPage_Diagrams, mOptsPage_SourceFields, mOptsPage_AttributesForm,
     * mOptsPage_Joins, mOptsPage_AuxiliaryStorage, mOptsPage_Actions, mOptsPage_Display,
     * mOptsPage_Rendering, mOptsPage_Temporal, mOptsPage_Variables, mOptsPage_Metadata,
     * mOptsPage_DataDependencies, mOptsPage_Legend, mOptsPage_Server
     *
     * Raster Layer:
     * mOptsPage_Information, mOptsPage_Source, mOptsPage_Style, mOptsPage_Transparency,
     * mOptsPage_Histogram, mOptsPage_Rendering, mOptsPage_Temporal, mOptsPage_Pyramids,
     * mOptsPage_Metadata, mOptsPage_Legend, mOptsPage_Server
     *
     * Mesh Layer:
     * mOptsPage_Information, mOptsPage_Source, mOptsPage_Style, mOptsPage_StyleContent,
     * mOptsPage_Rendering, mOptsPage_Temporal, mOptsPage_Metadata
     *
     * Point Cloud Layer:
     * mOptsPage_Information, mOptsPage_Source, mOptsPage_Metadata, mOptsPage_Statistics
     *
     * Vector Tile Layer:
     * mOptsPage_Information, mOptsPage_Style, mOptsPage_Labeling, mOptsPage_Metadata
     *
     * \note Page names are subject to change without notice between QGIS versions,
     * they are not considered part of the stable API.
     *
     * \note More strings may be available depending on the context, e.g. via plugins.
     */
    virtual void showLayerProperties( QgsMapLayer *l, const QString &page = QString() ) = 0;

    //! Open attribute table dialog
    virtual QDialog *showAttributeTable( QgsVectorLayer *l, const QString &filterExpression = QString() ) = 0;

    /**
     * Add window to Window menu. The action title is the window title
     * and the action should raise, unminimize and activate the window.
    */
    virtual void addWindow( QAction *action ) = 0;

    /**
     * Remove window from Window menu. Calling this is necessary only for
     * windows which are hidden rather than deleted when closed.
    */
    virtual void removeWindow( QAction *action ) = 0;

    //! Register action to the shortcuts manager so its shortcut can be changed in GUI
    virtual bool registerMainWindowAction( QAction *action, const QString &defaultShortcut ) = 0;

    //! Unregister a previously registered action. (e.g. when plugin is going to be unloaded)
    virtual bool unregisterMainWindowAction( QAction *action ) = 0;

    /**
     * Register a new tab in the map layer properties dialog.
     * \note Ownership of the factory is not transferred, and the factory must
     *       be unregistered when plugin is unloaded.
     * \see QgsMapLayerConfigWidgetFactory
     * \see unregisterMapLayerConfigWidgetFactory()
     * \since QGIS 2.16
     */
    virtual void registerMapLayerConfigWidgetFactory( QgsMapLayerConfigWidgetFactory *factory ) = 0;

    /**
     * Unregister a previously registered tab in the map layer properties dialog.
     * \see QgsMapLayerConfigWidgetFactory
     * \see registerMapLayerConfigWidgetFactory()
     * \since QGIS 2.16
    */
    virtual void unregisterMapLayerConfigWidgetFactory( QgsMapLayerConfigWidgetFactory *factory ) = 0;

    /**
     * Register a new tab in the options dialog.
     * \note Ownership of the factory is not transferred, and the factory must
     *       be unregistered when plugin is unloaded.
     * \see QgsOptionsWidgetFactory
     * \see unregisterOptionsWidgetFactory()
     * \since QGIS 3.0
     */
    virtual void registerOptionsWidgetFactory( QgsOptionsWidgetFactory *factory ) = 0;

    /**
     * Unregister a previously registered tab in the options dialog.
     * \see QgsOptionsWidgetFactory
     * \see registerOptionsWidgetFactory()
     * \since QGIS 3.0
    */
    virtual void unregisterOptionsWidgetFactory( QgsOptionsWidgetFactory *factory ) = 0;

    /**
     * Register a new tab in the project properties dialog.
     * \note Ownership of the factory is not transferred, and the factory must
     *       be unregistered when plugin is unloaded.
     * \see QgsOptionsWidgetFactory
     * \see unregisterProjectPropertiesWidgetFactory()
     * \since QGIS 3.16
     */
    virtual void registerProjectPropertiesWidgetFactory( QgsOptionsWidgetFactory *factory ) = 0;

    /**
     * Unregister a previously registered tab in the options dialog.
     * \see QgsOptionsWidgetFactory
     * \see registerProjectPropertiesWidgetFactory()
     * \since QGIS 3.16
    */
    virtual void unregisterProjectPropertiesWidgetFactory( QgsOptionsWidgetFactory *factory ) = 0;

    /**
     * Register a new tool in the development/debugging tools dock.
     * \note Ownership of the factory is not transferred, and the factory must
     *       be unregistered when plugin is unloaded.
     * \see unregisterDevToolWidgetFactory()
     * \since QGIS 3.14
     */
    virtual void registerDevToolWidgetFactory( QgsDevToolWidgetFactory *factory ) = 0;

    /**
     * Unregister a previously registered tool factory from the development/debugging tools dock.
     * \see registerDevToolWidgetFactory()
     * \since QGIS 3.14
    */
    virtual void unregisterDevToolWidgetFactory( QgsDevToolWidgetFactory *factory ) = 0;

    /**
     * Register a new application exit blocker, which can be used to prevent the QGIS application
     * from exiting while a plugin or script has unsaved changes.
     *
     * \note Ownership of \a blocker is not transferred, and the blocker must
     *       be unregistered when plugin is unloaded.
     *
     * \see unregisterApplicationExitBlocker()
     * \since QGIS 3.16
     */
    virtual void registerApplicationExitBlocker( QgsApplicationExitBlockerInterface *blocker ) = 0;

    /**
     * Unregister a previously registered application exit \a blocker.
     * \see registerApplicationExitBlocker()
     * \since QGIS 3.16
    */
    virtual void unregisterApplicationExitBlocker( QgsApplicationExitBlockerInterface *blocker ) = 0;

    /**
     * Register a new application map tool \a handler, which can be used to automatically setup all connections
     * and logic required to switch to a custom map tool whenever the state of the QGIS application
     * permits.
     *
     * \note Ownership of \a handler is not transferred, and the handler must
     *       be unregistered when plugin is unloaded.
     *
     * \see QgsAbstractMapToolHandler
     * \see unregisterMapToolHandler()
     * \since QGIS 3.16
     */
    virtual void registerMapToolHandler( QgsAbstractMapToolHandler *handler ) = 0;

    /**
     * Unregister a previously registered map tool \a handler.
     * \see registerMapToolHandler()
     * \since QGIS 3.16
    */
    virtual void unregisterMapToolHandler( QgsAbstractMapToolHandler *handler ) = 0;

    /**
     * Register a new custom drop \a handler.
     * \note Ownership of \a handler is not transferred, and the handler must
     *       be unregistered when plugin is unloaded.
     * \see QgsCustomDropHandler
     * \see unregisterCustomDropHandler()
     * \since QGIS 3.0
     */
    virtual void registerCustomDropHandler( QgsCustomDropHandler *handler ) = 0;

    /**
     * Unregister a previously registered custom drop \a handler.
     * \see QgsCustomDropHandler
     * \see registerCustomDropHandler()
     * \since QGIS 3.0
     */
    virtual void unregisterCustomDropHandler( QgsCustomDropHandler *handler ) = 0;

    /**
     * Register a new custom project open \a handler.
     * \note Ownership of \a handler is not transferred, and the handler must
     *       be unregistered when plugin is unloaded.
     * \see QgsCustomProjectOpenHandler
     * \see unregisterCustomProjectOpenHandler()
     * \since QGIS 3.14
     */
    virtual void registerCustomProjectOpenHandler( QgsCustomProjectOpenHandler *handler ) = 0;

    /**
     * Unregister a previously registered custom project open \a handler.
     * \see QgsCustomDropHandler
     * \see registerCustomProjectOpenHandler()
     * \since QGIS 3.14
     */
    virtual void unregisterCustomProjectOpenHandler( QgsCustomProjectOpenHandler *handler ) = 0;

    /**
     * Register a new custom drop \a handler for layout windows.
     * \note Ownership of \a handler is not transferred, and the handler must
     *       be unregistered when plugin is unloaded.
     * \see QgsLayoutCustomDropHandler
     * \see unregisterCustomLayoutDropHandler()
     * \since QGIS 3.0
     */
    virtual void registerCustomLayoutDropHandler( QgsLayoutCustomDropHandler *handler ) = 0;

    /**
     * Unregister a previously registered custom drop \a handler for layout windows.
     * \see QgsLayoutCustomDropHandler
     * \see registerCustomLayoutDropHandler()
     * \since QGIS 3.0
     */
    virtual void unregisterCustomLayoutDropHandler( QgsLayoutCustomDropHandler *handler ) = 0;

    // TODO is this deprecated in favour of QgsContextHelp?

    /**
     * Open a url in the users browser. By default the QGIS doc directory is used
     * as the base for the URL. To open a URL that is not relative to the installed
     * QGIS documentation, set useQgisDocDirectory to FALSE.
     * \param url URL to open
     * \param useQgisDocDirectory If TRUE, the URL will be formed by concatenating
     * url to the QGIS documentation directory path (prefix/share/doc)
     * \deprecated Use QDesktopServices instead
     */
#ifndef Q_MOC_RUN
    Q_DECL_DEPRECATED
#endif
    virtual void openURL( const QString &url, bool useQgisDocDirectory = true ) = 0 SIP_DEPRECATED;

    /**
     * Opens a new feature form.
     * Returns TRUE if dialog was accepted (if shown modal, TRUE otherwise).
     * \param l vector layer
     * \param f feature to show/modify
     * \param updateFeatureOnly only update the feature update (don't change any attributes of the layer) [UNUSED]
     * \param showModal if TRUE, will wait for the dialog to be executed (only shown otherwise)
     */
    virtual bool openFeatureForm( QgsVectorLayer *l, QgsFeature &f, bool updateFeatureOnly = false, bool showModal = true ) = 0;

    /**
     * Returns a feature form for a given feature.
     *
     * \param l The layer for which the dialog will be created
     * \param f The feature for which the dialog will be created
     *
     * \returns A feature form
     */
    virtual QgsAttributeDialog *getFeatureForm( QgsVectorLayer *l, QgsFeature &f ) = 0;

    /**
     * This method is only needed when using a UI form with a custom widget plugin and calling
     * openFeatureForm or getFeatureForm from Python (PyQt) and you haven't used the info tool first.
     * Python will crash bringing QGIS with it
     * if the custom form is not loaded from a C++ method call.
     *
     * This method uses a QTimer to call QUiLoader in order to load the form via C++.
     * You only need to call this once. After that you can call openFeatureForm/getFeatureForm
     * like usual.
     *
     * More information here: http://qt-project.org/forums/viewthread/27098/
     */
    virtual void preloadForm( const QString &uifile ) = 0;

    /**
     * This will perform a search in the locator bar
     * by setting the line edit text to \a searchText
     * and automatically displaying any results.
     * \since QGIS 3.10
     */
    virtual void locatorSearch( const QString &searchText ) = 0;

    /**
     * Registers a locator \a filter for the app's locator bar. Ownership of the filter is transferred to the
     * locator.
     * \warning Plugins which register filters to the locator bar must take care to correctly call
     * deregisterLocatorFilter() and deregister their filters upon plugin unload to avoid crashes.
     * \see deregisterLocatorFilter()
     * \since QGIS 3.0
     */
    virtual void registerLocatorFilter( QgsLocatorFilter *filter SIP_TRANSFER ) = 0;

    /**
     * Deregisters a locator \a filter from the app's locator bar and deletes it. Calling this will block whilst
     * any currently running query is terminated.
     *
     * Plugins which register filters to the locator bar must take care to correctly call
     * deregisterLocatorFilter() to deregister their filters upon plugin unload to avoid crashes.
     *
     * \see registerLocatorFilter()
     * \since QGIS 3.0
     */
    virtual void deregisterLocatorFilter( QgsLocatorFilter *filter ) = 0;

    /**
     * Invalidate results from the locator filter.
     *
     * This might be useful if the configuration of the filter changed without going through main application settings.
     *
     * \since QGIS 3.2
     */
    virtual void invalidateLocatorResults() = 0;

    /**
      * Checks available datum transforms and ask user if several are available and none
      * is chosen. Dialog is shown only if global option is set accordingly.
      * \returns TRUE if a datum transform has been specifically chosen by user or only one is available.
      * \since 3.0
      */
    virtual bool askForDatumTransform( QgsCoordinateReferenceSystem sourceCrs, QgsCoordinateReferenceSystem destinationCrs ) = 0;

    /**
     * Returns the application browser model. Using this shared model is more efficient than
     * creating a new browser model for every use.
     * \since QGIS 3.4
     */
    virtual QgsBrowserGuiModel *browserModel() = 0;

    /**
     * Sets a GPS \a connection to use within the GPS Panel widget.
     *
     * Any existing GPS connection used by the widget will be disconnect and replaced with this connection. The connection
     * is automatically registered within the QgsApplication::gpsConnectionRegistry().
     *
     * \since QGIS 3.16
     */
    virtual void setGpsPanelConnection( QgsGpsConnection *connection ) = 0;

  signals:

    /**
     * Emitted whenever current (selected) layer changes.
     *  The pointer to layer can be NULLPTR if no layer is selected.
     */
    void currentLayerChanged( QgsMapLayer *layer );

    /**
     * Emitted when the current \a theme is changed so plugins
     * can change their tool button icons.
     * \since QGIS 3.0
    */
    void currentThemeChanged( const QString &theme );

    /**
     * Emitted when a new layout \a designer has been opened.
     * \see layoutDesignerWillBeClosed()
     * \since QGIS 3.0
     */
    void layoutDesignerOpened( QgsLayoutDesignerInterface *designer );

    /**
     * Emitted before a layout \a designer is going to be closed
     * and deleted.
     * \see layoutDesignerClosed()
     * \see layoutDesignerOpened()
     * \since QGIS 3.0
     */
    void layoutDesignerWillBeClosed( QgsLayoutDesignerInterface *designer );

    /**
     * Emitted after a layout designer window is closed.
     * \see layoutDesignerWillBeClosed()
     * \see layoutDesignerOpened()
     * \since QGIS 3.0
     */
    void layoutDesignerClosed();

    /**
     * Emitted when the initialization is complete.
     */
    void initializationCompleted();

    /**
     * Emitted when a project file is successfully read.
     * \note This is useful for plugins that store properties with project files.
     *       A plugin can connect to this signal. When it is emitted the plugin
     *       knows to then check the project properties for any relevant state.
     */
    void projectRead();

    /**
     * Emitted when starting an entirely new project.
     * \note This is similar to projectRead(); plugins might want to be notified
     *       that they're in a new project. Yes, projectRead() could have been
     *       overloaded to be used in the case of new projects instead. However,
     *       it's probably more semantically correct to have an entirely separate
     *       signal for when this happens.
     */
    void newProjectCreated();

    /**
     * Emitted when a layer has been saved using save as.
     * \since QGIS 2.7
     */
    void layerSavedAs( QgsMapLayer *l, const QString &path );

};
Q_NOWARN_DEPRECATED_POP

#endif //#ifndef QGISINTERFACE_H
