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


static const char * const ident_ = "$Id$";


static const char *pluginVersion = "0.1";

static const char * const name_ = "Example Plug-in";
static const char * const description_ = "This example plugin installs menu items and a toolbar";
static const char * const version_ = "Version 0.0";
static const QgisPlugin::PLUGINTYPE type_ = QgisPlugin::UI;


/**
* Constructor for the plugin. The plugin is passed a pointer to the main app
* and an interface object that provides access to exposed functions in QGIS.
* @param qgis Pointer to the QGIS main window
* @parma _qI Pointer to the QGIS interface object
*/
ExamplePlugin::ExamplePlugin(QgisApp * qgis, QgisIface * _qI)
    : qgisMainWindow(qgis), qI(_qI), QgisPlugin( name_, description_, version_, type_ )
{
  // see if we can popup a message box in qgis on load
  QMessageBox::information(qgisMainWindow, "Message From Plugin", "This message is from within the example plugin.");

  // Zoom the map canvas to the full extent of all layers
  qI->zoomFull();

  QMessageBox::information(qgisMainWindow, "Message From Plugin", "Click Ok to zoom previous");
  // zoom the map back to previous extent
  qI->zoomPrevious();

  // call a function defined in the QgisIface class and send its value to stdout
  QgsMapLayer *myMapLayer = qI->activeLayer();
  std::cout << "Current map layer is: " << myMapLayer->name() << std::endl;

}

ExamplePlugin::~ExamplePlugin()
{
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

  // Add the icon to the toolbar
  qGisInterface->addToolBarIcon(myQActionPointer);

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
    return name_;
}

// Return the description
extern "C" QString description()
{
    return description_;
}

// Return the type (either UI or MapLayer plugin)
extern "C" int type()
{
    return type_;
}

// Delete ourself
extern "C" void unload(QgisPlugin * p)
{
  delete p;
}
