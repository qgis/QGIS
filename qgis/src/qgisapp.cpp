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

#ifndef WIN32
#include <dlfcn.h>
#endif

#include <qevent.h>
#include <qaction.h>
#include <qapplication.h>
#include <qbitmap.h>
#include <qcanvas.h>
#include <qcolor.h>
#include <qcursor.h>
#include <qdir.h>
#include <qerrormessage.h>
#include <qfile.h>
#include <qfileinfo.h>
#include <qinputdialog.h>
#include <qlabel.h>
#include <qlayout.h>
#include <qlibrary.h>
#include <qlistview.h>
#include <qmenubar.h>
#include <qmessagebox.h>
#include <qpainter.h>
#include <qprogressbar.h>
#include <qpixmap.h>
#include <qpoint.h>
#include <qpopupmenu.h>
#include <qprocess.h>
#include <qrect.h>
#include <qregexp.h>
#include <qscrollview.h>
#include <qsettings.h>
#include <qsocket.h>
#include <qsplitter.h>
#include <qstatusbar.h>
#include <qstringlist.h>
#include <qtextstream.h>
#include <qvbox.h>
#include <qwmatrix.h>
#include <qwhatsthis.h>
#include <qimage.h>
#include <qprinter.h>
#include <qpaintdevicemetrics.h>
#include <qclipboard.h>
#include <qapplication.h>
#include <qtoolbutton.h>
#include <qtimer.h>
#include <qcheckbox.h>

#include <iostream>
#include <iomanip>
#include <memory>
#include <vector>

#include <cmath>

#include "qgsencodingfiledialog.h"
#include "qgsrect.h"
#include "qgsmapcanvas.h"
#include "qgsacetaterectangle.h"
#include "qgsmaplayer.h"
#include "qgslegenditem.h"
#include "qgslegend.h"
#include "qgsproject.h"
#include "qgsmapserverexport.h"
#include "qgsgeomtypedialog.h"
#include "qgscustomprojectiondialog.h"

#ifdef HAVE_POSTGRESQL
#include "qgsdbsourceselect.h"
#endif
#ifdef WIN32
#include "qgsmessageviewer.h"
#include "qgsabout.h"
#else
#include "qgsabout.uic.h"
#include "qgsmessageviewer.uic.h"
#endif
#include "qgshelpviewer.h"
#include "qgsmaplayerregistry.h"
#include "qgsrasterlayerproperties.h"
#include "qgsvectorlayer.h"
#include "qgspluginmanager.h"
#include "qgsmaplayerinterface.h"
#include "qgis.h"
#include "qgisapp.h"
#include "qgspluginitem.h"
#include "qgsproviderregistry.h"
#include "qgspluginregistry.h"
#include "qgssinglesymrenderer.h"
//#include "qgssisydialog.h"
#include "../plugins/qgisplugin.h"
#include "qgsoptions.h"
#include "qgsprojectproperties.h"
#include "qgsvectorfilewriter.h"
#include "qgscomposer.h"
#include "qgsbookmarks.h"
#include "qgsbookmarkitem.h"


#include "xpm/qgis.xpm"

#include <ogrsf_frmts.h>

#ifdef Q_OS_MACX
#include <ApplicationServices/ApplicationServices.h>
#endif

/* typedefs for plugins */
typedef QgsMapLayerInterface *create_it();
typedef QgisPlugin *create_ui(QgisApp * qgis, QgisIface * qI);
typedef QString name_t();
typedef QString description_t();
typedef int type_t();

// IDs for locating particular menu items
const int BEFORE_RECENT_PATHS = 123;
const int AFTER_RECENT_PATHS = 321;

// cursors
static char *zoom_in[] = {
                             "16 16 3 1",
                             ". c None",
                             "a c #000000",
                             "# c #ffffff",
                             ".....#####......",
                             "...##aaaaa##....",
                             "..#.a.....a.#...",
                             ".#.a...a...a.#..",
                             ".#a....a....a#..",
                             "#a.....a.....a#.",
                             "#a.....a.....a#.",
                             "#a.aaaa#aaaa.a#.",
                             "#a.....a.....a#.",
                             "#a.....a.....a#.",
                             ".#a....a....a#..",
                             ".#.a...a...aaa#.",
                             "..#.a.....a#aaa#",
                             "...##aaaaa###aa#",
                             ".....#####...###",
                             "..............#."
                         };

static char *zoom_out[] = {
                              "16 16 4 1",
                              "b c None",
                              ". c None",
                              "a c #000000",
                              "# c #ffffff",
                              ".....#####......",
                              "...##aaaaa##....",
                              "..#.a.....a.#...",
                              ".#.a.......a.#..",
                              ".#a.........a#..",
                              "#a...........a#.",
                              "#a...........a#.",
                              "#a.aaaa#aaaa.a#.",
                              "#a...........a#.",
                              "#a...........a#.",
                              ".#a.........a#..",
                              ".#.a.......aaa#.",
                              "..#.a.....a#aaa#",
                              "...##aaaaa###aa#",
                              ".....#####...###",
                              "..............#."
                          };



static unsigned char pan_bits[] = {
                                      0xf0, 0x00, 0xf8, 0x01, 0x28, 0x07, 0x28, 0x05, 0x28, 0x1d, 0x28, 0x15,
                                      0x2f, 0x15, 0x0d, 0x14, 0x09, 0x10, 0x03, 0x18, 0x06, 0x08, 0x04, 0x08,
                                      0x0c, 0x0c, 0x18, 0x04, 0x30, 0x04, 0xe0, 0x07
                                  };

static unsigned char pan_mask_bits[] = {
                                           0xf0, 0x00, 0xf8, 0x01, 0xf8, 0x07, 0xf8, 0x07, 0xf8, 0x1f, 0xf8, 0x1f,
                                           0xff, 0x1f, 0xff, 0x1f, 0xff, 0x1f, 0xff, 0x1f, 0xfe, 0x0f, 0xfc, 0x0f,
                                           0xfc, 0x0f, 0xf8, 0x07, 0xf0, 0x07, 0xe0, 0x07
                                       };


static char *capture_point_cursor[] = {
                                          "16 16 3 1",
                                          " »     c None",
                                          ".»     c #000000",
                                          "+»     c #FFFFFF",
                                          "                ",
                                          "       +.+      ",
                                          "      ++.++     ",
                                          "     +.....+    ",
                                          "    +.     .+   ",
                                          "   +.   .   .+  ",
                                          "  +.    .    .+ ",
                                          " ++.    .    .++",
                                          " ... ...+... ...",
                                          " ++.    .    .++",
                                          "  +.    .    .+ ",
                                          "   +.   .   .+  ",
                                          "   ++.     .+   ",
                                          "    ++.....+    ",
                                          "      ++.++     ",
                                          "       +.+      "};

static char *select_cursor[] = {
                                   "16 16 3 1",
                                   "# c None",
                                   "a c #000000",
                                   ". c #ffffff",
                                   ".###############",
                                   "...#############",
                                   ".aa..###########",
                                   "#.aaa..a.a.a.a.#",
                                   "#.aaaaa..#####a#",
                                   "#a.aaaaaa..###.#",
                                   "#..aaaaaa...##a#",
                                   "#a.aaaaa.#####.#",
                                   "#.#.aaaaa.####a#",
                                   "#a#.aa.aaa.###.#",
                                   "#.##..#..aa.##a#",
                                   "#a##.####.aa.#.#",
                                   "#.########.aa.a#",
                                   "#a#########.aa..",
                                   "#.a.a.a.a.a..a.#",
                                   "#############.##"
                               };

static char *identify_cursor[] = {
                                     "16 16 3 1",
                                     "# c None",
                                     "a c #000000",
                                     ". c #ffffff",
                                     ".###########..##",
                                     "...########.aa.#",
                                     ".aa..######.aa.#",
                                     "#.aaa..#####..##",
                                     "#.aaaaa..##.aa.#",
                                     "##.aaaaaa...aa.#",
                                     "##.aaaaaa...aa.#",
                                     "##.aaaaa.##.aa.#",
                                     "###.aaaaa.#.aa.#",
                                     "###.aa.aaa..aa.#",
                                     "####..#..aa.aa.#",
                                     "####.####.aa.a.#",
                                     "##########.aa..#",
                                     "###########.aa..",
                                     "############.a.#",
                                     "#############.##"
                                 };


/** set the application title bar text

  If the current project title is null
     if the project file is null then
         set title text to just application name and version
     else
         set set title text to the the project file name
  else
     set the title text to project title
 */
static
void
setTitleBarText_( QWidget & qgisApp )
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

