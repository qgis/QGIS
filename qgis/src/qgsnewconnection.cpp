#include <qsqldatabase.h>
#include <qsettings.h>
#include <qlineedit.h>
#include <qmessagebox.h>
#include "qgsnewconnection.h"

QgsNewConnection::QgsNewConnection():QgsNewConnectionBase(){
}
QgsNewConnection::~QgsNewConnection(){
}
void QgsNewConnection::testConnection(){
  QSqlDatabase *testCon = QSqlDatabase::addDatabase("QPSQL7","testconnection");
  if(testCon){
    testCon->setDatabaseName(txtDatabase->text());
    testCon->setUserName(txtUsername->text());
    testCon->setPassword(txtPassword->text());
    testCon->setHostName(txtHost->text());
    if ( testCon->open() ) {
      // Database successfully opened; we can now issue SQL commands.
      QMessageBox::information(this,"Test connection","Connection to " + 
			       txtDatabase->text() +  " was successfull");
    }else{
      QMessageBox::information(this,"Test connection",
			       "Connection failed - Check settings and try againto ");
    }


  }
  //  testCon->close();
  //delete testCon;

  // 
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
