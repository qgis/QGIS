#ifndef QGISPLUGINMENU_H
#define QGISPLUGINMENU_H

#include <map>
#include "qgispluginguielement"
/*! \class QgisPluginMenu
* \brief Class to define a plugin menu
*
* 
* 
*/
class QgisPluginMenu : public QgisPluginGuiElement {
public:
//! Constructor
	QgisPluginMenu();
	//! Type of element (see ELEMENTS enum in qgisplugin.h)
	QGIS_GUI_TYPE type();
	//! destructor
	virtual ~QgisPluginMenu();
private:
	//! Map to define slot called when a menu item is activated
	std::map<QString menuItemName, QString menuItemSlot> itemSlots; 
};

#endif QGISPLUGINMENU_H

