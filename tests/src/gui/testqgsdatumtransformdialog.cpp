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
    void fallback();
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
  QgsDatumTransformDialog dlg( QgsCoordinateReferenceSystem( QStringLiteral( "EPSG:7844" ) ), QgsCoordinateReferenceSystem( QStringLiteral( "EPSG:4283" ) ) );

  QgsDatumTransformDialog::TransformInfo def = dlg.defaultDatumTransform();
  QCOMPARE( def.sourceCrs.authid(), QStringLiteral( "EPSG:7844" ) );
  QCOMPARE( def.destinationCrs.authid(), QStringLiteral( "EPSG:4283" ) );
  QCOMPARE( def.proj, QStringLiteral( "+proj=pipeline +step +proj=unitconvert +xy_in=deg +xy_out=rad +step +proj=push +v_3 +step +proj=cart +ellps=GRS80 +step +inv +proj=helmert +x=0.06155 +y=-0.01087 +z=-0.04019 +rx=-0.0394924 +ry=-0.0327221 +rz=-0.0328979 +s=-0.009994 +convention=coordinate_frame +step +inv +proj=cart +ellps=GRS80 +step +proj=pop +v_3 +step +proj=unitconvert +xy_in=rad +xy_out=deg" ) );
  QVERIFY( def.allowFallback );

  // default should be initially selected
  def = dlg.selectedDatumTransform();
  QCOMPARE( def.sourceCrs.authid(), QStringLiteral( "EPSG:7844" ) );
  QCOMPARE( def.destinationCrs.authid(), QStringLiteral( "EPSG:4283" ) );
  QCOMPARE( def.proj, QStringLiteral( "+proj=pipeline +step +proj=unitconvert +xy_in=deg +xy_out=rad +step +proj=push +v_3 +step +proj=cart +ellps=GRS80 +step +inv +proj=helmert +x=0.06155 +y=-0.01087 +z=-0.04019 +rx=-0.0394924 +ry=-0.0327221 +rz=-0.0328979 +s=-0.009994 +convention=coordinate_frame +step +inv +proj=cart +ellps=GRS80 +step +proj=pop +v_3 +step +proj=unitconvert +xy_in=rad +xy_out=deg" ) );

  const QgsDatumTransformDialog dlg2( QgsCoordinateReferenceSystem( QStringLiteral( "EPSG:4326" ) ), QgsCoordinateReferenceSystem( QStringLiteral( "EPSG:26742" ) ) );
  def = dlg2.defaultDatumTransform();
  QCOMPARE( def.sourceCrs.authid(), QStringLiteral( "EPSG:4326" ) );
  QCOMPARE( def.destinationCrs.authid(), QStringLiteral( "EPSG:26742" ) );
  if ( def.proj == QLatin1String( "+proj=pipeline +step +proj=unitconvert +xy_in=deg +xy_out=rad +step +inv +proj=hgridshift +grids=conus +step +proj=lcc +lat_0=37.6666666666667 +lon_0=-122 +lat_1=39.8333333333333 +lat_2=38.3333333333333 +x_0=609601.219202438 +y_0=0 +ellps=clrk66 +step +proj=unitconvert +xy_in=m +xy_out=us-ft" ) )
  {
    QCOMPARE( def.proj, QStringLiteral( "+proj=pipeline +step +proj=unitconvert +xy_in=deg +xy_out=rad +step +inv +proj=hgridshift +grids=conus +step +proj=lcc +lat_0=37.6666666666667 +lon_0=-122 +lat_1=39.8333333333333 +lat_2=38.3333333333333 +x_0=609601.219202438 +y_0=0 +ellps=clrk66 +step +proj=unitconvert +xy_in=m +xy_out=us-ft" ) );
  }
  else
  {
    QCOMPARE( def.proj, QStringLiteral( "+proj=pipeline +step +proj=unitconvert +xy_in=deg +xy_out=rad +step +proj=push +v_3 +step +proj=cart +ellps=WGS84 +step +proj=helmert +x=8 +y=-159 +z=-175 +step +inv +proj=cart +ellps=clrk66 +step +proj=pop +v_3 +step +proj=lcc +lat_0=37.6666666666667 +lon_0=-122 +lat_1=39.8333333333333 +lat_2=38.3333333333333 +x_0=609601.219202438 +y_0=0 +ellps=clrk66 +step +proj=unitconvert +xy_in=m +xy_out=us-ft" ) );
  }
}

