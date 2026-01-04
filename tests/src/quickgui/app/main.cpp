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

#include "qgis.h"
#include "qgsapplication.h"
#include "qgslayertree.h"
#include "qgslogger.h"
#include "qgsmessagelog.h"
#include "qgsproject.h"

#include <QCommandLineParser>
#include <QQmlApplicationEngine>
#include <QQmlComponent>
#include <QQmlContext>
#include <QQmlError>

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
  QgsDebugMsgLevel( u"project file:  %1"_s.arg( projectFile ), 2 );
  QgsProject project;
  bool res = project.read( projectFile );
  Q_ASSERT( res );

  QQmlEngine engine;
  engine.addImportPath( QgsApplication::qmlImportPath() );
  engine.rootContext()->setContextProperty( "__project", &project );
  engine.rootContext()->setContextProperty( "__layers", QVariant::fromValue( project.layerTreeRoot()->layerOrder() ) );

  QQmlComponent component( &engine, QUrl( u"qrc:/main.qml"_s ) );
  QObject *object = component.create();

  if ( !component.errors().isEmpty() )
  {
    QgsDebugError( u"%s"_s.arg( QgsApplication::showSettings().toLocal8Bit().data() ) );

    QgsDebugError( u"****************************************"_s );
    QgsDebugError( u"*****        QML errors:           *****"_s );
    QgsDebugError( u"****************************************"_s );
    const QList<QQmlError> errors = component.errors();
    for ( const QQmlError &error : errors )
    {
      Q_UNUSED( error );
      QgsDebugError( error.toString() );
    }
    QgsDebugError( u"****************************************"_s );
    QgsDebugError( u"****************************************"_s );
  }

  if ( object == nullptr )
  {
    QgsDebugError( u"FATAL ERROR: unable to create main.qml"_s );
    return EXIT_FAILURE;
  }

  // Add some data for debugging if needed
  QgsDebugError( u"data directory: %1"_s.arg( dataDir ) );

  return QgsApplication::exec();
}