QgisApp::QgisApp(QWidget * parent, const char *name, WFlags fl)
        : QgisAppBase(parent, name, fl),
        myHideSplashFlag(false)
{
    //
    // Splash screen global is declared in qgisapp.h header
    //
    QSettings settings;

    myHideSplashFlag = settings.readBoolEntry("/qgis/hideSplash");

    if (!myHideSplashFlag)
    {
        gSplashScreen = new SplashScreen(); //this is supposed to be instantiated in main.cpp but we get segfaults...
        gSplashScreen->setStatus(tr("Loading QGIS..."));
        qApp->processEvents();

        //  gSplashScreen->setStatus(tr("QGIS ready"));
    }

    // register all GDAL and OGR plug-ins
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
    if (!myHideSplashFlag)
    {
        gSplashScreen->setStatus(tr("Setting up QGIS gui..."));
        qApp->processEvents();
    }
    QGridLayout *canvasLegendLayout = new QGridLayout(frameMain, 1, 1, 4, 6, "canvasLegendLayout");
    QSplitter *canvasLegendSplit = new QSplitter(frameMain);
    QGridLayout *legendOverviewLayout = new QGridLayout(canvasLegendSplit, 1, 2, 4, 6, "canvasLegendLayout");
    QSplitter *legendOverviewSplit = new QSplitter(Qt::Vertical,canvasLegendSplit);
    mMapLegend = new QgsLegend(legendOverviewSplit, "theMapLegend", this);
    mMapLegend->addColumn(tr("Layers"));
    mMapLegend->setSorting(-1);
    QWhatsThis::add(mMapLegend, tr("Map legend that displays all the layers currently on the map canvas. Click on the check box to turn a layer on or off. Double click on a layer in the legend to customize its appearance and set other properties."));

    // "theOverviewCanvas" used to find canonical
    // instance later
    mOverviewCanvas = new QgsMapCanvas(legendOverviewSplit, "theOverviewCanvas");
    QWhatsThis::add(mOverviewCanvas, tr("Map overview canvas. This canvas can be used to display a locator map that shows the current extent of the map canvas. The current extent is shown as a red rectangle. Any layer on the map can be added to the overview canvas."));
    //tell the overview canvas it is an overview canvas so it will suppress labels etc
    mOverviewCanvas->setIsOverviewCanvas(true);
    // lock the canvas to prevent user interaction
    mOverviewCanvas->userInteractionAllowed(false);

    // mL = new QScrollView(canvasLegendSplit);

    // add a canvas
    // "theMapCanvas" used to find this canonical
    // instance later
    mMapCanvas = new QgsMapCanvas(canvasLegendSplit, "theMapCanvas" );
    QWhatsThis::add(mMapCanvas, tr("Map canvas. This is where raster and vector layers are displayed when added to the map"));
    // we need to cache the layer registry instance so plugins can get to it
    // now explicitly refer to Singleton -- mLayerRegistry = QgsMapLayerRegistry::instance();
    // resize it to fit in the frame
    //    QRect r = frmCanvas->rect();
    //    canvas->resize(r.width(), r.height());
    mMapCanvas->setBackgroundColor(Qt::white); //QColor (220, 235, 255));
    mMapCanvas->setMinimumWidth(400);
    canvasLegendLayout->addWidget(canvasLegendSplit, 0, 0);

    mMapLegend->setBackgroundColor(QColor(192, 192, 192));
    mMapLegend->setMapCanvas(mMapCanvas);
    mMapLegend->setResizeMode(QListView::AllColumns);

    QString caption = tr("Quantum GIS - ");
    caption += QString("%1 ('%2')").arg(QGis::qgisVersion).arg(QGis::qgisReleaseName);

    setCaption(caption);

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

    connect(mMapLegend, SIGNAL(doubleClicked(QListViewItem *)), this, SLOT(layerProperties(QListViewItem *)));
    connect(mMapLegend, SIGNAL(rightButtonPressed(QListViewItem *, const QPoint &, int)),
            this, SLOT(rightClickLegendMenu(QListViewItem *, const QPoint &, int)));
    connect(mMapLegend, SIGNAL(zOrderChanged(QgsLegend *)), mMapCanvas, SLOT(setZOrderFromLegend(QgsLegend *)));
    connect(mMapLegend, SIGNAL(zOrderChanged(QgsLegend *)), this, SLOT(setOverviewZOrder(QgsLegend *)));
    connect(mMapLegend, SIGNAL(currentChanged(QListViewItem *)), this, SLOT(currentLayerChanged(QListViewItem *)));

    // Add the whats this toolbar button
    //TODO Fix this ToolBar pointer to be named consistently
    QWhatsThis::whatsThisButton(helpToolbar);
    
    // Add the recently accessed project file paths to the File menu
    mRecentProjectPaths = settings.readListEntry("/qgis/UI/recentProjectsList");
    // Exit is the last item. Find it and insert these just before it
    uint count = popupMenuFile->count();
    // Remember the items before and after the paths so we can manipulate them later
    popupMenuFile->setId(count - 2, BEFORE_RECENT_PATHS); // item before paths
    popupMenuFile->setId(count - 1, AFTER_RECENT_PATHS); // Exit - comes after paths
    updateRecentProjectPaths();
    
    // Add the empty plugin menu
    mPluginMenu = new QPopupMenu(this);
    // Add the plugin manager action to it
    actionPluginManager->addTo(mPluginMenu);
    // Add separator. Plugins will add their menus to this
    // menu when they are loaded by the plugin manager
    mPluginMenu->insertSeparator();
    // Add to the menubar
    menuBar()->insertItem(tr("&Plugins"), mPluginMenu, -1, menuBar()->count() - 1);


    // create the layer popup menu
    mMapCursor = 0;

    // create the interfce
    mQgisInterface = new QgisIface(this);
    ///mQgisInterface->setParent(this);

    // set the legend control for the map canvas
    mMapCanvas->setLegend(mMapLegend);


    // disable functions based on build type
#ifndef HAVE_POSTGRESQL

    actionAddLayer->removeFrom(popupMenuLayers);
    actionAddLayer->removeFrom(DataToolbar);
#endif
    // connect the "cleanup" slot
    connect(qApp, SIGNAL(aboutToQuit()), this, SLOT(saveWindowState()));
    restoreWindowState();

    //
    // Add a panel to the status bar for the scale, coords and progress
    // And also rendering suppression checkbox
    //

    mProgressBar = new QProgressBar(100,this);
    mProgressBar->setMaximumWidth(100);
    QWhatsThis::add(mProgressBar, tr("Progress bar that displays the status of rendering layers and other time-intensive operations"));
    statusBar()->addWidget(mProgressBar, 1,true);
    QFont myFont( "Arial", 8 );
    statusBar()->setFont(myFont);
    mScaleLabel = new QLabel(QString("Scale"),this);
    mScaleLabel->setFont(myFont);
    mScaleLabel->setMinimumWidth(100);
    QWhatsThis::add(mScaleLabel, tr("Displays the current map scale"));
    statusBar()->addWidget(mScaleLabel, 0,true);
    mCoordsLabel = new QLabel(QString("Coordinates:"), this);
    mCoordsLabel->setMinimumWidth(200);
    mCoordsLabel->setFont(myFont);
    QWhatsThis::add(mCoordsLabel, tr("Shows the map coordinates at the current cursor postion. The display is continuously updated as the mouse is moved."));
    statusBar()->addWidget(mCoordsLabel, 0, true);
    mRenderSuppresionCBox = new QCheckBox(tr("Render"),this);
    mRenderSuppresionCBox->setChecked(true);
    mRenderSuppresionCBox->setFont(myFont);
    QWhatsThis::add(mRenderSuppresionCBox, tr("When checked, the map layers are rendered in response to map navigation commands and other events. When not checked, no rendering is done. This allows you to add a large number of layers and symbolize them before rendering."));
    statusBar()->addWidget(mRenderSuppresionCBox,0,true);

    connect(mRenderSuppresionCBox, SIGNAL(toggled(bool )),
            mMapCanvas, SLOT(setRenderFlag(bool)));
    connect(mRenderSuppresionCBox, SIGNAL(toggled(bool )),
            mOverviewCanvas, SLOT(setRenderFlag(bool)));


    //
    // Create the plugin registry and load plugins
    //

    if (! myHideSplashFlag)
    {
        gSplashScreen->setStatus(tr("Loading plugins..."));
        qApp->processEvents();
    }

    // store the application dir
#if defined(WIN32) || defined(Q_OS_MACX)
    mAppDir = qApp->applicationDirPath();
#else

    mAppDir = PREFIX;
#endif

    // Get pointer to the provider registry singleton
#if defined(WIN32) || defined(Q_OS_MACX)

    QString plib = mAppDir + "/lib/qgis";
#else

    QString plib = PLUGINPATH;
#endif

    mProviderRegistry = QgsProviderRegistry::instance(plib);

#ifdef QGISDEBUG

    std::cout << "Plugins are installed in " << plib << std::endl;
#endif

    // load any plugins that were running in the last session
#ifdef QGISDEBUG

    std::cerr << "About to restore plugins session" << std::endl;
#endif

    restoreSessionPlugins(plib);

    // set the provider plugin path
#ifdef QGISDEBUG

    std::cout << "Setting plugin lib dir to " << plib << std::endl;
#endif

    //
    // Create the map layer registry. Any layers loaded will
    // register themselves here. Deleting layers shouls ONLY
    // be done by means of a removeMapLayer call to the regsitry.
    // This allows a single layer instance to be shared
    // between more than one canvas. The registry is a singleton
    // and is constructed using the static instance call.

    // syntactic sugar shortcut for instance handle
    QgsMapLayerRegistry * mapLayerRegistry = QgsMapLayerRegistry::instance();

    //connect the legend, mapcanvas and overview canvas to the registry
    connect(mapLayerRegistry, SIGNAL(layerWillBeRemoved(QString)), mMapCanvas, SLOT(remove
            (QString)));
    // XXX following not necessary because canvas will notify its legend; I think
    // connect(mapLayerRegistry, SIGNAL(layerWillBeRemoved(QString)), mMapLegend, SLOT(removeLayer(QString)));
    connect(mapLayerRegistry, SIGNAL(layerWillBeRemoved(QString)), mOverviewCanvas, SLOT(remove
            (QString)));
    connect(mapLayerRegistry, SIGNAL(removedAll()), mOverviewCanvas, SLOT(removeAll()));
    connect(mapLayerRegistry, SIGNAL(removedAll()), mMapCanvas, SLOT(removeAll()));

    connect(mapLayerRegistry, SIGNAL(layerWasAdded(QgsMapLayer*)), mMapCanvas, SLOT(addLayer(QgsMapLayer *)));
    connect(mapLayerRegistry, SIGNAL(layerWasAdded(QgsMapLayer*)), mOverviewCanvas, SLOT(addLayer(QgsMapLayer *)));


    if (! myHideSplashFlag)
    {
        gSplashScreen->setStatus(tr("Setting theme..."));
        qApp->processEvents();
    }

    // get the users theme preference from the settings
    QString themeName = settings.readEntry("/qgis/theme","default");

    // set the theme
    setTheme(themeName);
    setupToolbarPopups(themeName);

    // Set the initial visibility flag for layers
    // This user option allows the user to turn off inital drawing of
    // layers when they are added to the map. This is useful when adding
    // many layers and the user wants to adjusty symbology, etc prior to
    // actually viewing the layer.
    mAddedLayersHidden = settings.readBoolEntry("/qgis/new_layers_visible", true);

    if (! myHideSplashFlag)
    {
        gSplashScreen->setStatus(tr("QGIS Ready"));

        // Set up the timer so the splash screen stays up for
        // another 1 seconds, then kill it.
        QTimer::singleShot( 1000, this, SLOT(killSplashScreen()) );
        qApp->processEvents();
    }
    // set the focus to the map canvase
    mMapCanvas->setFocus();

    // Map composer
    mComposer = new QgsComposer(this);

} // QgisApp ctor



