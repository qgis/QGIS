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
#include <QThread>

#include <qgsnetworkaccessmanager.h>
#include <qgsapplication.h>
#include <qgscomposition.h>

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
#include "qgisappstylesheet.h"
#include "qgis.h"
#include "qgisplugin.h"
#include "qgsabout.h"
#include "qgsapplication.h"
#include "qgsattributeaction.h"
#include "qgsattributetabledialog.h"
#include "qgsbookmarks.h"
#include "qgsbrowserdockwidget.h"
#include "qgsclipboard.h"
#include "qgscomposer.h"
#include "qgscomposermanager.h"
#include "qgscomposerview.h"
#include "qgsconfigureshortcutsdialog.h"
#include "qgscoordinatetransform.h"
#include "qgscredentialdialog.h"
#include "qgscursors.h"
#include "qgscustomization.h"
#include "qgscustomprojectiondialog.h"
#include "qgsdatasourceuri.h"
#include "qgsdecorationcopyright.h"
#include "qgsdecorationnortharrow.h"
#include "qgsdecorationscalebar.h"
#include "qgsdecorationgrid.h"
#include "qgsencodingfiledialog.h"
#include "qgserror.h"
#include "qgserrordialog.h"
#include "qgsexception.h"
#include "qgsexpressionselectiondialog.h"
#include "qgsfeature.h"
#include "qgsformannotationitem.h"
#include "qgsfieldcalculator.h"
#include "qgshtmlannotationitem.h"
#include "qgsgenericprojectionselector.h"
#include "qgsgpsinformationwidget.h"
#include "qgslabelinggui.h"
#include "qgslegend.h"
#include "qgslayerorder.h"
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
#include "qgsmessagebar.h"
#include "qgsmimedatautils.h"
#include "qgsmessagelog.h"
#include "qgsmultibandcolorrenderer.h"
#include "qgsnewvectorlayerdialog.h"
#include "qgsoptions.h"
// #include "qgspastetransformations.h"
#include "qgspluginlayer.h"
#include "qgspluginlayerregistry.h"
#include "qgspluginmanager.h"
#include "qgspluginregistry.h"
#include "qgspoint.h"
#include "qgshandlebadlayers.h"
#include "qgsproject.h"
#include "qgsprojectlayergroupdialog.h"
#include "qgsprojectproperties.h"
#include "qgsproviderregistry.h"
#include "qgspythonrunner.h"
#include "qgsquerybuilder.h"
#include "qgsrastercalcdialog.h"
#include "qgsrasterfilewriter.h"
#include "qgsrasteriterator.h"
#include "qgsrasterlayer.h"
#include "qgsrasterlayerproperties.h"
#include "qgsrasternuller.h"
#include "qgsbrightnesscontrastfilter.h"
#include "qgsrasterrenderer.h"
#include "qgsrasterlayersaveasdialog.h"
#include "qgsrectangle.h"
#include "qgsscalecombobox.h"
#include "qgsshortcutsmanager.h"
#include "qgssinglebandgrayrenderer.h"
#include "qgssnappingdialog.h"
#include "qgssponsors.h"
#include "qgssvgannotationitem.h"
#include "qgstextannotationitem.h"
#include "qgstipgui.h"
#include "qgsundowidget.h"
#include "qgsvectordataprovider.h"
#include "qgsvectorfilewriter.h"
#include "qgsvectorlayer.h"
#include "qgsvectorlayerproperties.h"
#include "qgsmessagelogviewer.h"
#include "qgsdataitem.h"

#include "qgssublayersdialog.h"
#include "ogr/qgsopenvectorlayerdialog.h"
#include "ogr/qgsvectorlayersaveasdialog.h"

#include "qgsosmdownloaddialog.h"
#include "qgsosmimportdialog.h"
#include "qgsosmexportdialog.h"

//
// GDAL/OGR includes
//
#include <ogr_api.h>
#include <proj_api.h>

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
#include "qgsmaptooladdpart.h"
#include "qgsmaptooladdring.h"
#include "qgsmaptoolannotation.h"
#include "qgsmaptooldeletering.h"
#include "qgsmaptooldeletepart.h"
#include "qgsmaptoolfeatureaction.h"
#include "qgsmaptoolformannotation.h"
#include "qgsmaptoolhtmlannotation.h"
#include "qgsmaptoolidentifyaction.h"
#include "qgsmaptoolmeasureangle.h"
#include "qgsmaptoolmovefeature.h"
#include "qgsmaptoolrotatefeature.h"
#include "qgsmaptooloffsetcurve.h"
#include "qgsmaptoolpan.h"
#include "qgsmaptoolselect.h"
#include "qgsmaptoolselectrectangle.h"
#include "qgsmaptoolselectfreehand.h"
#include "qgsmaptoolselectpolygon.h"
#include "qgsmaptoolselectradius.h"
#include "qgsmaptoolsvgannotation.h"
#include "qgsmaptoolreshape.h"
#include "qgsmaptoolrotatepointsymbols.h"
#include "qgsmaptoolsplitfeatures.h"
#include "qgsmaptooltextannotation.h"
#include "qgsmaptoolvertexedit.h"
#include "qgsmaptoolzoom.h"
#include "qgsmaptoolsimplify.h"
#include "qgsmeasuretool.h"
#include "qgsmaptoolpinlabels.h"
#include "qgsmaptoolshowhidelabels.h"
#include "qgsmaptoolmovelabel.h"
#include "qgsmaptoolrotatelabel.h"
#include "qgsmaptoolchangelabelproperties.h"

#include "nodetool/qgsmaptoolnodetool.h"

//
// Conditional Includes
//
#ifdef HAVE_PGCONFIG
#include <pg_config.h>
#else
#define PG_VERSION "unknown"
#endif

#include <sqlite3.h>

extern "C"
{
#include <spatialite.h>
}
#include "qgsnewspatialitelayerdialog.h"

#include "qgspythonutils.h"

#ifndef WIN32
#include <dlfcn.h>
#else
#include <windows.h>
#endif

#ifdef HAVE_TOUCH
#include "qgsmaptooltouch.h"
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
  set set title text to the project file name
  else
  set the title text to project title
  */
