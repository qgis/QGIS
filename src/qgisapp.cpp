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

#include <qapplication.h>
#include <qaction.h>
#include <qmenubar.h>
#include <qcanvas.h>
#include <qcolor.h>
#include <qdir.h>
#include <qscrollview.h>
#include <qstringlist.h>
#include <qerrormessage.h>
#include <qmessagebox.h>
#include <qstatusbar.h>
#include <qlabel.h>
#include <qfiledialog.h>
#include <qfileinfo.h>
#include <qpixmap.h>
#include <qbitmap.h>
#include <qsplitter.h>
#include <qpopupmenu.h>
#include <qprocess.h>
#include <qrect.h>
#include <qpoint.h>
#include <qpainter.h>
#include <qcursor.h>
#include <qlayout.h>
#include <qwmatrix.h>
#include <qfiledialog.h>
#include <qlibrary.h>
#include <qvbox.h>
#include <qlistview.h>
#include <qsettings.h>
#include <qtextstream.h>
#include <qsocket.h>
#include <qinputdialog.h>
#include <qregexp.h> 

#include <iostream>
#include <iomanip>


#ifndef GDAL_PRIV_H_INCLUDED
#include <gdal_priv.h>
#endif


#include "qgsrect.h"
#include "qgsmapcanvas.h"
#include "qgsmaplayer.h"
#include "qgslegenditem.h"
#include "qgslegend.h"
#include "qgslegendview.h"
#include "qgsprojectio.h"
#include "qgsmapserverexport.h"
#include <splashscreen.h>

#ifdef POSTGRESQL
#include "qgsdbsourceselect.h"
#endif
#include "qgsmessageviewer.h"
#include "qgshelpviewer.h"
#include "qgsrasterlayer.h"
#include "qgsrasterlayerproperties.h"
#include "qgsvectorlayer.h"
#include "qgslayerproperties.h"
#include "qgsabout.h"
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
#include "qgsoptionsbase.h"
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
    // Set up the splash screen
    //
    SplashScreen *mySplash = new SplashScreen();
    mySplash->setStatus(tr("Loading QGIS..."));

    GDALAllRegister();          // register all GDAL and OGR plug-ins
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

    mySplash->setStatus(tr("Setting up QGIS gui..."));
    QGridLayout *FrameLayout = new QGridLayout(frameMain, 1, 2, 4, 6, "mainFrameLayout");
    QSplitter *split = new QSplitter(frameMain);
    legendView = new QgsLegendView(split);
    legendView->addColumn(tr("Layers"));
    legendView->setSorting(-1);


    mapLegend = new QgsLegend(legendView);  //frameMain);
    // mL = new QScrollView(split);
    //add a canvas
    mapCanvas = new QgsMapCanvas(split);
    // resize it to fit in the frame
    //    QRect r = frmCanvas->rect();
    //    canvas->resize(r.width(), r.height());
    mapCanvas->setBackgroundColor(Qt::white);   //QColor (220, 235, 255));
    mapCanvas->setMinimumWidth(400);
    FrameLayout->addWidget(split, 0, 0);
    mapLegend->setBackgroundColor(QColor(192, 192, 192));
    mapLegend->setMapCanvas(mapCanvas);
    legendView->setResizeMode(QListView::AllColumns);
    QString caption = tr("Quantum GIS - ");
    caption += QGis::qgisVersion;
    setCaption(caption);
    connect(mapCanvas, SIGNAL(xyCoordinates(QgsPoint &)), this, SLOT(showMouseCoordinate(QgsPoint &)));
    connect(legendView, SIGNAL(doubleClicked(QListViewItem *)), this, SLOT(layerProperties(QListViewItem *)));
    connect(legendView, SIGNAL(rightButtonPressed(QListViewItem *, const QPoint &, int)),
            this, SLOT(rightClickLegendMenu(QListViewItem *, const QPoint &, int)));
    connect(legendView, SIGNAL(zOrderChanged(QgsLegendView *)), mapCanvas, SLOT(setZOrderFromLegend(QgsLegendView *)));
    connect(legendView, SIGNAL(currentChanged(QListViewItem *)), this, SLOT(clickLegendMenu(QListViewItem *)));

    // create the layer popup menu
    mapCursor = 0;
    // create the interfce
    qgisInterface = new QgisIface(this);
    ///qgisInterface->setParent(this);
    // set the legend control for the map canvas
    mapCanvas->setLegend(mapLegend);
    // disable functions based on build type
#ifndef POSTGRESQL
    actionAddLayer->removeFrom(PopupMenu_2);
    actionAddLayer->removeFrom(DataToolbar);
#endif
    mySplash->setStatus(tr("Loading plugins..."));
    // store the application dir
    appDir = PREFIX;
    // Get pointer to the provider registry singleton
    QString plib = PREFIX;
    plib += "/lib/qgis";
    providerRegistry = QgsProviderRegistry::instance(plib);
    // set the provider plugin path 
#ifdef DEBUG
    std::cout << "Setting plugin lib dir to " << plib << std::endl;
#endif
    // connect the "cleanup" slot
    connect(qApp, SIGNAL(aboutToQuit()), this, SLOT(saveWindowState()));
    restoreWindowState();
    // set the focus to the map canvase
    mapCanvas->setFocus();
    mySplash->finish(this);
    delete mySplash;

