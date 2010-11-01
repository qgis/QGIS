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
#include <QImageWriter>
#include <QInputDialog>
#include <QKeyEvent>
#include <QLabel>
#include <QLibrary>
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
#include <QProgressDialog>
#include <QRegExp>
#include <QRegExpValidator>
#include <QSettings>
#include <QSplashScreen>
#include <QStatusBar>
#include <QStringList>
#include <QTcpSocket>
#include <QTextStream>
#include <QtGlobal>
#include <QTimer>
#include <QToolButton>
#include <QVBoxLayout>
#include <QWhatsThis>

#include <qgsnetworkaccessmanager.h>

#include <QNetworkReply>
#include <QNetworkProxy>
#include <QAuthenticator>

#if QT_VERSION >= 0x40500
#include <QNetworkDiskCache>
#endif

//
// Mac OS X Includes
// Must include before GEOS 3 due to unqualified use of 'Point'
//
#ifdef Q_OS_MACX
#include <ApplicationServices/ApplicationServices.h>

// check macro breaks QItemDelegate
#ifdef check
#undef check
#endif
#endif

//
// QGIS Specific Includes
//
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
#include "qgscomposermanager.h"
#include "qgsconfigureshortcutsdialog.h"
#include "qgscoordinatetransform.h"
#include "qgscursors.h"
#include "qgscustomprojectiondialog.h"
#include "qgsencodingfiledialog.h"
#include "qgsexception.h"
#include "qgsfeature.h"
#include "qgsformannotationitem.h"
#include "qgslabelinggui.h"
#include "qgsnewvectorlayerdialog.h"
#include "qgshelpviewer.h"
#include "qgsgenericprojectionselector.h"
#include "qgslegend.h"
#include "qgslegendlayer.h"
#include "qgslogger.h"
#include "qgsmapcanvas.h"
#include "qgsmaplayer.h"
#include "qgsmaplayerregistry.h"
#include "qgsmapoverviewcanvas.h"
#include "qgsmaprenderer.h"
#include "qgsmaptip.h"
#include "qgsmergeattributesdialog.h"
#include "qgsmessageviewer.h"
#include "qgsoptions.h"
#include "qgspastetransformations.h"
#include "qgspluginitem.h"
#include "qgspluginlayer.h"
#include "qgspluginlayerregistry.h"
#include "qgspluginmanager.h"
#include "qgspluginmetadata.h"
#include "qgspluginregistry.h"
#include "qgspoint.h"
#include "qgsproject.h"
#include "qgsprojectbadlayerguihandler.h"
#include "qgsprojectproperties.h"
#include "qgsproviderregistry.h"
#include "qgsrastercalcdialog.h"
#include "qgsrasterlayer.h"
#include "qgsrasterlayerproperties.h"
#include "qgsvectorlayerproperties.h"
#include "qgsrectangle.h"
#include "qgsrenderer.h"
#include "qgstextannotationitem.h"
#include "qgswmssourceselect.h"
#include "qgsshortcutsmanager.h"
#include "qgsundowidget.h"
#include "qgsvectordataprovider.h"
#include "qgsvectorlayer.h"
#include "ogr/qgsogrsublayersdialog.h"
#include "ogr/qgsopenvectorlayerdialog.h"
#include "ogr/qgsvectorlayersaveasdialog.h"
#include "qgsattributetabledialog.h"
#include "qgsvectorfilewriter.h"
#include "qgscredentialdialog.h"
#include "qgstilescalewidget.h"
#include "qgsquerybuilder.h"
#include "qgsattributeaction.h"
#include "qgsgpsinformationwidget.h"

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
#include "qgsmaptoolannotation.h"
#include "qgsmaptooldeletering.h"
#include "qgsmaptooldeletepart.h"
#include "qgsmaptooldeletevertex.h"
#include "qgsmaptoolformannotation.h"
#include "qgsmaptoolidentify.h"
#include "qgsmaptoolmeasureangle.h"
#include "qgsmaptoolmovefeature.h"
#include "qgsmaptoolmovevertex.h"
#include "qgsmaptoolnodetool.h"
#include "qgsmaptoolpan.h"
#include "qgsmaptoolselect.h"
#include "qgsmaptoolselectrectangle.h"
#include "qgsmaptoolselectfreehand.h"
#include "qgsmaptoolselectpolygon.h"
#include "qgsmaptoolselectradius.h"
#include "qgsmaptoolreshape.h"
#include "qgsmaptoolrotatepointsymbols.h"
#include "qgsmaptoolsplitfeatures.h"
#include "qgsmaptooltextannotation.h"
#include "qgsmaptoolvertexedit.h"
#include "qgsmaptoolzoom.h"
#include "qgsmaptoolsimplify.h"
#include "qgsmeasuretool.h"

//
// Conditional Includes
//
#ifdef HAVE_POSTGRESQL
#include "postgres/qgspgsourceselect.h"
#ifdef HAVE_PGCONFIG
#include <pg_config.h>
#else
#define PG_VERSION "unknown"
#endif
#endif
#include <sqlite3.h>
#ifdef HAVE_SPATIALITE
extern "C"
{
#include <spatialite.h>
}
#include "qgsspatialitesourceselect.h"
#include "qgsnewspatialitelayerdialog.h"
#endif

#include "qgspythonutils.h"

#ifndef WIN32
#include <dlfcn.h>
#endif

#ifdef WIN32
#include <windows.h>
#endif

class QTreeWidgetItem;


// IDs for locating particular menu items
const int BEFORE_RECENT_PATHS = 123;
const int AFTER_RECENT_PATHS = 321;



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
  QString caption = QgisApp::tr( "Quantum GIS " );

  if( QString( QGis::QGIS_VERSION ).endsWith( "Trunk" ) )
  {
    caption += QString( "r%1" ).arg( QGis::QGIS_SVN_VERSION );
  }
  else
  {
    caption += QGis::QGIS_VERSION;
  }

  if( QgsProject::instance()->title().isEmpty() )
  {
    if( QgsProject::instance()->fileName().isEmpty() )
    {
      // no project title nor file name, so just leave caption with
      // application name and version
    }
    else
    {
      QFileInfo projectFileInfo( QgsProject::instance()->fileName() );
      caption += " - " + projectFileInfo.baseName();
    }
  }
  else
  {
    caption += " - " + QgsProject::instance()->title();
  }

  qgisApp.setWindowTitle( caption );
} // setTitleBarText_( QWidget * qgisApp )

/**
 Creator function for output viewer
*/
static QgsMessageOutput *messageOutputViewer_()
{
  return new QgsMessageViewer( QgisApp::instance() );
}

/**
 * This function contains forced validation of CRS used in QGIS.
 * There are 3 options depending on the settings:
 * - ask for CRS using projection selecter
 * - use project's CRS
 * - use predefined global CRS
 */
static void customSrsValidation_( QgsCoordinateReferenceSystem* srs )
{
  QString toProj4;
  QSettings mySettings;
  QString myDefaultProjectionOption = mySettings.value( "/Projections/defaultBehaviour" ).toString();
  if( myDefaultProjectionOption == "prompt" )
  {
    //@note this class is not a descendent of QWidget so we cant pass
    //it in the ctor of the layer projection selector

    QgsGenericProjectionSelector * mySelector = new QgsGenericProjectionSelector();
    mySelector->setMessage( srs->validationHint() ); //shows a generic message, if not specified
    toProj4 = QgsProject::instance()->readEntry( "SpatialRefSys", "//ProjectCRSProj4String", GEOPROJ4 );
    QgsCoordinateReferenceSystem defaultCRS;
    if( defaultCRS.createFromProj4( toProj4 ) )
    {
      mySelector->setSelectedCrsId( defaultCRS.srsid() );
    }

    QApplication::setOverrideCursor( Qt::ArrowCursor );

    if( mySelector->exec() )
    {
      QgsDebugMsg( "Layer srs set from dialog: " + QString::number( mySelector->selectedCrsId() ) );
      srs->createFromProj4( mySelector->selectedProj4String() );
    }

    QApplication::restoreOverrideCursor();

    delete mySelector;
  }
  else if( myDefaultProjectionOption == "useProject" )
  {
    // XXX TODO: Change project to store selected CS as 'projectCRS' not 'selectedWkt'
    toProj4 = QgsProject::instance()->readEntry( "SpatialRefSys", "//ProjectCRSProj4String", GEOPROJ4 );
    QgsDebugMsg( "Layer srs set from project: " + toProj4 );
    QgisApp::instance()->statusBar()->showMessage( QObject::tr( "CRS undefined - defaulting to project CRS" ) );
    srs->createFromProj4( toProj4 );
  }
  else ///Projections/defaultBehaviour==useGlobal
  {
    srs->createFromProj4( mySettings.value( "/Projections/defaultProjectionString", GEOPROJ4 ).toString() );
    QgisApp::instance()->statusBar()->showMessage( QObject::tr( "CRS undefined - defaulting to default CRS" ) );
  }
}


QgisApp *QgisApp::smInstance = 0;

// constructor starts here
QgisApp::QgisApp( QSplashScreen *splash, bool restorePlugins, QWidget * parent, Qt::WFlags fl )
  : QMainWindow( parent, fl )
  , mSettingsMenu( NULL )
  , mSplash( splash )
  , mPythonUtils( NULL )
  , mpTileScaleWidget( NULL )
#ifdef Q_OS_WIN
  , mSkipNextContextMenuEvent( 0 )
#endif
  , mpGpsWidget( NULL )
{
  if( smInstance )
  {
    QMessageBox::critical(
      this,
      tr( "Multiple Instances of QgisApp" ),
      tr( "Multiple instances of Quantum GIS application object detected.\nPlease contact the developers.\n" ) );
    abort();
  }

  smInstance = this;

  namSetup();

//  setupUi(this);
  resize( 640, 480 );

  mSplash->showMessage( tr( "Checking database" ), Qt::AlignHCenter | Qt::AlignBottom );
  qApp->processEvents();
  // Do this early on before anyone else opens it and prevents us copying it
  createDB();

  mSplash->showMessage( tr( "Reading settings" ), Qt::AlignHCenter | Qt::AlignBottom );
  qApp->processEvents();

  mSplash->showMessage( tr( "Setting up the GUI" ), Qt::AlignHCenter | Qt::AlignBottom );
  qApp->processEvents();

  // "theMapCanvas" used to find this canonical instance later
  mMapCanvas = new QgsMapCanvas( this, "theMapCanvas" );
  mMapCanvas->setWhatsThis( tr( "Map canvas. This is where raster and vector "
                                "layers are displayed when added to the map" ) );
  setCentralWidget( mMapCanvas );
  //set the focus to the map canvas
  mMapCanvas->setFocus();

  // "theMapLegend" used to find this canonical instance later
  mMapLegend = new QgsLegend( mMapCanvas, this, "theMapLegend" );

  // create undo widget
  mUndoWidget = new QgsUndoWidget( NULL, mMapCanvas );
  mUndoWidget->setObjectName( "Undo" );

  createActions();
  createActionGroups();
  createMenus();
  createToolBars();
  createStatusBar();
  createCanvasTools();
  mMapCanvas->freeze();
  initLegend();
  createOverview();
  createMapTips();
  readSettings();
  updateRecentProjectPaths();

  addDockWidget( Qt::LeftDockWidgetArea, mUndoWidget );
  mUndoWidget->hide();

  mInternalClipboard = new QgsClipboard; // create clipboard
  mQgisInterface = new QgisAppInterface( this ); // create the interfce

#ifdef Q_WS_MAC
  // action for Window menu (create before generating WindowTitleChange event))
  mWindowAction = new QAction( this );
  connect( mWindowAction, SIGNAL( triggered() ), this, SLOT( activate() ) );

  // add this window to Window menu
  addWindow( mWindowAction );
#endif

  // set application's caption
  QString caption = tr( "Quantum GIS - %1 ('%2')" ).arg( QGis::QGIS_VERSION ).arg( QGis::QGIS_RELEASE_NAME );
  setWindowTitle( caption );

  // set QGIS specific srs validation
  QgsCoordinateReferenceSystem::setCustomSrsValidation( customSrsValidation_ );

  // set graphical message output
  QgsMessageOutput::setMessageOutputCreator( messageOutputViewer_ );

  // set graphical credential requester
  new QgsCredentialDialog( this );

  fileNew(); // prepare empty project
  qApp->processEvents();

  // load providers
  mSplash->showMessage( tr( "Checking provider plugins" ), Qt::AlignHCenter | Qt::AlignBottom );
  qApp->processEvents();
  QgsApplication::initQgis();

  mSplash->showMessage( tr( "Starting Python" ), Qt::AlignHCenter | Qt::AlignBottom );
  qApp->processEvents();
  loadPythonSupport();


  // Create the plugin registry and load plugins
  // load any plugins that were running in the last session
  mSplash->showMessage( tr( "Restoring loaded plugins" ), Qt::AlignHCenter | Qt::AlignBottom );
  qApp->processEvents();
  QgsPluginRegistry::instance()->setQgisInterface( mQgisInterface );
  if( restorePlugins )
  {
    // Restoring of plugins can be disabled with --noplugins command line option
    // because some plugins may cause QGIS to crash during startup
    QgsPluginRegistry::instance()->restoreSessionPlugins( QgsApplication::pluginPath() );
  }

  mSplash->showMessage( tr( "Initializing file filters" ), Qt::AlignHCenter | Qt::AlignBottom );
  qApp->processEvents();
  // now build vector file filter
  mVectorFileFilter = QgsProviderRegistry::instance()->fileVectorFilters();
  // now build raster file filter
  QgsRasterLayer::buildSupportedRasterFileFilter( mRasterFileFilter );

  // set handler for missing layers (will be owned by QgsProject)
  QgsProject::instance()->setBadLayerHandler( new QgsProjectBadLayerGuiHandler() );

#if 0
  // Set the background color for toolbox and overview as they default to
  // white instead of the window color
  QPalette myPalette = toolBox->palette();
  myPalette.setColor( QPalette::Button, myPalette.window().color() );
  toolBox->setPalette( myPalette );
  //do the same for legend control
  myPalette = toolBox->palette();
  myPalette.setColor( QPalette::Button, myPalette.window().color() );
  mMapLegend->setPalette( myPalette );
  //and for overview control this is done in createOverView method
#endif
  // Do this last in the ctor to ensure that all members are instantiated properly
  setupConnections();
  //
  // Please make sure this is the last thing the ctor does so that we can ensure the
  // widgets are all initialised before trying to restore their state.
  //
  mSplash->showMessage( tr( "Restoring window state" ), Qt::AlignHCenter | Qt::AlignBottom );
  qApp->processEvents();
  restoreWindowState();

  mSplash->showMessage( tr( "QGIS Ready!" ), Qt::AlignHCenter | Qt::AlignBottom );

  mMapTipsVisible = false;

  // setup drag drop
  setAcceptDrops( true );

  mFullScreenMode = false;
  mPrevScreenModeMaximized = false;
  show();
  qApp->processEvents();
  //finally show all the application settings as initialised above

  QgsDebugMsg( "\n\n\nApplication Settings:\n--------------------------\n" );
  QgsDebugMsg( QgsApplication::showSettings() );
  QgsDebugMsg( "\n--------------------------\n\n\n" );
  mMapCanvas->freeze( false );
  mMapCanvas->clearExtentHistory(); // reset zoomnext/zoomlast
  mLastComposerId = 0;
  mLBL = new QgsPalLabeling();
  mMapCanvas->mapRenderer()->setLabelingEngine( mLBL );
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
  delete mMapTools.mMeasureAngle;
  delete mMapTools.mTextAnnotation;
  delete mMapTools.mFormAnnotation;
  delete mMapTools.mAnnotation;
  delete mMapTools.mCapturePoint;
  delete mMapTools.mCaptureLine;
  delete mMapTools.mCapturePolygon;
  delete mMapTools.mMoveFeature;
  delete mMapTools.mReshapeFeatures;
  delete mMapTools.mSplitFeatures;
  delete mMapTools.mSelect;
  delete mMapTools.mSelectRectangle;
  delete mMapTools.mSelectPolygon;
  delete mMapTools.mSelectFreehand;
  delete mMapTools.mSelectRadius;

#if 0 //these three tools to be deprecated - use node tool rather
  delete mMapTools.mVertexAdd;
  delete mMapTools.mVertexMove;
  delete mMapTools.mVertexDelete;
#endif

  delete mMapTools.mAddRing;
  delete mMapTools.mSimplifyFeature;
  delete mMapTools.mDeleteRing;
  delete mMapTools.mDeletePart;
  delete mMapTools.mAddIsland;
  delete mMapTools.mNodeTool;

  delete mPythonUtils;

  deletePrintComposers();
  removeAnnotationItems();

  // delete map layer registry and provider registry
  QgsApplication::exitQgis();
}

void QgisApp::dragEnterEvent( QDragEnterEvent *event )
{
  if( event->mimeData()->hasUrls() )
  {
    event->acceptProposedAction();
  }
}

