#include <libpq++.h>
#include <iostream>
#include <qsettings.h>
#include <qpixmap.h>
#include <qlistbox.h>
#include <qstringlist.h>
#include <qcombobox.h>
#include "qgisicons.h"
#include "qgsdbsourceselect.h"
#include "qgsnewconnection.h"

QgsDbSourceSelect::QgsDbSourceSelect():QgsDbSourceSelectBase(){
  QSettings settings;
  QStringList keys = settings.subkeyList("/Qgis/connections");
  QStringList::Iterator it = keys.begin();
  while( it != keys.end() ) {
    cmbConnections->insertItem(*it);

    ++it;
  }

}
QgsDbSourceSelect::~QgsDbSourceSelect(){
}
void QgsDbSourceSelect::addNewConnection(){
 
  QgsNewConnection *nc = new QgsNewConnection();
  
  if(nc->exec()){
  }
}
void QgsDbSourceSelect::editConnection(){
 
  QgsNewConnection *nc = new QgsNewConnection();
  // populate the fields with the stored setting parameters
  QSettings settings;

  QString key = "/Qgis/connections/" + cmbConnections->currentText();
  QString host = settings.readEntry(key+"/host");
  QString database = settings.readEntry(key+"/database");
  QString username = settings.readEntry(key+"/username");
  QString password = settings.readEntry(key+"/password");
  if(nc->exec()){
  }
}
void QgsDbSourceSelect::addTables(){
  //store the table info
  for(int idx=0; idx <lstTables->numRows(); idx++){
    if(lstTables->isSelected(idx))
      m_selectedTables += lstTables->text(idx);
  }
  accept();
}
void QgsDbSourceSelect::dbConnect(){
  // populate the table list
  QSettings settings;

  QString key = "/Qgis/connections/" + cmbConnections->currentText();
  QString host = "host="+settings.readEntry(key+"/host");
  QString database = "dbname="+settings.readEntry(key+"/database");
  QString username = "user="+settings.readEntry(key+"/username");
  QString password = "password="+settings.readEntry(key+"/password");
  m_connInfo = host +" " + database + " " + username + " " + password;
  qDebug(m_connInfo);
  PgDatabase *pd = new PgDatabase((const char *)m_connInfo);
  cout << pd->ErrorMessage();
  if(pd->Status()==CONNECTION_OK){
    // create the pixmaps for the layer types
    QPixmap pxPoint;
    pxPoint = QPixmap(layer_points);
    QPixmap pxLine;
    pxLine = QPixmap(layer_lines);
    QPixmap pxPoly;
    pxPoly = QPixmap(layer_polygon);
    qDebug("Connection succeeded");
    // get the list of tables
    QString sql =  "select * from geometry_columns where f_table_schema ='"
      + settings.readEntry(key+"/database") + "'";
    qDebug("Fetching tables using: " + sql);
    int result = pd->ExecTuplesOk((const char *)sql);
    if(result){
      QString msg;
      QTextOStream(&msg) << "Fetched " << pd->Tuples() << " tables from database";
      qDebug( msg);
      for(int idx = 0; idx < pd->Tuples(); idx++){
	QString v = pd->GetValue(idx,"f_table_name");
	QString type = pd->GetValue(idx,"type");
	QPixmap *p;
	if(type == "POINT" || type == "MULTIPOINT")
	  p = &pxPoint;
	else
	  if(type == "MULTIPOLYGON" || type == "POLYGON")
	    p = &pxPoly;
	  else
	    if(type == "LINESTRING" || type == "MULTILINESTRING")
	      p = &pxLine;
	    else
	      p = 0;
	lstTables->insertItem(*p,v);
      }
    }else{
      qDebug( "Unable to get list of spatially enabled tables from geometry_columns table");
      qDebug( pd->ErrorMessage());
    }
  }else{
    qDebug( "Connection failed");
  }
}
QStringList QgsDbSourceSelect::selectedTables(){
  return m_selectedTables;
}
QString  QgsDbSourceSelect::connInfo(){
  return m_connInfo;
}
