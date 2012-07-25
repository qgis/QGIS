/***************************************************************************
                             main.cpp
                             Browser main method
                             -------------------
    begin                : 2011-04-01
    copyright            : (C) 2011 Radim Blazek
    email                : radim dot blazek at gmail dot com
***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/
#include <QLocale>
#include <QSettings>
#include <QTranslator>
#include <QMainWindow>
#include <QLabel>
#include <QDialog>
#include <QApplication>
#include "qgsbrowser.h"
#include "qgsapplication.h"
#include "qgslogger.h"
#include "qgsconfig.h"
#include <qmainwindow.h>

int main( int argc, char ** argv )
{
  QSettings settings;

  QgsApplication a( argc, argv, true );
  a.setThemeName( settings.value( "/Themes", "default" ).toString() );
  a.initQgis();

  // Set up the QSettings environment must be done after qapp is created
  QCoreApplication::setOrganizationName( "QuantumGIS" );
  QCoreApplication::setOrganizationDomain( "qgis.org" );
  QCoreApplication::setApplicationName( "QGIS" );

  QgsBrowser w;

  a.connect( &a, SIGNAL( aboutToQuit() ), &w, SLOT( saveWindowState() ) );
  w.restoreWindowState();

  w.show();

  a.connect( &a, SIGNAL( lastWindowClosed() ), &a, SLOT( quit() ) );

  return a.exec();
}
