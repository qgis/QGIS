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
#include "../../src/qgisapp.h"

#include "exampleplugin.h" 
#include <qaction.h>
// xpm for creating the toolbar icon
#include "matrix1.xpm"

/**
* Constructor for the plugin
*/
ExamplePlugin::ExamplePlugin(QgisApp *qgis, QgisIface *_qI) 
: qgisMainWindow(qgis), qI(_qI){
	pName = "Test Plugin";
	pVersion = "Version 0.0";
	pDescription = "This test plugin does nothing but tell you its name, version, and description";
	// instantiate a map layer
	//QgsMapLayer *mlyr = new QgsMapLayer();
	
	// see if we can popup a message box in qgis on load
	QMessageBox::information(qgisMainWindow,"Message From Plugin", "This message is from within the test plugin");
	
		// call a function defined in the QgisIface class
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
ExamplePlugin::~ExamplePlugin(){
	
}
QString ExamplePlugin::name(){
	return pName;
}
QString ExamplePlugin::version(){
	return pVersion;
	
}
QString ExamplePlugin::description(){
	return pDescription;
	
}
int ExamplePlugin::type(){
  return QgisPlugin::UI;
}

void ExamplePlugin::initGui(){
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
}
void ExamplePlugin::open(){
	QMessageBox::information(qgisMainWindow, "Message from plugin", "You chose the open menu");
}
void ExamplePlugin::newThing(){
	QMessageBox::information(qgisMainWindow, "Message from plugin", "You chose the new menu");
}

void ExamplePlugin::zoomPrevious(){
	qI->zoomPrevious();
}

extern "C" QgisPlugin * classFactory(QgisApp *qgis, QgisIface *qI){
	return new ExamplePlugin(qgis, qI);
}
extern "C" QString name(){
	return QString("Test Plugin");
}
extern "C" QString description(){
	return QString("Default QGIS Test Plugin");
}
extern "C" int type(){
  return QgisPlugin::UI;
}
extern "C" void unload(QgisPlugin *p){
	delete p;
}

