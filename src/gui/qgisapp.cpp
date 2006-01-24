/***************************************************************************
  qgisapp.cpp  -  description
  -------------------

           begin                : Sat Jun 22 2002
           copyright            : (C) 2002 by Gary E.Sherman
           email                : sherman at mrcc.com
           Romans 3:23=>Romans 6:23=>Romans 10:9,10=>Romans 12
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/
/* $Id$ */

//
// QT4 includes make sure to use the new <CamelCase> style!
//
#include <Q3ListViewItem>
#include <QAction>
#include <QApplication>
#include <QBitmap>
#include <QByteArray>
#include <QCheckBox>
#include <QClipboard>
#include <QColor>
#include <QCursor>
#include <QDesktopWidget>
#include <QDialog>
#include <QDir>
#include <QErrorMessage>
#include <QEvent>
#include <QFile>
#include <QFileInfo>
#include <QGridLayout>
#include <QImage>
#include <QInputDialog>
#include <QKeyEvent>
#include <QLabel>
#include <QLayout>
#include <QLibrary>
#include <QListView>
#include <QMatrix>
#include <QMenu>
#include <QMenuBar>
#include <QMenuItem>
#include <QMessageBox>
#include <QPainter>
#include <QPictureIO>
#include <QPixmap>
#include <QPoint>
#include <QPrinter>
#include <QProcess>
#include <QProgressBar>
#include <QPushButton>
#include <QRect>
#include <QRegExp>
#include <QScrollArea>
#include <QSettings>
#include <QSplitter>
#include <QStatusBar>
#include <QStatusBar>
#include <QStringList>
#include <QTcpSocket>
#include <QTextStream>
#include <QTimer>
#include <QToolBar>
#include <QToolBox>
#include <QToolButton>
#include <QToolTip>
#include <QVBoxLayout>
#include <QWhatsThis>
//
// QGIS Specific Includes
//
#include "qgisapp.h"
#include "qgis.h"
#include "qgsabout.h"
#include "qgsacetaterectangle.h"
#include "qgsapplication.h"
#include "qgsbookmarkitem.h"
#include "qgsbookmarks.h"
#include "qgscomposer.h"
#include <qgscursors.h>
#include "qgscustomprojectiondialog.h"
#include "qgsencodingfiledialog.h"
#include "qgsfile.h"
#include "qgsgeomtypedialog.h"
#include "qgshelpviewer.h"
#include "qgslegend.h"
#include "qgslegendlayerfile.h"
#include "qgslegendlayerfile.h"
#include "qgslegendlayer.h"
#include "qgsmapcanvas.h"
#include "qgsmaplayer.h"
#include "qgsmaplayerinterface.h"
#include "qgsmaplayerregistry.h"
#include "qgsmapoverviewcanvas.h"
#include "qgsmapserverexport.h"
#include "qgsmessageviewer.h"
#include "qgsoptions.h"
#include "qgspastetransformations.h"
#include "qgspluginitem.h"
#include "qgspluginmanager.h"
#include "qgspluginregistry.h"
#include "qgsproject.h"
#include "qgsprojectproperties.h"
#include "qgsproviderregistry.h"
#include "qgsrasterlayer.h"
#include "qgsrasterlayerproperties.h"
#include "qgsrect.h"
#include "qgsserversourceselect.h"
#include "qgssinglesymrenderer.h"
#include "qgsvectorfilewriter.h"
#include "qgsvectorlayer.h"
#include "../../plugins/qgisplugin.h"
#include "../../images/themes/default/qgis.xpm"

//
// Gdal/Ogr includes
//
#include <ogrsf_frmts.h>

//
// Other includes
//
#include <algorithm>
#include <cmath>
#include <functional>
#include <iomanip>
#include <iostream>
#include <list>
#include <memory>
#include <vector>

//#include "qgssisydialog.h"
// XXX deprecated?? #include "qgslegenditem.h"


//
// Conditional Includes
//
#ifdef Q_OS_MACX
#include <ApplicationServices/ApplicationServices.h>
#endif

#ifdef HAVE_POSTGRESQL
#include "qgsdbsourceselect.h"
#endif

#ifndef WIN32
#include <dlfcn.h>
#endif

using namespace std;
class QTreeWidgetItem;

/* typedefs for plugins */
typedef QgsMapLayerInterface *create_it();
typedef QgisPlugin *create_ui(QgisApp * qgis, QgisIface * qI);
typedef QString name_t();
typedef QString description_t();
typedef int type_t();

// IDs for locating particular menu items
const int BEFORE_RECENT_PATHS = 123;
const int AFTER_RECENT_PATHS = 321;


/// build the vector file filter string for a QFileDialog
/*
   called in ctor for initializing mVectorFileFilter
   */
static void buildSupportedVectorFileFilter_(QString & fileFilters);


/** set the application title bar text

  If the current project title is null
  if the project file is null then
  set title text to just application name and version
  else
  set set title text to the the project file name
  else
  set the title text to project title
  */
static void setTitleBarText_( QWidget & qgisApp )
{
  QString caption = QgisApp::tr("Quantum GIS - ");
  caption += QString("%1 ('%2') ").arg(QGis::qgisVersion).arg(QGis::qgisReleaseName) + " ";

  if ( QgsProject::instance()->title().isEmpty() )
  {
    if ( QgsProject::instance()->filename().isEmpty() )
    {
      // no project title nor file name, so just leave caption with
      // application name and version
    }
    else
    {
      QFileInfo projectFileInfo( QgsProject::instance()->filename() );
      caption += projectFileInfo.baseName();
    }
  }
  else
  {
    caption += QgsProject::instance()->title();
  }

  qgisApp.setCaption( caption );
} // setTitleBarText_( QWidget * qgisApp )




// constructor starts here
  QgisApp::QgisApp(QSplashScreen *splash, QWidget * parent, Qt::WFlags fl)
: QMainWindow(parent,fl),
  mMapTool(QGis::NoTool),
  mSplash(splash)
{

  setupUi(this);
  mSplash->showMessage("Reading settings", Qt::AlignHCenter | Qt::AlignBottom);
  qApp->processEvents();
  readSettings();
  mSplash->showMessage("Setting up the GUI", Qt::AlignHCenter | Qt::AlignBottom);
  qApp->processEvents();
  createActions();
  createActionGroups();
  createMenus();
  createToolBars();
  createStatusBar();
  setTheme(mThemeName);
  setupToolbarPopups(mThemeName);
  updateRecentProjectPaths();
  createCanvas();
  createOverview();
  createLegend();

  mSplash->showMessage("Checking database", Qt::AlignHCenter | Qt::AlignBottom);
  qApp->processEvents();
  createDB();    

  // register all GDAL and OGR plug-ins
  // Should this be here? isnt it the job of the provider? Tim
  OGRRegisterAll();

  QPixmap icon;
  icon = QPixmap(qgis_xpm);
  setIcon(icon);
  // store startup location
  QDir *d = new QDir();
  mStartupPath = d->absPath();
  delete d;
  QBitmap zoomincur;
  //  zoomincur = QBitmap(cursorzoomin);
  QBitmap zoomincurmask;
  //  zoomincurmask = QBitmap(cursorzoomin_mask);
  QString caption = tr("Quantum GIS - ");
  caption += QString("%1 ('%2')").arg(QGis::qgisVersion).arg(QGis::qgisReleaseName);
  setCaption(caption);
  mMapCursor = 0;
  // create the interfce
  mQgisInterface = new QgisIface(this);
  // set the legend control for the map canvas
  mMapCanvas->setLegend(mMapLegend);


  //
  // Create the plugin registry and load plugins
  //
  // Get pointer to the provider registry singleton
  QString plib = QgsApplication::pluginPath();
  // set the provider plugin path
  mProviderRegistry = QgsProviderRegistry::instance(plib);
#ifdef QGISDEBUG
  std::cout << "Plugins and providers are installed in " << plib.toLocal8Bit().data() << std::endl;
#endif
  // load any plugins that were running in the last session
  mSplash->showMessage("Restoring loaded plugins", Qt::AlignHCenter | Qt::AlignBottom);
  qApp->processEvents();
  restoreSessionPlugins(plib);

  /* Delete this I think - Tim - FIXME 
  //
  // Create the map layer registry. Any layers loaded will
  // register themselves here. Deleting layers shouls ONLY
  // be done by means of a removeMapLayer call to the regsitry.
  // This allows a single layer instance to be shared
  // between more than one canvas. The registry is a singleton
  // and is constructed using the static instance call.

  // syntactic sugar shortcut for instance handle
  //QgsMapLayerRegistry * mapLayerRegistry = QgsMapLayerRegistry::instance();
  */

  // Map composer
  mComposer = new QgsComposer(this);

  mSplash->showMessage("Initializing file filters", Qt::AlignHCenter | Qt::AlignBottom);
  qApp->processEvents();
  // now build vector file filter
  buildSupportedVectorFileFilter_( mVectorFileFilter );

  // now build raster file filter
  QgsRasterLayer::buildSupportedRasterFileFilter( mRasterFileFilter );

  // Do this last in the ctor to ensure that all members are instantiated properly
  setupConnections();
  //
  // Please make sure this is the last thing the ctor does so that we can ensure teh 
  // widgets are all initialised before trying to restore their state.
  //
  mSplash->showMessage("Restoring window state", Qt::AlignHCenter | Qt::AlignBottom);
  qApp->processEvents();
  restoreWindowState();

  mSplash->showMessage("QGIS Ready!", Qt::AlignHCenter | Qt::AlignBottom);
  qApp->processEvents();
} // QgisApp ctor



QgisApp::~QgisApp()
{}

// restore any application settings stored in QSettings
void QgisApp::readSettings()
{
  // get the application dir
  mAppDir = QgsApplication::prefixPath();

  QSettings settings;
  // get the users theme preference from the settings
  mThemeName = settings.readEntry("/Themes","default");


  // Set the initial visibility flag for layers
  // This user option allows the user to turn off inital drawing of
  // layers when they are added to the map. This is useful when adding
  // many layers and the user wants to adjusty symbology, etc prior to
  // actually viewing the layer.
  mAddedLayersHidden = settings.readBoolEntry("/qgis/new_layers_visible", true);

  // Add the recently accessed project file paths to the File menu
  mRecentProjectPaths = settings.readListEntry("/UI/recentProjectsList");

}


//////////////////////////////////////////////////////////////////////
//            Set Up the gui toolbars, menus, statusbar etc 
//////////////////////////////////////////////////////////////////////            

void QgisApp::createActions()
{
  QString myIconPath = QgsApplication::themePath();
  //
  // File Menu Related Items
  //
  mActionFileNew= new QAction(QIcon(myIconPath+"/mActionFileNew.png"), tr("&New Project..."), this);
  mActionFileNew->setShortcut(tr("Ctrl+N"));
  mActionFileNew->setStatusTip(tr("New Project"));
  connect(mActionFileNew, SIGNAL(triggered()), this, SLOT(fileNew()));
  //
  mActionFileOpen= new QAction(QIcon(myIconPath+"/mActionFileOpen.png"), tr("&Open Project..."), this);
  mActionFileOpen->setShortcut(tr("Ctrl+O"));
  mActionFileOpen->setStatusTip(tr("Open a Project"));
  connect(mActionFileOpen, SIGNAL(triggered()), this, SLOT(fileOpen()));
  //
  mActionFileSave= new QAction(QIcon(myIconPath+"/mActionFileSave.png"), tr("&Save Project"), this);
  mActionFileSave->setShortcut(tr("Ctrl+O"));
  mActionFileSave->setStatusTip(tr("Open a Project"));
  connect(mActionFileSave, SIGNAL(triggered()), this, SLOT(fileSave()));
  //
  mActionFileSaveAs= new QAction(QIcon(myIconPath+"/mActionFileSaveAs.png"), tr("Save Project &As..."), this);
  mActionFileSaveAs->setShortcut(tr("Ctrl+A"));
  mActionFileSaveAs->setStatusTip(tr("Save Project under a new name"));
  connect(mActionFileSaveAs, SIGNAL(triggered()), this, SLOT(fileSaveAS()));
  //
  mActionFilePrint= new QAction(QIcon(myIconPath+"/mActionFilePrint.png"), tr("&Print"), this);
  mActionFilePrint->setShortcut(tr("Ctrl+P"));
  mActionFilePrint->setStatusTip(tr("Print"));
  connect(mActionFilePrint, SIGNAL(triggered()), this, SLOT(filePrint()));
  //
  mActionSaveMapAsImage= new QAction(QIcon(myIconPath+"/mActionSaveMapAsImage.png"), tr("Save as Image"), this);
  mActionSaveMapAsImage->setShortcut(tr("I"));
  mActionSaveMapAsImage->setStatusTip(tr("Save map as image"));
  connect(mActionSaveMapAsImage, SIGNAL(triggered()), this, SLOT(saveMapAsImage()));
  //
  mActionExportMapServer= new QAction(QIcon(myIconPath+"/mActionExportMapServer.png"), tr("Export to MapServer Map"), this);
  mActionExportMapServer->setShortcut(tr("M"));
  mActionExportMapServer->setStatusTip(tr("Export as MapServer .map file"));
  connect(mActionExportMapServer, SIGNAL(triggered()), this, SLOT(exportMapServer()));
  //
  mActionFileExit= new QAction(QIcon(myIconPath+"/mActionFileExit.png"), tr("Exit"), this);
  mActionFileExit->setShortcut(tr("Ctrl+Q"));
  mActionFileExit->setStatusTip(tr("Exit QGIS"));
  connect(mActionFileExit, SIGNAL(triggered()), qApp, SLOT(quit()));
  //
  // Layer Menu Related Items
  //
  mActionAddNonDbLayer= new QAction(QIcon(myIconPath+"/mActionAddNonDbLayer.png"), tr("Add a Vector Layer"), this);
  mActionAddNonDbLayer->setShortcut(tr("V"));
  mActionAddNonDbLayer->setStatusTip(tr("Add a Vector Layer"));
  connect(mActionAddNonDbLayer, SIGNAL(triggered()), this, SLOT(addLayer()));
  //
  mActionAddRasterLayer= new QAction(QIcon(myIconPath+"/mActionAddRasterLayer.png"), tr("Add a Raster Layer"), this);
  mActionAddRasterLayer->setShortcut(tr("R"));
  mActionAddRasterLayer->setStatusTip(tr("Add a Raster Layer"));
  assert(connect(mActionAddRasterLayer, SIGNAL(triggered()), this, SLOT(addRasterLayer())));
  //
  mActionAddLayer= new QAction(QIcon(myIconPath+"/mActionAddLayer.png"), tr("Add a PostGIS Layer"), this);
  mActionAddLayer->setShortcut(tr("D"));
  mActionAddLayer->setStatusTip(tr("Add a PostGIS Layer"));
//#ifdef HAVE_POSTGRESQL
//  std::cout << "HAVE_POSTGRESQL is defined" << std::endl; 
//  assert(0);
//#else
//  std::cout << "HAVE_POSTGRESQL not defined" << std::endl; 
//  assert(0);
//#endif
  assert(connect(mActionAddLayer, SIGNAL(triggered()), this, SLOT(addDatabaseLayer())));
  //
  mActionNewVectorLayer= new QAction(QIcon(myIconPath+"/mActionNewVectorLayer.png"), tr("New Vector Layer"), this);
  mActionNewVectorLayer->setShortcut(tr("N"));
  mActionNewVectorLayer->setStatusTip(tr("Create a New Vector Layer"));
  connect(mActionNewVectorLayer, SIGNAL(triggered()), this, SLOT(newVectorLayer()));
  //
  mActionRemoveLayer= new QAction(QIcon(myIconPath+"/mActionRemoveLayer.png"), tr("Remove Layer"), this);
  mActionRemoveLayer->setShortcut(tr("Ctrl+D"));
  mActionRemoveLayer->setStatusTip(tr("Remove a Layer"));
  connect(mActionRemoveLayer, SIGNAL(triggered()), this, SLOT(removeLayer()));
  //
  mActionAddAllToOverview= new QAction(QIcon(myIconPath+"/mActionAddAllToOverview.png"), tr("Add All To Overview"), this);
  mActionAddAllToOverview->setShortcut(tr("+"));
  mActionAddAllToOverview->setStatusTip(tr("Show all layers in the overview map"));
  connect(mActionAddAllToOverview, SIGNAL(triggered()), this, SLOT(addAllToOverview()));
  //
  mActionRemoveAllFromOverview= new QAction(QIcon(myIconPath+"/mActionRemoveAllFromOverview.png"), tr("Remove All From Overview"), this);
  mActionRemoveAllFromOverview->setShortcut(tr("-"));
  mActionRemoveAllFromOverview->setStatusTip(tr("Remove all layers from overview map"));
  connect(mActionRemoveAllFromOverview, SIGNAL(triggered()), this, SLOT(removeAllFromOverView()));
  //
  mActionShowAllLayers= new QAction(QIcon(myIconPath+"/mActionShowAllLayers.png"), tr("Show All Layers"), this);
  mActionShowAllLayers->setShortcut(tr("S"));
  mActionShowAllLayers->setStatusTip(tr("Show all layers"));
  connect(mActionShowAllLayers, SIGNAL(triggered()), this, SLOT(showAllLayers()));
  //
  mActionHideAllLayers= new QAction(QIcon(myIconPath+"/mActionHideAllLayers.png"), tr("Hide All Layers"), this);
  mActionHideAllLayers->setShortcut(tr("H"));
  mActionHideAllLayers->setStatusTip(tr("Hide all layers"));
  connect(mActionHideAllLayers, SIGNAL(triggered()), this, SLOT(hideAllLayers()));
  //
  // Settings Menu Related Items
  //
  mActionProjectProperties= new QAction(QIcon(myIconPath+"/mActionProjectProperties.png"), tr("Project Properties"), this);
  mActionProjectProperties->setShortcut(tr("Alt+P"));
  mActionProjectProperties->setStatusTip(tr("Set project properties"));
  connect(mActionProjectProperties, SIGNAL(triggered()), this, SLOT(projectProperties()));
  //
  mActionOptions= new QAction(QIcon(myIconPath+"/mActionOptions.png"), tr("QGIS Options"), this);
  mActionOptions->setShortcut(tr("Alt+O"));
  mActionOptions->setStatusTip(tr("Change various QGIS options"));
  connect(mActionOptions, SIGNAL(triggered()), this, SLOT(options()));
  //
  mActionCustomProjection= new QAction(QIcon(myIconPath+"/mActionCustomProjection.png"), tr("Custom Projection"), this);
  mActionCustomProjection->setShortcut(tr("Alt+I"));
  mActionCustomProjection->setStatusTip(tr("Manage custom projections"));
  connect(mActionCustomProjection, SIGNAL(triggered()), this, SLOT(customProjection()));
  //
  // Help Menu Related items
  //
  mActionHelpContents= new QAction(QIcon(myIconPath+"/mActionHelpContents.png"), tr("Help Contents"), this);
  mActionHelpContents->setShortcut(tr("F1"));
  mActionHelpContents->setStatusTip(tr("Help Documentation"));
  connect(mActionHelpContents, SIGNAL(triggered()), this, SLOT(helpContents()));
  //
  mActionQgisHomePage= new QAction(QIcon(myIconPath+"/mActionQgisHomePage.png"), tr("Qgis Home Page"), this);
  mActionQgisHomePage->setShortcut(tr("Ctrl+H"));
  mActionQgisHomePage->setStatusTip(tr("QGIS Home Page"));
  connect(mActionQgisHomePage, SIGNAL(triggered()), this, SLOT(helpQgisHomePage()));
  //
  mActionQgisSourceForgePage= new QAction(QIcon(myIconPath+"/mActionQgisSourceForgePage.png"), tr("Qgis Sourceforge"), this);
  mActionQgisSourceForgePage->setShortcut(tr("Alt+H"));
  mActionQgisSourceForgePage->setStatusTip(tr("Visit QGIS SourceForge page"));
  connect(mActionQgisSourceForgePage, SIGNAL(triggered()), this, SLOT(helpQgisSourceForge()));
  //
  mActionHelpAbout= new QAction(QIcon(myIconPath+"/mActionHelpAbout.png"), tr("About"), this);
  mActionHelpAbout->setStatusTip(tr("About QGIS"));
  connect(mActionHelpAbout, SIGNAL(triggered()), this, SLOT(about()));
  //
  mActionCheckQgisVersion= new QAction(QIcon(myIconPath+"/mActionCheckQgisVersion.png"), tr("Check Qgis Version"), this);
  mActionCheckQgisVersion->setStatusTip(tr("Check if your QGIS version is up to date (requires internet access)"));
  connect(mActionCheckQgisVersion, SIGNAL(triggered()), this, SLOT(checkQgisVersion()));
  // 
  // View Menu Items
  //
  mActionDraw= new QAction(QIcon(myIconPath+"/mActionDraw.png"), tr("Refresh"), this);
  mActionDraw->setShortcut(tr("Ctrl+R"));
  mActionDraw->setStatusTip(tr("Refresh Map"));
  connect(mActionDraw, SIGNAL(triggered()), this, SLOT(drawLayers()));
  //
  mActionZoomIn= new QAction(QIcon(myIconPath+"/mActionZoomIn.png"), tr("Zoom In"), this);
  mActionZoomIn->setShortcut(tr("z"));
  mActionZoomIn->setStatusTip(tr("Zoom In"));
  connect(mActionZoomIn, SIGNAL(triggered()), this, SLOT(zoomIn()));
  //
  mActionZoomOut= new QAction(QIcon(myIconPath+"/mActionZoomOut.png"), tr("Zoom Out"), this);
  mActionZoomOut->setShortcut(tr("Z"));
  mActionZoomOut->setStatusTip(tr("Zoom Out"));
  connect(mActionZoomOut, SIGNAL(triggered()), this, SLOT(zoomOut()));
  //
  mActionZoomFullExtent= new QAction(QIcon(myIconPath+"/mActionZoomFullExtent.png"), tr("Zoom Full"), this);
  mActionZoomFullExtent->setShortcut(tr("f"));
  mActionZoomFullExtent->setStatusTip(tr("Zoom to Full Extents"));
  connect(mActionZoomFullExtent, SIGNAL(triggered()), this, SLOT(zoomFull()));
  //
  mActionZoomToSelected= new QAction(QIcon(myIconPath+"/mActionZoomToSelected.png"), tr("Zoom To Selection"), this);
  mActionZoomToSelected->setShortcut(tr("F"));
  mActionZoomToSelected->setStatusTip(tr("Zoom to selection"));
  connect(mActionZoomToSelected, SIGNAL(triggered()), this, SLOT(zoomToSelected()));
  //
  mActionPan= new QAction(QIcon(myIconPath+"/mActionPan.png"), tr("Pan Map"), this);
  mActionPan->setStatusTip(tr("Pan the map"));
  connect(mActionPan, SIGNAL(triggered()), this, SLOT(pan()));
  //
  mActionZoomLast= new QAction(QIcon(myIconPath+"/mActionZoomLast.png"), tr("Zoom Last"), this);
  //mActionZoomLast->setShortcut(tr("Ctrl+O"));
  mActionZoomLast->setStatusTip(tr("Zoom to Last Extent"));
  connect(mActionZoomLast, SIGNAL(triggered()), this, SLOT(zoomPrevious()));
  //
  mActionZoomToLayer= new QAction(QIcon(myIconPath+"/mActionZoomToLayer.png"), tr("Zoom To Layer"), this);
  //mActionZoomToLayer->setShortcut(tr("Ctrl+O"));
  mActionZoomToLayer->setStatusTip(tr("Zoom to Layer"));
  connect(mActionZoomToLayer, SIGNAL(triggered()), this, SLOT(zoomToLayerExtent()));
  //
  mActionIdentify= new QAction(QIcon(myIconPath+"/mActionIdentify.png"), tr("Identify Features"), this);
  mActionIdentify->setShortcut(tr("I"));
  mActionIdentify->setStatusTip(tr("Click on features to identify them"));
  connect(mActionIdentify, SIGNAL(triggered()), this, SLOT(identify()));
  //
  mActionSelect= new QAction(QIcon(myIconPath+"/mActionSelect.png"), tr("Select Features"), this);
  mActionSelect->setStatusTip(tr("Select Features"));
  connect(mActionSelect, SIGNAL(triggered()), this, SLOT(select()));
  //
  mActionOpenTable= new QAction(QIcon(myIconPath+"/mActionOpenTable.png"), tr("Open Table"), this);
  //mActionOpenTable->setShortcut(tr("Ctrl+O"));
  mActionOpenTable->setStatusTip(tr("Open Table"));
  connect(mActionOpenTable, SIGNAL(triggered()), this, SLOT(attributeTable()));
  //
  mActionMeasure= new QAction(QIcon(myIconPath+"/mActionMeasure.png"), tr("Measure Line "), this);
  mActionMeasure->setShortcut(tr("Ctrl+M"));
  mActionMeasure->setStatusTip(tr("Measure a Line"));
  connect(mActionMeasure, SIGNAL(triggered()), this, SLOT(measure()));
  //
  mActionMeasureArea= new QAction(QIcon(myIconPath+"/mActionMeasureArea.png"), tr("Measure Area"), this);
  mActionMeasureArea->setShortcut(tr("Alt+M"));
  mActionMeasureArea->setStatusTip(tr("Measure an Area"));
  connect(mActionMeasureArea, SIGNAL(triggered()), this, SLOT(measureArea()));
  //
  mActionShowBookmarks= new QAction(QIcon(myIconPath+"/mActionShowBookmarks.png"), tr("Show Bookmark"), this);
  mActionShowBookmarks->setShortcut(tr("Ctrl+B"));
  mActionShowBookmarks->setStatusTip(tr("Show Bookmarks"));
  connect(mActionShowBookmarks, SIGNAL(triggered()), this, SLOT(showBookmarks()));
  //
  mActionNewBookmark= new QAction(QIcon(myIconPath+"/mActionNewBookmark.png"), tr("New Bookmark"), this);
  mActionNewBookmark->setShortcut(tr("Alt+B"));
  mActionNewBookmark->setStatusTip(tr("New Bookmark"));
  connect(mActionNewBookmark, SIGNAL(triggered()), this, SLOT(newBookmark()));
  //
  mActionAddWmsLayer= new QAction(QIcon(myIconPath+"/mActionAddWmsLayer.png"), tr("Add Wms Layer"), this);
  mActionAddWmsLayer->setShortcut(tr("Ctrl+M"));
  mActionAddWmsLayer->setStatusTip(tr("Add Web Mapping Server Layer"));
  connect(mActionAddWmsLayer, SIGNAL(triggered()), this, SLOT(addWmsLayer()));
  //
  mActionInOverview= new QAction(QIcon(myIconPath+"/mActionInOverview.png"), tr("In Overview"), this);
  mActionInOverview->setShortcut(tr("Alt+O"));
  mActionInOverview->setStatusTip(tr("Add current layer to overview map"));
  connect(mActionInOverview, SIGNAL(triggered()), this, SLOT(inOverview()));
  //
  // Plugin Menu Related Items
  //
  mActionShowPluginManager= new QAction(QIcon(myIconPath+"/mActionShowPluginManager.png"), tr("Plugin Manager"), this);
  mActionShowPluginManager->setShortcut(tr("Ctrl+P"));
  mActionShowPluginManager->setStatusTip(tr("Open the plugin manager"));
  connect(mActionShowPluginManager, SIGNAL(triggered()), this, SLOT(showPluginManager()));
  //
  // Add the whats this toolbar button
  // QWhatsThis::whatsThisButton(mHelpToolBar);
  // 
  //
  // Digitising Toolbar Items
  //
  mActionCapturePoint= new QAction(QIcon(myIconPath+"/mActionCapturePoint.png"), tr("Capture Point"), this);
  mActionCapturePoint->setShortcut(tr("."));
  mActionCapturePoint->setStatusTip(tr("Capture Points"));
  connect(mActionCapturePoint, SIGNAL(triggered()), this, SLOT(capturePoint()));
  //
  mActionCaptureLine= new QAction(QIcon(myIconPath+"/mActionCaptureLine.png"), tr("Capture Line"), this);
  mActionCaptureLine->setShortcut(tr("/"));
  mActionCaptureLine->setStatusTip(tr("Capture Lines"));
  connect(mActionCaptureLine, SIGNAL(triggered()), this, SLOT(captureLine()));
  //
  mActionCapturePolygon= new QAction(QIcon(myIconPath+"/mActionCapturePolygon.png"), tr("Capture Polygon"), this);
  mActionCapturePolygon->setShortcut(tr("Ctrl+/"));
  mActionCapturePolygon->setStatusTip(tr("Capture Polygons"));
  connect(mActionCapturePolygon, SIGNAL(triggered()), this, SLOT(capturePolygon()));
  //
  mActionDeleteSelected = new QAction(QIcon(myIconPath+"/mActionDeleteSelected.png"), tr("Delete Seleced"), this);
  mActionDeleteSelected->setStatusTip(tr("Delete Selected"));
  connect(mActionDeleteSelected, SIGNAL(triggered()), this, SLOT(deleteSelected()));
  //
  mActionAddVertex = new QAction(QIcon(myIconPath+"/mActionAddVertex.png"), tr("Add Vertex"), this);
  mActionAddVertex->setStatusTip(tr("Add Vertex"));
  connect(mActionAddVertex, SIGNAL(triggered()), this, SLOT(addVertex()));
  //
  mActionDeleteVertex = new QAction(QIcon(myIconPath+"/mActionDeleteVertex.png"), tr("Delete Vertex"), this);
  mActionDeleteVertex->setStatusTip(tr("Delete Vertex"));
  connect(mActionDeleteVertex, SIGNAL(triggered()), this, SLOT(deleteVertex()));
  //
  mActionMoveVertex = new QAction(QIcon(myIconPath+"/mActionMoveVertex.png"), tr("Move Vertex"), this);
  mActionMoveVertex->setStatusTip(tr("Move Vertex"));
  connect(mActionMoveVertex, SIGNAL(triggered()), this, SLOT(moveVertex()));
}

