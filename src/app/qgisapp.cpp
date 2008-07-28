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
#include <QAction>
#include <QApplication>
#include <QBitmap>
#include <QCheckBox>
#include <QClipboard>
#include <QColor>
#include <QCursor>
#include <QDesktopServices>
#include <QDesktopWidget>
#include <QDialog>
#include <QDir>
#include <QDockWidget>
#include <QEvent>
#include <QFile>
#include <QFileInfo>
#include <QInputDialog>
#include <QKeyEvent>
#include <QLabel>
#include <QLibrary>
#include <QMenu>
#include <QMenuBar>
#include <QMenuItem>
#include <QMessageBox>
#include <QNetworkProxy>
#include <QPainter>
#include <QPictureIO>
#include <QPixmap>
#include <QPoint>
#include <QPrinter>
#include <QProcess>
#include <QProgressBar>
#include <QRegExp>
#include <QRegExpValidator>
#include <QSettings>
#include <QSplashScreen>
#include <QStatusBar>
#include <QStringList>
#include <QTcpSocket>
#include <QTextStream>
#include <QTimer>
#include <QToolButton>
#include <QVBoxLayout>
#include <QWhatsThis>
#include <QtGlobal>
//
// Mac OS X Includes
// Must include before GEOS 3 due to unqualified use of 'Point'
//
#ifdef Q_OS_MACX
#include <ApplicationServices/ApplicationServices.h>
#endif

//
// QGIS Specific Includes
//
#include "../../images/themes/default/qgis.xpm"
#include "qgisapp.h"
#include "qgisappinterface.h"
#include "qgis.h"
#include "qgisplugin.h"
#include "qgsabout.h"
#include "qgsapplication.h"
#include "qgsbookmarkitem.h"
#include "qgsbookmarks.h"
#include "qgsclipboard.h"
#include "qgscomposer.h"
#include "qgscoordinatetransform.h"
#include "qgscursors.h"
#include "qgscustomprojectiondialog.h"
#include "qgsencodingfiledialog.h"
#include "qgsexception.h"
#include "qgsfeature.h"
#include "qgsgeomtypedialog.h"
#include "qgshelpviewer.h"
#include "qgslayerprojectionselector.h"
#include "qgslegend.h"
#include "qgslegendlayerfile.h"
#include "qgslegendlayer.h"
#include "qgslogger.h"
#include "qgsmapcanvas.h"
#include "qgsmaplayer.h"
#include "qgsmaplayerinterface.h"
#include "qgsmaplayerregistry.h"
#include "qgsmapoverviewcanvas.h"
#include "qgsmaprender.h"
#include "qgsmaptip.h"
#include "qgsmessageviewer.h"
#include "qgsoptions.h"
#include "qgspastetransformations.h"
#include "qgspluginitem.h"
#include "qgspluginmanager.h"
#include "qgspluginregistry.h"
#include "qgspoint.h"
#include "qgsproject.h"
#include "qgsprojectproperties.h"
#include "qgsproviderregistry.h"
#include "qgsrasterlayer.h"
#include "qgsrasterlayerproperties.h"
#include "qgsrect.h"
#include "qgsrenderer.h"
#include "qgsserversourceselect.h"
#include "qgsvectordataprovider.h"
#include "qgsvectorlayer.h"

//
// Gdal/Ogr includes
//
#include <ogr_api.h>

//
// Other includes
//
#include <algorithm>
#include <cassert>
#include <cmath>
#include <functional>
#include <iomanip>
#include <list>
#include <memory>
#include <vector>

//
// Map tools
//
#include "qgsmaptooladdfeature.h"
#include "qgsmaptooladdisland.h"
#include "qgsmaptooladdring.h"
#include "qgsmaptooladdvertex.h"
#include "qgsmaptooldeletevertex.h"
#include "qgsmaptoolidentify.h"
#include "qgsmaptoolmovefeature.h"
#include "qgsmaptoolmovevertex.h"
#include "qgsmaptoolpan.h"
#include "qgsmaptoolselect.h"
#include "qgsmaptoolsplitfeatures.h"
#include "qgsmaptoolvertexedit.h"
#include "qgsmaptoolzoom.h"
#include "qgsmeasuretool.h"

//
// Conditional Includes
//
#ifdef HAVE_POSTGRESQL
#include "qgsdbsourceselect.h"
#endif

#include "qgspythondialog.h"
#include "qgspythonutils.h"

#ifndef WIN32
#include <dlfcn.h>
#endif

#ifdef WIN32
#include <windows.h>
#endif

using namespace std;
class QTreeWidgetItem;

/* typedefs for plugins */
typedef QgsMapLayerInterface *create_it();
typedef QgisPlugin *create_ui(QgisInterface * qI);
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
  caption += QString("%1 ").arg(QGis::qgisVersion) + " ";

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

  qgisApp.setWindowTitle( caption );
} // setTitleBarText_( QWidget * qgisApp )

/**
 Creator function for output viewer
*/
static QgsMessageOutput* messageOutputViewer_()
{
  return new QgsMessageViewer();
}


/**
 * This function contains forced validation of SRS used in QGIS.
 * There are 3 options depending on the settings:
 * - ask for SRS using projection selecter
 * - use project's SRS
 * - use predefined global SRS
 */
static void customSrsValidation_(QgsSpatialRefSys* srs)
{
  QString proj4String;
  QSettings mySettings;
  QString myDefaultProjectionOption =
      mySettings.value("/Projections/defaultBehaviour").toString();
  if (myDefaultProjectionOption=="prompt")
  {
    //@note this class is not a descendent of QWidget so we cant pass
    //it in the ctor of the layer projection selector

    QgsLayerProjectionSelector * mySelector = new QgsLayerProjectionSelector();
    proj4String = QgsProject::instance()->readEntry("SpatialRefSys","//ProjectSRSProj4String",GEOPROJ4);
    QgsSpatialRefSys defaultSRS;
    if(defaultSRS.createFromProj4(proj4String))
      {
	mySelector->setSelectedSRSID(defaultSRS.srsid());
      }

    if(mySelector->exec())
    {
      QgsDebugMsg("Layer srs set from dialog: " + QString::number(mySelector->getCurrentSRSID()));
      srs->createFromProj4(mySelector->getCurrentProj4String());
      srs->debugPrint();
    }
    else
    {
      QApplication::restoreOverrideCursor();
    }
    delete mySelector;
  }
  else if (myDefaultProjectionOption=="useProject")
  {
    // XXX TODO: Change project to store selected CS as 'projectSRS' not 'selectedWKT'
    proj4String = QgsProject::instance()->readEntry("SpatialRefSys","//ProjectSRSProj4String",GEOPROJ4);
    QgsDebugMsg("Layer srs set from project: " + proj4String);
    srs->createFromProj4(proj4String);  
    srs->debugPrint();
  }
  else ///Projections/defaultBehaviour==useGlobal
  {
    srs->createFromProj4(mySettings.value("/Projections/defaultProjectionString",GEOPROJ4).toString());
    srs->debugPrint();
  }

}


// constructor starts here
  QgisApp::QgisApp(QSplashScreen *splash, QWidget * parent, Qt::WFlags fl)
: QMainWindow(parent,fl),
  mSplash(splash),
  mPythonUtils(NULL), mPythonConsole(NULL)
{
//  setupUi(this);
  resize(640, 480);

  mSplash->showMessage(tr("Checking database"), Qt::AlignHCenter | Qt::AlignBottom);
  qApp->processEvents();
  // Do this early on before anyone else opens it and prevents us copying it
  createDB();    


  mSplash->showMessage(tr("Reading settings"), Qt::AlignHCenter | Qt::AlignBottom);
  qApp->processEvents();

  mSplash->showMessage(tr("Setting up the GUI"), Qt::AlignHCenter | Qt::AlignBottom);
  qApp->processEvents();
  createActions();
  createActionGroups();
  createMenus();
  createToolBars();
  createStatusBar();
  updateRecentProjectPaths();
  createCanvas();
  createLegend();
  createOverview();
  createMapTips();
  setupProxy();
  readSettings();

  mComposer = new QgsComposer(this); // Map composer
  mInternalClipboard = new QgsClipboard; // create clipboard
  mQgisInterface = new QgisAppInterface(this); // create the interfce

  // set application's icon
  setWindowIcon(QPixmap(qgis_xpm));

  // set application's caption
  QString caption = tr("Quantum GIS - ");
  caption += QString("%1 ('%2')").arg(QGis::qgisVersion).arg(QGis::qgisReleaseName);
  setWindowTitle(caption);

  // set QGIS specific srs validation
  QgsSpatialRefSys::setCustomSrsValidation(customSrsValidation_);
  // set graphical message output
  QgsMessageOutput::setMessageOutputCreator(messageOutputViewer_);
  
  fileNew(); // prepare empty project
  qApp->processEvents();

  // load providers
  mSplash->showMessage(tr("Checking provider plugins"), Qt::AlignHCenter | Qt::AlignBottom);
  qApp->processEvents();
  QgsApplication::initQgis();
  
  mSplash->showMessage(tr("Starting Python"), Qt::AlignHCenter | Qt::AlignBottom);
  qApp->processEvents();
  // try to load python support
  QString pythonlibName("qgispython");
#ifdef Q_WS_MAC
  pythonlibName.prepend(QgsApplication::prefixPath() + "/lib/");
#endif
  QLibrary pythonlib(pythonlibName);
  // It's necessary to set these two load hints, otherwise Python library won't work correctly
  // see http://lists.kde.org/?l=pykde&m=117190116820758&w=2
  pythonlib.setLoadHints(QLibrary::ResolveAllSymbolsHint | QLibrary::ExportExternalSymbolsHint);
  if (pythonlib.load())
  {
    QgsDebugMsg("Python support library loaded successfully.");
    typedef QgsPythonUtils* (*inst)();
    inst pythonlib_inst = (inst) pythonlib.resolve("instance");
    if (pythonlib_inst)
    {
      QgsDebugMsg("Python support library's instance() symbol resolved.");
      mPythonUtils = pythonlib_inst();
      mPythonUtils->initPython(mQgisInterface);
    }
    else
    {
      QgsDebugMsg("Couldn't resolve python support library's instance() symbol.");
    }
  }
  else
  {
    QgsDebugMsg("Couldn't load Python support library.");
  }
  
  if (mPythonUtils && mPythonUtils->isEnabled())
  {
    mActionShowPythonDialog = new QAction(tr("Python console"), this);
    connect(mActionShowPythonDialog, SIGNAL(triggered()), this, SLOT(showPythonDialog()));

    mPluginMenu->addAction(mActionShowPythonDialog);
    QgsDebugMsg("Python support ENABLED :-)");
  }

  // Create the plugin registry and load plugins
  // load any plugins that were running in the last session
  mSplash->showMessage(tr("Restoring loaded plugins"), Qt::AlignHCenter | Qt::AlignBottom);
  qApp->processEvents();
  restoreSessionPlugins(QgsApplication::pluginPath());

  mSplash->showMessage(tr("Initializing file filters"), Qt::AlignHCenter | Qt::AlignBottom);
  qApp->processEvents();
  // now build vector file filter
  buildSupportedVectorFileFilter_( mVectorFileFilter );

  // now build raster file filter
  QgsRasterLayer::buildSupportedRasterFileFilter( mRasterFileFilter );
/*  
  // Set the background colour for toolbox and overview as they default to 
  // white instead of the window color
  QPalette myPalette = toolBox->palette();
  myPalette.setColor(QPalette::Button, myPalette.window().color());
  toolBox->setPalette(myPalette);
  //do the same for legend control
  myPalette = toolBox->palette();
  myPalette.setColor(QPalette::Button, myPalette.window().color());
  mMapLegend->setPalette(myPalette);
  //and for overview control this is done in createOverView method
*/  
  // Do this last in the ctor to ensure that all members are instantiated properly
  setupConnections();
  //
  // Please make sure this is the last thing the ctor does so that we can ensure teh 
  // widgets are all initialised before trying to restore their state.
  //
  mSplash->showMessage(tr("Restoring window state"), Qt::AlignHCenter | Qt::AlignBottom);
  qApp->processEvents();
  restoreWindowState();
  
  mSplash->showMessage(tr("QGIS Ready!"), Qt::AlignHCenter | Qt::AlignBottom);

  mMapTipsVisible = false;

  // setup drag drop 
  setAcceptDrops(true);

  mFullScreenMode = false;
  show();
  qApp->processEvents();
} // QgisApp ctor



QgisApp::~QgisApp()
{
  delete mInternalClipboard;
  delete mQgisInterface;
  
  delete mMapTools.mZoomIn;
  delete mMapTools.mZoomOut;
  delete mMapTools.mPan;
  delete mMapTools.mIdentify;
  delete mMapTools.mMeasureDist;
  delete mMapTools.mMeasureArea;
  delete mMapTools.mCapturePoint;
  delete mMapTools.mCaptureLine;
  delete mMapTools.mCapturePolygon;
  delete mMapTools.mMoveFeature;
  delete mMapTools.mSplitFeatures;
  delete mMapTools.mSelect;
  delete mMapTools.mVertexAdd;
  delete mMapTools.mVertexMove;
  delete mMapTools.mVertexDelete;
  delete mMapTools.mAddRing;
  delete mMapTools.mAddIsland;

  delete mPythonConsole;
  delete mPythonUtils;
  
  // delete map layer registry and provider registry
  QgsApplication::exitQgis();
}

void QgisApp::dragEnterEvent(QDragEnterEvent *event)
{
  if (event->mimeData()->hasUrls())
  {
    event->acceptProposedAction();
  }
}

void QgisApp::dropEvent(QDropEvent *event)
{
  // get the file list
  QList<QUrl>::iterator i;
  QList<QUrl>urls = event->mimeData()->urls();
  for (i = urls.begin(); i != urls.end(); i++)
  {
    QUrl mUrl = *i;
    // seems that some drag and drop operations include an empty url
    // so we test for length to make sure we have something
    if( mUrl.path().length() > 0)
    {
      // check to see if we are opening a project file
      QFileInfo fi(mUrl.path());
      if( fi.completeSuffix() == "qgs" )
      {
        QgsDebugMsg("Opening project " + mUrl.path());
      }
      else
      {
        QgsDebugMsg("Adding " + mUrl.path() + " to the map canvas");
        openLayer(mUrl.path());
      }
    }
  }
  event->acceptProposedAction();
}


// restore any application settings stored in QSettings
void QgisApp::readSettings()
{
  QSettings settings;
  // get the users theme preference from the settings
  setTheme(settings.value("/Themes","default").toString());

  // Add the recently accessed project file paths to the File menu
  mRecentProjectPaths = settings.value("/UI/recentProjectsList").toStringList();
/*
  // Set the behaviour when the map splitters are resized
  bool splitterRedraw = settings.value("/qgis/splitterRedraw", true).toBool();
  canvasLegendSplit->setOpaqueResize(splitterRedraw);
  legendOverviewSplit->setOpaqueResize(splitterRedraw);
*/
}


//////////////////////////////////////////////////////////////////////
//            Set Up the gui toolbars, menus, statusbar etc 
//////////////////////////////////////////////////////////////////////            

