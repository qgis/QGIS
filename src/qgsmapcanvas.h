/***************************************************************************
                          qgsmapcanvas.h  -  description
                             -------------------
    begin                : Sun Jun 30 2002
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

#ifndef QGSMAPCANVAS_H
#define QGSMAPCANVAS_H
#include <map>
#include <qwidget.h>

class QgsMapLayer;

/**Map canvas class for displaying all GIS data types
  *@author Gary E.Sherman
  */

class QgsMapCanvas : public QWidget  {
   Q_OBJECT
public: 
	QgsMapCanvas(QWidget *parent=0, const char *name=0);
	~QgsMapCanvas();
	void addLayer(QgsMapLayer *lyr);
	void render();
private:
//! map containing the layers by name
	map<QString,QgsMapLayer> layers;
};

#endif
