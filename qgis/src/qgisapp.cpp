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

#include <iostream>
#include <iomanip>
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
#include "qgssinglesymrenderer.h"
//#include "qgssisydialog.h"
#include "../plugins/qgisplugin.h"
#include "xpm/qgis.xpm"
#include <ogrsf_frmts.h>

/* typedefs for plugins */
typedef QgsMapLayerInterface* create_it();
typedef QgisPlugin* create_ui(QgisApp *qgis, QgisIface *qI);
typedef QString name_t();
typedef QString description_t();
typedef int type_t();

// cursors
static char *zoom_in[]={
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
"..............#."};

static char *zoom_out[]={
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
"..............#."};



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



static char *select_cursor[]={
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
"#############.##"};

static char *identify_cursor[]={
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
"#############.##"};

// constructor starts here   

QgisApp::QgisApp(QWidget * parent, const char *name, WFlags fl):QgisAppBase(parent, name, fl)
{
  //
  // Set up the splash screen
  //
  SplashScreen *mySplash = new SplashScreen(  );
  mySplash->setStatus(tr("Loading QGIS..."));
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


  mapLegend = new QgsLegend(legendView);	//frameMain);
  // mL = new QScrollView(split);
  //add a canvas
  mapCanvas = new QgsMapCanvas(split);
  // resize it to fit in the frame
  //    QRect r = frmCanvas->rect();
  //    canvas->resize(r.width(), r.height());
  mapCanvas->setBackgroundColor(Qt::white);	//QColor (220, 235, 255));
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

  // create the layer popup menu
  /* popMenu = new QPopupMenu();
  popMenu->insertItem(tr("&Zoom to extent of selected layer"), this, SLOT(zoomToLayerExtent()));
  popMenu->insertItem(tr("&Open attribute table"), this, SLOT(attributeTable()));
  popMenu->insertSeparator();
  popMenu->insertItem(tr("&Properties"), this, SLOT(layerProperties()));
  popMenu->insertSeparator();
  popMenu->insertItem(tr("&Remove"), this, SLOT(removeLayer())); */
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
  plib += "/lib";
  providerRegistry = QgsProviderRegistry::instance(plib);
  // set the provider plugin path 
  std::cout << "Setting plugin lib dir to " << plib << std::endl;
  // connect the "cleanup" slot
  connect(qApp, SIGNAL(aboutToQuit()), this, SLOT(saveWindowState()));
  restoreWindowState();
  // set the focus to the map canvase
  mapCanvas->setFocus();
  mySplash->finish( this );
  delete mySplash;
  
  std::cout << "Plugins are installed in " << plib << std::endl;
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
		versionString +=  tr(" (no PostgreSQL support)");
	#endif
	abt->setVersion(versionString);
	QString urls = tr("Web Page: http://qgis.sourceforge.net") +
  "\n" + tr("Sourceforge Project Page: http://sourceforge.net/projects/qgis");
	abt->setURLs(urls);
	QString watsNew = tr("Version") + " ";
	watsNew += QGis::qgisVersion;
	watsNew += "\n"
	"** Raster support\n"
  "** Improved data source handling\n"
  "** Symbology\n"
  "\n"
		;


	abt->setWhatsNew(watsNew);
  
  // add the available plugins to the list
  QString providerInfo = "<b>" + tr("Available Data Provider Plugins") + "</b><br>";
  abt->setPluginInfo(providerInfo + providerRegistry->pluginList(true));
	abt->exec();

}

