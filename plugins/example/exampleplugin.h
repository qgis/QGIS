#ifndef QGISEXAMPLEPLUGIN_H
#define QGISEXAMPLEPLUGIN_H
#include "../qgisplugin.h"
#include <qwidget.h>
#include <qmainwindow.h>
#include <qmessagebox.h>
#include <qpopupmenu.h>
#include <qmenubar.h>
//#include "qgsworkerclass.h"
#include "../../src/qgisapp.h"

/**
* \class ExamplePlugin
* \brief An example QGIS plugin, illustrating how to add menu items, a toolbar, 
* and perform an operation on the map canvas.
*
* When loaded, this plugin adds a new menu to QGIS with two items. It also
* adds a toolbar that has one button. When clicked, the button zooms the 
* map to the previous view.
*
* This class must inherit from QObject and QgisPlugin.
*/
class ExamplePlugin : public QObject, public QgisPlugin{
Q_OBJECT
public:
/** 
* Constructor for a plugin. The QgisApp and QgisIface pointers are passed by 
* QGIS when it attempts to instantiate the plugin.
* @param qgis Pointer to the QgisApp object
* @param qI Pointer to the QgisIface object. 
*/
	ExamplePlugin(QgisApp *qgis, QgisIface *qI);
	/**
	* Virtual function to return the name of the plugin. The name will be used when presenting a list 
	* of installable plugins to the user
	*/
	virtual QString name();
	/**
	* Virtual function to return the version of the plugin. 
	*/
	virtual QString version();
	/**
	* Virtual function to return a description of the plugins functions 
	*/
	virtual QString description();
	/**
  * Return the plugin type
  */
  virtual int type();
  //! init the gui
  virtual void initGui();
  //! Destructor
	virtual ~ExamplePlugin();
public slots:
	void open();
	void newThing();
	void zoomPrevious();
private:
	QString pName;
	QString pVersion;
	QString pDescription;
	int ptype;
	QgisApp *qgisMainWindow;
	QgisIface *qI;
};

#endif
