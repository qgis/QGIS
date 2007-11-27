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
 
// $Id$

#include <iostream>

#include <QSettings>
#include <QMessageBox>

extern "C"
{
  #include <libpq-fe.h>
}

#include "qgsconnectiondialog.h"
#include "qgsmessageviewer.h"
#include "qgsdatasourceuri.h"

QgsConnectionDialog::QgsConnectionDialog(QWidget *parent, const QString& connName, Qt::WFlags fl)
	: QDialog(parent, fl)
{
    setupUi(this);
	if (!connName.isEmpty()) {
		QSettings settings;
		QString key = "/PostgreSQL/connections/" + connName;
		txtHost->setText(settings.readEntry(key + "/host"));
		txtDatabase->setText(settings.readEntry(key + "/database"));
		if(settings.readEntry(key + "/port").length() ==0){
			txtPort->setText("5432");
		}
		else {
			txtPort->setText(settings.readEntry(key + "/port"));
		}
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
  QgsDataSourceURI uri;
  uri.setConnection( txtHost->text(), txtPort->text(), txtDatabase->text(), txtUsername->text(), txtPassword->text() );
  PGconn *pd = PQconnectdb((const char *) uri.connInfo() );

  if (PQstatus(pd) == CONNECTION_OK) {
    // Database successfully opened; we can now issue SQL commands.
    QMessageBox::information(this, tr("Test connection"), tr("Connection to ") + txtDatabase->text() + tr(" was successfull"));
  } else {
    QMessageBox::information(this, tr("Test connection"), tr("Connection failed - Check settings and try again "));
  }

  PQfinish(pd);
}

void QgsConnectionDialog::saveConnection()
{
	QSettings settings;
	QString baseKey = "/PostgreSQL/connections/";
	baseKey += txtName->text();
	settings.writeEntry(baseKey + "/host", txtHost->text());
	settings.writeEntry(baseKey + "/database", txtDatabase->text());
	settings.writeEntry(baseKey + "/port", txtPort->text());
	settings.writeEntry(baseKey + "/username", txtUsername->text());
	settings.writeEntry(baseKey + "/password", txtPassword->text());
  if(chkStorePassword->isChecked())  settings.writeEntry(baseKey + "/save", "true");
  else settings.writeEntry(baseKey + "/save", "false");
  accept();
}

void QgsConnectionDialog::helpInfo(){
  QString message = tr("General Interface Help:\n\n");
  QgsMessageViewer * e = new QgsMessageViewer(this);
  e->setMessageAsPlainText(message);
  e->exec();  // deletes itself on close
}