void QgisApp::createActionGroups()
{
  //
  // Map Tool Group
  mMapToolGroup = new QActionGroup(this);
  mActionPan->setCheckable(true);
  mMapToolGroup->addAction(mActionPan);
  mActionZoomIn->setCheckable(true);
  mMapToolGroup->addAction(mActionZoomIn);
  mActionZoomOut->setCheckable(true);
  mMapToolGroup->addAction(mActionZoomOut);
  mActionIdentify->setCheckable(true);
  mMapToolGroup->addAction(mActionIdentify);
  mActionSelect->setCheckable(true);
  mMapToolGroup->addAction(mActionSelect);
  mActionMeasure->setCheckable(true);
  mMapToolGroup->addAction(mActionMeasure);
  mActionMeasureArea->setCheckable(true);
  mMapToolGroup->addAction(mActionMeasureArea);
  mActionCaptureLine->setCheckable(true);
  mMapToolGroup->addAction(mActionCaptureLine);
  mActionCapturePoint->setCheckable(true);
  mMapToolGroup->addAction(mActionCapturePoint);
  mActionCapturePolygon->setCheckable(true);
  mMapToolGroup->addAction(mActionCapturePolygon);
  mMapToolGroup->addAction(mActionDeleteSelected);
  mActionAddVertex->setCheckable(true);
  mMapToolGroup->addAction(mActionAddVertex);
  mActionDeleteVertex->setCheckable(true);
  mMapToolGroup->addAction(mActionDeleteVertex);
  mActionMoveVertex->setCheckable(true);
  mMapToolGroup->addAction(mActionMoveVertex);
}

void QgisApp::createMenus()
{
  //
  // File Menu
  mFileMenu = menuBar()->addMenu(tr("&File"));
  mFileMenu->addAction(mActionFileNew);
  mFileMenu->addAction(mActionFileOpen);
  mRecentProjectsMenu = mFileMenu->addMenu("&Open Recent Projects");
  mFileMenu->addSeparator();
  mFileMenu->addAction(mActionFileSave);
  mFileMenu->addAction(mActionFileSaveAs);
  mFileMenu->addAction(mActionSaveMapAsImage);
  mFileMenu->addAction(mActionExportMapServer);
  mFileMenu->addAction(mActionFilePrint);
  mFileMenu->addSeparator();
  mFileMenu->addAction(mActionFileExit);

  //
  // View Menu
  mViewMenu = menuBar()->addMenu(tr("&View"));
  mViewMenu->addAction(mActionZoomFullExtent);
  mViewMenu->addAction(mActionZoomToSelected);
  mViewMenu->addAction(mActionZoomToLayer);
  mViewMenu->addAction(mActionZoomLast);
  mViewMenu->addAction(mActionDraw);
  mViewMenu->addAction(mActionShowBookmarks);
  mViewMenu->addAction(mActionNewBookmark);
    
  //
  // Layers Menu
  mLayerMenu = menuBar()->addMenu(tr("&Layer"));
  mLayerMenu->addAction(mActionAddNonDbLayer);
  mLayerMenu->addAction(mActionAddRasterLayer);
#ifdef HAVE_POSTGRESQL
  mLayerMenu->addAction(mActionAddLayer);
#endif
  mLayerMenu->addAction(mActionAddWmsLayer);
  mLayerMenu->addAction(mActionRemoveLayer);
  mLayerMenu->addAction(mActionNewVectorLayer);
  mLayerMenu->addAction(mActionInOverview);
  mLayerMenu->addAction(mActionAddAllToOverview);
  mLayerMenu->addAction(mActionRemoveAllFromOverview);
  mLayerMenu->addAction(mActionHideAllLayers);
  mLayerMenu->addAction(mActionShowAllLayers);

  //
  // Settings Menu
  mSettingsMenu = menuBar()->addMenu(tr("&Settings"));
  mSettingsMenu->addAction(mActionProjectProperties);
  mSettingsMenu->addAction(mActionCustomProjection);
  mSettingsMenu->addAction(mActionOptions);
  
  //
  // Plugins Menu
  mPluginMenu = menuBar()->addMenu(tr("&Plugins"));
  mPluginMenu->addAction(mActionShowPluginManager);

  // Add the plugin manager action to it
  //actionPluginManager->addTo(mPluginMenu);
  // Add separator. Plugins will add their menus to this
  // menu when they are loaded by the plugin manager
  //mPluginMenu->insertSeparator();
  // Add to the menubar
  //menuBar()->insertItem(tr("&Plugins"), mPluginMenu, -1, menuBar()->count() - 1);

  menuBar()->addSeparator();
  mHelpMenu = menuBar()->addMenu(tr("&Help"));
  mHelpMenu->addAction(mActionHelpContents);
  mHelpMenu->addAction(mActionQgisHomePage);
  mHelpMenu->addAction(mActionQgisSourceForgePage);
  mHelpMenu->addAction(mActionCheckQgisVersion);
  mHelpMenu->addAction(mActionHelpAbout);
}

void QgisApp::createToolBars()
{
  // Note: we need to set each object name to ensure that
  // qmainwindow::saveState and qmainwindow::restoreState
  // work properly
  
  //
  // File Toolbar
  mFileToolBar = addToolBar(tr("File"));
  mFileToolBar->setIconSize(QSize(24,24));
  mFileToolBar->setObjectName("FileToolBar");
  mFileToolBar->addAction(mActionFileNew);
  mFileToolBar->addAction(mActionFileNew);
  mFileToolBar->addAction(mActionFileSave);
  mFileToolBar->addAction(mActionFileSaveAs);
  mFileToolBar->addAction(mActionFileOpen);
  mFileToolBar->addAction(mActionFilePrint);
  //
  // Layer Toolbar
  mLayerToolBar = addToolBar(tr("Manage Layers"));
  mLayerToolBar->setIconSize(QSize(24,24));
  mLayerToolBar->setObjectName("LayerToolBar");
  mLayerToolBar->addAction(mActionAddNonDbLayer);
  mLayerToolBar->addAction(mActionAddRasterLayer);
#ifdef HAVE_POSTGRESQL
  mLayerToolBar->addAction(mActionAddLayer);
#endif
  mLayerToolBar->addAction(mActionAddWmsLayer);
  mLayerToolBar->addAction(mActionNewVectorLayer);
  mLayerToolBar->addAction(mActionRemoveLayer);
  mLayerToolBar->addAction(mActionInOverview);
  mLayerToolBar->addAction(mActionAddAllToOverview);
  mLayerToolBar->addAction(mActionRemoveAllFromOverview);
  mLayerToolBar->addAction(mActionShowAllLayers);
  mLayerToolBar->addAction(mActionHideAllLayers);
  //
  // Help Toolbar
  mHelpToolBar = addToolBar(tr("Help"));
  mHelpToolBar->setIconSize(QSize(24,24));
  mHelpToolBar->setObjectName("Help");
  mHelpToolBar->addAction(mActionHelpContents);
  //
  // Digitizing Toolbar
  mDigitizeToolBar = addToolBar(tr("Digitizing"));
  mDigitizeToolBar->setIconSize(QSize(24,24));
  mDigitizeToolBar->setObjectName("Digitizing");
  mDigitizeToolBar->addAction(mActionCapturePoint);
  mDigitizeToolBar->addAction(mActionCaptureLine);
  mDigitizeToolBar->addAction(mActionCapturePolygon);
  mDigitizeToolBar->addAction(mActionDeleteSelected);
  mDigitizeToolBar->addAction(mActionAddVertex);
  mDigitizeToolBar->addAction(mActionDeleteVertex);
  mDigitizeToolBar->addAction(mActionMoveVertex);
  //
  // Map Navigation Toolbar
  mMapNavToolBar = addToolBar(tr("Map Navigation"));
  mMapNavToolBar->setIconSize(QSize(24,24));
  mMapNavToolBar->setObjectName("Map Navigation");
  mMapNavToolBar->addAction(mActionPan);
  mMapNavToolBar->addAction(mActionZoomIn);
  mMapNavToolBar->addAction(mActionZoomOut);
  mMapNavToolBar->addAction(mActionZoomFullExtent);
  mMapNavToolBar->addAction(mActionZoomToSelected);
  mMapNavToolBar->addAction(mActionZoomToLayer);
  mMapNavToolBar->addAction(mActionZoomLast);
  mMapNavToolBar->addAction(mActionDraw);
  //
  // Attributes Toolbar
  mAttributesToolBar = addToolBar(tr("Attributes"));
  mAttributesToolBar->setIconSize(QSize(24,24));
  mAttributesToolBar->setObjectName("Attributes");
  mAttributesToolBar->addAction(mActionIdentify);
  mAttributesToolBar->addAction(mActionSelect);
  mAttributesToolBar->addAction(mActionOpenTable);
  mAttributesToolBar->addAction(mActionMeasure);
  mAttributesToolBar->addAction(mActionMeasureArea);
  mAttributesToolBar->addAction(mActionShowBookmarks);
  mAttributesToolBar->addAction(mActionNewBookmark);
  //
  // Plugins Toolbar
  mPluginToolBar = addToolBar(tr("Plugins"));
  mPluginToolBar->setIconSize(QSize(24,24));
  mPluginToolBar->setObjectName("Plugins");
}

void QgisApp::createStatusBar()
{
  //
  // Add a panel to the status bar for the scale, coords and progress
  // And also rendering suppression checkbox
  //
  mProgressBar = new QProgressBar(statusBar());
  mProgressBar->setMaximumWidth(100);
  QWhatsThis::add(mProgressBar, tr("Progress bar that displays the status of rendering layers and other time-intensive operations"));
  statusBar()->addWidget(mProgressBar, 1,true);
  // Bumped the font up one point size since 8 was too 
  // small on some platforms. A point size of 9 still provides
  // plenty of display space on 1024x768 resolutions
  QFont myFont( "Arial", 9 );
  statusBar()->setFont(myFont);
  mScaleLabel = new QLabel(QString("Scale"),statusBar());
  mScaleLabel->setFont(myFont);
  mScaleLabel->setMinimumWidth(100);
  QWhatsThis::add(mScaleLabel, tr("Displays the current map scale"));
  statusBar()->addWidget(mScaleLabel, 0,true);
  //coords status bar widget
  mCoordsLabel = new QLabel(QString("Coordinates:"), statusBar());
  mCoordsLabel->setMinimumWidth(200);
  mCoordsLabel->setFont(myFont);
  QWhatsThis::add(mCoordsLabel, tr("Shows the map coordinates at the current cursor postion. The display is continuously updated as the mouse is moved."));
  statusBar()->addWidget(mCoordsLabel, 0, true);
  // render suppression status bar widget
  mRenderSuppressionCBox = new QCheckBox(tr("Render"),statusBar());
  mRenderSuppressionCBox->setChecked(true);
  mRenderSuppressionCBox->setFont(myFont);
  QWhatsThis::add(mRenderSuppressionCBox, tr("When checked, the map layers are rendered in response to map navigation commands and other events. When not checked, no rendering is done. This allows you to add a large number of layers and symbolize them before rendering."));
  QToolTip::add( mRenderSuppressionCBox, tr("Toggle map rendering") );
  statusBar()->addWidget(mRenderSuppressionCBox,0,true);
  // On the fly projection status bar icon
  // Changed this to a tool button since a QPushButton is
  // sculpted on OS X and the icon is never displayed [gsherman]
  mOnTheFlyProjectionStatusButton = new QToolButton(statusBar());
  mOnTheFlyProjectionStatusButton->setMaximumWidth(20);
  // Maintain uniform widget height in status bar by setting button height same as labels
  // For Qt/Mac 3.3, the default toolbutton height is 30 and labels were expanding to match
  mOnTheFlyProjectionStatusButton->setMaximumHeight(mScaleLabel->height());
  QPixmap myProjPixmap;
  QString myIconPath = QgsApplication::themePath();
  myProjPixmap.load(myIconPath+"/mIconProjectionDisabled.png");
  qDebug ("Icon disabled projection icon to: " + myIconPath.toLocal8Bit()+"/mIconProjectionDisabled.png");
  mOnTheFlyProjectionStatusButton->setPixmap(myProjPixmap);
  assert(!myProjPixmap.isNull());
  QWhatsThis::add(mOnTheFlyProjectionStatusButton, tr("This icon shows whether on the fly projection is enabled or not. Click the icon to bring up the project properties dialog to alter this behaviour."));
  QToolTip::add( mOnTheFlyProjectionStatusButton, tr("Projection status - Click to open projection dialog"));
  connect(mOnTheFlyProjectionStatusButton, SIGNAL(clicked()),
      this, SLOT(projectPropertiesProjections()));//bring up the project props dialog when clicked
  statusBar()->addWidget(mOnTheFlyProjectionStatusButton,0,true);
  statusBar()->showMessage(tr("Ready"));
}

QString QgisApp::themePath()
{
  return mAppDir +"/share/qgis/themes/" + mThemeName + "/";
}


void QgisApp::setTheme(QString theThemeName)
{
  /*****************************************************************
  // Init the toolbar icons by setting the icon for each action.
  // All toolbar/menu items must be a QAction in order for this
  // to work.
  //
  // When new toolbar/menu QAction objects are added to the interface,
  // add an entry below to set the icon
  //
  // PNG names must match those defined for the default theme. The
  // default theme is installed in <prefix>/share/qgis/themes/default.
  //
  // New core themes can be added by creating a subdirectory under src/themes
  // and modifying the appropriate Makefile.am files. User contributed themes
  // will be installed directly into <prefix>/share/qgis/themes/<themedir>.
  //
  // Themes can be selected from the preferences dialog. The dialog parses
  // the themes directory and builds a list of themes (ie subdirectories)
  // for the user to choose from.
  //
  // TODO: Check as each icon is grabbed and if it doesn't exist, use the
  // one from the default theme (which is installed with qgis and should
  // always be good)
  */
  QgsApplication::selectTheme(theThemeName);
  QString myIconPath = QgsApplication::themePath();
  mActionFileNew->setIconSet(QIcon(QPixmap(myIconPath + "/mActionFileNew.png")));
  mActionFileSave->setIconSet(QIcon(QPixmap(myIconPath + "/mActionFileSave.png")));
  mActionFileSaveAs->setIconSet(QIcon(QPixmap(myIconPath + "/mActionFileSaveAs.png")));
  mActionFileOpen->setIconSet(QIcon(QPixmap(myIconPath + "/mActionFileOpen.png")));
  mActionFilePrint->setIconSet(QIcon(QPixmap(myIconPath + "/mActionFilePrint.png")));
  mActionSaveMapAsImage->setIconSet(QIcon(QPixmap(myIconPath + "/mActionSaveMapAsImage.png")));
  mActionExportMapServer->setIconSet(QIcon(QPixmap(myIconPath + "/mActionExportMapServer.png")));
  mActionFileExit->setIconSet(QIcon(QPixmap(myIconPath + "/mActionFileExit.png")));
  mActionAddNonDbLayer->setIconSet(QIcon(QPixmap(myIconPath + "/mActionAddNonDbLayer.png")));
  mActionAddRasterLayer->setIconSet(QIcon(QPixmap(myIconPath + "/mActionAddRasterLayer.png")));
  mActionAddLayer->setIconSet(QIcon(QPixmap(myIconPath + "/mActionAddLayer.png")));
  mActionRemoveLayer->setIconSet(QIcon(QPixmap(myIconPath + "/mActionRemoveLayer.png")));
  mActionNewVectorLayer->setIconSet(QIcon(QPixmap(myIconPath + "/mActionNewVectorLayer.png")));
  mActionAddAllToOverview->setIconSet(QIcon(QPixmap(myIconPath + "/mActionAddAllToOverview.png")));
  mActionHideAllLayers->setIconSet(QIcon(QPixmap(myIconPath + "/mActionHideAllLayers.png")));
  mActionShowAllLayers->setIconSet(QIcon(QPixmap(myIconPath + "/mActionShowAllLayers.png")));
  mActionRemoveAllFromOverview->setIconSet(QIcon(QPixmap(myIconPath + "/mActionRemoveAllFromOverview.png")));
  mActionProjectProperties->setIconSet(QIcon(QPixmap(myIconPath + "/mActionProjectProperties.png")));
  mActionShowPluginManager->setIconSet(QIcon(QPixmap(myIconPath + "/mActionShowPluginManager.png")));
  mActionCheckQgisVersion->setIconSet(QIcon(QPixmap(myIconPath + "/mActionCheckQgisVersion.png")));
  mActionOptions->setIconSet(QIcon(QPixmap(myIconPath + "/mActionOptions.png")));
  mActionHelpContents->setIconSet(QIcon(QPixmap(myIconPath + "/mActionHelpContents.png")));
  mActionQgisHomePage->setIconSet(QIcon(QPixmap(myIconPath + "/mActionQgisHomePage.png")));
  mActionQgisSourceForgePage->setIconSet(QIcon(QPixmap(myIconPath + "/mActionQgisSourceForgePage.png")));
  mActionHelpAbout->setIconSet(QIcon(QPixmap(myIconPath + "/mActionHelpAbout.png")));
  mActionDraw->setIconSet(QIcon(QPixmap(myIconPath + "/mActionDraw.png")));
  mActionCapturePoint->setIconSet(QIcon(QPixmap(myIconPath + "/mActionCapturePoint.png")));
  mActionCaptureLine->setIconSet(QIcon(QPixmap(myIconPath + "/mActionCaptureLine.png")));
  mActionCapturePolygon->setIconSet(QIcon(QPixmap(myIconPath + "/mActionCapturePolygon.png")));
  mActionZoomIn->setIconSet(QIcon(QPixmap(myIconPath + "/mActionZoomIn.png")));
  mActionZoomOut->setIconSet(QIcon(QPixmap(myIconPath + "/mActionZoomOut.png")));
  mActionZoomFullExtent->setIconSet(QIcon(QPixmap(myIconPath + "/mActionZoomFullExtent.png")));
  mActionZoomToSelected->setIconSet(QIcon(QPixmap(myIconPath + "/mActionZoomToSelected.png")));
  mActionPan->setIconSet(QIcon(QPixmap(myIconPath + "/mActionPan.png")));
  mActionZoomLast->setIconSet(QIcon(QPixmap(myIconPath + "/mActionZoomLast.png")));
  mActionZoomToLayer->setIconSet(QIcon(QPixmap(myIconPath + "/mActionZoomToLayer.png")));
  mActionIdentify->setIconSet(QIcon(QPixmap(myIconPath + "/mActionIdentify.png")));
  mActionSelect->setIconSet(QIcon(QPixmap(myIconPath + "/mActionSelect.png")));
  mActionOpenTable->setIconSet(QIcon(QPixmap(myIconPath + "/mActionOpenTable.png")));
  mActionMeasure->setIconSet(QIcon(QPixmap(myIconPath + "/mActionMeasure.png")));
  mActionMeasureArea->setIconSet(QIcon(QPixmap(myIconPath + "/mActionMeasureArea.png")));
  mActionShowBookmarks->setIconSet(QIcon(QPixmap(myIconPath + "/mActionShowBookmarks.png")));
  mActionNewBookmark->setIconSet(QIcon(QPixmap(myIconPath + "/mActionNewBookmark.png")));
  mActionCustomProjection->setIconSet(QIcon(QPixmap(myIconPath + "/mActionCustomProjection.png")));
  mActionAddWmsLayer->setIconSet(QIcon(QPixmap(myIconPath + "/mActionAddWmsLayer.png")));
  mActionInOverview->setIconSet(QIcon(QPixmap(myIconPath + "/mActionInOverview.png")));
}
void QgisApp::setupToolbarPopups(QString mThemeName)
{
  // Disabled by Tim for now during qt4 port - FIXME!!!!
  // 
  // Additional note by Tim. Popup tool icons are not nice UI
  // they are confusing for the user. We should get rid of using them
  // 

  //   23:25 < timlinux> I'm also proposing to do away with popup toolicons
  //   23:25 < timlinux> they are yuk from a ui perspective
  //   23:25 < timlinux> imho
  //   23:25 < gsherman> what are they?
  //   23:25 < timlinux> e.g. digitising tool icon
  //   23:25 < timlinux> where you have to hold and choose sub icon
  //   23:25 < timlinux> ala ffox back button
  //   23:26 < gsherman> that will clutter the ui to have all those floating around
  //   23:28 < timlinux> but they are no better than using menus to select the same actions
  //   23:28 < timlinux> .e.g. show / hide all in overview
  //   23:29 < timlinux> 2 clicks to achieve action from toolicon popup
  //   23:29 < timlinux> 2 clicks from menu
  //   23:29 < timlinux> so having it on icon bar offers no advantage
  //   23:29 < timlinux> be better to juditiously remove stuff from the icon bars imho
  //   23:30 < timlinux> ditto for show / hide all layers icon group
  //   23:31 < timlinux> and digitising icon should simply be changed based on the context
  //   23:31 < timlinux> e.g. if you start digitising a line layer, the digitising icon should reflect that lines will be digitised if line layer is active
  //   23:32 < timlinux> point and poly icons are then extraneaous


  /*
  // setup the toolbar popup menus that create a single toolbar icon
  // and a menu that is popped up when the mouse is held down (ala
  // the famous web browser Back button)
  QString myIconPath = mAppDir +"/share/qgis/themes/" + mThemeName;

  // Setup the overview tools
  // the toolbutton for the overview group:
  tbtnOverviewTools->setIconSet(QIcon(QPixmap(myIconPath + "/mActionAddAllToOverview.png")));
  toolPopupOverviews = new Q3PopupMenu();
  toolPopupOverviews->insertItem(QIcon(QPixmap(myIconPath + "/mActionAddAllToOverview.png")),
  tr("Add all layers to the overview map"),
  this, SLOT(addAllToOverview()));
  toolPopupOverviews->insertItem(QIcon(QPixmap(myIconPath + "/mActionRemoveAllFromOverview.png")),
  tr("Remove all layers from the overview map"),
  this, SLOT(removeAllFromOverview()));
  tbtnOverviewTools->setMenu(toolPopupOverviews);
  tbtnOverviewTools->setPopupMode(QToolButton::MenuButtonPopup);
  // connect the top overview tool to the appropriate slot
  connect(tbtnOverviewTools, SIGNAL(clicked()), this, SLOT(addAllToOverview()));

  // setup the add/remove all tools
  // the toolbutton for the display group:
  tbtnDisplayTools->setIconSet(QIcon(QPixmap(myIconPath + "/mActionShowAllLayers.png")));
  toolPopupDisplay = new Q3PopupMenu();
  toolPopupDisplay->insertItem(QIcon(QPixmap(myIconPath + "/mActionShowAllLayers.png")),
  tr("Show all layers"),
  this, SLOT(showAllLayers()));
  toolPopupDisplay->insertItem(QIcon(QPixmap(myIconPath + "/mActionHideAllLayers.png")),
  tr("Hide all layers"),
  this, SLOT(hideAllLayers()));
  tbtnDisplayTools->setMenu(toolPopupDisplay);
  tbtnDisplayTools->setPopupMode(QToolButton::MenuButtonPopup);
  // connect the top overview tool to the appropriate slot
  connect(tbtnDisplayTools, SIGNAL(clicked()), this, SLOT(showAllLayers()));

  // setup the capture (digitize) tools
  // the toolbutton for the capture group:
  tbtnCaptureTools->setIconSet(QIcon(QPixmap(myIconPath + "/mActionCapturePoint.png")));
  toolPopupCapture = new Q3PopupMenu();
  toolPopupCapture->insertItem(QIcon(QPixmap(myIconPath + "/mActionCapturePoint.png")),
  tr("Capture points"),
  this, SLOT(capturePoint()),0,0);
  toolPopupCapture->insertItem(QIcon(QPixmap(myIconPath + "/mActionCaptureLine.png")),
  tr("Capture lines"),
  this, SLOT(captureLine()),0,1);
  toolPopupCapture->insertItem(QIcon(QPixmap(myIconPath + "/mActionCapturePolygon.png")),
  tr("Capture polygons"),
  this, SLOT(capturePolygon()),0,2);
  toolPopupCapture->insertItem(QIcon(QPixmap(myIconPath + "/delete_selected.png")),
  tr("Delete selection"),
  this, SLOT(deleteSelected()),0,3);
  tbtnCaptureTools->setMenu(toolPopupCapture);
  tbtnCaptureTools->setPopupMode(QToolButton::MenuButtonPopup);
  */
}

