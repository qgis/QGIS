/***************************************************************************
                          qgisapp.cpp  -  description
                             -------------------
    begin                : Sat Jun 22 2002
    copyright            : (C) 2002 by Gary E.Sherman
    email                : sherman@mrcc.com
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
#include <qapplication.h>
#include <qcanvas.h>
#include <qcolor.h>
#include <qscrollview.h>
#include <qstringlist.h>
#include <qmessagebox.h>
#include <qstatusbar.h>
#include <qlabel.h>
#include <qfiledialog.h>
#include <qfileinfo.h>
#include <qpixmap.h>
#include <qsplitter.h>
#include <qpopupmenu.h>
#include <qrect.h>
#include <qpoint.h>
#include <qpainter.h>
#include <qlayout.h>
#include <qwmatrix.h>
#include <qfiledialog.h>
#include <qvbox.h>
#include <qlistview.h>
#include <libpq++.h>
#include <iostream>
#include <iomanip>
#include "qgsrect.h"
#include "qgsmapcanvas.h"
#include "qgslegenditem.h"
#include "qgslegend.h"
#include "qgsdbsourceselect.h"
#include "qgsdatabaselayer.h"
#include "qgsshapefilelayer.h"
#include "qgslayerproperties.h"
#include "qgsabout.h"
#include "qgis.h"
#include "qgisapp.h"
#include "xpm/qgis.xpm"
#include <ogrsf_frmts.h>
QgisApp::QgisApp (QWidget * parent, const char *name,
		  WFlags fl):QgisAppBase (parent, name, fl)
{
  OGRRegisterAll ();
  QPixmap icon;
  icon = QPixmap (qgis_xpm);
  setIcon (icon);
  QGridLayout *FrameLayout =
    new QGridLayout (frameMain, 1, 2, 4, 6, "mainFrameLayout");
  QSplitter *split = new QSplitter (frameMain);
  legendView = new QListView(split);
  legendView->addColumn("Layers");
  legendView->setSorting(-1);
 
  
  mapLegend = new QgsLegend(legendView);	//frameMain);
 // mL = new QScrollView(split);
  //add a canvas
  mapCanvas = new QgsMapCanvas (split);
  // resize it to fit in the frame
  //    QRect r = frmCanvas->rect();
  //    canvas->resize(r.width(), r.height());
  mapCanvas->setBackgroundColor (Qt::white);	//QColor (220, 235, 255));
  mapCanvas->setMinimumWidth (400);
  FrameLayout->addWidget (split, 0, 0);
  mapLegend->setBackgroundColor (QColor (192, 192, 192));
  mapLegend->setMapCanvas(mapCanvas);
  legendView->setResizeMode(QListView::AllColumns);
  QString caption = "Quantum GIS - ";
  caption += QGis::qgisVersion; 
  setCaption(caption);
  connect (mapCanvas, SIGNAL (xyCoordinates (QgsPoint &)), this,
	   SLOT (showMouseCoordinate (QgsPoint &)));
	connect (legendView, SIGNAL(doubleClicked(QListViewItem *)), this, SLOT(layerProperties(QListViewItem *)));
  connect (legendView, SIGNAL(rightButtonPressed ( QListViewItem *, const QPoint &, int )),
  				this, SLOT(rightClickLegendMenu(QListViewItem *, const QPoint &, int )));

	// create the layer popup menu
	popMenu = new QPopupMenu();
	popMenu->insertItem("&Remove", this, SLOT(removeLayer()));
	popMenu->insertItem("&Properties", this, SLOT(layerProperties()));

}

QgisApp::~QgisApp ()
{
}
void QgisApp::about(){
  QgsAbout *abt = new QgsAbout();
  QString versionString = "Version ";
  versionString += QGis::qgisVersion;
  abt->setVersion(versionString);
  QString urls = "Web Page: http://qgis.sourceforge.net\nSourceforge Project Page: http://sourceforge.net/projects/qgis";
  abt->setURLs(urls);
  QString watsNew =  "Version ";
  watsNew += QGis::qgisVersion;
  watsNew += "\n* Random color now assigned to a layer when added\n"
  					"  instead of all layers being the same color by type\n"
					  "* Right-click menu to remove a layer or edit properties\n"
						"* Properties page for layer. Allows setting of color and\n"
						"  other options.";

	 abt->setWhatsNew(watsNew);
	 abt->exec();
  
}
void QgisApp::addLayer ()
{
  QStringList files =
    QFileDialog::getOpenFileNames ("Shapefiles (*.shp);;All files (*.*)", 0, this, "open files dialog",
				   "Select one or more layers to add");
  QStringList::Iterator it = files.begin ();
  while (it != files.end ())
    {


      QFileInfo fi (*it);
      QString base = fi.baseName ();


      // create the layer

      QgsShapeFileLayer *lyr = new QgsShapeFileLayer (*it, base);
      // give it a random color

      if(lyr->isValid()){
	      // add it to the mapcanvas collection
	      mapCanvas->addLayer (lyr);
	      }else{
	        QString msg = *it;
	        msg += " is not a valid or recognized data source";
	      	QMessageBox::critical(this,"Invalid Data Source",msg);
	      }

      ++it;
    }
  qApp->processEvents ();
  // update legend
  /*! \todo Need legend scrollview and legenditem classes */
  // draw the map
  mapLegend->update();
  mapCanvas->render2 ();
  statusBar ()->message (mapCanvas->extent ().stringRep ());



}

