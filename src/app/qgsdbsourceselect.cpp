/***************************************************************************
                             qgsdbsourceselect.cpp  
       Dialog to select PostgreSQL layer(s) and add it to the map canvas
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

#include "qgsdbsourceselect.h"

#include "qgisapp.h"
#include "qgslogger.h"
#include "qgsapplication.h"
#include "qgscontexthelp.h"
#include "qgsnewconnection.h"
#include "qgspgquerybuilder.h"

#include <QInputDialog>
#include <QMessageBox>
#include <QSettings>
#include <QTextOStream>
#include <QTableWidgetItem>
#include <QHeaderView>
#include <QStringList>

#include <cassert>
#include <iostream>

QgsDbSourceSelect::QgsDbSourceSelect(QgisApp *app, Qt::WFlags fl)
  : QDialog(app, fl), mColumnTypeThread(NULL), qgisApp(app), pd(0)
{
  setupUi(this);
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
  QSettings settings; 
  QString lastUsedEncoding = settings.readEntry("/UI/encoding");
  if(lastUsedEncoding.isNull()||lastUsedEncoding.isEmpty()||lastUsedEncoding=="\0")
    {
      mEncodingComboBox->setCurrentText(QString(QTextCodec::codecForLocale()->name()));
    }
  else
    {
      mEncodingComboBox->setCurrentText(lastUsedEncoding);
    }

  // Do some things that couldn't be done in designer
  lstTables->horizontalHeader()->setStretchLastSection(true);
  // Set the column count
  lstTables->setColumnCount(dbssColumns);
  mColumnLabels += tr("Type"); 
  mColumnLabels += tr("Name"); 
  mColumnLabels += tr("Sql");
  lstTables->setHorizontalHeaderLabels(mColumnLabels);
  lstTables->verticalHeader()->hide();
}
/** Autoconnected SLOTS **/
// Slot for adding a new connection
void QgsDbSourceSelect::on_btnNew_clicked()
{
  addNewConnection();
}
// Slot for deleting an existing connection
void QgsDbSourceSelect::on_btnDelete_clicked()
{
  deleteConnection();
}
// Slot for performing action when the Add button is clicked
void QgsDbSourceSelect::on_btnAdd_clicked()
{
  addTables();
}

// Slot for opening the query builder when a layer is double clicked
void QgsDbSourceSelect::on_lstTables_itemDoubleClicked(QTableWidgetItem *item)
{
  setSql(item);
}

// Slot for editing a connection
void QgsDbSourceSelect::on_btnEdit_clicked()
{
  editConnection();
}

// Slot for showing help
void QgsDbSourceSelect::on_btnHelp_clicked()
{
  showHelp();
}
/** End Autoconnected SLOTS **/

// Remember which database is selected
void QgsDbSourceSelect::on_cmbConnections_activated(int)
{
  dbChanged();
}

void QgsDbSourceSelect::setLayerType(QString schema, 
                                     QString table, QString column,
                                     QString type)
{
  QgsDebugMsg("Received layer type of " + type + " for " + schema + '.' + table + '.' + column);

  // Find the right row in the table by searching for the text that
  // was put into the Name column.
  QString full_desc = fullDescription(schema, table, column, "WAITING");

  QList<QTableWidgetItem*> ii = lstTables->findItems(full_desc, Qt::MatchExactly);

  if (ii.count() > 0)
  {
    int row = lstTables->row(ii.at(0));

    if(type!="")
    {
      QStringList types = type.split(",");
      updateRow(row, fullDescription(schema, table, column, "AS " + types[0]), types[0]);

      for(int i=1; i<types.size(); i++) {
        lstTables->insertRow(row+1);
        initRow(row+1);
        updateRow(row+1, fullDescription(schema, table, column, "AS " + types[i]), types[i]);
      }

      lstTables->resizeColumnsToContents();
    }
    else
    {
      lstTables->removeRow(row);
    }
  }
}