void QgisApp::setupConnections()
{
  // connect the "cleanup" slot
  connect(qApp, SIGNAL(aboutToQuit()), this, SLOT(saveWindowState()));
  //connect the legend, mapcanvas and overview canvas to the registry
  connect(QgsMapLayerRegistry::instance(), 
      SIGNAL(layerWillBeRemoved(QString)), mMapCanvas, SLOT(remove(QString)));
  connect(QgsMapLayerRegistry::instance(), 
      SIGNAL(layerWillBeRemoved(QString)), mOverviewCanvas, SLOT(remove(QString)));
  connect(QgsMapLayerRegistry::instance(), 
      SIGNAL(removedAll()), mOverviewCanvas, SLOT(removeAll()));
  connect(QgsMapLayerRegistry::instance(), 
      SIGNAL(removedAll()), mMapCanvas, SLOT(removeAll()));
  connect(QgsMapLayerRegistry::instance(), 
      SIGNAL(layerWasAdded(QgsMapLayer*)), mMapCanvas, SLOT(addLayer(QgsMapLayer *)));
  connect(QgsMapLayerRegistry::instance(), 
      SIGNAL(layerWasAdded(QgsMapLayer*)), mOverviewCanvas, SLOT(addLayer(QgsMapLayer *)));
  //signal when mouse moved over window (coords display in status bar)
  connect(mMapCanvas, SIGNAL(xyCoordinates(QgsPoint &)), this, SLOT(showMouseCoordinate(QgsPoint &)));
  //signal when mouse in capturePoint mode and mouse clicked on canvas
  connect(mMapCanvas, SIGNAL(xyClickCoordinates(QgsPoint &)), this, SLOT(showCapturePointCoordinate(QgsPoint &)));
  connect(mMapCanvas, SIGNAL(setProgress(int,int)), this, SLOT(showProgress(int,int)));
  connect(mMapCanvas, SIGNAL(extentsChanged(QgsRect )),this,SLOT(showExtents(QgsRect )));
  connect(mMapCanvas, SIGNAL(scaleChanged(QString)), this, SLOT(showScale(QString)));
  connect(mMapCanvas, SIGNAL(scaleChanged(QString)), this, SLOT(updateMouseCoordinatePrecision()));
  connect(mMapCanvas, SIGNAL(addedLayer(QgsMapLayer *)), mMapLegend, SLOT(addLayer(QgsMapLayer *)));
  connect(mMapCanvas, SIGNAL(removedLayer(QString)), mMapLegend, SLOT(removeLayer(QString)));
  connect(mMapCanvas, SIGNAL(removedAll()), mMapLegend, SLOT(removeAll()));
  connect(mMapCanvas, SIGNAL(stopZoom()), this, SLOT(stopZoom()));
  connect(mMapLegend, SIGNAL(zOrderChanged(QgsLegend *)), mMapCanvas, SLOT(setZOrderFromLegend(QgsLegend *)));
  connect(mMapLegend, SIGNAL(zOrderChanged(QgsLegend *)), this, SLOT(setOverviewZOrder(QgsLegend *)));
  connect(mMapLegend, SIGNAL(currentChanged(Q3ListViewItem *)), this, SLOT(currentLayerChanged(Q3ListViewItem *)));

  connect(mRenderSuppressionCBox, SIGNAL(toggled(bool )), mMapCanvas, SLOT(setRenderFlag(bool)));
  connect(mRenderSuppressionCBox, SIGNAL(toggled(bool )), mOverviewCanvas, SLOT(setRenderFlag(bool)));
}
void QgisApp::createCanvas()
{
  // "theMapCanvas" used to find this canonical instance later
  mMapCanvas = new QgsMapCanvas(NULL, "theMapCanvas" );
  QWhatsThis::add(mMapCanvas, tr("Map canvas. This is where raster and vector layers are displayed when added to the map"));
  //set the canvas to the default background colour
  //the default can be set in qgisoptions 
  //use project properties to override the colour on a per project basis
  QSettings mySettings;
  int myRed = mySettings.value("/qgis/default_canvas_color_red",255).toInt();
  int myGreen = mySettings.value("/qgis/default_canvas_color_green",255).toInt();
  int myBlue = mySettings.value("/qgis/default_canvas_color_blue",255).toInt();
  mMapCanvas->setCanvasColor(QColor(myRed,myGreen,myBlue));  // this is the fill co;our when rendering
  mMapCanvas->setBackgroundColor(QColor(myRed,myGreen,myBlue)); // this is for the widget itself
  
  mMapCanvas->enableAntiAliasing(mySettings.value("/qgis/enable_anti_aliasing",false).toBool());
  mMapCanvas->setMinimumWidth(400);
  QVBoxLayout *myCanvasLayout = new QVBoxLayout;
  myCanvasLayout->addWidget(mMapCanvas);
  tabWidget->widget(0)->setLayout(myCanvasLayout);
  // set the focus to the map canvas
  mMapCanvas->setFocus();
}

void QgisApp::createOverview()
{
  // overview canvas
  mOverviewCanvas = new QgsMapOverviewCanvas(NULL, mMapCanvas);
  QWhatsThis::add(mOverviewCanvas, tr("Map overview canvas. This canvas can be used to display a locator map that shows the current extent of the map canvas. The current extent is shown as a red rectangle. Any layer on the map can be added to the overview canvas."));
  QBitmap overviewPanBmp(16, 16, pan_bits, true);
  QBitmap overviewPanBmpMask(16, 16, pan_mask_bits, true);
  mOverviewMapCursor = new QCursor(overviewPanBmp, overviewPanBmpMask, 5, 5);
  mOverviewCanvas->setCursor(*mOverviewMapCursor);
  QVBoxLayout *myOverviewLayout = new QVBoxLayout;
  myOverviewLayout->addWidget(mOverviewCanvas);
  overviewFrame->setLayout(myOverviewLayout);
}


void QgisApp::createLegend()
{
  //legend
  mMapLegend = new QgsLegend(this,NULL, "theMapLegend");
  mMapLegend->setMapCanvas(mMapCanvas);
  QWhatsThis::add(mMapLegend, tr("Map legend that displays all the layers currently on the map canvas. Click on the check box to turn a layer on or off. Double click on a layer in the legend to customize its appearance and set other properties."));
  QVBoxLayout *myLegendLayout = new QVBoxLayout;
  myLegendLayout->addWidget(mMapLegend);
  toolBox->widget(0)->setLayout(myLegendLayout);
  return;
}
void QgisApp::createDB()
{
  // Check qgis.db and make private copy if necessary
  QString qgisSettingsDirPath(QDir::homeDirPath () + "/.qgis/");
  QgsFile qgisPrivateDbFile(qgisSettingsDirPath + "qgis.db");

  // first we look for ~/.qgis/qgis.db
  if (!qgisPrivateDbFile.exists())
  {
    // if it doesnt exist we copy it in from the global resources dir
    QString qgisMasterDbFileName = QgsApplication::qgisMasterDbFilePath();

    // Must be sure there is destination directory ~/.qgis
    // @todo XXX REPLACE with recursive dir creator, but first define QgsDir class and
    // move i.e. makeDir from QgsBookmarks to QgsDir
    QDir destDir;
    destDir.mkdir(qgisSettingsDirPath, TRUE);

    //now copy the master file into the users .qgis dir
    bool isDbFileCopied = QgsFile::copy(qgisMasterDbFileName, qgisPrivateDbFile.name());

#ifdef QGISDEBUG
    if (!isDbFileCopied)
    {
      std::cout << "[ERROR] Can not make qgis.db private copy" << std::endl;
    }
#endif
  }
}

// Update file menu with the current list of recently accessed projects
void QgisApp::updateRecentProjectPaths()
{
  if (mRecentProjectPaths.size() > 0)
  {
    uint myId= 1;
    for ( QStringList::Iterator it = mRecentProjectPaths.begin();
        it != mRecentProjectPaths.end();
        ++it )
    {
      myId++;
      mRecentProjectsMenu->insertItem((*it), this, SLOT(openProject(int)),0, myId,myId);
      // This parameter corresponds to this path's index
      // in mRecentProjectPaths
      mFileMenu->setItemParameter(myId, myId - 1);
      // Disable this menu item if the file has been removed
      if (!QFile::exists((*it)))
      {
        mFileMenu->setItemEnabled(myId, FALSE);
      }
    }
  }
} // QgisApp::updateRecentProjectPaths

// add this file to the recently opened/saved projects list
void QgisApp::saveRecentProjectPath(QString projectPath, QSettings & settings)
{
  // Get canonical absolute path
  QFileInfo myFileInfo(projectPath);
  projectPath = myFileInfo.absFilePath();

  // If this file is already in the list, remove it
  mRecentProjectPaths.remove(projectPath);

  // Prepend this file to the list
  mRecentProjectPaths.prepend(projectPath);

  // Keep the list to 8 items by trimming excess off the bottom
  while (mRecentProjectPaths.count() > 8)
  {
    mRecentProjectPaths.pop_back();
  }

  // Persist the list
  settings.writeEntry("/UI/recentProjectsList", mRecentProjectPaths);

  // Remove existing paths from the recent projects menu
  for (uint i=mRecentProjectPaths.count();i>0;i--)
  {
    mRecentProjectsMenu->removeItemAt(i);
  }
  // Add back in the updated list of paths
  updateRecentProjectPaths();

} // QgisApp::saveRecentProjectPath

void QgisApp::saveWindowState()
{
  // store window and toolbar positions
  QSettings settings;
  // store the toolbar/dock widget settings using Qt4 settings API
  settings.setValue("/Geometry/state", this->saveState());

  // store window geometry
  QPoint p = this->pos();
  QSize s = this->size();
  settings.writeEntry("/Geometry/maximized", this->isMaximized());
  settings.writeEntry("/Geometry/x", p.x());
  settings.writeEntry("/Geometry/y", p.y());
  settings.writeEntry("/Geometry/w", s.width());
  settings.writeEntry("/Geometry/h", s.height());

}

void QgisApp::restoreWindowState()
{
  // restore the toolbar and dock widgets postions using Qt4 settings API
  QSettings settings;
  QVariant vstate = settings.value("/Geometry/state");
  this->restoreState(vstate.toByteArray());

  // restore window geometry
  QDesktopWidget *d = QApplication::desktop();
  int dw = d->width();          // returns desktop width
  int dh = d->height();         // returns desktop height
  int w = settings.readNumEntry("/Geometry/w", 600);
  int h = settings.readNumEntry("/Geometry/h", 400);
  int x = settings.readNumEntry("/Geometry/x", (dw - 600) / 2);
  int y = settings.readNumEntry("/Geometry/y", (dh - 400) / 2);
  resize(w, h);
  move(x, y);
}
///////////// END OF GUI SETUP ROUTINES ///////////////

void QgisApp::about()
{
     QApplication::setOverrideCursor(Qt::WaitCursor);
     QgsAbout *abt = new QgsAbout();
     QString versionString = tr("Version ");
     versionString += QGis::qgisVersion;
#ifdef HAVE_POSTGRESQL

versionString += tr(" with PostgreSQL support");
#else

versionString += tr(" (no PostgreSQL support)");
#endif
#ifdef WIN32
  // special version stuff for windows (if required)
  //  versionString += "\nThis is a Windows preview release - not for production use";
#endif

abt->setVersion(versionString);
QString urls = "<p align=\"center\">" +
tr("Quantum GIS is licensed under the GNU General Public License") +
"</p><p align=\"center\">" +
tr("http://www.gnu.org/licenses") +
"</p>";
abt->setURLs(urls);
QString watsNew = "<html><body>" + tr("Version") + " ";
watsNew += QGis::qgisVersion;
watsNew += "<h3>" + tr("New features") + "</h3>";
watsNew += "<ul><li>"
+ tr("On the fly projection support to automatically project layers from different coordinate systems")
+ "<li>"
+ tr("Map Composer for creating map layouts")
+ "<li>"
+ tr("Toolbox for running GRASS tools from QGIS. The toolbox can be easily customized to add additional tools")
+ "<li>"
+ tr("Handling of spatially enabled tables and views in PostgreSQL has been greatly improved. QGIS can now load any table in the database that contains a geometry column")
+ "<li>"
+ tr("PostgreSQL views containing a geometry column can be viewed in QGIS")
+ "<li>"
+ tr("Raster graphing tool to produce a histogram for a raster layer")
+ "<li>"
+ tr("Raster query using the identify tool allows you to get the pixel values from a raster by making it the active layer and clicking on the point of interest")  
+ "<li>"
+ tr("User preferences provides customizable settings for the digitizing line width, color, and selection color")  
+ "<li>"
+ tr("New symbols for use with point layers are available from the layer properties dialog") 
+ "<li>"
+ tr("Spatial bookmarks allow you to create and manage bookmarks for an area on the map. Bookmarks are persistent and global; meaning they are available for all projects")
+ "<li>"
+ tr("Measure tool allows you to measure distances on the map with both segment length and total length displayed as you click")
+ "<li>"
+ tr("GPX loading times and memory consumption for large GPX (GPS) files has been drastically reduced")  
+ "<li>"
+ tr("Many enhancements to the digitizing tools have been made, including the ability to capture data straight into PostgreSQL/PostGIS, and improvements to the definition of attribute tables for newly created layers")
+ "<li>"
+ tr("The Raster Georeferencer plugin can be used to generate a world file for a raster. The plugin allows you to define known control points in the raster coordinate system. Once enough control points are defined, the world file can be generated and the raster properly displayed in QGIS or other GIS applications")
+ "<li>"
+ tr("Improvements to the Mapserver export tool")
+ "</ul></body></html>";


abt->setWhatsNew(watsNew);

  // add the available plugins to the list
  QString providerInfo = "<b>" + tr("Available Data Provider Plugins") + "</b><br>";
  abt->setPluginInfo(providerInfo + mProviderRegistry->pluginList(true));
  QApplication::restoreOverrideCursor();
  abt->show();
  abt->exec();

}

/** Load up any plugins used in the last session
*/

void QgisApp::restoreSessionPlugins(QString thePluginDirString)
{

  QSettings mySettings;
#ifdef QGISDEBUG

  std::cerr << " -------------------- Restoring plugins from last session " << thePluginDirString.toLocal8Bit().data() << std::endl;
#endif
  // check all libs in the current plugin directory and get name and descriptions
#ifdef WIN32

  QDir myPluginDir(thePluginDirString, "*.dll", QDir::Name | QDir::IgnoreCase, QDir::Files | QDir::NoSymLinks);
#else

  QDir myPluginDir(thePluginDirString, "*.so*", QDir::Name | QDir::IgnoreCase, QDir::Files | QDir::NoSymLinks);
#endif

  if (myPluginDir.count() == 0)
  {
    //erk! do nothing
    return;
  }
  else
  {
    for (unsigned i = 0; i < myPluginDir.count(); i++)
    {
      QString myFullPath = thePluginDirString + "/" + myPluginDir[i];
#ifdef QGISDEBUG

      std::cerr << "Examining " << myFullPath.toLocal8Bit().data() << std::endl;
#endif

      QLibrary *myLib = new QLibrary(myFullPath);
      bool loaded = myLib->load();
      if (loaded)
      {
        //purposely leaving this one to stdout!
        std::cout << "Loaded " << myLib->library().toLocal8Bit().data() << std::endl;
        name_t * myName =(name_t *) myLib->resolve("name");
        description_t *  myDescription = (description_t *)  myLib->resolve("description");
        version_t *  myVersion =  (version_t *) myLib->resolve("version");
        if (myName && myDescription  && myVersion )
        {
          //check if the plugin was active on last session
          QString myEntryName = myName();
          // Windows stores a "true" value as a 1 in the registry so we
          // have to use readBoolEntry in this function

          if (mySettings.readBoolEntry("/Plugins/" + myEntryName))
          {
#ifdef QGISDEBUG
            std::cerr << " -------------------- loading " << myEntryName.toLocal8Bit().data() << std::endl;
#endif

            loadPlugin(myName(), myDescription(), myFullPath);
          }
        }
        else
        {
#ifdef QGISDEBUG
          std::cerr << "Failed to get name, description, or type for " << myLib->library().toLocal8Bit().data() << std::endl;
#endif

        }
      }
      else
      {
        std::cerr << "Failed to load " << myLib->library().toLocal8Bit().data() << std::endl;
      }
    }
  }

}


/**

  Convenience function for readily creating file filters.

  Given a long name for a file filter and a regular expression, return
  a file filter string suitable for use in a QFileDialog::OpenFiles()
  call.  The regular express, glob, will have both all lower and upper
  case versions added.

*/
static QString createFileFilter_(QString const &longName, QString const &glob)
{
  return longName + " (" + glob.lower() + " " + glob.upper() + ");;";
}                               // createFileFilter_



/**
  Builds the list of file filter strings to later be used by
  QgisApp::addLayer()

  We query OGR for a list of supported vector formats; we then build a list
  of file filter strings from that list.  We return a string that contains
  this list that is suitable for use in a a QFileDialog::getOpenFileNames()
  call.

  XXX Most of the file name filters need to be filled in; however we
  XXX may want to wait until we've tested each format before committing
  XXX them permanently instead of blindly relying on OGR to properly
  XXX supply all needed spatial data.

*/
static void buildSupportedVectorFileFilter_(QString & fileFilters)
{

#ifdef DEPRECATED
  static QString myFileFilters;

  // if we've already built the supported vector string, just return what
  // we've already built
  if ( ! ( myFileFilters.isEmpty() || myFileFilters.isNull() ) )
  {
    fileFilters = myFileFilters;

    return;
  }

  // first get the GDAL driver manager

  OGRSFDriverRegistrar *driverRegistrar = OGRSFDriverRegistrar::GetRegistrar();

  if (!driverRegistrar)
  {
    QMessageBox::warning(0,"OGR Driver Manager","unable to get OGRDriverManager");
    return;                 // XXX good place to throw exception if we
  }                           // XXX decide to do exceptions

  // then iterate through all of the supported drivers, adding the
  // corresponding file filter

  OGRSFDriver *driver;          // current driver

  QString driverName;           // current driver name

  // Grind through all the drivers and their respective metadata.
  // We'll add a file filter for those drivers that have a file
  // extension defined for them; the others, welll, even though
  // theoreticaly we can open those files because there exists a
  // driver for them, the user will have to use the "All Files" to
  // open datasets with no explicitly defined file name extension.
#ifdef QGISDEBUG

  std::cerr << "Driver count: " << driverRegistrar->GetDriverCount() << std::endl;
#endif

  for (int i = 0; i < driverRegistrar->GetDriverCount(); ++i)
  {
    driver = driverRegistrar->GetDriver(i);

    Q_CHECK_PTR(driver);

    if (!driver)
    {
      qWarning("unable to get driver %d", i);
      continue;
    }

    driverName = driver->GetName();



    if (driverName.startsWith("ESRI"))
    {
      myFileFilters += createFileFilter_("ESRI Shapefiles", "*.shp");
    }
    else if (driverName.startsWith("UK"))
    {
      // XXX needs file filter extension
    }
    else if (driverName.startsWith("SDTS"))
    {
      myFileFilters += createFileFilter_( "Spatial Data Transfer Standard",
          "*catd.ddf" );
    }
    else if (driverName.startsWith("TIGER"))
    {
      // XXX needs file filter extension
    }
    else if (driverName.startsWith("S57"))
    {
      // XXX needs file filter extension
    }
    else if (driverName.startsWith("MapInfo"))
    {
      myFileFilters += createFileFilter_("MapInfo", "*.mif *.tab");
      // XXX needs file filter extension
    }
    else if (driverName.startsWith("DGN"))
    {
      // XXX needs file filter extension
    }
    else if (driverName.startsWith("VRT"))
    {
      // XXX needs file filter extension
    }
    else if (driverName.startsWith("AVCBin"))
    {
      // XXX needs file filter extension
    }
    else if (driverName.startsWith("REC"))
    {
      // XXX needs file filter extension
    }
    else if (driverName.startsWith("Memory"))
    {
      // XXX needs file filter extension
    }
    else if (driverName.startsWith("Jis"))
    {
      // XXX needs file filter extension
    }
    else if (driverName.startsWith("GML"))
    {
      // XXX not yet supported; post 0.1 release task
      myFileFilters += createFileFilter_( "Geography Markup Language",
          "*.gml" );
    }
    else
    {
      // NOP, we don't know anything about the current driver
      // with regards to a proper file filter string
      qDebug( "%s:%d unknown driver %s", __FILE__, __LINE__, (const char *)driverName.toLocal8Bit().data() );
    }

  }                           // each loaded GDAL driver

  std::cout << myFileFilters.toLocal8Bit().data() << std::endl;

  // can't forget the default case

  myFileFilters += "All files (*.*)";
  fileFilters = myFileFilters;

#endif // DEPRECATED

  fileFilters = QgsProviderRegistry::instance()->fileVectorFilters();

}                               // buildSupportedVectorFileFilter_()




/**
  Open files, preferring to have the default file selector be the
  last one used, if any; also, prefer to start in the last directory
  associated with filterName.

  @param filterName the name of the filter; used for persistent store
  key
  @param filters    the file filters used for QFileDialog

  @param selectedFiles string list of selected files; will be empty
  if none selected
  @param enc        encoding?
  @param title      the title for the dialog
  @note

  Stores persistent settings under /UI/.  The sub-keys will be
  filterName and filterName + "Dir".

  Opens dialog on last directory associated with the filter name, or
  the current working directory if this is the first time invoked
  with the current filter name.

*/
static void openFilesRememberingFilter_(QString const &filterName, 
    QString const &filters, QStringList & selectedFiles, QString& enc, QString &title)
{

  bool haveLastUsedFilter = false; // by default, there is no last
  // used filter

  QSettings settings;         // where we keep last used filter in
  // persistant state

  QString lastUsedFilter = settings.readEntry("/UI/" + filterName,
      QString::null,
      &haveLastUsedFilter);

  QString lastUsedDir = settings.readEntry("/UI/" + filterName + "Dir",".");

  QString lastUsedEncoding = settings.readEntry("/UI/encoding");

#ifdef QGISDEBUG
  std::cerr << "Opening file dialog with filters: " << filters.toLocal8Bit().data() << std::endl;
#endif

  QgsEncodingFileDialog* openFileDialog = new QgsEncodingFileDialog(0,
      title, lastUsedDir, filters, lastUsedEncoding);

  // allow for selection of more than one file
  openFileDialog->setMode(QFileDialog::ExistingFiles);

  if (haveLastUsedFilter)       // set the filter to the last one used
  {
    openFileDialog->selectFilter(lastUsedFilter);
  }

  if (openFileDialog->exec() == QDialog::Accepted)
  {
    selectedFiles = openFileDialog->selectedFiles();
    enc = openFileDialog->encoding();
    // Fix by Tim - getting the dirPath from the dialog
    // directly truncates the last node in the dir path.
    // This is a workaround for that
    QString myFirstFileName = selectedFiles.first();
    QFileInfo myFI(myFirstFileName);
    QString myPath = myFI.dirPath();
#ifdef QGISDEBUG
    qDebug("Writing last used dir: " + myPath);
#endif

    settings.writeEntry("/UI/" + filterName, openFileDialog->selectedFilter());
    settings.writeEntry("/UI/" + filterName + "Dir", myPath);
    settings.writeEntry("/UI/encoding", openFileDialog->encoding());
  }

  delete openFileDialog;
}   // openFilesRememberingFilter_


