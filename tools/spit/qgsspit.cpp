
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
#include <qinputdialog.h>
#include <qfiledialog.h>
#include "qgsspit.h"
#include "qgsconnectiondialog.h"

QgsSpit::QgsSpit(QWidget *parent, const char *name) : QgsSpitBase(parent, name){
  populateConnectionList();
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
}

void QgsSpit::newConnection()
{  
	QgsConnectionDialog *con = new QgsConnectionDialog();

	if (con->exec()) {
		populateConnectionList();
	}
}

void QgsSpit::editConnection()
{
	QgsConnectionDialog *con = new QgsConnectionDialog(cmbConnections->currentText());
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
		//if(!success){
		//	QMessageBox::information(this,"Unable to Remove","Unable to remove the connection " + cmbConnections->currentText());
		//}
		cmbConnections->removeItem(cmbConnections->currentItem());// populateConnectionList();
	}
}

void QgsSpit::addFile()
{
  QStringList files = QFileDialog::getOpenFileNames(
    "Shapefiles (*.shp);; All Files (*)", "", this, "add file dialog", "Add Shapefiles" );
  for ( QStringList::Iterator it = files.begin(); it != files.end(); ++it ){
    QCheckListItem *lvi = new QCheckListItem(lstShapefiles, *it ,QCheckListItem::CheckBox);
  	lvi->setText(1, "Polygon");
    lvi->setText(2, "2Mb");
  }
}
void QgsSpit::removeFile()
{
    QListViewItem *temp = lstShapefiles->selectedItem();
    delete temp;
}