void QgisApp::dropEvent( QDropEvent *event )
{
  // get the file list
  QList<QUrl>::iterator i;
  QList<QUrl>urls = event->mimeData()->urls();
  for( i = urls.begin(); i != urls.end(); i++ )
  {
    QString fileName = i->toLocalFile();
    // seems that some drag and drop operations include an empty url
    // so we test for length to make sure we have something
    if( !fileName.isEmpty() )
    {
      // check to see if we are opening a project file
      QFileInfo fi( fileName );
      if( fi.completeSuffix() == "qgs" )
      {
        QgsDebugMsg( "Opening project " + fileName );
        openProject( fileName );
      }
      else
      {
        QgsDebugMsg( "Adding " + fileName + " to the map canvas" );
        openLayer( fileName, true );
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
  setTheme( settings.value( "/Themes", "default" ).toString() );

  // Add the recently accessed project file paths to the File menu
  mRecentProjectPaths = settings.value( "/UI/recentProjectsList" ).toStringList();

  // Restore state of tile scale widget
  if( settings.value( "/UI/tileScaleEnabled", false ).toBool() )
  {
    showTileScale();
  }
  // Restore state of GPS Tracker
  if( settings.value( "/gps/widgetEnabled", false ).toBool() )
  {
    showGpsTool();
  }
}


//////////////////////////////////////////////////////////////////////
//            Set Up the gui toolbars, menus, statusbar etc
//////////////////////////////////////////////////////////////////////

void QgisApp::createActions()
{
  QgsShortcutsManager* shortcuts = QgsShortcutsManager::instance();

  // File Menu Items

  mActionNewProject = new QAction( getThemeIcon( "mActionFileNew.png" ), tr( "&New Project" ), this );
  shortcuts->registerAction( mActionNewProject, tr( "Ctrl+N", "New Project" ) );
  mActionNewProject->setStatusTip( tr( "New Project" ) );
  connect( mActionNewProject, SIGNAL( triggered() ), this, SLOT( fileNew() ) );

  mActionOpenProject = new QAction( getThemeIcon( "mActionFileOpen.png" ), tr( "&Open Project..." ), this );
  shortcuts->registerAction( mActionOpenProject, tr( "Ctrl+O", "Open a Project" ) );
  mActionOpenProject->setStatusTip( tr( "Open a Project" ) );
  connect( mActionOpenProject, SIGNAL( triggered() ), this, SLOT( fileOpen() ) );

  mActionSaveProject = new QAction( getThemeIcon( "mActionFileSave.png" ), tr( "&Save Project" ), this );
  shortcuts->registerAction( mActionSaveProject, tr( "Ctrl+S", "Save Project" ) );
  mActionSaveProject->setStatusTip( tr( "Save Project" ) );
  connect( mActionSaveProject, SIGNAL( triggered() ), this, SLOT( fileSave() ) );

  mActionSaveProjectAs = new QAction( getThemeIcon( "mActionFileSaveAs.png" ), tr( "Save Project &As..." ), this );
  shortcuts->registerAction( mActionSaveProjectAs, tr( "Shift+Ctrl+S", "Save Project under a new name" ) );
  mActionSaveProjectAs->setStatusTip( tr( "Save Project under a new name" ) );
  connect( mActionSaveProjectAs, SIGNAL( triggered() ), this, SLOT( fileSaveAs() ) );

  mActionSaveMapAsImage = new QAction( getThemeIcon( "mActionSaveMapAsImage.png" ), tr( "Save as Image..." ), this );
  shortcuts->registerAction( mActionSaveMapAsImage ); // tr( "Ctrl+I", "Save map as image" )
  mActionSaveMapAsImage->setStatusTip( tr( "Save map as image" ) );
  connect( mActionSaveMapAsImage, SIGNAL( triggered() ), this, SLOT( saveMapAsImage() ) );

  mActionNewPrintComposer = new QAction( getThemeIcon( "mActionNewComposer.png" ), tr( "&New Print Composer" ), this );
  shortcuts->registerAction( mActionNewPrintComposer, tr( "Ctrl+P", "New Print Composer" ) );
  mActionNewPrintComposer->setStatusTip( tr( "New Print Composer" ) );
  connect( mActionNewPrintComposer, SIGNAL( triggered() ), this, SLOT( newPrintComposer() ) );

  mActionShowComposerManager = new QAction( getThemeIcon( "mActionComposerManager.png" ), tr( "Composer manager..." ), this );
  mActionShowComposerManager->setStatusTip( tr( "Composer manager" ) );
  connect( mActionShowComposerManager, SIGNAL( triggered() ), this, SLOT( showComposerManager() ) );

  mActionExit = new QAction( getThemeIcon( "mActionFileExit.png" ), tr( "Exit" ), this );
  shortcuts->registerAction( mActionExit, tr( "Ctrl+Q", "Exit QGIS" ) );
  mActionExit->setStatusTip( tr( "Exit QGIS" ) );
  mActionExit->setMenuRole( QAction::QuitRole ); // put in Application menu on Mac OS X
  connect( mActionExit, SIGNAL( triggered() ), this, SLOT( fileExit() ) );

  // Edit Menu Items

#if 0
  mActionCut = new QAction( tr( "Cu&t" ), this );
  shortcuts->registerAction( mActionCut, tr( "Ctrl+X" ) );
  mActionCut->setStatusTip( tr( "Cut the current selection's contents to the clipboard" ) );
  connect( mActionCut, SIGNAL( triggered ), this, SLOT( cut() ) );

  mActionCopy = new QAction( tr( "&Copy" ), this );
  shortcuts->registerAction( mActionCopy, tr( "Ctrl+C" ) );
  mActionCopy->setStatusTip( tr( "Copy the current selection's contents to the clipboard" ) );
  connect( mActionCopy, SIGNAL( triggered ), this, SLOT( copy() ) );

  mActionPaste = new QAction( tr( "&Paste" ), this );
  shortcuts->registerAction( mActionPaste, tr( "Ctrl+V" ) );
  mActionPaste->setStatusTip( tr( "Paste the clipboard's contents into the current selection" ) );
  connect( mActionPaste, SIGNAL( triggered ), this, SLOT( paste() ) );
#endif

  mActionUndo = new QAction( getThemeIcon( "mActionUndo.png" ), tr( "&Undo" ), this );
  shortcuts->registerAction( mActionUndo, tr( "Ctrl+Z" ) );
  mActionUndo->setStatusTip( tr( "Undo the last operation" ) );
  connect( mActionUndo, SIGNAL( triggered() ), mUndoWidget, SLOT( undo() ) );
  mActionUndo->setEnabled( false );

  mActionRedo = new QAction( getThemeIcon( "mActionRedo.png" ), tr( "&Redo" ), this );
  shortcuts->registerAction( mActionRedo, tr( "Ctrl+Shift+Z" ) );
  mActionRedo->setStatusTip( tr( "Redo the last operation" ) );
  connect( mActionRedo, SIGNAL( triggered() ), mUndoWidget, SLOT( redo() ) );
  mActionRedo->setEnabled( false );

  mActionCutFeatures = new QAction( getThemeIcon( "mActionEditCut.png" ), tr( "Cut Features" ), this );
  shortcuts->registerAction( mActionCutFeatures, tr( "Ctrl+X" ) );
  mActionCutFeatures->setStatusTip( tr( "Cut selected features" ) );
  connect( mActionCutFeatures, SIGNAL( triggered() ), this, SLOT( editCut() ) );
  mActionCutFeatures->setEnabled( false );

  mActionCopyFeatures = new QAction( getThemeIcon( "mActionEditCopy.png" ), tr( "Copy Features" ), this );
  shortcuts->registerAction( mActionCopyFeatures, tr( "Ctrl+C" ) );
  mActionCopyFeatures->setStatusTip( tr( "Copy selected features" ) );
  connect( mActionCopyFeatures, SIGNAL( triggered() ), this, SLOT( editCopy() ) );
  mActionCopyFeatures->setEnabled( false );

  mActionPasteFeatures = new QAction( getThemeIcon( "mActionEditPaste.png" ), tr( "Paste Features" ), this );
  shortcuts->registerAction( mActionPasteFeatures, tr( "Ctrl+V" ) );
  mActionPasteFeatures->setStatusTip( tr( "Paste selected features" ) );
  connect( mActionPasteFeatures, SIGNAL( triggered() ), this, SLOT( editPaste() ) );
  mActionPasteFeatures->setEnabled( false );

  mActionCapturePoint = new QAction( getThemeIcon( "mActionCapturePoint.png" ), tr( "Capture Point" ), this );
  shortcuts->registerAction( mActionCapturePoint, tr( "Ctrl+.", "Capture Points" ) );
  mActionCapturePoint->setStatusTip( tr( "Capture Points" ) );
  connect( mActionCapturePoint, SIGNAL( triggered() ), this, SLOT( capturePoint() ) );
  mActionCapturePoint->setEnabled( false );

  mActionCaptureLine = new QAction( getThemeIcon( "mActionCaptureLine.png" ), tr( "Capture Line" ), this );
  shortcuts->registerAction( mActionCaptureLine, tr( "Ctrl+/", "Capture Lines" ) );
  mActionCaptureLine->setStatusTip( tr( "Capture Lines" ) );
  connect( mActionCaptureLine, SIGNAL( triggered() ), this, SLOT( captureLine() ) );
  mActionCaptureLine->setEnabled( false );

  mActionCapturePolygon = new QAction( getThemeIcon( "mActionCapturePolygon.png" ), tr( "Capture Polygon" ), this );
  shortcuts->registerAction( mActionCapturePolygon, tr( "Ctrl+Shift+/", "Capture Polygons" ) );
  mActionCapturePolygon->setStatusTip( tr( "Capture Polygons" ) );
  connect( mActionCapturePolygon, SIGNAL( triggered() ), this, SLOT( capturePolygon() ) );
  mActionCapturePolygon->setEnabled( false );

  mActionMoveFeature = new QAction( getThemeIcon( "mActionMoveFeature.png" ), tr( "Move Feature(s)" ), this );
  shortcuts->registerAction( mActionMoveFeature );
  mActionMoveFeature->setStatusTip( tr( "Move Feature(s)" ) );
  connect( mActionMoveFeature, SIGNAL( triggered() ), this, SLOT( moveFeature() ) );
  mActionMoveFeature->setEnabled( false );

  mActionReshapeFeatures = new QAction( getThemeIcon( "mActionReshape.png" ), tr( "Reshape Features" ), this );
  shortcuts->registerAction( mActionReshapeFeatures );
  mActionReshapeFeatures->setStatusTip( tr( "Reshape Features" ) );
  connect( mActionReshapeFeatures, SIGNAL( triggered() ), this, SLOT( reshapeFeatures() ) );
  mActionReshapeFeatures->setEnabled( false );

  mActionSplitFeatures = new QAction( getThemeIcon( "mActionSplitFeatures.png" ), tr( "Split Features" ), this );
  shortcuts->registerAction( mActionSplitFeatures );
  mActionSplitFeatures->setStatusTip( tr( "Split Features" ) );
  connect( mActionSplitFeatures, SIGNAL( triggered() ), this, SLOT( splitFeatures() ) );
  mActionSplitFeatures->setEnabled( false );

  mActionDeleteSelected = new QAction( getThemeIcon( "mActionDeleteSelected.png" ), tr( "Delete Selected" ), this );
  shortcuts->registerAction( mActionDeleteSelected );
  mActionDeleteSelected->setStatusTip( tr( "Delete Selected" ) );
  connect( mActionDeleteSelected, SIGNAL( triggered() ), this, SLOT( deleteSelected() ) );
  mActionDeleteSelected->setEnabled( false );

#if 0 //these three tools to be deprecated - use node tool rather
  mActionAddVertex = new QAction( getThemeIcon( "mActionAddVertex.png" ), tr( "Add Vertex" ), this );
  shortcuts->registerAction( mActionAddVertex );
  mActionAddVertex->setStatusTip( tr( "Add Vertex" ) );
  connect( mActionAddVertex, SIGNAL( triggered() ), this, SLOT( addVertex() ) );
  mActionAddVertex->setEnabled( false );

  mActionMoveVertex = new QAction( getThemeIcon( "mActionMoveVertex.png" ), tr( "Move Vertex" ), this );
  shortcuts->registerAction( mActionMoveVertex );
  mActionMoveVertex->setStatusTip( tr( "Move Vertex" ) );
  connect( mActionMoveVertex, SIGNAL( triggered() ), this, SLOT( moveVertex() ) );
  mActionMoveVertex->setEnabled( false );

  mActionDeleteVertex = new QAction( getThemeIcon( "mActionDeleteVertex.png" ), tr( "Delete Vertex" ), this );
  shortcuts->registerAction( mActionDeleteVertex );
  mActionDeleteVertex->setStatusTip( tr( "Delete Vertex" ) );
  connect( mActionDeleteVertex, SIGNAL( triggered() ), this, SLOT( deleteVertex() ) );
  mActionDeleteVertex->setEnabled( false );
#endif

  mActionAddRing = new QAction( getThemeIcon( "mActionAddRing.png" ), tr( "Add Ring" ), this );
  shortcuts->registerAction( mActionAddRing );
  mActionAddRing->setStatusTip( tr( "Add Ring" ) );
  connect( mActionAddRing, SIGNAL( triggered() ), this, SLOT( addRing() ) );
  mActionAddRing->setEnabled( false );

  mActionAddIsland = new QAction( getThemeIcon( "mActionAddIsland.png" ), tr( "Add Part" ), this );
  shortcuts->registerAction( mActionAddIsland );
  mActionAddIsland->setStatusTip( tr( "Add part to multipolygon" ) );
  connect( mActionAddIsland, SIGNAL( triggered() ), this, SLOT( addIsland() ) );
  mActionAddIsland->setEnabled( false );

  mActionSimplifyFeature = new QAction( getThemeIcon( "mActionSimplify.png" ), tr( "Simplify Feature" ), this );
  shortcuts->registerAction( mActionSimplifyFeature );
  mActionSimplifyFeature->setStatusTip( tr( "Simplify Feature" ) );
  connect( mActionSimplifyFeature, SIGNAL( triggered() ), this, SLOT( simplifyFeature() ) );
  mActionSimplifyFeature->setEnabled( false );

  mActionDeleteRing = new QAction( getThemeIcon( "mActionDeleteRing.png" ), tr( "Delete Ring" ), this );
  shortcuts->registerAction( mActionDeleteRing );
  mActionDeleteRing->setStatusTip( tr( "Click a vertex of the ring to delete" ) );
  connect( mActionDeleteRing, SIGNAL( triggered() ), this, SLOT( deleteRing() ) );
  mActionDeleteRing->setEnabled( false );

  mActionDeletePart = new QAction( getThemeIcon( "mActionDeletePart.png" ), tr( "Delete Part" ), this );
  shortcuts->registerAction( mActionDeletePart );
  mActionDeletePart->setStatusTip( tr( "Click a vertex of the part to delete" ) );
  connect( mActionDeletePart, SIGNAL( triggered() ), this, SLOT( deletePart() ) );
  mActionDeletePart->setEnabled( false );

  mActionMergeFeatures = new QAction( getThemeIcon( "mActionMergeFeatures.png" ), tr( "Merge selected features" ), this );
  shortcuts->registerAction( mActionMergeFeatures );
  mActionMergeFeatures->setStatusTip( tr( "Merge selected features" ) );
  connect( mActionMergeFeatures, SIGNAL( triggered() ), this, SLOT( mergeSelectedFeatures() ) );
  mActionMergeFeatures->setEnabled( false );

  mActionNodeTool = new QAction( getThemeIcon( "mActionNodeTool.png" ), tr( "Node Tool" ), this );
  shortcuts->registerAction( mActionNodeTool );
  mActionNodeTool->setStatusTip( tr( "Node Tool" ) );
  connect( mActionNodeTool, SIGNAL( triggered() ), this, SLOT( nodeTool() ) );
  mActionNodeTool->setEnabled( false );

  mActionRotatePointSymbols = new QAction( getThemeIcon( "mActionRotatePointSymbols.png" ), tr( "Rotate Point Symbols" ), this );
  shortcuts->registerAction( mActionRotatePointSymbols );
  mActionRotatePointSymbols->setStatusTip( tr( "Rotate Point Symbols" ) );
  connect( mActionRotatePointSymbols, SIGNAL( triggered() ), this, SLOT( rotatePointSymbols() ) );
  mActionRotatePointSymbols->setEnabled( false );

  // View Menu Items

  mActionPan = new QAction( getThemeIcon( "mActionPan.png" ), tr( "Pan Map" ), this );
  shortcuts->registerAction( mActionPan );
  mActionPan->setStatusTip( tr( "Pan the map" ) );
  connect( mActionPan, SIGNAL( triggered() ), this, SLOT( pan() ) );

  mActionZoomIn = new QAction( getThemeIcon( "mActionZoomIn.png" ), tr( "Zoom In" ), this );
  shortcuts->registerAction( mActionZoomIn, tr( "Ctrl++", "Zoom In" ) );
  mActionZoomIn->setStatusTip( tr( "Zoom In" ) );
  connect( mActionZoomIn, SIGNAL( triggered() ), this, SLOT( zoomIn() ) );

  mActionZoomOut = new QAction( getThemeIcon( "mActionZoomOut.png" ), tr( "Zoom Out" ), this );
  shortcuts->registerAction( mActionZoomOut, tr( "Ctrl+-", "Zoom Out" ) );
  mActionZoomOut->setStatusTip( tr( "Zoom Out" ) );
  connect( mActionZoomOut, SIGNAL( triggered() ), this, SLOT( zoomOut() ) );

  mActionSelect = new QAction( getThemeIcon( "mActionSelect.png" ), tr( "Select Features" ) , this );
  shortcuts->registerAction( mActionSelect );
  mActionSelect->setStatusTip( tr( "Select Features" ) );
  connect( mActionSelect, SIGNAL( triggered() ), this, SLOT( select() ) );
  mActionSelect->setEnabled( false );

  mActionSelectRectangle = new QAction( getThemeIcon( "mActionSelectRectangle.png" ), tr( "Select features by rectangle" ), this );
  shortcuts->registerAction( mActionSelectRectangle );
  mActionSelectRectangle->setStatusTip( tr( "Select features by rectangle" ) );
  connect( mActionSelectRectangle, SIGNAL( triggered() ), this, SLOT( selectByRectangle() ) );
  mActionSelectRectangle->setEnabled( false );

  mActionSelectPolygon = new QAction( getThemeIcon( "mActionSelectPolygon.png" ), tr( "Select features by polygon" ), this );
  shortcuts->registerAction( mActionSelectPolygon );
  mActionSelectPolygon->setStatusTip( tr( "Select features by polygon" ) );
  connect( mActionSelectPolygon, SIGNAL( triggered() ), this, SLOT( selectByPolygon() ) );
  mActionSelectPolygon->setEnabled( false );

  mActionSelectFreehand = new QAction( getThemeIcon( "mActionSelectFreehand.png" ), tr( "Select features by freehand" ), this );
  shortcuts->registerAction( mActionSelectFreehand );
  mActionSelectFreehand->setStatusTip( tr( "Select features by freehand" ) );
  connect( mActionSelectFreehand, SIGNAL( triggered() ), this, SLOT( selectByFreehand() ) );
  mActionSelectFreehand->setEnabled( false );

  mActionSelectRadius = new QAction( getThemeIcon( "mActionSelectRadius.png" ), tr( "Select features by radius" ), this );
  shortcuts->registerAction( mActionSelectRadius );
  mActionSelectRadius->setStatusTip( tr( "Select features by radius" ) );
  connect( mActionSelectRadius, SIGNAL( triggered() ), this, SLOT( selectByRadius() ) );
  mActionSelectRadius->setEnabled( false );

  mActionDeselectAll = new QAction( getThemeIcon( "mActionDeselectAll.png" ), tr( "Deselect features from all layers" ), this );
  shortcuts->registerAction( mActionDeselectAll );
  mActionDeselectAll->setStatusTip( tr( "Deselect features from all layers" ) );
  connect( mActionDeselectAll, SIGNAL( triggered() ), this, SLOT( deselectAll() ) );
  mActionDeselectAll->setEnabled( true );

  mActionIdentify = new QAction( getThemeIcon( "mActionIdentify.png" ), tr( "Identify Features" ), this );
  shortcuts->registerAction( mActionIdentify, tr( "Ctrl+Shift+I", "Click on features to identify them" ) );
  mActionIdentify->setStatusTip( tr( "Click on features to identify them" ) );
  connect( mActionIdentify, SIGNAL( triggered() ), this, SLOT( identify() ) );
  mActionIdentify->setEnabled( QSettings().value( "/Map/identifyMode", 0 ).toInt() != 0 );

  mActionMeasure = new QAction( getThemeIcon( "mActionMeasure.png" ), tr( "Measure Line " ), this );
  shortcuts->registerAction( mActionMeasure, tr( "Ctrl+Shift+M", "Measure a Line" ) );
  mActionMeasure->setStatusTip( tr( "Measure a Line" ) );
  connect( mActionMeasure, SIGNAL( triggered() ), this, SLOT( measure() ) );

  mActionMeasureArea = new QAction( getThemeIcon( "mActionMeasureArea.png" ), tr( "Measure Area" ), this );
  shortcuts->registerAction( mActionMeasureArea, tr( "Ctrl+Shift+J", "Measure an Area" ) );
  mActionMeasureArea->setStatusTip( tr( "Measure an Area" ) );
  connect( mActionMeasureArea, SIGNAL( triggered() ), this, SLOT( measureArea() ) );

  mActionMeasureAngle = new QAction( getThemeIcon( "mActionMeasureAngle.png" ), tr( "Measure Angle" ), this );
  mActionMeasureAngle->setStatusTip( tr( "Measure Angle" ) );
  connect( mActionMeasureAngle, SIGNAL( triggered() ), this, SLOT( measureAngle() ) );

  mActionZoomFullExtent = new QAction( getThemeIcon( "mActionZoomFullExtent.png" ), tr( "Zoom Full" ), this );
  shortcuts->registerAction( mActionZoomFullExtent, tr( "Ctrl+Shift+F", "Zoom to Full Extents" ) );
  mActionZoomFullExtent->setStatusTip( tr( "Zoom to Full Extents" ) );
  connect( mActionZoomFullExtent, SIGNAL( triggered() ), this, SLOT( zoomFull() ) );

  mActionZoomToLayer = new QAction( getThemeIcon( "mActionZoomToLayer.png" ), tr( "Zoom to Layer" ), this );
  shortcuts->registerAction( mActionZoomToLayer ); // tr("Ctrl+O","Zoom to Layer")
  mActionZoomToLayer->setStatusTip( tr( "Zoom to Layer" ) );
  connect( mActionZoomToLayer, SIGNAL( triggered() ), this, SLOT( zoomToLayerExtent() ) );

  mActionZoomToSelected = new QAction( getThemeIcon( "mActionZoomToSelected.png" ), tr( "Zoom to Selection" ), this );
  shortcuts->registerAction( mActionZoomToSelected, tr( "Ctrl+J", "Zoom to Selection" ) );
  mActionZoomToSelected->setStatusTip( tr( "Zoom to Selection" ) );
  connect( mActionZoomToSelected, SIGNAL( triggered() ), this, SLOT( zoomToSelected() ) );

  mActionZoomLast = new QAction( getThemeIcon( "mActionZoomLast.png" ), tr( "Zoom Last" ), this );
  shortcuts->registerAction( mActionZoomLast ); // tr("Ctrl+O","Zoom to Last Extent")
  mActionZoomLast->setStatusTip( tr( "Zoom to Last Extent" ) );
  connect( mActionZoomLast, SIGNAL( triggered() ), this, SLOT( zoomToPrevious() ) );

  mActionZoomNext = new QAction( getThemeIcon( "mActionZoomNext.png" ), tr( "Zoom Next" ), this );
  shortcuts->registerAction( mActionZoomNext );
  mActionZoomNext->setStatusTip( tr( "Zoom to Forward Extent" ) );
  connect( mActionZoomNext, SIGNAL( triggered() ), this, SLOT( zoomToNext() ) );

  mActionZoomActualSize = new QAction( tr( "Zoom Actual Size" ), this );
  shortcuts->registerAction( mActionZoomActualSize );
  mActionZoomActualSize->setStatusTip( tr( "Zoom to Actual Size" ) );
  connect( mActionZoomActualSize, SIGNAL( triggered() ), this, SLOT( zoomActualSize() ) );
  mActionZoomActualSize->setEnabled( false );

  mActionMapTips = new QAction( getThemeIcon( "mActionMapTips.png" ), tr( "Map Tips" ), this );
  shortcuts->registerAction( mActionMapTips );
  mActionMapTips->setStatusTip( tr( "Show information about a feature when the mouse is hovered over it" ) );
  connect( mActionMapTips, SIGNAL( triggered() ), this, SLOT( toggleMapTips() ) );
  mActionMapTips->setCheckable( true );

  mActionNewBookmark = new QAction( getThemeIcon( "mActionNewBookmark.png" ), tr( "New Bookmark..." ), this );
  shortcuts->registerAction( mActionNewBookmark, tr( "Ctrl+B", "New Bookmark" ) );
  mActionNewBookmark->setStatusTip( tr( "New Bookmark" ) );
  connect( mActionNewBookmark, SIGNAL( triggered() ), this, SLOT( newBookmark() ) );

  mActionShowBookmarks = new QAction( getThemeIcon( "mActionShowBookmarks.png" ), tr( "Show Bookmarks" ), this );
  shortcuts->registerAction( mActionShowBookmarks, tr( "Ctrl+Shift+B", "Show Bookmarks" ) );
  mActionShowBookmarks->setStatusTip( tr( "Show Bookmarks" ) );
  connect( mActionShowBookmarks, SIGNAL( triggered() ), this, SLOT( showBookmarks() ) );

  mActionDraw = new QAction( getThemeIcon( "mActionDraw.png" ), tr( "Refresh" ), this );
  shortcuts->registerAction( mActionDraw, tr( "Ctrl+R", "Refresh Map" ) );
  mActionDraw->setStatusTip( tr( "Refresh Map" ) );
  connect( mActionDraw, SIGNAL( triggered() ), this, SLOT( refreshMapCanvas() ) );

  mActionTextAnnotation = new QAction( getThemeIcon( "mActionTextAnnotation.png" ), tr( "Text Annotation" ), this );
  mActionTextAnnotation->setCheckable( true );
  connect( mActionTextAnnotation, SIGNAL( triggered() ), this, SLOT( addTextAnnotation() ) );

  mActionFormAnnotation = new QAction( getThemeIcon( "mActionFormAnnotation.png" ), tr( "Form annotation" ), this );
  mActionFormAnnotation->setCheckable( true );
  connect( mActionFormAnnotation, SIGNAL( triggered() ), this, SLOT( addFormAnnotation() ) );

  mActionAnnotation = new QAction( getThemeIcon( "mActionAnnotation.png" ), tr( "Move Annotation" ), this );
  mActionAnnotation->setCheckable( true );
  connect( mActionAnnotation, SIGNAL( triggered() ), this, SLOT( modifyAnnotation() ) );

  mActionLabeling = new QAction( getThemeIcon( "mActionLabeling.png" ), tr( "Labeling" ), this );
  connect( mActionLabeling, SIGNAL( triggered() ), this, SLOT( labeling() ) );

  // Layer Menu Items

  mActionNewVectorLayer = new QAction( getThemeIcon( "mActionNewVectorLayer.png" ), tr( "New Shapefile Layer..." ), this );
  shortcuts->registerAction( mActionNewVectorLayer, tr( "Ctrl+Shift+N", "Create a New Shapefile layer" ) );
  mActionNewVectorLayer->setStatusTip( tr( "Create a New Shapefile layer" ) );
  connect( mActionNewVectorLayer, SIGNAL( triggered() ), this, SLOT( newVectorLayer() ) );

#ifdef HAVE_SPATIALITE
  mActionNewSpatialiteLayer = new QAction( getThemeIcon( "mActionNewVectorLayer.png" ), tr( "New SpatiaLite Layer ..." ), this );
  shortcuts->registerAction( mActionNewSpatialiteLayer, tr( "Ctrl+Shift+A", "Create a New SpatiaLite Layer " ) );
  mActionNewSpatialiteLayer->setStatusTip( tr( "Create a New SpatiaLite Layer " ) );
  connect( mActionNewSpatialiteLayer, SIGNAL( triggered() ), this, SLOT( newSpatialiteLayer() ) );
#endif

  mActionShowRasterCalculator = new QAction( tr( "Raster calculator ..." ), this ); //todo: icon
  connect( mActionShowRasterCalculator, SIGNAL( triggered() ), this, SLOT( showRasterCalculator() ) );

  mActionAddOgrLayer = new QAction( getThemeIcon( "mActionAddOgrLayer.png" ), tr( "Add Vector Layer..." ), this );
  shortcuts->registerAction( mActionAddOgrLayer, tr( "Ctrl+Shift+V", "Add a Vector Layer" ) );
  mActionAddOgrLayer->setStatusTip( tr( "Add a Vector Layer" ) );
  connect( mActionAddOgrLayer, SIGNAL( triggered() ), this, SLOT( addVectorLayer() ) );

  mActionAddRasterLayer = new QAction( getThemeIcon( "mActionAddRasterLayer.png" ), tr( "Add Raster Layer..." ), this );
  shortcuts->registerAction( mActionAddRasterLayer, tr( "Ctrl+Shift+R", "Add a Raster Layer" ) );
  mActionAddRasterLayer->setStatusTip( tr( "Add a Raster Layer" ) );
  connect( mActionAddRasterLayer, SIGNAL( triggered() ), this, SLOT( addRasterLayer() ) );

  mActionAddPgLayer = new QAction( getThemeIcon( "mActionAddLayer.png" ), tr( "Add PostGIS Layer..." ), this );
  shortcuts->registerAction( mActionAddPgLayer, tr( "Ctrl+Shift+D", "Add a PostGIS Layer" ) );
  mActionAddPgLayer->setStatusTip( tr( "Add a PostGIS Layer" ) );
  connect( mActionAddPgLayer, SIGNAL( triggered() ), this, SLOT( addDatabaseLayer() ) );

  mActionAddSpatiaLiteLayer = new QAction( getThemeIcon( "mActionAddSpatiaLiteLayer.png" ), tr( "Add SpatiaLite Layer..." ), this );
  shortcuts->registerAction( mActionAddSpatiaLiteLayer, tr( "Ctrl+Shift+L", "Add a SpatiaLite Layer" ) );
  mActionAddSpatiaLiteLayer->setStatusTip( tr( "Add a SpatiaLite Layer" ) );
  connect( mActionAddSpatiaLiteLayer, SIGNAL( triggered() ), this, SLOT( addSpatiaLiteLayer() ) );

  mActionAddWmsLayer = new QAction( getThemeIcon( "mActionAddWmsLayer.png" ), tr( "Add WMS Layer..." ), this );
  shortcuts->registerAction( mActionAddWmsLayer, tr( "Ctrl+Shift+W", "Add a Web Mapping Server Layer" ) );
  mActionAddWmsLayer->setStatusTip( tr( "Add a Web Mapping Server Layer" ) );
  connect( mActionAddWmsLayer, SIGNAL( triggered() ), this, SLOT( addWmsLayer() ) );

  mActionOpenTable = new QAction( getThemeIcon( "mActionOpenTable.png" ), tr( "Open Attribute Table" ), this );
  shortcuts->registerAction( mActionOpenTable ); // tr("Ctrl+O","Open Table")
  mActionOpenTable->setStatusTip( tr( "Open Attribute Table" ) );
  connect( mActionOpenTable, SIGNAL( triggered() ), this, SLOT( attributeTable() ) );
  mActionOpenTable->setEnabled( false );

  mActionToggleEditing = new QAction( getThemeIcon( "mActionToggleEditing.png" ), tr( "Toggle editing" ), this );
  shortcuts->registerAction( mActionToggleEditing );
  mActionToggleEditing->setStatusTip( tr( "Toggles the editing state of the current layer" ) );
  mActionToggleEditing->setCheckable( true );
  connect( mActionToggleEditing, SIGNAL( triggered() ), this, SLOT( toggleEditing() ) );
  mActionToggleEditing->setEnabled( false );

  mActionSaveEdits = new QAction( getThemeIcon( "mActionSaveEdits.png" ), tr( "Save edits" ), this );
  shortcuts->registerAction( mActionSaveEdits );
  mActionSaveEdits->setStatusTip( tr( "Save edits to current layer, but continue editing" ) );
  connect( mActionSaveEdits, SIGNAL( triggered() ), this, SLOT( saveEdits() ) );
  mActionSaveEdits->setEnabled( false );

  mActionLayerSaveAs = new QAction( tr( "Save as..." ), this );
  shortcuts->registerAction( mActionLayerSaveAs );
  mActionLayerSaveAs->setStatusTip( tr( "Save the current layer as a vector file" ) );
  connect( mActionLayerSaveAs, SIGNAL( triggered() ), this, SLOT( saveAsVectorFile() ) );
  mActionLayerSaveAs->setEnabled( false );

  mActionLayerSelectionSaveAs = new QAction( tr( "Save Selection as vector file..." ), this );
  shortcuts->registerAction( mActionLayerSelectionSaveAs );
  mActionLayerSelectionSaveAs->setStatusTip( tr( "Save the selection as a vector file" ) );
  connect( mActionLayerSelectionSaveAs, SIGNAL( triggered() ), this, SLOT( saveSelectionAsVectorFile() ) );
  mActionLayerSelectionSaveAs->setEnabled( false );

  mActionRemoveLayer = new QAction( getThemeIcon( "mActionRemoveLayer.png" ), tr( "Remove Layer(s)" ), this );
  shortcuts->registerAction( mActionRemoveLayer, tr( "Ctrl+D", "Remove Layer(s)" ) );
  mActionRemoveLayer->setStatusTip( tr( "Remove Layer(s)" ) );
  connect( mActionRemoveLayer, SIGNAL( triggered() ), this, SLOT( removeLayer() ) );
  mActionRemoveLayer->setEnabled( false );

  mActionTileScale = new QAction( getThemeIcon( "mActionTileScale.png" ), tr( "Tile scale slider" ), this );
  shortcuts->registerAction( mActionTileScale, tr( "", "Tile scale slider" ) );
  mActionTileScale->setStatusTip( tr( "Show tile scale slider" ) );
  connect( mActionTileScale, SIGNAL( triggered() ), this, SLOT( showTileScale() ) );
  mActionTileScale->setEnabled( true );

  mActionGpsTool = new QAction( getThemeIcon( "mActionGpsTool.png" ), tr( "Live GPS tracking" ), this );
  shortcuts->registerAction( mActionGpsTool, tr( "", "Live GPS tracking" ) );
  mActionGpsTool->setStatusTip( tr( "Show GPS tool" ) );
  connect( mActionGpsTool, SIGNAL( triggered() ), this, SLOT( showGpsTool() ) );
  mActionGpsTool->setEnabled( true );

  mActionLayerProperties = new QAction( tr( "Properties..." ), this );
  shortcuts->registerAction( mActionLayerProperties );
  mActionLayerProperties->setStatusTip( tr( "Set properties of the current layer" ) );
  connect( mActionLayerProperties, SIGNAL( triggered() ), this, SLOT( layerProperties() ) );
  mActionLayerProperties->setEnabled( false );

  mActionLayerSubsetString = new QAction( tr( "Query..." ), this );
  shortcuts->registerAction( mActionLayerSubsetString );
  mActionLayerSubsetString->setStatusTip( tr( "Set subset query of the current layer" ) );
  connect( mActionLayerSubsetString, SIGNAL( triggered() ), this, SLOT( layerSubsetString() ) );
  mActionLayerSubsetString->setEnabled( false );

  mActionAddToOverview = new QAction( getThemeIcon( "mActionInOverview.png" ), tr( "Add to Overview" ), this );
  shortcuts->registerAction( mActionAddToOverview, tr( "Ctrl+Shift+O", "Add current layer to overview map" ) );
  mActionAddToOverview->setStatusTip( tr( "Add current layer to overview map" ) );
  connect( mActionAddToOverview, SIGNAL( triggered() ), this, SLOT( isInOverview() ) );
  mActionAddToOverview->setEnabled( false );

  mActionAddAllToOverview = new QAction( getThemeIcon( "mActionAddAllToOverview.png" ), tr( "Add All to Overview" ), this );
  shortcuts->registerAction( mActionAddAllToOverview ); //, tr( "+", "Show all layers in the overview map" ) );
  mActionAddAllToOverview->setStatusTip( tr( "Show all layers in the overview map" ) );
  connect( mActionAddAllToOverview, SIGNAL( triggered() ), this, SLOT( addAllToOverview() ) );

  mActionRemoveAllFromOverview = new QAction( getThemeIcon( "mActionRemoveAllFromOverview.png" ), tr( "Remove All From Overview" ), this );
  shortcuts->registerAction( mActionRemoveAllFromOverview ); //, tr( "-", "Remove all layers from overview map" ) );
  mActionRemoveAllFromOverview->setStatusTip( tr( "Remove all layers from overview map" ) );
  connect( mActionRemoveAllFromOverview, SIGNAL( triggered() ), this, SLOT( removeAllFromOverview() ) );

  mActionShowAllLayers = new QAction( getThemeIcon( "mActionShowAllLayers.png" ), tr( "Show All Layers" ), this );
  shortcuts->registerAction( mActionShowAllLayers, tr( "Ctrl+Shift+U", "Show all layers" ) );
  mActionShowAllLayers->setStatusTip( tr( "Show all layers" ) );
  connect( mActionShowAllLayers, SIGNAL( triggered() ), this, SLOT( showAllLayers() ) );

  mActionHideAllLayers = new QAction( getThemeIcon( "mActionHideAllLayers.png" ), tr( "Hide All Layers" ), this );
  shortcuts->registerAction( mActionHideAllLayers, tr( "Ctrl+Shift+H", "Hide all layers" ) );
  mActionHideAllLayers->setStatusTip( tr( "Hide all layers" ) );
  connect( mActionHideAllLayers, SIGNAL( triggered() ), this, SLOT( hideAllLayers() ) );

  // Plugin Menu Items

  mActionManagePlugins = new QAction( getThemeIcon( "mActionShowPluginManager.png" ), tr( "Manage Plugins..." ), this );
  shortcuts->registerAction( mActionManagePlugins ); // tr("Ctrl+P","Open the plugin manager")
  mActionManagePlugins->setStatusTip( tr( "Open the plugin manager" ) );
  connect( mActionManagePlugins, SIGNAL( triggered() ), this, SLOT( showPluginManager() ) );

  // Settings Menu Items

  mActionToggleFullScreen = new QAction( getThemeIcon( "mActionToggleFullScreen.png" ), tr( "Toggle Full Screen Mode" ), this );
  shortcuts->registerAction( mActionToggleFullScreen, tr( "Ctrl+F", "Toggle fullscreen mode" ) );
  mActionToggleFullScreen->setStatusTip( tr( "Toggle fullscreen mode" ) );
  connect( mActionToggleFullScreen, SIGNAL( triggered() ), this, SLOT( toggleFullScreen() ) );

  mActionProjectProperties = new QAction( getThemeIcon( "mActionProjectProperties.png" ), tr( "Project Properties..." ), this );
  shortcuts->registerAction( mActionProjectProperties, tr( "Ctrl+Shift+P", "Set project properties" ) );
  mActionProjectProperties->setStatusTip( tr( "Set project properties" ) );
  connect( mActionProjectProperties, SIGNAL( triggered() ), this, SLOT( projectProperties() ) );

  mActionOptions = new QAction( getThemeIcon( "mActionOptions.png" ), tr( "Options..." ), this );
  shortcuts->registerAction( mActionOptions ); // tr("Alt+O","Change various QGIS options")
  mActionOptions->setStatusTip( tr( "Change various QGIS options" ) );
  mActionOptions->setMenuRole( QAction::PreferencesRole ); // put in application menu on Mac OS X
  connect( mActionOptions, SIGNAL( triggered() ), this, SLOT( options() ) );

  mActionCustomProjection = new QAction( getThemeIcon( "mActionCustomProjection.png" ), tr( "Custom CRS..." ), this );
  shortcuts->registerAction( mActionCustomProjection ); // tr("Alt+I","Manage custom projections")
  mActionCustomProjection->setStatusTip( tr( "Manage custom coordinate reference systems" ) );
  // mActionCustomProjection->setMenuRole( QAction::ApplicationSpecificRole ); // put in application menu on Mac OS X
  connect( mActionCustomProjection, SIGNAL( triggered() ), this, SLOT( customProjection() ) );

  mActionConfigureShortcuts = new QAction( getThemeIcon( "mActionOptions.png" ), tr( "Configure shortcuts..." ), this );
  shortcuts->registerAction( mActionConfigureShortcuts );
  mActionConfigureShortcuts->setStatusTip( tr( "Configure shortcuts" ) );
  connect( mActionConfigureShortcuts, SIGNAL( triggered() ), this, SLOT( configureShortcuts() ) );

#ifdef Q_WS_MAC
  // Window Menu Items

  mActionWindowMinimize = new QAction( tr( "Minimize" ), this );
  shortcuts->registerAction( mActionWindowMinimize, tr( "Ctrl+M", "Minimize Window" ) );
  mActionWindowMinimize->setStatusTip( tr( "Minimizes the active window to the dock" ) );
  connect( mActionWindowMinimize, SIGNAL( triggered() ), this, SLOT( showActiveWindowMinimized() ) );

  mActionWindowZoom = new QAction( tr( "Zoom" ), this );
  shortcuts->registerAction( mActionWindowZoom );
  mActionWindowZoom->setStatusTip( tr( "Toggles between a predefined size and the window size set by the user" ) );
  connect( mActionWindowZoom, SIGNAL( triggered() ), this, SLOT( toggleActiveWindowMaximized() ) );

  mActionWindowAllToFront = new QAction( tr( "Bring All to Front" ), this );
  shortcuts->registerAction( mActionWindowAllToFront );
  mActionWindowAllToFront->setStatusTip( tr( "Bring forward all open windows" ) );
  connect( mActionWindowAllToFront, SIGNAL( triggered() ), this, SLOT( bringAllToFront() ) );

  // list of open windows
  mWindowActions = new QActionGroup( this );
#endif

  // Help Menu Items

  mActionHelpContents = new QAction( getThemeIcon( "mActionHelpContents.png" ), tr( "Help Contents" ), this );
#ifdef Q_WS_MAC
  shortcuts->registerAction( mActionHelpContents, tr( "Ctrl+?", "Help Documentation (Mac)" ) );
#else
  shortcuts->registerAction( mActionHelpContents, tr( "F1", "Help Documentation" ) );
#endif
  mActionHelpContents->setStatusTip( tr( "Help Documentation" ) );
  connect( mActionHelpContents, SIGNAL( triggered() ), this, SLOT( helpContents() ) );

  mActionQgisHomePage = new QAction( getThemeIcon( "mActionQgisHomePage.png" ), tr( "QGIS Home Page" ), this );
#ifndef Q_WS_MAC
  shortcuts->registerAction( mActionQgisHomePage, tr( "Ctrl+H", "QGIS Home Page" ) );
#else
  shortcuts->registerAction( mActionQgisHomePage );
#endif
  mActionQgisHomePage->setStatusTip( tr( "QGIS Home Page" ) );
  connect( mActionQgisHomePage, SIGNAL( triggered() ), this, SLOT( helpQgisHomePage() ) );

  mActionCheckQgisVersion = new QAction( getThemeIcon( "mActionCheckQgisVersion.png" ), tr( "Check Qgis Version" ), this );
  shortcuts->registerAction( mActionCheckQgisVersion );
  mActionCheckQgisVersion->setStatusTip( tr( "Check if your QGIS version is up to date (requires internet access)" ) );
  connect( mActionCheckQgisVersion, SIGNAL( triggered() ), this, SLOT( checkQgisVersion() ) );

  mActionAbout = new QAction( getThemeIcon( "mActionHelpAbout.png" ), tr( "About" ), this );
  shortcuts->registerAction( mActionAbout );
  mActionAbout->setStatusTip( tr( "About QGIS" ) );
  mActionAbout->setMenuRole( QAction::AboutRole ); // put in application menu on Mac OS X
  connect( mActionAbout, SIGNAL( triggered() ), this, SLOT( about() ) );

  mActionStyleManagerV2 = new QAction( tr( "Style manager..." ), this );
  shortcuts->registerAction( mActionStyleManagerV2 );
  mActionStyleManagerV2->setStatusTip( tr( "Show style manager V2" ) );
  connect( mActionStyleManagerV2, SIGNAL( triggered() ), this, SLOT( showStyleManagerV2() ) );
}

#include "qgsstylev2.h"
#include "qgsstylev2managerdialog.h"

void QgisApp::showStyleManagerV2()
{
  QgsStyleV2ManagerDialog dlg( QgsStyleV2::defaultStyle(), this );
  dlg.exec();
}

void QgisApp::writeAnnotationItemsToProject( QDomDocument& doc )
{
  QList<QgsAnnotationItem*> items = annotationItems();
  QList<QgsAnnotationItem*>::const_iterator itemIt = items.constBegin();
  for( ; itemIt != items.constEnd(); ++itemIt )
  {
    if( ! *itemIt )
    {
      continue;
    }
    ( *itemIt )->writeXML( doc );
  }
}

void QgisApp::showPythonDialog()
{
  if( !mPythonUtils || !mPythonUtils->isEnabled() )
    return;

  bool res = mPythonUtils->runStringUnsafe(
               "import qgis.console\n"
               "qgis.console.show_console()\n", false );

  if( !res )
  {
    QString className, text;
    mPythonUtils->getError( className, text );
    QMessageBox::critical( this, tr( "Error" ), tr( "Failed to open Python console:" ) + "\n" + className + ": " + text );
  }
#ifdef Q_WS_MAC
  else
  {
    addWindow( mActionShowPythonDialog );
  }
#endif
}

void QgisApp::createActionGroups()
{
  //
  // Map Tool Group
  mMapToolGroup = new QActionGroup( this );
  mActionPan->setCheckable( true );
  mMapToolGroup->addAction( mActionPan );
  mActionZoomIn->setCheckable( true );
  mMapToolGroup->addAction( mActionZoomIn );
  mActionZoomOut->setCheckable( true );
  mMapToolGroup->addAction( mActionZoomOut );
  mActionIdentify->setCheckable( true );
  mMapToolGroup->addAction( mActionIdentify );
  mActionSelect->setCheckable( true );
  mMapToolGroup->addAction( mActionSelect );
  mActionSelectRectangle->setCheckable( true );
  mMapToolGroup->addAction( mActionSelectRectangle );
  mActionSelectPolygon->setCheckable( true );
  mMapToolGroup->addAction( mActionSelectPolygon );
  mActionSelectFreehand->setCheckable( true );
  mMapToolGroup->addAction( mActionSelectFreehand );
  mActionSelectRadius->setCheckable( true );
  mMapToolGroup->addAction( mActionSelectRadius );
  mActionDeselectAll->setCheckable( false );
  mMapToolGroup->addAction( mActionDeselectAll );
  mActionMeasure->setCheckable( true );
  mMapToolGroup->addAction( mActionMeasure );
  mActionMeasureArea->setCheckable( true );
  mMapToolGroup->addAction( mActionMeasureArea );
  mActionMeasureAngle->setCheckable( true );
  mMapToolGroup->addAction( mActionMeasureAngle );
  mActionCaptureLine->setCheckable( true );
  mMapToolGroup->addAction( mActionCaptureLine );
  mActionCapturePoint->setCheckable( true );
  mMapToolGroup->addAction( mActionCapturePoint );
  mActionCapturePolygon->setCheckable( true );
  mMapToolGroup->addAction( mActionCapturePolygon );
  mActionMoveFeature->setCheckable( true );
  mMapToolGroup->addAction( mActionMoveFeature );
  mActionReshapeFeatures->setCheckable( true );
  mMapToolGroup->addAction( mActionReshapeFeatures );
  mActionSplitFeatures->setCheckable( true );
  mMapToolGroup->addAction( mActionSplitFeatures );
  mMapToolGroup->addAction( mActionDeleteSelected );
#if 0 //these three tools are deprecated - use node tool rather
  mActionAddVertex->setCheckable( true );
  mMapToolGroup->addAction( mActionAddVertex );
  mActionDeleteVertex->setCheckable( true );
  mMapToolGroup->addAction( mActionDeleteVertex );
  mActionMoveVertex->setCheckable( true );
  mMapToolGroup->addAction( mActionMoveVertex );
#endif
  mActionAddRing->setCheckable( true );
  mMapToolGroup->addAction( mActionAddRing );
  mActionAddIsland->setCheckable( true );
  mMapToolGroup->addAction( mActionAddIsland );
  mActionSimplifyFeature->setCheckable( true );
  mMapToolGroup->addAction( mActionSimplifyFeature );
  mActionDeleteRing->setCheckable( true );
  mMapToolGroup->addAction( mActionDeleteRing );
  mActionDeletePart->setCheckable( true );
  mMapToolGroup->addAction( mActionDeletePart );
  mMapToolGroup->addAction( mActionMergeFeatures );
  mActionNodeTool->setCheckable( true );
  mMapToolGroup->addAction( mActionNodeTool );
  mActionRotatePointSymbols->setCheckable( true );
  mMapToolGroup->addAction( mActionRotatePointSymbols );
}

void QgisApp::createMenus()
{
  /*
   * The User Interface Guidelines for each platform specify different locations
   * for the following items.
   *
   * Project Properties:
   * Gnome, Mac - File menu above print commands
   * Kde, Win - Settings menu (Win doesn't specify)
   *
   * Custom CRS, Options:
   * Gnome - bottom of Edit menu
   * Mac - Application menu (moved automatically by Qt)
   * Kde, Win - Settings menu (Win should use Tools menu but we don't have one)
   *
   * Panel and Toolbar submenus, Toggle Full Screen:
   * Gnome, Mac, Win - View menu
   * Kde - Settings menu
   *
   * For Mac, About and Exit are also automatically moved by Qt to the Application menu.
   */

  // Get platform for menu layout customization (Gnome, Kde, Mac, Win)
  QDialogButtonBox::ButtonLayout layout =
    QDialogButtonBox::ButtonLayout( style()->styleHint( QStyle::SH_DialogButtonLayout, 0, this ) );

  // File Menu

  mFileMenu = menuBar()->addMenu( tr( "&File" ) );

  mFileMenu->addAction( mActionNewProject );
  mFileMenu->addAction( mActionOpenProject );
  mRecentProjectsMenu = mFileMenu->addMenu( tr( "&Open Recent Projects" ) );
  // Connect once for the entire submenu.
  connect( mRecentProjectsMenu, SIGNAL( triggered( QAction * ) ),
           this, SLOT( openProject( QAction * ) ) );
  mActionFileSeparator1 = mFileMenu->addSeparator();

  mFileMenu->addAction( mActionSaveProject );
  mFileMenu->addAction( mActionSaveProjectAs );
  mFileMenu->addAction( mActionSaveMapAsImage );
  mActionFileSeparator2 = mFileMenu->addSeparator();

  if( layout == QDialogButtonBox::GnomeLayout || layout == QDialogButtonBox::MacLayout )
  {
    mFileMenu->addAction( mActionProjectProperties );
    mActionFileSeparator3 = mFileMenu->addSeparator();
  }

  mFileMenu->addAction( mActionNewPrintComposer );
  mFileMenu->addAction( mActionShowComposerManager );
  mPrintComposersMenu = mFileMenu->addMenu( tr( "Print Composers" ) );
  mActionFileSeparator4 = mFileMenu->addSeparator();

  mFileMenu->addAction( mActionExit );

  // Edit Menu

  mEditMenu = menuBar()->addMenu( tr( "&Edit" ) );

#if 0
  mEditMenu->addAction( mActionCut );
  mEditMenu->addAction( mActionCopy );
  mEditMenu->addAction( mActionPaste );
#endif
  mEditMenu->addAction( mActionUndo );
  mEditMenu->addAction( mActionRedo );
  mActionEditSeparator0 = mEditMenu->addSeparator();

  mEditMenu->addAction( mActionCutFeatures );
  mEditMenu->addAction( mActionCopyFeatures );
  mEditMenu->addAction( mActionPasteFeatures );
  mActionEditSeparator1 = mEditMenu->addSeparator();

  mEditMenu->addAction( mActionCapturePoint );
  mEditMenu->addAction( mActionCaptureLine );
  mEditMenu->addAction( mActionCapturePolygon );
  mEditMenu->addAction( mActionMoveFeature );
  mEditMenu->addAction( mActionDeleteSelected );
#if 0 //these three tools are deprecated - use node tool rather
  mEditMenu->addAction( mActionAddVertex );
  mEditMenu->addAction( mActionMoveVertex );
  mEditMenu->addAction( mActionDeleteVertex );
#endif

  mActionEditSeparator2 = mEditMenu->addSeparator();

  mEditMenu->addAction( mActionSimplifyFeature );
  mEditMenu->addAction( mActionAddRing );
  mEditMenu->addAction( mActionAddIsland );
  mEditMenu->addAction( mActionDeleteRing );
  mEditMenu->addAction( mActionDeletePart );
  mEditMenu->addAction( mActionReshapeFeatures );
  mEditMenu->addAction( mActionSplitFeatures );
  mEditMenu->addAction( mActionMergeFeatures );
  mEditMenu->addAction( mActionNodeTool );
  mEditMenu->addAction( mActionRotatePointSymbols );

  if( layout == QDialogButtonBox::GnomeLayout || layout == QDialogButtonBox::MacLayout )
  {
    mActionEditSeparator3 = mEditMenu->addSeparator();
    mEditMenu->addAction( mActionOptions );
    mEditMenu->addAction( mActionConfigureShortcuts );
    mEditMenu->addAction( mActionStyleManagerV2 );
    mEditMenu->addAction( mActionCustomProjection );
  }

  // Panel and Toolbar Submenus

  mPanelMenu = new QMenu( tr( "Panels" ) );
  mToolbarMenu = new QMenu( tr( "Toolbars" ) );

  // View Menu

  mViewMenu = menuBar()->addMenu( tr( "&View" ) );

  mViewMenu->addAction( mActionPan );
  mViewMenu->addAction( mActionZoomIn );
  mViewMenu->addAction( mActionZoomOut );
  mActionViewSeparator1 = mViewMenu->addSeparator();

  QMenu *menu = mViewMenu->addMenu( tr( "Select" ) );
  menu->addAction( mActionSelect );
  menu->addAction( mActionSelectRectangle );
  menu->addAction( mActionSelectPolygon );
  menu->addAction( mActionSelectFreehand );
  menu->addAction( mActionSelectRadius );
  menu->addAction( mActionDeselectAll );

  mViewMenu->addAction( mActionIdentify );

  menu = mViewMenu->addMenu( tr( "Measure" ) );
  menu->addAction( mActionMeasure );
  menu->addAction( mActionMeasureArea );
  menu->addAction( mActionMeasureAngle );

  mActionViewSeparator3 = mViewMenu->addSeparator();

  mViewMenu->addAction( mActionZoomFullExtent );
  mViewMenu->addAction( mActionZoomToLayer );
  mViewMenu->addAction( mActionZoomToSelected );
  mViewMenu->addAction( mActionZoomLast );
  mViewMenu->addAction( mActionZoomNext );
  mViewMenu->addAction( mActionZoomActualSize );
  mActionViewSeparator4 = mViewMenu->addSeparator();

  mViewMenu->addAction( mActionMapTips );
  mViewMenu->addAction( mActionNewBookmark );
  mViewMenu->addAction( mActionShowBookmarks );
  mViewMenu->addAction( mActionDraw );

  if( layout != QDialogButtonBox::KdeLayout )
  {
    mActionViewSeparator5 = mViewMenu->addSeparator();
    mViewMenu->addMenu( mPanelMenu );
    mViewMenu->addMenu( mToolbarMenu );
    mViewMenu->addAction( mActionToggleFullScreen );
  }

  mViewMenu->addAction( mActionTileScale );

  mViewMenu->addAction( mActionGpsTool );

  // Layers Menu

  mLayerMenu = menuBar()->addMenu( tr( "&Layer" ) );

#ifdef HAVE_SPATIALITE
  QMenu *newLayerMenu = mLayerMenu->addMenu( tr( "New" ) );
  newLayerMenu->addAction( mActionNewVectorLayer );
  newLayerMenu->addAction( mActionNewSpatialiteLayer );
#else
  mLayerMenu->addAction( mActionNewVectorLayer );
#endif

  //todo: should probably go into a raster menu
  mLayerMenu->addAction( mActionShowRasterCalculator );

  mLayerMenu->addAction( mActionAddOgrLayer );
  mLayerMenu->addAction( mActionAddRasterLayer );
#ifdef HAVE_POSTGRESQL
  mLayerMenu->addAction( mActionAddPgLayer );
#endif
#ifdef HAVE_SPATIALITE
  mLayerMenu->addAction( mActionAddSpatiaLiteLayer );
#endif
  mLayerMenu->addAction( mActionAddWmsLayer );
  mActionLayerSeparator1 = mLayerMenu->addSeparator();

  mLayerMenu->addAction( mActionOpenTable );
  mLayerMenu->addAction( mActionSaveEdits );
  mLayerMenu->addAction( mActionToggleEditing );
  mLayerMenu->addAction( mActionLayerSaveAs );
  mLayerMenu->addAction( mActionLayerSelectionSaveAs );
  mLayerMenu->addAction( mActionRemoveLayer );
  mLayerMenu->addAction( mActionLayerProperties );
  mLayerMenu->addAction( mActionLayerSubsetString );
  mActionLayerSeparator2 = mLayerMenu->addSeparator();

  mLayerMenu->addAction( mActionAddToOverview );
  mLayerMenu->addAction( mActionAddAllToOverview );
  mLayerMenu->addAction( mActionRemoveAllFromOverview );
  mActionLayerSeparator3 = mLayerMenu->addSeparator();

  mLayerMenu->addAction( mActionHideAllLayers );
  mLayerMenu->addAction( mActionShowAllLayers );
  mLayerMenu->addAction( mActionLabeling );

  // Settings Menu

#ifndef Q_WS_MAC
  if( layout == QDialogButtonBox::KdeLayout || layout == QDialogButtonBox::WinLayout )
  {
    mSettingsMenu = menuBar()->addMenu( tr( "&Settings" ) );

#ifndef Q_WS_WIN
    mSettingsMenu->addMenu( mPanelMenu );
    mSettingsMenu->addMenu( mToolbarMenu );
    mSettingsMenu->addAction( mActionToggleFullScreen );
    mActionSettingsSeparator1 = mSettingsMenu->addSeparator();
#endif

    mSettingsMenu->addAction( mActionProjectProperties );
    mSettingsMenu->addAction( mActionCustomProjection );
    mSettingsMenu->addAction( mActionStyleManagerV2 );
    mSettingsMenu->addAction( mActionConfigureShortcuts );
    mSettingsMenu->addAction( mActionOptions );
  }
#endif

  // Plugins Menu

  mPluginMenu = menuBar()->addMenu( tr( "&Plugins" ) );

  mPluginMenu->addAction( mActionManagePlugins );
  mActionPluginSeparator1 = NULL;  // plugin list separator will be created when the first plugin is loaded
  mActionPluginSeparator2 = NULL;  // python separator will be created only if python is found

#ifdef Q_WS_MAC
  // Window Menu

  mWindowMenu = menuBar()->addMenu( tr( "&Window" ) );

  mWindowMenu->addAction( mActionWindowMinimize );
  mWindowMenu->addAction( mActionWindowZoom );
  mActionWindowSeparator1 = mWindowMenu->addSeparator();

  mWindowMenu->addAction( mActionWindowAllToFront );
  mActionWindowSeparator2 = mWindowMenu->addSeparator();
#endif

  // Help Menu

  menuBar()->addSeparator();
  mHelpMenu = menuBar()->addMenu( tr( "&Help" ) );

  mHelpMenu->addAction( mActionHelpContents );
  mActionHelpSeparator1 = mHelpMenu->addSeparator();

  mHelpMenu->addAction( mActionQgisHomePage );
  mHelpMenu->addAction( mActionCheckQgisVersion );
  mActionHelpSeparator2 = mHelpMenu->addSeparator();

  mHelpMenu->addAction( mActionAbout );
}

void QgisApp::createToolBars()
{
  QSize myIconSize( 24, 24 );
  // QSize myIconSize ( 32,32 ); //large icons
  // Note: we need to set each object name to ensure that
  // qmainwindow::saveState and qmainwindow::restoreState
  // work properly

  //
  // File Toolbar
  mFileToolBar = addToolBar( tr( "File" ) );
  mFileToolBar->setIconSize( myIconSize );
  mFileToolBar->setObjectName( "FileToolBar" );
  mFileToolBar->addAction( mActionNewProject );
  mFileToolBar->addAction( mActionOpenProject );
  mFileToolBar->addAction( mActionSaveProject );
  mFileToolBar->addAction( mActionSaveProjectAs );
  mFileToolBar->addAction( mActionNewPrintComposer );
  mFileToolBar->addAction( mActionShowComposerManager );
  mToolbarMenu->addAction( mFileToolBar->toggleViewAction() );
  //
  // Layer Toolbar
  mLayerToolBar = addToolBar( tr( "Manage Layers" ) );
  mLayerToolBar->setIconSize( myIconSize );
  mLayerToolBar->setObjectName( "LayerToolBar" );
  mLayerToolBar->addAction( mActionAddOgrLayer );
  mLayerToolBar->addAction( mActionAddRasterLayer );
#ifdef HAVE_POSTGRESQL
  mLayerToolBar->addAction( mActionAddPgLayer );
#endif
#ifdef HAVE_SPATIALITE
  mLayerToolBar->addAction( mActionAddSpatiaLiteLayer );
#endif
  mLayerToolBar->addAction( mActionAddWmsLayer );
  mLayerToolBar->addAction( mActionNewVectorLayer );
  mLayerToolBar->addAction( mActionRemoveLayer );
  //commented out for QGIS 1.4 by Tim
  //mLayerToolBar->addAction( mActionAddToOverview );
  //mLayerToolBar->addAction( mActionShowAllLayers );
  //mLayerToolBar->addAction( mActionHideAllLayers );
  mToolbarMenu->addAction( mLayerToolBar->toggleViewAction() );
  //
  // Digitizing Toolbar
  mDigitizeToolBar = addToolBar( tr( "Digitizing" ) );
  mDigitizeToolBar->setIconSize( myIconSize );
  mDigitizeToolBar->setObjectName( "Digitizing" );
  mDigitizeToolBar->addAction( mActionToggleEditing );
  mDigitizeToolBar->addAction( mActionSaveEdits );
  mDigitizeToolBar->addAction( mActionCapturePoint );
  mDigitizeToolBar->addAction( mActionCaptureLine );
  mDigitizeToolBar->addAction( mActionCapturePolygon );
  mDigitizeToolBar->addAction( mActionMoveFeature );
  mDigitizeToolBar->addAction( mActionNodeTool );
#if 0 //these three tools are deprecated - use node tool rather
  mDigitizeToolBar->addAction( mActionMoveVertex );
  mDigitizeToolBar->addAction( mActionAddVertex );
  mDigitizeToolBar->addAction( mActionDeleteVertex );
#endif
  mDigitizeToolBar->addAction( mActionDeleteSelected );
  mDigitizeToolBar->addAction( mActionCutFeatures );
  mDigitizeToolBar->addAction( mActionCopyFeatures );
  mDigitizeToolBar->addAction( mActionPasteFeatures );
  mToolbarMenu->addAction( mDigitizeToolBar->toggleViewAction() );

  mAdvancedDigitizeToolBar = addToolBar( tr( "Advanced Digitizing" ) );
  mAdvancedDigitizeToolBar->setIconSize( myIconSize );
  mAdvancedDigitizeToolBar->setObjectName( "Advanced Digitizing" );
  mAdvancedDigitizeToolBar->addAction( mActionUndo );
  mAdvancedDigitizeToolBar->addAction( mActionRedo );
  mAdvancedDigitizeToolBar->addAction( mActionSimplifyFeature );
  mAdvancedDigitizeToolBar->addAction( mActionAddRing );
  mAdvancedDigitizeToolBar->addAction( mActionAddIsland );
  mAdvancedDigitizeToolBar->addAction( mActionDeleteRing );
  mAdvancedDigitizeToolBar->addAction( mActionDeletePart );
  mAdvancedDigitizeToolBar->addAction( mActionReshapeFeatures );
  mAdvancedDigitizeToolBar->addAction( mActionSplitFeatures );
  mAdvancedDigitizeToolBar->addAction( mActionMergeFeatures );
  mAdvancedDigitizeToolBar->addAction( mActionRotatePointSymbols );
  mToolbarMenu->addAction( mAdvancedDigitizeToolBar->toggleViewAction() );


  //
  // Map Navigation Toolbar
  mMapNavToolBar = addToolBar( tr( "Map Navigation" ) );
  mMapNavToolBar->setIconSize( myIconSize );
  mMapNavToolBar->setObjectName( "Map Navigation" );
  mMapNavToolBar->addAction( mActionPan );
  mMapNavToolBar->addAction( mActionZoomIn );
  mMapNavToolBar->addAction( mActionZoomOut );
  mMapNavToolBar->addAction( mActionZoomFullExtent );
  mMapNavToolBar->addAction( mActionZoomToSelected );
  mMapNavToolBar->addAction( mActionZoomToLayer );
  mMapNavToolBar->addAction( mActionZoomLast );
  mMapNavToolBar->addAction( mActionZoomNext );
  mMapNavToolBar->addAction( mActionDraw );
  mToolbarMenu->addAction( mMapNavToolBar->toggleViewAction() );

  //
  // Attributes Toolbar
  mAttributesToolBar = addToolBar( tr( "Attributes" ) );
  mAttributesToolBar->setIconSize( myIconSize );
  mAttributesToolBar->setObjectName( "Attributes" );
  mAttributesToolBar->addAction( mActionIdentify );

  QToolButton *bt = new QToolButton( mAttributesToolBar );
  QMenu *menu = new QMenu( bt );
  bt->setMenu( menu );
  menu->addAction( mActionSelect );
  menu->addAction( mActionSelectRectangle );
  menu->addAction( mActionSelectPolygon );
  menu->addAction( mActionSelectFreehand );
  menu->addAction( mActionSelectRadius );
  bt->setDefaultAction( mActionSelect );
  mAttributesToolBar->addWidget( bt );
  connect( bt, SIGNAL( triggered( QAction * ) ), this, SLOT( toolButtonActionTriggered( QAction * ) ) );

  mAttributesToolBar->addAction( mActionDeselectAll );
  mAttributesToolBar->addAction( mActionOpenTable );

  bt = new QToolButton( mAttributesToolBar );
  menu = new QMenu( bt );
  bt->setMenu( menu );
  menu->addAction( mActionMeasure );
  menu->addAction( mActionMeasureArea );
  menu->addAction( mActionMeasureAngle );
  bt->setDefaultAction( mActionMeasure );
  mAttributesToolBar->addWidget( bt );
  connect( bt, SIGNAL( triggered( QAction * ) ), this, SLOT( toolButtonActionTriggered( QAction * ) ) );

  mAttributesToolBar->addAction( mActionMapTips );
  mAttributesToolBar->addAction( mActionShowBookmarks );
  mAttributesToolBar->addAction( mActionNewBookmark );
  mAttributesToolBar->addAction( mActionLabeling );

  // Annotation tools
  QToolButton *annotationToolButton = new QToolButton();
  annotationToolButton->setPopupMode( QToolButton::InstantPopup );
  annotationToolButton->setAutoRaise( true );
  annotationToolButton->setToolButtonStyle( Qt::ToolButtonIconOnly );
  annotationToolButton->setCheckable( true );
  annotationToolButton->addAction( mActionTextAnnotation );
  annotationToolButton->addAction( mActionFormAnnotation );
  annotationToolButton->addAction( mActionAnnotation );
  annotationToolButton->setDefaultAction( mActionTextAnnotation );
  connect( annotationToolButton, SIGNAL( triggered( QAction* ) ),
           annotationToolButton, SLOT( setDefaultAction( QAction* ) ) );
  mAttributesToolBar->addWidget( annotationToolButton );
  mToolbarMenu->addAction( mAttributesToolBar->toggleViewAction() );
  //
  // Plugins Toolbar
  mPluginToolBar = addToolBar( tr( "Plugins" ) );
  mPluginToolBar->setIconSize( myIconSize );
  mPluginToolBar->setObjectName( "Plugins" );
  mToolbarMenu->addAction( mPluginToolBar->toggleViewAction() );
  //
  // Help Toolbar
  mHelpToolBar = addToolBar( tr( "Help" ) );
  mHelpToolBar->setIconSize( myIconSize );
  mHelpToolBar->setObjectName( "Help" );
  mHelpToolBar->addAction( mActionHelpContents );
  mHelpToolBar->addAction( QWhatsThis::createAction() );
  mToolbarMenu->addAction( mHelpToolBar->toggleViewAction() );
}

void QgisApp::createStatusBar()
{
  //
  // Add a panel to the status bar for the scale, coords and progress
  // And also rendering suppression checkbox
  //
  mProgressBar = new QProgressBar( statusBar() );
  mProgressBar->setMaximumWidth( 100 );
  mProgressBar->hide();
  mProgressBar->setWhatsThis( tr( "Progress bar that displays the status "
                                  "of rendering layers and other time-intensive operations" ) );
  statusBar()->addPermanentWidget( mProgressBar, 1 );
  // Bumped the font up one point size since 8 was too
  // small on some platforms. A point size of 9 still provides
  // plenty of display space on 1024x768 resolutions
  QFont myFont( "Arial", 9 );

  statusBar()->setFont( myFont );
  //toggle to switch between mouse pos and extents display in status bar widget
  mToggleExtentsViewButton = new QToolButton( statusBar() );
  mToggleExtentsViewButton->setMaximumWidth( 20 );
  mToggleExtentsViewButton->setMaximumHeight( 20 );
  mToggleExtentsViewButton->setIcon( getThemeIcon( "tracking.png" ) );
  mToggleExtentsViewButton->setToolTip( tr( "Toggle extents and mouse position display" ) );
  mToggleExtentsViewButton->setCheckable( true );
  connect( mToggleExtentsViewButton, SIGNAL( toggled( bool ) ), this, SLOT( extentsViewToggled( bool ) ) );
  statusBar()->addPermanentWidget( mToggleExtentsViewButton, 0 );

  // add a label to show current scale
  mCoordsLabel = new QLabel( QString(), statusBar() );
  mCoordsLabel->setFont( myFont );
  mCoordsLabel->setMinimumWidth( 10 );
  mCoordsLabel->setMaximumHeight( 20 );
  mCoordsLabel->setMargin( 3 );
  mCoordsLabel->setAlignment( Qt::AlignCenter );
  mCoordsLabel->setFrameStyle( QFrame::NoFrame );
  mCoordsLabel->setText( tr( "Coordinate:" ) );
  mCoordsLabel->setToolTip( tr( "Current map coordinate" ) );
  statusBar()->addPermanentWidget( mCoordsLabel, 0 );

  //coords status bar widget
  mCoordsEdit = new QLineEdit( QString(), statusBar() );
  mCoordsEdit->setFont( myFont );
  mCoordsEdit->setMinimumWidth( 10 );
  mCoordsEdit->setMaximumWidth( 200 );
  mCoordsEdit->setMaximumHeight( 20 );
  mCoordsEdit->setContentsMargins( 0, 0, 0, 0 );
  mCoordsEdit->setAlignment( Qt::AlignCenter );
  mCoordsEdit->setAlignment( Qt::AlignCenter );
  QRegExp coordValidator( "[+-]?\\d+\\.?\\d*\\s*,\\s*[+-]?\\d+\\.?\\d*" );
  mCoordsEditValidator = new QRegExpValidator( coordValidator, mCoordsEdit );
  mCoordsEdit->setWhatsThis( tr( "Shows the map coordinates at the "
                                 "current cursor position. The display is continuously updated "
                                 "as the mouse is moved. It also allows editing to set the canvas "
                                 "center to a given position." ) );
  mCoordsEdit->setToolTip( tr( "Current map coordinate (formatted as x,y)" ) );
  statusBar()->addPermanentWidget( mCoordsEdit, 0 );
  connect( mCoordsEdit, SIGNAL( editingFinished() ), this, SLOT( userCenter() ) );

  // add a label to show current scale
  mScaleLabel = new QLabel( QString(), statusBar() );
  mScaleLabel->setFont( myFont );
  mScaleLabel->setMinimumWidth( 10 );
  mScaleLabel->setMaximumHeight( 20 );
  mScaleLabel->setMargin( 3 );
  mScaleLabel->setAlignment( Qt::AlignCenter );
  mScaleLabel->setFrameStyle( QFrame::NoFrame );
  mScaleLabel->setText( tr( "Scale " ) );
  mScaleLabel->setToolTip( tr( "Current map scale" ) );
  statusBar()->addPermanentWidget( mScaleLabel, 0 );

  mScaleEdit = new QLineEdit( QString(), statusBar() );
  mScaleEdit->setFont( myFont );
  mScaleEdit->setMinimumWidth( 10 );
  mScaleEdit->setMaximumWidth( 100 );
  mScaleEdit->setMaximumHeight( 20 );
  mScaleEdit->setContentsMargins( 0, 0, 0, 0 );
  mScaleEdit->setAlignment( Qt::AlignLeft );
  // QRegExp validator( "\\d+\\.?\\d*:\\d+\\.?\\d*" );
  QRegExp validator( "\\d+\\.?\\d*:\\d+\\.?\\d*|\\d+\\.?\\d*" );
  mScaleEditValidator = new QRegExpValidator( validator, mScaleEdit );
  mScaleEdit->setValidator( mScaleEditValidator );
  mScaleEdit->setWhatsThis( tr( "Displays the current map scale" ) );
  mScaleEdit->setToolTip( tr( "Current map scale (formatted as x:y)" ) );
  statusBar()->addPermanentWidget( mScaleEdit, 0 );
  connect( mScaleEdit, SIGNAL( editingFinished() ), this, SLOT( userScale() ) );

  //stop rendering status bar widget
  mStopRenderButton = new QToolButton( statusBar() );
  mStopRenderButton->setMaximumWidth( 20 );
  mStopRenderButton->setMaximumHeight( 20 );
  mStopRenderButton->setIcon( getThemeIcon( "mIconDelete.png" ) );
  mStopRenderButton->setToolTip( tr( "Stop map rendering" ) );
  statusBar()->addPermanentWidget( mStopRenderButton, 0 );
  // render suppression status bar widget
  mRenderSuppressionCBox = new QCheckBox( tr( "Render" ), statusBar() );
  mRenderSuppressionCBox->setChecked( true );
  mRenderSuppressionCBox->setFont( myFont );
  mRenderSuppressionCBox->setWhatsThis( tr( "When checked, the map layers "
                                        "are rendered in response to map navigation commands and other "
                                        "events. When not checked, no rendering is done. This allows you "
                                        "to add a large number of layers and symbolize them before rendering." ) );
  mRenderSuppressionCBox->setToolTip( tr( "Toggle map rendering" ) );
  statusBar()->addPermanentWidget( mRenderSuppressionCBox, 0 );
  // On the fly projection status bar icon
  // Changed this to a tool button since a QPushButton is
  // sculpted on OS X and the icon is never displayed [gsherman]
  mOnTheFlyProjectionStatusButton = new QToolButton( statusBar() );
  mOnTheFlyProjectionStatusButton->setMaximumWidth( 20 );
  // Maintain uniform widget height in status bar by setting button height same as labels
  // For Qt/Mac 3.3, the default toolbutton height is 30 and labels were expanding to match
  mOnTheFlyProjectionStatusButton->setMaximumHeight( mScaleLabel->height() );
  mOnTheFlyProjectionStatusButton->setIcon( getThemeIcon( "mIconProjectionDisabled.png" ) );
  if( !QFile::exists( QgsApplication::defaultThemePath() + "/mIconProjectionDisabled.png" ) )
  {
    QMessageBox::critical( this, tr( "Resource Location Error" ),
                           tr( "Error reading icon resources from: \n %1\n Quitting..." ).arg(
                             QgsApplication::defaultThemePath() + "/mIconProjectionDisabled.png"
                           ) );
    exit( 0 );
  }
  mOnTheFlyProjectionStatusButton->setWhatsThis( tr( "This icon shows whether "
      "on the fly coordinate reference system transformation is enabled or not. "
      "Click the icon to bring up "
      "the project properties dialog to alter this behaviour." ) );
  mOnTheFlyProjectionStatusButton->setToolTip( tr( "CRS status - Click "
      "to open coordinate reference system dialog" ) );
  connect( mOnTheFlyProjectionStatusButton, SIGNAL( clicked() ),
           this, SLOT( projectPropertiesProjections() ) );//bring up the project props dialog when clicked
  statusBar()->addPermanentWidget( mOnTheFlyProjectionStatusButton, 0 );
  statusBar()->showMessage( tr( "Ready" ) );
}


void QgisApp::setTheme( QString theThemeName )
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
  */
  QgsApplication::setThemeName( theThemeName );
  //QgsDebugMsg("Setting theme to \n" + theThemeName);
  mActionNewProject->setIcon( getThemeIcon( "/mActionFileNew.png" ) );
  mActionOpenProject->setIcon( getThemeIcon( "/mActionFileOpen.png" ) );
  mActionSaveProject->setIcon( getThemeIcon( "/mActionFileSave.png" ) );
  mActionSaveProjectAs->setIcon( getThemeIcon( "/mActionFileSaveAs.png" ) );
  mActionNewPrintComposer->setIcon( getThemeIcon( "/mActionNewComposer.png" ) );
  mActionShowComposerManager->setIcon( getThemeIcon( "/mActionComposerManager.png" ) );
  mActionSaveMapAsImage->setIcon( getThemeIcon( "/mActionSaveMapAsImage.png" ) );
  mActionExit->setIcon( getThemeIcon( "/mActionFileExit.png" ) );
  mActionAddOgrLayer->setIcon( getThemeIcon( "/mActionAddOgrLayer.png" ) );
  mActionAddRasterLayer->setIcon( getThemeIcon( "/mActionAddRasterLayer.png" ) );
  mActionAddPgLayer->setIcon( getThemeIcon( "/mActionAddLayer.png" ) );
  mActionAddSpatiaLiteLayer->setIcon( getThemeIcon( "/mActionAddSpatiaLiteLayer.png" ) );
  mActionRemoveLayer->setIcon( getThemeIcon( "/mActionRemoveLayer.png" ) );
  mActionNewVectorLayer->setIcon( getThemeIcon( "/mActionNewVectorLayer.png" ) );
  mActionAddAllToOverview->setIcon( getThemeIcon( "/mActionAddAllToOverview.png" ) );
  mActionHideAllLayers->setIcon( getThemeIcon( "/mActionHideAllLayers.png" ) );
  mActionShowAllLayers->setIcon( getThemeIcon( "/mActionShowAllLayers.png" ) );
  mActionRemoveAllFromOverview->setIcon( getThemeIcon( "/mActionRemoveAllFromOverview.png" ) );
  mActionToggleFullScreen->setIcon( getThemeIcon( "/mActionToggleFullScreen.png" ) );
  mActionProjectProperties->setIcon( getThemeIcon( "/mActionProjectProperties.png" ) );
  mActionManagePlugins->setIcon( getThemeIcon( "/mActionShowPluginManager.png" ) );
  mActionCheckQgisVersion->setIcon( getThemeIcon( "/mActionCheckQgisVersion.png" ) );
  mActionOptions->setIcon( getThemeIcon( "/mActionOptions.png" ) );
  mActionConfigureShortcuts->setIcon( getThemeIcon( "/mActionOptions.png" ) );
  mActionHelpContents->setIcon( getThemeIcon( "/mActionHelpContents.png" ) );
  mActionQgisHomePage->setIcon( getThemeIcon( "/mActionQgisHomePage.png" ) );
  mActionAbout->setIcon( getThemeIcon( "/mActionHelpAbout.png" ) );
  mActionDraw->setIcon( getThemeIcon( "/mActionDraw.png" ) );
  mActionToggleEditing->setIcon( getThemeIcon( "/mActionToggleEditing.png" ) );
  mActionSaveEdits->setIcon( getThemeIcon( "/mActionSaveEdits.png" ) );
  mActionCutFeatures->setIcon( getThemeIcon( "/mActionEditCut.png" ) );
  mActionCopyFeatures->setIcon( getThemeIcon( "/mActionEditCopy.png" ) );
  mActionPasteFeatures->setIcon( getThemeIcon( "/mActionEditPaste.png" ) );
  mActionCapturePoint->setIcon( getThemeIcon( "/mActionCapturePoint.png" ) );
  mActionCaptureLine->setIcon( getThemeIcon( "/mActionCaptureLine.png" ) );
  mActionCapturePolygon->setIcon( getThemeIcon( "/mActionCapturePolygon.png" ) );
  mActionMoveFeature->setIcon( getThemeIcon( "/mActionMoveFeature.png" ) );
  mActionReshapeFeatures->setIcon( getThemeIcon( "/mActionReshape.png" ) );
  mActionSplitFeatures->setIcon( getThemeIcon( "/mActionSplitFeatures.png" ) );
  mActionDeleteSelected->setIcon( getThemeIcon( "/mActionDeleteSelected.png" ) );
#if 0 //these three icons to be deprecated
  mActionAddVertex->setIcon( getThemeIcon( "/mActionAddVertex.png" ) );
  mActionMoveVertex->setIcon( getThemeIcon( "/mActionMoveVertex.png" ) );
  mActionDeleteVertex->setIcon( getThemeIcon( "/mActionDeleteVertex.png" ) );
#endif
  mActionNodeTool->setIcon( getThemeIcon( "/mActionNodeTool.png" ) );
  mActionSimplifyFeature->setIcon( getThemeIcon( "/mActionSimplify.png" ) );
  mActionUndo->setIcon( getThemeIcon( "/mActionUndo.png" ) );
  mActionRedo->setIcon( getThemeIcon( "/mActionRedo.png" ) );
  mActionAddRing->setIcon( getThemeIcon( "/mActionAddRing.png" ) );
  mActionAddIsland->setIcon( getThemeIcon( "/mActionAddIsland.png" ) );
  mActionDeleteRing->setIcon( getThemeIcon( "/mActionDeleteRing.png" ) );
  mActionDeletePart->setIcon( getThemeIcon( "/mActionDeletePart.png" ) );
  mActionMergeFeatures->setIcon( getThemeIcon( "/mActionMergeFeatures.png" ) );
  mActionRotatePointSymbols->setIcon( getThemeIcon( "mActionRotatePointSymbols.png" ) );
  mActionZoomIn->setIcon( getThemeIcon( "/mActionZoomIn.png" ) );
  mActionZoomOut->setIcon( getThemeIcon( "/mActionZoomOut.png" ) );
  mActionZoomFullExtent->setIcon( getThemeIcon( "/mActionZoomFullExtent.png" ) );
  mActionZoomToSelected->setIcon( getThemeIcon( "/mActionZoomToSelected.png" ) );
  mActionPan->setIcon( getThemeIcon( "/mActionPan.png" ) );
  mActionZoomLast->setIcon( getThemeIcon( "/mActionZoomLast.png" ) );
  mActionZoomNext->setIcon( getThemeIcon( "/mActionZoomNext.png" ) );
  mActionZoomToLayer->setIcon( getThemeIcon( "/mActionZoomToLayer.png" ) );
  mActionIdentify->setIcon( getThemeIcon( "/mActionIdentify.png" ) );
  mActionSelect->setIcon( getThemeIcon( "/mActionSelect.png" ) );
  mActionSelectRectangle->setIcon( getThemeIcon( "/mActionSelectRectangle.png" ) );
  mActionSelectPolygon->setIcon( getThemeIcon( "/mActionSelectPolygon.png" ) );
  mActionSelectFreehand->setIcon( getThemeIcon( "/mActionSelectFreehand.png" ) );
  mActionSelectRadius->setIcon( getThemeIcon( "/mActionSelectRadius.png" ) );
  mActionDeselectAll->setIcon( getThemeIcon( "/mActionDeselectAll.png" ) );
  mActionOpenTable->setIcon( getThemeIcon( "/mActionOpenTable.png" ) );
  mActionMeasure->setIcon( getThemeIcon( "/mActionMeasure.png" ) );
  mActionMeasureArea->setIcon( getThemeIcon( "/mActionMeasureArea.png" ) );
  mActionMeasureAngle->setIcon( getThemeIcon( "/mActionMeasureAngle.png" ) );
  mActionMapTips->setIcon( getThemeIcon( "/mActionMapTips.png" ) );
  mActionShowBookmarks->setIcon( getThemeIcon( "/mActionShowBookmarks.png" ) );
  mActionNewBookmark->setIcon( getThemeIcon( "/mActionNewBookmark.png" ) );
  mActionCustomProjection->setIcon( getThemeIcon( "/mActionCustomProjection.png" ) );
  mActionAddWmsLayer->setIcon( getThemeIcon( "/mActionAddWmsLayer.png" ) );
  mActionAddToOverview->setIcon( getThemeIcon( "/mActionInOverview.png" ) );
  mActionAnnotation->setIcon( getThemeIcon( "/mActionAnnotation.png" ) );
  mActionFormAnnotation->setIcon( getThemeIcon( "/mActionFormAnnotation.png" ) );
  mActionTextAnnotation->setIcon( getThemeIcon( "/mActionTextAnnotation.png" ) );

  //change themes of all composers
  QSet<QgsComposer*>::iterator composerIt = mPrintComposers.begin();
  for( ; composerIt != mPrintComposers.end(); ++composerIt )
  {
    ( *composerIt )->setupTheme();
  }

  emit currentThemeChanged( theThemeName );
}

void QgisApp::setupConnections()
{
  // connect the "cleanup" slot
  connect( qApp, SIGNAL( aboutToQuit() ), this, SLOT( saveWindowState() ) );

  // signal when mouse moved over window (coords display in status bar)
  connect( mMapCanvas, SIGNAL( xyCoordinates( const QgsPoint & ) ),
           this, SLOT( showMouseCoordinate( const QgsPoint & ) ) );
  connect( mMapCanvas, SIGNAL( extentsChanged() ),
           this, SLOT( showExtents() ) );
  connect( mMapCanvas, SIGNAL( scaleChanged( double ) ),
           this, SLOT( showScale( double ) ) );
  connect( mMapCanvas, SIGNAL( scaleChanged( double ) ),
           this, SLOT( updateMouseCoordinatePrecision() ) );
  connect( mMapCanvas, SIGNAL( mapToolSet( QgsMapTool * ) ),
           this, SLOT( mapToolChanged( QgsMapTool * ) ) );
  connect( mMapCanvas, SIGNAL( selectionChanged( QgsMapLayer * ) ),
           this, SLOT( selectionChanged( QgsMapLayer * ) ) );
  connect( mMapCanvas, SIGNAL( extentsChanged() ),
           this, SLOT( markDirty() ) );
  connect( mMapCanvas, SIGNAL( layersChanged() ),
           this, SLOT( markDirty() ) );

  connect( mMapCanvas, SIGNAL( zoomLastStatusChanged( bool ) ),
           mActionZoomLast, SLOT( setEnabled( bool ) ) );
  connect( mMapCanvas, SIGNAL( zoomNextStatusChanged( bool ) ),
           mActionZoomNext, SLOT( setEnabled( bool ) ) );
  connect( mRenderSuppressionCBox, SIGNAL( toggled( bool ) ),
           mMapCanvas, SLOT( setRenderFlag( bool ) ) );

  // connect renderer
  connect( mMapCanvas->mapRenderer(), SIGNAL( drawingProgress( int, int ) ),
           this, SLOT( showProgress( int, int ) ) );
  connect( mMapCanvas->mapRenderer(), SIGNAL( hasCrsTransformEnabled( bool ) ),
           this, SLOT( hasCrsTransformEnabled( bool ) ) );
  connect( mMapCanvas->mapRenderer(), SIGNAL( destinationSrsChanged() ),
           this, SLOT( destinationSrsChanged() ) );

  // connect legend signals
  connect( mMapLegend, SIGNAL( currentLayerChanged( QgsMapLayer * ) ),
           this, SLOT( activateDeactivateLayerRelatedActions( QgsMapLayer * ) ) );
  connect( mMapLegend, SIGNAL( itemSelectionChanged() ),
           this, SLOT( legendLayerSelectionChanged() ) );
  connect( mMapLegend, SIGNAL( zOrderChanged() ),
           this, SLOT( markDirty() ) );

  // connect map layer registry
  connect( QgsMapLayerRegistry::instance(), SIGNAL( layerWasAdded( QgsMapLayer * ) ),
           this, SLOT( layerWasAdded( QgsMapLayer * ) ) );
  connect( QgsMapLayerRegistry::instance(), SIGNAL( layerWillBeRemoved( QString ) ),
           this, SLOT( removingLayer( QString ) ) );

  // Connect warning dialog from project reading
  connect( QgsProject::instance(), SIGNAL( oldProjectVersionWarning( QString ) ),
           this, SLOT( oldProjectVersionWarning( QString ) ) );
  connect( QgsProject::instance(), SIGNAL( layerLoaded( int, int ) ),
           this, SLOT( showProgress( int, int ) ) );
  connect( QgsProject::instance(), SIGNAL( readProject( const QDomDocument & ) ),
           this, SLOT( readProject( const QDomDocument & ) ) );
  connect( QgsProject::instance(), SIGNAL( writeProject( QDomDocument & ) ),
           this, SLOT( writeProject( QDomDocument & ) ) );
  connect( QgsProject::instance(), SIGNAL( writeProject( QDomDocument& ) ),
           this, SLOT( writeAnnotationItemsToProject( QDomDocument& ) ) );

  connect( QgsProject::instance(), SIGNAL( readProject( const QDomDocument & ) ), this, SLOT( loadComposersFromProject( const QDomDocument& ) ) );
  connect( QgsProject::instance(), SIGNAL( readProject( const QDomDocument & ) ), this, SLOT( loadAnnotationItemsFromProject( const QDomDocument& ) ) );

  //
  // Do we really need this ??? - its already connected to the esc key...TS
  //
  connect( mStopRenderButton, SIGNAL( clicked() ),
           this, SLOT( stopRendering() ) );

  // setup undo/redo actions
  connect( mUndoWidget, SIGNAL( undoStackChanged() ), this, SLOT( updateUndoActions() ) );
}

void QgisApp::createCanvasTools()
{
  // create tools
  mMapTools.mZoomIn = new QgsMapToolZoom( mMapCanvas, false /* zoomIn */ );
  mMapTools.mZoomIn->setAction( mActionZoomIn );
  mMapTools.mZoomOut = new QgsMapToolZoom( mMapCanvas, true /* zoomOut */ );
  mMapTools.mZoomOut->setAction( mActionZoomOut );
  mMapTools.mPan = new QgsMapToolPan( mMapCanvas );
  mMapTools.mPan->setAction( mActionPan );
  mMapTools.mIdentify = new QgsMapToolIdentify( mMapCanvas );
  mMapTools.mIdentify->setAction( mActionIdentify );
  mMapTools.mMeasureDist = new QgsMeasureTool( mMapCanvas, false /* area */ );
  mMapTools.mMeasureDist->setAction( mActionMeasure );
  mMapTools.mMeasureArea = new QgsMeasureTool( mMapCanvas, true /* area */ );
  mMapTools.mMeasureArea->setAction( mActionMeasureArea );
  mMapTools.mMeasureAngle = new QgsMapToolMeasureAngle( mMapCanvas );
  mMapTools.mMeasureAngle->setAction( mActionMeasureAngle );
  mMapTools.mTextAnnotation = new QgsMapToolTextAnnotation( mMapCanvas );
  mMapTools.mTextAnnotation->setAction( mActionTextAnnotation );
  mMapTools.mFormAnnotation = new QgsMapToolFormAnnotation( mMapCanvas );
  mMapTools.mFormAnnotation->setAction( mActionFormAnnotation );
  mMapTools.mAnnotation = new QgsMapToolAnnotation( mMapCanvas );
  mMapTools.mAnnotation->setAction( mActionAnnotation );
  mMapTools.mCapturePoint = new QgsMapToolAddFeature( mMapCanvas, QgsMapToolCapture::CapturePoint );
  mMapTools.mCapturePoint->setAction( mActionCapturePoint );
  mActionCapturePoint->setVisible( false );
  mMapTools.mCaptureLine = new QgsMapToolAddFeature( mMapCanvas, QgsMapToolCapture::CaptureLine );
  mMapTools.mCaptureLine->setAction( mActionCaptureLine );
  mActionCaptureLine->setVisible( false );
  mMapTools.mCapturePolygon = new QgsMapToolAddFeature( mMapCanvas, QgsMapToolCapture::CapturePolygon );
  mMapTools.mCapturePolygon->setAction( mActionCapturePolygon );
  mActionCapturePolygon->setVisible( false );
  mMapTools.mMoveFeature = new QgsMapToolMoveFeature( mMapCanvas );
  mMapTools.mMoveFeature->setAction( mActionMoveFeature );
  mMapTools.mReshapeFeatures = new QgsMapToolReshape( mMapCanvas );
  mMapTools.mReshapeFeatures->setAction( mActionReshapeFeatures );
  mMapTools.mSplitFeatures = new QgsMapToolSplitFeatures( mMapCanvas );
  mMapTools.mSplitFeatures->setAction( mActionSplitFeatures );
  mMapTools.mSelect = new QgsMapToolSelect( mMapCanvas );
  mMapTools.mSelect->setAction( mActionSelect );
  mMapTools.mSelectRectangle = new QgsMapToolSelectRectangle( mMapCanvas );
  mMapTools.mSelectRectangle->setAction( mActionSelectRectangle );
  mMapTools.mSelectPolygon = new QgsMapToolSelectPolygon( mMapCanvas );
  mMapTools.mSelectPolygon->setAction( mActionSelectPolygon );
  mMapTools.mSelectFreehand = new QgsMapToolSelectFreehand( mMapCanvas );
  mMapTools.mSelectFreehand->setAction( mActionSelectFreehand );
  mMapTools.mSelectRadius = new QgsMapToolSelectRadius( mMapCanvas );
  mMapTools.mSelectRadius->setAction( mActionSelectRadius );

#if 0 //these three tools to be deprecated - use node tool rather
  mMapTools.mVertexAdd = new QgsMapToolAddVertex( mMapCanvas );
  mMapTools.mVertexAdd->setAction( mActionAddVertex );
  mMapTools.mVertexMove = new QgsMapToolMoveVertex( mMapCanvas );
  mMapTools.mVertexMove->setAction( mActionMoveVertex );
  mMapTools.mVertexDelete = new QgsMapToolDeleteVertex( mMapCanvas );
  mMapTools.mVertexDelete->setAction( mActionDeleteVertex );
#endif
  mMapTools.mAddRing = new QgsMapToolAddRing( mMapCanvas );
  mMapTools.mAddRing->setAction( mActionAddRing );
  mMapTools.mAddIsland = new QgsMapToolAddIsland( mMapCanvas );
  mMapTools.mSimplifyFeature = new QgsMapToolSimplify( mMapCanvas );
  mMapTools.mSimplifyFeature->setAction( mActionSimplifyFeature );
  mMapTools.mDeleteRing = new QgsMapToolDeleteRing( mMapCanvas );
  mMapTools.mDeleteRing->setAction( mActionDeleteRing );
  mMapTools.mDeletePart = new QgsMapToolDeletePart( mMapCanvas );
  mMapTools.mDeletePart->setAction( mActionDeletePart );
  mMapTools.mNodeTool = new QgsMapToolNodeTool( mMapCanvas );
  mMapTools.mNodeTool->setAction( mActionNodeTool );
  mMapTools.mRotatePointSymbolsTool = new QgsMapToolRotatePointSymbols( mMapCanvas );
  mMapTools.mRotatePointSymbolsTool->setAction( mActionRotatePointSymbols );
  //ensure that non edit tool is initialised or we will get crashes in some situations
  mNonEditMapTool = mMapTools.mPan;
}

void QgisApp::createOverview()
{
  // overview canvas
  QgsMapOverviewCanvas* overviewCanvas = new QgsMapOverviewCanvas( NULL, mMapCanvas );
  overviewCanvas->setWhatsThis( tr( "Map overview canvas. This canvas can be used to display a locator map that shows the current extent of the map canvas. The current extent is shown as a red rectangle. Any layer on the map can be added to the overview canvas." ) );

  QBitmap overviewPanBmp = QBitmap::fromData( QSize( 16, 16 ), pan_bits );
  QBitmap overviewPanBmpMask = QBitmap::fromData( QSize( 16, 16 ), pan_mask_bits );
  mOverviewMapCursor = new QCursor( overviewPanBmp, overviewPanBmpMask, 0, 0 ); //set upper left corner as hot spot - this is better when extent marker is small; hand won't cover the marker
  overviewCanvas->setCursor( *mOverviewMapCursor );
//  QVBoxLayout *myOverviewLayout = new QVBoxLayout;
//  myOverviewLayout->addWidget(overviewCanvas);
//  overviewFrame->setLayout(myOverviewLayout);
  mOverviewDock = new QDockWidget( tr( "Overview" ), this );
  mOverviewDock->setObjectName( "Overview" );
  mOverviewDock->setAllowedAreas( Qt::LeftDockWidgetArea | Qt::RightDockWidgetArea );
  mOverviewDock->setWidget( overviewCanvas );
  addDockWidget( Qt::LeftDockWidgetArea, mOverviewDock );
  // add to the Panel submenu
  mPanelMenu->addAction( mOverviewDock->toggleViewAction() );

  mMapCanvas->enableOverviewMode( overviewCanvas );

  // moved here to set anti aliasing to both map canvas and overview
  QSettings mySettings;
  mMapCanvas->enableAntiAliasing( mySettings.value( "/qgis/enable_anti_aliasing", false ).toBool() );
  mMapCanvas->useImageToRender( mySettings.value( "/qgis/use_qimage_to_render", false ).toBool() );

  int action = mySettings.value( "/qgis/wheel_action", 0 ).toInt();
  double zoomFactor = mySettings.value( "/qgis/zoom_factor", 2 ).toDouble();
  mMapCanvas->setWheelAction(( QgsMapCanvas::WheelAction ) action, zoomFactor );
}

void QgisApp::addDockWidget( Qt::DockWidgetArea theArea, QDockWidget * thepDockWidget )
{
  QMainWindow::addDockWidget( theArea, thepDockWidget );
  // Make the right and left docks consume all vertical space and top
  // and bottom docks nest between them
  setCorner( Qt::TopLeftCorner, Qt::LeftDockWidgetArea );
  setCorner( Qt::BottomLeftCorner, Qt::LeftDockWidgetArea );
  setCorner( Qt::TopRightCorner, Qt::RightDockWidgetArea );
  setCorner( Qt::BottomRightCorner, Qt::RightDockWidgetArea );
  // add to the Panel submenu
  mPanelMenu->addAction( thepDockWidget->toggleViewAction() );

  thepDockWidget->show();

  // refresh the map canvas
  mMapCanvas->refresh();
}

QToolBar *QgisApp::addToolBar( QString name )
{
  QToolBar *toolBar = QMainWindow::addToolBar( name );
  // add to the Toolbar submenu
  mToolbarMenu->addAction( toolBar->toggleViewAction() );
  return toolBar;
}

QgsLegend *QgisApp::legend()
{
  Q_ASSERT( mMapLegend );
  return mMapLegend;
}

QgsMapCanvas *QgisApp::mapCanvas()
{
  Q_ASSERT( mMapCanvas );
  return mMapCanvas;
}

void QgisApp::initLegend()
{
  mMapLegend->setWhatsThis( tr( "Map legend that displays all the layers currently on the map canvas. Click on the check box to turn a layer on or off. Double click on a layer in the legend to customize its appearance and set other properties." ) );
  mLegendDock = new QDockWidget( tr( "Layers" ), this );
  mLegendDock->setObjectName( "Legend" );
  mLegendDock->setAllowedAreas( Qt::LeftDockWidgetArea | Qt::RightDockWidgetArea );
  mLegendDock->setWidget( mMapLegend );
  addDockWidget( Qt::LeftDockWidgetArea, mLegendDock );
  // add to the Panel submenu
  mPanelMenu->addAction( mLegendDock->toggleViewAction() );
  return;
}

bool QgisApp::createDB()
{
  // Check qgis.db and make private copy if necessary
  QFile qgisPrivateDbFile( QgsApplication::qgisUserDbFilePath() );

  // first we look for ~/.qgis/qgis.db
  if( !qgisPrivateDbFile.exists() )
  {
    // if it doesnt exist we copy it in from the global resources dir
    QString qgisMasterDbFileName = QgsApplication::qgisMasterDbFilePath();
    QFile masterFile( qgisMasterDbFileName );

    // Must be sure there is destination directory ~/.qgis
    QDir().mkpath( QgsApplication::qgisSettingsDirPath() );

    //now copy the master file into the users .qgis dir
    bool isDbFileCopied = masterFile.copy( qgisPrivateDbFile.fileName() );

    if( !isDbFileCopied )
    {
      QgsDebugMsg( "[ERROR] Can not make qgis.db private copy" );
      return false;
    }
  }
  else
  {
    // migrate if necessary
    sqlite3 *db;
    if( sqlite3_open( QgsApplication::qgisUserDbFilePath().toUtf8().constData(), &db ) != SQLITE_OK )
    {
      QMessageBox::critical( this, tr( "Private qgis.db" ), tr( "Could not open qgis.db" ) );
      return false;
    }

    char *errmsg;
    int res = sqlite3_exec( db, "SELECT epsg FROM tbl_srs LIMIT 0", 0, 0, &errmsg );
    if( res == SQLITE_OK )
    {
      // epsg column exists => need migration
      if( sqlite3_exec( db,
                        "ALTER TABLE tbl_srs RENAME TO tbl_srs_bak;"
                        "CREATE TABLE tbl_srs ("
                        "srs_id INTEGER PRIMARY KEY,"
                        "description text NOT NULL,"
                        "projection_acronym text NOT NULL,"
                        "ellipsoid_acronym NOT NULL,"
                        "parameters text NOT NULL,"
                        "srid integer,"
                        "auth_name varchar,"
                        "auth_id varchar,"
                        "is_geo integer NOT NULL,"
                        "deprecated boolean);"
                        "CREATE INDEX idx_srsauthid on tbl_srs(auth_name,auth_id);"
                        "DROP VIEW vw_srs;"
                        "CREATE VIEW vw_srs as "
                        "select a.description as description,"
                        "a.srs_id as srs_id,"
                        "a.is_geo as is_geo,"
                        "b.name as name,"
                        "a.parameters as parameters,"
                        "a.auth_name as auth_name,"
                        "a.auth_id as auth_id,"
                        "a.deprecated as deprecated"
                        " from "
                        "tbl_srs a inner join tbl_projection b on a.projection_acronym=b.acronym"
                        " order by "
                        "b.name,"
                        "a.description;"
                        "INSERT INTO tbl_srs(srs_id,description,projection_acronym,ellipsoid_acronym,parameters,srid,auth_name,auth_id,is_geo,deprecated) SELECT srs_id,description,projection_acronym,ellipsoid_acronym,parameters,srid,'','',is_geo,0 FROM tbl_srs_bak;"
                        "DROP TABLE tbl_srs_bak", 0, 0, &errmsg ) != SQLITE_OK
        )
      {
        QMessageBox::critical( this, tr( "Private qgis.db" ), tr( "Migration of private qgis.db failed.\n%1" ).arg( QString::fromUtf8( errmsg ) ) );
        sqlite3_free( errmsg );
        sqlite3_close( db );
        return false;
      }
    }

    sqlite3_close( db );
  }
  return true;
}

void QgisApp::createMapTips()
{
  // Set up the timer for maptips. The timer is reset everytime the mouse is moved
  mpMapTipsTimer = new QTimer( mMapCanvas );
  // connect the timer to the maptips slot
  connect( mpMapTipsTimer, SIGNAL( timeout() ), this, SLOT( showMapTip() ) );
  // set the interval to 0.850 seconds - timer will be started next time the mouse moves
  mpMapTipsTimer->setInterval( 850 );
  // Create the maptips object
  mpMaptip = new QgsMapTip();
}

// Update file menu with the current list of recently accessed projects
void QgisApp::updateRecentProjectPaths()
{
  // Remove existing paths from the recent projects menu
  int i;

  int menusize = mRecentProjectsMenu->actions().size();

  for( i = menusize; i < mRecentProjectPaths.size(); i++ )
  {
    mRecentProjectsMenu->addAction( "Dummy text" );
  }

  QList<QAction *> menulist = mRecentProjectsMenu->actions();

  assert( menulist.size() == mRecentProjectPaths.size() );

  for( i = 0; i < mRecentProjectPaths.size(); i++ )
  {
    menulist.at( i )->setText( mRecentProjectPaths.at( i ) );

    // Disable this menu item if the file has been removed, if not enable it
    menulist.at( i )->setEnabled( QFile::exists(( mRecentProjectPaths.at( i ) ) ) );

  }
} // QgisApp::updateRecentProjectPaths

// add this file to the recently opened/saved projects list
void QgisApp::saveRecentProjectPath( QString projectPath, QSettings & settings )
{
  // Get canonical absolute path
  QFileInfo myFileInfo( projectPath );
  projectPath = myFileInfo.absoluteFilePath();

  // If this file is already in the list, remove it
  mRecentProjectPaths.removeAll( projectPath );

  // Prepend this file to the list
  mRecentProjectPaths.prepend( projectPath );

  // Keep the list to 8 items by trimming excess off the bottom
  while( mRecentProjectPaths.count() > 8 )
  {
    mRecentProjectPaths.pop_back();
  }

  // Persist the list
  settings.setValue( "/UI/recentProjectsList", mRecentProjectPaths );


  // Update menu list of paths
  updateRecentProjectPaths();

} // QgisApp::saveRecentProjectPath

void QgisApp::saveWindowState()
{
  // store window and toolbar positions
  QSettings settings;
  // store the toolbar/dock widget settings using Qt4 settings API
  settings.setValue( "/UI/state", saveState() );

  // store window geometry
  settings.setValue( "/UI/geometry", saveGeometry() );

  // Persist state of tile scale slider
  if( mpTileScaleWidget )
  {
    settings.setValue( "/UI/tileScaleEnabled", true );
    delete mpTileScaleWidget;
  }
  else
  {
    settings.setValue( "/UI/tileScaleEnabled", false );
  }

  // Persist state of GPS Tracker
  if( mpGpsWidget )
  {
    settings.setValue( "/gps/widgetEnabled", true );
    delete mpGpsWidget;
  }
  else
  {
    settings.setValue( "/gps/widgetEnabled", false );
  }

  QgsPluginRegistry::instance()->unloadAll();
}

static const unsigned char defaultUIgeometry[] =
{
  0x01, 0xd9, 0xd0, 0xcb, 0x00, 0x01, 0x00, 0x00, 0xff, 0xff, 0xff, 0xfc, 0xff, 0xff, 0xff, 0xfc, 0x00, 0x00, 0x04, 0xb8, 0x00, 0x00, 0x03, 0x22, 0x00, 0x00, 0x00, 0x30, 0x00, 0x00, 0x00, 0x43, 0x00, 0x00, 0x02, 0xaf, 0x00, 0x00, 0x02, 0x22, 0x00, 0x00, 0x00, 0x00, 0x02, 0x00
};

static const unsigned char defaultUIstate[] =
{
  0x00, 0x00, 0x00, 0xff, 0x00, 0x00, 0x00, 0x00, 0xfd, 0x00, 0x00, 0x00, 0x01, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x01, 0x00, 0x00, 0x00, 0x02, 0x87, 0xfc, 0x02, 0x00, 0x00, 0x00, 0x03, 0xfb, 0x00, 0x00, 0x00, 0x0c, 0x00, 0x4c, 0x00, 0x65, 0x00, 0x67, 0x00, 0x65, 0x00, 0x6e, 0x00, 0x64, 0x01, 0x00, 0x00, 0x00, 0x68, 0x00, 0x00, 0x02, 0x87, 0x00, 0x00, 0x00, 0x72, 0x00, 0xff, 0xff, 0xff, 0xfb, 0x00, 0x00, 0x00, 0x10, 0x00, 0x4f, 0x00, 0x76, 0x00, 0x65, 0x00, 0x72, 0x00, 0x76, 0x00, 0x69, 0x00, 0x65, 0x00, 0x77, 0x00, 0x00, 0x00, 0x01, 0xf4, 0x00, 0x00, 0x00, 0x2a, 0x00, 0x00, 0x00, 0x14, 0x00, 0xff, 0xff, 0xff, 0xfb, 0x00, 0x00, 0x00, 0x22, 0x00, 0x43, 0x00, 0x6f, 0x00, 0x6f, 0x00, 0x72, 0x00, 0x64, 0x00, 0x69, 0x00, 0x6e, 0x00, 0x61, 0x00, 0x74, 0x00, 0x65, 0x00, 0x43, 0x00, 0x61, 0x00, 0x70, 0x00, 0x74, 0x00, 0x75, 0x00, 0x72, 0x00, 0x65, 0x00, 0x00, 0x00, 0x01, 0x7e, 0x00, 0x00, 0x00, 0xa0, 0x00, 0x00, 0x00, 0x7e, 0x00, 0xff, 0xff, 0xff, 0x00, 0x00, 0x03, 0xaf, 0x00, 0x00, 0x02, 0x87, 0x00, 0x00, 0x00, 0x01, 0x00, 0x00, 0x00, 0x02, 0x00, 0x00, 0x00, 0x01, 0x00, 0x00, 0x00, 0x02, 0xfc, 0x00, 0x00, 0x00, 0x04, 0x00, 0x00, 0x00, 0x01, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x02, 0x00, 0x00, 0x00, 0x05, 0x00, 0x00, 0x00, 0x16, 0x00, 0x46, 0x00, 0x69, 0x00, 0x6c, 0x00, 0x65, 0x00, 0x54, 0x00, 0x6f, 0x00, 0x6f, 0x00, 0x6c, 0x00, 0x42, 0x00, 0x61, 0x00, 0x72, 0x01, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x01, 0x90, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x18, 0x00, 0x4c, 0x00, 0x61, 0x00, 0x79, 0x00, 0x65, 0x00, 0x72, 0x00, 0x54, 0x00, 0x6f, 0x00, 0x6f, 0x00, 0x6c, 0x00, 0x42, 0x00, 0x61, 0x00, 0x72, 0x01, 0x00, 0x00, 0x01, 0x90, 0x00, 0x00, 0x00, 0xbe, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x14, 0x00, 0x41, 0x00, 0x74, 0x00, 0x74, 0x00, 0x72, 0x00, 0x69, 0x00, 0x62, 0x00, 0x75, 0x00, 0x74, 0x00, 0x65, 0x00, 0x73, 0x01, 0x00, 0x00, 0x02, 0x4e, 0x00, 0x00, 0x01, 0x27, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x08, 0x00, 0x48, 0x00, 0x65, 0x00, 0x6c, 0x00, 0x70, 0x00, 0x00, 0x00, 0x03, 0xd9, 0x00, 0x00, 0x00, 0xdd, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x0e, 0x00, 0x50, 0x00, 0x6c, 0x00, 0x75, 0x00, 0x67, 0x00, 0x69, 0x00, 0x6e, 0x00, 0x73, 0x01, 0x00, 0x00, 0x03, 0x75, 0x00, 0x00, 0x01, 0x40, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x02, 0x00, 0x00, 0x00, 0x02, 0x00, 0x00, 0x00, 0x14, 0x00, 0x44, 0x00, 0x69, 0x00, 0x67, 0x00, 0x69, 0x00, 0x74, 0x00, 0x69, 0x00, 0x7a, 0x00, 0x69, 0x00, 0x6e, 0x00, 0x67, 0x01, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x02, 0x1c, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x1c, 0x00, 0x4d, 0x00, 0x61, 0x00, 0x70, 0x00, 0x20, 0x00, 0x4e, 0x00, 0x61, 0x00, 0x76, 0x00, 0x69, 0x00, 0x67, 0x00, 0x61, 0x00, 0x74, 0x00, 0x69, 0x00, 0x6f, 0x00, 0x6e, 0x01, 0x00, 0x00, 0x02, 0x1c, 0x00, 0x00, 0x02, 0x99, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x02, 0x00, 0x00, 0x00, 0x00
};


void QgisApp::restoreWindowState()
{
  // restore the toolbar and dock widgets postions using Qt4 settings API
  QSettings settings;

  if( !restoreState( settings.value( "/UI/state", QByteArray::fromRawData(( char * )defaultUIstate, sizeof defaultUIstate ) ).toByteArray() ) )
  {
    QgsDebugMsg( "restore of UI state failed" );
  }

  // restore window geometry
  if( !restoreGeometry( settings.value( "/UI/geometry", QByteArray::fromRawData(( char * )defaultUIgeometry, sizeof defaultUIgeometry ) ).toByteArray() ) )
  {
    QgsDebugMsg( "restore of UI geometry failed" );
  }

}
///////////// END OF GUI SETUP ROUTINES ///////////////

void QgisApp::about()
{
  static QgsAbout *abt = NULL;
  if( !abt )
  {
    QApplication::setOverrideCursor( Qt::WaitCursor );
    abt = new QgsAbout();
    QString versionString = tr( "You are using QGIS version %1 built against code revision %2." )
                            .arg( QGis::QGIS_VERSION )
                            .arg( QGis::QGIS_SVN_VERSION );

    versionString += tr( "\nThis copy of QGIS has been built with GDAL/OGR %1." ).arg( GDAL_RELEASE_NAME );

#ifdef HAVE_POSTGRESQL
    versionString += tr( "\nThis copy of QGIS has been built with PostgreSQL support (%1)." ).arg( PG_VERSION );
#else
    versionString += tr( "\nThis copy of QGIS has been built without PostgreSQL support." );
#endif

#ifdef HAVE_SPATIALITE
    versionString += tr( "\nThis copy of QGIS has been built with SpatiaLite support (%1)." ).arg( spatialite_version() );
#else
    versionString += tr( "\nThis copy of QGIS has been built without SpatiaLite support." );
#endif

    versionString += tr( "\nThis copy of QGIS has been built with QWT %1." ).arg( QWT_VERSION_STR );

#ifdef QGISDEBUG
    versionString += tr( "\nThis copy of QGIS writes debugging output." );
#endif

    versionString += tr( "\nThis binary was compiled against Qt %1,"
                         "and is currently running against Qt %2" )
                     .arg( QT_VERSION_STR )
                     .arg( qVersion() );

    abt->setVersion( versionString );
    QString whatsNew = "<html><body>" ;
    whatsNew += "<h2>" + tr( "Version" ) + " " + QString( QGis::QGIS_VERSION ) +  "</h2>";
    whatsNew +=  "<h2>" + trUtf8( "What's new in Version 1.6.0 'Capiapó'?" ) + "</h2>";
    whatsNew +=  "<p>";
    whatsNew +=  tr( "Please note that this is a release in our 'cutting edge' release series. As such it contains new features and extends the programmatic interface over QGIS 1.0.x and QGIS 1.5.0. We recommend that you use this version over previous releases." );
    whatsNew +=  "</p>";
    whatsNew +=  "<p>";
    whatsNew +=  tr( "This release includes over 177 bug fixes and many new features and enhancements. Once again it is impossible to document everything here that has changed so we will just provide a bullet list of key new features here." );
    whatsNew +=  "</p>";

    whatsNew +=  "<h3>" + tr( "General Improvements" ) + "</h3>";
    whatsNew +=  "<ul>";
    whatsNew +=  "<li>" + tr( "Added gpsd support to live gps tracking." ) + "</li>";
    whatsNew +=  "<li>" + tr( "A new plugin has been included that allows for offline editing." ) + "</li>";
    whatsNew +=  "<li>" + tr( "Field calculator will now insert NULL feature value in case of calculation error instead of stopping and reverting calculation for all features." ) + "</li>";
    whatsNew +=  "<li>" + tr( "Update srs.db to include grid reference." ) + "</li>";
    whatsNew +=  "<li>" + tr( "Added a native (C++) raster calculator implementation which can deal with large rasters efficiently." ) + "</li>";
    whatsNew +=  "<li>" + tr( "Improved interaction with extents widget in statusbar so that the text contents of the widget can be copied and pasted." ) + "</li>";
    whatsNew +=  "<li>" + tr( "Many improvements and new operators to the field calculator including field concatenation, row counter etc." ) + "</li>";
    whatsNew +=  "<li>" + tr( "Added --configpath option that overrides the default path (~/.qgis) for user configuration and forces QSettings to use this directory, too. This allows users to e.g. carry QGIS installation on a flash drive together with all plugins and settings." ) + "</li>";
    whatsNew +=  "<li>" + tr( "Experimental WFS-T support. Additionally ported wfs to network manager." ) + "</li>";
    whatsNew +=  "<li>" + tr( "Georeferencer has had many tidy ups and improvements." ) + "</li>";
    whatsNew +=  "<li>" + tr( "Support for long int in attribute dialog and editor." ) + "</li>";
    whatsNew +=  "<li>" + tr( "The QGIS Mapserver project has been incorporated into the main SVN repository and packages are being made available. QGIS Mapserver allows you to serve your QGIS project files via the OGC WMS protocol." ) + " <a href=\"http://linfiniti.com/2010/08/qgis-mapserver-a-wms-server-for-the-masses/\">" + tr("Read More." ) + "</a></li>";
    whatsNew +=  "<li>" + tr( "Select and measure toolbar flyouts and submenus." ) + "</li>";
    whatsNew +=  "<li>" + tr( "Support has been added for non-spatial tables (currently OGR, delimited text and PostgreSQL providers). These tables can be used for field lookups or just generally browsed and edited using the table view." ) + "</li>";
    whatsNew +=  "<li>" + tr( "Added search string support for feature ids ($id) and various other search related improvements." ) + "</li>";
    whatsNew +=  "<li>" + tr( "Added reload method to map layers and provider interface. Like this, caching providers (currently WMS and WFS) can synchronize with changes in the datasource." ) + "</li>";
    whatsNew +=  "</ul>";

    whatsNew +=  "<h3>" + tr( "Table of contents (TOC) improvements" ) + "</h3>";
    whatsNew +=  "<ul>";
    whatsNew +=  "  <li>" + tr( "Added a new option to the raster legend menu that will stretch the current layer using the min and max pixel values of the current extent." ) + "</li>";
    whatsNew +=  "  <li>" + tr( "When writing vector files using the table of contents context menu's 'Save as' option, you can now specify OGR creation options." ) + "</li>";
    whatsNew +=  "  <li>" + tr( "In the table of contents, it is now possible to select and remove or move several layers at once." ) + "</li>";
    whatsNew +=  "</ul>";

    whatsNew +=  "<h3>" + tr( "Labelling (New generation only)" ) + "</h3>";
    whatsNew +=  "<ul>";
    whatsNew +=  "  <li>" + tr( "Data defined label position." ) + "</li>";
    whatsNew +=  "  <li>" + tr( "Line wrapping, data defined font and buffer settings." ) + "</li>";
    whatsNew +=  "</ul>";
    whatsNew +=  "<h3>" + tr( "Layer properties and symbology" ) + "</h3>";
    whatsNew +=  "<ul>";
    whatsNew +=  "  <li>" + tr( "Three new classification modes added to graduated symbol renderer (version 2), including Natural Breaks (Jenks), Standard Deviations, and Pretty Breaks (based on pretty from the R statistical environment). " ) + "<a href=\"http://linfiniti.com/2010/09/new-class-breaks-for-graduated-symbols-in-qgis/\">" + tr( "Read More." ) + "</a></li>";
    whatsNew +=  "  <li>" + tr( "Improved loading speed of the symbol properties dialog." ) + "</li>";
    whatsNew +=  "  <li>" + tr( "Data-defined rotation and size for categorized and graduated renderer (symbology-ng)." ) + "</li>";
    whatsNew +=  "  <li>" + tr( "Use size scale also for line symbols to modify line width." ) + "</li>";
    whatsNew +=  "  <li>" + tr( "Replaced raster histogram implementation with one based on Qwt. Added option to save histogram as image file. Show actual pixel values on x axis of raster histogram." ) + "</li>";
    whatsNew +=  "  <li>" + tr( "Added ability to interactively select pixels from the canvas to populate the transparency table in the raster layer properties dialog." ) + "</li>";
    whatsNew +=  "  <li>" + tr( "Allow creation of color ramps in vector color ramp combo box." ) + "</li>";
    whatsNew +=  "  <li>" + tr( "Added 'style manager...' button to symbol selector so that users will find the style manager more easily." ) + "</li>";
    whatsNew +=  "</ul>";

    whatsNew +=  "<h3>" + tr( "Map Composer" ) + "</h3>";
    whatsNew +=  "<ul>";
    whatsNew +=  "  <li>" + tr( "add capability to show and manipulate composer item width/ height in item position dialog." ) + "</li>";
    whatsNew +=  "  <li>" + tr( "Composer items can now be deleted with the backspace key." ) + "</li>";
    whatsNew +=  "  <li>" + tr( "Sorting for composer attribute table (several columns and ascending / descending)." ) + "</li>";
    whatsNew += " </ul>";

    whatsNew += "</ul>";
    whatsNew += "</body></html>";

    abt->setWhatsNew( whatsNew );

    QApplication::restoreOverrideCursor();
  }
  abt->show();
  abt->raise();
  abt->activateWindow();
}








/**
  This method prompts the user for a list of vector file names  with a dialog.
  */
void QgisApp::addVectorLayer()
{
  if( mMapCanvas && mMapCanvas->isDrawing() )
  {
    return;
  }
  mMapCanvas->freeze();
  QgsOpenVectorLayerDialog *ovl = new QgsOpenVectorLayerDialog( this );

  if( ovl->exec() )
  {
    QStringList selectedSources = ovl->dataSources();
    QString enc = ovl->encoding();
    if( !selectedSources.isEmpty() )
    {
      addVectorLayers( selectedSources, enc, ovl->dataSourceType() );
    }
  }

  mMapCanvas->freeze( false );

  delete ovl;
  // update UI
  qApp->processEvents();
}


bool QgisApp::addVectorLayers( QStringList const & theLayerQStringList, const QString& enc, const QString dataSourceType )
{

  foreach( QString src, theLayerQStringList )
  {
    src = src.trimmed();
    QString base;
    if( dataSourceType == "file" )
    {
      QFileInfo fi( src );
      base = fi.completeBaseName();
    }
    else if( dataSourceType == "database" )
    {
      base = src;
    }
    else //directory //protocol
    {
      QFileInfo fi( src );
      base = fi.completeBaseName();
    }

    QgsDebugMsg( "completeBaseName: " + base );

    // create the layer

    QgsVectorLayer *layer = new QgsVectorLayer( src, base, "ogr" );
    Q_CHECK_PTR( layer );

    if( ! layer )
    {
      mMapCanvas->freeze( false );

// Let render() do its own cursor management
//      QApplication::restoreOverrideCursor();

      // XXX insert meaningful whine to the user here
      return false;
    }

    if( layer->isValid() )
    {
      layer->setProviderEncoding( enc );

      QStringList sublayers = layer->dataProvider()->subLayers();

      // If the newly created layer has more than 1 layer of data available, we show the
      // sublayers selection dialog so the user can select the sublayers to actually load.
      if( sublayers.count() > 1 )
      {
        askUserForOGRSublayers( layer );

        // The first layer loaded is not useful in that case. The user can select it in
        // the list if he wants to load it.
        delete layer;

      }
      else if( sublayers.count() > 0 )  // there is 1 layer of data available
      {
        //set friendly name for datasources with only one layer
        QStringList sublayers = layer->dataProvider()->subLayers();
        QString ligne = sublayers.at( 0 );
        QStringList elements = ligne.split( ":" );
        layer->setLayerName( elements.at( 1 ) );
        // Register this layer with the layers registry
        QgsMapLayerRegistry::instance()->addMapLayer( layer );
      }
      else
      {
        QString msg = tr( "%1 doesn't have any layers" ).arg( src );
        QMessageBox::critical( this, tr( "Invalid Data Source" ), msg );
        delete layer;
      }
    }
    else
    {
      QString msg = tr( "%1 is not a valid or recognized data source" ).arg( src );
      QMessageBox::critical( this, tr( "Invalid Data Source" ), msg );

      // since the layer is bad, stomp on it
      delete layer;

      // XXX should we return false here, or just grind through
      // XXX the remaining arguments?
    }

  }

  // update UI
  qApp->processEvents();

  // draw the map
  mMapCanvas->freeze( false );
  mMapCanvas->refresh();

// Let render() do its own cursor management
//  QApplication::restoreOverrideCursor();

  // statusBar()->showMessage( mMapCanvas->extent().toString( 2 ) );

  return true;
} // QgisApp::addVectorLayer()

void QgisApp::askUserForGDALSublayers( QgsRasterLayer *layer )
{
  if( !layer )
    return;

  QStringList sublayers = layer->subLayers();

  QgsDebugMsg( "sublayers:\n  " + sublayers.join( "  \n" ) + "\n" );

  // We initialize a selection dialog and display it.
  QgsOGRSublayersDialog chooseSublayersDialog( this );
  chooseSublayersDialog.setWindowTitle( tr( "Select raster layers to add..." ) );

  QStringList layers;
  for( int i = 0; i < sublayers.size(); i++ )
  {
    layers << QString( "%1|%2|1|%3" ).arg( i ).arg( sublayers[i] ).arg( tr( "Raster" ) );
  }

  chooseSublayersDialog.populateLayerTable( layers, "|" );

  if( chooseSublayersDialog.exec() )
  {
    foreach( QString layer, chooseSublayersDialog.getSelection() )
    {
      QgsRasterLayer *rlayer = new QgsRasterLayer( layer, layer );
      if( rlayer && rlayer->isValid() )
      {
        addRasterLayer( rlayer );
      }
    }
  }
}

// This method is the method that does the real job. If the layer given in
// parameter is NULL, then the method tries to act on the activeLayer.
void QgisApp::askUserForOGRSublayers( QgsVectorLayer *layer )
{
  if( layer == NULL )
  {
    layer = qobject_cast<QgsVectorLayer *>( activeLayer() );
    if( !layer || layer->dataProvider()->name() != "ogr" )
      return;
  }

  QStringList sublayers = layer->dataProvider()->subLayers();
  QString layertype = layer->dataProvider()->storageType();

  // We initialize a selection dialog and display it.
  QgsOGRSublayersDialog chooseSublayersDialog( this );
  chooseSublayersDialog.setWindowTitle( tr( "Select vector layers to add..." ) );
  chooseSublayersDialog.populateLayerTable( sublayers );

  if( chooseSublayersDialog.exec() )
  {
    QString uri = layer->source();
    //the separator char & was changed to | to be compatible
    //with url for protocol drivers
    if( uri.contains( '|', Qt::CaseSensitive ) )
    {
      // If we get here, there are some options added to the filename.
      // A valid uri is of the form: filename&option1=value1&option2=value2,...
      // We want only the filename here, so we get the first part of the split.
      QStringList theURIParts = uri.split( "|" );
      uri = theURIParts.at( 0 );
    }
    QgsDebugMsg( "Layer type " + layertype );
    // the user has done his choice
    loadOGRSublayers( layertype , uri, chooseSublayersDialog.getSelection() );
  }
}

// This method will load with OGR the layers  in parameter.
// This method has been conceived to use the new URI
// format of the ogrprovider so as to give precisions about which
// sublayer to load into QGIS. It is normally triggered by the
// sublayer selection dialog.
void QgisApp::loadOGRSublayers( QString layertype, QString uri, QStringList list )
{
  // The uri must contain the actual uri of the vectorLayer from which we are
  // going to load the sublayers.
  QString fileName = QFileInfo( uri ).baseName();
  for( int i = 0; i < list.size(); i++ )
  {
    QString composedURI;
    if( layertype != "GRASS" )
    {
      composedURI = uri + "|layername=" + list.at( i );
    }
    else
    {
      composedURI = uri + "|layerindex=" + list.at( i );
    }
    addVectorLayer( composedURI,  list.at( i ), "ogr" );
  }
}

#ifndef HAVE_POSTGRESQL
void QgisApp::addDatabaseLayer() {}
#else
void QgisApp::addDatabaseLayer()
{
  if( mMapCanvas && mMapCanvas->isDrawing() )
  {
    return;
  }

  // only supports postgis layers at present
  // show the postgis dialog

  QgsPgSourceSelect *dbs = new QgsPgSourceSelect( this );

  mMapCanvas->freeze();

  if( dbs->exec() )
  {
// Let render() do its own cursor management
//    QApplication::setOverrideCursor(Qt::WaitCursor);


    // repaint the canvas if it was covered by the dialog

    // add files to the map canvas
    QStringList tables = dbs->selectedTables();

    QApplication::setOverrideCursor( Qt::WaitCursor );

    QString connectionInfo = dbs->connectionInfo();
    // for each selected table, connect to the database, parse the Wkt geometry,
    // and build a canvasitem for it
    // readWKB(connectionInfo,tables);
    QStringList::Iterator it = tables.begin();
    while( it != tables.end() )
    {

      // create the layer
      //qWarning("creating layer");
      QgsVectorLayer *layer = new QgsVectorLayer( connectionInfo + " " + *it, *it, "postgres" );
      if( layer->isValid() )
      {
        // register this layer with the central layers registry
        QgsMapLayerRegistry::instance()->addMapLayer( layer );
      }
      else
      {
        QgsDebugMsg(( *it ) + " is an invalid layer - not loaded" );
        QMessageBox::critical( this, tr( "Invalid Layer" ), tr( "%1 is an invalid layer and cannot be loaded." ).arg( *it ) );
        delete layer;
      }
      //qWarning("incrementing iterator");
      ++it;
    }

    QApplication::restoreOverrideCursor();

    statusBar()->showMessage( mMapCanvas->extent().toString( 2 ) );
  }

  delete dbs;

  // update UI
  qApp->processEvents();

  // draw the map
  mMapCanvas->freeze( false );
  mMapCanvas->refresh();

// Let render() do its own cursor management
//  QApplication::restoreOverrideCursor();

} // QgisApp::addDatabaseLayer()
#endif


#ifndef HAVE_SPATIALITE
void QgisApp::addSpatiaLiteLayer() {}
#else
void QgisApp::addSpatiaLiteLayer()
{
  if( mMapCanvas && mMapCanvas->isDrawing() )
  {
    return;
  }

  // show the SpatiaLite dialog

  QgsSpatiaLiteSourceSelect *dbs = new QgsSpatiaLiteSourceSelect( this );

  mMapCanvas->freeze();

  if( dbs->exec() )
  {
// Let render() do its own cursor management
//    QApplication::setOverrideCursor(Qt::WaitCursor);


    // repaint the canvas if it was covered by the dialog

    // add files to the map canvas
    QStringList tables = dbs->selectedTables();

    QApplication::setOverrideCursor( Qt::WaitCursor );

    QString connectionInfo = dbs->connectionInfo();
    // for each selected table, connect to the database and build a canvasitem for it
    QStringList::Iterator it = tables.begin();
    while( it != tables.end() )
    {
      // normalizing the layer name
      QString layername = *it;
      layername = layername.mid( 1 );
      int idx = layername.indexOf( "\" (" );
      if( idx > 0 )
        layername.truncate( idx );

      // create the layer
      QgsVectorLayer *layer = new QgsVectorLayer( "dbname='" + connectionInfo + "' table=" + *it + ") sql=", layername, "spatialite" );
      if( layer->isValid() )
      {
        // register this layer with the central layers registry
        QgsMapLayerRegistry::instance()->addMapLayer( layer );
      }
      else
      {
        QgsDebugMsg(( *it ) + " is an invalid layer - not loaded" );
        QMessageBox::critical( this, tr( "Invalid Layer" ), tr( "%1 is an invalid layer and cannot be loaded." ).arg( *it ) );
        delete layer;
      }
      //qWarning("incrementing iterator");
      ++it;
    }

    QApplication::restoreOverrideCursor();

    statusBar()->showMessage( mMapCanvas->extent().toString( 2 ) );
  }
  delete dbs;

  // update UI
  qApp->processEvents();

  // draw the map
  mMapCanvas->freeze( false );
  mMapCanvas->refresh();

// Let render() do its own cursor management
//  QApplication::restoreOverrideCursor();

} // QgisApp::addSpatiaLiteLayer()
#endif

void QgisApp::addWmsLayer()
{
  if( mMapCanvas && mMapCanvas->isDrawing() )
  {
    return;
  }
  // Fudge for now
  QgsDebugMsg( "about to addRasterLayer" );

  QgsWMSSourceSelect *wmss = new QgsWMSSourceSelect( this );
  wmss->exec();
  delete wmss;
}

void QgisApp::fileExit()
{
  if( mMapCanvas && mMapCanvas->isDrawing() )
  {
    return;
  }

  if( saveDirty() )
  {
    deletePrintComposers();
    removeAnnotationItems();
    mMapCanvas->freeze( true );
    removeAllLayers();
    qApp->exit( 0 );
  }
}



void QgisApp::fileNew()
{
  fileNew( true ); // prompts whether to save project
} // fileNew()


//as file new but accepts flags to indicate whether we should prompt to save
void QgisApp::fileNew( bool thePromptToSaveFlag )
{
  if( mMapCanvas && mMapCanvas->isDrawing() )
  {
    return;
  }

  if( thePromptToSaveFlag )
  {
    if( !saveDirty() )
    {
      return; //cancel pressed
    }
  }

  deletePrintComposers();
  removeAnnotationItems();

  mMapCanvas->freeze( true );
  removeAllLayers();
  mMapCanvas->clear();

  QgsProject* prj = QgsProject::instance();
  prj->title( QString::null );
  prj->setFileName( QString::null );
  prj->clearProperties(); // why carry over properties from previous projects?

  QSettings settings;

  //set the color for selections
  //the default can be set in qgisoptions
  //use project properties to override the color on a per project basis
  int myRed = settings.value( "/qgis/default_selection_color_red", 255 ).toInt();
  int myGreen = settings.value( "/qgis/default_selection_color_green", 255 ).toInt();
  int myBlue = settings.value( "/qgis/default_selection_color_blue", 0 ).toInt();
  prj->writeEntry( "Gui", "/SelectionColorRedPart", myRed );
  prj->writeEntry( "Gui", "/SelectionColorGreenPart", myGreen );
  prj->writeEntry( "Gui", "/SelectionColorBluePart", myBlue );
  QgsRenderer::setSelectionColor( QColor( myRed, myGreen, myBlue ) );

  //set the canvas to the default background color
  //the default can be set in qgisoptions
  //use project properties to override the color on a per project basis
  myRed = settings.value( "/qgis/default_canvas_color_red", 255 ).toInt();
  myGreen = settings.value( "/qgis/default_canvas_color_green", 255 ).toInt();
  myBlue = settings.value( "/qgis/default_canvas_color_blue", 255 ).toInt();
  prj->writeEntry( "Gui", "/CanvasColorRedPart", myRed );
  prj->writeEntry( "Gui", "/CanvasColorGreenPart", myGreen );
  prj->writeEntry( "Gui", "/CanvasColorBluePart", myBlue );
  mMapCanvas->setCanvasColor( QColor( myRed, myGreen, myBlue ) );

  prj->dirty( false );

  setTitleBarText_( *this );

  //QgsDebugMsg("emiting new project signal");

  //emit signal so QgsComposer knows we have a new project
  emit newProject();

  mMapCanvas->freeze( false );
  mMapCanvas->refresh();
  mMapCanvas->clearExtentHistory();

  mMapCanvas->mapRenderer()->setProjectionsEnabled( false );

  // set the initial map tool
  mMapCanvas->setMapTool( mMapTools.mPan );
  mNonEditMapTool = mMapTools.mPan;  // signals are not yet setup to catch this
} // QgisApp::fileNew(bool thePromptToSaveFlag)


void QgisApp::newVectorLayer()
{
  if( mMapCanvas && mMapCanvas->isDrawing() )
  {
    return;
  }

  QgsNewVectorLayerDialog geomDialog( this );
  if( geomDialog.exec() == QDialog::Rejected )
  {
    return;
  }

  QGis::WkbType geometrytype = geomDialog.selectedType();
  QString fileformat = geomDialog.selectedFileFormat();
  int crsId = geomDialog.selectedCrsId();
  QgsDebugMsg( QString( "New file format will be: %1" ).arg( fileformat ) );

  std::list<std::pair<QString, QString> > attributes;
  geomDialog.attributes( attributes );

  QString enc;
  QString fileName;

  QSettings settings;
  QString lastUsedDir = settings.value( "/UI/lastVectorFileFilterDir", "." ).toString();

  QgsDebugMsg( "Saving vector file dialog without filters: " );

  QgsEncodingFileDialog* openFileDialog =
    new QgsEncodingFileDialog( this, tr( "Save As" ), lastUsedDir, "", QString( "" ) );

  openFileDialog->setFileMode( QFileDialog::AnyFile );
  openFileDialog->setAcceptMode( QFileDialog::AcceptSave );
  openFileDialog->setConfirmOverwrite( true );

  if( settings.contains( "/UI/lastVectorFileFilter" ) )
  {
    QString lastUsedFilter = settings.value( "/UI/lastVectorFileFilter", QVariant( QString::null ) ).toString();
    openFileDialog->selectFilter( lastUsedFilter );
  }

  if( openFileDialog->exec() == QDialog::Rejected )
  {
    delete openFileDialog;
    return;
  }

  fileName = openFileDialog->selectedFiles().first();

  if( fileformat == "ESRI Shapefile" && !fileName.endsWith( ".shp", Qt::CaseInsensitive ) )
    fileName += ".shp";

  enc = openFileDialog->encoding();

  settings.setValue( "/UI/lastVectorFileFilter", openFileDialog->selectedFilter() );
  settings.setValue( "/UI/lastVectorFileFilterDir", openFileDialog->directory().absolutePath() );

  delete openFileDialog;

  //try to create the new layer with OGRProvider instead of QgsVectorFileWriter
  QgsProviderRegistry * pReg = QgsProviderRegistry::instance();
  QString ogrlib = pReg->library( "ogr" );
  // load the data provider
  QLibrary* myLib = new QLibrary( ogrlib );
  bool loaded = myLib->load();
  if( loaded )
  {
    QgsDebugMsg( "ogr provider loaded" );

    typedef bool ( *createEmptyDataSourceProc )( const QString&, const QString&, const QString&, QGis::WkbType,
        const std::list<std::pair<QString, QString> >&, const QgsCoordinateReferenceSystem * );
    createEmptyDataSourceProc createEmptyDataSource = ( createEmptyDataSourceProc ) cast_to_fptr( myLib->resolve( "createEmptyDataSource" ) );
    if( createEmptyDataSource )
    {
      if( geometrytype != QGis::WKBUnknown )
      {
        QgsCoordinateReferenceSystem srs( crsId, QgsCoordinateReferenceSystem::InternalCrsId );
        createEmptyDataSource( fileName, fileformat, enc, geometrytype, attributes, &srs );
      }
      else
      {
        QgsDebugMsg( "geometry type not recognised" );
        return;
      }
    }
    else
    {
      QgsDebugMsg( "Resolving newEmptyDataSource(...) failed" );
    }
  }

  //then add the layer to the view
  QStringList fileNames;
  fileNames.append( fileName );
  //todo: the last parameter will change accordingly to layer type
  addVectorLayers( fileNames, enc, "file" );
}

#ifdef HAVE_SPATIALITE
void QgisApp::newSpatialiteLayer()
{
  if( mMapCanvas && mMapCanvas->isDrawing() )
  {
    return;
  }
  QgsNewSpatialiteLayerDialog spatialiteDialog( this );
  spatialiteDialog.exec();
}
#endif

void QgisApp::showRasterCalculator()
{
  QgsRasterCalcDialog d;
  if( d.exec() == QDialog::Accepted )
  {
    //invoke analysis library
    //extent and output resolution will come later...
    QgsRasterCalculator rc( d.formulaString(), d.outputFile(), d.outputFormat(), d.outputRectangle(), d.numberOfColumns(), d.numberOfRows(), d.rasterEntries() );

    QProgressDialog p( tr( "Calculating..." ), tr( "Abort..." ), 0, 0 );
    p.setWindowModality( Qt::WindowModal );
    if( rc.processCalculation( &p ) == 0 )
    {
      if( d.addLayerToProject() )
      {
        addRasterLayer( d.outputFile(), QFileInfo( d.outputFile() ).baseName() );
      }
    }
  }
}

void QgisApp::fileOpen()
{
  if( mMapCanvas && mMapCanvas->isDrawing() )
  {
    return;
  }

  // possibly save any pending work before opening a new project
  if( saveDirty() )
  {
    // Retrieve last used project dir from persistent settings
    QSettings settings;
    QString lastUsedDir = settings.value( "/UI/lastProjectDir", "." ).toString();
    QString fullPath = QFileDialog::getOpenFileName( this, tr( "Choose a QGIS project file to open" ), lastUsedDir, tr( "QGis files (*.qgs)" ) );
    if( fullPath.isNull() )
    {
      return;
    }

    // Fix by Tim - getting the dirPath from the dialog
    // directly truncates the last node in the dir path.
    // This is a workaround for that
    QFileInfo myFI( fullPath );
    QString myPath = myFI.path();
    // Persist last used project dir
    settings.setValue( "/UI/lastProjectDir", myPath );

    deletePrintComposers();
    removeAnnotationItems();
    // clear out any stuff from previous project
    mMapCanvas->freeze( true );
    removeAllLayers();

    QgsProject::instance()->setFileName( fullPath );

    if( ! QgsProject::instance()->read() )
    {
      QMessageBox::critical( this,
                             tr( "QGIS Project Read Error" ),
                             QgsProject::instance()->error() );
      mMapCanvas->freeze( false );
      mMapCanvas->refresh();
      return;
    }

    setTitleBarText_( *this );
    emit projectRead();     // let plug-ins know that we've read in a new
    // project so that they can check any project
    // specific plug-in state

    // add this to the list of recently used project files
    saveRecentProjectPath( fullPath, settings );

    mMapCanvas->freeze( false );
    mMapCanvas->refresh();
  }

} // QgisApp::fileOpen


/**
  adds a saved project to qgis, usually called on startup by specifying a
  project file on the command line
  */
bool QgisApp::addProject( QString projectFile )
{
  mMapCanvas->freeze( true );

  QApplication::setOverrideCursor( Qt::WaitCursor );

  deletePrintComposers();
  removeAnnotationItems();

  // clear the map canvas
  removeAllLayers();

  if( ! QgsProject::instance()->read( projectFile ) )
  {
    QMessageBox::critical( this,
                           tr( "Unable to open project" ),
                           QgsProject::instance()->error() );

    QApplication::restoreOverrideCursor();

    mMapCanvas->freeze( false );
    mMapCanvas->refresh();
    return false;
  }

  setTitleBarText_( *this );
  int  myRedInt = QgsProject::instance()->readNumEntry( "Gui", "/CanvasColorRedPart", 255 );
  int  myGreenInt = QgsProject::instance()->readNumEntry( "Gui", "/CanvasColorGreenPart", 255 );
  int  myBlueInt = QgsProject::instance()->readNumEntry( "Gui", "/CanvasColorBluePart", 255 );
  QColor myColor = QColor( myRedInt, myGreenInt, myBlueInt );
  mMapCanvas->setCanvasColor( myColor ); //this is fill color before rendering starts
  QgsDebugMsg( "Canvas background color restored..." );

  mMapCanvas->updateScale();
  QgsDebugMsg( "Scale restored..." );

  emit projectRead(); // let plug-ins know that we've read in a new
  // project so that they can check any project
  // specific plug-in state

  // add this to the list of recently used project files
  QSettings settings;
  saveRecentProjectPath( projectFile, settings );

  QApplication::restoreOverrideCursor();

  mMapCanvas->freeze( false );
  mMapCanvas->refresh();
  return true;
} // QgisApp::addProject(QString projectFile)



bool QgisApp::fileSave()
{
  if( mMapCanvas && mMapCanvas->isDrawing() )
  {
    return false;
  }

  // if we don't have a file name, then obviously we need to get one; note
  // that the project file name is reset to null in fileNew()
  QFileInfo fullPath;

  // we need to remember if this is a new project so that we know to later
  // update the "last project dir" settings; we know it's a new project if
  // the current project file name is empty
  bool isNewProject = false;

  if( QgsProject::instance()->fileName().isNull() )
  {
    isNewProject = true;

    // Retrieve last used project dir from persistent settings
    QSettings settings;
    QString lastUsedDir = settings.value( "/UI/lastProjectDir", "." ).toString();

    std::auto_ptr<QFileDialog> saveFileDialog( new QFileDialog( this,
        tr( "Choose a QGIS project file" ),
        lastUsedDir, tr( "QGis files (*.qgs)" ) ) );

    saveFileDialog->setFileMode( QFileDialog::AnyFile );
    saveFileDialog->setAcceptMode( QFileDialog::AcceptSave );
    saveFileDialog->setConfirmOverwrite( true );

    if( saveFileDialog->exec() == QDialog::Accepted )
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


    QgsProject::instance()->setFileName( fullPath.filePath() );
  }

  if( QgsProject::instance()->write() )
  {
    setTitleBarText_( *this ); // update title bar
    statusBar()->showMessage( tr( "Saved project to: %1" ).arg( QgsProject::instance()->fileName() ) );

    if( isNewProject )
    {
      // add this to the list of recently used project files
      QSettings settings;
      saveRecentProjectPath( fullPath.filePath(), settings );
    }
  }
  else
  {
    QMessageBox::critical( this,
                           tr( "Unable to save project %1" ).arg( QgsProject::instance()->fileName() ),
                           QgsProject::instance()->error() );
    return false;
  }
  return true;
} // QgisApp::fileSave



void QgisApp::fileSaveAs()
{
  if( mMapCanvas && mMapCanvas->isDrawing() )
  {
    return;
  }

  // Retrieve last used project dir from persistent settings
  QSettings settings;
  QString lastUsedDir = settings.value( "/UI/lastProjectDir", "." ).toString();
  QString saveFilePath = QFileDialog::getSaveFileName( this, tr( "Choose a file name to save the QGIS project file as" ), lastUsedDir, tr( "QGis files (*.qgs)" ) );
  if( saveFilePath.isNull() )  //canceled
  {
    return;
  }
  QFileInfo myFI( saveFilePath );
  QString myPath = myFI.path();
  settings.setValue( "/UI/lastProjectDir", myPath );

  // make sure the .qgs extension is included in the path name. if not, add it...
  if( "qgs" != myFI.suffix() )
  {
    saveFilePath = myFI.filePath() + ".qgs";
  }

  QgsProject::instance()->setFileName( saveFilePath );

  if( QgsProject::instance()->write() )
  {
    setTitleBarText_( *this ); // update title bar
    statusBar()->showMessage( tr( "Saved project to: %1" ).arg( QgsProject::instance()->fileName() ) );
    // add this to the list of recently used project files
    saveRecentProjectPath( saveFilePath, settings );
  }
  else
  {
    QMessageBox::critical( this,
                           tr( "Unable to save project %1" ).arg( QgsProject::instance()->fileName() ),
                           QgsProject::instance()->error(),
                           QMessageBox::Ok,
                           Qt::NoButton );
  }
} // QgisApp::fileSaveAs






// Open the project file corresponding to the
// path at the given index in mRecentProjectPaths
void QgisApp::openProject( QAction *action )
{

  // possibly save any pending work before opening a different project
  QString debugme;
  assert( action != NULL );

  debugme = action->text();

  if( saveDirty() )
  {
    addProject( debugme );

  }
  //set the projections enabled icon in the status bar
  int myProjectionEnabledFlag =
    QgsProject::instance()->readNumEntry( "SpatialRefSys", "/ProjectionsEnabled", 0 );
  mMapCanvas->mapRenderer()->setProjectionsEnabled( myProjectionEnabledFlag );

} // QgisApp::openProject

/**
  Open the specified project file; prompt to save previous project if necessary.
  Used to process a commandline argument or OpenDocument AppleEvent.
  */
void QgisApp::openProject( const QString & fileName )
{
  // possibly save any pending work before opening a different project
  if( saveDirty() )
  {
    // error handling and reporting is in addProject() function
    addProject( fileName );
  }
  return ;
}


/**
  Open a raster or vector file; ignore other files.
  Used to process a commandline argument or OpenDocument AppleEvent.
  @returns true if the file is successfully opened
  */
bool QgisApp::openLayer( const QString & fileName, bool allowInteractive )
{
  QFileInfo fileInfo( fileName );

  // try to load it as raster
  bool ok( false );
  CPLPushErrorHandler( CPLQuietErrorHandler );
  if( QgsRasterLayer::isValidRasterFileName( fileName ) )
  {
    ok  = addRasterLayer( fileName, fileInfo.completeBaseName() );
  }
  else // nope - try to load it as a shape/ogr
  {
    if( allowInteractive )
    {
      ok = addVectorLayers( QStringList( fileName ), "System", "file" );
    }
    else
    {
      ok = addVectorLayer( fileName, fileInfo.completeBaseName(), "ogr" );
    }
  }

  CPLPopErrorHandler();

  if( !ok )
  {
    // we have no idea what this file is...
    QgsDebugMsg( "Unable to load " + fileName );
  }

  return ok;
}

void QgisApp::newPrintComposer()
{
  if( mMapCanvas && mMapCanvas->isDrawing() )
  {
    return;
  }

  createNewComposer();
}

void QgisApp::showComposerManager()
{
  QgsComposerManager m;
  m.exec();
}

void QgisApp::saveMapAsImage()
{
  QPair< QString, QString> myFileNameAndFilter = QgisGui::getSaveAsImageName( this, tr( "Choose a file name to save the map image as" ) );
  if( myFileNameAndFilter.first != "" )
  {
    //save the mapview to the selected file
    mMapCanvas->saveAsImage( myFileNameAndFilter.first, NULL, myFileNameAndFilter.second );
    statusBar()->showMessage( tr( "Saved map image to %1" ).arg( myFileNameAndFilter.first ) );
  }

} // saveMapAsImage

//overloaded version of the above function
void QgisApp::saveMapAsImage( QString theImageFileNameQString, QPixmap * theQPixmap )
{
  if( theImageFileNameQString == "" )
  {
    //no fileName chosen
    return;
  }
  else
  {
    //force the size of the canvase
    mMapCanvas->resize( theQPixmap->width(), theQPixmap->height() );
    //save the mapview to the selected file
    mMapCanvas->saveAsImage( theImageFileNameQString, theQPixmap );
  }
} // saveMapAsImage


//reimplements method from base (gui) class
void QgisApp::addAllToOverview()
{
  if( mMapLegend )
  {
    mMapLegend->enableOverviewModeAllLayers( true );
  }

  markDirty();
}

//reimplements method from base (gui) class
void QgisApp::removeAllFromOverview()
{
  if( mMapLegend )
  {
    mMapLegend->enableOverviewModeAllLayers( false );
  }

  markDirty();
}

void QgisApp::toggleFullScreen()
{
  if( mFullScreenMode )
  {
    if( mPrevScreenModeMaximized )
    {
      // Change to maximized state. Just calling showMaximized() results in
      // the window going to the normal state. Calling showNormal() then
      // showMaxmized() is a work-around. Turn off rendering for this as it
      // would otherwise cause two re-renders of the map, which can take a
      // long time.
      bool renderFlag = mapCanvas()->renderFlag();
      if( renderFlag )
        mapCanvas()->setRenderFlag( false );
      showNormal();
      showMaximized();
      if( renderFlag )
        mapCanvas()->setRenderFlag( true );
      mPrevScreenModeMaximized = false;
    }
    else
    {
      showNormal();
    }
    mFullScreenMode = false;
  }
  else
  {
    if( isMaximized() )
    {
      mPrevScreenModeMaximized = true;
    }
    showFullScreen();
    mFullScreenMode = true;
  }
}

void QgisApp::showActiveWindowMinimized()
{
  QWidget *window = QApplication::activeWindow();
  if( window )
  {
    window->showMinimized();
  }
}

void QgisApp::toggleActiveWindowMaximized()
{
  QWidget *window = QApplication::activeWindow();
  if( window )
  {
    if( window->isMaximized() ) window->showNormal();
    else window->showMaximized();
  }
}

void QgisApp::activate()
{
  raise();
  setWindowState( windowState() & ~Qt::WindowMinimized );
  activateWindow();
}

void QgisApp::bringAllToFront()
{
#ifdef Q_WS_MAC
  // Bring forward all open windows while maintaining layering order
  ProcessSerialNumber psn;
  GetCurrentProcess( &psn );
  SetFrontProcess( &psn );
#endif
}

void QgisApp::addWindow( QAction *action )
{
#ifdef Q_WS_MAC
  mWindowActions->addAction( action );
  mWindowMenu->addAction( action );
  action->setCheckable( true );
  action->setChecked( true );
#endif
}

void QgisApp::removeWindow( QAction *action )
{
#ifdef Q_WS_MAC
  mWindowActions->removeAction( action );
  mWindowMenu->removeAction( action );
#endif
}

void QgisApp::stopRendering()
{
  if( mMapCanvas )
  {
    QgsMapRenderer* mypMapRenderer = mMapCanvas->mapRenderer();
    if( mypMapRenderer )
    {
      QgsRenderContext* mypRenderContext = mypMapRenderer->rendererContext();
      if( mypRenderContext )
      {
        mypRenderContext->setRenderingStopped( true );
      }
    }
  }
}

//reimplements method from base (gui) class
void QgisApp::hideAllLayers()
{
  QgsDebugMsg( "hiding all layers!" );

  legend()->selectAll( false );
}


// reimplements method from base (gui) class
void QgisApp::showAllLayers()
{
  QgsDebugMsg( "Showing all layers!" );

  legend()->selectAll( true );
}


void QgisApp::zoomIn()
{
  QgsDebugMsg( "Setting map tool to zoomIn" );

  mMapCanvas->setMapTool( mMapTools.mZoomIn );
}


void QgisApp::zoomOut()
{
  mMapCanvas->setMapTool( mMapTools.mZoomOut );
}

void QgisApp::zoomToSelected()
{
  mMapCanvas->zoomToSelected();
}

void QgisApp::pan()
{
  mMapCanvas->setMapTool( mMapTools.mPan );
}

void QgisApp::zoomFull()
{
  mMapCanvas->zoomToFullExtent();
}

void QgisApp::zoomToPrevious()
{
  mMapCanvas->zoomToPreviousExtent();
}

void QgisApp::zoomToNext()
{
  mMapCanvas->zoomToNextExtent();
}

void QgisApp::zoomActualSize()
{
  mMapLegend->legendLayerZoomNative();
}

void QgisApp::identify()
{
  mMapCanvas->setMapTool( mMapTools.mIdentify );
}

void QgisApp::measure()
{
  mMapCanvas->setMapTool( mMapTools.mMeasureDist );
}

void QgisApp::measureArea()
{
  mMapCanvas->setMapTool( mMapTools.mMeasureArea );
}

void QgisApp::measureAngle()
{
  mMapCanvas->setMapTool( mMapTools.mMeasureAngle );
}

void QgisApp::addFormAnnotation()
{
  mMapCanvas->setMapTool( mMapTools.mFormAnnotation );
}

void QgisApp::addTextAnnotation()
{
  mMapCanvas->setMapTool( mMapTools.mTextAnnotation );
}

void QgisApp::modifyAnnotation()
{
  mMapCanvas->setMapTool( mMapTools.mAnnotation );
}

void QgisApp::labeling()
{
  QgsMapLayer* layer = activeLayer();
  if( layer == NULL || layer->type() != QgsMapLayer::VectorLayer )
  {
    QMessageBox::warning( this, tr( "Labeling" ), tr( "Please select a vector layer first." ) );
    return;
  }
  QgsVectorLayer* vlayer = dynamic_cast<QgsVectorLayer*>( layer );

  QgsLabelingGui labelGui( mLBL, vlayer, this );

  if( labelGui.exec() )
  {
    // alter labeling - save the changes
    labelGui.layerSettings().writeToLayer( vlayer );

    // trigger refresh
    if( mMapCanvas )
    {
      mMapCanvas->refresh();
    }
  }
}

void QgisApp::attributeTable()
{
  if( mMapCanvas && mMapCanvas->isDrawing() )
  {
    return;
  }

  QgsVectorLayer *myLayer = qobject_cast<QgsVectorLayer *>( activeLayer() );
  if( !myLayer )
  {
    return;
  }

  QgsAttributeTableDialog *mDialog = new QgsAttributeTableDialog( myLayer );
  mDialog->show();
  // the dialog will be deleted by itself on close
}

void QgisApp::saveAsVectorFile()
{
  saveAsVectorFileGeneral( false );
}

void QgisApp::saveSelectionAsVectorFile()
{
  saveAsVectorFileGeneral( true );
}

void QgisApp::saveAsVectorFileGeneral( bool saveOnlySelection )
{
  if( mMapCanvas && mMapCanvas->isDrawing() )
    return;

  if( !mMapLegend )
    return;

  QgsVectorLayer *vlayer = qobject_cast<QgsVectorLayer *>( activeLayer() ); // FIXME: output of multiple layers at once?
  if( !vlayer )
    return;

  QgsCoordinateReferenceSystem destCRS;

  QgsVectorLayerSaveAsDialog *dialog = new QgsVectorLayerSaveAsDialog( this );

  if( dialog->exec() == QDialog::Accepted )
  {
    QString encoding = dialog->encoding();
    QString vectorFilename = dialog->filename();
    QString format = dialog->format();

    if( dialog->crs() < 0 )
    {
      // Find out if we have projections enabled or not
      if( mMapCanvas->mapRenderer()->hasCrsTransformEnabled() )
      {
        destCRS = mMapCanvas->mapRenderer()->destinationSrs();
      }
      else
      {
        destCRS = vlayer->srs();
      }
    }
    else
    {
      destCRS = QgsCoordinateReferenceSystem( dialog->crs(), QgsCoordinateReferenceSystem::InternalCrsId );
    }

    // ok if the file existed it should be deleted now so we can continue...
    QApplication::setOverrideCursor( Qt::WaitCursor );

    QgsVectorFileWriter::WriterError error;
    QString errorMessage;
    error = QgsVectorFileWriter::writeAsVectorFormat(
              vlayer, vectorFilename, encoding, &destCRS, format,
              saveOnlySelection,
              &errorMessage,
              dialog->datasourceOptions(), dialog->layerOptions() );

    QApplication::restoreOverrideCursor();

    if( error == QgsVectorFileWriter::NoError )
    {
      QMessageBox::information( 0, tr( "Saving done" ), tr( "Export to vector file has been completed" ) );
    }
    else
    {
      QgsMessageViewer *m = new QgsMessageViewer( 0 );
      m->setWindowTitle( tr( "Save error" ) );
      m->setMessageAsPlainText( tr( "Export to vector file failed.\nError: %1" ).arg( errorMessage ) );
      m->exec();
    }
  }

  delete dialog;
}


void QgisApp::layerProperties()
{
  if( mMapCanvas && mMapCanvas->isDrawing() )
    return;

  showLayerProperties( activeLayer() );
}

void QgisApp::deleteSelected( QgsMapLayer *layer )
{
  if( !layer )
  {
    layer = mMapLegend->currentLayer();
  }

  if( !layer )
  {
    QMessageBox::information( this,
                              tr( "No Layer Selected" ),
                              tr( "To delete features, you must select a vector layer in the legend" ) );
    return;
  }

  QgsVectorLayer* vlayer = qobject_cast<QgsVectorLayer *>( layer );
  if( !vlayer )
  {
    QMessageBox::information( this,
                              tr( "No Vector Layer Selected" ),
                              tr( "Deleting features only works on vector layers" ) );
    return;
  }

  if( !( vlayer->dataProvider()->capabilities() & QgsVectorDataProvider::DeleteFeatures ) )
  {
    QMessageBox::information( this, tr( "Provider does not support deletion" ),
                              tr( "Data provider does not support deleting features" ) );
    return;
  }

  if( !vlayer->isEditable() )
  {
    QMessageBox::information( this, tr( "Layer not editable" ),
                              tr( "The current layer is not editable. Choose 'Start editing' in the digitizing toolbar." ) );
    return;
  }

  //display a warning
  int numberOfDeletedFeatures = vlayer->selectedFeaturesIds().size();
  if( QMessageBox::warning( this, tr( "Delete features" ), tr( "Delete %n feature(s)?", "number of features to delete", numberOfDeletedFeatures ), QMessageBox::Ok, QMessageBox::Cancel ) == QMessageBox::Cancel )
  {
    return;
  }

  vlayer->beginEditCommand( tr( "Features deleted" ) );
  if( !vlayer->deleteSelectedFeatures() )
  {
    QMessageBox::information( this, tr( "Problem deleting features" ),
                              tr( "A problem occured during deletion of features" ) );
  }

  vlayer->endEditCommand();
}

void QgisApp::moveFeature()
{
  mMapCanvas->setMapTool( mMapTools.mMoveFeature );
}

void QgisApp::simplifyFeature()
{
  mMapCanvas->setMapTool( mMapTools.mSimplifyFeature );
}

void QgisApp::deleteRing()
{
  mMapCanvas->setMapTool( mMapTools.mDeleteRing );
}

void QgisApp::deletePart()
{
  mMapCanvas->setMapTool( mMapTools.mDeletePart );
}

QgsGeometry* QgisApp::unionGeometries( const QgsVectorLayer* vl, QgsFeatureList& featureList, bool& canceled )
{
  canceled = false;
  if( !vl || featureList.size() < 2 )
  {
    return 0;
  }

  QgsGeometry* unionGeom = featureList[0].geometry();
  QgsGeometry* backupPtr = 0; //pointer to delete intermediate results
  if( !unionGeom )
  {
    return 0;
  }

  QProgressDialog progress( tr( "Merging features..." ), tr( "Abort" ), 0, featureList.size(), this );
  progress.setWindowModality( Qt::WindowModal );

  QApplication::setOverrideCursor( Qt::WaitCursor );

  for( int i = 1; i < featureList.size(); ++i )
  {
    if( progress.wasCanceled() )
    {
      delete unionGeom;
      QApplication::restoreOverrideCursor();
      canceled = true;
      return 0;
    }
    progress.setValue( i );
    QgsGeometry* currentGeom = featureList[i].geometry();
    if( currentGeom )
    {
      backupPtr = unionGeom;
      unionGeom = unionGeom->combine( currentGeom );
      if( !unionGeom )
      {
        delete backupPtr;
        QApplication::restoreOverrideCursor();
        return 0;
      }
      if( i > 1 )  //delete previous intermediate results
      {
        delete backupPtr;
        backupPtr = 0;
      }
    }
  }

  //convert unionGeom to a multipart geometry in case it is necessary to match the layer type
  QGis::WkbType t = vl->wkbType();
  bool layerIsMultiType = ( t == QGis::WKBMultiPoint || t == QGis::WKBMultiPoint25D || t == QGis::WKBMultiLineString \
                            || t == QGis::WKBMultiLineString25D || t == QGis::WKBMultiPolygon || t == QGis::WKBMultiPoint25D );
  if( layerIsMultiType && !unionGeom->isMultipart() )
  {
    unionGeom->convertToMultiType();
  }

  QApplication::restoreOverrideCursor();
  progress.setValue( featureList.size() );
  return unionGeom;
}

QgsComposer* QgisApp::createNewComposer()
{
  //ask user about name
  mLastComposerId++;
  //create new composer object
  QgsComposer* newComposerObject = new QgsComposer( this, tr( "Composer %1" ).arg( mLastComposerId ) );
  //add it to the map of existing print composers
  mPrintComposers.insert( newComposerObject );
  //and place action into print composers menu
  mPrintComposersMenu->addAction( newComposerObject->windowAction() );
  newComposerObject->open();
  emit composerAdded( newComposerObject->view() );
  return newComposerObject;
}

void QgisApp::deleteComposer( QgsComposer* c )
{
  emit composerWillBeRemoved( c->view() );
  mPrintComposers.remove( c );
  mPrintComposersMenu->removeAction( c->windowAction() );
  delete c;
}

bool QgisApp::loadComposersFromProject( const QDomDocument& doc )
{
  if( doc.isNull() )
  {
    return false;
  }

  //restore each composer
  QDomNodeList composerNodes = doc.elementsByTagName( "Composer" );
  for( int i = 0; i < composerNodes.size(); ++i )
  {
    ++mLastComposerId;
    QgsComposer* composer = new QgsComposer( this, tr( "Composer %1" ).arg( mLastComposerId ) );
    composer->readXML( composerNodes.at( i ).toElement(), doc );
    mPrintComposers.insert( composer );
    mPrintComposersMenu->addAction( composer->windowAction() );
#ifndef Q_OS_MACX
    composer->showMinimized();
#endif
    composer->zoomFull();
    if( composerNodes.at( i ).toElement().attribute( "visible", "1" ).toInt() < 1 )
    {
      composer->close();
    }
    emit composerAdded( composer->view() );
  }
  return true;
}

void QgisApp::deletePrintComposers()
{
  QSet<QgsComposer*>::iterator it = mPrintComposers.begin();
  for( ; it != mPrintComposers.end(); ++it )
  {
    emit composerWillBeRemoved(( *it )->view() );
    delete( *it );
  }
  mPrintComposers.clear();
  mLastComposerId = 0;
}

bool QgisApp::loadAnnotationItemsFromProject( const QDomDocument& doc )
{
  if( !mMapCanvas )
  {
    return false;
  }

  removeAnnotationItems();

  if( doc.isNull() )
  {
    return false;
  }

  QDomNodeList textItemList = doc.elementsByTagName( "TextAnnotationItem" );
  for( int i = 0; i < textItemList.size(); ++i )
  {
    QgsTextAnnotationItem* newTextItem = new QgsTextAnnotationItem( mMapCanvas );
    newTextItem->readXML( doc, textItemList.at( i ).toElement() );
  }

  QDomNodeList formItemList = doc.elementsByTagName( "FormAnnotationItem" );
  for( int i = 0; i < formItemList.size(); ++i )
  {
    QgsFormAnnotationItem* newFormItem = new QgsFormAnnotationItem( mMapCanvas );
    newFormItem->readXML( doc, formItemList.at( i ).toElement() );
  }
  return true;
}

QList<QgsAnnotationItem*> QgisApp::annotationItems()
{
  QList<QgsAnnotationItem*> itemList;

  if( !mMapCanvas )
  {
    return itemList;
  }

  if( mMapCanvas )
  {
    QList<QGraphicsItem*> graphicsItems = mMapCanvas->items();
    QList<QGraphicsItem*>::iterator gIt = graphicsItems.begin();
    for( ; gIt != graphicsItems.end(); ++gIt )
    {
      QgsAnnotationItem* currentItem = dynamic_cast<QgsAnnotationItem*>( *gIt );
      if( currentItem )
      {
        itemList.push_back( currentItem );
      }
    }
  }
  return itemList;
}

void QgisApp::removeAnnotationItems()
{
  if( !mMapCanvas )
  {
    return;
  }
  QGraphicsScene* scene = mMapCanvas->scene();
  if( !scene )
  {
    return;
  }
  QList<QgsAnnotationItem*> itemList = annotationItems();
  QList<QgsAnnotationItem*>::iterator itemIt = itemList.begin();
  for( ; itemIt != itemList.end(); ++itemIt )
  {
    if( *itemIt )
    {
      scene->removeItem( *itemIt );
      delete *itemIt;
    }
  }
}

void QgisApp::mergeSelectedFeatures()
{
  //get active layer (hopefully vector)
  QgsMapLayer* activeMapLayer = activeLayer();
  if( !activeMapLayer )
  {
    QMessageBox::information( 0, tr( "No active layer" ), tr( "No active layer found. Please select a layer in the layer list" ) );
    return;
  }
  QgsVectorLayer* vl = qobject_cast<QgsVectorLayer *>( activeMapLayer );
  if( !vl )
  {
    QMessageBox::information( 0, tr( "Active layer is not vector" ), tr( "The merge features tool only works on vector layers. Please select a vector layer from the layer list" ) );
    return;
  }
  if( !vl->isEditable() )
  {
    QMessageBox::information( 0, tr( "Layer not editable" ), tr( "Merging features can only be done for layers in editing mode. To use the merge tool, go to  Layer->Toggle editing" ) );
    return;
  }

  QgsVectorDataProvider* dp = vl->dataProvider();
  bool providerChecksTypeStrictly = true;
  if( dp )
  {
    providerChecksTypeStrictly = dp->doesStrictFeatureTypeCheck();
  }

  //get selected feature ids (as a QSet<int> )
  const QgsFeatureIds& featureIdSet = vl->selectedFeaturesIds();
  if( featureIdSet.size() < 2 )
  {
    QMessageBox::information( 0, tr( "Not enough features selected" ), tr( "The merge tool requires at least two selected features" ) );
    return;
  }

  //get initial selection (may be altered by attribute merge dialog later)
  QgsFeatureList featureList = vl->selectedFeatures();  //get QList<QgsFeature>
  bool canceled;
  QgsGeometry* unionGeom = unionGeometries( vl, featureList, canceled );
  if( !unionGeom )
  {
    if( !canceled )
    {
      QMessageBox::critical( 0, tr( "Merge failed" ), tr( "An error occured during the merge operation" ) );
    }
    return;
  }

  //make a first geometry union and notify the user straight away if the union geometry type does not match the layer one
  QGis::WkbType originalType = vl->wkbType();
  QGis::WkbType newType = unionGeom->wkbType();
  if( providerChecksTypeStrictly && unionGeom->wkbType() != vl->wkbType() )
  {
    QMessageBox::critical( 0, tr( "Union operation canceled" ), tr( "The union operation would result in a geometry type that is not compatible with the current layer and therefore is canceled" ) );
    delete unionGeom;
    return;
  }

  //merge the attributes together
  QgsMergeAttributesDialog d( featureList, vl, mapCanvas() );
  if( d.exec() == QDialog::Rejected )
  {
    delete unionGeom;
    return;
  }

  QgsFeatureList featureListAfter = vl->selectedFeatures();

  if( featureListAfter.size() < 2 )
  {
    QMessageBox::information( 0, tr( "Not enough features selected" ), tr( "The merge tool requires at least two selected features" ) );
    delete unionGeom;
    return;
  }

  //if the user changed the feature selection in the merge dialog, we need to repead the union and check the type
  if( featureList.size() != featureListAfter.size() )
  {
    delete unionGeom;
    bool canceled;
    unionGeom = unionGeometries( vl, featureListAfter, canceled );
    if( !unionGeom )
    {
      if( !canceled )
      {
        QMessageBox::critical( 0, tr( "Merge failed" ), tr( "An error occured during the merge operation" ) );
      }
      return;
    }

    originalType = vl->wkbType();
    newType = unionGeom->wkbType();
    if( providerChecksTypeStrictly && unionGeom->wkbType() != vl->wkbType() )
    {
      QMessageBox::critical( 0, "Union operation canceled", tr( "The union operation would result in a geometry type that is not compatible with the current layer and therefore is canceled" ) );
      delete unionGeom;
      return;
    }
  }

  vl->beginEditCommand( tr( "Merged features" ) );

  //create new feature
  QgsFeature newFeature;
  newFeature.setGeometry( unionGeom );
  newFeature.setAttributeMap( d.mergedAttributesMap() );

  QgsFeatureList::const_iterator feature_it = featureListAfter.constBegin();
  for( ; feature_it != featureListAfter.constEnd(); ++feature_it )
  {
    vl->deleteFeature( feature_it->id() );
  }

  vl->addFeature( newFeature, false );

  vl->endEditCommand();

  if( mapCanvas() )
  {
    mapCanvas()->refresh();
  }
}

void QgisApp::nodeTool()
{
  mMapCanvas->setMapTool( mMapTools.mNodeTool );
}

void QgisApp::rotatePointSymbols()
{
  mMapCanvas->setMapTool( mMapTools.mRotatePointSymbolsTool );
}

void QgisApp::splitFeatures()
{
  mMapCanvas->setMapTool( mMapTools.mSplitFeatures );
}

void QgisApp::reshapeFeatures()
{
  mMapCanvas->setMapTool( mMapTools.mReshapeFeatures );
}

void QgisApp::capturePoint()
{
  if( mMapCanvas && mMapCanvas->isDrawing() )
  {
    return;
  }

  // set current map tool to select
  mMapCanvas->setMapTool( mMapTools.mCapturePoint );
}

void QgisApp::captureLine()
{
  if( mMapCanvas && mMapCanvas->isDrawing() )
  {
    return;
  }

  mMapCanvas->setMapTool( mMapTools.mCaptureLine );
}

void QgisApp::capturePolygon()
{
  if( mMapCanvas && mMapCanvas->isDrawing() )
  {
    return;
  }
  mMapCanvas->setMapTool( mMapTools.mCapturePolygon );
}

void QgisApp::select()
{
  mMapCanvas->setMapTool( mMapTools.mSelect );
}

void QgisApp::selectByRectangle()
{
  mMapCanvas->setMapTool( mMapTools.mSelectRectangle );
}

void QgisApp::selectByPolygon()
{
  mMapCanvas->setMapTool( mMapTools.mSelectPolygon );
}

void QgisApp::selectByFreehand()
{
  mMapCanvas->setMapTool( mMapTools.mSelectFreehand );
}

void QgisApp::selectByRadius()
{
  mMapCanvas->setMapTool( mMapTools.mSelectRadius );
}

void QgisApp::deselectAll()
{
  if( !mMapCanvas || mMapCanvas->isDrawing() )
  {
    return;
  }

  // Turn off rendering to improve speed.
  bool renderFlagState = mMapCanvas->renderFlag();
  if( renderFlagState )
    mMapCanvas->setRenderFlag( false );

  QMap<QString, QgsMapLayer*> layers = QgsMapLayerRegistry::instance()->mapLayers();
  for( QMap<QString, QgsMapLayer*>::iterator it = layers.begin(); it != layers.end(); it++ )
  {
    QgsVectorLayer *vl = qobject_cast<QgsVectorLayer *>( it.value() );
    if( !vl )
      continue;

    vl->removeSelection();
  }

  // Turn on rendering (if it was on previously)
  if( renderFlagState )
    mMapCanvas->setRenderFlag( true );
}

void QgisApp::addVertex()
{
  if( mMapCanvas && mMapCanvas->isDrawing() )
  {
    return;
  }
  mMapCanvas->setMapTool( mMapTools.mVertexAdd );

}

void QgisApp::moveVertex()
{
  if( mMapCanvas && mMapCanvas->isDrawing() )
  {
    return;
  }
  mMapCanvas->setMapTool( mMapTools.mVertexMove );
}

void QgisApp::addRing()
{
  if( mMapCanvas && mMapCanvas->isDrawing() )
  {
    return;
  }
  mMapCanvas->setMapTool( mMapTools.mAddRing );
}

void QgisApp::addIsland()
{
  if( mMapCanvas && mMapCanvas->isDrawing() )
  {
    return;
  }
  mMapCanvas->setMapTool( mMapTools.mAddIsland );
}


void QgisApp::deleteVertex()
{
  if( mMapCanvas && mMapCanvas->isDrawing() )
  {
    return;
  }
  mMapCanvas->setMapTool( mMapTools.mVertexDelete );
}


void QgisApp::editCut( QgsMapLayer * layerContainingSelection )
{
  if( mMapCanvas && mMapCanvas->isDrawing() )
  {
    return;
  }

  QgsMapLayer *selectionLayer = layerContainingSelection ? layerContainingSelection : activeLayer();

  if( selectionLayer )
  {
    // Test for feature support in this layer
    QgsVectorLayer* selectionVectorLayer = qobject_cast<QgsVectorLayer *>( selectionLayer );

    if( selectionVectorLayer != 0 )
    {
      QgsFeatureList features = selectionVectorLayer->selectedFeatures();
      clipboard()->replaceWithCopyOf( selectionVectorLayer->pendingFields(), features );
      clipboard()->setCRS( selectionVectorLayer->srs() );
      selectionVectorLayer->beginEditCommand( tr( "Features cut" ) );
      selectionVectorLayer->deleteSelectedFeatures();
      selectionVectorLayer->endEditCommand();
    }
  }
}


void QgisApp::editCopy( QgsMapLayer * layerContainingSelection )
{
  if( mMapCanvas && mMapCanvas->isDrawing() )
  {
    return;
  }

  QgsMapLayer *selectionLayer = layerContainingSelection ? layerContainingSelection : activeLayer();

  if( selectionLayer )
  {
    // Test for feature support in this layer
    QgsVectorLayer* selectionVectorLayer = qobject_cast<QgsVectorLayer *>( selectionLayer );

    if( selectionVectorLayer != 0 )
    {
      QgsFeatureList features = selectionVectorLayer->selectedFeatures();
      clipboard()->replaceWithCopyOf( selectionVectorLayer->pendingFields(), features );
      clipboard()->setCRS( selectionVectorLayer->srs() );
    }
  }
}


void QgisApp::editPaste( QgsMapLayer *destinationLayer )
{
  if( mMapCanvas && mMapCanvas->isDrawing() )
  {
    return;
  }

  QgsMapLayer *pasteLayer = destinationLayer ? destinationLayer : activeLayer();

  if( pasteLayer )
  {
    // Test for feature support in this layer
    QgsVectorLayer* pasteVectorLayer = qobject_cast<QgsVectorLayer *>( pasteLayer );

    if( pasteVectorLayer != 0 )
    {
      pasteVectorLayer->beginEditCommand( tr( "Features pasted" ) );
      QgsFeatureList features;
      if( mMapCanvas->mapRenderer()->hasCrsTransformEnabled() )
      {
        features = clipboard()->transformedCopyOf( pasteVectorLayer->srs() );
      }
      else
      {
        features = clipboard()->copyOf();
      }

      QgsAttributeList dstAttr = pasteVectorLayer->pendingAllAttributesList();

      for( int i = 0; i < features.size(); i++ )
      {
        QgsFeature &f = features[i];
        QgsAttributeMap srcMap = f.attributeMap();
        QgsAttributeMap dstMap;

        int j = 0;
        foreach( int id, srcMap.keys() )
        {
          if( j >= dstAttr.size() )
            break;

          dstMap[ dstAttr[j++] ] = srcMap[id];
        }

        f.setAttributeMap( dstMap );
      }

      pasteVectorLayer->addFeatures( features );
      pasteVectorLayer->endEditCommand();
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
  //clear all caches first
  QgsMapLayerRegistry::instance()->clearAllLayerCaches();
  //reload cached provider data
  QgsMapLayerRegistry::instance()->reloadAllLayers();
  //then refresh
  mMapCanvas->refresh();
}

void QgisApp::toggleMapTips()
{
  mMapTipsVisible = !mMapTipsVisible;
  // if off, stop the timer
  if( !mMapTipsVisible )
  {
    mpMapTipsTimer->stop();
  }
}

void QgisApp::toggleEditing()
{
  if( mMapCanvas && mMapCanvas->isDrawing() )
    return;

  QgsVectorLayer *currentLayer = qobject_cast<QgsVectorLayer*>( activeLayer() );
  if( currentLayer )
  {
    toggleEditing( currentLayer, true );
  }
  else
  {
    // active although there's no layer active!?
    mActionToggleEditing->setChecked( false );
  }
}

void QgisApp::saveEdits()
{
  if( mMapCanvas && mMapCanvas->isDrawing() )
    return;

  QgsVectorLayer *vlayer = qobject_cast<QgsVectorLayer *>( activeLayer() ); // FIXME: save edits of all selected layers
  if( !vlayer || !vlayer->isEditable() || !vlayer->isModified() )
    return;

  if( !vlayer->commitChanges() )
  {
    QMessageBox::information( 0,
                              tr( "Error" ),
                              tr( "Could not commit changes to layer %1\n\nErrors: %2\n" )
                              .arg( vlayer->name() )
                              .arg( vlayer->commitErrors().join( "\n  " ) ) );
  }

  vlayer->startEditing();
  vlayer->triggerRepaint();
}

void QgisApp::layerSubsetString()
{
  if( mMapCanvas && mMapCanvas->isDrawing() )
    return;

  QgsVectorLayer *vlayer = qobject_cast<QgsVectorLayer *>( activeLayer() );
  if( !vlayer )
    return;

  // launch the query builder
  QgsQueryBuilder *qb = new QgsQueryBuilder( vlayer, this );

  // Set the sql in the query builder to the same in the prop dialog
  // (in case the user has already changed it)
  qb->setSql( vlayer->subsetString() );
  // Open the query builder
  if( qb->exec() )
  {
    // if the sql is changed, update it in the prop subset text box
    vlayer->setSubsetString( qb->sql() );
    mMapCanvas->refresh();
  }

  // delete the query builder object
  delete qb;
}


bool QgisApp::toggleEditing( QgsMapLayer *layer, bool allowCancel )
{
  QgsVectorLayer *vlayer = qobject_cast<QgsVectorLayer *>( layer );
  if( !vlayer )
  {
    return false;
  }

  bool res = true;

  if( !vlayer->isEditable() && !vlayer->isReadOnly() )
  {
    vlayer->startEditing();
    if( !( vlayer->dataProvider()->capabilities() & QgsVectorDataProvider::EditingCapabilities ) )
    {
      QMessageBox::information( 0, tr( "Start editing failed" ), tr( "Provider cannot be opened for editing" ) );
      res = false;
    }
  }
  else if( vlayer->isModified() )
  {
    QMessageBox::StandardButtons buttons = QMessageBox::Save | QMessageBox::Discard;
    if( allowCancel )
      buttons |= QMessageBox::Cancel;

    switch( QMessageBox::information( 0,
                                      tr( "Stop editing" ),
                                      tr( "Do you want to save the changes to layer %1?" ).arg( vlayer->name() ),
                                      buttons ) )
    {
      case QMessageBox::Cancel:
        res = false;
        break;

      case QMessageBox::Save:
        if( !vlayer->commitChanges() )
        {
          QMessageBox::information( 0,
                                    tr( "Error" ),
                                    tr( "Could not commit changes to layer %1\n\nErrors: %2\n" )
                                    .arg( vlayer->name() )
                                    .arg( vlayer->commitErrors().join( "\n  " ) ) );
          // Leave the in-memory editing state alone,
          // to give the user a chance to enter different values
          // and try the commit again later
          res = false;
        }
        break;

      case QMessageBox::Discard:
        if( !vlayer->rollBack() )
        {
          QMessageBox::information( 0, tr( "Error" ), tr( "Problems during roll back" ) );
          res = false;
        }
        break;

      default:
        break;
    }
  }
  else //layer not modified
  {
    vlayer->rollBack();
    res = true;
  }

  if( layer == activeLayer() )
  {
    activateDeactivateLayerRelatedActions( layer );
  }

  vlayer->triggerRepaint();

  return res;
}

void QgisApp::showMouseCoordinate( const QgsPoint & p )
{
  if( mMapTipsVisible )
  {
    // store the point, we need it for when the maptips timer fires
    mLastMapPosition = p;

    // we use this slot to control the timer for maptips since it is fired each time
    // the mouse moves.
    if( mMapCanvas->underMouse() )
    {
      // Clear the maptip (this is done conditionally)
      mpMaptip->clear( mMapCanvas );
      // don't start the timer if the mouse is not over the map canvas
      mpMapTipsTimer->start();
      //QgsDebugMsg("Started maptips timer");
    }
  }
  if( mToggleExtentsViewButton->isChecked() )
  {
    //we are in show extents mode so no need to do anything
    return;
  }
  else
  {
    if( mMapCanvas->mapUnits() == QGis::DegreesMinutesSeconds )
    {
      mCoordsEdit->setText( p.toDegreesMinutesSeconds( mMousePrecisionDecimalPlaces ) );
    }
    else
    {
      mCoordsEdit->setText( p.toString( mMousePrecisionDecimalPlaces ) );
    }
    if( mCoordsEdit->width() > mCoordsEdit->minimumWidth() )
    {
      mCoordsEdit->setMinimumWidth( mCoordsEdit->width() );
    }
  }
}


void QgisApp::showScale( double theScale )
{
  if( theScale >= 1.0 )
    mScaleEdit->setText( "1:" + QString::number( theScale, 'f', 0 ) );
  else if( theScale > 0.0 )
    mScaleEdit->setText( QString::number( 1.0 / theScale, 'f', 0 ) + ":1" );
  else
    mScaleEdit->setText( tr( "Invalid scale" ) );

  // Set minimum necessary width
  if( mScaleEdit->width() > mScaleEdit->minimumWidth() )
  {
    mScaleEdit->setMinimumWidth( mScaleEdit->width() );
  }
}

void QgisApp::userScale()
{
  QStringList parts = mScaleEdit->text().split( ':' );
  if( parts.size() == 2 )
  {
    bool leftOk, rightOk;
    double leftSide = parts.at( 0 ).toDouble( &leftOk );
    double rightSide = parts.at( 1 ).toDouble( &rightOk );
    if( leftSide > 0.0 && leftOk && rightOk )
    {
      mMapCanvas->zoomScale( rightSide / leftSide );
    }
  }
  else
  {
    bool rightOk;
    double rightSide = parts.at( 0 ).toDouble( &rightOk );
    if( rightOk )
    {
      mMapCanvas->zoomScale( rightSide );
    }
  }
}

void QgisApp::userCenter()
{
  QStringList parts = mCoordsEdit->text().split( ',' );
  if( parts.size() != 2 )
    return;

  bool xOk;
  double x = parts.at( 0 ).toDouble( &xOk );
  if( !xOk )
    return;

  bool yOk;
  double y = parts.at( 1 ).toDouble( &yOk );
  if( !yOk )
    return;

  QgsRectangle r = mMapCanvas->extent();

  mMapCanvas->setExtent(
    QgsRectangle(
      x - r.width() / 2.0,  y - r.height() / 2.0,
      x + r.width() / 2.0, y + r.height() / 2.0
    )
  );
  mMapCanvas->refresh();
}


// toggle overview status
void QgisApp::isInOverview()
{
  mMapLegend->legendLayerShowInOverview();
}

void QgisApp::removingLayer( QString layerId )
{
  QgsVectorLayer *vlayer = qobject_cast<QgsVectorLayer*>( QgsMapLayerRegistry::instance()->mapLayer( layerId ) );
  if( !vlayer || !vlayer->isEditable() )
    return;

  toggleEditing( vlayer, false );
}

void QgisApp::removeAllLayers()
{
  QgsMapLayerRegistry::instance()->removeAllMapLayers();
}

void QgisApp::removeLayer()
{
  if( mMapCanvas && mMapCanvas->isDrawing() )
  {
    return;
  }

  if( !mMapLegend )
  {
    return;
  }

  foreach( QgsMapLayer * layer, mMapLegend->selectedLayers() )
  {
    QgsVectorLayer *vlayer = qobject_cast<QgsVectorLayer*>( layer );
    if( vlayer && vlayer->isEditable() && !toggleEditing( vlayer, true ) )
      return;
  }

  mMapLegend->removeSelectedLayers();

  mMapCanvas->refresh();
}

void QgisApp::showGpsTool()
{
  if( !mpGpsWidget )
  {
    mpGpsWidget = new QgsGPSInformationWidget( mMapCanvas );
    //create the dock widget
    mpGpsDock = new QDockWidget( tr( "GPS Information" ), this );
    mpGpsDock->setObjectName( "GPSInformation" );
    mpGpsDock->setAllowedAreas( Qt::LeftDockWidgetArea | Qt::RightDockWidgetArea );
    addDockWidget( Qt::LeftDockWidgetArea, mpGpsDock );
    // add to the Panel submenu
    mPanelMenu->addAction( mpGpsDock->toggleViewAction() );
    // now add our widget to the dock - ownership of the widget is passed to the dock
    mpGpsDock->setWidget( mpGpsWidget );
    mpGpsWidget->show();
  }
  else
  {
    mpGpsDock->setVisible( mpGpsDock->isHidden() );
  }
}

void QgisApp::showTileScale()
{
  if( !mpTileScaleWidget )
  {
    mpTileScaleWidget = new QgsTileScaleWidget( mMapCanvas );
    //create the dock widget
    mpTileScaleDock = new QDockWidget( tr( "Tile scale" ), this );
    mpTileScaleDock->setObjectName( "TileScale" );
    mpTileScaleDock->setAllowedAreas( Qt::LeftDockWidgetArea | Qt::RightDockWidgetArea );
    addDockWidget( Qt::RightDockWidgetArea, mpTileScaleDock );
    // add to the Panel submenu
    mPanelMenu->addAction( mpTileScaleDock->toggleViewAction() );
    // now add our widget to the dock - ownership of the widget is passed to the dock
    mpTileScaleDock->setWidget( mpTileScaleWidget );
    mpTileScaleDock->show();

    connect( mMapLegend, SIGNAL( currentLayerChanged( QgsMapLayer* ) ),
             mpTileScaleWidget, SLOT( layerChanged( QgsMapLayer* ) ) );

  }
  else
  {
    mpTileScaleDock->setVisible( mpTileScaleDock->isHidden() );
  }
}

void QgisApp::zoomToLayerExtent()
{
  mMapLegend->legendLayerZoom();
}

void QgisApp::showPluginManager()
{
  QgsPluginManager *pm = new QgsPluginManager( mPythonUtils, this );
  pm->resizeColumnsToContents();
  if( pm->exec() )
  {
    QgsPluginRegistry* pRegistry = QgsPluginRegistry::instance();
    // load selected plugins
    std::vector < QgsPluginItem > pi = pm->getSelectedPlugins();
    std::vector < QgsPluginItem >::iterator it = pi.begin();
    while( it != pi.end() )
    {
      QgsPluginItem plugin = *it;
      if( plugin.isPython() )
      {
        pRegistry->loadPythonPlugin( plugin.fullPath() );
      }
      else
      {
        pRegistry->loadCppPlugin( plugin.fullPath() );
      }
      it++;
    }
  }
}

static void _runPythonString( const QString &expr )
{
  QgisApp::instance()->runPythonString( expr );
}

void QgisApp::loadPythonSupport()
{
  QString pythonlibName( "qgispython" );
#if defined(Q_WS_MAC) || defined(Q_OS_LINUX)
  pythonlibName.prepend( QgsApplication::prefixPath() + "/" + QGIS_LIB_SUBDIR + "/" );
#endif
#ifdef __MINGW32__
  pythonlibName.prepend( "lib" );
#endif
  QString version = QString( "%1.%2.%3" ).arg( QGis::QGIS_VERSION_INT / 10000 ).arg( QGis::QGIS_VERSION_INT / 100 % 100 ).arg( QGis::QGIS_VERSION_INT % 100 );
  QgsDebugMsg( QString( "load library %1 (%2)" ).arg( pythonlibName ).arg( version ) );
  QLibrary pythonlib( pythonlibName, version );
  // It's necessary to set these two load hints, otherwise Python library won't work correctly
  // see http://lists.kde.org/?l=pykde&m=117190116820758&w=2
  pythonlib.setLoadHints( QLibrary::ResolveAllSymbolsHint | QLibrary::ExportExternalSymbolsHint );
  if( !pythonlib.load() )
  {
    //using stderr on purpose because we want end users to see this [TS]
    QgsDebugMsg( "Couldn't load Python support library: " + pythonlib.errorString() );
    pythonlib.setFileName( pythonlibName );
    if( !pythonlib.load() )
    {
      qWarning( "Couldn't load Python support library: %s", pythonlib.errorString().toUtf8().data() );
      return;
    }
  }

  //QgsDebugMsg("Python support library loaded successfully.");
  typedef QgsPythonUtils*( *inst )();
  inst pythonlib_inst = ( inst ) cast_to_fptr( pythonlib.resolve( "instance" ) );
  if( !pythonlib_inst )
  {
    //using stderr on purpose because we want end users to see this [TS]
    QgsDebugMsg( "Couldn't resolve python support library's instance() symbol." );
    return;
  }

  //QgsDebugMsg("Python support library's instance() symbol resolved.");
  mPythonUtils = pythonlib_inst();
  mPythonUtils->initPython( mQgisInterface );

  if( mPythonUtils && mPythonUtils->isEnabled() )
  {
    QgsPluginRegistry::instance()->setPythonUtils( mPythonUtils );
    QgsAttributeAction::setPythonExecute( _runPythonString );

    mActionShowPythonDialog = new QAction( tr( "Python Console" ), this );
    QgsShortcutsManager::instance()->registerAction( mActionShowPythonDialog );
    connect( mActionShowPythonDialog, SIGNAL( triggered() ), this, SLOT( showPythonDialog() ) );

    mActionPluginSeparator2 = mPluginMenu->addSeparator();
    mPluginMenu->addAction( mActionShowPythonDialog );
    std::cout << "Python support ENABLED :-) " << std::endl; // OK
  }
}

void QgisApp::checkQgisVersion()
{
  QApplication::setOverrideCursor( Qt::WaitCursor );
  /* QUrlOperator op = new QUrlOperator( "http://mrcc.com/qgis/version.txt" );
     connect(op, SIGNAL(data()), SLOT(urlData()));
     connect(op, SIGNAL(finished(QNetworkOperation)), SLOT(urlFinished(QNetworkOperation)));

     op.get(); */
  mSocket = new QTcpSocket( this );
  connect( mSocket, SIGNAL( connected() ), SLOT( socketConnected() ) );
  connect( mSocket, SIGNAL( connectionClosed() ), SLOT( socketConnectionClosed() ) );
  connect( mSocket, SIGNAL( readyRead() ), SLOT( socketReadyRead() ) );
  connect( mSocket, SIGNAL( error( QAbstractSocket::SocketError ) ),
           SLOT( socketError( QAbstractSocket::SocketError ) ) );
  mSocket->connectToHost( "mrcc.com", 80 );
}

void QgisApp::socketConnected()
{
  QTextStream os( mSocket );
  mVersionMessage = "";
  // send the qgis version string
  // os << QGIS_VERSION << "\r\n";
  os << "GET /qgis/version.txt HTTP/1.0\n\n";


}

void QgisApp::socketConnectionClosed()
{
  QApplication::restoreOverrideCursor();
  // strip the header
  QString contentFlag = "#QGIS Version";
  int pos = mVersionMessage.indexOf( contentFlag );
  if( pos > -1 )
  {
    pos += contentFlag.length();
// QgsDebugMsg(mVersionMessage);
// QgsDebugMsg(QString("Pos is %1").arg(pos));
    mVersionMessage = mVersionMessage.mid( pos );
    QStringList parts = mVersionMessage.split( "|", QString::SkipEmptyParts );
    // check the version from the  server against our version
    QString versionInfo;
    int currentVersion = parts[0].toInt();
    if( currentVersion > QGis::QGIS_VERSION_INT )
    {
      // show version message from server
      versionInfo = tr( "There is a new version of QGIS available" ) + "\n";
    }
    else
    {
      if( QGis::QGIS_VERSION_INT > currentVersion )
      {
        versionInfo = tr( "You are running a development version of QGIS" ) + "\n";
      }
      else
      {
        versionInfo = tr( "You are running the current version of QGIS" ) + "\n";
      }
    }
    if( parts.count() > 1 )
    {
      versionInfo += parts[1] + "\n\n" + tr( "Would you like more information?" );
      ;
      QMessageBox::StandardButton result = QMessageBox::information( this,
                                           tr( "QGIS Version Information" ), versionInfo, QMessageBox::Ok |
                                           QMessageBox::Cancel );
      if( result == QMessageBox::Ok )
      {
        // show more info
        QgsMessageViewer *mv = new QgsMessageViewer( this );
        mv->setWindowTitle( tr( "QGIS - Changes in SVN since last release" ) );
        mv->setMessageAsHtml( parts[2] );
        mv->exec();
      }
    }
    else
    {
      QMessageBox::information( this, tr( "QGIS Version Information" ), versionInfo );
    }
  }
  else
  {
    QMessageBox::warning( this, tr( "QGIS Version Information" ), tr( "Unable to get current version information from server" ) );
  }
}
void QgisApp::socketError( QAbstractSocket::SocketError e )
{
  if( e == QAbstractSocket::RemoteHostClosedError )
    return;

  QApplication::restoreOverrideCursor();
  // get error type
  QString detail;
  switch( e )
  {
    case QAbstractSocket::ConnectionRefusedError:
      detail = tr( "Connection refused - server may be down" );
      break;
    case QAbstractSocket::HostNotFoundError:
      detail = tr( "QGIS server was not found" );
      break;
    case QAbstractSocket::NetworkError:
      detail = tr( "Network error while communicating with server" );
      break;
    default:
      detail = tr( "Unknown network socket error" );
      break;
  }

  // show version message from server
  QMessageBox::critical( this, tr( "QGIS Version Information" ), tr( "Unable to communicate with QGIS Version server\n%1" ).arg( detail ) );
}

void QgisApp::socketReadyRead()
{
  while( mSocket->bytesAvailable() > 0 )
  {
    char *data = new char[mSocket->bytesAvailable() + 1];
    memset( data, '\0', mSocket->bytesAvailable() + 1 );
    mSocket->read( data, mSocket->bytesAvailable() );
    mVersionMessage += data;
    delete[]data;
  }

}

void QgisApp::configureShortcuts()
{
  QgsConfigureShortcutsDialog dlg;
  dlg.exec();
}


void QgisApp::options()
{
  if( mMapCanvas && mMapCanvas->isDrawing() )
  {
    return;
  }

  QgsOptions *optionsDialog = new QgsOptions( this );
  if( optionsDialog->exec() )
  {
    // set the theme if it changed
    setTheme( optionsDialog->theme() );

    QSettings mySettings;
    mMapCanvas->enableAntiAliasing( mySettings.value( "/qgis/enable_anti_aliasing" ).toBool() );
    mMapCanvas->useImageToRender( mySettings.value( "/qgis/use_qimage_to_render" ).toBool() );

    int action = mySettings.value( "/qgis/wheel_action", 0 ).toInt();
    double zoomFactor = mySettings.value( "/qgis/zoom_factor", 2 ).toDouble();
    mMapCanvas->setWheelAction(( QgsMapCanvas::WheelAction ) action, zoomFactor );
  }

  delete optionsDialog;
}

void QgisApp::helpContents()
{
  openURL( "index.html" );
}

void QgisApp::helpQgisHomePage()
{
  openURL( "http://qgis.org", false );
}

void QgisApp::openURL( QString url, bool useQgisDocDirectory )
{
  // open help in user browser
  if( useQgisDocDirectory )
  {
    url = "file://" + QgsApplication::pkgDataPath() + "/doc/" + url;
  }
#ifdef Q_OS_MACX
  /* Use Mac OS X Launch Services which uses the user's default browser
   * and will just open a new window if that browser is already running.
   * QProcess creates a new browser process for each invocation and expects a
   * commandline application rather than a bundled application.
   */
  CFURLRef urlRef = CFURLCreateWithBytes( kCFAllocatorDefault,
                                          reinterpret_cast<const UInt8*>( url.toUtf8().data() ), url.length(),
                                          kCFStringEncodingUTF8, NULL );
  OSStatus status = LSOpenCFURLRef( urlRef, NULL );
  status = 0; //avoid compiler warning
  CFRelease( urlRef );
#elif defined(WIN32)
  if( url.startsWith( "file://", Qt::CaseInsensitive ) )
    ShellExecute( 0, 0, url.mid( 7 ).toLocal8Bit().constData(), 0, 0, SW_SHOWNORMAL );
  else
    QDesktopServices::openUrl( url );
#else
  QDesktopServices::openUrl( url );
#endif
}

/** Get a pointer to the currently selected map layer */
QgsMapLayer *QgisApp::activeLayer()
{
  return mMapLegend ? mMapLegend->currentLayer() : 0;
}

/** set the current layer */
bool QgisApp::setActiveLayer( QgsMapLayer *layer )
{
  if( !layer )
    return false;

  return mMapLegend->setCurrentLayer( layer );
}

/** Add a vector layer directly without prompting user for location
  The caller must provide information compatible with the provider plugin
  using the vectorLayerPath and baseName. The provider can use these
  parameters in any way necessary to initialize the layer. The baseName
  parameter is used in the Map Legend so it should be formed in a meaningful
  way.
  */
QgsVectorLayer* QgisApp::addVectorLayer( QString vectorLayerPath, QString baseName, QString providerKey )
{
  if( mMapCanvas && mMapCanvas->isDrawing() )
  {
    return NULL;
  }

  mMapCanvas->freeze();

// Let render() do its own cursor management
//  QApplication::setOverrideCursor(Qt::WaitCursor);

  /* Eliminate the need to instantiate the layer based on provider type.
     The caller is responsible for cobbling together the needed information to
     open the layer
     */
  QgsDebugMsg( "Creating new vector layer using " + vectorLayerPath
               + " with baseName of " + baseName
               + " and providerKey of " + providerKey );

  // create the layer
  QgsVectorLayer *layer = new QgsVectorLayer( vectorLayerPath, baseName, providerKey );

  if( layer && layer->isValid() )
  {
    // Register this layer with the layers registry
    QgsMapLayerRegistry::instance()->addMapLayer( layer );
    statusBar()->showMessage( mMapCanvas->extent().toString( 2 ) );

  }
  else
  {
    QMessageBox::critical( this, tr( "Layer is not valid" ),
                           tr( "The layer %1 is not a valid layer and can not be added to the map" ).arg( vectorLayerPath ) );

    delete layer;
    mMapCanvas->freeze( false );
    return NULL;
  }

  // update UI
  qApp->processEvents();

  // draw the map
  mMapCanvas->freeze( false );
  mMapCanvas->refresh();

// Let render() do its own cursor management
//  QApplication::restoreOverrideCursor();

  return layer;

} // QgisApp::addVectorLayer



void QgisApp::addMapLayer( QgsMapLayer *theMapLayer )
{
  mMapCanvas->freeze();

// Let render() do its own cursor management
//  QApplication::setOverrideCursor(Qt::WaitCursor);

  if( theMapLayer->isValid() )
  {
    // Register this layer with the layers registry
    QgsMapLayerRegistry::instance()->addMapLayer( theMapLayer );
    // add it to the mapcanvas collection
    // not necessary since adding to registry adds to canvas mMapCanvas->addLayer(theMapLayer);

    statusBar()->showMessage( mMapCanvas->extent().toString( 2 ) );
  }
  else
  {
    QMessageBox::critical( this, tr( "Layer is not valid" ),
                           tr( "The layer is not a valid layer and can not be added to the map" ) );
  }

  // update UI
  qApp->processEvents();

  // draw the map
  mMapCanvas->freeze( false );
  mMapCanvas->refresh();

// Let render() do its own cursor management
//  QApplication::restoreOverrideCursor();

}

void QgisApp::setExtent( QgsRectangle theRect )
{
  mMapCanvas->setExtent( theRect );
}

/**
  Prompt and save if project has been modified.
  @return true if saved or discarded, false if cancelled
 */
bool QgisApp::saveDirty()
{
  QMessageBox::StandardButton answer( QMessageBox::Discard );
  mMapCanvas->freeze( true );

  //QgsDebugMsg(QString("Layer count is %1").arg(mMapCanvas->layerCount()));
  //QgsDebugMsg(QString("Project is %1dirty").arg( QgsProject::instance()->isDirty() ? "" : "not "));
  //QgsDebugMsg(QString("Map canvas is %1dirty").arg(mMapCanvas->isDirty() ? "" : "not "));

  QSettings settings;
  bool askThem = settings.value( "qgis/askToSaveProjectChanges", true ).toBool();

  if( askThem && ( QgsProject::instance()->isDirty() || mMapCanvas->isDirty() ) && mMapCanvas->layerCount() > 0 )
  {
    // flag project as dirty since dirty state of canvas is reset if "dirty"
    // is based on a zoom or pan
    markDirty();

    // old code: mProjectIsDirtyFlag = true;

    // prompt user to save
    answer = QMessageBox::information( this, tr( "Save?" ),
                                       tr( "Do you want to save the current project?" ),
                                       QMessageBox::Save | QMessageBox::Cancel | QMessageBox::Discard );
    if( QMessageBox::Save == answer )
    {
      if( !fileSave() )
        answer = QMessageBox::Cancel;
    }
  }

  mMapCanvas->freeze( false );

  return answer != QMessageBox::Cancel;
} // QgisApp::saveDirty()


void QgisApp::changeEvent( QEvent* event )
{
  QMainWindow::changeEvent( event );
#ifdef Q_WS_MAC
  switch( event->type() )
  {
    case QEvent::ActivationChange:
      if( QApplication::activeWindow() == this )
      {
        mWindowAction->setChecked( true );
      }
      // this should not be necessary since the action is part of an action group
      // however this check is not cleared if PrintComposer is closed and reopened
      else
      {
        mWindowAction->setChecked( false );
      }
      break;

    case QEvent::WindowTitleChange:
      mWindowAction->setText( windowTitle() );
      break;

    default:
      break;
  }
#endif
}

void QgisApp::closeEvent( QCloseEvent* event )
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

QMenu* QgisApp::getPluginMenu( QString menuName )
{
  /* Plugin menu items are below the plugin separator (which may not exist yet
   * if no plugins are loaded) and above the python separator. If python is not
   * present, there is no python separator and the plugin list is at the bottom
   * of the menu.
   */

#ifdef Q_WS_MAC
  // Mac doesn't have '&' keyboard shortcuts.
  menuName.remove( QChar( '&' ) );
#endif
  QAction *before = mActionPluginSeparator2;  // python separator or end of list
  if( !mActionPluginSeparator1 )
  {
    // First plugin - create plugin list separator
    mActionPluginSeparator1 = mPluginMenu->insertSeparator( before );
  }
  else
  {
    QString dst = menuName;
    dst.remove( QChar( '&' ) );

    // Plugins exist - search between plugin separator and python separator or end of list
    QList<QAction*> actions = mPluginMenu->actions();
    int end = mActionPluginSeparator2 ? actions.indexOf( mActionPluginSeparator2 ) : actions.count();
    for( int i = actions.indexOf( mActionPluginSeparator1 ) + 1; i < end; i++ )
    {
      QString src = actions.at( i )->text();
      src.remove( QChar( '&' ) );

      int comp = dst.localeAwareCompare( src );
      if( comp < 0 )
      {
        // Add item before this one
        before = actions.at( i );
        break;
      }
      else if( comp == 0 )
      {
        // Plugin menu item already exists
        return actions.at( i )->menu();
      }
    }
  }
  // It doesn't exist, so create
  QMenu *menu = new QMenu( menuName, this );
  // Where to put it? - we worked that out above...
  mPluginMenu->insertMenu( before, menu );

  return menu;
}

void QgisApp::addPluginToMenu( QString name, QAction* action )
{
  QMenu* menu = getPluginMenu( name );
  menu->addAction( action );
}

void QgisApp::removePluginMenu( QString name, QAction* action )
{
  QMenu* menu = getPluginMenu( name );
  menu->removeAction( action );
  if( menu->actions().count() == 0 )
  {
    mPluginMenu->removeAction( menu->menuAction() );
  }
  // Remove separator above plugins in Plugin menu if no plugins remain
  QList<QAction*> actions = mPluginMenu->actions();
  int end = mActionPluginSeparator2 ? actions.indexOf( mActionPluginSeparator2 ) : actions.count();
  if( actions.indexOf( mActionPluginSeparator1 ) + 1 == end )
  {
    mPluginMenu->removeAction( mActionPluginSeparator1 );
    mActionPluginSeparator1 = NULL;
  }
}

int QgisApp::addPluginToolBarIcon( QAction * qAction )
{
  mPluginToolBar->addAction( qAction );
  return 0;
}
void QgisApp::removePluginToolBarIcon( QAction *qAction )
{
  mPluginToolBar->removeAction( qAction );
}

void QgisApp::destinationSrsChanged()
{
  // save this information to project
  long srsid = mMapCanvas->mapRenderer()->destinationSrs().srsid();
  QgsProject::instance()->writeEntry( "SpatialRefSys", "/ProjectCRSID", ( int )srsid );

}

void QgisApp::hasCrsTransformEnabled( bool theFlag )
{
  // save this information to project
  QgsProject::instance()->writeEntry( "SpatialRefSys", "/ProjectionsEnabled", ( theFlag ? 1 : 0 ) );

  // update icon
  if( theFlag )
  {
    mOnTheFlyProjectionStatusButton->setIcon(
      getThemeIcon( "mIconProjectionEnabled.png" ) );
  }
  else
  {
    mOnTheFlyProjectionStatusButton->setIcon(
      getThemeIcon( "mIconProjectionDisabled.png" ) );
  }
}
// slot to update the progress bar in the status bar
void QgisApp::showProgress( int theProgress, int theTotalSteps )
{
  if( theProgress == theTotalSteps )
  {
    mProgressBar->reset();
    mProgressBar->hide();
  }
  else
  {
    //only call show if not already hidden to reduce flicker
    if( !mProgressBar->isVisible() )
    {
      mProgressBar->show();
    }
    mProgressBar->setMaximum( theTotalSteps );
    mProgressBar->setValue( theProgress );
  }
}

void QgisApp::mapToolChanged( QgsMapTool *tool )
{
  if( tool && !tool->isEditTool() )
  {
    mNonEditMapTool = tool;
  }
}

void QgisApp::extentsViewToggled( bool theFlag )
{
  if( theFlag )
  {
    //extents view mode!
    mToggleExtentsViewButton->setIcon( getThemeIcon( "extents.png" ) );
    mCoordsEdit->setToolTip( tr( "Map coordinates for the current view extents" ) );
    mCoordsEdit->setReadOnly( true );
    showExtents();
  }
  else
  {
    //mouse cursor pos view mode!
    mToggleExtentsViewButton->setIcon( getThemeIcon( "tracking.png" ) );
    mCoordsEdit->setToolTip( tr( "Map coordinates at mouse cursor position" ) );
    mCoordsEdit->setReadOnly( false );
    mCoordsLabel->setText( tr( "Coordinate:" ) );
  }
}

void QgisApp::markDirty()
{
  // notify the project that there was a change
  QgsProject::instance()->dirty( true );
}

void QgisApp::layerWasAdded( QgsMapLayer *layer )
{
  QgsDataProvider *provider = 0;

  QgsVectorLayer *vlayer = qobject_cast<QgsVectorLayer *>( layer );
  if( vlayer )
    provider = vlayer->dataProvider();

  QgsRasterLayer *rlayer = qobject_cast<QgsRasterLayer *>( layer );
  if( rlayer )
    provider = rlayer->dataProvider();

  if( provider )
  {
    connect( provider, SIGNAL( dataChanged() ), layer, SLOT( clearCacheImage() ) );
    connect( provider, SIGNAL( dataChanged() ), mMapCanvas, SLOT( refresh() ) );
  }
}

void QgisApp::showExtents()
{
  if( !mToggleExtentsViewButton->isChecked() )
  {
    return;
  }

  // update the statusbar with the current extents.
  QgsRectangle myExtents = mMapCanvas->extent();
  mCoordsLabel->setText( tr( "Extents:" ) );
  mCoordsEdit->setText( myExtents.toString( true ) );
  //ensure the label is big enough
  if( mCoordsEdit->width() > mCoordsEdit->minimumWidth() )
  {
    mCoordsEdit->setMinimumWidth( mCoordsEdit->width() );
  }
} // QgisApp::showExtents


void QgisApp::updateMouseCoordinatePrecision()
{
  // Work out what mouse display precision to use. This only needs to
  // be when the settings change or the zoom level changes. This
  // function needs to be called every time one of the above happens.

  // Get the display precision from the project settings
  bool automatic = QgsProject::instance()->readBoolEntry( "PositionPrecision", "/Automatic" );
  int dp = 0;

  if( automatic )
  {
    // Work out a suitable number of decimal places for the mouse
    // coordinates with the aim of always having enough decimal places
    // to show the difference in position between adjacent pixels.
    // Also avoid taking the log of 0.
    if( mapCanvas()->mapUnitsPerPixel() != 0.0 )
      dp = static_cast<int>( ceil( -1.0 * log10( mapCanvas()->mapUnitsPerPixel() ) ) );
  }
  else
    dp = QgsProject::instance()->readNumEntry( "PositionPrecision", "/DecimalPlaces" );

  // Keep dp sensible
  if( dp < 0 ) dp = 0;

  mMousePrecisionDecimalPlaces = dp;
}

void QgisApp::showStatusMessage( QString theMessage )
{
  statusBar()->showMessage( theMessage );
}

void QgisApp::showMapTip()

{
  /* Show the maptip using tooltip */
  // Stop the timer while we look for a maptip
  mpMapTipsTimer->stop();

  // Only show tooltip if the mouse is over the canvas
  if( mMapCanvas->underMouse() )
  {
    QPoint myPointerPos = mMapCanvas->mouseLastXY();

    //  Make sure there is an active layer before proceeding

    QgsMapLayer* mypLayer = mMapCanvas->currentLayer();
    if( mypLayer )
    {
      //QgsDebugMsg("Current layer for maptip display is: " + mypLayer->source());
      // only process vector layers
      if( mypLayer->type() == QgsMapLayer::VectorLayer )
      {
        // Show the maptip if the maptips button is depressed
        if( mMapTipsVisible )
        {
          mpMaptip->showMapTip( mypLayer, mLastMapPosition, myPointerPos, mMapCanvas );
        }
      }
    }
    else
    {
      showStatusMessage( tr( "Maptips require an active layer" ) );
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
  if( mMapCanvas && mMapCanvas->isDrawing() )
  {
    return;
  }

  /* Display the property sheet for the Project */
  // set wait cursor since construction of the project properties
  // dialog results in the construction of the spatial reference
  // system QMap
  QApplication::setOverrideCursor( Qt::WaitCursor );
  QgsProjectProperties *pp = new QgsProjectProperties( mMapCanvas, this );
  // if called from the status bar, show the projection tab
  if( mShowProjectionTab )
  {
    pp->showProjectionsTab();
    mShowProjectionTab = false;
  }
  qApp->processEvents();
  // Be told if the mouse display precision may have changed by the user
  // changing things in the project properties dialog box
  connect( pp, SIGNAL( displayPrecisionChanged() ), this,
           SLOT( updateMouseCoordinatePrecision() ) );
  QApplication::restoreOverrideCursor();

  //pass any refresh signals off to canvases
  // Line below was commented out by wonder three years ago (r4949).
  // It is needed to refresh scale bar after changing display units.
  connect( pp, SIGNAL( refresh() ), mMapCanvas, SLOT( refresh() ) );

  QgsMapRenderer* myRender = mMapCanvas->mapRenderer();
  bool wasProjected = myRender->hasCrsTransformEnabled();
  long oldCRSID = myRender->destinationSrs().srsid();

  // Display the modal dialog box.
  pp->exec();

  long newCRSID = myRender->destinationSrs().srsid();
  bool isProjected = myRender->hasCrsTransformEnabled();

  // projections have been turned on/off or dest CRS has changed while projections are on
  if( wasProjected != isProjected || ( isProjected && oldCRSID != newCRSID ) )
  {
    // TODO: would be better to try to reproject current extent to the new one
    mMapCanvas->updateFullExtent();
  }

  int  myRedInt = QgsProject::instance()->readNumEntry( "Gui", "/CanvasColorRedPart", 255 );
  int  myGreenInt = QgsProject::instance()->readNumEntry( "Gui", "/CanvasColorGreenPart", 255 );
  int  myBlueInt = QgsProject::instance()->readNumEntry( "Gui", "/CanvasColorBluePart", 255 );
  QColor myColor = QColor( myRedInt, myGreenInt, myBlueInt );
  mMapCanvas->setCanvasColor( myColor ); // this is fill color before rendering onto canvas

  // Set the window title.
  setTitleBarText_( *this );

  // delete the property sheet object
  delete pp;
} // QgisApp::projectProperties


QgsClipboard * QgisApp::clipboard()
{
  return mInternalClipboard;
}

void QgisApp::selectionChanged( QgsMapLayer *layer )
{
  QgsVectorLayer *vlayer = qobject_cast<QgsVectorLayer *>( layer );
  if( vlayer )
  {
    showStatusMessage( tr( "%n feature(s) selected on layer %1.", "number of selected features", vlayer->selectedFeatureCount() ).arg( vlayer->name() ) );
  }
  activateDeactivateLayerRelatedActions( layer );
}

void QgisApp::legendLayerSelectionChanged( void )
{
  mActionRemoveLayer->setEnabled( mMapLegend && mMapLegend->selectedLayers().size() > 0 );
}

void QgisApp::activateDeactivateLayerRelatedActions( QgsMapLayer* layer )
{
  if( !layer )
  {
    mActionSelect->setEnabled( false );
    mActionSelectRectangle->setEnabled( false );
    mActionSelectPolygon->setEnabled( false );
    mActionSelectFreehand->setEnabled( false );
    mActionSelectRadius->setEnabled( false );
    mActionIdentify->setEnabled( QSettings().value( "/Map/identifyMode", 0 ).toInt() != 0 );
    mActionZoomActualSize->setEnabled( false );
    mActionOpenTable->setEnabled( false );
    mActionToggleEditing->setEnabled( false );
    mActionSaveEdits->setEnabled( false );
    mActionLayerSaveAs->setEnabled( false );
    mActionLayerSelectionSaveAs->setEnabled( false );
    mActionLayerProperties->setEnabled( false );
    mActionLayerSubsetString->setEnabled( false );
    mActionAddToOverview->setEnabled( false );

    mActionCapturePoint->setEnabled( false );
    mActionCaptureLine->setEnabled( false );
    mActionCapturePolygon->setEnabled( false );
    mActionMoveFeature->setEnabled( false );
    mActionNodeTool->setEnabled( false );
    mActionDeleteSelected->setEnabled( false );
    mActionCutFeatures->setEnabled( false );
    mActionCopyFeatures->setEnabled( false );
    mActionPasteFeatures->setEnabled( false );

    mActionUndo->setEnabled( false );
    mActionRedo->setEnabled( false );
    mActionSimplifyFeature->setEnabled( false );
    mActionAddRing->setEnabled( false );
    mActionAddIsland->setEnabled( false );
    mActionDeleteRing->setEnabled( false );
    mActionDeletePart->setEnabled( false );
    mActionReshapeFeatures->setEnabled( false );
    mActionSplitFeatures->setEnabled( false );
    mActionMergeFeatures->setEnabled( false );
    mActionRotatePointSymbols->setEnabled( false );

    mActionCapturePoint->setVisible( false );
    mActionCaptureLine->setVisible( false );
    mActionCapturePolygon->setVisible( false );

    return;
  }

  mActionLayerProperties->setEnabled( true );
  mActionAddToOverview->setEnabled( true );

  /***********Vector layers****************/
  if( layer->type() == QgsMapLayer::VectorLayer )
  {
    QgsVectorLayer* vlayer = qobject_cast<QgsVectorLayer *>( layer );
    QgsVectorDataProvider* dprovider = vlayer->dataProvider();
    bool layerHasSelection = vlayer->selectedFeatureCount() != 0;

    mActionSelect->setEnabled( true );
    mActionSelectRectangle->setEnabled( true );
    mActionSelectPolygon->setEnabled( true );
    mActionSelectFreehand->setEnabled( true );
    mActionSelectRadius->setEnabled( true );
    mActionIdentify->setEnabled( true );
    mActionZoomActualSize->setEnabled( false );
    mActionOpenTable->setEnabled( true );
    mActionLayerSaveAs->setEnabled( true );
    mActionLayerSelectionSaveAs->setEnabled( true );
    mActionCopyFeatures->setEnabled( layerHasSelection );

    if( !vlayer->isEditable() && mMapCanvas->mapTool() && mMapCanvas->mapTool()->isEditTool() )
    {
      mMapCanvas->setMapTool( mNonEditMapTool );
    }

    if( dprovider )
    {
      mActionLayerSubsetString->setEnabled( dprovider->supportsSubsetString() && !vlayer->isEditable() );

      //start editing/stop editing
      if( dprovider->capabilities() & QgsVectorDataProvider::EditingCapabilities )
      {
        mActionToggleEditing->setEnabled( !vlayer->isReadOnly() );
        mActionToggleEditing->setChecked( vlayer->isEditable() );
        mActionSaveEdits->setEnabled( vlayer->isEditable() );
      }
      else
      {
        mActionToggleEditing->setEnabled( false );
        mActionSaveEdits->setEnabled( false );
      }

      if( dprovider->capabilities() & QgsVectorDataProvider::AddFeatures )
      {
        mActionPasteFeatures->setEnabled( vlayer->isEditable() && !clipboard()->empty() );
      }
      else
      {
        mActionPasteFeatures->setEnabled( false );
      }

      //does provider allow deleting of features?
      if( vlayer->isEditable() && dprovider->capabilities() & QgsVectorDataProvider::DeleteFeatures )
      {
        mActionDeleteSelected->setEnabled( layerHasSelection );
        mActionCutFeatures->setEnabled( layerHasSelection );
      }
      else
      {
        mActionDeleteSelected->setEnabled( false );
        mActionCutFeatures->setEnabled( false );
      }

      //merge tool needs editable layer and provider with the capability of adding and deleting features
      if( vlayer->isEditable() &&
          ( dprovider->capabilities() & QgsVectorDataProvider::DeleteFeatures ) &&
          ( dprovider->capabilities() & QgsVectorDataProvider::ChangeAttributeValues ) &&
          ( dprovider->capabilities() & QgsVectorDataProvider::AddFeatures ) )
      {
        mActionMergeFeatures->setEnabled( layerHasSelection );
      }
      else
      {
        mActionMergeFeatures->setEnabled( false );
      }

      // moving enabled if geometry changes are supported
      if( vlayer->isEditable() && dprovider->capabilities() & QgsVectorDataProvider::ChangeGeometries )
      {
        mActionMoveFeature->setEnabled( true );
        mActionNodeTool->setEnabled( true );
      }
      else
      {
        mActionMoveFeature->setEnabled( false );
        mActionNodeTool->setEnabled( false );
      }

      if( vlayer->geometryType() == QGis::Point )
      {
        if( vlayer->isEditable() && dprovider->capabilities() & QgsVectorDataProvider::AddFeatures )
        {
          mActionCapturePoint->setEnabled( true );
          mActionCapturePoint->setVisible( true );
          mActionDeletePart->setEnabled( true );
          mActionAddIsland->setEnabled( true );
        }
        else
        {
          mActionCapturePoint->setEnabled( false );
          mActionCapturePoint->setVisible( false );
          mActionDeletePart->setEnabled( false );
          mActionAddIsland->setEnabled( false );
        }
        mActionCaptureLine->setEnabled( false );
        mActionCapturePolygon->setEnabled( false );
        mActionCaptureLine->setVisible( false );
        mActionCapturePolygon->setVisible( false );
#if 0 //these three tools to be deprecated - use node tool rather
        mActionAddVertex->setEnabled( false );
        mActionDeleteVertex->setEnabled( false );
        mActionMoveVertex->setEnabled( false );
#endif
        mActionAddRing->setEnabled( false );
#if 0
        mActionAddIsland->setEnabled( false );
#endif
        mActionAddIsland->setEnabled( false );
        mActionReshapeFeatures->setEnabled( false );
        mActionSplitFeatures->setEnabled( false );
        mActionSimplifyFeature->setEnabled( false );
        mActionDeleteRing->setEnabled( false );
        mActionRotatePointSymbols->setEnabled( false );

#if 0
        if( vlayer->isEditable() && dprovider->capabilities() & QgsVectorDataProvider::ChangeGeometries )
        {
          mActionMoveVertex->setEnabled( true );
        }
#endif

        if( vlayer->isEditable() && dprovider->capabilities() & QgsVectorDataProvider::ChangeAttributeValues )
        {
          if( QgsMapToolRotatePointSymbols::layerIsRotatable( vlayer ) )
          {
            mActionRotatePointSymbols->setEnabled( true );
          }
        }
        return;
      }
      else if( vlayer->geometryType() == QGis::Line )
      {
        if( vlayer->isEditable() && dprovider->capabilities() & QgsVectorDataProvider::AddFeatures )
        {
          mActionCaptureLine->setEnabled( true );
          mActionCaptureLine->setVisible( true );
          mActionReshapeFeatures->setEnabled( true );
          mActionSplitFeatures->setEnabled( true );
          mActionSimplifyFeature->setEnabled( true );
          mActionDeletePart->setEnabled( true );

        }
        else
        {
          mActionCaptureLine->setEnabled( false );
          mActionCaptureLine->setVisible( false );
          mActionReshapeFeatures->setEnabled( false );
          mActionSplitFeatures->setEnabled( false );
          mActionSimplifyFeature->setEnabled( false );
          mActionDeletePart->setEnabled( false );
        }
        mActionCapturePoint->setEnabled( false );
        mActionCapturePolygon->setEnabled( false );
        mActionCapturePoint->setVisible( false );
        mActionCapturePolygon->setVisible( false );
        mActionAddRing->setEnabled( false );
        mActionAddIsland->setEnabled( false );
        mActionDeleteRing->setEnabled( false );
      }
      else if( vlayer->geometryType() == QGis::Polygon )
      {
        if( vlayer->isEditable() && dprovider->capabilities() & QgsVectorDataProvider::AddFeatures )
        {
          mActionCapturePolygon->setEnabled( true );
          mActionCapturePolygon->setVisible( true );
          mActionAddRing->setEnabled( true );
          mActionAddIsland->setEnabled( true );
          mActionReshapeFeatures->setEnabled( true );
          mActionSplitFeatures->setEnabled( true );
          mActionSimplifyFeature->setEnabled( true );
          mActionDeleteRing->setEnabled( true );
          mActionDeletePart->setEnabled( true );
        }
        else
        {
          mActionCapturePolygon->setEnabled( false );
          mActionCapturePolygon->setVisible( false );
          mActionAddRing->setEnabled( false );
          mActionAddIsland->setEnabled( false );
          mActionReshapeFeatures->setEnabled( false );
          mActionSplitFeatures->setEnabled( false );
          mActionSimplifyFeature->setEnabled( false );
          mActionDeleteRing->setEnabled( false );
          mActionDeletePart->setEnabled( false );
        }
        mActionCapturePoint->setEnabled( false );
        mActionCaptureLine->setEnabled( false );
        mActionCapturePoint->setVisible( false );
        mActionCaptureLine->setVisible( false );
      }

      //are add/delete/move vertex supported?
      if( vlayer->isEditable() && dprovider->capabilities() & QgsVectorDataProvider::ChangeGeometries )
      {
#if 0 // these three tools to be deprecated - use node tool rather
        mActionAddVertex->setEnabled( true );
        mActionMoveVertex->setEnabled( true );
        mActionDeleteVertex->setEnabled( true );
#endif
        if( vlayer->geometryType() == QGis::Polygon )
        {
          mActionAddRing->setEnabled( true );
          //some polygon layers contain also multipolygon features.
          //Therefore, the test for multipolygon is done in QgsGeometry
          mActionAddIsland->setEnabled( true );
        }
      }
      else
      {
#if 0
        mActionAddVertex->setEnabled( false );
        mActionMoveVertex->setEnabled( false );
        mActionDeleteVertex->setEnabled( false );
#endif
      }
      return;
    }

    mActionLayerSubsetString->setEnabled( false );
  }
  /*************Raster layers*************/
  else if( layer->type() == QgsMapLayer::RasterLayer )
  {
    mActionLayerSubsetString->setEnabled( false );
    mActionSelect->setEnabled( false );
    mActionSelectRectangle->setEnabled( false );
    mActionSelectPolygon->setEnabled( false );
    mActionSelectFreehand->setEnabled( false );
    mActionSelectRadius->setEnabled( false );
    mActionZoomActualSize->setEnabled( true );
    mActionOpenTable->setEnabled( false );
    mActionToggleEditing->setEnabled( false );
    mActionSaveEdits->setEnabled( false );
    mActionLayerSaveAs->setEnabled( false );
    mActionLayerSelectionSaveAs->setEnabled( false );
    mActionCapturePoint->setEnabled( false );
    mActionCaptureLine->setEnabled( false );
    mActionCapturePolygon->setEnabled( false );
    mActionDeleteSelected->setEnabled( false );
    mActionAddRing->setEnabled( false );
    mActionAddIsland->setEnabled( false );
#if 0 //these three tools to be deprecated - use node tool rather
    mActionAddVertex->setEnabled( false );
    mActionDeleteVertex->setEnabled( false );
    mActionMoveVertex->setEnabled( false );
#endif
    mActionNodeTool->setEnabled( false );
    mActionMoveFeature->setEnabled( false );
    mActionCopyFeatures->setEnabled( false );
    mActionCutFeatures->setEnabled( false );
    mActionPasteFeatures->setEnabled( false );
    mActionRotatePointSymbols->setEnabled( false );
    mActionDeletePart->setEnabled( false );
    mActionDeleteRing->setEnabled( false );
    mActionSimplifyFeature->setEnabled( false );
    mActionReshapeFeatures->setEnabled( false );
    mActionSplitFeatures->setEnabled( false );

    //NOTE: This check does not really add any protection, as it is called on load not on layer select/activate
    //If you load a layer with a provider and idenitfy ability then load another without, the tool would be disabled for both

    //Enable the Identify tool ( GDAL datasets draw without a provider )
    //but turn off if data provider exists and has no Identify capabilities
    mActionIdentify->setEnabled( true );

    QSettings settings;
    int identifyMode = settings.value( "/Map/identifyMode", 0 ).toInt();
    if( identifyMode == 0 )
    {
      const QgsRasterLayer *rlayer = qobject_cast<const QgsRasterLayer *>( layer );
      const QgsRasterDataProvider* dprovider = rlayer->dataProvider();
      if( dprovider )
      {
        // does provider allow the identify map tool?
        if( dprovider->capabilities() & QgsRasterDataProvider::Identify )
        {
          mActionIdentify->setEnabled( true );
        }
        else
        {
          mActionIdentify->setEnabled( false );
        }
      }
    }
  }
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
  if( mMapCanvas && mMapCanvas->isDrawing() )
  {
    return;
  }

  QString fileFilters;

  QStringList selectedFiles;
  QString e;//only for parameter correctness
  QString title = tr( "Open a GDAL Supported Raster Data Source" );
  QgisGui::openFilesRememberingFilter( "lastRasterFileFilter", mRasterFileFilter, selectedFiles, e,
                                       title );

  if( selectedFiles.isEmpty() )
  {
    // no files were selected, so just bail
    return;
  }

  addRasterLayers( selectedFiles );

}// QgisApp::addRasterLayer()

//
// This is the method that does the actual work of adding a raster layer - the others
// here simply create a raster layer object and delegate here. It is the responsibility
// of the calling method to manage things such as the frozen state of the mapcanvas and
// using waitcursors etc. - this method won't and SHOULDN'T do it
//
bool QgisApp::addRasterLayer( QgsRasterLayer *theRasterLayer )
{
  if( mMapCanvas && mMapCanvas->isDrawing() )
  {
    return false;
  }

  Q_CHECK_PTR( theRasterLayer );

  if( ! theRasterLayer )
  {
    // XXX insert meaningful whine to the user here; although be
    // XXX mindful that a null layer may mean exhausted memory resources
    return false;
  }

  if( !theRasterLayer->isValid() )
  {
    delete theRasterLayer;
    return false;
  }

  // register this layer with the central layers registry
  QgsMapLayerRegistry::instance()->addMapLayer( theRasterLayer );

  // connect up any request the raster may make to update the app progress
  connect( theRasterLayer, SIGNAL( drawingProgress( int, int ) ),
           this, SLOT( showProgress( int, int ) ) );

  // connect up any request the raster may make to update the statusbar message
  connect( theRasterLayer, SIGNAL( statusChanged( QString ) ),
           this, SLOT( showStatusMessage( QString ) ) );

  return true;
}


//create a raster layer object and delegate to addRasterLayer(QgsRasterLayer *)

QgsRasterLayer* QgisApp::addRasterLayer( QString const & rasterFile, QString const & baseName, bool guiWarning )
{
  if( mMapCanvas && mMapCanvas->isDrawing() )
  {
    return NULL;
  }

  // let the user know we're going to possibly be taking a while
  QApplication::setOverrideCursor( Qt::WaitCursor );

  mMapCanvas->freeze( true );

  // XXX ya know QgsRasterLayer can snip out the basename on its own;
  // XXX why do we have to pass it in for it?
  QgsRasterLayer *layer =
    new QgsRasterLayer( rasterFile, baseName ); // fi.completeBaseName());

  if( !addRasterLayer( layer ) )
  {
    mMapCanvas->freeze( false );
    QApplication::restoreOverrideCursor();

// Let render() do its own cursor management
//    QApplication::restoreOverrideCursor();

    if( guiWarning )
    {
      // don't show the gui warning (probably because we are loading from command line)
      QString msg( tr( "%1 is not a valid or recognized raster data source" ).arg( rasterFile ) );
      QMessageBox::critical( this, tr( "Invalid Data Source" ), msg );
    }
    return NULL;
  }
  else
  {
    statusBar()->showMessage( mMapCanvas->extent().toString( 2 ) );
    mMapCanvas->freeze( false );
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
QgsRasterLayer* QgisApp::addRasterLayer(
  QString const &rasterLayerPath,
  QString const &baseName,
  QString const &providerKey,
  QStringList const & layers,
  QStringList const & styles,
  QString const &format,
  QString const &crs )
{
  QgsDebugMsg( "about to get library for " + providerKey );

  if( mMapCanvas && mMapCanvas->isDrawing() )
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
               + " and layer list of " + layers.join( ", " )
               + " and style list of " + styles.join( ", " )
               + " and format of " + format
               + " and providerKey of " + providerKey
               + " and CRS of " + crs );

  // TODO: Remove the 0 when the raster layer becomes a full provider gateway.
  layer = new QgsRasterLayer( 0, rasterLayerPath, baseName, providerKey, layers, styles, format, crs );

  QgsDebugMsg( "Constructed new layer." );

  if( layer && layer->isValid() )
  {
    addRasterLayer( layer );

    statusBar()->showMessage( mMapCanvas->extent().toString( 2 ) );

  }
  else
  {
    QMessageBox::critical( this, tr( "Layer is not valid" ),
                           tr( "The layer is not a valid layer and can not be added to the map" ) );
  }

  // update UI
  qApp->processEvents();
  // draw the map
  mMapCanvas->freeze( false );
  mMapCanvas->refresh();
  return layer;

// Let render() do its own cursor management
//  QApplication::restoreOverrideCursor();

} // QgisApp::addRasterLayer



//create a raster layer object and delegate to addRasterLayer(QgsRasterLayer *)
bool QgisApp::addRasterLayers( QStringList const &theFileNameQStringList, bool guiWarning )
{
  if( mMapCanvas && mMapCanvas->isDrawing() )
  {
    return false;
  }

  if( theFileNameQStringList.empty() )
  {
    // no files selected so bail out, but
    // allow mMapCanvas to handle events
    // first
    mMapCanvas->freeze( false );
    return false;
  }

  mMapCanvas->freeze( true );

// Let render() do its own cursor management
//  QApplication::setOverrideCursor(Qt::WaitCursor);

  // this is messy since some files in the list may be rasters and others may
  // be ogr layers. We'll set returnValue to false if one or more layers fail
  // to load.
  bool returnValue = true;
  for( QStringList::ConstIterator myIterator = theFileNameQStringList.begin();
       myIterator != theFileNameQStringList.end();
       ++myIterator )
  {
    QString errMsg;

    if( QgsRasterLayer::isValidRasterFileName( *myIterator, errMsg ) )
    {
      QFileInfo myFileInfo( *myIterator );
      // get the directory the .adf file was in
      QString myDirNameQString = myFileInfo.path();
      QString myBaseNameQString = myFileInfo.completeBaseName();
      //only allow one copy of a ai grid file to be loaded at a
      //time to prevent the user selecting all adfs in 1 dir which
      //actually represent 1 coverage,

      // create the layer
      QgsRasterLayer *layer = new QgsRasterLayer( *myIterator, myBaseNameQString );
      QStringList sublayers = layer->subLayers();

      if( sublayers.size() > 0 )
      {
        askUserForGDALSublayers( layer );

        // The first layer loaded is not useful in that case. The user can select it in
        // the list if he wants to load it.
        delete layer;
      }
      else
      {
        addRasterLayer( layer );

        //only allow one copy of a ai grid file to be loaded at a
        //time to prevent the user selecting all adfs in 1 dir which
        //actually represent 1 coverate,

        if( myBaseNameQString.toLower().endsWith( ".adf" ) )
        {
          break;
        }
      }
    }
    else
    {
      // Issue message box warning unless we are loading from cmd line since
      // non-rasters are passed to this function first and then sucessfully
      // loaded afterwards (see main.cpp)

      if( guiWarning )
      {
        QString msg = tr( "%1 is not a supported raster data source" ).arg( *myIterator );

        if( errMsg.size() > 0 )
          msg += "\n" + errMsg;

        QMessageBox::critical( this, tr( "Unsupported Data Source" ), msg );
      }
      returnValue = false;
    }
  }

  statusBar()->showMessage( mMapCanvas->extent().toString( 2 ) );
  mMapCanvas->freeze( false );
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


void QgisApp::keyPressEvent( QKeyEvent * e )
{
  // The following statement causes a crash on WIN32 and should be
  // enclosed in an #ifdef QGISDEBUG if its really necessary. Its
  // commented out for now. [gsherman]
  // QgsDebugMsg(QString("%1 (keypress recevied)").arg(e->text()));
  emit keyPressed( e );

  //cancel rendering progress with esc key
  if( e->key() == Qt::Key_Escape )
  {
    stopRendering();
  }
  else
  {
    e->ignore();
  }
}

#ifdef Q_OS_WIN
// hope your wearing your peril sensitive sunglasses.
void QgisApp::contextMenuEvent( QContextMenuEvent *e )
{
  if( mSkipNextContextMenuEvent )
  {
    mSkipNextContextMenuEvent--;
    e->ignore();
    return;
  }

  QMainWindow::contextMenuEvent( e );
}

void QgisApp::skipNextContextMenuEvent()
{
  mSkipNextContextMenuEvent++;
}
#endif

// Debug hook - used to output diagnostic messages when evoked (usually from the menu)
/* Temporarily disabled...
   void QgisApp::debugHook()
   {
   QgsDebugMsg("Hello from debug hook");
// show the map canvas extent
QgsDebugMsg(mMapCanvas->extent());
}
*/
void QgisApp::customProjection()
{
  if( mMapCanvas && mMapCanvas->isDrawing() )
  {
    return;
  }

  // Create an instance of the Custom Projection Designer modeless dialog.
  // Autodelete the dialog when closing since a pointer is not retained.
  QgsCustomProjectionDialog * myDialog = new QgsCustomProjectionDialog( this );
  myDialog->setAttribute( Qt::WA_DeleteOnClose );
  myDialog->show();
}
void QgisApp::showBookmarks()
{
  // Create or show the single instance of the Bookmarks modeless dialog.
  // Closing a QWidget only hides it so it can be shown again later.
  static QgsBookmarks *bookmarks = NULL;
  if( bookmarks == NULL )
  {
    bookmarks = new QgsBookmarks( this, Qt::WindowMinMaxButtonsHint );
  }
  bookmarks->restorePosition();
  bookmarks->show();
  bookmarks->raise();
  bookmarks->setWindowState( bookmarks->windowState() & ~Qt::WindowMinimized );
  bookmarks->activateWindow();
}

void QgisApp::newBookmark()
{
  // Get the name for the bookmark. Everything else we fetch from
  // the mapcanvas

  bool ok;
  QString bookmarkName = QInputDialog::getText( this, tr( "New Bookmark" ),
                         tr( "Enter a name for the new bookmark:" ), QLineEdit::Normal,
                         QString::null, &ok );
  if( ok && !bookmarkName.isEmpty() )
  {
    if( createDB() )
    {
      // create the bookmark
      QgsBookmarkItem *bmi = new QgsBookmarkItem( bookmarkName,
          QgsProject::instance()->title(), mMapCanvas->extent(), -1,
          QgsApplication::qgisUserDbFilePath() );
      bmi->store();
      delete bmi;
      // emit a signal to indicate that the bookmark was added
      emit bookmarkAdded();
    }
    else
    {
      QMessageBox::warning( this, tr( "Error" ), tr( "Unable to create the bookmark. Your user database may be missing or corrupted" ) );
    }
  }
}

// Slot that gets called when the project file was saved with an older
// version of QGIS

void QgisApp::oldProjectVersionWarning( QString oldVersion )
{
  QSettings settings;

  if( settings.value( "/qgis/warnOldProjectVersion", QVariant( true ) ).toBool() )
  {
    QApplication::setOverrideCursor( Qt::ArrowCursor );
    QMessageBox::warning( NULL, tr( "Project file is older" ),
                          tr( "<p>This project file was saved by an older version of QGIS."
                              " When saving this project file, QGIS will update it to the latest version, "
                              "possibly rendering it useless for older versions of QGIS."
                              "<p>Even though QGIS developers try to maintain backwards "
                              "compatibility, some of the information from the old project "
                              "file might be lost."
                              " To improve the quality of QGIS, we appreciate "
                              "if you file a bug report at %3."
                              " Be sure to include the old project file, and state the version of "
                              "QGIS you used to discover the error."
                              "<p>To remove this warning when opening an older project file, "
                              "uncheck the box '%5' in the %4 menu."
                              "<p>Version of the project file: %1<br>Current version of QGIS: %2" )
                          .arg( oldVersion )
                          .arg( QGis::QGIS_VERSION )
                          .arg( "<a href=\"https://trac.osgeo.org/qgis\">http://trac.osgeo.org/qgis</a> " )
                          .arg( tr( "<tt>Settings:Options:General</tt>", "Menu path to setting options" ) )
                          .arg( tr( "Warn me when opening a project file saved with an older version of QGIS" ) )
                        );
    QApplication::restoreOverrideCursor();
  }
  return;
}

QIcon QgisApp::getThemeIcon( const QString theName )
{
  QString myPreferredPath = QgsApplication::activeThemePath() + QDir::separator() + theName;
  QString myDefaultPath = QgsApplication::defaultThemePath() + QDir::separator() + theName;
  if( QFile::exists( myPreferredPath ) )
  {
    return QIcon( myPreferredPath );
  }
  else if( QFile::exists( myDefaultPath ) )
  {
    //could still return an empty icon if it
    //doesnt exist in the default theme either!
    return QIcon( myDefaultPath );
  }
  else
  {
    return QIcon();
  }
}

QPixmap QgisApp::getThemePixmap( const QString theName )
{
  QString myPreferredPath = QgsApplication::activeThemePath()  + QDir::separator() + theName;
  QString myDefaultPath = QgsApplication::defaultThemePath()  + QDir::separator() + theName;
  if( QFile::exists( myPreferredPath ) )
  {
    return QPixmap( myPreferredPath );
  }
  else
  {
    //could still return an empty icon if it
    //doesnt exist in the default theme either!
    return QPixmap( myDefaultPath );
  }
}

void QgisApp::updateUndoActions()
{
  bool canUndo = false, canRedo = false;
  QgsMapLayer* layer = activeLayer();
  if( layer )
  {
    QgsVectorLayer* vlayer = qobject_cast<QgsVectorLayer *>( layer );
    if( vlayer && vlayer->isEditable() )
    {
      canUndo = vlayer->undoStack()->canUndo();
      canRedo = vlayer->undoStack()->canRedo();
    }
  }
  mActionUndo->setEnabled( canUndo );
  mActionRedo->setEnabled( canRedo );
}

void QgisApp::runPythonString( const QString &expr )
{
  if( mPythonUtils && mPythonUtils->isEnabled() )
  {
    mPythonUtils->runString( expr );
  }
}

// add project directory to python path
void QgisApp::projectChanged( const QDomDocument &doc )
{
  QgsProject *project = qobject_cast<QgsProject*>( sender() );
  if( !project )
    return;

  QFileInfo fi( project->fileName() );
  if( !fi.exists() )
    return;

  static QString prevProjectDir = QString::null;

  if( prevProjectDir == fi.canonicalPath() )
    return;

  QString expr;
  if( !prevProjectDir.isNull() )
    expr = QString( "sys.path.remove('%1'); " ).arg( prevProjectDir );

  prevProjectDir = fi.canonicalPath();

  expr += QString( "sys.path.append('%1')" ).arg( prevProjectDir );

  runPythonString( expr );
}

void QgisApp::writeProject( QDomDocument &doc )
{
  projectChanged( doc );
}

void QgisApp::readProject( const QDomDocument &doc )
{
  projectChanged( doc );
}

void QgisApp::showLayerProperties( QgsMapLayer *ml )
{
  /*
  TODO: Consider reusing the property dialogs again.
  Sometimes around mid 2005, the property dialogs were saved for later reuse;
  this resulted in a time savings when reopening the dialog. The code below
  cannot be used as is, however, simply by saving the dialog pointer here.
  Either the map layer needs to be passed as an argument to sync or else
  a separate copy of the dialog pointer needs to be stored with each layer.
  */

  if( !ml )
    return;

  if( ml->type() == QgsMapLayer::RasterLayer )
  {
    QgsRasterLayerProperties *rlp = NULL; // See note above about reusing this
    if( rlp )
    {
      rlp->sync();
    }
    else
    {
      rlp = new QgsRasterLayerProperties( ml, mMapCanvas );
      connect( rlp, SIGNAL( refreshLegend( QString, bool ) ), mMapLegend, SLOT( refreshLayerSymbology( QString, bool ) ) );
    }
    rlp->exec();
    delete rlp; // delete since dialog cannot be reused without updating code
  }
  else if( ml->type() == QgsMapLayer::VectorLayer )  // VECTOR
  {
    QgsVectorLayer* vlayer = qobject_cast<QgsVectorLayer *>( ml );

    QgsVectorLayerProperties *vlp = NULL; // See note above about reusing this
    if( vlp )
    {
      vlp->reset();
    }
    else
    {
      vlp = new QgsVectorLayerProperties( vlayer );
      connect( vlp, SIGNAL( refreshLegend( QString, bool ) ), mMapLegend, SLOT( refreshLayerSymbology( QString, bool ) ) );
    }
    vlp->exec();
    delete vlp; // delete since dialog cannot be reused without updating code
  }
  else if( ml->type() == QgsMapLayer::PluginLayer )
  {
    QgsPluginLayer* pl = qobject_cast<QgsPluginLayer *>( ml );
    if( !pl )
      return;

    QgsPluginLayerType* plt = QgsPluginLayerRegistry::instance()->pluginLayerType( pl->pluginLayerType() );
    if( !plt )
      return;

    if( !plt->showLayerProperties( pl ) )
    {
      QMessageBox::information( this, tr( "Warning" ), tr( "This layer doesn't have a properties dialog." ) );
    }
  }
}

void QgisApp::namSetup()
{
  QgsNetworkAccessManager *nam = QgsNetworkAccessManager::instance();

  namUpdate();

  connect( nam, SIGNAL( authenticationRequired( QNetworkReply *, QAuthenticator * ) ),
           this, SLOT( namAuthenticationRequired( QNetworkReply *, QAuthenticator * ) ) );

  connect( nam, SIGNAL( proxyAuthenticationRequired( const QNetworkProxy &, QAuthenticator * ) ),
           this, SLOT( namProxyAuthenticationRequired( const QNetworkProxy &, QAuthenticator * ) ) );

#ifndef QT_NO_OPENSSL
  connect( nam, SIGNAL( sslErrors( QNetworkReply *, const QList<QSslError> & ) ),
           this, SLOT( namSslErrors( QNetworkReply *, const QList<QSslError> & ) ) );
#endif
}

void QgisApp::namAuthenticationRequired( QNetworkReply *reply, QAuthenticator *auth )
{
  QString username = auth->user();
  QString password = auth->password();

  bool ok = QgsCredentials::instance()->get(
              QString( "%1 at %2" ).arg( auth->realm() ).arg( reply->url().host() ),
              username, password,
              tr( "Authentication required" ) );
  if( !ok )
    return;

  auth->setUser( username );
  auth->setPassword( password );
}

void QgisApp::namProxyAuthenticationRequired( const QNetworkProxy &proxy, QAuthenticator *auth )
{
  QString username = auth->user();
  QString password = auth->password();

  bool ok = QgsCredentials::instance()->get(
              QString( "proxy %1:%2 [%3]" ).arg( proxy.hostName() ).arg( proxy.port() ).arg( auth->realm() ),
              username, password,
              tr( "Proxy authentication required" ) );
  if( !ok )
    return;

  auth->setUser( username );
  auth->setPassword( password );
}

#ifndef QT_NO_OPENSSL
void QgisApp::namSslErrors( QNetworkReply *reply, const QList<QSslError> &errors )
{
  QString msg = tr( "SSL errors occured accessing URL %1:" ).arg( reply->request().url().toString() );
  bool otherError = false;

  foreach( QSslError error, errors )
  {
    if( error.error() != QSslError::SelfSignedCertificate &&
        error.error() != QSslError::HostNameMismatch )
      otherError = true;
    msg += "\n" + error.errorString();
  }

  msg += tr( "\n\nIgnore errors?" );

  if( !otherError ||
      QMessageBox::warning( this,
                            tr( "SSL errors occured" ),
                            msg,
                            QMessageBox::Ok | QMessageBox::Cancel ) == QMessageBox::Ok )
  {
    reply->ignoreSslErrors();
  }
}
#endif

void QgisApp::namUpdate()
{
  QNetworkProxy proxy;
  QStringList excludes;

  QSettings settings;

  //check if proxy is enabled
  bool proxyEnabled = settings.value( "proxy/proxyEnabled", false ).toBool();
  if( proxyEnabled )
  {
    excludes = settings.value( "proxy/proxyExcludedUrls", "" ).toString().split( "|", QString::SkipEmptyParts );

    //read type, host, port, user, passw from settings
    QString proxyHost = settings.value( "proxy/proxyHost", "" ).toString();
    int proxyPort = settings.value( "proxy/proxyPort", "" ).toString().toInt();
    QString proxyUser = settings.value( "proxy/proxyUser", "" ).toString();
    QString proxyPassword = settings.value( "proxy/proxyPassword", "" ).toString();

    QString proxyTypeString = settings.value( "proxy/proxyType", "" ).toString();
    QNetworkProxy::ProxyType proxyType = QNetworkProxy::NoProxy;
    if( proxyTypeString == "DefaultProxy" )
    {
      proxyType = QNetworkProxy::DefaultProxy;
    }
    else if( proxyTypeString == "Socks5Proxy" )
    {
      proxyType = QNetworkProxy::Socks5Proxy;
    }
    else if( proxyTypeString == "HttpProxy" )
    {
      proxyType = QNetworkProxy::HttpProxy;
    }
    else if( proxyTypeString == "HttpCachingProxy" )
    {
      proxyType = QNetworkProxy::HttpCachingProxy;
    }
    else if( proxyTypeString == "FtpCachingProxy" )
    {
      proxyType = QNetworkProxy::FtpCachingProxy;
    }
    QgsDebugMsg( QString( "setting proxy %1 %2:%3 %4/%5" )
                 .arg( proxyType )
                 .arg( proxyHost ).arg( proxyPort )
                 .arg( proxyUser ).arg( proxyPassword )
               );
    proxy = QNetworkProxy( proxyType, proxyHost, proxyPort, proxyUser, proxyPassword );
  }

#if QT_VERSION >= 0x40500
  QgsNetworkAccessManager *nam = QgsNetworkAccessManager::instance();

  nam->setFallbackProxyAndExcludes( proxy, excludes );

  QNetworkDiskCache *cache = qobject_cast<QNetworkDiskCache*>( nam->cache() );
  if( !cache )
    cache = new QNetworkDiskCache( this );

  QString cacheDirectory = settings.value( "cache/directory", QgsApplication::qgisSettingsDirPath() + "cache" ).toString();
  qint64 cacheSize = settings.value( "cache/size", 50 * 1024 * 1024 ).toULongLong();
  QgsDebugMsg( QString( "setCacheDirectory: %1" ).arg( cacheDirectory ) );
  QgsDebugMsg( QString( "setMaximumCacheSize: %1" ).arg( cacheSize ) );
  cache->setCacheDirectory( cacheDirectory );
  cache->setMaximumCacheSize( cacheSize );
  QgsDebugMsg( QString( "cacheDirectory: %1" ).arg( cache->cacheDirectory() ) );
  QgsDebugMsg( QString( "maximumCacheSize: %1" ).arg( cache->maximumCacheSize() ) );

  if( nam->cache() != cache )
    nam->setCache( cache );
#else
  QgsNetworkAccessManager::instance()->setProxy( proxy );
#endif
}

void QgisApp::completeInitialization()
{
  emit initializationCompleted();
}

void QgisApp::toolButtonActionTriggered( QAction *action )
{
  QToolButton *bt = qobject_cast<QToolButton *>( sender() );
  if( !bt )
    return;

  bt->setDefaultAction( action );
}
