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


#include<qmessagebox.h>
#include <libpq++.h>
#include <iostream>
#include <qsettings.h>
#include <qlistbox.h>
#include <qlistview.h>
#include <qstringlist.h>
#include <qcombobox.h>
#include <qpushbutton.h>
#include <qmessagebox.h>
#include <qspinbox.h>
#include <qcheckbox.h>
#include <qinputdialog.h>
#include <qfiledialog.h>
#include <qprogressdialog.h>
#include "qgsspit.h"
#include "qgsconnectiondialog.h"
#include "qgsmessageviewer.h"

QgsSpit::QgsSpit(QWidget *parent, const char *name) : QgsSpitBase(parent, name){
  populateConnectionList();
  default_value = -1;
  setFixedSize(QSize(579, 504));
}

QgsSpit::~QgsSpit(){}

void QgsSpit::populateConnectionList(){
	QSettings settings;
	QStringList keys = settings.subkeyList("/Qgis/connections");
	QStringList::Iterator it = keys.begin();
	cmbConnections->clear();
	while (it != keys.end()) {
		cmbConnections->insertItem(*it);
		++it;
	}
  if(cmbConnections->count()==0) changeEditAndRemove(0);
}

void QgsSpit::newConnection()
{  
	QgsConnectionDialog *con = new QgsConnectionDialog(this, "New Connection");

	if (con->exec()) {
		populateConnectionList();
	}
  if(cmbConnections->count()!=0) changeEditAndRemove(1);
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
    if(cmbConnections->count()==0) changeEditAndRemove(0);
	}
}

void QgsSpit::addFile()
{
  QListViewItemIterator n(lstShapefiles);
  QString error = "";
  bool exist;
  bool is_error = false;

  QStringList files = QFileDialog::getOpenFileNames(
    "Shapefiles (*.shp);; All Files (*)", "", this, "add file dialog", "Add Shapefiles" );
  
  for ( QStringList::Iterator it = files.begin(); it != files.end(); ++it ){
    exist = false;
    if(lstShapefiles->findItem(*it, 0)!=0)
      exist = true;
    if(!exist){
      QgsShapeFile * file = new QgsShapeFile(*it);
      if(file->is_valid()){
        fileList.push_back(file);
        QListViewItem *lvi = new QListViewItem(lstShapefiles, *it);
        lvi->setText(1, file->getFeatureClass());
        lvi->setText(2, file->getFeatureCount());
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
}

void QgsSpit::removeFile()
{
  QListViewItemIterator it(lstShapefiles);
  while(it.current())
    if ( it.current()->isSelected() ){
      delete it.current();
      it = lstShapefiles->firstChild();
    }
    else ++it;
}
void QgsSpit::removeAllFiles(){
  lstShapefiles->selectAll(true);
  removeFile();
}
void QgsSpit::useDefault(){
  if(chkUseDefault->isChecked()) {
    default_value = spinSrid->value();
    spinSrid->setValue(-1);
    spinSrid->setEnabled(false);
  }
  else {
    spinSrid->setEnabled(true);
    spinSrid->setValue(default_value);
  }
}

void QgsSpit::changeEditAndRemove(int mode){
  if(mode==0){
    btnEdit->setEnabled(false);
    btnRemove->setEnabled(false);
  }
  else if(mode==1){
    btnEdit->setEnabled(true);
    btnRemove->setEnabled(true);
  }
}

void QgsSpit::helpInfo(){
  QString message = "General Interface Help:\n\n";
  QgsMessageViewer * e = new QgsMessageViewer(this, "HelpMessage");
  e->setMessage(message);
  e->exec();
}

void QgsSpit::import(){
  QProgressDialog * pro = new QProgressDialog("Importing files", "Cancel", 100, this, "Progress");
  pro->setAutoClose(true);
  pro->exec();
}
