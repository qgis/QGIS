#include <qpopupmenu.h>
#include <qmessagebox.h>
#include "../qgisplugin.h"
#include "maplayertest.h"
// xpm for creating the toolbar icon
#include "matrix1.xpm"
MapLayerTest::MapLayerTest(){
}
void MapLayerTest::setQgisMainWindow(QMainWindow *app){
	qgisApp = app;
}
// set the coordinate transform for drawing the layer
void MapLayerTest::setCoordinateTransform(QgsCoordinateTransform *xform){
	coordTransform = xform;
}
void MapLayerTest::initGui(){
	// setup the menu
	 QPopupMenu *mapLayerPluginMenu = new QPopupMenu( qgisApp );

        mapLayerPluginMenu->insertItem("&Add Foobar Layer", this, SLOT(open()));
        mapLayerPluginMenu->insertItem(  "&Unload Foobar Plugin", this, SLOT(unload()));
	// create the menubar
	 menu = ((QMainWindow *)qgisApp)->menuBar();

        menuId = menu->insertItem( "&PluginMenu", mapLayerPluginMenu );
	//QAction *zoomPreviousAction = new QAction( "Zoom Previous",QIconSet(icon_matrix), "&Zoom Previous", CTRL+Key_S, qgisMainWindow, "zoomFull" );
		
     //   connect( zoomPreviousAction, SIGNAL( activated() ) , this, SLOT( zoomPrevious() ) );

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