/**
  This method prompts the user for a list of vector filenames with a dialog.

  @todo XXX I'd really like to return false, but can't because this
  XXX is for a slot that was defined void; need to fix.
  */
void QgisApp::addLayer()
{
  mMapCanvas->freeze();

  QStringList selectedFiles;
#ifdef QGISDEBUG
  std::cerr << "Vector file filters: " << mVectorFileFilter.toLocal8Bit().data() << std::endl;
#endif

  QString enc;
  QString title = tr("Open an OGR Supported Vector Layer");
  openFilesRememberingFilter_("lastVectorFileFilter", mVectorFileFilter, selectedFiles, enc,
      title);
  if (selectedFiles.isEmpty())
  {
    // no files were selected, so just bail
    mMapCanvas->freeze(false);

    return;
  }

  addLayer(selectedFiles, enc);
} // QgisApp::addLayer()




bool QgisApp::addLayer(QFileInfo const & vectorFile)
{
  // let the user know we're going to possibly be taking a while
  QApplication::setOverrideCursor(Qt::WaitCursor);

  mMapCanvas->freeze();         // XXX why do we do this?

  // create the layer

  QgsVectorLayer *layer = new QgsVectorLayer(vectorFile.filePath(),
      vectorFile.completeBaseName(),
      "ogr");
  Q_CHECK_PTR( layer );

  QObject::connect(layer, SIGNAL(editingStopped(bool)), mMapCanvas, SLOT(removeDigitizingLines(bool)));

  if ( ! layer )
  {
    mMapCanvas->freeze(false);
    QApplication::restoreOverrideCursor();

    // XXX insert meaningful whine to the user here
    return false;
  }

  if (layer->isValid())
  {
    // init the context menu so it can connect to slots
    // in main app
    // XXX move to legend::addLayer() layer->initContextMenu(this);

    // XXX What about the rest of these?  Where should they be moved, if at
    // XXX all?  Some of this functionality is taken care of in the
    // XXX QgsProject::read() (If layers added via that.)

    //add single symbol renderer as default
    QgsSingleSymRenderer *renderer = new QgsSingleSymRenderer(layer->vectorType());

    Q_CHECK_PTR( renderer );

    if ( ! renderer )
    {
      mMapCanvas->freeze(false);
      QApplication::restoreOverrideCursor();

      // XXX should we also delete the layer?

      // XXX insert meaningful whine to the user here
      return false;
    }

    layer->setRenderer(renderer);

    // Register this layer with the layers registry
    QgsMapLayerRegistry::instance()->addMapLayer(layer);
    layer->refreshLegend();

    // map canvas and overview canvas already know about this layer
    // when it is added to map registry
    //      mMapCanvas->addLayer(layer);
    //      // XXX some day use other means to  connect up a request from the raster layer to show in overview map
    //      QObject::connect(layer,
    //              SIGNAL(showInOverview(QString,bool)),
    //              this,
    //              SLOT(setLayerOverviewStatus(QString,bool)));

    // connect up any keypresses to be passed tot he layer (e.g. so esc can stop rendering)
#ifdef QGISDEBUG
    std::cout << " Connecting up maplayers keyPressed event to the QgisApp keyPress signal" << std::endl;
#endif
    QObject::connect(this,
        SIGNAL(keyPressed(QKeyEvent *)),
        layer,
        SLOT(keyPressed(QKeyEvent* )));
    //add hooks for letting layer know canvas needs to recalc the layer extents
    QObject::connect(layer,
        SIGNAL(recalculateExtents()),
        mMapCanvas,
        SLOT(recalculateExtents()));

    QObject::connect(layer,
        SIGNAL(recalculateExtents()),
        mOverviewCanvas,
        SLOT(recalculateExtents()));
  }
  else
  {
    QString msg = vectorFile.completeBaseName() + " ";
    msg += tr("is not a valid or recognized data source");
    QMessageBox::critical(this, tr("Invalid Data Source"), msg);

    // since the layer is bad, stomp on it
    delete layer;

    mMapCanvas->freeze(false);
    QApplication::restoreOverrideCursor();

    return false;
  }

  mMapCanvas->freeze(false);
  // mMapLegend->update(); NOW UPDATED VIA SIGNAL/SLOT
  qApp->processEvents();       // XXX why does this need to be called manually?
  // For Qt4, deprecate direct calling of render().  Let render() be called by the 
  // paint event loop of the map canvas widget.
  // XXX - this doesn't work -- or nobody implemented it so render() is still enabled
  // [gsherman]
  mMapCanvas->render();       // XXX eh, wot? -- without this nothing is drawn when
                              //                 the layer is intially added
  mMapCanvas->update();

  QApplication::restoreOverrideCursor();

  statusBar()->message(mMapCanvas->extent().stringRep(2));

  return true;

} // QgisApp::addLayer()





/** \brief overloaded vesion of the above method that takes a list of
 * filenames instead of prompting user with a dialog.

 XXX yah know, this could be changed to just iteratively call the above

*/
bool QgisApp::addLayer(QStringList const &theLayerQStringList, const QString& enc)
{
  mMapCanvas->freeze();

  QApplication::setOverrideCursor(Qt::WaitCursor);


  for ( QStringList::ConstIterator it = theLayerQStringList.begin();
      it != theLayerQStringList.end();
      ++it )
  {
    QFileInfo fi(*it);
    QString base = fi.completeBaseName();


    // create the layer

    QgsVectorLayer *layer = new QgsVectorLayer(*it, base, "ogr");
    QObject::connect(layer, SIGNAL(editingStopped(bool)), mMapCanvas, SLOT(removeDigitizingLines(bool)));

    Q_CHECK_PTR( layer );
    // set the visibility based on user preference for newly added
    // layers
    layer->setVisible(mAddedLayersHidden);

    if ( ! layer )
    {
      mMapCanvas->freeze(false);
      QApplication::restoreOverrideCursor();

      // XXX insert meaningful whine to the user here
      return false;
    }

    if (layer->isValid())
    {
      layer->setProviderEncoding(enc);
      // init the context menu so it can connect to slots
      // in main app

      // XXX now taken care of in legend layer->initContextMenu(this);

      //add single symbol renderer as default
      QgsSingleSymRenderer *renderer = new QgsSingleSymRenderer(layer->vectorType());

      Q_CHECK_PTR( renderer );

      if ( ! renderer )
      {
        mMapCanvas->freeze(false);
        QApplication::restoreOverrideCursor();

        // XXX insert meaningful whine to the user here
        return false;
      }

      layer->setRenderer(renderer);

      // Register this layer with the layers registry
      QgsMapLayerRegistry::instance()->addMapLayer(layer);
      layer->refreshLegend();

      // map canvas and overview canvas already know about this layer
      // when it is added to map registry
      //              mMapCanvas->addLayer(layer);
      //              // XXX some day use other means to  connect up a request from the raster layer to show in overview map
      //              QObject::connect(layer,
      //                      SIGNAL(showInOverview(QString,bool)),
      //                      this,
      //                      SLOT(setLayerOverviewStatus(QString,bool)));

      // connect up any keypresses to be passed tot he layer (e.g. so esc can stop rendering)
#ifdef QGISDEBUG
      std::cout << " Connecting up maplayers keyPressed event to the QgisApp keyPress signal" << std::endl;
#endif
      QObject::connect(this,
          SIGNAL(keyPressed(QKeyEvent *)),
          layer,
          SLOT(keyPressed(QKeyEvent* )));
      //add hooks for letting layer know canvas needs to recalc the layer extents
      QObject::connect(layer,
          SIGNAL(recalculateExtents()),
          mMapCanvas,
          SLOT(recalculateExtents()));

      QObject::connect(layer,
          SIGNAL(recalculateExtents()),
          mOverviewCanvas,
          SLOT(recalculateExtents()));
    }
    else
    {
      QString msg = *it + " ";
      msg += tr("is not a valid or recognized data source");
      QMessageBox::critical(this, tr("Invalid Data Source"), msg);

      // since the layer is bad, stomp on it
      delete layer;

      // XXX should we return false here, or just grind through
      // XXX the remaining arguments?
    }

  }

  //qApp->processEvents();
  // update legend
  /*! \todo Need legend scrollview and legenditem classes */
  // draw the map

  // mMapLegend->update(); NOW UPDATED VIA SIGNAL/SLOTS
  qApp->processEvents();    // XXX why does this need to be called manually?
  mMapCanvas->freeze(false);
  // For Qt4, deprecate direct calling of render().  Let render() be called by the 
  // paint event loop of the map canvas widget.
  // XXX - this doesn't work -- or nobody implemented it so render() is still enabled
  // [gsherman]
  mMapCanvas->render();
  mMapCanvas->update();
  QApplication::restoreOverrideCursor();
  statusBar()->message(mMapCanvas->extent().stringRep(2));


  return true;

} // QgisApp::addLayer()



/** This helper checks to see whether the filename appears to be a valid vector file name */
bool QgisApp::isValidVectorFileName(QString theFileNameQString)
{
  return (theFileNameQString.lower().endsWith(".shp"));
}

/** Overloaded of the above function provided for convenience that takes a qstring pointer */
bool QgisApp::isValidVectorFileName(QString * theFileNameQString)
{
  //dereference and delegate
  return isValidVectorFileName(*theFileNameQString);
}

#ifndef HAVE_POSTGRESQL
void QgisApp::addDatabaseLayer(){}
#else
void QgisApp::addDatabaseLayer()
{
  // only supports postgis layers at present

  // show the postgis dialog

  QgsDbSourceSelect *dbs = new QgsDbSourceSelect(this);

  mMapCanvas->freeze();

  if (dbs->exec())
  {
    QApplication::setOverrideCursor(Qt::WaitCursor);


    // repaint the canvas if it was covered by the dialog

    // add files to the map canvas
    QStringList tables = dbs->selectedTables();

    QString connInfo = dbs->connInfo();
    // for each selected table, connect to the database, parse the WKT geometry,
    // and build a cavnasitem for it
    // readWKB(connInfo,tables);
    QStringList::Iterator it = tables.begin();
    while (it != tables.end())
    {

      // create the layer
      //qWarning("creating layer");
      QgsVectorLayer *layer = new QgsVectorLayer(connInfo + " table=" + *it, *it, "postgres");
      QObject::connect(layer, SIGNAL(editingStopped(bool)), mMapCanvas, SLOT(removeDigitizingLines(bool)));

      if (layer->isValid())
      {
        // set initial visibility based on user preference
        layer->setVisible(mAddedLayersHidden);

        // give it a random color
        QgsSingleSymRenderer *renderer = new QgsSingleSymRenderer(layer->vectorType());  // add single symbol renderer as default
        layer->setRenderer(renderer);

        // register this layer with the central layers registry
        QgsMapLayerRegistry::instance()->addMapLayer(layer);
        layer->refreshLegend();

        // add it to the mapcanvas collection
        // mMapCanvas->addLayer(layer);
        //connect up a request from the raster layer to show in overview map
        //      QObject::connect(layer,
        //              SIGNAL(showInOverview(QString,bool)),
        //              this,
        //              SLOT(setLayerOverviewStatus(QString,bool)));

        // connect up any keypresses to be passed tot he layer (e.g. so esc can stop rendering)
#ifdef QGISDEBUG
        std::cout << " Connecting up maplayers keyPressed event to the QgisApp keyPress signal" << std::endl;
#endif
        QObject::connect(this,
            SIGNAL(keyPressed(QKeyEvent *)),
            layer,
            SLOT(keyPressed(QKeyEvent* )));
        //add hooks for letting layer know canvas needs to recalc the layer extents
        QObject::connect(layer,
            SIGNAL(recalculateExtents()),
            mMapCanvas,
            SLOT(recalculateExtents()));

        QObject::connect(layer,
            SIGNAL(recalculateExtents()),
            mOverviewCanvas,
            SLOT(recalculateExtents()));
      }
      else
      {
        std::cerr << (*it).toLocal8Bit().data() << " is an invalid layer - not loaded" << std::endl;
        QMessageBox::critical(this, tr("Invalid Layer"), tr("%1 is an invalid layer and cannot be loaded.").arg(*it));
        delete layer;
      }
      //qWarning("incrementing iterator");
      ++it;
    }
    statusBar()->message(mMapCanvas->extent().stringRep(2));
  }

  qApp->processEvents();
  mMapCanvas->freeze(false);
  // For Qt4, deprecate direct calling of render().  Let render() be called by the 
  // paint event loop of the map canvas widget.
  // XXX - this doesn't work -- or nobody implemented it so render() is still enabled
  // [gsherman]
  mMapCanvas->render();
  mMapCanvas->update();
  QApplication::restoreOverrideCursor();
} // QgisApp::addDatabaseLayer()
#endif


void QgisApp::addWmsLayer()
{
  // Fudge for now

#ifdef QGISDEBUG
  std::cout << "QgisApp::addWmsLayer: about to addRasterLayer" << std::endl;
#endif


  QgsServerSourceSelect *wmss = new QgsServerSourceSelect(this);

  mMapCanvas->freeze();

  if (wmss->exec())
  {

    addRasterLayer(wmss->connInfo(), 
        wmss->connName(), 
        "wms",
        wmss->selectedLayers(),
        wmss->selectedStylesForSelectedLayers(),
        wmss->selectedImageEncoding() );
  }
}




/// file data representation
enum dataType { IS_VECTOR, IS_RASTER, IS_BOGUS };



/** returns data type associated with the given QgsProject file DOM node

  The DOM node should represent the state associated with a specific layer.
  */
static
  dataType
dataType_( QDomNode & layerNode )
{
  QString type = layerNode.toElement().attribute( "type" );

  if ( QString::null == type )
  {
    qDebug( "%s:%d cannot find ``type'' attribute", __FILE__, __LINE__ );

    return IS_BOGUS;
  }

  if ( "raster" == type )
  {
    qDebug( "%s:%d is a raster", __FILE__, __LINE__ );

    return IS_RASTER;
  }
  else if ( "vector" == type )
  {
    qDebug( "%s:%d is a vector", __FILE__, __LINE__ );

    return IS_VECTOR;
  }

  qDebug( "%s:%d is unknown type %s", __FILE__, __LINE__, (const char *)type.toLocal8Bit().data() );

  return IS_BOGUS;
} // dataType_( QDomNode & layerNode )


/** return the data source for the given layer

  The QDomNode is a QgsProject DOM node corresponding to a map layer state.

  Essentially dumps <datasource> tag.

*/
static
  QString
dataSource_( QDomNode & layerNode )
{
  QDomNode dataSourceNode = layerNode.namedItem( "datasource" );

  if ( dataSourceNode.isNull() )
  {
    qDebug( "%s:%d cannot find datasource node", __FILE__, __LINE__ );

    return QString::null;
  }

  return dataSourceNode.toElement().text();

} // dataSource_( QDomNode & layerNode )



/// the three flavors for data
typedef enum { IS_FILE, IS_DATABASE, IS_URL, IS_UNKNOWN } providerType;


/** return the physical storage type associated with the given layer

  The QDomNode is a QgsProject DOM node corresponding to a map layer state.

  If the <provider> is "ogr", then it's a file type.

  However, if the layer is a raster, then there won't be a <provider> tag.  It
  will always have an associated file.

  If the layer doesn't fall into either of the previous two categories, then
  it's either a database or URL.  If the <datasource> tag has "url=", then
  it's URL based.  If the <datasource> tag has "dbname=">, then the layer data
  is in a database.

*/
static
  providerType
providerType_( QDomNode & layerNode )
{
  // XXX but what about rasters that can be URLs?  _Can_ they be URLs?

  switch( dataType_( layerNode ) )
  {
    case IS_VECTOR:
      {
        QString dataSource = dataSource_( layerNode );

#ifdef QGISDEBUG
        qDebug( "%s:%d datasource is %s", __FILE__, __LINE__, (const char *)dataSource.toLocal8Bit().data() );
#endif
        if ( dataSource.contains("host=") )
        {
          return IS_URL;
        }
#ifdef HAVE_POSTGRESQL
        else if ( dataSource.contains("dbname=") )
        {
          return IS_DATABASE;
        }
#endif
        // be default, then, this should be a file based layer data source
        // XXX is this a reasonable assumption?

        return IS_FILE;
      }

    case IS_RASTER:         // rasters are currently only accessed as
      // physical files
      return IS_FILE;

    default:
      qDebug( "%s:%d unknown ``type'' attribute", __FILE__, __LINE__ );
  }

  return IS_UNKNOWN;

} // providerType_



/** set the <datasource> to the new value
*/
static
  void
setDataSource_( QDomNode & layerNode, QString const & dataSource )
{
  QDomNode dataSourceNode = layerNode.namedItem("datasource");
  QDomElement dataSourceElement = dataSourceNode.toElement();
  QDomText dataSourceText = dataSourceElement.firstChild().toText();


#ifdef QGISDEBUG
  QString originalDataSource = dataSourceText.data();

  qDebug( "%s:%d datasource changed from %s", __FILE__, __LINE__, (const char *)originalDataSource.toLocal8Bit().data() );
#endif

  dataSourceText.setData( dataSource );

#ifdef QGISDEBUG
  QString newDataSource = dataSourceText.data();

  qDebug( "%s:%d to %s", __FILE__, __LINE__, (const char *)newDataSource.toLocal8Bit().data() );
#endif
} // setDataSource_




/** this is used to locate files that have moved or otherwise are missing

*/
static
  void
findMissingFile_( QString const & fileFilters, QDomNode & layerNode )
{
  // Prepend that file name to the valid file format filter list since it
  // makes it easier for the user to not only find the original file, but to
  // perhaps find a similar file.

  QFileInfo originalDataSource( dataSource_(layerNode) );

  QString memoryQualifier;    // to differentiate between last raster and
  // vector directories

  switch( dataType_( layerNode ) )
  {
    case IS_VECTOR:
      {
        memoryQualifier = "lastVectorFileFilter";

        break;
      }
    case IS_RASTER:
      {
        memoryQualifier = "lastRasterFileFilter";

        break;
      }
    default:
      qDebug( "%s:%d unable to determine data type", __FILE__, __LINE__ );
      return;
  }

  // Prepend the original data source base name to make it easier to pick it
  // out from a list of other files; however the appropriate filter strings
  // for the file type will also be added in case the file name itself has
  // changed, too.

  QString myFileFilters = originalDataSource.fileName() + ";;" + fileFilters;

  QStringList selectedFiles;
  QString     enc;
  QString     title( QObject::trUtf8("Open an OGR Supported Layer") );

  openFilesRememberingFilter_(memoryQualifier,
      myFileFilters, 
      selectedFiles, 
      enc,
      title);

  if (selectedFiles.isEmpty())
  {
    return;
  }
  else
  {
    setDataSource_( layerNode, selectedFiles.first() );
    if ( ! QgsProject::instance()->read( layerNode ) )
    {
      qDebug( "%s:%d unable to re-read layer", __FILE__, __LINE__ );
    }
  }

} // findMissingFile_




/** find relocated data source for the given layer

  This QDom object represents a QgsProject node that maps to a specific layer.

  @param layerNode QDom node containing layer project information

  @todo

  XXX Only implemented for file based layers.  It will need to be extended for
  XXX other data source types such as databases.

*/
static
  void
findLayer_( QString const & fileFilters, QDomNode const & constLayerNode ) 
{
  // XXX actually we could possibly get away with a copy of the node
  QDomNode & layerNode = const_cast<QDomNode&>(constLayerNode);

  switch ( providerType_(layerNode) )
  {
    case IS_FILE:
      qDebug( "%s:%d layer is file based", __FILE__, __LINE__ );
      findMissingFile_( fileFilters, layerNode );
      break;

    case IS_DATABASE:
      qDebug( "%s:%d layer is database based", __FILE__, __LINE__ );
      break;

    case IS_URL:
      qDebug( "%s:%d layer is URL based", __FILE__, __LINE__ );
      break;

    case IS_UNKNOWN:
      qDebug( "%s:%d layer has an unkown type", __FILE__, __LINE__ );
      break;
  }

}; // findLayer_




/** find relocated data sources for given layers

  These QDom objects represent QgsProject nodes that map to specific layers.

*/
static
  void
findLayers_( QString const & fileFilters, list<QDomNode> const & layerNodes )
{
#ifdef QGISDEBUG
  const char * fileFiltersC = fileFilters.ascii(); // debugger probe
#endif

  for( list<QDomNode>::const_iterator i = layerNodes.begin();
      i != layerNodes.end();
      ++i )
  {
    findLayer_( fileFilters, *i );
  }

} // findLayers_







void QgisApp::fileExit()
{
  removeAllLayers();
  QApplication::exit();
}



void QgisApp::fileNew()
{
  int answer = saveDirty();

  if (answer != QMessageBox::Cancel)
  {
    mMapCanvas->freeze(true);
    mOverviewCanvas->freeze(true);
    QgsMapLayerRegistry::instance()->removeAllMapLayers();
    mMapCanvas->clear();
    mOverviewCanvas->clear();


    QgsProject::instance()->title( QString::null );
    QgsProject::instance()->filename( QString::null );

#ifdef QGISDEBUG
    std::cout << "Clearing project properties" << std::endl;
#endif
    QgsProject::instance()->clearProperties(); // why carry over properties from previous projects?
    QgsProject::instance()->dirty(false);

    setTitleBarText_( *this );
#ifdef QGISDEBUG
    std::cout << "emiting new project signal" << std::endl ;
#endif
    //note by Tim: I did some casual egrepping and this signal doesnt actually
    //seem to be connected to anything....why is it here? Just for future needs?
    emit newProject();

    mMapCanvas->freeze(false);
    mOverviewCanvas->freeze(false);
  }
  //set the projections enabled icon in the status bar
  int myProjectionEnabledFlag =
    QgsProject::instance()->readNumEntry("SpatialRefSys","/ProjectionsEnabled",0);
  projectionsEnabled(myProjectionEnabledFlag);
} // fileNew()


//as file new but accepts flags to indicate whether we should prompt to save
void QgisApp::fileNew(bool thePromptToSaveFlag)
{
  if (thePromptToSaveFlag)
  {
    //just delegate this out
    fileNew();
  }
  else
  {
    mMapCanvas->removeAll();
    mMapCanvas->clear();
    mOverviewCanvas->removeAll();
    mOverviewCanvas->clear();

    QgsProject::instance()->title( QString::null );
    QgsProject::instance()->filename( QString::null );
    QgsProject::instance()->clearProperties(); // why carry over properties from previous projects?
    QgsProject::instance()->dirty(false);

    setTitleBarText_( *this );

    emit newProject();
  }
} // QgisApp::fileNew(bool thePromptToSaveFlag)


void QgisApp::newVectorLayer()
{

  QGis::WKBTYPE geometrytype;
  QString fileformat;

  QgsGeomTypeDialog geomDialog;
  if(geomDialog.exec()==QDialog::Rejected)
  {
    return;
  }
  geometrytype = geomDialog.selectedType();
  fileformat = geomDialog.selectedFileFormat();

  std::list<std::pair<QString, QString> > attributes;
  geomDialog.attributes(attributes);

  bool haveLastUsedFilter = false; // by default, there is no last
  // used filter
  QString enc;
  QString filename;

  QSettings settings;         // where we keep last used filter in
  // persistant state

  QString lastUsedFilter = settings.readEntry("/UI/lastVectorFileFilter",
      QString::null,
      &haveLastUsedFilter);

  QString lastUsedDir = settings.readEntry("/UI/lastVectorFileFilterDir",
      ".");

  QString lastUsedEncoding = settings.readEntry("/UI/encoding");

#ifdef QGISDEBUG

  std::cerr << "Saving vector file dialog without filters: " << std::endl;
#endif

  QgsEncodingFileDialog* openFileDialog = new QgsEncodingFileDialog(this,
      tr("Save As"), lastUsedDir, "", lastUsedEncoding);

  // allow for selection of more than one file
  openFileDialog->setMode(QFileDialog::AnyFile);

  if (haveLastUsedFilter)       // set the filter to the last one used
  {
    openFileDialog->selectFilter(lastUsedFilter);
  }

  if (openFileDialog->exec() != QDialog::Accepted)
  {
    delete openFileDialog;
    return;
  }

  filename = openFileDialog->selectedFile();
  enc = openFileDialog->encoding();

  settings.writeEntry("/UI//lastVectorFileFilter", openFileDialog->selectedFilter());

  settings.writeEntry("/UI//lastVectorFileFilterDir", openFileDialog->directory().absolutePath());
  settings.writeEntry("/UI/encoding", openFileDialog->encoding());

  delete openFileDialog;


  // check to see if user specified the extension. if not, add it...
  //if(filename.find(QRegExp("\\.shp$")) == -1)
  //{
  //filename += ".shp";
  //}

  //try to create the new layer with OGRProvider instead of QgsVectorFileWriter
  QgsProviderRegistry * pReg = QgsProviderRegistry::instance();
  QString ogrlib = pReg->library("ogr");
  // This var is not used...remove? TS FIXME
  const char *cOgrLib = (const char *) ogrlib;
  // load the data provider
  QLibrary* myLib = new QLibrary((const char *) ogrlib);
  bool loaded = myLib->load();
  if (loaded)
  {
#ifdef QGISDEBUG
    qWarning("ogr provider loaded");
#endif
    typedef bool (*createEmptyDataSourceProc)(const QString&, const QString&, QGis::WKBTYPE, \
        const std::list<std::pair<QString, QString> >&);
    createEmptyDataSourceProc createEmptyDataSource=(createEmptyDataSourceProc)myLib->resolve("createEmptyDataSource");
    if(createEmptyDataSource)
    {
      if(geometrytype == QGis::WKBPoint)
      {
        createEmptyDataSource(filename,fileformat,QGis::WKBPoint, attributes);
      }
      else if (geometrytype == QGis::WKBLineString)
      {
        createEmptyDataSource(filename,fileformat,QGis::WKBLineString, attributes);
      }
      else if(geometrytype == QGis::WKBPolygon)
      {
        createEmptyDataSource(filename,fileformat,QGis::WKBPolygon, attributes);
      }
      else
      {
#ifdef QGISDEBUG
        qWarning("QgisApp.cpp: geometry type not recognised");
#endif
        return;
      }
    }
    else
    {
#ifdef QGISDEBUG
      qWarning("Resolving newEmptyDataSource(...) failed");;
#endif
    }
  }

#if 0
  //create the new layer with QgsVectorFileWriter
  QgsVectorFileWriter* writer=0;

  if(geometrytype == QGis::WKBPoint)
  {
    writer=new QgsVectorFileWriter(filename,enc,wkbPoint);
    if(!writer->initialise())
    {
      QMessageBox::warning(0,tr("Warning"),tr("Writing of the layer failed"),QMessageBox::Ok,Qt::NoButton);
      return;
    }
  }
  else if(geometrytype == QGis::WKBLineString)
  {
    writer=new QgsVectorFileWriter(filename,enc,wkbLineString);
    if(!writer->initialise())
    {
      QMessageBox::warning(0,tr("Warning"),tr("Writing of the layer failed"),QMessageBox::Ok,Qt::NoButton);
      return;
    }
  }
  else if(geometrytype == QGis::WKBPolygon)
  {
    writer=new QgsVectorFileWriter(filename,enc,wkbPolygon);
    if(!writer->initialise())
    {
      QMessageBox::warning(0,tr("Warning"),tr("Writing of the layer failed"),QMessageBox::Ok,Qt::NoButton);
      return;
    }
  }
  else
  {
#ifdef QGISDEBUG
    qWarning("QgisApp.cpp: geometry type not recognised");
#endif

    return;
  }

  if(writer)
  {
    for(std::list<std::pair<QString, QString> >::iterator it=attributes.begin();it!=attributes.end();++it)
    {
      if(it->second=="Real")
      {
        writer->createField(it->first, OFTReal, 10, 3);
      }
      else if(it->second=="Integer")
      {
        writer->createField(it->first, OFTInteger, 10, 3);
      }
      else if(it->second=="String")
      {
        writer->createField(it->first, OFTString, 40, 1);
      }
    }
  }
  delete writer;
#endif //0

  //then add the layer to the view
  QStringList filelist;
  filelist.append(filename);
  addLayer(filelist, enc);
  return;
}

