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
#include <cpl_conv.h>
#include <gdal.h>

#include "qgstest.h"

#include <QApplication>
#include <QDesktopServices>
#include <QDir>
#include <QFileInfo>
#include <QObject>
#include <QPainter>
#include <QString>
#include <QStringList>
#include <QTime>

//qgis includes...
#include <qgsrasterlayer.h>
#include <qgsrasterpyramid.h>
#include <qgsrasterbandstats.h>
#include "qgsrasterdataprovider.h"
#include <qgsapplication.h>
#include <qgssinglebandgrayrenderer.h>
#include <qgssinglebandpseudocolorrenderer.h>
#include <qgscolorramp.h>
#include <qgscptcityarchive.h>

//qgis unit test includes
#include <qgsrenderchecker.h>

/**
 * \ingroup UnitTests
 * This is a unit test for raster sublayers
 */
class TestQgsRasterSubLayer : public QgsTest
{
    Q_OBJECT

  public:
    TestQgsRasterSubLayer()
      : QgsTest( u"Raster Sub Layer Tests"_s ) {}

  private slots:
    void initTestCase();    // will be called before the first testfunction is executed.
    void cleanupTestCase(); // will be called after the last testfunction was executed.
    void init() {}          // will be called before each testfunction is executed.
    void cleanup() {}       // will be called after every testfunction.

    void subLayersList();
    void checkStats();

  private:
    QString mTestDataDir;
    QString mFileName;
    QgsRasterLayer *mpRasterLayer = nullptr;
    bool mHasNetCDF = false;
};

//runs before all tests
void TestQgsRasterSubLayer::initTestCase()
{
  // init QGIS's paths - true means that all path will be inited from prefix
  QgsApplication::init();
  QgsApplication::initQgis();
  // disable any PAM stuff to make sure stats are consistent
  CPLSetConfigOption( "GDAL_PAM_ENABLED", "NO" );

  mTestDataDir = QStringLiteral( TEST_DATA_DIR ) + '/'; //defined in CmakeLists.txt

  GDALAllRegister();
  const QString format = u"netCDF"_s;
  GDALDriverH myGdalDriver = GDALGetDriverByName( format.toLocal8Bit().constData() );
  mHasNetCDF = myGdalDriver != nullptr;

  mFileName = mTestDataDir + "landsat2.nc";

  if ( mHasNetCDF )
  {
    const QFileInfo myRasterFileInfo( mFileName );
    mpRasterLayer = new QgsRasterLayer( myRasterFileInfo.filePath(), myRasterFileInfo.completeBaseName() );
  }
  else
  {
    mReport += "<p>NetCDF format is not compiled in GDAL library, cannot test sub layers.</p>"_L1;
  }
}

//runs after all tests
void TestQgsRasterSubLayer::cleanupTestCase()
{
  delete mpRasterLayer;
  QgsApplication::exitQgis();
}

void TestQgsRasterSubLayer::subLayersList()
{
  if ( !mHasNetCDF )
    return;

  const QString oldReport = mReport;

  mReport += "<h2>Check Sublayers List</h2>\n"_L1;
  // Layer with sublayers is not valid
  QVERIFY( !mpRasterLayer->isValid() );

  QStringList expected;
  // Sublayer format: NETCDF:"/path/to/landsat2.nc":Band1!!::!![200x200] Band1 (8-bit integer)
  //                  NETCDF:"c:/path/to/landsat2.nc":Band1!!::!![200x200] Band1 (8-bit integer)
  // File path is delicate on Windows -> compare only end of string

  expected << u":Band1!!::!![200x200] Band1 (8-bit integer)"_s;
  expected << u":Band2!!::!![200x200] Band2 (8-bit integer)"_s;

  QStringList sublayers;
  for ( const QString &s : mpRasterLayer->subLayers() )
  {
    qDebug() << "sublayer: " << s;
    const int pos = s.indexOf( ":Band"_L1 );
    if ( pos > 0 )
      sublayers << s.mid( pos );
    else
      sublayers << s;
  }
  qDebug() << "sublayers: " << sublayers.join( ','_L1 );
  mReport += u"sublayers:<br>%1<br>\n"_s.arg( sublayers.join( "<br>"_L1 ) );
  mReport += u"expected:<br>%1<br>\n"_s.arg( expected.join( "<br>"_L1 ) );

  QCOMPARE( sublayers, expected );

  // don't include reports for passing tests
  mReport = oldReport;
}

void TestQgsRasterSubLayer::checkStats()
{
  if ( !mHasNetCDF )
    return;

  const QString oldReport = mReport;

  mReport += "<h2>Check Stats</h2>\n"_L1;
  QString sublayerUri = mpRasterLayer->subLayers().value( 0 );
  mReport += "sublayer: " + sublayerUri + "<br>\n";

  sublayerUri = sublayerUri.split( QgsDataProvider::sublayerSeparator() )[0];
  QgsRasterLayer *sublayer = new QgsRasterLayer( sublayerUri, u"Sublayer 1"_s );

  const QgsRasterBandStats myStatistics = sublayer->dataProvider()->bandStatistics( 1, Qgis::RasterBandStatistic::Min | Qgis::RasterBandStatistic::Max );
  const int width = 200;
  const int height = 200;
  const double min = 122;
  const double max = 130;
  mReport += u"width = %1 expected = %2<br>\n"_s.arg( sublayer->width() ).arg( width );
  mReport += u"height = %1 expected = %2<br>\n"_s.arg( sublayer->height() ).arg( height );
  mReport += u"min = %1 expected = %2<br>\n"_s.arg( myStatistics.minimumValue ).arg( min );
  mReport += u"max = %1 expected = %2<br>\n"_s.arg( myStatistics.maximumValue ).arg( max );

  QVERIFY( sublayer->width() == width );
  QVERIFY( sublayer->height() == height );
  QGSCOMPARENEAR( myStatistics.minimumValue, min, 4 * std::numeric_limits<double>::epsilon() );
  QGSCOMPARENEAR( myStatistics.maximumValue, max, 4 * std::numeric_limits<double>::epsilon() );
  mReport += "<p>Passed</p>"_L1;
  delete sublayer;
  // don't include reports for passing tests
  mReport = oldReport;
}


QGSTEST_MAIN( TestQgsRasterSubLayer )
#include "testqgsrastersublayer.moc"
