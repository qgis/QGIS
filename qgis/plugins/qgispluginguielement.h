#ifndef QGISPLUGINGUIELEMENT_H
#define QGISPLUGINGUIELEMENT_H
#include "qgispluginns.h"
/*! \class QgisPluginGuiElement
* \brief Base class for a GUI element (menu, toolbar, etc) of a plugin
*
* QgsPluginGuiElement provides information about a GUI element that
* will be added to the QGis interface when the plugin is loaded
*/
class QgisPluginGuiElement {
public:
//! Constructor
	QgisPluginGuiElement();
	//! Type of element (see ELEMENTS enum in qgisplugin.h)
	virtual QGIS_GUI_TYPE elementType();
	//! destructor
	virtual ~QgisPluginGuiElement();
private:
	
};

#endif // QGISPLUGINGUIELEMENT_H
