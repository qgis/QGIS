/* Test plugin for QGis
* This code is a test plugin for QGis and a demonstration of the API
* All QGis plugins must inherit from the abstract base class QgisPlugin. A
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
#include "qgistestplugin.h" 
#include <qaction.h>
QgisTestPlugin::QgisTestPlugin(QWidget *qgis) : qgisMainWindow(qgis){
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
		 QAction *fileSaveAction = new QAction( "Save File","&Save", CTRL+Key_S, this, "save" );
        connect( fileSaveAction, SIGNAL( activated() ) , this, SLOT( save() ) );
		
		QToolBar * fileTools = new QToolBar( (QMainWindow *)qgisMainWindow, "file operations" );
        fileTools->setLabel( "File Operations" );
		fileSaveAction->addTo(fileTools);
       

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

void QgisTestPlugin::save(){
	QMessageBox::information(qgisMainWindow, "Message from plugin", "You chose the save toolbar function");
}

extern "C" QgisPlugin * classFactory(QWidget *qgis){
	return new QgisTestPlugin(qgis);
}

extern "C" void unload(QgisPlugin *p){
	delete p;
}
