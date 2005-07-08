/***************************************************************************
    qgspastetransformations.cpp - set up how source fields are transformed to
                                  destination fields in copy/paste operations
                             -------------------
    begin                : 8 July 2005
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

#include <qsettings.h>

#include "qgspastetransformations.h"

QgsPasteTransformations::QgsPasteTransformations()
  : QgsPasteTransformationsBase()
{
  /*
  if (!connName.isEmpty())
    {
      // populate the dialog with the information stored for the connection
      // populate the fields with the stored setting parameters
      
      QSettings settings;

      QString key = "/Qgis/connections-wms/" + connName;
      txtName->setText     (connName);
      txtUrl->setText      (settings.readEntry(key + "/url"));
      txtProxyHost->setText(settings.readEntry(key + "/proxyhost"));
      txtProxyPort->setText(settings.readEntry(key + "/proxyport"));
    }

  QWidget::setTabOrder(txtName, txtUrl);
  QWidget::setTabOrder(txtUrl, (QWidget*)btnOk);
  QWidget::setTabOrder((QWidget*)btnOk, (QWidget*)btnCancel);
  QWidget::setTabOrder((QWidget*)btnCancel, (QWidget*)btnHelp);
  QWidget::setTabOrder((QWidget*)btnHelp, txtName);
  */
}

QgsPasteTransformations::~QgsPasteTransformations()
{

  // TODO: Save settings here?

/*
  QSettings settings; 
  QString baseKey = "/Qgis/connections-wms/";
  
  baseKey += txtName->text();
  settings.writeEntry(baseKey + "/url", txtUrl->text());
  settings.writeEntry(baseKey + "/proxyhost", txtProxyHost->text());
  settings.writeEntry(baseKey + "/proxyport", txtProxyPort->text());
  
  accept();
*/

}


void QgsPasteTransformations::saveState()
{

  QSettings settings; 
  QString baseKey = "/Qgis/paste-transformations/";

/*
  baseKey += txtName->text();
  settings.writeEntry(baseKey + "/url", txtUrl->text());
  settings.writeEntry(baseKey + "/proxyhost", txtProxyHost->text());
  settings.writeEntry(baseKey + "/proxyport", txtProxyPort->text());
*/

}
