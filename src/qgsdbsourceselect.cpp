/***************************************************************************
                     qgsdbsourceselect.cpp  -  description
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
/* $Id$ */
#include <iostream>
#include <cassert>
#include <qsettings.h>
#include <qpixmap.h>
#include <qlistbox.h>
#include <qlistview.h>
#include <qstringlist.h>
#include <qcombobox.h>
#include <qpushbutton.h>
#include <qmessagebox.h>
#include <qinputdialog.h>
#include <qgroupbox.h>
#include "xpm/point_layer.xpm"
#include "xpm/line_layer.xpm"
#include "xpm/polygon_layer.xpm"
#include "qgsdbsourceselect.h"
#include "qgsnewconnection.h"
#include "qgspgquerybuilder.h"
#include "qgisapp.h"
QgsDbSourceSelect::QgsDbSourceSelect(QgisApp * app, QWidget * parent, const char *name):QgsDbSourceSelectBase(parent, name),
                                                                                        qgisApp(app)
{
  btnAdd->setEnabled(false);
  populateConnectionList();
  // connect the double-click signal to the addSingleLayer slot in the parent

  //disable the 'where clause' box for 0.4 release
  //  groupBox3->hide();

}

QgsDbSourceSelect::~QgsDbSourceSelect()
{
    PQfinish(pd);
}
void QgsDbSourceSelect::populateConnectionList()
{
  QSettings settings;
  QStringList keys = settings.subkeyList("/Qgis/connections");
  QStringList::Iterator it = keys.begin();
  cmbConnections->clear();
  while (it != keys.end())
  {
    cmbConnections->insertItem(*it);
    ++it;
  }
}
void QgsDbSourceSelect::addNewConnection()
{

  QgsNewConnection *nc = new QgsNewConnection();

  if (nc->exec())
  {
    populateConnectionList();
  }
}
void QgsDbSourceSelect::editConnection()
{

  QgsNewConnection *nc = new QgsNewConnection(cmbConnections->currentText());

  if (nc->exec())
  {
    nc->saveConnection();
  }
}

void QgsDbSourceSelect::deleteConnection()
{
  QSettings settings;
  QString key = "/Qgis/connections/" + cmbConnections->currentText();
  QString msg =
    tr("Are you sure you want to remove the ") + cmbConnections->currentText() + tr(" connection and all associated settings?");
  int result = QMessageBox::information(this, tr("Confirm Delete"), msg, tr("Yes"), tr("No"));
  if (result == 0)
  {
    settings.removeEntry(key + "/host");
    settings.removeEntry(key + "/database");
    settings.removeEntry(key + "/username");
    settings.removeEntry(key + "/password");
    //if(!success){
    //  QMessageBox::information(this,"Unable to Remove","Unable to remove the connection " + cmbConnections->currentText());
    //}
    cmbConnections->removeItem(cmbConnections->currentItem());  // populateConnectionList();
  }
}