void QgisApp::fileOpen()
{
  // possibly save any pending work before opening a new project
  int answer = saveDirty();

  if (answer != QMessageBox::Cancel)
  {
    // Retrieve last used project dir from persistent settings
    QSettings settings;
    QString lastUsedDir = settings.readEntry("/UI/lastProjectDir", ".");

    QFileDialog * openFileDialog = new QFileDialog(this,
        tr("Choose a QGIS project file to open"),
        lastUsedDir, QObject::tr("QGis files (*.qgs)"));
    openFileDialog->setMode(QFileDialog::ExistingFile);


    QString fullPath;
    if (openFileDialog->exec() == QDialog::Accepted)
    {
      // Fix by Tim - getting the dirPath from the dialog
      // directly truncates the last node in the dir path.
      // This is a workaround for that
      fullPath = openFileDialog->selectedFile();
      QFileInfo myFI(fullPath);
      QString myPath = myFI.dirPath();
      // Persist last used project dir
      settings.writeEntry("/UI/lastProjectDir", myPath);
    }
    else 
    {
      // if they didn't select anything, just return
      delete openFileDialog;
      return;
    }

    delete openFileDialog;

    // clear out any stuff from previous project
    removeAllLayers();

    QgsProject::instance()->filename( fullPath );

    try 
    {
      if ( QgsProject::instance()->read() )
      {
        setTitleBarText_( *this );
        mMapCanvas->setMapUnits(QgsProject::instance()->mapUnits());
        emit projectRead();     // let plug-ins know that we've read in a new
        // project so that they can check any project
        // specific plug-in state

        // add this to the list of recently used project files
        saveRecentProjectPath(fullPath, settings);
      }
    }
    catch ( QgsProjectBadLayerException & e )
    {
      QMessageBox::critical(this, 
          tr("QGIS Project Read Error"), 
          tr("") + "\n" + e.what() );
      qDebug( "%s:%d %d bad layers found", __FILE__, __LINE__, e.layers().size() );

      // attempt to find the new locations for missing layers
      // XXX vector file hard-coded -- but what if it's raster?
      findLayers_( mVectorFileFilter, e.layers() );
    }
    catch ( std::exception & e )
    {
      QMessageBox::critical(this, 
          tr("QGIS Project Read Error"), 
          tr("") + "\n" + e.what() );
      qDebug( "%s:%d BAD LAYERS FOUND", __FILE__, __LINE__ );
    }
  }
  //loop through all layers in the layers registry and connect up 
  // keybindings for the escape key
  std::map<QString, QgsMapLayer *> myMapLayers 
    = QgsMapLayerRegistry::instance()->mapLayers();
  std::map<QString, QgsMapLayer *>::iterator myMapIterator;
  for ( myMapIterator = myMapLayers.begin(); myMapIterator != myMapLayers.end(); ++myMapIterator )
  {

    QgsMapLayer * myMapLayer = myMapIterator->second;
    QObject::connect(this,
        SIGNAL(keyPressed(QKeyEvent *)),
        myMapLayer,
        SLOT(keyPressed(QKeyEvent* )));

    //add hooks for letting layer know canvas needs to recalc the layer extents
    QObject::connect(myMapLayer,
        SIGNAL(recalculateExtents()),
        mMapCanvas,
        SLOT(recalculateExtents()));

    QObject::connect(myMapLayer,
        SIGNAL(recalculateExtents()),
        mOverviewCanvas,
        SLOT(recalculateExtents()));
  }

  //set the projections enabled icon in the status bar
  int myProjectionEnabledFlag =
    QgsProject::instance()->readNumEntry("SpatialRefSys","/ProjectionsEnabled",0);
  projectionsEnabled(myProjectionEnabledFlag);
} // QgisApp::fileOpen



/**
  adds a saved project to qgis, usually called on startup by specifying a
  project file on the command line
  */
bool QgisApp::addProject(QString projectFile)
{
  mOverviewCanvas->freeze(true);
  mMapCanvas->freeze(true);

  // clear the map canvas
  removeAllLayers();

  try
  {
    if ( QgsProject::instance()->read( projectFile ) )
    {
      setTitleBarText_( *this );
      int  myRedInt = QgsProject::instance()->readNumEntry("Gui","/CanvasColorRedPart",255);
      int  myGreenInt = QgsProject::instance()->readNumEntry("Gui","/CanvasColorGreenPart",255);
      int  myBlueInt = QgsProject::instance()->readNumEntry("Gui","/CanvasColorBluePart",255);
      QColor myColor = QColor(myRedInt,myGreenInt,myBlueInt);
      mMapCanvas->setCanvasColor(myColor); //this is fill colour before rendering starts
      mMapCanvas->setBackgroundColor(myColor); // this is for the widget itself
      qDebug("Canvas bacground color restored...");

      emit projectRead(); // let plug-ins know that we've read in a new
      // project so that they can check any project
      // specific plug-in state

      // add this to the list of recently used project files
      QSettings settings;
      saveRecentProjectPath(projectFile, settings);
    }
    else
    {
      return false;
    }
  }
  catch ( QgsProjectBadLayerException & e )
  {
    qDebug( "%s:%d %d bad layers found", __FILE__, __LINE__, e.layers().size() );

    if ( QMessageBox::Yes == QMessageBox::critical( this, 
          tr("QGIS Project Read Error"), 
          tr("") + "\n" + e.what() + "\n" +
          tr("Try to find missing layers?"),
          QMessageBox::Yes | QMessageBox::Default, 
          QMessageBox::No  | QMessageBox::Escape ) )
    {
      qDebug( "%s:%d want to find missing layers is true", __FILE__, __LINE__ );

      // attempt to find the new locations for missing layers
      // XXX vector file hard-coded -- but what if it's raster?
      findLayers_( mVectorFileFilter, e.layers() );
    }

  }
  catch ( std::exception & e )
  {
    qDebug( "%s:%d BAD LAYERS FOUND", __FILE__, __LINE__ );

    QMessageBox::critical( 0x0, 
        tr("Unable to open project"), e.what(), QMessageBox::Ok, 
        Qt::NoButton );

    return false;
  }

  return true;
} // QgisApp::addProject(QString projectFile)



void QgisApp::fileSave()
{
  // if we don't have a filename, then obviously we need to get one; note
  // that the project file name is reset to null in fileNew()
  QFileInfo fullPath;

  // we need to remember if this is a new project so that we know to later
  // update the "last project dir" settings; we know it's a new project if
  // the current project file name is empty
  bool isNewProject = false;

  if ( QgsProject::instance()->filename().isNull() )
  {
    isNewProject = true;

    // Retrieve last used project dir from persistent settings
    QSettings settings;
    QString lastUsedDir = settings.readEntry("/UI/lastProjectDir", ".");

    std::auto_ptr<QFileDialog> saveFileDialog( new QFileDialog(this,
        tr("Choose a QGIS project file"),
        lastUsedDir, QObject::tr("QGis files (*.qgs)")) );

    saveFileDialog->setMode(QFileDialog::AnyFile);

    if (saveFileDialog->exec() == QDialog::Accepted)
    {
      fullPath.setFile( saveFileDialog->selectedFile() );
    }
    else 
    {
      // if they didn't select anything, just return
      // delete saveFileDialog; auto_ptr auto destroys
      return;
    }

    // make sure we have the .qgs extension in the file name
    if( "qgs" != fullPath.extension( false ) )
    {
      QString newFilePath = fullPath.filePath() + ".qgs";
#ifdef QGISDEBUG
      const char* filePathStr = newFilePath.ascii(); // debugger probe
#endif
      fullPath.setFile( newFilePath );
    }

    //  Check to see if the file exists before just blasting it into
    //  oblivion; abort saving the project if the user does not want to
    //  over-write an existing file.

    if ( fullPath.exists() )
    {
      if ( QMessageBox::No == QMessageBox::warning( 0x0, 
            tr("Project file exists."),
            tr("The given project file exists.  Do you wish to over-write it with a new one?"),
            QMessageBox::Yes | QMessageBox::Default,
            QMessageBox::No  | QMessageBox::Escape,
            Qt::NoButton ) )
      {
        return;         // abort saving the file since the user
        // doesn't want to over-write
      }

    }
    else
    {
      QgsDebug( " project file does not already exist" );
    }

#ifdef QGISDEBUG
    const char* filePathStr = fullPath.filePath().ascii(); // debugger probe
#endif
    QgsProject::instance()->filename( fullPath.filePath() );
  }

  try
  {
    if ( QgsProject::instance()->write() )
    {
      statusBar()->message(tr("Saved project to:") + " " + QgsProject::instance()->filename() );

      if (isNewProject)
      {
        // add this to the list of recently used project files
        QSettings settings;
        saveRecentProjectPath(fullPath.filePath(), settings);
      }
    }
    else
    {
      QMessageBox::critical(this,
          tr("Unable to save project"),
          tr("Unable to save project to ") + QgsProject::instance()->filename() );
    }
  }
  catch ( std::exception & e )
  {
    QMessageBox::critical( 0x0,
        tr("Unable to save project ") + QgsProject::instance()->filename(),
        e.what(),
        QMessageBox::Ok,
        Qt::NoButton );

  }

} // QgisApp::fileSave



void QgisApp::fileSaveAs()
{
  // Retrieve last used project dir from persistent settings
  QSettings settings;
  QString lastUsedDir = settings.readEntry("/UI/lastProjectDir", ".");

  auto_ptr<QFileDialog> saveFileDialog( new QFileDialog(this,
      tr("Choose a QGIS project file"),
      lastUsedDir, QObject::tr("QGis files (*.qgs)")) );

  saveFileDialog->setMode(QFileDialog::AnyFile);

  // if we don't have a filename, then obviously we need to get one; note
  // that the project file name is reset to null in fileNew()
  QFileInfo fullPath;

  if (saveFileDialog->exec() == QDialog::Accepted)
  {
    // Fix by Tim - getting the dirPath from the dialog
    // directly truncates the last node in the dir path.
    // This is a workaround for that
    fullPath.setFile(saveFileDialog->selectedFile());
    QString myPath = fullPath.dirPath();
    // Persist last used project dir
    settings.writeEntry("/UI/lastProjectDir", myPath);
  } 
  else
  {
    // if they didn't select anything, just return
    // delete saveFileDialog; auto_ptr auto deletes
    return;
  }

  // make sure the .qgs extension is included in the path name. if not, add it...
  if( "qgs" != fullPath.extension( false ) )
  {
    QString newFilePath = fullPath.filePath() + ".qgs";
#ifdef QGISDEBUG
    const char* filePathStr = newFilePath.ascii(); // debugger probe
#endif
    fullPath.setFile( newFilePath );
  }


  //  Check to see if the file exists before just blasting it into
  //  oblivion; abort saving the project if the user does not want to
  //  over-write an existing file.

  if ( fullPath.exists() )
  {
    if ( QMessageBox::No == QMessageBox::warning( 0x0, 
          tr("Project file exists."),
          tr("The given project file exists.  Do you wish to over-write it with a new one?"),
          QMessageBox::Yes | QMessageBox::Default,
          QMessageBox::No  | QMessageBox::Escape,
          Qt::NoButton ) )
    {
      return;             // abort saving the file since the user
      // doesn't want to over-write
    }
  }
  else
  {
    QgsDebug( " project file does not already exist" );
  }


  QgsProject::instance()->filename( fullPath.filePath() );

  if ( QgsProject::instance()->write() )
  {
    statusBar()->message(tr("Saved project to:") + " " + QgsProject::instance()->filename() );
    // add this to the list of recently used project files
    saveRecentProjectPath(fullPath.filePath(), settings);
  }
  else
  {
    QMessageBox::critical(this,
        tr("Unable to save project"),
        tr("Unable to save project to ") + QgsProject::instance()->filename() );
  }
} // QgisApp::fileSaveAs






// Open the project file corresponding to the
// path at the given index in mRecentProjectPaths
void QgisApp::openProject(int pathIndex)
{
  // possibly save any pending work before opening a different project
  int answer = saveDirty();

  if (answer != QMessageBox::Cancel)
  {
#if QT_VERSION < 0x040000
    QStringList::Iterator it = mRecentProjectPaths.at(pathIndex);
    addProject((*it));
#else
    addProject(mRecentProjectPaths.at(pathIndex));
#endif
  }
  //set the projections enabled icon in the status bar
  int myProjectionEnabledFlag =
    QgsProject::instance()->readNumEntry("SpatialRefSys","/ProjectionsEnabled",0);
  projectionsEnabled(myProjectionEnabledFlag);

} // QgisApp::openProject

/**
  Open the specified project file; prompt to save previous project if necessary.
  Used to process a commandline argument or OpenDocument AppleEvent.
  */
void QgisApp::openProject(const QString & fileName)
{
  // possibly save any pending work before opening a different project
  int answer = saveDirty();

  if (answer != QMessageBox::Cancel)
  {
    try
    {
      if ( ! addProject(fileName) )
      {
#ifdef QGISDEBUG
        std::cerr << "unable to load project " << fileName.toLocal8Bit().data() << "\n";
#endif
      }
      else
      {
      }
    }
    catch ( QgsIOException & io_exception )
    {
      QMessageBox::critical( 0x0, 
          tr("QGIS: Unable to load project"), 
          tr("Unable to load project ") + fileName );
    }
  }
  return ;
}


/**
  Open a raster or vector file; ignore other files.
  Used to process a commandline argument or OpenDocument AppleEvent.
  @returns true if the file is successfully opened
  */
bool QgisApp::openLayer(const QString & fileName)
{
  QFileInfo fileInfo(fileName);
  // try to load it as raster
  bool ok = addRasterLayer(fileInfo, false);
  if (!ok)
  {
    // nope - try to load it as a shape/ogr
    ok = addLayer(fileInfo);
    // we have no idea what this file is...
    if (!ok)
    {
      std::cout << "Unable to load " << fileName.toLocal8Bit().data() << std::endl;
    }
  }
#ifdef WIN32
  return true;
#endif
}


/*
   void QgisApp::filePrint()
   {
//
//  Warn the user first that priting is experimental still
//
QString myHeading = "QGIS Printing Support is Experimental";
QString myMessage = "Please note that printing only works on A4 landscape at the moment.\n";
myMessage += "For other page sizes your mileage may vary.\n";
QMessageBox::information( this, tr(myHeading),tr(myMessage) );

QPrinter myQPrinter;
if(myQPrinter.setup(this))
{
#ifdef QGISDEBUG
std::cout << ".............................." << std::endl;
std::cout << "...........Printing..........." << std::endl;
std::cout << ".............................." << std::endl;
#endif
// Ithought we could just do this:
//mMapCanvas->render(&myQPrinter);
//but it doesnt work so now we try this....
QPaintDeviceMetrics myMetrics( &myQPrinter ); // need width/height of printer surface
std::cout << "Print device width: " << myMetrics.width() << std::endl;
std::cout << "Print device height: " << myMetrics.height() << std::endl;
QPainter myQPainter;
myQPainter.begin( &myQPrinter );
QPixmap myQPixmap(myMetrics.width(),myMetrics.height());
myQPixmap.fill();
mMapCanvas->freeze(false);
mMapCanvas->setDirty(true);
mMapCanvas->render(&myQPixmap);
myQPainter.drawPixmap(0,0, myQPixmap);
myQPainter.end();
}
}
*/

void QgisApp::filePrint()
{
  mComposer->open();
  mComposer->zoomFull();
}

void QgisApp::saveMapAsImage()
{
  //create a map to hold the QImageIO names and the filter names
  //the QImageIO name must be passed to the mapcanvas saveas image function
  typedef QMap<QString, QString> FilterMap;
  FilterMap myFilterMap;

  //find out the last used filter
  QSettings myQSettings;  // where we keep last used filter in persistant state
  QString myLastUsedFilter = myQSettings.readEntry("/UI/saveAsImageFilter");
  QString myLastUsedDir = myQSettings.readEntry("/UI/lastSaveAsImageDir",".");


  // get a list of supported output image types
  int myCounterInt=0;
  QString myFilters;
  for ( ; myCounterInt < QPictureIO::outputFormats().count(); myCounterInt++ )
  {
    QString myFormat=QString(QPictureIO::outputFormats().at( myCounterInt ));
    QString myFilter = createFileFilter_(myFormat + " format", "*."+myFormat);
    myFilters += myFilter;
    myFilterMap[myFilter] = myFormat;
  }
#ifdef QGISDEBUG
  std::cout << "Available Filters Map: " << std::endl;
  FilterMap::Iterator myIterator;
  for ( myIterator = myFilterMap.begin(); myIterator != myFilterMap.end(); ++myIterator )
  {
    std::cout << myIterator.key().toLocal8Bit().data() << "  :  " << myIterator.data().toLocal8Bit().data() << std::endl;
  }

#endif

  //create a file dialog using the the filter list generated above
  std::auto_ptr < QFileDialog > myQFileDialog( new QFileDialog(this,
      tr("Choose a filename to save the map image as"),
      myLastUsedDir, myFilters) );

  // allow for selection of more than one file
  myQFileDialog->setMode(QFileDialog::AnyFile);

  if (!myLastUsedFilter.isEmpty())       // set the filter to the last one used
  {
    myQFileDialog->selectFilter(myLastUsedFilter);
  }


  //prompt the user for a filename
  QString myOutputFileNameQString; // = myQFileDialog->getSaveFileName(); //delete this
  if (myQFileDialog->exec() == QDialog::Accepted)
  {
    myOutputFileNameQString = myQFileDialog->selectedFile();
  }

  QString myFilterString = myQFileDialog->selectedFilter()+";;";
#ifdef QGISDEBUG

  std::cout << "Selected filter: " << myFilterString.toLocal8Bit().data() << std::endl;
  std::cout << "Image type to be passed to mapcanvas: " << (myFilterMap[myFilterString]).toLocal8Bit().data() << std::endl;
#endif

  myQSettings.writeEntry("/UI/lastSaveAsImageFilter" , myFilterString);
  myQSettings.writeEntry("/UI/lastSaveAsImageDir", myQFileDialog->directory().absolutePath());

  if ( myOutputFileNameQString !="")
  {

    //save the mapview to the selected file
    mMapCanvas->saveAsImage(myOutputFileNameQString,NULL,myFilterMap[myFilterString]);
    statusBar()->message(tr("Saved map image to") + " " + myOutputFileNameQString);
  }

} // saveMapAsImage



//overloaded version of the above function
void QgisApp::saveMapAsImage(QString theImageFileNameQString, QPixmap * theQPixmap)
{
  if ( theImageFileNameQString=="")
  {
    //no filename chosen
    return;
  }
  else
  {
    //force the size of the canvase
    mMapCanvas->resize(theQPixmap->width(), theQPixmap->height());
    //save the mapview to the selected file
    mMapCanvas->saveAsImage(theImageFileNameQString,theQPixmap);
  }
} // saveMapAsImage


//reimplements method from base (gui) class
void QgisApp::addAllToOverview()
{
  mOverviewCanvas->freeze(true);
  std::map<QString, QgsMapLayer *> myMapLayers = QgsMapLayerRegistry::instance()->mapLayers();
  std::map<QString, QgsMapLayer *>::iterator myMapIterator;
  for ( myMapIterator = myMapLayers.begin(); myMapIterator != myMapLayers.end(); ++myMapIterator )
  {
    QgsMapLayer * myMapLayer = myMapIterator->second;
    myMapLayer->inOverview(true); // won't do anything if already in overview
  }
  // draw the map
  mOverviewCanvas->clear();
  mOverviewCanvas->freeze(false);
  // For Qt4, deprecate direct calling of render().  Let render() be called by the 
  // paint event loop of the overview canvas widget.
  // XXX - this doesn't work -- or nobody implemented it so render() is still enabled
  // [gsherman]
  mOverviewCanvas->render();
  mOverviewCanvas->update();

  // notify the project we've made a change
  QgsProject::instance()->dirty(true);
}

//reimplements method from base (gui) class
void QgisApp::removeAllFromOverview()
{
  mOverviewCanvas->freeze(true);
  std::map<QString, QgsMapLayer *> myMapLayers = QgsMapLayerRegistry::instance()->mapLayers();
  std::map<QString, QgsMapLayer *>::iterator myMapIterator;
  for ( myMapIterator = myMapLayers.begin(); myMapIterator != myMapLayers.end(); ++myMapIterator )
  {
    QgsMapLayer * myMapLayer = myMapIterator->second;
    if (myMapLayer->showInOverviewStatus())
    {
      myMapLayer->inOverview(false); // should generate updates, unless already not in overview
    }
  }
  // draw the map
  mOverviewCanvas->clear();
  mOverviewCanvas->freeze(false);
  // For Qt4, deprecate direct calling of render().  Let render() be called by the 
  // paint event loop of the overview canvas widget.
  // XXX - this doesn't work -- or nobody implemented it so render() is still enabled
  // [gsherman]
  mOverviewCanvas->render();
  mOverviewCanvas->update();

  // notify the project we've made a change
  QgsProject::instance()->dirty(true);
} // QgisApp::removeAllFromOverview()


//reimplements method from base (gui) class
void QgisApp::hideAllLayers()
{
  // what's the point if we don't have any layers?
  if ( QgsMapLayerRegistry::instance()->mapLayers().empty() )
  {
    return;
  }

#ifdef QGISDEBUG
  std::cout << "hiding all layers!" << std::endl;
#endif

  mMapCanvas->freeze(true);
  mOverviewCanvas->freeze(true);
  std::map<QString, QgsMapLayer *> myMapLayers = QgsMapLayerRegistry::instance()->mapLayers();
  std::map<QString, QgsMapLayer *>::iterator myMapIterator;
  for ( myMapIterator = myMapLayers.begin(); myMapIterator != myMapLayers.end(); ++myMapIterator )
  {
    QgsMapLayer * myMapLayer = myMapIterator->second;
    myMapLayer->setVisible(false);
  }
  // draw the map
  mMapCanvas->clear();
  mMapCanvas->freeze(false);
  mOverviewCanvas->freeze(false);
  // For Qt4, deprecate direct calling of render().  Let render() be called by the 
  // paint event loop of the map canvas widget.
  // XXX - this doesn't work -- or nobody implemented it so render() is still enabled
  // [gsherman]
  mMapCanvas->render();
  mMapCanvas->update();
  // For Qt4, deprecate direct calling of render().  Let render() be called by the 
  // paint event loop of the overview canvas widget.
  // XXX - this doesn't work -- or nobody implemented it so render() is still enabled
  // [gsherman]
  mOverviewCanvas->render();
  mOverviewCanvas->update();

  // notify the project we've made a change
  QgsProject::instance()->dirty(true);
}


