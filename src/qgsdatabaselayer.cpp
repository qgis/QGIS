#include <qstring.h>
#include <qrect.h>
#include "qgsdatabaselayer.h"

QgsDatabaseLayer::QgsDatabaseLayer(const char *conninfo, QString table) :
 QgsMapLayer(QgsMapLayer::DATABASE, table), tableName(table){
  dataSource = conninfo;
  // set the extent of the layer
  layerExtent.setTop(0);
}
QgsDatabaseLayer::~QgsDatabaseLayer(){
}
void QgsDatabaseLayer::calculateExtent(){
}
