#ifndef QGISEXAMPLEPLUGIN_H
#define QGISEXAMPLEPLUGIN_H
#include "../qgisplugin.h"
#include <qwidget.h>
#include <qmainwindow.h>

class QMessageBox;
class QToolBar;
class QMenuBar;
class QPopupMenu;
//#include "qgsworkerclass.h"
#include "../../src/qgisapp.h"

/**
* \class ExamplePlugin
* \brief Example plugin for QGIS
*
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
* This plugin is not very useful. When loaded, it installs a new menu with two 
* items and  illustrates how to connect the items to slots which handle menu events. 
* It also installs a toolbar with one button. When clicked, the button zooms the
* map to the previous extent.
* 
* After the UI elements are initialized the plugin zooms the map canvas to the
* full extent of all layers.
*/
class ExamplePlugin:public QObject, public QgisPlugin
{
Q_OBJECT public:
/** 
* Constructor for a plugin. The QgisApp and QgisIface pointers are passed by 
* QGIS when it attempts to instantiate the plugin.
* @param qgis Pointer to the QgisApp object
* @param qI Pointer to the QgisIface object. 
*/
    ExamplePlugin(QgisApp * qgis, QgisIface * qI);
  //! Destructor
    virtual ~ ExamplePlugin();
  public slots:
  //! init the gui
  virtual void initGui();
//! open something
  void open();
  //! create something new
  void newThing();
  //! zoom the map to the previous extent
  void zoomPrevious();
  //! unload the plugin
  void unload();
private:
  int ptype;
  //! Id of the plugin's menu. Used for unloading
  int menuId;
  //! Pointer to our toolbar
  QToolBar *toolBar;
  //! Pointer to our menu
  QMenuBar *menu;
  //! Pionter to QGIS main application object
  QgisApp *qgisMainWindow;
  //! Pointer to the QGIS interface object
  QgisIface *qI;
};

#endif
