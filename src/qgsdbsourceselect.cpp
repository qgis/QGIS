#include <libpq++.h>
#include <iostream>
#include <qsettings.h>
#include <qlistbox.h>
#include <qstringlist.h>
#include <qcombobox.h>
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

void QgsDbSourceSelect::dbConnect(){
  // populate the table list
  QSettings settings;

  QString key = "/Qgis/connections/" + cmbConnections->currentText();
  QString host = "host="+settings.readEntry(key+"/host");
  QString database = "dbname="+settings.readEntry(key+"/database");
  QString username = "user="+settings.readEntry(key+"/username");
  QString password = "password="+settings.readEntry(key+"/password");
  QString conninfo = host +" " + database + " " + username + " " + password;
  PgDatabase *pd = new PgDatabase((const char *)conninfo);
  cout << pd->ErrorMessage();
  if(pd->Status()==CONNECTION_OK){
    qDebug("Connection succeeded");
    // get the list of tables
    QString sql =  "select f_table_name from geometry_columns where f_table_schema ='"
      + settings.readEntry(key+"/database") + "'";
    qDebug("Fetching tables using: " + sql);
    int result = pd->ExecTuplesOk((const char *)sql);
    if(result){
      QString msg;
      QTextOStream(&msg) << "Fetched " << pd->Tuples() << " tables from database";
      qDebug( msg);
      for(int idx = 0; idx < pd->Tuples(); idx++){
	QString v = pd->GetValue(idx,"f_table_name");
	lstTables->insertItem(v);
      }
    }else{
      qDebug( "Unable to get list of spatially enabled tables from geometry_columns table");
      qDebug( pd->ErrorMessage());
    }
  }else{
    qDebug( "Connection failed");
  }
}