void TestQgsDatumTransformDialog::fallback()
{
  // don't default to allow fallback
  QgsDatumTransformDialog dlg( QgsCoordinateReferenceSystem( QStringLiteral( "EPSG:7844" ) ), QgsCoordinateReferenceSystem( QStringLiteral( "EPSG:4283" ) ), false, true, false, qMakePair( -1, -1 ), nullptr, Qt::WindowFlags(), QString(), nullptr, false );

  const QgsDatumTransformDialog::TransformInfo def = dlg.selectedDatumTransform();
  QCOMPARE( def.sourceCrs.authid(), QStringLiteral( "EPSG:7844" ) );
  QCOMPARE( def.destinationCrs.authid(), QStringLiteral( "EPSG:4283" ) );
  QCOMPARE( def.proj, QStringLiteral( "+proj=pipeline +step +proj=unitconvert +xy_in=deg +xy_out=rad +step +proj=push +v_3 +step +proj=cart +ellps=GRS80 +step +inv +proj=helmert +x=0.06155 +y=-0.01087 +z=-0.04019 +rx=-0.0394924 +ry=-0.0327221 +rz=-0.0328979 +s=-0.009994 +convention=coordinate_frame +step +inv +proj=cart +ellps=GRS80 +step +proj=pop +v_3 +step +proj=unitconvert +xy_in=rad +xy_out=deg" ) );
  QVERIFY( !def.allowFallback );
}

void TestQgsDatumTransformDialog::shouldAskUser()
{
  // no prompts!
  QgsSettings().setValue( QStringLiteral( "/projections/promptWhenMultipleTransformsExist" ), false, QgsSettings::App );
  const QgsDatumTransformDialog dlg( QgsCoordinateReferenceSystem( QStringLiteral( "EPSG:26742" ) ), QgsCoordinateReferenceSystem( QStringLiteral( "EPSG:4326" ) ) );
  QVERIFY( !dlg.shouldAskUserForSelection() );
  const QgsDatumTransformDialog dlg2( QgsCoordinateReferenceSystem( QStringLiteral( "EPSG:26742" ) ), QgsCoordinateReferenceSystem( QStringLiteral( "EPSG:7406" ) ) );
  QVERIFY( !dlg2.shouldAskUserForSelection() );

//prompts
  QgsSettings().setValue( QStringLiteral( "/projections/promptWhenMultipleTransformsExist" ), true, QgsSettings::App );
  const QgsDatumTransformDialog dlg3( QgsCoordinateReferenceSystem( QStringLiteral( "EPSG:26742" ) ), QgsCoordinateReferenceSystem( QStringLiteral( "EPSG:4326" ) ) );
  QVERIFY( dlg3.shouldAskUserForSelection() );
  const QgsDatumTransformDialog dlg4( QgsCoordinateReferenceSystem( QStringLiteral( "EPSG:26742" ) ), QgsCoordinateReferenceSystem( QStringLiteral( "EPSG:7406" ) ) );
  QVERIFY( !dlg4.shouldAskUserForSelection() );
}

void TestQgsDatumTransformDialog::applyDefaultTransform()
{
  QgsSettings().setValue( QStringLiteral( "/projections/promptWhenMultipleTransformsExist" ), false, QgsSettings::App );

  Q_NOWARN_DEPRECATED_PUSH
  QgsDatumTransformDialog dlg( QgsCoordinateReferenceSystem( QStringLiteral( "EPSG:26742" ) ), QgsCoordinateReferenceSystem( QStringLiteral( "EPSG:7406" ) ) );
  dlg.applyDefaultTransform();
  QVERIFY( QgsProject::instance()->transformContext().sourceDestinationDatumTransforms().isEmpty() );
  QVERIFY( QgsProject::instance()->transformContext().coordinateOperations().isEmpty() );
  QgsProject::instance()->transformContext().addCoordinateOperation( QgsCoordinateReferenceSystem( QStringLiteral( "EPSG:26742" ) ), QgsCoordinateReferenceSystem( QStringLiteral( "EPSG:4326" ) ), QStringLiteral( "some proj" ) );

  QgsDatumTransformDialog dlg2( QgsCoordinateReferenceSystem( QStringLiteral( "EPSG:26742" ) ), QgsCoordinateReferenceSystem( QStringLiteral( "EPSG:4326" ) ) );
  dlg2.applyDefaultTransform();

  // a default transform means there should be no entry in the context
  QVERIFY( QgsProject::instance()->transformContext().coordinateOperations().isEmpty() );
  Q_NOWARN_DEPRECATED_POP
  QgsProject::instance()->clear();
}

void TestQgsDatumTransformDialog::runDialog()
{
  QgsSettings().setValue( QStringLiteral( "/projections/promptWhenMultipleTransformsExist" ), false, QgsSettings::App );
  QVERIFY( QgsDatumTransformDialog::run( QgsCoordinateReferenceSystem( QStringLiteral( "EPSG:26742" ) ), QgsCoordinateReferenceSystem( QStringLiteral( "EPSG:7406" ) ) ) );
  QVERIFY( QgsProject::instance()->transformContext().coordinateOperations().isEmpty() );

  QVERIFY( QgsDatumTransformDialog::run( QgsCoordinateReferenceSystem( QStringLiteral( "EPSG:26742" ) ), QgsCoordinateReferenceSystem( QStringLiteral( "EPSG:4326" ) ) ) );
  QVERIFY( QgsProject::instance()->transformContext().coordinateOperations().isEmpty() );
  QgsProject::instance()->clear();
}


QGSTEST_MAIN( TestQgsDatumTransformDialog )
#include "testqgsdatumtransformdialog.moc"
