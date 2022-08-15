/***************************************************************************
  testqgselevationmap.cpp
  --------------------------------------
Date                 : August 2022
Copyright            : (C) 2022 by Martin Dobias
Email                : wonder dot sk at gmail dot com
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/
#include "qgstest.h"

#include "qgsapplication.h"
#include "qgselevationmap.h"
#include "qgsrasterlayer.h"
#include "qgsrenderchecker.h"


static QString _fileNameForTest( const QString &testName )
{
  return QDir::tempPath() + '/' + testName + ".png";
}

static bool _verifyImage( const QString &testName, QString &report )
{
  QgsRenderChecker checker;
  checker.setControlPathPrefix( QStringLiteral( "elevation_map" ) );
  checker.setControlName( "expected_" + testName );
  checker.setRenderedImage( _fileNameForTest( testName ) );
  checker.setSizeTolerance( 3, 3 );
  const bool equal = checker.compareImages( testName, 500 );
  report += checker.report();
  return equal;
}


class TestQgsElevationMap : public QObject
{
    Q_OBJECT
  private slots:
    void initTestCase();
    void cleanupTestCase();
    void testRasterDemEdl();

  private:
    QString mReport;
};


void TestQgsElevationMap::initTestCase()
{
  //
  // Runs once before any tests are run
  //
  // init QGIS's paths - true means that all path will be inited from prefix
  QgsApplication::init();
  QgsApplication::initQgis();

  mReport += QLatin1String( "<h1>Elevation Map Tests</h1>\n" );
}

void TestQgsElevationMap::cleanupTestCase()
{
  const QString myReportFile = QDir::tempPath() + "/qgistest.html";
  QFile myFile( myReportFile );
  if ( myFile.open( QIODevice::WriteOnly | QIODevice::Append ) )
  {
    QTextStream myQTextStream( &myFile );
    myQTextStream << mReport;
    myFile.close();
  }

  QgsApplication::exitQgis();
}


void TestQgsElevationMap::testRasterDemEdl()
{

  QString testDataDir = QStringLiteral( TEST_DATA_DIR ); //defined in CmakeLists.txt
  QgsRasterLayer r( testDataDir + "/analysis/dem.tif" );
  QVERIFY( r.isValid() );

  const QgsRectangle fullExtent = r.extent();
  const int width = r.width();
  const int height = r.height();

  std::unique_ptr<QgsRasterBlock> block( r.dataProvider()->block( 1, fullExtent, width, height ) );
  QVERIFY( block );

  QImage img( block->width(), block->height(), QImage::Format_RGB32 );
  img.fill( Qt::cyan );

  std::unique_ptr<QgsElevationMap> elevationMap( QgsElevationMap::fromRasterBlock( block.get() ) );
  elevationMap->applyEyeDomeLighting( img, 2, 100, 10000 );

  img.save( _fileNameForTest( QStringLiteral( "dem_edl" ) ) );
  QVERIFY( _verifyImage( "dem_edl", mReport ) );
}


QGSTEST_MAIN( TestQgsElevationMap )
#include "testqgselevationmap.moc"