QString QgsDbSourceSelect::makeGeomQuery(QString schema, 
                                                QString table, QString column)
{
  return QString("select distinct "
		  "case"
		  " when geometrytype(%1) IN ('POINT','MULTIPOINT') THEN 'POINT'"
		  " when geometrytype(%1) IN ('LINESTRING','MULTILINESTRING') THEN 'LINESTRING'"
		  " when geometrytype(%1) IN ('POLYGON','MULTIPOLYGON') THEN 'POLYGON'"
		  " end "
		  "from \"%2\".\"%3\"").arg(column).arg(schema).arg(table);
}

QgsDbSourceSelect::~QgsDbSourceSelect()
{
    PQfinish(pd);
}
void QgsDbSourceSelect::populateConnectionList()
{
  QSettings settings;
  QStringList keys = settings.subkeyList("/PostgreSQL/connections");
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
  QSettings settings;
  QString key = "/Postgresql/connections/" + cmbConnections->currentText();
  QString msg =
    tr("Are you sure you want to remove the ") + cmbConnections->currentText() + tr(" connection and all associated settings?");
  QMessageBox::StandardButton result = QMessageBox::information(this, tr("Confirm Delete"), msg, QMessageBox::Ok | QMessageBox::Cancel);
  if (result == QMessageBox::Ok)
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

  for (int i=0; i<lstTables->rowCount(); i++)
  {
    if ( lstTables->isItemSelected(lstTables->item(i, dbssDetail)) )
    {
      QString table = lstTables->item(i,dbssDetail)->text();
      QString query;

      bool applyFilter = table.contains(") AS ");
      if( applyFilter ) {
        int i = table.find("(");
        int j = table.find(")");
        QString column = table.mid(i+1,j-i-1);
	QString type = table.mid(j+5);
	query += table.left(j+1) + " sql=";
	
        if( type=="POINT" ) {
          query += QString("GeometryType(\"%1\") IN ('POINT','MULTIPOINT')").arg(column);
        } else if(type=="LINESTRING") {
          query += QString("GeometryType(\"%1\") IN ('LINESTRING','MULTILINESTRING')").arg(column);
        } else if(type=="POLYGON") {
          query += QString("GeometryType(\"%1\") IN ('POLYGON','MULTIPOLYGON')").arg(column);
        } else {
          continue;
        }
      }
      else
      {
	query += table + " sql=";
      }

      QTableWidgetItem *sqlItem = lstTables->item(i, dbssSql);
      if (sqlItem && sqlItem->text()!="" )
      {
        if(applyFilter)
          query += QString(" AND (%1)").arg( sqlItem->text() );
        else
          query += sqlItem->text();
      }

      m_selectedTables += query;
    }
  }

  // BEGIN CHANGES ECOS
  if ( m_selectedTables.empty() )
    QMessageBox::information(this, tr("Select Table"), tr("You must select a table in order to add a Layer."));
  else
    accept();
  // END CHANGES ECOS
}

void QgsDbSourceSelect::initRow(int row)
{
  QTableWidgetItem *iconItem = new QTableWidgetItem();
  lstTables->setItem(row, dbssType, iconItem);

  QTableWidgetItem *textItem = new QTableWidgetItem();
  textItem->setToolTip( tr("double click to open PostgreSQL query builder") );
  lstTables->setItem(row, dbssDetail, textItem);
}

void QgsDbSourceSelect::updateRow(int row, QString detail, QString type)
{
  if (!mLayerIcons.contains(type))
    type="UNKNOWN";

  QTableWidgetItem *iconItem = lstTables->item(row, dbssType);
  iconItem->setIcon( mLayerIcons.value(type).second );
  iconItem->setToolTip( mLayerIcons.value(type).first );

  lstTables->item(row, dbssDetail)->setText( detail );
}

