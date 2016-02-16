/***************************************************************************
     testqgsrastersublayer.cpp
     --------------------------------------
    Date                 : Dec 2012
    Copyright            : (C) 2012 by Radim Blazek
    Email                : radim.blazek@gmail.com
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
#include <QString>
#include <QStringList>
#include <QApplication>
#include <QFileInfo>
#include <QDir>
#include <QPainter>
#include <QTime>
#include <QDesktopServices>

#include <gdal.h>
#include "cpl_conv.h"

//qgis includes...
#include <qgsrasterlayer.h>
#include <qgsrasterpyramid.h>
#include <qgsrasterbandstats.h>
#include <qgsmaplayerregistry.h>
#include <qgsapplication.h>
#include <qgsmaprenderer.h>
#include <qgssinglebandgrayrenderer.h>
#include <qgssinglebandpseudocolorrenderer.h>
#include <qgsvectorcolorrampv2.h>
#include <qgscptcityarchive.h>

//qgis unit test includes
#include <qgsrenderchecker.h>

/** \ingroup UnitTests
 * This is a unit test for raster sublayers
 */
class TestQgsRasterSubLayer : public QObject
{
    Q_OBJECT

  public:
    TestQgsRasterSubLayer();

  private slots:
    void initTestCase();// will be called before the first testfunction is executed.
    void cleanupTestCase();// will be called after the last testfunction was executed.
    void init() {}// will be called before each testfunction is executed.
    void cleanup() {}// will be called after every testfunction.

    void subLayersList();
    void checkStats();
  private:
    QString mTestDataDir;
    QString mFileName;
    QgsRasterLayer * mpRasterLayer;
    QString mReport;
    bool mHasNetCDF;
};

TestQgsRasterSubLayer::TestQgsRasterSubLayer()
    : mpRasterLayer( nullptr )
    , mHasNetCDF( false )
{

}

//runs before all tests
void TestQgsRasterSubLayer::initTestCase()
{
  // init QGIS's paths - true means that all path will be inited from prefix
  QgsApplication::init();
  QgsApplication::initQgis();
  // disable any PAM stuff to make sure stats are consistent
  CPLSetConfigOption( "GDAL_PAM_ENABLED", "NO" );
  QString mySettings = QgsApplication::showSettings();
  mySettings = mySettings.replace( '\n', "<br />" );
  mTestDataDir = QString( TEST_DATA_DIR ) + '/'; //defined in CmakeLists.txt

  GDALAllRegister();
  QString format = "netCDF";
  GDALDriverH myGdalDriver = GDALGetDriverByName( format.toLocal8Bit().constData() );
  mHasNetCDF = myGdalDriver != 0;

  mFileName = mTestDataDir + "landsat2.nc";

  mReport += "<h1>Raster Sub Layer Tests</h1>\n";
  //mReport += "<p>" + mySettings + "</p>";

  if ( mHasNetCDF )
  {
    QFileInfo myRasterFileInfo( mFileName );
    mpRasterLayer = new QgsRasterLayer( myRasterFileInfo.filePath(),
                                        myRasterFileInfo.completeBaseName() );
    qDebug() << "raster metadata: " << mpRasterLayer->dataProvider()->metadata();
    mReport += "raster metadata: " + mpRasterLayer->dataProvider()->metadata();
  }
  else
  {
    mReport += "<p>NetCDF format is not compiled in GDAL library, cannot test sub layers.</p>";
  }
}

//runs after all tests
void TestQgsRasterSubLayer::cleanupTestCase()
{
  delete mpRasterLayer;
  QgsApplication::exitQgis();
  QString myReportFile = QDir::tempPath() + "/qgistest.html";
  QFile myFile( myReportFile );
  if ( myFile.open( QIODevice::WriteOnly | QIODevice::Append ) )
  {
    QTextStream myQTextStream( &myFile );
    myQTextStream << mReport;
    myFile.close();
  }
}

void TestQgsRasterSubLayer::subLayersList()
{
  if ( mHasNetCDF )
  {
    mReport += "<h2>Check Sublayers List</h2>\n";
    // Layer with sublayers is not valid
    //QVERIFY( mpRasterLayer->isValid() );
    QStringList expected;
    // Sublayer format: NETCDF:"/path/to/landsat2.nc":Band1
    //                  NETCDF:"c:/path/to/landsat2.nc":Band1
    // File path is delicate on Windows -> compare only sublayers
    expected << "Band1";
    expected << "Band2";

    QStringList sublayers;
    Q_FOREACH ( const QString& s, mpRasterLayer->subLayers() )
    {
      qDebug() << "sublayer: " << s;
      sublayers << s.split( ':' ).last();
    }
    qDebug() << "sublayers: " << sublayers.join( "," );
    mReport += QString( "sublayers:<br>%1<br>\n" ).arg( sublayers.join( "<br>" ) );
    mReport += QString( "expected:<br>%1<br>\n" ).arg( expected.join( "<br>" ) );
    QVERIFY( sublayers == expected );
    mReport += "<p>Passed</p>";
  }
}

void TestQgsRasterSubLayer::checkStats()
{
  if ( mHasNetCDF )
  {
    mReport += "<h2>Check Stats</h2>\n";
    QString sublayerUri = mpRasterLayer->subLayers().value( 0 );
    mReport += "sublayer: " + sublayerUri + "<br>\n";

    QgsRasterLayer *sublayer = new QgsRasterLayer( sublayerUri, "Sublayer 1" );

    QgsRasterBandStats myStatistics = sublayer->dataProvider()->bandStatistics( 1,
                                      QgsRasterBandStats::Min | QgsRasterBandStats::Max );
    int width = 200;
    int height = 200;
    double min = 122;
    double max = 130;
    mReport += QString( "width = %1 expected = %2<br>\n" ).arg( sublayer->width() ).arg( width );
    mReport += QString( "height = %1 expected = %2<br>\n" ).arg( sublayer->height() ).arg( height );
    mReport += QString( "min = %1 expected = %2<br>\n" ).arg( myStatistics.minimumValue ).arg( min );
    mReport += QString( "max = %1 expected = %2<br>\n" ).arg( myStatistics.maximumValue ).arg( max );

    QVERIFY( sublayer->width() == width );
    QVERIFY( sublayer->height() == height );
    QVERIFY( qgsDoubleNear( myStatistics.minimumValue, min ) );
    QVERIFY( qgsDoubleNear( myStatistics.maximumValue, max ) );
    mReport += "<p>Passed</p>";
    delete sublayer;
  }
}


QTEST_MAIN( TestQgsRasterSubLayer )
#include "testqgsrastersublayer.moc"