QgisApp::~QgisApp()
{}


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
    watsNew += "<h3>New features</h3>";
    watsNew += "<ul><li>"
               + tr("GEOS support in the OGR provider to refine selection of features via identify. This improves over the previous method of feature selection which used a simple MBR intersection check.")
               + "<li>"
               + tr("PostGIS editing support in provider")
               + "<li>"
               + tr("Vector dialog redesign to improve usability")
               + "<li>"
               + tr("Improvement in project handling (loading and saving)")
               + "<li>"
               + tr("Scale dependent rendering")
               + "<li>"
               + tr("User option to load layers with out drawing them, thus allowing you to set scale dependency, etc without waiting for the initial draw to complete")
               + "<li>"
               + tr("Attribute actions - the ability to run an external program based on the contents of an attribute field in a layer")
               + "<li>"
               + tr("Create new vector layer (shapefile) for editing")
               + "<li>"
               + tr("Windows installer")
               + "<li>"
               + tr("Mac OSX binary")
               + "<li>"
               + tr("New options in the graticule builder plugin")
               + "<li>"
               + tr("Enhancements to the GPS plugin")
               + "<li>"
               + tr("Man page")
               + "<li>"
               + tr("Save delimited text as shapefile")
               + "<li>"
               + tr("Improved Delimited Text plugin, including preview of text file")
               + "<li>"
               + tr("Improved SPIT handling of PostgreSQL reserved words and shapefiles with multiple geometry types")
               + "<li>"
               + tr("Display SQL query used to create a PostGIS layer")
               + "<li>"
               + tr("PostgreSQL query builder")
               + "<li>"
               + tr("Ability to redefine the query used for PostgreSQL layers from the layer properties dialog")
               + "<li>"
               + tr("North arrow, scalebar, and copyright plugins save their state in the project file")
               + "<li>"
               + tr("Datasets with UTF8, Kanjii and CJK filenames now load properly")
               + "</ul></body></html>";


    abt->setWhatsNew(watsNew);

    // add the available plugins to the list
    QString providerInfo = "<b>" + tr("Available Data Provider Plugins") + "</b><br>";
    abt->setPluginInfo(providerInfo + mProviderRegistry->pluginList(true));
    QApplication::restoreOverrideCursor();
    abt->exec();


}

/** Load up any plugins used in the last session
 */

void QgisApp::restoreSessionPlugins(QString thePluginDirString)
{
  
  QSettings mySettings;
  #ifdef QGISDEBUG
  
  std::cerr << " -------------------- Restoring plugins from last session " << thePluginDirString << std::endl;
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
      
      std::cerr << "Examining " << myFullPath << std::endl;
      #endif
      
      QLibrary *myLib = new QLibrary(myFullPath);
      bool loaded = myLib->load();
      if (loaded)
      {
        //purposely leaving this one to stdout!
        std::cout << "Loaded " << myLib->library() << std::endl;
        name_t * myName =(name_t *) myLib->resolve("name");
        description_t *  myDescription = (description_t *)  myLib->resolve("description");
        version_t *  myVersion =  (version_t *) myLib->resolve("version");
        if (myName && myDescription  && myVersion )
        {
          //check if the plugin was active on last session
          QString myEntryName = myName();
          // Windows stores a "true" value as a 1 in the registry so we
          // have to use readBoolEntry in this function
          
          if (mySettings.readBoolEntry("/qgis/Plugins/" + myEntryName))
          {
            #ifdef QGISDEBUG
            std::cerr << " -------------------- loading " << myEntryName << std::endl;
            #endif
            
            loadPlugin(myName(), myDescription(), myFullPath);
          }
        }
        else
        {
          #ifdef QGISDEBUG
          std::cerr << "Failed to get name, description, or type for " << myLib->library() << std::endl;
          #endif
          
        }
      }
      else
      {
        std::cerr << "Failed to load " << myLib->library() << std::endl;
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

   We query OGR for a list of supported raster formats; we then build
   a list of file filter strings from that list.  We return a string
   that contains this list that is suitable for use in a a
   QFileDialog::getOpenFileNames() call.

   XXX Most of the file name filters need to be filled in; however we
   XXX may want to wait until we've tested each format before committing
   XXX them permanently instead of blindly relying on OGR to properly
   XXX supply all needed spatial data.

 */
static void buildSupportedVectorFileFilter_(QString & fileFilters)
{

    static QString myFileFilters = "";

    // if we've already built the supported vector string, just return what
    // we've already built
    if ( ! myFileFilters.isEmpty() )
    {
        fileFilters = myFileFilters;

        return;
    }

    // first get the GDAL driver manager

    OGRSFDriverRegistrar *driverRegistrar = OGRSFDriverRegistrar::GetRegistrar();

    if (!driverRegistrar)
    {
        QMessageBox::warning(0,"OGR Driver Manger","unable to get OGRDriverManager");
        return;                   // XXX good place to throw exception if we
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
            qDebug( "%s:%d unknown driver %s", __FILE__, __LINE__, driverName.ascii() );
        }

    }                           // each loaded GDAL driver

    std::cout << myFileFilters << std::endl;

    // can't forget the default case

    myFileFilters += "All files (*.*)";
    fileFilters = myFileFilters;

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

   Stores persistent settings under /qgis/UI/.  The sub-keys will be
   filterName and filterName + "Dir".

   Opens dialog on last directory associated with the filter name, or
   the current working directory if this is the first time invoked
   with the current filter name.

*/
static void openFilesRememberingFilter_(QString const &filterName, 
  QString const &filters, QStringList & selectedFiles, QString& enc, QString &title)
{

    bool haveLastUsedFilter = false;  // by default, there is no last
    // used filter

    QSettings settings;           // where we keep last used filter in
    // persistant state

    QString lastUsedFilter = settings.readEntry("/qgis/UI/" + filterName,
                             QString::null,
                             &haveLastUsedFilter);

    QString lastUsedDir = settings.readEntry("/qgis/UI/" + filterName + "Dir",
                          ".");

    QString lastUsedEncoding = settings.readEntry("/qgis/UI/encoding");

#ifdef QGISDEBUG

    std::cerr << "Opening vector file dialog with filters: " << filters << std::endl;
#endif

    QgsEncodingFileDialog* openFileDialog = new QgsEncodingFileDialog(lastUsedDir, filters, 0, QFileDialog::tr("open files dialog"), lastUsedEncoding);

    // allow for selection of more than one file
    openFileDialog->setMode(QFileDialog::ExistingFiles);
    openFileDialog->setCaption(title);

    if (haveLastUsedFilter)       // set the filter to the last one used
    {
        openFileDialog->setSelectedFilter(lastUsedFilter);
    }

    if (openFileDialog->exec() == QDialog::Accepted)
    {
        selectedFiles = openFileDialog->selectedFiles();
  enc = openFileDialog->encoding();
    }

    settings.writeEntry("/qgis/UI/" + filterName, openFileDialog->selectedFilter());


    settings.writeEntry("/qgis/UI/" + filterName + "Dir", openFileDialog->dirPath());
    settings.writeEntry("/qgis/UI/encoding", openFileDialog->encoding());

    delete openFileDialog;
}                               // openFilesRememberingFilter_




/**
   This method prompts the user for a list of vector filenames with a dialog.

   @todo XXX I'd really like to return false, but can't because this
         XXX is for a slot that was defined void; need to fix.
*/
void QgisApp::addLayer()
{

    QString fileFilters;

    buildSupportedVectorFileFilter_(fileFilters);

    //qDebug( "vector file filters: " + fileFilters );

    QString pOgr = mProviderRegistry->library("ogr");

    if (pOgr.isEmpty())
    {
#ifdef QGSDEBUG
        qDebug("unable to get OGR registry");
#endif

        return;
    }
    else
    {
        mMapCanvas->freeze();

        QStringList selectedFiles;
#ifdef QGISDEBUG

        std::cerr << "Vector file filters: " << fileFilters << std::endl;
#endif

  QString enc;
  QString title = tr("Open an OGR Supported Vector Layer");
        openFilesRememberingFilter_("lastVectorFileFilter", fileFilters, selectedFiles, enc,
          title);
        if (selectedFiles.isEmpty())
        {
            // no files were selected, so just bail
            mMapCanvas->freeze(false);

            return;
        }

        addLayer(selectedFiles, enc);
    }
}                               // QgisApp::addLayer()




bool QgisApp::addLayer(QFileInfo const & vectorFile)
{
    // check to see if we have an ogr provider available
    QString pOgr = mProviderRegistry->library("ogr");

    if ( pOgr.isEmpty() )
    {
        QMessageBox::critical(this,
                              tr("No OGR Provider"),
                              tr("No OGR data provider was found in the QGIS lib directory"));
        return false;
    }

    // let the user know we're going to possibly be taking a while
    QApplication::setOverrideCursor(Qt::WaitCursor);

    mMapCanvas->freeze();         // XXX why do we do this?

    // create the layer

    QgsVectorLayer *layer = new QgsVectorLayer(vectorFile.filePath(),
                            vectorFile.baseName(),
                            "ogr");
    Q_CHECK_PTR( layer );

    if ( ! layer )
    {
        mMapCanvas->freeze(false);
        QApplication::restoreOverrideCursor();

        // XXX insert meaningful whine to the user here
        return false;
    }

    if (layer->isValid())
    {
        // Register this layer with the layers registry
        QgsMapLayerRegistry::instance()->addMapLayer(layer);
        // init the context menu so it can connect to slots
        // in main app
        // XXX move to legend::addLayer() layer->initContextMenu(this);

        // XXX What about the rest of these?  Where should they be moved, if at
        // XXX all?  Some of this functionality is taken care of in the
        // XXX QgsProject::read() (If layers added via that.)

        //add single symbol renderer as default
        QgsSingleSymRenderer *renderer = new QgsSingleSymRenderer();

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
        renderer->initializeSymbology(layer);
        // not necessary since registry will add to canvas mMapCanvas->addLayer(layer);
        // XXX some day will not necessary since connect up a request from
        // the raster layer to show in overview map
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
        QString msg = vectorFile.baseName() + " ";
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
    mMapCanvas->render();        // XXX eh, wot?

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
    // check to see if we have an ogr provider available
    QString pOgr = mProviderRegistry->library("ogr");

    if ( pOgr.isEmpty() )
    {
        QMessageBox::critical(this,
                              tr("No OGR Provider"),
                              tr("No OGR data provider was found in the QGIS lib directory"));
        return false;
    }
    else
    {
        mMapCanvas->freeze();

        QApplication::setOverrideCursor(Qt::WaitCursor);


        for ( QStringList::ConstIterator it = theLayerQStringList.begin();
                it != theLayerQStringList.end();
                ++it )
        {
            QFileInfo fi(*it);
            QString base = fi.baseName();


            // create the layer

            QgsVectorLayer *layer = new QgsVectorLayer(*it, base, "ogr");

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
    layer->getDataProvider()->setEncoding(enc);
                //Register the layer with the layer registry
                QgsMapLayerRegistry::instance()->addMapLayer(layer);
                // init the context menu so it can connect to slots
                // in main app

                // XXX now taken care of in legend layer->initContextMenu(this);

                //add single symbol renderer as default
                QgsSingleSymRenderer *renderer = new QgsSingleSymRenderer();

                Q_CHECK_PTR( renderer );

                if ( ! renderer )
                {
                    mMapCanvas->freeze(false);
                    QApplication::restoreOverrideCursor();

                    // XXX insert meaningful whine to the user here
                    return false;
                }

                layer->setRenderer(renderer);
                renderer->initializeSymbology(layer);

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
        mMapCanvas->render();
        QApplication::restoreOverrideCursor();
        statusBar()->message(mMapCanvas->extent().stringRep(2));

    }

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



#ifdef HAVE_POSTGRESQL
void QgisApp::addDatabaseLayer()
{
    // check to see if we have a postgres provider available
    QString pOgr = mProviderRegistry->library("postgres");

    if ( ! pOgr.isNull() )
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
		
		if(layer->getDataProvider())
		  {
		    layer->getDataProvider()->setEncoding(dbs->encoding());
		  }

                if (layer->isValid())
                {
                    // set initial visibility based on user preference
                    layer->setVisible(mAddedLayersHidden);

                    // register this layer with the central layers registry
                    QgsMapLayerRegistry::instance()->addMapLayer(layer);

                    // init the context menu so it can connect to slots in main app
                    // XXX now taken care of in legend layer->initContextMenu(this);

                    // give it a random color
                    QgsSingleSymRenderer *renderer = new QgsSingleSymRenderer();  // add single symbol renderer as default
                    layer->setRenderer(renderer);
                    renderer->initializeSymbology(layer);
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
                    std::cerr << *it << " is an invalid layer - not loaded" << std::endl;
                    QMessageBox::critical(this, tr("Invalid Layer"), tr("%1 is an invalid layer and cannot be loaded.").arg(*it));
                    delete layer;
                }
                //qWarning("incrementing iterator");
                ++it;
            }
            statusBar()->message(mMapCanvas->extent().stringRep(2));
        }

	QSettings settings;
	settings.writeEntry("/qgis/UI/encoding", dbs->encoding());

        qApp->processEvents();

        mMapCanvas->freeze(false);
        mMapCanvas->render();
        QApplication::restoreOverrideCursor();
    }
    else
    {
        QMessageBox::critical(this, tr("No PostgreSQL Provider"), tr("No PostgreSQL data provider was found in the QGIS lib directory"));

    }
}
#endif
void QgisApp::fileExit()
{
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

    QgsGeomTypeDialog geomDialog;
    if(geomDialog.exec()==QDialog::Rejected)
    {
        return;
    }
    geometrytype = geomDialog.selectedType();

    std::list<std::pair<QString, QString> > attributes;
    geomDialog.attributes(attributes);

    QString filename=QFileDialog::getSaveFileName();
    if(filename.isNull())
    {
        //file dialog rejected
        return;
    }
    // check to see if user specified the extension. if not, add it...
    if(filename.find(QRegExp("\\.shp$")) == -1)
    {
      filename += ".shp";
    }
    //strange, the 'provider way' does not work...
    /*QgsProviderRegistry * pReg = QgsProviderRegistry::instance();
    QString ogrlib = pReg->library("ogr");
    const char *cOgrLib = (const char *) ogrlib;
    // load the data provider
    QLibrary* myLib = new QLibrary((const char *) ogrlib);
    bool loaded = myLib->load();
    if (loaded)
    {
    #ifdef QGISDEBUG
        qWarning("ogr provider loaded");
    #endif
        typedef bool (*createEmptyDataSourceProc)(const QString&,const QString&, QGis::WKBTYPE);
        createEmptyDataSourceProc createEmptyDataSource=(createEmptyDataSourceProc)myLib->resolve("createEmptyDataSource");
        if(createEmptyDataSource)
        {
            QString format("ESRI Shapefile");
            if(geometrytype == QGis::WKBPoint)
            {
                createEmptyDataSource(filename,format,QGis::WKBPoint);
            }
            else if (geometrytype == QGis::WKBLineString)
            {
                createEmptyDataSource(filename,format,QGis::WKBLineString);
            }
            else if(geometrytype == QGis::WKBPolygon)
            {
                createEmptyDataSource(filename,format,QGis::WKBPolygon);
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
        }*/

    QgsVectorFileWriter* writer=0;

    if(geometrytype == QGis::WKBPoint)
    {
        writer=new QgsVectorFileWriter(filename,wkbPoint);
        if(!writer->initialise())
        {
            QMessageBox::warning(0,"Warning","Writing of the layer failed",QMessageBox::Ok,QMessageBox::NoButton);
            return;
        }
        //writer.createField("dummy", OFTReal, 1, 1);
    }
    else if(geometrytype == QGis::WKBLineString)
    {
        writer=new QgsVectorFileWriter(filename,wkbLineString);
        if(!writer->initialise())
        {
            QMessageBox::warning(0,"Warning","Writing of the layer failed",QMessageBox::Ok,QMessageBox::NoButton);
            return;
        }
        //writer.createField("dummy", OFTReal, 1, 1);
    }
    else if(geometrytype == QGis::WKBPolygon)
    {
        writer=new QgsVectorFileWriter(filename,wkbPolygon);
        if(!writer->initialise())
        {
            QMessageBox::warning(0,"Warning","Writing of the layer failed",QMessageBox::Ok,QMessageBox::NoButton);
            return;
        }
        //writer.createField("dummy", OFTReal, 1, 1);
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
	    if(it->second=="OFTReal")
	    {
		writer->createField(it->first, OFTReal, 10, 3);
	    }
	    else if(it->second=="OFTInteger")
	    {
		writer->createField(it->first, OFTInteger, 10, 3);
	    }
	    else if(it->second=="OFTString")
	    {
		writer->createField(it->first, OFTString, 40, 1);
	    }
	}
    }
    delete writer;

    //then add the layer to the view
    QFileInfo fileinfo(filename);
    addLayer(fileinfo);
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
        QString lastUsedDir = settings.readEntry("/qgis/UI/lastProjectDir", ".");
        
        QFileDialog * openFileDialog = new QFileDialog(lastUsedDir, QObject::tr("QGis files (*.qgs)"), 0, 
                                                        QFileDialog::tr("Choose a QGIS project file to open"));
        openFileDialog->setMode(QFileDialog::ExistingFile);
        
        QString fullPath;
        if (openFileDialog->exec() == QDialog::Accepted)
        {
            fullPath = openFileDialog->selectedFile();
        } else {
            // if they didn't select anything, just return
            delete openFileDialog;
            return;
        }
        
        // Persist last used project dir
        settings.writeEntry("/qgis/UI/lastProjectDir", openFileDialog->dirPath());
        
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
        catch ( std::exception & e )
        {
            QMessageBox::critical(this, 
                                  tr("QGIS Project Read Error"), 
                                  tr("") + "\n" + e.what() );
        }
    }

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

            emit projectRead();       // let plug-ins know that we've read in a new
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
    catch ( std::exception & e )
    {
        QMessageBox::critical( 0x0, "Unable to open project", e.what(), QMessageBox::Ok, QMessageBox::NoButton );

        return false;
    }

    return true;
} // QgisApp::addProject(QString projectFile)