#ifdef DEBUG
    std::cout << "Plugins are installed in " << plib << std::endl;
#endif
    // set the dirty flag to false -- no changes yet
    projectIsDirty = false;
}

QgisApp::~QgisApp()
{
}
void QgisApp::about()
{
    QgsAbout *abt = new QgsAbout();
    QString versionString = tr("Version ");
    versionString += QGis::qgisVersion;
#ifdef POSTGRESQL
    versionString += tr(" with PostgreSQL support");
#else
    versionString += tr(" (no PostgreSQL support)");
#endif
    abt->setVersion(versionString);
    QString urls = tr("Web Page: http://qgis.sourceforge.net") +
      "\n" + tr("Sourceforge Project Page: http://sourceforge.net/projects/qgis");
    abt->setURLs(urls);
    QString watsNew = tr("Version") + " ";
    watsNew += QGis::qgisVersion;
    watsNew += "\n" "** Raster support\n" "** Improved data source handling\n" "** Symbology\n" "\n";


    abt->setWhatsNew(watsNew);

    // add the available plugins to the list
    QString providerInfo = "<b>" + tr("Available Data Provider Plugins") + "</b><br>";
    abt->setPluginInfo(providerInfo + providerRegistry->pluginList(true));
    abt->exec();

}




/**

  Convenience function for readily creating file filters.

  Given a long name for a file filter and a regular expression, return
  a file filter string suitable for use in a QFileDialog::OpenFiles()
  call.  The regular express, glob, will have both all lower and upper
  case versions added.

*/
static
QString
createFileFilter_( QString const & longName, QString const & glob )
{
   return longName + " (" + glob.lower() + " " + glob.upper() + ");;";
} // createFileFilter_



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
static
void
buildSupportedVectorFileFilter_( QString & fileFilters )
{
   // first get the GDAL driver manager

   OGRSFDriverRegistrar * driverRegistrar =  OGRSFDriverRegistrar::GetRegistrar( );

   if ( ! driverRegistrar )
   {
      qWarning( "unable to get OGRDriverManager" );
      return;                   // XXX good place to throw exception if we 
   }                            // XXX decide to do exceptions

   // then iterate through all of the supported drivers, adding the
   // corresponding file filter

   OGRSFDriver * driver;        // current driver

   QString       driverName;    // current driver name

   // Grind through all the drivers and their respective metadata.
   // We'll add a file filter for those drivers that have a file
   // extension defined for them; the others, welll, even though
   // theoreticaly we can open those files because there exists a
   // driver for them, the user will have to use the "All Files" to
   // open datasets with no explicitly defined file name extension.

   for ( int i = 0; i < driverRegistrar->GetDriverCount(); ++i )
   {
      driver = driverRegistrar->GetDriver( i );

      if ( ! driver )
      {
         qWarning( "unable to get driver %d", i );
         continue;
      }

      driverName = driver->GetName();

#ifdef QT_DEBUG
      qDebug( "got driver string %s", driver->GetName() );
#endif

      if ( driverName.startsWith( "ESRI" ) )
      {
         fileFilters += createFileFilter_( "ESRI Shapefiles", 
                                           "*.shp" );
      }
      else if ( driverName.startsWith( "UK" ) )
      {
         // XXX needs file filter extension
      }
      else if ( driverName.startsWith( "SDTS" ) )
      {
         fileFilters += createFileFilter_( "Spatial Data Transfer Standard", 
                                           "*catd.ddf" );
      }
      else if ( driverName.startsWith( "TIGER" ) )
      {
         // XXX needs file filter extension
      }
      else if ( driverName.startsWith( "S57" ) )
      {
         // XXX needs file filter extension
      }
      else if ( driverName.startsWith( "MapInfo" ) )
      {
         // XXX needs file filter extension
      }
      else if ( driverName.startsWith( "DGN" ) )
      {
         // XXX needs file filter extension
      }
      else if ( driverName.startsWith( "VRT" ) )
      {
         // XXX needs file filter extension
      }
      else if ( driverName.startsWith( "AVCBin" ) )
      {
         // XXX needs file filter extension
      }
      else if ( driverName.startsWith( "REC" ) )
      {
         // XXX needs file filter extension
      }
      else if ( driverName.startsWith( "Memory" ) )
      {
         // XXX needs file filter extension
      }
      else if ( driverName.startsWith( "GML" ) )
      {
         fileFilters += createFileFilter_( "Geography Markup Language", 
                                           "*.gml" );
      }
      else
      {
         // NOP, we don't know anything about the current driver
         // with regards to a proper file filter string
      }

   } // each loaded GDAL driver

   // can't forget the default case
   fileFilters += "All files (*.*)";

} // buildSupportedVectorFileFilter_()






/**
   This method prompts the user for a list of vector filenames with a dialog. 
*/
void
QgisApp::addLayer()
{
   QString fileFilters;

   buildSupportedVectorFileFilter_( fileFilters );

   //qDebug( "vector file filters: " + fileFilters );
   
   QString pOgr = providerRegistry->library( "ogr" );

   if ( ! pOgr.isEmpty() )
   {
      mapCanvas->freeze();

      QStringList files = 
         QFileDialog::getOpenFileNames( fileFilters, 0, this, 
                                        tr("open files dialog"),
                                        tr("Select one or more layers to add") );
      addLayer(files);
   }

} // QgisApp::addLayer()





