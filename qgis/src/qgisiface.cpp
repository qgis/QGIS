#include <iostream>
#include "qgisinterface.h"
#include "qgisapp.h"

QgisIface::QgisIface(QgisApp *_qgis, const char * name) : qgis(_qgis) {


}
QgisIface::~QgisIface(){
}

void QgisIface::zoomFull(){
	qgis->zoomFull();
}
void QgisIface::zoomPrevious(){
	qgis->zoomPrevious();
}
void QgisIface::zoomActiveLayer(){
	qgis->zoomToLayerExtent();
}
int QgisIface::getInt(){
	return qgis->getInt();
}