void QgisApp::fileSave()
{
    // if we don't have a filename, then obviously we need to get one; note
    // that the project file name is reset to null in fileNew()
    QString fullPath;
    bool isNewProject = FALSE;
    if ( QgsProject::instance()->filename().isNull() )
    {
        isNewProject = TRUE;
        
        // Retrieve last used project dir from persistent settings
        QSettings settings;
        QString lastUsedDir = settings.readEntry("/qgis/UI/lastProjectDir", ".");
        
        QFileDialog * saveFileDialog = new QFileDialog(lastUsedDir, QObject::tr("QGis files (*.qgs)"), 0, 
                                                        QFileDialog::tr("Choose a QGIS project file"));
        saveFileDialog->setMode(QFileDialog::AnyFile);
        
        if (saveFileDialog->exec() == QDialog::Accepted)
        {
            fullPath = saveFileDialog->selectedFile();
        } else {
            // if they didn't select anything, just return
            delete saveFileDialog;
            return;
        }
        
        delete saveFileDialog;
        
        // make sure we have the .qgs extension in the file name
        if(fullPath.find(QRegExp("\\.qgs$")) == -1)
        {
          fullPath += ".qgs";
        }

        QgsProject::instance()->filename( fullPath );
    }

    try
    {
        if ( QgsProject::instance()->write() )
        {
            statusBar()->message(tr("Saved map to:") + " " + QgsProject::instance()->filename() );
            
            if (isNewProject)
            {
                // add this to the list of recently used project files
                QSettings settings;
                saveRecentProjectPath(fullPath, settings);
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
                               "Unable to save project " + QgsProject::instance()->filename(),
                               e.what(),
                               QMessageBox::Ok,
                               QMessageBox::NoButton );

    }

} // QgisApp::fileSave



void QgisApp::fileSaveAs()
{
    // Retrieve last used project dir from persistent settings
    QSettings settings;
    QString lastUsedDir = settings.readEntry("/qgis/UI/lastProjectDir", ".");
    
    QFileDialog * saveFileDialog = new QFileDialog(lastUsedDir, QObject::tr("QGis files (*.qgs)"), 0, 
                                                    QFileDialog::tr("Choose a QGIS project file"));
    saveFileDialog->setMode(QFileDialog::AnyFile);
    
    QString fullPath;
    if (saveFileDialog->exec() == QDialog::Accepted)
    {
        fullPath = saveFileDialog->selectedFile();
    } else {
        // if they didn't select anything, just return
        delete saveFileDialog;
        return;
    }
    
    // Persist last used project dir
    settings.writeEntry("/qgis/UI/lastProjectDir", saveFileDialog->dirPath());
    
    delete saveFileDialog;
    
    // make sure the .qgs extension is included in the path name. if not, add it...
    if (fullPath.find(QRegExp("\\.qgs$")) == -1)
    {
      fullPath += ".qgs";
    }

    QgsProject::instance()->filename( fullPath );

    if ( QgsProject::instance()->write() )
    {
        statusBar()->message(tr("Saved map to:") + " " + QgsProject::instance()->filename() );
        // add this to the list of recently used project files
        saveRecentProjectPath(fullPath, settings);
    }
    else
    {
        QMessageBox::critical(this,
                              tr("Unable to save project"),
                              tr("Unable to save project to ") + QgsProject::instance()->filename() );
    }
} // QgisApp::fileSaveAs


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
    settings.writeEntry("/qgis/UI/recentProjectsList", mRecentProjectPaths);
    
    // Update the file menu with the changed list
    // Find the item before the paths in the menu
    uint currentIndex = popupMenuFile->indexOf(BEFORE_RECENT_PATHS);
    currentIndex++;
    // Remove existing paths from the file menu
    while (currentIndex != popupMenuFile->indexOf(AFTER_RECENT_PATHS))
    {
        popupMenuFile->removeItemAt(currentIndex);
    }
    // Add back in the updated list of paths
    updateRecentProjectPaths();
} // QgisApp::saveRecentProjectPath


