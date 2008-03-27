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
#include "qgsdatasourceuri.h"

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

  mSearchModeComboBox->addItem(tr("Wildcard"));
  mSearchModeComboBox->addItem(tr("RegExp"));

  mSearchColumnComboBox->addItem(tr("All"));
  mSearchColumnComboBox->addItem(tr("Schema"));
  mSearchColumnComboBox->addItem(tr("Table"));
  mSearchColumnComboBox->addItem(tr("Type"));
  mSearchColumnComboBox->addItem(tr("Geometry column"));
  mSearchColumnComboBox->addItem(tr("Sql"));

  mProxyModel.setParent(this);
  mProxyModel.setFilterKeyColumn(-1);
  mProxyModel.setFilterCaseSensitivity(Qt::CaseInsensitive);
  mProxyModel.setDynamicSortFilter(true);
  mProxyModel.setSourceModel(&mTableModel);
  mTablesTreeView->setModel(&mProxyModel);
  mTablesTreeView->setSortingEnabled(true);

  mSearchGroupBox->hide();
  connect(mTablesTreeView, SIGNAL(doubleClicked(const QModelIndex&)), this, SLOT(setSql(const QModelIndex&)));

  //for Qt < 4.3.2, passing -1 to include all model columns
  //in search does not seem to work
  mSearchColumnComboBox->setCurrentIndex(2);
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

void QgsDbSourceSelect::on_mSearchOptionsButton_clicked()
{
  if(mSearchGroupBox->isVisible())
  {
    mSearchGroupBox->hide();
  }
  else
  {
    mSearchGroupBox->show();
  }
}

void QgsDbSourceSelect::on_mSearchTableEdit_textChanged(const QString & text)
{
  if(mSearchModeComboBox->currentText() == tr("Wildcard"))
  {
    mProxyModel._setFilterWildcard(text);
  }
  else if(mSearchModeComboBox->currentText() == tr("RegExp"))
  {
    mProxyModel._setFilterRegExp(text);
  }
}

void QgsDbSourceSelect::on_mSearchColumnComboBox_currentIndexChanged(const QString & text)
{
  if(text == tr("All"))
  {
    mProxyModel.setFilterKeyColumn(-1);
  }
  else if(text == tr("Schema"))
  {
    mProxyModel.setFilterKeyColumn(0);
  }
  else if(text == tr("Table"))
  {
    mProxyModel.setFilterKeyColumn(1);
  }
  else if(text == tr("Type"))
  {
    mProxyModel.setFilterKeyColumn(2);
  }
  else if(text == tr("Geometry column"))
  {
    mProxyModel.setFilterKeyColumn(3);
  }
  else if(text == tr("Sql"))
  {
    mProxyModel.setFilterKeyColumn(4);
  }
}

void QgsDbSourceSelect::on_mSearchModeComboBox_currentIndexChanged(const QString & text)
{
  on_mSearchTableEdit_textChanged(mSearchTableEdit->text());
}

void QgsDbSourceSelect::setLayerType(QString schema, 
                                     QString table, QString column,
                                     QString type)
{
  mTableModel.setGeometryTypesForTable(schema, table, column, type);
  mTablesTreeView->sortByColumn(1, Qt::AscendingOrder);
  mTablesTreeView->sortByColumn(0, Qt::AscendingOrder);
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
    "from \"%2\".\"%3\"").arg("\""+column+"\"").arg(schema).arg(table);
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
  m_selectedTables.clear();

  typedef QMap<int, QVector<QString> > schemaInfo;
  QMap<QString, schemaInfo> dbInfo;

  QItemSelection selection = mTablesTreeView->selectionModel()->selection();
  QModelIndexList selectedIndices = selection.indexes();
  QStandardItem* currentItem = 0;

  QModelIndexList::const_iterator selected_it = selectedIndices.constBegin();
  for(; selected_it != selectedIndices.constEnd(); ++selected_it)
  {
    if(!selected_it->parent().isValid())
    {
      //top level items only contain the schema names
      continue;
    }
    currentItem = mTableModel.itemFromIndex(mProxyModel.mapToSource(*selected_it));
    if(!currentItem)
    {
      continue;
    }

    QString currentSchemaName = currentItem->parent()->text();

    int currentRow = currentItem->row();
    int currentColumn = currentItem->column();

    if(dbInfo[currentSchemaName][currentRow].size() == 0)
    {
      dbInfo[currentSchemaName][currentRow].resize(5);
    }

    dbInfo[currentSchemaName][currentRow][currentColumn] = currentItem->text();
  }

  //now traverse all the schemas and table infos
  QString schemaName, tableName, geomColumnName, sql;
  QString query;

  QMap<QString, schemaInfo>::const_iterator schema_it = dbInfo.constBegin();
  for(; schema_it != dbInfo.constEnd(); ++schema_it)
  {
    schemaInfo scheme = schema_it.value();
    schemaInfo::const_iterator entry_it = scheme.constBegin();
    for(; entry_it != scheme.constEnd(); ++entry_it)
    {
      schemaName = entry_it->at(0);
      tableName = entry_it->at(1);
      geomColumnName = entry_it->at(3);
      sql = entry_it->at(4);

      if(geomColumnName.contains(" AS "))
      {
        int a = geomColumnName.find(" AS ");
        QString typeName = geomColumnName.mid(a+4); //only the type name
        geomColumnName = geomColumnName.mid(0, a); //only the geom column name

        if(!sql.isEmpty())
        {
          sql += " AND ";
        }
        if( typeName=="POINT" ) 
        {
          sql += QString("GeometryType(\"%1\") IN ('POINT','MULTIPOINT')").arg(geomColumnName);
        } 
        else if(typeName=="LINESTRING") 
        {
          sql += QString("GeometryType(\"%1\") IN ('LINESTRING','MULTILINESTRING')").arg(geomColumnName);
        } 
        else if(typeName=="POLYGON") 
        {
          sql += QString("GeometryType(\"%1\") IN ('POLYGON','MULTIPOLYGON')").arg(geomColumnName);
        } 
        else 
        {
          continue;
        }
      }
      query = "\"" + schemaName + "\".\"" + tableName + "\" " + "(" + geomColumnName + ") sql=" + sql;
      m_selectedTables.push_back(query);
    }
  }

  if(m_selectedTables.empty())
  {
    QMessageBox::information(this, tr("Select Table"), tr("You must select a table in order to add a Layer."));
  }
  else
  {
    accept();
  }
}

