/***************************************************************************
     testqgsdecorationscalebar.cpp
     ----------------------
    Date                 : 2021-01-19
    Copyright            : (C) 2021 by Nyall Dawson
    Email                : nyall dot dawson at gmail dot com
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/
#include "qgisapp.h"
#include "qgsapplication.h"
#include "qgsdecorationscalebar.h"
#include "qgsmapsettings.h"
#include "qgstest.h"

class TestQgsDecorationScalebar : public QObject
{
    Q_OBJECT
  public:
    TestQgsDecorationScalebar();

  private slots:
    void initTestCase();    // will be called before the first testfunction is executed.
    void cleanupTestCase(); // will be called after the last testfunction was executed.
    void init() {}          // will be called before each testfunction is executed.
    void cleanup() {}       // will be called after every testfunction.
    void mapWidth();

  private:
    QgisApp *mQgisApp = nullptr;
};

TestQgsDecorationScalebar::TestQgsDecorationScalebar() = default;

//runs before all tests
void TestQgsDecorationScalebar::initTestCase()
{
  qDebug() << "TestQgsDecorationScalebar::initTestCase()";
  // init QGIS's paths - true means that all path will be inited from prefix
  QgsApplication::init();
  QgsApplication::initQgis();

  // Set up the QgsSettings environment
  QCoreApplication::setOrganizationName( u"QGIS"_s );
  QCoreApplication::setOrganizationDomain( u"qgis.org"_s );
  QCoreApplication::setApplicationName( u"QGIS-TEST"_s );

  mQgisApp = new QgisApp();

  // enforce C locale because the tests expect it
  // (decimal separators / thousand separators)
  QLocale::setDefault( QLocale::c() );
}

//runs after all tests
void TestQgsDecorationScalebar::cleanupTestCase()
{
  QgsApplication::exitQgis();
}

void TestQgsDecorationScalebar::mapWidth()
{
  QgsProject::instance()->setCrs( QgsCoordinateReferenceSystem( u"EPSG:3857"_s ) );
  QgsMapSettings settings;
  settings.setDestinationCrs( QgsCoordinateReferenceSystem( u"EPSG:3857"_s ) );
  settings.setOutputSize( QSize( 800, 400 ) );
  // same aspect ratio as output size
  settings.setExtent( QgsRectangle( 16700000, -4210000, 16708000, -4206000 ) );

  // unknown units, no conversion
  QgsDecorationScaleBar scalebar;
  QGSCOMPARENEAR( scalebar.mapWidth( settings ), 8000, 0.000001 );

  // Cartesian measure
  QgsProject::instance()->setEllipsoid( QString() );
  scalebar.mSettings.setUnits( Qgis::DistanceUnit::Miles );
  QGSCOMPARENEAR( scalebar.mapWidth( settings ), 4.97097, 0.0001 );

  // ellipsoidal measure
  QgsProject::instance()->setEllipsoid( u"EPSG:7030"_s );
  QGSCOMPARENEAR( scalebar.mapWidth( settings ), 4.060337, 0.0001 );
  QgsProject::instance()->setEllipsoid( QString() );

  // with non-uniform output size vs extent aspect ratio
  settings.setExtent( QgsRectangle( 16700000, -4212000, 16708000, -4204000 ) );
  QGSCOMPARENEAR( scalebar.mapWidth( settings ), 9.941939, 0.0001 );
  settings.setExtent( settings.visibleExtent() );
  QGSCOMPARENEAR( scalebar.mapWidth( settings ), 9.941939, 0.0001 );
}

QGSTEST_MAIN( TestQgsDecorationScalebar )
#include "testqgsdecorationscalebar.moc"