// Update file menu with the current list of recently accessed projects
void QgisApp::updateRecentProjectPaths()
{
    int currentIndex = popupMenuFile->indexOf(AFTER_RECENT_PATHS);
    popupMenuFile->insertSeparator(currentIndex++);
    if (mRecentProjectPaths.size() > 0)
    {
        uint IdCounter = 1;
        for ( QStringList::Iterator it = mRecentProjectPaths.begin();
                it != mRecentProjectPaths.end();
                ++it )
        {
            int myId = IdCounter++;
            popupMenuFile->insertItem((*it), this, SLOT(openProject(int)), 0, myId, currentIndex++);
            // This parameter corresponds to this path's index
            // in mRecentProjectPaths
            popupMenuFile->setItemParameter(myId, myId - 1);
            // Disable this menu item if the file has been removed
            if (!QFile::exists((*it)))
            {
                popupMenuFile->setItemEnabled(myId, FALSE);
            }
        }
        popupMenuFile->insertSeparator(currentIndex);
    }
} // QgisApp::updateRecentProjectPaths


// Open the project file corresponding to the
// path at the given index in mRecentProjectPaths
void QgisApp::openProject(int pathIndex)
{
    // possibly save any pending work before opening a different project
    int answer = saveDirty();

    if (answer != QMessageBox::Cancel)
    {
        QStringList::Iterator it = mRecentProjectPaths.at(pathIndex);
        addProject((*it));
    }
} // QgisApp::openProject

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
    QString myLastUsedFilter = myQSettings.readEntry("/qgis/UI/saveAsImageFilter");
    QString myLastUsedDir = myQSettings.readEntry("/qgis/UI/lastSaveAsImageDir",".");


    // get a list of supported output image types
    int myCounterInt=0;
    QString myFilters;
    for ( ; myCounterInt < QImageIO::outputFormats().count(); myCounterInt++ )
    {
        QString myFormat=QString(QImageIO::outputFormats().at( myCounterInt ));
        QString myFilter = createFileFilter_(myFormat + " format", "*."+myFormat);
        myFilters += myFilter;
        myFilterMap[myFilter] = myFormat;
    }
#ifdef QGISDEBUG
    std::cout << "Available Filters Map: " << std::endl;
    FilterMap::Iterator myIterator;
    for ( myIterator = myFilterMap.begin(); myIterator != myFilterMap.end(); ++myIterator )
    {
        std::cout << myIterator.key() << "  :  " << myIterator.data() << std::endl;
    }

#endif

    //create a file dialog using the the filter list generated above
    std::auto_ptr < QFileDialog > myQFileDialog(
        new QFileDialog(
            myLastUsedDir,
            myFilters,
            0,
            QFileDialog::tr("Save file dialog"),
            tr("Choose a filename to save the map image as")
        )
    );


    // allow for selection of more than one file
    myQFileDialog->setMode(QFileDialog::AnyFile);

    if (myLastUsedFilter!=QString::null)       // set the filter to the last one used
    {
        myQFileDialog->setSelectedFilter(myLastUsedFilter);
    }


    //prompt the user for a filename
    QString myOutputFileNameQString; // = myQFileDialog->getSaveFileName(); //delete this
    if (myQFileDialog->exec() == QDialog::Accepted)
    {
        myOutputFileNameQString = myQFileDialog->selectedFile();
    }

    QString myFilterString = myQFileDialog->selectedFilter()+";;";
#ifdef QGISDEBUG

    std::cout << "Selected filter: " << myFilterString << std::endl;
    std::cout << "Image type to be passed to mapcanvas: " << myFilterMap[myFilterString] << std::endl;
#endif

    myQSettings.writeEntry("/qgis/UI/lastSaveAsImageFilter" , myFilterString);
    myQSettings.writeEntry("/qgis/UI/lastSaveAsImageDir", myQFileDialog->dirPath());

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
    mOverviewCanvas->render();

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
    mOverviewCanvas->render();

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
    mMapCanvas->render();
    mOverviewCanvas->render();

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
    mMapCanvas->render();
    mOverviewCanvas->render();

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
        QMessageBox::warning(this, "No Map Layers",
                             "No layers to export. You must add at least one layer to the map in order to export the view.");
    }
}
void QgisApp::zoomIn()
{
    /*  QWMatrix m = mMapCanvas->worldMatrix();
       m.scale( 2.0, 2.0 );
       mMapCanvas->setWorldMatrix( m );
     */

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
    actionIdentify->setOn(true);
}

void QgisApp::measure()
{
    mMapTool = QGis::Measure;
    mMapCanvas->setMapTool(mMapTool);

    //QPixmap pm = QPixmap((const char **)  capture_point_cursor);
    //delete mMapCursor;
    //mMapCursor = new QCursor(pm, 8, 8);
    mMapCanvas->setCursor( Qt::CrossCursor );
    actionMeasure->setOn(true);
}

void QgisApp::stopZoom() 
{
    actionZoomIn->setOn(false);
    actionZoomIn->setOn(false);
    actionPan->setOn(false);

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
  case QGis::Measure:
      measure();
      break;
    }
}


