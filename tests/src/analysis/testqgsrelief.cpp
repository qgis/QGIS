/***************************************************************************
                         testqgsrelief.cpp
                         ---------------------
    begin                : August 2026
    copyright            : (C) 2026 by ALexander Bruy
    email                : alexander dot bruy at gmail dot com
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "qgsrasterlayer.h"
#include "qgsrelief.h"
#include "qgstest.h"

#include <QString>

using namespace Qt::StringLiterals;

class TestQgsRelief : public QgsTest
{
    Q_OBJECT

  public:
    TestQgsRelief()
      : QgsTest( u"Relief Tests"_s )
    {}

    QString SRC_FILE;

  private slots:

    void initTestCase();
    void cleanupTestCase();
    void init();
    void cleanup();

    void testCreationOptions();
    void testNoDataValue();

  private:
    static QString tempFile( const QString &name ) { return u"%1/relieftest-%2.tif"_s.arg( QDir::tempPath(), name ); }
};

void TestQgsRelief::initTestCase()
{
  SRC_FILE = QStringLiteral( TEST_DATA_DIR ) + "/analysis/dem.tif";

  QgsApplication::init();
}

void TestQgsRelief::cleanupTestCase()
{
  QgsApplication::exitQgis();
}

void TestQgsRelief::init()
{}

void TestQgsRelief::cleanup()
{}

void TestQgsRelief::testCreationOptions()
{
  QString tmpFile( tempFile( u"createopts"_s ) );

  QString worldFileName = tmpFile.replace( ".tif"_L1, ".tfw"_L1 );
  QFile worldFile( worldFileName );
  QVERIFY( !worldFile.exists() );

  QgsRelief relief( SRC_FILE, tmpFile, "GTiff" );
  QList<QgsRasterReliefColor> reliefColors = relief.calculateOptimizedReliefClasses();
  relief.setReliefColors( reliefColors );
  relief.setCreationOptions( QStringList() << "TFW=YES" );
  QCOMPARE( static_cast<int>( relief.processRaster() ), 0 );

  QVERIFY( worldFile.exists() );
  worldFile.remove();
}

void TestQgsRelief::testNoDataValue()
{
  QString tmpFile( tempFile( u"nodata"_s ) );

  QgsRelief relief( SRC_FILE, tmpFile, "GTiff" );
  QList<QgsRasterReliefColor> reliefColors = relief.calculateOptimizedReliefClasses();
  relief.setReliefColors( reliefColors );
  relief.setOutputNodataValue( 255 );
  QCOMPARE( static_cast<int>( relief.processRaster() ), 0 );

  const std::unique_ptr<QgsRasterLayer> result = std::make_unique<QgsRasterLayer>( tmpFile, u"raster"_s, u"gdal"_s );
  QVERIFY( result->dataProvider()->sourceHasNoDataValue( 1 ) );
  QCOMPARE( result->dataProvider()->sourceNoDataValue( 1 ), 255 );
}

QGSTEST_MAIN( TestQgsRelief )

#include "testqgsrelief.moc"
