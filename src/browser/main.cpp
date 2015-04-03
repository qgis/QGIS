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

#include "qgseditorwidgetregistry.h"

int main( int argc, char ** argv )
{
  QSettings settings;

  QgsApplication a( argc, argv, true );
  // update any saved setting for older themes to new default 'gis' theme (2013-04-15)
  QString theme = settings.value( "/Themes", "default" ).toString();
  if ( theme == QString( "gis" )
       || theme == QString( "classic" )
       || theme == QString( "nkids" ) )
  {
    theme = QString( "default" );
  }
  a.setThemeName( theme );
  a.initQgis();

  // Set up the QSettings environment must be done after qapp is created
  QCoreApplication::setOrganizationName( "QGIS" );
  QCoreApplication::setOrganizationDomain( "qgis.org" );
  QCoreApplication::setApplicationName( "QGIS2" );

#ifdef Q_OS_MACX
  // If the GDAL plugins are bundled with the application and GDAL_DRIVER_PATH
  // is not already defined, use the GDAL plugins in the application bundle.
  QString gdalPlugins( QCoreApplication::applicationDirPath().append( "/lib/gdalplugins" ) );
  if ( QFile::exists( gdalPlugins ) && !getenv( "GDAL_DRIVER_PATH" ) )
  {
    setenv( "GDAL_DRIVER_PATH", gdalPlugins.toUtf8(), 1 );
  }

  // Point GDAL_DATA at any GDAL share directory embedded in the app bundle
  if ( !getenv( "GDAL_DATA" ) )
  {
    QStringList gdalShares;
    QString appResources( QDir::cleanPath( QgsApplication::pkgDataPath() ) );
    gdalShares << QCoreApplication::applicationDirPath().append( "/share/gdal" )
    << appResources.append( "/share/gdal" )
    << appResources.append( "/gdal" );
    Q_FOREACH ( const QString& gdalShare, gdalShares )
    {
      if ( QFile::exists( gdalShare ) )
      {
        setenv( "GDAL_DATA", gdalShare.toUtf8().constData(), 1 );
        break;
      }
    }
  }
#endif

  QgsBrowser w;

  a.connect( &a, SIGNAL( aboutToQuit() ), &w, SLOT( saveWindowState() ) );
  w.restoreWindowState();

  w.show();

  a.connect( &a, SIGNAL( lastWindowClosed() ), &a, SLOT( quit() ) );

  QgsEditorWidgetRegistry::initEditors();

  return a.exec();
}