void QgisApp::attributeTable()
{
    QListViewItem *li = mMapLegend->currentItem();
    if (li)
    {
        QgsMapLayer *layer = ((QgsLegendItem *) li)->layer();
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
}

void QgisApp::deleteSelected()
{
#ifdef QGISDEBUG
    qWarning("In slot QgisApp::deleteSelected");
#endif

    QListViewItem *li = mMapLegend->currentItem();
    if (li)
    {
        QgsVectorLayer* vlayer = dynamic_cast<QgsVectorLayer*>(((QgsLegendItem *) li)->layer());
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
    
  actionCapturePoint->setOn(true);
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

  actionCaptureLine->setOn(true);
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
  
  actionCapturePolygon->setOn(true);
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
    actionSelect->setOn(true);
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
    mMapCanvas->render();

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

void QgisApp::layerProperties()
{
    layerProperties(mMapLegend->currentItem());
}

void QgisApp::layerProperties(QListViewItem * lvi)
{
    QgsMapLayer *layer;
    if (lvi)
    {
        layer = ((QgsLegendItem *) lvi)->layer();
    }
    else
    {
        // get the selected item
        QListViewItem *li = mMapLegend->currentItem();
        layer = ((QgsLegendItem *) li)->layer();
    }



    QString currentName = layer->name();
    //test if we have a raster or vector layer and show the appropriate dialog
    if (layer->type() == QgsMapLayer::RASTER)
    {
        QgsRasterLayerProperties *rlp = new QgsRasterLayerProperties(layer);
        // The signals to change the raster layer properties will only be emitted
        // when the user clicks ok or apply
        //connect(rlp, SIGNAL(setTransparency(unsigned int)), SLOT(layer(slot_setTransparency(unsigned int))));
        if (rlp->exec())
        {
            //this code will be called it the user selects ok
            mMapCanvas->setDirty(true);
            mMapCanvas->refresh();
            mMapCanvas->render();
            // mMapLegend->update(); XXX WHY CALL UPDATE HERE?
            delete rlp;
            qApp->processEvents();
        }
    }
    else
    {
        layer->showLayerProperties();
    }

    //TODO Fix this area below and above
    //this is a very hacky way to force the legend entry to refresh - the call above does ne happen for some reason
    //mMapCanvas->render();
    //mMapLegend->update();


    /* else if ((layer->type()==QgsMapLayer::VECTOR) || (layer->type()==QgsMapLayer::DATABASE))
       {
       QgsLayerProperties *lp = new QgsLayerProperties(layer);
       if (lp->exec()) {
    // update the symbol
    layer->setSymbol(lp->getSymbol());
    mMapCanvas->freeze();
    layer->setlayerName(lp->displayName());
    if (currentName != lp->displayName())
    mMapLegend->update();
    delete lp;
    qApp->processEvents();

    // apply changes
    mMapCanvas->freeze(false);
    mMapCanvas->setDirty(true);
    mMapCanvas->render();
    }
    }
    else if (layer->type()==QgsMapLayer::DATABASE)
    {
    //do me!
    QMessageBox::information( this, "QGis",
    "Properties box not yet implemented for database layer");
    }
    else
    {
    QMessageBox::information( this, "QGis",
    "Unknown Layer Type");
    }

    */



    //  layer->showLayerProperties();
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
            actionRemoveLayer->setEnabled(false);
            actionRemoveAllFromOverview->setEnabled(false);
            actionInOverview->setEnabled(false);
            actionShowAllLayers->setEnabled(false);
            actionHideAllLayers->setEnabled(false);
            actionOpenTable->setEnabled(false);
            actionLayerProperties->setEnabled(false);

            enabled = false;
        }
        else
        {
            if ( ! enabled )
            {
                actionRemoveLayer->setEnabled(true);
                actionRemoveAllFromOverview->setEnabled(true);
                actionInOverview->setEnabled(true);
                actionShowAllLayers->setEnabled(true);
                actionHideAllLayers->setEnabled(true);
                actionOpenTable->setEnabled(true);
                actionLayerProperties->setEnabled(true);
            }
        }
    }
} // QgisApp::menubar_highlighted( int i )





void QgisApp::inOverview( bool in_overview )
{
  #ifdef QGISDEBUG
  std::cout << "QGisApp::inOverview(" << in_overview << ")" << std::endl;
  #endif
  
  QListViewItem *lvi = mMapLegend->currentItem();
  
  // check to make sure there is a current layer
  // TODO: We really need to set disable/enable all menu options based on a logical scheme. 
  //       This is just a hack...
  if(lvi)
  {
    QgsMapLayer *layer = ((QgsLegendItem *) lvi)->layer();
    
    layer->inOverview( in_overview );
    mOverviewCanvas->render();
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
  QListViewItem *lvi = mMapLegend->currentItem();
  if(lvi)
  {
    QgsMapLayer *layer = ((QgsLegendItem *) lvi)->layer();
    //call the registry to unregister the layer. It will in turn
    //fire a qt signal to notify any objects using that layer that they should
    //remove it immediately
    QgsMapLayerRegistry::instance()->removeMapLayer(layer->getLayerID());
    mOverviewCanvas->freeze(false);
    // draw the map
    mOverviewCanvas->zoomFullExtent();
    mOverviewCanvas->clear();
    mOverviewCanvas->render();
    mMapCanvas->freeze(false);

    //remove remaining digitising acetates
    mMapCanvas->removeEditingAcetates();

    // draw the map
    mMapCanvas->clear();
    mMapCanvas->render();
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
    QListViewItem *li = mMapLegend->currentItem();
    QgsMapLayer *layer = ((QgsLegendItem *) li)->layer();
    // the layer extent has to be transformed to the map canvas
    // coordinate system 
    QgsCoordinateTransform *ct = layer->coordinateTransform();
    QgsRect transformedExtent = ct->transform(layer->extent());
    mMapCanvas->setExtent(transformedExtent);
    mMapCanvas->clear();
    mMapCanvas->render();

    // notify the project we've made a change
    QgsProject::instance()->dirty(true);
  }
}

void QgisApp::rightClickLegendMenu(QListViewItem * lvi, const QPoint & pt, int)
{
    if (!mMapCanvas->isDrawing()&&lvi)
    {
        // get the context menu from the layer and display it
        QgsMapLayer *layer = ((QgsLegendItem *) lvi)->layer();
        QPopupMenu *mPopupMenu = layer->contextMenu();
        if (mPopupMenu)
        {
            mPopupMenu->exec(pt);
        }
    }
}

void QgisApp::currentLayerChanged(QListViewItem * lvi)
{
    if (lvi)
    {
        // disable/enable toolbar buttons as appropriate based on selected
        // layer type

  toolPopupCapture->setItemEnabled(0,FALSE);
  toolPopupCapture->setItemEnabled(1,FALSE);
  toolPopupCapture->setItemEnabled(2,FALSE);
  toolPopupCapture->setItemEnabled(3,FALSE);

        QgsMapLayer *layer = ((QgsLegendItem *) lvi)->layer();
        if (layer->type() == QgsMapLayer::RASTER)
        {
            //actionIdentify->setEnabled(FALSE);
            actionSelect->setEnabled(FALSE);
            actionOpenTable->setEnabled(FALSE);
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
            QgsVectorLayer* vlayer=dynamic_cast<QgsVectorLayer*>(((QgsLegendItem *) lvi)->layer());
            if(vlayer)
            {
    QgsVectorDataProvider* provider=vlayer->getDataProvider();
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

            actionIdentify->setEnabled(TRUE);
            actionSelect->setEnabled(TRUE);
            actionOpenTable->setEnabled(TRUE);
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
  mMapCanvas->removeEditingAcetates();

        // notify the project we've made a change
        QgsProject::instance()->dirty(true);
    }
} // QgisApp::currentLayerChanged


QgisIface *QgisApp::getInterface()
{
    return mQgisInterface;
}

void QgisApp::actionPluginManager_activated()
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

        std::cerr << "Library name is " << myLib->library() << std::endl;
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
                            settings.writeEntry("/qgis/Plugins/" + name, true);
                        }
                        else
                        {
                            // something went wrong
                            QMessageBox::warning(this, tr("Error Loading Plugin"), tr("There was an error loading %1."));
                            //disable it to the qsettings file [ts]
                            settings.writeEntry("/qgis/Plugins/" + name, false);
                        }
                    }
                    else
                    {
                        //#ifdef QGISDEBUG
                        std::cerr << "Unable to find the class factory for " << theFullPathName << std::endl;
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
                            settings.writeEntry("/qgis/Plugins/" + name, true);

                        }
                        else
                        {
                            // something went wrong
                            QMessageBox::warning(this, tr("Error Loading Plugin"), tr("There was an error loading %1."));
                            //add it to the qsettings file [ts]
                            settings.writeEntry("/qgis/Plugins/" + name, false);
                        }
                    }
                    else
                    {
                        //#ifdef QGISDEBUG
                        std::cerr << "Unable to find the class factory for " << theFullPathName << std::endl;
                        //#endif
                    }
                }
                break;
            default:
                // type is unknown
                //#ifdef QGISDEBUG
                std::cerr << "Plugin " << theFullPathName << " did not return a valid type and cannot be loaded" << std::endl;
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
            std::cerr << "Failed to load " << theFullPathName << "\n";
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
            std::cout << "Getting information for plugin: " << mlpDir[i] << std::endl;
            std::cout << "Attempting to load the plugin using dlopen\n";
#endif
            //          void *handle = dlopen("../plugins/maplayer/" + mlpDir[i], RTLD_LAZY);
            void *handle = dlopen("../plugins/maplayer/" + mlpDir[i], RTLD_LAZY | RTLD_GLOBAL );
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

            std::cout << "Library name is " << myLib->library() << std::endl;
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
                std::cout << "Failed to load " << mlpDir[i] << "\n";
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
                std::cout << "Getting information for plugin: " << pluginDir[i] << std::endl;
#endif

                QLibrary *myLib = new QLibrary("../plugins/" + pluginDir[i]); //"/home/gsherman/development/qgis/plugins/" + pluginDir[i]);
#ifdef QGISDEBUG

                std::cout << "Library name is " << myLib->library() << std::endl;
#endif
                //QLibrary myLib("../plugins/" + pluginDir[i]);
#ifdef QGISDEBUG

                std::cout << "Attempting to load " << "../plugins/" + pluginDir[i] << std::endl;
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
                        std::cout << "Plugin name: " << pl->name() << std::endl;
                        std::cout << "Plugin version: " << pl->version() << std::endl;
                        std::cout << "Plugin description: " << pl->description() << std::endl;
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

void QgisApp::saveWindowState()
{
    // store window and toolbar positions
    QSettings settings;

    QString dockStatus;
    QTextStream ts(&dockStatus, IO_WriteOnly);
    ts << *this;
    settings.writeEntry("/qgis/Geometry/ToolBars", dockStatus);
    // store window geometry
    QPoint p = this->pos();
    QSize s = this->size();
    settings.writeEntry("/qgis/Geometry/maximized", this->isMaximized());
    settings.writeEntry("/qgis/Geometry/x", p.x());
    settings.writeEntry("/qgis/Geometry/y", p.y());
    settings.writeEntry("/qgis/Geometry/w", s.width());
    settings.writeEntry("/qgis/Geometry/h", s.height());

}

void QgisApp::restoreWindowState()
{
    QSettings settings;

    QString dockStatus = settings.readEntry("/qgis/Geometry/ToolBars");
    QTextStream ts(&dockStatus, IO_ReadOnly);
    ts >> *this;

    // restore window geometry
    QDesktopWidget *d = QApplication::desktop();
    int dw = d->width();          // returns desktop width
    int dh = d->height();         // returns desktop height
    int w = settings.readNumEntry("/qgis/Geometry/w", 600);
    int h = settings.readNumEntry("/qgis/Geometry/h", 400);
    int x = settings.readNumEntry("/qgis/Geometry/x", (dw - 600) / 2);
    int y = settings.readNumEntry("/qgis/Geometry/y", (dh - 400) / 2);
    resize(w, h);
    move(x, y);
}

void QgisApp::checkQgisVersion()
{
    QApplication::setOverrideCursor(Qt::WaitCursor);
    /* QUrlOperator op = new QUrlOperator( "http://mrcc.com/qgis/version.txt" );
       connect(op, SIGNAL(data()), SLOT(urlData()));
       connect(op, SIGNAL(finished(QNetworkOperation)), SLOT(urlFinished(QNetworkOperation)));

       op.get(); */
    mSocket = new QSocket(this);
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
    case QSocket::ErrConnectionRefused:
        detail = tr("Connection refused - server may be down");
        break;
    case QSocket::ErrHostNotFound:
        detail = tr("QGIS server was not found");
        break;
    case QSocket::ErrSocketRead:
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

    // add the themes to the combo box on the option dialog
    QDir themeDir(mAppDir + "/share/qgis/themes");
    themeDir.setFilter(QDir::Dirs);
    QStringList dirs = themeDir.entryList("*");
    for(int i=0; i < dirs.count(); i++)
    {
        if(dirs[i] != "." && dirs[i] != "..")
        {
            optionsDialog->addTheme(dirs[i]);
        }
    }
    optionsDialog->setCurrentTheme();
    if(optionsDialog->exec())
    {
        // set the theme if it changed
        setTheme(optionsDialog->theme());
        setupToolbarPopups(optionsDialog->theme());

        // set the visible flag for new layers
        mAddedLayersHidden = optionsDialog->newVisible();
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
                                           reinterpret_cast<const UInt8*>(url.ascii()), url.length(),
                                           kCFStringEncodingMacRoman, NULL);
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
        QString myHeading = "QGIS Browser Selection";
        QString myMessage = "Enter the name of a web browser to use (eg. konqueror).\n";
        myMessage += "Enter the full path if the browser is not in your PATH.\n";
        myMessage += "You can change this option later by selecting Preferences from the Settings menu.";
        QString text = QInputDialog::getText(tr(myHeading),
                                             tr(myMessage),
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
        helpProcess->addArgument(browser);
        helpProcess->addArgument(url);
        helpProcess->start();
    }
    /*  mHelpViewer = new QgsHelpViewer(this,"helpviewer",false);
       mHelpViewer->showContent(mAppDir +"/share/doc","index.html");
       mHelpViewer->show(); */
#endif
}

