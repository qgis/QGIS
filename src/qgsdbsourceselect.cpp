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
#include <q3listbox.h>
#include <q3listview.h>
#include <qstringlist.h>
#include <QComboBox>
#include <qpushbutton.h>
#include <qmessagebox.h>
#include <qinputdialog.h>
#include <q3groupbox.h>
#include "xpm/point_layer.xpm"
#include "xpm/line_layer.xpm"
#include "xpm/polygon_layer.xpm"
#include "qgsdbsourceselect.h"
#include "qgsnewconnection.h"
#include "qgspgquerybuilder.h"
#include "qgisapp.h"
#include "qgscontexthelp.h"
QgsDbSourceSelect::QgsDbSourceSelect(QgisApp *app, const char *name, bool modal)
: QgsDbSourceSelectBase(app, name, modal), qgisApp(app)
{
  btnAdd->setEnabled(false);
  populateConnectionList();
  // connect the double-click signal to the addSingleLayer slot in the parent

  //disable the 'where clause' box for 0.4 release
  //  groupBox3->hide();
  //insert the encoding types available in qt
  mEncodingComboBox->insertItem("BIG5"); 
  mEncodingComboBox->insertItem("BIG5-HKSCS"); 
  mEncodingComboBox->insertItem("EUCJP"); 
  mEncodingComboBox->insertItem("EUCKR"); 
  mEncodingComboBox->insertItem("GB2312"); 
  mEncodingComboBox->insertItem("GBK"); 
  mEncodingComboBox->insertItem("GB18030"); 
  mEncodingComboBox->insertItem("JIS7"); 
  mEncodingComboBox->insertItem("SHIFT-JIS"); 
  mEncodingComboBox->insertItem("TSCII"); 
  mEncodingComboBox->insertItem("UTF-8"); 
  mEncodingComboBox->insertItem("UTF-16"); 
  mEncodingComboBox->insertItem("KOI8-R"); 
  mEncodingComboBox->insertItem("KOI8-U"); 
  mEncodingComboBox->insertItem("ISO8859-1"); 
  mEncodingComboBox->insertItem("ISO8859-2");
  mEncodingComboBox->insertItem("ISO8859-3"); 
  mEncodingComboBox->insertItem("ISO8859-4"); 
  mEncodingComboBox->insertItem("ISO8859-5"); 
  mEncodingComboBox->insertItem("ISO8859-6");
  mEncodingComboBox->insertItem("ISO8859-7"); 
  mEncodingComboBox->insertItem("ISO8859-8"); 
  mEncodingComboBox->insertItem("ISO8859-8-I"); 
  mEncodingComboBox->insertItem("ISO8859-9"); 
  mEncodingComboBox->insertItem("ISO8859-10"); 
  mEncodingComboBox->insertItem("ISO8859-13"); 
  mEncodingComboBox->insertItem("ISO8859-14"); 
  mEncodingComboBox->insertItem("ISO8859-15"); 
  mEncodingComboBox->insertItem("IBM 850"); 
  mEncodingComboBox->insertItem("IBM 866"); 
  mEncodingComboBox->insertItem("CP874"); 
  mEncodingComboBox->insertItem("CP1250"); 
  mEncodingComboBox->insertItem("CP1251"); 
  mEncodingComboBox->insertItem("CP1252"); 
  mEncodingComboBox->insertItem("CP1253"); 
  mEncodingComboBox->insertItem("CP1254"); 
  mEncodingComboBox->insertItem("CP1255"); 
  mEncodingComboBox->insertItem("CP1256"); 
  mEncodingComboBox->insertItem("CP1257"); 
  mEncodingComboBox->insertItem("CP1258"); 
  mEncodingComboBox->insertItem("Apple Roman"); 
  mEncodingComboBox->insertItem("TIS-620"); 

  //read the last encoding from the settings
  //or use local as default
  QSettings settings("QuantumGIS", "qgis"); 
  QString lastUsedEncoding = settings.readEntry("/qgis/UI/encoding");
  if(lastUsedEncoding.isNull()||lastUsedEncoding.isEmpty()||lastUsedEncoding=="\0")
    {
      mEncodingComboBox->setCurrentText(QString(QTextCodec::codecForLocale()->name()));
    }
  else
    {
      mEncodingComboBox->setCurrentText(lastUsedEncoding);
    }
}

