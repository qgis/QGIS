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
#include <qsplitter.h>
#include <qlayout.h>
#include <qwmatrix.h>
#include <qfiledialog.h>
#include "qgsmapcanvas.h"
#include "qgsdbsourceselect.h"
#include "qgisapp.h"

QgisApp::QgisApp(QWidget *parent, const char * name, WFlags fl ) : QgisAppBase(parent, name, fl ){
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
  QStringList list = qApp->libraryPaths();
  QString msg;
  QStringList::Iterator it = list.begin();
  while( it != list.end() ) {
    msg += "\n" + *it;
    ++it;
  }
   
  //  QMessageBox::information(this,"Search Paths",msg);

  // show the postgis dialog
  QgsDbSourceSelect *dbs = new QgsDbSourceSelect();
  if(dbs->exec()){
    // add files to the map canvas
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

