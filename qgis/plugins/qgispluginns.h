#ifndef QGISPLUGINNS_H
#define QGISPLUGINNS_H
/*!  \mainpage Quantum GIS
*
*  \section about  About QGis
* QGis aims to be an easy to use desktop GIS tool. Initial focus is on viewing spatial
* and tabular data from common data stores, including Shapefiles and PostGIS.
*
* This API documentation provides information about all classes that make up QGis.
*
*/
//! Element type corresponding to one of the values in the ELEMENTS enum
typedef int QGIS_GUI_TYPE;

namespace QGisPlugin
{
	//! Element types that can be added to the interface
	enum UI_ELEMENTS {
		MENU,
		MENU_ITEM,
		TOOLBAR,
		TOOLBAR_BUTTON,
	};
}
#endif //QGISPLUGINNS_H
