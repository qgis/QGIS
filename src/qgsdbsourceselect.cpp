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
 /* $Id$ */
#include <iostream>
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
#include "qgisapp.h"
extern "C"
{
#include <libpq-fe.h>
}
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
      PGconn *pd = PQconnectdb((const char *) m_connInfo);
//  std::cout << pd->ErrorMessage();
      if (PQstatus(pd) == CONNECTION_OK)
        {
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
          PGresult *result = PQexec(pd, (const char *) sql);
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
                  QPixmap *p;
                  if (type == "POINT" || type == "MULTIPOINT")
                    p = &pxPoint;
                  else if (type == "MULTIPOLYGON" || type == "POLYGON")
                    p = &pxPoly;
                  else if (type == "LINESTRING" || type == "MULTILINESTRING")
                    p = &pxLine;
                  else
                    p = 0;
                  if (p != 0)
                  {
                    QListViewItem *lItem = new QListViewItem(lstTables);
                    lItem->setText(1,v);
                    lItem->setPixmap(0,*p);
                    lstTables->insertItem(lItem);
                  }
                }
// BEGIN CHANGES ECOS
              if (cmbConnections->count() > 0)
                btnAdd->setEnabled(true);
// END CHANGES ECOS
          } else
            {
              qDebug("Unable to get list of spatially enabled tables from geometry_columns table");
              qDebug(PQerrorMessage(pd));
            }
      } else
        {
          QMessageBox::warning(this, tr("Connection failed"),
                               tr
                               ("Connection to %1 on %2 failed. Either the database is down or your settings are incorrect.%3Check your username and password and try again.").
                               arg(settings.readEntry(key + "/database")).arg(settings.readEntry(key + "/host")).arg("\n\n"));
        }
      PQfinish(pd);
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
  item->setText(2,QInputDialog::getText("Define SQL Layer","Enter the SQL where clause (without the where keyword) to define the layer"));
}
void QgsDbSourceSelect::addLayer(QListBoxItem * item)
{
  qgisApp->addVectorLayer(m_connInfo, item->text(), "postgres");
//  lstTables->setSelected(item, false);
}
