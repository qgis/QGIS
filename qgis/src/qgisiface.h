#ifndef QGISIFACE_H
#define QGISIFACE_H
#include "qgisinterface.h"
class QgisIface : public QgisInterface{
	public:
	QgisIface(QgisApp *qgis=0, const char *name=0);
	~QgisIface();
	void zoomFull2();
	void zoomPrevious();
private:
QgisApp *qgis;
};


#endif //#define QGISIFACE_H