void QgisApp::createActions()
{
  //
  // File Menu Related Items
  //
  mActionFileNew= new QAction(getThemeIcon("/mActionFileNew.png"), tr("&New Project"), this);
  mActionFileNew->setShortcut(tr("Ctrl+N","New Project"));
  mActionFileNew->setStatusTip(tr("New Project"));
  connect(mActionFileNew, SIGNAL(triggered()), this, SLOT(fileNew()));
  //
  mActionFileOpen= new QAction(getThemeIcon("/mActionFileOpen.png"), tr("&Open Project..."), this);
  mActionFileOpen->setShortcut(tr("Ctrl+O","Open a Project"));
  mActionFileOpen->setStatusTip(tr("Open a Project"));
  connect(mActionFileOpen, SIGNAL(triggered()), this, SLOT(fileOpen()));
  //
  mActionFileSave= new QAction(getThemeIcon("/mActionFileSave.png"), tr("&Save Project"), this);
  mActionFileSave->setShortcut(tr("Ctrl+S","Save Project"));
  mActionFileSave->setStatusTip(tr("Save Project"));
  connect(mActionFileSave, SIGNAL(triggered()), this, SLOT(fileSave()));
  //
  mActionFileSaveAs= new QAction(getThemeIcon("/mActionFileSaveAs.png"), tr("Save Project &As..."), this);
  mActionFileSaveAs->setShortcut(tr("Ctrl+A","Save Project under a new name"));
  mActionFileSaveAs->setStatusTip(tr("Save Project under a new name"));
  connect(mActionFileSaveAs, SIGNAL(triggered()), this, SLOT(fileSaveAs()));
  //
  mActionFilePrint= new QAction(getThemeIcon("/mActionFilePrint.png"), tr("&Print..."), this);
  mActionFilePrint->setShortcut(tr("Ctrl+P","Print"));
  mActionFilePrint->setStatusTip(tr("Print"));
  connect(mActionFilePrint, SIGNAL(triggered()), this, SLOT(filePrint()));
  //
  mActionSaveMapAsImage= new QAction(getThemeIcon("/mActionSaveMapAsImage.png"), tr("Save as Image..."), this);
  mActionSaveMapAsImage->setShortcut(tr("Ctrl+I","Save map as image"));
  mActionSaveMapAsImage->setStatusTip(tr("Save map as image"));
  connect(mActionSaveMapAsImage, SIGNAL(triggered()), this, SLOT(saveMapAsImage()));
  //
 // TODO: Remove the mActionExportMapServer related code once the mapserver export plugin
 // is complete and tested.
 /* mActionExportMapServer= new QAction(getThemeIcon("/mActionExportMapServer.png"), tr("Export to MapServer Map..."), this);
  mActionExportMapServer->setShortcut(tr("M","Export as MapServer .map file"));
  mActionExportMapServer->setStatusTip(tr("Export as MapServer .map file"));
  connect(mActionExportMapServer, SIGNAL(triggered()), this, SLOT(exportMapServer()));
  */
  //
  mActionFileExit= new QAction(getThemeIcon("/mActionFileExit.png"), tr("Exit"), this);
  mActionFileExit->setShortcut(tr("Ctrl+Q","Exit QGIS"));
  mActionFileExit->setStatusTip(tr("Exit QGIS"));
  connect(mActionFileExit, SIGNAL(triggered()), this, SLOT(fileExit()));
  //
  // Layer Menu Related Items
  //
  mActionAddOgrLayer= new QAction(getThemeIcon("/mActionAddOgrLayer.png"), tr("Add a Vector Layer..."), this);
  mActionAddOgrLayer->setShortcut(tr("V","Add a Vector Layer"));
  mActionAddOgrLayer->setStatusTip(tr("Add a Vector Layer"));
  connect(mActionAddOgrLayer, SIGNAL(triggered()), this, SLOT(addVectorLayer()));
  //
  mActionAddRasterLayer= new QAction(getThemeIcon("/mActionAddRasterLayer.png"), tr("Add a Raster Layer..."), this);
  mActionAddRasterLayer->setShortcut(tr("R","Add a Raster Layer"));
  mActionAddRasterLayer->setStatusTip(tr("Add a Raster Layer"));
  connect(mActionAddRasterLayer, SIGNAL(triggered()), this, SLOT(addRasterLayer()));
  //
  mActionAddLayer= new QAction(getThemeIcon("/mActionAddLayer.png"), tr("Add a PostGIS Layer..."), this);
  mActionAddLayer->setShortcut(tr("D","Add a PostGIS Layer"));
  mActionAddLayer->setStatusTip(tr("Add a PostGIS Layer"));
//#ifdef HAVE_POSTGRESQL
//  std::cout << "HAVE_POSTGRESQL is defined" << std::endl; 
//  assert(0);
//#else
//  std::cout << "HAVE_POSTGRESQL not defined" << std::endl; 
//  assert(0);
//#endif
  connect(mActionAddLayer, SIGNAL(triggered()), this, SLOT(addDatabaseLayer()));
  //
  mActionNewVectorLayer= new QAction(getThemeIcon("/mActionNewVectorLayer.png"), tr("New Vector Layer..."), this);
  mActionNewVectorLayer->setShortcut(tr("N","Create a New Vector Layer"));
  mActionNewVectorLayer->setStatusTip(tr("Create a New Vector Layer"));
  connect(mActionNewVectorLayer, SIGNAL(triggered()), this, SLOT(newVectorLayer()));
  //
  mActionRemoveLayer= new QAction(getThemeIcon("/mActionRemoveLayer.png"), tr("Remove Layer"), this);
  mActionRemoveLayer->setShortcut(tr("Ctrl+D","Remove a Layer"));
  mActionRemoveLayer->setStatusTip(tr("Remove a Layer"));
  connect(mActionRemoveLayer, SIGNAL(triggered()), this, SLOT(removeLayer()));
  mActionRemoveLayer->setEnabled(false);
  //
  mActionAddAllToOverview= new QAction(getThemeIcon("/mActionAddAllToOverview.png"), tr("Add All To Overview"), this);
  mActionAddAllToOverview->setShortcut(tr("+","Show all layers in the overview map"));
  mActionAddAllToOverview->setStatusTip(tr("Show all layers in the overview map"));
  connect(mActionAddAllToOverview, SIGNAL(triggered()), this, SLOT(addAllToOverview()));
  //
  mActionRemoveAllFromOverview= new QAction(getThemeIcon("/mActionRemoveAllFromOverview.png"), tr("Remove All From Overview"), this);
  mActionRemoveAllFromOverview->setShortcut(tr("-","Remove all layers from overview map"));
  mActionRemoveAllFromOverview->setStatusTip(tr("Remove all layers from overview map"));
  connect(mActionRemoveAllFromOverview, SIGNAL(triggered()), this, SLOT(removeAllFromOverview()));
  //
  mActionToggleFullScreen = new QAction(getThemeIcon("/mActionToggleFullScreen.png"), tr("Toggle full screen mode"), this);
  mActionToggleFullScreen->setShortcut(tr("Ctrl-F","Toggle fullscreen mode"));
  mActionToggleFullScreen->setStatusTip(tr("Toggle fullscreen mode"));
  connect(mActionToggleFullScreen, SIGNAL(triggered()), this, SLOT(toggleFullScreen()));

  //
  mActionShowAllLayers= new QAction(getThemeIcon("/mActionShowAllLayers.png"), tr("Show All Layers"), this);
  mActionShowAllLayers->setShortcut(tr("S","Show all layers"));
  mActionShowAllLayers->setStatusTip(tr("Show all layers"));
  connect(mActionShowAllLayers, SIGNAL(triggered()), this, SLOT(showAllLayers()));
  //
  mActionHideAllLayers= new QAction(getThemeIcon("/mActionHideAllLayers.png"), tr("Hide All Layers"), this);
  mActionHideAllLayers->setShortcut(tr("H","Hide all layers"));
  mActionHideAllLayers->setStatusTip(tr("Hide all layers"));
  connect(mActionHideAllLayers, SIGNAL(triggered()), this, SLOT(hideAllLayers()));
  //
  // Settings Menu Related Items
  //
  mActionProjectProperties= new QAction(getThemeIcon("/mActionProjectProperties.png"), tr("Project Properties..."), this);
  mActionProjectProperties->setShortcut(tr("P","Set project properties"));
  mActionProjectProperties->setStatusTip(tr("Set project properties"));
  connect(mActionProjectProperties, SIGNAL(triggered()), this, SLOT(projectProperties()));
  //
  mActionOptions= new QAction(getThemeIcon("/mActionOptions.png"), tr("Options..."), this);
  // mActionOptions->setShortcut(tr("Alt+O","Change various QGIS options"));
  mActionOptions->setStatusTip(tr("Change various QGIS options"));
  connect(mActionOptions, SIGNAL(triggered()), this, SLOT(options()));
  //
  mActionCustomProjection= new QAction(getThemeIcon("/mActionCustomProjection.png"), tr("Custom Projection..."), this);
  // mActionCustomProjection->setShortcut(tr("Alt+I","Manage custom projections"));
  mActionCustomProjection->setStatusTip(tr("Manage custom projections"));
  connect(mActionCustomProjection, SIGNAL(triggered()), this, SLOT(customProjection()));
  //
  // Help Menu Related items
  //
  mActionHelpContents= new QAction(getThemeIcon("/mActionHelpContents.png"), tr("Help Contents"), this);
#ifdef Q_WS_MAC
  mActionHelpContents->setShortcut(tr("Ctrl+?","Help Documentation (Mac)"));
#else
  mActionHelpContents->setShortcut(tr("F1","Help Documentation"));
#endif
  mActionHelpContents->setStatusTip(tr("Help Documentation"));
  connect(mActionHelpContents, SIGNAL(triggered()), this, SLOT(helpContents()));
  //
  mActionQgisHomePage= new QAction(getThemeIcon("/mActionQgisHomePage.png"), tr("Qgis Home Page"), this);
#ifndef Q_WS_MAC
  mActionQgisHomePage->setShortcut(tr("Ctrl+H","QGIS Home Page"));
#endif
  mActionQgisHomePage->setStatusTip(tr("QGIS Home Page"));
  connect(mActionQgisHomePage, SIGNAL(triggered()), this, SLOT(helpQgisHomePage()));
  //
  mActionHelpAbout= new QAction(getThemeIcon("/mActionHelpAbout.png"), tr("About"), this);
  mActionHelpAbout->setStatusTip(tr("About QGIS"));
  connect(mActionHelpAbout, SIGNAL(triggered()), this, SLOT(about()));
  //
  mActionCheckQgisVersion= new QAction(getThemeIcon("/mActionCheckQgisVersion.png"), tr("Check Qgis Version"), this);
  mActionCheckQgisVersion->setStatusTip(tr("Check if your QGIS version is up to date (requires internet access)"));
  connect(mActionCheckQgisVersion, SIGNAL(triggered()), this, SLOT(checkQgisVersion()));
  // 
  // View Menu Items
  //
  mActionDraw= new QAction(getThemeIcon("/mActionDraw.png"), tr("Refresh"), this);
  mActionDraw->setShortcut(tr("Ctrl+R","Refresh Map"));
  mActionDraw->setStatusTip(tr("Refresh Map"));
  connect(mActionDraw, SIGNAL(triggered()), this, SLOT(refreshMapCanvas()));
  //
  mActionZoomIn= new QAction(getThemeIcon("/mActionZoomIn.png"), tr("Zoom In"), this);
  mActionZoomIn->setShortcut(tr("Ctrl++","Zoom In"));
  mActionZoomIn->setStatusTip(tr("Zoom In"));
  connect(mActionZoomIn, SIGNAL(triggered()), this, SLOT(zoomIn()));
  //
  mActionZoomOut= new QAction(getThemeIcon("/mActionZoomOut.png"), tr("Zoom Out"), this);
  mActionZoomOut->setShortcut(tr("Ctrl+-","Zoom Out"));
  mActionZoomOut->setStatusTip(tr("Zoom Out"));
  connect(mActionZoomOut, SIGNAL(triggered()), this, SLOT(zoomOut()));
  //
  mActionZoomFullExtent= new QAction(getThemeIcon("/mActionZoomFullExtent.png"), tr("Zoom Full"), this);
  mActionZoomFullExtent->setShortcut(tr("F","Zoom to Full Extents"));
  mActionZoomFullExtent->setStatusTip(tr("Zoom to Full Extents"));
  connect(mActionZoomFullExtent, SIGNAL(triggered()), this, SLOT(zoomFull()));
  //
  mActionZoomToSelected= new QAction(getThemeIcon("/mActionZoomToSelected.png"), tr("Zoom To Selection"), this);
  mActionZoomToSelected->setShortcut(tr("Ctrl+F","Zoom to selection"));
  mActionZoomToSelected->setStatusTip(tr("Zoom to selection"));
  connect(mActionZoomToSelected, SIGNAL(triggered()), this, SLOT(zoomToSelected()));
  //
  mActionPan= new QAction(getThemeIcon("/mActionPan.png"), tr("Pan Map"), this);
  mActionPan->setStatusTip(tr("Pan the map"));
  connect(mActionPan, SIGNAL(triggered()), this, SLOT(pan()));
  //
  mActionZoomLast= new QAction(getThemeIcon("/mActionZoomLast.png"), tr("Zoom Last"), this);
  //mActionZoomLast->setShortcut(tr("Ctrl+O","Zoom to Last Extent"));
  mActionZoomLast->setStatusTip(tr("Zoom to Last Extent"));
  connect(mActionZoomLast, SIGNAL(triggered()), this, SLOT(zoomPrevious()));
  //
  mActionZoomToLayer= new QAction(getThemeIcon("/mActionZoomToLayer.png"), tr("Zoom To Layer"), this);
  //mActionZoomToLayer->setShortcut(tr("Ctrl+O","Zoom to Layer"));
  mActionZoomToLayer->setStatusTip(tr("Zoom to Layer"));
  connect(mActionZoomToLayer, SIGNAL(triggered()), this, SLOT(zoomToLayerExtent()));
  //
  mActionIdentify= new QAction(getThemeIcon("/mActionIdentify.png"), tr("Identify Features"), this);
  mActionIdentify->setShortcut(tr("I","Click on features to identify them"));
  mActionIdentify->setStatusTip(tr("Click on features to identify them"));
  connect(mActionIdentify, SIGNAL(triggered()), this, SLOT(identify()));
  //
  mActionSelect= new QAction(getThemeIcon("/mActionSelect.png"), tr("Select Features"), this);
  mActionSelect->setStatusTip(tr("Select Features"));
  connect(mActionSelect, SIGNAL(triggered()), this, SLOT(select()));
  mActionSelect->setEnabled(false);
  //
  mActionOpenTable= new QAction(getThemeIcon("/mActionOpenTable.png"), tr("Open Table"), this);
  //mActionOpenTable->setShortcut(tr("Ctrl+O","Open Table"));
  mActionOpenTable->setStatusTip(tr("Open Table"));
  connect(mActionOpenTable, SIGNAL(triggered()), this, SLOT(attributeTable()));
  mActionOpenTable->setEnabled(false);
  //
  mActionMeasure= new QAction(getThemeIcon("/mActionMeasure.png"), tr("Measure Line "), this);
  mActionMeasure->setShortcut(tr("Ctrl+M","Measure a Line"));
  mActionMeasure->setStatusTip(tr("Measure a Line"));
  connect(mActionMeasure, SIGNAL(triggered()), this, SLOT(measure()));
  //
  mActionMeasureArea= new QAction(getThemeIcon("/mActionMeasureArea.png"), tr("Measure Area"), this);
  mActionMeasureArea->setShortcut(tr("Ctrl+J","Measure an Area"));
  mActionMeasureArea->setStatusTip(tr("Measure an Area"));
  connect(mActionMeasureArea, SIGNAL(triggered()), this, SLOT(measureArea()));
  //
  mActionShowBookmarks= new QAction(getThemeIcon("/mActionShowBookmarks.png"), tr("Show Bookmarks"), this);
  mActionShowBookmarks->setShortcut(tr("B","Show Bookmarks"));
  mActionShowBookmarks->setStatusTip(tr("Show Bookmarks"));
  connect(mActionShowBookmarks, SIGNAL(triggered()), this, SLOT(showBookmarks()));
  //
  mActionShowAllToolbars = new QAction(tr("Show most toolbars"), this);
  mActionShowAllToolbars->setShortcut(tr("T", "Show most toolbars"));
  mActionShowAllToolbars->setStatusTip(tr("Show most toolbars"));
  connect(mActionShowAllToolbars, SIGNAL(triggered()), this,
          SLOT(showAllToolbars()));
  //
  mActionHideAllToolbars = new QAction(tr("Hide most toolbars"), this);
  mActionHideAllToolbars->setShortcut(tr("Ctrl+T", "Hide most toolbars"));
  mActionHideAllToolbars->setStatusTip(tr("Hide most toolbars"));
  connect(mActionHideAllToolbars, SIGNAL(triggered()), this,
          SLOT(hideAllToolbars()));
  //
  mActionNewBookmark= new QAction(getThemeIcon("/mActionNewBookmark.png"), tr("New Bookmark..."), this);
  mActionNewBookmark->setShortcut(tr("Ctrl+B","New Bookmark"));
  mActionNewBookmark->setStatusTip(tr("New Bookmark"));
  connect(mActionNewBookmark, SIGNAL(triggered()), this, SLOT(newBookmark()));
  //
  mActionAddWmsLayer= new QAction(getThemeIcon("/mActionAddWmsLayer.png"), tr("Add WMS Layer..."), this);
  mActionAddWmsLayer->setShortcut(tr("W","Add Web Mapping Server Layer"));
  mActionAddWmsLayer->setStatusTip(tr("Add Web Mapping Server Layer"));
  connect(mActionAddWmsLayer, SIGNAL(triggered()), this, SLOT(addWmsLayer()));
  //
  mActionInOverview= new QAction(getThemeIcon("/mActionInOverview.png"), tr("In Overview"), this);
  mActionInOverview->setShortcut(tr("O","Add current layer to overview map"));
  mActionInOverview->setStatusTip(tr("Add current layer to overview map"));
  connect(mActionInOverview, SIGNAL(triggered()), this, SLOT(inOverview()));
  mActionInOverview->setEnabled(false);
  //
  // Plugin Menu Related Items
  //
  mActionShowPluginManager= new QAction(getThemeIcon("/mActionShowPluginManager.png"), tr("Plugin Manager..."), this);
  // mActionShowPluginManager->setShortcut(tr("Ctrl+P","Open the plugin manager"));
  mActionShowPluginManager->setStatusTip(tr("Open the plugin manager"));
  connect(mActionShowPluginManager, SIGNAL(triggered()), this, SLOT(showPluginManager()));
  //
  // Add the whats this toolbar button
  // QWhatsThis::whatsThisButton(mHelpToolBar);
  // 
  //
  // Digitising Toolbar Items
  //

  mActionToggleEditing = new QAction(getThemeIcon("/mActionToggleEditing.png"), 
                                    tr("Toggle editing"), this);
  mActionToggleEditing->setStatusTip(tr("Toggles the editing state of the current layer")); 
  mActionToggleEditing->setCheckable(true);
  connect(mActionToggleEditing, SIGNAL(triggered()), this, SLOT(toggleEditing()));
  mActionToggleEditing->setEnabled(false);
  
  //
  mActionCapturePoint= new QAction(getThemeIcon("/mActionCapturePoint.png"), tr("Capture Point"), this);
  mActionCapturePoint->setShortcut(tr(".","Capture Points"));
  mActionCapturePoint->setStatusTip(tr("Capture Points"));
  connect(mActionCapturePoint, SIGNAL(triggered()), this, SLOT(capturePoint()));
  mActionCapturePoint->setEnabled(false);
  //
  mActionCaptureLine= new QAction(getThemeIcon("/mActionCaptureLine.png"), tr("Capture Line"), this);
  mActionCaptureLine->setShortcut(tr("/","Capture Lines"));
  mActionCaptureLine->setStatusTip(tr("Capture Lines"));
  connect(mActionCaptureLine, SIGNAL(triggered()), this, SLOT(captureLine()));
  mActionCaptureLine->setEnabled(false);
  //
  mActionCapturePolygon= new QAction(getThemeIcon("/mActionCapturePolygon.png"), tr("Capture Polygon"), this);
  mActionCapturePolygon->setShortcut(tr("Ctrl+/","Capture Polygons"));
  mActionCapturePolygon->setStatusTip(tr("Capture Polygons"));
  connect(mActionCapturePolygon, SIGNAL(triggered()), this, SLOT(capturePolygon()));
  mActionCapturePolygon->setEnabled(false);
  //
  mActionDeleteSelected = new QAction(getThemeIcon("/mActionDeleteSelected.png"), tr("Delete Selected"), this);
  mActionDeleteSelected->setStatusTip(tr("Delete Selected"));
  connect(mActionDeleteSelected, SIGNAL(triggered()), this, SLOT(deleteSelected()));
  mActionDeleteSelected->setEnabled(false);
  //
  mActionMoveFeature = new QAction(getThemeIcon("/mActionMoveFeature.png"), tr("Move Feature"), this);
  mActionMoveFeature->setStatusTip(tr("Move Feature"));
  connect(mActionMoveFeature, SIGNAL(triggered()), this, SLOT(moveFeature()));
  mActionMoveFeature->setEnabled(false);
  //
  mActionSplitFeatures = new QAction(getThemeIcon("/mActionSplitFeatures.png"), tr("Split Features"), this);
  mActionSplitFeatures->setStatusTip(tr("Split Features"));
  connect(mActionSplitFeatures, SIGNAL(triggered()), this, SLOT(splitFeatures()));
  mActionSplitFeatures->setEnabled(false);
  //
  mActionAddVertex = new QAction(getThemeIcon("/mActionAddVertex.png"), tr("Add Vertex"), this);
  mActionAddVertex->setStatusTip(tr("Add Vertex"));
  connect(mActionAddVertex, SIGNAL(triggered()), this, SLOT(addVertex()));
  mActionAddVertex->setEnabled(false);
  //
  mActionDeleteVertex = new QAction(getThemeIcon("/mActionDeleteVertex.png"), tr("Delete Vertex"), this);
  mActionDeleteVertex->setStatusTip(tr("Delete Vertex"));
  connect(mActionDeleteVertex, SIGNAL(triggered()), this, SLOT(deleteVertex()));
  mActionDeleteVertex->setEnabled(false);
  //
  mActionMoveVertex = new QAction(getThemeIcon("/mActionMoveVertex.png"), tr("Move Vertex"), this);
  mActionMoveVertex->setStatusTip(tr("Move Vertex"));
  connect(mActionMoveVertex, SIGNAL(triggered()), this, SLOT(moveVertex()));
  mActionMoveVertex->setEnabled(false);

  mActionAddRing = new QAction(getThemeIcon("/mActionAddRing.png"), tr("Add Ring"), this);
  mActionAddRing->setStatusTip(tr("Add Ring"));
  connect(mActionAddRing, SIGNAL(triggered()), this, SLOT(addRing()));
  mActionAddRing->setEnabled(false);

  mActionAddIsland = new QAction(getThemeIcon("/mActionAddIsland.png"), tr("Add Island"), this);
  mActionAddIsland->setStatusTip(tr("Add Island to multipolygon"));
  connect(mActionAddIsland, SIGNAL(triggered()), this, SLOT(addIsland()));
  mActionAddIsland->setEnabled(false);

  mActionEditCut = new QAction(getThemeIcon("/mActionEditCut.png"), tr("Cut Features"), this);
  mActionEditCut->setStatusTip(tr("Cut selected features"));
  connect(mActionEditCut, SIGNAL(triggered()), this, SLOT(editCut()));
  mActionEditCut->setEnabled(false);

  mActionEditCopy = new QAction(getThemeIcon("/mActionEditCopy.png"), tr("Copy Features"), this);
  mActionEditCopy->setStatusTip(tr("Copy selected features"));
  connect(mActionEditCopy, SIGNAL(triggered()), this, SLOT(editCopy()));
  mActionEditCopy->setEnabled(false);

  mActionEditPaste = new QAction(getThemeIcon("/mActionEditPaste.png"), tr("Paste Features"), this);
  mActionEditPaste->setStatusTip(tr("Paste selected features"));
  connect(mActionEditPaste, SIGNAL(triggered()), this, SLOT(editPaste()));
  mActionEditPaste->setEnabled(false);

  // maptips
  mActionMapTips = new QAction(getThemeIcon("/mActionMapTips.png"), tr("Map Tips"), this);
  mActionMapTips->setStatusTip(tr("Show information about a feature when the mouse is hovered over it"));
  connect ( mActionMapTips, SIGNAL ( triggered() ), this, SLOT ( toggleMapTips() ) );
  mActionMapTips->setCheckable(true);
}

void QgisApp::showPythonDialog()
{
  if (!mPythonUtils || !mPythonUtils->isEnabled())
    return;
  
  if (mPythonConsole == NULL)
    mPythonConsole = new QgsPythonDialog(mQgisInterface, mPythonUtils);
  mPythonConsole->show();
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
  mActionMoveFeature->setCheckable(true);
  mMapToolGroup->addAction(mActionMoveFeature);
  mActionSplitFeatures->setCheckable(true);
  mMapToolGroup->addAction(mActionSplitFeatures);
  mMapToolGroup->addAction(mActionDeleteSelected);
  mActionAddVertex->setCheckable(true);
  mMapToolGroup->addAction(mActionAddVertex);
  mActionDeleteVertex->setCheckable(true);
  mMapToolGroup->addAction(mActionDeleteVertex);
  mActionMoveVertex->setCheckable(true);
  mMapToolGroup->addAction(mActionMoveVertex);
  mActionAddRing->setCheckable(true);
  mMapToolGroup->addAction(mActionAddRing);
  mActionAddIsland->setCheckable(true);
  mMapToolGroup->addAction(mActionAddIsland);
}

