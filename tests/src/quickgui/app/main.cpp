/***************************************************************************
  main.cpp
  --------------------------------------
  Date                 : Nov 2017
  Copyright            : (C) 2017 by Peter Petrik
  Email                : zilolv at gmail dot com
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include <QCommandLineParser>
#include <QQmlApplicationEngine>
#include <QQmlComponent>
#include <QQmlContext>
#include <QQmlError>

#include "qgis.h"
#include "qgsapplication.h"
#include "qgsproject.h"
#include "qgslayertree.h"
#include "qgsmessagelog.h"
#include "qgslogger.h"

int main( int argc, char *argv[] )
{
  // 1) Initialize QGIS
  QgsApplication app( argc, argv, true );

  // Set up the QSettings environment must be done after qapp is created
  QCoreApplication::setOrganizationName( "QGIS" );
  QCoreApplication::setOrganizationDomain( "qgis.org" );
  QCoreApplication::setApplicationName( "QgsQuick Test App" );
  QCoreApplication::setApplicationVersion( Qgis::version() );

  QCommandLineParser parser;
  parser.addVersionOption();
  parser.process( app );

  QgsApplication::init();
  QgsApplication::initQgis();

  // 2) Load QGIS Project
  QString dataDir( TEST_DATA_DIR ); // defined in CMakeLists.txt
  QString projectFile = dataDir + "/quickapp_project.qgs";
  QgsDebugMsgLevel( QStringLiteral( "project file:  %1" ).arg( projectFile ), 2 );
  QgsProject project;
  bool res = project.read( projectFile );
  Q_ASSERT( res );

  QQmlEngine engine;
  engine.addImportPath( QgsApplication::qmlImportPath() );
  engine.rootContext()->setContextProperty( "__project", &project );
  engine.rootContext()->setContextProperty( "__layers", QVariant::fromValue( project.layerTreeRoot()->layerOrder() ) );

  QQmlComponent component( &engine, QUrl( QStringLiteral( "qrc:/main.qml" ) ) );
  QObject *object = component.create();

  if ( !component.errors().isEmpty() )
  {
    QgsDebugError( QStringLiteral( "%s" ).arg( QgsApplication::showSettings().toLocal8Bit().data() ) );

    QgsDebugError( QStringLiteral( "****************************************" ) );
    QgsDebugError( QStringLiteral( "*****        QML errors:           *****" ) );
    QgsDebugError( QStringLiteral( "****************************************" ) );
    const QList<QQmlError> errors = component.errors();
    for ( const QQmlError &error : errors )
    {
      Q_UNUSED( error );
      QgsDebugError( error.toString() );
    }
    QgsDebugError( QStringLiteral( "****************************************" ) );
    QgsDebugError( QStringLiteral( "****************************************" ) );
  }

  if ( object == nullptr )
  {
    QgsDebugError( QStringLiteral( "FATAL ERROR: unable to create main.qml" ) );
    return EXIT_FAILURE;
  }

  // Add some data for debugging if needed
  QgsDebugError( QStringLiteral( "data directory: %1" ).arg( dataDir ) );

  return QgsApplication::exec();
}
