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
extern "C"
{
  #include <libpq-fe.h>
}

#include "qgsconnectiondialog.h"
#include "qgsmessageviewer.h"

QgsConnectionDialog::QgsConnectionDialog (QWidget* parent, QString connName, bool modal, WFlags fl)
	: QgsConnectionDialogBase(parent,(const char *)connName,modal,fl)
{
	if (!connName.isEmpty()) {
		QSettings settings;
		QString key = "/Qgis/connections/" + connName;
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
	
	QWidget::setTabOrder(txtName, txtHost);
	QWidget::setTabOrder(txtHost, txtDatabase);
	QWidget::setTabOrder(txtDatabase, txtPort);
	QWidget::setTabOrder(txtPort, txtUsername);
	QWidget::setTabOrder(txtUsername, txtPassword);
	QWidget::setTabOrder(txtPassword, chkStorePassword);
	QWidget::setTabOrder(chkStorePassword, (QWidget*)btnConnect);
	QWidget::setTabOrder((QWidget*)btnConnect, (QWidget*)btnOk);
	QWidget::setTabOrder((QWidget*)btnOk, (QWidget*)btnCancel);
	QWidget::setTabOrder((QWidget*)btnCancel, (QWidget*)btnHelp);
	QWidget::setTabOrder((QWidget*)btnHelp, txtName);
}

QgsConnectionDialog::~QgsConnectionDialog()
{

}


void QgsConnectionDialog::testConnection()
{
	QString connInfo = "host=" + txtHost->text() + " dbname=" + txtDatabase->text() + 
		" port=" + txtPort->text() + " user=" + txtUsername->text() + " password=" + txtPassword->text();
  PGconn *pd = PQconnectdb((const char *) connInfo);

	if (PQstatus(pd) == CONNECTION_OK) {
		// Database successfully opened; we can now issue SQL commands.
		QMessageBox::information(this, "Test connection", "Connection to " + txtDatabase->text() + " was successfull");
	} else {
		QMessageBox::information(this, "Test connection", "Connection failed - Check settings and try again ");
	}
 
  PQfinish(pd);
}

void QgsConnectionDialog::saveConnection()
{
	QSettings settings;
	QString baseKey = "/Qgis/connections/";
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
  QString message = "General Interface Help:\n\n";
  QgsMessageViewer * e = new QgsMessageViewer(this, "HelpMessage");
  e->setMessage(message);
  e->exec();
}
