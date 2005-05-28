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
#include <iostream>
#include <qsettings.h>
#include <qlineedit.h>
#include <qcheckbox.h>
#include <qmessagebox.h>
#include "qgsnewhttpconnection.h"

QgsNewHttpConnection::QgsNewHttpConnection(QString connName)
                    : QgsNewHttpConnectionBase()
{
  if (!connName.isEmpty())
    {
      // populate the dialog with the information stored for the connection
      // populate the fields with the stored setting parameters
      
      QSettings settings;

      QString key = "/Qgis/connections-wms/" + connName;
      txtUrl->setText(settings.readEntry(key + "/url"));
      txtName->setText(connName);
    }

  QWidget::setTabOrder(txtName, txtUrl);
  QWidget::setTabOrder(txtUrl, (QWidget*)btnOk);
  QWidget::setTabOrder((QWidget*)btnOk, (QWidget*)btnCancel);
  QWidget::setTabOrder((QWidget*)btnCancel, (QWidget*)btnHelp);
  QWidget::setTabOrder((QWidget*)btnHelp, txtName);
  
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
  QString baseKey = "/Qgis/connections-wms/";
  
  baseKey += txtName->text();
  settings.writeEntry(baseKey + "/url", txtUrl->text());
  
  accept();
}

