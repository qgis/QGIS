#ifndef QGISPLUGINGUI_H
#define QGISPLUGINGUI_H

#include <vector>
class QgisPluginGuiElement;

/*! \class QgisPluginGui
* \brief Class to encapsulate the gui elements of a plugin
*
* QgsPluginGui encapsulates all the GUI elements a plugin supports,
* including menu items, toolbar buttons, and associated graphics
*/
class QgisPluginGui {
public:
//! Constructor
	QgisPluginGui();
	//! Returns the number of GUI elements in the plugin
	int elementCount();
	//! Returns a specific GUI element by index from the vector
	QgisPluginGuiElement element(int index);
	//! Adds a new element
	void addElement(QgisPluginGuiElement);
	//! Destructor
	virtual ~QgisPluginGui();
private:
	std::vector<QgisPluginGuiElement> elements;
};

#endif // QGISPLUGINGUI_H
