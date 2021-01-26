/***************************************************************************
  testqgscoordinatereferencesystemregistry.cpp
  --------------------------------------
    begin                : January 2021
    copyright            : (C) 2021 by Nyall Dawson
    email                : nyall dot dawson at gmail dot com
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/
#include "qgstest.h"
#include <QPixmap>

#include "qgsapplication.h"
#include "qgslogger.h"

//header for class being tested
#include "qgscoordinatereferencesystemregistry.h"
#include "qgsapplication.h"

class TestQgsCoordinateReferenceSystemRegistry: public QObject
{
    Q_OBJECT
  private slots:
    void initTestCase();
    void cleanupTestCase();
    void addUserCrs();
    void changeUserCrs();

  private:

    QString mTempFolder;

};

void TestQgsCoordinateReferenceSystemRegistry::initTestCase()
{
  // we start from a clean profile - we don't want to mess with user custom SRSes
  // create temporary folder
  QString subPath = QUuid::createUuid().toString().remove( '-' ).remove( '{' ).remove( '}' );
  mTempFolder = QDir::tempPath() + '/' + subPath;
  if ( !QDir( mTempFolder ).exists() )
    QDir().mkpath( mTempFolder );

  //
  // Runs once before any tests are run
  //
  // init QGIS's paths - true means that all path will be inited from prefix
  QgsApplication::init( mTempFolder );
  QgsApplication::createDatabase();
  QgsApplication::initQgis();
  QCoreApplication::setOrganizationName( QStringLiteral( "QGIS" ) );
  QCoreApplication::setOrganizationDomain( QStringLiteral( "qgis.org" ) );
  QCoreApplication::setApplicationName( QStringLiteral( "QGIS-TEST" ) );

  QSettings().clear();

  QgsDebugMsg( QStringLiteral( "Custom srs database: %1" ).arg( QgsApplication::qgisUserDatabaseFilePath() ) );
}

void TestQgsCoordinateReferenceSystemRegistry::cleanupTestCase()
{
  QgsApplication::exitQgis();
}

void TestQgsCoordinateReferenceSystemRegistry::addUserCrs()
{
  QgsCoordinateReferenceSystemRegistry *registry = QgsApplication::coordinateReferenceSystemRegistry();

  QString madeUpProjection = QStringLiteral( "+proj=aea +lat_1=20 +lat_2=-23 +lat_0=4 +lon_0=29 +x_0=10 +y_0=3 +datum=WGS84 +units=m +no_defs" );
  QgsCoordinateReferenceSystem userCrs = QgsCoordinateReferenceSystem::fromProj( madeUpProjection );
  QVERIFY( userCrs.isValid() );
  QCOMPARE( userCrs.toProj(), madeUpProjection );
  QCOMPARE( userCrs.srsid(), 0L ); // not saved to database yet

  QSignalSpy spyAdded( registry, &QgsCoordinateReferenceSystemRegistry::userCrsAdded );
  QSignalSpy spyChanged( registry, &QgsCoordinateReferenceSystemRegistry::userCrsChanged );
  QSignalSpy spyCrsDefsChanged( registry, &QgsCoordinateReferenceSystemRegistry::crsDefinitionsChanged );

  // invalid crs -- should be rejected
  long res = registry->addUserCrs( QgsCoordinateReferenceSystem(), QStringLiteral( "test" ) );
  QCOMPARE( res, -1L );
  QCOMPARE( spyAdded.length(), 0 );
  QCOMPARE( spyChanged.length(), 0 );
  QCOMPARE( spyCrsDefsChanged.length(), 0 );

  // valid new crs
  res = registry->addUserCrs( userCrs, QStringLiteral( "test" ) );
  QCOMPARE( res, 100000L );
  QCOMPARE( spyAdded.length(), 1 );
  QCOMPARE( spyAdded.at( 0 ).at( 0 ).toString(), QStringLiteral( "USER:100000" ) );
  QCOMPARE( spyChanged.length(), 0 );
  QCOMPARE( spyCrsDefsChanged.length(), 1 );
  QCOMPARE( userCrs.srsid(), 100000L );
  QCOMPARE( userCrs.authid(), QStringLiteral( "USER:100000" ) );
  QCOMPARE( userCrs.description(), QStringLiteral( "test" ) );

  // try adding again, should be assigned a new ID because we are calling the "add" method
  res = registry->addUserCrs( userCrs, QStringLiteral( "test2" ) );
  QCOMPARE( res, 100001L );
  QCOMPARE( spyAdded.length(), 2 );
  QCOMPARE( spyAdded.at( 1 ).at( 0 ).toString(), QStringLiteral( "USER:100001" ) );
  QCOMPARE( spyChanged.length(), 0 );
  QCOMPARE( spyCrsDefsChanged.length(), 2 );
  QCOMPARE( userCrs.srsid(), 100001L );
  QCOMPARE( userCrs.authid(), QStringLiteral( "USER:100001" ) );
  QCOMPARE( userCrs.description(), QStringLiteral( "test2" ) );
}

void TestQgsCoordinateReferenceSystemRegistry::changeUserCrs()
{
  QgsCoordinateReferenceSystemRegistry *registry = QgsApplication::coordinateReferenceSystemRegistry();

  QString madeUpProjection = QStringLiteral( "+proj=aea +lat_1=22 +lat_2=-24 +lat_0=4 +lon_0=29 +x_0=10 +y_0=3 +datum=WGS84 +units=m +no_defs" );
  QgsCoordinateReferenceSystem userCrs = QgsCoordinateReferenceSystem::fromProj( madeUpProjection );
  QVERIFY( userCrs.isValid() );
  QCOMPARE( userCrs.toProj(), madeUpProjection );
  QCOMPARE( userCrs.srsid(), 0L ); // not saved to database yet

  QSignalSpy spyAdded( registry, &QgsCoordinateReferenceSystemRegistry::userCrsAdded );
  QSignalSpy spyChanged( registry, &QgsCoordinateReferenceSystemRegistry::userCrsChanged );
  QSignalSpy spyCrsDefsChanged( registry, &QgsCoordinateReferenceSystemRegistry::crsDefinitionsChanged );

  // invalid crs -- should be rejected
  bool res = registry->updateUserCrs( 100000, QgsCoordinateReferenceSystem(), QStringLiteral( "test" ) );
  QVERIFY( !res );
  QCOMPARE( spyAdded.length(), 0 );
  QCOMPARE( spyChanged.length(), 0 );
  QCOMPARE( spyCrsDefsChanged.length(), 0 );
  // non-existing crs - should be rejected
  res = registry->updateUserCrs( 100100, userCrs, QStringLiteral( "test" ) );
  QVERIFY( !res );
  QCOMPARE( spyAdded.length(), 0 );
  QCOMPARE( spyChanged.length(), 0 );
  QCOMPARE( spyCrsDefsChanged.length(), 0 );

  // add valid new user crs
  const long id = registry->addUserCrs( userCrs, QStringLiteral( "test" ) );
  QVERIFY( id != -1L );
  const QString authid = userCrs.authid();
  QCOMPARE( spyAdded.length(), 1 );
  QCOMPARE( spyAdded.at( 0 ).at( 0 ).toString(), authid );
  QCOMPARE( spyChanged.length(), 0 );
  QCOMPARE( spyCrsDefsChanged.length(), 1 );

  // now try changing it
  QgsCoordinateReferenceSystem crs2( authid );
  QCOMPARE( crs2.toProj(), madeUpProjection );
  const QString madeUpProjection2 = QStringLiteral( "+proj=aea +lat_1=22.5 +lat_2=-24.5 +lat_0=4 +lon_0=29 +x_0=10 +y_0=3 +datum=WGS84 +units=m +no_defs" );
  crs2.createFromProj( madeUpProjection2 );

  connect( registry, &QgsCoordinateReferenceSystemRegistry::userCrsChanged, this, [&]
  {
    // make sure that caches are invalidated before the signals are emitted, so that slots will
    // get the new definition!
    QgsCoordinateReferenceSystem crs4( authid );
    QCOMPARE( crs4.toProj(), madeUpProjection2 );
  } );

  QVERIFY( registry->updateUserCrs( id, crs2, QStringLiteral( "test 2" ) ) );

  QCOMPARE( spyAdded.length(), 1 );
  QCOMPARE( spyChanged.length(), 1 );
  QCOMPARE( spyChanged.at( 0 ).at( 0 ).toString(), authid );
  QCOMPARE( spyCrsDefsChanged.length(), 2 );

  // newly created crs should get new definition, not old
  QgsCoordinateReferenceSystem crs3( authid );
  QCOMPARE( crs3.toProj(), madeUpProjection2 );
}

QGSTEST_MAIN( TestQgsCoordinateReferenceSystemRegistry )
#include "testqgscoordinatereferencesystemregistry.moc"
