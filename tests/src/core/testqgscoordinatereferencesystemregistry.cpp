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
#include <QSettings>
#include <QSignalSpy>

#include "qgsapplication.h"
#include "qgslogger.h"

//header for class being tested
#include "qgscoordinatereferencesystemregistry.h"
#include "qgsapplication.h"
#include "qgsprojoperation.h"

class TestQgsCoordinateReferenceSystemRegistry: public QObject
{
    Q_OBJECT
  private slots:
    void initTestCase();
    void cleanupTestCase();
    void addUserCrs();
    void changeUserCrs();
    void removeUserCrs();
    void projOperations();

  private:

    QString mTempFolder;

};

void TestQgsCoordinateReferenceSystemRegistry::initTestCase()
{
  // we start from a clean profile - we don't want to mess with user custom SRSes
  // create temporary folder
  const QString subPath = QUuid::createUuid().toString().remove( '-' ).remove( '{' ).remove( '}' );
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

  QVERIFY( registry->userCrsList().isEmpty() );

  const QString madeUpProjection = QStringLiteral( "+proj=aea +lat_1=20 +lat_2=-23 +lat_0=4 +lon_0=29 +x_0=10 +y_0=3 +datum=WGS84 +units=m +no_defs" );
  const QgsCoordinateReferenceSystem userCrs = QgsCoordinateReferenceSystem::fromProj( madeUpProjection );
  QVERIFY( userCrs.isValid() );
  QCOMPARE( userCrs.toProj(), madeUpProjection );
  QCOMPARE( userCrs.srsid(), 0L ); // not saved to database yet

  const QSignalSpy spyAdded( registry, &QgsCoordinateReferenceSystemRegistry::userCrsAdded );
  const QSignalSpy spyChanged( registry, &QgsCoordinateReferenceSystemRegistry::userCrsChanged );
  const QSignalSpy spyCrsDefsChanged( registry, &QgsCoordinateReferenceSystemRegistry::crsDefinitionsChanged );

  // invalid crs -- should be rejected
  long res = registry->addUserCrs( QgsCoordinateReferenceSystem(), QStringLiteral( "test" ) );
  QCOMPARE( res, -1L );
  QCOMPARE( spyAdded.length(), 0 );
  QCOMPARE( spyChanged.length(), 0 );
  QCOMPARE( spyCrsDefsChanged.length(), 0 );
  QVERIFY( registry->userCrsList().isEmpty() );

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

  QCOMPARE( registry->userCrsList().count(), 1 );
  QCOMPARE( registry->userCrsList().at( 0 ).id, 100000L );
  QCOMPARE( registry->userCrsList().at( 0 ).name, QStringLiteral( "test" ) );
  QCOMPARE( registry->userCrsList().at( 0 ).proj, madeUpProjection );
  QVERIFY( !registry->userCrsList().at( 0 ).wkt.isEmpty() );
  QCOMPARE( registry->userCrsList().at( 0 ).crs, userCrs );

  // try adding again, should be assigned a new ID because we are calling the "add" method
  res = registry->addUserCrs( userCrs, QStringLiteral( "test2" ), Qgis::CrsDefinitionFormat::Proj );
  QCOMPARE( res, 100001L );
  QCOMPARE( spyAdded.length(), 2 );
  QCOMPARE( spyAdded.at( 1 ).at( 0 ).toString(), QStringLiteral( "USER:100001" ) );
  QCOMPARE( spyChanged.length(), 0 );
  QCOMPARE( spyCrsDefsChanged.length(), 2 );
  QCOMPARE( userCrs.srsid(), 100001L );
  QCOMPARE( userCrs.authid(), QStringLiteral( "USER:100001" ) );
  QCOMPARE( userCrs.description(), QStringLiteral( "test2" ) );

  QCOMPARE( registry->userCrsList().count(), 2 );
  QCOMPARE( registry->userCrsList().at( 0 ).id, 100000L );
  QCOMPARE( registry->userCrsList().at( 0 ).name, QStringLiteral( "test" ) );
  QCOMPARE( registry->userCrsList().at( 0 ).proj, madeUpProjection );
  QVERIFY( !registry->userCrsList().at( 0 ).wkt.isEmpty() );
  QCOMPARE( registry->userCrsList().at( 0 ).crs.toProj(), madeUpProjection );
  QCOMPARE( registry->userCrsList().at( 1 ).id, 100001L );
  QCOMPARE( registry->userCrsList().at( 1 ).name, QStringLiteral( "test2" ) );
  QCOMPARE( registry->userCrsList().at( 1 ).proj, madeUpProjection );
  QVERIFY( registry->userCrsList().at( 1 ).wkt.isEmpty() );
  QCOMPARE( registry->userCrsList().at( 1 ).crs.toProj(), userCrs.toProj() );
}

