/***************************************************************************
                          qgstipgui.cpp  -  description
                             -------------------
    begin                : Sat Aug 10 2002
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

#include <QSettings>
#include "qgstipgui.h"
#include "qgsapplication.h"
#include <qgstip.h>
#include <qgstipfactory.h>

#ifdef Q_OS_MACX
QgsTipGui::QgsTipGui()
    : QDialog( NULL, Qt::WindowSystemMenuHint )  // Modeless dialog with close button only
#else
QgsTipGui::QgsTipGui()
    : QDialog( NULL )  // Normal dialog in non Mac-OS
#endif
{
  setupUi( this );
  init();
}

QgsTipGui::~QgsTipGui()
{
}

void QgsTipGui::init()
{

  // set the 60x60 icon pixmap
  QPixmap icon( QgsApplication::iconsPath() + "qgis-icon-60x60.png" );
  qgisIcon->setPixmap( icon );
  QgsTipFactory myFactory;
  QgsTip myTip = myFactory.getTip();
  lblTitle->setText(myTip.title());
  txtTip->setHtml(myTip.content());

}

void QgsTipGui::on_cbxDisableTips_toggled(bool theFlag)
{
  QSettings settings;
  //note the ! below as when the cbx is checked (true) we want to 
  //change the setting to false
  settings.setValue( "/qgis/showTips", !theFlag );
  hide();
}
