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

#include <dlfcn.h>

#include <qaction.h>
#include <qapplication.h>
#include <qbitmap.h>
#include <qcanvas.h>
#include <qcolor.h>
#include <qcursor.h>
#include <qdir.h>
#include <qerrormessage.h>
#include <qfiledialog.h>
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

#include <iostream>
#include <iomanip>
#include <memory>



#ifndef GDAL_PRIV_H_INCLUDED
#include <gdal_priv.h>
#endif


#include "qgsrect.h"
#include "qgsmapcanvas.h"
#include "qgsmaplayer.h"
#include "qgslegenditem.h"
#include "qgslegend.h"
#include "qgsprojectio.h"
#include "qgsmapserverexport.h"


#ifdef HAVE_POSTGRESQL
#include "qgsdbsourceselect.h"
#endif
#include "qgsmessageviewer.uic.h"
#include "qgshelpviewer.h"
#include "qgsrasterlayer.h"
#include "qgsrasterlayerproperties.h"
#include "qgsvectorlayer.h"
#include "qgsabout.uic.h"
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
#include "xpm/qgis.xpm"
#include <ogrsf_frmts.h>

/* typedefs for plugins */
typedef QgsMapLayerInterface *create_it();
typedef QgisPlugin *create_ui(QgisApp * qgis, QgisIface * qI);
typedef QString name_t();
typedef QString description_t();
typedef int type_t();

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

// constructor starts here   

QgisApp::QgisApp(QWidget * parent, const char *name, WFlags fl):QgisAppBase(parent, name, fl)
{
  //
  // Splash screen global is declared in qgisapp.h header
  //
  QSettings settings;
  bool myHideSplashFlag = false;
  if (settings.readEntry("/qgis/hideSplash")=="true") { myHideSplashFlag=true; }
  if (!myHideSplashFlag)
  {
    gSplashScreen = new SplashScreen(); //this is supposed to be instantiated in main.cpp but we get segfaults...
    gSplashScreen->setStatus(tr("Loading QGIS..."));
  }

  // register all GDAL and OGR plug-ins
  GDALAllRegister();           
  OGRRegisterAll();

  QPixmap icon;
  icon = QPixmap(qgis_xpm);
  setIcon(icon);
  // store startup location
  QDir *d = new QDir();
  startupPath = d->absPath();
  delete d;
  QBitmap zoomincur;
  //  zoomincur = QBitmap(cursorzoomin);
  QBitmap zoomincurmask;
  //  zoomincurmask = QBitmap(cursorzoomin_mask);
  if (!myHideSplashFlag)
  {

    gSplashScreen->setStatus(tr("Setting up QGIS gui..."));
  }
  QGridLayout *canvasLegendLayout = new QGridLayout(frameMain, 1, 1, 4, 6, "canvasLegendLayout");
  QSplitter *canvasLegendSplit = new QSplitter(frameMain);
  QGridLayout *legendOverviewLayout = new QGridLayout(canvasLegendSplit, 1, 2, 4, 6, "canvasLegendLayout");
#ifdef QGISDEBUG
  QSplitter *legendOverviewSplit = new QSplitter(Qt::Vertical,canvasLegendSplit);
  mapLegend = new QgsLegend(legendOverviewSplit); //frameMain);
#else
  mapLegend = new QgsLegend(canvasLegendSplit); //frameMain);
#endif
  mapLegend->addColumn(tr("Layers"));
  mapLegend->setSorting(-1);

#ifdef QGISDEBUG
  mOverviewLabel = new QLabel(legendOverviewSplit);
  mOverviewLabel->setText("Overview");
#endif
  // mL = new QScrollView(canvasLegendSplit);
  //add a canvas
  mapCanvas = new QgsMapCanvas(canvasLegendSplit);
  // resize it to fit in the frame
  //    QRect r = frmCanvas->rect();
  //    canvas->resize(r.width(), r.height());
  mapCanvas->setBackgroundColor(Qt::white); //QColor (220, 235, 255));
  mapCanvas->setMinimumWidth(400);
  canvasLegendLayout->addWidget(canvasLegendSplit, 0, 0);
  mapLegend->setBackgroundColor(QColor(192, 192, 192));
  mapLegend->setMapCanvas(mapCanvas);
  mapLegend->setResizeMode(QListView::AllColumns);
  QString caption = tr("Quantum GIS - ");
  caption += QString("%1 ('%2')").arg(QGis::qgisVersion).arg(QGis::qgisReleaseName);

  setCaption(caption);

  connect(mapCanvas, SIGNAL(xyCoordinates(QgsPoint &)), this, SLOT(showMouseCoordinate(QgsPoint &)));
  connect(mapCanvas, SIGNAL(extentsChanged(QString )),this,SLOT(showExtents(QString )));
  connect(mapCanvas, SIGNAL(scaleChanged(QString)), this, SLOT(showScale(QString)));
  connect(mapCanvas, SIGNAL(addedLayer(QgsMapLayer *)), mapLegend, SLOT(addLayer(QgsMapLayer *)));
  connect(mapCanvas, SIGNAL(removedLayer(QString)), mapLegend, SLOT(removeLayer(QString)));

  connect(mapLegend, SIGNAL(doubleClicked(QListViewItem *)), this, SLOT(layerProperties(QListViewItem *)));
  connect(mapLegend, SIGNAL(rightButtonPressed(QListViewItem *, const QPoint &, int)),
          this, SLOT(rightClickLegendMenu(QListViewItem *, const QPoint &, int)));
  connect(mapLegend, SIGNAL(zOrderChanged(QgsLegend *)), mapCanvas, SLOT(setZOrderFromLegend(QgsLegend *)));
  connect(mapLegend, SIGNAL(currentChanged(QListViewItem *)), this, SLOT(currentLayerChanged(QListViewItem *)));

  // add the whats this toolbar button
  //TODO Fix this ToolBar pointer to be named consistently
  QWhatsThis::whatsThisButton(helpToolbar);

  // add the empty plugin menu
  pluginMenu = new QPopupMenu(this);  
  menuBar()->insertItem("&Plugins", pluginMenu, -1, menuBar()->count() - 1);

  // create the layer popup menu
  mapCursor = 0;

  // create the interfce
  qgisInterface = new QgisIface(this);
  ///qgisInterface->setParent(this);

  // set the legend control for the map canvas
  mapCanvas->setLegend(mapLegend);

  // disable functions based on build type
#ifndef HAVE_POSTGRESQL
  actionAddLayer->removeFrom(popupMenuLayers);
  actionAddLayer->removeFrom(DataToolbar);
#endif
  // connect the "cleanup" slot
  connect(qApp, SIGNAL(aboutToQuit()), this, SLOT(saveWindowState()));
  restoreWindowState();

  // set the dirty flag to false -- no changes yet
  projectIsDirty = false;

  //
  // Add a panel to the status bar for the scale, coords and progress
  //
  mProgressBar = new QProgressBar(100,NULL);
  mProgressBar->setMaximumWidth(100);
  statusBar()->addWidget(mProgressBar, 1,true);   
  mScaleLabel = new QLabel(QString("Scale"),NULL);
  mScaleLabel->setMinimumWidth(100);
  statusBar()->addWidget(mScaleLabel, 0,true);
  mCoordsLabel = new QLabel(QString("Coordinates:"), NULL);
  mCoordsLabel->setMinimumWidth(200);
  statusBar()->addWidget(mCoordsLabel, 0, true);


  if (! myHideSplashFlag)
  {

    gSplashScreen->setStatus(tr("Loading plugins..."));
  }

  // store the application dir
  appDir = PREFIX;
  // get the users theme preference from the settings
  QString themeName = settings.readEntry("/qgis/theme","default");
  
  // set the theme 
  setTheme(themeName);
  // Get pointer to the provider registry singleton
  QString plib = PLUGINPATH;
  providerRegistry = QgsProviderRegistry::instance(plib);

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


  // set the focus to the map canvase
  mapCanvas->setFocus();

  if (!myHideSplashFlag)
  {
    gSplashScreen->finish(this);
    delete gSplashScreen;
  }
 
} // QgisApp ctor



