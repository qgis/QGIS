#include <qstring.h>
#include "qgsdatabaselayer.h"

QgsDatabaseLayer::QgsDatabaseLayer(const char *conninfo, QString table) :
 QgsMapLayer(QgsMapLayer::DATABASE, table), tableName(table){
  dataSource = conninfo;
}
QgsDatabaseLayer::~QgsDatabaseLayer(){
}