void QgisApp::addLayer()
{
  // check to see if we have an ogr provider available
  QString pOgr = providerRegistry->library("ogr");
  if(pOgr.length() > 0){
	mapCanvas->freeze();
	QStringList files = QFileDialog::getOpenFileNames(tr("Shapefiles (*.shp);;All files (*.*)"), 0, this, "open files dialog",
													  tr("Select one or more layers to add"));
	QApplication::setOverrideCursor(Qt::WaitCursor);
	QStringList::Iterator it = files.begin();
	while (it != files.end()) {


		QFileInfo fi(*it);
		QString base = fi.baseName();


		// create the layer

	//dp	QgsShapeFileLayer *lyr = new QgsShapeFileLayer(*it, base);
    	QgsVectorLayer *lyr = new QgsVectorLayer(*it, base, "ogr");

		if (lyr->isValid()) {
       // init the context menu so it can connect to slots in main app
       lyr->initContextMenu(this);
       //add single symbol renderer as default
		    QgsSingleSymRenderer* renderer=new QgsSingleSymRenderer();
		    lyr->setRenderer(renderer);
		    renderer->initializeSymbology(lyr);
		    mapCanvas->addLayer(lyr);
        projectIsDirty = true;
		} else {
			QString msg = *it + " ";
			msg += tr("is not a valid or recognized data source");
			QMessageBox::critical(this, tr("Invalid Data Source"), msg);
		}

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

  }else{
    QMessageBox::critical(this, tr("No OGR Provider"),tr("No OGR data provider was found in the QGIS lib directory"));    
  }

}


void
QgisApp::addRasterLayer()
{
   // insure that the map canvas temporarily suspends event processing
   // until we've loaded the raster

  mapCanvas->freeze();

  QString myFileTypeQString;

  QString myArcInfoBinaryGridFilterString="Arc Info Binary Grid (*.adf)";
  QString myArcInfoAsciiGridFilterString="Arc Info Ascii Grid (*.asc;*.grd)";
  QString myERDASFilterString="ERDAS Imagine (*.img)";
  QString myGeoTiffFilterString="Geo tiff (*.tif)";
  QString myUSGSAsciiDemFilterString="USGS Ascii DEM (*.dem;*.DEM)";
  QString myGrassFilterString="Grass raster (*.*)";
  QString mySDTSFilterString="SDTS (*CATD*.DDF)";
  QString myAllRasterFormatsFilterString = "All Rasters (*.asc;*.grd;*.img;*.tif;*.png;*.jpg;*.dem;*.DEM;*.DDF)";
  QString myOtherFormatsFilterString = "Other (*.*)";
  //QString myBilFilterString="Band Interleaved by Line (*.bil)";
  //QString myJpgFilterString="Geo jpg (*.jpg)";

  QStringList myFileNameQStringList = QFileDialog::getOpenFileNames(
          myAllRasterFormatsFilterString + ";;" +
          myArcInfoBinaryGridFilterString + ";;" +
          myArcInfoAsciiGridFilterString + ";;" +
          myERDASFilterString + ";;" +
          //myBilFilterString + ";;" +
          //myJpgFilterString + ";;" +  
          myGeoTiffFilterString + ";;" +
          myGrassFilterString + ";;" +
          myUSGSAsciiDemFilterString + ";;" + 
	  mySDTSFilterString + ";;" +
          myOtherFormatsFilterString, //filters to select 
          "" , //initial dir
          this , //parent dialog
          "OpenFileDialog" , //QFileDialog qt object name
          "Select file name and type" , //caption
          &myFileTypeQString //the pointer to store selected filter
          );  
  //cout << "Selected filetype filter is : " << myFileTypeQString << endl;

  if ( myFileNameQStringList.empty() ) 
  {                             // no files selected so bail out, but
                                // allow mapCanvas to handle events
                                // first
     mapCanvas->freeze( false );
     return; 
  }

  QApplication::setOverrideCursor(Qt::WaitCursor);

  // handle ArcInfo rasters
  if ( myArcInfoBinaryGridFilterString == myFileTypeQString )
  {
    //if multiple file were selected ignore the others because currently we 
    //can only select one AI Binary Grid dir at a time

    QStringList::Iterator it = myFileNameQStringList.begin();
    QFileInfo fi(*it);
    QString base = fi.dirPath(); //get the directory the .adf file was in

    // create the layer
    QgsRasterLayer *layer = new QgsRasterLayer(*it, base);
    QObject::connect(layer,SIGNAL(repaintRequested()),mapCanvas,SLOT(refresh()));

    if ( layer->isValid() )
    {
      // add it to the mapcanvas collection
      mapCanvas->addLayer( layer );
      projectIsDirty = true;
    } else 
    {
      QString msg( *it + " is not a valid or recognized raster data source" );
      QMessageBox::critical( this, "Invalid Data Source", msg );
    }
      // init the context menu so it can connect to slots in main app
      layer->initContextMenu( this );
  }
  else // Any other GDAL type
  {
    for ( QStringList::Iterator it = myFileNameQStringList.begin();
          it != myFileNameQStringList.end();
          ++it )
    {
      QFileInfo fi(*it);
      QString baseName = fi.baseName();

      // create the layer
      QgsRasterLayer *layer = new QgsRasterLayer(*it, baseName);
      QObject::connect( layer, SIGNAL(repaintRequested()), mapCanvas, SLOT(refresh()) );

      if ( layer->isValid() )
      {// add it to the mapcanvas collection
        mapCanvas->addLayer( layer );
      } else 
      {
        QString msg( *it + " is not a valid or recognized raster data source" );
        QMessageBox::critical( this, "Invalid Data Source", msg );
      }

      // init the context menu so it can connect to slots in main app
      // XXX Yes, but what if the layer is invalid?  Should we still be doing this?
      layer->initContextMenu( this );
    }

  }

  mapLegend->update();

  qApp->processEvents();

  mapCanvas->freeze(false);

  mapCanvas->render2();

  QApplication::restoreOverrideCursor();

  statusBar()->message( mapCanvas->extent().stringRep() );

} // QgisApp::addRasterLayer()





#ifdef POSTGRESQL
void QgisApp::addDatabaseLayer()
{
  // check to see if we have a postgres provider available
  QString pOgr = providerRegistry->library("postgres");
  if(pOgr.length() > 0){
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
      if(lyr->isValid()){
        // init the context menu so it can connect to slots in main app
        lyr->initContextMenu(this);
        
        // give it a random color
        QgsSingleSymRenderer* renderer=new QgsSingleSymRenderer();//add single symbol renderer as default
        lyr->setRenderer(renderer);
        renderer->initializeSymbology(lyr);
        // add it to the mapcanvas collection
        mapCanvas->addLayer(lyr);
        projectIsDirty = true;
      }else{
        std::cerr << *it << " is an invalid layer - not loaded" << std::endl;
        QMessageBox::critical(this, tr("Invalid Layer"),
          tr("%1 is an invalid layer and cannot be loaded.").arg(*it));
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
  }else{
    QMessageBox::critical(this, tr("No PostgreSQL Provider"),tr("No PostgreSQL data provider was found in the QGIS lib directory"));    

  }
}
#endif
void QgisApp::fileExit()
{
	QApplication::exit();

}
void QgisApp::fileNew(){
  int answer= saveDirty();
 
  if(answer != QMessageBox::Cancel){
    mapCanvas->removeAll();
    setCaption(tr("Quantum GIS -- Untitled"));
    mapCanvas->clear();
    mapLegend->update();
    fullPath = "";
    projectIsDirty = false;
  }
}
void QgisApp::fileOpen(){
  int answer= saveDirty();
 
  if(answer != QMessageBox::Cancel){
    mapCanvas->freeze(true);
    QgsProjectIo *pio = new QgsProjectIo(mapCanvas, QgsProjectIo::OPEN, this);
    
    if(pio->read()){
      setCaption(tr("Quantum GIS --") +" " + pio->baseName());
      fullPath = pio->fullPathName();
      }
    delete pio;
    
    mapLegend->update();
    mapCanvas->freeze(false);
    projectIsDirty = false;
  }
}
void QgisApp::fileSave(){
	QgsProjectIo *pio = new QgsProjectIo(mapCanvas, QgsProjectIo::SAVE);
	pio->setFileName(fullPath);
	if(pio->write()){
		setCaption(tr("Quantum GIS --") + " " + pio->baseName());
		statusBar()->message(tr("Saved map to:") +" "  + pio->fullPathName());
	}
	delete pio;
  projectIsDirty = false;
}

void QgisApp::fileSaveAs(){
	QgsProjectIo *pio = new QgsProjectIo(mapCanvas, QgsProjectIo::SAVEAS);
	if(pio->write()){
		setCaption(tr("Quantum GIS --") + " " + pio->baseName());
		statusBar()->message(tr("Saved map to:") + " " + pio->fullPathName());
	}
	delete pio;	
   projectIsDirty = false;
}

void QgisApp::exportMapServer(){
	// check to see if there are any layers to export
	if(mapCanvas->layerCount() > 0){
		QgsMapserverExport *mse = new QgsMapserverExport(mapCanvas, this);
		if(mse->exec()){
			mse->write();
		}
		delete mse;
	}else{
		QMessageBox::warning(this,"No Map Layers",
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
        
 
        QPixmap myZoomInQPixmap=QPixmap((const char **) zoom_in);
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

        QPixmap myZoomOutQPixmap=QPixmap((const char **) zoom_out);
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
        
        QPixmap myIdentifyQPixmap=QPixmap((const char **) identify_cursor);
	delete mapCursor;
	mapCursor = new QCursor(myIdentifyQPixmap, 1, 1);
	mapCanvas->setCursor(*mapCursor);
}

void QgisApp::attributeTable()
{
	QListViewItem *li = legendView->currentItem();
	if(li){
	QgsMapLayer *lyr = ((QgsLegendItem *) li)->layer();
		if (lyr) {
			lyr->table();
	
		} else {
			QMessageBox::information(this, tr("No Layer Selected"), tr("To open an attribute table, you must select a layer in the legend"));
		}
	}
}

void QgisApp::select()
{

       // set current map tool to select
       mapCanvas->setMapTool(QGis::Select);
    
            
        QPixmap mySelectQPixmap=QPixmap((const char **) select_cursor);
	delete mapCursor;
	mapCursor = new QCursor(mySelectQPixmap, 1, 1);
	mapCanvas->setCursor(*mapCursor);
}

//void QgisApp::readWKB (const char *connInfo, QStringList tables)
//{
//    PgCursor pgc (connInfo, "testcursor");
//   // get "endianness"
//   char *chkEndian = new char[4];
//   memset (chkEndian, '\0', 4);
//   chkEndian[0] = 0xE8;
//   int *ce = (int *) chkEndian;
//   bool isNDR = (232 == *ce);
//   /*     if(*ce != 232)
//   cout << "Big endian" << endl;
//   else
//   cout << "Little endian" << endl;
//   */
//   QStringList::Iterator it = tables.begin ();
//   while (it != tables.end ())
//     {

//       // get the extent of the layer
//       QString esql = "select extent(the_geom) from " + *it;
//       PgDatabase *pd = new PgDatabase (connInfo);
//       int result = pd->ExecTuplesOk ((const char *) esql);
//       QString extent = pd->GetValue (0, 0);
//       // parse out the x and y values
//       extent = extent.right (extent.length () - extent.find ("BOX3D(") - 6);
//       QStringList coordPairs = QStringList::split (",", extent);
//       QStringList x1y1 = QStringList::split (" ", coordPairs[0]);
//       QStringList x2y2 = QStringList::split (" ", coordPairs[1]);
//       double x1 = x1y1[0].toDouble ();
//       double y1 = x1y1[1].toDouble ();
//       double x2 = x2y2[0].toDouble ();
//       double y2 = x2y2[1].toDouble ();
//       double xMu = x2 - x1;
//       double yMu = y2 - y1;
//       int subordinantAxisLength;


//       // determine the dominate direction for the mapcanvas
//       if (mapCanvas->width () > mapCanvas->height ())
//      {
//        subordinantAxisLength = mapCanvas->height ();
//        scaleFactor = yMu / subordinantAxisLength;
//        mapWindow = new QRect (x1, y1, xMu, xMu);
//      }
//       else
//      {
//        subordinantAxisLength = mapCanvas->width ();
//        scaleFactor = xMu / subordinantAxisLength;
//        mapWindow = new QRect (x1, y1, yMu, yMu);
//      }

//       const char *xtent = (const char *) extent;
//       string sql = "select asbinary(the_geom,";
//       if (isNDR)
//      sql += "'NDR'";
//       else
//      sql += "'XDR'";
//       sql += ") as features from ";
//       sql += *it++;
//       cout << sql.c_str () << endl;
//       pgc.Declare (sql.c_str (), true);
//       int res = pgc.Fetch ();
//       cout << "Number of binary records: " << pgc.Tuples () << endl;
//       bool setExtent = true;
//       // process each record
//       QPainter paint;

//       paint.begin (mapCanvas);
//       paint.setWindow (*mapWindow);
//       QRect v = paint.viewport ();
//       int d = QMIN (v.width (), v.height ());
//       paint.setViewport (v.left () + (v.width () - d) / 2,
//                       v.top () + (v.height () - d) / 2, d, d);


//       paint.setPen (Qt::red);

//       for (int idx = 0; idx < pgc.Tuples (); idx++)
//      {
//        cout << "Size of this record: " << pgc.GetLength (idx, 0) << endl;
//        // allocate memory for the item
//        char *feature = new char[pgc.GetLength (idx, 0) + 1];
//        memset (feature, '\0', pgc.GetLength (idx, 0) + 1);
//        memcpy (feature, pgc.GetValue (idx, 0), pgc.GetLength (idx, 0));


//        cout << "Endian is: " << (int) feature[0] << endl;
//        cout << "Geometry type is: " << (int) feature[1] << endl;
//        // print the x,y coordinates
//        double *x = (double *) (feature + 5);
//        double *y = (double *) (feature + 5 + sizeof (double));
//        cout << "x,y: " << setprecision (16) << *x << ", " << *y << endl;
//        QPoint pt = paint.xForm (QPoint ((int) *x, (int) *y));
//        cout << "Plotting " << *x << ", " << *y << " at " << pt.
//          x () << ", " << pt.y () << endl;
//        paint.drawRect ((int) *x, mapWindow->bottom () - (int) *y, 15000,
//                        15000);
//        // free it 
//        delete[]feature;
//      }
//       paint.end ();
//     }

//}


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
 /*    QgsShapeFileLayer *sfl = new QgsShapeFileLayer("foo");
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
	if (lvi)
	{
	    lyr = ((QgsLegendItem *) lvi)->layer();
	}
	else 
	{
	    // get the selected item
	    QListViewItem *li = legendView->currentItem();
	    lyr = ((QgsLegendItem *) li)->layer();
	}
	
  
  
  QString currentName = lyr->name();
        //test if we have a raster or vector layer and show the appropriate dialog
        if (lyr->type()==QgsMapLayer::RASTER)
        {
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
        }else{
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
  
  
  
//	lyr->showLayerProperties();
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
	if (lvi){
    // get the context menu from the layer and display it 
    QgsMapLayer *lyr = ((QgsLegendItem *) lvi)->layer();
    QPopupMenu *popMenu = lyr->contextMenu();
    if(popMenu){
      popMenu->exec(pt);
    }
  }
}

QgisIface * QgisApp::getInterface(){
	return qgisInterface;
}
int QgisApp::getInt(){
	return 99;
}
void QgisApp::actionPluginManager_activated(){
	QgsPluginManager *pm = new QgsPluginManager(this);
	if(pm->exec()){
		// load selected plugins
		std::vector<QgsPluginItem> pi = pm->getSelectedPlugins();
		std::vector<QgsPluginItem>::iterator it = pi.begin();
		while(it != pi.end()){
			QgsPluginItem plugin = *it;
			loadPlugin(plugin.name(), plugin.description(), plugin.fullPath());
			it++;
		}

		
	}
	

}
void QgisApp::loadPlugin(QString name, QString description, QString fullPath){
	QLibrary *myLib = new QLibrary(fullPath);
		std::cout << "Library name is " << myLib->library() << std::endl;
		bool loaded = myLib->load();
		if (loaded) {
			std::cout << "Loaded test plugin library" << std::endl;
			std::cout << "Attempting to resolve the classFactory function" << std::endl;
			
      type_t *pType = (type_t *) myLib->resolve("type");
      
			
        switch(pType()){
          case QgisPlugin::UI:
          {
            // UI only -- doesn't use mapcanvas
            create_ui *cf = (create_ui *) myLib->resolve("classFactory");
            if (cf) {
              QgisPlugin *pl = cf(this, qgisInterface);
              if(pl){
               pl->initGui();
              }else{
                 // something went wrong
                QMessageBox::warning(this, tr("Error Loading Plugin"), tr("There was an error loading %1."));
              }
            }else{
              std::cout << "Unable to find the class factory for " << fullPath << std::endl;
            }
          }
          break;
          case QgisPlugin::MAPLAYER:
          {
            // Map layer - requires interaction with the canvas
            create_it *cf = (create_it *) myLib->resolve("classFactory");
            if (cf) {
              QgsMapLayerInterface *pl = cf();
              if(pl){
                // set the main window pointer for the plugin
                pl->setQgisMainWindow(this);
                pl->initGui();
              }else{
                // something went wrong
                QMessageBox::warning(this, tr("Error Loading Plugin"), tr("There was an error loading %1."));
              }
            }else{
              std::cout << "Unable to find the class factory for " << fullPath << std::endl;
            }
          }
          break;
          default:
          // type is unknown
            std::cout << "Plugin " << fullPath << " did not return a valid type and cannot be loaded" << std::endl;
            break;
        }
       
			/* 	}else{
					std::cout << "Unable to find the class factory for " << fullPath << std::endl;
				} */
			
		}else{
			std::cout << "Failed to load " << fullPath << "\n";
		}
}
void QgisApp::testMapLayerPlugins(){
	// map layer plugins live in their own directory (somewhere to be determined)
	QDir mlpDir("../plugins/maplayer", "*.so.1.0.0", QDir::Name | QDir::IgnoreCase, QDir::Files );
	if(mlpDir.count() == 0){
		QMessageBox::information(this,tr("No MapLayer Plugins"), tr("No MapLayer plugins in ../plugins/maplayer"));
	}else{
		for(unsigned i = 0; i < mlpDir.count(); i++){
		std::cout << "Getting information for plugin: " << mlpDir[i] << std::endl;
		std::cout << "Attempting to load the plugin using dlopen\n";
		void *handle = dlopen("../plugins/maplayer/" + mlpDir[i], RTLD_LAZY);
	             if (!handle) {
			std::cout << "Error in dlopen: " <<  dlerror() << std::endl;
		     }else{
			std::cout << "dlopen suceeded" << std::endl;
			dlclose(handle);
		     }
		
		QLibrary *myLib = new QLibrary("../plugins/maplayer/" + mlpDir[i]);
		std::cout << "Library name is " << myLib->library() << std::endl;
		bool loaded = myLib->load();
		if (loaded) {
			std::cout << "Loaded test plugin library" << std::endl;
			std::cout << "Attempting to resolve the classFactory function" << std::endl;
			create_it *cf = (create_it *) myLib->resolve("classFactory");
	
			if (cf) {
				std::cout << "Getting pointer to a MapLayerInterface object from the library\n";
				QgsMapLayerInterface *pl = cf();
				if(pl){
					std::cout << "Instantiated the maplayer test plugin\n";
					// set the main window pointer for the plugin
					pl->setQgisMainWindow(this);
					std::cout << "getInt returned " << pl->getInt() << " from map layer plugin\n";
					// set up the gui
					pl->initGui();
				}else{
					std::cout << "Unable to instantiate the maplayer test plugin\n";
				}
			}
		}else{
			std::cout << "Failed to load " << mlpDir[i] << "\n";
		}
	}
}
}
void QgisApp::testPluginFunctions()
{
	// test maplayer plugins first
	testMapLayerPlugins();
	if(false){
// try to load plugins from the plugin directory and test each one

	QDir pluginDir("../plugins", "*.so*", QDir::Name | QDir::IgnoreCase, QDir::Files | QDir::NoSymLinks);
	//pluginDir.setFilter(QDir::Files || QDir::NoSymLinks);
	//pluginDir.setNameFilter("*.so*");
	if(pluginDir.count() == 0){
		QMessageBox::information(this, tr("No Plugins"), tr("No plugins found in ../plugins. To test plugins, start qgis from the src directory"));
	}else{
		
		for(unsigned i = 0; i < pluginDir.count(); i++){
		std::cout << "Getting information for plugin: " << pluginDir[i] << std::endl;
		QLibrary *myLib = new QLibrary("../plugins/" + pluginDir[i]);//"/home/gsherman/development/qgis/plugins/" + pluginDir[i]);
		std::cout << "Library name is " << myLib->library() << std::endl;
		//QLibrary myLib("../plugins/" + pluginDir[i]);
		std::cout << "Attempting to load " << + "../plugins/" + pluginDir[i] << std::endl;
	/*	void *handle = dlopen("/home/gsherman/development/qgis/plugins/" + pluginDir[i], RTLD_LAZY);
	             if (!handle) {
                      std::cout << "Error in dlopen: " <<  dlerror() << std::endl;
                     
                  }else{
				  	std::cout << "dlopen suceeded" << std::endl;
					dlclose(handle);
					}
				  	
*/
		bool loaded = myLib->load();
		if (loaded) {
			std::cout << "Loaded test plugin library" << std::endl;
			std::cout << "Getting the name of the plugin" << std::endl;
			name_t *pName = (name_t *) myLib->resolve("name");
			if(pName){
        QMessageBox::information(this,tr("Name"), tr("Plugin %1 is named %2").arg(pluginDir[i]).arg(pName()));
			}
			std::cout << "Attempting to resolve the classFactory function" << std::endl;
			create_t *cf = (create_t *) myLib->resolve("classFactory");
	
			if (cf) {
				std::cout << "Getting pointer to a QgisPlugin object from the library\n";
				QgisPlugin *pl = cf(this, qgisInterface);
				std::cout << "Displaying name, version, and description\n";
				std::cout << "Plugin name: " << pl->name() << std::endl;
				std::cout << "Plugin version: " << pl->version() << std::endl;
				std::cout << "Plugin description: " << pl->description() << std::endl;
				QMessageBox::information(this, tr("Plugin Information"), tr("QGis loaded the following plugin:") + 
          tr("Name: %1").arg(pl->name())  + "\n" +tr("Version: %1").arg(pl->version()) + "\n" + tr("Description: %1").arg(pl->description()));
				// unload the plugin (delete it)
				std::cout << "Attempting to resolve the unload function" << std::endl;
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
			std::cout << "Unable to load library" << std::endl;
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
	int dw = d->width();		// returns desktop width
	int dh = d->height();		// returns desktop height
	int w = settings.readNumEntry("/qgis/Geometry/w", 600);
	int h = settings.readNumEntry("/qgis/Geometry/h", 400);
	int x = settings.readNumEntry("/qgis/Geometry/x", (dw - 600) / 2);
	int y = settings.readNumEntry("/qgis/Geometry/y", (dh - 400) / 2);
	setGeometry(x, y, w, h);
}
void QgisApp::checkQgisVersion(){
QApplication::setOverrideCursor(Qt::WaitCursor);
/* QUrlOperator op = new QUrlOperator( "http://mrcc.com/qgis/version.txt" );
connect(op, SIGNAL(data()), SLOT(urlData()));
connect(op, SIGNAL(finished(QNetworkOperation)), SLOT(urlFinished(QNetworkOperation)));

op.get(); */
		socket = new QSocket( this );
        connect( socket, SIGNAL(connected()),
                SLOT(socketConnected()) );
        connect( socket, SIGNAL(connectionClosed()),
                SLOT(socketConnectionClosed()) );
		connect( socket, SIGNAL(readyRead()),
                SLOT(socketReadyRead()) );
		connect( socket, SIGNAL(error(int)),
                SLOT(socketError(int)) );
		socket->connectToHost("mrcc.com", 80);
}
void QgisApp::socketConnected(){
		QTextStream os(socket);
		versionMessage = "";
		// send the qgis version string
       // os << qgisVersion << "\r\n";
	   os << "GET /qgis/version.txt HTTP/1.0\n\n";
		
	
}
void QgisApp::socketConnectionClosed(){
	QApplication::restoreOverrideCursor();
	// strip the header
	QString contentFlag = "#QGIS Version";
	int pos = versionMessage.find(contentFlag);
	if(pos >-1){
		pos += contentFlag.length();
		/* std::cout << versionMessage << "\n ";
		std::cout << "Pos is " << pos <<"\n"; */
		versionMessage = versionMessage.mid(pos);
		QStringList parts = QStringList::split("|",versionMessage);
		// check the version from the  server against our version
		QString versionInfo;
		int currentVersion = parts[0].toInt();
		if(currentVersion > QGis::qgisVersionInt){
		// show version message from server
			versionInfo = tr("There is a new version of QGIS available") + "\n";
		}else{
			if(QGis::qgisVersionInt > currentVersion){
				versionInfo = tr("You are running a development version of QGIS") + "\n";
			}else{
			versionInfo = tr("You are running the current version of QGIS") + "\n";
			}
		}
		if(parts.count() > 1){
			versionInfo += parts[1] + "\n\n" + tr("Would you like more information?");;
			int result = QMessageBox::information(this,tr("QGIS Version Information"), versionInfo, tr("Yes"), tr("No"));
			if(result ==0){
				// show more info
				QgsMessageViewer *mv = new QgsMessageViewer(this);
				mv->setCaption(tr("QGIS - Changes in CVS"));
				mv->setMessage(parts[2]);
				mv->exec();
			}	
		}else{
			QMessageBox::information(this, tr("QGIS Version Information"), versionInfo);
		}
	}else{
		QMessageBox::warning(this, tr("QGIS Version Information"), tr("Unable to get current version information from server"));
	}
}
void QgisApp::socketError(int e){
		QApplication::restoreOverrideCursor();
// get errror type
QString detail;
switch(e){
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
	while(socket->bytesAvailable() > 0){
		char *data = new char[socket->bytesAvailable() +1];
		memset(data, '\0', socket->bytesAvailable() +1);
		socket->readBlock(data, socket->bytesAvailable());
		versionMessage += data;
		delete[] data;
	}
           
    }
    
void QgisApp::helpContents(){
  // open help in user browser
  // find a browser
  // find the installed location of the help files
  // open index.html using browser
  helpViewer = new QgsHelpViewer(this,"helpviewer",false);
  //helpViewer->showContent(appDir +"/share/doc","index.html");
  helpViewer->show();
}
/** Get a pointer to the currently selected map layer */
QgsMapLayer *QgisApp::activeLayer(){
  QListViewItem *lvi = legendView->currentItem();
  QgsMapLayer *lyr =0;
	if (lvi)
	{
	     lyr = ((QgsLegendItem *) lvi)->layer();
	}
  return lyr;
}
QString QgisApp::activeLayerSource(){
  QString source;
   QListViewItem *lvi = legendView->currentItem();
  QgsMapLayer *lyr =0;
	if (lvi)
	{
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
  if(pProvider.length() > 0){
    mapCanvas->freeze();
		QApplication::setOverrideCursor(Qt::WaitCursor);
			// create the layer
      QgsVectorLayer *lyr;
      if(providerKey == "postgres"){
        lyr = new QgsVectorLayer(vectorLayerPath + " table=" + baseName, baseName, "postgres");
      }else{
        if(providerKey == "ogr"){
          lyr = new QgsVectorLayer(vectorLayerPath, baseName, "ogr");
        }
      }
      
      // init the context menu so it can connect to slots in main app
      lyr->initContextMenu(this);
      
			// give it a random color
			QgsSingleSymRenderer* renderer=new QgsSingleSymRenderer();//add single symbol renderer as default
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
int QgisApp::saveDirty(){
   int answer = 0;
   mapCanvas->freeze(true);
   if((projectIsDirty || mapCanvas->isDirty()) && mapCanvas->layerCount() > 0){
    // prompt user to save
    answer =  QMessageBox::information(this, "Save?","Do you want to save the current project?",
    QMessageBox::Yes | QMessageBox::Default,
    QMessageBox::No,
    QMessageBox::Cancel | QMessageBox::Escape);
    if(answer == QMessageBox::Yes){
      fileSave();
    }
  }
  mapCanvas->freeze(false);
  return answer;
}
