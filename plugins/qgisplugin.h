/*!  \mainpage Quantum GIS - Plugin API
*
*  \section about  About QGis Plugins
* Plugins provide additional functionality to QGis. Plugins must
* implement several required methods in order to be registered with
* QGis. These methods include:
* <ul>name
* <li>version
* <li>description
* </ul>
*
* All QGis plugins must inherit from the abstract base class QgisPlugin. A
* This list will grow as the API is expanded.
* 
* In addition, a plugin must implement the classFactory and unload
* functions. Note that these functions must be declared as extern "C" in
* order to be resolved properly and prevent C++ name mangling.
*/

#ifndef qgisplugin_h
#define qgisplugin_h
#include <qstring.h>
#include <qwidget.h>
#include <qmainwindow.h>
#include "../src/qgisapp.h"

//#include "qgisplugingui.h"

/*! \class QgisPlugin 
* \brief Abstract base class from which all plugins must inherit
*
*/
class QgisPlugin {
public:
//! Get the name of the plugin
	virtual QString name() = 0;
	//! Version of the plugin
	virtual QString version() =0;
	//! A brief description of the plugin
	virtual QString description() = 0;
	//! Interface to gui element collection object
	//virtual QgisPluginGui *gui()=0;
	//! Element types that can be added to the interface
	/* enum ELEMENTS {
		MENU,
		MENU_ITEM,
		TOOLBAR,
		TOOLBAR_BUTTON,
	};
	*/
}; 

// Typedefs

//! Typedef for function that returns a generic pointer to a plugin object
typedef QgisPlugin* create_t(QgisApp *, QgisInterface *);
//! Typedef for the function to unload a plugin and free its resources
typedef void unload_t(QgisPlugin *);

#endif //qgisplugin_h