/** \brief overloaded vesion of the above method that takes a list of
 * filenames instead of prompting user with a dialog. */
void QgisApp::addLayer(QStringList theLayerQStringList)
{
    // check to see if we have an ogr provider available
    QString pOgr = providerRegistry->library("ogr");
    if (pOgr.length() > 0) {
        mapCanvas->freeze();
        QApplication::setOverrideCursor(Qt::WaitCursor);
        QStringList::Iterator it = theLayerQStringList.begin();
        while (it != theLayerQStringList.end()) {
            if (isValidVectorFileName(*it)) {

                QFileInfo fi(*it);
                QString base = fi.baseName();


                // create the layer

                //dp    QgsShapeFileLayer *lyr = new QgsShapeFileLayer(*it, base);
                QgsVectorLayer *lyr = new QgsVectorLayer(*it, base, "ogr");

                if (lyr->isValid()) {
                    // init the context menu so it can connect to slots in main app
                    lyr->initContextMenu(this);
                    //add single symbol renderer as default
                    QgsSingleSymRenderer *renderer = new QgsSingleSymRenderer();
                    lyr->setRenderer(renderer);
                    renderer->initializeSymbology(lyr);
                    mapCanvas->addLayer(lyr);
                    projectIsDirty = true;
                } else {
                    QString msg = *it + " ";
                    msg += tr("is not a valid or recognized data source");
                    QMessageBox::critical(this, tr("Invalid Data Source"), msg);
                }
            }                   //end of isValidVectorFileName check
            ++it;
        }
        //qApp->processEvents();
        // update legend
        /*! \todo Need legend scrollview and legenditem classes */
        // draw the map

        mapLegend->update();
        qApp->processEvents();
        mapCanvas->freeze(false);
        mapCanvas->render2();
        QApplication::restoreOverrideCursor();
        statusBar()->message(mapCanvas->extent().stringRep());

    } else {
        QMessageBox::critical(this, tr("No OGR Provider"), tr("No OGR data provider was found in the QGIS lib directory"));
    }

}




/**
   Builds the list of file filter strings to later be used by
   QgisApp::addRasterLayer()

   We query GDAL for a list of supported raster formats; we then build
   a list of file filter strings from that list.  We return a string
   that contains this list that is suitable for use in a a
   QFileDialog::getOpenFileNames() call.

 */
static
void
buildSupportedRasterFileFilter_( QString & fileFilters )
{
   // first get the GDAL driver manager

   GDALDriverManager * driverManager =  GetGDALDriverManager( );

   if ( ! driverManager )
   {
      std::cerr << "unable to get GDALDriverManager\n";
      return;                   // XXX good place to throw exception if we 
   }                            // XXX decide to do exceptions

   // then iterate through all of the supported drivers, adding the
   // corresponding file filter

   GDALDriver * driver;         // current driver

   char **  driverMetadata;     // driver metadata strings

   QString driverLongName( "" ); // long name for the given driver
   QString driverExtension( "" ); // file name extension for given driver

   QStringList metadataTokens;  // essentially the metadata string delimited by '='

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

   for ( int i = 0; i < driverManager->GetDriverCount(); ++i )
   {
      driver = driverManager->GetDriver( i );

      if ( ! driver )
      {
         qWarning( "unable to get driver %d", i );
         continue;
      }

      // std::cerr << "got driver string " << driver->GetDescription() << "\n";

      driverMetadata = driver->GetMetadata();

      // presumably we know we've run out of metadta if either the
      // address is 0, or the first character is null
      while ( driverMetadata && '\0' != driverMetadata[0] )
      {
         metadataTokens = QStringList::split( "=", *driverMetadata );
         // std::cerr << "\t" << *driverMetadata << "\n";

         // XXX add check for malformed metadataTokens

         // Note that it's oddly possible for there to be a
         // DMD_EXTENSION with no corresponding defined extension
         // string; so we check that there're more than two tokens.

         if ( metadataTokens.count() > 1 )
         {
            if ( "DMD_EXTENSION" == metadataTokens[0] )
            {
               driverExtension = metadataTokens[1];

            }
            else if ( "DMD_LONGNAME" == metadataTokens[0] )
            {
               driverLongName = metadataTokens[1];

               // remove any superfluous (.*) strings at the end as
               // they'll confuse QFileDialog::getOpenFileNames()

               driverLongName.remove( QRegExp("\\(.*\\)$") );
            }
         }

         // if we have both the file name extension and the long name,
         // then we've all the information we need for the current
         // driver; therefore emit a file filter string and move to
         // the next driver
         if ( ! (driverExtension.isEmpty() || driverLongName.isEmpty()) )
         {
            // XXX add check for SDTS; in that case we want (*CATD.DDF)
            fileFilters += createFileFilter_( driverLongName,
                                              "*." + driverExtension );

            driverExtension = driverLongName = ""; // reset for next driver

            continue;           // ... to next driver, if any.
         }

         ++driverMetadata;

      } // each metadata item

      // XXX if you want to insert whining about formats with no
      // XXX extensions, this is where you'd check for it and complain

      driverExtension = driverLongName = ""; // reset for next driver

   } // each loaded GDAL driver

   // can't forget the default case
   fileFilters += "All files (*.*)";

} // buildSupportedRasterFileFilter_()