void TestQgsCoordinateReferenceSystemRegistry::changeUserCrs()
{
  QgsCoordinateReferenceSystemRegistry *registry = QgsApplication::coordinateReferenceSystemRegistry();

  QString madeUpProjection = QStringLiteral( "+proj=aea +lat_1=22 +lat_2=-24 +lat_0=4 +lon_0=29 +x_0=10 +y_0=3 +datum=WGS84 +units=m +no_defs" );
  QgsCoordinateReferenceSystem userCrs = QgsCoordinateReferenceSystem::fromProj( madeUpProjection );
  QVERIFY( userCrs.isValid() );
  QCOMPARE( userCrs.toProj(), madeUpProjection );
  QCOMPARE( userCrs.srsid(), 0L ); // not saved to database yet

  const QSignalSpy spyAdded( registry, &QgsCoordinateReferenceSystemRegistry::userCrsAdded );
  const QSignalSpy spyChanged( registry, &QgsCoordinateReferenceSystemRegistry::userCrsChanged );
  const QSignalSpy spyCrsDefsChanged( registry, &QgsCoordinateReferenceSystemRegistry::crsDefinitionsChanged );

  // invalid crs -- should be rejected
  bool res = registry->updateUserCrs( 100000, QgsCoordinateReferenceSystem(), QStringLiteral( "test" ) );
  QVERIFY( !res );
  QCOMPARE( spyAdded.length(), 0 );
  QCOMPARE( spyChanged.length(), 0 );
  QCOMPARE( spyCrsDefsChanged.length(), 0 );
  QCOMPARE( registry->userCrsList().count(), 2 );
  // non-existing crs - should be rejected
  res = registry->updateUserCrs( 100100, userCrs, QStringLiteral( "test" ) );
  QVERIFY( !res );
  QCOMPARE( spyAdded.length(), 0 );
  QCOMPARE( spyChanged.length(), 0 );
  QCOMPARE( spyCrsDefsChanged.length(), 0 );
  QCOMPARE( registry->userCrsList().count(), 2 );

  // add valid new user crs
  const long id = registry->addUserCrs( userCrs, QStringLiteral( "test" ) );
  QVERIFY( id != -1L );
  const QString authid = userCrs.authid();
  QCOMPARE( spyAdded.length(), 1 );
  QCOMPARE( spyAdded.at( 0 ).at( 0 ).toString(), authid );
  QCOMPARE( spyChanged.length(), 0 );
  QCOMPARE( spyCrsDefsChanged.length(), 1 );
  QCOMPARE( registry->userCrsList().count(), 3 );

  // now try changing it
  QgsCoordinateReferenceSystem crs2( authid );
  QCOMPARE( crs2.toProj(), madeUpProjection );
  const QString madeUpProjection2 = QStringLiteral( "+proj=aea +lat_1=22.5 +lat_2=-24.5 +lat_0=4 +lon_0=29 +x_0=10 +y_0=3 +datum=WGS84 +units=m +no_defs" );
  crs2.createFromProj( madeUpProjection2 );

  const QMetaObject::Connection conn1 =  connect( registry, &QgsCoordinateReferenceSystemRegistry::userCrsChanged, this, [&]
  {
    // make sure that caches are invalidated before the signals are emitted, so that slots will
    // get the new definition!
    const QgsCoordinateReferenceSystem crs4( authid );
    QCOMPARE( crs4.toProj(), madeUpProjection2 );

    // original crs object should be unchanged until it's refreshed
    QCOMPARE( userCrs.toProj(), madeUpProjection );
    userCrs.updateDefinition();
    QCOMPARE( userCrs.toProj(), madeUpProjection2 );
  } );

  QVERIFY( registry->updateUserCrs( id, crs2, QStringLiteral( "test 2" ) ) );

  QCOMPARE( spyAdded.length(), 1 );
  QCOMPARE( spyChanged.length(), 1 );
  QCOMPARE( spyChanged.at( 0 ).at( 0 ).toString(), authid );
  QCOMPARE( spyCrsDefsChanged.length(), 2 );

  QCOMPARE( registry->userCrsList().count(), 3 );
  QCOMPARE( registry->userCrsList().at( 2 ).id, 100002L );
  QCOMPARE( registry->userCrsList().at( 2 ).name, QStringLiteral( "test 2" ) );
  QCOMPARE( registry->userCrsList().at( 2 ).proj, madeUpProjection2 );
  QVERIFY( !registry->userCrsList().at( 2 ).wkt.isEmpty() );
  QCOMPARE( registry->userCrsList().at( 2 ).crs.toProj(), madeUpProjection2 );

  // newly created crs should get new definition, not old
  const QgsCoordinateReferenceSystem crs3( authid );
  QCOMPARE( crs3.toProj(), madeUpProjection2 );

  // with proj native format
  QObject::disconnect( conn1 );
  QVERIFY( registry->updateUserCrs( id, crs2, QStringLiteral( "test 2" ), Qgis::CrsDefinitionFormat::Proj ) );

  QCOMPARE( spyAdded.length(), 1 );
  QCOMPARE( spyChanged.length(), 2 );
  QCOMPARE( spyCrsDefsChanged.length(), 3 );

  QCOMPARE( registry->userCrsList().count(), 3 );
  QCOMPARE( registry->userCrsList().at( 2 ).id, 100002L );
  QCOMPARE( registry->userCrsList().at( 2 ).name, QStringLiteral( "test 2" ) );
  QCOMPARE( registry->userCrsList().at( 2 ).proj, madeUpProjection2 );
  QVERIFY( registry->userCrsList().at( 2 ).wkt.isEmpty() );
  QCOMPARE( registry->userCrsList().at( 2 ).crs.toProj(), madeUpProjection2 );
}

