/* Example plugin for QGIS
* This code is an example plugin for QGIS and a demonstration of the API
* All QGIS plugins must inherit from the abstract base class QgisPlugin. A
* plugin must implement the virtual functions defined in QgisPlugin:
* 	*name
*	  *version
*	  *description
*   *type
*
* In addition, a plugin must implement a the classFactory and unload
* functions. Note that these functions must be declared as extern "C"
*
* This plugin is not very useful. It installs a new menu with two items and
* illustrates how to connect the items to slots which handle menu events. It
* also installs a toolbar with one button. When clicked, the button zooms the
* map to the previous extent.
*/

// includes
#include <iostream>
#include "../../src/qgisapp.h"
#include <qtoolbar.h>
#include <qmenubar.h>
#include <qmessagebox.h>
#include <qpopupmenu.h>
#include <qaction.h>
#include "exampleplugin.h"
// xpm for creating the toolbar icon
#include "matrix1.xpm"
// qgis includes
#include "../../src/qgsmaplayer.h"
/**
* Constructor for the plugin. The plugin is passed a pointer to the main app
* and an interface object that provides access to exposed functions in QGIS.
* @param qgis Pointer to the QGIS main window
* @parma _qI Pointer to the QGIS interface object
*/
ExamplePlugin::ExamplePlugin(QgisApp * qgis, QgisIface * _qI):qgisMainWindow(qgis), qI(_qI)
{
  /** Initialize the plugin and set the required attributes */
    pName = "Example Plugin";
    pVersion = "Version 0.0";
    pDescription = "This example plugin installs menu items and a toolbar";

    // see if we can popup a message box in qgis on load
    QMessageBox::information(qgisMainWindow, "Message From Plugin", 
      "This message is from within the example plugin.");

    // Zoom the map canvas to the full extent of all layers
    qI->zoomFull();
    
    QMessageBox::information(qgisMainWindow, "Message From Plugin", "Click Ok to zoom previous");
    // zoom the map back to previous extent
    qI->zoomPrevious();
  
    // call a function defined in the QgisIface class and send its value to stdout
    QgsMapLayer * myMapLayer = qI->activeLayer();
    std::cout << "Current map layer is: " << myMapLayer->name() << std::endl;

}

ExamplePlugin::~ExamplePlugin()
{

}
/* Following functions return name, description, version, and type for the plugin */
QString ExamplePlugin::name()
{
    return pName;
}

QString ExamplePlugin::version()
{
    return pVersion;

}

QString ExamplePlugin::description()
{
    return pDescription;

}

int ExamplePlugin::type()
{
    return QgisPlugin::UI;
}

/*
* Initialize the GUI interface for the plugin 
*/
void ExamplePlugin::initGui()
{
    // add a test menu with 3 items
    QPopupMenu *pluginMenu = new QPopupMenu(qgisMainWindow);

    pluginMenu->insertItem("&Open", this, SLOT(open()));
    pluginMenu->insertItem("&New", this, SLOT(newThing()));
    pluginMenu->insertItem("&Unload Example Plugin", this, SLOT(unload()));
   
    menu = ((QMainWindow *) qgisMainWindow)->menuBar();

    menuId = menu->insertItem("&ExamplePluginMenu", pluginMenu);
    
    /* Add a test toolbar with one tool (a zoom previous tool) */
    // Create the action for tool
    QAction *zoomPreviousAction = new QAction("Zoom Previous", QIconSet(icon_matrix), "&Zoom Previous",
                                              CTRL + Key_S, qgisMainWindow, "zoomPrevious");
    // Connect the action to the zoomPrevous slot
    connect(zoomPreviousAction, SIGNAL(activated()), this, SLOT(zoomPrevious()));
    // Add the toolbar
    toolBar = new QToolBar((QMainWindow *) qgisMainWindow, "zoom operations");
    toolBar->setLabel("Zoom Operations");
    // Add the zoom previous tool to the toolbar
    zoomPreviousAction->addTo(toolBar);

}
// Slot called when open is selected on the menu
void ExamplePlugin::open()
{
    QMessageBox::information(qgisMainWindow, "Message from plugin", "You chose the open menu");
}
// Slot called when new is selected on the menu
void ExamplePlugin::newThing()
{
    QMessageBox::information(qgisMainWindow, "Message from plugin", "You chose the new menu");
}
// Slot called when the zoomPrevious button is clicked
void ExamplePlugin::zoomPrevious()
{
    qI->zoomPrevious();
}
// Unload the plugin bye cleaning up the GUI
void ExamplePlugin::unload()
{
    // remove the GUI
    menu->removeItem(menuId);
    // cleanup anything else that needs to be nuked
    delete toolBar;
}
/** 
* Required extern functions needed  for every plugin 
* These functions can be called prior to creating an instance
* of the plugin class
*/
// Class factory to return a new instance of the plugin class
extern "C" QgisPlugin * classFactory(QgisApp * qgis, QgisIface * qI)
{
    return new ExamplePlugin(qgis, qI);
}
// Return the name of the plugin
extern "C" QString name()
{
    return QString("Test Plugin");
}
// Return the description
extern "C" QString description()
{
    return QString("Default QGIS Test Plugin");
}
// Return the type (either UI or MapLayer plugin)
extern "C" int type()
{
    return QgisPlugin::UI;
}
// Delete ourself
extern "C" void unload(QgisPlugin * p)
{

    delete p;
}
