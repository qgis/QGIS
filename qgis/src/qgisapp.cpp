/***************************************************************************
                          qgisapp.cpp  -  description
                             -------------------
    begin                : Sat Jun 22 2002
    copyright            : (C) 2002 by Gary E.Sherman
    email                : sherman@mrcc.com
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
#include <qpixmap.h>
#include <qsplitter.h>
#include <qrect.h>
#include <qpoint.h>
#include <qpainter.h>
#include <qlayout.h>
#include <qwmatrix.h>
#include <qfiledialog.h>
#include <libpq++.h>
#include <iostream>
#include <iomanip>
#include "qgsmapcanvas.h"
#include "qgsdbsourceselect.h"
#include "qgsdatabaselayer.h"
#include "qgisapp.h"
#include "qgisicons.h"

QgisApp::QgisApp(QWidget *parent, const char * name, WFlags fl ) : QgisAppBase(parent, name, fl ){
  QPixmap icon;
  icon = QPixmap(appicon_xpm);
  setIcon(icon);
  QGridLayout *FrameLayout = new QGridLayout( frameMain, 1, 2, 4, 6, "mainFrameLayout");
  QSplitter *split = new QSplitter(frameMain);
  mapToc = new QWidget(split);//frameMain);
  //add a canvas
  mapCanvas = new QgsMapCanvas(split);
  // resize it to fit in the frame
  //	QRect r = frmCanvas->rect();
  //	canvas->resize(r.width(), r.height());
  mapCanvas->setBackgroundColor(QColor(220,235,255));
  mapCanvas->setMinimumWidth(400);





 
 
 
 
  FrameLayout->addWidget( split, 0, 0 );
  mapToc->setBackgroundColor(QColor(192,192,192));
 
}
QgisApp::~QgisApp(){
}
void QgisApp::addLayer(){
  // show the postgis dialog
  QgsDbSourceSelect *dbs = new QgsDbSourceSelect();
  if(dbs->exec()){
    // add files to the map canvas
    QStringList tables = dbs->selectedTables();
    QString connInfo = dbs->connInfo();
    // for each selected table, connect to the datbase, parse the WKT geometry,
    // and build a cavnasitem for it
    // readWKB(connInfo,tables);
    QStringList::Iterator it = tables.begin();
    while( it != tables.end() ) {
      
      // create the layer
      QgsDatabaseLayer *lyr = new QgsDatabaseLayer(connInfo, *it);
      // add it to the mapcanvas collection
      mapCanvas->addLayer(lyr);
  
      ++it;
    }
    
  }
  // show the file dialog
  /*
    QFileDialog* fd = new QFileDialog( this, "file dialog", TRUE );
    fd->setMode( QFileDialog::ExistingFile );
    fd->setFilter("Shapefiles (*.shp)" );
    if(fd->exec()){
    QStringList files = fd->selectedFiles();
    }
  */
}
void QgisApp::fileExit(){
  QApplication::exit();

}
void QgisApp::zoomIn(){
  /*  QWMatrix m = mapCanvas->worldMatrix();
      m.scale( 2.0, 2.0 );
      mapCanvas->setWorldMatrix( m );
  */
  
}

void QgisApp::zoomOut()
{
  /*    QWMatrix m = mapCanvas->worldMatrix();
      m.scale( 0.5, 0.5 );
      mapCanvas->setWorldMatrix( m );
  */
  

}
void QgisApp::readWKB(const char *connInfo, QStringList tables){
  PgCursor pgc(connInfo, "testcursor");
  // get "endianness"
  char *chkEndian = new char[4];
  memset(chkEndian,'\0',4);
  chkEndian[0] = 0xE8;
  int *ce = (int *)chkEndian;
  bool isNDR = (232 == *ce);
  /*     if(*ce != 232)
	 cout << "Big endian" << endl;
	 else
	 cout << "Little endian" << endl;
  */
   QStringList::Iterator it = tables.begin();
  while( it != tables.end() ) {
  
  // get the extent of the layer
  QString esql = "select extent(the_geom) from " + *it;
  PgDatabase *pd = new PgDatabase(connInfo);
  int result = pd->ExecTuplesOk((const char *)esql);
  QString extent = pd->GetValue(0,0);
  // parse out the x and y values
  extent = extent.right(extent.length() - extent.find("BOX3D(")- 6);
  QStringList coordPairs = QStringList::split(",", extent);
  QStringList x1y1 = QStringList::split(" ", coordPairs[0]);
  QStringList x2y2 = QStringList::split(" ", coordPairs[1]);
  double x1 = x1y1[0].toDouble();
  double y1 = x1y1[1].toDouble();
  double x2 = x2y2[0].toDouble();
  double y2 = x2y2[1].toDouble();
  double xMu = x2 - x1;
  double yMu = y2 - y1;
  int subordinantAxisLength;

 
  // determine the dominate direction for the mapcanvas
  if(mapCanvas->width() > mapCanvas->height()){
    subordinantAxisLength = mapCanvas->height();
    scaleFactor = yMu/subordinantAxisLength;
  mapWindow = new QRect(x1,y1, xMu, xMu);
  } else{
    subordinantAxisLength = mapCanvas->width();
    scaleFactor = xMu/subordinantAxisLength;
  mapWindow = new QRect(x1,y1, yMu, yMu);
  }

  const char * xtent = (const char *)extent;
  string sql = "select asbinary(the_geom,";
  if(isNDR)
    sql += "'NDR'";
  else
    sql += "'XDR'";
  sql += ") as features from ";
  sql += *it++;
  cout << sql.c_str() << endl;
  pgc.Declare(sql.c_str(), true);
  int res = pgc.Fetch();
  cout << "Number of binary records: " << pgc.Tuples() << endl;
  bool setExtent = true;
  // process each record
  QPainter paint;
 
  paint.begin(mapCanvas);
  paint.setWindow(*mapWindow);
  QRect v = paint.viewport();
    int d = QMIN( v.width(), v.height() );
    paint.setViewport( v.left() + (v.width()-d)/2,
    v.top() + (v.height()-d)/2, d, d );


  paint.setPen(Qt::red);
 
  for(int idx = 0; idx < pgc.Tuples(); idx++){
    cout << "Size of this record: " << pgc.GetLength(idx,0) << endl;
    // allocate memory for the item
    char *feature = new char[pgc.GetLength(idx,0) +1]; 
    memset(feature,'\0',pgc.GetLength(idx,0) +1);
    memcpy(feature,pgc.GetValue(idx,0),pgc.GetLength(idx,0) );
  

    cout << "Endian is: " << (int)feature[0] << endl;
    cout << "Geometry type is: " << (int)feature[1] << endl;
    // print the x,y coordinates
    double *x = (double *)(feature+5);
    double *y = (double*)(feature+5 + sizeof(double));
    cout << "x,y: " << setprecision(16) << *x << ", " << *y << endl;
    QPoint pt = paint.xForm(QPoint((int)*x, (int) *y));
    cout << "Plotting " << *x << ", " << *y << " at " << pt.x() << ", " << pt.y() << endl;
    paint.drawRect((int)*x, mapWindow->bottom()-(int)*y,15000,15000);
    // free it 
    delete[] feature;
  }
  paint.end();
  }
}


void QgisApp::drawPoint(double x,double y){
  QPainter paint;
  QWMatrix mat(scaleFactor,0,0,scaleFactor,0,0);
  paint.begin(mapCanvas);
  // paint.setWorldMatrix(mat);
  paint.setWindow(*mapWindow);
 
  paint.setPen(Qt::blue);
  paint.drawPoint(x,y);
  paint.end();
}
