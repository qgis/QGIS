/***************************************************************************
                          qgsconnectiondialog.cpp  -  description
                             -------------------
    begin                : Thu Dec 10 2003
    copyright            : (C) 2003 by Denis Antipov
    email                : 
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/
 
#include <iostream>
#include <qsettings.h>
#include <qlineedit.h>
#include <qcheckbox.h>
#include <qmessagebox.h>
#include <libpq++.h>
#include "qgsconnectiondialog.h"

QgsConnectionDialog::QgsConnectionDialog(QString connName):QgsConnectionDialogBase()
{
	if (!connName.isEmpty()) {
		QSettings settings;
		QString key = "/Qgis/connections/" + connName;
		txtHost->setText(settings.readEntry(key + "/host"));
		txtDatabase->setText(settings.readEntry(key + "/database"));
		txtUsername->setText(settings.readEntry(key + "/username"));
    if(settings.readEntry(key + "/save") == "true"){
      txtPassword->setText(settings.readEntry(key + "/password"));
      chkStorePassword->setChecked(true);
    }
		txtName->setText(connName);
	}
}

QgsConnectionDialog::~QgsConnectionDialog()
{

}


void QgsConnectionDialog::testConnection()
{
	QString connInfo = "host=" + txtHost->text() + " dbname=" + txtDatabase->text() +
	  " user=" + txtUsername->text() + " password=" + txtPassword->text();
	PgDatabase *pd = new PgDatabase((const char *) connInfo);

	if (pd->Status() == CONNECTION_OK) {
		// Database successfully opened; we can now issue SQL commands.
		QMessageBox::information(this, "Test connection", "Connection to " + txtDatabase->text() + " was successfull");
	} else {
		QMessageBox::information(this, "Test connection", "Connection failed - Check settings and try again ");
	}
  
	delete pd;
}

void QgsConnectionDialog::saveConnection()
{
	QSettings settings;
	QString baseKey = "/Qgis/connections/";
	baseKey += txtName->text();
	settings.writeEntry(baseKey + "/host", txtHost->text());
	settings.writeEntry(baseKey + "/database", txtDatabase->text());

	settings.writeEntry(baseKey + "/username", txtUsername->text());
	settings.writeEntry(baseKey + "/password", txtPassword->text());
  if(chkStorePassword->isChecked())  settings.writeEntry(baseKey + "/save", "true");
  else settings.writeEntry(baseKey + "/save", "false");
  accept();
}
