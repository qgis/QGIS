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

  // load providers
#if defined(Q_WS_WIN)
  QString prefixPath = QApplication::applicationDirPath();
#else
  QString prefixPath = QApplication::applicationDirPath() + "/..";
#endif
  a.setPrefixPath( prefixPath, true );
  a.initQgis();

  // Set up the QSettings environment must be done after qapp is created
  QCoreApplication::setOrganizationName( "QuantumGIS" );
  QCoreApplication::setOrganizationDomain( "qgis.org" );
  QCoreApplication::setApplicationName( "QGIS" );

#if 0
  QString myTranslationCode = "";

  // This is mostly copy from Help viewer - not sure if important
#if defined(Q_WS_MACX)
  // If we're on Mac, we have the resource library way above us...
  a.setPkgDataPath( QgsApplication::prefixPath() + "/../../../../" + QString( QGIS_DATA_SUBDIR ) );
#elif defined(Q_WS_WIN)
  a.setPkgDataPath( QgsApplication::prefixPath() + "/" QGIS_DATA_SUBDIR );
#else
  a.setPkgDataPath( QgsApplication::prefixPath() + "/../" QGIS_DATA_SUBDIR );
#endif

  QString i18nPath = QgsApplication::i18nPath();
  if ( myTranslationCode.isEmpty() )
  {
    myTranslationCode = QLocale::system().name();

    QSettings settings;
    if ( settings.value( "locale/overrideFlag", false ).toBool() )
    {
      myTranslationCode = settings.value( "locale/userLocale", "en_US" ).toString();
    }
  }
  QgsDebugMsg( QString( "Setting translation to %1/qgis_%2" ).arg( i18nPath ).arg( myTranslationCode ) );

  /* Translation file for Qt.
   * The strings from the QMenuBar context section are used by Qt/Mac to shift
   * the About, Preferences and Quit items to the Mac Application menu.
   * These items must be translated identically in both qt_ and qgis_ files.
   */

  QTranslator qttor( 0 );
  if ( qttor.load( QString( "qt_" ) + myTranslationCode, i18nPath ) )
  {
    a.installTranslator( &qttor );
  }

  /* Translation file for QGIS.
   */

  QTranslator qgistor( 0 );
  if ( qgistor.load( QString( "qgis_" ) + myTranslationCode, i18nPath ) )
  {
    a.installTranslator( &qgistor );
  }
#endif

  QgsBrowser w;

  a.connect( &a, SIGNAL( aboutToQuit() ), &w, SLOT( saveWindowState() ) );
  w.restoreWindowState();

  w.show();

  a.connect( &a, SIGNAL( lastWindowClosed() ), &a, SLOT( quit() ) );

  return a.exec();
}