// reimplements method from base (gui) class
void QgisApp::showAllLayers()
{
  // what's the point if we don't have any layers?
  if ( QgsMapLayerRegistry::instance()->mapLayers().empty() )
  {
    return;
  }

#ifdef QGISDEBUG
  std::cout << "Showing all layers!" << std::endl;
#endif

  mMapCanvas->freeze(true);
  mOverviewCanvas->freeze(true);
  std::map<QString, QgsMapLayer *> myMapLayers = QgsMapLayerRegistry::instance()->mapLayers();
  std::map<QString, QgsMapLayer *>::iterator myMapIterator;
  for ( myMapIterator = myMapLayers.begin(); myMapIterator != myMapLayers.end(); ++myMapIterator )
  {
    QgsMapLayer * myMapLayer = myMapIterator->second;
    myMapLayer->setVisible(true);
  }
  // draw the map
  mMapCanvas->clear();
  mMapCanvas->freeze(false);
  mOverviewCanvas->freeze(false);
  // For Qt4, deprecate direct calling of render().  Let render() be called by the 
  // paint event loop of the map canvas widget.
  // XXX - this doesn't work -- or nobody implemented it so render() is still enabled
  // [gsherman]
  mMapCanvas->render();
  mMapCanvas->update();
  // For Qt4, deprecate direct calling of render().  Let render() be called by the 
  // paint event loop of the overview canvas widget.
  // XXX - this doesn't work -- or nobody implemented it so render() is still enabled
  // [gsherman]
  mOverviewCanvas->render();
  mOverviewCanvas->update();

  // notify the project we've made a change
  QgsProject::instance()->dirty(true);
}

void QgisApp::exportMapServer()
{
  // check to see if there are any layers to export
  if (mMapCanvas->layerCount() > 0)
  {
    QgsMapserverExport *mse = new QgsMapserverExport(mMapCanvas, this);
    if (mse->exec())
    {
      mse->write();
    }
    delete mse;
  }
  else
  {
    QMessageBox::warning(this, tr("No Map Layers"),
        tr("No layers to export. You must add at least one layer to the map in order to export the view."));
  }
}
void QgisApp::zoomIn()
{
  /*  QWMatrix m = mMapCanvas->worldMatrix();
      m.scale( 2.0, 2.0 );
      mMapCanvas->setWorldMatrix( m );
      */
  qDebug ("Setting map tool to zoomIn");
  if ( mMapCanvas->mapTool() != QGis::ZoomIn && mMapCanvas->mapTool() != QGis::ZoomOut
      && mMapCanvas->mapTool() != QGis::Pan )
  {
    mPreviousNonZoomMapTool = mMapCanvas->mapTool();
  }

  mMapTool = QGis::ZoomIn;
  mMapCanvas->setMapTool(mMapTool);
  // set the cursor


  QPixmap myZoomInQPixmap = QPixmap((const char **) zoom_in);
  delete mMapCursor;
  mMapCursor = new QCursor(myZoomInQPixmap, 7, 7);
  mMapCanvas->setCursor(*mMapCursor);
  // scale the extent
  /* QgsRect ext = mMapCanvas->extent();
     ext.scale(0.5);
     mMapCanvas->setExtent(ext);
     statusBar()->message(ext.stringRep(2));
     mMapCanvas->clear();
     mMapCanvas->render(); */

  // notify the project we've made a change
  QgsProject::instance()->dirty(true);
}


void QgisApp::zoomOut()
{
  if ( mMapCanvas->mapTool() != QGis::ZoomIn && mMapCanvas->mapTool() != QGis::ZoomOut
      && mMapCanvas->mapTool() != QGis::Pan )
  {
    mPreviousNonZoomMapTool = mMapCanvas->mapTool();
  }

  mMapTool = QGis::ZoomOut;
  mMapCanvas->setMapTool(mMapTool);

  QPixmap myZoomOutQPixmap = QPixmap((const char **) zoom_out);
  delete mMapCursor;
  mMapCursor = new QCursor(myZoomOutQPixmap, 7, 7);
  mMapCanvas->setCursor(*mMapCursor);

  /*    QWMatrix m = mMapCanvas->worldMatrix();
        m.scale( 0.5, 0.5 );
        mMapCanvas->setWorldMatrix( m );
        */
  // notify the project we've made a change
  QgsProject::instance()->dirty(true);
}

void QgisApp::zoomToSelected()
{
  mMapCanvas->zoomToSelected();

  // notify the project we've made a change
  QgsProject::instance()->dirty(true);
}

void QgisApp::pan()
{
  if ( mMapCanvas->mapTool() != QGis::ZoomIn && mMapCanvas->mapTool() != QGis::ZoomOut
      && mMapCanvas->mapTool() != QGis::Pan )
  {
    mPreviousNonZoomMapTool = mMapCanvas->mapTool();
  }

  mMapTool = QGis::Pan;
  mMapCanvas->setMapTool(mMapTool);
  QBitmap panBmp(16, 16, pan_bits, true);
  QBitmap panBmpMask(16, 16, pan_mask_bits, true);
  delete mMapCursor;
  mMapCursor = new QCursor(panBmp, panBmpMask, 5, 5);
  mMapCanvas->setCursor(*mMapCursor);
  // notify the project we've made a change
  QgsProject::instance()->dirty(true);

}

void QgisApp::zoomFull()
{
  mMapCanvas->zoomFullExtent();
  // notify the project we've made a change
  QgsProject::instance()->dirty(true);

}

void QgisApp::zoomPrevious()
{
  mMapCanvas->zoomPreviousExtent();
  // notify the project we've made a change
  QgsProject::instance()->dirty(true);

}

void QgisApp::identify()
{
  mMapTool = QGis::Identify;
  mMapCanvas->setMapTool(mMapTool);

  QPixmap myIdentifyQPixmap = QPixmap((const char **) identify_cursor);
  delete mMapCursor;
  mMapCursor = new QCursor(myIdentifyQPixmap, 1, 1);
  mMapCanvas->setCursor(*mMapCursor);
  mActionIdentify->setOn(true);
}

void QgisApp::measure()
{
  mMapTool = QGis::MeasureDist;
  mMapCanvas->setMapTool(mMapTool);

  //QPixmap pm = QPixmap((const char **)  capture_point_cursor);
  //delete mMapCursor;
  //mMapCursor = new QCursor(pm, 8, 8);
  mMapCanvas->setCursor( Qt::CrossCursor );
  mActionMeasure->setOn(true);
}

void QgisApp::measureArea()
{
  mMapTool = QGis::MeasureArea;
  mMapCanvas->setMapTool(mMapTool);

  mMapCanvas->setCursor( Qt::CrossCursor );
  mActionMeasureArea->setOn(true);
}

void QgisApp::stopZoom() 
{
  mActionZoomIn->setOn(false);
  mActionZoomIn->setOn(false);
  mActionPan->setOn(false);

  switch ( mPreviousNonZoomMapTool ) {
    case QGis::Identify:
      identify();
      break;
    case QGis::Select:
      select();
      break;
    case QGis::CapturePoint:
      capturePoint();
      break;
    case QGis::CaptureLine:
      captureLine();
      break;
    case QGis::CapturePolygon:
      capturePolygon();
      break;
    case QGis::EmitPoint:
      mMapCanvas->setMapTool( QGis::EmitPoint );
      break;
    case QGis::MeasureDist:
      measure();
      break;
    case QGis::MeasureArea:
      measureArea();
      break;
  }
}


void QgisApp::attributeTable()
{
  QgsMapLayer *layer = mMapLegend->currentLayer();
  if (layer)
  {
    layer->table();
  }
  else
  {
    QMessageBox::information(this, tr("No Layer Selected"),
        tr("To open an attribute table, you must select a layer in the legend"));
  }
}

void QgisApp::deleteSelected()
{
  QgsMapLayer *layer = mMapLegend->currentLayer();
  if(layer)
  {
    QgsVectorLayer* vlayer = dynamic_cast<QgsVectorLayer*>(layer);
    if(vlayer)
    {
      if(!vlayer->deleteSelectedFeatures())
      {
        QMessageBox::information(this, tr("Problem deleting features"),
            tr("A problem occured during deletion of features"));
      }
    }
    else
    {
      QMessageBox::information(this, tr("No Vector Layer Selected"),
          tr("Deleting features only works on vector layers"));
    }
  }
  else
  {
    QMessageBox::information(this, tr("No Layer Selected"),
        tr("To delete features, you must select a vector layer in the legend"));
  }

  // notify the project we've made a change
  QgsProject::instance()->dirty(true);
}

void QgisApp::capturePoint()
{
  {
    // set current map tool to select
    mMapCanvas->setMapTool(QGis::CapturePoint);
    QPixmap mySelectQPixmap = QPixmap((const char **) capture_point_cursor);
    delete mMapCursor;
    mMapCursor = new QCursor(mySelectQPixmap, 8, 8);
    mMapCanvas->setCursor(*mMapCursor);

    mActionCapturePoint->setOn(true);
  }
}

void QgisApp::captureLine()
{
  {
    mMapCanvas->setMapTool(QGis::CaptureLine);

    QPixmap mySelectQPixmap = QPixmap((const char **) capture_point_cursor);
    delete mMapCursor;
    mMapCursor = new QCursor(mySelectQPixmap, 8, 8);
    mMapCanvas->setCursor(*mMapCursor);

    mActionCaptureLine->setOn(true);
  }
}

void QgisApp::capturePolygon()
{
  {
    mMapCanvas->setMapTool(QGis::CapturePolygon);

    QPixmap mySelectQPixmap = QPixmap((const char **) capture_point_cursor);
    delete mMapCursor;
    mMapCursor = new QCursor(mySelectQPixmap, 8, 8);
    mMapCanvas->setCursor(*mMapCursor);

    mActionCapturePolygon->setOn(true);
  }
}

void QgisApp::select()
{

  // set current map tool to select
  mMapCanvas->setMapTool(QGis::Select);

  QPixmap mySelectQPixmap = QPixmap((const char **) select_cursor);
  delete mMapCursor;
  mMapCursor = new QCursor(mySelectQPixmap, 1, 1);
  mMapCanvas->setCursor(*mMapCursor);
  mActionSelect->setOn(true);
}


// TODO - select a real cursor
void QgisApp::addVertex()
{

#ifdef QGISDEBUG
  std::cout << "QgisApp::addVertex." << std::endl;
#endif

  {
    // set current map tool to select
    mMapCanvas->setMapTool(QGis::AddVertex);
    QPixmap mySelectQPixmap = QPixmap((const char **) capture_point_cursor);
    delete mMapCursor;
    mMapCursor = new QCursor(mySelectQPixmap, 8, 8);
    mMapCanvas->setCursor(*mMapCursor);
    mActionAddVertex->setOn(true);
  }
}

// TODO - select a real cursor
void QgisApp::moveVertex()
{

#ifdef QGISDEBUG
  std::cout << "QgisApp::moveVertex." << std::endl;
#endif

  {
    // set current map tool to select
    mMapCanvas->setMapTool(QGis::MoveVertex);
    QPixmap mySelectQPixmap = QPixmap((const char **) capture_point_cursor);
    delete mMapCursor;
    mMapCursor = new QCursor(mySelectQPixmap, 8, 8);
    mMapCanvas->setCursor(*mMapCursor);
    mActionMoveVertex->setOn(true);
  }
}


// TODO - select a real cursor
void QgisApp::deleteVertex()
{

#ifdef QGISDEBUG
  std::cout << "QgisApp::deleteVertex." << std::endl;
#endif

  {
    // set current map tool to select
    mMapCanvas->setMapTool(QGis::DeleteVertex);
    QPixmap mySelectQPixmap = QPixmap((const char **) capture_point_cursor);
    delete mMapCursor;
    mMapCursor = new QCursor(mySelectQPixmap, 8, 8);
    mMapCanvas->setCursor(*mMapCursor);
    mActionDeleteVertex->setOn(true);
  }
}


void QgisApp::editCut()
{
  if (activeLayer())
  {
    // Test for feature support in this layer
    QgsVectorLayer* activeVectorLayer = dynamic_cast<QgsVectorLayer*>(activeLayer());

    if (activeVectorLayer != 0)
    {

      clipboard()->replaceWithCopyOf( *(activeVectorLayer->selectedFeatures()) );
      activeVectorLayer->deleteSelectedFeatures();
    }  
  }  
}


void QgisApp::editCopy()
{
#ifdef QGISDEBUG
  std::cerr << "QgisApp::editCopy: entered."
    << std::endl;
#endif
  if (activeLayer())
  {
#ifdef QGISDEBUG
    std::cerr << "QgisApp::editCopy: has active layer, feature type " << activeLayer()->featureType() << "."
      << std::endl;
#endif
    // Test for feature support in this layer
    QgsVectorLayer* activeVectorLayer = dynamic_cast<QgsVectorLayer*>(activeLayer());

    if (activeVectorLayer != 0)
    {
#ifdef QGISDEBUG
      std::cerr << "QgisApp::editCopy: has active vector layer."
        << std::endl;
#endif
      clipboard()->replaceWithCopyOf( *(activeVectorLayer->selectedFeatures()) );
    }  
  }  
}


void QgisApp::editPaste()
{
  if (activeLayer())
  {
    // Test for feature support in this layer
    QgsVectorLayer* activeVectorLayer = dynamic_cast<QgsVectorLayer*>(activeLayer());

    if (activeVectorLayer != 0)
    {
      //      activeVectorLayer->addFeatures( &(clipboard()->copyOf()) );
      activeVectorLayer->addFeatures( clipboard()->copyOf() );
    }  
  }  
}


void QgisApp::pasteTransformations()
{
  QgsPasteTransformations *pt = new QgsPasteTransformations();

  mMapCanvas->freeze();

  pt->exec();
}


void QgisApp::drawPoint(double x, double y)
{
  QPainter paint;
  //  QWMatrix mat (mScaleFactor, 0, 0, mScaleFactor, 0, 0);
  paint.begin(mMapCanvas);
  // paint.setWorldMatrix(mat);
  paint.setWindow(*mMapWindow);

  paint.setPen(Qt::blue);
  paint.drawPoint((int) x, (int) y);
  paint.end();
}

void QgisApp::drawLayers()
{
  // what's the point if we don't have any layers?
  if ( QgsMapLayerRegistry::instance()->mapLayers().empty() )
  {
    return;
  }

  std::cout << "In  QgisApp::drawLayers()" << std::endl;
  mMapCanvas->setDirty(true);
  // For Qt4, deprecate direct calling of render().  Let render() be called by the 
  // paint event loop of the map canvas widget.
  // XXX - this doesn't work -- or nobody implemented it so render() is still enabled
  // [gsherman]
  mMapCanvas->render();
  mMapCanvas->update();

}

void QgisApp::showMouseCoordinate(QgsPoint & p)
{
  mCoordsLabel->setText(p.stringRep(mMousePrecisionDecimalPlaces));
}

void QgisApp::showScale(QString theScale)
{
  mScaleLabel->setText(theScale);
}

void QgisApp::testButton()
{
  /* QgsShapeFileLayer *sfl = new QgsShapeFileLayer("foo");
     mMapCanvas->addLayer(sfl); */
  //      delete sfl;

}

void QgisApp::menubar_highlighted( int i )
{
  // used to save us from re-enabling layer menu items every single time the
  // user tweaks the layers drop down menu
  static bool enabled;

  if ( 6 == i )               // XXX I hate magic numbers; where is '6' defined
    // XXX for Layers menu?
  {
    // first, if there are NO layers, disable everything that assumes we
    // have at least one layer loaded
    if ( QgsMapLayerRegistry::instance()->mapLayers().empty() )
    {
      mActionRemoveLayer->setEnabled(false);
      mActionRemoveAllFromOverview->setEnabled(false);
      mActionInOverview->setEnabled(false);
      mActionShowAllLayers->setEnabled(false);
      mActionHideAllLayers->setEnabled(false);
      mActionOpenTable->setEnabled(false);
      mActionLayerProperties->setEnabled(false);

      enabled = false;
    }
    else
    {
      if ( ! enabled )
      {
        mActionRemoveLayer->setEnabled(true);
        mActionRemoveAllFromOverview->setEnabled(true);
        mActionInOverview->setEnabled(true);
        mActionShowAllLayers->setEnabled(true);
        mActionHideAllLayers->setEnabled(true);
        mActionOpenTable->setEnabled(true);
        mActionLayerProperties->setEnabled(true);
      }
    }
  }
} // QgisApp::menubar_highlighted( int i )





void QgisApp::inOverview( bool in_overview )
{
#ifdef QGISDEBUG
  std::cout << "QGisApp::inOverview(" << in_overview << ")" << std::endl;
#endif

  QgsMapLayer* layer = mMapLegend->currentLayer();
  if(layer)
  {
    layer->inOverview( in_overview );

    // For Qt4, deprecate direct calling of render().  Let render() be called by the 
    // paint event loop of the overview canvas widget.
    //but adding a layer to overview canvas does only work at the moment when calling render()...
    // XXX - this doesn't work -- or nobody implemented it so render() is still enabled
    // [gsherman]
    mOverviewCanvas->render();
    //mOverviewCanvas->update();
  }
} // QgisApp::inOverview(bool)



void QgisApp::removeLayer()
{
#ifdef QGISDEBUG
  std::cout << "QGisApp Removing layer" << std::endl;
#endif
  //make sure canvase is not rendering first by faking an escape keypress
  emit keyPressEvent(new QKeyEvent(QEvent::KeyPress ,Qt::Key_Escape,Qt::Key_Escape,0 ));
  mMapCanvas->freeze();
  QTreeWidgetItem *lvi = mMapLegend->currentItem();
  if(lvi)
  {
    QgsLegendLayerFile* llf = dynamic_cast<QgsLegendLayerFile*>(lvi);
    QgsLegendLayer* ll = dynamic_cast<QgsLegendLayer*>(lvi);
    if(llf)
    {
      QgsMapLayer *layer = llf->layer();
      //call the registry to unregister the layer. It will in turn
      //fire a qt signal to notify any objects using that layer that they should
      //remove it immediately
      QgsMapLayerRegistry::instance()->removeMapLayer(layer->getLayerID());
      mOverviewCanvas->freeze(false);
      // draw the map
      mOverviewCanvas->zoomFullExtent();
      mOverviewCanvas->clear();
      // For Qt4, deprecate direct calling of render().  Let render() be called by the 
      // paint event loop of the overview canvas widget.
      // XXX - this doesn't work -- or nobody implemented it so render() is still enabled
      // [gsherman]
      mOverviewCanvas->render();
      mOverviewCanvas->update();
      mMapCanvas->freeze(false);

      //remove remaining digitising acetates
      mMapCanvas->removeDigitizingLines();

      // draw the map
      mMapCanvas->clear();
      // For Qt4, deprecate direct calling of render().  Let render() be called by the 
      // paint event loop of the map canvas widget.
      // XXX - this doesn't work -- or nobody implemented it so render() is still enabled
      // [gsherman]
      mMapCanvas->render();
      mMapCanvas->update();
    }
    else if(ll)
    {
      mMapLegend->legendLayerRemove();
    }
  }
}


void QgisApp::removeAllLayers()
{
  QgsMapLayerRegistry::instance()->removeAllMapLayers();
  mOverviewCanvas->clear();
  mMapCanvas->clear();
} //remove all layers


void QgisApp::zoomToLayerExtent()
{
  // zoom only if one or more layers loaded
  if(QgsMapLayerRegistry::instance()->count() > 0)
  {
    // get the selected item
    QTreeWidgetItem *li = mMapLegend->currentItem();
    QgsLegendLayerFile* llf = dynamic_cast<QgsLegendLayerFile*>(li);
    if(llf)
    {
      QgsMapLayer *layer = llf->layer();
      // Check if the layer extent has to be transformed to the map canvas
      // coordinate system
#ifdef QGISDEBUG
      std::cout << "Layer extent is : " << (layer->extent()).stringRep().toLocal8Bit().data() << std::endl;
#endif
      if (QgsProject::instance()->readNumEntry("SpatialRefSys","/ProjectionsEnabled",0)!=0)
      {
        QgsCoordinateTransform *ct = layer->coordinateTransform();
        try {
          QgsRect transformedExtent = ct->transform(layer->extent());
          mMapCanvas->setExtent(transformedExtent);
#ifdef QGISDEBUG
          std::cout << "Canvas extent is : " << transformedExtent.stringRep().toLocal8Bit().data() << std::endl;
#endif
        }
        catch(QgsCsException &cse)
        {
#ifdef QGISDEBUG
          std::cout << "Caught transform error in zoomToLayerExtent(). "
            << "Setting untransformed extents." << std::endl;
#endif	
          mMapCanvas->setExtent(layer->extent());
        }
      }
      else
      {
        mMapCanvas->setExtent(layer->extent());
      }
      mMapCanvas->clear();
      // For Qt4, deprecate direct calling of render().  Let render() be called by the 
      // paint event loop of the map canvas widget.
      // XXX - this doesn't work -- or nobody implemented it so render() is still enabled
      // [gsherman]
      mMapCanvas->render();
      mMapCanvas->update();

      // notify the project we've made a change
      QgsProject::instance()->dirty(true);
    }
  }
} // QgisApp::zoomToLayerExtent()



void QgisApp::currentLayerChanged(Q3ListViewItem * lvi)
{
  if (lvi)
  {
    // disable/enable toolbar buttons as appropriate based on selected
    // layer type

    toolPopupCapture->setItemEnabled(0,FALSE);
    toolPopupCapture->setItemEnabled(1,FALSE);
    toolPopupCapture->setItemEnabled(2,FALSE);
    toolPopupCapture->setItemEnabled(3,FALSE);


    QgsLegendLayerFile* llf = dynamic_cast<QgsLegendLayerFile*>(lvi);
    if(llf)
    {
      QgsMapLayer *layer = llf->layer();
      if (layer->type() == QgsMapLayer::RASTER)
      {
        //actionIdentify->setEnabled(FALSE);
        mActionSelect->setEnabled(FALSE);
        mActionOpenTable->setEnabled(FALSE);
        // if one of these map tools is selected, set cursor to default
        if (mMapTool == QGis::Identify || mMapTool == QGis::Select || mMapTool == QGis::Table)
        {
          delete mMapCursor;
          mMapCursor = new QCursor();
          mMapCanvas->setCursor(*mMapCursor);
        }
      }
      else
      {
        //vector layer editing buttons
        QgsMapLayer* mlayer=llf->layer();
        if(mlayer)
        {
          QgsVectorLayer* vlayer=dynamic_cast<QgsVectorLayer*>(mlayer);
          if(vlayer)
          {
            const QgsVectorDataProvider* provider=vlayer->getDataProvider();
            if(provider)
            {
              int cap=vlayer->getDataProvider()->capabilities();
              if(cap&QgsVectorDataProvider::DeleteFeatures)
              {
                toolPopupCapture->setItemEnabled(3,TRUE);
              }
              if(cap&QgsVectorDataProvider::AddFeatures)
              {
                if(vlayer->vectorType()==QGis::Point)
                {
                  toolPopupCapture->setItemEnabled(0,TRUE);
                }
                else if(vlayer->vectorType()==QGis::Line)
                {
                  toolPopupCapture->setItemEnabled(1,TRUE);
                }
                else if(vlayer->vectorType()==QGis::Polygon)
                {
                  toolPopupCapture->setItemEnabled(2,TRUE);
                }
              }
            }
          }

          mActionIdentify->setEnabled(TRUE);
          mActionSelect->setEnabled(TRUE);
          mActionOpenTable->setEnabled(TRUE);
          // if one of these map tools is selected, make sure appropriate cursor gets set
          switch (mMapTool)
          {
            case QGis::Identify:
              identify();
              break;
            case QGis::Select:
              select();
              break;
            case QGis::Table:
              attributeTable();
              break;
          }
        }

        //let the mapcanvas know that the current layer changed
        //so any remaining digitizing acetates can be removed
        mMapCanvas->removeDigitizingLines();

        // notify the project we've made a change
        QgsProject::instance()->dirty(true);
      }
    }
  }
} // QgisApp::currentLayerChanged


QgisIface *QgisApp::getInterface()
{
  return mQgisInterface;
}

void QgisApp::showPluginManager()
{
  QgsPluginManager *pm = new QgsPluginManager(this);
  if (pm->exec())
  {
    // load selected plugins
    std::vector < QgsPluginItem > pi = pm->getSelectedPlugins();
    std::vector < QgsPluginItem >::iterator it = pi.begin();
    while (it != pi.end())
    {
      QgsPluginItem plugin = *it;
      loadPlugin(plugin.name(), plugin.description(), plugin.fullPath());
      it++;
    }
  }
}

