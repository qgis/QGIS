/***************************************************************************
                    qgsnewconnection.cpp  -  description
                             -------------------
    begin                : Sat Jun 22 2002
    copyright            : (C) 2002 by Gary E.Sherman
    email                : sherman@mrcc.com
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
#include <qsqldatabase.h>
#include <qsettings.h>
#include <qlineedit.h>
#include <qmessagebox.h>
#include "libpq++.h"
#include "qgsnewconnection.h"

QgsNewConnection::QgsNewConnection(QString connName):QgsNewConnectionBase(){
  if(!connName.isEmpty()){
    // populate the dialog with the information stored for the connection
    // populate the fields with the stored setting parameters
    QSettings settings;

    QString key = "/Qgis/connections/" +connName;
    txtHost->setText(settings.readEntry(key+"/host"));
    txtDatabase->setText(settings.readEntry(key+"/database"));
    txtUsername->setText(settings.readEntry(key+"/username"));
    txtPassword->setText(settings.readEntry(key+"/password"));
    txtName->setText(connName);
  }
}
QgsNewConnection::~QgsNewConnection(){
}
void QgsNewConnection::testConnection(){
  // following line uses Qt SQL plugin - currently not used
  // QSqlDatabase *testCon = QSqlDatabase::addDatabase("QPSQL7","testconnection");

  QString connInfo = "host=" + txtHost->text() +" dbname=" + txtDatabase->text() + " user=" + txtUsername->text() + " password=" + txtPassword->text();
  PgDatabase *pd = new PgDatabase((const char *)connInfo);
std::cout << pd->ErrorMessage();
  if(pd->Status()==CONNECTION_OK){
    // Database successfully opened; we can now issue SQL commands.
    QMessageBox::information(this,"Test connection","Connection to " + 
			     txtDatabase->text() +  " was successfull");
  }else{
    QMessageBox::information(this,"Test connection",
			     "Connection failed - Check settings and try again ");
  }
  delete pd;


}
 


void QgsNewConnection::saveConnection(){
  QSettings settings;
  QString baseKey = "/Qgis/connections/";
  baseKey += txtName->text();
  settings.writeEntry( baseKey + "/host",txtHost->text() );
  settings.writeEntry( baseKey + "/database",txtDatabase->text() );
  settings.writeEntry( baseKey + "/username",txtUsername->text() );
  settings.writeEntry( baseKey + "/password",txtPassword->text() );   


}
