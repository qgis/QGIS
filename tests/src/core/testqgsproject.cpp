/***************************************************************************
     testqgsproject.cpp
     --------------------------------------
    Date                 : June 2014
    Copyright            : (C) 2014 by Martin Dobias
    Email                : wonder.sk at gmail dot com
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/
#include <QtTest/QtTest>
#include <QObject>

#include <qgsapplication.h>
#include <qgsproject.h>
#include "qgsunittypes.h"


class TestQgsProject : public QObject
{
    Q_OBJECT
  private slots:
    void initTestCase();// will be called before the first testfunction is executed.
    void cleanupTestCase();// will be called after the last testfunction was executed.
    void init();// will be called before each testfunction is executed.
    void cleanup();// will be called after every testfunction.

    void testReadPath();
    void testProjectUnits();
};

void TestQgsProject::init()
{
}

void TestQgsProject::cleanup()
{
  // will be called after every testfunction.
}

void TestQgsProject::initTestCase()
{
  // Runs once before any tests are run

  // Set up the QSettings environment
  QCoreApplication::setOrganizationName( "QGIS" );
  QCoreApplication::setOrganizationDomain( "qgis.org" );
  QCoreApplication::setApplicationName( "QGIS-TEST" );
}


void TestQgsProject::cleanupTestCase()
{
  // Runs once after all tests are run
}

void TestQgsProject::testReadPath()
{
  QgsProject* prj = QgsProject::instance();
  // this is a bit hacky as we do not really load such project
  QString prefix;
#if defined(Q_OS_WIN)
  prefix = "C:";
#endif
  prj->setFileName( prefix + "/home/qgis/a-project-file.qgs" ); // not expected to exist
  // make sure we work with relative paths!
  prj->writeEntry( "Paths", "Absolute", false );

  QCOMPARE( prj->readPath( "./x.shp" ), QString( prefix + "/home/qgis/x.shp" ) );
  QCOMPARE( prj->readPath( "../x.shp" ), QString( prefix + "/home/x.shp" ) );

  // TODO: old style (seems QGIS < 1.3) - needs existing project file and existing file
  // QCOMPARE( prj->readPath( "x.shp" ), QString( "/home/qgis/x.shp" ) );

  // VSI: /vsizip, /vsitar, /vsigzip, *.zip, *.gz, *.tgz, ...

  QCOMPARE( prj->readPath( "./x.gz" ), QString( prefix + "/home/qgis/x.gz" ) );
  QCOMPARE( prj->readPath( "/vsigzip/./x.gz" ), QString( "/vsigzip/%1/home/qgis/x.gz" ).arg( prefix ) ); // not sure how useful this really is...

}

void TestQgsProject::testProjectUnits()
{
  //test setting and retrieving project units

  // DISTANCE

  //first set a default QGIS distance unit
  QSettings s;
  s.setValue( "/qgis/measure/displayunits", QgsUnitTypes::encodeUnit( QGis::Feet ) );

  QgsProject* prj = QgsProject::instance();
  // new project should inherit QGIS default distance unit
  prj->clear();
  QCOMPARE( prj->distanceUnits(), QGis::Feet );

  //changing default QGIS unit should not affect existing project
  s.setValue( "/qgis/measure/displayunits", QgsUnitTypes::encodeUnit( QGis::NauticalMiles ) );
  QCOMPARE( prj->distanceUnits(), QGis::Feet );

  //test setting new units for project
  prj->writeEntry( "Measurement", "/DistanceUnits", QgsUnitTypes::encodeUnit( QGis::NauticalMiles ) );
  QCOMPARE( prj->distanceUnits(), QGis::NauticalMiles );

  // AREA

  //first set a default QGIS area unit
  s.setValue( "/qgis/measure/areaunits", QgsUnitTypes::encodeUnit( QgsUnitTypes::SquareYards ) );

  // new project should inherit QGIS default area unit
  prj->clear();
  QCOMPARE( prj->areaUnits(), QgsUnitTypes::SquareYards );

  //changing default QGIS unit should not affect existing project
  s.setValue( "/qgis/measure/areaunits", QgsUnitTypes::encodeUnit( QgsUnitTypes::Acres ) );
  QCOMPARE( prj->areaUnits(), QgsUnitTypes::SquareYards );

  //test setting new units for project
  prj->writeEntry( "Measurement", "/AreaUnits", QgsUnitTypes::encodeUnit( QgsUnitTypes::Acres ) );
  QCOMPARE( prj->areaUnits(), QgsUnitTypes::Acres );
}


QTEST_MAIN( TestQgsProject )
#include "testqgsproject.moc"