void QgsDbSourceSelect::addTables()
{
  //store the table info
  QListViewItemIterator it( lstTables );
  while ( it.current() ) 
  {
    QListViewItem *item = it.current();
    ++it;

    if ( item->isSelected() )
    {
      m_selectedTables += item->text(1) + " sql=" + item->text(2);
    }
  }

  // BEGIN CHANGES ECOS
  if (m_selectedTables.empty() == true)
    QMessageBox::information(this, tr("Select Table"), tr("You must select a table in order to add a Layer."));
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
  connString += database + " port=";
  QString port = settings.readEntry(key + "/port");
  if(port.length() == 0){
    port = "5432";
  }
  connString += port + " user=";
  QString username = settings.readEntry(key + "/username");
  connString += username;
  QString password = settings.readEntry(key + "/password");
  bool makeConnection = true;
  if (password == QString::null)
  {
    // get password from user 
    makeConnection = false;
    QString password = QInputDialog::getText(tr("Password for ") + database + "@" + host,
        tr("Please enter your password:"),
        QLineEdit::Password, QString::null, &makeConnection, this);

    //  allow null password entry in case its valid for the database
  }
  connString += " password=" + password;
#ifdef QGISDEBUG
  std::cout << "Connection info: " << connString << std::endl;
#endif
  if (makeConnection)
  {
    m_connInfo = connString;  //host + " " + database + " " + username + " " + password;
    //qDebug(m_connInfo);
    pd = PQconnectdb((const char *) m_connInfo);
    //  std::cout << pd->ErrorMessage();
    if (PQstatus(pd) == CONNECTION_OK)
    {
      // create the pixmaps for the layer types
      QPixmap pxPoint(point_layer_xpm);
      QPixmap pxLine(line_layer_xpm);
      QPixmap pxPoly(polygon_layer_xpm);
      //qDebug("Connection succeeded");

      // clear the existing entries
      lstTables->clear();
      // get the list of suitable tables and columns and populate the UI
      geomCol details;
      if (getGeometryColumnInfo(pd, details))
      {
	geomCol::const_iterator iter = details.begin();
	for (; iter != details.end(); ++iter)
	{
	  QPixmap *p = 0;
	  if (iter->second == "POINT" || iter->second == "MULTIPOINT")
	    p = &pxPoint;
	  else if (iter->second == "MULTIPOLYGON" || iter->second == "POLYGON")
	    p = &pxPoly;
	  else if (iter->second == "LINESTRING" || iter->second == "MULTILINESTRING")
	    p = &pxLine;

	  if (p != 0)
	  {
	    QListViewItem *lItem = new QListViewItem(lstTables);
	    lItem->setText(1,iter->first);
	    lItem->setPixmap(0,*p);
	    lstTables->insertItem(lItem);
	  }
	  else
	  {
	    qDebug("Unknown geometry type of " + iter->second);
	  }
	}
      }
      else
      {
        qDebug("Unable to get list of spatially enabled tables from the database");
        qDebug(PQerrorMessage(pd));
      }
      // BEGIN CHANGES ECOS
      if (cmbConnections->count() > 0)
	btnAdd->setEnabled(true);
      // END CHANGES ECOS
    } else
    {
      QMessageBox::warning(this, tr("Connection failed"),
          tr
          ("Connection to %1 on %2 failed. Either the database is down or your settings are incorrect.%3Check your username and password and try again.%4%5").
          arg(settings.readEntry(key + "/database")).arg(settings.readEntry(key + "/host")).arg("\n\n").arg("\n\n").arg(PQerrorMessage(pd)));
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
void QgsDbSourceSelect::setSql(QListViewItem *item)
{
  // Parse out the table name
  QString table = item->text(1).left(item->text(1).find("("));
  assert(pd != 0);
  // create a query builder object
  QgsPgQueryBuilder * pgb = new QgsPgQueryBuilder(table, pd, this);
  // set the current sql in the query builder sql box
  pgb->setSql(item->text(2));
  // set the PG connection object so it can be used to fetch the
  // fields for the table, get sample values, and test the query
  pgb->setConnection(pd);
  // show the dialog
  if(pgb->exec())
  {
    // if user accepts, store the sql for the layer so it can be used
    // if and when the layer is added to the map
    item->setText(2, pgb->sql());
  }
  // delete the query builder object
  delete pgb;
}
void QgsDbSourceSelect::addLayer(QListBoxItem * item)
{
  qgisApp->addVectorLayer(m_connInfo, item->text(), "postgres");
  //  lstTables->setSelected(item, false);
}

bool QgsDbSourceSelect::getGeometryColumnInfo(PGconn *pg, 
					      geomCol& details)
{
  bool ok = false;

  QString sql = "select * from geometry_columns";
  // where f_table_schema ='" + settings.readEntry(key + "/database") + "'";
  sql += " order by f_table_name";
  //qDebug("Fetching tables using: " + sql);
  PGresult *result = PQexec(pg, (const char *) sql);
  if (result)
  {
    QString msg;
    QTextOStream(&msg) << "Fetched " << PQntuples(result) << " tables from database";
    //qDebug(msg);
    for (int idx = 0; idx < PQntuples(result); idx++)
    {
      QString v = "";
      if (strlen(PQgetvalue(result, idx, PQfnumber(result, "f_table_catalog"))))
      {
	v += PQgetvalue(result, idx, PQfnumber(result, "f_table_catalog"));
	v += ".";
      }
      if (strlen(PQgetvalue(result, idx, PQfnumber(result, "f_table_schema"))))
      {
	v += PQgetvalue(result, idx, PQfnumber(result, "f_table_schema"));
	v += ".";
      }
      v += PQgetvalue(result, idx, PQfnumber(result, "f_table_name"));
      v += " (";
      v += PQgetvalue(result, idx, PQfnumber(result, "f_geometry_column"));
      v += ")";

      QString type = PQgetvalue(result, idx, PQfnumber(result, "type"));

      details.push_back(geomPair(v, type));
    }
    ok = true;
  }
  PQclear(result);

  // Now have a look for geometry columns that aren't in the
  // geometry_columns table. This code is specific to postgresql,
  // but an equivalent query should be possible in other
  // databases.
  sql = "select pg_class.relname, pg_attribute.attname from "
    "pg_attribute, pg_class where pg_type.typname = 'geometry' and "
    "pg_attribute.atttypid = pg_type.oid and pg_attribute.attrelid = pg_class.oid "
    "and cast(pg_class.relname as character varying) not in "
    "(select f_table_name from geometry_columns)";
  
  result = PQexec(pg, (const char *) sql);
  if (result)
  {
    for (int i = 0; i < PQntuples(result); i++)
    {
      // Have the column name and the table name. The concept of a
      // catalog doesn't exist in postgresql so we ignore that, but we
      // do need to get the schema name and geometry type.

      // Make the assumption that the geometry type for the first
      // row is the same as for all other rows. 

      // There may be more than one geometry column per table, so need
      // to deal with that. Currently just take the first column
      // returned. XXXX
      
      // Flag these not geometry_columns table tables so that the UI
      // can indicate this????

      QString table  = PQgetvalue(result, i, PQfnumber(result, "relname"));
      QString column = PQgetvalue(result, i, PQfnumber(result, "attname"));
      
      QString query = "select GeometryType(" + 
	column + "), current_schema() from " + table + " limit 1";
      PGresult* gresult = PQexec(pg, (const char*) query);
      if (gresult)
      {
	QString type = PQgetvalue(gresult, 0, PQfnumber(gresult, "geometrytype"));
	QString schema = PQgetvalue(gresult, 0, PQfnumber(gresult, "current_schema"));
	QString full_desc = "";
	if (schema.length() > 0)
	  full_desc = schema + ".";
	full_desc += table + " (" + column + ")";
	details.push_back(geomPair(full_desc, type));
      }
      PQclear(gresult);
    }
    ok = true;
  }

  PQclear(result);

  return ok;
}