void QgsDbSourceSelect::on_btnConnect_clicked()
{
  if(mColumnTypeThread)
  {
    mColumnTypeThread->stop();
    mColumnTypeThread=0;
  }
  // populate the table list
  QSettings settings;

  QString key = "/PostgreSQL/connections/" + cmbConnections->currentText();
  QString connString = "host=";
  QString host = settings.readEntry(key + "/host");
  connString += host;
  connString += " dbname=";
  QString database = settings.readEntry(key + "/database");
  connString += database + " port=";
  QString port = settings.readEntry(key + "/port");
  if(port.length() == 0)
  {
    port = "5432";
  }
  connString += port + " user=";
  QString username = settings.readEntry(key + "/username");
  connString += username;
  QString password = settings.readEntry(key + "/password");
  bool searchPublicOnly = settings.readBoolEntry(key + "/publicOnly");
  bool searchGeometryColumnsOnly = settings.readBoolEntry(key + "/geometryColumnsOnly");
  bool makeConnection = true;
  if (password == QString::null)
  {
    // get password from user 
    makeConnection = false;
    QString password = QInputDialog::getText(tr("Password for ") + database + "@" + host,
        tr("Please enter your password:"),
        QLineEdit::Password, QString::null, &makeConnection, this);
    // allow null password entry in case its valid for the database
  }

  // Need to escape the password to allow for single quotes and backslashes
  password.replace('\\', "\\\\");
  password.replace('\'', "\\'");
  connString += " password='" + password + "'";

  QgsDebugMsg("Connection info: " + connString);

  if (makeConnection)
  {
    m_connInfo = connString;  //host + " " + database + " " + username + " " + password;
    //qDebug(m_connInfo);
    // Tidy up an existing connection if one exists.
    if (pd != 0)
      PQfinish(pd);

    pd = PQconnectdb(m_connInfo.toLocal8Bit().data());
    //  std::cout << pd->ErrorMessage();
    if (PQstatus(pd) == CONNECTION_OK)
    {
      // create the pixmaps for the layer types if we haven't already
      // done so.
      if (mLayerIcons.count() == 0)
      {
        QString myThemePath = QgsApplication::themePath();
        mLayerIcons.insert("POINT",
            qMakePair(tr("Point layer"), 
              QIcon(myThemePath+"/mIconPointLayer.png")));
        mLayerIcons.insert("MULTIPOINT", 
            qMakePair(tr("Multi-point layer"), 
              mLayerIcons.value("POINT").second));

        mLayerIcons.insert("LINESTRING",
            qMakePair(tr("Linestring layer"), 
              QIcon(myThemePath+"/mIconLineLayer.png")));
        mLayerIcons.insert("MULTILINESTRING",
            qMakePair(tr("Multi-linestring layer"), 
              mLayerIcons.value("LINESTRING").second));

        mLayerIcons.insert("POLYGON",
            qMakePair(tr("Polygon layer"), 
              QIcon(myThemePath+"/mIconPolygonLayer.png")));
        mLayerIcons.insert("MULTIPOLYGON",
            qMakePair(tr("Multi-polygon layer"),
              mLayerIcons.value("POLYGON").second));

        mLayerIcons.insert("GEOMETRY",
            qMakePair(tr("Mixed geometry layer"), 
              QIcon(myThemePath+"/mIconGeometryLayer.png")));
        mLayerIcons.insert("GEOMETRYCOLLECTION",
            qMakePair(tr("Geometry collection layer"), 
              mLayerIcons.value("GEOMETRY").second));

        mLayerIcons.insert("WAITING",
            qMakePair(tr("Waiting for layer type"), 
              QIcon(myThemePath+"/mIconWaitingForLayerType.png")));
        mLayerIcons.insert("UNKNOWN",
            qMakePair(tr("Unknown layer type"), 
              QIcon(myThemePath+"/mIconUnknownLayerType.png")));
      }
      //qDebug("Connection succeeded");
      // tell the DB that we want text encoded in UTF8
      PQsetClientEncoding(pd, "UNICODE");

      // Here's an annoying thing... calling clear() removes the
      // header items too, so we need to reinstate them after calling
      // clear(). 
      lstTables->clear();
      lstTables->setRowCount(0);
      lstTables->setHorizontalHeaderLabels(mColumnLabels);

      // get the list of suitable tables and columns and populate the UI
      geomCol details;
      if (getGeometryColumnInfo(pd, details, searchGeometryColumnsOnly,
                                searchPublicOnly))
      {
        details.sort();
        geomCol::const_iterator iter = details.begin();
        for (; iter != details.end(); ++iter)
        {
          QString type = iter->second;
          int row = lstTables->rowCount();
          lstTables->setRowCount(row+1);

          initRow(row);
          updateRow(row, iter->first, type);
        }

        // And tidy up the columns & rows
        lstTables->resizeColumnsToContents();
        lstTables->resizeRowsToContents();

        // Start the thread that gets the geometry type for relations that
        // may take a long time to return
        if (mColumnTypeThread != NULL)
        {
           connect(mColumnTypeThread, SIGNAL(setLayerType(QString,QString,QString,QString)),
                   this, SLOT(setLayerType(QString,QString,QString,QString)));
	   connect(this, SIGNAL(finished()),
	           mColumnTypeThread, SLOT(stop()) );

            // Do it in a thread.
            mColumnTypeThread->start();
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
    }
    else
    {
      QMessageBox::warning(this, tr("Connection failed"),
          tr
          ("Connection to %1 on %2 failed. Either the database is down or your settings are incorrect.%3Check your username and password and try again.%4The database said:%5%6").
          arg(settings.readEntry(key + "/database")).arg(settings.readEntry(key + "/host")).arg("\n\n").arg("\n\n").arg("\n").arg(PQerrorMessage(pd)));
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
void QgsDbSourceSelect::setSql(QTableWidgetItem *item)
{
  int row = lstTables->row(item);
  QString tableText = lstTables->item(row, dbssDetail)->text();

  QTableWidgetItem* sqlItem = lstTables->item(row, dbssSql);
  QString sqlText;
  if (sqlItem)
    sqlText = sqlItem->text();
  // Parse out the table name
  QString table = tableText.left(tableText.find("("));
  assert(pd != 0);
  // create a query builder object
  QgsPgQueryBuilder * pgb = new QgsPgQueryBuilder(table, pd, this);
  // set the current sql in the query builder sql box
  pgb->setSql(sqlText);
  // set the PG connection object so it can be used to fetch the
  // fields for the table, get sample values, and test the query
  pgb->setConnection(pd);
  // show the dialog
  if(pgb->exec())
  {
    // if user accepts, store the sql for the layer so it can be used
    // if and when the layer is added to the map
    if (!sqlItem)
    {
      sqlItem = new QTableWidgetItem();
      lstTables->setItem(row, dbssSql, sqlItem);
    }
    sqlItem->setText(pgb->sql());
  }
  // delete the query builder object
  delete pgb;
}

void QgsDbSourceSelect::addSearchGeometryColumn(const QString &schema, const QString &table, const QString &column)
{
  // store the column details and do the query in a thread
  if (mColumnTypeThread == NULL)
  {
    mColumnTypeThread = new QgsGeomColumnTypeThread();
    mColumnTypeThread->setConnInfo(m_connInfo);
  }
  mColumnTypeThread->addGeometryColumn(schema, table, column);
}

bool QgsDbSourceSelect::getGeometryColumnInfo(PGconn *pg, 
                geomCol& details, bool searchGeometryColumnsOnly,
                                              bool searchPublicOnly)
{
  bool ok = false;

  QApplication::setOverrideCursor(Qt::waitCursor);

  QString sql = "select * from geometry_columns";
  // where f_table_schema ='" + settings.readEntry(key + "/database") + "'";
  sql += " order by f_table_schema,f_table_name";
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
        QString column = PQgetvalue(result, idx, PQfnumber(result, "f_geometry_column"));
        QString type = PQgetvalue(result, idx, PQfnumber(result, "type"));
        
	QString as = "";
	if(type=="GEOMETRY" && !searchGeometryColumnsOnly) {
	  addSearchGeometryColumn(schemaName, tableName,  column);
	  as=type="WAITING";
	}

	details.push_back(geomPair(fullDescription(schemaName, tableName, column, as), type));
      }
      PQclear(exists);
    }
    ok = true;
  }
  PQclear(result);

  QApplication::restoreOverrideCursor();

  if (searchGeometryColumnsOnly)
    return ok;

  // Now have a look for geometry columns that aren't in the
  // geometry_columns table. This code is specific to postgresql,
  // but an equivalent query should be possible in other
  // databases.
  sql = "select pg_class.relname, pg_namespace.nspname, pg_attribute.attname, "
    "pg_class.relkind from "
    "pg_attribute, pg_class, pg_type, pg_namespace where pg_type.typname = 'geometry' and "
    "pg_attribute.atttypid = pg_type.oid and pg_attribute.attrelid = pg_class.oid ";

  if (searchPublicOnly)
    sql += "and pg_namespace.nspname = 'public' ";

  sql += "and cast(pg_class.relname as character varying) not in "
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

    QString table  = PQgetvalue(result, i, 0); // relname
    QString schema = PQgetvalue(result, i, 1); // nspname
    QString column = PQgetvalue(result, i, 2); // attname
    QString relkind = PQgetvalue(result, i, 3); // relation kind

    addSearchGeometryColumn(schema, table, column);
    details.push_back(geomPair(fullDescription(schema, table, column, "WAITING"), "WAITING"));
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
QString QgsDbSourceSelect::fullDescription(QString schema, QString table, 
					   QString column, QString type)
{
  QString full_desc = "";
  if (schema.length() > 0)
    full_desc = '"' + schema + "\".\"";
  full_desc += table + "\" (" + column + ") " + type;
  return full_desc;
}
void QgsDbSourceSelect::dbChanged()
{
  // Remember which database was selected.
  QSettings settings;
  settings.writeEntry("/PostgreSQL/connections/selected", 
		      cmbConnections->currentText());
}

void QgsDbSourceSelect::setConnectionListPosition()
{
  QSettings settings;
  // If possible, set the item currently displayed database
  QString toSelect = settings.readEntry("/PostgreSQL/connections/selected");
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

void QgsGeomColumnTypeThread::setConnInfo(QString s)
{
  mConnInfo = s;
}

void QgsGeomColumnTypeThread::addGeometryColumn(QString schema, QString table, QString column)
{
  schemas.push_back(schema);
  tables.push_back(table);
  columns.push_back(column);
}

void QgsGeomColumnTypeThread::stop()
{
  mStopped=true;
}

void QgsGeomColumnTypeThread::getLayerTypes()
{
  mStopped=false;

  PGconn *pd = PQconnectdb(mConnInfo.toLocal8Bit().data());
  if (PQstatus(pd) == CONNECTION_OK)
  {
    PQsetClientEncoding(pd, "UNICODE");

    for (uint i = 0; i<schemas.size(); i++)
    {
      QString query = QgsDbSourceSelect::makeGeomQuery(schemas[i],
                                                       tables[i],
                                                       columns[i]);
      PGresult* gresult = PQexec(pd, query.toLocal8Bit().data());
      QString type;
      if (PQresultStatus(gresult) == PGRES_TUPLES_OK) {
	QStringList types;

	for(int j=0; j<PQntuples(gresult); j++) {
		QString type = PQgetvalue(gresult, j, 0);
		if(type!="")
		  types += type;
	}

	type = types.join(",");
      }
      PQclear(gresult);

      if(mStopped)
        break;

      // Now tell the layer list dialog box...
      emit setLayerType(schemas[i], tables[i], columns[i], type);
    }
  }

  PQfinish(pd);
}