void QgisApp::loadPlugin(QString name, QString description, QString theFullPathName)
{
  QSettings settings;
  // first check to see if its already loaded
  QgsPluginRegistry *pRegistry = QgsPluginRegistry::instance();
  QString lib = pRegistry->library(name);
  if (lib.length() > 0)
  {
    // plugin is loaded
    // QMessageBox::warning(this, "Already Loaded", description + " is already loaded");
  }
  else
  {
    QLibrary *myLib = new QLibrary(theFullPathName);
#ifdef QGISDEBUG

    std::cerr << "Library name is " << myLib->library().toLocal8Bit().data() << std::endl;
#endif

    bool loaded = myLib->load();
    if (loaded)
    {
#ifdef QGISDEBUG
      std::cerr << "Loaded test plugin library" << std::endl;
      std::cerr << "Attempting to resolve the classFactory function" << std::endl;
#endif

      type_t *pType = (type_t *) myLib->resolve("type");


      switch (pType())
      {
        case QgisPlugin::UI:
          {
            // UI only -- doesn't use mapcanvas
            create_ui *cf = (create_ui *) myLib->resolve("classFactory");
            if (cf)
            {
              QgisPlugin *pl = cf(this, mQgisInterface);
              if (pl)
              {
                pl->initGui();
                // add it to the plugin registry
                pRegistry->addPlugin(myLib->library(), name, pl);
                //add it to the qsettings file [ts]
                settings.writeEntry("/Plugins/" + name, true);
              }
              else
              {
                // something went wrong
                QMessageBox::warning(this, tr("Error Loading Plugin"), tr("There was an error loading %1."));
                //disable it to the qsettings file [ts]
                settings.writeEntry("/Plugins/" + name, false);
              }
            }
            else
            {
              //#ifdef QGISDEBUG
              std::cerr << "Unable to find the class factory for " << theFullPathName.toLocal8Bit().data() << std::endl;
              //#endif
            }

          }
          break;
        case QgisPlugin::MAPLAYER:
          {
            // Map layer - requires interaction with the canvas
            create_it *cf = (create_it *) myLib->resolve("classFactory");
            if (cf)
            {
              QgsMapLayerInterface *pl = cf();
              if (pl)
              {
                // set the main window pointer for the plugin
                pl->setQgisMainWindow(this);
                pl->initGui();
                //add it to the qsettings file [ts]
                settings.writeEntry("/Plugins/" + name, true);

              }
              else
              {
                // something went wrong
                QMessageBox::warning(this, tr("Error Loading Plugin"), tr("There was an error loading %1."));
                //add it to the qsettings file [ts]
                settings.writeEntry("/Plugins/" + name, false);
              }
            }
            else
            {
              //#ifdef QGISDEBUG
              std::cerr << "Unable to find the class factory for " << theFullPathName.toLocal8Bit().data() << std::endl;
              //#endif
            }
          }
          break;
        default:
          // type is unknown
          //#ifdef QGISDEBUG
          std::cerr << "Plugin " << theFullPathName.toLocal8Bit().data() << " did not return a valid type and cannot be loaded" << std::endl;
          //#endif
          break;
      }

      /*  }else{
          std::cout << "Unable to find the class factory for " << mFullPathName << std::endl;
          } */

  }
  else
  {
    //#ifdef QGISDEBUG
    std::cerr << "Failed to load " << theFullPathName.toLocal8Bit().data() << "\n";
    //#endif
  }
}
}
void QgisApp::testMapLayerPlugins()
{
#ifndef WIN32
  // map layer plugins live in their own directory (somewhere to be determined)
  QDir mlpDir("../plugins/maplayer", "*.so.1.0.0", QDir::Name | QDir::IgnoreCase, QDir::Files);
  if (mlpDir.count() == 0)
  {
    QMessageBox::information(this, tr("No MapLayer Plugins"), tr("No MapLayer plugins in ../plugins/maplayer"));
  }
  else
  {
    for (unsigned i = 0; i < mlpDir.count(); i++)
    {
#ifdef QGISDEBUG
      std::cout << "Getting information for plugin: " << mlpDir[i].toLocal8Bit().data() << std::endl;
      std::cout << "Attempting to load the plugin using dlopen\n";
#endif
      //          void *handle = dlopen("../plugins/maplayer/" + mlpDir[i], RTLD_LAZY);
      void *handle = dlopen(("../plugins/maplayer/" + mlpDir[i]).toLocal8Bit().data(), RTLD_LAZY | RTLD_GLOBAL );
      if (!handle)
      {
#ifdef QGISDEBUG
        std::cout << "Error in dlopen: " << dlerror() << std::endl;
#endif

      }
      else
      {
#ifdef QGISDEBUG
        std::cout << "dlopen suceeded" << std::endl;
#endif

        dlclose(handle);
      }

      QLibrary *myLib = new QLibrary("../plugins/maplayer/" + mlpDir[i]);
#ifdef QGISDEBUG

      std::cout << "Library name is " << myLib->library().toLocal8Bit().data() << std::endl;
#endif

      bool loaded = myLib->load();
      if (loaded)
      {
#ifdef QGISDEBUG
        std::cout << "Loaded test plugin library" << std::endl;
        std::cout << "Attempting to resolve the classFactory function" << std::endl;
#endif

        create_it *cf = (create_it *) myLib->resolve("classFactory");

        if (cf)
        {
#ifdef QGISDEBUG
          std::cout << "Getting pointer to a MapLayerInterface object from the library\n";
#endif

          QgsMapLayerInterface *pl = cf();
          if (pl)
          {
#ifdef QGISDEBUG
            std::cout << "Instantiated the maplayer test plugin\n";
#endif
            // set the main window pointer for the plugin
            pl->setQgisMainWindow(this);
#ifdef QGISDEBUG
            //the call to getInt is deprecated and this line should be removed
            //std::cout << "getInt returned " << pl->getInt() << " from map layer plugin\n";
#endif
            // set up the gui
            pl->initGui();
          }
          else
          {
#ifdef QGISDEBUG
            std::cout << "Unable to instantiate the maplayer test plugin\n";
#endif

          }
        }
      }
      else
      {
#ifdef QGISDEBUG
        std::cout << "Failed to load " << mlpDir[i].toLocal8Bit().data() << "\n";
#endif

      }
    }
  }
#endif //#ifndef WIN32
}
void QgisApp::testPluginFunctions()
{
  // test maplayer plugins first
  testMapLayerPlugins();
  if (false)
  {
    // try to load plugins from the plugin directory and test each one

    QDir pluginDir("../plugins", "*.so*", QDir::Name | QDir::IgnoreCase, QDir::Files | QDir::NoSymLinks);
    //pluginDir.setFilter(QDir::Files || QDir::NoSymLinks);
    //pluginDir.setNameFilter("*.so*");
    if (pluginDir.count() == 0)
    {
      QMessageBox::information(this, tr("No Plugins"),
          tr("No plugins found in ../plugins. To test plugins, start qgis from the src directory"));
    }
    else
    {

      for (unsigned i = 0; i < pluginDir.count(); i++)
      {
#ifdef QGISDEBUG
        std::cout << "Getting information for plugin: " << pluginDir[i].toLocal8Bit().data() << std::endl;
#endif

        QLibrary *myLib = new QLibrary("../plugins/" + pluginDir[i]); //"/home/gsherman/development/qgis/plugins/" + pluginDir[i]);
#ifdef QGISDEBUG

        std::cout << "Library name is " << myLib->library().toLocal8Bit().data() << std::endl;
#endif
        //QLibrary myLib("../plugins/" + pluginDir[i]);
#ifdef QGISDEBUG

        std::cout << "Attempting to load ../plugins/" << pluginDir[i].toLocal8Bit().data() << std::endl;
#endif
        /*  void *handle = dlopen("/home/gsherman/development/qgis/plugins/" + pluginDir[i], RTLD_LAZY);
            if (!handle) {
            std::cout << "Error in dlopen: " <<  dlerror() << std::endl;

            }else{
            std::cout << "dlopen suceeded" << std::endl;
            dlclose(handle);
            }

*/
        bool loaded = myLib->load();
        if (loaded)
        {
#ifdef QGISDEBUG
          std::cout << "Loaded test plugin library" << std::endl;
          std::cout << "Getting the name of the plugin" << std::endl;
#endif

          name_t *pName = (name_t *) myLib->resolve("name");
          if (pName)
          {
            QMessageBox::information(this, tr("Name"), tr("Plugin %1 is named %2").arg(pluginDir[i]).arg(pName()));
          }
#ifdef QGISDEBUG
          std::cout << "Attempting to resolve the classFactory function" << std::endl;
#endif

          create_t *cf = (create_t *) myLib->resolve("classFactory");

          if (cf)
          {
#ifdef QGISDEBUG
            std::cout << "Getting pointer to a QgisPlugin object from the library\n";
#endif

            QgisPlugin *pl = cf(this, mQgisInterface);
#ifdef QGISDEBUG

            std::cout << "Displaying name, version, and description\n";
            std::cout << "Plugin name: " << pl->name().toLocal8Bit().data() << std::endl;
            std::cout << "Plugin version: " << pl->version().toLocal8Bit().data() << std::endl;
            std::cout << "Plugin description: " << pl->description().toLocal8Bit().data() << std::endl;
#endif

            QMessageBox::information(this, tr("Plugin Information"), tr("QGis loaded the following plugin:") +
                tr("Name: %1").arg(pl->name()) + "\n" + tr("Version: %1").arg(pl->version()) + "\n" +
                tr("Description: %1").arg(pl->description()));
            // unload the plugin (delete it)
#ifdef QGISDEBUG

            std::cout << "Attempting to resolve the unload function" << std::endl;
#endif
            /*
               unload_t *ul = (unload_t *) myLib.resolve("unload");
               if (ul) {
               ul(pl);
               std::cout << "Unloaded the plugin\n";
               } else {
               std::cout << "Unable to resolve unload function. Plugin was not unloaded\n";
               }
               */
          }
        }
        else
        {
          QMessageBox::warning(this, tr("Unable to Load Plugin"),
              tr("QGIS was unable to load the plugin from: %1").arg(pluginDir[i]));
#ifdef QGISDEBUG

          std::cout << "Unable to load library" << std::endl;
#endif

        }
      }
    }
  }
}


void QgisApp::checkQgisVersion()
{
  QApplication::setOverrideCursor(Qt::WaitCursor);
  /* QUrlOperator op = new QUrlOperator( "http://mrcc.com/qgis/version.txt" );
     connect(op, SIGNAL(data()), SLOT(urlData()));
     connect(op, SIGNAL(finished(QNetworkOperation)), SLOT(urlFinished(QNetworkOperation)));

     op.get(); */
  mSocket = new QTcpSocket(this);
  connect(mSocket, SIGNAL(connected()), SLOT(socketConnected()));
  connect(mSocket, SIGNAL(connectionClosed()), SLOT(socketConnectionClosed()));
  connect(mSocket, SIGNAL(readyRead()), SLOT(socketReadyRead()));
  connect(mSocket, SIGNAL(error(int)), SLOT(socketError(int)));
  mSocket->connectToHost("mrcc.com", 80);
}

void QgisApp::socketConnected()
{
  QTextStream os(mSocket);
  mVersionMessage = "";
  // send the qgis version string
  // os << qgisVersion << "\r\n";
  os << "GET /qgis/version.txt HTTP/1.0\n\n";


}

void QgisApp::socketConnectionClosed()
{
  QApplication::restoreOverrideCursor();
  // strip the header
  QString contentFlag = "#QGIS Version";
  int pos = mVersionMessage.find(contentFlag);
  if (pos > -1)
  {
    pos += contentFlag.length();
    /* std::cout << mVersionMessage << "\n ";
       std::cout << "Pos is " << pos <<"\n"; */
    mVersionMessage = mVersionMessage.mid(pos);
    QStringList parts = QStringList::split("|", mVersionMessage);
    // check the version from the  server against our version
    QString versionInfo;
    int currentVersion = parts[0].toInt();
    if (currentVersion > QGis::qgisVersionInt)
    {
      // show version message from server
      versionInfo = tr("There is a new version of QGIS available") + "\n";
    }
    else
    {
      if (QGis::qgisVersionInt > currentVersion)
      {
        versionInfo = tr("You are running a development version of QGIS") + "\n";
      }
      else
      {
        versionInfo = tr("You are running the current version of QGIS") + "\n";
      }
    }
    if (parts.count() > 1)
    {
      versionInfo += parts[1] + "\n\n" + tr("Would you like more information?");
      ;
      int result = QMessageBox::information(this, tr("QGIS Version Information"), versionInfo, tr("Yes"), tr("No"));
      if (result == 0)
      {
        // show more info
        QgsMessageViewer *mv = new QgsMessageViewer(this);
        mv->setCaption(tr("QGIS - Changes in CVS Since Last Release"));
        mv->setMessage(parts[2]);
        mv->exec();
      }
    }
    else
    {
      QMessageBox::information(this, tr("QGIS Version Information"), versionInfo);
    }
  }
  else
  {
    QMessageBox::warning(this, tr("QGIS Version Information"), tr("Unable to get current version information from server"));
  }
}
void QgisApp::socketError(int e)
{
  QApplication::restoreOverrideCursor();
  // get error type
  QString detail;
  switch (e)
  {
    case QTcpSocket::ErrConnectionRefused:
      detail = tr("Connection refused - server may be down");
      break;
    case QTcpSocket::ErrHostNotFound:
      detail = tr("QGIS server was not found");
      break;
    case QTcpSocket::ErrSocketRead:
      detail = tr("Error reading from server");
      break;
  }
  // show version message from server
  QMessageBox::critical(this, tr("QGIS Version Information"), tr("Unable to connect to the QGIS Version server") + "\n" + detail);
}

void QgisApp::socketReadyRead()
{
  while (mSocket->bytesAvailable() > 0)
  {
    char *data = new char[mSocket->bytesAvailable() + 1];
    memset(data, '\0', mSocket->bytesAvailable() + 1);
    mSocket->readBlock(data, mSocket->bytesAvailable());
    mVersionMessage += data;
    delete[]data;
  }

}
void QgisApp::options()
{
  QgsOptions *optionsDialog = new QgsOptions(this);
  if(optionsDialog->exec())
  {
    // set the theme if it changed
    setTheme(optionsDialog->theme());
    setupToolbarPopups(optionsDialog->theme());
    // set the visible flag for new layers
    mAddedLayersHidden = optionsDialog->newVisible();
    QSettings mySettings;
    mMapCanvas->enableAntiAliasing(mySettings.value("/qgis/enable_anti_aliasing").toBool());
  }
}

void QgisApp::helpContents()
{
  openURL("index.html");
}

void QgisApp::helpQgisHomePage()
{
  openURL("http://qgis.org", false);
}

void QgisApp::helpQgisSourceForge()
{
  openURL("http://sourceforge.net/projects/qgis", false);
}

void QgisApp::openURL(QString url, bool useQgisDocDirectory)
{
  // open help in user browser
  if (useQgisDocDirectory)
  {
    url = "file://" + mAppDir + "/share/qgis/doc/" + url;
  }
#ifdef Q_OS_MACX
  /* Use Mac OS X Launch Services which uses the user's default browser
   * and will just open a new window if that browser is already running.
   * QProcess creates a new browser process for each invocation and expects a
   * commandline application rather than a bundled application.
   */
  CFURLRef urlRef = CFURLCreateWithBytes(kCFAllocatorDefault,
      reinterpret_cast<const UInt8*>(url.utf8().data()), url.length(),
      kCFStringEncodingUTF8, NULL);
  OSStatus status = LSOpenCFURLRef(urlRef, NULL);
  CFRelease(urlRef);
#else
  // find a browser
  QSettings settings;
  QString browser = settings.readEntry("/qgis/browser");
  if (browser.length() == 0)
  {
    // ask user for browser and use it
    bool ok;
    QString myHeading = tr("QGIS Browser Selection");
    QString myMessage = tr("Enter the name of a web browser to use (eg. konqueror).\n");
    myMessage += tr("Enter the full path if the browser is not in your PATH.\n");
    myMessage += tr("You can change this option later by selecting Preferences from the Settings menu.");
    QString text = QInputDialog::getText(myHeading,
        myMessage,
        QLineEdit::Normal,
        QString::null, &ok, this);
    if (ok && !text.isEmpty())
    {
      // user entered something and pressed OK
      browser = text;
      // save the setting
      settings.writeEntry("/qgis/browser", browser);
    }
    else
    {
      browser = "";
    }

  }
  if (browser.length() > 0)
  {
    // find the installed location of the help files
    // open index.html using browser
    //XXX for debug on win32      QMessageBox::information(this,"Help opening...", browser + " - " + url);
    QProcess *helpProcess = new QProcess(this);
    QStringList myArgs;
    myArgs << url;
    helpProcess->start(browser,myArgs);
  }
  /*  mHelpViewer = new QgsHelpViewer(this,"helpviewer",false);
      mHelpViewer->showContent(mAppDir +"/share/doc","index.html");
      mHelpViewer->show(); */
#endif
}

/** Get a pointer to the currently selected map layer */
QgsMapLayer *QgisApp::activeLayer()
{
  QTreeWidgetItem *lvi = mMapLegend->currentItem();
  QgsMapLayer *layer = 0;
  if (lvi)
  {
    QgsLegendLayerFile* llf = dynamic_cast<QgsLegendLayerFile*>(lvi);
    if(llf)
    {
      layer = llf->layer();
    }
  }
  return layer;
}

QString QgisApp::activeLayerSource()
{
  QString source;
  QTreeWidgetItem *lvi = mMapLegend->currentItem();
  QgsMapLayer *layer = 0;
  if (lvi)
  {
    QgsLegendLayerFile* llf = dynamic_cast<QgsLegendLayerFile*>(lvi);
    if(llf)
    {
      layer = llf->layer();
    }
    source = layer->source();
  }
  return source;
}

/** Add a vector layer directly without prompting user for location
  The caller must provide information compatible with the provider plugin
  using the vectorLayerPath and baseName. The provider can use these
  parameters in any way necessary to initialize the layer. The baseName
  parameter is used in the Map Legend so it should be formed in a meaningful
  way.
  */
void QgisApp::addVectorLayer(QString vectorLayerPath, QString baseName, QString providerKey)
{
  mMapCanvas->freeze();
  QApplication::setOverrideCursor(Qt::WaitCursor);
  // create the layer
  QgsVectorLayer *layer;
  /* Eliminate the need to instantiate the layer based on provider type.
     The caller is responsible for cobbling together the needed information to
     open the layer
     */
#ifdef QGISDEBUG

  std::cout << "Creating new vector layer using " <<
    vectorLayerPath.toLocal8Bit().data() << " with baseName of " << baseName.toLocal8Bit().data() <<
    " and providerKey of " << providerKey.toLocal8Bit().data() << std::endl;
#endif

  layer = new QgsVectorLayer(vectorLayerPath, baseName, providerKey);
  QObject::connect(layer, SIGNAL(editingStopped(bool)), mMapCanvas, SLOT(removeDigitizingLines(bool)));

  if( layer && layer->isValid() )
  {
    // Register this layer with the layers registry
    QgsMapLayerRegistry::instance()->addMapLayer(layer);
    // init the context menu so it can connect to slots in main app
    // now taken care of in legend layer->initContextMenu(this);

    // give it a random color
    QgsSingleSymRenderer *renderer = new QgsSingleSymRenderer(layer->vectorType());  //add single symbol renderer as default
    layer->setRenderer(renderer);
    // add it to the mapcanvas collection
    // mMapCanvas->addLayer(layer); No longer necessary since adding to registry will add to canvas

    //connect up a request from the raster layer to show in overview map
    // XXX some day will no longer necessary since adding to registry will add to overview layer
    //       QObject::connect(layer,
    //               SIGNAL(showInOverview(QString,bool)),
    //               this,
    //               SLOT(setLayerOverviewStatus(QString,bool)));

    // connect up any keypresses to be passed tot he layer (e.g. so esc can stop rendering)
#ifdef QGISDEBUG
    std::cout << " Connecting up maplayers keyPressed event to the QgisApp keyPress signal" << std::endl;
#endif
    QObject::connect(this,
        SIGNAL(keyPressed(QKeyEvent * )),
        layer,
        SLOT(keyPressed(QKeyEvent* )));


    //add hooks for letting layer know canvas needs to recalc the layer extents
    QObject::connect(layer,
        SIGNAL(recalculateExtents()),
        mMapCanvas,
        SLOT(recalculateExtents()));

    QObject::connect(layer,
        SIGNAL(recalculateExtents()),
        mOverviewCanvas,
        SLOT(recalculateExtents()));


    QgsProject::instance()->dirty(false); // XXX this might be redundant

    statusBar()->message(mMapCanvas->extent().stringRep(2));

  }
  else
  {
    QMessageBox::critical(this,tr("Layer is not valid"),
        tr("The layer is not a valid layer and can not be added to the map"));
  }
  qApp->processEvents();
  mMapCanvas->freeze(false);
  // For Qt4, deprecate direct calling of render().  Let render() be called by the 
  // paint event loop of the map canvas widget.
  // XXX - this doesn't work -- or nobody implemented it so render() is still enabled
  // [gsherman]
  mMapCanvas->render();
  mMapCanvas->update();
  QApplication::restoreOverrideCursor();

} // QgisApp::addVectorLayer



void QgisApp::addMapLayer(QgsMapLayer *theMapLayer)
{
  mMapCanvas->freeze();
  QApplication::setOverrideCursor(Qt::WaitCursor);
  if(theMapLayer->isValid())
  {
    // Register this layer with the layers registry
    QgsMapLayerRegistry::instance()->addMapLayer(theMapLayer);
    // init the context menu so it can connect to slots in main app
    // XXX now taken care of in legend theMapLayer->initContextMenu(this);
    // add it to the mapcanvas collection
    // not necessary since adding to registry adds to canvas mMapCanvas->addLayer(theMapLayer);
    //connect up a request from the raster layer to show in overview map

    // XXX some day not necessary since adding to registry will add to overview
    //     QObject::connect(theMapLayer,
    //                      SIGNAL(showInOverview(QString,bool)),
    //                      this,
    //                      SLOT(setLayerOverviewStatus(QString,bool)));

    statusBar()->message(mMapCanvas->extent().stringRep(2));

  }
  else
  {
    QMessageBox::critical(this,tr("Layer is not valid"),
        tr("The layer is not a valid layer and can not be added to the map"));
  }
  qApp->processEvents();
  mMapCanvas->freeze(false);
  // For Qt4, deprecate direct calling of render().  Let render() be called by the 
  // paint event loop of the map canvas widget.
  // XXX - this doesn't work -- or nobody implemented it so render() is still enabled
  // [gsherman]
  mMapCanvas->render();
  mMapCanvas->update();
  QApplication::restoreOverrideCursor();

}

void QgisApp::setExtent(QgsRect theRect)
{
  mMapCanvas->setExtent(theRect);
}




int QgisApp::saveDirty()
{
  int answer = 0;
  mMapCanvas->freeze(true);     // XXX shouldn't we freeze overview canvas, too?

#ifdef QGISDEBUG

  std::cout << "Layer count is " << mMapCanvas->layerCount() << std::endl;
  std::cout << "Project is ";
  if ( QgsProject::instance()->dirty() )
  {
    std::cout << "dirty" << std::endl;
  }
  else
  {
    std::cout << "not dirty" << std::endl;
  }

  std::cout << "Map canvas is ";
  if (mMapCanvas->isDirty())
  {
    std::cout << "dirty" << std::endl;
  }
  else
  {
    std::cout << "not dirty" << std::endl;
  }
#endif

  if ((QgsProject::instance()->dirty() || (mMapCanvas->isDirty()) && mMapCanvas->layerCount() > 0))
  {
    // flag project as dirty since dirty state of canvas is reset if "dirty"
    // is based on a zoom or pan
    QgsProject::instance()->dirty( true );
    // old code: mProjectIsDirtyFlag = true;

    // prompt user to save
    answer = QMessageBox::information(this, tr("Save?"),
        tr("Do you want to save the current project?"),
        QMessageBox::Yes | QMessageBox::Default,
        QMessageBox::No,
        QMessageBox::Cancel | QMessageBox::Escape);
    if (QMessageBox::Yes == answer )
    {
      fileSave();
    }
  }

  mMapCanvas->freeze(false);

  return answer;

} // QgisApp::saveDirty()


void QgisApp::whatsThis()
{
  QWhatsThis::enterWhatsThisMode();
} // QgisApp::whatsThis()


std::map<QString, int> QgisApp::menuMapByName()
{
  // Must populate the maps with each call since menus might have been
  // added or deleted
  populateMenuMaps();
  // Return the menu items mapped by name (key is name, value is menu id)
  return mMenuMapByName;
}
std::map<int, QString> QgisApp::menuMapById()
{
  // Must populate the maps with each call since menus might have been
  // added or deleted
  populateMenuMaps();
  // Return the menu items mapped by menu id (key is menu id, value is name)
  return mMenuMapById;
}
void QgisApp::populateMenuMaps()
{
  // Populate the two menu maps by iterating through the menu bar
  mMenuMapByName.clear();
  mMenuMapById.clear();
  int idx = 0;
  int menuId;
  // Loop until we get an id of -1, which indicates there are no more
  // items.
  do
  {
    menuId = menuBar()->idAt(idx++);
    std::cout << "Menu id " << menuId << " is " << menuBar()->text(menuId).toLocal8Bit().data() << std::endl;
    mMenuMapByName[menuBar()->text(menuId)] = menuId;
    mMenuMapById[menuId] = menuBar()->text(menuId);
  }
  while(menuId != -1);
}
int QgisApp::addPluginMenu(QString menuText, QMenu *menu)
{
  return mPluginMenu->insertItem(menuText, menu);
}

QMenu* QgisApp::getPluginMenu(QString menuName)
{
  for (unsigned int i = 0; i < mPluginMenu->count(); ++i)
    if (mPluginMenu->text(mPluginMenu->idAt(i)) == menuName)
    {
      QMenuItem* item = mPluginMenu->findItem(mPluginMenu->idAt(i));
      return item->menu();
    }

  // It doesn't exist, so create one
  Q3PopupMenu* menu = new Q3PopupMenu(mPluginMenu);
  mPluginMenu->insertItem(menuName, menu);
  return menu;
}

void QgisApp::removePluginMenuItem(QString name, int menuId)
{
  // TODO: Qt4 will have to do this a different way...
#if QT_VERSION < 0x040000
  for (int i = 0; i < mPluginMenu->count(); ++i)
    if (mPluginMenu->text(mPluginMenu->idAt(i)) == name)
    {
      QMenuItem* item = mPluginMenu->findItem(mPluginMenu->idAt(i));
      Q3PopupMenu* popup = item->popup();
      popup->removeItem(menuId);
      if (popup->count() == 0)
        mPluginMenu->removeItem(mPluginMenu->idAt(i));
      break;
    }
#endif
}

int QgisApp::addPluginToolBarIcon (QAction * qAction)
{
  qAction->addTo(mPluginToolBar);
  return 0;
}
void QgisApp::removePluginToolBarIcon(QAction *qAction)
{
  qAction->removeFrom(mPluginToolBar);
}

