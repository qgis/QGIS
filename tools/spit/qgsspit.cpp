
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
#include "qgsspit.h"
#include "qgsconnectiondialog.h"

QgsSpit::QgsSpit(QWidget *parent, const char *name) : QgsSpitBase(parent, name){
  populateConnectionList();
  default_value = -1;
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
  
  setMinimumSize(QSize(579, 504));
  setMaximumSize(QSize(579, 504));
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
  bool exist;
  QStringList files = QFileDialog::getOpenFileNames(
    "Shapefiles (*.shp);; All Files (*)", "", this, "add file dialog", "Add Shapefiles" );
    
  for ( QStringList::Iterator it = files.begin(); it != files.end(); ++it ){
    exist = false;
    for(;n.current();++n)
        if(n.current()->text(0) == *it){
          exist = true;
          break;
        }
    if(!exist){
      QgsShapeFile * file = new QgsShapeFile(*it);
      QListViewItem *lvi = new QListViewItem(lstShapefiles, *it);
      lvi->setText(1, file->getFeatureClass());
      lvi->setText(2, file->getFeatureCount());
    }
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
