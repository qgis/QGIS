#ifndef QGISINTERFACE_H
#define QGISINTERFACE_H
/*
* $Id$
*/
//#include "qgisapp.h"
#include <qwidget.h>
class QgisApp;
class QgsMapLayer;
class QPopupMenu;

// interface class for plugins
class QgisInterface : public QWidget{
Q_OBJECT
public:
	QgisInterface(QgisApp *qgis=0, const char *name=0);
	virtual ~QgisInterface();
	public slots:
	virtual void zoomFull()=0;
	virtual void zoomPrevious()=0;
	virtual void zoomActiveLayer()=0;
	virtual int getInt() = 0;
   //! Add a vector layer
  virtual void addVectorLayer(QString vectorLayerPath, QString baseName, QString providerKey)=0;
  //! Get pointer to the active layer (layer selected in the legend)
  virtual QgsMapLayer *activeLayer()=0;
  //! add a menu item to the main menu, postioned to the left of the Help menu
  virtual int addMenu(QString menuText, QPopupMenu *menu) =0;
private:
	//QgisApp *qgis;
};

#endif //#ifndef QGISINTERFACE_H

