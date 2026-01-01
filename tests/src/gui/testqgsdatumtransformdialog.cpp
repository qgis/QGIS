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


#include "qgsdatumtransformdialog.h"
#include "qgsproject.h"
#include "qgssettings.h"
#include "qgstest.h"

class TestQgsDatumTransformDialog : public QObject
{
    Q_OBJECT
  private slots:
    void initTestCase();    // will be called before the first testfunction is executed.
    void cleanupTestCase(); // will be called after the last testfunction was executed.
    void init();            // will be called before each testfunction is executed.
    void cleanup();         // will be called after every testfunction.

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
  QCoreApplication::setOrganizationName( u"QGIS"_s );
  QCoreApplication::setOrganizationDomain( u"qgis.org"_s );
  QCoreApplication::setApplicationName( u"QGIS-TEST"_s );
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
  QgsDatumTransformDialog dlg( QgsCoordinateReferenceSystem( u"EPSG:7844"_s ), QgsCoordinateReferenceSystem( u"EPSG:4283"_s ) );

  QgsDatumTransformDialog::TransformInfo def = dlg.defaultDatumTransform();
  QCOMPARE( def.sourceCrs.authid(), u"EPSG:7844"_s );
  QCOMPARE( def.destinationCrs.authid(), u"EPSG:4283"_s );
  QCOMPARE( def.proj, u"+proj=pipeline +step +proj=unitconvert +xy_in=deg +xy_out=rad +step +proj=push +v_3 +step +proj=cart +ellps=GRS80 +step +inv +proj=helmert +x=0.06155 +y=-0.01087 +z=-0.04019 +rx=-0.0394924 +ry=-0.0327221 +rz=-0.0328979 +s=-0.009994 +convention=coordinate_frame +step +inv +proj=cart +ellps=GRS80 +step +proj=pop +v_3 +step +proj=unitconvert +xy_in=rad +xy_out=deg"_s );
  QVERIFY( def.allowFallback );

  // default should be initially selected
  def = dlg.selectedDatumTransform();
  QCOMPARE( def.sourceCrs.authid(), u"EPSG:7844"_s );
  QCOMPARE( def.destinationCrs.authid(), u"EPSG:4283"_s );
  QCOMPARE( def.proj, u"+proj=pipeline +step +proj=unitconvert +xy_in=deg +xy_out=rad +step +proj=push +v_3 +step +proj=cart +ellps=GRS80 +step +inv +proj=helmert +x=0.06155 +y=-0.01087 +z=-0.04019 +rx=-0.0394924 +ry=-0.0327221 +rz=-0.0328979 +s=-0.009994 +convention=coordinate_frame +step +inv +proj=cart +ellps=GRS80 +step +proj=pop +v_3 +step +proj=unitconvert +xy_in=rad +xy_out=deg"_s );