QgisApp::~QgisApp()
{
}


void QgisApp::about()
{
  QgsAbout *abt = new QgsAbout();
  QString versionString = tr("Version ");
  versionString += QGis::qgisVersion;
#ifdef HAVE_POSTGRESQL
  versionString += tr(" with PostgreSQL support");
#else
  versionString += tr(" (no PostgreSQL support)");
#endif
  abt->setVersion(versionString);
  QString urls = tr("Web Page: http://qgis.org") +
    "\n" + tr("Sourceforge Project Page: http://sourceforge.net/projects/qgis");
  abt->setURLs(urls);
  QString watsNew = "<html><body>" + tr("Version") + " ";
  watsNew += QGis::qgisVersion;
  watsNew += "<h3>New features</h3>";
  watsNew += "<ul>"
          "<li>Raster pyramids</li>"
          "<li>Scale display</li>"
          "<li>Map units can be specified (meters, feet, decimal degrees)</li>"
          "<li>Desktop icons (installed in share/qgis/images/icons)</li>"
          "<li>Mac OS X vector support now works</li>"
        "</ul>"
        "<h3>Core Plugins</h3>"
        "<ul>"
        "<li>New North Arrow plugin</li>"
        "<li>Improved GPS tools plugin</li>"
        "<li>New Copyright plugin</li>"
        "<li>Improved Grass plugin with raster and vector loaders</li>"
      "</ul>"
      "<h3>New Contributed Plugins</h3>"
      "These plugins are available from community.qgis.org or CVS (module plugins)"
      "<ul>"
        "<li>HTTP server</li>"
      "</ul>"
"</body></html>";


  abt->setWhatsNew(watsNew);

  // add the available plugins to the list
  QString providerInfo = "<b>" + tr("Available Data Provider Plugins") + "</b><br>";
  abt->setPluginInfo(providerInfo + providerRegistry->pluginList(true));
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
  QDir myPluginDir(thePluginDirString, "*.so*", QDir::Name | QDir::IgnoreCase, QDir::Files | QDir::NoSymLinks);

  if (myPluginDir.count() == 0)
  {
    //erk! do nothing
    return;
  } else
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
          if (mySettings.readEntry("/qgis/Plugins/" + myEntryName)=="true")
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
  // first get the GDAL driver manager

  OGRSFDriverRegistrar *driverRegistrar = OGRSFDriverRegistrar::GetRegistrar();

  if (!driverRegistrar)
    {
      qWarning("unable to get OGRDriverManager");
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

#ifdef QGISDEBUG
      qDebug("got driver string %s", driver->GetName());
#endif

      if (driverName.startsWith("ESRI"))
        {
          fileFilters += createFileFilter_("ESRI Shapefiles", "*.shp");
      } else if (driverName.startsWith("UK"))
        {
          // XXX needs file filter extension
      } else if (driverName.startsWith("SDTS"))
        {
// XXX not yet supported; post 0.1 release task
//          fileFilters += createFileFilter_( "Spatial Data Transfer Standard", 
//                                            "*catd.ddf" );
      } else if (driverName.startsWith("TIGER"))
        {
          // XXX needs file filter extension
      } else if (driverName.startsWith("S57"))
        {
          // XXX needs file filter extension
      } else if (driverName.startsWith("MapInfo"))
        {
          // XXX needs file filter extension
      } else if (driverName.startsWith("DGN"))
        {
          // XXX needs file filter extension
      } else if (driverName.startsWith("VRT"))
        {
          // XXX needs file filter extension
      } else if (driverName.startsWith("AVCBin"))
        {
          // XXX needs file filter extension
      } else if (driverName.startsWith("REC"))
        {
          // XXX needs file filter extension
      } else if (driverName.startsWith("Memory"))
        {
          // XXX needs file filter extension
      } else if (driverName.startsWith("GML"))
        {
// XXX not yet supported; post 0.1 release task
//          fileFilters += createFileFilter_( "Geography Markup Language", 
//                                            "*.gml" );
      } else
        {
          // NOP, we don't know anything about the current driver
          // with regards to a proper file filter string
        }

    }                           // each loaded GDAL driver

  // can't forget the default case
  fileFilters += "All files (*.*)";

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
                        
   @note
   
   Stores persistent settings under /qgis/UI/.  The sub-keys will be
   filterName and filterName + "Dir".
   
   Opens dialog on last directory associated with the filter name, or
   the current working directory if this is the first time invoked
   with the current filter name.
   
*/
static void openFilesRememberingFilter_(QString const &filterName, QString const &filters, QStringList & selectedFiles)
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


  std::auto_ptr < QFileDialog > openFileDialog(new QFileDialog(lastUsedDir, filters, 0, QFileDialog::tr("open files dialog")));

  // allow for selection of more than one file
  openFileDialog->setMode(QFileDialog::ExistingFiles);

  if (haveLastUsedFilter)       // set the filter to the last one used
    {
      openFileDialog->setSelectedFilter(lastUsedFilter);
    }

  if (openFileDialog->exec() == QDialog::Accepted)
    {
      selectedFiles = openFileDialog->selectedFiles();
    }

  settings.writeEntry("/qgis/UI/" + filterName, openFileDialog->selectedFilter());


  settings.writeEntry("/qgis/UI/" + filterName + "Dir", openFileDialog->dirPath());

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

  QString pOgr = providerRegistry->library("ogr");

  if (pOgr.isEmpty())
    {
#ifdef QGISDEBUG
      qDebug("unable to get OGR registry");
#endif
      return;
  } else
    {
      mapCanvas->freeze();

      QStringList selectedFiles;

      openFilesRememberingFilter_("lastVectorFileFilter", fileFilters, selectedFiles);
      if (selectedFiles.isEmpty())
        {
          // no files were selected, so just bail
          mapCanvas->freeze(false);

          return;
        }

      addLayer(selectedFiles);
    }
}                               // QgisApp::addLayer()




bool QgisApp::addLayer(QFileInfo const & vectorFile)
{
  // check to see if we have an ogr provider available
  QString pOgr = providerRegistry->library("ogr");

  if ( pOgr.isEmpty() )
    {
      QMessageBox::critical(this, 
                            tr("No OGR Provider"), 
                            tr("No OGR data provider was found in the QGIS lib directory"));
      return false;
    }

   // let the user know we're going to possibly be taking a while
   QApplication::setOverrideCursor(Qt::WaitCursor);

   mapCanvas->freeze();         // XXX why do we do this?

   // create the layer

   QgsVectorLayer *layer = new QgsVectorLayer(vectorFile.filePath(), 
                                            vectorFile.baseName(), 
                                            "ogr");
   Q_CHECK_PTR( layer );

   if ( ! layer )
   { 
      mapCanvas->freeze(false);
      QApplication::restoreOverrideCursor();

      // XXX insert meaningful whine to the user here
      return false; 
   }

   if (layer->isValid())
   {
      // init the context menu so it can connect to slots
      // in main app

      layer->initContextMenu(this);

      //add single symbol renderer as default
      QgsSingleSymRenderer *renderer = new QgsSingleSymRenderer();

      Q_CHECK_PTR( renderer );

      if ( ! renderer )
      {
         mapCanvas->freeze(false);
         QApplication::restoreOverrideCursor();

         // XXX should we also delete the layer?

         // XXX insert meaningful whine to the user here
         return false; 
      }

      layer->setRenderer(renderer);
      renderer->initializeSymbology(layer);
      mapCanvas->addLayer(layer);
      projectIsDirty = true;

   } else
   {
      QString msg = vectorFile.baseName() + " ";
      msg += tr("is not a valid or recognized data source");
      QMessageBox::critical(this, tr("Invalid Data Source"), msg);

      // since the layer is bad, stomp on it
      delete layer;

      mapCanvas->freeze(false);
      QApplication::restoreOverrideCursor();

      return false;
   }

   mapCanvas->freeze(false);
   // mapLegend->update(); NOW UPDATED VIA SIGNAL/SLOT
   qApp->processEvents();       // XXX why does this need to be called manually?
   mapCanvas->render();        // XXX eh, wot?

   QApplication::restoreOverrideCursor();

   statusBar()->message(mapCanvas->extent().stringRep(2));

   return true;

} // QgisApp::addLayer()





