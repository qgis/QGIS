/***************************************************************************
                          qgisapp.h  -  description
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

#ifndef QGISAPP_H
#define QGISAPP_H
class QCanvas;
class QCanvasView;
#include "qgisappbase.h"
/**
  *@author Gary E.Sherman
  */
class QgsMapCanvas;

class QgisApp : public QgisAppBase  {
public: 
	QgisApp(QWidget *parent=0, const char * name=0, WFlags fl = WType_TopLevel );
	
	~QgisApp();
	//public slots:
	void addLayer();
	void fileExit();
 	void zoomOut();
  	void zoomIn();
private:
QCanvasView *cv;
QCanvas *canvas;
QgsMapCanvas *mapCanvas;
QWidget *mapToc;

};

#endif
