/* Test plugin for QGis
* This code is an example plugin for QGIS and a demonstration of the API
* All QGIS plugins must inherit from the abstract base class QgisPlugin. A
* plugin must implement the virtual functions defined in QgisPlugin:
* 	*name
*	*version
*	*description
*
* This list may grow as the API is expanded.
* 
* In addition, a plugin must implement a the classFactory and unload
* functions. Note that these functions must be declared as extern "C"
*/

// includes
#include <iostream>
#include "../src/qgisapp.h"
#include "qgistestplugin.h" 
#include <qaction.h>
// xpm for creating the toolbar icon
#include "matrix1.xpm"

/**
* Constructor for the plugin
*/
QgisTestPlugin::QgisTestPlugin(QgisApp *qgis, QgisIface *_qI) 
: qgisMainWindow(qgis), qI(_qI){
	pName = "Test Plugin";
	pVersion = "Version 0.0";
	pDescription = "This test plugin does nothing but tell you its name, version, and description";

	// see if we can popup a message box in qgis on load
	QMessageBox::information(qgisMainWindow,"Message From Plugin", "This message is from within the test plugin");
	// add a test menu
	    QPopupMenu *pluginMenu = new QPopupMenu( qgisMainWindow );

        pluginMenu->insertItem("&Open", this, SLOT(open()));
        pluginMenu->insertItem(  "&New" , this, SLOT(newThing()));
	// a test toolbar
        QMenuBar *menu = ((QMainWindow *)qgisMainWindow)->menuBar();

        menu->insertItem( "&PluginMenu", pluginMenu );
		QAction *zoomPreviousAction = new QAction( "Zoom Previous",QIconSet(icon_matrix), "&Zoom Previous", CTRL+Key_S, qgisMainWindow, "zoomFull" );
		
        connect( zoomPreviousAction, SIGNAL( activated() ) , this, SLOT( zoomPrevious() ) );
		
		QToolBar * fileTools = new QToolBar( (QMainWindow *)qgisMainWindow, "zoom operations" );
        fileTools->setLabel( "Zoom Operations" );
		zoomPreviousAction->addTo(fileTools);
		
		int foo = qI->getInt();
		/*
		QgisIface *qI2 = qgisMainWindow->getInterface();
		if(qI2)
			std::cout << "qI2 pointer is good" << std::endl;
		else
			std::cout << "qI2 pointer is bad" << std::endl;
		*/
		//zoomFullX();
       qI->zoomFull();
	  // qgisMainWindow->zoomFull();
	  	QMessageBox::information(qgisMainWindow,"Message From Plugin", "Click Ok to zoom previous");
	
	   qI->zoomPrevious();
	   
	   std::cout << "Result of getInt is: " << foo << std::endl;

}
QgisTestPlugin::~QgisTestPlugin(){
	
}
QString QgisTestPlugin::name(){
	return pName;
}
QString QgisTestPlugin::version(){
	return pVersion;
	
}
QString QgisTestPlugin::description(){
	return pDescription;
	
}

void QgisTestPlugin::open(){
	QMessageBox::information(qgisMainWindow, "Message from plugin", "You chose the open menu");
}
void QgisTestPlugin::newThing(){
	QMessageBox::information(qgisMainWindow, "Message from plugin", "You chose the new menu");
}

void QgisTestPlugin::zoomPrevious(){
	qI->zoomPrevious();
}

extern "C" QgisPlugin * classFactory(QgisApp *qgis, QgisIface *qI){
	return new QgisTestPlugin(qgis, qI);
}

extern "C" void unload(QgisPlugin *p){
	delete p;
}