/** \brief overloaded vesion of the above method that takes a list of
 * filenames instead of prompting user with a dialog.

  XXX yah know, this could be changed to just iteratively call the above

 */
bool QgisApp::addLayer(QStringList const &theLayerQStringList)
{
  // check to see if we have an ogr provider available
  QString pOgr = providerRegistry->library("ogr");

  if ( pOgr.isEmpty() )
    {
      QMessageBox::critical(this, 
                            tr("No OGR Provider"), 
                            tr("No OGR data provider was found in the QGIS lib directory"));
      return false;
    }
  else
    {
      mapCanvas->freeze();

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

           if ( ! layer )
           { 
              mapCanvas->freeze(false);
              QApplication::restoreOverrideCursor();

              // XXX insert meaningful whine to the user here
              return false; 
           }

           if (layer->isValid())
           {
              // init the context menu so it can connect to slots
              // in main app

              layer->initContextMenu(this);

              //add single symbol renderer as default
              QgsSingleSymRenderer *renderer = new QgsSingleSymRenderer();

              Q_CHECK_PTR( renderer );

              if ( ! renderer )
              {
                 mapCanvas->freeze(false);
                 QApplication::restoreOverrideCursor();

                 // XXX insert meaningful whine to the user here
                 return false; 
              }

              layer->setRenderer(renderer);
              renderer->initializeSymbology(layer);
              mapCanvas->addLayer(layer);
              projectIsDirty = true;
           } else
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

      // mapLegend->update(); NOW UPDATED VIA SIGNAL/SLOTS
      qApp->processEvents();    // XXX why does this need to be called manually?
      mapCanvas->freeze(false);
      mapCanvas->render();
      QApplication::restoreOverrideCursor();
      statusBar()->message(mapCanvas->extent().stringRep(2));

  }

  return true;

} // QgisApp::addLayer()




/**
   The subset of GDAL formats that we currently support.
   
   @note
   
   Some day this won't be necessary as there'll be a time when
   theoretically we'll support everything that GDAL can throw at us.
   
   These are GDAL driver description strings.
*/
static const char *const supportedRasterFormats_[] = {
  "SDTS",
  "AIG",
  "AAIGrid",
  "GTiff",
  "USGSDEM",
  "HFA",
  "GRASS",
  "DTED",
  ""                            // used to indicate end of list
};



/**
   returns true if the given raster driver name is one currently
   supported, otherwise it returns false
   
   @param driverName GDAL driver description string
*/
static bool isSupportedRasterDriver_(QString const &driverName)
{
  size_t i = 0;

  while (supportedRasterFormats_[i][0]) // while not end of string list
    {
      // If we've got a case-insensitive match for a GDAL aware driver
      // description, then we've got a match.  Why case-insensitive?
      // I'm just being paranoid in that I can envision a situation
      // whereby GDAL slightly changes driver description string case,
      // in which case we'd catch it here.  Not that that would likely
      // happen, but if it does, we'll already compensate.
      // GS - At Qt 3.1.2, the case sensitive argument. So we change the
      // driverName to lower case before testing
      QString format = supportedRasterFormats_[i];
      if (driverName.lower().startsWith(format.lower()))
        {
          return true;
        }

      i++;
    }

  return false;
}                               // isSupportedRasterDriver_




/**
   Builds the list of file filter strings to later be used by
   QgisApp::addRasterLayer()
   
   We query GDAL for a list of supported raster formats; we then build
   a list of file filter strings from that list.  We return a string
   that contains this list that is suitable for use in a a
   QFileDialog::getOpenFileNames() call.
   
 */
static void buildSupportedRasterFileFilter_(QString & fileFilters)
{
  // first get the GDAL driver manager

  GDALDriverManager *driverManager = GetGDALDriverManager();

  if (!driverManager)
    {
      std::cerr << "unable to get GDALDriverManager\n";
      return;                   // XXX good place to throw exception if we 
    }                           // XXX decide to do exceptions

  // then iterate through all of the supported drivers, adding the
  // corresponding file filter

  GDALDriver *driver;           // current driver

  char **driverMetadata;        // driver metadata strings

  QString driverLongName("");   // long name for the given driver
  QString driverExtension("");  // file name extension for given driver
  QString driverDescription;    // QString wrapper of GDAL driver description

  QStringList metadataTokens;   // essentially the metadata string delimited by '='

  QString catchallFilter;       // for Any file(*.*), but also for those
  // drivers with no specific file
  // filter

  // Grind through all the drivers and their respective metadata.
  // We'll add a file filter for those drivers that have a file
  // extension defined for them; the others, welll, even though
  // theoreticaly we can open those files because there exists a
  // driver for them, the user will have to use the "All Files" to
  // open datasets with no explicitly defined file name extension.
  // Note that file name extension strings are of the form
  // "DMD_EXTENSION=.*".  We'll also store the long name of the
  // driver, which will be found in DMD_LONGNAME, which will have the
  // same form.

  for (int i = 0; i < driverManager->GetDriverCount(); ++i)
    {
      driver = driverManager->GetDriver(i);

      Q_CHECK_PTR(driver);

      if (!driver)
        {
          qWarning("unable to get driver %d", i);
          continue;
        }
      // now we need to see if the driver is for something currently
      // supported; if not, we give it a miss for the next driver

      driverDescription = driver->GetDescription();

      if (!isSupportedRasterDriver_(driverDescription))
        {
          // not supported, therefore skip
#ifdef QGISDEBUG
          qWarning("skipping unsupported driver %s", driver->GetDescription());
#endif
          continue;
        }
      // std::cerr << "got driver string " << driver->GetDescription() << "\n";

      driverMetadata = driver->GetMetadata();

      // presumably we know we've run out of metadta if either the
      // address is 0, or the first character is null
      while (driverMetadata && '\0' != driverMetadata[0])
        {
          metadataTokens = QStringList::split("=", *driverMetadata);
          // std::cerr << "\t" << *driverMetadata << "\n";

          // XXX add check for malformed metadataTokens

          // Note that it's oddly possible for there to be a
          // DMD_EXTENSION with no corresponding defined extension
          // string; so we check that there're more than two tokens.

          if (metadataTokens.count() > 1)
            {
              if ("DMD_EXTENSION" == metadataTokens[0])
                {
                  driverExtension = metadataTokens[1];

              } else if ("DMD_LONGNAME" == metadataTokens[0])
                {
                  driverLongName = metadataTokens[1];

                  // remove any superfluous (.*) strings at the end as
                  // they'll confuse QFileDialog::getOpenFileNames()

                  driverLongName.remove(QRegExp("\\(.*\\)$"));
                }
            }
          // if we have both the file name extension and the long name,
          // then we've all the information we need for the current
          // driver; therefore emit a file filter string and move to
          // the next driver
          if (!(driverExtension.isEmpty() || driverLongName.isEmpty()))
            {
              // XXX add check for SDTS; in that case we want (*CATD.DDF)
              fileFilters += createFileFilter_(driverLongName, "*." + driverExtension);

              break;            // ... to next driver, if any.
            }

          ++driverMetadata;

        }                       // each metadata item

      if (driverExtension.isEmpty() && !driverLongName.isEmpty())
        {
          // Then what we have here is a driver with no corresponding
          // file extension; e.g., GRASS.  In which case we append the
          // string to the "catch-all" which will match all file types.
          // (I.e., "*.*") We use the driver description intead of the
          // long time to prevent the catch-all line from getting too
          // large.

          // ... OTOH, there are some drivers with missing
          // DMD_EXTENSION; so let's check for them here and handle
          // them appropriately

          // USGS DEMs use "*.dem"
          if (driverDescription.startsWith("USGSDEM"))
            {
              fileFilters += createFileFilter_(driverLongName, "*.dem");
            } else if (driverDescription.startsWith("DTED"))
            {
              // DTED use "*.dt0"
              fileFilters += createFileFilter_(driverLongName, "*.dt0");
            } else
            {
              catchallFilter += QString(driver->GetDescription()) + " ";
            }
        }

      driverExtension = driverLongName = "";  // reset for next driver

    }                           // each loaded GDAL driver

  // can't forget the default case
  fileFilters += catchallFilter + "All other files (*)";

}                               // buildSupportedRasterFileFilter_()