void QgisApp::createMenus()
{
  //
  // File Menu
  mFileMenu = menuBar()->addMenu(tr("&File"));
  mFileMenu->addAction(mActionFileNew);
  mFileMenu->addAction(mActionFileOpen);
  mRecentProjectsMenu = mFileMenu->addMenu(tr("&Open Recent Projects"));
  // Connect once for the entire submenu.
  connect(mRecentProjectsMenu, SIGNAL(triggered(QAction *)),
          this, SLOT(openProject(QAction *)));

  mFileMenu->addSeparator();
  mFileMenu->addAction(mActionFileSave);
  mFileMenu->addAction(mActionFileSaveAs);
  mFileMenu->addAction(mActionSaveMapAsImage);
  mFileMenu->addSeparator();
  // TODO: remove the addAction once the mapserver export plugin is complete and tested
  /*
  mFileMenu->addAction(mActionExportMapServer);
  */
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
  mViewMenu->addAction(mActionToggleFullScreen);
  mViewMenu->addSeparator();
  mViewMenu->addAction(mActionShowBookmarks);
  mViewMenu->addAction(mActionNewBookmark);
  mViewMenu->addSeparator();

  //
  // View:toolbars menu
  mViewMenu->addAction(mActionShowAllToolbars);
  mViewMenu->addAction(mActionHideAllToolbars);

  //
  // Layers Menu
  mLayerMenu = menuBar()->addMenu(tr("&Layer"));
  mLayerMenu->addAction(mActionAddOgrLayer);
  mLayerMenu->addAction(mActionAddRasterLayer);
#ifdef HAVE_POSTGRESQL
  mLayerMenu->addAction(mActionAddLayer);
#endif
  mLayerMenu->addAction(mActionAddWmsLayer);
  mLayerMenu->addSeparator();
  mLayerMenu->addAction(mActionRemoveLayer);
  mLayerMenu->addAction(mActionNewVectorLayer);
  mLayerMenu->addSeparator();
  mLayerMenu->addAction(mActionInOverview);
  mLayerMenu->addAction(mActionAddAllToOverview);
  mLayerMenu->addAction(mActionRemoveAllFromOverview);
  mLayerMenu->addSeparator();
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
  mPluginMenu->addSeparator();

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
  mHelpMenu->addSeparator();
  mHelpMenu->addAction(mActionQgisHomePage);
  mHelpMenu->addAction(mActionCheckQgisVersion);
  mHelpMenu->addSeparator();
  mHelpMenu->addAction(mActionHelpAbout);
}

void QgisApp::createToolBars()
{
  QSize myIconSize ( 24,24 );
  // QSize myIconSize ( 32,32 ); //large icons
  // Note: we need to set each object name to ensure that
  // qmainwindow::saveState and qmainwindow::restoreState
  // work properly
  
  //
  // File Toolbar
  mFileToolBar = addToolBar(tr("File"));
  mFileToolBar->setIconSize(myIconSize);
  mFileToolBar->setObjectName("FileToolBar");
  mFileToolBar->addAction(mActionFileNew);
  mFileToolBar->addAction(mActionFileNew);
  mFileToolBar->addAction(mActionFileSave);
  mFileToolBar->addAction(mActionFileSaveAs);
  mFileToolBar->addAction(mActionFileOpen);
  mFileToolBar->addAction(mActionFilePrint);
  mFileToolBar->addAction(mActionAddOgrLayer);
  mFileToolBar->addAction(mActionAddRasterLayer);
#ifdef HAVE_POSTGRESQL
  mFileToolBar->addAction(mActionAddLayer);
#endif
  mFileToolBar->addAction(mActionAddWmsLayer);
  //
  // Layer Toolbar
  mLayerToolBar = addToolBar(tr("Manage Layers"));
  mLayerToolBar->setIconSize(myIconSize);
  mLayerToolBar->setObjectName("LayerToolBar");
  mLayerToolBar->addAction(mActionNewVectorLayer);
  mLayerToolBar->addAction(mActionRemoveLayer);
  mLayerToolBar->addAction(mActionInOverview);
  mLayerToolBar->addAction(mActionShowAllLayers);
  mLayerToolBar->addAction(mActionHideAllLayers);
  //
  // Digitizing Toolbar
  mDigitizeToolBar = addToolBar(tr("Digitizing"));
  mDigitizeToolBar->setIconSize(myIconSize);
  mDigitizeToolBar->setObjectName("Digitizing");
  mDigitizeToolBar->addAction(mActionToggleEditing);
  mDigitizeToolBar->addAction(mActionCapturePoint);
  mDigitizeToolBar->addAction(mActionCaptureLine);
  mDigitizeToolBar->addAction(mActionCapturePolygon);
  mDigitizeToolBar->addAction(mActionAddRing);
  mDigitizeToolBar->addAction(mActionAddIsland);
  mDigitizeToolBar->addAction(mActionSplitFeatures);
  mDigitizeToolBar->addAction(mActionMoveFeature);
  mDigitizeToolBar->addAction(mActionMoveVertex);
  mDigitizeToolBar->addAction(mActionAddVertex);
  mDigitizeToolBar->addAction(mActionDeleteVertex);
  mDigitizeToolBar->addAction(mActionDeleteSelected);
  mDigitizeToolBar->addAction(mActionEditCut);
  mDigitizeToolBar->addAction(mActionEditCopy);
  mDigitizeToolBar->addAction(mActionEditPaste);
  //
  // Map Navigation Toolbar
  mMapNavToolBar = addToolBar(tr("Map Navigation"));
  mMapNavToolBar->setIconSize(myIconSize);
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
  mAttributesToolBar->setIconSize(myIconSize);
  mAttributesToolBar->setObjectName("Attributes");
  mAttributesToolBar->addAction(mActionIdentify);
  mAttributesToolBar->addAction(mActionSelect);
  mAttributesToolBar->addAction(mActionOpenTable);
  mAttributesToolBar->addAction(mActionMeasure);
  mAttributesToolBar->addAction(mActionMeasureArea);
  mAttributesToolBar->addAction(mActionMapTips);
  mAttributesToolBar->addAction(mActionShowBookmarks);
  mAttributesToolBar->addAction(mActionNewBookmark);
  //
  // Plugins Toolbar
  mPluginToolBar = addToolBar(tr("Plugins"));
  mPluginToolBar->setIconSize(myIconSize);
  mPluginToolBar->setObjectName("Plugins");
  //
  // Help Toolbar
  mHelpToolBar = addToolBar(tr("Help"));
  mHelpToolBar->setIconSize(myIconSize);
  mHelpToolBar->setObjectName("Help");
  mHelpToolBar->addAction(mActionHelpContents);
  mHelpToolBar->addAction(QWhatsThis::createAction());

  //Add the menu for toolbar visibility here
  //because createPopupMenu() would return 0
  //before the toolbars are created
  QMenu* toolbarVisibilityMenu = createPopupMenu();
  if(toolbarVisibilityMenu)
    {
      toolbarVisibilityMenu->setTitle(tr("Toolbar Visibility..."));
      mViewMenu->addMenu(toolbarVisibilityMenu);
    }
}

void QgisApp::createStatusBar()
{
  //
  // Add a panel to the status bar for the scale, coords and progress
  // And also rendering suppression checkbox
  //
  mProgressBar = new QProgressBar(statusBar());
  mProgressBar->setMaximumWidth(100);
  mProgressBar->hide();
  mProgressBar->setWhatsThis(tr("Progress bar that displays the status "
        "of rendering layers and other time-intensive operations"));
  statusBar()->addPermanentWidget(mProgressBar, 1);
  // Bumped the font up one point size since 8 was too 
  // small on some platforms. A point size of 9 still provides
  // plenty of display space on 1024x768 resolutions
  QFont myFont( "Arial", 9 );

  statusBar()->setFont(myFont);
  mScaleLabel = new QLabel(QString(),statusBar());
  mScaleLabel->setFont(myFont);
  mScaleLabel->setMinimumWidth(10);
  mScaleLabel->setMaximumHeight(20);
  mScaleLabel->setMargin(3);
  mScaleLabel->setAlignment(Qt::AlignCenter);
  mScaleLabel->setFrameStyle(QFrame::NoFrame);
  mScaleLabel->setText(tr("Scale "));
  mScaleLabel->setToolTip(tr("Current map scale"));
  statusBar()->addPermanentWidget(mScaleLabel, 0);

  mScaleEdit = new QLineEdit(QString(),statusBar());
  mScaleEdit->setFont(myFont);
  mScaleEdit->setMinimumWidth(10);
  mScaleEdit->setMaximumWidth(100);
  mScaleEdit->setMaximumHeight(20);
  mScaleEdit->setContentsMargins(0, 0, 0, 0);
  mScaleEdit->setAlignment(Qt::AlignLeft);
  QRegExp validator("\\d+\\.?\\d*:\\d+\\.?\\d*");
  mScaleEditValidator = new QRegExpValidator(validator, mScaleEdit);
  mScaleEdit->setValidator(mScaleEditValidator);
  mScaleEdit->setWhatsThis(tr("Displays the current map scale"));
  mScaleEdit->setToolTip(tr("Current map scale (formatted as x:y)"));
  statusBar()->addPermanentWidget(mScaleEdit, 0);
  connect(mScaleEdit, SIGNAL(editingFinished()), this, SLOT(userScale()));

  //coords status bar widget
  mCoordsLabel = new QLabel(QString(), statusBar());
  mCoordsLabel->setMinimumWidth(10);
  mCoordsLabel->setMaximumHeight(20);
  mCoordsLabel->setFont(myFont);
  mCoordsLabel->setMargin(3);
  mCoordsLabel->setAlignment(Qt::AlignCenter);
  mCoordsLabel->setWhatsThis(tr("Shows the map coordinates at the "
        "current cursor position. The display is continuously updated "
        "as the mouse is moved."));
  mCoordsLabel->setToolTip(tr("Map coordinates at mouse cursor position"));
  statusBar()->addPermanentWidget(mCoordsLabel, 0);
  //stop rendering status bar widget
  mStopRenderButton = new QToolButton( statusBar() );
  mStopRenderButton->setMaximumWidth(20);
  mStopRenderButton->setMaximumHeight(20);
  mStopRenderButton->setIcon(getThemeIcon("/mIconDelete.png"));
  mStopRenderButton->setToolTip(tr("Stop map rendering") );
  statusBar()->addPermanentWidget(mStopRenderButton, 0);
  // render suppression status bar widget
  mRenderSuppressionCBox = new QCheckBox(tr("Render"),statusBar());
  mRenderSuppressionCBox->setChecked(true);
  mRenderSuppressionCBox->setFont(myFont);
  mRenderSuppressionCBox->setWhatsThis(tr("When checked, the map layers "
        "are rendered in response to map navigation commands and other "
        "events. When not checked, no rendering is done. This allows you "
        "to add a large number of layers and symbolize them before rendering."));
  mRenderSuppressionCBox->setToolTip(tr("Toggle map rendering") );
  statusBar()->addPermanentWidget(mRenderSuppressionCBox,0);
  // On the fly projection status bar icon
  // Changed this to a tool button since a QPushButton is
  // sculpted on OS X and the icon is never displayed [gsherman]
  mOnTheFlyProjectionStatusButton = new QToolButton(statusBar());
  mOnTheFlyProjectionStatusButton->setMaximumWidth(20);
  // Maintain uniform widget height in status bar by setting button height same as labels
  // For Qt/Mac 3.3, the default toolbutton height is 30 and labels were expanding to match
  mOnTheFlyProjectionStatusButton->setMaximumHeight(mScaleLabel->height());
  mOnTheFlyProjectionStatusButton->setIcon(getThemeIcon("/mIconProjectionDisabled.png"));
  if (!QFile::exists(QgsApplication::defaultThemePath() + "/mIconProjectionDisabled.png"))
  {
    QMessageBox::critical(this, tr("Resource Location Error"), 
        tr("Error reading icon resources from: \n %1\n Quitting...").arg(
          QgsApplication::defaultThemePath() + "/mIconProjectionDisabled.png"
          ));
    exit(0);
  }
  mOnTheFlyProjectionStatusButton->setWhatsThis(tr("This icon shows whether "
        "on the fly projection is enabled or not. Click the icon to bring up "
        "the project properties dialog to alter this behaviour."));
  mOnTheFlyProjectionStatusButton->setToolTip(tr("Projection status - Click "
        "to open projection dialog"));
  connect(mOnTheFlyProjectionStatusButton, SIGNAL(clicked()),
      this, SLOT(projectPropertiesProjections()));//bring up the project props dialog when clicked
  statusBar()->addPermanentWidget(mOnTheFlyProjectionStatusButton,0);
  statusBar()->showMessage(tr("Ready"));
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
  // and modifying the appropriate CMakeLists.txt files. User contributed themes
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
  QgsApplication::setTheme(theThemeName);
  QgsDebugMsg("Setting theme to \n" + QgsApplication::activeThemePath());
  mActionFileNew->setIcon(getThemeIcon( "/mActionFileNew.png"));
  mActionFileSave->setIcon(getThemeIcon( "/mActionFileSave.png"));
  mActionFileSaveAs->setIcon(getThemeIcon( "/mActionFileSaveAs.png"));
  mActionFileOpen->setIcon(getThemeIcon( "/mActionFileOpen.png"));
  mActionFilePrint->setIcon(getThemeIcon( "/mActionFilePrint.png"));
  mActionSaveMapAsImage->setIcon(getThemeIcon( "/mActionSaveMapAsImage.png"));
  // TODO: Remove the mActionExportMapServer related code once the mapserver export plugin 
  /*
  mActionExportMapServer->setIcon(getThemeIcon( "/mActionExportMapServer.png"));
  */
  mActionFileExit->setIcon(getThemeIcon( "/mActionFileExit.png"));
  mActionAddOgrLayer->setIcon(getThemeIcon( "/mActionAddOgrLayer.png"));
  mActionAddRasterLayer->setIcon(getThemeIcon( "/mActionAddRasterLayer.png"));
  mActionAddLayer->setIcon(getThemeIcon( "/mActionAddLayer.png"));
  mActionRemoveLayer->setIcon(getThemeIcon( "/mActionRemoveLayer.png"));
  mActionNewVectorLayer->setIcon(getThemeIcon( "/mActionNewVectorLayer.png"));
  mActionAddAllToOverview->setIcon(getThemeIcon( "/mActionAddAllToOverview.png"));
  mActionHideAllLayers->setIcon(getThemeIcon( "/mActionHideAllLayers.png"));
  mActionShowAllLayers->setIcon(getThemeIcon( "/mActionShowAllLayers.png"));
  mActionRemoveAllFromOverview->setIcon(getThemeIcon( "/mActionRemoveAllFromOverview.png"));
  mActionProjectProperties->setIcon(getThemeIcon( "/mActionProjectProperties.png"));
  mActionShowPluginManager->setIcon(getThemeIcon( "/mActionShowPluginManager.png"));
  mActionCheckQgisVersion->setIcon(getThemeIcon( "/mActionCheckQgisVersion.png"));
  mActionOptions->setIcon(getThemeIcon( "/mActionOptions.png"));
  mActionHelpContents->setIcon(getThemeIcon( "/mActionHelpContents.png"));
  mActionQgisHomePage->setIcon(getThemeIcon( "/mActionQgisHomePage.png"));
  mActionHelpAbout->setIcon(getThemeIcon( "/mActionHelpAbout.png"));
  mActionDraw->setIcon(getThemeIcon( "/mActionDraw.png"));
  mActionCapturePoint->setIcon(getThemeIcon( "/mActionCapturePoint.png"));
  mActionCaptureLine->setIcon(getThemeIcon( "/mActionCaptureLine.png"));
  mActionCapturePolygon->setIcon(getThemeIcon( "/mActionCapturePolygon.png"));
  mActionZoomIn->setIcon(getThemeIcon( "/mActionZoomIn.png"));
  mActionZoomOut->setIcon(getThemeIcon( "/mActionZoomOut.png"));
  mActionZoomFullExtent->setIcon(getThemeIcon( "/mActionZoomFullExtent.png"));
  mActionZoomToSelected->setIcon(getThemeIcon( "/mActionZoomToSelected.png"));
  mActionPan->setIcon(getThemeIcon( "/mActionPan.png"));
  mActionZoomLast->setIcon(getThemeIcon( "/mActionZoomLast.png"));
  mActionZoomToLayer->setIcon(getThemeIcon( "/mActionZoomToLayer.png"));
  mActionIdentify->setIcon(getThemeIcon( "/mActionIdentify.png"));
  mActionSelect->setIcon(getThemeIcon( "/mActionSelect.png"));
  mActionOpenTable->setIcon(getThemeIcon( "/mActionOpenTable.png"));
  mActionMeasure->setIcon(getThemeIcon( "/mActionMeasure.png"));
  mActionMeasureArea->setIcon(getThemeIcon( "/mActionMeasureArea.png"));
  mActionShowBookmarks->setIcon(getThemeIcon( "/mActionShowBookmarks.png"));
  mActionNewBookmark->setIcon(getThemeIcon( "/mActionNewBookmark.png"));
  mActionCustomProjection->setIcon(getThemeIcon( "/mActionCustomProjection.png"));
  mActionAddWmsLayer->setIcon(getThemeIcon( "/mActionAddWmsLayer.png"));
  mActionInOverview->setIcon(getThemeIcon( "/mActionInOverview.png"));
}

