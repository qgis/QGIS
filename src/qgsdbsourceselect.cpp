/***************************************************************************
                    qgsdbsourceselect.h  -  description
                             -------------------
    begin                : Sat Jun 22 2002
    copyright            : (C) 2002 by Gary E.Sherman
    email                : sherman@mrcc.com
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/
#include <libpq++.h>
#include <iostream>
#include <qsettings.h>
#include <qpixmap.h>
#include <qlistbox.h>
#include <qstringlist.h>
#include <qcombobox.h>
#include "xpm/point_layer.xpm"
#include "xpm/line_layer.xpm"
#include "xpm/polygon_layer.xpm"
#include "qgsdbsourceselect.h"
#include "qgsnewconnection.h"

QgsDbSourceSelect::QgsDbSourceSelect():QgsDbSourceSelectBase()
{
	QSettings settings;
	QStringList keys = settings.subkeyList("/Qgis/connections");
	QStringList::Iterator it = keys.begin();
	while (it != keys.end()) {
		cmbConnections->insertItem(*it);

		++it;
	}

}

QgsDbSourceSelect::~QgsDbSourceSelect()
{
}
void QgsDbSourceSelect::addNewConnection()
{

	QgsNewConnection *nc = new QgsNewConnection();

	if (nc->exec()) {
	}
}
void QgsDbSourceSelect::editConnection()
{

	QgsNewConnection *nc = new QgsNewConnection(cmbConnections->currentText());

	if (nc->exec()) {
		nc->saveConnection();
	}
}
void QgsDbSourceSelect::addTables()
{
	//store the table info
	for (int idx = 0; idx < lstTables->numRows(); idx++) {
		if (lstTables->isSelected(idx))
			m_selectedTables += lstTables->text(idx);
	}
	accept();
}

void QgsDbSourceSelect::dbConnect()
{
	// populate the table list
	QSettings settings;

	QString key = "/Qgis/connections/" + cmbConnections->currentText();
	QString host = "host=" + settings.readEntry(key + "/host");
	QString database = "dbname=" + settings.readEntry(key + "/database");
	QString username = "user=" + settings.readEntry(key + "/username");
	QString password = "password=" + settings.readEntry(key + "/password");
	m_connInfo = host + " " + database + " " + username + " " + password;
	qDebug(m_connInfo);
	PgDatabase *pd = new PgDatabase((const char *) m_connInfo);
//	std::cout << pd->ErrorMessage();
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
		qDebug("Connection succeeded");
		// get the list of tables
		QString sql = "select * from geometry_columns where f_table_schema ='" + settings.readEntry(key + "/database") + "'";
		sql += " order by f_table_name";
		qDebug("Fetching tables using: " + sql);
		int result = pd->ExecTuplesOk((const char *) sql);
		if (result) {
			QString msg;
			QTextOStream(&msg) << "Fetched " << pd->Tuples() << " tables from database";
			qDebug(msg);
			for (int idx = 0; idx < pd->Tuples(); idx++) {
				QString v = pd->GetValue(idx, "f_table_name");
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
				lstTables->insertItem(*p, v);
			}
		} else {
			qDebug("Unable to get list of spatially enabled tables from geometry_columns table");
			qDebug(pd->ErrorMessage());
		}
	} else {
		qDebug("Connection failed");
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
