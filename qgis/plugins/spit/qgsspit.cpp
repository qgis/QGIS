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
/* $Id$ */

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
				tblShapefiles->setText(row, 4, "public");
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
    
	bool cancelled = false;
  QString connName = cmbConnections->currentText();	
  PGresult *res;
  QString query;
  if (connName.isEmpty()){
    QMessageBox::warning(this, "Import Shapefiles", "You need to specify a Connection first");
  }
	else if(total_features==0){
		QMessageBox::warning(this, "Import Shapefiles", "You need to add shapefiles to the list first");
	}
  else{
    QSettings settings;
    QString key = "/Qgis/connections/" + connName;
    QString connInfo = "host=" + settings.readEntry(key + "/host") + " dbname=" + settings.readEntry(key + "/database") +
	  " user=" + settings.readEntry(key + "/username") + " password=" + settings.readEntry(key + "/password");
    PGconn *pd = PQconnectdb((const char *) connInfo);

    if (PQstatus(pd) != CONNECTION_OK) {     
      QMessageBox::warning(this, "Import Shapefiles", "Connection failed - Check settings and try again");
    }
    else { // if connection is ok
      QProgressDialog * pro = new QProgressDialog("Importing files", "Cancel", total_features, this, "Progress", true);
      pro->setProgress(0);
      pro->setAutoClose(true);
      pro->show();
      qApp->processEvents();
      
      for(int i=0; i<fileList.size() ; i++){
				QString error = "Problem inserting features from file:\n"+tblShapefiles->text(i,0);
        // if a name starts with invalid character
        if(!(tblShapefiles->text(i,3))[0].isLetter()){
          QMessageBox::warning(pro, "Import Shapefiles", "Problem inserting file:\n"+tblShapefiles->text(i,0)+"\nInvalid table name.");
          pro->setProgress(pro->progress()+(tblShapefiles->text(i,2)).toInt());
          continue;
        }
        
        fileList[i]->setTable(tblShapefiles->text(i, 3));
        pro->setLabelText("Importing files\n"+tblShapefiles->text(i,0));
        bool rel_exists1 = false;
        bool rel_exists2 = false;
        query = "SELECT f_table_name FROM geometry_columns WHERE f_table_name=\'"+tblShapefiles->text(i,3)+"\'";
        res = PQexec(pd, (const char *)query);
        rel_exists1 = (PQntuples(res)>0);
        if(PQresultStatus(res)!=PGRES_TUPLES_OK){
					qWarning(PQresultErrorMessage(res));
          QMessageBox::warning(pro, "Import Shapefiles", error);
          pro->setProgress(pro->progress()+(tblShapefiles->text(i,2)).toInt());
          continue;
        }
        else {
          PQclear(res);
        }
        
        query = "SELECT tablename FROM pg_tables WHERE tablename=\'"+tblShapefiles->text(i,3)+"\'";
        res = PQexec(pd, (const char *)query);
        qWarning(query);
        rel_exists2 = (PQntuples(res)>0);
        if(PQresultStatus(res)!=PGRES_TUPLES_OK){
          qWarning(PQresultErrorMessage(res));
          QMessageBox::warning(pro, "Import Shapefiles", error);
          pro->setProgress(pro->progress()+(tblShapefiles->text(i,2)).toInt());
          continue;
        }
        else {
          PQclear(res);
        }
				
				// begin session
        query = "BEGIN";
        res = PQexec(pd, query);
        if(PQresultStatus(res)!=PGRES_COMMAND_OK){
          qWarning(PQresultErrorMessage(res));
					QMessageBox::warning(pro, "Import Shapefiles", error);
          pro->setProgress(pro->progress()+(tblShapefiles->text(i,2)).toInt());
          continue;
        }
        else {
          PQclear(res);
        }
				        
				query = "SET SEARCH_PATH TO \'"+tblShapefiles->text(i,4)+"\'";
        res = PQexec(pd, query);
				qWarning(query);
        if(PQresultStatus(res)!=PGRES_COMMAND_OK){
          qWarning(PQresultErrorMessage(res));
					qWarning(PQresStatus(PQresultStatus(res)));
					QMessageBox::warning(pro, "Import Shapefiles", error);
          pro->setProgress(pro->progress()+(tblShapefiles->text(i,2)).toInt());
          continue;
        }
        else {
          PQclear(res);
        }
				
        QMessageBox *del_confirm;
        if(rel_exists1 || rel_exists2){
          del_confirm = new QMessageBox("Import Shapefiles - Relation Exists",
            "The Shapefile:\n"+tblShapefiles->text(i,0)+"\nwill use ["+
            tblShapefiles->text(i,3)+"] relation for its data,\nwhich already exists and possibly contains data.\n"+
            "To avoid data loss change the \"DB Relation Name\" \nfor this Shapefile in the main dialog file list.\n\n"+
            "Do you want to overwrite the ["+tblShapefiles->text(i,3)+"] relation?",
          QMessageBox::Warning,
          QMessageBox::Yes | QMessageBox::Default,
          QMessageBox::No  | QMessageBox::Escape,
          QMessageBox::NoButton, this, "Relation Exists");

					if(del_confirm->exec() == QMessageBox::Yes){
         		if (rel_exists2){
            	query = "DROP TABLE " + tblShapefiles->text(i,3);            
            	qWarning(query);
            	res = PQexec(pd, (const char *)query);
            	if(PQresultStatus(res)!=PGRES_COMMAND_OK){
              	qWarning(PQresultErrorMessage(res));
          			QMessageBox::warning(pro, "Import Shapefiles", error);
          			pro->setProgress(pro->progress()+(tblShapefiles->text(i,2)).toInt());
          			continue;								
            	}
            	else {
              	PQclear(res);
            	}            
						}

          	if(rel_exists1){
            	/*query = "SELECT DropGeometryColumn(\'"+QString(settings.readEntry(key + "/database"))+"\', \'"+
              	fileList[i]->getTable()+"\', \'"+txtGeomName->text()+"')";*/
							query = "DELETE FROM geometry_columns WHERE f_table_schema=\'"+tblShapefiles->text(i,4)+"\' AND "+
								"f_table_name=\'"+tblShapefiles->text(i,3)+"\'";
            	qWarning(query);
            	res = PQexec(pd, (const char *)query);
            	if(PQresultStatus(res)!=PGRES_COMMAND_OK){
              	qWarning(PQresultErrorMessage(res));
          			QMessageBox::warning(pro, "Import Shapefiles", error);
          			pro->setProgress(pro->progress()+(tblShapefiles->text(i,2)).toInt());
          			continue;
            	}
            	else {
              	PQclear(res);
            	}
						}
          }
          else {
            query = "ROLLBACK";
      	    res = PQexec(pd, query);
            if(PQresultStatus(res)!=PGRES_COMMAND_OK){
              qWarning(PQresultErrorMessage(res));
          		QMessageBox::warning(pro, "Import Shapefiles", error);
            }
            else {
              PQclear(res);
            }
						pro->setProgress(pro->progress()+(tblShapefiles->text(i,2)).toInt());
						continue;
          }
        }
	
				// importing file here
				int temp_progress = pro->progress();
				cancelled = false;
        if(fileList[i]->insertLayer(settings.readEntry(key + "/database"), txtGeomName->text(),
          QString("%1").arg(spinSrid->value()), pd, pro, cancelled) && !cancelled)
				{ // if file has been imported successfully
          query = "COMMIT";
          res = PQexec(pd, query);
          if(PQresultStatus(res)!=PGRES_COMMAND_OK){					
            qWarning(PQresultErrorMessage(res));
          	QMessageBox::warning(pro, "Import Shapefiles", error);
          	continue;
          }
          else {
            PQclear(res);
          }
					
          // remove file
          for(int j=0; j<tblShapefiles->numRows(); j++){
            if(tblShapefiles->text(j,0)==QString(fileList[i]->getName())){
              tblShapefiles->selectRow(j);
              removeFile();
              i--;
              break;
            }
          }
					
        }
				else if(!cancelled){ // if problem importing file occured
					pro->setProgress(temp_progress+(tblShapefiles->text(i,2)).toInt());
          QMessageBox::warning(this, "Import Shapefiles", error);
          query = "ROLLBACK";
          res = PQexec(pd, query);
          if(PQresultStatus(res)!=PGRES_COMMAND_OK){
            qWarning(PQresultErrorMessage(res));
          	QMessageBox::warning(pro, "Import Shapefiles", error);
          }
          else {
          	PQclear(res);
          }
      	}
				else{ // if import was actually cancelled
					delete pro;
					break;
				}

      }
    }
    PQfinish(pd);
  }
}