void QgisApp::setupConnections()
{
  // connect the "cleanup" slot
  connect(qApp, SIGNAL(aboutToQuit()), this, SLOT(saveWindowState()));
  //connect the legend, mapcanvas and overview canvas to the registry
  
  // connect map layer registry signals to legend
  connect(QgsMapLayerRegistry::instance(), SIGNAL(layerWillBeRemoved(QString)),
          mMapLegend, SLOT(removeLayer(QString)));
  connect(QgsMapLayerRegistry::instance(), SIGNAL(removedAll()),
          mMapLegend, SLOT(removeAll()));
  connect(QgsMapLayerRegistry::instance(), SIGNAL(layerWasAdded(QgsMapLayer*)),
          mMapLegend, SLOT(addLayer(QgsMapLayer *)));
  connect(mMapLegend, SIGNAL(currentLayerChanged(QgsMapLayer*)),
          this, SLOT(activateDeactivateLayerRelatedActions(QgsMapLayer*)));

  
  //signal when mouse moved over window (coords display in status bar)
  connect(mMapCanvas, SIGNAL(xyCoordinates(QgsPoint &)), this, SLOT(showMouseCoordinate(QgsPoint &)));
  //signal when mouse in capturePoint mode and mouse clicked on canvas
  connect(mMapCanvas->mapRender(), SIGNAL(drawingProgress(int,int)), this, SLOT(showProgress(int,int)));
  connect(mMapCanvas->mapRender(), SIGNAL(projectionsEnabled(bool)), this, SLOT(projectionsEnabled(bool)));
  connect(mMapCanvas->mapRender(), SIGNAL(destinationSrsChanged()), this, SLOT(destinationSrsChanged()));
  connect(mMapCanvas, SIGNAL(extentsChanged()),this,SLOT(showExtents()));
  connect(mMapCanvas, SIGNAL(scaleChanged(double)), this, SLOT(showScale(double)));
  connect(mMapCanvas, SIGNAL(scaleChanged(double)), this, SLOT(updateMouseCoordinatePrecision()));
  connect(mMapCanvas, SIGNAL(mapToolSet(QgsMapTool *)), this, SLOT(mapToolChanged(QgsMapTool *)));

  connect(mRenderSuppressionCBox, SIGNAL(toggled(bool )), mMapCanvas, SLOT(setRenderFlag(bool)));
  //
  // Do we really need this ??? - its already connected to the esc key...TS
  //
  connect(mStopRenderButton, SIGNAL(clicked()), this, SLOT(stopRendering()));

  // Connect warning dialog from project reading
  connect(QgsProject::instance(), SIGNAL(warnOlderProjectVersion(QString)),
          this, SLOT(warnOlderProjectVersion(QString)));

}
void QgisApp::createCanvas()
{
  // "theMapCanvas" used to find this canonical instance later
  mMapCanvas = new QgsMapCanvas(this, "theMapCanvas" );
  mMapCanvas->setWhatsThis(tr("Map canvas. This is where raster and vector "
        "layers are displayed when added to the map"));
  
//  mMapCanvas->setMinimumWidth(10);
//  QVBoxLayout *myCanvasLayout = new QVBoxLayout;
//  myCanvasLayout->addWidget(mMapCanvas);
//  tabWidget->widget(0)->setLayout(myCanvasLayout);
  setCentralWidget(mMapCanvas);
  // set the focus to the map canvas
  mMapCanvas->setFocus();

  // create tools
  mMapTools.mZoomIn = new QgsMapToolZoom(mMapCanvas, FALSE /* zoomIn */);
  mMapTools.mZoomIn->setAction(mActionZoomIn);
  mMapTools.mZoomOut = new QgsMapToolZoom(mMapCanvas, TRUE /* zoomOut */);
  mMapTools.mZoomOut->setAction(mActionZoomOut);
  mMapTools.mPan = new QgsMapToolPan(mMapCanvas);
  mMapTools.mPan->setAction(mActionPan);
  mMapTools.mIdentify = new QgsMapToolIdentify(mMapCanvas);
  mMapTools.mIdentify->setAction(mActionIdentify);
  mMapTools.mMeasureDist = new QgsMeasureTool(mMapCanvas, FALSE /* area */);
  mMapTools.mMeasureDist->setAction(mActionMeasure);
  mMapTools.mMeasureArea = new QgsMeasureTool(mMapCanvas, TRUE /* area */);
  mMapTools.mMeasureArea->setAction(mActionMeasureArea);
  mMapTools.mCapturePoint = new QgsMapToolAddFeature(mMapCanvas, QgsMapToolCapture::CapturePoint);
  mMapTools.mCapturePoint->setAction(mActionCapturePoint);
  mMapTools.mCaptureLine = new QgsMapToolAddFeature(mMapCanvas, QgsMapToolCapture::CaptureLine);
  mMapTools.mCaptureLine->setAction(mActionCaptureLine);
  mMapTools.mCapturePolygon = new QgsMapToolAddFeature(mMapCanvas, QgsMapToolCapture::CapturePolygon);
  mMapTools.mCapturePolygon->setAction(mActionCapturePolygon);
  mMapTools.mMoveFeature = new QgsMapToolMoveFeature(mMapCanvas);
  mMapTools.mMoveFeature->setAction(mActionMoveFeature);
  mMapTools.mSplitFeatures = new QgsMapToolSplitFeatures(mMapCanvas);
  mMapTools.mSplitFeatures->setAction(mActionSplitFeatures);
  mMapTools.mSelect = new QgsMapToolSelect(mMapCanvas);
  mMapTools.mSelect->setAction(mActionSelect);
  mMapTools.mVertexAdd = new QgsMapToolAddVertex(mMapCanvas);
  mMapTools.mVertexAdd->setAction(mActionAddVertex);
  mMapTools.mVertexMove = new QgsMapToolMoveVertex(mMapCanvas);
  mMapTools.mVertexMove->setAction(mActionMoveVertex);
  mMapTools.mVertexDelete = new QgsMapToolDeleteVertex(mMapCanvas);
  mMapTools.mVertexDelete->setAction(mActionDeleteVertex);
  mMapTools.mAddRing = new QgsMapToolAddRing(mMapCanvas);
  mMapTools.mAddRing->setAction(mActionAddRing);
  mMapTools.mAddIsland = new QgsMapToolAddIsland(mMapCanvas);
  //ensure that non edit tool is initialised or we will get crashes in some situations
  mNonEditMapTool = mMapTools.mPan; 
}

void QgisApp::createOverview()
{
  // overview canvas
  QgsMapOverviewCanvas* overviewCanvas = new QgsMapOverviewCanvas(NULL, mMapCanvas);
  overviewCanvas->setWhatsThis(tr("Map overview canvas. This canvas can be used to display a locator map that shows the current extent of the map canvas. The current extent is shown as a red rectangle. Any layer on the map can be added to the overview canvas."));
        
  QBitmap overviewPanBmp = QBitmap::fromData(QSize(16, 16), pan_bits);
  QBitmap overviewPanBmpMask = QBitmap::fromData(QSize(16, 16), pan_mask_bits);
  mOverviewMapCursor = new QCursor(overviewPanBmp, overviewPanBmpMask, 5, 5);
  overviewCanvas->setCursor(*mOverviewMapCursor);
//  QVBoxLayout *myOverviewLayout = new QVBoxLayout;
//  myOverviewLayout->addWidget(overviewCanvas);
//  overviewFrame->setLayout(myOverviewLayout);
  mOverviewDock = new QDockWidget(tr("Overview"), this);
  mOverviewDock->setObjectName("Overview");
  mOverviewDock->setAllowedAreas(Qt::LeftDockWidgetArea | Qt::RightDockWidgetArea);
  mOverviewDock->setWidget(overviewCanvas);
  addDockWidget(Qt::LeftDockWidgetArea, mOverviewDock);
  mViewMenu->addAction(mOverviewDock->toggleViewAction());

  mMapCanvas->setOverview(overviewCanvas);
  
  // moved here to set anti aliasing to both map canvas and overview
  QSettings mySettings;
  mMapCanvas->enableAntiAliasing(mySettings.value("/qgis/enable_anti_aliasing",false).toBool());
  mMapCanvas->useQImageToRender(mySettings.value("/qgis/use_qimage_to_render",false).toBool());

  int action = mySettings.value("/qgis/wheel_action", 0).toInt();
  double zoomFactor = mySettings.value("/qgis/zoom_factor", 2).toDouble();
  mMapCanvas->setWheelAction((QgsMapCanvas::WheelAction) action, zoomFactor);
}


void QgisApp::createLegend()
{
  //legend
  mMapLegend = new QgsLegend(NULL, "theMapLegend");
  mMapLegend->setObjectName("theMapLegend");
  mMapLegend->setMapCanvas(mMapCanvas);

  //add the toggle editing action also to legend such that right click menu and button show the same state
  mMapLegend->setToggleEditingAction(mActionToggleEditing);

  mMapLegend->setWhatsThis(tr("Map legend that displays all the layers currently on the map canvas. Click on the check box to turn a layer on or off. Double click on a layer in the legend to customize its appearance and set other properties."));
//  QVBoxLayout *myLegendLayout = new QVBoxLayout;
//  myLegendLayout->addWidget(mMapLegend);
//  toolBox->widget(0)->setLayout(myLegendLayout);
  mLegendDock = new QDockWidget(tr("Legend"), this);
  mLegendDock->setObjectName("Legend");
  mLegendDock->setAllowedAreas(Qt::LeftDockWidgetArea | Qt::RightDockWidgetArea);
//  mLegendDock->setFeatures(mLegendDock->features() & ~QDockWidget::DockWidgetClosable);
  mLegendDock->setWidget(mMapLegend);
  addDockWidget(Qt::LeftDockWidgetArea, mLegendDock);
  mViewMenu->addAction(mLegendDock->toggleViewAction());
  return;
}

bool QgisApp::createDB()
{
  // Check qgis.db and make private copy if necessary
  QFile qgisPrivateDbFile(QgsApplication::qgisUserDbFilePath());

  // first we look for ~/.qgis/qgis.db
  if (!qgisPrivateDbFile.exists())
  {
    // if it doesnt exist we copy it in from the global resources dir
    QString qgisMasterDbFileName = QgsApplication::qgisMasterDbFilePath();
    QFile masterFile(qgisMasterDbFileName);

    // Must be sure there is destination directory ~/.qgis
    QDir().mkpath(QgsApplication::qgisSettingsDirPath());

    //now copy the master file into the users .qgis dir
    bool isDbFileCopied = masterFile.copy(qgisPrivateDbFile.fileName());

    if (!isDbFileCopied)
    {
      QgsDebugMsg("[ERROR] Can not make qgis.db private copy");
      return FALSE;
    }
  }
  return TRUE;
}

void QgisApp::createMapTips()
{
  // Set up the timer for maptips. The timer is reset everytime the mouse is moved
  mpMapTipsTimer = new QTimer ( mMapCanvas );
  // connect the timer to the maptips slot
  connect ( mpMapTipsTimer, SIGNAL ( timeout() ), this, SLOT ( showMapTip() ) );
  // set the interval to 0.850 seconds - timer will be started next time the mouse moves
  mpMapTipsTimer->setInterval ( 850 );
  // Create the maptips object
  mpMaptip = new QgsMapTip ();
}

// Update file menu with the current list of recently accessed projects
void QgisApp::updateRecentProjectPaths()
{
  // Remove existing paths from the recent projects menu
  int i;

  int menusize = mRecentProjectsMenu->actions().size();

  for(i = menusize; i < mRecentProjectPaths.size(); i++)
  {
    mRecentProjectsMenu->addAction("Dummy text");
  }

  QList<QAction *> menulist = mRecentProjectsMenu->actions();

  assert(menulist.size() == mRecentProjectPaths.size());

  for (i = 0; i < mRecentProjectPaths.size(); i++)
    {
      menulist.at(i)->setText(mRecentProjectPaths.at(i));

      // Disable this menu item if the file has been removed, if not enable it
      menulist.at(i)->setEnabled(QFile::exists((mRecentProjectPaths.at(i))));

    }
} // QgisApp::updateRecentProjectPaths

// add this file to the recently opened/saved projects list
void QgisApp::saveRecentProjectPath(QString projectPath, QSettings & settings)
{
  // Get canonical absolute path
  QFileInfo myFileInfo(projectPath);
  projectPath = myFileInfo.absoluteFilePath();

  // If this file is already in the list, remove it
  mRecentProjectPaths.removeAll(projectPath);

  // Prepend this file to the list
  mRecentProjectPaths.prepend(projectPath);

  // Keep the list to 8 items by trimming excess off the bottom
  while (mRecentProjectPaths.count() > 8)
  {
    mRecentProjectPaths.pop_back();
  }

  // Persist the list
  settings.setValue("/UI/recentProjectsList", mRecentProjectPaths);

  // Update menu list of paths
  updateRecentProjectPaths();

} // QgisApp::saveRecentProjectPath

void QgisApp::saveWindowState()
{
  // store window and toolbar positions
  QSettings settings;
  // store the toolbar/dock widget settings using Qt4 settings API
  settings.setValue("/UI/state", this->saveState());

  // store window geometry
  settings.setValue("/UI/geometry", saveGeometry());
//  settings.setValue("/UI/canvasSplitterState", canvasLegendSplit->saveState());
//  settings.setValue("/UI/legendSplitterState", legendOverviewSplit->saveState());
}

void QgisApp::restoreWindowState()
{
  // restore the toolbar and dock widgets postions using Qt4 settings API
  QSettings settings;
  QVariant vstate = settings.value("/UI/state");
  this->restoreState(vstate.toByteArray());

  // restore window geometry
  restoreGeometry(settings.value("/UI/geometry").toByteArray());
//  canvasLegendSplit->restoreState(settings.value("/UI/canvasSplitterState").toByteArray());
//  legendOverviewSplit->restoreState(settings.value("/UI/legendSplitterState").toByteArray());
}
///////////// END OF GUI SETUP ROUTINES ///////////////

void QgisApp::about()
{
  static QgsAbout *abt = NULL;
  if (!abt) {
     QApplication::setOverrideCursor(Qt::WaitCursor);
     abt = new QgsAbout();
     QString versionString = tr("You are using QGIS version %1 built against code revision %2.")
       .arg(QGis::qgisVersion)
       .arg(QGis::qgisSvnVersion);
#ifdef HAVE_POSTGRESQL

     versionString += tr(" This copy of QGIS has been built with PostgreSQL support.");
#else

     versionString += tr(" This copy of QGIS has been built without PostgreSQL support.");
#endif
     versionString += tr("\nThis binary was compiled against Qt %1,"
         "and is currently running against Qt %2") 
       .arg(QT_VERSION_STR)
       .arg(qVersion());

#ifdef WIN32
     // special version stuff for windows (if required)
     //  versionString += "\nThis is a Windows preview release - not for production use";
#endif

     abt->setVersion(versionString);
     QString whatsNew = "<html><body>" + tr("Version") + " ";
     whatsNew += QGis::qgisVersion;
     whatsNew += "<h3>" + tr("New features") + "</h3>" +
       tr("This release candidate includes over 60 bug fixes and enchancements "
           "over the QGIS 0.10.0 release. In addition we have added "
           "the following new features:");
     whatsNew += "<ul><li>"
       + tr("Revision of all dialogs for user interface consistancy")
       + "</li>"
       + "<li>"
       + tr("Improvements to unique value renderer vector dialog")
       + "</li>"
       + "<li>"
       + tr("Symbol previews when defining vector classes")
       + "</li>"
       + "<li>"
       + tr("Separation of python support into its own library")
       + "</li>"
       + "<li>"
       + tr("List view and filter for GRASS toolbox to find tools more quickly")
       + "</li>"
       + "<li>"
       + tr("List view and filter for Plugin Manager to find plugins more easily")
       + "</li>"
       + "<li>"
       + tr("Updated Spatial Reference System definitions")
       + "</li>"
       + "<li>"
       + tr("QML Style support for rasters and database layers")
       + "</li>"
//+ "<li>"
//+ tr("X")
//+ "</li>"
       + "</ul></body></html>";


     abt->setWhatsNew(whatsNew);

     QApplication::restoreOverrideCursor();
  }
  abt->show();
  abt->raise();
  abt->activateWindow();
}

/** Load up any plugins used in the last session
*/

void QgisApp::restoreSessionPlugins(QString thePluginDirString)
{
  QSettings mySettings;

  QgsApplication::showSettings();
  QgsDebugMsg("\n\n*************************************************");
  QgsDebugMsg("Restoring plugins from last session " + thePluginDirString);
  
#ifdef WIN32
  QString pluginExt = "*.dll";
#else
  QString pluginExt = "*.so*";
#endif

  // check all libs in the current plugin directory and get name and descriptions
  QDir myPluginDir(thePluginDirString, pluginExt, QDir::Name | QDir::IgnoreCase, QDir::Files | QDir::NoSymLinks);

  for (uint i = 0; i < myPluginDir.count(); i++)
  {
    QString myFullPath = thePluginDirString + "/" + myPluginDir[i];

    QgsDebugMsg("Examining " + myFullPath);

    QLibrary *myLib = new QLibrary(myFullPath);
    bool loaded = myLib->load();
    if (loaded)
    {
        //purposely leaving this one to stdout!
        std::cout << "Loaded " << myLib->fileName().toLocal8Bit().data() << std::endl;
        
        name_t * myName =(name_t *) myLib->resolve("name");
        description_t *  myDescription = (description_t *)  myLib->resolve("description");
        version_t *  myVersion =  (version_t *) myLib->resolve("version");
        if (myName && myDescription  && myVersion )
        {
          //check if the plugin was active on last session
          QString myEntryName = myName();
          // Windows stores a "true" value as a 1 in the registry so we
          // have to use readBoolEntry in this function

          if (mySettings.value("/Plugins/" + myEntryName).toBool())
          {
            QgsDebugMsg("Loading plugin: " + myEntryName);

            loadPlugin(myName(), myDescription(), myFullPath);
          }
          else
          {
            QgsDebugMsg("Plugin was not active last session, leaving disabled: " + myEntryName);
          }
        }
        else
        {
          QgsDebugMsg("Failed to get name, description, or type for " + myLib->fileName());
        }
    }
    else
    {
      QgsDebugMsg("Failed to load " + myLib->fileName());
      QgsDebugMsg("Reason: " + myLib->errorString());
    }
    delete myLib;
  }

  QString pluginName, description, version;
  
  if (mPythonUtils && mPythonUtils->isEnabled())
  {
  
    // check for python plugins system-wide
    QStringList pluginList = mPythonUtils->pluginList();

    for (int i = 0; i < pluginList.size(); i++)
    {
      QString packageName = pluginList[i];
   
      // import plugin's package
      if (!mPythonUtils->loadPlugin(packageName))
        continue;
      
      // get information from the plugin
      // if there are some problems, don't continue with metadata retreival
      pluginName = mPythonUtils->getPluginMetadata(packageName, "name");
      if (pluginName != "__error__")
      {
        description = mPythonUtils->getPluginMetadata(packageName, "description");
        if (description != "__error__")
          version = mPythonUtils->getPluginMetadata(packageName, "version");
      }
      
      if (pluginName == "__error__" || description == "__error__" || version == "__error__")
      {
        QMessageBox::warning(this, tr("Python error"), tr("Error when reading metadata of plugin ") + packageName);
        continue;
      }
      
      if (mySettings.value("/PythonPlugins/" + packageName).toBool())
      {
        loadPythonPlugin(packageName, pluginName);
      }
    }
  }
  
  QgsDebugMsg("Loading plugins completed");
  QgsDebugMsg("*************************************************\n\n");
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
  return longName + " (" + glob.toLower() + " " + glob.toUpper() + ");;";
}                               // createFileFilter_



/**
  Builds the list of file filter strings to later be used by
  QgisApp::addVectorLayer()

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

  // then iterate through all of the supported drivers, adding the
  // corresponding file filter

  OGRSFDriverH driver;          // current driver

  QString driverName;           // current driver name

  // Grind through all the drivers and their respective metadata.
  // We'll add a file filter for those drivers that have a file
  // extension defined for them; the others, welll, even though
  // theoreticaly we can open those files because there exists a
  // driver for them, the user will have to use the "All Files" to
  // open datasets with no explicitly defined file name extension.
  QgsDebugMsg("Driver count: " + QString::number(driverRegistrar->GetDriverCount()));

  for (int i = 0; i < OGRGetDriverCount(); ++i)
  {
    driver = OGRGetDriver(i);

    Q_CHECK_PTR(driver);

    if (!driver)
    {
      qWarning("unable to get driver %d", i);
      continue;
    }

    driverName = OGR_Dr_GetName(driver);

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
      QgsDebugMsg("unknown driver " + driverName );
    }

  }                           // each loaded GDAL driver

  std::cout << myFileFilters.toLocal8Bit().data() << std::endl;

  // can't forget the default case

  myFileFilters += "All files (*.*)";
  fileFilters = myFileFilters;

#endif // DEPRECATED

  fileFilters = QgsProviderRegistry::instance()->fileVectorFilters();
  QgsDebugMsg("Vector file filters: " + fileFilters);

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

  haveLastUsedFilter = settings.contains("/UI/" + filterName);
  QString lastUsedFilter = settings.value("/UI/" + filterName,
      QVariant(QString::null)).toString();

  QString lastUsedDir = settings.value("/UI/" + filterName + "Dir",".").toString();

  QString lastUsedEncoding = settings.value("/UI/encoding").toString();

  QgsDebugMsg("Opening file dialog with filters: " + filters);

  QgsEncodingFileDialog* openFileDialog = new QgsEncodingFileDialog(0,
      title, lastUsedDir, filters, lastUsedEncoding);

  // allow for selection of more than one file
  openFileDialog->setFileMode(QFileDialog::ExistingFiles);

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
    QString myPath = myFI.path();

    QgsDebugMsg("Writing last used dir: " + myPath);

    settings.setValue("/UI/" + filterName, openFileDialog->selectedFilter());
    settings.setValue("/UI/" + filterName + "Dir", myPath);
    settings.setValue("/UI/encoding", openFileDialog->encoding());
  }

  delete openFileDialog;
}   // openFilesRememberingFilter_


/**
  This method prompts the user for a list of vector filenames with a dialog.
  */
void QgisApp::addVectorLayer()
{
  if(mMapCanvas && mMapCanvas->isDrawing())
    {
      return;
    }
  mMapCanvas->freeze();

  QStringList selectedFiles;
  QgsDebugMsg("Vector file filters: " + mVectorFileFilter);

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

  addVectorLayers(selectedFiles, enc);
}
  

