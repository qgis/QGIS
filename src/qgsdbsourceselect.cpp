/***************************************************************************
                    qgsdbsourceselect.h  -  description
                             -------------------
    begin                : Sat Jun 22 2002
    copyright            : (C) 2002 by Gary E.Sherman
    email                : sherman at mrcc.com
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/
 /* $Id */
#include <libpq++.h>
#include <iostream>
#include <qsettings.h>
#include <qpixmap.h>
#include <qlistbox.h>
#include <qstringlist.h>
#include <qcombobox.h>
#include <qpushbutton.h>
#include <qmessagebox.h>
#include <qinputdialog.h>
#include "xpm/point_layer.xpm"
#include "xpm/line_layer.xpm"
#include "xpm/polygon_layer.xpm"
#include "qgsdbsourceselect.h"
#include "qgsnewconnection.h"

QgsDbSourceSelect::QgsDbSourceSelect(QWidget *parent, const char *name):QgsDbSourceSelectBase()
{
    btnAdd->setEnabled(false);
	populateConnectionList();
	// connect the double-click signal to the addSingleLayer slot in the parent


}

QgsDbSourceSelect::~QgsDbSourceSelect()
{
}
void QgsDbSourceSelect::populateConnectionList(){
	QSettings settings;
	QStringList keys = settings.subkeyList("/Qgis/connections");
	QStringList::Iterator it = keys.begin();
	cmbConnections->clear();
	while (it != keys.end()) {
		cmbConnections->insertItem(*it);
		++it;
	}
}
void QgsDbSourceSelect::addNewConnection()
{

	QgsNewConnection *nc = new QgsNewConnection();

	if (nc->exec()) {
		populateConnectionList();
	}
}
void QgsDbSourceSelect::editConnection()
{

	QgsNewConnection *nc = new QgsNewConnection(cmbConnections->currentText());

	if (nc->exec()) {
		nc->saveConnection();
	}
}

void QgsDbSourceSelect::deleteConnection(){
	QSettings settings;
	QString key = "/Qgis/connections/" + cmbConnections->currentText();
	QString msg = "Are you sure you want to remove the " + cmbConnections->currentText() + " connection and all associated settings?";
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

void QgsDbSourceSelect::addTables()
{
	//store the table info
	for (int idx = 0; idx < lstTables->numRows(); idx++) {
		if (lstTables->isSelected(idx))
			m_selectedTables += lstTables->text(idx);
	}

// BEGIN CHANGES ECOS
    if( m_selectedTables.empty() == true )
        QMessageBox::information(this, "Select Table","You must select a table in order to add a Layer.");
    else
	    accept();
// END CHANGES ECOS
}

void QgsDbSourceSelect::dbConnect()
{
	// populate the table list
	QSettings settings;

	QString key = "/Qgis/connections/" + cmbConnections->currentText();
	QString connString = "host=";
	QString host = settings.readEntry(key + "/host");
	connString += host;
	connString += " dbname=";
	QString database = settings.readEntry(key + "/database");
	connString += database + " user=";
	QString username = settings.readEntry(key + "/username");
	connString += username;
	QString password = settings.readEntry(key + "/password");
  bool makeConnection = true;
	if (password == QString::null) {
		// get password from user
	    makeConnection = false;
		QString password = QInputDialog::getText("Password for " + database + "@" + host,
													 "Please enter your password:",
													 QLineEdit::Password, QString::null, &makeConnection, this);
		
		//  allow null password entry in case its valid for the database
    }
		connString += " password=" + password;
	  if(makeConnection){
	m_connInfo = connString;	//host + " " + database + " " + username + " " + password;
	//qDebug(m_connInfo);
	PgDatabase *pd = new PgDatabase((const char *) m_connInfo);
//  std::cout << pd->ErrorMessage();
	if (pd->Status() == CONNECTION_OK) {
		// clear the existing entries
		lstTables->clear();
		// create the pixmaps for the layer types
		QPixmap pxPoint;
		pxPoint = QPixmap(point_layer_xpm);
		QPixmap pxLine;
		pxLine = QPixmap(line_layer_xpm);
		QPixmap pxPoly;
		pxPoly = QPixmap(polygon_layer_xpm);
		//qDebug("Connection succeeded");
		// get the list of tables
		QString sql = "select * from geometry_columns";
// where f_table_schema ='" + settings.readEntry(key + "/database") + "'";
		sql += " order by f_table_name";
		//qDebug("Fetching tables using: " + sql);
		int result = pd->ExecTuplesOk((const char *) sql);
		if (result) {
			QString msg;
			QTextOStream(&msg) << "Fetched " << pd->Tuples() << " tables from database";
			//qDebug(msg);
			for (int idx = 0; idx < pd->Tuples(); idx++) {
				QString v = "";
				if ( strlen(pd->GetValue(idx, "f_table_catalog") )) {
					v +=  pd->GetValue(idx, "f_table_catalog");
					v +=  ".";
				}
				if ( strlen(pd->GetValue(idx, "f_table_schema") )) {
					v += 	pd->GetValue(idx, "f_table_schema");
					v +=  ".";
				}
				v +=	pd->GetValue(idx, "f_table_name");
				v +=  " (";
				v += 	pd->GetValue(idx, "f_geometry_column");
				v +=  ")";

				QString type = pd->GetValue(idx, "type");
				QPixmap *p;
				if (type == "POINT" || type == "MULTIPOINT")
					p = &pxPoint;
				else if (type == "MULTIPOLYGON" || type == "POLYGON")
					p = &pxPoly;
				else if (type == "LINESTRING" || type == "MULTILINESTRING")
					p = &pxLine;
				else
					p = 0;
                                if ( p != 0 )
					lstTables->insertItem(*p, v);
			}
// BEGIN CHANGES ECOS
            if( cmbConnections->count() > 0 )
                btnAdd->setEnabled(true);
// END CHANGES ECOS
		} else {
			qDebug("Unable to get list of spatially enabled tables from geometry_columns table");
			qDebug(pd->ErrorMessage());
		}
	} else {
		QMessageBox::warning(this, "Connection failed",
							 "Connection to " + settings.readEntry(key + "/database") +
							 " on " + settings.readEntry(key + "/host") +
							 " failed. Either the database is down or your settings are incorrect.\n\nCheck your username and password and try again.");
	}
  }
}

QStringList QgsDbSourceSelect::selectedTables()
{
	return m_selectedTables;
}

QString QgsDbSourceSelect::connInfo()
{
	return m_connInfo;
}
