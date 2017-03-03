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
#include "qgstest.h"
#include <QObject>

#include <qgsapplication.h>
#include "qgspathresolver.h"
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
    void testPathResolver();
    void testProjectUnits();
    void variablesChanged();
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
  QCoreApplication::setOrganizationName( QStringLiteral( "QGIS" ) );
  QCoreApplication::setOrganizationDomain( QStringLiteral( "qgis.org" ) );
  QCoreApplication::setApplicationName( QStringLiteral( "QGIS-TEST" ) );
}


void TestQgsProject::cleanupTestCase()
{
  // Runs once after all tests are run
}

void TestQgsProject::testReadPath()
{
  QgsProject *prj = new QgsProject;
  // this is a bit hacky as we do not really load such project
  QString prefix;
#if defined(Q_OS_WIN)
  prefix = "C:";
#endif
  prj->setFileName( prefix + "/home/qgis/a-project-file.qgs" ); // not expected to exist
  // make sure we work with relative paths!
  prj->writeEntry( QStringLiteral( "Paths" ), QStringLiteral( "Absolute" ), false );

  QCOMPARE( prj->readPath( "./x.shp" ), QString( prefix + "/home/qgis/x.shp" ) );
  QCOMPARE( prj->readPath( "../x.shp" ), QString( prefix + "/home/x.shp" ) );

  // TODO: old style (seems QGIS < 1.3) - needs existing project file and existing file
  // QCOMPARE( prj->readPath( "x.shp" ), QString( "/home/qgis/x.shp" ) );

  // VSI: /vsizip, /vsitar, /vsigzip, *.zip, *.gz, *.tgz, ...

  QCOMPARE( prj->readPath( "./x.gz" ), QString( prefix + "/home/qgis/x.gz" ) );
  QCOMPARE( prj->readPath( "/vsigzip/./x.gz" ), QString( "/vsigzip/%1/home/qgis/x.gz" ).arg( prefix ) ); // not sure how useful this really is...

  delete prj;
}

void TestQgsProject::testPathResolver()
{
  QgsPathResolver resolverRel( "/home/qgis/test.qgs" );
  QCOMPARE( resolverRel.writePath( "/home/qgis/file1.txt" ), QString( "./file1.txt" ) );
  QCOMPARE( resolverRel.writePath( "/home/qgis/subdir/file1.txt" ), QString( "./subdir/file1.txt" ) );
  QCOMPARE( resolverRel.writePath( "/home/file1.txt" ), QString( "../file1.txt" ) );
  QCOMPARE( resolverRel.readPath( "./file1.txt" ), QString( "/home/qgis/file1.txt" ) );
  QCOMPARE( resolverRel.readPath( "./subdir/file1.txt" ), QString( "/home/qgis/subdir/file1.txt" ) );
  QCOMPARE( resolverRel.readPath( "../file1.txt" ), QString( "/home/file1.txt" ) );
  QCOMPARE( resolverRel.readPath( "/home/qgis/file1.txt" ), QString( "/home/qgis/file1.txt" ) );

  QgsPathResolver resolverAbs;
  QCOMPARE( resolverAbs.writePath( "/home/qgis/file1.txt" ), QString( "/home/qgis/file1.txt" ) );
  QCOMPARE( resolverAbs.readPath( "/home/qgis/file1.txt" ), QString( "/home/qgis/file1.txt" ) );
  QCOMPARE( resolverAbs.readPath( "./file1.txt" ), QString( "./file1.txt" ) );
}

void TestQgsProject::testProjectUnits()
{
  //test setting and retrieving project units

  // DISTANCE

  //first set a default QGIS distance unit
  QSettings s;
  s.setValue( QStringLiteral( "/qgis/measure/displayunits" ), QgsUnitTypes::encodeUnit( QgsUnitTypes::DistanceFeet ) );

  QgsProject *prj = new QgsProject;
  // new project should inherit QGIS default distance unit
  prj->clear();
  QCOMPARE( prj->distanceUnits(), QgsUnitTypes::DistanceFeet );

  //changing default QGIS unit should not affect existing project
  s.setValue( QStringLiteral( "/qgis/measure/displayunits" ), QgsUnitTypes::encodeUnit( QgsUnitTypes::DistanceNauticalMiles ) );
  QCOMPARE( prj->distanceUnits(), QgsUnitTypes::DistanceFeet );

  //test setting new units for project
  prj->setDistanceUnits( QgsUnitTypes::DistanceNauticalMiles );
  QCOMPARE( prj->distanceUnits(), QgsUnitTypes::DistanceNauticalMiles );

  // AREA

  //first set a default QGIS area unit
  s.setValue( QStringLiteral( "/qgis/measure/areaunits" ), QgsUnitTypes::encodeUnit( QgsUnitTypes::AreaSquareYards ) );

  // new project should inherit QGIS default area unit
  prj->clear();
  QCOMPARE( prj->areaUnits(), QgsUnitTypes::AreaSquareYards );

  //changing default QGIS unit should not affect existing project
  s.setValue( QStringLiteral( "/qgis/measure/areaunits" ), QgsUnitTypes::encodeUnit( QgsUnitTypes::AreaAcres ) );
  QCOMPARE( prj->areaUnits(), QgsUnitTypes::AreaSquareYards );

  //test setting new units for project
  prj->setAreaUnits( QgsUnitTypes::AreaAcres );
  QCOMPARE( prj->areaUnits(), QgsUnitTypes::AreaAcres );

  delete prj;
}

void TestQgsProject::variablesChanged()
{
  QgsProject *prj = new QgsProject;
  QSignalSpy spyVariablesChanged( prj, &QgsProject::customVariablesChanged );
  QVariantMap vars;
  vars.insert( QStringLiteral( "variable" ), QStringLiteral( "1" ) );
  prj->setCustomVariables( vars );
  QVERIFY( spyVariablesChanged.count() == 1 );
  delete prj;
}


QGSTEST_MAIN( TestQgsProject )
#include "testqgsproject.moc"
