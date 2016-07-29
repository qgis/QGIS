/***************************************************************************
                             main.cpp
                             Helpviewer main method
                             -------------------
    begin                : 2007
    copyright            : (C) 2007 by Gary E. Sherman
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
#include <QLocale>
#include <QSettings>
#include <QTranslator>
#include <QLibraryInfo>

#include "qgshelpviewer.h"
#include "qgsapplication.h"
#include "qgslogger.h"
#include "qgsconfig.h"

int main( int argc, char ** argv )
{
  QgsApplication a( argc, argv, true );

  // Set up the QSettings environment must be done after qapp is created
  QCoreApplication::setOrganizationName( "QGIS" );
  QCoreApplication::setOrganizationDomain( "qgis.org" );
  QCoreApplication::setApplicationName( "QGIS2" );

  QString myTranslationCode = "";

  if ( argc == 2 )
  {
    myTranslationCode = argv[1];
  }

  if ( !QgsApplication::isRunningFromBuildDir() )
  {
#if defined(Q_OS_MACX)
    // If we're on Mac, we have the resource library way above us...
    a.setPkgDataPath( QgsApplication::prefixPath() + "/../../../../" + QString( QGIS_DATA_SUBDIR ) );
#elif defined(Q_OS_WIN)
    a.setPkgDataPath( QgsApplication::prefixPath() + "/" QGIS_DATA_SUBDIR );
#else
    a.setPkgDataPath( QgsApplication::prefixPath() + "/../" QGIS_DATA_SUBDIR );
#endif
  }

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
  QgsDebugMsg( QString( "Setting translation to %1/qgis_%2" ).arg( i18nPath, myTranslationCode ) );

  /* Translation file for QGIS.
   */
  QTranslator qgistor( nullptr );
  if ( qgistor.load( QString( "qgis_" ) + myTranslationCode, i18nPath ) )
  {
    a.installTranslator( &qgistor );
  }

  /* Translation file for Qt.
   * The strings from the QMenuBar context section are used by Qt/Mac to shift
   * the About, Preferences and Quit items to the Mac Application menu.
   * These items must be translated identically in both qt_ and qgis_ files.
   */
  QTranslator qttor( nullptr );
  if ( qttor.load( QString( "qt_" ) + myTranslationCode, QLibraryInfo::location( QLibraryInfo::TranslationsPath ) ) )
  {
    a.installTranslator( &qttor );
  }

  QgsHelpViewer w;
  a.exec();
}
