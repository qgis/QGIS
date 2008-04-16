/***************************************************************************
    qgsnewhttpconnection.cpp -  selector for a new HTTP server for WMS, etc.
                             -------------------
    begin                : 3 April 2005
    copyright            : (C) 2005 by Brendan Morley
    email                : morb at ozemail dot com dot au
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
#include "qgsnewhttpconnection.h"
#include "qgscontexthelp.h"
#include <QSettings>

QgsNewHttpConnection::QgsNewHttpConnection(QWidget *parent, const QString& baseKey, const QString& connName, Qt::WFlags fl): QDialog(parent, fl), mBaseKey(baseKey), mOriginalConnName(connName)
{
  setupUi(this);
  connect(btnCancel, SIGNAL(clicked()), this, SLOT(reject()));
  connect(btnOk, SIGNAL(clicked()), this, SLOT(saveConnection()));

  if (!connName.isEmpty())
    {
      // populate the dialog with the information stored for the connection
      // populate the fields with the stored setting parameters
      
      QSettings settings;

      QString key = mBaseKey + connName;
      txtName->setText     (connName);
      txtUrl->setText      (settings.readEntry(key + "/url"));
      txtProxyHost->setText(settings.readEntry(key + "/proxyhost"));
      txtProxyPort->setText(settings.readEntry(key + "/proxyport"));
      txtProxyUser->setText(settings.readEntry(key + "/proxyuser"));
      txtProxyPass->setText(settings.readEntry(key + "/proxypassword"));
    }
}

QgsNewHttpConnection::~QgsNewHttpConnection()
{
}

void QgsNewHttpConnection::testConnection()
{
  // following line uses Qt SQL plugin - currently not used
  // QSqlDatabase *testCon = QSqlDatabase::addDatabase("QPSQL7","testconnection");


}

void QgsNewHttpConnection::saveConnection()
{
  QSettings settings; 
  QString key = mBaseKey + txtName->text();
  
  //delete original entry first
  if(!mOriginalConnName.isNull() && mOriginalConnName != key)
    {
      settings.remove(mBaseKey + mOriginalConnName);
    }
  settings.writeEntry(key + "/url", txtUrl->text().trimmed());
  settings.writeEntry(key + "/proxyhost", txtProxyHost->text().trimmed());
  settings.writeEntry(key + "/proxyport", txtProxyPort->text().trimmed());
  settings.writeEntry(key + "/proxyuser", txtProxyUser->text().trimmed());
  settings.writeEntry(key + "/proxypassword", txtProxyPass->text().trimmed());
  
  accept();
}

void QgsNewHttpConnection::on_btnHelp_clicked()
{
  QgsContextHelp::run(context_id);
}