  const QgsDatumTransformDialog dlg2( QgsCoordinateReferenceSystem( u"EPSG:4326"_s ), QgsCoordinateReferenceSystem( u"EPSG:26742"_s ) );
  def = dlg2.defaultDatumTransform();
  QCOMPARE( def.sourceCrs.authid(), u"EPSG:4326"_s );
  QCOMPARE( def.destinationCrs.authid(), u"EPSG:26742"_s );
  if ( def.proj == "+proj=pipeline +step +proj=unitconvert +xy_in=deg +xy_out=rad +step +inv +proj=hgridshift +grids=conus +step +proj=lcc +lat_0=37.6666666666667 +lon_0=-122 +lat_1=39.8333333333333 +lat_2=38.3333333333333 +x_0=609601.219202438 +y_0=0 +ellps=clrk66 +step +proj=unitconvert +xy_in=m +xy_out=us-ft"_L1 )
  {
    QCOMPARE( def.proj, u"+proj=pipeline +step +proj=unitconvert +xy_in=deg +xy_out=rad +step +inv +proj=hgridshift +grids=conus +step +proj=lcc +lat_0=37.6666666666667 +lon_0=-122 +lat_1=39.8333333333333 +lat_2=38.3333333333333 +x_0=609601.219202438 +y_0=0 +ellps=clrk66 +step +proj=unitconvert +xy_in=m +xy_out=us-ft"_s );
  }
  else if ( def.proj == "+proj=pipeline +step +proj=unitconvert +xy_in=deg +xy_out=rad +step +inv +proj=hgridshift +grids=us_noaa_cnhpgn.tif +step +inv +proj=hgridshift +grids=us_noaa_conus.tif +step +proj=lcc +lat_0=37.6666666666667 +lon_0=-122 +lat_1=39.8333333333333 +lat_2=38.3333333333333 +x_0=609601.219202438 +y_0=0 +ellps=clrk66 +step +proj=unitconvert +xy_in=m +xy_out=us-ft"_L1 )
  {
    QCOMPARE( def.proj, u"+proj=pipeline +step +proj=unitconvert +xy_in=deg +xy_out=rad +step +inv +proj=hgridshift +grids=us_noaa_cnhpgn.tif +step +inv +proj=hgridshift +grids=us_noaa_conus.tif +step +proj=lcc +lat_0=37.6666666666667 +lon_0=-122 +lat_1=39.8333333333333 +lat_2=38.3333333333333 +x_0=609601.219202438 +y_0=0 +ellps=clrk66 +step +proj=unitconvert +xy_in=m +xy_out=us-ft"_s );
  }
  else if ( def.proj == "+proj=pipeline +step +proj=unitconvert +xy_in=deg +xy_out=rad +step +inv +proj=hgridshift +grids=us_noaa_conus.tif +step +proj=lcc +lat_0=37.6666666666667 +lon_0=-122 +lat_1=39.8333333333333 +lat_2=38.3333333333333 +x_0=609601.219202438 +y_0=0 +ellps=clrk66 +step +proj=unitconvert +xy_in=m +xy_out=us-ft"_L1 )
  {
    QCOMPARE( def.proj, u"+proj=pipeline +step +proj=unitconvert +xy_in=deg +xy_out=rad +step +inv +proj=hgridshift +grids=us_noaa_conus.tif +step +proj=lcc +lat_0=37.6666666666667 +lon_0=-122 +lat_1=39.8333333333333 +lat_2=38.3333333333333 +x_0=609601.219202438 +y_0=0 +ellps=clrk66 +step +proj=unitconvert +xy_in=m +xy_out=us-ft"_s );
  }
  else
  {
    QCOMPARE( def.proj, u"+proj=pipeline +step +proj=unitconvert +xy_in=deg +xy_out=rad +step +proj=push +v_3 +step +proj=cart +ellps=WGS84 +step +proj=helmert +x=8 +y=-159 +z=-175 +step +inv +proj=cart +ellps=clrk66 +step +proj=pop +v_3 +step +proj=lcc +lat_0=37.6666666666667 +lon_0=-122 +lat_1=39.8333333333333 +lat_2=38.3333333333333 +x_0=609601.219202438 +y_0=0 +ellps=clrk66 +step +proj=unitconvert +xy_in=m +xy_out=us-ft"_s );
  }
}

void TestQgsDatumTransformDialog::fallback()
{
  // don't default to allow fallback
  QgsDatumTransformDialog dlg( QgsCoordinateReferenceSystem( u"EPSG:7844"_s ), QgsCoordinateReferenceSystem( u"EPSG:4283"_s ), false, true, false, qMakePair( -1, -1 ), nullptr, Qt::WindowFlags(), QString(), nullptr, false );

  const QgsDatumTransformDialog::TransformInfo def = dlg.selectedDatumTransform();
  QCOMPARE( def.sourceCrs.authid(), u"EPSG:7844"_s );
  QCOMPARE( def.destinationCrs.authid(), u"EPSG:4283"_s );
  QCOMPARE( def.proj, u"+proj=pipeline +step +proj=unitconvert +xy_in=deg +xy_out=rad +step +proj=push +v_3 +step +proj=cart +ellps=GRS80 +step +inv +proj=helmert +x=0.06155 +y=-0.01087 +z=-0.04019 +rx=-0.0394924 +ry=-0.0327221 +rz=-0.0328979 +s=-0.009994 +convention=coordinate_frame +step +inv +proj=cart +ellps=GRS80 +step +proj=pop +v_3 +step +proj=unitconvert +xy_in=rad +xy_out=deg"_s );
  QVERIFY( !def.allowFallback );
}

