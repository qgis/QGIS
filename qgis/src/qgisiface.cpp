#include <iostream>
#include "qgisinterface.h"
#include "qgisapp.h"

QgisIface::QgisIface(QgisApp *_qgis, const char * name) : qgis(_qgis) { 

	
}
QgisIface::~QgisIface(){
}

 void QgisIface::zoomFull2(){
	qgis->zoomFull();
	}
 void QgisIface::zoomPrevious(){
	qgis->zoomPrevious();
	}

