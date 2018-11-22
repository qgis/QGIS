/***************************************************************************
     testqgsgdalutils.cpp
     --------------------
    begin                : September 2018
    copyright            : (C) 2018 Even Rouault
    email                : even.rouault at spatialys.com
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
#include <QString>
#include <QStringList>
#include <QSettings>

#include <gdal.h>

#include "qgsgdalutils.h"
#include "qgsapplication.h"

class TestQgsGdalUtils: public QObject
{
    Q_OBJECT

  private slots:
    void initTestCase();// will be called before the first testfunction is executed.
    void cleanupTestCase();// will be called after the last testfunction was executed.
    void init();// will be called before each testfunction is executed.
    void cleanup();// will be called after every testfunction.
    void supportsRasterCreate();

  private:
};

void TestQgsGdalUtils::initTestCase()
{
  QgsApplication::init();
  QgsApplication::initQgis();
  GDALAllRegister();
}

void TestQgsGdalUtils::cleanupTestCase()
{
  QgsApplication::exitQgis();
}

void TestQgsGdalUtils::init()
{

}

void TestQgsGdalUtils::cleanup()
{

}

void TestQgsGdalUtils::supportsRasterCreate()
{
  QVERIFY( QgsGdalUtils::supportsRasterCreate( GDALGetDriverByName( "GTiff" ) ) );
  QVERIFY( QgsGdalUtils::supportsRasterCreate( GDALGetDriverByName( "GPKG" ) ) );

  // special case
  QVERIFY( !QgsGdalUtils::supportsRasterCreate( GDALGetDriverByName( "SQLite" ) ) );

  // create-only
  QVERIFY( !QgsGdalUtils::supportsRasterCreate( GDALGetDriverByName( "DTED" ) ) );

  // vector-only
  QVERIFY( !QgsGdalUtils::supportsRasterCreate( GDALGetDriverByName( "ESRI Shapefile" ) ) );
}

QGSTEST_MAIN( TestQgsGdalUtils )
#include "testqgsgdalutils.moc"