/** Get a pointer to the currently selected map layer */
QgsMapLayer *QgisApp::activeLayer()
{
    QListViewItem *lvi = mMapLegend->currentItem();
    QgsMapLayer *layer = 0;
    if (lvi)
    {
        layer = ((QgsLegendItem *) lvi)->layer();
    }
    return layer;
}

QString QgisApp::activeLayerSource()
{
    QString source;
    QListViewItem *lvi = mMapLegend->currentItem();
    QgsMapLayer *layer = 0;
    if (lvi)
    {
        layer = ((QgsLegendItem *) lvi)->layer();
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
    // check to see if the appropriate provider is available
    QString providerName;

    QString pProvider = mProviderRegistry->library(providerKey);

    if ( ! pProvider.isNull() )
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
        vectorLayerPath << " with baseName of " << baseName <<
        " and providerKey of " << providerKey << std::endl;
#endif

        layer = new QgsVectorLayer(vectorLayerPath, baseName, providerKey);

        if( layer && layer->isValid() )
        {
            // Register this layer with the layers registry
            QgsMapLayerRegistry::instance()->addMapLayer(layer);
            // init the context menu so it can connect to slots in main app
            // now taken care of in legend layer->initContextMenu(this);

            // give it a random color
            QgsSingleSymRenderer *renderer = new QgsSingleSymRenderer();  //add single symbol renderer as default
            layer->setRenderer(renderer);
            renderer->initializeSymbology(layer);
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
            QMessageBox::critical(this,"Layer is not valid",
                                  "The layer is not a valid layer and can not be added to the map");
        }
        qApp->processEvents();
        mMapCanvas->freeze(false);
        mMapCanvas->render();
        QApplication::restoreOverrideCursor();
    }

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
        QMessageBox::critical(this,"Layer is not valid",
                              "The layer is not a valid layer and can not be added to the map");
    }
    qApp->processEvents();
    mMapCanvas->freeze(false);
    mMapCanvas->render();
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
        answer = QMessageBox::information(this, "Save?",
                                          "Do you want to save the current project?",
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
        menuId = menubar->idAt(idx++);
        std::cout << "Menu id " << menuId << " is " << menubar->text(menuId) << std::endl;
        mMenuMapByName[menubar->text(menuId)] = menuId;
        mMenuMapById[menuId] = menubar->text(menuId);
    }
    while(menuId != -1);
}
int QgisApp::addPluginMenu(QString menuText, QPopupMenu *menu)
{
  return mPluginMenu->insertItem(menuText, menu);
}

QPopupMenu* QgisApp::getPluginMenu(QString menuName)
{
  for (int i = 0; i < mPluginMenu->count(); ++i)
    if (mPluginMenu->text(mPluginMenu->idAt(i)) == menuName)
    {
      QMenuItem* item = mPluginMenu->findItem(mPluginMenu->idAt(i));
      return item->popup();
    }

  // It doesn't exist, so create one
  QPopupMenu* menu = new QPopupMenu(mPluginMenu);
  mPluginMenu->insertItem(menuName, menu);
  return menu;
}

void QgisApp::removePluginMenuItem(QString name, int menuId)
{
  for (int i = 0; i < mPluginMenu->count(); ++i)
    if (mPluginMenu->text(mPluginMenu->idAt(i)) == name)
    {
      QMenuItem* item = mPluginMenu->findItem(mPluginMenu->idAt(i));
      QPopupMenu* popup = item->popup();
      popup->removeItem(menuId);
      if (popup->count() == 0)
	mPluginMenu->removeItem(mPluginMenu->idAt(i));
      break;
    }
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
        mProgressBar->setProgress(theProgress,theTotalSteps);
    }


}

void QgisApp::showExtents(QgsRect theExtents)
{
    // update the statusbar with the current extents.
    statusBar()->message(QString(tr("Extents: ")) + theExtents.stringRep(true));
    // set the extents of the overview to match the mapcanvas
    mOverviewCanvas->setExtent(mMapCanvas->fullExtent());
    // Update the extent rectangle in the overview map
    QgsPoint origin(0,0);
    // create the new acetate object
    QgsAcetateRectangle *acRect = new QgsAcetateRectangle(origin, mMapCanvas->extent());
    // add it to the acetate layer
    mOverviewCanvas->addAcetateObject("extent", acRect);
    // refresh the overview map
#ifdef QGISDEBUG

    std::cerr << "Adding extent to acetate layer" << std::endl;
#endif

    mOverviewCanvas->refresh();

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
    statusBar()->message(theMessage);
}

void QgisApp::projectProperties()
{
  /* Display the property sheet for the Project */
  // set wait cursor since construction of the project properties
  // dialog results in the construction of the spatial reference
  // system QMap
  QApplication::setOverrideCursor(Qt::WaitCursor);
  QgsProjectProperties *pp = new QgsProjectProperties(this);
  qApp->processEvents();
  // Be told if the mouse display precision may have changed by the user
  // changing things in the project properties dialog box
  connect(pp, SIGNAL(displayPrecisionChanged()), this, 
      SLOT(updateMouseCoordinatePrecision()));
  QApplication::restoreOverrideCursor();
  //pass any refresg signals off to canvases
  connect (pp,SIGNAL(refresh()), mMapCanvas, SLOT(refresh()));
  connect (pp,SIGNAL(refresh()), mOverviewCanvas, SLOT(refresh()));
  // Display the modal dialog box.
  pp->exec();

  // set the map units for the project if they have changed
  if (mMapCanvas->mapUnits() != pp->mapUnits())
  {
    mMapCanvas->setMapUnits(pp->mapUnits());
  }

  // If the canvas is projected, we need to recalculate the extents in the
  // new coordinate system
  if(pp->isProjected())
  {
    mMapCanvas->recalculateExtents();
  }
  // Set the window title. No way to do a comparison like for the map
  // units above, so redo it everytime.
  setTitleBarText_( *this );

  // delete the property sheet object
  delete pp;
} // QgisApp::projectProperties


QgsMapLayerRegistry * QgisApp::getLayerRegistry()
{
    return QgsMapLayerRegistry::instance();
}


