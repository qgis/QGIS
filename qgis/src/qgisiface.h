#ifndef QGISIFACE_H
#define QGISIFACE_H
#include "qgisinterface.h"

class QgsMapLayer;
/** \class QgisIface
* \brief Interface class to provide access to private methods in QgisApp
* for use by plugins.
* 
* Only those functions "exposed" by QgisIface can be called from within a
* plugin.
*/
class QgisIface : public QgisInterface{
	public:
  /**
  * Constructor.
  * @param qgis Pointer to the QgisApp object
  */
	QgisIface(QgisApp *qgis=0, const char *name=0);
	~QgisIface();
  /* Exposed functions */
  //! Zoom map to full extent
	void zoomFull();
  //! Zoom map to previous extent
	void zoomPrevious();
  //! Zoom to active layer
	void zoomActiveLayer();
  //! Add a vector layer
  void addVectorLayer(QString vectorLayerPath, QString baseName, QString providerKey);
  //! Get pointer to the active layer (layer selected in the legend)
  QgsMapLayer *activeLayer();
  //! Get source of the active layer
  QString activeLayerSource();
  //! Add a menu to the main menu bar of the application, positioned to the left of Help
  int addMenu(QString menuText, QPopupMenu *menu);
  //! Get an integer from the QgisApp object. This is a test function with no real utility
	int getInt();
private:
//! Pointer to the QgisApp object
QgisApp *qgis;
};


#endif //#define QGISIFACE_H