void QgisApp::projectionsEnabled(bool theFlag)
{
  QString myIconPath = QgsApplication::themePath();
  if (theFlag)
  {
    QPixmap myProjPixmap;
    myProjPixmap.load(myIconPath+"/mIconProjectionEnabled.png");
    qDebug ("Icon disabled projection icon to: " + myIconPath.toLocal8Bit()+"/mIconProjectionEnabled.png");
    assert(!myProjPixmap.isNull());
    mOnTheFlyProjectionStatusButton->setPixmap(myProjPixmap);
  }
  else
  {
    QPixmap myProjPixmap;
    myProjPixmap.load(myIconPath+"/mIconProjectionDisabled.png");
    qDebug ("Icon disabled projection icon to: " + myIconPath.toLocal8Bit()+"/mIconProjectionDisabled.png");
    assert(!myProjPixmap.isNull());
    mOnTheFlyProjectionStatusButton->setPixmap(myProjPixmap);
  }
}
// slot to update the progress bar in the status bar
void QgisApp::showProgress(int theProgress, int theTotalSteps)
{
#ifdef QGISDEBUG
  //std::cout << "setProgress called with " << theProgress << "/" << theTotalSteps << std::endl;
#endif

  if (theProgress==theTotalSteps)
  {
    mProgressBar->reset();
  }
  else
  {
    /* @todo fix this!
    //only call show if not already hidden to reduce flicker
    if (!mProgressBar->isVisible())
    {
    mProgressBar->show();
    }
    */
    mProgressBar->setMaximum(theTotalSteps);
    mProgressBar->setValue(theProgress);
  }


}

void QgisApp::showExtents(QgsRect theExtents)
{
  // update the statusbar with the current extents.
  statusBar()->message(QString(tr("Extents: ")) + theExtents.stringRep(true));

  if (mOverviewCanvas->extent() != mMapCanvas->fullExtent())
  {
#ifdef QGISDEBUG
    std::cout << "showExtents: " << std::endl << "  full: " << mMapCanvas->fullExtent() << std::endl <<
      "  over: " << mOverviewCanvas->extent() << std::endl;
#endif

    // set the extents of the overview to match the mapcanvas
    mOverviewCanvas->setExtent(mMapCanvas->fullExtent());
    // refresh the overview map
    mOverviewCanvas->clear();
    // For Qt4, deprecate direct calling of render().  Let render() be called by the 
    // paint event loop of the overview canvas widget.
    // XXX - this doesn't work -- or nobody implemented it so render() is still enabled
    // [gsherman]
    mOverviewCanvas->render();
    mOverviewCanvas->update();
  }

  // update panning widget in overview
  mOverviewCanvas->reflectChangedExtent();

} // QgisApp::showExtents


void QgisApp::drawExtentRectangle(QPainter *painter)
{
  //XXX - This code is not used but lets save it for a bit...
  // Draw the current extents rectangle on the overview
  // We don't care about the painter since we want to draw
  // on a copy of the overview pixmap
  /*
     QgsCoordinateTransform *cXf = mOverviewCanvas->getCoordinateTransform();
  // get the upper right and lower left corners of the extent rectangle
  QgsRect theExtents = mMapCanvas->extent();
  QgsPoint ul(theExtents.xMin(), theExtents.yMax());
  QgsPoint lr(theExtents.xMax(), theExtents.yMin());
  // transform the points from map coordinates to device coordinates
  cXf->transform(&ul);
  cXf->transform(&lr);
  // copy the overview pixmap
  QPixmap *overviewPixmap = new QPixmap(*mOverviewCanvas->canvasPixmap());
  QPainter *canvasPainter = new QPainter();
  canvasPainter->begin(overviewPixmap);
  canvasPainter->setPen(QColor(255,0,0));
  canvasPainter->drawRect(ul.xToInt(), lr.yToInt(),
  lr.xToInt() - ul.xToInt(), ul.yToInt() - lr.yToInt());
  canvasPainter->end();
  delete canvasPainter;
  // bitblt the new overview to the overview canvas
  bitBlt(mOverviewCanvas,0,0, overviewPixmap, 0,0);

*/
}

void QgisApp::updateMouseCoordinatePrecision()
{
  // Work out what mouse display precision to use. This only needs to
  // be when the settings change or the zoom level changes. This
  // function needs to be called every time one of the above happens.

  // Get the display precision from the project settings
  bool automatic = QgsProject::instance()->readBoolEntry("PositionPrecision","/Automatic");
  int dp = 0;

  if (automatic)
  {
    // Work out a suitable number of decimal places for the mouse
    // coordinates with the aim of always having enough decimal places
    // to show the difference in position between adjacent pixels.
    // Also avoid taking the log of 0.
    if (getMapCanvas()->mupp() != 0.0)
      dp = static_cast<int> (ceil(-1.0*log10(getMapCanvas()->mupp())));
  }
  else
    dp = QgsProject::instance()->readNumEntry("PositionPrecision","/DecimalPlaces");

  // Keep dp sensible
  if (dp < 0) dp = 0;

  mMousePrecisionDecimalPlaces = dp;
}

void QgisApp::showStatusMessage(QString theMessage)
{
#ifdef QGISDEBUG
  //  std::cout << "QgisApp::showStatusMessage: entered with '" << theMessage << "'." << std::endl;
#endif

  statusBar()->message(theMessage);
}

void QgisApp::projectPropertiesProjections()
{
  // Driver to display the project props dialog and switch to the
  // projections tab
  mShowProjectionTab = true;
  projectProperties();
}

void QgisApp::projectProperties()
{
  /* Display the property sheet for the Project */
  // set wait cursor since construction of the project properties
  // dialog results in the construction of the spatial reference
  // system QMap
  QApplication::setOverrideCursor(Qt::WaitCursor);
  QgsProjectProperties *pp = new QgsProjectProperties(this);
  // if called from the status bar, show the projection tab
  if(mShowProjectionTab)
  {
    pp->showProjectionsTab();
    mShowProjectionTab = false;
  }
  qApp->processEvents();
  // Be told if the mouse display precision may have changed by the user
  // changing things in the project properties dialog box
  connect(pp, SIGNAL(displayPrecisionChanged()), this, 
      SLOT(updateMouseCoordinatePrecision()));
  //listen to changes in on the fly projection state
  connect(pp, SIGNAL(projectionEnabled(bool)), this, 
      SLOT(projectionsEnabled(bool)));
  QApplication::restoreOverrideCursor();
  //pass any refresg signals off to canvases
  connect (pp,SIGNAL(refresh()), mMapCanvas, SLOT(refresh()));
  connect (pp,SIGNAL(mapUnitsChanged()), mMapCanvas, SLOT(mapUnitsChanged()));  
  connect (pp,SIGNAL(refresh()), mOverviewCanvas, SLOT(refresh()));

  bool wasProjected = pp->isProjected();

  // Display the modal dialog box.
  if (pp->exec())
  {
    // If the canvas projection settings changed, we need to recalculate the extents in the
    // new coordinate system
    if(pp->isProjected() != wasProjected)
    {
      mMapCanvas->recalculateExtents();
    }
  }
    int  myRedInt = QgsProject::instance()->readNumEntry("Gui","/CanvasColorRedPart",255);
    int  myGreenInt = QgsProject::instance()->readNumEntry("Gui","/CanvasColorGreenPart",255);
    int  myBlueInt = QgsProject::instance()->readNumEntry("Gui","/CanvasColorBluePart",255);
    QColor myColor = QColor(myRedInt,myGreenInt,myBlueInt);
    mMapCanvas->setCanvasColor(myColor); //this is fil colour before rendering onto canvas
    mMapCanvas->setBackgroundColor(myColor); // this is for the widget itself
  // Set the window title.
  setTitleBarText_( *this );
  // delete the property sheet object
  delete pp;
} // QgisApp::projectProperties


QgsMapLayerRegistry * QgisApp::getLayerRegistry()
{
  return QgsMapLayerRegistry::instance();
}




QgsClipboard * QgisApp::clipboard()
{
  return &mInternalClipboard;
}

void QgisApp::setLayerOverviewStatus(QString theLayerId, bool theVisibilityFlag)
{
  if (theVisibilityFlag)
  {
    mOverviewCanvas->addLayer(QgsMapLayerRegistry::instance()->mapLayer(theLayerId));
#ifdef QGISDEBUG

    std::cout << " Added layer " << theLayerId.toLocal8Bit().data() << " to overview map" << std::endl;
#endif

  }
  else
  {
    mOverviewCanvas->remove
      (theLayerId);
#ifdef QGISDEBUG

    std::cout << " Removed layer " << theLayerId.toLocal8Bit().data() << " from overview map" << std::endl;
#endif

  }

  //check zorder is in sync
  setOverviewZOrder(mMapLegend);

  // notify the project we've made a change
  QgsProject::instance()->dirty(true);
} // QgisApp::setLayerOverviewStatus


void QgisApp::setOverviewZOrder(QgsLegend * lv)
{
#ifdef QGISDEBUG
  std::cout << " Resetting z-order of overview map" << std::endl;
#endif
  //we must clear the overview canvas first to ensure layers are added again
  //in the correect order!
  mOverviewCanvas->clear();
  mOverviewCanvas->freeze(false);
  std::deque<QString> myOverviewLayerVector = lv->layerIDs();
  for(std::deque<QString>::iterator it= myOverviewLayerVector.begin(); it != myOverviewLayerVector.end(); ++it)
  {
    mOverviewCanvas->remove(*it);
  }

  std::deque<QString>::iterator myIterator=myOverviewLayerVector.begin();
  while (myIterator != myOverviewLayerVector.end())
  {
    QgsMapLayer *lyr = QgsMapLayerRegistry::instance()->mapLayer(*myIterator);
    if (lyr->showInOverviewStatus())
    {
      mOverviewCanvas->addLayer(lyr);
    }
    myIterator++;
  }
  //mOverviewCanvas->render();
  //mOverviewCanvas->zoomFullExtent();
  // set the extents of the overview to match the mapcanvas
  mOverviewCanvas->setExtent(mMapCanvas->fullExtent());
  mOverviewCanvas->refresh();

  // notify the project we've made a change
  QgsProject::instance()->dirty(true);
}

//set the zorder of both overview and mapcanvas
void QgisApp::setZOrder (std::list<QString> theZOrder)
{
  mMapCanvas->setZOrder(theZOrder);
  //mOverviewCanvas->setZOrder(theZOrder);

  // notify the project we've made a change
  // QgsProject::instance()->dirty(true);
}

//copy the click coord to clipboard and let the user know its there
void QgisApp::showCapturePointCoordinate(QgsPoint & theQgsPoint)
{
#ifdef QGISDEBUG
  std::cout << "Capture point (clicked on map) at position " << theQgsPoint.stringRep(2).toLocal8Bit().data() << std::endl;
#endif

  QClipboard *myClipboard = QApplication::clipboard();
  //if we are on x11 system put text into selection ready for middle button pasting
  if (myClipboard->supportsSelection())
  {
    myClipboard->setText(theQgsPoint.stringRep(2),QClipboard::Selection);
    QString myMessage = tr("Clipboard contents set to: ");
    statusBar()->message(myMessage + myClipboard->text(QClipboard::Selection));
  }
  else
  {
    //user has an inferior operating system....
    myClipboard->setText(theQgsPoint.stringRep(2),QClipboard::Clipboard );
    QString myMessage = tr("Clipboard contents set to: ");
    statusBar()->message(myMessage + myClipboard->text(QClipboard::Clipboard));
  }
#ifdef QGISDEBUG
  /* Well use this in ver 0.5 when we do digitising! */
  /*
     QgsVectorFileWriter myFileWriter("/tmp/test.shp", wkbPoint);
     if (myFileWriter.initialise())
     {
     myFileWriter.createField("TestInt",OFTInteger,8,0);
     myFileWriter.createField("TestRead",OFTReal,8,3);
     myFileWriter.createField("TestStr",OFTString,255,0);
     myFileWriter.writePoint(&theQgsPoint);
     }
     */
#endif
}



/////////////////////////////////////////////////////////////////
//
//
// Only functions relating to raster layer management in this
// section (look for a similar comment block to this to find
// the end of this section).
//
// Tim Sutton
//
//
/////////////////////////////////////////////////////////////////


/** @todo XXX I'd *really* like to return, ya know, _false_.
*/
//create a raster layer object and delegate to addRasterLayer(QgsRasterLayer *)
void QgisApp::addRasterLayer()
{
  //mMapCanvas->freeze(true);

  QString fileFilters;


  QStringList selectedFiles;
  QString e;//only for parameter correctness
  QString title = tr("Open a GDAL Supported Raster Data Source");
  openFilesRememberingFilter_("lastRasterFileFilter", mRasterFileFilter, selectedFiles,e,
      title);

  if (selectedFiles.isEmpty())
  {
    // no files were selected, so just bail
    return;
  }

  addRasterLayer(selectedFiles);
  mMapCanvas->freeze(false);
  mMapCanvas->refresh();
}// QgisApp::addRasterLayer()

//
// This is the method that does the actual work of adding a raster layer - the others
// here simply create a raster layer object and delegate here. It is the responsibility
// of the calling method to manage things such as the frozen state of the mapcanvas and
// using waitcursors etc. - this method wont and SHOULDNT do it
//
bool QgisApp::addRasterLayer(QgsRasterLayer * theRasterLayer, bool theForceRedrawFlag)
{

  Q_CHECK_PTR( theRasterLayer );

  if ( ! theRasterLayer )
  {
    // XXX insert meaningful whine to the user here; although be
    // XXX mindful that a null layer may mean exhausted memory resources
    return false;
  }

  if (theRasterLayer->isValid())
  {
    // register this layer with the central layers registry
    QgsMapLayerRegistry::instance()->addMapLayer(theRasterLayer);
    // XXX doesn't the mMapCanvas->addLayer() do this?
    // XXX now it does
    //     QObject::connect(theRasterLayer,
    //             SIGNAL(repaintRequested()),
    //             mMapCanvas,
    //             SLOT(refresh()));

    // connect up any request the raster may make to update the app progress
    QObject::connect(theRasterLayer,
        SIGNAL(setProgress(int,int)),
        this,
        SLOT(showProgress(int,int)));
    // connect up any request the raster may make to update the statusbar message
    QObject::connect(theRasterLayer,
        SIGNAL(setStatus(QString)),
        this,
        SLOT(showStatusMessage(QString)));
    // connect up any keypresses to be passed tot he layer (e.g. so esc can stop rendering)
#ifdef QGISDEBUG
    std::cout << " Connecting up maplayers keyPressed event to the QgisApp keyPress signal" << std::endl;
#endif
    QObject::connect(this ,
        SIGNAL(keyPressed(QKeyEvent * )),
        theRasterLayer,
        SLOT(keyPressed(QKeyEvent* )));
    //add hooks for letting layer know canvas needs to recalc the layer extents
    QObject::connect(theRasterLayer,
        SIGNAL(recalculateExtents()),
        mMapCanvas,
        SLOT(recalculateExtents()));

    QObject::connect(theRasterLayer,
        SIGNAL(recalculateExtents()),
        mOverviewCanvas,
        SLOT(recalculateExtents()));
    // init the context menu so it can connect to slots in main app
    // XXX now taken care of in legend theRasterLayer->initContextMenu(this);

    // add it to the mapcanvas collection
    // no longer necessary since adding to registry automatically adds to canvas
    // mMapCanvas->addLayer(theRasterLayer);

    // connect up a request from the raster layer to show in overview map
    // XXX some day will be no longer necessary since adding to registry adds to overview, too
    //     QObject::connect(theRasterLayer,
    //             SIGNAL(showInOverview(QString,bool)),
    //             this,
    //             SLOT(setLayerOverviewStatus(QString,bool)));

  }
  else
  {
    delete theRasterLayer;
    return false;
  }

  if (theForceRedrawFlag)
  {
    qApp->processEvents();
    mMapCanvas->freeze(false);
    // For Qt4, deprecate direct calling of render().  Let render() be called by the 
    // paint event loop of the map canvas widget.
    // XXX - this doesn't work -- or nobody implemented it so render() is still enabled
    // [gsherman]
    mMapCanvas->render();
    mMapCanvas->update();
  }
  return true;

}


//create a raster layer object and delegate to addRasterLayer(QgsRasterLayer *)

bool QgisApp::addRasterLayer(QFileInfo const & rasterFile, bool guiWarning)
{
  // let the user know we're going to possibly be taking a while
  QApplication::setOverrideCursor(Qt::WaitCursor);

  mMapCanvas->freeze(true);
  mOverviewCanvas->freeze(true);

  // XXX ya know QgsRasterLayer can snip out the basename on its own;
  // XXX why do we have to pass it in for it?
  QgsRasterLayer *layer =
    new QgsRasterLayer(rasterFile.filePath(), rasterFile.completeBaseName());

  if (!addRasterLayer(layer))
  {
    mMapCanvas->freeze(false);
    QApplication::restoreOverrideCursor();
    if(guiWarning)
    {
      // don't show the gui warning (probably because we are loading from command line)
      QString msg(rasterFile.completeBaseName()
          + tr(" is not a valid or recognized raster data source"));
      QMessageBox::critical(this, tr("Invalid Data Source"), msg);
    }
    return false;
  }
  else
  {
    statusBar()->message(mMapCanvas->extent().stringRep(2));
    mMapCanvas->freeze(false);
    mOverviewCanvas->freeze(false);
    QApplication::restoreOverrideCursor();
    return true;
  }

} // QgisApp::addRasterLayer



/** Add a raster layer directly without prompting user for location
  The caller must provide information compatible with the provider plugin
  using the rasterLayerPath and baseName. The provider can use these
  parameters in any way necessary to initialize the layer. The baseName
  parameter is used in the Map Legend so it should be formed in a meaningful
  way.

  \note   Copied from the equivalent addVectorLayer function in this file
  TODO    Make it work for rasters specifically.
  */
void QgisApp::addRasterLayer(QString const & rasterLayerPath, 
    QString const & baseName, 
    QString const & providerKey, 
    QStringList const & layers,
    QStringList const & styles,
    QString const & format)
{

#ifdef QGISDEBUG
  std::cout << "QgisApp::addRasterLayer: about to get library for " << providerKey.toLocal8Bit().data() << std::endl;
#endif

  mMapCanvas->freeze();
  QApplication::setOverrideCursor(Qt::WaitCursor);
  // create the layer
  QgsRasterLayer *layer;
  /* Eliminate the need to instantiate the layer based on provider type.
     The caller is responsible for cobbling together the needed information to
     open the layer
     */
#ifdef QGISDEBUG

  std::cout << "QgisApp::addRasterLayer: Creating new raster layer using " <<
    rasterLayerPath.toLocal8Bit().data() << " with baseName of " << baseName.toLocal8Bit().data() <<
    " and layer list of " << layers.join(", ").toLocal8Bit().data() <<
    " and style list of " << styles.join(", ").toLocal8Bit().data() <<
    " and format of " << format.toLocal8Bit().data() <<
    " and providerKey of " << providerKey.toLocal8Bit().data() << std::endl;
#endif

  // TODO: Remove the 0 when the raster layer becomes a full provider gateway.
  layer = new QgsRasterLayer(0, rasterLayerPath, baseName, providerKey, layers, styles, format);

#ifdef QGISDEBUG
  std::cout << "QgisApp::addRasterLayer: Constructed new layer." << std::endl;
#endif

  if( layer && layer->isValid() )
  {
    // Register this layer with the layers registry
    QgsMapLayerRegistry::instance()->addMapLayer(layer);
    // init the context menu so it can connect to slots in main app
    // now taken care of in legend layer->initContextMenu(this);

    // connect up any request the raster may make to update the app progress
    QObject::connect(layer,
        SIGNAL(setProgress(int,int)),
        this,
        SLOT(showProgress(int,int)));
    // connect up any request the raster may make to update the statusbar message
    QObject::connect(layer,
        SIGNAL(setStatus(QString)),
        this,
        SLOT(showStatusMessage(QString)));

    // connect up any keypresses to be passed tot he layer (e.g. so esc can stop rendering)
#ifdef QGISDEBUG
    std::cout << " Connecting up maplayers keyPressed event to the QgisApp keyPress signal" << std::endl;
#endif
    QObject::connect(this,
        SIGNAL(keyPressed(QKeyEvent * )),
        layer,
        SLOT(keyPressed(QKeyEvent* )));


    //add hooks for letting layer know canvas needs to recalc the layer extents
    QObject::connect(layer,
        SIGNAL(recalculateExtents()),
        mMapCanvas,
        SLOT(recalculateExtents()));

    QObject::connect(layer,
        SIGNAL(recalculateExtents()),
        mOverviewCanvas,
        SLOT(recalculateExtents()));


    QgsProject::instance()->dirty(false); // XXX this might be redundant

    statusBar()->message(mMapCanvas->extent().stringRep(2));

  }
  else
  {
    QMessageBox::critical(this,"Layer is not valid",
        "The layer is not a valid layer and can not be added to the map");
  }

  qApp->processEvents();
  mMapCanvas->freeze(false);
  // For Qt4, deprecate direct calling of render().  Let render() be called by the 
  // paint event loop of the map canvas widget.
  // XXX - this doesn't work -- or nobody implemented it so render() is still enabled
  // [gsherman]
  mMapCanvas->render();
  mMapCanvas->update();
  QApplication::restoreOverrideCursor();

} // QgisApp::addRasterLayer



//create a raster layer object and delegate to addRasterLayer(QgsRasterLayer *)
bool QgisApp::addRasterLayer(QStringList const &theFileNameQStringList, bool guiWarning)
{
  if (theFileNameQStringList.empty())
  {
    // no files selected so bail out, but
    // allow mMapCanvas to handle events
    // first
    mMapCanvas->freeze(false);
    mOverviewCanvas->freeze(false);
    return false;
  }

  mMapCanvas->freeze(true);
  mOverviewCanvas->freeze(true);

  QApplication::setOverrideCursor(Qt::WaitCursor);
  // this is messy since some files in the list may be rasters and others may
  // be ogr layers. We'll set returnValue to false if one or more layers fail
  // to load.
  bool returnValue = true;
  for (QStringList::ConstIterator myIterator = theFileNameQStringList.begin();
      myIterator != theFileNameQStringList.end();
      ++myIterator)
  {
    if (QgsRasterLayer::isValidRasterFileName(*myIterator))
    {
      QFileInfo myFileInfo(*myIterator);
      // get the directory the .adf file was in
      QString myDirNameQString = myFileInfo.dirPath();
      QString myBaseNameQString = myFileInfo.completeBaseName();
      //only allow one copy of a ai grid file to be loaded at a
      //time to prevent the user selecting all adfs in 1 dir which
      //actually represent 1 coverage,

      // create the layer
      QgsRasterLayer *layer = new QgsRasterLayer(*myIterator, myBaseNameQString);

      // set initial visibility based on user preference
      layer->setVisible(mAddedLayersHidden);

      addRasterLayer(layer);
      layer->refreshLegend();

      //only allow one copy of a ai grid file to be loaded at a
      //time to prevent the user selecting all adfs in 1 dir which
      //actually represent 1 coverate,

      if (myBaseNameQString.lower().endsWith(".adf"))
      {
        break;
      }
    }
    else
    {
      // Issue message box warning unless we are loading from cmd line since
      // non-rasters are passed to this function first and then sucessfully
      // loaded afterwards (see main.cpp)

      if(guiWarning)
      {
        QString msg(*myIterator + tr(" is not a supported raster data source"));
        QMessageBox::critical(this, tr("Unsupported Data Source"), msg);
      }
      returnValue = false;
    }
  }

  statusBar()->message(mMapCanvas->extent().stringRep(2));
  mMapCanvas->freeze(false);
  mOverviewCanvas->freeze(false);
  QApplication::restoreOverrideCursor();

  return returnValue;

}// QgisApp::addRasterLayer()




///////////////////////////////////////////////////////////////////
//
//
//
//
//    RASTER ONLY RELATED FUNCTIONS BLOCK ENDS....
//
//
//
//
///////////////////////////////////////////////////////////////////


void QgisApp::keyPressEvent ( QKeyEvent * e )
{
  // The following statment causes a crash on WIN32 and should be 
  // enclosed in an #ifdef QGISDEBUG if its really necessary. Its
  // commented out for now. [gsherman]
  //    std::cout << e->text().toLocal8Bit().data() << " (keypress recevied)" << std::endl;
  emit keyPressed (e);
  e->ignore();

}
// Debug hook - used to output diagnostic messages when evoked (usually from the menu)
/* Temporarily disabled...
   void QgisApp::debugHook()
   {
   std::cout << "Hello from debug hook" << std::endl; 
// show the map canvas extent
std::cout << mMapCanvas->extent() << std::endl; 
}
*/
void QgisApp::customProjection()
{
  // Create an instance of the Custom Projection Designer modeless dialog.
  // Autodelete the dialog when closing since a pointer is not retained.
  QgsCustomProjectionDialog * myDialog = new QgsCustomProjectionDialog(this,
      Qt::WDestructiveClose);
  myDialog->show();
}
void QgisApp::showBookmarks()
{
  // Create or show the single instance of the Bookmarks modeless dialog.
  // Closing a QWidget only hides it so it can be shown again later.
  static QgsBookmarks *bookmarks = NULL;
  if (bookmarks == NULL)
  {
    bookmarks = new QgsBookmarks(this);
  }
  bookmarks->show();
  bookmarks->raise();
  bookmarks->setActiveWindow();
}

void QgisApp::newBookmark()
{
  // Get the name for the bookmark. Everything else we fetch from
  // the mapcanvas

  bool ok;
  QString bookmarkName = QInputDialog::getText(tr("New Bookmark"), 
      tr("Enter a name for the new bookmark:"), QLineEdit::Normal,
      QString::null, &ok, this);
  if( ok && !bookmarkName.isEmpty())
  {
    // Make sure the database exists
    if(QgsBookmarks::createDatabase())
    {
      // create the bookmark
      QgsBookmarkItem *bmi = new QgsBookmarkItem(bookmarkName, 
          QgsProject::instance()->title(), mMapCanvas->extent(), -1,
          QDir::homeDirPath () + "/.qgis/qgis.db");
      bmi->store();
      delete bmi;
      // emit a signal to indicate that the bookmark was added
      emit bookmarkAdded();
    }
    else
    {
      QMessageBox::warning(this,tr("Error"), tr("Unable to create the bookmark. Your user database may be missing or corrupted"));
    }
  }
}      