void QgisApp::setTheme(QString themeName)
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
    QString iconPath = mAppDir +"/share/qgis/themes/" + themeName;
    actionFileNew->setIconSet(QIconSet(QPixmap(iconPath + "/file_new.png")));
    actionFileSave->setIconSet(QIconSet(QPixmap(iconPath + "/file_save.png")));
    actionFileSaveAs->setIconSet(QIconSet(QPixmap(iconPath + "/file_save_as.png")));
    actionFileOpen->setIconSet(QIconSet(QPixmap(iconPath + "/project_open.png")));
    actionFilePrint->setIconSet(QIconSet(QPixmap(iconPath + "/file_print.png")));
    actionSaveMapAsImage->setIconSet(QIconSet(QPixmap(iconPath + "/save_map_image.png")));
    actionExportMapServer->setIconSet(QIconSet(QPixmap(iconPath + "/export_map_server.png")));
    actionFileExit->setIconSet(QIconSet(QPixmap(iconPath + "/exit.png")));
    actionAddNonDbLayer->setIconSet(QIconSet(QPixmap(iconPath + "/add_vector_layer.png")));
    actionAddRasterLayer->setIconSet(QIconSet(QPixmap(iconPath + "/add_raster_layer.png")));
    actionAddLayer->setIconSet(QIconSet(QPixmap(iconPath + "/add_pg_layer.png")));
    actionRemoveLayer->setIconSet(QIconSet(QPixmap(iconPath + "/remove_layer.png")));
    actionNewVectorLayer->setIconSet(QIconSet(QPixmap(iconPath + "/digitise_vector_layer.png")));
    actionAddAllToOverview->setIconSet(QIconSet(QPixmap(iconPath + "/add_all_to_overview.png")));
    actionHideAllLayers->setIconSet(QIconSet(QPixmap(iconPath + "/hide_all_layers.png")));
    actionShowAllLayers->setIconSet(QIconSet(QPixmap(iconPath + "/show_all_layers.png")));
    actionRemoveAllFromOverview->setIconSet(QIconSet(QPixmap(iconPath + "/remove_all_from_overview.png")));
    actionProjectProperties->setIconSet(QIconSet(QPixmap(iconPath + "/project_properties.png")));
    actionPluginManager->setIconSet(QIconSet(QPixmap(iconPath + "/plugin_manager.png")));
    actionCheckQgisVersion->setIconSet(QIconSet(QPixmap(iconPath + "/check_version.png")));
    actionOptions->setIconSet(QIconSet(QPixmap(iconPath + "/preferences.png")));
    actionHelpContents->setIconSet(QIconSet(QPixmap(iconPath + "/help_contents.png")));
    actionQgisHomePage->setIconSet(QIconSet(QPixmap(iconPath + "/home_page.png")));
    actionQgisSourceForgePage->setIconSet(QIconSet(QPixmap(iconPath + "/sourceforge_page.png")));
    actionHelpAbout->setIconSet(QIconSet(QPixmap(iconPath + "/help_about.png")));
    drawAction->setIconSet(QIconSet(QPixmap(iconPath + "/reload.png")));
    actionCapturePoint->setIconSet(QIconSet(QPixmap(iconPath + "/digitising_point.png")));
    actionCaptureLine->setIconSet(QIconSet(QPixmap(iconPath + "/digitising_line.png")));
    actionCapturePolygon->setIconSet(QIconSet(QPixmap(iconPath + "/digitising_general.png")));
    actionZoomIn->setIconSet(QIconSet(QPixmap(iconPath + "/zoom_in.png")));
    actionZoomOut->setIconSet(QIconSet(QPixmap(iconPath + "/zoom_out.png")));
    actionZoomFullExtent->setIconSet(QIconSet(QPixmap(iconPath + "/zoom_full.png")));
    actionZoomToSelected->setIconSet(QIconSet(QPixmap(iconPath + "/zoom_selected.png")));
    actionPan->setIconSet(QIconSet(QPixmap(iconPath + "/pan.png")));
    actionZoomLast->setIconSet(QIconSet(QPixmap(iconPath + "/zoom_last.png")));
    actionZoomToLayer->setIconSet(QIconSet(QPixmap(iconPath + "/zoom_layer.png")));
    actionIdentify->setIconSet(QIconSet(QPixmap(iconPath + "/identify.png")));
    actionSelect->setIconSet(QIconSet(QPixmap(iconPath + "/select.png")));
    actionOpenTable->setIconSet(QIconSet(QPixmap(iconPath + "/attribute_table.png")));
    actionMeasure->setIconSet(QIconSet(QPixmap(iconPath + "/measure.png")));
    actionShowBookmarks->setIconSet(QIconSet(QPixmap(iconPath + "/bookmarks.png")));
    actionNewBookmark->setIconSet(QIconSet(QPixmap(iconPath + "/new_bookmark.png")));
    actionCustomProjection->setIconSet(QIconSet(QPixmap(iconPath + "/custom_projection.png")));

}
void QgisApp::setupToolbarPopups(QString themeName)
{
    // setup the toolbar popup menus that create a single toolbar icon
    // and a menu that is popped up when the mouse is held down (ala
    // the famous web browser Back button)
    QString iconPath = mAppDir +"/share/qgis/themes/" + themeName;

    // Setup the overview tools
    // the toolbutton for the overview group:
    tbtnOverviewTools->setIconSet(QIconSet(QPixmap(iconPath + "/add_all_to_overview.png")));
    toolPopupOverviews = new QPopupMenu();
    toolPopupOverviews->insertItem(QIconSet(QPixmap(iconPath + "/add_all_to_overview.png")),
                                   "Add all layers to the overview map",
                                   this, SLOT(addAllToOverview()));
    toolPopupOverviews->insertItem(QIconSet(QPixmap(iconPath + "/remove_all_from_overview.png")),
                                   "Remove all layers from the overview map",
                                   this, SLOT(removeAllFromOverview()));
    tbtnOverviewTools->setPopup(toolPopupOverviews);
    tbtnOverviewTools->setPopupDelay(0);
    // connect the top overview tool to the appropriate slot
    connect(tbtnOverviewTools, SIGNAL(clicked()), this, SLOT(addAllToOverview()));

    // setup the add/remove all tools
    // the toolbutton for the display group:
    tbtnDisplayTools->setIconSet(QIconSet(QPixmap(iconPath + "/show_all_layers.png")));
    toolPopupDisplay = new QPopupMenu();
    toolPopupDisplay->insertItem(QIconSet(QPixmap(iconPath + "/show_all_layers.png")),
                                 "Show all layers",
                                 this, SLOT(showAllLayers()));
    toolPopupDisplay->insertItem(QIconSet(QPixmap(iconPath + "/hide_all_layers.png")),
                                 "Hide all layers",
                                 this, SLOT(hideAllLayers()));
    tbtnDisplayTools->setPopup(toolPopupDisplay);
    tbtnDisplayTools->setPopupDelay(0);
    // connect the top overview tool to the appropriate slot
    connect(tbtnDisplayTools, SIGNAL(clicked()), this, SLOT(showAllLayers()));

    // setup the capture (digitize) tools
    // the toolbutton for the capture group:
    tbtnCaptureTools->setIconSet(QIconSet(QPixmap(iconPath + "/digitising_point.png")));
    toolPopupCapture = new QPopupMenu();
    toolPopupCapture->insertItem(QIconSet(QPixmap(iconPath + "/digitising_point.png")),
                                 "Capture points",
                                 this, SLOT(capturePoint()),0,0);
    toolPopupCapture->insertItem(QIconSet(QPixmap(iconPath + "/digitising_line.png")),
                                 "Capture lines",
                                 this, SLOT(captureLine()),0,1);
    toolPopupCapture->insertItem(QIconSet(QPixmap(iconPath + "/digitising_general.png")),
                                 "Capture polygons",
                                 this, SLOT(capturePolygon()),0,2);
    toolPopupCapture->insertItem(QIconSet(QPixmap(iconPath + "/delete_selected.png")),
                                 "Delete selection",
                                 this, SLOT(deleteSelected()),0,3);
    tbtnCaptureTools->setPopup(toolPopupCapture);
    tbtnCaptureTools->setPopupDelay(0);
}


void QgisApp::setLayerOverviewStatus(QString theLayerId, bool theVisibilityFlag)
{
    if (theVisibilityFlag)
    {
        mOverviewCanvas->addLayer(QgsMapLayerRegistry::instance()->mapLayer(theLayerId));
#ifdef QGISDEBUG

        std::cout << " Added layer " << theLayerId << " to overview map" << std::endl;
#endif

    }
    else
    {
        mOverviewCanvas->remove
        (theLayerId);
#ifdef QGISDEBUG

        std::cout << " Removed layer " << theLayerId << " from overview map" << std::endl;
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
    QListViewItemIterator it(lv);
    std::vector<QString> myOverviewLayerVector;
    /** Move to the end of the list first, making sure all layers are removed as we go */
    while (it.current())
    {
        QgsLegendItem *li = (QgsLegendItem *) it.current();
        QgsMapLayer *lyr = li->layer();
        QString myLayerId = lyr->getLayerID();
        mOverviewCanvas->remove
        (myLayerId);
#ifdef QGISDEBUG

        std::cout << " Removed layer " << myLayerId << " from overview map" << std::endl;
#endif

        myOverviewLayerVector.push_back(myLayerId);
        ++it;
    }
    std::vector<QString>::reverse_iterator myIterator=myOverviewLayerVector.rbegin();
    while (myIterator != myOverviewLayerVector.rend())
    {
        QgsMapLayer *lyr = QgsMapLayerRegistry::instance()->mapLayer(*myIterator);
        if (lyr->showInOverviewStatus())
        {
            mOverviewCanvas->addLayer(lyr);
#ifdef QGISDEBUG

            std::cout << " Added layer " << *myIterator << " to overview map" << std::endl;
#endif

        }
        myIterator++;
    }

    //mOverviewCanvas->render();
    mOverviewCanvas->zoomFullExtent();
    // set the extents of the overview to match the mapcanvas
    mOverviewCanvas->setExtent(mMapCanvas->fullExtent());
    //mOverviewCanvas->refresh();

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
    std::cout << "Capture point (clicked on map) at position " << theQgsPoint.stringRep(2) << std::endl;
#endif

    QClipboard *myClipboard = QApplication::clipboard();
    //if we are on x11 system put text into selection ready for middle button pasting
    if (myClipboard->supportsSelection())
    {
        myClipboard->setText(theQgsPoint.stringRep(2),QClipboard::Selection);
        QString myMessage = "Clipboard contents set to: ";
        statusBar()->message(myMessage + myClipboard->text(QClipboard::Selection));
    }
    else
    {
        //user has an inferior operating system....
        myClipboard->setText(theQgsPoint.stringRep(2),QClipboard::Clipboard );
        QString myMessage = "Clipboard contents set to: ";
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

    // build the file filters based on the loaded GDAL drivers
    QgsRasterLayer::buildSupportedRasterFileFilter(fileFilters);

    QStringList selectedFiles;
    QString e;//only for parameter correctness
    QString title = tr("Open a GDAL Supported Raster Data Source");
    openFilesRememberingFilter_("lastRasterFileFilter", fileFilters, selectedFiles,e,
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
        mMapCanvas->render();
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
        new QgsRasterLayer(rasterFile.filePath(), rasterFile.baseName());

    if (!addRasterLayer(layer))
    {
        mMapCanvas->freeze(false);
        QApplication::restoreOverrideCursor();
        if(guiWarning)
        {
          // don't show the gui warning (probably because we are loading from command line)
          QString msg(rasterFile.baseName()
              + " is not a valid or recognized raster data source");
          QMessageBox::critical(this, "Invalid Data Source", msg);
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
            QString myBaseNameQString = myFileInfo.baseName();
            //only allow one copy of a ai grid file to be loaded at a
            //time to prevent the user selecting all adfs in 1 dir which
            //actually represent 1 coverage,

            // create the layer
            QgsRasterLayer *layer = new QgsRasterLayer(*myIterator, myBaseNameQString);

            // set initial visibility based on user preference
            layer->setVisible(mAddedLayersHidden);

            addRasterLayer(layer);

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
                QString msg(*myIterator + " is not a supported raster data source");
                QMessageBox::critical(this, "Unsupported Data Source", msg);
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

void QgisApp::killSplashScreen()
{
    // Set the hide flag to true in case someone
    // tries to set the status after we have killed the splash
    // screen
    myHideSplashFlag = true;
    gSplashScreen->finish(this);
    delete gSplashScreen;
}

void QgisApp::keyPressEvent ( QKeyEvent * e )
{
    std::cout << e->ascii() << " (keypress recevied)" << std::endl;
    emit keyPressed (e);
}
// Debug hook - used to output diagnostic messages when evoked (usually from the menu)
void QgisApp::debugHook()
{
  std::cout << "Hello from debug hook" << std::endl; 
  // show the map canvas extent
  std::cout << mMapCanvas->extent() << std::endl; 
}
void QgisApp::actionCustomProjection_activated()
{
  QgsCustomProjectionDialog * myDialog = new QgsCustomProjectionDialog(this);
  myDialog->show();
}
void QgisApp::actionShowBookmarks_activated()
{
  QgsBookmarks *bookmarks = new QgsBookmarks(this);
  bookmarks->show();
}

void QgisApp::actionNewBookmark_activated()
{
  // Get the name for the bookmark. Everything else we fetch from
  // the mapcanvas
  
  bool ok;
  QString bookmarkName = QInputDialog::getText("New Bookmark", 
      "Enter a name for the new bookmark:", QLineEdit::Normal,
      QString::null, &ok, this);
  if( ok && !bookmarkName.isEmpty())
  {
    // create the bookmark
     QgsBookmarkItem *bmi = new QgsBookmarkItem(bookmarkName, 
         QgsProject::instance()->title(), mMapCanvas->extent(), -1,
         QDir::homeDirPath () + "/.qgis/qgis.db");
     bmi->store();
     delete bmi;
  }
}
