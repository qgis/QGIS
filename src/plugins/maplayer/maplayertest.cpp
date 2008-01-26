/***************************************************************************
     maplayertest.cpp
     --------------------------------------
    Date                 : Sun Sep 16 12:10:17 AKDT 2007
    Copyright            : (C) 2007 by Gary E. Sherman
    Email                : sherman at mrcc dot com
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/
#include <q3popupmenu.h>
#include <qmessagebox.h>
#include "../qgisplugin.h"
#include "maplayertest.h"
// xpm for creating the toolbar icon
#include "matrix1.xpm"
MapLayerTest::MapLayerTest(){
}
void MapLayerTest::setQgisMainWindow(Q3MainWindow *app){
	qgisApp = app;
}
// set the coordinate transform for drawing the layer
void MapLayerTest::setCoordinateTransform(QgsMapToPixel *xform){
	coordTransform = xform;
}
void MapLayerTest::initGui(){
	// setup the menu
	 Q3PopupMenu *mapLayerPluginMenu = new Q3PopupMenu( qgisApp );

        mapLayerPluginMenu->insertItem("&Add Foobar Layer", this, SLOT(open()));
        mapLayerPluginMenu->insertItem(  "&Unload Foobar Plugin", this, SLOT(unload()));
	// create the menubar
	 menu = ((Q3MainWindow *)qgisApp)->menuBar();

        menuId = menu->insertItem( "&PluginMenu", mapLayerPluginMenu );
	//QAction *zoomPreviousAction = new QAction( "Zoom Previous",QIconSet(icon_matrix), "&Zoom Previous", CTRL+Key_S, qgisMainWindow, "zoomFull" );
		
     //   connect( zoomPreviousAction, SIGNAL( triggered() ) , this, SLOT( zoomPrevious() ) );

}
void MapLayerTest::unload(){
	// remove the GUI
	menu->removeItem(menuId);
	// cleanup anything else that needs to be nuked
}
void MapLayerTest::open(){
	// try and open a layer dialog
	QMessageBox::information(qgisApp,"Plugin Message","You clicked the Add Foobar Layer menu item");
}
void MapLayerTest::draw(){
}
extern "C" QgsMapLayerInterface * classFactory(){
	return new MapLayerTest();
}
extern "C" QString name(){
	return QString("Map Layer test plugin");
}
extern "C" QString description(){
	return QString("Map Layer test plugin using QgsMapLayerInterface interface");
}
// Return the type (either UI or MapLayer plugin)
extern "C" int type()
{
    return QgisPlugin::MAPLAYER;
}
