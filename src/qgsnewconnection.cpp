/***************************************************************************
                    qgsnewconnection.cpp  -  description
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
#include <qlineedit.h>
#include <qcheckbox.h>
#include <qmessagebox.h>
#include "qgsnewconnection.h"
extern "C"
{
#include <libpq-fe.h>
}
QgsNewConnection::QgsNewConnection(QString connName):QgsNewConnectionBase()
{
  if (!connName.isEmpty())
    {
      // populate the dialog with the information stored for the connection
      // populate the fields with the stored setting parameters
      QSettings settings;

      QString key = "/Qgis/connections/" + connName;
      txtHost->setText(settings.readEntry(key + "/host"));
      txtDatabase->setText(settings.readEntry(key + "/database"));
      QString port = settings.readEntry(key + "/port");
      if(port.length() ==0){
      	port = "5432";
      }
      txtPort->setText(port);
      txtUsername->setText(settings.readEntry(key + "/username"));
      if (settings.readEntry(key + "/save") == "true")
        {
          txtPassword->setText(settings.readEntry(key + "/password"));
          chkStorePassword->setChecked(true);
        }
      txtName->setText(connName);
    }
}

QgsNewConnection::~QgsNewConnection()
{
}
void QgsNewConnection::testConnection()
{
  // following line uses Qt SQL plugin - currently not used
  // QSqlDatabase *testCon = QSqlDatabase::addDatabase("QPSQL7","testconnection");

  QString connInfo =
    "host=" + txtHost->text() + 
    " dbname=" + txtDatabase->text() + 
    " port=" + txtPort->text() +
    " user=" + txtUsername->text() + 
    " password=" + txtPassword->text();
  PGconn *pd = PQconnectdb((const char *) connInfo);
//  std::cout << pd->ErrorMessage();
  if (PQstatus(pd) == CONNECTION_OK)
    {
      // Database successfully opened; we can now issue SQL commands.
      QMessageBox::information(this, tr("Test connection"), tr("Connection to %1 was successfull").arg(txtDatabase->text()));
  } else
    {
      QMessageBox::information(this, tr("Test connection"), tr("Connection failed - Check settings and try again "));
    }
  // free pg connection resources
  PQfinish(pd);


}

void QgsNewConnection::saveConnection()
{
  QSettings settings; 
  QString baseKey = "/Qgis/connections/";
  baseKey += txtName->text();
  settings.writeEntry(baseKey + "/host", txtHost->text());
  settings.writeEntry(baseKey + "/database", txtDatabase->text());
  settings.writeEntry(baseKey + "/port", txtPort->text());
  settings.writeEntry(baseKey + "/username", txtUsername->text());
  settings.writeEntry(baseKey + "/password", txtPassword->text());
  if (chkStorePassword->isChecked())
    {
      settings.writeEntry(baseKey + "/save", "true");
  } else
    {
      settings.writeEntry(baseKey + "/save", "false");
    }
  accept();
}

/* void QgsNewConnection::saveConnection()
{
	QSettings settings;
	QString baseKey = "/Qgis/connections/";
	baseKey += txtName->text();
	settings.writeEntry(baseKey + "/host", txtHost->text());
	settings.writeEntry(baseKey + "/database", txtDatabase->text());

	settings.writeEntry(baseKey + "/username", txtUsername->text());
	if (chkStorePassword->isChecked()) {
		settings.writeEntry(baseKey + "/password", txtPassword->text());
	} else{
        settings.writeEntry(baseKey + "/password", "");
    }

  accept();
} */