/** @todo XXX I'd *really* like to return, ya know, _false_.
 */
void QgisApp::addRasterLayer()
{
  mapCanvas->freeze();
  QString fileFilters;

  // build the file filters based on the loaded GDAL drivers
  buildSupportedRasterFileFilter_(fileFilters);

  QStringList selectedFiles;

  openFilesRememberingFilter_("lastRasterFileFilter", fileFilters, selectedFiles);

  if (selectedFiles.isEmpty())
    {
      // no files were selected, so just bail		
       return;
    }
 
   addRasterLayer(selectedFiles);

}                               // QgisApp::addRasterLayer()





bool QgisApp::addRasterLayer(QFileInfo const & rasterFile)
{	
   // let the user know we're going to possibly be taking a while
   QApplication::setOverrideCursor(Qt::WaitCursor);

   mapCanvas->freeze();         // XXX why do we do this?

   // XXX ya know QgsRasterLayer can snip out the basename on its own;
   // XXX why do we have to pass it in for it?
   QgsRasterLayer *layer = 
      new QgsRasterLayer(rasterFile.filePath(), rasterFile.baseName());

   Q_CHECK_PTR( layer );

   if ( ! layer )
   {
      mapCanvas->freeze(false);
      QApplication::restoreOverrideCursor();

      // XXX insert meaningful whine to the user here; although be
      // XXX mindful that a null layer may mean exhausted memory resources
      return false; 
   }

   if (layer->isValid())
   {
      // XXX doesn't the mapCanvas->addLayer() do this?
      QObject::connect(layer, 
                       SIGNAL(repaintRequested()), 
                       mapCanvas, 
                       SLOT(refresh()));

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
      // add it to the mapcanvas collection
      mapCanvas->addLayer(layer);
      projectIsDirty = true;
      // use this layer in the map overview - well need to move this logic elsewher later
    
      //get the thumbnail for the layer

#ifdef QGISDEBUG
      QPixmap myQPixmap = QPixmap(mOverviewLabel->width(),mOverviewLabel->height());
      layer->drawThumbnail(&myQPixmap);
      mOverviewLabel->setText("");
      mOverviewLabel->setPixmap(myQPixmap);
#endif          
      // init the context menu so it can connect to slots in main app
      layer->initContextMenu(this);
   } else
   {
      QString msg(rasterFile.baseName() + " is not a valid or recognized raster data source");
      QMessageBox::critical(this, "Invalid Data Source", msg);

      delete layer;

      mapCanvas->freeze(false);
      QApplication::restoreOverrideCursor();

      return false;
   }


   // mapLegend->update(); NOW UPDATED VIA SIGNAL/SLOTS
  qApp->processEvents();
  mapCanvas->freeze(false);
  mapCanvas->render();
  QApplication::restoreOverrideCursor();
  statusBar()->message(mapCanvas->extent().stringRep(2));

   return true;
   
} // QgisApp::addRasterLayer



/**
   @todo XXX ya know, this could be changed to iteratively call the above
*/
bool QgisApp::addRasterLayer(QStringList const &theFileNameQStringList)
{
  if (theFileNameQStringList.empty())
    {
      // no files selected so bail out, but
      // allow mapCanvas to handle events
      // first
      mapCanvas->freeze(false);
      return false;
  } else
    {
      mapCanvas->freeze();
    }

  QApplication::setOverrideCursor(Qt::WaitCursor);
  // this is messy since some files in the list may be rasters and others may
  // be ogr layers. We'll set returnValue to false if one or more layers fail
  // to load.
  bool returnValue = true;
  for (QStringList::ConstIterator myIterator = theFileNameQStringList.begin(); 
       myIterator != theFileNameQStringList.end();
       ++myIterator)
    {
      if (isValidRasterFileName(*myIterator))
        {
          QFileInfo myFileInfo(*myIterator);
          // get the directory the .adf file was in
          QString myDirNameQString = myFileInfo.dirPath();
          QString myBaseNameQString = myFileInfo.baseName();
          //only allow one copy of a ai grid file to be loaded at a
          //time to prevent the user selecting all adfs in 1 dir which
          //actually represent 1 coverate,

          // create the layer
          QgsRasterLayer *layer = new QgsRasterLayer(*myIterator, myBaseNameQString);
          Q_CHECK_PTR( layer );

          if ( ! layer )
          {
             mapCanvas->freeze(false);
             QApplication::restoreOverrideCursor();

             // XXX insert meaningful whine to the user here
             return false; 
          }
          if (layer->isValid())
            {
               QObject::connect(layer, 
                                SIGNAL(repaintRequested()), 
                                mapCanvas, 
                                SLOT(refresh()));
             // connect up any request the raster may make to update the app statusbar
             QObject::connect(layer, 
                       SIGNAL(setProgress(int,int)), 
                       this, 
                       SLOT(showProgress(int,int))); 
            // connect up any request the raster may make to update the statusbar message
            QObject::connect(layer, 
                       SIGNAL(setStatus(QString)), 
                       this, 
                       SLOT(showStatusMessage(QString)));                
	       // add it to the mapcanvas collection
              mapCanvas->addLayer(layer);
              
#ifdef QGISDEBUG
             //get the thumbnail for the layer
             QPixmap myQPixmap = QPixmap(mOverviewLabel->width(),mOverviewLabel->height());
             layer->drawThumbnail(&myQPixmap);
             mOverviewLabel->setText("");
             mOverviewLabel->setPixmap(myQPixmap);
#endif      
      	      
	      projectIsDirty = true;
              // init the context menu so it can connect to slots in main app
              // XXX Yes, but what if the layer is invalid?  Should we still be doing this?
              layer->initContextMenu(this);
          } else
            {
              QString msg(*myIterator + " is not a valid or recognized raster data source");
              QMessageBox::critical(this, "Invalid Data Source", msg);

              delete layer;

              // XXX should we return false here, or just grind through
              // XXX the remaining arguments?
              returnValue = false;
            }

          //only allow one copy of a ai grid file to be loaded at a
          //time to prevent the user selecting all adfs in 1 dir which
          //actually represent 1 coverate,

          if (myBaseNameQString.lower().endsWith(".adf"))
            {

              break;
            }
        }else
        {
          QString msg(*myIterator + " is not a supported raster data source");
          QMessageBox::critical(this, "Unsupported Data Source", msg);
          returnValue = false;
        }
    }

  // mapLegend->update(); NOW UPDATED VIA SIGNAL/SLOTS

  qApp->processEvents();

  mapCanvas->freeze(false);

  mapCanvas->render();

  QApplication::restoreOverrideCursor();

  statusBar()->message(mapCanvas->extent().stringRep(2));

  return returnValue;

}                               // QgisApp::addRasterLayer()






