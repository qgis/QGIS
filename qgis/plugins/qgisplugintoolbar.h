#include <map>
#include "qgispluginguielement"
/*! \class QgisPluginToolbar
* \brief Class to define a plugin toolbar
*
* 
* 
*/
class QgisPluginToolbar : public QgisPluginGuiElement {
public:
//! Constructor
	QgisPluginToolbar();
	//! Type of element (see ELEMENTS enum in qgisplugin.h)
	QGIS_GUI_TYPE type();
	//! destructor
	virtual ~QgisPluginToolbar();
private:
	//! Map to define slot called when a toolbar button is activated
	std::map<QString toolName, QString toolSlot> toolSlots;
};
