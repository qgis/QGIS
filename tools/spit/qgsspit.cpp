/***************************************************************************
                          qgsspit.cpp  -  description
                             -------------------
    begin                : Fri Dec 19 2003
    copyright            : (C) 2003 by Denis Antipov
    email                :
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/


#include <qsettings.h>
#include <qlistbox.h>
#include <qtable.h>
#include <qstringlist.h>
#include <qmessagebox.h>
#include <qcombobox.h>
#include <qpushbutton.h>
#include <qspinbox.h>
#include <qcheckbox.h>
#include <qinputdialog.h>
#include <qfiledialog.h>
#include <qprogressdialog.h>
#include <qmemarray.h>
#include <qapplication.h>
#include <iostream>
extern "C"
{
  #include <libpq-fe.h>
}

#include "qgsspit.h"
#include "qgsconnectiondialog.h"
#include "qgsmessageviewer.h"

// Qt implementation of alignment() + changed the numeric types to be shown on the left as well
int QTableItem::alignment() const
{
    bool num;
    bool ok1 = FALSE, ok2 = FALSE;
    (void)txt.toInt( &ok1 );
    if ( !ok1 )
        (void)txt.toDouble( &ok2 );
    num = ok1 || ok2;
    
    return ( num ? AlignLeft : AlignLeft ) | AlignVCenter;
}

QgsSpit::QgsSpit(QWidget *parent, const char *name) : QgsSpitBase(parent, name){
  populateConnectionList();
  defSrid = -1;
  defGeom = "the_geom";
  total_features = 0;
  setFixedSize(QSize(579, 504));
  
  tblShapefiles->verticalHeader()->hide();
  tblShapefiles->adjustColumn(3);
  tblShapefiles->setLeftMargin(0);

  tblShapefiles->setColumnReadOnly(0, true);
  tblShapefiles->setColumnReadOnly(1, true);
  tblShapefiles->setColumnReadOnly(2, true);  

  chkUseDefaultSrid->setChecked(true);
  chkUseDefaultGeom->setChecked(true);
  useDefaultSrid();
  useDefaultGeom();
  
}

QgsSpit::~QgsSpit(){  
}

void QgsSpit::populateConnectionList(){
	QSettings settings;
	QStringList keys = settings.subkeyList("/Qgis/connections");
	QStringList::Iterator it = keys.begin();
	cmbConnections->clear();
	while (it != keys.end()) {
		cmbConnections->insertItem(*it);
		++it;
	}
}

void QgsSpit::newConnection()
{  
	QgsConnectionDialog *con = new QgsConnectionDialog(this, "New Connection");

	if (con->exec()) {
		populateConnectionList();
	}
}

void QgsSpit::editConnection()
{
	QgsConnectionDialog *con = new QgsConnectionDialog(this, cmbConnections->currentText());
	if (con->exec()) {
		con->saveConnection();
	}
}

void QgsSpit::removeConnection()
{
  QSettings settings;
	QString key = "/Qgis/connections/" + cmbConnections->currentText();
	QString msg = "Are you sure you want to remove the [" + cmbConnections->currentText() + "] connection and all associated settings?";
	int result = QMessageBox::information(this, "Confirm Delete", msg, "Yes", "No");
	if(result == 0){
		settings.removeEntry(key + "/host");
		settings.removeEntry(key + "/database");
		settings.removeEntry(key + "/username");
		settings.removeEntry(key + "/password");
		settings.removeEntry(key + "/save");

		cmbConnections->removeItem(cmbConnections->currentItem());
	}
}

void QgsSpit::addFile()
{
  QString error = "";
  bool exist;
  bool is_error = false;

  QStringList files = QFileDialog::getOpenFileNames(
    "Shapefiles (*.shp);; All Files (*)", "", this, "add file dialog", "Add Shapefiles" );
  
  for ( QStringList::Iterator it = files.begin(); it != files.end(); ++it){
    exist = false;
    for(int n=0; n<tblShapefiles->numRows(); n++)
      if(tblShapefiles->text(n,0)==*it){
        exist = true;
        break;
      }
    if(!exist){
      QgsShapeFile * file = new QgsShapeFile(*it);
      if(file->is_valid()){
        int row = tblShapefiles->numRows();
        fileList.push_back(file);
        tblShapefiles->insertRows(row);
        tblShapefiles->setText(row, 0, *it);
        tblShapefiles->setText(row, 1, file->getFeatureClass());
        tblShapefiles->setText(row, 2, QString("%1").arg(file->getFeatureCount()));        
        tblShapefiles->setText(row, 3, file->getTable());
        total_features += file->getFeatureCount();
      }
      else{
        error += *it + "\n";
        is_error = true;
      }
    }
  }
  
  if(is_error){
    error = "The following Shapefile(s) could not be loaded:\n\n" + error;
    QgsMessageViewer * e = new QgsMessageViewer(this, "error");
    e->setMessage(error);
    e->exec();
  }
  
  tblShapefiles->adjustColumn(0);
  tblShapefiles->adjustColumn(1);
  tblShapefiles->adjustColumn(2);
  tblShapefiles->adjustColumn(3);
  tblShapefiles->setCurrentCell(-1, 0);  
}

void QgsSpit::removeFile()
{
  std::vector <int> temp;
  for(int n=0; n<tblShapefiles->numRows(); n++)
    if (tblShapefiles->isRowSelected(n)){
      for(std::vector<QgsShapeFile *>::iterator vit = fileList.begin(); vit!=fileList.end(); vit++){
        if((*vit)->getName()==tblShapefiles->text(n,0)){
          total_features -= (*vit)->getFeatureCount();
          fileList.erase(vit);
          break;
        }
      }  
      temp.push_back(n);
    }
  QMemArray<int> array(temp.size());
  for(int i=0; i<temp.size(); i++)
    array[i] = temp[i];
  tblShapefiles->removeRows(array);
  tblShapefiles->setCurrentCell(-1, 0);
}

void QgsSpit::removeAllFiles(){
  QMemArray<int> array(tblShapefiles->numRows());
  for(int n=0; n<tblShapefiles->numRows(); n++)
    array[n] = n;

  fileList.clear();
  total_features = 0;
  tblShapefiles->removeRows(array);
}

void QgsSpit::useDefaultSrid(){
  if(chkUseDefaultSrid->isChecked()) {
    defaultSridValue = spinSrid->value();
    spinSrid->setValue(defSrid);
    spinSrid->setEnabled(false);
  }
  else {
    spinSrid->setEnabled(true);
    spinSrid->setValue(defaultSridValue);
  }
}

void QgsSpit::useDefaultGeom(){
  if(chkUseDefaultGeom->isChecked()) {
    defaultGeomValue = txtGeomName->text();
    txtGeomName->setText(defGeom);
    txtGeomName->setEnabled(false);
  }
  else {
    txtGeomName->setEnabled(true);
    txtGeomName->setText(defaultGeomValue);
  }
}

void QgsSpit::helpInfo(){
  QString message = "General Interface Help:\n\n";
  QgsMessageViewer * e = new QgsMessageViewer(this, "HelpMessage");
  e->setMessage(message);
  e->exec();
}

void QgsSpit::import(){
  tblShapefiles->setCurrentCell(-1, 0);
    
  QString connName = cmbConnections->currentText();
  bool finished = false;
  PGresult *res;
  if (!connName.isEmpty()) {
    QSettings settings;
    QString key = "/Qgis/connections/" + connName;
    QString connInfo = "host=" + settings.readEntry(key + "/host") + " dbname=" + settings.readEntry(key + "/database") +
	  " user=" + settings.readEntry(key + "/username") + " password=" + settings.readEntry(key + "/password");
    PGconn *pd = PQconnectdb((const char *) connInfo);

  	if (PQstatus(pd) == CONNECTION_OK) {     
      QProgressDialog * pro = new QProgressDialog("Importing files", "Cancel", total_features, this, "Progress", true);
      pro->setProgress(0);
      pro->setAutoClose(true);
      qApp->processEvents();
      //pro->setAutoReset(true);
      
      for(int i=0; i<fileList.size() ; i++){
        // if a name starts with invalid character
        if(!(fileList[i]->getTable()[0]).isLetter()){
          QMessageBox::warning(pro, "Import Shapefiles",
          "Problem inserting file:\n"+fileList[i]->getName()+"\nInvalid table name.");
          std::cout<<i<<std::endl;
          continue;
        }
        PQexec(pd, "BEGIN");
        
        fileList[i]->setTable(tblShapefiles->text(i, 3));
        pro->show();
        pro->setLabelText("Importing files\n"+fileList[i]->getName());
        int rel_exists1 = 0;
        int rel_exists2 = 0;
        QMessageBox *del_confirm;
        QString query = "SELECT f_table_name FROM geometry_columns WHERE f_table_name=\'"+fileList[i]->getTable()+"\'";
        res = PQexec(pd, (const char *)query);
        rel_exists1 = PQntuples(res);
        PQclear(res);
        query = "SELECT relname FROM pg_stat_all_tables WHERE relname=\'"+fileList[i]->getTable()+"\'";
        res = PQexec(pd, (const char *)query);
        rel_exists2 = PQntuples(res);
        PQclear(res);
        
        if(rel_exists1 || rel_exists2){
          del_confirm = new QMessageBox("Import Shapefiles - Relation Exists",
            "The Shapefile:\n"+fileList[i]->getName()+"\nwill use ["+
            QString(fileList[i]->getTable())+"] relation for its data,\nwhich already exists and possibly contains data.\n"+
            "To avoid data loss change the \"DB Relation Name\" \nfor this Shapefile in the main dialog file list.\n\n"+
            "Do you want to overwrite the ["+fileList[i]->getTable()+"] relation?",
          QMessageBox::Warning,
          QMessageBox::Yes | QMessageBox::Default,
          QMessageBox::No  | QMessageBox::Escape,
          QMessageBox::NoButton, pro, "Relation Exists");
        }
        if ((!rel_exists1 && !rel_exists2) || del_confirm->exec() == QMessageBox::Yes){
          if(rel_exists1){
            query = "SELECT DropGeometryColumn(\'"+QString(settings.readEntry(key + "/database"))+"\', \'"+
              fileList[i]->getTable()+"\', \'"+txtGeomName->text()+"')";
            res = PQexec(pd, (const char *)query);
            PQclear(res);
          }
          if(rel_exists2){
            query = "DROP TABLE " + fileList[i]->getTable();
            res = PQexec(pd, (const char *)query);
            PQclear(res);
          }
          
          if(!fileList[i]->insertLayer(settings.readEntry(key + "/database"), txtGeomName->text(),
            QString("%1").arg(spinSrid->value()), pd, pro, finished)){
            if(!finished){
              QMessageBox::warning(pro, "Import Shapefiles",
                "Problem inserting features from file:\n"+fileList[i]->getName());
              PQexec(pd, "ROLLBACK");
            }
          }
          else{  // if file has been imported, remove it from the list
            PQexec(pd, "COMMIT");
            for(int j=0; j<tblShapefiles->numRows(); j++)
              if(tblShapefiles->text(j,0)==QString(fileList[i]->getName())){
                tblShapefiles->selectRow(j);
                removeFile();
                i--;
                break;
              }
          }
        }
        else{
          pro->setProgress(pro->progress()+fileList[i]->getFeatureCount());
        }        
      }
    }
    else 
      QMessageBox::warning(this, "Import Shapefiles", "Connection failed - Check settings and try again");
  
    PQfinish(pd);
  }
  else
    QMessageBox::warning(this, "Import Shapefiles", "You need to specify a Connection first");
}