/** This helper checks to see whether the filename appears to be a valid raster file name */
bool QgisApp::isValidRasterFileName(QString theFileNameQString)
{

  GDALDatasetH myDataset;
  GDALAllRegister();


  myDataset = GDALOpen( theFileNameQString, GA_ReadOnly );

  if( myDataset == NULL )
  {
    
    return false;
  }
  else
  {
    GDALClose(myDataset);
    return true;
  }

  /*
   * This way is no longer a good idea because it does not
   * cater for filetypes such as grass rasters that dont
   * have a predictable file extension.
   * 
   QString name = theFileNameQString.lower();
   return (name.endsWith(".adf") ||
   name.endsWith(".asc") ||
   name.endsWith(".grd") ||
   name.endsWith(".img") ||
   name.endsWith(".tif") || 
   name.endsWith(".png") || 
   name.endsWith(".jpg") || 
   name.endsWith(".dem") || 
   name.endsWith(".ddf")) ||
   name.endsWith(".dt0");

*/
}

/** Overloaded of the above function provided for convenience that takes a qstring pointer */
bool QgisApp::isValidRasterFileName(QString * theFileNameQString)
{
  //dereference and delegate
  return isValidRasterFileName(*theFileNameQString);
}

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
  QString pOgr = providerRegistry->library("postgres");
  if (pOgr.length() > 0)
    {
      // only supports postgis layers at present
      // show the postgis dialog


      QgsDbSourceSelect *dbs = new QgsDbSourceSelect(this);
      mapCanvas->freeze();
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
              if (layer->isValid())
                {
                  // init the context menu so it can connect to slots in main app
                  layer->initContextMenu(this);

                  // give it a random color
                  QgsSingleSymRenderer *renderer = new QgsSingleSymRenderer();  //add single symbol renderer as default
                  layer->setRenderer(renderer);
                  renderer->initializeSymbology(layer);
                  // add it to the mapcanvas collection
                  mapCanvas->addLayer(layer);
                  projectIsDirty = true;
              } else
                {
                  std::cerr << *it << " is an invalid layer - not loaded" << std::endl;
                  QMessageBox::critical(this, tr("Invalid Layer"), tr("%1 is an invalid layer and cannot be loaded.").arg(*it));
                  delete layer;
                }
              //qWarning("incrementing iterator");
              ++it;
            }
          //  qApp->processEvents();
          // update legend
          /*! \todo Need legend scrollview and legenditem classes */
          // mapLegend->update(); NOW UPDATED VIA SIGNAL/SLOTS

          // draw the map
          //mapCanvas->render();
          statusBar()->message(mapCanvas->extent().stringRep(2));

        }
      qApp->processEvents();

      mapCanvas->freeze(false);
      mapCanvas->render();
      QApplication::restoreOverrideCursor();
  } else
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
      mapCanvas->removeAll();
      setCaption(tr("Quantum GIS -- Untitled"));
      mapCanvas->clear();
      // mapLegend->update(); NOW UPDATED VIA SIGNAL/SLOT
      fullPath = "";
      projectIsDirty = false;
    }
} // fileNew


void QgisApp::fileOpen()
{
  int answer = saveDirty();

  if (answer != QMessageBox::Cancel)
    {
      mapCanvas->freeze(true);
      QgsProjectIo *pio = new QgsProjectIo(mapCanvas, QgsProjectIo::OPEN, this);

      if (pio->read())
        {
          setCaption(tr("Quantum GIS --") + " " + pio->baseName());
          fullPath = pio->fullPathName();
        }
      delete pio;

      // mapLegend->update(); UPDATED VIA SIGNAL/SLOTS
      mapCanvas->freeze(false);
      projectIsDirty = false;
    }
}


void QgisApp::fileSave()
{
  QgsProjectIo *pio = new QgsProjectIo(mapCanvas, QgsProjectIo::SAVE);
  pio->setFileName(fullPath);
  if (pio->write())
    {
      setCaption(tr("Quantum GIS --") + " " + pio->baseName());
      statusBar()->message(tr("Saved map to:") + " " + pio->fullPathName());
      fullPath = pio->fullPathName();
    }
  delete pio;
  projectIsDirty = false;
}

void QgisApp::fileSaveAs()
{
  QgsProjectIo *pio = new QgsProjectIo(mapCanvas, QgsProjectIo::SAVEAS);
  if (pio->write())
    {
      setCaption(tr("Quantum GIS --") + " " + pio->baseName());
      statusBar()->message(tr("Saved map to:") + " " + pio->fullPathName());
      fullPath = pio->fullPathName();
    }
  delete pio;
  projectIsDirty = false;
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
  for (myCounterInt;myCounterInt < QImageIO::outputFormats().count(); myCounterInt++ ) 
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
    mapCanvas->saveAsImage(myOutputFileNameQString,NULL,myFilterMap[myFilterString]);
    statusBar()->message(tr("Saved map image to") + " " + myOutputFileNameQString);
  }

}
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
    mapCanvas->resize(theQPixmap->width(), theQPixmap->height());
    //save the mapview to the selected file
    mapCanvas->saveAsImage(theImageFileNameQString,theQPixmap);
  }
}
bool QgisApp::addProject(QString projectFile)
{
  // adds a saved project to qgis, usually called on startup by
  // specifying a project file on the command line
  bool returnValue = false;
  QgsProjectIo *pio = new QgsProjectIo(mapCanvas, QgsProjectIo::OPEN, this);
#ifdef QGISDEBUG
  std::cout << "Loading Project - about to call ProjectIO->read()" << std::endl;
#endif
  if (pio->read(projectFile))
  {
    mapCanvas->freeze(true);
    setCaption(tr("Quantum GIS --") + " " + pio->baseName());
    fullPath = pio->fullPathName();
    // mapLegend->update(); UPDATED VIA SIGNAL/SLOTS
    mapCanvas->freeze(false);
    projectIsDirty = false;
    returnValue = true;
  }
  delete pio;
  
 return returnValue;
}
void QgisApp::exportMapServer()
{
  // check to see if there are any layers to export
  if (mapCanvas->layerCount() > 0)
    {
      QgsMapserverExport *mse = new QgsMapserverExport(mapCanvas, this);
      if (mse->exec())
        {
          mse->write();
        }
      delete mse;
  } else
    {
      QMessageBox::warning(this, "No Map Layers",
                           "No layers to export. You must add at least one layer to the map in order to export the view.");
    }
}
void QgisApp::zoomIn()
{
  /*  QWMatrix m = mapCanvas->worldMatrix();
     m.scale( 2.0, 2.0 );
     mapCanvas->setWorldMatrix( m );
   */

  mapTool = QGis::ZoomIn;
  mapCanvas->setMapTool(mapTool);
  // set the cursor


  QPixmap myZoomInQPixmap = QPixmap((const char **) zoom_in);
  delete mapCursor;
  mapCursor = new QCursor(myZoomInQPixmap, 7, 7);
  mapCanvas->setCursor(*mapCursor);
  // scale the extent
  /* QgsRect ext = mapCanvas->extent();
     ext.scale(0.5);
     mapCanvas->setExtent(ext);
     statusBar()->message(ext.stringRep(2));
     mapCanvas->clear();
     mapCanvas->render(); */
  
  

}

void QgisApp::zoomOut()
{
  mapTool = QGis::ZoomOut;
  mapCanvas->setMapTool(mapTool);

  QPixmap myZoomOutQPixmap = QPixmap((const char **) zoom_out);
  delete mapCursor;
  mapCursor = new QCursor(myZoomOutQPixmap, 7, 7);
  mapCanvas->setCursor(*mapCursor);

  /*    QWMatrix m = mapCanvas->worldMatrix();
     m.scale( 0.5, 0.5 );
     mapCanvas->setWorldMatrix( m );
   */
     
}

void QgisApp::zoomToSelected()
{
  mapCanvas->zoomToSelected();
    
}

