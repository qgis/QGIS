#include <iostream>
#include <qstring.h>
#include <qmenubar.h>
#include "qgisinterface.h"
#include "qgisapp.h"
#include "qgsmaplayer.h"

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

void QgisIface::addVectorLayer(QString vectorLayerPath, QString baseName, QString providerKey){
  qgis->addVectorLayer(vectorLayerPath, baseName, providerKey);
}

QgsMapLayer * QgisIface::activeLayer(){
  return qgis->activeLayer();
}

QString QgisIface::activeLayerSource(){
  return qgis->activeLayerSource();
}

int QgisIface::addMenu(QString menuText, QPopupMenu *menu){
  QMenuBar *mainMenu = qgis->menuBar();
  // get the index of the help menu 
  #ifdef DEBUG
  std::cout << "Menu item count is : " << mainMenu->count() << std::endl;
  #endif
  return mainMenu->insertItem(menuText, menu,-1, mainMenu->count() -1);
}
