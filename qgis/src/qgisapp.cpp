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
#include <qlayout.h>
#include <qwmatrix.h>
#include <qfiledialog.h>
#include <libpq++.h>
#include <iostream>
#include <iomanip>
#include "qgsmapcanvas.h"
#include "qgsdbsourceselect.h"
#include "qgisapp.h"
#include "qgisicons.h"

QgisApp::QgisApp(QWidget *parent, const char * name, WFlags fl ) : QgisAppBase(parent, name, fl ){
  QPixmap icon;
  icon = QPixmap(appicon_xpm);
  setIcon(icon);

  //add a canvas
  canvas = new QCanvas(1024,768);
  // resize it to fit in the frame
  //	QRect r = frmCanvas->rect();
  //	canvas->resize(r.width(), r.height());
  canvas->setBackgroundColor(QColor(220,235,255));





  QGridLayout *FrameLayout = new QGridLayout( frameMain, 1, 2, 4, 6, "mainFrameLayout");
  QSplitter *split = new QSplitter(frameMain);
  mapToc = new QWidget(split);//frameMain);
  cv = new QCanvasView(canvas,split);
 
  FrameLayout->addWidget( split, 0, 0 );
  mapToc->setBackgroundColor(QColor(192,192,192));
  canvas->update();
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
    QStringList::Iterator it = tables.begin();
    while( it != tables.end() ) {
    
    
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
  QWMatrix m = cv->worldMatrix();
  m.scale( 2.0, 2.0 );
  cv->setWorldMatrix( m );
}

void QgisApp::zoomOut()
{
  QWMatrix m = cv->worldMatrix();
  m.scale( 0.5, 0.5 );
  cv->setWorldMatrix( m );
}
void QgisApp::readWKB(const char *connInfo){
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
  string sql = "select asbinary(the_geom,";
  if(isNDR)
    sql += "'NDR'";
  else
    sql += "'XDR'";
  sql += ") as features from towns";
  cout << sql.c_str() << endl;
  pgc.Declare(sql.c_str(), true);
  int res = pgc.Fetch();
  cout << "Number of binary records: " << pgc.Tuples() << endl;
 
  // process each record
  for(int idx = 0; idx < pgc.Tuples(); idx++){
    cout << "Size of this record: " << pgc.GetLength(idx,0) << endl;
    // allocate memory for the item
    char *feature = new char[pgc.GetLength(idx,0) +1]; 
    memset(feature,'\0',pgc.GetLength(idx,0) +1);
    memcpy(feature,pgc.GetValue(idx,0),pgc.GetLength(idx,0) );
    char endian;
    endian = *feature;
    int *geotype;
    geotype =(int *) (feature + 1);
   

    cout << "Endian is: " << (int)feature[0] << endl;
    cout << "Geometry type is: " << (int)feature[1] << endl;
    // print the x,y coordinates
    double *x = (double *)(feature+5);
    double *y = (double*)(feature+5 + sizeof(double));
    cout << "x,y: " << setprecision(16) << *x << ", " << *y << endl;
    // free it 
    delete[] feature;
  }
}