void
QgisApp::addRasterLayer()
{
    QString fileFilters;

    // build the file filters based on the loaded GDAL drivers
    buildSupportedRasterFileFilter_( fileFilters );

    QStringList selectedFilenames = 
       QFileDialog::getOpenFileNames( fileFilters,
                                      "",   // initial dir
                                      this, // parent dialog
                                      "OpenFileDialog", // QFileDialog qt object name
                                      "Select file name and type" );

    addRasterLayer( selectedFilenames );

} // QgisApp::addRasterLayer()




void QgisApp::addRasterLayer(QStringList theFileNameQStringList)
{
    if (theFileNameQStringList.empty()) {   // no files selected so bail out, but
        // allow mapCanvas to handle events
        // first
        mapCanvas->freeze(false);
        return;
    }

    QApplication::setOverrideCursor(Qt::WaitCursor);
    for (QStringList::Iterator myIterator = theFileNameQStringList.begin(); myIterator != theFileNameQStringList.end(); ++myIterator) {
        if (isValidRasterFileName(*myIterator)) {
            QFileInfo myFileInfo(*myIterator);
            QString myDirNameQString = myFileInfo.dirPath();    //get the directory the .adf file was in
            QString myBaseNameQString = myFileInfo.baseName();
            //only allow one copy of a ai grid file to be loaded at a time to
            //prevent the user selecting all adfs in 1 dir which actually represent 1 coverate,

            // create the layer
            QgsRasterLayer *layer = new QgsRasterLayer(*myIterator, myBaseNameQString);
            QObject::connect(layer, SIGNAL(repaintRequested()), mapCanvas, SLOT(refresh()));

            if (layer->isValid()) {
                // add it to the mapcanvas collection
                mapCanvas->addLayer(layer);
                projectIsDirty = true;
                // init the context menu so it can connect to slots in main app
                // XXX Yes, but what if the layer is invalid?  Should we still be doing this?
                layer->initContextMenu(this);
            } else {
                QString msg(*myIterator + " is not a valid or recognized raster data source");
                QMessageBox::critical(this, "Invalid Data Source", msg);
            }

            //only allow one copy of a ai grid file to be loaded at a time to
            //prevent the user selecting all adfs in 1 dir which actually represent 1 coverate,
            if (myBaseNameQString.lower().endsWith(".adf")) {

                break;
            }
        }                       //end of isValidRasterFileName check
    }
    mapLegend->update();

    qApp->processEvents();

    mapCanvas->freeze(false);

    mapCanvas->render2();

    QApplication::restoreOverrideCursor();

    statusBar()->message(mapCanvas->extent().stringRep());

}                               // QgisApp::addRasterLayer()