QgsDbSourceSelect::~QgsDbSourceSelect()
{
    PQfinish(pd);
}
void QgsDbSourceSelect::populateConnectionList()
{
  QSettings settings("QuantumGIS", "qgis");
  QStringList keys = settings.subkeyList("/Qgis/connections");
  QStringList::Iterator it = keys.begin();
  cmbConnections->clear();
  while (it != keys.end())
  {
    cmbConnections->insertItem(*it);
    ++it;
  }
  setConnectionListPosition();
}
void QgsDbSourceSelect::addNewConnection()
{

  QgsNewConnection *nc = new QgsNewConnection(this);

  if (nc->exec())
  {
    populateConnectionList();
  }
}
void QgsDbSourceSelect::editConnection()
{

  QgsNewConnection *nc = new QgsNewConnection(this, cmbConnections->currentText());

  if (nc->exec())
  {
    nc->saveConnection();
  }
}

void QgsDbSourceSelect::deleteConnection()
{
  QSettings settings("QuantumGIS", "qgis");
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
    settings.removeEntry(key + "/port");
    settings.removeEntry(key + "/save");
    settings.removeEntry(key);
    //if(!success){
    //  QMessageBox::information(this,"Unable to Remove","Unable to remove the connection " + cmbConnections->currentText());
    //}
    cmbConnections->removeItem(cmbConnections->currentItem());  // populateConnectionList();
    setConnectionListPosition();
  }
}

