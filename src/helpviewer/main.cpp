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
#include "qgshelpserver.h"
#include "qgshelpviewer.h"
#include "qgsapplication.h"
#include "qgslogger.h"
#include "qgsconfig.h"

int main( int argc, char ** argv )
{
  QgsApplication a( argc, argv, true );

  // Set up the QSettings environment must be done after qapp is created
  QCoreApplication::setOrganizationName( "QuantumGIS" );
  QCoreApplication::setOrganizationDomain( "qgis.org" );
  QCoreApplication::setApplicationName( "QGIS" );

  QString context = QString::null;
  QString myTranslationCode = "";

  if ( argc == 2 )
  {
    context = argv[1];
  }

  if ( !QgsApplication::isRunningFromBuildDir() )
  {
#if defined(Q_WS_MACX)
    // If we're on Mac, we have the resource library way above us...
    a.setPkgDataPath( QgsApplication::prefixPath() + "/../../../../" + QString( QGIS_DATA_SUBDIR ) );
#elif defined(Q_WS_WIN)
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

  QgsHelpViewer w( context );
  w.show();

  a.connect( &a, SIGNAL( lastWindowClosed() ), &a, SLOT( quit() ) );

  // Create socket for client to send context requests to.
  // This allows an existing viewer to be reused rather then creating
  // an additional viewer if one is already running.
  QgsHelpContextServer *helpServer = new QgsHelpContextServer();
  // Make port number available to client
  std::cout << helpServer->serverPort() << std::endl;
  // Pass context request from socket to viewer widget
  QObject::connect( helpServer, SIGNAL( setContext( const QString& ) ),
                    &w, SLOT( setContext( const QString& ) ) );

  return a.exec();
}
