#include <qstring.h>
#include "qgsrect.h"
#include <libpq++.h>
#include <qmessagebox.h>
#include "qgsdatabaselayer.h"

QgsDatabaseLayer::QgsDatabaseLayer(const char *conninfo, QString table) :
  QgsMapLayer(QgsMapLayer::DATABASE, table),tableName(table){
  // create the database layer and get the needed information
  // about it from the database
  dataSource = conninfo;
  PgDatabase *pd = new PgDatabase(conninfo);
  if(pd->Status()==CONNECTION_OK){
    // get the geometry column
    QString sql = "select f_geometry_column from geometry_columns where f_table_name='"
      + tableName + "'";
    qWarning("Getting geometry column: " + sql);
    int result = pd->ExecTuplesOk((const char *) sql);
    if(result){
      geometryColumn =  pd->GetValue(0,"f_geometry_column");
      // set the extent of the layer
      QString sql =  "select xmax(extent(" + geometryColumn + ")) as xmax,"
	"xmin(extent(" + geometryColumn + ")) as xmin,"
	"ymax(extent(" + geometryColumn + ")) as ymax,"
	"ymin(extent(" + geometryColumn + ")) as ymin"
	" from " + tableName;
      qWarning("Getting extents: " + sql);
      result = pd->ExecTuplesOk((const char *)sql);
     			  
      if(result){
	QString vRight = pd->GetValue(0,"right");
	layerExtent.setXmax(QString(pd->GetValue(0,"xmax")).toDouble());
	layerExtent.setXmin(QString(pd->GetValue(0,"xmin")).toDouble());
	layerExtent.setYmax(QString(pd->GetValue(0,"ymax")).toDouble());
	layerExtent.setYmin(QString(pd->GetValue(0,"ymin")).toDouble());
	QString xMsg;
	QTextOStream(&xMsg).precision(18);
	QTextOStream(&xMsg).width(18);
	QTextOStream(&xMsg) << "Set extents to: " << layerExtent.xMin() << ", " <<
	  layerExtent.yMin() << " " << layerExtent.xMax() << ", " << layerExtent.yMax();
	qWarning(xMsg);
       
      }else{
	QString msg = "Unable to access " + tableName;
	//QMessageBox::warning(this,"Connection Problem",msg); 
	valid = false;
      }
    }else{
      QString msg =  "Unable to get geometry information for " + tableName;
      //QMessageBox::warning(this,"Connection Problem",msg); 
      valid = false;
    }
   
    delete pd;
  }
}
QgsDatabaseLayer::~QgsDatabaseLayer(){
}
QgsRect QgsDatabaseLayer::calculateExtent(){
  return layerExtent;
}
void QgsDatabaseLayer::draw(QPainter *p, QgsRect *viewExtent){
  // painter is active (begin has been called
  /* Steps to draw the layer
     1. get the features in the view extent by SQL query
     2. read WKB for a feature
     3. transform
     4. draw
  */
  PgCursor pgs(dataSource, "drawCursor");
  QString sql = "select asbinary(" + geometryColumn + ",'" + endianString() +
    "') as features from " + tableName + " where " + geometryColumn + 
    " && GeometryFromText('BOX3D(" + viewExtent->xMin() + " " + viewExtent->yMin() 
    + "," + viewExtent->xMax() + " " + viewExtent->yMax() + ")'::box3d,-1)";
  pgs.Declare((const char *)sql, true);
  int res = pgs.Fetch();
  cout << "Number of matching records: " << pgs.Tuples() << endl;
  for (int idx = 0; idx < pgs.Tuples (); idx++)
    {
      // read each feature based on its type
    }



}
int QgsDatabaseLayer::endian(){
  char *chkEndian = new char[4];
  memset (chkEndian, '\0', 4);
  chkEndian[0] = 0xE8;
  int *ce = (int *) chkEndian;
  if(232 == *ce)
    return NDR;
  else
    return XDR;
}
QString QgsDatabaseLayer::endianString(){
  char *chkEndian = new char[4];
  memset (chkEndian, '\0', 4);
  chkEndian[0] = 0xE8;
  int *ce = (int *) chkEndian;
  if(232 == *ce)
    return QString("NDR");
  else
    return QString("XDR");
}