static void setTitleBarText_( QWidget & qgisApp )
{
  QString caption = QgisApp::tr( "QGIS " );

  if ( QString( QGis::QGIS_VERSION ).endsWith( "Master" ) )
  {
    caption += QString( "%1" ).arg( QGis::QGIS_DEV_VERSION );
  }
  else
  {
    caption += QGis::QGIS_VERSION;
  }

  if ( QgsProject::instance()->title().isEmpty() )
  {
    if ( QgsProject::instance()->fileName().isEmpty() )
    {
      // no project title nor file name, so just leave caption with
      // application name and version
    }
    else
    {
      QFileInfo projectFileInfo( QgsProject::instance()->fileName() );
      caption += " - " + projectFileInfo.completeBaseName();
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
  if ( QThread::currentThread() == QApplication::instance()->thread() )
    return new QgsMessageViewer( QgisApp::instance() );
  else
    return new QgsMessageOutputConsole();
}

static void customSrsValidation_( QgsCoordinateReferenceSystem &srs )
{
  QgisApp::instance()->emitCustomSrsValidation( srs );
}

void QgisApp::emitCustomSrsValidation( QgsCoordinateReferenceSystem &srs )
{
  emit customSrsValidation( srs );
}

/**
 * This function contains forced validation of CRS used in QGIS.
 * There are 3 options depending on the settings:
 * - ask for CRS using projection selecter
 * - use project's CRS
 * - use predefined global CRS
 */
void QgisApp::validateSrs( QgsCoordinateReferenceSystem &srs )
{
  static QString authid = QString::null;
  QSettings mySettings;
  QString myDefaultProjectionOption = mySettings.value( "/Projections/defaultBehaviour", "prompt" ).toString();
  if ( myDefaultProjectionOption == "prompt" )
  {
    //@note this class is not a descendent of QWidget so we cant pass
    //it in the ctor of the layer projection selector

    QgsGenericProjectionSelector *mySelector = new QgsGenericProjectionSelector();
    mySelector->setMessage( srs.validationHint() ); //shows a generic message, if not specified
    if ( authid.isNull() )
      authid = QgisApp::instance()->mapCanvas()->mapRenderer()->destinationCrs().authid();

    QgsCoordinateReferenceSystem defaultCrs;
    if ( defaultCrs.createFromOgcWmsCrs( authid ) )
    {
      mySelector->setSelectedCrsId( defaultCrs.srsid() );
    }

    bool waiting = QApplication::overrideCursor() && QApplication::overrideCursor()->shape() == Qt::WaitCursor;
    if ( waiting )
      QApplication::setOverrideCursor( Qt::ArrowCursor );

    if ( mySelector->exec() )
    {
      QgsDebugMsg( "Layer srs set from dialog: " + QString::number( mySelector->selectedCrsId() ) );
      authid = mySelector->selectedAuthId();
      srs.createFromOgcWmsCrs( mySelector->selectedAuthId() );
    }

    if ( waiting )
      QApplication::restoreOverrideCursor();

    delete mySelector;
  }
  else if ( myDefaultProjectionOption == "useProject" )
  {
    // XXX TODO: Change project to store selected CS as 'projectCRS' not 'selectedWkt'
    authid = QgisApp::instance()->mapCanvas()->mapRenderer()->destinationCrs().authid();
    QgsDebugMsg( "Layer srs set from project: " + authid );
    QgisApp::instance()->statusBar()->showMessage( tr( "CRS undefined - defaulting to project CRS" ) );
    srs.createFromOgcWmsCrs( authid );
  }
  else ///Projections/defaultBehaviour==useGlobal
  {
    authid = mySettings.value( "/Projections/layerDefaultCrs", GEO_EPSG_CRS_AUTHID ).toString();
    srs.createFromOgcWmsCrs( authid );
    QgisApp::instance()->statusBar()->showMessage( tr( "CRS undefined - defaulting to default CRS: %1" ).arg( authid ) );
  }
}

static bool cmpByText_( QAction* a, QAction* b )
{
  return QString::localeAwareCompare( a->text(), b->text() ) < 0;
}


QgisApp *QgisApp::smInstance = 0;

// constructor starts here
QgisApp::QgisApp( QSplashScreen *splash, bool restorePlugins, QWidget * parent, Qt::WFlags fl )
    : QMainWindow( parent, fl )
    , mSplash( splash )
    , mShowProjectionTab( false )
    , mPythonUtils( NULL )
#ifdef Q_OS_WIN
    , mSkipNextContextMenuEvent( 0 )
#endif
    , mpGpsWidget( NULL )
{
  if ( smInstance )
  {
    QMessageBox::critical(
      this,
      tr( "Multiple Instances of QgisApp" ),
      tr( "Multiple instances of QGIS application object detected.\nPlease contact the developers.\n" ) );
    abort();
  }

  smInstance = this;

  namSetup();

  // load GUI: actions, menus, toolbars
  setupUi( this );

  //////////

  mSplash->showMessage( tr( "Checking database" ), Qt::AlignHCenter | Qt::AlignBottom );
  qApp->processEvents();
  // Do this early on before anyone else opens it and prevents us copying it
  QString dbError;
  if ( !QgsApplication::createDB( &dbError ) )
  {
    QMessageBox::critical( this, tr( "Private qgis.db" ), dbError );
  }

  mSplash->showMessage( tr( "Reading settings" ), Qt::AlignHCenter | Qt::AlignBottom );
  qApp->processEvents();

  mSplash->showMessage( tr( "Setting up the GUI" ), Qt::AlignHCenter | Qt::AlignBottom );
  qApp->processEvents();

  QSettings settings;

  // set up stylesheet builder and apply saved or default style options
  mStyleSheetBuilder = new QgisAppStyleSheet( this );
  connect( mStyleSheetBuilder, SIGNAL( appStyleSheetChanged( const QString& ) ),
           this, SLOT( setAppStyleSheet( const QString& ) ) );
  mStyleSheetBuilder->buildStyleSheet( mStyleSheetBuilder->defaultOptions() );

  QWidget *centralWidget = this->centralWidget();
  QGridLayout *centralLayout = new QGridLayout( centralWidget );
  centralWidget->setLayout( centralLayout );
  centralLayout->setContentsMargins( 0, 0, 0, 0 );

  // "theMapCanvas" used to find this canonical instance later
  mMapCanvas = new QgsMapCanvas( centralWidget, "theMapCanvas" );
  mMapCanvas->setWhatsThis( tr( "Map canvas. This is where raster and vector "
                                "layers are displayed when added to the map" ) );

  // set canvas color right away
  int myRed = settings.value( "/qgis/default_canvas_color_red", 255 ).toInt();
  int myGreen = settings.value( "/qgis/default_canvas_color_green", 255 ).toInt();
  int myBlue = settings.value( "/qgis/default_canvas_color_blue", 255 ).toInt();
  mMapCanvas->setCanvasColor( QColor( myRed, myGreen, myBlue ) );

  centralLayout->addWidget( mMapCanvas, 0, 0, 2, 1 );

  // a bar to warn the user with non-blocking messages
  mInfoBar = new QgsMessageBar( centralWidget );
  mInfoBar->setSizePolicy( QSizePolicy::Minimum, QSizePolicy::Fixed );
  centralLayout->addWidget( mInfoBar, 0, 0, 1, 1 );

  //set the focus to the map canvas
  mMapCanvas->setFocus();

  // "theMapLegend" used to find this canonical instance later
  mMapLegend = new QgsLegend( mMapCanvas, this, "theMapLegend" );

  mMapLayerOrder = new QgsLayerOrder( mMapLegend, this, "theMapLayerOrder" );

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
  createDecorations();
  readSettings();
  updateRecentProjectPaths();
  updateProjectFromTemplates();
  legendLayerSelectionChanged();
  mSaveRollbackInProgress = false;
  activateDeactivateLayerRelatedActions( NULL );

  // initialize the plugin manager
  mPluginManager = new QgsPluginManager( this, restorePlugins );

  addDockWidget( Qt::LeftDockWidgetArea, mUndoWidget );
  mUndoWidget->hide();

  mSnappingDialog = new QgsSnappingDialog( this, mMapCanvas );
  mSnappingDialog->setObjectName( "SnappingOption" );

  mBrowserWidget = new QgsBrowserDockWidget( tr( "Browser" ), this );
  mBrowserWidget->setObjectName( "Browser" );
  addDockWidget( Qt::LeftDockWidgetArea, mBrowserWidget );
  mBrowserWidget->hide();

  mBrowserWidget2 = new QgsBrowserDockWidget( tr( "Browser (2)" ), this );
  mBrowserWidget2->setObjectName( "Browser2" );
  addDockWidget( Qt::LeftDockWidgetArea, mBrowserWidget2 );
  mBrowserWidget2->hide();

  // create the GPS tool on starting QGIS - this is like the browser
  mpGpsWidget = new QgsGPSInformationWidget( mMapCanvas );
  //create the dock widget
  mpGpsDock = new QDockWidget( tr( "GPS Information" ), this );
  mpGpsDock->setObjectName( "GPSInformation" );
  mpGpsDock->setAllowedAreas( Qt::LeftDockWidgetArea | Qt::RightDockWidgetArea );
  addDockWidget( Qt::LeftDockWidgetArea, mpGpsDock );
  // add to the Panel submenu
  // now add our widget to the dock - ownership of the widget is passed to the dock
  mpGpsDock->setWidget( mpGpsWidget );
  mpGpsDock->hide();

  mLogViewer = new QgsMessageLogViewer( statusBar(), this );

  mLogDock = new QDockWidget( tr( "Log Messages" ), this );
  mLogDock->setObjectName( "MessageLog" );
  mLogDock->setAllowedAreas( Qt::BottomDockWidgetArea | Qt::TopDockWidgetArea );
  addDockWidget( Qt::BottomDockWidgetArea, mLogDock );
  mLogDock->setWidget( mLogViewer );
  mLogDock->hide();

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
  QString caption = tr( "QGIS - %1 ('%2')" ).arg( QGis::QGIS_VERSION ).arg( QGis::QGIS_RELEASE_NAME );
  setWindowTitle( caption );

  QgsMessageLog::logMessage( tr( "QGIS starting..." ), QString::null, QgsMessageLog::INFO );

  // set QGIS specific srs validation
  connect( this, SIGNAL( customSrsValidation( QgsCoordinateReferenceSystem& ) ),
           this, SLOT( validateSrs( QgsCoordinateReferenceSystem& ) ) );
  QgsCoordinateReferenceSystem::setCustomSrsValidation( customSrsValidation_ );

  // set graphical message output
  QgsMessageOutput::setMessageOutputCreator( messageOutputViewer_ );

  // set graphical credential requester
  new QgsCredentialDialog( this );

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
  if ( restorePlugins )
  {
    // Restoring of plugins can be disabled with --noplugins command line option
    // because some plugins may cause QGIS to crash during startup
    QgsPluginRegistry::instance()->restoreSessionPlugins( QgsApplication::pluginPath() );

    // Also restore plugins from user specified plugin directories - added for 1.7
    QString myPaths = settings.value( "plugins/searchPathsForPlugins", "" ).toString();
    if ( !myPaths.isEmpty() )
    {
      QStringList myPathList = myPaths.split( "|" );
      QgsPluginRegistry::instance()->restoreSessionPlugins( myPathList );
    }
  }

  if ( mPythonUtils && mPythonUtils->isEnabled() )
  {
    // initialize the plugin installer to start fetching repositories in background
    QgsPythonRunner::run( "import pyplugin_installer" );
    QgsPythonRunner::run( "pyplugin_installer.initPluginInstaller()" );
    // enable Python in the Plugin Manager and pass the PythonUtils to it
    mPluginManager -> setPythonUtils( mPythonUtils );
  }

  mSplash->showMessage( tr( "Initializing file filters" ), Qt::AlignHCenter | Qt::AlignBottom );
  qApp->processEvents();

  // now build vector and raster file filters
  mVectorFileFilter = QgsProviderRegistry::instance()->fileVectorFilters();
  mRasterFileFilter = QgsProviderRegistry::instance()->fileRasterFilters();

  // set handler for missing layers (will be owned by QgsProject)
  QgsProject::instance()->setBadLayerHandler( new QgsHandleBadLayersHandler() );

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

  // do main window customization - after window state has been restored, before the window is shown
  QgsCustomization::instance()->updateMainWindow( mToolbarMenu );

  mSplash->showMessage( tr( "QGIS Ready!" ), Qt::AlignHCenter | Qt::AlignBottom );

  QgsMessageLog::logMessage( QgsApplication::showSettings(), QString::null, QgsMessageLog::INFO );

  QgsMessageLog::logMessage( tr( "QGIS Ready!" ), QString::null, QgsMessageLog::INFO );

  mMapTipsVisible = false;
  mTrustedMacros = false;

  // setup drag drop
  setAcceptDrops( true );

  mFullScreenMode = false;
  mPrevScreenModeMaximized = false;
  show();
  qApp->processEvents();

  mMapCanvas->freeze( false );
  mMapCanvas->clearExtentHistory(); // reset zoomnext/zoomlast
  mLastComposerId = 0;
  mLBL = new QgsPalLabeling();
  mMapCanvas->mapRenderer()->setLabelingEngine( mLBL );

  // Show a nice tip of the day
  if ( settings.value( "/qgis/showTips", 1 ).toBool() )
  {
    mSplash->hide();
    QgsTipGui myTip;
    myTip.exec();
  }
  else
  {
    QgsDebugMsg( "Tips are disabled" );
  }

#ifdef HAVE_TOUCH
  //add reacting to long click in touch
  grabGesture( Qt::TapAndHoldGesture );
#else
  //remove mActionTouch button
  delete mActionTouch;
  mActionTouch = 0;
#endif

  // supposedly all actions have been added, now register them to the shortcut manager
  QgsShortcutsManager::instance()->registerAllChildrenActions( this );

  QgsProviderRegistry::instance()->registerGuis( this );

  // update windows
  qApp->processEvents();

  fileNewBlank(); // prepare empty project, also skips any default templates from loading

  // request notification of FileOpen events (double clicking a file icon in Mac OS X Finder)
  // should come after fileNewBlank to ensure project is properly set up to receive any data source files
  QgsApplication::setFileOpenEventReceiver( this );

} // QgisApp ctor



QgisApp::~QgisApp()
{
  delete mInternalClipboard;
  delete mQgisInterface;
  delete mStyleSheetBuilder;

  delete mMapTools.mZoomIn;
  delete mMapTools.mZoomOut;
  delete mMapTools.mPan;
#ifdef HAVE_TOUCH
  delete mMapTools.mTouch;
#endif
  delete mMapTools.mAddFeature;
  delete mMapTools.mAddPart;
  delete mMapTools.mAddRing;
  delete mMapTools.mAnnotation;
  delete mMapTools.mChangeLabelProperties;
  delete mMapTools.mDeletePart;
  delete mMapTools.mDeleteRing;
  delete mMapTools.mFeatureAction;
  delete mMapTools.mFormAnnotation;
  delete mMapTools.mHtmlAnnotation;
  delete mMapTools.mIdentify;
  delete mMapTools.mMeasureAngle;
  delete mMapTools.mMeasureArea;
  delete mMapTools.mMeasureDist;
  delete mMapTools.mMoveFeature;
  delete mMapTools.mMoveLabel;
  delete mMapTools.mNodeTool;
  delete mMapTools.mOffsetCurve;
  delete mMapTools.mPinLabels;
  delete mMapTools.mReshapeFeatures;
  delete mMapTools.mRotateFeature;
  delete mMapTools.mRotateLabel;
  delete mMapTools.mRotatePointSymbolsTool;
  delete mMapTools.mSelect;
  delete mMapTools.mSelectFreehand;
  delete mMapTools.mSelectPolygon;
  delete mMapTools.mSelectRadius;
  delete mMapTools.mSelectRectangle;
  delete mMapTools.mShowHideLabels;
  delete mMapTools.mSimplifyFeature;
  delete mMapTools.mSplitFeatures;
  delete mMapTools.mSvgAnnotation;
  delete mMapTools.mTextAnnotation;

  delete mpMaptip;

  delete mpGpsWidget;

  delete mOverviewMapCursor;

  deletePrintComposers();
  removeAnnotationItems();

  // cancel request for FileOpen events
  QgsApplication::setFileOpenEventReceiver( 0 );

  // delete map layer registry and provider registry
  QgsApplication::exitQgis();

  delete QgsProject::instance();

  if ( mPythonUtils )
    mPythonUtils->exitPython();
  delete mPythonUtils;
}

void QgisApp::dragEnterEvent( QDragEnterEvent *event )
{
  if ( event->mimeData()->hasUrls() || event->mimeData()->hasFormat( "application/x-vnd.qgis.qgis.uri" ) )
  {
    event->acceptProposedAction();
  }
}

void QgisApp::dropEvent( QDropEvent *event )
{
  mMapCanvas->freeze();
  // get the file list
  QList<QUrl>::iterator i;
  QList<QUrl>urls = event->mimeData()->urls();
  for ( i = urls.begin(); i != urls.end(); i++ )
  {
    QString fileName = i->toLocalFile();
    // seems that some drag and drop operations include an empty url
    // so we test for length to make sure we have something
    if ( !fileName.isEmpty() )
    {
      openFile( fileName );
    }
  }

  if ( QgsMimeDataUtils::isUriList( event->mimeData() ) )
  {
    QgsMimeDataUtils::UriList lst = QgsMimeDataUtils::decodeUriList( event->mimeData() );
    foreach ( const QgsMimeDataUtils::Uri& u, lst )
    {
      if ( u.layerType == "vector" )
      {
        addVectorLayer( u.uri, u.name, u.providerKey );
      }
      else if ( u.layerType == "raster" )
      {
        addRasterLayer( u.uri, u.name, u.providerKey );
      }
    }
  }
  mMapCanvas->freeze( false );
  mMapCanvas->refresh();
  event->acceptProposedAction();
}

bool QgisApp::event( QEvent * event )
{
  bool done = false;
  if ( event->type() == QEvent::FileOpen )
  {
    // handle FileOpen event (double clicking a file icon in Mac OS X Finder)
    QFileOpenEvent *foe = static_cast<QFileOpenEvent *>( event );
    openFile( foe->file() );
    done = true;
  }
#ifdef HAVE_TOUCH
  else if ( event->type() == QEvent::Gesture )
  {
    done = gestureEvent( static_cast<QGestureEvent*>( event ) );
  }
#endif
  else
  {
    // pass other events to base class
    done = QMainWindow::event( event );
  }
  return done;
}

QgisAppStyleSheet* QgisApp::styleSheetBuilder()
{
  Q_ASSERT( mStyleSheetBuilder );
  return mStyleSheetBuilder;
}

// restore any application settings stored in QSettings
void QgisApp::readSettings()
{
  QSettings settings;
  // get the users theme preference from the settings
  // 'gis' theme is new /themes/default directory (2013-04-15)
  setTheme( settings.value( "/Themes", "default" ).toString() );

  // Add the recently accessed project file paths to the Project menu
  mRecentProjectPaths = settings.value( "/UI/recentProjectsList" ).toStringList();

  // this is a new session! reset enable macros value to "ask"
  // whether set to "just for this session"
  if ( settings.value( "/qgis/enableMacros", 1 ).toInt() == 2 )
  {
    settings.setValue( "/qgis/enableMacros", 1 );
  }
}


//////////////////////////////////////////////////////////////////////
//            Set Up the gui toolbars, menus, statusbar etc
//////////////////////////////////////////////////////////////////////

void QgisApp::createActions()
{
  mActionPluginSeparator1 = NULL;  // plugin list separator will be created when the first plugin is loaded
  mActionPluginSeparator2 = NULL;  // python separator will be created only if python is found
  mActionRasterSeparator = NULL;   // raster plugins list separator will be created when the first plugin is loaded

  // Project Menu Items

  connect( mActionNewProject, SIGNAL( triggered() ), this, SLOT( fileNew() ) );
  connect( mActionNewBlankProject, SIGNAL( triggered() ), this, SLOT( fileNewBlank() ) );
  connect( mActionOpenProject, SIGNAL( triggered() ), this, SLOT( fileOpen() ) );
  connect( mActionSaveProject, SIGNAL( triggered() ), this, SLOT( fileSave() ) );
  connect( mActionSaveProjectAs, SIGNAL( triggered() ), this, SLOT( fileSaveAs() ) );
  connect( mActionSaveMapAsImage, SIGNAL( triggered() ), this, SLOT( saveMapAsImage() ) );
  connect( mActionNewPrintComposer, SIGNAL( triggered() ), this, SLOT( newPrintComposer() ) );
  connect( mActionShowComposerManager, SIGNAL( triggered() ), this, SLOT( showComposerManager() ) );
  connect( mActionExit, SIGNAL( triggered() ), this, SLOT( fileExit() ) );

  // Edit Menu Items

  connect( mActionUndo, SIGNAL( triggered() ), mUndoWidget, SLOT( undo() ) );
  connect( mActionRedo, SIGNAL( triggered() ), mUndoWidget, SLOT( redo() ) );
  connect( mActionCutFeatures, SIGNAL( triggered() ), this, SLOT( editCut() ) );
  connect( mActionCopyFeatures, SIGNAL( triggered() ), this, SLOT( editCopy() ) );
  connect( mActionPasteFeatures, SIGNAL( triggered() ), this, SLOT( editPaste() ) );
  connect( mActionCopyStyle, SIGNAL( triggered() ), this, SLOT( copyStyle() ) );
  connect( mActionPasteStyle, SIGNAL( triggered() ), this, SLOT( pasteStyle() ) );
  connect( mActionAddFeature, SIGNAL( triggered() ), this, SLOT( addFeature() ) );
  connect( mActionMoveFeature, SIGNAL( triggered() ), this, SLOT( moveFeature() ) );
  connect( mActionRotateFeature, SIGNAL( triggered() ), this, SLOT( rotateFeature() ) );

  connect( mActionReshapeFeatures, SIGNAL( triggered() ), this, SLOT( reshapeFeatures() ) );
  connect( mActionSplitFeatures, SIGNAL( triggered() ), this, SLOT( splitFeatures() ) );
  connect( mActionDeleteSelected, SIGNAL( triggered() ), this, SLOT( deleteSelected() ) );
  connect( mActionAddRing, SIGNAL( triggered() ), this, SLOT( addRing() ) );
  connect( mActionAddPart, SIGNAL( triggered() ), this, SLOT( addPart() ) );
  connect( mActionSimplifyFeature, SIGNAL( triggered() ), this, SLOT( simplifyFeature() ) );
  connect( mActionDeleteRing, SIGNAL( triggered() ), this, SLOT( deleteRing() ) );
  connect( mActionDeletePart, SIGNAL( triggered() ), this, SLOT( deletePart() ) );
  connect( mActionMergeFeatures, SIGNAL( triggered() ), this, SLOT( mergeSelectedFeatures() ) );
  connect( mActionMergeFeatureAttributes, SIGNAL( triggered() ), this, SLOT( mergeAttributesOfSelectedFeatures() ) );
  connect( mActionNodeTool, SIGNAL( triggered() ), this, SLOT( nodeTool() ) );
  connect( mActionRotatePointSymbols, SIGNAL( triggered() ), this, SLOT( rotatePointSymbols() ) );
  connect( mActionSnappingOptions, SIGNAL( triggered() ), this, SLOT( snappingOptions() ) );
  connect( mActionOffsetCurve, SIGNAL( triggered() ), this, SLOT( offsetCurve() ) );

  // View Menu Items

#ifdef HAVE_TOUCH
  connect( mActionTouch, SIGNAL( triggered() ), this, SLOT( touch() ) );
#endif
  connect( mActionPan, SIGNAL( triggered() ), this, SLOT( pan() ) );
  connect( mActionPanToSelected, SIGNAL( triggered() ), this, SLOT( panToSelected() ) );
  connect( mActionZoomIn, SIGNAL( triggered() ), this, SLOT( zoomIn() ) );
  connect( mActionZoomOut, SIGNAL( triggered() ), this, SLOT( zoomOut() ) );
  connect( mActionSelect, SIGNAL( triggered() ), this, SLOT( select() ) );
  connect( mActionSelectRectangle, SIGNAL( triggered() ), this, SLOT( selectByRectangle() ) );
  connect( mActionSelectPolygon, SIGNAL( triggered() ), this, SLOT( selectByPolygon() ) );
  connect( mActionSelectFreehand, SIGNAL( triggered() ), this, SLOT( selectByFreehand() ) );
  connect( mActionSelectRadius, SIGNAL( triggered() ), this, SLOT( selectByRadius() ) );
  connect( mActionDeselectAll, SIGNAL( triggered() ), this, SLOT( deselectAll() ) );
  connect( mActionSelectByExpression, SIGNAL( triggered() ), this, SLOT( selectByExpression() ) );
  connect( mActionIdentify, SIGNAL( triggered() ), this, SLOT( identify() ) );
  connect( mActionFeatureAction, SIGNAL( triggered() ), this, SLOT( doFeatureAction() ) );
  connect( mActionMeasure, SIGNAL( triggered() ), this, SLOT( measure() ) );
  connect( mActionMeasureArea, SIGNAL( triggered() ), this, SLOT( measureArea() ) );
  connect( mActionMeasureAngle, SIGNAL( triggered() ), this, SLOT( measureAngle() ) );
  connect( mActionZoomFullExtent, SIGNAL( triggered() ), this, SLOT( zoomFull() ) );
  connect( mActionZoomToLayer, SIGNAL( triggered() ), this, SLOT( zoomToLayerExtent() ) );
  connect( mActionZoomToSelected, SIGNAL( triggered() ), this, SLOT( zoomToSelected() ) );
  connect( mActionZoomLast, SIGNAL( triggered() ), this, SLOT( zoomToPrevious() ) );
  connect( mActionZoomNext, SIGNAL( triggered() ), this, SLOT( zoomToNext() ) );
  connect( mActionZoomActualSize, SIGNAL( triggered() ), this, SLOT( zoomActualSize() ) );
  connect( mActionMapTips, SIGNAL( triggered() ), this, SLOT( toggleMapTips() ) );
  connect( mActionNewBookmark, SIGNAL( triggered() ), this, SLOT( newBookmark() ) );
  connect( mActionShowBookmarks, SIGNAL( triggered() ), this, SLOT( showBookmarks() ) );
  connect( mActionDraw, SIGNAL( triggered() ), this, SLOT( refreshMapCanvas() ) );
  connect( mActionTextAnnotation, SIGNAL( triggered() ), this, SLOT( addTextAnnotation() ) );
  connect( mActionFormAnnotation, SIGNAL( triggered() ), this, SLOT( addFormAnnotation() ) );
  connect( mActionHtmlAnnotation, SIGNAL( triggered() ), this, SLOT( addHtmlAnnotation() ) );
  connect( mActionSvgAnnotation, SIGNAL( triggered() ), this, SLOT( addSvgAnnotation() ) );
  connect( mActionAnnotation, SIGNAL( triggered() ), this, SLOT( modifyAnnotation() ) );
  connect( mActionLabeling, SIGNAL( triggered() ), this, SLOT( labeling() ) );

  // Layer Menu Items

  connect( mActionNewVectorLayer, SIGNAL( triggered() ), this, SLOT( newVectorLayer() ) );
  connect( mActionNewSpatialiteLayer, SIGNAL( triggered() ), this, SLOT( newSpatialiteLayer() ) );
  connect( mActionShowRasterCalculator, SIGNAL( triggered() ), this, SLOT( showRasterCalculator() ) );
  connect( mActionEmbedLayers, SIGNAL( triggered() ) , this, SLOT( embedLayers() ) );
  connect( mActionAddOgrLayer, SIGNAL( triggered() ), this, SLOT( addVectorLayer() ) );
  connect( mActionAddRasterLayer, SIGNAL( triggered() ), this, SLOT( addRasterLayer() ) );
  connect( mActionAddPgLayer, SIGNAL( triggered() ), this, SLOT( addDatabaseLayer() ) );
  connect( mActionAddSpatiaLiteLayer, SIGNAL( triggered() ), this, SLOT( addSpatiaLiteLayer() ) );
  connect( mActionAddMssqlLayer, SIGNAL( triggered() ), this, SLOT( addMssqlLayer() ) );
  connect( mActionAddOracleLayer, SIGNAL( triggered() ), this, SLOT( addOracleLayer() ) );
  connect( mActionAddWmsLayer, SIGNAL( triggered() ), this, SLOT( addWmsLayer() ) );
  connect( mActionAddWcsLayer, SIGNAL( triggered() ), this, SLOT( addWcsLayer() ) );
  connect( mActionAddWfsLayer, SIGNAL( triggered() ), this, SLOT( addWfsLayer() ) );
  connect( mActionAddDelimitedText, SIGNAL( triggered() ), this, SLOT( addDelimitedTextLayer() ) );
  connect( mActionOpenTable, SIGNAL( triggered() ), this, SLOT( attributeTable() ) );
  connect( mActionOpenFieldCalc, SIGNAL( triggered() ), this, SLOT( fieldCalculator() ) );
  connect( mActionToggleEditing, SIGNAL( triggered() ), this, SLOT( toggleEditing() ) );
  connect( mActionSaveLayerEdits, SIGNAL( triggered() ), this, SLOT( saveActiveLayerEdits() ) );
  connect( mActionSaveEdits, SIGNAL( triggered() ), this, SLOT( saveEdits() ) );
  connect( mActionSaveAllEdits, SIGNAL( triggered() ), this, SLOT( saveAllEdits() ) );
  connect( mActionRollbackEdits, SIGNAL( triggered() ), this, SLOT( rollbackEdits() ) );
  connect( mActionRollbackAllEdits, SIGNAL( triggered() ), this, SLOT( rollbackAllEdits() ) );
  connect( mActionCancelEdits, SIGNAL( triggered() ), this, SLOT( cancelEdits() ) );
  connect( mActionCancelAllEdits, SIGNAL( triggered() ), this, SLOT( cancelAllEdits() ) );
  connect( mActionLayerSaveAs, SIGNAL( triggered() ), this, SLOT( saveAsFile() ) );
  connect( mActionLayerSelectionSaveAs, SIGNAL( triggered() ), this, SLOT( saveSelectionAsVectorFile() ) );
  connect( mActionRemoveLayer, SIGNAL( triggered() ), this, SLOT( removeLayer() ) );
  connect( mActionDuplicateLayer, SIGNAL( triggered() ), this, SLOT( duplicateLayers() ) );
  connect( mActionSetLayerCRS, SIGNAL( triggered() ), this, SLOT( setLayerCRS() ) );
  connect( mActionSetProjectCRSFromLayer, SIGNAL( triggered() ), this, SLOT( setProjectCRSFromLayer() ) );
  connect( mActionLayerProperties, SIGNAL( triggered() ), this, SLOT( layerProperties() ) );
  connect( mActionLayerSubsetString, SIGNAL( triggered() ), this, SLOT( layerSubsetString() ) );
  connect( mActionAddToOverview, SIGNAL( triggered() ), this, SLOT( isInOverview() ) );
  connect( mActionAddAllToOverview, SIGNAL( triggered() ), this, SLOT( addAllToOverview() ) );
  connect( mActionRemoveAllFromOverview, SIGNAL( triggered() ), this, SLOT( removeAllFromOverview() ) );
  connect( mActionShowAllLayers, SIGNAL( triggered() ), this, SLOT( showAllLayers() ) );
  connect( mActionHideAllLayers, SIGNAL( triggered() ), this, SLOT( hideAllLayers() ) );

  // Plugin Menu Items

  connect( mActionManagePlugins, SIGNAL( triggered() ), this, SLOT( showPluginManager() ) );
  connect( mActionShowPythonDialog, SIGNAL( triggered() ), this, SLOT( showPythonDialog() ) );

  // Settings Menu Items

  connect( mActionToggleFullScreen, SIGNAL( triggered() ), this, SLOT( toggleFullScreen() ) );
  connect( mActionProjectProperties, SIGNAL( triggered() ), this, SLOT( projectProperties() ) );
  connect( mActionOptions, SIGNAL( triggered() ), this, SLOT( options() ) );
  connect( mActionCustomProjection, SIGNAL( triggered() ), this, SLOT( customProjection() ) );
  connect( mActionConfigureShortcuts, SIGNAL( triggered() ), this, SLOT( configureShortcuts() ) );
  connect( mActionStyleManagerV2, SIGNAL( triggered() ), this, SLOT( showStyleManagerV2() ) );
  connect( mActionCustomization, SIGNAL( triggered() ), this, SLOT( customize() ) );

#ifdef Q_WS_MAC
  // Window Menu Items

  mActionWindowMinimize = new QAction( tr( "Minimize" ), this );
  mActionWindowMinimize->setShortcut( tr( "Ctrl+M", "Minimize Window" ) );
  mActionWindowMinimize->setStatusTip( tr( "Minimizes the active window to the dock" ) );
  connect( mActionWindowMinimize, SIGNAL( triggered() ), this, SLOT( showActiveWindowMinimized() ) );

  mActionWindowZoom = new QAction( tr( "Zoom" ), this );
  mActionWindowZoom->setStatusTip( tr( "Toggles between a predefined size and the window size set by the user" ) );
  connect( mActionWindowZoom, SIGNAL( triggered() ), this, SLOT( toggleActiveWindowMaximized() ) );

  mActionWindowAllToFront = new QAction( tr( "Bring All to Front" ), this );
  mActionWindowAllToFront->setStatusTip( tr( "Bring forward all open windows" ) );
  connect( mActionWindowAllToFront, SIGNAL( triggered() ), this, SLOT( bringAllToFront() ) );

  // list of open windows
  mWindowActions = new QActionGroup( this );
#endif

  // Vector edits menu
  QMenu* menuAllEdits = new QMenu( tr( "Current Edits" ), this );
  menuAllEdits->addAction( mActionSaveEdits );
  menuAllEdits->addAction( mActionRollbackEdits );
  menuAllEdits->addAction( mActionCancelEdits );
  menuAllEdits->addSeparator();
  menuAllEdits->addAction( mActionSaveAllEdits );
  menuAllEdits->addAction( mActionRollbackAllEdits );
  menuAllEdits->addAction( mActionCancelAllEdits );
  mActionAllEdits->setMenu( menuAllEdits );

  // Raster toolbar items
  connect( mActionLocalHistogramStretch, SIGNAL( triggered() ), this, SLOT( localHistogramStretch() ) );
  connect( mActionFullHistogramStretch, SIGNAL( triggered() ), this, SLOT( fullHistogramStretch() ) );
  connect( mActionLocalCumulativeCutStretch, SIGNAL( triggered() ), this, SLOT( localCumulativeCutStretch() ) );
  connect( mActionFullCumulativeCutStretch, SIGNAL( triggered() ), this, SLOT( fullCumulativeCutStretch() ) );
  connect( mActionIncreaseBrightness, SIGNAL( triggered() ), this, SLOT( increaseBrightness() ) );
  connect( mActionDecreaseBrightness, SIGNAL( triggered() ), this, SLOT( decreaseBrightness() ) );
  connect( mActionIncreaseContrast, SIGNAL( triggered() ), this, SLOT( increaseContrast() ) );
  connect( mActionDecreaseContrast, SIGNAL( triggered() ), this, SLOT( decreaseContrast() ) );

  // Vector Menu Items
  connect( mActionOSMDownload, SIGNAL( triggered() ), this, SLOT( osmDownloadDialog() ) );
  connect( mActionOSMImport, SIGNAL( triggered() ), this, SLOT( osmImportDialog() ) );
  connect( mActionOSMExport, SIGNAL( triggered() ), this, SLOT( osmExportDialog() ) );

  // Help Menu Items

#ifdef Q_WS_MAC
  mActionHelpContents->setShortcut( QString( "Ctrl+?" ) );
  mActionQgisHomePage->setShortcut( QString() );
#endif

  mActionHelpContents->setEnabled( QFileInfo( QgsApplication::pkgDataPath() + "/doc/index.html" ).exists() );

  connect( mActionHelpContents, SIGNAL( triggered() ), this, SLOT( helpContents() ) );
  connect( mActionHelpAPI, SIGNAL( triggered() ), this, SLOT( apiDocumentation() ) );
  connect( mActionNeedSupport, SIGNAL( triggered() ), this, SLOT( supportProviders() ) );
  connect( mActionQgisHomePage, SIGNAL( triggered() ), this, SLOT( helpQgisHomePage() ) );
  connect( mActionCheckQgisVersion, SIGNAL( triggered() ), this, SLOT( checkQgisVersion() ) );
  connect( mActionAbout, SIGNAL( triggered() ), this, SLOT( about() ) );
  connect( mActionSponsors, SIGNAL( triggered() ), this, SLOT( sponsors() ) );

  connect( mActionShowPinnedLabels, SIGNAL( toggled( bool ) ), this, SLOT( showPinnedLabels( bool ) ) );
  connect( mActionPinLabels, SIGNAL( triggered() ), this, SLOT( pinLabels() ) );
  connect( mActionShowHideLabels, SIGNAL( triggered() ), this, SLOT( showHideLabels() ) );
  connect( mActionMoveLabel, SIGNAL( triggered() ), this, SLOT( moveLabel() ) );
  connect( mActionRotateLabel, SIGNAL( triggered() ), this, SLOT( rotateLabel() ) );
  connect( mActionChangeLabelProperties, SIGNAL( triggered() ), this, SLOT( changeLabelProperties() ) );

#ifndef HAVE_POSTGRESQL
  delete mActionAddPgLayer;
  mActionAddPgLayer = 0;
#endif

#ifndef HAVE_MSSQL
  delete mActionAddMssqlLayer;
  mActionAddMssqlLayer = 0;
#endif

#ifndef HAVE_ORACLE
  delete mActionAddOracleLayer;
  mActionAddOracleLayer = 0;
#endif

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
  for ( ; itemIt != items.constEnd(); ++itemIt )
  {
    if ( ! *itemIt )
    {
      continue;
    }
    ( *itemIt )->writeXML( doc );
  }
}

void QgisApp::showPythonDialog()
{
  if ( !mPythonUtils || !mPythonUtils->isEnabled() )
    return;

  bool res = mPythonUtils->runString(
               "import console\n"
               "console.show_console()\n", tr( "Failed to open Python console:" ), false );

  if ( !res )
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
#ifdef HAVE_TOUCH
  mMapToolGroup->addAction( mActionTouch );
#endif
  mMapToolGroup->addAction( mActionPan );
  mMapToolGroup->addAction( mActionZoomIn );
  mMapToolGroup->addAction( mActionZoomOut );
  mMapToolGroup->addAction( mActionIdentify );
  mMapToolGroup->addAction( mActionFeatureAction );
  mMapToolGroup->addAction( mActionSelect );
  mMapToolGroup->addAction( mActionSelectRectangle );
  mMapToolGroup->addAction( mActionSelectPolygon );
  mMapToolGroup->addAction( mActionSelectFreehand );
  mMapToolGroup->addAction( mActionSelectRadius );
  mMapToolGroup->addAction( mActionDeselectAll );
  mMapToolGroup->addAction( mActionMeasure );
  mMapToolGroup->addAction( mActionMeasureArea );
  mMapToolGroup->addAction( mActionMeasureAngle );
  mMapToolGroup->addAction( mActionAddFeature );
  mMapToolGroup->addAction( mActionMoveFeature );
  mMapToolGroup->addAction( mActionRotateFeature );
#if defined(GEOS_VERSION_MAJOR) && defined(GEOS_VERSION_MINOR) && \
    ((GEOS_VERSION_MAJOR>3) || ((GEOS_VERSION_MAJOR==3) && (GEOS_VERSION_MINOR>=3)))
  mMapToolGroup->addAction( mActionOffsetCurve );
#endif
  mMapToolGroup->addAction( mActionReshapeFeatures );
  mMapToolGroup->addAction( mActionSplitFeatures );
  mMapToolGroup->addAction( mActionDeleteSelected );
  mMapToolGroup->addAction( mActionAddRing );
  mMapToolGroup->addAction( mActionAddPart );
  mMapToolGroup->addAction( mActionSimplifyFeature );
  mMapToolGroup->addAction( mActionDeleteRing );
  mMapToolGroup->addAction( mActionDeletePart );
  mMapToolGroup->addAction( mActionMergeFeatures );
  mMapToolGroup->addAction( mActionMergeFeatureAttributes );
  mMapToolGroup->addAction( mActionNodeTool );
  mMapToolGroup->addAction( mActionRotatePointSymbols );
  mMapToolGroup->addAction( mActionPinLabels );
  mMapToolGroup->addAction( mActionShowHideLabels );
  mMapToolGroup->addAction( mActionMoveLabel );
  mMapToolGroup->addAction( mActionRotateLabel );
  mMapToolGroup->addAction( mActionChangeLabelProperties );
}

void QgisApp::setAppStyleSheet( const QString& stylesheet )
{
  setStyleSheet( stylesheet );

  // cascade styles to any current project composers
  foreach ( QgsComposer *c, mPrintComposers )
  {
    c->setStyleSheet( stylesheet );
  }
}

int QgisApp::messageTimeout()
{
  QSettings settings;
  return settings.value( "/qgis/messageTimeout", 5 ).toInt();
}

void QgisApp::createMenus()
{
  /*
   * The User Interface Guidelines for each platform specify different locations
   * for the following items.
   *
   * Project Properties:
   * Gnome, Mac, Win - File/Project menu above print commands (Win doesn't specify)
   * Kde - Settings menu
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

  // Panel and Toolbar Submenus
  mPanelMenu = new QMenu( tr( "Panels" ), this );
  mPanelMenu->setObjectName( "mPanelMenu" );
  mToolbarMenu = new QMenu( tr( "Toolbars" ), this );
  mToolbarMenu->setObjectName( "mToolbarMenu" );

  // Get platform for menu layout customization (Gnome, Kde, Mac, Win)
  QDialogButtonBox::ButtonLayout layout =
    QDialogButtonBox::ButtonLayout( style()->styleHint( QStyle::SH_DialogButtonLayout, 0, this ) );

  // Project Menu

  // Connect once for the entire submenu.
  connect( mRecentProjectsMenu, SIGNAL( triggered( QAction * ) ),
           this, SLOT( openProject( QAction * ) ) );
  connect( mProjectFromTemplateMenu, SIGNAL( triggered( QAction * ) ),
           this, SLOT( fileNewFromTemplateAction( QAction * ) ) );

  if ( layout == QDialogButtonBox::GnomeLayout || layout == QDialogButtonBox::MacLayout || layout == QDialogButtonBox::WinLayout )
  {
    QAction* before = mActionNewPrintComposer;
    mSettingsMenu->removeAction( mActionProjectProperties );
    mProjectMenu->insertAction( before, mActionProjectProperties );
    mProjectMenu->insertSeparator( before );
  }

  // View Menu

  if ( layout != QDialogButtonBox::KdeLayout )
  {
    mViewMenu->addSeparator();
    mViewMenu->addMenu( mPanelMenu );
    mViewMenu->addMenu( mToolbarMenu );
    mViewMenu->addAction( mActionToggleFullScreen );
  }
  else
  {
    // on the top of the settings menu
    QAction* before = mActionProjectProperties;
    mSettingsMenu->insertMenu( before, mPanelMenu );
    mSettingsMenu->insertMenu( before, mToolbarMenu );
    mSettingsMenu->insertAction( before, mActionToggleFullScreen );
    mSettingsMenu->insertSeparator( before );
  }

#ifdef Q_WS_MAC

  // keep plugins from hijacking About and Preferences application menus
  // these duplicate actions will be moved to application menus by Qt
  mProjectMenu->addAction( mActionAbout );
  mProjectMenu->addAction( mActionOptions );

  // Window Menu

  mWindowMenu = new QMenu( tr( "Window" ), this );

  mWindowMenu->addAction( mActionWindowMinimize );
  mWindowMenu->addAction( mActionWindowZoom );
  mWindowMenu->addSeparator();

  mWindowMenu->addAction( mActionWindowAllToFront );
  mWindowMenu->addSeparator();

  // insert before Help menu, as per Mac OS convention
  menuBar()->insertMenu( mHelpMenu->menuAction(), mWindowMenu );
#endif

  // Database Menu
  // don't add it yet, wait for a plugin
  mDatabaseMenu = new QMenu( tr( "&Database" ), menuBar() );
  mDatabaseMenu->setObjectName( "mDatabaseMenu" );
  // Web Menu
  // don't add it yet, wait for a plugin
  mWebMenu = new QMenu( tr( "&Web" ), menuBar() );
  mWebMenu->setObjectName( "mWebMenu" );

  // Help menu
  // add What's this button to it
  QAction* before = mActionHelpAPI;
  mHelpMenu->insertAction( before, QWhatsThis::createAction( this ) );
}

void QgisApp::createToolBars()
{
  QSettings settings;
  int size = settings.value( "/IconSize", QGIS_ICON_SIZE ).toInt();
  setIconSize( QSize( size, size ) );
  // QSize myIconSize ( 32,32 ); //large icons
  // Note: we need to set each object name to ensure that
  // qmainwindow::saveState and qmainwindow::restoreState
  // work properly

  QList<QToolBar*> toolbarMenuToolBars;
  toolbarMenuToolBars << mFileToolBar
  << mLayerToolBar
  << mDigitizeToolBar
  << mAdvancedDigitizeToolBar
  << mMapNavToolBar
  << mAttributesToolBar
  << mPluginToolBar
  << mHelpToolBar
  << mRasterToolBar
  << mVectorToolBar
  << mDatabaseToolBar
  << mWebToolBar
  << mLabelToolBar;

  QList<QAction*> toolbarMenuActions;
  // Set action names so that they can be used in customization
  foreach ( QToolBar *toolBar, toolbarMenuToolBars )
  {
    toolBar->toggleViewAction()->setObjectName( "mActionToggle" + toolBar->objectName().mid( 1 ) );
    toolbarMenuActions << toolBar->toggleViewAction();
  }

  // sort actions in toolbar menu
  qSort( toolbarMenuActions.begin(), toolbarMenuActions.end(), cmpByText_ );

  mToolbarMenu->addActions( toolbarMenuActions );

  // select tool button

  QToolButton *bt = new QToolButton( mAttributesToolBar );
  bt->setPopupMode( QToolButton::MenuButtonPopup );
  QList<QAction*> selectActions;
  selectActions << mActionSelect << mActionSelectRectangle << mActionSelectPolygon
  << mActionSelectFreehand << mActionSelectRadius;
  bt->addActions( selectActions );

  QAction* defSelectAction = mActionSelect;
  switch ( settings.value( "/UI/selectTool", 0 ).toInt() )
  {
    case 0: defSelectAction = mActionSelect; break;
    case 1: defSelectAction = mActionSelectRectangle; break;
    case 2: defSelectAction = mActionSelectPolygon; break;
    case 3: defSelectAction = mActionSelectFreehand; break;
    case 4: defSelectAction = mActionSelectRadius; break;
  }
  bt->setDefaultAction( defSelectAction );
  QAction* selectAction = mAttributesToolBar->insertWidget( mActionDeselectAll, bt );
  selectAction->setObjectName( "ActionSelect" );
  connect( bt, SIGNAL( triggered( QAction * ) ), this, SLOT( toolButtonActionTriggered( QAction * ) ) );

  // feature action tool button

  bt = new QToolButton( mAttributesToolBar );
  bt->setPopupMode( QToolButton::MenuButtonPopup );
  bt->setDefaultAction( mActionFeatureAction );
  mFeatureActionMenu = new QMenu( bt );
  connect( mFeatureActionMenu, SIGNAL( triggered( QAction * ) ), this, SLOT( updateDefaultFeatureAction( QAction * ) ) );
  connect( mFeatureActionMenu, SIGNAL( aboutToShow() ), this, SLOT( refreshFeatureActions() ) );
  bt->setMenu( mFeatureActionMenu );
  QAction* featureActionAction = mAttributesToolBar->insertWidget( selectAction, bt );
  featureActionAction->setObjectName( "ActionFeatureAction" );

  // measure tool button

  bt = new QToolButton( mAttributesToolBar );
  bt->setPopupMode( QToolButton::MenuButtonPopup );
  bt->addAction( mActionMeasure );
  bt->addAction( mActionMeasureArea );
  bt->addAction( mActionMeasureAngle );

  QAction* defMeasureAction = mActionMeasure;
  switch ( settings.value( "/UI/measureTool", 0 ).toInt() )
  {
    case 0: defMeasureAction = mActionMeasure; break;
    case 1: defMeasureAction = mActionMeasureArea; break;
    case 2: defMeasureAction = mActionMeasureAngle; break;
  }
  bt->setDefaultAction( defMeasureAction );
  QAction* measureAction = mAttributesToolBar->insertWidget( mActionMapTips, bt );
  measureAction->setObjectName( "ActionMeasure" );
  connect( bt, SIGNAL( triggered( QAction * ) ), this, SLOT( toolButtonActionTriggered( QAction * ) ) );

  // annotation tool button

  bt = new QToolButton();
  bt->setPopupMode( QToolButton::MenuButtonPopup );
  bt->addAction( mActionTextAnnotation );
  bt->addAction( mActionFormAnnotation );
  bt->addAction( mActionHtmlAnnotation );
  bt->addAction( mActionSvgAnnotation );
  bt->addAction( mActionAnnotation );

  QAction* defAnnotationAction = mActionTextAnnotation;
  switch ( settings.value( "/UI/annotationTool", 0 ).toInt() )
  {
    case 0: defAnnotationAction = mActionTextAnnotation; break;
    case 1: defAnnotationAction = mActionFormAnnotation; break;
    case 2: defAnnotationAction = mActionHtmlAnnotation; break;
    case 3: defAnnotationAction = mActionSvgAnnotation; break;
    case 4: defAnnotationAction =  mActionAnnotation; break;

  }
  bt->setDefaultAction( defAnnotationAction );
  QAction* annotationAction = mAttributesToolBar->addWidget( bt );
  annotationAction->setObjectName( "ActionAnnotation" );
  connect( bt, SIGNAL( triggered( QAction * ) ), this, SLOT( toolButtonActionTriggered( QAction * ) ) );

  // vector layer edits tool buttons
  QToolButton* tbAllEdits = qobject_cast<QToolButton *>( mDigitizeToolBar->widgetForAction( mActionAllEdits ) );
  tbAllEdits->setPopupMode( QToolButton::InstantPopup );

  // Help Toolbar

  mHelpToolBar->addAction( QWhatsThis::createAction( this ) );

}

void QgisApp::createStatusBar()
{
  //
  // Add a panel to the status bar for the scale, coords and progress
  // And also rendering suppression checkbox
  //
  mProgressBar = new QProgressBar( statusBar() );
  mProgressBar->setObjectName( "mProgressBar" );
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
  mToggleExtentsViewButton->setObjectName( "mToggleExtentsViewButton" );
  mToggleExtentsViewButton->setMaximumWidth( 20 );
  mToggleExtentsViewButton->setMaximumHeight( 20 );
  mToggleExtentsViewButton->setIcon( QgsApplication::getThemeIcon( "tracking.png" ) );
  mToggleExtentsViewButton->setToolTip( tr( "Toggle extents and mouse position display" ) );
  mToggleExtentsViewButton->setCheckable( true );
  connect( mToggleExtentsViewButton, SIGNAL( toggled( bool ) ), this, SLOT( extentsViewToggled( bool ) ) );
  statusBar()->addPermanentWidget( mToggleExtentsViewButton, 0 );

  // add a label to show current scale
  mCoordsLabel = new QLabel( QString(), statusBar() );
  mCoordsLabel->setObjectName( "mCoordsLabel" );
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
  mCoordsEdit->setObjectName( "mCoordsEdit" );
  mCoordsEdit->setFont( myFont );
  mCoordsEdit->setMinimumWidth( 10 );
  mCoordsEdit->setMaximumWidth( 300 );
  mCoordsEdit->setMaximumHeight( 20 );
  mCoordsEdit->setContentsMargins( 0, 0, 0, 0 );
  mCoordsEdit->setAlignment( Qt::AlignCenter );
  mCoordsEdit->setAlignment( Qt::AlignCenter );
  QRegExp coordValidator( "[+-]?\\d+\\.?\\d*\\s*,\\s*[+-]?\\d+\\.?\\d*" );
  mCoordsEditValidator = new QRegExpValidator( coordValidator, mCoordsEdit );
  mCoordsEdit->setWhatsThis( tr( "Shows the map coordinates at the "
                                 "current cursor position. The display is continuously updated "
                                 "as the mouse is moved. It also allows editing to set the canvas "
                                 "center to a given position. The format is lat,lon or east,north" ) );
  mCoordsEdit->setToolTip( tr( "Current map coordinate (lat,lon or east,north)" ) );
  statusBar()->addPermanentWidget( mCoordsEdit, 0 );
  connect( mCoordsEdit, SIGNAL( editingFinished() ), this, SLOT( userCenter() ) );

  // add a label to show current scale
  mScaleLabel = new QLabel( QString(), statusBar() );
  mScaleLabel->setObjectName( "mScaleLable" );
  mScaleLabel->setFont( myFont );
  mScaleLabel->setMinimumWidth( 10 );
  mScaleLabel->setMaximumHeight( 20 );
  mScaleLabel->setMargin( 3 );
  mScaleLabel->setAlignment( Qt::AlignCenter );
  mScaleLabel->setFrameStyle( QFrame::NoFrame );
  mScaleLabel->setText( tr( "Scale " ) );
  mScaleLabel->setToolTip( tr( "Current map scale" ) );
  statusBar()->addPermanentWidget( mScaleLabel, 0 );

  mScaleEdit = new QgsScaleComboBox( statusBar() );
  mScaleEdit->setObjectName( "mScaleEdit" );
  mScaleEdit->setFont( myFont );
  // seems setFont() change font only for popup not for line edit,
  // so we need to set font for it separately
  mScaleEdit->lineEdit()->setFont( myFont );
  mScaleEdit->setMinimumWidth( 10 );
  mScaleEdit->setMaximumWidth( 100 );
  mScaleEdit->setMaximumHeight( 20 );
  mScaleEdit->setContentsMargins( 0, 0, 0, 0 );
  mScaleEdit->setWhatsThis( tr( "Displays the current map scale" ) );
  mScaleEdit->setToolTip( tr( "Current map scale (formatted as x:y)" ) );

  statusBar()->addPermanentWidget( mScaleEdit, 0 );
  connect( mScaleEdit, SIGNAL( scaleChanged() ), this, SLOT( userScale() ) );

  //stop rendering status bar widget
  mStopRenderButton = new QToolButton( statusBar() );
  mStopRenderButton->setObjectName( "mStopRenderButton" );
  mStopRenderButton->setMaximumWidth( 20 );
  mStopRenderButton->setMaximumHeight( 20 );
  mStopRenderButton->setIcon( QgsApplication::getThemeIcon( "mIconStopRendering.png" ) );
  mStopRenderButton->setToolTip( tr( "Stop map rendering" ) );
  statusBar()->addPermanentWidget( mStopRenderButton, 0 );
  // render suppression status bar widget
  mRenderSuppressionCBox = new QCheckBox( tr( "Render" ), statusBar() );
  mRenderSuppressionCBox->setObjectName( "mRenderSuppressionCBox" );
  mRenderSuppressionCBox->setChecked( true );
  mRenderSuppressionCBox->setFont( myFont );
  mRenderSuppressionCBox->setWhatsThis( tr( "When checked, the map layers "
                                        "are rendered in response to map navigation commands and other "
                                        "events. When not checked, no rendering is done. This allows you "
                                        "to add a large number of layers and symbolize them before rendering." ) );
  mRenderSuppressionCBox->setToolTip( tr( "Toggle map rendering" ) );
  statusBar()->addPermanentWidget( mRenderSuppressionCBox, 0 );
  // On the fly projection active CRS label
  mOnTheFlyProjectionStatusLabel = new QLabel( QString(), statusBar() );
  mOnTheFlyProjectionStatusLabel->setObjectName( "mOnTheFlyProjectionStatusLabel" );
  mOnTheFlyProjectionStatusLabel->setFont( myFont );
  mOnTheFlyProjectionStatusLabel->setMinimumWidth( 10 );
  mOnTheFlyProjectionStatusLabel->setMaximumHeight( 20 );
  mOnTheFlyProjectionStatusLabel->setMargin( 3 );
  mOnTheFlyProjectionStatusLabel->setAlignment( Qt::AlignCenter );
  mOnTheFlyProjectionStatusLabel->setFrameStyle( QFrame::NoFrame );
  QString myCrs = mMapCanvas->mapRenderer()->destinationCrs().authid();
  statusBar()->addPermanentWidget( mOnTheFlyProjectionStatusLabel, 0 );
  // On the fly projection status bar icon
  // Changed this to a tool button since a QPushButton is
  // sculpted on OS X and the icon is never displayed [gsherman]
  mOnTheFlyProjectionStatusButton = new QToolButton( statusBar() );
  mOnTheFlyProjectionStatusButton->setObjectName( "mOntheFlyProjectionStatusButton" );
  mOnTheFlyProjectionStatusButton->setMaximumWidth( 20 );
  // Maintain uniform widget height in status bar by setting button height same as labels
  // For Qt/Mac 3.3, the default toolbutton height is 30 and labels were expanding to match
  mOnTheFlyProjectionStatusButton->setMaximumHeight( mScaleLabel->height() );
  mOnTheFlyProjectionStatusButton->setIcon( QgsApplication::getThemeIcon( "mIconProjectionEnabled.png" ) );
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

void QgisApp::setIconSizes( int size )
{
  //Set the icon size of for all the toolbars created in the future.
  setIconSize( QSize( size, size ) );

  //Change all current icon sizes.
  QList<QToolBar *> toolbars = findChildren<QToolBar *>();
  foreach ( QToolBar * toolbar, toolbars )
  {
    toolbar->setIconSize( QSize( size, size ) );
  }

  foreach ( QgsComposer *c, mPrintComposers )
  {
    c->setIconSizes( size );
  }
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
  mActionNewProject->setIcon( QgsApplication::getThemeIcon( "/mActionFileNew.png" ) );
  mActionOpenProject->setIcon( QgsApplication::getThemeIcon( "/mActionFileOpen.png" ) );
  mActionSaveProject->setIcon( QgsApplication::getThemeIcon( "/mActionFileSave.png" ) );
  mActionSaveProjectAs->setIcon( QgsApplication::getThemeIcon( "/mActionFileSaveAs.png" ) );
  mActionNewPrintComposer->setIcon( QgsApplication::getThemeIcon( "/mActionNewComposer.png" ) );
  mActionShowComposerManager->setIcon( QgsApplication::getThemeIcon( "/mActionComposerManager.png" ) );
  mActionSaveMapAsImage->setIcon( QgsApplication::getThemeIcon( "/mActionSaveMapAsImage.png" ) );
  mActionExit->setIcon( QgsApplication::getThemeIcon( "/mActionFileExit.png" ) );
  mActionAddOgrLayer->setIcon( QgsApplication::getThemeIcon( "/mActionAddOgrLayer.png" ) );
  mActionAddRasterLayer->setIcon( QgsApplication::getThemeIcon( "/mActionAddRasterLayer.png" ) );
#ifdef HAVE_POSTGRESQL
  mActionAddPgLayer->setIcon( QgsApplication::getThemeIcon( "/mActionAddLayer.png" ) );
#endif
  mActionNewSpatialiteLayer->setIcon( QgsApplication::getThemeIcon( "/mActionNewVectorLayer.png" ) );
  mActionAddSpatiaLiteLayer->setIcon( QgsApplication::getThemeIcon( "/mActionAddSpatiaLiteLayer.png" ) );
#ifdef HAVE_MSSQL
  mActionAddMssqlLayer->setIcon( QgsApplication::getThemeIcon( "/mActionAddMssqlLayer.png" ) );
#endif
#ifdef HAVE_ORACLE
  mActionAddOracleLayer->setIcon( QgsApplication::getThemeIcon( "/mActionAddOracleLayer.png" ) );
#endif
  mActionRemoveLayer->setIcon( QgsApplication::getThemeIcon( "/mActionRemoveLayer.png" ) );
  mActionDuplicateLayer->setIcon( QgsApplication::getThemeIcon( "/mActionAddMap.png" ) );
  mActionSetLayerCRS->setIcon( QgsApplication::getThemeIcon( "/mActionSetLayerCRS.png" ) );
  mActionSetProjectCRSFromLayer->setIcon( QgsApplication::getThemeIcon( "/mActionSetProjectCRSFromLayer.png" ) );
  mActionNewVectorLayer->setIcon( QgsApplication::getThemeIcon( "/mActionNewVectorLayer.png" ) );
  mActionAddAllToOverview->setIcon( QgsApplication::getThemeIcon( "/mActionAddAllToOverview.png" ) );
  mActionHideAllLayers->setIcon( QgsApplication::getThemeIcon( "/mActionHideAllLayers.png" ) );
  mActionShowAllLayers->setIcon( QgsApplication::getThemeIcon( "/mActionShowAllLayers.png" ) );
  mActionRemoveAllFromOverview->setIcon( QgsApplication::getThemeIcon( "/mActionRemoveAllFromOverview.png" ) );
  mActionToggleFullScreen->setIcon( QgsApplication::getThemeIcon( "/mActionToggleFullScreen.png" ) );
  mActionProjectProperties->setIcon( QgsApplication::getThemeIcon( "/mActionProjectProperties.png" ) );
  mActionManagePlugins->setIcon( QgsApplication::getThemeIcon( "/mActionShowPluginManager.png" ) );
  mActionShowPythonDialog->setIcon( QgsApplication::getThemeIcon( "console/iconRunConsole.png" ) );
  mActionCheckQgisVersion->setIcon( QgsApplication::getThemeIcon( "/mActionCheckQgisVersion.png" ) );
  mActionOptions->setIcon( QgsApplication::getThemeIcon( "/mActionOptions.png" ) );
  mActionConfigureShortcuts->setIcon( QgsApplication::getThemeIcon( "/mActionOptions.png" ) );
  mActionCustomization->setIcon( QgsApplication::getThemeIcon( "/mActionOptions.png" ) );
  mActionHelpContents->setIcon( QgsApplication::getThemeIcon( "/mActionHelpContents.png" ) );
  mActionLocalHistogramStretch->setIcon( QgsApplication::getThemeIcon( "/mActionLocalHistogramStretch.png" ) );
  mActionFullHistogramStretch->setIcon( QgsApplication::getThemeIcon( "/mActionFullHistogramStretch.png" ) );
  mActionIncreaseBrightness->setIcon( QgsApplication::getThemeIcon( "/mActionIncreaseBrightness.svg" ) );
  mActionDecreaseBrightness->setIcon( QgsApplication::getThemeIcon( "/mActionDecreaseBrightness.svg" ) );
  mActionIncreaseContrast->setIcon( QgsApplication::getThemeIcon( "/mActionIncreaseContrast.svg" ) );
  mActionDecreaseContrast->setIcon( QgsApplication::getThemeIcon( "/mActionDecreaseContrast.svg" ) );
  mActionZoomActualSize->setIcon( QgsApplication::getThemeIcon( "/mActionZoomNative.png" ) );
  mActionQgisHomePage->setIcon( QgsApplication::getThemeIcon( "/mActionQgisHomePage.png" ) );
  mActionAbout->setIcon( QgsApplication::getThemeIcon( "/mActionHelpAbout.png" ) );
  mActionSponsors->setIcon( QgsApplication::getThemeIcon( "/mActionHelpSponsors.png" ) );
  mActionDraw->setIcon( QgsApplication::getThemeIcon( "/mActionDraw.png" ) );
  mActionToggleEditing->setIcon( QgsApplication::getThemeIcon( "/mActionToggleEditing.svg" ) );
  mActionSaveLayerEdits->setIcon( QgsApplication::getThemeIcon( "/mActionSaveAllEdits.svg" ) );
  mActionAllEdits->setIcon( QgsApplication::getThemeIcon( "/mActionAllEdits.svg" ) );
  mActionSaveEdits->setIcon( QgsApplication::getThemeIcon( "/mActionSaveEdits.svg" ) );
  mActionSaveAllEdits->setIcon( QgsApplication::getThemeIcon( "/mActionSaveAllEdits.svg" ) );
  mActionRollbackEdits->setIcon( QgsApplication::getThemeIcon( "/mActionRollbackEdits.svg" ) );
  mActionRollbackAllEdits->setIcon( QgsApplication::getThemeIcon( "/mActionRollbackAllEdits.svg" ) );
  mActionCancelEdits->setIcon( QgsApplication::getThemeIcon( "/mActionCancelEdits.svg" ) );
  mActionCancelAllEdits->setIcon( QgsApplication::getThemeIcon( "/mActionCancelAllEdits.svg" ) );
  mActionCutFeatures->setIcon( QgsApplication::getThemeIcon( "/mActionEditCut.png" ) );
  mActionCopyFeatures->setIcon( QgsApplication::getThemeIcon( "/mActionEditCopy.png" ) );
  mActionPasteFeatures->setIcon( QgsApplication::getThemeIcon( "/mActionEditPaste.png" ) );
  mActionAddFeature->setIcon( QgsApplication::getThemeIcon( "/mActionCapturePoint.png" ) );
  mActionMoveFeature->setIcon( QgsApplication::getThemeIcon( "/mActionMoveFeature.png" ) );
  mActionRotateFeature->setIcon( QgsApplication::getThemeIcon( "/mActionRotateFeature.png" ) );
  mActionReshapeFeatures->setIcon( QgsApplication::getThemeIcon( "/mActionReshape.png" ) );
  mActionSplitFeatures->setIcon( QgsApplication::getThemeIcon( "/mActionSplitFeatures.svg" ) );
  mActionDeleteSelected->setIcon( QgsApplication::getThemeIcon( "/mActionDeleteSelected.png" ) );
  mActionNodeTool->setIcon( QgsApplication::getThemeIcon( "/mActionNodeTool.png" ) );
  mActionSimplifyFeature->setIcon( QgsApplication::getThemeIcon( "/mActionSimplify.png" ) );
  mActionUndo->setIcon( QgsApplication::getThemeIcon( "/mActionUndo.png" ) );
  mActionRedo->setIcon( QgsApplication::getThemeIcon( "/mActionRedo.png" ) );
  mActionAddRing->setIcon( QgsApplication::getThemeIcon( "/mActionAddRing.png" ) );
  mActionAddPart->setIcon( QgsApplication::getThemeIcon( "/mActionAddPart.png" ) );
  mActionDeleteRing->setIcon( QgsApplication::getThemeIcon( "/mActionDeleteRing.png" ) );
  mActionDeletePart->setIcon( QgsApplication::getThemeIcon( "/mActionDeletePart.png" ) );
  mActionMergeFeatures->setIcon( QgsApplication::getThemeIcon( "/mActionMergeFeatures.png" ) );
  mActionOffsetCurve->setIcon( QgsApplication::getThemeIcon( "/mActionOffsetCurve.png" ) );
  mActionMergeFeatureAttributes->setIcon( QgsApplication::getThemeIcon( "/mActionMergeFeatureAttributes.png" ) );
  mActionRotatePointSymbols->setIcon( QgsApplication::getThemeIcon( "mActionRotatePointSymbols.png" ) );
  mActionZoomIn->setIcon( QgsApplication::getThemeIcon( "/mActionZoomIn.png" ) );
  mActionZoomOut->setIcon( QgsApplication::getThemeIcon( "/mActionZoomOut.png" ) );
  mActionZoomFullExtent->setIcon( QgsApplication::getThemeIcon( "/mActionZoomFullExtent.png" ) );
  mActionZoomToSelected->setIcon( QgsApplication::getThemeIcon( "/mActionZoomToSelected.png" ) );
  mActionShowRasterCalculator->setIcon( QgsApplication::getThemeIcon( "/mActionShowRasterCalculator.png" ) );
#ifdef HAVE_TOUCH
  mActionTouch->setIcon( QgsApplication::getThemeIcon( "/mActionTouch.png" ) );
#endif
  mActionPan->setIcon( QgsApplication::getThemeIcon( "/mActionPan.png" ) );
  mActionPanToSelected->setIcon( QgsApplication::getThemeIcon( "/mActionPanToSelected.svg" ) );
  mActionZoomLast->setIcon( QgsApplication::getThemeIcon( "/mActionZoomLast.png" ) );
  mActionZoomNext->setIcon( QgsApplication::getThemeIcon( "/mActionZoomNext.png" ) );
  mActionZoomToLayer->setIcon( QgsApplication::getThemeIcon( "/mActionZoomToLayer.png" ) );
  mActionZoomActualSize->setIcon( QgsApplication::getThemeIcon( "/mActionZoomActual.png" ) );
  mActionIdentify->setIcon( QgsApplication::getThemeIcon( "/mActionIdentify.svg" ) );
  mActionFeatureAction->setIcon( QgsApplication::getThemeIcon( "/mAction.svg" ) );
  mActionSelect->setIcon( QgsApplication::getThemeIcon( "/mActionSelect.png" ) );
  mActionSelectRectangle->setIcon( QgsApplication::getThemeIcon( "/mActionSelectRectangle.png" ) );
  mActionSelectPolygon->setIcon( QgsApplication::getThemeIcon( "/mActionSelectPolygon.svg" ) );
  mActionSelectFreehand->setIcon( QgsApplication::getThemeIcon( "/mActionSelectFreehand.png" ) );
  mActionSelectRadius->setIcon( QgsApplication::getThemeIcon( "/mActionSelectRadius.png" ) );
  mActionDeselectAll->setIcon( QgsApplication::getThemeIcon( "/mActionDeselectAll.png" ) );
  mActionSelectByExpression->setIcon( QgsApplication::getThemeIcon( "/mIconExpressionSelect.svg" ) );
  mActionOpenTable->setIcon( QgsApplication::getThemeIcon( "/mActionOpenTable.png" ) );
  mActionOpenFieldCalc->setIcon( QgsApplication::getThemeIcon( "/mActionCalculateField.png" ) );
  mActionMeasure->setIcon( QgsApplication::getThemeIcon( "/mActionMeasure.png" ) );
  mActionMeasureArea->setIcon( QgsApplication::getThemeIcon( "/mActionMeasureArea.png" ) );
  mActionMeasureAngle->setIcon( QgsApplication::getThemeIcon( "/mActionMeasureAngle.png" ) );
  mActionMapTips->setIcon( QgsApplication::getThemeIcon( "/mActionMapTips.png" ) );
  mActionShowBookmarks->setIcon( QgsApplication::getThemeIcon( "/mActionShowBookmarks.png" ) );
  mActionNewBookmark->setIcon( QgsApplication::getThemeIcon( "/mActionNewBookmark.png" ) );
  mActionCustomProjection->setIcon( QgsApplication::getThemeIcon( "/mActionCustomProjection.png" ) );
  mActionAddWmsLayer->setIcon( QgsApplication::getThemeIcon( "/mActionAddWmsLayer.png" ) );
  mActionAddWcsLayer->setIcon( QgsApplication::getThemeIcon( "/mActionAddWcsLayer.png" ) );
  mActionAddWfsLayer->setIcon( QgsApplication::getThemeIcon( "/mActionAddWfsLayer.png" ) );
  mActionAddToOverview->setIcon( QgsApplication::getThemeIcon( "/mActionInOverview.png" ) );
  mActionAnnotation->setIcon( QgsApplication::getThemeIcon( "/mActionAnnotation.png" ) );
  mActionFormAnnotation->setIcon( QgsApplication::getThemeIcon( "/mActionFormAnnotation.png" ) );
  mActionHtmlAnnotation->setIcon( QgsApplication::getThemeIcon( "/mActionFormAnnotation.png" ) );
  mActionTextAnnotation->setIcon( QgsApplication::getThemeIcon( "/mActionTextAnnotation.png" ) );
  mActionLabeling->setIcon( QgsApplication::getThemeIcon( "/mActionLabeling.png" ) );
  mActionShowPinnedLabels->setIcon( QgsApplication::getThemeIcon( "/mActionShowPinnedLabels.svg" ) );
  mActionPinLabels->setIcon( QgsApplication::getThemeIcon( "/mActionPinLabels.svg" ) );
  mActionShowHideLabels->setIcon( QgsApplication::getThemeIcon( "/mActionShowHideLabels.svg" ) );
  mActionMoveLabel->setIcon( QgsApplication::getThemeIcon( "/mActionMoveLabel.png" ) );
  mActionRotateLabel->setIcon( QgsApplication::getThemeIcon( "/mActionRotateLabel.svg" ) );
  mActionChangeLabelProperties->setIcon( QgsApplication::getThemeIcon( "/mActionChangeLabelProperties.png" ) );
  mActionDecorationCopyright->setIcon( QgsApplication::getThemeIcon( "/plugins/copyright_label.png" ) );
  mActionDecorationNorthArrow->setIcon( QgsApplication::getThemeIcon( "/plugins/north_arrow.png" ) );
  mActionDecorationScaleBar->setIcon( QgsApplication::getThemeIcon( "/plugins/scale_bar.png" ) );
  mActionDecorationGrid->setIcon( QgsApplication::getThemeIcon( "/transformed.png" ) );

  //change themes of all composers
  QSet<QgsComposer*>::iterator composerIt = mPrintComposers.begin();
  for ( ; composerIt != mPrintComposers.end(); ++composerIt )
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
  connect( QgsMapLayerRegistry::instance(), SIGNAL( layersAdded( QList<QgsMapLayer *> ) ),
           this, SLOT( layersWereAdded( QList<QgsMapLayer *> ) ) );
  connect( QgsMapLayerRegistry::instance(),
           SIGNAL( layersWillBeRemoved( QStringList ) ),
           this, SLOT( removingLayers( QStringList ) ) );

  // connect initialization signal
  connect( this, SIGNAL( initializationCompleted() ),
           this, SLOT( fileOpenAfterLaunch() ) );

  // Connect warning dialog from project reading
  connect( QgsProject::instance(), SIGNAL( oldProjectVersionWarning( QString ) ),
           this, SLOT( oldProjectVersionWarning( QString ) ) );
  connect( QgsProject::instance(), SIGNAL( layerLoaded( int, int ) ),
           this, SLOT( showProgress( int, int ) ) );
  connect( QgsProject::instance(), SIGNAL( loadingLayer( QString ) ),
           this, SLOT( showStatusMessage( QString ) ) );
  connect( QgsProject::instance(), SIGNAL( readProject( const QDomDocument & ) ),
           this, SLOT( readProject( const QDomDocument & ) ) );
  connect( QgsProject::instance(), SIGNAL( writeProject( QDomDocument & ) ),
           this, SLOT( writeProject( QDomDocument & ) ) );
  connect( QgsProject::instance(), SIGNAL( writeProject( QDomDocument& ) ),
           this, SLOT( writeAnnotationItemsToProject( QDomDocument& ) ) );

  connect( QgsProject::instance(), SIGNAL( readProject( const QDomDocument & ) ), this, SLOT( loadComposersFromProject( const QDomDocument& ) ) );
  connect( QgsProject::instance(), SIGNAL( readProject( const QDomDocument & ) ), this, SLOT( loadAnnotationItemsFromProject( const QDomDocument& ) ) );

  connect( this, SIGNAL( projectRead() ),
           this, SLOT( fileOpenedOKAfterLaunch() ) );

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
#ifdef HAVE_TOUCH
  mMapTools.mTouch = new QgsMapToolTouch( mMapCanvas );
  mMapTools.mTouch->setAction( mActionTouch );
#endif
  mMapTools.mIdentify = new QgsMapToolIdentifyAction( mMapCanvas );
  mMapTools.mIdentify->setAction( mActionIdentify );
  connect( mMapTools.mIdentify, SIGNAL( copyToClipboard( QgsFeatureStore & ) ),
           this, SLOT( copyFeatures( QgsFeatureStore & ) ) );
  mMapTools.mFeatureAction = new QgsMapToolFeatureAction( mMapCanvas );
  mMapTools.mFeatureAction->setAction( mActionFeatureAction );
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
  mMapTools.mHtmlAnnotation = new QgsMapToolHtmlAnnotation( mMapCanvas );
  mMapTools.mHtmlAnnotation->setAction( mActionHtmlAnnotation );
  mMapTools.mSvgAnnotation = new QgsMapToolSvgAnnotation( mMapCanvas );
  mMapTools.mSvgAnnotation->setAction( mActionSvgAnnotation );
  mMapTools.mAnnotation = new QgsMapToolAnnotation( mMapCanvas );
  mMapTools.mAnnotation->setAction( mActionAnnotation );
  mMapTools.mAddFeature = new QgsMapToolAddFeature( mMapCanvas );
  mMapTools.mAddFeature->setAction( mActionAddFeature );
  mMapTools.mMoveFeature = new QgsMapToolMoveFeature( mMapCanvas );
  mMapTools.mMoveFeature->setAction( mActionMoveFeature );
  mMapTools.mRotateFeature = new QgsMapToolRotateFeature( mMapCanvas );
  mMapTools.mRotateFeature->setAction( mActionRotateFeature );
  //need at least geos 3.3 for OffsetCurve tool
#if defined(GEOS_VERSION_MAJOR) && defined(GEOS_VERSION_MINOR) && \
  ((GEOS_VERSION_MAJOR>3) || ((GEOS_VERSION_MAJOR==3) && (GEOS_VERSION_MINOR>=3)))
  mMapTools.mOffsetCurve = new QgsMapToolOffsetCurve( mMapCanvas );
  mMapTools.mOffsetCurve->setAction( mActionOffsetCurve );
#else
  mAdvancedDigitizeToolBar->removeAction( mActionOffsetCurve );
  mEditMenu->removeAction( mActionOffsetCurve );
  mMapTools.mOffsetCurve = 0;
#endif //GEOS_VERSION
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
  mMapTools.mAddRing = new QgsMapToolAddRing( mMapCanvas );
  mMapTools.mAddRing->setAction( mActionAddRing );
  mMapTools.mAddPart = new QgsMapToolAddPart( mMapCanvas );
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
  mMapTools.mPinLabels = new QgsMapToolPinLabels( mMapCanvas );
  mMapTools.mPinLabels->setAction( mActionPinLabels );
  mMapTools.mShowHideLabels = new QgsMapToolShowHideLabels( mMapCanvas );
  mMapTools.mShowHideLabels->setAction( mActionShowHideLabels );
  mMapTools.mMoveLabel = new QgsMapToolMoveLabel( mMapCanvas );
  mMapTools.mMoveLabel->setAction( mActionMoveLabel );

  mMapTools.mRotateLabel = new QgsMapToolRotateLabel( mMapCanvas );
  mMapTools.mRotateLabel->setAction( mActionRotateLabel );
  mMapTools.mChangeLabelProperties = new QgsMapToolChangeLabelProperties( mMapCanvas );
  mMapTools.mChangeLabelProperties->setAction( mActionChangeLabelProperties );
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
  // Anti Aliasing enabled by default as of QGIS 1.7
  mMapCanvas->enableAntiAliasing( mySettings.value( "/qgis/enable_anti_aliasing", true ).toBool() );
  mMapCanvas->useImageToRender( mySettings.value( "/qgis/use_qimage_to_render", true ).toBool() );

  int action = mySettings.value( "/qgis/wheel_action", 2 ).toInt();
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

void QgisApp::removeDockWidget( QDockWidget * thepDockWidget )
{
  QMainWindow::removeDockWidget( thepDockWidget );
  mPanelMenu->removeAction( thepDockWidget->toggleViewAction() );
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

QgsPluginManager *QgisApp::pluginManager()
{
  Q_ASSERT( mPluginManager );
  return mPluginManager;
}

QgsMapCanvas *QgisApp::mapCanvas()
{
  Q_ASSERT( mMapCanvas );
  return mMapCanvas;
}

QgsPalLabeling *QgisApp::palLabeling()
{
  Q_ASSERT( mLBL );
  return mLBL;
}

QgsMessageBar* QgisApp::messageBar()
{
  Q_ASSERT( mInfoBar );
  return mInfoBar;
}

void QgisApp::initLegend()
{
  mMapLegend->setWhatsThis( tr( "Map legend that displays all the layers currently on the map canvas. Click on the check box to turn a layer on or off. Double click on a layer in the legend to customize its appearance and set other properties." ) );
  mLegendDock = new QDockWidget( tr( "Layers" ), this );
  mLegendDock->setObjectName( "Legend" );
  mLegendDock->setAllowedAreas( Qt::LeftDockWidgetArea | Qt::RightDockWidgetArea );

  QCheckBox *orderCb = new QCheckBox( tr( "Control rendering order" ) );
  orderCb->setChecked( false );

  connect( orderCb, SIGNAL( toggled( bool ) ), mMapLegend, SLOT( unsetUpdateDrawingOrder( bool ) ) );
  connect( mMapLegend, SIGNAL( updateDrawingOrderUnchecked( bool ) ), orderCb, SLOT( setChecked( bool ) ) );

  QWidget *w = new QWidget( this );
  QLayout *l = new QVBoxLayout;
  l->setMargin( 0 );
  l->addWidget( mMapLegend );
  w->setLayout( l );
  mLegendDock->setWidget( w );
  addDockWidget( Qt::LeftDockWidgetArea, mLegendDock );

  // add to the Panel submenu
  mPanelMenu->addAction( mLegendDock->toggleViewAction() );

  mMapLayerOrder->setWhatsThis( tr( "Map layer list that displays all layers in drawing order." ) );
  mLayerOrderDock = new QDockWidget( tr( "Layer order" ), this );
  mLayerOrderDock->setObjectName( "LayerOrder" );
  mLayerOrderDock->setAllowedAreas( Qt::LeftDockWidgetArea | Qt::RightDockWidgetArea );

  w = new QWidget( this );
  l = new QVBoxLayout;
  l->setMargin( 0 );
  l->addWidget( mMapLayerOrder );
  l->addWidget( orderCb );
  w->setLayout( l );
  mLayerOrderDock->setWidget( w );
  addDockWidget( Qt::LeftDockWidgetArea, mLayerOrderDock );
  mLayerOrderDock->hide();

  // add to the Panel submenu
  mPanelMenu->addAction( mLayerOrderDock->toggleViewAction() );

  return;
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

void QgisApp::createDecorations()
{
  QgsDecorationCopyright* mDecorationCopyright = new QgsDecorationCopyright( this );
  connect( mActionDecorationCopyright, SIGNAL( triggered() ), mDecorationCopyright, SLOT( run() ) );

  QgsDecorationNorthArrow* mDecorationNorthArrow = new QgsDecorationNorthArrow( this );
  connect( mActionDecorationNorthArrow, SIGNAL( triggered() ), mDecorationNorthArrow, SLOT( run() ) );

  QgsDecorationScaleBar* mDecorationScaleBar = new QgsDecorationScaleBar( this );
  connect( mActionDecorationScaleBar, SIGNAL( triggered() ), mDecorationScaleBar, SLOT( run() ) );

  QgsDecorationGrid* mDecorationGrid = new QgsDecorationGrid( this );
  connect( mActionDecorationGrid, SIGNAL( triggered() ), mDecorationGrid, SLOT( run() ) );

  // add the decorations in a particular order so they are rendered in that order
  addDecorationItem( mDecorationGrid );
  addDecorationItem( mDecorationCopyright );
  addDecorationItem( mDecorationNorthArrow );
  addDecorationItem( mDecorationScaleBar );
  connect( mMapCanvas, SIGNAL( renderComplete( QPainter * ) ), this, SLOT( renderDecorationItems( QPainter * ) ) );
  connect( this, SIGNAL( newProject() ), this, SLOT( projectReadDecorationItems() ) );
  connect( this, SIGNAL( projectRead() ), this, SLOT( projectReadDecorationItems() ) );
}

void QgisApp::renderDecorationItems( QPainter *p )
{
  foreach ( QgsDecorationItem* item, mDecorationItems )
  {
    item->render( p );
  }
}

void QgisApp::projectReadDecorationItems()
{
  foreach ( QgsDecorationItem* item, mDecorationItems )
  {
    item->projectRead( );
  }
}

// Update project menu with the current list of recently accessed projects
void QgisApp::updateRecentProjectPaths()
{
  // Remove existing paths from the recent projects menu
  int i;

  int menusize = mRecentProjectsMenu->actions().size();

  for ( i = menusize; i < mRecentProjectPaths.size(); i++ )
  {
    mRecentProjectsMenu->addAction( "Dummy text" );
  }

  QList<QAction *> menulist = mRecentProjectsMenu->actions();

  assert( menulist.size() == mRecentProjectPaths.size() );

  for ( i = 0; i < mRecentProjectPaths.size(); i++ )
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
  while ( mRecentProjectPaths.count() > 8 )
  {
    mRecentProjectPaths.pop_back();
  }

  // Persist the list
  settings.setValue( "/UI/recentProjectsList", mRecentProjectPaths );


  // Update menu list of paths
  updateRecentProjectPaths();

} // QgisApp::saveRecentProjectPath

// Update project menu with the project templates
void QgisApp::updateProjectFromTemplates()
{
  // get list of project files in template dir
  QSettings settings;
  QString templateDirName = settings.value( "/qgis/projectTemplateDir",
                            QgsApplication::qgisSettingsDirPath() + "project_templates" ).toString();
  QDir templateDir( templateDirName );
  QStringList filters( "*.qgs" );
  templateDir.setNameFilters( filters );
  QStringList templateFiles = templateDir.entryList( filters );

  // Remove existing entries
  mProjectFromTemplateMenu->clear();

  // Add entries
  foreach ( QString templateFile, templateFiles )
  {
    mProjectFromTemplateMenu->addAction( templateFile );
  }

  // add <blank> entry, which loads a blank template (regardless of "default template")
  if ( settings.value( "/qgis/newProjectDefault", QVariant( false ) ).toBool() )
    mProjectFromTemplateMenu->addAction( tr( "< Blank >" ) );

} // QgisApp::updateProjectFromTemplates

void QgisApp::saveWindowState()
{
  // store window and toolbar positions
  QSettings settings;
  // store the toolbar/dock widget settings using Qt4 settings API
  settings.setValue( "/UI/state", saveState() );

  // store window geometry
  settings.setValue( "/UI/geometry", saveGeometry() );

  QgsPluginRegistry::instance()->unloadAll();
}

#include "ui_defaults.h"

void QgisApp::restoreWindowState()
{
  // restore the toolbar and dock widgets postions using Qt4 settings API
  QSettings settings;

  if ( !restoreState( settings.value( "/UI/state", QByteArray::fromRawData(( char * )defaultUIstate, sizeof defaultUIstate ) ).toByteArray() ) )
  {
    QgsDebugMsg( "restore of UI state failed" );
  }

  // restore window geometry
  if ( !restoreGeometry( settings.value( "/UI/geometry", QByteArray::fromRawData(( char * )defaultUIgeometry, sizeof defaultUIgeometry ) ).toByteArray() ) )
  {
    QgsDebugMsg( "restore of UI geometry failed" );
  }

}
///////////// END OF GUI SETUP ROUTINES ///////////////
void QgisApp::sponsors()
{
  QgsSponsors * sponsors = new QgsSponsors( this );
  sponsors->show();
  sponsors->raise();
  sponsors->activateWindow();
}

void QgisApp::about()
{
  static QgsAbout *abt = NULL;
  if ( !abt )
  {
    QApplication::setOverrideCursor( Qt::WaitCursor );
    abt = new QgsAbout( this );
    QString versionString = "<html><body><div align='center'><table width='100%'>";

    versionString += "<tr>";
    versionString += "<td>" + tr( "QGIS version" )       + "</td><td>" + QGis::QGIS_VERSION + "</td>";
    versionString += "<td>" + tr( "QGIS code revision" ) + "</td><td>" + QGis::QGIS_DEV_VERSION + "</td>";

    versionString += "</tr><tr>";

    versionString += "<td>" + tr( "Compiled against Qt" ) + "</td><td>" + QT_VERSION_STR + "</td>";
    versionString += "<td>" + tr( "Running against Qt" )  + "</td><td>" + qVersion() + "</td>";

    versionString += "</tr><tr>";

    versionString += "<td>" + tr( "Compiled against GDAL/OGR" ) + "</td><td>" + GDAL_RELEASE_NAME + "</td>";
    versionString += "<td>" + tr( "Running against GDAL/OGR" )  + "</td><td>" + GDALVersionInfo( "RELEASE_NAME" ) + "</td>";

    versionString += "</tr><tr>";

    versionString += "<td>" + tr( "GEOS Version" ) + "</td><td>" + GEOS_VERSION + "</td>";
    versionString += "<td>" + tr( "PostgreSQL Client Version" ) + "</td><td>";
#ifdef HAVE_POSTGRESQL
    versionString += PG_VERSION;
#else
    versionString += tr( "No support." );
#endif
    versionString += "</td>";

    versionString += "</tr><tr>";

    versionString += "<td>" +  tr( "SpatiaLite Version" ) + "</td><td>";
    versionString += spatialite_version();
    versionString += "</td>";

    versionString += "<td>" + tr( "QWT Version" ) + "</td><td>" + QWT_VERSION_STR + "</td>";

    versionString += "</tr><tr>";

    versionString += "<td>" + tr( "PROJ.4 Version" ) + "</td><td>" + QString::number( PJ_VERSION ) + "</td>";

    versionString += "<td>" + tr( "QScintilla2 Version" ) + "</td><td>" + QSCINTILLA_VERSION_STR + "</td>";

#ifdef QGISDEBUG
    versionString += "</tr><tr><td colspan=4>" + tr( "This copy of QGIS writes debugging output." ) + "</td>";
#endif

    versionString += "</tr></table></div></body></html>";

    abt->setVersion( versionString );

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
  if ( mMapCanvas && mMapCanvas->isDrawing() )
  {
    return;
  }
  mMapCanvas->freeze();
  QgsOpenVectorLayerDialog *ovl = new QgsOpenVectorLayerDialog( this );

  if ( ovl->exec() )
  {
    QStringList selectedSources = ovl->dataSources();
    QString enc = ovl->encoding();
    if ( !selectedSources.isEmpty() )
    {
      addVectorLayers( selectedSources, enc, ovl->dataSourceType() );
    }
  }

  mMapCanvas->freeze( false );
  mMapCanvas->refresh();

  delete ovl;
  // update UI
  qApp->processEvents();
}


bool QgisApp::addVectorLayers( QStringList const & theLayerQStringList, const QString& enc, const QString dataSourceType )
{
  bool wasfrozen = mMapCanvas->isFrozen();
  QList<QgsMapLayer *> myList;
  foreach ( QString src, theLayerQStringList )
  {
    src = src.trimmed();
    QString base;
    if ( dataSourceType == "file" )
    {
      QFileInfo fi( src );
      base = fi.completeBaseName();

      // if needed prompt for zipitem layers
      QString vsiPrefix = QgsZipItem::vsiPrefix( src );
      if ( ! src.startsWith( "/vsi", Qt::CaseInsensitive ) &&
           ( vsiPrefix == "/vsizip/" || vsiPrefix == "/vsitar/" ) )
      {
        if ( askUserForZipItemLayers( src ) )
          continue;
      }
    }
    else if ( dataSourceType == "database" )
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

    if ( ! layer )
    {
      mMapCanvas->freeze( false );

      // Let render() do its own cursor management
      //      QApplication::restoreOverrideCursor();

      // XXX insert meaningful whine to the user here
      return false;
    }

    if ( layer->isValid() )
    {
      layer->setProviderEncoding( enc );

      QStringList sublayers = layer->dataProvider()->subLayers();
      QgsDebugMsg( QString( "got valid layer with %1 sublayers" ).arg( sublayers.count() ) );

      // If the newly created layer has more than 1 layer of data available, we show the
      // sublayers selection dialog so the user can select the sublayers to actually load.
      if ( sublayers.count() > 1 )
      {
        askUserForOGRSublayers( layer );

        // The first layer loaded is not useful in that case. The user can select it in
        // the list if he wants to load it.
        delete layer;

      }
      else if ( sublayers.count() > 0 ) // there is 1 layer of data available
      {
        //set friendly name for datasources with only one layer
        QStringList sublayers = layer->dataProvider()->subLayers();
        QString ligne = sublayers.at( 0 );
        QStringList elements = ligne.split( ":" );
        layer->setLayerName( elements.at( 1 ) );
        myList << layer;
      }
      else
      {
        QString msg = tr( "%1 doesn't have any layers" ).arg( src );
        messageBar()->pushMessage( tr( "Invalid Data Source" ), msg, QgsMessageBar::CRITICAL, messageTimeout() );
        delete layer;
      }
    }
    else
    {
      QString msg = tr( "%1 is not a valid or recognized data source" ).arg( src );
      messageBar()->pushMessage( tr( "Invalid Data Source" ), msg, QgsMessageBar::CRITICAL, messageTimeout() );

      // since the layer is bad, stomp on it
      delete layer;
    }

  }

  // make sure at least one layer was successfully added
  if ( myList.count() == 0 )
  {
    return false;
  }

  // Register this layer with the layers registry
  QgsMapLayerRegistry::instance()->addMapLayers( myList );

  // update UI
  qApp->processEvents();

  // Only update the map if we frozen in this method
  // Let the caller do it otherwise
  if ( !wasfrozen )
  {
    mMapCanvas->freeze( false );
    mMapCanvas->refresh();
  }
// Let render() do its own cursor management
//  QApplication::restoreOverrideCursor();

  // statusBar()->showMessage( mMapCanvas->extent().toString( 2 ) );

  return true;
} // QgisApp::addVectorLayer()

// present a dialog to choose zipitem layers
bool QgisApp::askUserForZipItemLayers( QString path )
{
  bool ok = false;
  QVector<QgsDataItem*> childItems;
  QgsZipItem *zipItem = 0;
  QSettings settings;
  int promptLayers = settings.value( "/qgis/promptForRasterSublayers", 1 ).toInt();

  QgsDebugMsg( "askUserForZipItemLayers( " + path + ")" );

  // if scanZipBrowser == no: skip to the next file
  if ( settings.value( "/qgis/scanZipInBrowser2", "basic" ).toString() == "no" )
  {
    return false;
  }

  zipItem = new QgsZipItem( 0, path, path );
  if ( ! zipItem )
    return false;

  zipItem->populate();
  QgsDebugMsg( QString( "Path= %1 got zipitem with %2 children" ).arg( path ).arg( zipItem->rowCount() ) );

  // if 1 or 0 child found, exit so a normal item is created by gdal or ogr provider
  if ( zipItem->rowCount() <= 1 )
  {
    delete zipItem;
    return false;
  }

  // if promptLayers=Load all, load all layers without prompting
  if ( promptLayers == 3 )
  {
    childItems = zipItem->children();
  }
  // exit if promptLayers=Never
  else if ( promptLayers == 2 )
  {
    delete zipItem;
    return false;
  }
  else
  {
    // We initialize a selection dialog and display it.
    QgsSublayersDialog chooseSublayersDialog( QgsSublayersDialog::Vsifile, "vsi", this );

    QStringList layers;
    for ( int i = 0; i < zipItem->children().size(); i++ )
    {
      QgsDataItem *item = zipItem->children()[i];
      QgsLayerItem *layerItem = dynamic_cast<QgsLayerItem *>( item );
      QgsDebugMsgLevel( QString( "item path=%1 provider=%2" ).arg( item->path() ).arg( layerItem->providerKey() ), 2 );
      if ( layerItem && layerItem->providerKey() == "gdal" )
      {
        layers << QString( "%1|%2|%3" ).arg( i ).arg( item->name() ).arg( "Raster" );
      }
      else if ( layerItem && layerItem->providerKey() == "ogr" )
      {
        layers << QString( "%1|%2|%3" ).arg( i ).arg( item->name() ).arg( tr( "Vector" ) );
      }
    }

    chooseSublayersDialog.populateLayerTable( layers, "|" );

    if ( chooseSublayersDialog.exec() )
    {
      foreach ( int i, chooseSublayersDialog.selectionIndexes() )
      {
        childItems << zipItem->children()[i];
      }
    }
  }

  if ( childItems.isEmpty() )
  {
    // return true so dialog doesn't popup again (#6225) - hopefully this doesn't create other trouble
    ok = true;
  }

  // add childItems
  foreach ( QgsDataItem* item, childItems )
  {
    QgsLayerItem *layerItem = dynamic_cast<QgsLayerItem *>( item );
    QgsDebugMsg( QString( "item path=%1 provider=%2" ).arg( item->path() ).arg( layerItem->providerKey() ) );
    if ( layerItem && layerItem->providerKey() == "gdal" )
    {
      if ( addRasterLayer( item->path(), QFileInfo( item->name() ).completeBaseName() ) )
        ok = true;
    }
    else if ( layerItem && layerItem->providerKey() == "ogr" )
    {
      if ( addVectorLayers( QStringList( item->path() ), "System", "file" ) )
        ok = true;
    }
  }

  delete zipItem;
  return ok;
}

// present a dialog to choose GDAL raster sublayers
void QgisApp::askUserForGDALSublayers( QgsRasterLayer *layer )
{
  if ( !layer )
    return;

  QStringList sublayers = layer->subLayers();
  QgsDebugMsg( QString( "raster has %1 sublayers" ).arg( layer->subLayers().size() ) );

  if ( sublayers.size() < 1 )
    return;

  // if promptLayers=Load all, load all sublayers without prompting
  QSettings settings;
  if ( settings.value( "/qgis/promptForRasterSublayers", 1 ).toInt() == 3 )
  {
    loadGDALSublayers( layer->source(), sublayers );
    return;
  }

  // We initialize a selection dialog and display it.
  QgsSublayersDialog chooseSublayersDialog( QgsSublayersDialog::Gdal, "gdal", this );

  QStringList layers;
  QStringList names;
  for ( int i = 0; i < sublayers.size(); i++ )
  {
    // simplify raster sublayer name - should add a function in gdal provider for this?
    // code is copied from QgsGdalLayerItem::createChildren
    QString name = sublayers[i];
    QString path = layer->source();
    // if netcdf/hdf use all text after filename
    // for hdf4 it would be best to get description, because the subdataset_index is not very practical
    if ( name.startsWith( "netcdf", Qt::CaseInsensitive ) ||
         name.startsWith( "hdf", Qt::CaseInsensitive ) )
      name = name.mid( name.indexOf( path ) + path.length() + 1 );
    else
    {
      // remove driver name and file name
      name.replace( name.split( ":" )[0], "" );
      name.replace( path, "" );
    }
    // remove any : or " left over
    if ( name.startsWith( ":" ) )
      name.remove( 0, 1 );

    if ( name.startsWith( "\"" ) )
      name.remove( 0, 1 );

    if ( name.endsWith( ":" ) )
      name.chop( 1 );

    if ( name.endsWith( "\"" ) )
      name.chop( 1 );

    names << name;
    layers << QString( "%1|%2" ).arg( i ).arg( name );
  }

  chooseSublayersDialog.populateLayerTable( layers, "|" );

  if ( chooseSublayersDialog.exec() )
  {
    foreach ( int i, chooseSublayersDialog.selectionIndexes() )
    {
      QgsRasterLayer *rlayer = new QgsRasterLayer( sublayers[i], names[i] );
      if ( rlayer && rlayer->isValid() )
      {
        addRasterLayer( rlayer );
      }
    }
  }
}

// should the GDAL sublayers dialog should be presented to the user?
bool QgisApp::shouldAskUserForGDALSublayers( QgsRasterLayer *layer )
{
  // return false if layer is empty or raster has no sublayers
  if ( !layer || layer->providerType() != "gdal" || layer->subLayers().size() < 1 )
    return false;

  QSettings settings;
  int promptLayers = settings.value( "/qgis/promptForRasterSublayers", 1 ).toInt();
  // 0 = Always -> always ask (if there are existing sublayers)
  // 1 = If needed -> ask if layer has no bands, but has sublayers
  // 2 = Never -> never prompt, will not load anything
  // 3 = Load all -> never prompt, but load all sublayers

  return promptLayers == 0 || promptLayers == 3 || ( promptLayers == 1 && layer->bandCount() == 0 );
}

// This method will load with GDAL the layers in parameter.
// It is normally triggered by the sublayer selection dialog.
void QgisApp::loadGDALSublayers( QString uri, QStringList list )
{
  QString path, name;
  QgsRasterLayer *subLayer = NULL;

  //add layers in reverse order so they appear in the right order in the layer dock
  for ( int i = list.size() - 1; i >= 0 ; i-- )
  {
    path = list[i];
    // shorten name by replacing complete path with filename
    name = path;
    name.replace( uri, QFileInfo( uri ).completeBaseName() );
    subLayer = new QgsRasterLayer( path, name );
    if ( subLayer )
    {
      if ( subLayer->isValid() )
        addRasterLayer( subLayer );
      else
        delete subLayer;
    }

  }
}

// This method is the method that does the real job. If the layer given in
// parameter is NULL, then the method tries to act on the activeLayer.
void QgisApp::askUserForOGRSublayers( QgsVectorLayer *layer )
{
  if ( !layer )
  {
    layer = qobject_cast<QgsVectorLayer *>( activeLayer() );
    if ( !layer || layer->dataProvider()->name() != "ogr" )
      return;
  }

  QStringList sublayers = layer->dataProvider()->subLayers();
  QString layertype = layer->dataProvider()->storageType();

  // We initialize a selection dialog and display it.
  QgsSublayersDialog chooseSublayersDialog( QgsSublayersDialog::Ogr, "ogr", this );
  chooseSublayersDialog.populateLayerTable( sublayers );

  if ( chooseSublayersDialog.exec() )
  {
    QString uri = layer->source();
    //the separator char & was changed to | to be compatible
    //with url for protocol drivers
    if ( uri.contains( '|', Qt::CaseSensitive ) )
    {
      // If we get here, there are some options added to the filename.
      // A valid uri is of the form: filename&option1=value1&option2=value2,...
      // We want only the filename here, so we get the first part of the split.
      QStringList theURIParts = uri.split( "|" );
      uri = theURIParts.at( 0 );
    }
    QgsDebugMsg( "Layer type " + layertype );
    // the user has done his choice
    loadOGRSublayers( layertype , uri, chooseSublayersDialog.selectionNames() );
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
  QList<QgsMapLayer *> myList;
  for ( int i = 0; i < list.size(); i++ )
  {
    QString composedURI;
    QString layerName = list.at( i ).split( ':' ).value( 0 );
    QString layerType = list.at( i ).split( ':' ).value( 1 );

    if ( layertype != "GRASS" )
    {
      composedURI = uri + "|layername=" + layerName;
    }
    else
    {
      composedURI = uri + "|layerindex=" + layerName;
    }

    if ( !layerType.isEmpty() )
    {
      composedURI += "|geometrytype=" + layerType;
    }

    // addVectorLayer( composedURI,  list.at( i ), "ogr" );

    QgsDebugMsg( "Creating new vector layer using " + composedURI );
    QString name = list.at( i );
    name.replace( ":", " " );
    QgsVectorLayer *layer = new QgsVectorLayer( composedURI, name, "ogr" );
    if ( layer && layer->isValid() )
    {
      myList << layer;
    }
    else
    {
      QString msg = tr( "%1 is not a valid or recognized data source" ).arg( composedURI );
      messageBar()->pushMessage( tr( "Invalid Data Source" ), msg, QgsMessageBar::CRITICAL, messageTimeout() );
      if ( layer )
        delete layer;
    }
  }

  if ( ! myList.isEmpty() )
  {
    // Register layer(s) with the layers registry
    QgsMapLayerRegistry::instance()->addMapLayers( myList );
  }
}

void QgisApp::addDatabaseLayer()
{
#ifdef HAVE_POSTGRESQL
  if ( mMapCanvas && mMapCanvas->isDrawing() )
  {
    return;
  }
  // Fudge for now
  QgsDebugMsg( "about to addRasterLayer" );

  // TODO: QDialog for now, switch to QWidget in future
  QDialog *dbs = dynamic_cast<QDialog*>( QgsProviderRegistry::instance()->selectWidget( "postgres", this ) );
  if ( !dbs )
  {
    QMessageBox::warning( this, tr( "PostgreSQL" ), tr( "Cannot get PostgreSQL select dialog from provider." ) );
    return;
  }
  connect( dbs, SIGNAL( addDatabaseLayers( QStringList const &, QString const & ) ),
           this, SLOT( addDatabaseLayers( QStringList const &, QString const & ) ) );
  connect( dbs, SIGNAL( progress( int, int ) ),
           this, SLOT( showProgress( int, int ) ) );
  connect( dbs, SIGNAL( progressMessage( QString ) ),
           this, SLOT( showStatusMessage( QString ) ) );
  dbs->exec();
  delete dbs;
#endif
} // QgisApp::addDatabaseLayer()

void QgisApp::addDatabaseLayers( QStringList const & layerPathList, QString const & providerKey )
{
  QList<QgsMapLayer *> myList;
  if ( mMapCanvas && mMapCanvas->isDrawing() )
  {
    return;
  }

  if ( layerPathList.empty() )
  {
    // no layers to add so bail out, but
    // allow mMapCanvas to handle events
    // first
    mMapCanvas->freeze( false );
    return;
  }

  mMapCanvas->freeze( true );

  QApplication::setOverrideCursor( Qt::WaitCursor );

  foreach ( QString layerPath, layerPathList )
  {
    // create the layer
    QgsDataSourceURI uri( layerPath );

    QgsVectorLayer *layer = new QgsVectorLayer( uri.uri(), uri.table(), providerKey );
    Q_CHECK_PTR( layer );

    if ( ! layer )
    {
      mMapCanvas->freeze( false );
      QApplication::restoreOverrideCursor();

      // XXX insert meaningful whine to the user here
      return;
    }

    if ( layer->isValid() )
    {
      // add to list of layers to register
      //with the central layers registry
      myList << layer;
    }
    else
    {
      QgsMessageLog::logMessage( tr( "%1 is an invalid layer - not loaded" ).arg( layerPath ) );
      QLabel *msgLabel = new QLabel( tr( "%1 is an invalid layer and cannot be loaded. Please check the <a href=\"#messageLog\">message log</a> for further info." ).arg( layerPath ), messageBar() );
      msgLabel->setWordWrap( true );
      connect( msgLabel, SIGNAL( linkActivated( QString ) ), mLogDock, SLOT( show() ) );
      messageBar()->pushWidget( msgLabel,
                                QgsMessageBar::WARNING );
      delete layer;
    }
    //qWarning("incrementing iterator");
  }

  QgsMapLayerRegistry::instance()->addMapLayers( myList );
  statusBar()->showMessage( mMapCanvas->extent().toString( 2 ) );

  // update UI
  qApp->processEvents();

  // draw the map
  mMapCanvas->freeze( false );
  mMapCanvas->refresh();

  QApplication::restoreOverrideCursor();
}


void QgisApp::addSpatiaLiteLayer()
{
  if ( mMapCanvas && mMapCanvas->isDrawing() )
  {
    return;
  }

  // show the SpatiaLite dialog
  QDialog *dbs = dynamic_cast<QDialog*>( QgsProviderRegistry::instance()->selectWidget( "spatialite", this ) );
  if ( !dbs )
  {
    QMessageBox::warning( this, tr( "SpatiaLite" ), tr( "Cannot get SpatiaLite select dialog from provider." ) );
    return;
  }
  connect( dbs, SIGNAL( addDatabaseLayers( QStringList const &, QString const & ) ),
           this, SLOT( addDatabaseLayers( QStringList const &, QString const & ) ) );
  dbs->exec();
  delete dbs;
} // QgisApp::addSpatiaLiteLayer()

void QgisApp::addDelimitedTextLayer()
{
  if ( mMapCanvas && mMapCanvas->isDrawing() )
  {
    return;
  }

  // show the Delimited text dialog
  QDialog *dts = dynamic_cast<QDialog*>( QgsProviderRegistry::instance()->selectWidget( "delimitedtext", this ) );
  if ( !dts )
  {
    QMessageBox::warning( this, tr( "Delimited Text" ), tr( "Cannot get Delimited Text select dialog from provider." ) );
    return;
  }
  connect( dts, SIGNAL( addVectorLayer( QString, QString, QString ) ),
           this, SLOT( addSelectedVectorLayer( QString, QString, QString ) ) );
  dts->exec();
  delete dts;
} // QgisApp::addDelimitedTextLayer()

void QgisApp::addSelectedVectorLayer( QString uri, QString layerName, QString provider )
{
  addVectorLayer( uri, layerName, provider );
} // QgisApp:addSelectedVectorLayer

void QgisApp::addMssqlLayer()
{
#ifdef HAVE_MSSQL
  if ( mMapCanvas && mMapCanvas->isDrawing() )
  {
    return;
  }

  // show the MSSQL dialog
  QDialog *dbs = dynamic_cast<QDialog*>( QgsProviderRegistry::instance()->selectWidget( "mssql", this ) );
  if ( !dbs )
  {
    QMessageBox::warning( this, tr( "MSSQL" ), tr( "Cannot get MSSQL select dialog from provider." ) );
    return;
  }
  connect( dbs, SIGNAL( addDatabaseLayers( QStringList const &, QString const & ) ),
           this, SLOT( addDatabaseLayers( QStringList const &, QString const & ) ) );
  dbs->exec();
  delete dbs;
#endif
} // QgisApp::addMssqlLayer()

void QgisApp::addOracleLayer()
{
#ifdef HAVE_ORACLE
  if ( mMapCanvas && mMapCanvas->isDrawing() )
  {
    return;
  }

  // show the Oracle dialog
  QDialog *dbs = dynamic_cast<QDialog*>( QgsProviderRegistry::instance()->selectWidget( "oracle", this ) );
  if ( !dbs )
  {
    QMessageBox::warning( this, tr( "Oracle" ), tr( "Cannot get Oracle select dialog from provider." ) );
    return;
  }
  connect( dbs, SIGNAL( addDatabaseLayers( QStringList const &, QString const & ) ),
           this, SLOT( addDatabaseLayers( QStringList const &, QString const & ) ) );
  connect( dbs, SIGNAL( progress( int, int ) ),
           this, SLOT( showProgress( int, int ) ) );
  connect( dbs, SIGNAL( progressMessage( QString ) ),
           this, SLOT( showStatusMessage( QString ) ) );
  dbs->exec();
  delete dbs;
#endif
} // QgisApp::addOracleLayer()

void QgisApp::addWmsLayer()
{
  if ( mMapCanvas && mMapCanvas->isDrawing() )
  {
    return;
  }
  // Fudge for now
  QgsDebugMsg( "about to addRasterLayer" );

  // TODO: QDialog for now, switch to QWidget in future
  QDialog *wmss = dynamic_cast<QDialog*>( QgsProviderRegistry::instance()->selectWidget( QString( "wms" ), this ) );
  if ( !wmss )
  {
    QMessageBox::warning( this, tr( "WMS" ), tr( "Cannot get WMS select dialog from provider." ) );
    return;
  }
  connect( wmss, SIGNAL( addRasterLayer( QString const &, QString const &, QString const & ) ),
           this, SLOT( addRasterLayer( QString const &, QString const &, QString const & ) ) );
  wmss->exec();
  delete wmss;
}

void QgisApp::addWcsLayer()
{
  if ( mMapCanvas && mMapCanvas->isDrawing() )
  {
    return;
  }
  QgsDebugMsg( "about to addWcsLayer" );

  // TODO: QDialog for now, switch to QWidget in future
  QDialog *wcss = dynamic_cast<QDialog*>( QgsProviderRegistry::instance()->selectWidget( QString( "wcs" ), this ) );
  if ( !wcss )
  {
    QMessageBox::warning( this, tr( "WCS" ), tr( "Cannot get WCS select dialog from provider." ) );
    return;
  }
  connect( wcss, SIGNAL( addRasterLayer( QString const &, QString const &, QString const & ) ),
           this, SLOT( addRasterLayer( QString const &, QString const &, QString const & ) ) );
  wcss->exec();
  delete wcss;
}

void QgisApp::addWfsLayer()
{
  if ( !mMapCanvas )
  {
    return;
  }

  if ( mMapCanvas && mMapCanvas->isDrawing() )
  {
    return;
  }

  QgsDebugMsg( "about to addWfsLayer" );

  // TODO: QDialog for now, switch to QWidget in future
  QDialog *wfss = dynamic_cast<QDialog*>( QgsProviderRegistry::instance()->selectWidget( QString( "WFS" ), this ) );
  if ( !wfss )
  {
    QMessageBox::warning( this, tr( "WFS" ), tr( "Cannot get WFS select dialog from provider." ) );
    return;
  }
  connect( wfss, SIGNAL( addWfsLayer( QString, QString ) ),
           this, SLOT( addWfsLayer( QString, QString ) ) );

  //re-enable wfs with extent setting: pass canvas info to source select
  wfss->setProperty( "MapExtent", mMapCanvas->extent().toString() );
  if ( mMapCanvas->mapRenderer()->hasCrsTransformEnabled() )
  { //if "on the fly" reprojection is active, pass canvas CRS
    wfss->setProperty( "MapCRS", mMapCanvas->mapRenderer()->destinationCrs().authid() );
  }

  bool bkRenderFlag = mMapCanvas->renderFlag();
  mMapCanvas->setRenderFlag( false );
  wfss->exec();
  mMapCanvas->setRenderFlag( bkRenderFlag );
  delete wfss;
}

void QgisApp::addWfsLayer( QString uri, QString typeName )
{
  // TODO: this should be eventually moved to a more reasonable place
  addVectorLayer( uri, typeName, "WFS" );
}


void QgisApp::fileExit()
{
  if ( mMapCanvas && mMapCanvas->isDrawing() )
  {
    return;
  }

  if ( saveDirty() )
  {
    closeProject();
    qApp->exit( 0 );
  }
}


void QgisApp::fileNew()
{
  fileNew( true ); // prompts whether to save project
} // fileNew()


void QgisApp::fileNewBlank()
{
  fileNew( true, true );
}


//as file new but accepts flags to indicate whether we should prompt to save
void QgisApp::fileNew( bool thePromptToSaveFlag, bool forceBlank )
{
  if ( mMapCanvas && mMapCanvas->isDrawing() )
  {
    return;
  }

  if ( thePromptToSaveFlag )
  {
    if ( !saveDirty() )
    {
      return; //cancel pressed
    }
  }

  QSettings settings;

  closeProject();
  mMapCanvas->clear();

  QgsProject* prj = QgsProject::instance();
  prj->title( QString::null );
  prj->setFileName( QString::null );
  prj->clearProperties(); // why carry over properties from previous projects?

  //set the color for selections
  //the default can be set in qgisoptions
  //use project properties to override the color on a per project basis
  int myRed = settings.value( "/qgis/default_selection_color_red", 255 ).toInt();
  int myGreen = settings.value( "/qgis/default_selection_color_green", 255 ).toInt();
  int myBlue = settings.value( "/qgis/default_selection_color_blue", 0 ).toInt();
  int myAlpha = settings.value( "/qgis/default_selection_color_alpha", 255 ).toInt();
  prj->writeEntry( "Gui", "/SelectionColorRedPart", myRed );
  prj->writeEntry( "Gui", "/SelectionColorGreenPart", myGreen );
  prj->writeEntry( "Gui", "/SelectionColorBluePart", myBlue );
  prj->writeEntry( "Gui", "/SelectionColorAlphaPart", myAlpha );

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
  mScaleEdit->updateScales();

  // set project CRS
  QgsMapRenderer* myRenderer = mMapCanvas->mapRenderer();
  QString defCrs = settings.value( "/Projections/projectDefaultCrs", GEO_EPSG_CRS_AUTHID ).toString();
  QgsCoordinateReferenceSystem srs;
  srs.createFromOgcWmsCrs( defCrs );
  myRenderer->setDestinationCrs( srs );
  // write the projections _proj string_ to project settings
  prj->writeEntry( "SpatialRefSys", "/ProjectCrs", defCrs );
  prj->dirty( false );
  if ( srs.mapUnits() != QGis::UnknownUnit )
  {
    myRenderer->setMapUnits( srs.mapUnits() );
  }

  // enable OTF CRS transformation if necessary
  myRenderer->setProjectionsEnabled( settings.value( "/Projections/otfTransformEnabled", 0 ).toBool() );

  updateCRSStatusBar();

  /** New Empty Project Created
      (before attempting to load custom project templates/filepaths) */

  // load default template
  /* NOTE: don't open default template on launch until after initialization,
           in case a project was defined via command line */

  // don't open template if last auto-opening of a project failed
  if ( ! forceBlank )
  {
    forceBlank = ! settings.value( "/qgis/projOpenedOKAtLaunch", QVariant( true ) ).toBool();
  }

  if ( ! forceBlank && settings.value( "/qgis/newProjectDefault", QVariant( false ) ).toBool() )
  {
    fileNewFromDefaultTemplate();
  }

  // set the initial map tool
#ifndef HAVE_TOUCH
  mMapCanvas->setMapTool( mMapTools.mPan );
  mNonEditMapTool = mMapTools.mPan;  // signals are not yet setup to catch this
#else
  mMapCanvas->setMapTool( mMapTools.mTouch );
  mNonEditMapTool = mMapTools.mTouch;  // signals are not yet setup to catch this
#endif

} // QgisApp::fileNew(bool thePromptToSaveFlag)

bool QgisApp::fileNewFromTemplate( QString fileName )
{
  QgsDebugMsg( QString( "loading project template: %1" ).arg( fileName ) );
  if ( addProject( fileName ) )
  {
    // set null filename so we don't override the template
    QgsProject::instance()->setFileName( QString() );
    return true;
  }
  return false;
}

void QgisApp::fileNewFromDefaultTemplate()
{
  QString projectTemplate = QgsApplication::qgisSettingsDirPath() + QString( "project_default.qgs" );
  QString msgTxt;
  if ( !projectTemplate.isEmpty() && QFile::exists( projectTemplate ) )
  {
    if ( fileNewFromTemplate( projectTemplate ) )
    {
      return;
    }
    msgTxt = tr( "Default failed to open: %1" );
  }
  else
  {
    msgTxt = tr( "Default not found: %1" );
  }
  messageBar()->pushMessage( tr( "Open Template Project" ),
                             msgTxt.arg( projectTemplate ),
                             QgsMessageBar::WARNING );
}

void QgisApp::fileOpenAfterLaunch()
{
  // TODO: move auto-open project options to enums

  // check if a project is already loaded via command line or filesystem
  if ( !QgsProject::instance()->fileName().isNull() )
  {
    return;
  }

  // check if a data source is already loaded via command line or filesystem
  // empty project with layer loaded, but may not trigger a dirty project at this point
  if ( QgsProject::instance() && QgsMapLayerRegistry::instance()->count() > 0 )
  {
    return;
  }

  // fileNewBlank() has already been called in QgisApp constructor
  // loaded project is either a new blank one, or one from command line/filesystem
  QSettings settings;
  QString autoOpenMsgTitle = tr( "Auto-open Project" );

  // what type of project to auto-open
  int projOpen = settings.value( "/qgis/projOpenAtLaunch", 0 ).toInt();

  // get path of project file to open, or was attempted
  QString projPath = QString();
  if ( projOpen == 1 && mRecentProjectPaths.size() > 0 ) // most recent project
  {
    projPath = mRecentProjectPaths.at( 0 );
  }
  if ( projOpen == 2 ) // specific project
  {
    projPath = settings.value( "/qgis/projOpenAtLaunchPath" ).toString();
  }

  // whether last auto-opening of a project failed
  bool projOpenedOK = settings.value( "/qgis/projOpenedOKAtLaunch", QVariant( true ) ).toBool();

  // notify user if last attempt at auto-opening a project failed
  /** NOTE: Notification will not show if last auto-opened project failed but
      next project opened is from command line (minor issue) */
  /** TODO: Keep projOpenedOKAtLaunch from being reset to true after
      reading command line project (which happens before initialization signal) */
  if ( !projOpenedOK )
  {
    // only show the following 'auto-open project failed' message once, at launch
    settings.setValue( "/qgis/projOpenedOKAtLaunch", QVariant( true ) );

    // set auto-open project back to 'New' to avoid re-opening bad project
    settings.setValue( "/qgis/projOpenAtLaunch" , QVariant( 0 ) );

    messageBar()->pushMessage( autoOpenMsgTitle,
                               tr( "Failed to open: %1" ).arg( projPath ),
                               QgsMessageBar::CRITICAL );
    return;
  }

  if ( projOpen == 0 ) // new project (default)
  {
    // open default template, if defined
    if ( settings.value( "/qgis/newProjectDefault", QVariant( false ) ).toBool() )
    {
      fileNewFromDefaultTemplate();
    }
    return;
  }

  if ( projPath.isEmpty() ) // projPath required from here
  {
    return;
  }

  if ( !projPath.endsWith( QString( "qgs" ), Qt::CaseInsensitive ) )
  {
    messageBar()->pushMessage( autoOpenMsgTitle,
                               tr( "Not valid project file: %1" ).arg( projPath ),
                               QgsMessageBar::WARNING );
    return;
  }

  if ( QFile::exists( projPath ) )
  {
    // set flag to check on next app launch if the following project opened OK
    settings.setValue( "/qgis/projOpenedOKAtLaunch" , QVariant( false ) );

    if ( !addProject( projPath ) )
    {
      messageBar()->pushMessage( autoOpenMsgTitle,
                                 tr( "Project failed to open: %1" ).arg( projPath ),
                                 QgsMessageBar::WARNING );
    }

    if ( projPath.endsWith( QString( "project_default.qgs" ) ) )
    {
      messageBar()->pushMessage( autoOpenMsgTitle,
                                 tr( "Default template has been reopened: %1" ).arg( projPath ),
                                 QgsMessageBar::INFO );
    }
  }
  else
  {
    messageBar()->pushMessage( autoOpenMsgTitle,
                               tr( "File not found: %1" ).arg( projPath ),
                               QgsMessageBar::WARNING );
  }
}

void QgisApp::fileOpenedOKAfterLaunch()
{
  QSettings settings;
  settings.setValue( "/qgis/projOpenedOKAtLaunch" , QVariant( true ) );
}

void QgisApp::fileNewFromTemplateAction( QAction * qAction )
{
  if ( ! qAction )
    return;

  if ( qAction->text() == tr( "< Blank >" ) )
  {
    fileNewBlank();
  }
  else
  {
    QSettings settings;
    QString templateDirName = settings.value( "/qgis/projectTemplateDir",
                              QgsApplication::qgisSettingsDirPath() + "project_templates" ).toString();
    fileNewFromTemplate( templateDirName + QDir::separator() + qAction->text() );
  }
}


void QgisApp::newVectorLayer()
{
  if ( mMapCanvas && mMapCanvas->isDrawing() )
  {
    return;
  }

  QString enc;
  QString fileName = QgsNewVectorLayerDialog::runAndCreateLayer( this, &enc );

  if ( !fileName.isEmpty() )
  {
    //then add the layer to the view
    QStringList fileNames;
    fileNames.append( fileName );
    //todo: the last parameter will change accordingly to layer type
    addVectorLayers( fileNames, enc, "file" );
  }
}

void QgisApp::newSpatialiteLayer()
{
  if ( mMapCanvas && mMapCanvas->isDrawing() )
  {
    return;
  }
  QgsNewSpatialiteLayerDialog spatialiteDialog( this );
  spatialiteDialog.exec();
}

void QgisApp::showRasterCalculator()
{
  QgsRasterCalcDialog d( this );
  if ( d.exec() == QDialog::Accepted )
  {
    //invoke analysis library
    //extent and output resolution will come later...
    QgsRasterCalculator rc( d.formulaString(), d.outputFile(), d.outputFormat(), d.outputRectangle(), d.numberOfColumns(), d.numberOfRows(), d.rasterEntries() );

    QProgressDialog p( tr( "Calculating..." ), tr( "Abort..." ), 0, 0 );
    p.setWindowModality( Qt::WindowModal );
    if ( rc.processCalculation( &p ) == 0 )
    {
      if ( d.addLayerToProject() )
      {
        addRasterLayer( d.outputFile(), QFileInfo( d.outputFile() ).baseName() );
      }
    }
  }
}

void QgisApp::fileOpen()
{
  if ( mMapCanvas && mMapCanvas->isDrawing() )
  {
    return;
  }

  // possibly save any pending work before opening a new project
  if ( saveDirty() )
  {
    // Retrieve last used project dir from persistent settings
    QSettings settings;
    QString lastUsedDir = settings.value( "/UI/lastProjectDir", "." ).toString();
    QString fullPath = QFileDialog::getOpenFileName( this,
                       tr( "Choose a QGIS project file to open" ),
                       lastUsedDir,
                       tr( "QGIS files" ) + " (*.qgs *.QGS)" );
    if ( fullPath.isNull() )
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

    // open the selected project
    addProject( fullPath );
  }
} // QgisApp::fileOpen

void QgisApp::enableProjectMacros()
{
  mTrustedMacros = true;

  // load macros
  QgsPythonRunner::run( "qgis.utils.reloadProjectMacros()" );
}


/**
  adds a saved project to qgis, usually called on startup by specifying a
  project file on the command line
  */
bool QgisApp::addProject( QString projectFile )
{
  QFileInfo pfi( projectFile );
  statusBar()->showMessage( tr( "Loading project: %1" ).arg( pfi.fileName() ) );
  qApp->processEvents();

  QApplication::setOverrideCursor( Qt::WaitCursor );

  // close the previous opened project if any
  closeProject();

  if ( ! QgsProject::instance()->read( projectFile ) )
  {
    QApplication::restoreOverrideCursor();
    statusBar()->clearMessage();

    QMessageBox::critical( this,
                           tr( "Unable to open project" ),
                           QgsProject::instance()->error() );


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

  //load project scales
  bool projectScales = QgsProject::instance()->readBoolEntry( "Scales", "/useProjectScales" );
  if ( projectScales )
  {
    mScaleEdit->updateScales( QgsProject::instance()->readListEntry( "Scales", "/ScalesList" ) );
  }

  mMapCanvas->updateScale();
  QgsDebugMsg( "Scale restored..." );

  QSettings settings;

  // does the project have any macros?
  if ( mPythonUtils && mPythonUtils->isEnabled() )
  {
    if ( !QgsProject::instance()->readEntry( "Macros", "/pythonCode", QString::null ).isEmpty() )
    {
      int enableMacros = settings.value( "/qgis/enableMacros", 1 ).toInt();
      // 0 = never, 1 = ask, 2 = just for this session, 3 = always

      if ( enableMacros == 3 || enableMacros == 2 )
      {
        enableProjectMacros();
      }
      else if ( enableMacros == 1 ) // ask
      {
        // create the notification widget for macros

        QWidget *macroMsg = QgsMessageBar::createMessage( tr( "Security warning" ),
                            tr( "project macros have been disabled." ),
                            QgsApplication::getThemeIcon( "/mIconWarn.png" ),
                            mInfoBar );

        QToolButton *btnEnableMacros = new QToolButton( macroMsg );
        btnEnableMacros->setText( tr( "Enable macros" ) );
        btnEnableMacros->setStyleSheet( "background-color: rgba(255, 255, 255, 0); color: black; text-decoration: underline;" );
        btnEnableMacros->setCursor( Qt::PointingHandCursor );
        btnEnableMacros->setSizePolicy( QSizePolicy::Maximum, QSizePolicy::Preferred );
        connect( btnEnableMacros, SIGNAL( clicked() ), mInfoBar, SLOT( popWidget() ) );
        connect( btnEnableMacros, SIGNAL( clicked() ), this, SLOT( enableProjectMacros() ) );
        macroMsg->layout()->addWidget( btnEnableMacros );

        // display the macros notification widget
        mInfoBar->pushWidget( macroMsg, QgsMessageBar::WARNING );
      }
    }
  }

  // load PAL engine settings
  mLBL->loadEngineSettings();

  emit projectRead(); // let plug-ins know that we've read in a new
  // project so that they can check any project
  // specific plug-in state

  // add this to the list of recently used project files
  saveRecentProjectPath( projectFile, settings );

  QApplication::restoreOverrideCursor();

  mMapCanvas->freeze( false );
  mMapCanvas->refresh();

  statusBar()->showMessage( tr( "Project loaded" ), 3000 );
  return true;
} // QgisApp::addProject(QString projectFile)



bool QgisApp::fileSave()
{
  if ( mMapCanvas && mMapCanvas->isDrawing() )
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

  if ( QgsProject::instance()->fileName().isNull() )
  {
    isNewProject = true;

    // Retrieve last used project dir from persistent settings
    QSettings settings;
    QString lastUsedDir = settings.value( "/UI/lastProjectDir", "." ).toString();

    QString path = QFileDialog::getSaveFileName(
                     this,
                     tr( "Choose a QGIS project file" ),
                     lastUsedDir + "/" + QgsProject::instance()->title(),
                     tr( "QGIS files" ) + " (*.qgs *.QGS)" );
    if ( path.isEmpty() )
      return false;

    fullPath.setFile( path );

    // make sure we have the .qgs extension in the file name
    if ( "qgs" != fullPath.suffix().toLower() )
    {
      fullPath.setFile( fullPath.filePath() + ".qgs" );
    }


    QgsProject::instance()->setFileName( fullPath.filePath() );
  }

  if ( QgsProject::instance()->write() )
  {
    setTitleBarText_( *this ); // update title bar
    statusBar()->showMessage( tr( "Saved project to: %1" ).arg( QgsProject::instance()->fileName() ), 5000 );

    if ( isNewProject )
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

  // run the saved project macro
  if ( mTrustedMacros )
  {
    QgsPythonRunner::run( "qgis.utils.saveProjectMacro();" );
  }

  return true;
} // QgisApp::fileSave

void QgisApp::fileSaveAs()
{
  if ( mMapCanvas && mMapCanvas->isDrawing() )
  {
    return;
  }

  // Retrieve last used project dir from persistent settings
  QSettings settings;
  QString lastUsedDir = settings.value( "/UI/lastProjectDir", "." ).toString();

  QString path = QFileDialog::getSaveFileName( this,
                 tr( "Choose a file name to save the QGIS project file as" ),
                 lastUsedDir + "/" + QgsProject::instance()->title(),
                 tr( "QGIS files" ) + " (*.qgs *.QGS)" );
  if ( path.isEmpty() )
    return;

  QFileInfo fullPath( path );

  settings.setValue( "/UI/lastProjectDir", fullPath.path() );

  // make sure the .qgs extension is included in the path name. if not, add it...
  if ( "qgs" != fullPath.suffix().toLower() )
  {
    fullPath.setFile( fullPath.filePath() + ".qgs" );
  }

  QgsProject::instance()->setFileName( fullPath.filePath() );

  if ( QgsProject::instance()->write() )
  {
    setTitleBarText_( *this ); // update title bar
    statusBar()->showMessage( tr( "Saved project to: %1" ).arg( QgsProject::instance()->fileName() ), 5000 );
    // add this to the list of recently used project files
    saveRecentProjectPath( fullPath.filePath(), settings );
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

  if ( saveDirty() )
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
  if ( saveDirty() )
  {
    // error handling and reporting is in addProject() function
    addProject( fileName );
  }
  return;
}

/**
  Open a raster or vector file; ignore other files.
  Used to process a commandline argument or OpenDocument AppleEvent.
  @returns true if the file is successfully opened
  */
bool QgisApp::openLayer( const QString & fileName, bool allowInteractive )
{
  QFileInfo fileInfo( fileName );
  bool ok( false );

  CPLPushErrorHandler( CPLQuietErrorHandler );

  // if needed prompt for zipitem layers
  QString vsiPrefix = QgsZipItem::vsiPrefix( fileName );
  if ( vsiPrefix == "/vsizip/" || vsiPrefix == "/vsitar/" )
  {
    if ( askUserForZipItemLayers( fileName ) )
    {
      CPLPopErrorHandler();
      return true;
    }
  }

  // try to load it as raster
  if ( QgsRasterLayer::isValidRasterFileName( fileName ) )
  {
    ok  = addRasterLayer( fileName, fileInfo.completeBaseName() );
  }
  // TODO - should we really call isValidRasterFileName() before addRasterLayer()
  //        this results in 2 calls to GDALOpen()
  // I think (Radim) that it is better to test only first if valid,
  // addRasterLayer() is really trying to add layer and gives error if fails
  //
  // if ( addRasterLayer( fileName, fileInfo.completeBaseName() ) )
  // {
  //   ok  = true );
  // }
  else // nope - try to load it as a shape/ogr
  {
    if ( allowInteractive )
    {
      ok = addVectorLayers( QStringList( fileName ), "System", "file" );
    }
    else
    {
      ok = addVectorLayer( fileName, fileInfo.completeBaseName(), "ogr" );
    }
  }

  CPLPopErrorHandler();

  if ( !ok )
  {
    // we have no idea what this file is...
    QgsMessageLog::logMessage( tr( "Unable to load %1" ).arg( fileName ) );
  }

  return ok;
}


// Open a file specified by a commandline argument, Drop or FileOpen event.
void QgisApp::openFile( const QString & fileName )
{
  // check to see if we are opening a project file
  QFileInfo fi( fileName );
  if ( fi.completeSuffix() == "qgs" )
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


void QgisApp::newPrintComposer()
{
  if ( mMapCanvas && mMapCanvas->isDrawing() )
  {
    return;
  }

  QString title = uniqueComposerTitle( this, true );
  if ( title.isNull() )
  {
    return;
  }
  createNewComposer( title );
}

void QgisApp::showComposerManager()
{
  QgsComposerManager m( this );
  m.exec();
}

void QgisApp::saveMapAsImage()
{
  QPair< QString, QString> myFileNameAndFilter = QgisGui::getSaveAsImageName( this, tr( "Choose a file name to save the map image as" ) );
  if ( myFileNameAndFilter.first != "" )
  {
    //save the mapview to the selected file
    mMapCanvas->saveAsImage( myFileNameAndFilter.first, NULL, myFileNameAndFilter.second );
    statusBar()->showMessage( tr( "Saved map image to %1" ).arg( myFileNameAndFilter.first ) );
  }

} // saveMapAsImage

//overloaded version of the above function
void QgisApp::saveMapAsImage( QString theImageFileNameQString, QPixmap * theQPixmap )
{
  if ( theImageFileNameQString == "" )
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
  if ( mMapLegend )
  {
    mMapLegend->enableOverviewModeAllLayers( true );
  }

  markDirty();
}

//reimplements method from base (gui) class
void QgisApp::removeAllFromOverview()
{
  if ( mMapLegend )
  {
    mMapLegend->enableOverviewModeAllLayers( false );
  }

  markDirty();
}

void QgisApp::toggleFullScreen()
{
  if ( mFullScreenMode )
  {
    if ( mPrevScreenModeMaximized )
    {
      // Change to maximized state. Just calling showMaximized() results in
      // the window going to the normal state. Calling showNormal() then
      // showMaxmized() is a work-around. Turn off rendering for this as it
      // would otherwise cause two re-renders of the map, which can take a
      // long time.
      bool renderFlag = mapCanvas()->renderFlag();
      if ( renderFlag )
        mapCanvas()->setRenderFlag( false );
      showNormal();
      showMaximized();
      if ( renderFlag )
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
    if ( isMaximized() )
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
  if ( window )
  {
    window->showMinimized();
  }
}

void QgisApp::toggleActiveWindowMaximized()
{
  QWidget *window = QApplication::activeWindow();
  if ( window )
  {
    if ( window->isMaximized() )
      window->showNormal();
    else
      window->showMaximized();
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
#else
  Q_UNUSED( action );
#endif
}

void QgisApp::removeWindow( QAction *action )
{
#ifdef Q_WS_MAC
  mWindowActions->removeAction( action );
  mWindowMenu->removeAction( action );
#else
  Q_UNUSED( action );
#endif
}

void QgisApp::stopRendering()
{
  if ( mMapCanvas )
  {
    QgsMapRenderer* mypMapRenderer = mMapCanvas->mapRenderer();
    if ( mypMapRenderer )
    {
      QgsRenderContext* mypRenderContext = mypMapRenderer->rendererContext();
      if ( mypRenderContext )
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

  legend()->setLayersVisible( false );
}


// reimplements method from base (gui) class
void QgisApp::showAllLayers()
{
  QgsDebugMsg( "Showing all layers!" );

  legend()->setLayersVisible( true );
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

void QgisApp::panToSelected()
{
  mMapCanvas->panToSelected();
}

void QgisApp::pan()
{
  mMapCanvas->setMapTool( mMapTools.mPan );
}

#ifdef HAVE_TOUCH
void QgisApp::touch()
{
  mMapCanvas->setMapTool( mMapTools.mTouch );
}
#endif

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

void QgisApp::doFeatureAction()
{
  mMapCanvas->setMapTool( mMapTools.mFeatureAction );
}

void QgisApp::updateDefaultFeatureAction( QAction *action )
{
  QgsVectorLayer *vlayer = qobject_cast<QgsVectorLayer *>( activeLayer() );
  if ( !vlayer )
    return;

  mFeatureActionMenu->setActiveAction( action );

  int index = mFeatureActionMenu->actions().indexOf( action );
  vlayer->actions()->setDefaultAction( index );

  doFeatureAction();
}

void QgisApp::refreshFeatureActions()
{
  mFeatureActionMenu->clear();

  QgsVectorLayer *vlayer = qobject_cast<QgsVectorLayer *>( activeLayer() );
  if ( !vlayer )
    return;

  QgsAttributeAction *actions = vlayer->actions();
  for ( int i = 0; i < actions->size(); i++ )
  {
    QAction *action = mFeatureActionMenu->addAction( actions->at( i ).name() );
    if ( i == actions->defaultAction() )
    {
      mFeatureActionMenu->setActiveAction( action );
    }
  }
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

void QgisApp::addHtmlAnnotation()
{
  mMapCanvas->setMapTool( mMapTools.mHtmlAnnotation );
}

void QgisApp::addTextAnnotation()
{
  mMapCanvas->setMapTool( mMapTools.mTextAnnotation );
}

void QgisApp::addSvgAnnotation()
{
  mMapCanvas->setMapTool( mMapTools.mSvgAnnotation );
}

void QgisApp::modifyAnnotation()
{
  mMapCanvas->setMapTool( mMapTools.mAnnotation );
}

void QgisApp::labelingFontNotFound( QgsVectorLayer* vlayer, const QString& fontfamily )
{
  // TODO: update when pref for how to resolve missing family (use matching algorithm or just default font) is implemented
  QString substitute = tr( "Default system font substituted." );

  QWidget* fontMsg = QgsMessageBar::createMessage(
                       tr( "Labeling" ),
                       tr( "Font for layer <b><u>%1</u></b> was not found (<i>%2</i>). %3" ).arg( vlayer->name() ).arg( fontfamily ).arg( substitute ),
                       QgsApplication::getThemeIcon( "/mIconWarn.png" ),
                       messageBar() );

  QToolButton* btnOpenPrefs = new QToolButton( fontMsg );
  btnOpenPrefs->setStyleSheet( "QToolButton{ background-color: rgba(255, 255, 255, 0); color: black; text-decoration: underline; }" );
  btnOpenPrefs->setCursor( Qt::PointingHandCursor );
  btnOpenPrefs->setSizePolicy( QSizePolicy::Maximum, QSizePolicy::Preferred );
  btnOpenPrefs->setToolButtonStyle( Qt::ToolButtonTextOnly );

  // store pointer to vlayer in data of QAction
  QAction* act = new QAction( fontMsg );
  act->setData( QVariant( QMetaType::QObjectStar, &vlayer ) );
  act->setText( tr( "Open labeling dialog" ) );
  btnOpenPrefs->addAction( act );
  btnOpenPrefs->setDefaultAction( act );
  btnOpenPrefs->setToolTip( "" );

  connect( btnOpenPrefs, SIGNAL( triggered( QAction* ) ), this, SLOT( labelingDialogFontNotFound( QAction* ) ) );
  fontMsg->layout()->addWidget( btnOpenPrefs );

  // no timeout set, since notice needs attention and is only shown first time layer is labeled
  messageBar()->pushWidget( fontMsg, QgsMessageBar::WARNING );
}

void QgisApp::labelingDialogFontNotFound( QAction* act )
{
  if ( !act )
  {
    return;
  }

  // get base pointer to layer
  QObject* obj = qvariant_cast<QObject*>( act->data() );

  // remove calling messagebar widget
  messageBar()->popWidget();

  if ( !obj )
  {
    return;
  }

  QgsMapLayer* layer = qobject_cast<QgsMapLayer*>( obj );
  if ( layer && setActiveLayer( layer ) )
  {
    labeling();
  }
}

void QgisApp::labeling()
{
  QgsVectorLayer *vlayer = qobject_cast<QgsVectorLayer*>( activeLayer() );
  if ( !vlayer )
  {
    messageBar()->pushMessage( tr( "Labeling Options" ),
                               tr( "Please select a vector layer first" ),
                               QgsMessageBar::INFO,
                               messageTimeout() );
    return;
  }


  QDialog *dlg = new QDialog( this );
  dlg->setWindowTitle( tr( "Layer labeling settings" ) );
  QgsLabelingGui *labelingGui = new QgsLabelingGui( mLBL, vlayer, mMapCanvas, dlg );
  labelingGui->init(); // load QgsPalLayerSettings for layer
  labelingGui->layout()->setContentsMargins( 0, 0, 0, 0 );
  QVBoxLayout *layout = new QVBoxLayout( dlg );
  layout->addWidget( labelingGui );

  QDialogButtonBox *buttonBox = new QDialogButtonBox( QDialogButtonBox::Ok | QDialogButtonBox::Cancel | QDialogButtonBox::Apply, Qt::Horizontal, dlg );
  layout->addWidget( buttonBox );

  dlg->setLayout( layout );

  QSettings settings;
  dlg->restoreGeometry( settings.value( "/Windows/Labeling/geometry" ).toByteArray() );

  connect( buttonBox->button( QDialogButtonBox::Ok ), SIGNAL( clicked() ), dlg, SLOT( accept() ) );
  connect( buttonBox->button( QDialogButtonBox::Cancel ), SIGNAL( clicked() ), dlg, SLOT( reject() ) );
  connect( buttonBox->button( QDialogButtonBox::Apply ), SIGNAL( clicked() ), labelingGui, SLOT( apply() ) );

  if ( dlg->exec() )
  {
    labelingGui->apply();

    settings.setValue( "/Windows/Labeling/geometry", dlg->saveGeometry() );

    // alter labeling - save the changes
    labelingGui->layerSettings().writeToLayer( vlayer );

    // trigger refresh
    if ( mMapCanvas )
    {
      mMapCanvas->refresh();
    }
  }

  delete dlg;

  activateDeactivateLayerRelatedActions( vlayer );
}

void QgisApp::fieldCalculator()
{
  if ( mMapCanvas && mMapCanvas->isDrawing() )
  {
    return;
  }

  QgsVectorLayer *myLayer = qobject_cast<QgsVectorLayer *>( activeLayer() );
  if ( !myLayer )
  {
    return;
  }

  QgsFieldCalculator calc( myLayer );
  if ( calc.exec() )
  {
    mMapCanvas->refresh();
  }
}

void QgisApp::attributeTable()
{
  if ( mMapCanvas && mMapCanvas->isDrawing() )
  {
    return;
  }

  QgsVectorLayer *myLayer = qobject_cast<QgsVectorLayer *>( activeLayer() );
  if ( !myLayer )
  {
    return;
  }

  QgsAttributeTableDialog *mDialog = new QgsAttributeTableDialog( myLayer );
  mDialog->show();
  // the dialog will be deleted by itself on close
}

void QgisApp::saveAsRasterFile()
{
  QgsRasterLayer* rasterLayer = qobject_cast<QgsRasterLayer *>( activeLayer() );
  if ( !rasterLayer )
  {
    return;
  }

  QgsRasterLayerSaveAsDialog d( rasterLayer, rasterLayer->dataProvider(),
                                mMapCanvas->extent(), rasterLayer->crs(),
                                mMapCanvas->mapRenderer()->destinationCrs(),
                                this );
  if ( d.exec() == QDialog::Accepted )
  {
    QgsRasterFileWriter fileWriter( d.outputFileName() );
    if ( d.tileMode() )
    {
      fileWriter.setTiledMode( true );
      fileWriter.setMaxTileWidth( d.maximumTileSizeX() );
      fileWriter.setMaxTileHeight( d.maximumTileSizeY() );
    }

    QProgressDialog pd( 0, tr( "Abort..." ), 0, 0 );
    // Show the dialo immediately because cloning pipe can take some time (WCS)
    pd.setLabelText( tr( "Reading raster" ) );
    pd.show();
    pd.setWindowModality( Qt::WindowModal );

    // TODO: show error dialogs
    // TODO: this code should go somewhere else, but probably not into QgsRasterFileWriter
    // clone pipe/provider is not really necessary, ready for threads
    QgsRasterPipe* pipe = 0;

    if ( d.mode() == QgsRasterLayerSaveAsDialog::RawDataMode )
    {
      QgsDebugMsg( "Writing raw data" );
      //QgsDebugMsg( QString( "Writing raw data" ).arg( pos ) );
      pipe = new QgsRasterPipe();
      if ( !pipe->set( rasterLayer->dataProvider()->clone() ) )
      {
        QgsDebugMsg( "Cannot set pipe provider" );
        return;
      }

      QgsRasterNuller *nuller = new QgsRasterNuller();
      for ( int band = 1; band <= rasterLayer->dataProvider()->bandCount(); band ++ )
      {
        nuller->setNoData( band, d.noData() );
      }
      if ( !pipe->insert( 1, nuller ) )
      {
        QgsDebugMsg( "Cannot set pipe nuller" );
        return;
      }

      // add projector if necessary
      if ( d.outputCrs() != rasterLayer->crs() )
      {
        QgsRasterProjector * projector = new QgsRasterProjector;
        projector->setCRS( rasterLayer->crs(), d.outputCrs() );
        if ( !pipe->insert( 2, projector ) )
        {
          QgsDebugMsg( "Cannot set pipe projector" );
          return;
        }
      }
    }
    else // RenderedImageMode
    {
      // clone the whole pipe
      QgsDebugMsg( "Writing rendered image" );
      pipe = new QgsRasterPipe( *rasterLayer->pipe() );
      QgsRasterProjector *projector = pipe->projector();
      if ( !projector )
      {
        QgsDebugMsg( "Cannot get pipe projector" );
        delete pipe;
        return;
      }
      projector->setCRS( rasterLayer->crs(), d.outputCrs() );
    }

    if ( !pipe->last() )
    {
      delete pipe;
      return;
    }
    fileWriter.setCreateOptions( d.createOptions() );

    fileWriter.setBuildPyramidsFlag( d.buildPyramidsFlag() );
    fileWriter.setPyramidsList( d.pyramidsList() );
    fileWriter.setPyramidsResampling( d.pyramidsResamplingMethod() );
    fileWriter.setPyramidsFormat( d.pyramidsFormat() );
    fileWriter.setPyramidsConfigOptions( d.pyramidsConfigOptions() );

    QgsRasterFileWriter::WriterError err = fileWriter.writeRaster( pipe, d.nColumns(), d.nRows(), d.outputRectangle(), d.outputCrs(), &pd );
    if ( err != QgsRasterFileWriter::NoError )
    {
      QMessageBox::warning( this, tr( "Error" ),
                            tr( "Cannot write raster error code: %1" ).arg( err ),
                            QMessageBox::Ok );

    }
    delete pipe;
  }
}

void QgisApp::saveAsFile()
{
  QgsMapLayer* layer = activeLayer();
  if ( !layer )
    return;

  QgsMapLayer::LayerType layerType = layer->type();
  if ( layerType == QgsMapLayer::RasterLayer )
  {
    saveAsRasterFile();
  }
  else if ( layerType == QgsMapLayer::VectorLayer )
  {
    saveAsVectorFileGeneral( false );
  }
}

void QgisApp::saveSelectionAsVectorFile()
{
  saveAsVectorFileGeneral( true );
}

void QgisApp::saveAsVectorFileGeneral( bool saveOnlySelection )
{
  if ( mMapCanvas && mMapCanvas->isDrawing() )
    return;

  if ( !mMapLegend )
    return;

  QgsVectorLayer *vlayer = qobject_cast<QgsVectorLayer *>( activeLayer() ); // FIXME: output of multiple layers at once?
  if ( !vlayer )
    return;

  QgsCoordinateReferenceSystem destCRS;

  QgsVectorLayerSaveAsDialog *dialog = new QgsVectorLayerSaveAsDialog( vlayer->crs().srsid(), this );

  if ( dialog->exec() == QDialog::Accepted )
  {
    QString encoding = dialog->encoding();
    QString vectorFilename = dialog->filename();
    QString format = dialog->format();
    QStringList datasourceOptions = dialog->datasourceOptions();

    switch ( dialog->crs() )
    {
      case -2: // Project CRS
        destCRS = mMapCanvas->mapRenderer()->destinationCrs();
        break;
      case -1: // Layer CRS
        destCRS = vlayer->crs();
        break;

      default: // Selected CRS
        destCRS = QgsCoordinateReferenceSystem( dialog->crs(), QgsCoordinateReferenceSystem::InternalCrsId );
        break;
    }

    // ok if the file existed it should be deleted now so we can continue...
    QApplication::setOverrideCursor( Qt::WaitCursor );

    QgsVectorFileWriter::WriterError error;
    QString errorMessage;
    QString newFilename;
    error = QgsVectorFileWriter::writeAsVectorFormat(
              vlayer, vectorFilename, encoding, &destCRS, format,
              saveOnlySelection,
              &errorMessage,
              datasourceOptions, dialog->layerOptions(),
              dialog->skipAttributeCreation(),
              &newFilename,
              ( QgsVectorFileWriter::SymbologyExport )( dialog->symbologyExport() ),
              dialog->scaleDenominator() );

    QApplication::restoreOverrideCursor();

    if ( error == QgsVectorFileWriter::NoError )
    {
      if ( dialog->addToCanvas() )
      {
        addVectorLayers( QStringList( newFilename ), encoding, "file" );
      }
      messageBar()->pushMessage( tr( "Saving done" ),
                                 tr( "Export to vector file has been completed" ),
                                 QgsMessageBar::INFO, messageTimeout() );
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
  if ( mMapCanvas && mMapCanvas->isDrawing() )
    return;

  showLayerProperties( activeLayer() );
}

void QgisApp::deleteSelected( QgsMapLayer *layer, QWidget* parent )
{
  if ( !layer )
  {
    layer = mMapLegend->currentLayer();
  }

  if ( !parent )
  {
    parent = this;
  }

  if ( !layer )
  {
    messageBar()->pushMessage( tr( "No Layer Selected" ),
                               tr( "To delete features, you must select a vector layer in the legend" ),
                               QgsMessageBar::INFO, messageTimeout() );
    return;
  }

  QgsVectorLayer* vlayer = qobject_cast<QgsVectorLayer *>( layer );
  if ( !vlayer )
  {
    messageBar()->pushMessage( tr( "No Vector Layer Selected" ),
                               tr( "Deleting features only works on vector layers" ),
                               QgsMessageBar::INFO, messageTimeout() );
    return;
  }

  if ( !( vlayer->dataProvider()->capabilities() & QgsVectorDataProvider::DeleteFeatures ) )
  {
    messageBar()->pushMessage( tr( "Provider does not support deletion" ),
                               tr( "Data provider does not support deleting features" ),
                               QgsMessageBar::INFO, messageTimeout() );
    return;
  }

  if ( !vlayer->isEditable() )
  {
    messageBar()->pushMessage( tr( "Layer not editable" ),
                               tr( "The current layer is not editable. Choose 'Start editing' in the digitizing toolbar." ),
                               QgsMessageBar::INFO, messageTimeout() );
    return;
  }

  //display a warning
  int numberOfDeletedFeatures = vlayer->selectedFeaturesIds().size();
  if ( QMessageBox::warning( parent, tr( "Delete features" ), tr( "Delete %n feature(s)?", "number of features to delete", numberOfDeletedFeatures ), QMessageBox::Ok, QMessageBox::Cancel ) == QMessageBox::Cancel )
  {
    return;
  }

  vlayer->beginEditCommand( tr( "Features deleted" ) );
  if ( !vlayer->deleteSelectedFeatures() )
  {
    messageBar()->pushMessage( tr( "Problem deleting features" ),
                               tr( "A problem occured during deletion of features" ),
                               QgsMessageBar::WARNING );
  }

  vlayer->endEditCommand();
}

void QgisApp::moveFeature()
{
  mMapCanvas->setMapTool( mMapTools.mMoveFeature );
}

void QgisApp::offsetCurve()
{
  mMapCanvas->setMapTool( mMapTools.mOffsetCurve );
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
  if ( !vl || featureList.size() < 2 )
  {
    return 0;
  }

  QgsGeometry* unionGeom = featureList[0].geometry();
  QgsGeometry* backupPtr = 0; //pointer to delete intermediate results
  if ( !unionGeom )
  {
    return 0;
  }

  QProgressDialog progress( tr( "Merging features..." ), tr( "Abort" ), 0, featureList.size(), this );
  progress.setWindowModality( Qt::WindowModal );

  QApplication::setOverrideCursor( Qt::WaitCursor );

  for ( int i = 1; i < featureList.size(); ++i )
  {
    if ( progress.wasCanceled() )
    {
      delete unionGeom;
      QApplication::restoreOverrideCursor();
      canceled = true;
      return 0;
    }
    progress.setValue( i );
    QgsGeometry* currentGeom = featureList[i].geometry();
    if ( currentGeom )
    {
      backupPtr = unionGeom;
      unionGeom = unionGeom->combine( currentGeom );
      if ( i > 1 ) //delete previous intermediate results
      {
        delete backupPtr;
        backupPtr = 0;
      }
      if ( !unionGeom )
      {
        QApplication::restoreOverrideCursor();
        return 0;
      }
    }
  }

  //convert unionGeom to a multipart geometry in case it is necessary to match the layer type
  QGis::WkbType t = vl->wkbType();
  bool layerIsMultiType = ( t == QGis::WKBMultiPoint || t == QGis::WKBMultiPoint25D || t == QGis::WKBMultiLineString
                            || t == QGis::WKBMultiLineString25D || t == QGis::WKBMultiPolygon || t == QGis::WKBMultiPoint25D );
  if ( layerIsMultiType && !unionGeom->isMultipart() )
  {
    unionGeom->convertToMultiType();
  }

  QApplication::restoreOverrideCursor();
  progress.setValue( featureList.size() );
  return unionGeom;
}

QString QgisApp::uniqueComposerTitle( QWidget* parent, bool acceptEmpty, const QString& currentName )
{
  if ( !parent )
  {
    parent = this;
  }
  bool ok = false;
  bool titleValid = false;
  QString newTitle = QString( currentName );
  QString chooseMsg = tr( "Create unique print composer title" );
  if ( acceptEmpty )
  {
    chooseMsg += "\n" + tr( "(title generated if left empty)" );
  }
  QString titleMsg = chooseMsg;

  QStringList cNames;
  cNames << newTitle;
  foreach ( QgsComposer* c, printComposers() )
  {
    cNames << c->title();
  }

  while ( !titleValid )
  {
    newTitle = QInputDialog::getItem( parent,
                                      tr( "Composer title" ),
                                      titleMsg,
                                      cNames,
                                      cNames.indexOf( newTitle ),
                                      true,
                                      &ok );
    if ( !ok )
    {
      return QString::null;
    }

    if ( newTitle.isEmpty() )
    {
      if ( !acceptEmpty )
      {
        titleMsg = chooseMsg + "\n\n" + tr( "Title can not be empty!" );
      }
      else
      {
        newTitle = QString( "" );
        titleValid = true;
      }
    }
    else if ( cNames.indexOf( newTitle, 1 ) >= 0 )
    {
      cNames[0] = QString( "" ); // clear non-unique name
      titleMsg = chooseMsg + "\n\n" + tr( "Title already exists!" );
    }
    else
    {
      titleValid = true;
    }
  }

  return newTitle;
}

QgsComposer* QgisApp::createNewComposer( QString title )
{
  //ask user about name
  mLastComposerId++;
  if ( title.isEmpty() )
  {
    title = tr( "Composer %1" ).arg( mLastComposerId );
  }
  //create new composer object
  QgsComposer* newComposerObject = new QgsComposer( this, title );

  //add it to the map of existing print composers
  mPrintComposers.insert( newComposerObject );
  //and place action into print composers menu
  mPrintComposersMenu->addAction( newComposerObject->windowAction() );
  newComposerObject->open();
  emit composerAdded( newComposerObject->view() );
  connect( newComposerObject, SIGNAL( composerAdded( QgsComposerView* ) ), this, SIGNAL( composerAdded( QgsComposerView* ) ) );
  connect( newComposerObject, SIGNAL( composerWillBeRemoved( QgsComposerView* ) ), this, SIGNAL( composerWillBeRemoved( QgsComposerView* ) ) );
  markDirty();
  return newComposerObject;
}

void QgisApp::deleteComposer( QgsComposer* c )
{
  emit composerWillBeRemoved( c->view() );
  mPrintComposers.remove( c );
  mPrintComposersMenu->removeAction( c->windowAction() );
  markDirty();
  delete c;
}

QgsComposer* QgisApp::duplicateComposer( QgsComposer* currentComposer, QString title )
{
  QgsComposer* newComposer = 0;

  // test that current composer template write is valid
  QDomDocument currentDoc;
  currentComposer->templateXML( currentDoc );
  QDomElement compositionElem = currentDoc.documentElement().firstChildElement( "Composition" );
  if ( compositionElem.isNull() )
  {
    QgsDebugMsg( "selected composer could not be stored as temporary template" );
    return newComposer;
  }

  if ( title.isEmpty() )
  {
    // TODO: inject a bit of randomness in auto-titles?
    title = currentComposer->title() + tr( " copy" );
  }

  newComposer = createNewComposer( title );
  if ( !newComposer )
  {
    QgsDebugMsg( "could not create new composer" );
    return newComposer;
  }

  // hiding composer until template is loaded is much faster, provide feedback to user
  newComposer->hide();
  QApplication::setOverrideCursor( Qt::BusyCursor );
  if ( !newComposer->composition()->loadFromTemplate( currentDoc, 0, false ) )
  {
    deleteComposer( newComposer );
    newComposer = 0;
    QgsDebugMsg( "Error, composer could not be duplicated" );
    return newComposer;
  }
  newComposer->activate();
  QApplication::restoreOverrideCursor();

  return newComposer;
}

bool QgisApp::loadComposersFromProject( const QDomDocument& doc )
{
  if ( doc.isNull() )
  {
    return false;
  }

  //restore each composer
  QDomNodeList composerNodes = doc.elementsByTagName( "Composer" );
  for ( int i = 0; i < composerNodes.size(); ++i )
  {
    ++mLastComposerId;
    QgsComposer* composer = new QgsComposer( this, tr( "Composer %1" ).arg( mLastComposerId ) );
    composer->readXML( composerNodes.at( i ).toElement(), doc );
    mPrintComposers.insert( composer );
    mPrintComposersMenu->addAction( composer->windowAction() );
#ifndef Q_OS_MACX
    composer->setWindowState( Qt::WindowMinimized );
    composer->show();
#endif
    composer->zoomFull();
    QgsComposerView* composerView = composer->view();
    if ( composerView )
    {
      composerView->updateRulers();
    }
    if ( composerNodes.at( i ).toElement().attribute( "visible", "1" ).toInt() < 1 )
    {
      composer->close();
    }
    emit composerAdded( composer->view() );
    connect( composer, SIGNAL( composerAdded( QgsComposerView* ) ), this, SIGNAL( composerAdded( QgsComposerView* ) ) );
    connect( composer, SIGNAL( composerWillBeRemoved( QgsComposerView* ) ), this, SIGNAL( composerWillBeRemoved( QgsComposerView* ) ) );
  }
  return true;
}

void QgisApp::deletePrintComposers()
{
  QSet<QgsComposer*>::iterator it = mPrintComposers.begin();
  for ( ; it != mPrintComposers.end(); ++it )
  {
    emit composerWillBeRemoved(( *it )->view() );
    delete(( *it )->composition() );
    delete( *it );
  }
  mPrintComposers.clear();
  mLastComposerId = 0;
  markDirty();
}

void QgisApp::on_mPrintComposersMenu_aboutToShow()
{
  QList<QAction*> acts = mPrintComposersMenu->actions();
  mPrintComposersMenu->clear();
  if ( acts.size() > 1 )
  {
    // sort actions by text
    qSort( acts.begin(), acts.end(), cmpByText_ );
  }
  mPrintComposersMenu->addActions( acts );
}

bool QgisApp::loadAnnotationItemsFromProject( const QDomDocument& doc )
{
  if ( !mMapCanvas )
  {
    return false;
  }

  removeAnnotationItems();

  if ( doc.isNull() )
  {
    return false;
  }

  QDomNodeList textItemList = doc.elementsByTagName( "TextAnnotationItem" );
  for ( int i = 0; i < textItemList.size(); ++i )
  {
    QgsTextAnnotationItem* newTextItem = new QgsTextAnnotationItem( mMapCanvas );
    newTextItem->readXML( doc, textItemList.at( i ).toElement() );
  }

  QDomNodeList formItemList = doc.elementsByTagName( "FormAnnotationItem" );
  for ( int i = 0; i < formItemList.size(); ++i )
  {
    QgsFormAnnotationItem* newFormItem = new QgsFormAnnotationItem( mMapCanvas );
    newFormItem->readXML( doc, formItemList.at( i ).toElement() );
  }

  QDomNodeList htmlItemList = doc.elementsByTagName( "HtmlAnnotationItem" );
  for ( int i = 0; i < htmlItemList.size(); ++i )
  {
    QgsHtmlAnnotationItem* newHtmlItem = new QgsHtmlAnnotationItem( mMapCanvas );
    newHtmlItem->readXML( doc, htmlItemList.at( i ).toElement() );
  }

  QDomNodeList svgItemList = doc.elementsByTagName( "SVGAnnotationItem" );
  for ( int i = 0; i < svgItemList.size(); ++i )
  {
    QgsSvgAnnotationItem* newSvgItem = new QgsSvgAnnotationItem( mMapCanvas );
    newSvgItem->readXML( doc, svgItemList.at( i ).toElement() );
  }
  return true;
}

void QgisApp::showPinnedLabels( bool show )
{
  qobject_cast<QgsMapToolPinLabels*>( mMapTools.mPinLabels )->showPinnedLabels( show );
}

void QgisApp::pinLabels()
{
  mActionShowPinnedLabels->setChecked( true );
  mMapCanvas->setMapTool( mMapTools.mPinLabels );
}

void QgisApp::showHideLabels()
{
  mMapCanvas->setMapTool( mMapTools.mShowHideLabels );
}

void QgisApp::moveLabel()
{
  mMapCanvas->setMapTool( mMapTools.mMoveLabel );
}

void QgisApp::rotateFeature()
{
  mMapCanvas->setMapTool( mMapTools.mRotateFeature );
}

void QgisApp::rotateLabel()
{
  mMapCanvas->setMapTool( mMapTools.mRotateLabel );
}

void QgisApp::changeLabelProperties()
{
  mMapCanvas->setMapTool( mMapTools.mChangeLabelProperties );
}

QList<QgsAnnotationItem*> QgisApp::annotationItems()
{
  QList<QgsAnnotationItem*> itemList;

  if ( !mMapCanvas )
  {
    return itemList;
  }

  if ( mMapCanvas )
  {
    QList<QGraphicsItem*> graphicsItems = mMapCanvas->items();
    QList<QGraphicsItem*>::iterator gIt = graphicsItems.begin();
    for ( ; gIt != graphicsItems.end(); ++gIt )
    {
      QgsAnnotationItem* currentItem = dynamic_cast<QgsAnnotationItem*>( *gIt );
      if ( currentItem )
      {
        itemList.push_back( currentItem );
      }
    }
  }
  return itemList;
}

void QgisApp::removeAnnotationItems()
{
  if ( !mMapCanvas )
  {
    return;
  }
  QGraphicsScene* scene = mMapCanvas->scene();
  if ( !scene )
  {
    return;
  }
  QList<QgsAnnotationItem*> itemList = annotationItems();
  QList<QgsAnnotationItem*>::iterator itemIt = itemList.begin();
  for ( ; itemIt != itemList.end(); ++itemIt )
  {
    if ( *itemIt )
    {
      scene->removeItem( *itemIt );
      delete *itemIt;
    }
  }
}

void QgisApp::mergeAttributesOfSelectedFeatures()
{
  //get active layer (hopefully vector)
  QgsMapLayer *activeMapLayer = activeLayer();
  if ( !activeMapLayer )
  {
    messageBar()->pushMessage( tr( "No active layer" ),
                               tr( "No active layer found. Please select a layer in the layer list" ),
                               QgsMessageBar::INFO, messageTimeout() );
    return;
  }

  QgsVectorLayer* vl = qobject_cast<QgsVectorLayer *>( activeMapLayer );
  if ( !vl )
  {
    QMessageBox::information( 0, tr( "Active layer is not vector" ), tr( "The merge features tool only works on vector layers. Please select a vector layer from the layer list" ) );
    return;
  }

  if ( !vl->isEditable() )
  {
    QMessageBox::information( 0, tr( "Layer not editable" ), tr( "Merging features can only be done for layers in editing mode. To use the merge tool, go to  Layer->Toggle editing" ) );
    return;
  }

  //get selected feature ids (as a QSet<int> )
  const QgsFeatureIds& featureIdSet = vl->selectedFeaturesIds();
  if ( featureIdSet.size() < 2 )
  {
    QMessageBox::information( 0, tr( "Not enough features selected" ), tr( "The merge tool requires at least two selected features" ) );
    return;
  }

  //get initial selection (may be altered by attribute merge dialog later)
  QgsFeatureList featureList = vl->selectedFeatures();  //get QList<QgsFeature>

  //merge the attributes together
  QgsMergeAttributesDialog d( featureList, vl, mapCanvas() );
  if ( d.exec() == QDialog::Rejected )
  {
    return;
  }

  vl->beginEditCommand( tr( "Merged feature attributes" ) );

  const QgsAttributes &merged = d.mergedAttributes();

  foreach ( QgsFeatureId fid, vl->selectedFeaturesIds() )
  {
    for ( int i = 0; i < merged.count(); ++i )
    {
      vl->changeAttributeValue( fid, i, merged.at( i ) );
    }
  }

  vl->endEditCommand();

  if ( mapCanvas() )
  {
    mapCanvas()->refresh();
  }
}

void QgisApp::mergeSelectedFeatures()
{
  //get active layer (hopefully vector)
  QgsMapLayer* activeMapLayer = activeLayer();
  if ( !activeMapLayer )
  {
    QMessageBox::information( 0, tr( "No active layer" ), tr( "No active layer found. Please select a layer in the layer list" ) );
    return;
  }
  QgsVectorLayer* vl = qobject_cast<QgsVectorLayer *>( activeMapLayer );
  if ( !vl )
  {
    QMessageBox::information( 0, tr( "Active layer is not vector" ), tr( "The merge features tool only works on vector layers. Please select a vector layer from the layer list" ) );
    return;
  }
  if ( !vl->isEditable() )
  {
    QMessageBox::information( 0, tr( "Layer not editable" ), tr( "Merging features can only be done for layers in editing mode. To use the merge tool, go to  Layer->Toggle editing" ) );
    return;
  }

  QgsVectorDataProvider* dp = vl->dataProvider();
  bool providerChecksTypeStrictly = true;
  if ( dp )
  {
    providerChecksTypeStrictly = dp->doesStrictFeatureTypeCheck();
  }

  //get selected feature ids (as a QSet<int> )
  const QgsFeatureIds& featureIdSet = vl->selectedFeaturesIds();
  if ( featureIdSet.size() < 2 )
  {
    QMessageBox::information( 0, tr( "Not enough features selected" ), tr( "The merge tool requires at least two selected features" ) );
    return;
  }

  //get initial selection (may be altered by attribute merge dialog later)
  QgsFeatureList featureList = vl->selectedFeatures();  //get QList<QgsFeature>
  bool canceled;
  QgsGeometry* unionGeom = unionGeometries( vl, featureList, canceled );
  if ( !unionGeom )
  {
    if ( !canceled )
    {
      QMessageBox::critical( 0, tr( "Merge failed" ), tr( "An error occured during the merge operation" ) );
    }
    return;
  }

  //make a first geometry union and notify the user straight away if the union geometry type does not match the layer one
  if ( providerChecksTypeStrictly && unionGeom->wkbType() != vl->wkbType() )
  {
    QMessageBox::critical( 0, tr( "Union operation canceled" ), tr( "The union operation would result in a geometry type that is not compatible with the current layer and therefore is canceled" ) );
    delete unionGeom;
    return;
  }

  //merge the attributes together
  QgsMergeAttributesDialog d( featureList, vl, mapCanvas() );
  if ( d.exec() == QDialog::Rejected )
  {
    delete unionGeom;
    return;
  }

  QgsFeatureList featureListAfter = vl->selectedFeatures();

  if ( featureListAfter.size() < 2 )
  {
    QMessageBox::information( 0, tr( "Not enough features selected" ), tr( "The merge tool requires at least two selected features" ) );
    delete unionGeom;
    return;
  }

  //if the user changed the feature selection in the merge dialog, we need to repeat the union and check the type
  if ( featureList.size() != featureListAfter.size() )
  {
    delete unionGeom;
    bool canceled;
    unionGeom = unionGeometries( vl, featureListAfter, canceled );
    if ( !unionGeom )
    {
      if ( !canceled )
      {
        QMessageBox::critical( 0, tr( "Merge failed" ), tr( "An error occured during the merge operation" ) );
      }
      return;
    }

    if ( providerChecksTypeStrictly && unionGeom->wkbType() != vl->wkbType() )
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
  newFeature.setAttributes( d.mergedAttributes() );

  QgsFeatureList::const_iterator feature_it = featureListAfter.constBegin();
  for ( ; feature_it != featureListAfter.constEnd(); ++feature_it )
  {
    vl->deleteFeature( feature_it->id() );
  }

  vl->addFeature( newFeature, false );

  vl->endEditCommand();

  if ( mapCanvas() )
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

void QgisApp::snappingOptions()
{
  mSnappingDialog->show();
}

void QgisApp::splitFeatures()
{
  mMapCanvas->setMapTool( mMapTools.mSplitFeatures );
}

void QgisApp::reshapeFeatures()
{
  mMapCanvas->setMapTool( mMapTools.mReshapeFeatures );
}

void QgisApp::addFeature()
{
  if ( mMapCanvas && mMapCanvas->isDrawing() )
  {
    return;
  }
  mMapCanvas->setMapTool( mMapTools.mAddFeature );
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
  if ( !mMapCanvas || mMapCanvas->isDrawing() )
  {
    return;
  }

  // Turn off rendering to improve speed.
  bool renderFlagState = mMapCanvas->renderFlag();
  if ( renderFlagState )
    mMapCanvas->setRenderFlag( false );

  QMap<QString, QgsMapLayer*> layers = QgsMapLayerRegistry::instance()->mapLayers();
  for ( QMap<QString, QgsMapLayer*>::iterator it = layers.begin(); it != layers.end(); it++ )
  {
    QgsVectorLayer *vl = qobject_cast<QgsVectorLayer *>( it.value() );
    if ( !vl )
      continue;

    vl->removeSelection();
  }

  // Turn on rendering (if it was on previously)
  if ( renderFlagState )
    mMapCanvas->setRenderFlag( true );
}

void QgisApp::selectByExpression()
{
  QgsVectorLayer *vlayer = qobject_cast<QgsVectorLayer *>( mMapCanvas->currentLayer() );
  if ( !vlayer )
  {
    messageBar()->pushMessage(
      tr( "No active vector layer" ),
      tr( "To select features, choose a vector layer in the legend" ),
      QgsMessageBar::INFO,
      messageTimeout() );
    return;
  }

  QgsExpressionSelectionDialog* dlg = new QgsExpressionSelectionDialog( vlayer );
  dlg->setAttribute( Qt::WA_DeleteOnClose );
  dlg->show();
}

void QgisApp::addRing()
{
  if ( mMapCanvas && mMapCanvas->isDrawing() )
  {
    return;
  }
  mMapCanvas->setMapTool( mMapTools.mAddRing );
}

void QgisApp::addPart()
{
  if ( mMapCanvas && mMapCanvas->isDrawing() )
  {
    return;
  }
  mMapCanvas->setMapTool( mMapTools.mAddPart );
}


void QgisApp::editCut( QgsMapLayer * layerContainingSelection )
{
  if ( mMapCanvas && mMapCanvas->isDrawing() )
  {
    return;
  }

  // Test for feature support in this layer
  QgsVectorLayer* selectionVectorLayer = qobject_cast<QgsVectorLayer *>( layerContainingSelection ? layerContainingSelection : activeLayer() );
  if ( !selectionVectorLayer )
    return;

  clipboard()->replaceWithCopyOf( selectionVectorLayer );

  selectionVectorLayer->beginEditCommand( tr( "Features cut" ) );
  selectionVectorLayer->deleteSelectedFeatures();
  selectionVectorLayer->endEditCommand();
  activateDeactivateLayerRelatedActions( activeLayer() );
}

void QgisApp::editCopy( QgsMapLayer * layerContainingSelection )
{
  if ( mMapCanvas && mMapCanvas->isDrawing() )
  {
    return;
  }

  QgsVectorLayer* selectionVectorLayer = qobject_cast<QgsVectorLayer *>( layerContainingSelection ? layerContainingSelection : activeLayer() );
  if ( !selectionVectorLayer )
    return;

  // Test for feature support in this layer
  clipboard()->replaceWithCopyOf( selectionVectorLayer );
  activateDeactivateLayerRelatedActions( activeLayer() );
}


void QgisApp::editPaste( QgsMapLayer *destinationLayer )
{
  if ( mMapCanvas && mMapCanvas->isDrawing() )
  {
    return;
  }

  QgsVectorLayer *pasteVectorLayer = qobject_cast<QgsVectorLayer *>( destinationLayer ? destinationLayer : activeLayer() );
  if ( !pasteVectorLayer )
    return;

  pasteVectorLayer->beginEditCommand( tr( "Features pasted" ) );
  QgsFeatureList features;
  if ( mMapCanvas->mapRenderer()->hasCrsTransformEnabled() )
  {
    features = clipboard()->transformedCopyOf( pasteVectorLayer->crs() );
  }
  else
  {
    features = clipboard()->copyOf();
  }

  QHash<int, int> remap;
  const QgsFields &fields = clipboard()->fields();
  QgsAttributeList pkAttrList = pasteVectorLayer->pendingPkAttributesList();
  for ( int idx = 0; idx < fields.count(); ++idx )
  {
    int dst = pasteVectorLayer->fieldNameIndex( fields[idx].name() );
    if ( dst < 0 || pkAttrList.contains( dst ) )
    {
      // skip primary key attributes
      continue;
    }
    remap.insert( idx, dst );
  }

  int dstAttrCount = pasteVectorLayer->pendingFields().count();
  for ( int i = 0; i < features.size(); i++ )
  {
    QgsFeature &f = features[i];
    const QgsAttributes &srcAttr = f.attributes();
    QgsAttributes dstAttr( dstAttrCount );

    for ( int src = 0; src < srcAttr.count(); ++src )
    {
      int dst = remap.value( src, -1 );
      if ( dst < 0 )
        continue;

      dstAttr[ dst ] = srcAttr[ src ];
    }

    f.setAttributes( dstAttr );

    //avoid intersection if enabled in digitize settings
    if ( f.geometry() )
    {
      f.geometry()->avoidIntersections();
    }
  }

  pasteVectorLayer->addFeatures( features );
  pasteVectorLayer->endEditCommand();
  mMapCanvas->refresh();
}

void QgisApp::copyStyle( QgsMapLayer * sourceLayer )
{
  QgsMapLayer *selectionLayer = sourceLayer ? sourceLayer : activeLayer();
  if ( selectionLayer )
  {
    QDomImplementation DomImplementation;
    QDomDocumentType documentType =
      DomImplementation.createDocumentType(
        "qgis", "http://mrcc.com/qgis.dtd", "SYSTEM" );
    QDomDocument doc( documentType );
    QDomElement rootNode = doc.createElement( "qgis" );
    rootNode.setAttribute( "version", QString( "%1" ).arg( QGis::QGIS_VERSION ) );
    doc.appendChild( rootNode );
    QString errorMsg;
    if ( !selectionLayer->writeSymbology( rootNode, doc, errorMsg ) )
    {
      QMessageBox::warning( this,
                            tr( "Error" ),
                            tr( "Cannot copy style: %1" )
                            .arg( errorMsg ),
                            QMessageBox::Ok );
      return;
    }
    // Copies data in text form as well, so the XML can be pasted into a text editor
    clipboard()->setData( QGSCLIPBOARD_STYLE_MIME, doc.toByteArray(), doc.toString() );
    // Enables the paste menu element
    mActionPasteStyle->setEnabled( true );
  }
}

void QgisApp::pasteStyle( QgsMapLayer * destinationLayer )
{
  QgsMapLayer *selectionLayer = destinationLayer ? destinationLayer : activeLayer();
  if ( selectionLayer )
  {
    if ( clipboard()->hasFormat( QGSCLIPBOARD_STYLE_MIME ) )
    {
      QDomDocument doc( "qgis" );
      QString errorMsg;
      int errorLine, errorColumn;
      if ( !doc.setContent( clipboard()->data( QGSCLIPBOARD_STYLE_MIME ), false, &errorMsg, &errorLine, &errorColumn ) )
      {
        QMessageBox::information( this,
                                  tr( "Error" ),
                                  tr( "Cannot parse style: %1:%2:%3" )
                                  .arg( errorMsg )
                                  .arg( errorLine )
                                  .arg( errorColumn ),
                                  QMessageBox::Ok );
        return;
      }
      QDomElement rootNode = doc.firstChildElement( "qgis" );
      if ( !selectionLayer->readSymbology( rootNode, errorMsg ) )
      {
        QMessageBox::information( this,
                                  tr( "Error" ),
                                  tr( "Cannot read style: %1" )
                                  .arg( errorMsg ),
                                  QMessageBox::Ok );
        return;
      }

      mMapLegend->refreshLayerSymbology( selectionLayer->id(), false );
      mMapCanvas->refresh();
    }
  }
}

#if 0
void QgisApp::pasteTransformations()
{
  QgsPasteTransformations *pt = new QgsPasteTransformations();

  mMapCanvas->freeze();

  pt->exec();
}
#endif

void QgisApp::copyFeatures( QgsFeatureStore & featureStore )
{
  clipboard()->replaceWithCopyOf( featureStore );
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
  if ( !mMapTipsVisible )
  {
    mpMapTipsTimer->stop();
  }
}

void QgisApp::toggleEditing()
{
  if ( mMapCanvas && mMapCanvas->isDrawing() )
    return;

  QgsVectorLayer *currentLayer = qobject_cast<QgsVectorLayer*>( activeLayer() );
  if ( currentLayer )
  {
    toggleEditing( currentLayer, true );
  }
  else
  {
    // active although there's no layer active!?
    mActionToggleEditing->setChecked( false );
  }
}

bool QgisApp::toggleEditing( QgsMapLayer *layer, bool allowCancel )
{
  QgsVectorLayer *vlayer = qobject_cast<QgsVectorLayer *>( layer );
  if ( !vlayer )
  {
    return false;
  }

  bool res = true;

  if ( !vlayer->isEditable() && !vlayer->isReadOnly() )
  {
    if ( !( vlayer->dataProvider()->capabilities() & QgsVectorDataProvider::EditingCapabilities ) )
    {
      mActionToggleEditing->setChecked( false );
      mActionToggleEditing->setEnabled( false );
      messageBar()->pushMessage( tr( "Start editing failed" ),
                                 tr( "Provider cannot be opened for editing" ),
                                 QgsMessageBar::INFO, messageTimeout() );
      return false;
    }

    vlayer->startEditing();

    QSettings settings;
    QString markerType = settings.value( "/qgis/digitizing/marker_style", "Cross" ).toString();
    bool markSelectedOnly = settings.value( "/qgis/digitizing/marker_only_for_selected", false ).toBool();

    // redraw only if markers will be drawn
    if (( !markSelectedOnly || vlayer->selectedFeatureCount() > 0 ) &&
        ( markerType == "Cross" || markerType == "SemiTransparentCircle" ) )
    {
      vlayer->triggerRepaint();
    }
  }
  else if ( vlayer->isModified() )
  {
    QMessageBox::StandardButtons buttons = QMessageBox::Save | QMessageBox::Discard;
    if ( allowCancel )
      buttons |= QMessageBox::Cancel;

    switch ( QMessageBox::information( 0,
                                       tr( "Stop editing" ),
                                       tr( "Do you want to save the changes to layer %1?" ).arg( vlayer->name() ),
                                       buttons ) )
    {
      case QMessageBox::Cancel:
        res = false;
        break;

      case QMessageBox::Save:
        if ( !vlayer->commitChanges() )
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

        vlayer->triggerRepaint();
        break;

      case QMessageBox::Discard:
        mMapCanvas->freeze( true );
        if ( !vlayer->rollBack() )
        {
          messageBar()->pushMessage( tr( "Error" ),
                                     tr( "Problems during roll back" ),
                                     QgsMessageBar::CRITICAL );
          res = false;
        }
        mMapCanvas->freeze( false );

        vlayer->triggerRepaint();
        break;

      default:
        break;
    }
  }
  else //layer not modified
  {
    mMapCanvas->freeze( true );
    vlayer->rollBack();
    mMapCanvas->freeze( false );
    res = true;
    vlayer->triggerRepaint();
  }

  if ( !res && layer == activeLayer() )
  {
    // while also called when layer sends editingStarted/editingStopped signals,
    // this ensures correct restoring of gui state if toggling was canceled
    // or layer commit/rollback functions failed
    activateDeactivateLayerRelatedActions( layer );
  }

  return res;
}

void QgisApp::saveActiveLayerEdits()
{
  saveEdits( activeLayer(), true, true );
}

void QgisApp::saveEdits( QgsMapLayer *layer, bool leaveEditable, bool triggerRepaint )
{
  QgsVectorLayer *vlayer = qobject_cast<QgsVectorLayer *>( layer );
  if ( !vlayer || !vlayer->isEditable() || !vlayer->isModified() )
    return;

  if ( vlayer == activeLayer() )
    mSaveRollbackInProgress = true;

  if ( !vlayer->commitChanges() )
  {
    mSaveRollbackInProgress = false;
    QMessageBox::information( 0,
                              tr( "Error" ),
                              tr( "Could not commit changes to layer %1\n\nErrors: %2\n" )
                              .arg( vlayer->name() )
                              .arg( vlayer->commitErrors().join( "\n  " ) ) );
  }

  if ( leaveEditable )
  {
    vlayer->startEditing();
  }
  if ( triggerRepaint )
  {
    vlayer->triggerRepaint();
  }
}

void QgisApp::cancelEdits( QgsMapLayer *layer, bool leaveEditable, bool triggerRepaint )
{
  QgsVectorLayer *vlayer = qobject_cast<QgsVectorLayer *>( layer );
  if ( !vlayer || !vlayer->isEditable() )
    return;

  if ( vlayer == activeLayer() && leaveEditable )
    mSaveRollbackInProgress = true;

  mMapCanvas->freeze( true );
  if ( !vlayer->rollBack( !leaveEditable ) )
  {
    mSaveRollbackInProgress = false;
    QMessageBox::information( 0,
                              tr( "Error" ),
                              tr( "Could not %1 changes to layer %2\n\nErrors: %3\n" )
                              .arg( leaveEditable ? tr( "rollback" ) : tr( "cancel" ) )
                              .arg( vlayer->name() )
                              .arg( vlayer->commitErrors().join( "\n  " ) ) );
  }
  mMapCanvas->freeze( false );

  if ( leaveEditable )
  {
    vlayer->startEditing();
  }
  if ( triggerRepaint )
  {
    vlayer->triggerRepaint();
  }
}

void QgisApp::saveEdits()
{
  if ( mMapCanvas && mMapCanvas->isDrawing() )
    return;

  foreach ( QgsMapLayer * layer, mMapLegend->selectedLayers() )
  {
    saveEdits( layer, true, false );
  }
  mMapCanvas->refresh();
  activateDeactivateLayerRelatedActions( activeLayer() );
}

void QgisApp::saveAllEdits( bool verifyAction )
{
  if ( mMapCanvas && mMapCanvas->isDrawing() )
    return;

  if ( verifyAction )
  {
    if ( !verifyEditsActionDialog( tr( "Save" ), tr( "all" ) ) )
      return;
  }

  foreach ( QgsMapLayer * layer, editableLayers( true ) )
  {
    saveEdits( layer, true, false );
  }
  mMapCanvas->refresh();
  activateDeactivateLayerRelatedActions( activeLayer() );
}

void QgisApp::rollbackEdits()
{
  if ( mMapCanvas && mMapCanvas->isDrawing() )
    return;

  foreach ( QgsMapLayer * layer, mMapLegend->selectedLayers() )
  {
    cancelEdits( layer, true, false );
  }
  mMapCanvas->refresh();
  activateDeactivateLayerRelatedActions( activeLayer() );
}

void QgisApp::rollbackAllEdits( bool verifyAction )
{
  if ( mMapCanvas && mMapCanvas->isDrawing() )
    return;

  if ( verifyAction )
  {
    if ( !verifyEditsActionDialog( tr( "Rollback" ), tr( "all" ) ) )
      return;
  }

  foreach ( QgsMapLayer * layer, editableLayers( true ) )
  {
    cancelEdits( layer, true, false );
  }
  mMapCanvas->refresh();
  activateDeactivateLayerRelatedActions( activeLayer() );
}

void QgisApp::cancelEdits()
{
  if ( mMapCanvas && mMapCanvas->isDrawing() )
    return;

  foreach ( QgsMapLayer * layer, mMapLegend->selectedLayers() )
  {
    cancelEdits( layer, false, false );
  }
  mMapCanvas->refresh();
  activateDeactivateLayerRelatedActions( activeLayer() );
}

void QgisApp::cancelAllEdits( bool verifyAction )
{
  if ( mMapCanvas && mMapCanvas->isDrawing() )
    return;

  if ( verifyAction )
  {
    if ( !verifyEditsActionDialog( tr( "Cancel" ), tr( "all" ) ) )
      return;
  }

  foreach ( QgsMapLayer * layer, editableLayers() )
  {
    cancelEdits( layer, false, false );
  }
  mMapCanvas->refresh();
  activateDeactivateLayerRelatedActions( activeLayer() );
}

bool QgisApp::verifyEditsActionDialog( const QString& act, const QString& upon )
{
  bool res = false;
  switch ( QMessageBox::information( 0,
                                     tr( "Current edits" ),
                                     tr( "%1 current changes for %2 layer(s)?" )
                                     .arg( act )
                                     .arg( upon ),
                                     QMessageBox::Cancel | QMessageBox::Ok ) )
  {
    case QMessageBox::Ok:
      res = true;
      break;
    default:
      break;
  }
  return res;
}

void QgisApp::updateLayerModifiedActions()
{
  bool enableSaveLayerEdits = false;
  QgsVectorLayer* vlayer = qobject_cast<QgsVectorLayer *>( activeLayer() );
  if ( vlayer )
  {
    QgsVectorDataProvider* dprovider = vlayer->dataProvider();
    if ( dprovider )
    {
      enableSaveLayerEdits = ( dprovider->capabilities() & QgsVectorDataProvider::ChangeAttributeValues
                               && vlayer->isEditable()
                               && vlayer->isModified() );
    }
  }
  mActionSaveLayerEdits->setEnabled( enableSaveLayerEdits );

  mActionSaveEdits->setEnabled( mMapLegend && mMapLegend->selectedLayersEditable( true ) );
  mActionRollbackEdits->setEnabled( mMapLegend && mMapLegend->selectedLayersEditable( true ) );
  mActionCancelEdits->setEnabled( mMapLegend && mMapLegend->selectedLayersEditable() );

  bool hasEditLayers = ( editableLayers().count() > 0 );
  mActionAllEdits->setEnabled( hasEditLayers );
  mActionCancelAllEdits->setEnabled( hasEditLayers );

  bool hasModifiedLayers = ( editableLayers( true ).count() > 0 );
  mActionSaveAllEdits->setEnabled( hasModifiedLayers );
  mActionRollbackAllEdits->setEnabled( hasModifiedLayers );
}

QList<QgsMapLayer *> QgisApp::editableLayers( bool modified ) const
{
  QList<QgsMapLayer*> editLayers;
  // use legend layers (instead of registry) so QList mirrors its order
  QList<QgsMapLayer*> layers = mMapLegend->layers();
  if ( layers.count() > 0 )
  {
    foreach ( QgsMapLayer* layer, layers )
    {
      // get layer's editable/modified state from registry since we may be
      // responding to same signal that legend is also responding to
      QgsVectorLayer *vl = qobject_cast<QgsVectorLayer*>(
                             QgsMapLayerRegistry::instance()->mapLayer( layer->id() ) );
      if ( !vl )
      {
        continue;
      }
      if ( vl->isEditable() && ( !modified || ( modified && vl->isModified() ) ) )
      {
        editLayers << vl;
      }
    }
  }
  return editLayers;
}

void QgisApp::layerSubsetString()
{
  if ( mMapCanvas && mMapCanvas->isDrawing() )
    return;

  QgsVectorLayer *vlayer = qobject_cast<QgsVectorLayer *>( activeLayer() );
  if ( !vlayer )
    return;

  // launch the query builder
  QgsQueryBuilder *qb = new QgsQueryBuilder( vlayer, this );
  QString subsetBefore = vlayer->subsetString();

  // Set the sql in the query builder to the same in the prop dialog
  // (in case the user has already changed it)
  qb->setSql( vlayer->subsetString() );
  // Open the query builder
  if ( qb->exec() )
  {
    if ( subsetBefore != qb->sql() )
    {
      mMapCanvas->refresh();
      if ( mMapLegend )
      {
        mMapLegend->refreshLayerSymbology( vlayer->id(), false );
      }
    }
  }

  // delete the query builder object
  delete qb;
}

void QgisApp::showMouseCoordinate( const QgsPoint & p )
{
  if ( mMapTipsVisible )
  {
    // store the point, we need it for when the maptips timer fires
    mLastMapPosition = p;

    // we use this slot to control the timer for maptips since it is fired each time
    // the mouse moves.
    if ( mMapCanvas->underMouse() )
    {
      // Clear the maptip (this is done conditionally)
      mpMaptip->clear( mMapCanvas );
      // don't start the timer if the mouse is not over the map canvas
      mpMapTipsTimer->start();
      //QgsDebugMsg("Started maptips timer");
    }
  }
  if ( mToggleExtentsViewButton->isChecked() )
  {
    //we are in show extents mode so no need to do anything
    return;
  }
  else
  {
    if ( mMapCanvas->mapUnits() == QGis::Degrees )
    {
      QString format = QgsProject::instance()->readEntry( "PositionPrecision", "/DegreeFormat", "D" );

      if ( format == "DM" )
        mCoordsEdit->setText( p.toDegreesMinutes( mMousePrecisionDecimalPlaces ) );
      else if ( format == "DMS" )
        mCoordsEdit->setText( p.toDegreesMinutesSeconds( mMousePrecisionDecimalPlaces ) );
      else
        mCoordsEdit->setText( p.toString( mMousePrecisionDecimalPlaces ) );
    }
    else
    {
      mCoordsEdit->setText( p.toString( mMousePrecisionDecimalPlaces ) );
    }

    if ( mCoordsEdit->width() > mCoordsEdit->minimumWidth() )
    {
      mCoordsEdit->setMinimumWidth( mCoordsEdit->width() );
    }
  }
}


void QgisApp::showScale( double theScale )
{
  // Why has MapCanvas the scale inverted?
  mScaleEdit->setScale( 1.0 / theScale );

  // Not sure if the lines below do anything meaningful /Homann
  if ( mScaleEdit->width() > mScaleEdit->minimumWidth() )
  {
    mScaleEdit->setMinimumWidth( mScaleEdit->width() );
  }
}

void QgisApp::userScale()
{
  // Why has MapCanvas the scale inverted?
  mMapCanvas->zoomScale( 1.0 / mScaleEdit->scale() );
}

void QgisApp::userCenter()
{
  QStringList parts = mCoordsEdit->text().split( ',' );
  if ( parts.size() != 2 )
    return;

  bool xOk;
  double x = parts.at( 0 ).toDouble( &xOk );
  if ( !xOk )
    return;

  bool yOk;
  double y = parts.at( 1 ).toDouble( &yOk );
  if ( !yOk )
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

void QgisApp::removingLayers( QStringList theLayers )
{
  foreach ( const QString &layerId, theLayers )
  {
    QgsVectorLayer *vlayer = qobject_cast<QgsVectorLayer*>(
                               QgsMapLayerRegistry::instance()->mapLayer( layerId ) );
    if ( !vlayer || !vlayer->isEditable() )
      return;

    toggleEditing( vlayer, false );
  }
}

void QgisApp::removeAllLayers()
{
  QgsMapLayerRegistry::instance()->removeAllMapLayers();
}

void QgisApp::removeLayer()
{
  if ( mMapCanvas && mMapCanvas->isDrawing() )
  {
    return;
  }

  if ( !mMapLegend )
  {
    return;
  }

  foreach ( QgsMapLayer * layer, mMapLegend->selectedLayers() )
  {
    QgsVectorLayer *vlayer = qobject_cast<QgsVectorLayer*>( layer );
    if ( vlayer && vlayer->isEditable() && !toggleEditing( vlayer, true ) )
      return;
  }

  mMapLegend->removeSelectedLayers();

  mMapCanvas->refresh();
}

void QgisApp::duplicateLayers( QList<QgsMapLayer *> lyrList )
{
  if ( mMapCanvas && mMapCanvas->isDrawing() )
  {
    return;
  }

  if ( !mMapLegend )
  {
    return;
  }

  const QList<QgsMapLayer *> selectedLyrs = lyrList.empty() ? mMapLegend->selectedLayers() : lyrList;
  if ( selectedLyrs.empty() )
  {
    return;
  }

  mMapCanvas->freeze();
  QgsMapLayer *dupLayer;
  QString layerDupName, unSppType;
  QList<QWidget *> msgBars;

  foreach ( QgsMapLayer * selectedLyr, selectedLyrs )
  {
    dupLayer = 0;
    unSppType = QString( "" );
    layerDupName = selectedLyr->name() + " " + tr( "copy" );

    if ( selectedLyr->type() == QgsMapLayer::PluginLayer )
    {
      unSppType = tr( "Plugin layer" );
    }

    // duplicate the layer's basic parameters

    if ( unSppType.isEmpty() )
    {
      QgsVectorLayer *vlayer = qobject_cast<QgsVectorLayer*>( selectedLyr );
      // TODO: add other layer types that can be duplicated
      // currently memory and plugin layers are skipped
      if ( vlayer && vlayer->storageType() == "Memory storage" )
      {
        unSppType = tr( "Memory layer" );
      }
      else if ( vlayer )
      {
        dupLayer = new QgsVectorLayer( vlayer->source(), layerDupName, vlayer->providerType() );
      }
    }

    if ( unSppType.isEmpty() && !dupLayer )
    {
      QgsRasterLayer *rlayer = qobject_cast<QgsRasterLayer*>( selectedLyr );
      if ( rlayer )
      {
        dupLayer = new QgsRasterLayer( rlayer->source(), layerDupName );
      }
    }

    if ( unSppType.isEmpty() && dupLayer && !dupLayer->isValid() )
    {
      msgBars.append( QgsMessageBar::createMessage(
                        tr( "Duplicate layer: " ),
                        tr( "%1 (duplication resulted in invalid layer)" ).arg( selectedLyr->name() ) ,
                        QgsApplication::getThemeIcon( "/mIconWarn.png" ),
                        mInfoBar ) );
      continue;
    }

    if ( !unSppType.isEmpty() || !dupLayer )
    {
      msgBars.append( QgsMessageBar::createMessage(
                        tr( "Duplicate layer: " ),
                        tr( "%1 (%2type unsupported)" )
                        .arg( selectedLyr->name() )
                        .arg( !unSppType.isEmpty() ? QString( "'" ) + unSppType + "' " : "" ),
                        QgsApplication::getThemeIcon( "/mIconWarn.png" ),
                        mInfoBar ) );
      continue;
    }

    // add layer to layer registry and legend
    QList<QgsMapLayer *> myList;
    myList << dupLayer;
    QgsMapLayerRegistry::instance()->addMapLayers( myList );

    // verify layer has been added to legend
    QgsLegendLayer *duplLayer = 0;
    duplLayer = mMapLegend->findLegendLayer( dupLayer );
    if ( !duplLayer )
    {
      // some source layers, like items > 4th in a container, have their layer
      // registered but do not show up in the legend, so manually add them
      QgsLegendLayer* llayer = new QgsLegendLayer( dupLayer );
      mMapLegend->insertTopLevelItem( 0, llayer );
      // double-check, or move of non-existent legend layer will segfault
      duplLayer = mMapLegend->findLegendLayer( dupLayer );
    }

    QgsLegendLayer *srclLayer = mMapLegend->findLegendLayer( selectedLyr );
    if ( duplLayer && srclLayer )
    {
      // move layer to just below source layer
      mMapLegend->moveItem( duplLayer, srclLayer );

      // duplicate the layer style
      copyStyle( selectedLyr );
      pasteStyle( dupLayer );

      // always set duplicated layers to not visible
      // so layer can be configured before being turned on,
      // and no map canvas refresh needed when doing multiple duplications
      mMapLegend->setLayerVisible( dupLayer, false );
    }
  }

  dupLayer = 0;

  // update UI
  qApp->processEvents();

  mMapCanvas->freeze( false );

  // display errors in message bar after duplication of layers
  foreach ( QWidget * msgBar, msgBars )
  {
    mInfoBar->pushWidget( msgBar, QgsMessageBar::WARNING );
  }

}

void QgisApp::setLayerCRS()
{
  if ( mMapCanvas && mMapCanvas->isDrawing() )
  {
    return;
  }

  if ( !( mMapLegend && mMapLegend->currentLayer() ) )
  {
    return;
  }

  QgsGenericProjectionSelector * mySelector = new QgsGenericProjectionSelector( this );
  mySelector->setSelectedCrsId( mMapLegend->currentLayer()->crs().srsid() );
  mySelector->setMessage();
  if ( mySelector->exec() )
  {
    QgsCoordinateReferenceSystem crs( mySelector->selectedCrsId(), QgsCoordinateReferenceSystem::InternalCrsId );
    mMapLegend->setCRSForSelectedLayers( crs );
    mMapCanvas->refresh();
  }
  else
  {
    QApplication::restoreOverrideCursor();
  }

  delete mySelector;
}

void QgisApp::setProjectCRSFromLayer()
{
  if ( mMapCanvas && mMapCanvas->isDrawing() )
  {
    return;
  }

  if ( !( mMapLegend && mMapLegend->currentLayer() ) )
  {
    return;
  }

  QgsCoordinateReferenceSystem crs = mMapLegend->currentLayer()->crs();
  QgsMapRenderer* myRenderer = mMapCanvas->mapRenderer();
  mMapCanvas->freeze();
  myRenderer->setDestinationCrs( crs );
  if ( crs.mapUnits() != QGis::UnknownUnit )
  {
    myRenderer->setMapUnits( crs.mapUnits() );
  }
  mMapCanvas->freeze( false );
  mMapCanvas->refresh();
}

void QgisApp::zoomToLayerExtent()
{
  mMapLegend->legendLayerZoom();
}

void QgisApp::showPluginManager()
{

  if ( mPythonUtils && mPythonUtils->isEnabled() )
  {
    // Call pluginManagerInterface()->showPluginManager() as soon as the plugin installer says the remote data is fetched.
    QgsPythonRunner::run( "pyplugin_installer.instance().showPluginManagerWhenReady()" );
  }
  else
  {
    // Call the pluginManagerInterface directly
    mQgisInterface->pluginManagerInterface()->showPluginManager();
  }
}

// implementation of the python runner
class QgsPythonRunnerImpl : public QgsPythonRunner
{
  public:
    QgsPythonRunnerImpl( QgsPythonUtils* pythonUtils ) : mPythonUtils( pythonUtils ) {}

    virtual bool runCommand( QString command, QString messageOnError = QString() )
    {
      if ( mPythonUtils && mPythonUtils->isEnabled() )
      {
        return mPythonUtils->runString( command, messageOnError, false );
      }
      return false;
    }

    virtual bool evalCommand( QString command, QString &result )
    {
      if ( mPythonUtils && mPythonUtils->isEnabled() )
      {
        return mPythonUtils->evalString( command, result );
      }
      return false;
    }

  protected:
    QgsPythonUtils* mPythonUtils;
};

void QgisApp::loadPythonSupport()
{
  QString pythonlibName( "qgispython" );
#if defined(Q_WS_MAC) || defined(Q_OS_LINUX)
  pythonlibName.prepend( QgsApplication::libraryPath() );
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
  if ( !pythonlib.load() )
  {
    pythonlib.setFileName( pythonlibName );
    if ( !pythonlib.load() )
    {
      QgsMessageLog::logMessage( tr( "Couldn't load Python support library: %1" ).arg( pythonlib.errorString() ) );
      return;
    }
  }

  //QgsDebugMsg("Python support library loaded successfully.");
  typedef QgsPythonUtils*( *inst )();
  inst pythonlib_inst = ( inst ) cast_to_fptr( pythonlib.resolve( "instance" ) );
  if ( !pythonlib_inst )
  {
    //using stderr on purpose because we want end users to see this [TS]
    QgsMessageLog::logMessage( tr( "Couldn't resolve python support library's instance() symbol." ) );
    return;
  }

  //QgsDebugMsg("Python support library's instance() symbol resolved.");
  mPythonUtils = pythonlib_inst();
  mPythonUtils->initPython( mQgisInterface );

  if ( mPythonUtils && mPythonUtils->isEnabled() )
  {
    QgsPluginRegistry::instance()->setPythonUtils( mPythonUtils );

    // init python runner
    QgsPythonRunner::setInstance( new QgsPythonRunnerImpl( mPythonUtils ) );

    QgsMessageLog::logMessage( tr( "Python support ENABLED :-) " ), QString::null, QgsMessageLog::INFO );
  }
  else
  {
    delete mActionShowPythonDialog;
    mActionShowPythonDialog = 0;
  }
}

void QgisApp::checkQgisVersion()
{
  QApplication::setOverrideCursor( Qt::WaitCursor );

  QNetworkReply *reply = QgsNetworkAccessManager::instance()->get( QNetworkRequest( QUrl( "http://qgis.org/version.txt" ) ) );
  connect( reply, SIGNAL( finished() ), this, SLOT( versionReplyFinished() ) );
}

void QgisApp::versionReplyFinished()
{
  QApplication::restoreOverrideCursor();

  QNetworkReply *reply = qobject_cast<QNetworkReply*>( sender() );
  if ( !reply )
    return;

  QNetworkReply::NetworkError error = reply->error();

  if ( error == QNetworkReply::NoError )
  {
    QString versionMessage = reply->readAll();
    QgsDebugMsg( QString( "version message: %1" ).arg( versionMessage ) );

    // strip the header
    QString contentFlag = "#QGIS Version";
    int pos = versionMessage.indexOf( contentFlag );
    if ( pos > -1 )
    {
      pos += contentFlag.length();
      QgsDebugMsg( QString( "Pos is %1" ).arg( pos ) );

      versionMessage = versionMessage.mid( pos );
      QStringList parts = versionMessage.split( "|", QString::SkipEmptyParts );
      // check the version from the  server against our version
      QString versionInfo;
      int currentVersion = parts[0].toInt();
      if ( currentVersion > QGis::QGIS_VERSION_INT )
      {
        // show version message from server
        versionInfo = tr( "There is a new version of QGIS available" ) + "\n";
      }
      else if ( QGis::QGIS_VERSION_INT > currentVersion )
      {
        versionInfo = tr( "You are running a development version of QGIS" ) + "\n";
      }
      else
      {
        versionInfo = tr( "You are running the current version of QGIS" ) + "\n";
      }

      if ( parts.count() > 1 )
      {
        versionInfo += parts[1] + "\n\n" + tr( "Would you like more information?" );

        QMessageBox::StandardButton result = QMessageBox::information( this,
                                             tr( "QGIS Version Information" ), versionInfo, QMessageBox::Ok |
                                             QMessageBox::Cancel );
        if ( result == QMessageBox::Ok )
        {
          // show more info
          QgsMessageViewer *mv = new QgsMessageViewer( this );
          mv->setWindowTitle( tr( "QGIS - Changes since last release" ) );
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
  else
  {
    // get error type
    QString detail;
    switch ( error )
    {
      case QNetworkReply::ConnectionRefusedError:
        detail = tr( "Connection refused - server may be down" );
        break;
      case QNetworkReply::HostNotFoundError:
        detail = tr( "QGIS server was not found" );
        break;
      default:
        detail = tr( "Unknown network socket error: %1" ).arg( error );
        break;
    }

    // show version message from server
    QMessageBox::critical( this, tr( "QGIS Version Information" ), tr( "Unable to communicate with QGIS Version server\n%1" ).arg( detail ) );
  }

  reply->deleteLater();
}

void QgisApp::configureShortcuts()
{
  QgsConfigureShortcutsDialog dlg;
  dlg.exec();
}

void QgisApp::customize()
{
  QgsCustomization::instance()->openDialog( this );
}


void QgisApp::options()
{
  if ( mMapCanvas && mMapCanvas->isDrawing() )
  {
    return;
  }

  QSettings mySettings;
  QString oldScales = mySettings.value( "Map/scales", PROJECT_SCALES ).toString();

  QgsOptions *optionsDialog = new QgsOptions( this );
  if ( optionsDialog->exec() )
  {
    // set the theme if it changed
    setTheme( optionsDialog->theme() );

    mMapCanvas->enableAntiAliasing( mySettings.value( "/qgis/enable_anti_aliasing" ).toBool() );
    mMapCanvas->useImageToRender( mySettings.value( "/qgis/use_qimage_to_render" ).toBool() );

    int action = mySettings.value( "/qgis/wheel_action", 2 ).toInt();
    double zoomFactor = mySettings.value( "/qgis/zoom_factor", 2 ).toDouble();
    mMapCanvas->setWheelAction(( QgsMapCanvas::WheelAction ) action, zoomFactor );

    //do we need this? TS
    mMapCanvas->refresh();

    mRasterFileFilter = QgsProviderRegistry::instance()->fileRasterFilters();

    if ( oldScales != mySettings.value( "Map/scales", PROJECT_SCALES ).toString() )
    {
      mScaleEdit->updateScales();
    }

    qobject_cast<QgsMeasureTool*>( mMapTools.mMeasureDist )->updateSettings();
    qobject_cast<QgsMeasureTool*>( mMapTools.mMeasureArea )->updateSettings();
    qobject_cast<QgsMapToolMeasureAngle*>( mMapTools.mMeasureAngle )->updateSettings();
  }

  delete optionsDialog;
}

void QgisApp::fullHistogramStretch()
{
  histogramStretch( false, QgsRaster::ContrastEnhancementMinMax );
}

void QgisApp::localHistogramStretch()
{
  histogramStretch( true, QgsRaster::ContrastEnhancementMinMax );
}

void QgisApp::fullCumulativeCutStretch()
{
  histogramStretch( false, QgsRaster::ContrastEnhancementCumulativeCut );
}

void QgisApp::localCumulativeCutStretch()
{
  histogramStretch( true, QgsRaster::ContrastEnhancementCumulativeCut );
}

void QgisApp::histogramStretch( bool visibleAreaOnly, QgsRaster::ContrastEnhancementLimits theLimits )
{
  QgsMapLayer * myLayer = mMapLegend->currentLayer();

  if ( !myLayer )
  {
    messageBar()->pushMessage( tr( "No Layer Selected" ),
                               tr( "To perform a full histogram stretch, you need to have a raster layer selected." ),
                               QgsMessageBar::INFO, messageTimeout() );
    return;
  }

  QgsRasterLayer* myRasterLayer = qobject_cast<QgsRasterLayer *>( myLayer );
  if ( !myRasterLayer )
  {
    messageBar()->pushMessage( tr( "No Layer Selected" ),
                               tr( "To perform a full histogram stretch, you need to have a raster layer selected." ),
                               QgsMessageBar::INFO, messageTimeout() );
    return;
  }

  QgsRectangle myRectangle;
  if ( visibleAreaOnly )
    myRectangle = mMapCanvas->mapRenderer()->outputExtentToLayerExtent( myRasterLayer, mMapCanvas->extent() );

  myRasterLayer->setContrastEnhancement( QgsContrastEnhancement::StretchToMinimumMaximum, theLimits, myRectangle );

  myRasterLayer->setCacheImage( NULL );
  mMapCanvas->refresh();
}

void QgisApp::increaseBrightness()
{
  adjustBrightnessContrast( 1 );
}

void QgisApp::decreaseBrightness()
{
  adjustBrightnessContrast( -1 );
}

void QgisApp::increaseContrast()
{
  adjustBrightnessContrast( 1, false );
}

void QgisApp::decreaseContrast()
{
  adjustBrightnessContrast( -1, false );
}


void QgisApp::adjustBrightnessContrast( int delta, bool updateBrightness )
{
  QgsMapLayer * myLayer = mMapLegend->currentLayer();

  if ( !myLayer )
  {
    messageBar()->pushMessage( tr( "No Layer Selected" ),
                               tr( "To change brightness or contrast, you need to have a raster layer selected." ),
                               QgsMessageBar::INFO, messageTimeout() );
    return;
  }

  QgsRasterLayer* myRasterLayer = qobject_cast<QgsRasterLayer *>( myLayer );
  if ( !myRasterLayer )
  {
    messageBar()->pushMessage( tr( "No Layer Selected" ),
                               tr( "To change brightness or contrast, you need to have a raster layer selected." ),
                               QgsMessageBar::INFO, messageTimeout() );
    return;
  }

  QgsBrightnessContrastFilter* brightnessFilter = myRasterLayer->brightnessFilter();

  if ( updateBrightness )
  {
    brightnessFilter->setBrightness( brightnessFilter->brightness() + delta );
  }
  else
  {
    brightnessFilter->setContrast( brightnessFilter->contrast() + delta );
  }

  myRasterLayer->setCacheImage( NULL );
  mMapCanvas->refresh();
}

void QgisApp::helpContents()
{
  // We should really ship the HTML version of the docs local too.
  openURL( QString( "http://docs.qgis.org/%1.%2/html/%3/docs/user_manual/" )
           .arg( QGis::QGIS_VERSION_INT / 10000 )
           .arg( QGis::QGIS_VERSION_INT / 100 % 100 )
           .arg( tr( "en", "documentation language" ) ),
           false );
}

void QgisApp::apiDocumentation()
{
  if ( QFileInfo( QgsApplication::pkgDataPath() + "/doc/api/index.html" ).exists() )
  {
    openURL( "api/index.html" );
  }
  else
  {
    openURL( "http://qgis.org/api/", false );
  }
}

void QgisApp::supportProviders()
{
  openURL( tr( "http://www.qgis.org/en/commercial-support.html" ), false );
}

void QgisApp::helpQgisHomePage()
{
  openURL( "http://qgis.org", false );
}

void QgisApp::openURL( QString url, bool useQgisDocDirectory )
{
  // open help in user browser
  if ( useQgisDocDirectory )
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
  if ( url.startsWith( "file://", Qt::CaseInsensitive ) )
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
  if ( !layer )
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
  if ( mMapCanvas && mMapCanvas->isDrawing() )
  {
    return NULL;
  }

  bool wasfrozen = mMapCanvas->isFrozen();

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

  if ( layer && layer->isValid() )
  {
    QStringList sublayers = layer->dataProvider()->subLayers();
    QgsDebugMsg( QString( "got valid layer with %1 sublayers" ).arg( sublayers.count() ) );

    // If the newly created layer has more than 1 layer of data available, we show the
    // sublayers selection dialog so the user can select the sublayers to actually load.
    if ( sublayers.count() > 1 )
    {
      askUserForOGRSublayers( layer );

      // The first layer loaded is not useful in that case. The user can select it in
      // the list if he wants to load it.
      delete layer;

    }
    else
    {
      // Register this layer with the layers registry
      QList<QgsMapLayer *> myList;
      myList << layer;
      QgsMapLayerRegistry::instance()->addMapLayers( myList );
      statusBar()->showMessage( mMapCanvas->extent().toString( 2 ) );
    }
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

  // Only update the map if we frozen in this method
  // Let the caller do it otherwise
  if ( !wasfrozen )
  {
    mMapCanvas->freeze( false );
    mMapCanvas->refresh();
  }

// Let render() do its own cursor management
//  QApplication::restoreOverrideCursor();

  return layer;

} // QgisApp::addVectorLayer



void QgisApp::addMapLayer( QgsMapLayer *theMapLayer )
{
  mMapCanvas->freeze();

// Let render() do its own cursor management
//  QApplication::setOverrideCursor(Qt::WaitCursor);

  if ( theMapLayer->isValid() )
  {
    // Register this layer with the layers registry
    QList<QgsMapLayer *> myList;
    myList << theMapLayer;
    QgsMapLayerRegistry::instance()->addMapLayers( myList );
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

void QgisApp::embedLayers()
{
  //dialog to select groups/layers from other project files
  QgsProjectLayerGroupDialog d( this );
  if ( d.exec() == QDialog::Accepted )
  {
    mMapCanvas->freeze( true );

    QString projectFile = d.selectedProjectFile();

    //groups
    QStringList groups = d.selectedGroups();
    QStringList::const_iterator groupIt = groups.constBegin();
    for ( ; groupIt != groups.constEnd(); ++groupIt )
    {
      mMapLegend->addEmbeddedGroup( *groupIt, projectFile );
    }

    //layer ids
    QList<QDomNode> brokenNodes;
    QList< QPair< QgsVectorLayer*, QDomElement > > vectorLayerList;
    QStringList layerIds = d.selectedLayerIds();
    QStringList::const_iterator layerIt = layerIds.constBegin();
    for ( ; layerIt != layerIds.constEnd(); ++layerIt )
    {
      QgsProject::instance()->createEmbeddedLayer( *layerIt, projectFile, brokenNodes, vectorLayerList );
    }

    mMapCanvas->freeze( false );
    if ( groups.size() > 0 || layerIds.size() > 0 )
    {
      mMapCanvas->refresh();
    }
  }
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
  QString whyDirty = "";
  bool hasUnsavedEdits = false;
  // extra check to see if there are any vector layers with unsaved provider edits
  // to ensure user has opportunity to save any editing
  if ( QgsMapLayerRegistry::instance()->count() > 0 )
  {
    QMap<QString, QgsMapLayer*> layers = QgsMapLayerRegistry::instance()->mapLayers();
    for ( QMap<QString, QgsMapLayer*>::iterator it = layers.begin(); it != layers.end(); it++ )
    {
      QgsVectorLayer *vl = qobject_cast<QgsVectorLayer *>( it.value() );
      if ( !vl )
      {
        continue;
      }

      hasUnsavedEdits = ( vl->isEditable() && vl->isModified() );
      if ( hasUnsavedEdits )
      {
        break;
      }
    }

    if ( hasUnsavedEdits )
    {
      markDirty();
      whyDirty = "<p style='color:darkred;'>";
      whyDirty += tr( "Project has layer(s) in edit mode with unsaved edits, which will NOT be saved!" );
      whyDirty += "</p>";
    }
  }

  QMessageBox::StandardButton answer( QMessageBox::Discard );
  mMapCanvas->freeze( true );

  //QgsDebugMsg(QString("Layer count is %1").arg(mMapCanvas->layerCount()));
  //QgsDebugMsg(QString("Project is %1dirty").arg( QgsProject::instance()->isDirty() ? "" : "not "));
  //QgsDebugMsg(QString("Map canvas is %1dirty").arg(mMapCanvas->isDirty() ? "" : "not "));

  QSettings settings;
  bool askThem = settings.value( "qgis/askToSaveProjectChanges", true ).toBool();

  if ( askThem && ( QgsProject::instance()->isDirty() || mMapCanvas->isDirty() ) && QgsMapLayerRegistry::instance()->count() > 0 )
  {
    // flag project as dirty since dirty state of canvas is reset if "dirty"
    // is based on a zoom or pan
    markDirty();

    // old code: mProjectIsDirtyFlag = true;

    // prompt user to save
    answer = QMessageBox::information( this, tr( "Save?" ),
                                       tr( "Do you want to save the current project?%1" )
                                       .arg( whyDirty ),
                                       QMessageBox::Save | QMessageBox::Cancel | QMessageBox::Discard,
                                       hasUnsavedEdits ? QMessageBox::Cancel : QMessageBox::Save );
    if ( QMessageBox::Save == answer )
    {
      if ( !fileSave() )
        answer = QMessageBox::Cancel;
    }
  }

  mMapCanvas->freeze( false );

  return answer != QMessageBox::Cancel;
} // QgisApp::saveDirty()

void QgisApp::closeProject()
{
  // unload the project macros before changing anything
  if ( mTrustedMacros )
  {
    QgsPythonRunner::run( "qgis.utils.unloadProjectMacros();" );
  }

  // remove any message widgets from the message bar
  mInfoBar->clearWidgets();

  mTrustedMacros = false;

  deletePrintComposers();
  removeAnnotationItems();
  // clear out any stuff from project
  mMapCanvas->freeze( true );
  removeAllLayers();
}


void QgisApp::changeEvent( QEvent* event )
{
  QMainWindow::changeEvent( event );
#ifdef Q_WS_MAC
  switch ( event->type() )
  {
    case QEvent::ActivationChange:
      if ( QApplication::activeWindow() == this )
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
  if ( !mActionPluginSeparator1 )
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
    for ( int i = actions.indexOf( mActionPluginSeparator1 ) + 1; i < end; i++ )
    {
      QString src = actions.at( i )->text();
      src.remove( QChar( '&' ) );

      int comp = dst.localeAwareCompare( src );
      if ( comp < 0 )
      {
        // Add item before this one
        before = actions.at( i );
        break;
      }
      else if ( comp == 0 )
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
  if ( menu->actions().count() == 0 )
  {
    mPluginMenu->removeAction( menu->menuAction() );
  }
  // Remove separator above plugins in Plugin menu if no plugins remain
  QList<QAction*> actions = mPluginMenu->actions();
  int end = mActionPluginSeparator2 ? actions.indexOf( mActionPluginSeparator2 ) : actions.count();
  if ( actions.indexOf( mActionPluginSeparator1 ) + 1 == end )
  {
    mPluginMenu->removeAction( mActionPluginSeparator1 );
    mActionPluginSeparator1 = NULL;
  }
}

QMenu* QgisApp::getDatabaseMenu( QString menuName )
{
#ifdef Q_WS_MAC
  // Mac doesn't have '&' keyboard shortcuts.
  menuName.remove( QChar( '&' ) );
#endif
  QString dst = menuName;
  dst.remove( QChar( '&' ) );

  QAction *before = NULL;
  QList<QAction*> actions = mDatabaseMenu->actions();
  for ( int i = 0; i < actions.count(); i++ )
  {
    QString src = actions.at( i )->text();
    src.remove( QChar( '&' ) );

    int comp = dst.localeAwareCompare( src );
    if ( comp < 0 )
    {
      // Add item before this one
      before = actions.at( i );
      break;
    }
    else if ( comp == 0 )
    {
      // Plugin menu item already exists
      return actions.at( i )->menu();
    }
  }
  // It doesn't exist, so create
  QMenu *menu = new QMenu( menuName, this );
  if ( before )
    mDatabaseMenu->insertMenu( before, menu );
  else
    mDatabaseMenu->addMenu( menu );

  return menu;
}

QMenu* QgisApp::getRasterMenu( QString menuName )
{
#ifdef Q_WS_MAC
  // Mac doesn't have '&' keyboard shortcuts.
  menuName.remove( QChar( '&' ) );
#endif

  QAction *before = NULL;
  if ( !mActionRasterSeparator )
  {
    // First plugin - create plugin list separator
    mActionRasterSeparator = mRasterMenu->insertSeparator( before );
  }
  else
  {
    QString dst = menuName;
    dst.remove( QChar( '&' ) );
    // Plugins exist - search between plugin separator and python separator or end of list
    QList<QAction*> actions = mRasterMenu->actions();
    for ( int i = actions.indexOf( mActionRasterSeparator ) + 1; i < actions.count(); i++ )
    {
      QString src = actions.at( i )->text();
      src.remove( QChar( '&' ) );

      int comp = dst.localeAwareCompare( src );
      if ( comp < 0 )
      {
        // Add item before this one
        before = actions.at( i );
        break;
      }
      else if ( comp == 0 )
      {
        // Plugin menu item already exists
        return actions.at( i )->menu();
      }
    }
  }

  // It doesn't exist, so create
  QMenu *menu = new QMenu( menuName, this );
  if ( before )
    mRasterMenu->insertMenu( before, menu );
  else
    mRasterMenu->addMenu( menu );

  return menu;
}

QMenu* QgisApp::getVectorMenu( QString menuName )
{
#ifdef Q_WS_MAC
  // Mac doesn't have '&' keyboard shortcuts.
  menuName.remove( QChar( '&' ) );
#endif
  QString dst = menuName;
  dst.remove( QChar( '&' ) );

  QAction *before = NULL;
  QList<QAction*> actions = mVectorMenu->actions();
  for ( int i = 0; i < actions.count(); i++ )
  {
    QString src = actions.at( i )->text();
    src.remove( QChar( '&' ) );

    int comp = dst.localeAwareCompare( src );
    if ( comp < 0 )
    {
      // Add item before this one
      before = actions.at( i );
      break;
    }
    else if ( comp == 0 )
    {
      // Plugin menu item already exists
      return actions.at( i )->menu();
    }
  }
  // It doesn't exist, so create
  QMenu *menu = new QMenu( menuName, this );
  if ( before )
    mVectorMenu->insertMenu( before, menu );
  else
    mVectorMenu->addMenu( menu );

  return menu;
}

QMenu* QgisApp::getWebMenu( QString menuName )
{
#ifdef Q_WS_MAC
  // Mac doesn't have '&' keyboard shortcuts.
  menuName.remove( QChar( '&' ) );
#endif
  QString dst = menuName;
  dst.remove( QChar( '&' ) );

  QAction *before = NULL;
  QList<QAction*> actions = mWebMenu->actions();
  for ( int i = 0; i < actions.count(); i++ )
  {
    QString src = actions.at( i )->text();
    src.remove( QChar( '&' ) );

    int comp = dst.localeAwareCompare( src );
    if ( comp < 0 )
    {
      // Add item before this one
      before = actions.at( i );
      break;
    }
    else if ( comp == 0 )
    {
      // Plugin menu item already exists
      return actions.at( i )->menu();
    }
  }
  // It doesn't exist, so create
  QMenu *menu = new QMenu( menuName, this );
  if ( before )
    mWebMenu->insertMenu( before, menu );
  else
    mWebMenu->addMenu( menu );

  return menu;
}

void QgisApp::insertAddLayerAction( QAction *action )
{
  mLayerMenu->insertAction( mActionAddLayerSeparator, action );
}

void QgisApp::removeAddLayerAction( QAction *action )
{
  mLayerMenu->removeAction( action );
}

void QgisApp::addPluginToDatabaseMenu( QString name, QAction* action )
{
  QMenu* menu = getDatabaseMenu( name );
  menu->addAction( action );

  // add the Database menu to the menuBar if not added yet
  if ( mDatabaseMenu->actions().count() != 1 )
    return;

  QAction* before = NULL;
  QList<QAction*> actions = menuBar()->actions();
  for ( int i = 0; i < actions.count(); i++ )
  {
    if ( actions.at( i )->menu() == mDatabaseMenu )
      return;

    // goes before Web menu, if present
    if ( actions.at( i )->menu() == mWebMenu )
    {
      before = actions.at( i );
      break;
    }
  }
  for ( int i = 0; i < actions.count(); i++ )
  {
    // defaults to after Raster menu, which is already in qgisapp.ui
    if ( actions.at( i )->menu() == mRasterMenu )
    {
      if ( !before )
      {
        before = actions.at( i += 1 );
        break;
      }
    }
  }
  if ( before )
    menuBar()->insertMenu( before, mDatabaseMenu );
  else
    // fallback insert
    menuBar()->insertMenu( firstRightStandardMenu()->menuAction(), mDatabaseMenu );
}

void QgisApp::addPluginToRasterMenu( QString name, QAction* action )
{
  QMenu* menu = getRasterMenu( name );
  menu->addAction( action );
}

void QgisApp::addPluginToVectorMenu( QString name, QAction* action )
{
  QMenu* menu = getVectorMenu( name );
  menu->addAction( action );
}

void QgisApp::addPluginToWebMenu( QString name, QAction* action )
{
  QMenu* menu = getWebMenu( name );
  menu->addAction( action );

  // add the Vector menu to the menuBar if not added yet
  if ( mWebMenu->actions().count() != 1 )
    return;

  QAction* before = NULL;
  QList<QAction*> actions = menuBar()->actions();
  for ( int i = 0; i < actions.count(); i++ )
  {
    // goes after Database menu, if present
    if ( actions.at( i )->menu() == mDatabaseMenu )
    {
      before = actions.at( i += 1 );
      // don't break here
    }

    if ( actions.at( i )->menu() == mWebMenu )
      return;
  }
  for ( int i = 0; i < actions.count(); i++ )
  {
    // defaults to after Raster menu, which is already in qgisapp.ui
    if ( actions.at( i )->menu() == mRasterMenu )
    {
      if ( !before )
      {
        before = actions.at( i += 1 );
        break;
      }
    }
  }

  if ( before )
    menuBar()->insertMenu( before, mWebMenu );
  else
    // fallback insert
    menuBar()->insertMenu( firstRightStandardMenu()->menuAction(), mWebMenu );
}

void QgisApp::removePluginDatabaseMenu( QString name, QAction* action )
{
  QMenu* menu = getDatabaseMenu( name );
  menu->removeAction( action );
  if ( menu->actions().count() == 0 )
  {
    mDatabaseMenu->removeAction( menu->menuAction() );
  }

  // remove the Database menu from the menuBar if there are no more actions
  if ( mDatabaseMenu->actions().count() > 0 )
    return;

  QList<QAction*> actions = menuBar()->actions();
  for ( int i = 0; i < actions.count(); i++ )
  {
    if ( actions.at( i )->menu() == mDatabaseMenu )
    {
      menuBar()->removeAction( actions.at( i ) );
      return;
    }
  }
}

void QgisApp::removePluginRasterMenu( QString name, QAction* action )
{
  QMenu* menu = getRasterMenu( name );
  menu->removeAction( action );
  if ( menu->actions().count() == 0 )
  {
    mRasterMenu->removeAction( menu->menuAction() );
  }

  // Remove separator above plugins in Raster menu if no plugins remain
  QList<QAction*> actions = mRasterMenu->actions();
  if ( actions.indexOf( mActionRasterSeparator ) + 1 == actions.count() )
  {
    mRasterMenu->removeAction( mActionRasterSeparator );
    mActionRasterSeparator = NULL;
  }
}

void QgisApp::removePluginVectorMenu( QString name, QAction* action )
{
  QMenu* menu = getVectorMenu( name );
  menu->removeAction( action );
  if ( menu->actions().count() == 0 )
  {
    mVectorMenu->removeAction( menu->menuAction() );
  }

  // remove the Vector menu from the menuBar if there are no more actions
  if ( mVectorMenu->actions().count() > 0 )
    return;

  QList<QAction*> actions = menuBar()->actions();
  for ( int i = 0; i < actions.count(); i++ )
  {
    if ( actions.at( i )->menu() == mVectorMenu )
    {
      menuBar()->removeAction( actions.at( i ) );
      return;
    }
  }
}

void QgisApp::removePluginWebMenu( QString name, QAction* action )
{
  QMenu* menu = getWebMenu( name );
  menu->removeAction( action );
  if ( menu->actions().count() == 0 )
  {
    mWebMenu->removeAction( menu->menuAction() );
  }

  // remove the Web menu from the menuBar if there are no more actions
  if ( mWebMenu->actions().count() > 0 )
    return;

  QList<QAction*> actions = menuBar()->actions();
  for ( int i = 0; i < actions.count(); i++ )
  {
    if ( actions.at( i )->menu() == mWebMenu )
    {
      menuBar()->removeAction( actions.at( i ) );
      return;
    }
  }
}

int QgisApp::addPluginToolBarIcon( QAction * qAction )
{
  mPluginToolBar->addAction( qAction );
  return 0;
}

QAction*QgisApp::addPluginToolBarWidget( QWidget* widget )
{
  return mPluginToolBar->addWidget( widget );
}

void QgisApp::removePluginToolBarIcon( QAction *qAction )
{
  mPluginToolBar->removeAction( qAction );
}

int QgisApp::addRasterToolBarIcon( QAction * qAction )
{
  mRasterToolBar->addAction( qAction );
  return 0;
}

QAction*QgisApp::addRasterToolBarWidget( QWidget* widget )
{
  return mRasterToolBar->addWidget( widget );
}

void QgisApp::removeRasterToolBarIcon( QAction *qAction )
{
  mRasterToolBar->removeAction( qAction );
}

int QgisApp::addVectorToolBarIcon( QAction * qAction )
{
  mVectorToolBar->addAction( qAction );
  return 0;
}

QAction*QgisApp::addVectorToolBarWidget( QWidget* widget )
{
  return mVectorToolBar->addWidget( widget );
}

void QgisApp::removeVectorToolBarIcon( QAction *qAction )
{
  mVectorToolBar->removeAction( qAction );
}

int QgisApp::addDatabaseToolBarIcon( QAction * qAction )
{
  mDatabaseToolBar->addAction( qAction );
  return 0;
}

QAction*QgisApp::addDatabaseToolBarWidget( QWidget* widget )
{
  return mDatabaseToolBar->addWidget( widget );
}

void QgisApp::removeDatabaseToolBarIcon( QAction *qAction )
{
  mDatabaseToolBar->removeAction( qAction );
}

int QgisApp::addWebToolBarIcon( QAction * qAction )
{
  mWebToolBar->addAction( qAction );
  return 0;
}

QAction*QgisApp::addWebToolBarWidget( QWidget* widget )
{
  return mWebToolBar->addWidget( widget );
}

void QgisApp::removeWebToolBarIcon( QAction *qAction )
{
  mWebToolBar->removeAction( qAction );
}

void QgisApp::updateCRSStatusBar()
{
  mOnTheFlyProjectionStatusLabel->setText( mMapCanvas->mapRenderer()->destinationCrs().authid() );

  if ( mMapCanvas->mapRenderer()->hasCrsTransformEnabled() )
  {
    mOnTheFlyProjectionStatusLabel->setEnabled( true );
    mOnTheFlyProjectionStatusLabel->setToolTip(
      tr( "Current CRS: %1 (OTFR enabled)" ).arg( mMapCanvas->mapRenderer()->destinationCrs().description() ) );
    mOnTheFlyProjectionStatusButton->setIcon( QgsApplication::getThemeIcon( "mIconProjectionEnabled.png" ) );
  }
  else
  {
    mOnTheFlyProjectionStatusLabel->setEnabled( false );
    mOnTheFlyProjectionStatusLabel->setToolTip(
      tr( "Current CRS: %1 (OTFR disabled)" ).arg( mMapCanvas->mapRenderer()->destinationCrs().description() ) );
    mOnTheFlyProjectionStatusButton->setIcon( QgsApplication::getThemeIcon( "mIconProjectionDisabled.png" ) );
  }
}

void QgisApp::destinationSrsChanged()
{
  // save this information to project
  long srsid = mMapCanvas->mapRenderer()->destinationCrs().srsid();
  QgsProject::instance()->writeEntry( "SpatialRefSys", "/ProjectCRSID", ( int )srsid );
  updateCRSStatusBar();
}

void QgisApp::hasCrsTransformEnabled( bool theFlag )
{
  // save this information to project
  QgsProject::instance()->writeEntry( "SpatialRefSys", "/ProjectionsEnabled", ( theFlag ? 1 : 0 ) );
  updateCRSStatusBar();
}
// slot to update the progress bar in the status bar
void QgisApp::showProgress( int theProgress, int theTotalSteps )
{
  if ( theProgress == theTotalSteps )
  {
    mProgressBar->reset();
    mProgressBar->hide();
  }
  else
  {
    //only call show if not already hidden to reduce flicker
    if ( !mProgressBar->isVisible() )
    {
      mProgressBar->show();
    }
    mProgressBar->setMaximum( theTotalSteps );
    mProgressBar->setValue( theProgress );
  }
}

void QgisApp::mapToolChanged( QgsMapTool *tool )
{
  if ( tool && !tool->isEditTool() )
  {
    mNonEditMapTool = tool;
  }
}

void QgisApp::extentsViewToggled( bool theFlag )
{
  if ( theFlag )
  {
    //extents view mode!
    mToggleExtentsViewButton->setIcon( QgsApplication::getThemeIcon( "extents.png" ) );
    mCoordsEdit->setToolTip( tr( "Map coordinates for the current view extents" ) );
    mCoordsEdit->setReadOnly( true );
    showExtents();
  }
  else
  {
    //mouse cursor pos view mode!
    mToggleExtentsViewButton->setIcon( QgsApplication::getThemeIcon( "tracking.png" ) );
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

//changed from layerWasAdded to layersWereAdded in 1.8
void QgisApp::layersWereAdded( QList<QgsMapLayer *> theLayers )
{
  for ( int i = 0; i < theLayers.size(); ++i )
  {
    QgsMapLayer * layer = theLayers.at( i );
    QgsDataProvider *provider = 0;

    QgsVectorLayer *vlayer = qobject_cast<QgsVectorLayer *>( layer );
    if ( vlayer )
    {
      // notify user about any font family substitution, but only when rendering labels (i.e. not when opening settings dialog)
      connect( vlayer, SIGNAL( labelingFontNotFound( QgsVectorLayer*, QString ) ), this, SLOT( labelingFontNotFound( QgsVectorLayer*, QString ) ) );

      QgsVectorDataProvider* vProvider = vlayer->dataProvider();
      if ( vProvider && vProvider->capabilities() & QgsVectorDataProvider::EditingCapabilities )
      {
        connect( vlayer, SIGNAL( layerModified() ), this, SLOT( updateLayerModifiedActions() ) );
        connect( vlayer, SIGNAL( editingStarted() ), this, SLOT( layerEditStateChanged() ) );
        connect( vlayer, SIGNAL( editingStopped() ), this, SLOT( layerEditStateChanged() ) );
      }
      provider = vProvider;
    }

    QgsRasterLayer *rlayer = qobject_cast<QgsRasterLayer *>( layer );
    if ( rlayer )
    {
      // connect up any request the raster may make to update the app progress
      connect( rlayer, SIGNAL( drawingProgress( int, int ) ), this, SLOT( showProgress( int, int ) ) );

      // connect up any request the raster may make to update the statusbar message
      connect( rlayer, SIGNAL( statusChanged( QString ) ), this, SLOT( showStatusMessage( QString ) ) );

      provider = rlayer->dataProvider();
    }

    if ( provider )
    {
      connect( provider, SIGNAL( dataChanged() ), layer, SLOT( clearCacheImage() ) );
      connect( provider, SIGNAL( dataChanged() ), mMapCanvas, SLOT( refresh() ) );
    }
  }
}

void QgisApp::showExtents()
{
  if ( !mToggleExtentsViewButton->isChecked() )
  {
    return;
  }

  // update the statusbar with the current extents.
  QgsRectangle myExtents = mMapCanvas->extent();
  mCoordsLabel->setText( tr( "Extents:" ) );
  mCoordsEdit->setText( myExtents.toString( true ) );
  //ensure the label is big enough
  if ( mCoordsEdit->width() > mCoordsEdit->minimumWidth() )
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

  if ( automatic )
  {
    // Work out a suitable number of decimal places for the mouse
    // coordinates with the aim of always having enough decimal places
    // to show the difference in position between adjacent pixels.
    // Also avoid taking the log of 0.
    if ( mapCanvas()->mapUnitsPerPixel() != 0.0 )
      dp = static_cast<int>( ceil( -1.0 * log10( mapCanvas()->mapUnitsPerPixel() ) ) );
  }
  else
    dp = QgsProject::instance()->readNumEntry( "PositionPrecision", "/DecimalPlaces" );

  // Keep dp sensible
  if ( dp < 0 )
    dp = 0;

  mMousePrecisionDecimalPlaces = dp;
}

void QgisApp::showStatusMessage( QString theMessage )
{
  statusBar()->showMessage( theMessage );
}

// Show the maptip using tooltip
void QgisApp::showMapTip()
{
  // Stop the timer while we look for a maptip
  mpMapTipsTimer->stop();

  // Only show tooltip if the mouse is over the canvas
  if ( mMapCanvas->underMouse() )
  {
    QPoint myPointerPos = mMapCanvas->mouseLastXY();

    //  Make sure there is an active layer before proceeding
    QgsMapLayer* mypLayer = mMapCanvas->currentLayer();
    if ( mypLayer )
    {
      //QgsDebugMsg("Current layer for maptip display is: " + mypLayer->source());
      // only process vector layers
      if ( mypLayer->type() == QgsMapLayer::VectorLayer )
      {
        // Show the maptip if the maptips button is depressed
        if ( mMapTipsVisible )
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
  if ( mMapCanvas && mMapCanvas->isDrawing() )
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
  if ( mShowProjectionTab )
  {
    pp->showProjectionsTab();
    mShowProjectionTab = false;
  }
  qApp->processEvents();
  // Be told if the mouse display precision may have changed by the user
  // changing things in the project properties dialog box
  connect( pp, SIGNAL( displayPrecisionChanged() ), this,
           SLOT( updateMouseCoordinatePrecision() ) );

  connect( pp, SIGNAL( scalesChanged( const QStringList & ) ), mScaleEdit,
           SLOT( updateScales( const QStringList & ) ) );
  QApplication::restoreOverrideCursor();

  //pass any refresh signals off to canvases
  // Line below was commented out by wonder three years ago (r4949).
  // It is needed to refresh scale bar after changing display units.
  connect( pp, SIGNAL( refresh() ), mMapCanvas, SLOT( refresh() ) );

  QgsMapRenderer* myRender = mMapCanvas->mapRenderer();
  bool wasProjected = myRender->hasCrsTransformEnabled();
  long oldCRSID = myRender->destinationCrs().srsid();

  // Display the modal dialog box.
  pp->exec();

  long newCRSID = myRender->destinationCrs().srsid();
  bool isProjected = myRender->hasCrsTransformEnabled();

  // projections have been turned on/off or dest CRS has changed while projections are on
  if ( wasProjected != isProjected || ( isProjected && oldCRSID != newCRSID ) )
  {
    // TODO: would be better to try to reproject current extent to the new one
    mMapCanvas->updateFullExtent();
  }

  int  myRedInt = QgsProject::instance()->readNumEntry( "Gui", "/CanvasColorRedPart", 255 );
  int  myGreenInt = QgsProject::instance()->readNumEntry( "Gui", "/CanvasColorGreenPart", 255 );
  int  myBlueInt = QgsProject::instance()->readNumEntry( "Gui", "/CanvasColorBluePart", 255 );
  QColor myColor = QColor( myRedInt, myGreenInt, myBlueInt );
  mMapCanvas->setCanvasColor( myColor ); // this is fill color before rendering onto canvas

  qobject_cast<QgsMeasureTool*>( mMapTools.mMeasureDist )->updateSettings();
  qobject_cast<QgsMeasureTool*>( mMapTools.mMeasureArea )->updateSettings();
  qobject_cast<QgsMapToolMeasureAngle*>( mMapTools.mMeasureAngle )->updateSettings();

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
  if ( vlayer )
  {
    showStatusMessage( tr( "%n feature(s) selected on layer %1.", "number of selected features", vlayer->selectedFeatureCount() ).arg( vlayer->name() ) );
  }
  activateDeactivateLayerRelatedActions( layer );
}

void QgisApp::legendLayerSelectionChanged( void )
{
  mActionRemoveLayer->setEnabled( mMapLegend && mMapLegend->selectedLayers().size() > 0 );
  mActionDuplicateLayer->setEnabled( mMapLegend && mMapLegend->selectedLayers().size() > 0 );
  mActionSetLayerCRS->setEnabled( mMapLegend && mMapLegend->selectedLayers().size() > 0 );
  mActionSetProjectCRSFromLayer->setEnabled( mMapLegend && mMapLegend->selectedLayers().size() == 1 );

  mActionSaveEdits->setEnabled( mMapLegend && mMapLegend->selectedLayersEditable( true ) );
  mActionRollbackEdits->setEnabled( mMapLegend && mMapLegend->selectedLayersEditable( true ) );
  mActionCancelEdits->setEnabled( mMapLegend && mMapLegend->selectedLayersEditable() );
}

void QgisApp::layerEditStateChanged()
{
  QgsMapLayer* layer = qobject_cast<QgsMapLayer *>( sender() );
  if ( layer && layer == activeLayer() )
  {
    activateDeactivateLayerRelatedActions( layer );
    mSaveRollbackInProgress = false;
  }
}

void QgisApp::activateDeactivateLayerRelatedActions( QgsMapLayer* layer )
{
  bool enableMove = false, enableRotate = false, enablePin = false, enableShowHide = false, enableChange = false;

  QMap<QString, QgsMapLayer*> layers = QgsMapLayerRegistry::instance()->mapLayers();
  for ( QMap<QString, QgsMapLayer*>::iterator it = layers.begin(); it != layers.end(); it++ )
  {
    QgsVectorLayer *vlayer = qobject_cast<QgsVectorLayer *>( it.value() );
    if ( !vlayer || !vlayer->isEditable() ||
         ( !vlayer->diagramRenderer() && vlayer->customProperty( "labeling" ).toString() != QString( "pal" ) ) )
      continue;

    int colX, colY, colShow, colAng;
    enablePin =
      enablePin ||
      ( qobject_cast<QgsMapToolPinLabels*>( mMapTools.mPinLabels ) &&
        qobject_cast<QgsMapToolPinLabels*>( mMapTools.mPinLabels )->layerCanPin( vlayer, colX, colY ) );

    enableShowHide =
      enableShowHide ||
      ( qobject_cast<QgsMapToolShowHideLabels*>( mMapTools.mShowHideLabels ) &&
        qobject_cast<QgsMapToolShowHideLabels*>( mMapTools.mShowHideLabels )->layerCanShowHide( vlayer, colShow ) );

    enableMove =
      enableMove ||
      ( qobject_cast<QgsMapToolMoveLabel*>( mMapTools.mMoveLabel ) &&
        ( qobject_cast<QgsMapToolMoveLabel*>( mMapTools.mMoveLabel )->labelMoveable( vlayer, colX, colY )
          || qobject_cast<QgsMapToolMoveLabel*>( mMapTools.mMoveLabel )->diagramMoveable( vlayer, colX, colY ) )
      );

    enableRotate =
      enableRotate ||
      ( qobject_cast<QgsMapToolRotateLabel*>( mMapTools.mRotateLabel ) &&
        qobject_cast<QgsMapToolRotateLabel*>( mMapTools.mRotateLabel )->layerIsRotatable( vlayer, colAng ) );

    enableChange = true;

    if ( enablePin && enableShowHide && enableMove && enableRotate && enableChange )
      break;
  }

  mActionPinLabels->setEnabled( enablePin );
  mActionShowHideLabels->setEnabled( enableShowHide );
  mActionMoveLabel->setEnabled( enableMove );
  mActionRotateLabel->setEnabled( enableRotate );
  mActionChangeLabelProperties->setEnabled( enableChange );

  updateLayerModifiedActions();

  if ( !layer )
  {
    mActionSelect->setEnabled( false );
    mActionSelectRectangle->setEnabled( false );
    mActionSelectPolygon->setEnabled( false );
    mActionSelectFreehand->setEnabled( false );
    mActionSelectRadius->setEnabled( false );
    mActionIdentify->setEnabled( QSettings().value( "/Map/identifyMode", 0 ).toInt() != 0 );
    mActionSelectByExpression->setEnabled( false );
    mActionLabeling->setEnabled( false );
    mActionOpenTable->setEnabled( false );
    mActionOpenFieldCalc->setEnabled( false );
    mActionToggleEditing->setEnabled( false );
    mActionToggleEditing->setChecked( false );
    mActionSaveLayerEdits->setEnabled( false );
    mActionLayerSaveAs->setEnabled( false );
    mActionLayerSelectionSaveAs->setEnabled( false );
    mActionLayerProperties->setEnabled( false );
    mActionLayerSubsetString->setEnabled( false );
    mActionAddToOverview->setEnabled( false );
    mActionFeatureAction->setEnabled( false );
    mActionAddFeature->setEnabled( false );
    mActionMoveFeature->setEnabled( false );
    mActionRotateFeature->setEnabled( false );
    mActionOffsetCurve->setEnabled( false );
    mActionNodeTool->setEnabled( false );
    mActionDeleteSelected->setEnabled( false );
    mActionCutFeatures->setEnabled( false );
    mActionCopyFeatures->setEnabled( false );
    mActionPasteFeatures->setEnabled( false );
    mActionCopyStyle->setEnabled( false );
    mActionPasteStyle->setEnabled( false );

    mUndoWidget->dockContents()->setEnabled( false );
    mActionUndo->setEnabled( false );
    mActionRedo->setEnabled( false );
    mActionSimplifyFeature->setEnabled( false );
    mActionAddRing->setEnabled( false );
    mActionAddPart->setEnabled( false );
    mActionDeleteRing->setEnabled( false );
    mActionDeletePart->setEnabled( false );
    mActionReshapeFeatures->setEnabled( false );
    mActionOffsetCurve->setEnabled( false );
    mActionSplitFeatures->setEnabled( false );
    mActionMergeFeatures->setEnabled( false );
    mActionMergeFeatureAttributes->setEnabled( false );
    mActionRotatePointSymbols->setEnabled( false );

    mActionPinLabels->setEnabled( false );
    mActionShowHideLabels->setEnabled( false );
    mActionMoveLabel->setEnabled( false );
    mActionRotateLabel->setEnabled( false );
    mActionChangeLabelProperties->setEnabled( false );

    mActionLocalHistogramStretch->setEnabled( false );
    mActionFullHistogramStretch->setEnabled( false );
    mActionLocalCumulativeCutStretch->setEnabled( false );
    mActionFullCumulativeCutStretch->setEnabled( false );
    mActionIncreaseBrightness->setEnabled( false );
    mActionDecreaseBrightness->setEnabled( false );
    mActionIncreaseContrast->setEnabled( false );
    mActionDecreaseContrast->setEnabled( false );
    mActionZoomActualSize->setEnabled( false );
    mActionZoomToLayer->setEnabled( false );
    return;
  }

  mActionLayerProperties->setEnabled( true );
  mActionAddToOverview->setEnabled( true );
  mActionZoomToLayer->setEnabled( true );

  mActionCopyStyle->setEnabled( true );
  mActionPasteStyle->setEnabled( clipboard()->hasFormat( QGSCLIPBOARD_STYLE_MIME ) );

  /***********Vector layers****************/
  if ( layer->type() == QgsMapLayer::VectorLayer )
  {
    QgsVectorLayer* vlayer = qobject_cast<QgsVectorLayer *>( layer );
    QgsVectorDataProvider* dprovider = vlayer->dataProvider();

    bool isEditable = vlayer->isEditable();
    bool layerHasSelection = vlayer->selectedFeatureCount() > 0;
    bool layerHasActions = vlayer->actions()->size() > 0;

    bool canChangeAttributes = dprovider->capabilities() & QgsVectorDataProvider::ChangeAttributeValues;
    bool canDeleteFeatures = dprovider->capabilities() & QgsVectorDataProvider::DeleteFeatures;
    bool canAddAttributes = dprovider->capabilities() & QgsVectorDataProvider::AddAttributes;
    bool canAddFeatures = dprovider->capabilities() & QgsVectorDataProvider::AddFeatures;
    bool canSupportEditing = dprovider->capabilities() & QgsVectorDataProvider::EditingCapabilities;
    bool canChangeGeometry = dprovider->capabilities() & QgsVectorDataProvider::ChangeGeometries;

    mActionLocalHistogramStretch->setEnabled( false );
    mActionFullHistogramStretch->setEnabled( false );
    mActionLocalCumulativeCutStretch->setEnabled( false );
    mActionFullCumulativeCutStretch->setEnabled( false );
    mActionIncreaseBrightness->setEnabled( false );
    mActionDecreaseBrightness->setEnabled( false );
    mActionIncreaseContrast->setEnabled( false );
    mActionDecreaseContrast->setEnabled( false );
    mActionZoomActualSize->setEnabled( false );
    mActionLabeling->setEnabled( true );

    mActionSelect->setEnabled( true );
    mActionSelectRectangle->setEnabled( true );
    mActionSelectPolygon->setEnabled( true );
    mActionSelectFreehand->setEnabled( true );
    mActionSelectRadius->setEnabled( true );
    mActionIdentify->setEnabled( true );
    mActionSelectByExpression->setEnabled( true );
    mActionOpenTable->setEnabled( true );
    mActionLayerSaveAs->setEnabled( true );
    mActionLayerSelectionSaveAs->setEnabled( true );
    mActionCopyFeatures->setEnabled( layerHasSelection );
    mActionFeatureAction->setEnabled( layerHasActions );

    if ( !isEditable && mMapCanvas->mapTool()
         && mMapCanvas->mapTool()->isEditTool() && !mSaveRollbackInProgress )
    {
      mMapCanvas->setMapTool( mNonEditMapTool );
    }

    if ( dprovider )
    {
      mActionLayerSubsetString->setEnabled( !isEditable && dprovider->supportsSubsetString() );

      mActionToggleEditing->setEnabled( canSupportEditing && !vlayer->isReadOnly() );
      mActionToggleEditing->setChecked( canSupportEditing && isEditable );
      mActionSaveLayerEdits->setEnabled( canSupportEditing && isEditable && vlayer->isModified() );
      mUndoWidget->dockContents()->setEnabled( canSupportEditing && isEditable );
      mActionUndo->setEnabled( canSupportEditing );
      mActionRedo->setEnabled( canSupportEditing );

      //start editing/stop editing
      if ( canSupportEditing )
      {
        updateUndoActions();
      }

      mActionPasteFeatures->setEnabled( isEditable && canAddFeatures && !clipboard()->empty() );
      mActionAddFeature->setEnabled( isEditable && canAddFeatures );

      //does provider allow deleting of features?
      mActionDeleteSelected->setEnabled( isEditable && canDeleteFeatures && layerHasSelection );
      mActionCutFeatures->setEnabled( isEditable && canDeleteFeatures && layerHasSelection );

      //merge tool needs editable layer and provider with the capability of adding and deleting features
      if ( isEditable && canChangeAttributes )
      {
        mActionMergeFeatures->setEnabled( layerHasSelection && canDeleteFeatures && canAddFeatures );
        mActionMergeFeatureAttributes->setEnabled( layerHasSelection );
      }
      else
      {
        mActionMergeFeatures->setEnabled( false );
        mActionMergeFeatureAttributes->setEnabled( false );
      }

      // moving enabled if geometry changes are supported
      mActionAddPart->setEnabled( isEditable && canChangeGeometry );
      mActionDeletePart->setEnabled( isEditable && canChangeGeometry );
      mActionMoveFeature->setEnabled( isEditable && canChangeGeometry );
      mActionRotateFeature->setEnabled( isEditable && canChangeGeometry );
      mActionNodeTool->setEnabled( isEditable && canChangeGeometry );

      if ( vlayer->geometryType() == QGis::Point )
      {
        mActionAddFeature->setIcon( QgsApplication::getThemeIcon( "/mActionCapturePoint.png" ) );

        mActionAddRing->setEnabled( false );
        mActionReshapeFeatures->setEnabled( false );
        mActionSplitFeatures->setEnabled( false );
        mActionSimplifyFeature->setEnabled( false );
        mActionDeleteRing->setEnabled( false );
        mActionRotatePointSymbols->setEnabled( false );
        mActionOffsetCurve->setEnabled( false );

        if ( isEditable && canChangeAttributes )
        {
          if ( QgsMapToolRotatePointSymbols::layerIsRotatable( vlayer ) )
          {
            mActionRotatePointSymbols->setEnabled( true );
          }
        }

        return;
      }
      else if ( vlayer->geometryType() == QGis::Line )
      {
        mActionAddFeature->setIcon( QgsApplication::getThemeIcon( "/mActionCaptureLine.png" ) );

        mActionReshapeFeatures->setEnabled( isEditable && canAddFeatures );
        mActionSplitFeatures->setEnabled( isEditable && canAddFeatures );
        mActionSimplifyFeature->setEnabled( isEditable && canAddFeatures );
        mActionOffsetCurve->setEnabled( isEditable && canAddFeatures && canChangeAttributes );

        mActionAddRing->setEnabled( false );
        mActionDeleteRing->setEnabled( false );
      }
      else if ( vlayer->geometryType() == QGis::Polygon )
      {
        mActionAddFeature->setIcon( QgsApplication::getThemeIcon( "/mActionCapturePolygon.png" ) );

        mActionAddRing->setEnabled( isEditable && canAddFeatures );
        mActionReshapeFeatures->setEnabled( isEditable && canAddFeatures );
        mActionSplitFeatures->setEnabled( isEditable && canAddFeatures );
        mActionSimplifyFeature->setEnabled( isEditable && canAddFeatures );
        mActionDeleteRing->setEnabled( isEditable && canAddFeatures );
        mActionOffsetCurve->setEnabled( false );
      }

      mActionOpenFieldCalc->setEnabled( isEditable && ( canChangeAttributes || canAddAttributes ) );

      return;
    }
    else
    {
      mUndoWidget->dockContents()->setEnabled( false );
      mActionUndo->setEnabled( false );
      mActionRedo->setEnabled( false );
    }

    mActionLayerSubsetString->setEnabled( false );
  } //end vector layer block
  /*************Raster layers*************/
  else if ( layer->type() == QgsMapLayer::RasterLayer )
  {
    const QgsRasterLayer *rlayer = qobject_cast<const QgsRasterLayer *>( layer );
    if ( rlayer->dataProvider()->dataType( 1 ) != QGis::ARGB32
         && rlayer->dataProvider()->dataType( 1 ) != QGis::ARGB32_Premultiplied )
    {
      if ( rlayer->dataProvider()->capabilities() & QgsRasterDataProvider::Size )
      {
        mActionFullHistogramStretch->setEnabled( true );
      }
      else
      {
        // it would hang up reading the data for WMS for example
        mActionFullHistogramStretch->setEnabled( false );
      }
      mActionLocalHistogramStretch->setEnabled( true );
    }
    else
    {
      mActionLocalHistogramStretch->setEnabled( false );
      mActionFullHistogramStretch->setEnabled( false );
    }

    mActionLocalCumulativeCutStretch->setEnabled( true );
    mActionFullCumulativeCutStretch->setEnabled( true );
    mActionIncreaseBrightness->setEnabled( true );
    mActionDecreaseBrightness->setEnabled( true );
    mActionIncreaseContrast->setEnabled( true );
    mActionDecreaseContrast->setEnabled( true );

    mActionLayerSubsetString->setEnabled( false );
    mActionFeatureAction->setEnabled( false );
    mActionSelect->setEnabled( false );
    mActionSelectRectangle->setEnabled( false );
    mActionSelectPolygon->setEnabled( false );
    mActionSelectFreehand->setEnabled( false );
    mActionSelectRadius->setEnabled( false );
    mActionZoomActualSize->setEnabled( true );
    mActionOpenTable->setEnabled( false );
    mActionOpenFieldCalc->setEnabled( false );
    mActionToggleEditing->setEnabled( false );
    mActionToggleEditing->setChecked( false );
    mActionSaveLayerEdits->setEnabled( false );
    mUndoWidget->dockContents()->setEnabled( false );
    mActionUndo->setEnabled( false );
    mActionRedo->setEnabled( false );
    mActionLayerSaveAs->setEnabled( true );
    mActionLayerSelectionSaveAs->setEnabled( false );
    mActionAddFeature->setEnabled( false );
    mActionDeleteSelected->setEnabled( false );
    mActionAddRing->setEnabled( false );
    mActionAddPart->setEnabled( false );
    mActionNodeTool->setEnabled( false );
    mActionMoveFeature->setEnabled( false );
    mActionRotateFeature->setEnabled( false );
    mActionOffsetCurve->setEnabled( false );
    mActionCopyFeatures->setEnabled( false );
    mActionCutFeatures->setEnabled( false );
    mActionPasteFeatures->setEnabled( false );
    mActionRotatePointSymbols->setEnabled( false );
    mActionDeletePart->setEnabled( false );
    mActionDeleteRing->setEnabled( false );
    mActionSimplifyFeature->setEnabled( false );
    mActionReshapeFeatures->setEnabled( false );
    mActionSplitFeatures->setEnabled( false );
    mActionLabeling->setEnabled( false );

    //NOTE: This check does not really add any protection, as it is called on load not on layer select/activate
    //If you load a layer with a provider and idenitfy ability then load another without, the tool would be disabled for both

    //Enable the Identify tool ( GDAL datasets draw without a provider )
    //but turn off if data provider exists and has no Identify capabilities
    mActionIdentify->setEnabled( true );

    QSettings settings;
    int identifyMode = settings.value( "/Map/identifyMode", 0 ).toInt();
    if ( identifyMode == 0 )
    {
      const QgsRasterLayer *rlayer = qobject_cast<const QgsRasterLayer *>( layer );
      const QgsRasterDataProvider* dprovider = rlayer->dataProvider();
      if ( dprovider )
      {
        // does provider allow the identify map tool?
        if ( dprovider->capabilities() & QgsRasterDataProvider::Identify )
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
  if ( mMapCanvas && mMapCanvas->isDrawing() )
  {
    return;
  }

  QString fileFilters;

  QStringList selectedFiles;
  QString e;//only for parameter correctness
  QString title = tr( "Open a GDAL Supported Raster Data Source" );
  QgisGui::openFilesRememberingFilter( "lastRasterFileFilter", mRasterFileFilter, selectedFiles, e,
                                       title );

  if ( selectedFiles.isEmpty() )
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
  if ( mMapCanvas && mMapCanvas->isDrawing() )
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

  if ( !theRasterLayer->isValid() )
  {
    delete theRasterLayer;
    return false;
  }

  // register this layer with the central layers registry
  QList<QgsMapLayer *> myList;
  myList << theRasterLayer;
  QgsMapLayerRegistry::instance()->addMapLayers( myList );

  return true;
}


// Open a raster layer - this is the generic function which takes all parameters
// this method is a blend of addRasterLayer() functions (with and without provider)
// and addRasterLayers()
QgsRasterLayer* QgisApp::addRasterLayerPrivate(
  const QString & uri, const QString & baseName, const QString & providerKey,
  bool guiWarning, bool guiUpdate )
{
  if ( mMapCanvas && mMapCanvas->isDrawing() )
  {
    return NULL;
  }

  if ( guiUpdate )
  {
    // let the user know we're going to possibly be taking a while
    // QApplication::setOverrideCursor( Qt::WaitCursor );
    mMapCanvas->freeze( true );
  }

  QgsDebugMsg( "Creating new raster layer using " + uri
               + " with baseName of " + baseName );

  QgsRasterLayer *layer = 0;
  // XXX ya know QgsRasterLayer can snip out the basename on its own;
  // XXX why do we have to pass it in for it?
  // ET : we may not be getting "normal" files here, so we still need the baseName argument
  if ( providerKey.isEmpty() )
    layer = new QgsRasterLayer( uri, baseName ); // fi.completeBaseName());
  else
    layer = new QgsRasterLayer( uri, baseName, providerKey );

  QgsDebugMsg( "Constructed new layer" );

  QgsError error;
  QString title;
  bool ok = false;

  if ( !layer->isValid() )
  {
    error = layer->error();
    title = tr( "Invalid Layer" );

    if ( shouldAskUserForGDALSublayers( layer ) )
    {
      askUserForGDALSublayers( layer );
      ok = true;

      // The first layer loaded is not useful in that case. The user can select it in
      // the list if he wants to load it.
      delete layer;
      layer = NULL;
    }
  }
  else
  {
    ok = addRasterLayer( layer );
    if ( !ok )
    {
      error.append( QGS_ERROR_MESSAGE( tr( "Error adding valid layer to map canvas" ),
                                       tr( "Raster layer" ) ) );
      title = tr( "Error" );
    }
  }

  if ( !ok )
  {
    if ( guiUpdate )
      mMapCanvas->freeze( false );

    // don't show the gui warning if we are loading from command line
    if ( guiWarning )
    {
      QgsErrorDialog::show( error, title );
    }

    if ( layer )
    {
      delete layer;
      layer = NULL;
    }
  }

  if ( guiUpdate )
  {
    // update UI
    qApp->processEvents();
    // draw the map
    mMapCanvas->freeze( false );
    mMapCanvas->refresh();
    //update status
    statusBar()->showMessage( mMapCanvas->extent().toString( 2 ) );
    // Let render() do its own cursor management
    //    QApplication::restoreOverrideCursor();
  }

  return layer;

} // QgisApp::addRasterLayer


//create a raster layer object and delegate to addRasterLayer(QgsRasterLayer *)
QgsRasterLayer* QgisApp::addRasterLayer(
  QString const & rasterFile, QString const & baseName, bool guiWarning )
{
  return addRasterLayerPrivate( rasterFile, baseName, QString(), guiWarning, true );
}


/** Add a raster layer directly without prompting user for location
  The caller must provide information compatible with the provider plugin
  using the uri and baseName. The provider can use these
  parameters in any way necessary to initialize the layer. The baseName
  parameter is used in the Map Legend so it should be formed in a meaningful
  way.

  \note   Copied from the equivalent addVectorLayer function in this file
  */
QgsRasterLayer* QgisApp::addRasterLayer(
  QString const &uri, QString const &baseName, QString const &providerKey )
{
  return addRasterLayerPrivate( uri, baseName, providerKey, true, true );
}


//create a raster layer object and delegate to addRasterLayer(QgsRasterLayer *)
bool QgisApp::addRasterLayers( QStringList const &theFileNameQStringList, bool guiWarning )
{
  if ( mMapCanvas && mMapCanvas->isDrawing() )
  {
    return false;
  }

  if ( theFileNameQStringList.empty() )
  {
    // no files selected so bail out, but
    // allow mMapCanvas to handle events
    // first
    mMapCanvas->freeze( false );
    return false;
  }

  mMapCanvas->freeze( true );

  // this is messy since some files in the list may be rasters and others may
  // be ogr layers. We'll set returnValue to false if one or more layers fail
  // to load.
  bool returnValue = true;
  for ( QStringList::ConstIterator myIterator = theFileNameQStringList.begin();
        myIterator != theFileNameQStringList.end();
        ++myIterator )
  {
    QString errMsg;
    bool ok = false;

    // if needed prompt for zipitem layers
    QString vsiPrefix = QgsZipItem::vsiPrefix( *myIterator );
    if ( ! myIterator->startsWith( "/vsi", Qt::CaseInsensitive ) &&
         ( vsiPrefix == "/vsizip/" || vsiPrefix == "/vsitar/" ) )
    {
      if ( askUserForZipItemLayers( *myIterator ) )
        continue;
    }

    if ( QgsRasterLayer::isValidRasterFileName( *myIterator, errMsg ) )
    {
      QFileInfo myFileInfo( *myIterator );
      // get the directory the .adf file was in
      QString myDirNameQString = myFileInfo.path();
      //extract basename
      QString myBaseNameQString = myFileInfo.completeBaseName();
      //only allow one copy of a ai grid file to be loaded at a
      //time to prevent the user selecting all adfs in 1 dir which
      //actually represent 1 coverage,

      // try to create the layer
      QgsRasterLayer *layer = addRasterLayerPrivate( *myIterator, myBaseNameQString,
                              QString(), guiWarning, true );
      if ( layer && layer->isValid() )
      {
        //only allow one copy of a ai grid file to be loaded at a
        //time to prevent the user selecting all adfs in 1 dir which
        //actually represent 1 coverate,

        if ( myBaseNameQString.toLower().endsWith( ".adf" ) )
        {
          break;
        }
      }
      // if layer is invalid addRasterLayerPrivate() will show the error

    } // valid raster filename
    else
    {
      ok = false;

      // Issue message box warning unless we are loading from cmd line since
      // non-rasters are passed to this function first and then successfully
      // loaded afterwards (see main.cpp)
      if ( guiWarning )
      {
        QgsError error;
        QString msg;

        msg = tr( "%1 is not a supported raster data source" ).arg( *myIterator );
        if ( errMsg.size() > 0 )
          msg += "\n" + errMsg;
        error.append( QGS_ERROR_MESSAGE( msg, tr( "Raster layer" ) ) );

        QgsErrorDialog::show( error, tr( "Unsupported Data Source" ) );
      }
    }
    if ( ! ok )
    {
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
  if ( e->key() == Qt::Key_Escape )
  {
    stopRendering();
  }
#if defined(Q_OS_WIN)&& defined(QGISDEBUG)
  else if ( e->key() == Qt::Key_Backslash && e->modifiers() & Qt::ControlModifier )
  {
    extern LONG WINAPI qgisCrashDump( struct _EXCEPTION_POINTERS *ExceptionInfo );
    qgisCrashDump( 0 );
  }
#endif
  else
  {
    e->ignore();
  }
}

#ifdef Q_OS_WIN
// hope your wearing your peril sensitive sunglasses.
void QgisApp::contextMenuEvent( QContextMenuEvent *e )
{
  if ( mSkipNextContextMenuEvent )
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
  if ( mMapCanvas && mMapCanvas->isDrawing() )
  {
    return;
  }

  // Create an instance of the Custom Projection Designer modeless dialog.
  // Autodelete the dialog when closing since a pointer is not retained.
  QgsCustomProjectionDialog * myDialog = new QgsCustomProjectionDialog( this );
  myDialog->setAttribute( Qt::WA_DeleteOnClose );
  myDialog->show();
}

void QgisApp::newBookmark()
{
  QgsBookmarks::newBookmark();
}

void QgisApp::showBookmarks()
{
  QgsBookmarks::showBookmarks();
}

// Slot that gets called when the project file was saved with an older
// version of QGIS

void QgisApp::oldProjectVersionWarning( QString oldVersion )
{
  QSettings settings;

  if ( settings.value( "/qgis/warnOldProjectVersion", QVariant( true ) ).toBool() )
  {
    QApplication::setOverrideCursor( Qt::ArrowCursor );
    QString text =  tr( "<p>This project file was saved by an older version of QGIS."
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
                    .arg( "<a href=\"http://hub.qgis.org/projects/quantum-gis\">http://hub.qgis.org/projects/quantum-gis</a> " )
                    .arg( tr( "<tt>Settings:Options:General</tt>", "Menu path to setting options" ) )
                    .arg( tr( "Warn me when opening a project file saved with an older version of QGIS" ) );
    QString title =  tr( "Project file is older" );

#ifdef ANDROID
    //this is needed to deal with http://hub.qgis.org/issues/4573
    QMessageBox box( QMessageBox::Warning, title, tr( "This project file was saved by an older version of QGIS" ), QMessageBox::Ok, NULL );
    box.setDetailedText(
      text.remove( 0, 3 )
      .replace( QString( "<p>" ), QString( "\n\n" ) )
      .replace( QString( "<br>" ), QString( "\n" ) )
      .replace( QString( "<a href=\"http://hub.qgis.org/projects/quantum-gis\">http://hub.qgis.org/projects/quantum-gis</a> " ), QString( "\nhttp://hub.qgis.org/projects/quantum-gis" ) )
      .replace( QRegExp( "</?tt>" ), QString( "" ) )
    );
    box.exec();
#else
    QMessageBox::warning( NULL, title, text );
#endif
    QApplication::restoreOverrideCursor();
  }
  return;
}

void QgisApp::updateUndoActions()
{
  bool canUndo = false, canRedo = false;
  QgsMapLayer* layer = activeLayer();
  if ( layer )
  {
    QgsVectorLayer* vlayer = qobject_cast<QgsVectorLayer *>( layer );
    if ( vlayer && vlayer->isEditable() )
    {
      canUndo = vlayer->undoStack()->canUndo();
      canRedo = vlayer->undoStack()->canRedo();
    }
  }
  mActionUndo->setEnabled( canUndo );
  mActionRedo->setEnabled( canRedo );
}


// add project directory to python path
void QgisApp::projectChanged( const QDomDocument &doc )
{
  Q_UNUSED( doc );
  QgsProject *project = qobject_cast<QgsProject*>( sender() );
  if ( !project )
    return;

  QFileInfo fi( project->fileName() );
  if ( !fi.exists() )
    return;

  static QString prevProjectDir = QString::null;

  if ( prevProjectDir == fi.canonicalPath() )
    return;

  QString expr;
  if ( !prevProjectDir.isNull() )
    expr = QString( "sys.path.remove('%1'); " ).arg( prevProjectDir );

  prevProjectDir = fi.canonicalPath();

  expr += QString( "sys.path.append('%1')" ).arg( prevProjectDir );

  QgsPythonRunner::run( expr );
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

  if ( !ml )
    return;

  if ( !QgsProject::instance()->layerIsEmbedded( ml->id() ).isEmpty() )
  {
    return; //don't show properties of embedded layers
  }

  if ( ml->type() == QgsMapLayer::RasterLayer )
  {
    QgsRasterLayerProperties *rlp = NULL; // See note above about reusing this
    if ( rlp )
    {
      rlp->sync();
    }
    else
    {
      rlp = new QgsRasterLayerProperties( ml, mMapCanvas, this );
      connect( rlp, SIGNAL( refreshLegend( QString, bool ) ), mMapLegend, SLOT( refreshLayerSymbology( QString, bool ) ) );
    }

    rlp->exec();
    delete rlp; // delete since dialog cannot be reused without updating code
  }
  else if ( ml->type() == QgsMapLayer::VectorLayer ) // VECTOR
  {
    QgsVectorLayer* vlayer = qobject_cast<QgsVectorLayer *>( ml );

    QgsVectorLayerProperties *vlp = NULL; // See note above about reusing this
    if ( vlp )
    {
      vlp->syncToLayer();
    }
    else
    {
      vlp = new QgsVectorLayerProperties( vlayer, this );
      connect( vlp, SIGNAL( refreshLegend( QString, QgsLegendItem::Expansion ) ), mMapLegend, SLOT( refreshLayerSymbology( QString, QgsLegendItem::Expansion ) ) );
    }

    if ( vlp->exec() )
    {
      activateDeactivateLayerRelatedActions( ml );
    }
    delete vlp; // delete since dialog cannot be reused without updating code
  }
  else if ( ml->type() == QgsMapLayer::PluginLayer )
  {
    QgsPluginLayer* pl = qobject_cast<QgsPluginLayer *>( ml );
    if ( !pl )
      return;

    QgsPluginLayerType* plt = QgsPluginLayerRegistry::instance()->pluginLayerType( pl->pluginLayerType() );
    if ( !plt )
      return;

    if ( !plt->showLayerProperties( pl ) )
    {
      messageBar()->pushMessage( tr( "Warning" ),
                                 tr( "This layer doesn't have a properties dialog." ),
                                 QgsMessageBar::INFO, messageTimeout() );
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
  if ( !ok )
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
  if ( !ok )
    return;

  auth->setUser( username );
  auth->setPassword( password );
}

#ifndef QT_NO_OPENSSL
void QgisApp::namSslErrors( QNetworkReply *reply, const QList<QSslError> &errors )
{
  QString msg = tr( "SSL errors occured accessing URL %1:" ).arg( reply->request().url().toString() );
  bool otherError = false;
  static QSet<QSslError::SslError> ignoreErrors;

  foreach ( QSslError error, errors )
  {
    if ( error.error() == QSslError::NoError )
      continue;

    QgsDebugMsg( QString( "SSL error %1: %2" ).arg( error.error() ).arg( error.errorString() ) );

    otherError = otherError || !ignoreErrors.contains( error.error() );

    msg += "\n" + error.errorString();
  }

  msg += tr( "\n\nAlways ignore these errors?" );

  if ( !otherError ||
       QMessageBox::warning( this,
                             tr( "%n SSL errors occured", "number of errors", errors.size() ),
                             msg,
                             QMessageBox::Ok | QMessageBox::Cancel ) == QMessageBox::Ok )
  {
    foreach ( QSslError error, errors )
    {
      ignoreErrors << error.error();
    }
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
  if ( proxyEnabled )
  {
    excludes = settings.value( "proxy/proxyExcludedUrls", "" ).toString().split( "|", QString::SkipEmptyParts );

    //read type, host, port, user, passw from settings
    QString proxyHost = settings.value( "proxy/proxyHost", "" ).toString();
    int proxyPort = settings.value( "proxy/proxyPort", "" ).toString().toInt();
    QString proxyUser = settings.value( "proxy/proxyUser", "" ).toString();
    QString proxyPassword = settings.value( "proxy/proxyPassword", "" ).toString();

    QString proxyTypeString = settings.value( "proxy/proxyType", "" ).toString();
    QNetworkProxy::ProxyType proxyType = QNetworkProxy::NoProxy;
    if ( proxyTypeString == "DefaultProxy" )
    {
      proxyType = QNetworkProxy::DefaultProxy;
    }
    else if ( proxyTypeString == "Socks5Proxy" )
    {
      proxyType = QNetworkProxy::Socks5Proxy;
    }
    else if ( proxyTypeString == "HttpProxy" )
    {
      proxyType = QNetworkProxy::HttpProxy;
    }
    else if ( proxyTypeString == "HttpCachingProxy" )
    {
      proxyType = QNetworkProxy::HttpCachingProxy;
    }
    else if ( proxyTypeString == "FtpCachingProxy" )
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
  if ( !cache )
    cache = new QNetworkDiskCache( this );

  QString cacheDirectory = settings.value( "cache/directory", QgsApplication::qgisSettingsDirPath() + "cache" ).toString();
  qint64 cacheSize = settings.value( "cache/size", 50 * 1024 * 1024 ).toULongLong();
  QgsDebugMsg( QString( "setCacheDirectory: %1" ).arg( cacheDirectory ) );
  QgsDebugMsg( QString( "setMaximumCacheSize: %1" ).arg( cacheSize ) );
  cache->setCacheDirectory( cacheDirectory );
  cache->setMaximumCacheSize( cacheSize );
  QgsDebugMsg( QString( "cacheDirectory: %1" ).arg( cache->cacheDirectory() ) );
  QgsDebugMsg( QString( "maximumCacheSize: %1" ).arg( cache->maximumCacheSize() ) );

  if ( nam->cache() != cache )
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
  if ( !bt )
    return;

  QSettings settings;
  if ( action == mActionSelect )
    settings.setValue( "/UI/selectTool", 0 );
  else if ( action == mActionSelectRectangle )
    settings.setValue( "/UI/selectTool", 1 );
  else if ( action == mActionSelectPolygon )
    settings.setValue( "/UI/selectTool", 2 );
  else if ( action == mActionSelectFreehand )
    settings.setValue( "/UI/selectTool", 3 );
  else if ( action == mActionSelectRadius )
    settings.setValue( "/UI/selectTool", 4 );
  else if ( action == mActionMeasure )
    settings.setValue( "/UI/measureTool", 0 );
  else if ( action == mActionMeasureArea )
    settings.setValue( "/UI/measureTool", 1 );
  else if ( action == mActionMeasureAngle )
    settings.setValue( "/UI/measureTool", 2 );
  else if ( action == mActionTextAnnotation )
    settings.setValue( "/UI/annotationTool", 0 );
  else if ( action == mActionFormAnnotation )
    settings.setValue( "/UI/annotationTool", 1 );
  else if ( action == mActionHtmlAnnotation )
    settings.setValue( "/UI/annotationTool", 2 );
  else if ( action == mActionSvgAnnotation )
    settings.setValue( "UI/annotationTool", 3 );
  else if ( action == mActionAnnotation )
    settings.setValue( "/UI/annotationTool", 4 );
  bt->setDefaultAction( action );
}

QMenu* QgisApp::createPopupMenu()
{
  QMenu* menu = QMainWindow::createPopupMenu();
  QList< QAction* > al = menu->actions();
  QList< QAction* > panels, toolbars;

  if ( !al.isEmpty() )
  {
    bool found = false;
    for ( int i = 0; i < al.size(); ++i )
    {
      if ( al[ i ]->isSeparator() )
      {
        found = true;
        continue;
      }

      if ( !found )
      {
        panels.append( al[ i ] );
      }
      else
      {
        toolbars.append( al[ i ] );
      }
    }

    qSort( panels.begin(), panels.end(), cmpByText_ );
    foreach ( QAction* a, panels )
    {
      menu->addAction( a );
    }
    menu->addSeparator();
    qSort( toolbars.begin(), toolbars.end(), cmpByText_ );
    foreach ( QAction* a, toolbars )
    {
      menu->addAction( a );
    }
  }

  return menu;
}

void QgisApp::osmDownloadDialog()
{
  QgsOSMDownloadDialog dlg;
  dlg.exec();
}

void QgisApp::osmImportDialog()
{
  QgsOSMImportDialog dlg;
  dlg.exec();
}

void QgisApp::osmExportDialog()
{
  QgsOSMExportDialog dlg;
  dlg.exec();
}


#ifdef HAVE_TOUCH
bool QgisApp::gestureEvent( QGestureEvent *event )
{
  if ( QGesture *tapAndHold = event->gesture( Qt::TapAndHoldGesture ) )
  {
    tapAndHoldTriggered( static_cast<QTapAndHoldGesture *>( tapAndHold ) );
  }
  return true;
}

void QgisApp::tapAndHoldTriggered( QTapAndHoldGesture *gesture )
{
  if ( gesture->state() == Qt::GestureFinished )
  {
    QPoint pos = gesture->position().toPoint();
    QWidget * receiver = QApplication::widgetAt( pos );
    qDebug() << "tapAndHoldTriggered: LONG CLICK gesture happened at " << pos;
    qDebug() << "widget under point of click: " << receiver;

    QApplication::postEvent( receiver, new QMouseEvent( QEvent::MouseButtonPress, receiver->mapFromGlobal( pos ), Qt::RightButton, Qt::RightButton, Qt::NoModifier ) );
    QApplication::postEvent( receiver, new QMouseEvent( QEvent::MouseButtonRelease, receiver->mapFromGlobal( pos ), Qt::RightButton, Qt::RightButton, Qt::NoModifier ) );
  }
}
#endif