void QgisApp::pan()
{
  mapTool = QGis::Pan;
  mapCanvas->setMapTool(mapTool);
  QBitmap panBmp(16, 16, pan_bits, true);
  QBitmap panBmpMask(16, 16, pan_mask_bits, true);
  delete mapCursor;
  mapCursor = new QCursor(panBmp, panBmpMask, 5, 5);
  mapCanvas->setCursor(*mapCursor);
    
}

void QgisApp::zoomFull()
{
  mapCanvas->zoomFullExtent();
    
}

void QgisApp::zoomPrevious()
{
  mapCanvas->zoomPreviousExtent();
    
}

void QgisApp::identify()
{
  mapTool = QGis::Identify;
  mapCanvas->setMapTool(mapTool);

  QPixmap myIdentifyQPixmap = QPixmap((const char **) identify_cursor);
  delete mapCursor;
  mapCursor = new QCursor(myIdentifyQPixmap, 1, 1);
  mapCanvas->setCursor(*mapCursor);
}

void QgisApp::attributeTable()
{
  QListViewItem *li = mapLegend->currentItem();
  if (li)
    {
      QgsMapLayer *layer = ((QgsLegendItem *) li)->layer();
      if (layer)
        {
          layer->table();

      } else
        {
          QMessageBox::information(this, tr("No Layer Selected"),
                                   tr("To open an attribute table, you must select a layer in the legend"));
        }
    }
}

void QgisApp::select()
{

  // set current map tool to select
  mapCanvas->setMapTool(QGis::Select);


  QPixmap mySelectQPixmap = QPixmap((const char **) select_cursor);
  delete mapCursor;
  mapCursor = new QCursor(mySelectQPixmap, 1, 1);
  mapCanvas->setCursor(*mapCursor);
}

void QgisApp::drawPoint(double x, double y)
{
  QPainter paint;
  //  QWMatrix mat (scaleFactor, 0, 0, scaleFactor, 0, 0);
  paint.begin(mapCanvas);
  // paint.setWorldMatrix(mat);
  paint.setWindow(*mapWindow);

  paint.setPen(Qt::blue);
  paint.drawPoint((int) x, (int) y);
  paint.end();
}

void QgisApp::drawLayers()
{
  std::cout << "In  QgisApp::drawLayers()" << std::endl;
  mapCanvas->setDirty(true);
  mapCanvas->render();
  
}

void QgisApp::showMouseCoordinate(QgsPoint & p)
{
  //@todo Softcode precsion later
  mCoordsLabel->setText(p.stringRep(2));
  //qWarning("X,Y is: " + p.stringRep(2));

}

void QgisApp::showScale(QString theScale)
{
  mScaleLabel->setText(theScale);
}

void QgisApp::testButton()
{
  /* QgsShapeFileLayer *sfl = new QgsShapeFileLayer("foo");
     mapCanvas->addLayer(sfl); */
  //      delete sfl;

}

void QgisApp::layerProperties()
{
  layerProperties(mapLegend->currentItem());
}

void QgisApp::layerProperties(QListViewItem * lvi)
{
  QgsMapLayer *lyr;
  if (lvi)
    {
      lyr = ((QgsLegendItem *) lvi)->layer();
  } else
    {
      // get the selected item
      QListViewItem *li = mapLegend->currentItem();
      lyr = ((QgsLegendItem *) li)->layer();
    }



  QString currentName = lyr->name();
  //test if we have a raster or vector layer and show the appropriate dialog
  if (lyr->type() == QgsMapLayer::RASTER)
    {
      QgsRasterLayerProperties *rlp = new QgsRasterLayerProperties(lyr);
      // The signals to change the raster layer properties will only be emitted
      // when the user clicks ok or apply
      //connect(rlp, SIGNAL(setTransparency(unsigned int)), SLOT(lyr(slot_setTransparency(unsigned int))));
      if (rlp->exec())
        {
          //this code will be called it the user selects ok
          mapCanvas->setDirty(true);
          mapCanvas->refresh();
          mapCanvas->render();
          // mapLegend->update(); XXX WHY CALL UPDATE HERE?
          delete rlp;
          qApp->processEvents();
        }
  } else
    {
      lyr->showLayerProperties();
    }

  //TODO Fix this area below and above
  //this is a very hacky way to force the legend entry to refresh - the call above does ne happen for some reason
  //mapCanvas->render();
  //mapLegend->update();


  /* else if ((lyr->type()==QgsMapLayer::VECTOR) || (lyr->type()==QgsMapLayer::DATABASE))
     {
     QgsLayerProperties *lp = new QgsLayerProperties(lyr);
     if (lp->exec()) {
     // update the symbol
     lyr->setSymbol(lp->getSymbol());
     mapCanvas->freeze();
     lyr->setlayerName(lp->displayName());
     if (currentName != lp->displayName())
     mapLegend->update();
     delete lp;
     qApp->processEvents();

     // apply changes
     mapCanvas->freeze(false);
     mapCanvas->setDirty(true);
     mapCanvas->render();
     }
     }
     else if (lyr->type()==QgsMapLayer::DATABASE)
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



  //  lyr->showLayerProperties();
}

//>>>>>>> 1.97.2.17

void QgisApp::removeLayer()
{
  mapCanvas->freeze();
  QListViewItem *lvi = mapLegend->currentItem();
  QgsMapLayer *lyr = ((QgsLegendItem *) lvi)->layer();
  //mapCanvas->remove(lyr->getLayerID());
  mapLegend->removeLayer(lyr->getLayerID()); // this implicitly calls mapCanvas::remove()
  mapCanvas->freeze(false);
  // draw the map
  mapCanvas->clear();
  mapCanvas->render();
  

}

void QgisApp::zoomToLayerExtent()
{

  // get the selected item
  QListViewItem *li = mapLegend->currentItem();
  QgsMapLayer *lyr = ((QgsLegendItem *) li)->layer();
  mapCanvas->setExtent(lyr->extent());
  mapCanvas->clear();
  mapCanvas->render();
  

}

void QgisApp::rightClickLegendMenu(QListViewItem * lvi, const QPoint & pt, int)
{
  if (lvi)
    {
      // get the context menu from the layer and display it 
      QgsMapLayer *lyr = ((QgsLegendItem *) lvi)->layer();
      QPopupMenu *popMenu = lyr->contextMenu();
      if (popMenu)
        {
          popMenu->exec(pt);
        }
    }
}

void QgisApp::currentLayerChanged(QListViewItem * lvi)
{
  if (lvi)
    {
      // disable/enable toolbar buttons as appropriate based on selected
      // layer type
      QgsMapLayer *lyr = ((QgsLegendItem *) lvi)->layer();
      if (lyr->type() == QgsMapLayer::RASTER)
        {
          actionIdentify->setEnabled(FALSE);
          actionSelect->setEnabled(FALSE);
          actionOpenTable->setEnabled(FALSE);
          // if one of these map tools is selected, set cursor to default
          if (mapTool == QGis::Identify || mapTool == QGis::Select || mapTool == QGis::Table)
            {
              delete mapCursor;
              mapCursor = new QCursor();
              mapCanvas->setCursor(*mapCursor);
            }
      } else
        {
          actionIdentify->setEnabled(TRUE);
          actionSelect->setEnabled(TRUE);
          actionOpenTable->setEnabled(TRUE);
          // if one of these map tools is selected, make sure appropriate cursor gets set
          switch (mapTool)
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
    }
}

QgisIface *QgisApp::getInterface()
{
  return qgisInterface;
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

void QgisApp::loadPlugin(QString name, QString description, QString fullPath)
{
  QSettings settings;
  // first check to see if its already loaded
  QgsPluginRegistry *pRegistry = QgsPluginRegistry::instance();
  QString lib = pRegistry->library(name);
  if (lib.length() > 0)
    {
      // plugin is loaded
      // QMessageBox::warning(this, "Already Loaded", description + " is already loaded");
  } else
    {
      QLibrary *myLib = new QLibrary(fullPath);
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
                      QgisPlugin *pl = cf(this, qgisInterface);
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
                      std::cerr << "Unable to find the class factory for " << fullPath << std::endl;
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

                      } else
                        {
                          // something went wrong
                          QMessageBox::warning(this, tr("Error Loading Plugin"), tr("There was an error loading %1."));
                        //add it to the qsettings file [ts]
                       settings.writeEntry("/qgis/Plugins/" + name, false);
                        }
                  } else
                    {
//#ifdef QGISDEBUG
                      std::cerr << "Unable to find the class factory for " << fullPath << std::endl;
//#endif
                    }
                }
                break;
              default:
                // type is unknown
//#ifdef QGISDEBUG
                std::cerr << "Plugin " << fullPath << " did not return a valid type and cannot be loaded" << std::endl;
//#endif
                break;
            }

          /*  }else{
             std::cout << "Unable to find the class factory for " << fullPath << std::endl;
             } */

      } else
        {
//#ifdef QGISDEBUG
          std::cerr << "Failed to load " << fullPath << "\n";
//#endif
        }
    }
}
void QgisApp::testMapLayerPlugins()
{
  // map layer plugins live in their own directory (somewhere to be determined)
  QDir mlpDir("../plugins/maplayer", "*.so.1.0.0", QDir::Name | QDir::IgnoreCase, QDir::Files);
  if (mlpDir.count() == 0)
    {
      QMessageBox::information(this, tr("No MapLayer Plugins"), tr("No MapLayer plugins in ../plugins/maplayer"));
  } else
    {
      for (unsigned i = 0; i < mlpDir.count(); i++)
        {
#ifdef QGISDEBUG
          std::cout << "Getting information for plugin: " << mlpDir[i] << std::endl;
          std::cout << "Attempting to load the plugin using dlopen\n";
#endif
          void *handle = dlopen("../plugins/maplayer/" + mlpDir[i], RTLD_LAZY);
          if (!handle)
            {
#ifdef QGISDEBUG
              std::cout << "Error in dlopen: " << dlerror() << std::endl;
#endif
          } else
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
                  } else
                    {
#ifdef QGISDEBUG
                      std::cout << "Unable to instantiate the maplayer test plugin\n";
#endif
                    }
                }
          } else
            {
#ifdef QGISDEBUG
              std::cout << "Failed to load " << mlpDir[i] << "\n";
#endif
            }
        }
    }
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
      } else
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
              std::cout << "Attempting to load " << +"../plugins/" + pluginDir[i] << std::endl;
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
                      QgisPlugin *pl = cf(this, qgisInterface);
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
              } else
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
  setGeometry(x, y, w, h);
}

