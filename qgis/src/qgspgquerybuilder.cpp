/***************************************************************************
                qgspgquerybuilder.cpp - PostgreSQL Query Builder
                     --------------------------------------
               Date                 : 2004-11-19
               Copyright            : (C) 2004 by Gary E.Sherman
               Email                : sherman at mrcc.com
/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/
#include <iostream>
#include <qlistbox.h>
#include <qmessagebox.h>
#include <qtextedit.h>
#include <qlabel.h>
#include "qgsfield.h"
#include "qgspgquerybuilder.h"

  QgsPgQueryBuilder::QgsPgQueryBuilder(QWidget *parent, const char *name)
: QgsPgQueryBuilderBase(parent, name)
{
}

QgsPgQueryBuilder::QgsPgQueryBuilder(QString tableName, PGconn *con, 
    QWidget *parent, const char *name) : QgsPgQueryBuilderBase(parent, name),
                                         mTableName(tableName), mPgConnection(con)
{
  QString datasource = QString(tr("Table <b>%1</b> in database <b>%2</b> on host <b>%3</b>, user <b>%4</b>"))
    .arg(mTableName)
    .arg(PQdb(mPgConnection))
    .arg(PQhost(mPgConnection))
    .arg(PQuser(mPgConnection));

  lblDataUri->setText(datasource);
  populateFields();
}
QgsPgQueryBuilder::~QgsPgQueryBuilder()
{
}

void QgsPgQueryBuilder::populateFields()
{
  // Populate the field vector for this layer. The field vector contains
  // field name, type, length, and precision (if numeric)
  QString sql = "select * from " + mTableName + " limit 1";
  PGresult *result = PQexec(mPgConnection, (const char *) sql);
  qWarning("Query executed: " + sql);
  if (PQresultStatus(result) == PGRES_TUPLES_OK) 
  {
    //--std::cout << "Field: Name, Type, Size, Modifier:" << std::endl;
    for (int i = 0; i < PQnfields(result); i++) {

      QString fieldName = PQfname(result, i);
      int fldtyp = PQftype(result, i);
      QString typOid = QString().setNum(fldtyp);
      std::cerr << "typOid is: " << typOid << std::endl; 
      int fieldModifier = PQfmod(result, i);
      QString sql = "select typelem from pg_type where typelem = " + typOid + " and typlen = -1";
      //  //--std::cout << sql << std::endl;
      PGresult *oidResult = PQexec(mPgConnection, (const char *) sql);
      if (PQresultStatus(oidResult) == PGRES_TUPLES_OK) 
        std::cerr << "Ok fetching typelem using\n" << sql << std::endl; 

      // get the oid of the "real" type
      QString poid = PQgetvalue(oidResult, 0, PQfnumber(oidResult, "typelem"));
      std::cerr << "poid is: " << poid << std::endl; 
      PQclear(oidResult);
      sql = "select typname, typlen from pg_type where oid = " + poid;
      // //--std::cout << sql << std::endl;
      oidResult = PQexec(mPgConnection, (const char *) sql);
      if (PQresultStatus(oidResult) == PGRES_TUPLES_OK) 
        std::cerr << "Ok fetching typenam,etc\n";

      QString fieldType = PQgetvalue(oidResult, 0, 0);
      QString fieldSize = PQgetvalue(oidResult, 0, 1);
      PQclear(oidResult);
#ifdef QGISDEBUG
      std::cerr << "Field parms: Name = " + fieldName 
        + ", Type = " + fieldType << std::endl; 
#endif
      mFieldMap[fieldName] = QgsField(fieldName, fieldType, 
          fieldSize.toInt(), fieldModifier);
      lstFields->insertItem(fieldName);
    }
  }else
  {
#ifdef QGISDEBUG 
    std::cerr << "Error fetching a row from " + mTableName << std::endl; 
#endif 
  }
  PQclear(result);
}

void QgsPgQueryBuilder::getSampleValues()
{
  QString sql = "select distinct " + lstFields->currentText() 
    + " from " + mTableName + " order by " + lstFields->currentText()
    + " limit 25";
  // clear the values list 
  lstValues->clear();
  // determine the field type
  QgsField field = mFieldMap[lstFields->currentText()];
  bool isCharField = field.type().find("char") > -1;
  PGresult *result = PQexec(mPgConnection, (const char *) sql);

  if (PQresultStatus(result) == PGRES_TUPLES_OK) 
  {
    int rowCount =  PQntuples(result);
    for(int i=0; i < rowCount; i++)
    {
      QString value = PQgetvalue(result, i, 0);
      if(isCharField)
      {
        lstValues->insertItem("'" + value + "'");
      }
      else
      {
        lstValues->insertItem(value);
      }
    }

  }else
  {
    std::cerr << "Failed to get sample of field values\n";
  }
  // free the result set
  PQclear(result);
}

void QgsPgQueryBuilder::getAllValues()
{
  QString sql = "select distinct " + lstFields->currentText() 
    + " from " + mTableName + " order by " + lstFields->currentText();
  // clear the values list 
  lstValues->clear();
  // determine the field type
  QgsField field = mFieldMap[lstFields->currentText()];
  bool isCharField = field.type().find("char") > -1;

  PGresult *result = PQexec(mPgConnection, (const char *) sql);

  if (PQresultStatus(result) == PGRES_TUPLES_OK) 
  {
    int rowCount =  PQntuples(result);
    for(int i=0; i < rowCount; i++)
    {
      QString value = PQgetvalue(result, i, 0);

      if(isCharField)
      {
        lstValues->insertItem("'" + value + "'");
      }
      else
      {
        lstValues->insertItem(value);
      }
    }

  }else
  {
    std::cerr << "Failed to get sample of field values\n";
  }
  // free the result set
  PQclear(result);
}

void QgsPgQueryBuilder::testSql()
{
  // test the sql statement to see if it works
  // by counting the number of records that would be
  // returned
  QString sql = "select count(*) from " + mTableName
    + " where " + txtSQL->text();
  PGresult *result = PQexec(mPgConnection, (const char *)sql);
  if (PQresultStatus(result) == PGRES_TUPLES_OK) 
  {
    QString numRows = PQgetvalue(result, 0, 0);
    //numRows.setNum(rowCount);
    QMessageBox::information(this, tr("Query Result"), tr("The where clause returned ") + numRows + tr(" rows."));
  }
  else
  {
    QMessageBox::warning(this, tr("Query Failed"), tr("An error occurred when executing the query:") + "\n" + QString(PQresultErrorMessage(result)));
  }
  // free the result set
  PQclear(result);
}

void QgsPgQueryBuilder::setConnection(PGconn *con)
{
  // store the connection 
  mPgConnection = con;
}