void QgsDbSourceSelect::addTables()
{
  //store the table info
  Q3ListViewItemIterator it( lstTables );
  while ( it.current() ) 
  {
    Q3ListViewItem *item = it.current();
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
  QSettings settings("QuantumGIS", "qgis");

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
  std::cout << "Connection info: " << connString.toLocal8Bit().data() << std::endl;
#endif
  if (makeConnection)
  {
    m_connInfo = connString;  //host + " " + database + " " + username + " " + password;
    //qDebug(m_connInfo);
    pd = PQconnectdb(m_connInfo.toLocal8Bit().data());
    //  std::cout << pd->ErrorMessage();
    if (PQstatus(pd) == CONNECTION_OK)
    {
      // create the pixmaps for the layer types
      QPixmap pxPoint(point_layer_xpm);
      QPixmap pxLine(line_layer_xpm);
      QPixmap pxPoly(polygon_layer_xpm);
      //qDebug("Connection succeeded");
      // tell the DB that we want text encoded in UTF8
      PQsetClientEncoding(pd, "UNICODE");

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
      Q3ListViewItem *lItem = new Q3ListViewItem(lstTables);
      lItem->setText(1,iter->first);
      lItem->setPixmap(0,*p);
      lstTables->insertItem(lItem);
    }
    else
    {
      qDebug(("Unknown geometry type of " + iter->second).toLocal8Bit().data());
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
void QgsDbSourceSelect::setSql(Q3ListViewItem *item)
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
void QgsDbSourceSelect::addLayer(Q3ListBoxItem * item)
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
  PGresult *result = PQexec(pg, sql.toLocal8Bit().data());
  if (result)
  {
    QString msg;
    QTextOStream(&msg) << "Fetched " << PQntuples(result) << " tables from database";
    //qDebug(msg);
    for (int idx = 0; idx < PQntuples(result); idx++)
    {
      // Be a bit paranoid and check that the table actually
      // exists. This is not done as a subquery in the query above
      // because I can't get it to work correctly when there are tables
      // with capital letters in the name.

      // Take care to deal with tables with the same name but in different schema.
      QString tableName = PQgetvalue(result, idx, PQfnumber(result, "f_table_name"));
      QString schemaName = PQgetvalue(result, idx, PQfnumber(result, "f_table_schema"));
      sql = "select oid from pg_class where relname = '" + tableName + "'";
      if (schemaName.length() > 0)
	sql +=" and relnamespace = (select oid from pg_namespace where nspname = '" +
	  schemaName + "')";

      PGresult* exists = PQexec(pg, sql.toLocal8Bit().data());
      if (PQntuples(exists) == 1)
  {
    QString v = "";

    if (schemaName.length() > 0)
    {
      v += schemaName;
      v += ".";
    }

    v += tableName;
    v += " (";
    v += PQgetvalue(result, idx, PQfnumber(result, "f_geometry_column"));
    v += ")";

    QString type = PQgetvalue(result, idx, PQfnumber(result, "type"));
    details.push_back(geomPair(v, type));
  }
      PQclear(exists);
    }
    ok = true;
  }
  PQclear(result);

  // Now have a look for geometry columns that aren't in the
  // geometry_columns table. This code is specific to postgresql,
  // but an equivalent query should be possible in other
  // databases.
  sql = "select pg_class.relname, pg_namespace.nspname, pg_attribute.attname from "
    "pg_attribute, pg_class, pg_type, pg_namespace where pg_type.typname = 'geometry' and "
    "pg_attribute.atttypid = pg_type.oid and pg_attribute.attrelid = pg_class.oid "
    "and cast(pg_class.relname as character varying) not in "
    "(select f_table_name from geometry_columns) "
    "and pg_namespace.oid = pg_class.relnamespace "
    "and pg_class.relkind in ('v', 'r')"; // only from views and relations (tables)
  
  result = PQexec(pg, sql.toLocal8Bit().data());

  for (int i = 0; i < PQntuples(result); i++)
  {
    // Have the column name, schema name and the table name. The concept of a
    // catalog doesn't exist in postgresql so we ignore that, but we
    // do need to get the geometry type.

    // Make the assumption that the geometry type for the first
    // row is the same as for all other rows. 

    // Flag these not geometry_columns table tables so that the UI
    // can indicate this????
    QString table  = PQgetvalue(result, i, 0); // relname
    QString schema = PQgetvalue(result, i, 1); // nspname
    QString column = PQgetvalue(result, i, 2); // attname

    QString query = "select GeometryType(" + column + ") from ";
    if (schema.length() > 0)
      query += "\"" + schema + "\".";
    query += "\"" + table + "\" where " + column + " is not null limit 1";

    PGresult* gresult = PQexec(pg, query.toLocal8Bit().data());
    if (PQresultStatus(gresult) != PGRES_TUPLES_OK)
    {
      QString myError = (tr("Access to relation ") + table + tr(" using sql;\n") + query +
       tr("\nhas failed. The database said:\n"));
      qDebug(myError + QString(PQresultErrorMessage(gresult)));
    }
    else
    {
      QString type = PQgetvalue(gresult, 0, 0); // GeometryType
      QString full_desc = "";
      if (schema.length() > 0)
  full_desc = schema + ".";
      full_desc += table + " (" + column + ")";
      details.push_back(geomPair(full_desc, type));
    }
    PQclear(gresult);
  }
  ok = true;

  PQclear(result);

  return ok;
}
QString QgsDbSourceSelect::encoding()
{
  return mEncodingComboBox->currentText();
}
void QgsDbSourceSelect::showHelp()
{
  QgsContextHelp::run(context_id);
}
void QgsDbSourceSelect::dbChanged()
{
  // Remember which database was selected.
  QSettings settings("QuantumGIS", "qgis");
  settings.writeEntry("/Qgis/connections/selected", 
		      cmbConnections->currentText());
}
void QgsDbSourceSelect::setConnectionListPosition()
{
  QSettings settings("QuantumGIS", "qgis");
  // If possible, set the item currently displayed database
  QString toSelect = settings.readEntry("/Qgis/connections/selected");
  // Does toSelect exist in cmbConnections?
  bool set = false;
  for (int i = 0; i < cmbConnections->count(); ++i)
    if (cmbConnections->text(i) == toSelect)
    {
      cmbConnections->setCurrentItem(i);
      set = true;
      break;
    }
  // If we couldn't find the stored item, but there are some, 
  // default to the last item (this makes some sense when deleting
  // items as it allows the user to repeatidly click on delete to
  // remove a whole lot of items).
  if (!set && cmbConnections->count() > 0)
  {
    // If toSelect is null, then the selected connection wasn't found
    // by QSettings, which probably means that this is the first time
    // the user has used qgis with database connections, so default to
    // the first in the list of connetions. Otherwise default to the last.
    if (toSelect.isNull())
      cmbConnections->setCurrentItem(0);
    else
      cmbConnections->setCurrentItem(cmbConnections->count()-1);
  }
}
