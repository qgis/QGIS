/***************************************************************************
                          qgslegend.h  -  description
                             -------------------
    begin                : Sun Jul 28 2002
    copyright            : (C) 2002 by Gary E.Sherman
    email                : sherman at mrcc dot com
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
/* $Id */

#ifndef QGSLEGEND_H
#define QGSLEGEND_H
#include <qwidget.h>
class QgsMapCanvas;
class QgsMapLayer;
class QListView;

class QPainter;
/**
* \class QgsLegend
* \brief Map legend
*
* The map legend is a worker class that controls the display of legend items.
  *@author Gary E.Sherman
  */

class QgsLegend:public QWidget
{
  Q_OBJECT public:
/*! Constructor.
* @param lv ListView control containing legend items
* @param parent Parent widget
* @param name Name of the widget
*/
	  QgsLegend(QListView * lv, QWidget * parent = 0, const char *name = 0);
	//! Destructor
	 ~QgsLegend();
	//! Set the pointer to the map canvas
	void setMapCanvas(QgsMapCanvas * canvas);
	//! Update the legend
	void update();
	QString currentLayerName();

	QgsMapLayer *currentLayer();

  private:
	  QListView * listView;
	QgsMapCanvas *map;
};

#endif
