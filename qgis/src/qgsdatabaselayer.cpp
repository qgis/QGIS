#include <qstring.h>
#include <qrect.h>
#include <libpq++.h>
#include <qmessagebox.h>
#include "qgsdatabaselayer.h"

QgsDatabaseLayer::QgsDatabaseLayer(const char *conninfo, QString table) :
  QgsMapLayer(QgsMapLayer::DATABASE, table), tableName(table){
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
  void QgsDatabaseLayer::calculateExtent(){
  }
  void QgsDatabaseLayer::draw(QPainter *p, QRect *viewExtent){
    // painter is active (begin has been called
    /* Steps to draw the layer
       1. get the features in the view extent by SQL query
       2. read WKB for a feature
       3. transform
       4. draw
    */

  }