bool QgisApp::addVectorLayers(QStringList const & theLayerQStringList, const QString& enc)
{

  for ( QStringList::ConstIterator it = theLayerQStringList.begin();
        it != theLayerQStringList.end();
        ++it )
  {
    QFileInfo fi(*it);
    QString base = fi.completeBaseName();

    QgsDebugMsg("completeBaseName: "+base);

    // create the layer

    QgsVectorLayer *layer = new QgsVectorLayer(*it, base, "ogr");
    Q_CHECK_PTR( layer );

    if ( ! layer )
    {
      mMapCanvas->freeze(false);

// Let render() do its own cursor management
//      QApplication::restoreOverrideCursor();

      // XXX insert meaningful whine to the user here
      return false;
    }

    if (layer->isValid())
    {
      layer->setProviderEncoding(enc);

      // Register this layer with the layers registry
      QgsMapLayerRegistry::instance()->addMapLayer(layer);
      // notify the project we've made a change
      QgsProject::instance()->dirty(true);

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

  // update UI
  qApp->processEvents();
  
  // draw the map
  mMapCanvas->freeze(false);
  mMapCanvas->refresh();

// Let render() do its own cursor management
//  QApplication::restoreOverrideCursor();

  statusBar()->showMessage(mMapCanvas->extent().stringRep(2));

  return true;
} // QgisApp::addVectorLayer()



/** This helper checks to see whether the filename appears to be a valid vector file name */
bool QgisApp::isValidVectorFileName(QString theFileNameQString)
{
  return (theFileNameQString.toLower().endsWith(".shp"));
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
  if(mMapCanvas && mMapCanvas->isDrawing())
    {
      return;
    }

  // only supports postgis layers at present

  // show the postgis dialog

  QgsDbSourceSelect *dbs = new QgsDbSourceSelect(this);

  mMapCanvas->freeze();

  if (dbs->exec())
  {
// Let render() do its own cursor management
//    QApplication::setOverrideCursor(Qt::WaitCursor);


    // repaint the canvas if it was covered by the dialog

    // add files to the map canvas
    QStringList tables = dbs->selectedTables();

    QApplication::setOverrideCursor(Qt::WaitCursor);

    QString connInfo = dbs->connInfo();
    // for each selected table, connect to the database, parse the WKT geometry,
    // and build a canvasitem for it
    // readWKB(connInfo,tables);
    QStringList::Iterator it = tables.begin();
    while (it != tables.end())
    {

      // create the layer
      //qWarning("creating layer");
      QgsVectorLayer *layer = new QgsVectorLayer(connInfo + " table=" + *it, *it, "postgres");
      if (layer->isValid())
      {
        // register this layer with the central layers registry
        QgsMapLayerRegistry::instance()->addMapLayer(layer);
        // notify the project we've made a change
        QgsProject::instance()->dirty(true);
      }
      else
      {
        QgsDebugMsg( (*it) + " is an invalid layer - not loaded");
        QMessageBox::critical(this, tr("Invalid Layer"), tr("%1 is an invalid layer and cannot be loaded.").arg(*it));
        delete layer;
      }
      //qWarning("incrementing iterator");
      ++it;
    }

    QApplication::restoreOverrideCursor();

    statusBar()->showMessage(mMapCanvas->extent().stringRep(2));
  }

  delete dbs;
  
  // update UI
  qApp->processEvents();
  
  // draw the map
  mMapCanvas->freeze(false);
  mMapCanvas->refresh();

// Let render() do its own cursor management
//  QApplication::restoreOverrideCursor();

} // QgisApp::addDatabaseLayer()
#endif


void QgisApp::addWmsLayer()
{
  if(mMapCanvas && mMapCanvas->isDrawing())
    {
      return;
    }
  // Fudge for now
  QgsDebugMsg("about to addRasterLayer");

  QgsServerSourceSelect *wmss = new QgsServerSourceSelect(this);

  if (wmss->exec())
  {

    addRasterLayer(wmss->connInfo(),
        wmss->connName(),
        "wms",
        wmss->selectedLayers(),
        wmss->selectedStylesForSelectedLayers(),
        wmss->selectedImageEncoding(),
        wmss->selectedCrs());
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
    QgsDebugMsg("cannot find ``type'' attribute");

    return IS_BOGUS;
  }

  if ( "raster" == type )
  {
    QgsDebugMsg("is a raster");

    return IS_RASTER;
  }
  else if ( "vector" == type )
  {
    QgsDebugMsg("is a vector");

    return IS_VECTOR;
  }

  QgsDebugMsg("is unknown type " + type);

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
    QgsDebugMsg("cannot find datasource node");

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

        QgsDebugMsg("datasource is " + dataSource);

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
      QgsDebugMsg("unknown ``type'' attribute");
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

  QgsDebugMsg("datasource changed from " + dataSourceText.data() );

  dataSourceText.setData( dataSource );

  QgsDebugMsg("to " + dataSourceText.data() );
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
      QgsDebugMsg("unable to determine data type");
      return;
  }

  // Prepend the original data source base name to make it easier to pick it
  // out from a list of other files; however the appropriate filter strings
  // for the file type will also be added in case the file name itself has
  // changed, too.

  QString myFileFilters = originalDataSource.fileName() + ";;" + fileFilters;

  QStringList selectedFiles;
  QString     enc;
  QString     title( QObject::tr("Where is '") + originalDataSource.fileName() + "'? (" + QObject::tr("original location: ") + originalDataSource.absoluteFilePath() + ")");

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
      QgsDebugMsg("unable to re-read layer");
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
      QgsDebugMsg("layer is file based");
      findMissingFile_( fileFilters, layerNode );
      break;

    case IS_DATABASE:
      QgsDebugMsg("layer is database based");
      break;

    case IS_URL:
      QgsDebugMsg("layer is URL based");
      break;

    case IS_UNKNOWN:
      QgsDebugMsg("layer has an unkown type");
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

  for( list<QDomNode>::const_iterator i = layerNodes.begin();
      i != layerNodes.end();
      ++i )
  {
    findLayer_( fileFilters, *i );
  }

} // findLayers_



void QgisApp::fileExit()
{
  if(mMapCanvas && mMapCanvas->isDrawing())
    {
      return;
    }
  
  if (saveDirty())
  {
    removeAllLayers();
    qApp->exit(0);
  }
}



void QgisApp::fileNew()
{
  fileNew(TRUE); // prompts whether to save project
} // fileNew()


//as file new but accepts flags to indicate whether we should prompt to save
void QgisApp::fileNew(bool thePromptToSaveFlag)
{ 
  if(mMapCanvas && mMapCanvas->isDrawing())
    {
      return;
    }

  if (thePromptToSaveFlag)
  {
    if (!saveDirty())
    {
      return;
    }
  }
  
  QgsDebugMsg("erasing project");
  
  mMapCanvas->freeze(true);
  QgsMapLayerRegistry::instance()->removeAllMapLayers();
  mMapCanvas->clear();

  QgsProject* prj = QgsProject::instance();
  prj->title( QString::null );
  prj->filename( QString::null );
  prj->clearProperties(); // why carry over properties from previous projects?
  
  QSettings settings;
  
  //set the colour for selections
  //the default can be set in qgisoptions 
  //use project properties to override the colour on a per project basis
  int myRed = settings.value("/qgis/default_selection_color_red",255).toInt();
  int myGreen = settings.value("/qgis/default_selection_color_green",255).toInt();
  int myBlue = settings.value("/qgis/default_selection_color_blue",0).toInt();
  prj->writeEntry("Gui","/SelectionColorRedPart",myRed);
  prj->writeEntry("Gui","/SelectionColorGreenPart",myGreen);
  prj->writeEntry("Gui","/SelectionColorBluePart",myBlue); 
  QgsRenderer::setSelectionColor(QColor(myRed,myGreen,myBlue));

  //set the canvas to the default background colour
  //the default can be set in qgisoptions 
  //use project properties to override the colour on a per project basis
  myRed = settings.value("/qgis/default_canvas_color_red",255).toInt();
  myGreen = settings.value("/qgis/default_canvas_color_green",255).toInt();
  myBlue = settings.value("/qgis/default_canvas_color_blue",255).toInt();
  prj->writeEntry("Gui","/CanvasColorRedPart",myRed);
  prj->writeEntry("Gui","/CanvasColorGreenPart",myGreen);
  prj->writeEntry("Gui","/CanvasColorBluePart",myBlue);  
  mMapCanvas->setCanvasColor(QColor(myRed,myGreen,myBlue));
    
  prj->dirty(false);

  setTitleBarText_( *this );
    
  QgsDebugMsg("emiting new project signal");

  //note by Tim: I did some casual egrepping and this signal doesnt actually
  //seem to be connected to anything....why is it here? Just for future needs?
  //note by Martin: actually QgsComposer does use it
  emit newProject();

  mMapCanvas->freeze(false);
  mMapCanvas->refresh();
  
  mMapCanvas->mapRender()->setProjectionsEnabled(FALSE);
  
  // set the initial map tool
  mMapCanvas->setMapTool(mMapTools.mPan);
  mNonEditMapTool = mMapTools.mPan;  // signals are not yet setup to catch this
} // QgisApp::fileNew(bool thePromptToSaveFlag)


void QgisApp::newVectorLayer()
{

  if(mMapCanvas && mMapCanvas->isDrawing())
    {
      return;
    }

  QGis::WKBTYPE geometrytype;
  QString fileformat;

  QgsGeomTypeDialog geomDialog(this);
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

  haveLastUsedFilter = settings.contains("/UI/lastVectorFileFilter");
  QString lastUsedFilter = settings.value("/UI/lastVectorFileFilter",
    QVariant(QString::null)).toString();

  QString lastUsedDir = settings.value("/UI/lastVectorFileFilterDir",
    ".").toString();

  QString lastUsedEncoding = settings.value("/UI/encoding").toString();

  QgsDebugMsg("Saving vector file dialog without filters: ");

  QgsEncodingFileDialog* openFileDialog = new QgsEncodingFileDialog(this,
    tr("Save As"), lastUsedDir, "", lastUsedEncoding);

  // allow for selection of more than one file
  openFileDialog->setFileMode(QFileDialog::AnyFile);
  openFileDialog->setAcceptMode(QFileDialog::AcceptSave); 
  openFileDialog->setConfirmOverwrite( true ); 

  if (haveLastUsedFilter)       // set the filter to the last one used
  {
    openFileDialog->selectFilter(lastUsedFilter);
  }

  if (openFileDialog->exec() != QDialog::Accepted)
  {
    delete openFileDialog;
    return;
  }

  filename = openFileDialog->selectedFiles().first();
  enc = openFileDialog->encoding();

  // If the file exists, delete it otherwise we'll end up loading that
  // file, which can cause problems (e.g., if the file contains
  // linestrings, but we're wanting to create points, we'll end up
  // with a linestring file).
  QFile::remove(filename);

  settings.setValue("/UI/lastVectorFileFilter", openFileDialog->selectedFilter());

  settings.setValue("/UI/lastVectorFileFilterDir", openFileDialog->directory().absolutePath());
  settings.setValue("/UI/encoding", openFileDialog->encoding());

  delete openFileDialog;

  //try to create the new layer with OGRProvider instead of QgsVectorFileWriter
  QgsProviderRegistry * pReg = QgsProviderRegistry::instance();
  QString ogrlib = pReg->library("ogr");
  // load the data provider
  QLibrary* myLib = new QLibrary(ogrlib);
  bool loaded = myLib->load();
  if (loaded)
  {
    QgsDebugMsg("ogr provider loaded");

    typedef bool (*createEmptyDataSourceProc)(const QString&, const QString&, const QString&, QGis::WKBTYPE, \
      const std::list<std::pair<QString, QString> >&);
    createEmptyDataSourceProc createEmptyDataSource=(createEmptyDataSourceProc)myLib->resolve("createEmptyDataSource");
    if(createEmptyDataSource)
    {
#if 0
      if(geometrytype == QGis::WKBPoint)
      {
        createEmptyDataSource(filename,fileformat, enc, QGis::WKBPoint, attributes);
      }
      else if (geometrytype == QGis::WKBLineString)
      {
        createEmptyDataSource(filename,fileformat, enc, QGis::WKBLineString, attributes);
      }
      else if(geometrytype == QGis::WKBPolygon)
      {
        createEmptyDataSource(filename,fileformat, enc, QGis::WKBPolygon, attributes);
      }
#endif
      if(geometrytype != QGis::WKBUnknown)
      {
        createEmptyDataSource(filename,fileformat, enc, geometrytype, attributes);
      }
      else
      {
        QgsDebugMsg("geometry type not recognised");
        return;
      }
    }
    else
    {
      QgsDebugMsg("Resolving newEmptyDataSource(...) failed");
    }
  }

  //then add the layer to the view
  QStringList filenames;
  filenames.append(filename);
  addVectorLayers(filenames, enc);
}

void QgisApp::fileOpen()
{
  if(mMapCanvas && mMapCanvas->isDrawing())
    {
      return;
    }

  // possibly save any pending work before opening a new project
  if (saveDirty())
  {
    // Retrieve last used project dir from persistent settings
    QSettings settings;
    QString lastUsedDir = settings.value("/UI/lastProjectDir", ".").toString();

    QFileDialog * openFileDialog = new QFileDialog(this,
        tr("Choose a QGIS project file to open"),
        lastUsedDir, QObject::tr("QGis files (*.qgs)"));
    openFileDialog->setFileMode(QFileDialog::ExistingFile);


    QString fullPath;
    if (openFileDialog->exec() == QDialog::Accepted)
    {
      // Fix by Tim - getting the dirPath from the dialog
      // directly truncates the last node in the dir path.
      // This is a workaround for that
      fullPath = openFileDialog->selectedFiles().first();
      QFileInfo myFI(fullPath);
      QString myPath = myFI.path();
      // Persist last used project dir
      settings.setValue("/UI/lastProjectDir", myPath);
    }
    else 
    {
      // if they didn't select anything, just return
      delete openFileDialog;
      return;
    }

    delete openFileDialog;

    // clear out any stuff from previous project
    mMapCanvas->freeze(true);
    removeAllLayers();

    QgsProject::instance()->filename( fullPath );

    try 
    {
      if ( QgsProject::instance()->read() )
      {
        setTitleBarText_( *this );
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
          tr("") + "\n" + QString::fromLocal8Bit( e.what() ) );
      QgsDebugMsg( QString("%1 bad layers found").arg(e.layers().size()) );

      // attempt to find the new locations for missing layers
      // XXX vector file hard-coded -- but what if it's raster?
      findLayers_( mVectorFileFilter, e.layers() );
    }
    catch ( std::exception & e )
    {
      QMessageBox::critical(this, 
          tr("QGIS Project Read Error"), 
          tr("") + "\n" + QString::fromLocal8Bit( e.what() ) );
      QgsDebugMsg("BAD LAYERS FOUND");
    }

    mMapCanvas->freeze(false);
    mMapCanvas->refresh();
  }

} // QgisApp::fileOpen



/**
  adds a saved project to qgis, usually called on startup by specifying a
  project file on the command line
  */
bool QgisApp::addProject(QString projectFile)
{
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
      QgsDebugMsg("Canvas background color restored...");

      emit projectRead(); // let plug-ins know that we've read in a new
      // project so that they can check any project
      // specific plug-in state

      // add this to the list of recently used project files
      QSettings settings;
      saveRecentProjectPath(projectFile, settings);
    }
    else
    {
      mMapCanvas->freeze(false);
      mMapCanvas->refresh();
      return false;
    }
  }
  catch ( QgsProjectBadLayerException & e )
  {
    QgsDebugMsg( QString("%1 bad layers found").arg( e.layers().size() ) );

    if ( QMessageBox::Ok == QMessageBox::critical( this, 
          tr("QGIS Project Read Error"), 
          tr("") + "\n" + QString::fromLocal8Bit( e.what() ) + "\n" +
          tr("Try to find missing layers?"),
          QMessageBox::Ok | QMessageBox::Cancel ) )
    {
      QgsDebugMsg("want to find missing layers is true");

      // attempt to find the new locations for missing layers
      // XXX vector file hard-coded -- but what if it's raster?
      findLayers_( mVectorFileFilter, e.layers() );
    }

  }
  catch ( std::exception & e )
  {
    QgsDebugMsg("BAD LAYERS FOUND");

    QMessageBox::critical( this, 
        tr("Unable to open project"), QString::fromLocal8Bit( e.what() ) );

    mMapCanvas->freeze(false);
    mMapCanvas->refresh();
    return false;
  }

  mMapCanvas->freeze(false);
  mMapCanvas->refresh();
  return true;
} // QgisApp::addProject(QString projectFile)



bool QgisApp::fileSave()
{
  if(mMapCanvas && mMapCanvas->isDrawing())
    {
      return false;
    }

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
    QString lastUsedDir = settings.value("/UI/lastProjectDir", ".").toString();

    std::auto_ptr<QFileDialog> saveFileDialog( new QFileDialog(this,
        tr("Choose a QGIS project file"),
        lastUsedDir, QObject::tr("QGis files (*.qgs)")) );

    saveFileDialog->setFileMode(QFileDialog::AnyFile);
    saveFileDialog->setAcceptMode(QFileDialog::AcceptSave); 
    saveFileDialog->setConfirmOverwrite( true ); 

    if (saveFileDialog->exec() == QDialog::Accepted)
    {
      fullPath.setFile( saveFileDialog->selectedFiles().first() );
    }
    else 
    {
      // if they didn't select anything, just return
      // delete saveFileDialog; auto_ptr auto destroys
      return false;
    }

    // make sure we have the .qgs extension in the file name
    if( "qgs" != fullPath.suffix() )
    {
      QString newFilePath = fullPath.filePath() + ".qgs";
      fullPath.setFile( newFilePath );
    }


    QgsProject::instance()->filename( fullPath.filePath() );
  }

  try
  {
    if ( QgsProject::instance()->write() )
    {
      setTitleBarText_(*this); // update title bar
      statusBar()->showMessage(tr("Saved project to:") + " " + QgsProject::instance()->filename() );

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
    QMessageBox::critical( this,
        tr("Unable to save project ") + QgsProject::instance()->filename(),
        QString::fromLocal8Bit( e.what() ) );
  }
  return true;
} // QgisApp::fileSave



void QgisApp::fileSaveAs()
{
  if(mMapCanvas && mMapCanvas->isDrawing())
    {
      return;
    }

  // Retrieve last used project dir from persistent settings
  QSettings settings;
  QString lastUsedDir = settings.value("/UI/lastProjectDir", ".").toString();

  auto_ptr<QFileDialog> saveFileDialog( new QFileDialog(this,
      tr("Choose a filename to save the QGIS project file as"),
      lastUsedDir, QObject::tr("QGis files (*.qgs)")) );

  saveFileDialog->setFileMode(QFileDialog::AnyFile);

  saveFileDialog->setAcceptMode(QFileDialog::AcceptSave);

  saveFileDialog->setConfirmOverwrite( true );

  // if we don't have a filename, then obviously we need to get one; note
  // that the project file name is reset to null in fileNew()
  QFileInfo fullPath;

  if (saveFileDialog->exec() == QDialog::Accepted)
  {
    // Fix by Tim - getting the dirPath from the dialog
    // directly truncates the last node in the dir path.
    // This is a workaround for that
    fullPath.setFile(saveFileDialog->selectedFiles().first());
    QString myPath = fullPath.path();
    // Persist last used project dir
    settings.setValue("/UI/lastProjectDir", myPath);
  } 
  else
  {
    // if they didn't select anything, just return
    // delete saveFileDialog; auto_ptr auto deletes
    return;
  }

  // make sure the .qgs extension is included in the path name. if not, add it...
  if( "qgs" != fullPath.suffix() )
  {
    QString newFilePath = fullPath.filePath() + ".qgs";
    fullPath.setFile( newFilePath );
  }

  try
  {
    QgsProject::instance()->filename( fullPath.filePath() );

    if ( QgsProject::instance()->write() )
    {
      setTitleBarText_(*this); // update title bar
      statusBar()->showMessage(tr("Saved project to:") + " " + QgsProject::instance()->filename() );
      // add this to the list of recently used project files
      saveRecentProjectPath(fullPath.filePath(), settings);
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
        QString::fromLocal8Bit( e.what() ),
        QMessageBox::Ok,
        Qt::NoButton );
  }
} // QgisApp::fileSaveAs






// Open the project file corresponding to the
// path at the given index in mRecentProjectPaths
void QgisApp::openProject(QAction *action)
{

  // possibly save any pending work before opening a different project
  QString debugme;
  assert(action != NULL);

  debugme = action->text();

  if (saveDirty())
  {
    addProject(debugme);

  }
  //set the projections enabled icon in the status bar
  int myProjectionEnabledFlag =
    QgsProject::instance()->readNumEntry("SpatialRefSys","/ProjectionsEnabled",0);
  mMapCanvas->mapRender()->setProjectionsEnabled(myProjectionEnabledFlag);

} // QgisApp::openProject

/**
  Open the specified project file; prompt to save previous project if necessary.
  Used to process a commandline argument or OpenDocument AppleEvent.
  */