void QgisApp::checkQgisVersion()
{
  QApplication::setOverrideCursor(Qt::WaitCursor);
  /* QUrlOperator op = new QUrlOperator( "http://mrcc.com/qgis/version.txt" );
     connect(op, SIGNAL(data()), SLOT(urlData()));
     connect(op, SIGNAL(finished(QNetworkOperation)), SLOT(urlFinished(QNetworkOperation)));

     op.get(); */
  socket = new QSocket(this);
  connect(socket, SIGNAL(connected()), SLOT(socketConnected()));
  connect(socket, SIGNAL(connectionClosed()), SLOT(socketConnectionClosed()));
  connect(socket, SIGNAL(readyRead()), SLOT(socketReadyRead()));
  connect(socket, SIGNAL(error(int)), SLOT(socketError(int)));
  socket->connectToHost("mrcc.com", 80);
}

void QgisApp::socketConnected()
{
  QTextStream os(socket);
  versionMessage = "";
  // send the qgis version string
  // os << qgisVersion << "\r\n";
  os << "GET /qgis/version.txt HTTP/1.0\n\n";


}

void QgisApp::socketConnectionClosed()
{
  QApplication::restoreOverrideCursor();
  // strip the header
  QString contentFlag = "#QGIS Version";
  int pos = versionMessage.find(contentFlag);
  if (pos > -1)
    {
      pos += contentFlag.length();
      /* std::cout << versionMessage << "\n ";
         std::cout << "Pos is " << pos <<"\n"; */
      versionMessage = versionMessage.mid(pos);
      QStringList parts = QStringList::split("|", versionMessage);
      // check the version from the  server against our version
      QString versionInfo;
      int currentVersion = parts[0].toInt();
      if (currentVersion > QGis::qgisVersionInt)
        {
          // show version message from server
          versionInfo = tr("There is a new version of QGIS available") + "\n";
      } else
        {
          if (QGis::qgisVersionInt > currentVersion)
            {
              versionInfo = tr("You are running a development version of QGIS") + "\n";
          } else
            {
              versionInfo = tr("You are running the current version of QGIS") + "\n";
            }
        }
      if (parts.count() > 1)
        {
          versionInfo += parts[1] + "\n\n" + tr("Would you like more information?");;
          int result = QMessageBox::information(this, tr("QGIS Version Information"), versionInfo, tr("Yes"), tr("No"));
          if (result == 0)
            {
              // show more info
              QgsMessageViewer *mv = new QgsMessageViewer(this);
              mv->setCaption(tr("QGIS - Changes in CVS Since Last Release"));
              mv->setMessage(parts[2]);
              mv->exec();
            }
      } else
        {
          QMessageBox::information(this, tr("QGIS Version Information"), versionInfo);
        }
  } else
    {
      QMessageBox::warning(this, tr("QGIS Version Information"), tr("Unable to get current version information from server"));
    }
}
void QgisApp::socketError(int e)
{
  QApplication::restoreOverrideCursor();
  // get errror type
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
  while (socket->bytesAvailable() > 0)
    {
      char *data = new char[socket->bytesAvailable() + 1];
      memset(data, '\0', socket->bytesAvailable() + 1);
      socket->readBlock(data, socket->bytesAvailable());
      versionMessage += data;
      delete[]data;
    }

}
void QgisApp::options()
{
  QgsOptions *optionsDialog = new QgsOptions(this);

  // add the themes to the combo box on the option dialog
  QDir themeDir(appDir + "/share/qgis/themes");
  themeDir.setFilter(QDir::Dirs);
  QStringList dirs = themeDir.entryList("*");
  for(int i=0; i < dirs.count(); i++)
  {
    if(dirs[i] != "." && dirs[i] != "..")
    {
      optionsDialog->addTheme(dirs[i]);
    }
  }
  if(optionsDialog->exec())
  {
    // set the theme if it changed
   setTheme(optionsDialog->theme()); 
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
      url = appDir + "/share/qgis/doc/" + url;
    }
  // find a browser
  QSettings settings;
  QString browser = settings.readEntry("/qgis/browser");
  if (browser.length() == 0)
    {
      // ask user for browser and use it
      bool ok;
      QString text = QInputDialog::getText("QGIS Browser Selection",
                                           "Enter the name of a web browser to use (eg. konqueror).\nEnter the full path if the browser is not in your PATH.\nYou can change this option later by selection Options from the Tools menu.",
                                           QLineEdit::Normal,
                                           QString::null, &ok, this);
      if (ok && !text.isEmpty())
        {
          // user entered something and pressed OK
          browser = text;
          // save the setting
          settings.writeEntry("/qgis/browser", browser);
      } else
        {
          browser = "";
        }

    }
  if (browser.length() > 0)
    {
      // find the installed location of the help files
      // open index.html using browser
      QProcess *helpProcess = new QProcess(this);
      helpProcess->addArgument(browser);
      helpProcess->addArgument(url);
      helpProcess->start();
    }
  /*  helpViewer = new QgsHelpViewer(this,"helpviewer",false);
     helpViewer->showContent(appDir +"/share/doc","index.html");
     helpViewer->show(); */
}

/** Get a pointer to the currently selected map layer */
QgsMapLayer *QgisApp::activeLayer()
{
  QListViewItem *lvi = mapLegend->currentItem();
  QgsMapLayer *lyr = 0;
  if (lvi)
    {
      lyr = ((QgsLegendItem *) lvi)->layer();
    }
  return lyr;
}