/** This helper checks to see whether the filename appears to be a valid raster file name */
bool QgisApp::isValidRasterFileName(QString theFileNameQString)
{
    QString name = theFileNameQString.lower();
    return (name.endsWith(".adf") ||
            name.endsWith(".asc") ||
            name.endsWith(".grd") ||
            name.endsWith(".img") ||
            name.endsWith(".tif") || name.endsWith(".png") || name.endsWith(".jpg") || name.endsWith(".dem") || name.endsWith(".ddf"));
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



#ifdef POSTGRESQL
void QgisApp::addDatabaseLayer()
{
    // check to see if we have a postgres provider available
    QString pOgr = providerRegistry->library("postgres");
    if (pOgr.length() > 0) {
        // only supports postgis layers at present
        // show the postgis dialog


        QgsDbSourceSelect *dbs = new QgsDbSourceSelect(this);
        mapCanvas->freeze();
        if (dbs->exec()) {
            QApplication::setOverrideCursor(Qt::WaitCursor);


            // repaint the canvas if it was covered by the dialog

            // add files to the map canvas
            QStringList tables = dbs->selectedTables();

            QString connInfo = dbs->connInfo();
            // for each selected table, connect to the database, parse the WKT geometry,
            // and build a cavnasitem for it
            // readWKB(connInfo,tables);
            QStringList::Iterator it = tables.begin();
            while (it != tables.end()) {

                // create the layer
                //qWarning("creating lyr");
                QgsVectorLayer *lyr = new QgsVectorLayer(connInfo + " table=" + *it, *it, "postgres");
                if (lyr->isValid()) {
                    // init the context menu so it can connect to slots in main app
                    lyr->initContextMenu(this);

                    // give it a random color
                    QgsSingleSymRenderer *renderer = new QgsSingleSymRenderer();    //add single symbol renderer as default
                    lyr->setRenderer(renderer);
                    renderer->initializeSymbology(lyr);
                    // add it to the mapcanvas collection
                    mapCanvas->addLayer(lyr);
                    projectIsDirty = true;
                } else {
                    std::cerr << *it << " is an invalid layer - not loaded" << std::endl;
                    QMessageBox::critical(this, tr("Invalid Layer"), tr("%1 is an invalid layer and cannot be loaded.").arg(*it));
                    delete lyr;
                }
                //qWarning("incrementing iterator");
                ++it;
            }
            //  qApp->processEvents();
            // update legend
            /*! \todo Need legend scrollview and legenditem classes */
            mapLegend->update();

            // draw the map
            //mapCanvas->render2();
            statusBar()->message(mapCanvas->extent().stringRep());

        }
        qApp->processEvents();

        mapCanvas->freeze(false);
        mapCanvas->render2();
        QApplication::restoreOverrideCursor();
    } else {
        QMessageBox::critical(this, tr("No PostgreSQL Provider"),
                              tr("No PostgreSQL data provider was found in the QGIS lib directory"));

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

    if (answer != QMessageBox::Cancel) {
        mapCanvas->removeAll();
        setCaption(tr("Quantum GIS -- Untitled"));
        mapCanvas->clear();
        mapLegend->update();
        fullPath = "";
        projectIsDirty = false;
    }
}
void QgisApp::fileOpen()
{
    int answer = saveDirty();

    if (answer != QMessageBox::Cancel) {
        mapCanvas->freeze(true);
        QgsProjectIo *pio = new QgsProjectIo(mapCanvas, QgsProjectIo::OPEN, this);

        if (pio->read()) {
            setCaption(tr("Quantum GIS --") + " " + pio->baseName());
            fullPath = pio->fullPathName();
        }
        delete pio;

        mapLegend->update();
        mapCanvas->freeze(false);
        projectIsDirty = false;
    }
}
void QgisApp::fileSave()
{
    QgsProjectIo *pio = new QgsProjectIo(mapCanvas, QgsProjectIo::SAVE);
    pio->setFileName(fullPath);
    if (pio->write()) {
        setCaption(tr("Quantum GIS --") + " " + pio->baseName());
        statusBar()->message(tr("Saved map to:") + " " + pio->fullPathName());
    }
    delete pio;
    projectIsDirty = false;
}

void QgisApp::fileSaveAs()
{
    QgsProjectIo *pio = new QgsProjectIo(mapCanvas, QgsProjectIo::SAVEAS);
    if (pio->write()) {
        setCaption(tr("Quantum GIS --") + " " + pio->baseName());
        statusBar()->message(tr("Saved map to:") + " " + pio->fullPathName());
    }
    delete pio;
    projectIsDirty = false;
}

void QgisApp::exportMapServer()
{
    // check to see if there are any layers to export
    if (mapCanvas->layerCount() > 0) {
        QgsMapserverExport *mse = new QgsMapserverExport(mapCanvas, this);
        if (mse->exec()) {
            mse->write();
        }
        delete mse;
    } else {
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
       statusBar()->message(ext.stringRep());
       mapCanvas->clear();
       mapCanvas->render2(); */

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
    QListViewItem *li = legendView->currentItem();
    if (li) {
        QgsMapLayer *lyr = ((QgsLegendItem *) li)->layer();
        if (lyr) {
            lyr->table();

        } else {
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
//  std::cout << "In  QgisApp::drawLayers()" << std::endl;
    mapCanvas->setDirty(true);
    mapCanvas->render2();
}

void QgisApp::showMouseCoordinate(QgsPoint & p)
{
    statusBar()->message(p.stringRep());
    //qWarning("X,Y is: " + p.stringRep());

}

void QgisApp::testButton()
{
    /* QgsShapeFileLayer *sfl = new QgsShapeFileLayer("foo");
       mapCanvas->addLayer(sfl); */
//      delete sfl;

}

void QgisApp::layerProperties()
{
    layerProperties(legendView->currentItem());
}

void QgisApp::layerProperties(QListViewItem * lvi)
{
    QgsMapLayer *lyr;
    if (lvi) {
        lyr = ((QgsLegendItem *) lvi)->layer();
    } else {
        // get the selected item
        QListViewItem *li = legendView->currentItem();
        lyr = ((QgsLegendItem *) li)->layer();
    }



    QString currentName = lyr->name();
    //test if we have a raster or vector layer and show the appropriate dialog
    if (lyr->type() == QgsMapLayer::RASTER) {
        QgsRasterLayerProperties *rlp = new QgsRasterLayerProperties(lyr);
        // The signals to change the raster layer properties will only be emitted
        // when the user clicks ok or apply
        //connect(rlp, SIGNAL(setTransparency(unsigned int)), SLOT(lyr(slot_setTransparency(unsigned int))));
        if (rlp->exec()) {
            //this code will be called it the user selects ok
            mapCanvas->setDirty(true);
            mapCanvas->refresh();
            mapCanvas->render2();
            mapLegend->update();
            delete rlp;
            qApp->processEvents();
        }
    } else {
        lyr->showLayerProperties();
    }

    //TODO Fix this area below and above
    //this is a very hacky way to force the legend entry to refresh - the call above does ne happen for some reason
    //mapCanvas->render2();
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
       mapCanvas->render2();
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
    QListViewItem *lvi = legendView->currentItem();
    QgsMapLayer *lyr = ((QgsLegendItem *) lvi)->layer();
    mapCanvas->remove(lyr->getLayerID());
    mapLegend->update();
    mapCanvas->freeze(false);
    // draw the map
    mapCanvas->clear();
    mapCanvas->render2();


}

void QgisApp::zoomToLayerExtent()
{

    // get the selected item
    QListViewItem *li = legendView->currentItem();
    QgsMapLayer *lyr = ((QgsLegendItem *) li)->layer();
    mapCanvas->setExtent(lyr->extent());
    mapCanvas->clear();
    mapCanvas->render2();

}

void QgisApp::rightClickLegendMenu(QListViewItem * lvi, const QPoint & pt, int)
{
    if (lvi) {
        // get the context menu from the layer and display it 
        QgsMapLayer *lyr = ((QgsLegendItem *) lvi)->layer();
        QPopupMenu *popMenu = lyr->contextMenu();
        if (popMenu) {
            popMenu->exec(pt);
        }
    }
}

void QgisApp::clickLegendMenu(QListViewItem * lvi)
{
    if (lvi) {
        // disable/enable toolbar buttons as appropriate based on selected
        // layer type
        QgsMapLayer *lyr = ((QgsLegendItem *) lvi)->layer();
        if (lyr->type() == QgsMapLayer::RASTER) {
            actionIdentify->setEnabled(FALSE);
            actionSelect->setEnabled(FALSE);
            actionOpenTable->setEnabled(FALSE);
            // if one of these map tools is selected, set cursor to default
            if (mapTool == QGis::Identify || mapTool == QGis::Select || mapTool == QGis::Table) {
                delete mapCursor;
                mapCursor = new QCursor();
                mapCanvas->setCursor(*mapCursor);
            }
        } else {
            actionIdentify->setEnabled(TRUE);
            actionSelect->setEnabled(TRUE);
            actionOpenTable->setEnabled(TRUE);
            // if one of these map tools is selected, make sure appropriate cursor gets set
            switch (mapTool) {
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
// This function here to annoy the purists :)
int QgisApp::getInt()
{
    return 99;
}

void QgisApp::actionPluginManager_activated()
{
    QgsPluginManager *pm = new QgsPluginManager(this);
    if (pm->exec()) {
        // load selected plugins
        std::vector < QgsPluginItem > pi = pm->getSelectedPlugins();
        std::vector < QgsPluginItem >::iterator it = pi.begin();
        while (it != pi.end()) {
            QgsPluginItem plugin = *it;
            loadPlugin(plugin.name(), plugin.description(), plugin.fullPath());
            it++;
        }


    }


}
void QgisApp::loadPlugin(QString name, QString description, QString fullPath)
{
    // first check to see if its already loaded
    QgsPluginRegistry *pRegistry = QgsPluginRegistry::instance();
    QString lib = pRegistry->library(name);
    if (lib.length() > 0) {
        // plugin is loaded
        // QMessageBox::warning(this, "Already Loaded", description + " is already loaded");
    } else {
        QLibrary *myLib = new QLibrary(fullPath);
#ifdef DEBUG
        std::cout << "Library name is " << myLib->library() << std::endl;
#endif
        bool loaded = myLib->load();
        if (loaded) {
#ifdef DEBUG
            std::cout << "Loaded test plugin library" << std::endl;
            std::cout << "Attempting to resolve the classFactory function" << std::endl;
#endif

            type_t *pType = (type_t *) myLib->resolve("type");


            switch (pType()) {
              case QgisPlugin::UI:
                  {
                      // UI only -- doesn't use mapcanvas
                      create_ui *cf = (create_ui *) myLib->resolve("classFactory");
                      if (cf) {
                          QgisPlugin *pl = cf(this, qgisInterface);
                          if (pl) {
                              pl->initGui();
                              // add it to the plugin registry
                              pRegistry->addPlugin(myLib->library(), name, pl);
                          } else {
                              // something went wrong
                              QMessageBox::warning(this, tr("Error Loading Plugin"), tr("There was an error loading %1."));
                          }
                      } else {
#ifdef DEBUG
                          std::cout << "Unable to find the class factory for " << fullPath << std::endl;
#endif
                      }
                  }
                  break;
              case QgisPlugin::MAPLAYER:
                  {
                      // Map layer - requires interaction with the canvas
                      create_it *cf = (create_it *) myLib->resolve("classFactory");
                      if (cf) {
                          QgsMapLayerInterface *pl = cf();
                          if (pl) {
                              // set the main window pointer for the plugin
                              pl->setQgisMainWindow(this);
                              pl->initGui();

                          } else {
                              // something went wrong
                              QMessageBox::warning(this, tr("Error Loading Plugin"), tr("There was an error loading %1."));
                          }
                      } else {
#ifdef DEBUG
                          std::cout << "Unable to find the class factory for " << fullPath << std::endl;
#endif
                      }
                  }
                  break;
              default:
                  // type is unknown
#ifdef DEBUG
                  std::cout << "Plugin " << fullPath << " did not return a valid type and cannot be loaded" << std::endl;
#endif
                  break;
            }

            /*  }else{
               std::cout << "Unable to find the class factory for " << fullPath << std::endl;
               } */

        } else {
#ifdef DEBUG
            std::cout << "Failed to load " << fullPath << "\n";
#endif
        }
    }
}
void QgisApp::testMapLayerPlugins()
{
    // map layer plugins live in their own directory (somewhere to be determined)
    QDir mlpDir("../plugins/maplayer", "*.so.1.0.0", QDir::Name | QDir::IgnoreCase, QDir::Files);
    if (mlpDir.count() == 0) {
        QMessageBox::information(this, tr("No MapLayer Plugins"), tr("No MapLayer plugins in ../plugins/maplayer"));
    } else {
        for (unsigned i = 0; i < mlpDir.count(); i++) {
#ifdef DEBUG
            std::cout << "Getting information for plugin: " << mlpDir[i] << std::endl;
            std::cout << "Attempting to load the plugin using dlopen\n";
#endif
            void *handle = dlopen("../plugins/maplayer/" + mlpDir[i], RTLD_LAZY);
            if (!handle) {
#ifdef DEBUG
                std::cout << "Error in dlopen: " << dlerror() << std::endl;
#endif
            } else {
#ifdef DEBUG
                std::cout << "dlopen suceeded" << std::endl;
#endif
                dlclose(handle);
            }

            QLibrary *myLib = new QLibrary("../plugins/maplayer/" + mlpDir[i]);
#ifdef DEBUG
            std::cout << "Library name is " << myLib->library() << std::endl;
#endif
            bool loaded = myLib->load();
            if (loaded) {
#ifdef DEBUG
                std::cout << "Loaded test plugin library" << std::endl;
                std::cout << "Attempting to resolve the classFactory function" << std::endl;
#endif
                create_it *cf = (create_it *) myLib->resolve("classFactory");

                if (cf) {
#ifdef DEBUG
                    std::cout << "Getting pointer to a MapLayerInterface object from the library\n";
#endif
                    QgsMapLayerInterface *pl = cf();
                    if (pl) {
#ifdef DEBUG
                        std::cout << "Instantiated the maplayer test plugin\n";
#endif
                        // set the main window pointer for the plugin
                        pl->setQgisMainWindow(this);
#ifdef DEBUG
                        std::cout << "getInt returned " << pl->getInt() << " from map layer plugin\n";
#endif
                        // set up the gui
                        pl->initGui();
                    } else {
#ifdef DEBUG
                        std::cout << "Unable to instantiate the maplayer test plugin\n";
#endif
                    }
                }
            } else {
#ifdef DEBUG
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
    if (false) {
// try to load plugins from the plugin directory and test each one

        QDir pluginDir("../plugins", "*.so*", QDir::Name | QDir::IgnoreCase, QDir::Files | QDir::NoSymLinks);
        //pluginDir.setFilter(QDir::Files || QDir::NoSymLinks);
        //pluginDir.setNameFilter("*.so*");
        if (pluginDir.count() == 0) {
            QMessageBox::information(this, tr("No Plugins"),
                                     tr("No plugins found in ../plugins. To test plugins, start qgis from the src directory"));
        } else {

            for (unsigned i = 0; i < pluginDir.count(); i++) {
#ifdef DEBUG
                std::cout << "Getting information for plugin: " << pluginDir[i] << std::endl;
#endif
                QLibrary *myLib = new QLibrary("../plugins/" + pluginDir[i]);   //"/home/gsherman/development/qgis/plugins/" + pluginDir[i]);
#ifdef DEBUG
                std::cout << "Library name is " << myLib->library() << std::endl;
#endif
                //QLibrary myLib("../plugins/" + pluginDir[i]);
#ifdef DEBUG
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
                if (loaded) {
#ifdef DEBUG
                    std::cout << "Loaded test plugin library" << std::endl;
                    std::cout << "Getting the name of the plugin" << std::endl;
#endif
                    name_t *pName = (name_t *) myLib->resolve("name");
                    if (pName) {
                        QMessageBox::information(this, tr("Name"), tr("Plugin %1 is named %2").arg(pluginDir[i]).arg(pName()));
                    }
#ifdef DEBUG
                    std::cout << "Attempting to resolve the classFactory function" << std::endl;
#endif
                    create_t *cf = (create_t *) myLib->resolve("classFactory");

                    if (cf) {
#ifdef DEBUG
                        std::cout << "Getting pointer to a QgisPlugin object from the library\n";
#endif
                        QgisPlugin *pl = cf(this, qgisInterface);
#ifdef DEBUG
                        std::cout << "Displaying name, version, and description\n";
                        std::cout << "Plugin name: " << pl->name() << std::endl;
                        std::cout << "Plugin version: " << pl->version() << std::endl;
                        std::cout << "Plugin description: " << pl->description() << std::endl;
#endif
                        QMessageBox::information(this, tr("Plugin Information"), tr("QGis loaded the following plugin:") +
                                                 tr("Name: %1").arg(pl->name()) + "\n" + tr("Version: %1").arg(pl->version()) + "\n" +
                                                 tr("Description: %1").arg(pl->description()));
                        // unload the plugin (delete it)
#ifdef DEBUG
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
                } else {
                    QMessageBox::warning(this, tr("Unable to Load Plugin"),
                                         tr("QGIS was unable to load the plugin from: %1").arg(pluginDir[i]));
#ifdef DEBUG
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
    int dw = d->width();        // returns desktop width
    int dh = d->height();       // returns desktop height
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
    if (pos > -1) {
        pos += contentFlag.length();
        /* std::cout << versionMessage << "\n ";
           std::cout << "Pos is " << pos <<"\n"; */
        versionMessage = versionMessage.mid(pos);
        QStringList parts = QStringList::split("|", versionMessage);
        // check the version from the  server against our version
        QString versionInfo;
        int currentVersion = parts[0].toInt();
        if (currentVersion > QGis::qgisVersionInt) {
            // show version message from server
            versionInfo = tr("There is a new version of QGIS available") + "\n";
        } else {
            if (QGis::qgisVersionInt > currentVersion) {
                versionInfo = tr("You are running a development version of QGIS") + "\n";
            } else {
                versionInfo = tr("You are running the current version of QGIS") + "\n";
            }
        }
        if (parts.count() > 1) {
            versionInfo += parts[1] + "\n\n" + tr("Would you like more information?");;
            int result = QMessageBox::information(this, tr("QGIS Version Information"), versionInfo, tr("Yes"), tr("No"));
            if (result == 0) {
                // show more info
                QgsMessageViewer *mv = new QgsMessageViewer(this);
                mv->setCaption(tr("QGIS - Changes in CVS"));
                mv->setMessage(parts[2]);
                mv->exec();
            }
        } else {
            QMessageBox::information(this, tr("QGIS Version Information"), versionInfo);
        }
    } else {
        QMessageBox::warning(this, tr("QGIS Version Information"), tr("Unable to get current version information from server"));
    }
}
void QgisApp::socketError(int e)
{
    QApplication::restoreOverrideCursor();
// get errror type
    QString detail;
    switch (e) {
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
    while (socket->bytesAvailable() > 0) {
        char *data = new char[socket->bytesAvailable() + 1];
        memset(data, '\0', socket->bytesAvailable() + 1);
        socket->readBlock(data, socket->bytesAvailable());
        versionMessage += data;
        delete[]data;
    }

}
void QgisApp::options()
{
    QgsOptionsBase *optionsDialog = new QgsOptionsBase(this);
    optionsDialog->exec();
}

void QgisApp::helpContents()
{
    openURL("index.html");
}

void QgisApp::openURL(QString url, bool useQgisDocDirectory)
{
    // open help in user browser
    if (useQgisDocDirectory) {
        url = appDir + "/share/doc/" + url;
    }
    // find a browser
    QSettings settings;
    QString browser = settings.readEntry("/qgis/browser");
    if (browser.length() == 0) {
        // ask user for browser and use it
        bool ok;
        QString text = QInputDialog::getText("QGIS Browser Selection",
                                             "Enter the name of a web browser to use (eg. konqueror).\nEnter the full path if the browser is not in your PATH.\nYou can change this option later by selection Options from the Tools menu.",
                                             QLineEdit::Normal,
                                             QString::null, &ok, this);
        if (ok && !text.isEmpty()) {
            // user entered something and pressed OK
            browser = text;
            // save the setting
            settings.writeEntry("/qgis/browser", browser);
        } else {
            browser = "";
        }

    }
    if (browser.length() > 0) {
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
    QListViewItem *lvi = legendView->currentItem();
    QgsMapLayer *lyr = 0;
    if (lvi) {
        lyr = ((QgsLegendItem *) lvi)->layer();
    }
    return lyr;
}

QString QgisApp::activeLayerSource()
{
    QString source;
    QListViewItem *lvi = legendView->currentItem();
    QgsMapLayer *lyr = 0;
    if (lvi) {
        lyr = ((QgsLegendItem *) lvi)->layer();
        source = lyr->source();
    }
    return source;
}

/** Add a vector layer directly without prompting user for location */
void QgisApp::addVectorLayer(QString vectorLayerPath, QString baseName, QString providerKey)
{
    // check to see if the appropriate provider is available
    QString providerName;

    QString pProvider = providerRegistry->library(providerKey);
    if (pProvider.length() > 0) {
        mapCanvas->freeze();
        QApplication::setOverrideCursor(Qt::WaitCursor);
        // create the layer
        QgsVectorLayer *lyr;
        if (providerKey == "postgres") {
            lyr = new QgsVectorLayer(vectorLayerPath + " table=" + baseName, baseName, "postgres");
        } else {
            if (providerKey == "ogr") {
                lyr = new QgsVectorLayer(vectorLayerPath, baseName, "ogr");
            }
        }

        // init the context menu so it can connect to slots in main app
        lyr->initContextMenu(this);

        // give it a random color
        QgsSingleSymRenderer *renderer = new QgsSingleSymRenderer();    //add single symbol renderer as default
        lyr->setRenderer(renderer);
        renderer->initializeSymbology(lyr);
        // add it to the mapcanvas collection
        mapCanvas->addLayer(lyr);
        projectIsDirty = true;
        //qWarning("incrementing iterator");
        /*! \todo Need legend scrollview and legenditem classes */
        mapLegend->update();

        // draw the map
        //mapCanvas->render2();
        statusBar()->message(mapCanvas->extent().stringRep());

    }
    qApp->processEvents();

    mapCanvas->freeze(false);
    mapCanvas->render2();
    QApplication::restoreOverrideCursor();
}

int QgisApp::saveDirty()
{
    int answer = 0;
    mapCanvas->freeze(true);
#ifdef DEBUG
    std::cout << "Layer count is " << mapCanvas->layerCount() << std::endl;
    std::cout << "Project is ";
    if (projectIsDirty) {
        std::cout << "dirty" << std::endl;
    } else {
        std::cout << "not dirty" << std::endl;
    }
    std::cout << "Map canvas is ";
    if (mapCanvas->isDirty()) {
        std::cout << "dirty" << std::endl;
    } else {
        std::cout << "not dirty" << std::endl;
    }
#endif
    if ((projectIsDirty || (mapCanvas->isDirty()) && mapCanvas->layerCount() > 0)) {
        // flag project is dirty since dirty state of canvas is reset if "dirty"
        // is based on a zoom or pan
        projectIsDirty = true;
        // prompt user to save
        answer = QMessageBox::information(this, "Save?", "Do you want to save the current project?",
                                          QMessageBox::Yes | QMessageBox::Default,
                                          QMessageBox::No, QMessageBox::Cancel | QMessageBox::Escape);
        if (answer == QMessageBox::Yes) {
            fileSave();
        }
    }
    mapCanvas->freeze(false);
    return answer;
}
