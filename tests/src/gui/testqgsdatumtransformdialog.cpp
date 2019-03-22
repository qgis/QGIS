/***************************************************************************
    testqgsdatumtransformdialog.cpp
     --------------------------------------
    Date                 : March 2019
    Copyright            : (C) 2019 Nyall Dawson
    Email                : nyall dot dawson at gmail dot com
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/


#include "qgstest.h"

#include "qgsdatumtransformdialog.h"
#include "qgssettings.h"
#include "qgsproject.h"

class TestQgsDatumTransformDialog: public QObject
{
    Q_OBJECT
  private slots:
    void initTestCase(); // will be called before the first testfunction is executed.
    void cleanupTestCase(); // will be called after the last testfunction was executed.
    void init(); // will be called before each testfunction is executed.
    void cleanup(); // will be called after every testfunction.

    void defaultTransform();
    void shouldAskUser();
    void applyDefaultTransform();
    void runDialog();

  private:

};

void TestQgsDatumTransformDialog::initTestCase()
{
  // initialize with test settings directory so we don't mess with user's stuff
  QgsApplication::init( QDir::tempPath() + "/dot-qgis" );
  QgsApplication::initQgis();
  QgsApplication::createDatabase();
  // output test environment
  QgsApplication::showSettings();

  // Set up the QgsSettings environment
  QCoreApplication::setOrganizationName( QStringLiteral( "QGIS" ) );
  QCoreApplication::setOrganizationDomain( QStringLiteral( "qgis.org" ) );
  QCoreApplication::setApplicationName( QStringLiteral( "QGIS-TEST" ) );
}

void TestQgsDatumTransformDialog::cleanupTestCase()
{
}

void TestQgsDatumTransformDialog::init()
{
}

void TestQgsDatumTransformDialog::cleanup()
{
}

void TestQgsDatumTransformDialog::defaultTransform()
{
  QgsDatumTransformDialog dlg( QgsCoordinateReferenceSystem( QStringLiteral( "EPSG:26742" ) ), QgsCoordinateReferenceSystem( QStringLiteral( "EPSG:4326" ) ) );

  QgsDatumTransformDialog::TransformInfo def = dlg.defaultDatumTransform();
  QCOMPARE( def.sourceCrs.authid(), QStringLiteral( "EPSG:26742" ) );
  QCOMPARE( def.destinationCrs.authid(), QStringLiteral( "EPSG:4326" ) );
  QCOMPARE( QgsDatumTransform::datumTransformToProj( def.sourceTransformId ), QStringLiteral( "+towgs84=-10,158,187" ) );
  QCOMPARE( QgsDatumTransform::datumTransformToProj( def.destinationTransformId ), QString() );

  // default should be initially selected
  def = dlg.selectedDatumTransform();
  QCOMPARE( def.sourceCrs.authid(), QStringLiteral( "EPSG:26742" ) );
  QCOMPARE( def.destinationCrs.authid(), QStringLiteral( "EPSG:4326" ) );
  QCOMPARE( QgsDatumTransform::datumTransformToProj( def.sourceTransformId ), QStringLiteral( "+towgs84=-10,158,187" ) );
  QCOMPARE( QgsDatumTransform::datumTransformToProj( def.destinationTransformId ), QString() );

  QgsDatumTransformDialog dlg2( QgsCoordinateReferenceSystem( QStringLiteral( "EPSG:4326" ) ), QgsCoordinateReferenceSystem( QStringLiteral( "EPSG:26742" ) ) );
  def = dlg2.defaultDatumTransform();
  QCOMPARE( def.sourceCrs.authid(), QStringLiteral( "EPSG:4326" ) );
  QCOMPARE( def.destinationCrs.authid(), QStringLiteral( "EPSG:26742" ) );
  QCOMPARE( QgsDatumTransform::datumTransformToProj( def.sourceTransformId ), QString() );
  QCOMPARE( QgsDatumTransform::datumTransformToProj( def.destinationTransformId ), QStringLiteral( "+towgs84=-10,158,187" ) );

}

void TestQgsDatumTransformDialog::shouldAskUser()
{
  // no prompts!
  QgsSettings().setValue( QStringLiteral( "/projections/promptWhenMultipleTransformsExist" ), false, QgsSettings::App );
  QgsDatumTransformDialog dlg( QgsCoordinateReferenceSystem( QStringLiteral( "EPSG:26742" ) ), QgsCoordinateReferenceSystem( QStringLiteral( "EPSG:4326" ) ) );
  QVERIFY( !dlg.shouldAskUserForSelection() );
  QgsDatumTransformDialog dlg2( QgsCoordinateReferenceSystem( QStringLiteral( "EPSG:26742" ) ), QgsCoordinateReferenceSystem( QStringLiteral( "EPSG:7406" ) ) );
  QVERIFY( !dlg2.shouldAskUserForSelection() );

//prompts
  QgsSettings().setValue( QStringLiteral( "/projections/promptWhenMultipleTransformsExist" ), true, QgsSettings::App );
  QgsDatumTransformDialog dlg3( QgsCoordinateReferenceSystem( QStringLiteral( "EPSG:26742" ) ), QgsCoordinateReferenceSystem( QStringLiteral( "EPSG:4326" ) ) );
  QVERIFY( dlg3.shouldAskUserForSelection() );
  QgsDatumTransformDialog dlg4( QgsCoordinateReferenceSystem( QStringLiteral( "EPSG:26742" ) ), QgsCoordinateReferenceSystem( QStringLiteral( "EPSG:7406" ) ) );
  QVERIFY( !dlg4.shouldAskUserForSelection() );
}

void TestQgsDatumTransformDialog::applyDefaultTransform()
{
  QgsSettings().setValue( QStringLiteral( "/projections/promptWhenMultipleTransformsExist" ), false, QgsSettings::App );

  QgsDatumTransformDialog dlg( QgsCoordinateReferenceSystem( QStringLiteral( "EPSG:26742" ) ), QgsCoordinateReferenceSystem( QStringLiteral( "EPSG:7406" ) ) );
  dlg.applyDefaultTransform();
  QVERIFY( QgsProject::instance()->transformContext().sourceDestinationDatumTransforms().isEmpty() );

  QgsDatumTransformDialog dlg2( QgsCoordinateReferenceSystem( QStringLiteral( "EPSG:26742" ) ), QgsCoordinateReferenceSystem( QStringLiteral( "EPSG:4326" ) ) );
  dlg2.applyDefaultTransform();

  QVERIFY( !QgsProject::instance()->transformContext().sourceDestinationDatumTransforms().isEmpty() );
  QCOMPARE( QgsDatumTransform::datumTransformToProj( QgsProject::instance()->transformContext().calculateDatumTransforms( QgsCoordinateReferenceSystem( QStringLiteral( "EPSG:26742" ) ), QgsCoordinateReferenceSystem( QStringLiteral( "EPSG:4326" ) ) ).sourceTransformId ), QStringLiteral( "+towgs84=-10,158,187" ) );
  QgsProject::instance()->clear();
}

void TestQgsDatumTransformDialog::runDialog()
{
  QgsSettings().setValue( QStringLiteral( "/projections/promptWhenMultipleTransformsExist" ), false, QgsSettings::App );

  QVERIFY( QgsDatumTransformDialog::run( QgsCoordinateReferenceSystem( QStringLiteral( "EPSG:26742" ) ), QgsCoordinateReferenceSystem( QStringLiteral( "EPSG:7406" ) ) ) );
  QVERIFY( QgsProject::instance()->transformContext().sourceDestinationDatumTransforms().isEmpty() );

  QVERIFY( QgsDatumTransformDialog::run( QgsCoordinateReferenceSystem( QStringLiteral( "EPSG:26742" ) ), QgsCoordinateReferenceSystem( QStringLiteral( "EPSG:4326" ) ) ) );

  QVERIFY( !QgsProject::instance()->transformContext().sourceDestinationDatumTransforms().isEmpty() );
  QCOMPARE( QgsDatumTransform::datumTransformToProj( QgsProject::instance()->transformContext().calculateDatumTransforms( QgsCoordinateReferenceSystem( QStringLiteral( "EPSG:26742" ) ), QgsCoordinateReferenceSystem( QStringLiteral( "EPSG:4326" ) ) ).sourceTransformId ), QStringLiteral( "+towgs84=-10,158,187" ) );
  QgsProject::instance()->clear();
  QVERIFY( QgsDatumTransformDialog::run( QgsCoordinateReferenceSystem( QStringLiteral( "EPSG:26742" ) ), QgsCoordinateReferenceSystem( QStringLiteral( "EPSG:4326" ) ) ) );
}


QGSTEST_MAIN( TestQgsDatumTransformDialog )
#include "testqgsdatumtransformdialog.moc"