QString QgisApp::activeLayerSource()
{
  QString source;
  QListViewItem *lvi = mapLegend->currentItem();
  QgsMapLayer *lyr = 0;
  if (lvi)
    {
      lyr = ((QgsLegendItem *) lvi)->layer();
      source = lyr->source();
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

  QString pProvider = providerRegistry->library(providerKey);
  if (pProvider.length() > 0)
  {
    mapCanvas->freeze();
    QApplication::setOverrideCursor(Qt::WaitCursor);
    // create the layer
    QgsVectorLayer *lyr;
    /* Eliminate the need to instantiate the layer based on provider type.
       The caller is responsible for cobbling together the needed information to
       open the layer
       */
#ifdef QGISDEBUG
    std::cout << "Creating new vector layer using " <<
      vectorLayerPath << " with baseName of " << baseName <<
      " and providerKey of " << providerKey << std::endl;
#endif
    lyr = new QgsVectorLayer(vectorLayerPath, baseName, providerKey);
    if(lyr->isValid())
    {
      // init the context menu so it can connect to slots in main app
      lyr->initContextMenu(this);

      // give it a random color
      QgsSingleSymRenderer *renderer = new QgsSingleSymRenderer();  //add single symbol renderer as default
      lyr->setRenderer(renderer);
      renderer->initializeSymbology(lyr);
      // add it to the mapcanvas collection
      mapCanvas->addLayer(lyr);
      projectIsDirty = true;
      //qWarning("incrementing iterator");
      /*! \todo Need legend scrollview and legenditem classes */
      // mapLegend->update(); NOW DONE VIA SIGNAL/SLOTS

      // draw the map
      //mapCanvas->render();
      statusBar()->message(mapCanvas->extent().stringRep(2));

    }else
    {
      QMessageBox::critical(this,"Layer is not valid",
          "The layer is not a valid layer and can not be added to the map");
    }
    qApp->processEvents();
    mapCanvas->freeze(false);
    mapCanvas->render();
    QApplication::restoreOverrideCursor();
  }
    
}

int QgisApp::saveDirty()
{
  int answer = 0;
  mapCanvas->freeze(true);
#ifdef QGISDEBUG
  std::cout << "Layer count is " << mapCanvas->layerCount() << std::endl;
  std::cout << "Project is ";
  if (projectIsDirty)
    {
      std::cout << "dirty" << std::endl;
  } else
    {
      std::cout << "not dirty" << std::endl;
    }
  std::cout << "Map canvas is ";
  if (mapCanvas->isDirty())
    {
      std::cout << "dirty" << std::endl;
  } else
    {
      std::cout << "not dirty" << std::endl;
    }
#endif
  if ((projectIsDirty || (mapCanvas->isDirty()) && mapCanvas->layerCount() > 0))
    {
      // flag project is dirty since dirty state of canvas is reset if "dirty"
      // is based on a zoom or pan
      projectIsDirty = true;
      // prompt user to save
      answer = QMessageBox::information(this, "Save?", 
          "Do you want to save the current project?",
          QMessageBox::Yes | QMessageBox::Default,
          QMessageBox::No, QMessageBox::Cancel | QMessageBox::Escape);
      if (answer == QMessageBox::Yes)
        {
          fileSave();
        }
    }
  mapCanvas->freeze(false);
  return answer;
}
void QgisApp::whatsThis()
{
  QWhatsThis::enterWhatsThisMode();
}
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
  do{
    menuId = menubar->idAt(idx++);
    std::cout << "Menu id " << menuId << " is " << menubar->text(menuId) << std::endl; 
    mMenuMapByName[menubar->text(menuId)] = menuId;
    mMenuMapById[menuId] = menubar->text(menuId);
  }while(menuId != -1);
}
int QgisApp::addPluginMenu(QString menuText, QPopupMenu *menu)
{
  pluginMenu->insertItem(menuText, menu);
}

// slot to update the progress bar in the status bar
void QgisApp::showProgress(int theProgress, int theTotalSteps)
{
#ifdef QGISDEBUG
  std::cout << "setProgress called with " << theProgress << "/" << theTotalSteps << endl;
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

void QgisApp::showExtents(QString theExtents)
{
  statusBar()->message(QString(tr("Extents: ")) + theExtents);

}

void QgisApp::showStatusMessage(QString theMessage)
{
  statusBar()->message(theMessage);
}

void QgisApp::projectProperties()
{
  QgsProjectProperties *pp = new QgsProjectProperties(this);
  pp->setMapUnits(mapCanvas->mapUnits());
  if(pp->exec())
  {
    // set the map units for the project (ie the map canvas)
    mapCanvas->setMapUnits(pp->mapUnits());

  }
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
  QString iconPath = appDir +"/share/qgis/themes/" + themeName;
  actionFileNew->setIconSet(QIconSet(QPixmap(iconPath + "/file_new.png")));
  actionFileSave->setIconSet(QIconSet(QPixmap(iconPath + "/file_save.png")));
  actionFileSaveAs->setIconSet(QIconSet(QPixmap(iconPath + "/file_save_as.png")));
  actionFileOpen->setIconSet(QIconSet(QPixmap(iconPath + "/project_open.png")));
  actionSaveMapAsImage->setIconSet(QIconSet(QPixmap(iconPath + "/save_map_image.png")));
  actionExportMapServer->setIconSet(QIconSet(QPixmap(iconPath + "/export_map_server.png")));
  actionFileExit->setIconSet(QIconSet(QPixmap(iconPath + "/exit.png")));
  actionAddNonDbLayer->setIconSet(QIconSet(QPixmap(iconPath + "/add_vector_layer.png")));
  actionAddRasterLayer->setIconSet(QIconSet(QPixmap(iconPath + "/add_raster_layer.png")));
  actionAddLayer->setIconSet(QIconSet(QPixmap(iconPath + "/add_pg_layer.png")));
  actionProjectProperties->setIconSet(QIconSet(QPixmap(iconPath + "/project_properties.png")));
  actionPluginManager->setIconSet(QIconSet(QPixmap(iconPath + "/plugin_manager.png")));
  actionCheckQgisVersion->setIconSet(QIconSet(QPixmap(iconPath + "/check_version.png")));
  actionOptions->setIconSet(QIconSet(QPixmap(iconPath + "/preferences.png")));
  actionHelpContents->setIconSet(QIconSet(QPixmap(iconPath + "/help_contents.png")));
  actionQgisHomePage->setIconSet(QIconSet(QPixmap(iconPath + "/home_page.png")));
  actionQgisSourceForgePage->setIconSet(QIconSet(QPixmap(iconPath + "/sourceforge_page.png")));
  actionHelpAbout->setIconSet(QIconSet(QPixmap(iconPath + "/help_about.png")));
  drawAction->setIconSet(QIconSet(QPixmap(iconPath + "/reload.png")));
  actionZoomIn->setIconSet(QIconSet(QPixmap(iconPath + "/zoom_in.png")));
  actionZoomOut->setIconSet(QIconSet(QPixmap(iconPath + "/zoom_out.png")));
  actionZoomFullExtent->setIconSet(QIconSet(QPixmap(iconPath + "/zoom_full.png")));
  actionZoomToSelected->setIconSet(QIconSet(QPixmap(iconPath + "/zoom_selected.png")));
  actionPan->setIconSet(QIconSet(QPixmap(iconPath + "/pan.png")));
  actionZoomLast->setIconSet(QIconSet(QPixmap(iconPath + "/zoom_last.png")));
  actionIdentify->setIconSet(QIconSet(QPixmap(iconPath + "/identify.png")));
  actionSelect->setIconSet(QIconSet(QPixmap(iconPath + "/select.png")));
  actionOpenTable->setIconSet(QIconSet(QPixmap(iconPath + "/attribute_table.png")));
}
