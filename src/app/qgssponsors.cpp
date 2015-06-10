/***************************************************************************
                          qgssponsors.cpp  -  description
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

#include "qgssponsors.h"
#include "qgsapplication.h"
#include <QFile>
#include <QTextStream>

#ifdef Q_OS_MACX
QgsSponsors::QgsSponsors( QWidget *parent )
    : QDialog( parent, Qt::WindowSystemMenuHint )  // Modeless dialog with close button only
#else
QgsSponsors::QgsSponsors( QWidget *parent )
    : QDialog( parent )  // Normal dialog in non Mac-OS
#endif
{
  setupUi( this );
  init();
}

QgsSponsors::~QgsSponsors()
{
}

void QgsSponsors::init()
{

  // set the 60x60 icon pixmap
  QPixmap icon( QgsApplication::iconsPath() + "qgis-icon-60x60.png" );
  qgisIcon->setPixmap( icon );

}
