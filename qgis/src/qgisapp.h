/***************************************************************************
                          qgisapp.h  -  description
                             -------------------
    begin                : Sat Jun 22 2002
    copyright            : (C) 2002 by Gary E.Sherman
    email                : sherman at mrcc.com
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
class QRect;
class QCanvasView;
class QStringList;
class QScrollView;
class QgsPoint;
class QgsLegend;
class QVBox;
class QCursor;
class QListView;
class QListViewItem;
class QgsMapLayer;
#include "qgisappbase.h"
#include "qgisiface.h"
class QgsMapCanvas;
/*! \class QgisApp
 * \brief Main window for the Qgis application
 */
class QgisApp:public QgisAppBase
{
  Q_OBJECT public:
//! Constructor
	QgisApp(QWidget * parent = 0, const char *name = 0, WFlags fl = WType_TopLevel);

	 ~QgisApp();
	public:
	 QgisIface *getInterface();
	 int getInt();
	  private:
	 //private:
	//public slots:
	//! Add a layer to the map
	void addLayer();
	#ifdef PGDB
	//! Add a databaselayer to the map
	void addDatabaseLayer();
	#endif
	//! Exit Qgis
	void fileExit();
	
	//! Set map tool to Zoom out
	void zoomOut();
	//! Set map tool to Zoom in
	void zoomIn();
	//! Zoom to full extent
	void zoomFull();
	//! Zoom to the previous extent
	void zoomPrevious();
	//! Set map tool to pan
	void pan();
	//! Identify feature(s) on the currently selected layer
	void identify();
	//! show the attribute table for the currently selected layer
	void attributeTable();
	//! Read Well Known Binary stream from PostGIS
	//void readWKB(const char *, QStringList tables);
	//! Draw a point on the map canvas
	void drawPoint(double x, double y);
	//! draw layers
	void drawLayers();
	//! test function
	void testButton();
	//! About QGis
	void about();

	private slots:
//! Slot to show the map coordinate position of the mouse cursor
	void showMouseCoordinate(QgsPoint &);
	//! Show layer properties for the selected layer
	void layerProperties(QListViewItem *);
	//! Show layer properties for selected layer (called by right-click menu)
	void layerProperties();
	//! Show the right-click menu for the legend
	void rightClickLegendMenu(QListViewItem *, const QPoint &, int);
	//! Remove a layer from the map and legend
	void removeLayer();
	//! zoom to extent of layer
	void zoomToLayerExtent();
	//! test plugin functionality
	void testPluginFunctions();
	//! Save window state
	void saveWindowState();
	//! Restore the window and toolbar state
	void restoreWindowState();
	//! Save project
	void fileSave();
	//! Save project as
	void fileSaveAs();
	//! Open a project
	void fileOpen();
	//! Create a new project
	void fileNew();
  private:
//! Popup menu
	  QPopupMenu * popMenu;
//! Legend list view control
	QListView *legendView;
	//! Map canvas
	QgsMapCanvas *mapCanvas;
//! Table of contents (legend) for the map
	QgsLegend *mapLegend;
	QCursor *mapCursor;
//! scale factor
	double scaleFactor;
	//! Current map window extent in real-world coordinates
	QRect *mapWindow;
	//! Current map tool
	int mapTool;
	QCursor *cursorZoomIn;
	QString startupPath;
	//! full path name of the current map file (if it has been saved or loaded)
	QString fullPath;
	QgisIface *qgisInterface;
	friend class QgisIface;
};

#endif