void TestQgsCoordinateReferenceSystemRegistry::removeUserCrs()
{
  QgsCoordinateReferenceSystemRegistry *registry = QgsApplication::coordinateReferenceSystemRegistry();

  const QString madeUpProjection = QStringLiteral( "+proj=aea +lat_1=27 +lat_2=-26 +lat_0=4 +lon_0=29 +x_0=10 +y_0=3 +datum=WGS84 +units=m +no_defs" );
  const QgsCoordinateReferenceSystem userCrs = QgsCoordinateReferenceSystem::fromProj( madeUpProjection );
  QVERIFY( userCrs.isValid() );
  QCOMPARE( userCrs.toProj(), madeUpProjection );
  QCOMPARE( userCrs.srsid(), 0L ); // not saved to database yet

  const QSignalSpy spyAdded( registry, &QgsCoordinateReferenceSystemRegistry::userCrsAdded );
  const QSignalSpy spyChanged( registry, &QgsCoordinateReferenceSystemRegistry::userCrsChanged );
  const QSignalSpy spyRemoved( registry, &QgsCoordinateReferenceSystemRegistry::userCrsRemoved );
  const QSignalSpy spyCrsDefsChanged( registry, &QgsCoordinateReferenceSystemRegistry::crsDefinitionsChanged );

  QCOMPARE( registry->userCrsList().count(), 3 );
  // non-existing crs - should be rejected
  const bool res = registry->removeUserCrs( 100100 );
  QVERIFY( !res );
  QCOMPARE( spyAdded.length(), 0 );
  QCOMPARE( spyChanged.length(), 0 );
  QCOMPARE( spyRemoved.length(), 0 );
  QCOMPARE( spyCrsDefsChanged.length(), 0 );
  QCOMPARE( registry->userCrsList().count(), 3 );

  // add valid new user crs
  const long id = registry->addUserCrs( userCrs, QStringLiteral( "test" ) );
  QVERIFY( id != -1L );
  const QString authid = userCrs.authid();
  QCOMPARE( spyAdded.length(), 1 );
  QCOMPARE( spyAdded.at( 0 ).at( 0 ).toString(), authid );
  QCOMPARE( spyChanged.length(), 0 );
  QCOMPARE( spyRemoved.length(), 0 );
  QCOMPARE( spyCrsDefsChanged.length(), 1 );
  QCOMPARE( registry->userCrsList().count(), 4 );

  const QgsCoordinateReferenceSystem crs2( authid );
  QVERIFY( crs2.isValid() );

  // now try removing it
  connect( registry, &QgsCoordinateReferenceSystemRegistry::userCrsRemoved, this, [&]
  {
    // make sure that caches are invalidated before the signals are emitted
    const QgsCoordinateReferenceSystem crs4( authid );
    QVERIFY( !crs4.isValid() );
  } );
  QVERIFY( registry->removeUserCrs( id ) );

  QCOMPARE( spyAdded.length(), 1 );
  QCOMPARE( spyChanged.length(), 0 );
  QCOMPARE( spyRemoved.length(), 1 );
  QCOMPARE( spyRemoved.at( 0 ).at( 0 ).toLongLong(), static_cast< long long >( id ) );
  QCOMPARE( spyCrsDefsChanged.length(), 2 );

  QCOMPARE( registry->userCrsList().count(), 3 );
  QCOMPARE( registry->userCrsList().at( 0 ).id, 100000L );
  QCOMPARE( registry->userCrsList().at( 0 ).name, QStringLiteral( "test" ) );
  QCOMPARE( registry->userCrsList().at( 0 ).proj, QStringLiteral( "+proj=aea +lat_1=20 +lat_2=-23 +lat_0=4 +lon_0=29 +x_0=10 +y_0=3 +datum=WGS84 +units=m +no_defs" ) );
  QVERIFY( !registry->userCrsList().at( 0 ).wkt.isEmpty() );
  QCOMPARE( registry->userCrsList().at( 0 ).crs.toProj(), QStringLiteral( "+proj=aea +lat_1=20 +lat_2=-23 +lat_0=4 +lon_0=29 +x_0=10 +y_0=3 +datum=WGS84 +units=m +no_defs" ) );
  QCOMPARE( registry->userCrsList().at( 1 ).id, 100001L );
  QCOMPARE( registry->userCrsList().at( 1 ).name, QStringLiteral( "test2" ) );
  QCOMPARE( registry->userCrsList().at( 1 ).proj, QStringLiteral( "+proj=aea +lat_1=20 +lat_2=-23 +lat_0=4 +lon_0=29 +x_0=10 +y_0=3 +datum=WGS84 +units=m +no_defs" ) );
  QVERIFY( registry->userCrsList().at( 1 ).wkt.isEmpty() );
  QCOMPARE( registry->userCrsList().at( 1 ).crs.toProj(), QStringLiteral( "+proj=aea +lat_1=20 +lat_2=-23 +lat_0=4 +lon_0=29 +x_0=10 +y_0=3 +datum=WGS84 +units=m +no_defs" ) );
  QCOMPARE( registry->userCrsList().at( 2 ).id, 100002L );
  QCOMPARE( registry->userCrsList().at( 2 ).name, QStringLiteral( "test 2" ) );
  QCOMPARE( registry->userCrsList().at( 2 ).proj, QStringLiteral( "+proj=aea +lat_1=22.5 +lat_2=-24.5 +lat_0=4 +lon_0=29 +x_0=10 +y_0=3 +datum=WGS84 +units=m +no_defs" ) );
  QVERIFY( registry->userCrsList().at( 2 ).wkt.isEmpty() );

  // doesn't exist anymore...
  const QgsCoordinateReferenceSystem crs3( authid );
  QVERIFY( !crs3.isValid() );
}

void TestQgsCoordinateReferenceSystemRegistry::projOperations()
{
  const QMap< QString, QgsProjOperation > operations = QgsApplication::coordinateReferenceSystemRegistry()->projOperations();

  QVERIFY( operations.contains( QStringLiteral( "lcc" ) ) );
  QVERIFY( operations.value( QStringLiteral( "lcc" ) ).isValid() );
  QCOMPARE( operations.value( QStringLiteral( "lcc" ) ).id(), QStringLiteral( "lcc" ) );
  QCOMPARE( operations.value( QStringLiteral( "lcc" ) ).description(), QStringLiteral( "Lambert Conformal Conic" ) );
  QVERIFY( operations.value( QStringLiteral( "lcc" ) ).details().contains( QStringLiteral( "Conic" ) ) );
}

QGSTEST_MAIN( TestQgsCoordinateReferenceSystemRegistry )
#include "testqgscoordinatereferencesystemregistry.moc"