void QgsDbSourceSelect::on_btnConnect_clicked()
{
  if(mColumnTypeThread)
  {
    mColumnTypeThread->stop();
    mColumnTypeThread=0;
  }

  QModelIndex rootItemIndex = mTableModel.indexFromItem(mTableModel.invisibleRootItem());
  mTableModel.removeRows(0, mTableModel.rowCount(rootItemIndex), rootItemIndex);

  // populate the table list
  QSettings settings;

  bool makeConnection = true;
  QString key = "/PostgreSQL/connections/" + cmbConnections->currentText();

  QString database = settings.readEntry(key + "/database");
  QString username = settings.readEntry(key + "/username");
  QString password = settings.readEntry(key + "/password");

  if ( password.isEmpty() )
  {
    // get password from user 
    makeConnection = false;
    password = QInputDialog::getText(tr("Password for ") + username,
      tr("Please enter your password:"),
      QLineEdit::Password, QString::null, &makeConnection, this);
    // allow null password entry in case its valid for the database
  }

  QgsDataSourceURI uri;
  uri.setConnection( settings.readEntry(key + "/host"),
    settings.readEntry(key + "/port"),
    database,
    settings.readEntry(key + "/username"),
    password );

  bool searchPublicOnly = settings.readBoolEntry(key + "/publicOnly");
  bool searchGeometryColumnsOnly = settings.readBoolEntry(key + "/geometryColumnsOnly");

  // Need to escape the password to allow for single quotes and backslashes

  QgsDebugMsg("Connection info: " + uri.connInfo());

  if (makeConnection)
  {
    m_connInfo = uri.connInfo();
    //qDebug(m_connInfo);
    // Tidy up an existing connection if one exists.
    if (pd != 0)
      PQfinish(pd);

    pd = PQconnectdb(m_connInfo.toLocal8Bit());		// use what is set based on locale; after connecting, use Utf8
    //  std::cout << pd->ErrorMessage();
    if (PQstatus(pd) == CONNECTION_OK)
    {
      //qDebug("Connection succeeded");
      // tell the DB that we want text encoded in UTF8
      PQsetClientEncoding(pd, QString("UNICODE").toLocal8Bit());

      // get the list of suitable tables and columns and populate the UI
      geomCol details;

      if(getTableInfo(pd, searchGeometryColumnsOnly, searchPublicOnly))	
      {
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
        arg(settings.readEntry(key + "/database")).arg(settings.readEntry(key + "/host")).arg("\n\n").arg("\n\n").arg("\n").arg(QString::fromLocal8Bit(PQerrorMessage(pd))));
    }
  }

  mTablesTreeView->sortByColumn(1, Qt::AscendingOrder);
  mTablesTreeView->sortByColumn(0, Qt::AscendingOrder);

  //if we have only one schema item, expand it by default
  int numTopLevelItems = mTableModel.invisibleRootItem()->rowCount();
  if(numTopLevelItems < 2 || mTableModel.tableCount() < 20)
  {
    //expand all the toplevel items
    for(int i = 0; i < numTopLevelItems; ++i)
    {
      mTablesTreeView->expand(mProxyModel.mapFromSource(mTableModel.indexFromItem(mTableModel.invisibleRootItem()->child(i))));
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

void QgsDbSourceSelect::setSql(const QModelIndex& index)
{
  if(!index.parent().isValid())
  {
    qWarning("schema item found");
    return;
  }

  if(pd == 0)
  {
    return;
  }

  //create "Schema"."Table" and find out existing sql string
  QModelIndex schemaSibling = index.sibling(index.row(), 0);
  QModelIndex tableSibling = index.sibling(index.row(), 1);
  if(!schemaSibling.isValid() || !tableSibling.isValid())
  {
    return;
  }

  QString schemaName = mTableModel.itemFromIndex(mProxyModel.mapToSource(schemaSibling))->text();
  QString tableName = mTableModel.itemFromIndex(mProxyModel.mapToSource(tableSibling))->text(); 
  QString tableString = "\"" + schemaName + "\".\"" + tableName + "\"";
  qWarning(tableString);

  QString currentSql;
  QModelIndex sqlSibling = index.sibling(index.row(), 4);
  if(sqlSibling.isValid())
  {
    currentSql = mTableModel.itemFromIndex(mProxyModel.mapToSource(sqlSibling))->text();
  }

  // create a query builder object
  QgsPgQueryBuilder * pgb = new QgsPgQueryBuilder(tableString, pd, this); 
  // set the current sql in the query builder sql box
  pgb->setSql(currentSql);
  // set the PG connection object so it can be used to fetch the
  // fields for the table, get sample values, and test the query
  pgb->setConnection(pd);
  // show the dialog
  if(pgb->exec())
  {
    mTableModel.setSql(mProxyModel.mapToSource(index), pgb->sql());
  }
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

bool QgsDbSourceSelect::getTableInfo(PGconn *pg, bool searchGeometryColumnsOnly, bool searchPublicOnly)
{
  bool ok = false;
  QApplication::setOverrideCursor(Qt::waitCursor);

  // The following query returns only tables that exist and the user has SELECT privilege on.
  // Can't use regclass here because table must exist, else error occurs.
  QString sql = "select * from geometry_columns,pg_class,pg_namespace "
    "where relname=f_table_name and f_table_schema=nspname "
    "and pg_namespace.oid = pg_class.relnamespace "
    "and has_schema_privilege(pg_namespace.nspname,'usage') "
    "and has_table_privilege('\"'||pg_namespace.nspname||'\".\"'||pg_class.relname||'\"','select') "	// user has select privilege
    "order by f_table_schema,f_table_name";

  PGresult *result = PQexec(pg, sql.toUtf8());
  if (result)
  {
    if( PQresultStatus(result) != PGRES_TUPLES_OK) 
    {
      QMessageBox::warning(this,
                           tr("Accessible tables could not be determined"),
                           QString ( tr("Database connection was successful, but the accessible tables could not be determined.\n\n"
                                      "The error message from the database was:\n%1\n" ) )
                              .arg( QString::fromUtf8(PQresultErrorMessage(result)) ) );
    }
    else if( PQntuples(result)==0 )
    {
      QMessageBox::warning(this, tr("No accessible tables found"),
        tr
        ("Database connection was successful, but no accessible tables were found.\n\n"
         "Please verify that you have SELECT privilege on a table carrying PostGIS\n"
         "geometry."));
    }
    else 
    {
      for (int idx = 0; idx < PQntuples(result); idx++)
      {
        QString tableName = QString::fromUtf8(PQgetvalue(result, idx, PQfnumber(result, QString("f_table_name").toUtf8())));
        QString schemaName = QString::fromUtf8(PQgetvalue(result, idx, PQfnumber(result, QString("f_table_schema").toUtf8())));

        QString column = QString::fromUtf8(PQgetvalue(result, idx, PQfnumber(result, QString("f_geometry_column").toUtf8())));
        QString type = QString::fromUtf8(PQgetvalue(result, idx, PQfnumber(result, QString("type").toUtf8())));

        QString as = "";
        if(type=="GEOMETRY" && !searchGeometryColumnsOnly) 
        {
          addSearchGeometryColumn(schemaName, tableName,  column);
          as=type="WAITING";
        }

        mTableModel.addTableEntry(type, schemaName, tableName, column, "");
      }
    }
    ok = true;
  }
  PQclear(result);

  //search for geometry columns in tables that are not in the geometry_columns metatable
  QApplication::restoreOverrideCursor();
  if (searchGeometryColumnsOnly)
  {
    return ok;
  }

  // Now have a look for geometry columns that aren't in the
  // geometry_columns table. This code is specific to postgresql,
  // but an equivalent query should be possible in other
  // databases.
  sql = "select pg_class.relname,pg_namespace.nspname,pg_attribute.attname,pg_class.relkind "
    "from pg_attribute, pg_class, pg_namespace "
    "where pg_namespace.oid = pg_class.relnamespace "
    "and pg_attribute.atttypid = regtype('geometry') "
    "and pg_attribute.attrelid = pg_class.oid "
    "and has_schema_privilege(pg_namespace.nspname,'usage') "
    "and has_table_privilege('\"'||pg_namespace.nspname||'\".\"'||pg_class.relname||'\"','select') ";
  // user has select privilege
  if (searchPublicOnly)
    sql += "and pg_namespace.nspname = 'public' ";

  sql += "and pg_namespace.nspname||'.'||pg_class.relname not in "	//  needs to be table and schema
    "(select f_table_schema||'.'||f_table_name from geometry_columns) "
    "and pg_class.relkind in ('v', 'r')"; // only from views and relations (tables)

  result = PQexec(pg, sql.toUtf8());

  for (int i = 0; i < PQntuples(result); i++)
  {
    // Have the column name, schema name and the table name. The concept of a
    // catalog doesn't exist in postgresql so we ignore that, but we
    // do need to get the geometry type.

    // Make the assumption that the geometry type for the first
    // row is the same as for all other rows. 

    QString table  = QString::fromUtf8(PQgetvalue(result, i, 0)); // relname
    QString schema = QString::fromUtf8(PQgetvalue(result, i, 1)); // nspname
    QString column = QString::fromUtf8(PQgetvalue(result, i, 2)); // attname
    QString relkind = QString::fromUtf8(PQgetvalue(result, i, 3)); // relation kind

    addSearchGeometryColumn(schema, table, column);
    //details.push_back(geomPair(fullDescription(schema, table, column, "WAITING"), "WAITING"));
    mTableModel.addTableEntry("Waiting", schema, table, column, "");
  }
  ok = true;

  PQclear(result);
  return ok;
}

#if 0	// this function is never called - smizuno
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
  PGresult *result = PQexec(pg, sql.toUtf8());
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
      QString tableName = QString::fromUtf8(PQgetvalue(result, idx, PQfnumber(result, "f_table_name")));
      QString schemaName = QString::fromUtf8(PQgetvalue(result, idx, PQfnumber(result, "f_table_schema")));
      sql = "select oid from pg_class where relname = '" + tableName + "'";
      if (schemaName.length() > 0)
        sql +=" and relnamespace = (select oid from pg_namespace where nspname = '" +
        schemaName + "')";

      PGresult* exists = PQexec(pg, sql.toUtf8());
      if (PQntuples(exists) == 1)
      {
        QString column = QString::fromUtf8(PQgetvalue(result, idx, PQfnumber(result, "f_geometry_column")));
        QString type = QString::fromUtf8(PQgetvalue(result, idx, PQfnumber(result, "type")));

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

  result = PQexec(pg, sql.toUtf8());

  for (int i = 0; i < PQntuples(result); i++)
  {
    // Have the column name, schema name and the table name. The concept of a
    // catalog doesn't exist in postgresql so we ignore that, but we
    // do need to get the geometry type.

    // Make the assumption that the geometry type for the first
    // row is the same as for all other rows. 

    QString table  = QString::fromUtf8(PQgetvalue(result, i, 0)); // relname
    QString schema = QString::fromUtf8(PQgetvalue(result, i, 1)); // nspname
    QString column = QString::fromUtf8(PQgetvalue(result, i, 2)); // attname
    QString relkind = QString::fromUtf8(PQgetvalue(result, i, 3)); // relation kind

    addSearchGeometryColumn(schema, table, column);
    details.push_back(geomPair(fullDescription(schema, table, column, "WAITING"), "WAITING"));
  }
  ok = true;

  PQclear(result);

  return ok;
}
#endif

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

void QgsDbSourceSelect::setSearchExpression(const QString& regexp)
{
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

  PGconn *pd = PQconnectdb(mConnInfo.toLocal8Bit());
  if (PQstatus(pd) == CONNECTION_OK)
  {
    PQsetClientEncoding(pd, QString("UNICODE").toLocal8Bit());

    for (uint i = 0; i<schemas.size(); i++)
    {
      QString query = QgsDbSourceSelect::makeGeomQuery(schemas[i],
        tables[i],
        columns[i]);
      PGresult* gresult = PQexec(pd, query.toUtf8());
      QString type;
      if (PQresultStatus(gresult) == PGRES_TUPLES_OK) {
        QStringList types;

        for(int j=0; j<PQntuples(gresult); j++) {
          QString type = QString::fromUtf8(PQgetvalue(gresult, j, 0));
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