void QgisApp::openProject(const QString & fileName)
{
  // possibly save any pending work before opening a different project
  if (saveDirty())
  {
    try
    {
      if ( ! addProject(fileName) )
      {
        QgsDebugMsg("unable to load project " + fileName);
      }
      else
      {
      }
    }
    catch ( QgsIOException & io_exception )
    {
      Q_UNUSED(io_exception);
      QMessageBox::critical( this, 
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
  QgsMapLayer* ok = NULL;
  CPLPushErrorHandler(CPLQuietErrorHandler); 
  if (QgsRasterLayer::isValidRasterFileName(fileName))
    ok = addRasterLayer(fileName, false);
  else // nope - try to load it as a shape/ogr
    ok = addVectorLayer(fileName, fileName, "ogr");
  CPLPopErrorHandler();

  if (!ok)
  {
    // we have no idea what this file is...
    QgsDebugMsg("Unable to load " + fileName);
  }

  return ok;
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
  QgsDebugMsg("..............................");
  QgsDebugMsg("...........Printing...........");
  QgsDebugMsg("..............................");
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
  if(mMapCanvas && mMapCanvas->isDrawing())
    {
      return;
    }
  mComposer->open();
}

void QgisApp::saveMapAsImage()
{
  //create a map to hold the QImageIO names and the filter names
  //the QImageIO name must be passed to the mapcanvas saveas image function
  typedef QMap<QString, QString> FilterMap;
  FilterMap myFilterMap;

  //find out the last used filter
  QSettings myQSettings;  // where we keep last used filter in persistant state
  QString myLastUsedFilter = myQSettings.value("/UI/saveAsImageFilter").toString();
  QString myLastUsedDir = myQSettings.value("/UI/lastSaveAsImageDir",".").toString();


  // get a list of supported output image types
  int myCounterInt=0;
  QString myFilters;
  QList<QByteArray> formats = QPictureIO::outputFormats();
  // Workaround for a problem with Qt4 - calls to outputFormats tend
  // to return nothing :(
  if (formats.count() == 0)
  {
    formats.append("png");
    formats.append("jpg");
  }

  for ( ; myCounterInt < formats.count(); myCounterInt++ )
  {
    QString myFormat=QString(formats.at( myCounterInt ));
    QString myFilter = createFileFilter_(myFormat + " format", "*."+myFormat);
    myFilters += myFilter;
    myFilterMap[myFilter] = myFormat;
  }
#ifdef QGISDEBUG
  QgsDebugMsg("Available Filters Map: ");
  FilterMap::Iterator myIterator;
  for ( myIterator = myFilterMap.begin(); myIterator != myFilterMap.end(); ++myIterator )
  {
    QgsDebugMsg( myIterator.key() + "  :  " + myIterator.value());
  }
#endif

  //create a file dialog using the the filter list generated above
  std::auto_ptr < QFileDialog > myQFileDialog( new QFileDialog(this,
      tr("Choose a filename to save the map image as"),
      myLastUsedDir, myFilters) );

  // allow for selection of more than one file
  myQFileDialog->setFileMode(QFileDialog::AnyFile);

  myQFileDialog->setAcceptMode(QFileDialog::AcceptSave);

  myQFileDialog->setConfirmOverwrite( true );


  if (!myLastUsedFilter.isEmpty())       // set the filter to the last one used
  {
    myQFileDialog->selectFilter(myLastUsedFilter);
  }


  //prompt the user for a filename
  QString myOutputFileNameQString; // = myQFileDialog->getSaveFileName(); //delete this
  if (myQFileDialog->exec() == QDialog::Accepted)
  {
    myOutputFileNameQString = myQFileDialog->selectedFiles().first();
  }

  QString myFilterString = myQFileDialog->selectedFilter()+";;";
  QgsDebugMsg("Selected filter: " + myFilterString);
  QgsDebugMsg("Image type to be passed to mapcanvas: " + myFilterMap[myFilterString]);

  // Add the file type suffix to the filename if required
  if (!myOutputFileNameQString.endsWith(myFilterMap[myFilterString]))
  {
    myOutputFileNameQString += "." + myFilterMap[myFilterString];
  }

  myQSettings.setValue("/UI/lastSaveAsImageFilter" , myFilterString);
  myQSettings.setValue("/UI/lastSaveAsImageDir", myQFileDialog->directory().absolutePath());

  if ( myOutputFileNameQString !="")
  {

    //save the mapview to the selected file
    mMapCanvas->saveAsImage(myOutputFileNameQString,NULL,myFilterMap[myFilterString]);
    statusBar()->showMessage(tr("Saved map image to") + " " + myOutputFileNameQString);
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
  if(mMapLegend)
    {
      mMapLegend->setOverviewAllLayers(true);
    }
 
  // notify the project we've made a change
  QgsProject::instance()->dirty(true);
}

//reimplements method from base (gui) class
void QgisApp::removeAllFromOverview()
{
  if(mMapLegend)
    {
      mMapLegend->setOverviewAllLayers(false);
    }
 
  // notify the project we've made a change
  QgsProject::instance()->dirty(true);
}

void QgisApp::toggleFullScreen()
{
  if (true == mFullScreenMode)
  {
    showNormal();
    mFullScreenMode = false;
  }
  else
  {
    showFullScreen();
    mFullScreenMode = true;
  }
}

void QgisApp::stopRendering()
{
  if(mMapCanvas)
  {
    QgsMapRender* mypMapRender = mMapCanvas->mapRender();
    if(mypMapRender)
    {
      QgsRenderContext* mypRenderContext = mypMapRender->renderContext();
      if(mypRenderContext)
      {
        mypRenderContext->setRenderingStopped(true);
      }
    }
  }
}

QToolBar * QgisApp::fileToolBar()
{
  return mFileToolBar;
}

//reimplements method from base (gui) class
void QgisApp::hideAllLayers()
{
  QgsDebugMsg("hiding all layers!");

  legend()->selectAll(false);
}


// reimplements method from base (gui) class
void QgisApp::showAllLayers()
{
  QgsDebugMsg("Showing all layers!");

  legend()->selectAll(true);
}

// TODO: remove the method once the mapserver export plugin is complete and tested
/*
void QgisApp::exportMapServer()
{
  // check to see if there are any layers to export
  // Possibly we may reinstate this in the future if we provide 'active project' export again
  //if (mMapCanvas->layerCount() > 0)
  //{
    QString myMSExportPath = QgsApplication::msexportAppPath(); 
    QProcess *process = new QProcess;
#ifdef WIN32
    // quote the application path on windows
    myMSExportPath = QString("\"") + myMSExportPath + QString("\"");
#endif
    process->start(myMSExportPath);

    // Delete this object if the process terminates
    connect(process, SIGNAL(finished(int, QProcess::ExitStatus)), 
        SLOT(processExited()));

    // Delete the process if the application quits
    connect(qApp, SIGNAL(aboutToQuit()), process, SLOT(terminate()));

  //}
  //else
  //{
  //  QMessageBox::warning(this, tr("No Map Layers"),
  //      tr("No layers to export. You must add at least one layer to the map in order to export the view."));
  //}
}
*/



void QgisApp::zoomIn()
{
  QgsDebugMsg ("Setting map tool to zoomIn");
  
  mMapCanvas->setMapTool(mMapTools.mZoomIn);

  // notify the project we've made a change
  QgsProject::instance()->dirty(true);
}


void QgisApp::zoomOut()
{
  mMapCanvas->setMapTool(mMapTools.mZoomOut);

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
  mMapCanvas->setMapTool(mMapTools.mPan);
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
  mMapCanvas->setMapTool(mMapTools.mIdentify);
}

void QgisApp::measure()
{
  mMapCanvas->setMapTool(mMapTools.mMeasureDist);
}

void QgisApp::measureArea()
{
  mMapCanvas->setMapTool(mMapTools.mMeasureArea);
}



void QgisApp::attributeTable()
{
  mMapLegend->legendLayerAttributeTable();
}


void QgisApp::deleteSelected()
{
  QgsMapLayer *layer = mMapLegend->currentLayer();
  if(!layer)
  {
    QMessageBox::information(this, tr("No Layer Selected"),
                              tr("To delete features, you must select a vector layer in the legend"));
    return;
  }
    
  QgsVectorLayer* vlayer = dynamic_cast<QgsVectorLayer*>(layer);
  if(!vlayer)
  {
    QMessageBox::information(this, tr("No Vector Layer Selected"),
                              tr("Deleting features only works on vector layers"));
    return;
  }
      
  if(!(vlayer->getDataProvider()->capabilities() & QgsVectorDataProvider::DeleteFeatures))
  {
    QMessageBox::information(this, tr("Provider does not support deletion"), 
                            tr("Data provider does not support deleting features"));
    return;
  }

  if(!vlayer->isEditable())
  {
    QMessageBox::information(this, tr("Layer not editable"), 
                             tr("The current layer is not editable. Choose 'Start editing' in the digitizing toolbar."));
    return;
  }

  
  if(!vlayer->deleteSelectedFeatures())
  {
    QMessageBox::information(this, tr("Problem deleting features"),
        tr("A problem occured during deletion of features"));
  }

  // notify the project we've made a change
  QgsProject::instance()->dirty(true);
}

void QgisApp::moveFeature()
{
  mMapCanvas->setMapTool(mMapTools.mMoveFeature);
}

void QgisApp::splitFeatures()
{
  mMapCanvas->setMapTool(mMapTools.mSplitFeatures);
}

void QgisApp::capturePoint()
{
  if(mMapCanvas && mMapCanvas->isDrawing())
    {
      return;
    }
  
  // set current map tool to select
  mMapCanvas->setMapTool(mMapTools.mCapturePoint);
  
  // FIXME: is this still actual or something old that's not used anymore?
  //connect(t, SIGNAL(xyClickCoordinates(QgsPoint &)), this, SLOT(showCapturePointCoordinate(QgsPoint &)));
}

void QgisApp::captureLine()
{
  if(mMapCanvas && mMapCanvas->isDrawing())
    {
      return;
    }
  
  mMapCanvas->setMapTool(mMapTools.mCaptureLine);
}

void QgisApp::capturePolygon()
{
  if(mMapCanvas && mMapCanvas->isDrawing())
    {
      return;
    }
  mMapCanvas->setMapTool(mMapTools.mCapturePolygon);
}

void QgisApp::select()
{
  mMapCanvas->setMapTool(mMapTools.mSelect);
}


void QgisApp::addVertex()
{
  if(mMapCanvas && mMapCanvas->isDrawing())
    {
      return;
    }
  mMapCanvas->setMapTool(mMapTools.mVertexAdd);
  
}

void QgisApp::moveVertex()
{
  if(mMapCanvas && mMapCanvas->isDrawing())
    {
      return;
    }
  mMapCanvas->setMapTool(mMapTools.mVertexMove);
}

void QgisApp::addRing()
{
 if(mMapCanvas && mMapCanvas->isDrawing())
    {
      return;
    } 
  mMapCanvas->setMapTool(mMapTools.mAddRing);
}

void QgisApp::addIsland()
{
  if(mMapCanvas && mMapCanvas->isDrawing())
    {
      return;
    }
  mMapCanvas->setMapTool(mMapTools.mAddIsland);
}


void QgisApp::deleteVertex()
{
  if(mMapCanvas && mMapCanvas->isDrawing())
    {
      return;
    }
  mMapCanvas->setMapTool(mMapTools.mVertexDelete);
}


void QgisApp::editCut(QgsMapLayer * layerContainingSelection)
{
  if(mMapCanvas && mMapCanvas->isDrawing())
    {
      return;
    }

  QgsMapLayer * selectionLayer = (layerContainingSelection != 0) ?
                                 (layerContainingSelection) :
                                 (activeLayer());

  if (selectionLayer)
  {
    // Test for feature support in this layer
    QgsVectorLayer* selectionVectorLayer = dynamic_cast<QgsVectorLayer*>(selectionLayer);

    if (selectionVectorLayer != 0)
    {
      QgsFeatureList features = selectionVectorLayer->selectedFeatures();
      clipboard()->replaceWithCopyOf( selectionVectorLayer->getDataProvider()->fields(), features );
      selectionVectorLayer->deleteSelectedFeatures();
    }
  }
}


void QgisApp::editCopy(QgsMapLayer * layerContainingSelection)
{
  if(mMapCanvas && mMapCanvas->isDrawing())
    {
      return;
    }

  QgsMapLayer * selectionLayer = (layerContainingSelection != 0) ?
                                 (layerContainingSelection) :
                                 (activeLayer());

  if (selectionLayer)
  {
    // Test for feature support in this layer
    QgsVectorLayer* selectionVectorLayer = dynamic_cast<QgsVectorLayer*>(selectionLayer);

    if (selectionVectorLayer != 0)
    {
      QgsFeatureList features = selectionVectorLayer->selectedFeatures();
      clipboard()->replaceWithCopyOf( selectionVectorLayer->getDataProvider()->fields(), features );
    }
  }
}


void QgisApp::editPaste(QgsMapLayer * destinationLayer)
{
  if(mMapCanvas && mMapCanvas->isDrawing())
    {
      return;
    }

  QgsMapLayer * pasteLayer = (destinationLayer != 0) ?
                             (destinationLayer) :
                             (activeLayer());

  if (pasteLayer)
  {
    // Test for feature support in this layer
    QgsVectorLayer* pasteVectorLayer = dynamic_cast<QgsVectorLayer*>(pasteLayer);

    if (pasteVectorLayer != 0)
    {
      pasteVectorLayer->addFeatures( clipboard()->copyOf() );
      mMapCanvas->refresh();
    }
  }
}


void QgisApp::pasteTransformations()
{
  QgsPasteTransformations *pt = new QgsPasteTransformations();

  mMapCanvas->freeze();

  pt->exec();
}


void QgisApp::refreshMapCanvas()
{
  QgsDebugMsg("called.");

  mMapCanvas->refresh();
}

void QgisApp::toggleMapTips()
{
  mMapTipsVisible = !mMapTipsVisible;
  // if off, stop the timer
  if ( !mMapTipsVisible )
  {
    mpMapTipsTimer->stop();
  }
}

void QgisApp::toggleEditing()
{
  if(mMapCanvas && mMapCanvas->isDrawing())
    {
      mActionToggleEditing->setChecked(!mActionToggleEditing->isChecked());
      return;
    }

  QgsLegendLayerFile* currentLayerFile = mMapLegend->currentLayerFile();
  if(currentLayerFile)
    {
      currentLayerFile->toggleEditing();
      activateDeactivateLayerRelatedActions( mMapLegend->currentLayer() );
    }
  else
    {
      mActionToggleEditing->setChecked(false);
    }
}

void QgisApp::showMouseCoordinate(QgsPoint & p)
{
  mCoordsLabel->setText(p.stringRep(mMousePrecisionDecimalPlaces));
  // Set minimum necessary width
  if ( mCoordsLabel->width() > mCoordsLabel->minimumWidth() )
  {
    mCoordsLabel->setMinimumWidth(mCoordsLabel->width());
  }

  if ( mMapTipsVisible )
  {
    // store the point, we need it for when the maptips timer fires
    mLastMapPosition = p;

    // we use this slot to control the timer for maptips since it is fired each time
    // the mouse moves.
    if ( mMapCanvas->underMouse() )
    {
      // Clear the maptip (this is done conditionally)
      mpMaptip->clear ( mMapCanvas );
      // don't start the timer if the mouse is not over the map canvas
      mpMapTipsTimer->start();
      QgsDebugMsg("Started maptips timer");
    }
  }
}


void QgisApp::showScale(double theScale)
{
  if (theScale >= 1.0)
    mScaleEdit->setText("1:" + QString::number(theScale, 'f', 0));
  else if (theScale > 0.0)
    mScaleEdit->setText(QString::number(1.0/theScale, 'f', 0) + ":1");
  else
    mScaleEdit->setText(tr("Invalid scale"));

   // Set minimum necessary width
  if ( mScaleEdit->width() > mScaleEdit->minimumWidth() )
  {
    mScaleEdit->setMinimumWidth(mScaleEdit->width());
  }
}

void QgisApp::userScale()
{
  double currentScale = mMapCanvas->getScale();

  QStringList parts = mScaleEdit->text().split(':');
  if (parts.size() == 2)
  {
    bool leftOk, rightOk;
    double leftSide = parts.at(0).toDouble(&leftOk);
    double rightSide = parts.at(1).toDouble(&rightOk);
    if (leftSide > 0.0 && leftOk && rightOk)
    {
       double wantedScale = rightSide / leftSide;
       mMapCanvas->zoom(wantedScale/currentScale);
    }
  }
}
void QgisApp::testButton()
{
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




// toggle overview status
void QgisApp::inOverview()
{
  mMapLegend->legendLayerShowInOverview();
}

void QgisApp::removeLayer()
{
  QgsLegendLayerFile* currentLayerFile = mMapLegend->currentLayerFile();
  if(currentLayerFile && currentLayerFile->isEditing() )
  {
    currentLayerFile->toggleEditing();
  }
  mMapLegend->legendLayerRemove();
  // notify the project we've made a change
  QgsProject::instance()->dirty(true);
}


void QgisApp::removeAllLayers()
{
  QgsMapLayerRegistry::instance()->removeAllMapLayers();
  mMapCanvas->refresh();
  // notify the project we've made a change
  QgsProject::instance()->dirty(true);
} //remove all layers


void QgisApp::zoomToLayerExtent()
{
  mMapLegend->legendLayerZoom();
}

void QgisApp::showPluginManager()
{
  QgsPluginManager *pm = new QgsPluginManager(mPythonUtils, this);
  pm->resizeColumnsToContents(); 
  if (pm->exec())
  {
    // load selected plugins
    std::vector < QgsPluginItem > pi = pm->getSelectedPlugins();
    std::vector < QgsPluginItem >::iterator it = pi.begin();
    while (it != pi.end())
    {
      QgsPluginItem plugin = *it;
      if (plugin.isPython())
      {
        loadPythonPlugin(plugin.fullPath(), plugin.name());
      }
      else
      {
        loadPlugin(plugin.name(), plugin.description(), plugin.fullPath());
      }
      it++;
    }
  }
}

void QgisApp::loadPythonPlugin(QString packageName, QString pluginName)
{
  if (!mPythonUtils || !mPythonUtils->isEnabled())
  {
    QgsDebugMsg("Python is not enabled in QGIS.");
    return;
  }
  
  QgsDebugMsg("I should load python plugin: " + pluginName + " (package: " + packageName + ")");
  
  QgsPluginRegistry *pRegistry = QgsPluginRegistry::instance();
  
  // is loaded already?
  if (pRegistry->library(pluginName).isEmpty())
  {
    mPythonUtils->loadPlugin(packageName);
    mPythonUtils->startPlugin(packageName);
  
    // TODO: test success
  
    // add to plugin registry
    pRegistry->addPythonPlugin(packageName, pluginName);
  
    // add to settings
    QSettings settings;
    settings.setValue("/PythonPlugins/" + packageName, true);
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

    QgsDebugMsg("Library name is " + myLib->fileName());

    bool loaded = myLib->load();
    if (loaded)
    {
      QgsDebugMsg("Loaded test plugin library");
      QgsDebugMsg("Attempting to resolve the classFactory function");

      type_t *pType = (type_t *) myLib->resolve("type");

      switch (pType())
      {
        case QgisPlugin::RENDERER:
        case QgisPlugin::UI:
          {
            // UI only -- doesn't use mapcanvas
            create_ui *cf = (create_ui *) myLib->resolve("classFactory");
            if (cf)
            {
              QgisPlugin *pl = cf(mQgisInterface);
              if (pl)
              {
                pl->initGui();
                // add it to the plugin registry
                pRegistry->addPlugin(myLib->fileName(), name, pl);
                //add it to the qsettings file [ts]
                settings.setValue("/Plugins/" + name, true);
              }
              else
              {
                // something went wrong
                QMessageBox::warning(this, tr("Error Loading Plugin"), tr("There was an error loading %1."));
                //disable it to the qsettings file [ts]
                settings.setValue("/Plugins/" + name, false);
              }
            }
            else
            {
              QgsDebugMsg("Unable to find the class factory for " + theFullPathName);
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
                settings.setValue("/Plugins/" + name, true);

              }
              else
              {
                // something went wrong
                QMessageBox::warning(this, tr("Error Loading Plugin"), tr("There was an error loading %1."));
                //add it to the qsettings file [ts]
                settings.setValue("/Plugins/" + name, false);
              }
            }
            else
            {
              QgsDebugMsg("Unable to find the class factory for " + theFullPathName);
            }
          }
          break;
        default:
          // type is unknown
          QgsDebugMsg("Plugin " + theFullPathName + " did not return a valid type and cannot be loaded");
          break;
      }

      /*  }else{
          QgsDebugMsg("Unable to find the class factory for " + mFullPathName);
          } */

  }
  else
  {
    QgsDebugMsg("Failed to load " + theFullPathName);
  }
  delete myLib;
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
      QgsDebugMsg("Getting information for plugin: " + mlpDir[i]);
      QgsDebugMsg("Attempting to load the plugin using dlopen");

      //          void *handle = dlopen("../plugins/maplayer/" + mlpDir[i], RTLD_LAZY);
      void *handle = dlopen(("../plugins/maplayer/" + mlpDir[i]).toLocal8Bit().data(), RTLD_LAZY | RTLD_GLOBAL );
      if (!handle)
      {
        QgsDebugMsg( QString("Error in dlopen: %1").arg( dlerror() ));
      }
      else
      {
        QgsDebugMsg("dlopen succeeded");
        dlclose(handle);
      }

      QLibrary *myLib = new QLibrary("../plugins/maplayer/" + mlpDir[i]);
      QgsDebugMsg("Library name is " + myLib->fileName());

      bool loaded = myLib->load();
      if (loaded)
      {
        QgsDebugMsg("Loaded test plugin library");
        QgsDebugMsg("Attempting to resolve the classFactory function");

        create_it *cf = (create_it *) myLib->resolve("classFactory");

        if (cf)
        {
          QgsDebugMsg("Getting pointer to a MapLayerInterface object from the library");

          QgsMapLayerInterface *pl = cf();
          if (pl)
          {
            QgsDebugMsg("Instantiated the maplayer test plugin");

            // set the main window pointer for the plugin
            pl->setQgisMainWindow(this);

            //the call to getInt is deprecated and this line should be removed
            //QgsDebugMsg("getInt returned " + QString::number(pl->getInt()) + " from map layer plugin");

            // set up the gui
            pl->initGui();
          }
          else
          {
            QgsDebugMsg("Unable to instantiate the maplayer test plugin");
          }
        }
      }
      else
      {
        QgsDebugMsg("Failed to load " + mlpDir[i]);
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
        QgsDebugMsg("Getting information for plugin: " + pluginDir[i]);

        QLibrary *myLib = new QLibrary("../plugins/" + pluginDir[i]); //"/home/gsherman/development/qgis/plugins/" + pluginDir[i]);

        QgsDebugMsg("Library name is " + myLib->fileName());
        //QLibrary myLib("../plugins/" + pluginDir[i]);
        QgsDebugMsg("Attempting to load ../plugins/" + pluginDir[i]);

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
          QgsDebugMsg("Loaded test plugin library");
          QgsDebugMsg("Getting the name of the plugin");

          name_t *pName = (name_t *) myLib->resolve("name");
          if (pName)
          {
            QMessageBox::information(this, tr("Name"), tr("Plugin %1 is named %2").arg(pluginDir[i]).arg(pName()));
          }
          QgsDebugMsg("Attempting to resolve the classFactory function");

          create_t *cf = (create_t *) myLib->resolve("classFactory");

          if (cf)
          {
            QgsDebugMsg("Getting pointer to a QgisPlugin object from the library");

            QgisPlugin *pl = cf(mQgisInterface);

            QgsDebugMsg("Displaying name, version, and description");
            QgsDebugMsg("Plugin name: " + pl->name());
            QgsDebugMsg("Plugin version: " + pl->version());
            QgsDebugMsg("Plugin description: " + pl->description());

            QMessageBox::information(this, tr("Plugin Information"), tr("QGis loaded the following plugin:") +
                tr("Name: %1").arg(pl->name()) + "\n" + tr("Version: %1").arg(pl->version()) + "\n" +
                tr("Description: %1").arg(pl->description()));
            // unload the plugin (delete it)

            QgsDebugMsg("Attempting to resolve the unload function");

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
          QgsDebugMsg("Unable to load library");
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
  connect(mSocket, SIGNAL(error(QAbstractSocket::SocketError)), 
                   SLOT(socketError(QAbstractSocket::SocketError)));
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
  int pos = mVersionMessage.indexOf(contentFlag);
  if (pos > -1)
  {
    pos += contentFlag.length();
    /* std::cout << mVersionMessage << "\n ";
       std::cout << "Pos is " << pos <<"\n"; */
    mVersionMessage = mVersionMessage.mid(pos);
    QStringList parts = mVersionMessage.split("|", QString::SkipEmptyParts);
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
      QMessageBox::StandardButton result = QMessageBox::information(this,
          tr("QGIS Version Information"), versionInfo, QMessageBox::Ok |
          QMessageBox::Cancel);
      if (result == QMessageBox::Ok)
      {
        // show more info
        QgsMessageViewer *mv = new QgsMessageViewer(this);
        mv->setWindowTitle(tr("QGIS - Changes in SVN Since Last Release"));
        mv->setMessageAsHtml(parts[2]);
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
void QgisApp::socketError(QAbstractSocket::SocketError e)
{
  if (e == QAbstractSocket::RemoteHostClosedError)
    return;

  QApplication::restoreOverrideCursor();
  // get error type
  QString detail;
  switch (e)
  {
    case QAbstractSocket::ConnectionRefusedError:
      detail = tr("Connection refused - server may be down");
      break;
    case QAbstractSocket::HostNotFoundError:
      detail = tr("QGIS server was not found");
      break;
    case QAbstractSocket::NetworkError:
      detail = tr("Network error while communicating with server");
      break;
    default:
      detail = tr("Unknown network socket error");
      break;
  }

  // show version message from server
  QMessageBox::critical(this, tr("QGIS Version Information"), tr("Unable to communicate with QGIS Version server") + "\n" + detail);
}

void QgisApp::socketReadyRead()
{
  while (mSocket->bytesAvailable() > 0)
  {
    char *data = new char[mSocket->bytesAvailable() + 1];
    memset(data, '\0', mSocket->bytesAvailable() + 1);
    mSocket->read(data, mSocket->bytesAvailable());
    mVersionMessage += data;
    delete[]data;
  }

}
void QgisApp::options()
{
  if(mMapCanvas && mMapCanvas->isDrawing())
    {
      return;
    }

  QgsOptions *optionsDialog = new QgsOptions(this);
  if(optionsDialog->exec())
  {
    // set the theme if it changed
    setTheme(optionsDialog->theme());

    QSettings mySettings;
    mMapCanvas->enableAntiAliasing(mySettings.value("/qgis/enable_anti_aliasing").toBool());
    mMapCanvas->useQImageToRender(mySettings.value("/qgis/use_qimage_to_render").toBool());
  
    int action = mySettings.value("/qgis/wheel_action", 0).toInt();
    double zoomFactor = mySettings.value("/qgis/zoom_factor", 2).toDouble();
    mMapCanvas->setWheelAction((QgsMapCanvas::WheelAction) action, zoomFactor);

//    bool splitterRedraw = mySettings.value("/qgis/splitterRedraw", true).toBool();
//    canvasLegendSplit->setOpaqueResize(splitterRedraw);
//    legendOverviewSplit->setOpaqueResize(splitterRedraw);
    setupProxy();
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

void QgisApp::openURL(QString url, bool useQgisDocDirectory)
{
  // open help in user browser
  if (useQgisDocDirectory)
  {
    url = "file://" + QgsApplication::pkgDataPath() + "/doc/" + url;
  }
#ifdef Q_OS_MACX
  /* Use Mac OS X Launch Services which uses the user's default browser
   * and will just open a new window if that browser is already running.
   * QProcess creates a new browser process for each invocation and expects a
   * commandline application rather than a bundled application.
   */
  CFURLRef urlRef = CFURLCreateWithBytes(kCFAllocatorDefault,
      reinterpret_cast<const UInt8*>(url.toUtf8().data()), url.length(),
      kCFStringEncodingUTF8, NULL);
  OSStatus status = LSOpenCFURLRef(urlRef, NULL);
  status = 0; //avoid compiler warning
  CFRelease(urlRef);
#elif defined(WIN32)
  if(url.startsWith("file://", Qt::CaseInsensitive))
    ShellExecute(0, 0, url.mid(7).toLocal8Bit().constData(), 0, 0, SW_SHOWNORMAL);
  else
    QDesktopServices::openUrl(url);
#else
  QDesktopServices::openUrl(url);
#endif
}

/** Get a pointer to the currently selected map layer */
QgsMapLayer *QgisApp::activeLayer()
{
  return (mMapLegend->currentLayer());
}

/** Add a vector layer directly without prompting user for location
  The caller must provide information compatible with the provider plugin
  using the vectorLayerPath and baseName. The provider can use these
  parameters in any way necessary to initialize the layer. The baseName
  parameter is used in the Map Legend so it should be formed in a meaningful
  way.
  */
QgsVectorLayer* QgisApp::addVectorLayer(QString vectorLayerPath, QString baseName, QString providerKey)
{
  if(mMapCanvas && mMapCanvas->isDrawing())
    {
      return NULL;
    }

  mMapCanvas->freeze();

// Let render() do its own cursor management
//  QApplication::setOverrideCursor(Qt::WaitCursor);

  // create the layer
  QgsVectorLayer *layer;
  /* Eliminate the need to instantiate the layer based on provider type.
     The caller is responsible for cobbling together the needed information to
     open the layer
     */
  QgsDebugMsg( "Creating new vector layer using " + vectorLayerPath
             + " with baseName of " + baseName
             + " and providerKey of " + providerKey);

  layer = new QgsVectorLayer(vectorLayerPath, baseName, providerKey);

  if( layer && layer->isValid() )
  {
    // Register this layer with the layers registry
    QgsMapLayerRegistry::instance()->addMapLayer(layer);
    // notify the project we've made a change
    QgsProject::instance()->dirty(true);

    statusBar()->showMessage(mMapCanvas->extent().stringRep(2));

  }
  else
  {
    QMessageBox::critical(this,tr("Layer is not valid"),
        tr("The layer is not a valid layer and can not be added to the map"));
    
    delete layer;
    mMapCanvas->freeze(false);
    return NULL;
  }
  
  // update UI
  qApp->processEvents();
  
  // draw the map
  mMapCanvas->freeze(false);
  mMapCanvas->refresh();

// Let render() do its own cursor management
//  QApplication::restoreOverrideCursor();
  
  return layer;

} // QgisApp::addVectorLayer



void QgisApp::addMapLayer(QgsMapLayer *theMapLayer)
{
  mMapCanvas->freeze();

// Let render() do its own cursor management
//  QApplication::setOverrideCursor(Qt::WaitCursor);

  if(theMapLayer->isValid())
  {
    // Register this layer with the layers registry
    QgsMapLayerRegistry::instance()->addMapLayer(theMapLayer);
    // add it to the mapcanvas collection
    // not necessary since adding to registry adds to canvas mMapCanvas->addLayer(theMapLayer);

    statusBar()->showMessage(mMapCanvas->extent().stringRep(2));
    // notify the project we've made a change
    QgsProject::instance()->dirty(true);

  }
  else
  {
    QMessageBox::critical(this,tr("Layer is not valid"),
        tr("The layer is not a valid layer and can not be added to the map"));
  }
  
  // update UI
  qApp->processEvents();
  
  // draw the map
  mMapCanvas->freeze(false);
  mMapCanvas->refresh();

// Let render() do its own cursor management
//  QApplication::restoreOverrideCursor();

}

void QgisApp::setExtent(QgsRect theRect)
{
  mMapCanvas->setExtent(theRect);
}

/**
  Prompt and save if project has been modified.
  @return true if saved or discarded, false if cancelled
 */
bool QgisApp::saveDirty()
{
  QMessageBox::StandardButton answer(QMessageBox::Discard);
  mMapCanvas->freeze(true);

  QgsDebugMsg(QString("Layer count is %1").arg(mMapCanvas->layerCount()));
  QgsDebugMsg(QString("Project is %1dirty").arg( QgsProject::instance()->dirty() ? "" : "not "));
  QgsDebugMsg(QString("Map canvas is %1dirty").arg(mMapCanvas->isDirty() ? "" : "not "));

  QSettings settings;
  bool askThem = settings.value("qgis/askToSaveProjectChanges", true).toBool();

  if (askThem && (QgsProject::instance()->dirty() || (mMapCanvas->isDirty()) && mMapCanvas->layerCount() > 0))
  {
    // flag project as dirty since dirty state of canvas is reset if "dirty"
    // is based on a zoom or pan
    QgsProject::instance()->dirty( true );
    // old code: mProjectIsDirtyFlag = true;

    // prompt user to save
    answer = QMessageBox::information(this, tr("Save?"),
        tr("Do you want to save the current project?"),
        QMessageBox::Save | QMessageBox::Cancel | QMessageBox::Discard);
    if (QMessageBox::Save == answer)
    {
      if (!fileSave())
        answer = QMessageBox::Cancel;
    }
  }

  mMapCanvas->freeze(false);

  return (answer != QMessageBox::Cancel);

} // QgisApp::saveDirty()


void QgisApp::closeEvent(QCloseEvent* event)
{
  // We'll close in our own good time, thank you very much
  event->ignore();
  // Do the usual checks and ask if they want to save, etc
  fileExit();
}


void QgisApp::whatsThis()
{
  QWhatsThis::enterWhatsThisMode();
} // QgisApp::whatsThis()

QMenu* QgisApp::getPluginMenu(QString menuName)
{
  // This is going to record the menu item that the potentially new
  // menu item is going to be inserted before. A value of 0 will a new
  // menu item to be appended.
  QAction* before = 0;

  QList<QAction*> actions = mPluginMenu->actions();
  // Avoid 1 because the first item (number 0) is 'Plugin Manager',
  // which we  want to stay first. Search in reverse order as that
  // makes it easier to find out where which item a new menu item
  // should go before (since the insertMenu() function requires a
  // 'before' argument).
  for (unsigned int i = actions.count()-1; i > 0; --i)
  {
    if (actions.at(i)->text() == menuName)
    {
      return actions.at(i)->menu();
    }
    // Find out where to put the menu item, assuming that it is a new one
    //
    // This bit of code assumes that the menu items are already in
    // alphabetical order, which they will be if the menus are all
    // created using this function.
    if (menuName.localeAwareCompare(actions.at(i)->text()) <= 0)
      before = actions.at(i);
  }

  // It doesn't exist, so create 
  QMenu* menu = new QMenu(menuName, this);
  // Where to put it? - we worked that out above...
  mPluginMenu->insertMenu(before, menu);

  return menu;
}

void QgisApp::addPluginMenu(QString name, QAction* action)
{
  QMenu* menu = getPluginMenu(name);
  menu->addAction(action);
}

void QgisApp::removePluginMenu(QString name, QAction* action)
{
  QMenu* menu = getPluginMenu(name);
  menu->removeAction(action);
  if (menu->actions().count() == 0)
    {
      mPluginMenu->removeAction(menu->menuAction());
    }
}

int QgisApp::addPluginToolBarIcon (QAction * qAction)
{
  mPluginToolBar->addAction(qAction);
  return 0;
}
void QgisApp::removePluginToolBarIcon(QAction *qAction)
{
  mPluginToolBar->removeAction(qAction);
}

void QgisApp::destinationSrsChanged()
{
  // save this information to project
  long srsid = mMapCanvas->mapRender()->destinationSrs().srsid();
  QgsProject::instance()->writeEntry("SpatialRefSys", "/ProjectSRSID", (int)srsid);

}

void QgisApp::projectionsEnabled(bool theFlag)
{
  // save this information to project
  QgsProject::instance()->writeEntry("SpatialRefSys","/ProjectionsEnabled", (theFlag?1:0));

  // update icon
  if (theFlag)
  {
    mOnTheFlyProjectionStatusButton->setIcon(
        getThemeIcon("/mIconProjectionEnabled.png"));
  }
  else
  {
    mOnTheFlyProjectionStatusButton->setIcon(
        getThemeIcon("/mIconProjectionDisabled.png"));
  }
}
// slot to update the progress bar in the status bar
void QgisApp::showProgress(int theProgress, int theTotalSteps)
{
  QgsDebugMsg( QString("%1/%2").arg(theProgress).arg(theTotalSteps) );

  if (theProgress==theTotalSteps)
  {
    mProgressBar->reset();
    mProgressBar->hide();
   }
  else
  {
    //only call show if not already hidden to reduce flicker
    if (!mProgressBar->isVisible())
    {
      mProgressBar->show();
    }
    mProgressBar->setMaximum(theTotalSteps);
    mProgressBar->setValue(theProgress);
  }


}

void QgisApp::mapToolChanged(QgsMapTool *tool)
{
  if( tool && !tool->isEditTool() )
  {
    mNonEditMapTool = tool;
  }
}

void QgisApp::showExtents()
{
  // update the statusbar with the current extents.
  QgsRect myExtents = mMapCanvas->extent();
  statusBar()->showMessage(QString(tr("Extents: ")) + myExtents.stringRep(true));

} // QgisApp::showExtents


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
  //QgsDebugMsg("message '" + theMessage + "'.");

  statusBar()->showMessage(theMessage);
}

void QgisApp::showMapTip()

{
  /* Show the maptip using tooltip */
  // Stop the timer while we look for a maptip
  mpMapTipsTimer->stop();

  // Only show tooltip if the mouse is over the canvas
  if ( mMapCanvas->underMouse() )
  {
    QPoint myPointerPos = mMapCanvas->mouseLastXY();

    // Following is debug stuff
    QgsDebugMsg ( "Mouse IS over canvas" );
    QgsDebugMsg ( "Maptips timer fired:" );
    QgsDebugMsg ( mLastMapPosition.stringRep() );
    QgsDebugMsg ( "Pixel coordinates of mouse position:" );
    QgsDebugMsg ( QString::number ( myPointerPos.x() ) + "," + QString::number ( myPointerPos.y() ) );
    // end debug stuff

    //  Make sure there is an active layer before proceeding

    QgsMapLayer* mypLayer = mMapCanvas->currentLayer();
    if ( mypLayer )
    {
      QgsDebugMsg("Current layer for maptip display is: " + mypLayer->source());
      // only process vector layers
      if ( mypLayer->type() == QgsMapLayer::VECTOR )
      {
        // Show the maptip if the maptips button is depressed
        if(mMapTipsVisible)
        {
          mpMaptip->showMapTip ( mypLayer, mLastMapPosition, myPointerPos, mMapCanvas );
        }
      }
    }
    else
    {
      QgsDebugMsg ( "Maptips require an active layer" );
    }
  }
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
  if(mMapCanvas && mMapCanvas->isDrawing())
    {
      return;
    }

  /* Display the property sheet for the Project */
  // set wait cursor since construction of the project properties
  // dialog results in the construction of the spatial reference
  // system QMap
  QApplication::setOverrideCursor(Qt::WaitCursor);
  QgsProjectProperties *pp = new QgsProjectProperties(mMapCanvas, this);
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
  QApplication::restoreOverrideCursor();

  //pass any refresg signals off to canvases
  //connect (pp,SIGNAL(refresh()), mMapCanvas, SLOT(refresh()));

  QgsMapRender* myRender = mMapCanvas->mapRender();
  bool wasProjected = myRender->projectionsEnabled();
  long oldSRSID = myRender->destinationSrs().srsid();

  // Display the modal dialog box.
  pp->exec();

  long newSRSID = myRender->destinationSrs().srsid();
  bool isProjected = myRender->projectionsEnabled();
  
  // projections have been turned on/off or dest SRS has changed while projections are on
  if (wasProjected != isProjected || (isProjected && oldSRSID != newSRSID))
  {
    // TODO: would be better to try to reproject current extent to the new one
    mMapCanvas->updateFullExtent();
  }

  int  myRedInt = QgsProject::instance()->readNumEntry("Gui","/CanvasColorRedPart",255);
  int  myGreenInt = QgsProject::instance()->readNumEntry("Gui","/CanvasColorGreenPart",255);
  int  myBlueInt = QgsProject::instance()->readNumEntry("Gui","/CanvasColorBluePart",255);
  QColor myColor = QColor(myRedInt,myGreenInt,myBlueInt);
  mMapCanvas->setCanvasColor(myColor); // this is fill colour before rendering onto canvas
  
  // Set the window title.
  setTitleBarText_( *this );
  
  // delete the property sheet object
  delete pp;
} // QgisApp::projectProperties


QgsClipboard * QgisApp::clipboard()
{
  return mInternalClipboard;
}

void QgisApp::activateDeactivateLayerRelatedActions(QgsMapLayer* layer)
{
  if(!layer)
  {
    mActionToggleEditing->setEnabled(false);
    mActionRemoveLayer->setEnabled(false);
    mActionInOverview->setEnabled(false);
    mActionEditCopy->setEnabled(false);
    return;
  }

  mActionToggleEditing->setEnabled(true);
  mActionRemoveLayer->setEnabled(true);
  mActionInOverview->setEnabled(true);

  /***********Vector layers****************/
  if(layer->type() == QgsMapLayer::VECTOR)
  {
    mActionSelect->setEnabled(true);
    mActionOpenTable->setEnabled(true);
    mActionIdentify->setEnabled(true);
    mActionEditCopy->setEnabled(true);

    const QgsVectorLayer* vlayer = dynamic_cast<const QgsVectorLayer*>(layer);
    const QgsVectorDataProvider* dprovider = vlayer->getDataProvider();

    if( !vlayer->isEditable() && mMapCanvas->mapTool() && mMapCanvas->mapTool()->isEditTool() )
    {
      mMapCanvas->setMapTool(mNonEditMapTool);
    }

    if (dprovider)
    {
      //start editing/stop editing
      if(dprovider->capabilities() & QgsVectorDataProvider::AddFeatures)
      {
        mActionToggleEditing->setEnabled(true);
        mActionToggleEditing->setChecked(vlayer->isEditable());
        mActionEditPaste->setEnabled(vlayer->isEditable());
      }
      else
      {
        mActionToggleEditing->setEnabled(false);
        mActionEditPaste->setEnabled(false);
      }

      //does provider allow deleting of features?
      if(vlayer->isEditable() && dprovider->capabilities() & QgsVectorDataProvider::DeleteFeatures)
      {
        mActionDeleteSelected->setEnabled(true);
        mActionEditCut->setEnabled(true);
      }
      else
      {
        mActionDeleteSelected->setEnabled(false);
        mActionEditCut->setEnabled(false);
      }


      if(vlayer->vectorType() == QGis::Point)
      {
        if(vlayer->isEditable() && dprovider->capabilities() & QgsVectorDataProvider::AddFeatures)
        {
          mActionCapturePoint->setEnabled(true);
          mActionMoveFeature->setEnabled(true);
        }
        else
        {
          mActionCapturePoint->setEnabled(false);
          mActionMoveFeature->setEnabled(false);
        }
        mActionCaptureLine->setEnabled(false);
        mActionCapturePolygon->setEnabled(false);
        mActionAddVertex->setEnabled(false);
        mActionDeleteVertex->setEnabled(false);
        mActionMoveVertex->setEnabled(false);
        mActionAddRing->setEnabled(false);
        mActionAddIsland->setEnabled(false);
        mActionSplitFeatures->setEnabled(false);
        if(vlayer->isEditable() && dprovider->capabilities() & QgsVectorDataProvider::ChangeGeometries)
        {
          //don't enable vertex move for single point
          if(vlayer->geometryType() != QGis::WKBPoint && vlayer->geometryType() != QGis::WKBPoint25D)
          {
            mActionMoveVertex->setEnabled(true);
          }
          mActionMoveFeature->setEnabled(true);
        }
        return;
      }
      else if(vlayer->vectorType() == QGis::Line)
      {
        if(vlayer->isEditable() && dprovider->capabilities() & QgsVectorDataProvider::AddFeatures)
        {
          mActionCaptureLine->setEnabled(true);
          mActionSplitFeatures->setEnabled(true);
          mActionMoveFeature->setEnabled(true);
        }
        else
        {
          mActionCaptureLine->setEnabled(false);
          mActionSplitFeatures->setEnabled(false);
          mActionMoveFeature->setEnabled(false);
        }
        mActionCapturePoint->setEnabled(false);
        mActionCapturePolygon->setEnabled(false);
        mActionAddRing->setEnabled(false);
        mActionAddIsland->setEnabled(false);
      }
      else if(vlayer->vectorType() == QGis::Polygon)
      {
        if(vlayer->isEditable() && dprovider->capabilities() & QgsVectorDataProvider::AddFeatures)
        {
          mActionCapturePolygon->setEnabled(true);
          mActionAddRing->setEnabled(true);
          mActionAddIsland->setEnabled(true);
          mActionSplitFeatures->setEnabled(true);
          mActionMoveFeature->setEnabled(true);
        }
        else
        {
          mActionCapturePolygon->setEnabled(false);
          mActionAddRing->setEnabled(false);
          mActionAddIsland->setEnabled(false);
          mActionSplitFeatures->setEnabled(false);
          mActionMoveFeature->setEnabled(false);
        }
        mActionCapturePoint->setEnabled(false);
        mActionCaptureLine->setEnabled(false);
      }

      //are add/delete/move vertex supported?
      if(vlayer->isEditable() && dprovider->capabilities() & QgsVectorDataProvider::ChangeGeometries)
      {
        mActionAddVertex->setEnabled(true);
        mActionMoveVertex->setEnabled(true);
        mActionDeleteVertex->setEnabled(true);
        mActionMoveFeature->setEnabled(true);
        if(vlayer->vectorType() == QGis::Polygon)
        {
          mActionAddRing->setEnabled(true);
          //some polygon layers contain also multipolygon features. 
          //Therefore, the test for multipolygon is done in QgsGeometry
          mActionAddIsland->setEnabled(true);
        }
      }
      else
      {
        mActionAddVertex->setEnabled(false);
        mActionMoveVertex->setEnabled(false);
        mActionDeleteVertex->setEnabled(false);
        mActionMoveFeature->setEnabled(false);
      }
      return;
    }
  }
  /*************Raster layers*************/
  else if(layer->type() == QgsMapLayer::RASTER)
  {
    mActionSelect->setEnabled(false);
    mActionOpenTable->setEnabled(false);
    mActionToggleEditing->setEnabled(false);
    mActionCapturePoint->setEnabled(false);
    mActionCaptureLine->setEnabled(false);
    mActionCapturePolygon->setEnabled(false);
    mActionDeleteSelected->setEnabled(false);
    mActionAddRing->setEnabled(false);
    mActionAddIsland->setEnabled(false);
    mActionAddVertex->setEnabled(false);
    mActionDeleteVertex->setEnabled(false);
    mActionMoveVertex->setEnabled(false);
    mActionMoveFeature->setEnabled(false);
    mActionEditCopy->setEnabled(false);
    mActionEditCut->setEnabled(false);
    mActionEditPaste->setEnabled(false);

    const QgsRasterLayer* vlayer = dynamic_cast<const QgsRasterLayer*> (layer);
    const QgsRasterDataProvider* dprovider = vlayer->getDataProvider();
    if (dprovider)
    {
      // does provider allow the identify map tool?
      if (dprovider->capabilities() & QgsRasterDataProvider::Identify)
      {
        mActionIdentify->setEnabled(TRUE);
      }
      else
      {
        mActionIdentify->setEnabled(FALSE);
      }
    }
  }
}


//copy the click coord to clipboard and let the user know its there
void QgisApp::showCapturePointCoordinate(QgsPoint & theQgsPoint)
{
  QgsDebugMsg("Capture point (clicked on map) at position " + theQgsPoint.stringRep(2));

  QClipboard *myClipboard = QApplication::clipboard();
  //if we are on x11 system put text into selection ready for middle button pasting
  if (myClipboard->supportsSelection())
  {
    myClipboard->setText(theQgsPoint.stringRep(2),QClipboard::Selection);
    QString myMessage = tr("Clipboard contents set to: ");
    statusBar()->showMessage(myMessage + myClipboard->text(QClipboard::Selection));
  }
  else
  {
    //user has an inferior operating system....
    myClipboard->setText(theQgsPoint.stringRep(2),QClipboard::Clipboard );
    QString myMessage = tr("Clipboard contents set to: ");
    statusBar()->showMessage(myMessage + myClipboard->text(QClipboard::Clipboard));
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


// this is a slot for action from GUI to add raster layer
void QgisApp::addRasterLayer()
{
  if(mMapCanvas && mMapCanvas->isDrawing())
    {
      return;
    }
  
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

  addRasterLayers(selectedFiles);

}// QgisApp::addRasterLayer()

//
// This is the method that does the actual work of adding a raster layer - the others
// here simply create a raster layer object and delegate here. It is the responsibility
// of the calling method to manage things such as the frozen state of the mapcanvas and
// using waitcursors etc. - this method wont and SHOULDNT do it
//
bool QgisApp::addRasterLayer(QgsRasterLayer * theRasterLayer)
{
  if(mMapCanvas && mMapCanvas->isDrawing())
    {
      return false;
    }

  Q_CHECK_PTR( theRasterLayer );

  if ( ! theRasterLayer )
  {
    // XXX insert meaningful whine to the user here; although be
    // XXX mindful that a null layer may mean exhausted memory resources
    return false;
  }

  if (!theRasterLayer->isValid())
  {
    delete theRasterLayer;
    return false;
  }
  
  // register this layer with the central layers registry
  QgsMapLayerRegistry::instance()->addMapLayer(theRasterLayer);

  // connect up any request the raster may make to update the app progress
  QObject::connect(theRasterLayer,
      SIGNAL(drawingProgress(int,int)),
      this,
      SLOT(showProgress(int,int)));
  // connect up any request the raster may make to update the statusbar message
  QObject::connect(theRasterLayer,
      SIGNAL(setStatus(QString)),
      this,
      SLOT(showStatusMessage(QString)));
  // notify the project we've made a change
  QgsProject::instance()->dirty(true);

  return true;
}


//create a raster layer object and delegate to addRasterLayer(QgsRasterLayer *)

QgsRasterLayer* QgisApp::addRasterLayer(QString const & rasterFile, QString const & baseName, bool guiWarning)
{
  if(mMapCanvas && mMapCanvas->isDrawing())
    {
      return NULL;
    }

  // let the user know we're going to possibly be taking a while
  QApplication::setOverrideCursor(Qt::WaitCursor);

  mMapCanvas->freeze(true);
  
  // XXX ya know QgsRasterLayer can snip out the basename on its own;
  // XXX why do we have to pass it in for it?
  QgsRasterLayer *layer =
    new QgsRasterLayer(rasterFile, baseName); // fi.completeBaseName());

  if (!addRasterLayer(layer))
  {
    mMapCanvas->freeze(false);
    QApplication::restoreOverrideCursor();

// Let render() do its own cursor management
//    QApplication::restoreOverrideCursor();

    if(guiWarning)
    {
      // don't show the gui warning (probably because we are loading from command line)
      QString msg(rasterFile
          + tr(" is not a valid or recognized raster data source"));
      QMessageBox::critical(this, tr("Invalid Data Source"), msg);
    }
    return NULL;
  }
  else
  {
    statusBar()->showMessage(mMapCanvas->extent().stringRep(2));
    mMapCanvas->freeze(false);
    QApplication::restoreOverrideCursor();

// Let render() do its own cursor management
//    QApplication::restoreOverrideCursor();

    mMapCanvas->refresh();

    return layer;
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
QgsRasterLayer* QgisApp::addRasterLayer(QString const & rasterLayerPath,
    QString const & baseName,
    QString const & providerKey,
    QStringList const & layers,
    QStringList const & styles,
    QString const & format,
    QString const & crs)
{
  QgsDebugMsg("about to get library for " + providerKey);

  if(mMapCanvas && mMapCanvas->isDrawing())
    {
      return 0;
    }

  mMapCanvas->freeze();

// Let render() do its own cursor management
//  QApplication::setOverrideCursor(Qt::WaitCursor);

  // create the layer
  QgsRasterLayer *layer;
  /* Eliminate the need to instantiate the layer based on provider type.
     The caller is responsible for cobbling together the needed information to
     open the layer
     */
  QgsDebugMsg( "Creating new raster layer using " + rasterLayerPath
             + " with baseName of " + baseName
             + " and layer list of " + layers.join(", ")
             + " and style list of " + styles.join(", ")
             + " and format of " + format
             + " and providerKey of " + providerKey
             + " and CRS of " + crs );

  // TODO: Remove the 0 when the raster layer becomes a full provider gateway.
  layer = new QgsRasterLayer(0, rasterLayerPath, baseName, providerKey, layers, styles, format, crs);

  QgsDebugMsg("Constructed new layer.");

  if( layer && layer->isValid() )
  {
    addRasterLayer(layer);
    
    statusBar()->showMessage(mMapCanvas->extent().stringRep(2));

  }
  else
  {
    QMessageBox::critical(this,tr("Layer is not valid"),
        tr("The layer is not a valid layer and can not be added to the map"));
  }

  // update UI
  qApp->processEvents();
  // draw the map
  mMapCanvas->freeze(false);
  mMapCanvas->refresh();
  return layer;

// Let render() do its own cursor management
//  QApplication::restoreOverrideCursor();

} // QgisApp::addRasterLayer



//create a raster layer object and delegate to addRasterLayer(QgsRasterLayer *)
bool QgisApp::addRasterLayers(QStringList const &theFileNameQStringList, bool guiWarning)
{
  if(mMapCanvas && mMapCanvas->isDrawing())
    {
      return false;
    }

  if (theFileNameQStringList.empty())
  {
    // no files selected so bail out, but
    // allow mMapCanvas to handle events
    // first
    mMapCanvas->freeze(false);
    return false;
  }

  mMapCanvas->freeze(true);

// Let render() do its own cursor management
//  QApplication::setOverrideCursor(Qt::WaitCursor);

  // this is messy since some files in the list may be rasters and others may
  // be ogr layers. We'll set returnValue to false if one or more layers fail
  // to load.
  bool returnValue = true;
  for (QStringList::ConstIterator myIterator = theFileNameQStringList.begin();
      myIterator != theFileNameQStringList.end();
      ++myIterator)
  {
    QString errMsg;

    if (QgsRasterLayer::isValidRasterFileName(*myIterator,errMsg))
    {
      QFileInfo myFileInfo(*myIterator);
      // get the directory the .adf file was in
      QString myDirNameQString = myFileInfo.path();
      QString myBaseNameQString = myFileInfo.completeBaseName();
      //only allow one copy of a ai grid file to be loaded at a
      //time to prevent the user selecting all adfs in 1 dir which
      //actually represent 1 coverage,

      // create the layer
      QgsRasterLayer *layer = new QgsRasterLayer(*myIterator, myBaseNameQString);

      addRasterLayer(layer);

      //only allow one copy of a ai grid file to be loaded at a
      //time to prevent the user selecting all adfs in 1 dir which
      //actually represent 1 coverate,

      if (myBaseNameQString.toLower().endsWith(".adf"))
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
        
        if( errMsg.size() > 0 )
            msg += "\n" + errMsg;

        QMessageBox::critical(this, tr("Unsupported Data Source"), msg);
      }
      returnValue = false;
    }
  }

  statusBar()->showMessage(mMapCanvas->extent().stringRep(2));
  mMapCanvas->freeze(false);
  mMapCanvas->refresh();

// Let render() do its own cursor management
//  QApplication::restoreOverrideCursor();

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

  //cancel rendering progress with esc key
  if(e->key() == Qt::Key_Escape)
  {
    stopRendering();
  }
  else
  {
    e->ignore();
  }
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
  if(mMapCanvas && mMapCanvas->isDrawing())
    {
      return;
    }

  // Create an instance of the Custom Projection Designer modeless dialog.
  // Autodelete the dialog when closing since a pointer is not retained.
  QgsCustomProjectionDialog * myDialog = new QgsCustomProjectionDialog(this);
  myDialog->setAttribute(Qt::WA_DeleteOnClose);
  myDialog->show();
}
void QgisApp::showBookmarks()
{
  // Create or show the single instance of the Bookmarks modeless dialog.
  // Closing a QWidget only hides it so it can be shown again later.
  static QgsBookmarks *bookmarks = NULL;
  if (bookmarks == NULL)
  {
    bookmarks = new QgsBookmarks(this, Qt::WindowMinMaxButtonsHint);
  }
  bookmarks->restorePosition();
  bookmarks->show();
  bookmarks->raise();
  bookmarks->activateWindow();
}

void QgisApp::newBookmark()
{
  // Get the name for the bookmark. Everything else we fetch from
  // the mapcanvas

  bool ok;
  QString bookmarkName = QInputDialog::getText(this, tr("New Bookmark"), 
      tr("Enter a name for the new bookmark:"), QLineEdit::Normal,
      QString::null, &ok);
  if( ok && !bookmarkName.isEmpty())
  {
    if (createDB())
    {
      // create the bookmark
      QgsBookmarkItem *bmi = new QgsBookmarkItem(bookmarkName, 
          QgsProject::instance()->title(), mMapCanvas->extent(), -1,
          QgsApplication::qgisUserDbFilePath());
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

void QgisApp::showAllToolbars()
{
  setToolbarVisibility(true);
}

void QgisApp::hideAllToolbars()
{
  setToolbarVisibility(false);
}

void QgisApp::setToolbarVisibility(bool visibility)
{
  mFileToolBar->setVisible(visibility);
  mLayerToolBar->setVisible(visibility);
  mMapNavToolBar->setVisible(visibility);
  mDigitizeToolBar->setVisible(visibility);
  mAttributesToolBar->setVisible(visibility);
  mPluginToolBar->setVisible(visibility);
  mHelpToolBar->setVisible(visibility);
}

// Slot that gets called when the project file was saved with an older
// version of QGIS

void QgisApp::warnOlderProjectVersion(QString oldVersion)
{
  QSettings settings;

  if ( settings.value("/qgis/warnOldProjectVersion", QVariant(true)).toBool() )
  {
    QMessageBox::warning(NULL, tr("Project file is older"),
                         (tr("<p>This project file was saved by an older version of QGIS.") +
                          tr(" When saving this project file, QGIS will update it to the latest version, "
                             "possibly rendering it useless for older versions of QGIS.") +
                          tr("<p>Even though QGIS developers try to maintain backwards "
                             "compatibility, some of the information from the old project "
                             "file might be lost.") +
                          tr(" To improve the quality of QGIS, we appreciate "
                             "if you file a bug report at %3.") +
                          tr(" Be sure to include the old project file, and state the version of "
                             "QGIS you used to discover the error.") +
                          tr("<p>To remove this warning when opening an older project file, "
                             "uncheck the box '%5' in the %4 menu.") +
                          tr("<p>Version of the project file: %1<br>Current version of QGIS: %2"))
                         .arg(oldVersion)
                         .arg(QGis::qgisVersion)
                         .arg("<a href=https://svn.qgis.org/trac/wiki>http://svn.qgis.org/trac/wiki</a> ")
                         .arg(tr("<tt>Settings:Options:General</tt>", "Menu path to setting options"))
                         .arg(tr("Warn me when opening a project file saved with an older version of QGIS"))
      );
    
  }  
  return;
}

void QgisApp::setupProxy()
{
  QSettings mySettings;
  bool myFlag = mySettings.value("proxy/proxyEnabled", "0").toBool();
  QNetworkProxy myProxy;
  if (myFlag)
  {
    myProxy.setType(QNetworkProxy::HttpProxy);
    myProxy.setHostName(mySettings.value("proxy/proxyHost", "").toString());
    myProxy.setPort(mySettings.value("proxy/proxyPort", "").toInt());
    myProxy.setUser(mySettings.value("proxy/proxyUser", "").toString());
    myProxy.setPassword(mySettings.value("proxy/proxyPassword", "").toString());
  }
  else
  {
    // otherwise leave it blank to disable proxy usage
  }
  QNetworkProxy::setApplicationProxy(myProxy);
}

QIcon QgisApp::getThemeIcon(const QString theName)
{
  if (QFile::exists(QgsApplication::activeThemePath() + theName))
  {
    return QIcon(QgsApplication::activeThemePath() + theName);
  }
  else if (QFile::exists(QgsApplication::defaultThemePath() + theName))
  {
    //could still return an empty icon if it
    //doesnt exist in the default theme either!
    return QIcon(QgsApplication::defaultThemePath() + theName);
  }
  else
  {
    return QIcon();
  }
}

QPixmap QgisApp::getThemePixmap(const QString theName)
{
  if (QFile::exists(QgsApplication::activeThemePath() + theName))
  {
    return QPixmap(QgsApplication::activeThemePath() + theName);
  }
  else
  {
    //could still return an empty icon if it
    //doesnt exist in the default theme either!
    return QPixmap(QgsApplication::defaultThemePath() + theName);
  }
}