void QgisApp::addDatabaseLayer ()
{
  // only supports postgis layers at present
  // show the postgis dialog
  QgsDbSourceSelect *dbs = new QgsDbSourceSelect ();
  if (dbs->exec ())
    {
      // add files to the map canvas
      QStringList tables = dbs->selectedTables ();
      QString connInfo = dbs->connInfo ();
      // for each selected table, connect to the datbase, parse the WKT geometry,
      // and build a cavnasitem for it
      // readWKB(connInfo,tables);
      QStringList::Iterator it = tables.begin ();
      while (it != tables.end ())
	{

	  // create the layer
	  QgsDatabaseLayer *lyr = new QgsDatabaseLayer (connInfo, *it);
	  // give it a random color

	  // add it to the mapcanvas collection
	  mapCanvas->addLayer (lyr);

	  ++it;
	}
      qApp->processEvents ();
      // update legend
      /*! \todo Need legend scrollview and legenditem classes */
      mapLegend->update();
      // draw the map
      mapCanvas->render2 ();
      statusBar ()->message (mapCanvas->extent ().stringRep ());

    }

}
void QgisApp::fileExit ()
{
  QApplication::exit ();

}

void QgisApp::zoomIn ()
{
  /*  QWMatrix m = mapCanvas->worldMatrix();
     m.scale( 2.0, 2.0 );
     mapCanvas->setWorldMatrix( m );
   */

  mapTool = QGis::ZoomIn;
  mapCanvas->setMapTool (mapTool);
  // scale the extent
  /* QgsRect ext = mapCanvas->extent();
     ext.scale(0.5);
     mapCanvas->setExtent(ext);
     statusBar()->message(ext.stringRep());
     mapCanvas->clear();
     mapCanvas->render2(); */

}

void QgisApp::zoomOut ()
{
  mapTool = QGis::ZoomOut;
  mapCanvas->setMapTool (mapTool);
  /*    QWMatrix m = mapCanvas->worldMatrix();
     m.scale( 0.5, 0.5 );
     mapCanvas->setWorldMatrix( m );
   */


}

void QgisApp::pan ()
{
  mapTool = QGis::Pan;
  mapCanvas->setMapTool (mapTool);
}

void QgisApp::zoomFull ()
{
  mapCanvas->zoomFullExtent ();
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


void QgisApp::drawPoint (double x, double y)
{
  QPainter paint;
  //  QWMatrix mat (scaleFactor, 0, 0, scaleFactor, 0, 0);
  paint.begin (mapCanvas);
  // paint.setWorldMatrix(mat);
  paint.setWindow (*mapWindow);

  paint.setPen (Qt::blue);
  paint.drawPoint ((int) x, (int) y);
  paint.end ();
}

void QgisApp::drawLayers ()
{
  std::cout << "In  QgisApp::drawLayers()" << std::endl;
  mapCanvas->render2 ();
}

void QgisApp::showMouseCoordinate (QgsPoint & p)
{
  statusBar ()->message (p.stringRep ());
  //qWarning("X,Y is: " + p.stringRep());

}

void QgisApp::testButton ()
{
  QgsShapeFileLayer *sfl = new QgsShapeFileLayer ("foo");
  mapCanvas->addLayer (sfl);
//      delete sfl;

}
void QgisApp::layerProperties(){
	layerProperties(legendView->currentItem());
}
void QgisApp::layerProperties(QListViewItem *lvi){
QgsMapLayer *lyr;
if(lvi)
		lyr = ((QgsLegendItem *)lvi)->layer();
else{
	// get the selected item
		QListViewItem *li =  legendView->currentItem();
		
		lyr = ((QgsLegendItem *)li)->layer();
}
		QgsLayerProperties *lp = new QgsLayerProperties(lyr);
		if(	lp->exec()){
			// apply changes
			mapCanvas->render2();
			}
}
void QgisApp::removeLayer(){
}
void QgisApp::rightClickLegendMenu(QListViewItem *lvi, const QPoint &pt, int i){
if(lvi)
	popMenu->exec(pt);
}