void TestQgsDatumTransformDialog::shouldAskUser()
{
  // no prompts!
  QgsSettings().setValue( u"/projections/promptWhenMultipleTransformsExist"_s, false, QgsSettings::App );
  const QgsDatumTransformDialog dlg( QgsCoordinateReferenceSystem( u"EPSG:26742"_s ), QgsCoordinateReferenceSystem( u"EPSG:4326"_s ) );
  QVERIFY( !dlg.shouldAskUserForSelection() );
  const QgsDatumTransformDialog dlg2( QgsCoordinateReferenceSystem( u"EPSG:26742"_s ), QgsCoordinateReferenceSystem( u"EPSG:7406"_s ) );
  QVERIFY( !dlg2.shouldAskUserForSelection() );

  //prompts
  QgsSettings().setValue( u"/projections/promptWhenMultipleTransformsExist"_s, true, QgsSettings::App );
  const QgsDatumTransformDialog dlg3( QgsCoordinateReferenceSystem( u"EPSG:26742"_s ), QgsCoordinateReferenceSystem( u"EPSG:4326"_s ) );
  QVERIFY( dlg3.shouldAskUserForSelection() );
  const QgsDatumTransformDialog dlg4( QgsCoordinateReferenceSystem( u"EPSG:26742"_s ), QgsCoordinateReferenceSystem( u"EPSG:7406"_s ) );
  QVERIFY( !dlg4.shouldAskUserForSelection() );
}

void TestQgsDatumTransformDialog::applyDefaultTransform()
{
  QgsSettings().setValue( u"/projections/promptWhenMultipleTransformsExist"_s, false, QgsSettings::App );

  Q_NOWARN_DEPRECATED_PUSH
  QgsDatumTransformDialog dlg( QgsCoordinateReferenceSystem( u"EPSG:26742"_s ), QgsCoordinateReferenceSystem( u"EPSG:7406"_s ) );
  dlg.applyDefaultTransform();
  QVERIFY( QgsProject::instance()->transformContext().sourceDestinationDatumTransforms().isEmpty() );
  QVERIFY( QgsProject::instance()->transformContext().coordinateOperations().isEmpty() );
  QgsProject::instance()->transformContext().addCoordinateOperation( QgsCoordinateReferenceSystem( u"EPSG:26742"_s ), QgsCoordinateReferenceSystem( u"EPSG:4326"_s ), u"some proj"_s );

  QgsDatumTransformDialog dlg2( QgsCoordinateReferenceSystem( u"EPSG:26742"_s ), QgsCoordinateReferenceSystem( u"EPSG:4326"_s ) );
  dlg2.applyDefaultTransform();

  // a default transform means there should be no entry in the context
  QVERIFY( QgsProject::instance()->transformContext().coordinateOperations().isEmpty() );
  Q_NOWARN_DEPRECATED_POP
  QgsProject::instance()->clear();
}

void TestQgsDatumTransformDialog::runDialog()
{
  QgsSettings().setValue( u"/projections/promptWhenMultipleTransformsExist"_s, false, QgsSettings::App );
  QVERIFY( QgsDatumTransformDialog::run( QgsCoordinateReferenceSystem( u"EPSG:26742"_s ), QgsCoordinateReferenceSystem( u"EPSG:7406"_s ) ) );
  QVERIFY( QgsProject::instance()->transformContext().coordinateOperations().isEmpty() );

  QVERIFY( QgsDatumTransformDialog::run( QgsCoordinateReferenceSystem( u"EPSG:26742"_s ), QgsCoordinateReferenceSystem( u"EPSG:4326"_s ) ) );
  QVERIFY( QgsProject::instance()->transformContext().coordinateOperations().isEmpty() );
  QgsProject::instance()->clear();
}


QGSTEST_MAIN( TestQgsDatumTransformDialog )
#include "testqgsdatumtransformdialog.moc"
