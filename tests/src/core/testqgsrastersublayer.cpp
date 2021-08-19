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
#include "qgstest.h"
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
    QgsRasterLayer *mpRasterLayer = nullptr;
    QString mReport;
    bool mHasNetCDF =  false ;
};

TestQgsRasterSubLayer::TestQgsRasterSubLayer() = default;

//runs before all tests
void TestQgsRasterSubLayer::initTestCase()
{
  // init QGIS's paths - true means that all path will be inited from prefix
  QgsApplication::init();
  QgsApplication::initQgis();
  // disable any PAM stuff to make sure stats are consistent
  CPLSetConfigOption( "GDAL_PAM_ENABLED", "NO" );
  QString mySettings = QgsApplication::showSettings();
  mySettings = mySettings.replace( '\n', QLatin1String( "<br />" ) );
  mTestDataDir = QStringLiteral( TEST_DATA_DIR ) + '/'; //defined in CmakeLists.txt

  GDALAllRegister();
  const QString format = QStringLiteral( "netCDF" );
  GDALDriverH myGdalDriver = GDALGetDriverByName( format.toLocal8Bit().constData() );
  mHasNetCDF = myGdalDriver != nullptr;

  mFileName = mTestDataDir + "landsat2.nc";

  mReport += QLatin1String( "<h1>Raster Sub Layer Tests</h1>\n" );
  //mReport += "<p>" + mySettings + "</p>";

  if ( mHasNetCDF )
  {
    const QFileInfo myRasterFileInfo( mFileName );
    mpRasterLayer = new QgsRasterLayer( myRasterFileInfo.filePath(),
                                        myRasterFileInfo.completeBaseName() );
    qDebug() << "raster metadata: " << mpRasterLayer->dataProvider()->htmlMetadata();
    mReport += "raster metadata: " + mpRasterLayer->dataProvider()->htmlMetadata();
  }
  else
  {
    mReport += QLatin1String( "<p>NetCDF format is not compiled in GDAL library, cannot test sub layers.</p>" );
  }
}

//runs after all tests
void TestQgsRasterSubLayer::cleanupTestCase()
{
  delete mpRasterLayer;
  QgsApplication::exitQgis();
  const QString myReportFile = QDir::tempPath() + "/qgistest.html";
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
  if ( !mHasNetCDF )
    return;

  mReport += QLatin1String( "<h2>Check Sublayers List</h2>\n" );
  // Layer with sublayers is not valid
  QVERIFY( !mpRasterLayer->isValid() );

  QStringList expected;
  // Sublayer format: NETCDF:"/path/to/landsat2.nc":Band1!!::!![200x200] Band1 (8-bit integer)
  //                  NETCDF:"c:/path/to/landsat2.nc":Band1!!::!![200x200] Band1 (8-bit integer)
  // File path is delicate on Windows -> compare only end of string

  expected << QStringLiteral( ":Band1!!::!![200x200] Band1 (8-bit integer)" );
  expected << QStringLiteral( ":Band2!!::!![200x200] Band2 (8-bit integer)" );

  QStringList sublayers;
  for ( const QString &s : mpRasterLayer->subLayers() )
  {
    qDebug() << "sublayer: " << s;
    const int pos = s.indexOf( QLatin1String( ":Band" ) );
    if ( pos > 0 )
      sublayers << s.mid( pos );
    else
      sublayers << s;
  }
  qDebug() << "sublayers: " << sublayers.join( QLatin1Char( ',' ) );
  mReport += QStringLiteral( "sublayers:<br>%1<br>\n" ).arg( sublayers.join( QLatin1String( "<br>" ) ) );
  mReport += QStringLiteral( "expected:<br>%1<br>\n" ).arg( expected.join( QLatin1String( "<br>" ) ) );
  QCOMPARE( sublayers, expected );
  mReport += QLatin1String( "<p>Passed</p>" );
}

void TestQgsRasterSubLayer::checkStats()
{
  if ( !mHasNetCDF )
    return;

  mReport += QLatin1String( "<h2>Check Stats</h2>\n" );
  QString sublayerUri = mpRasterLayer->subLayers().value( 0 );
  mReport += "sublayer: " + sublayerUri + "<br>\n";

  sublayerUri = sublayerUri.split( QgsDataProvider::sublayerSeparator() )[0];
  QgsRasterLayer *sublayer = new QgsRasterLayer( sublayerUri, QStringLiteral( "Sublayer 1" ) );

  const QgsRasterBandStats myStatistics = sublayer->dataProvider()->bandStatistics( 1,
                                          QgsRasterBandStats::Min | QgsRasterBandStats::Max );
  const int width = 200;
  const int height = 200;
  const double min = 122;
  const double max = 130;
  mReport += QStringLiteral( "width = %1 expected = %2<br>\n" ).arg( sublayer->width() ).arg( width );
  mReport += QStringLiteral( "height = %1 expected = %2<br>\n" ).arg( sublayer->height() ).arg( height );
  mReport += QStringLiteral( "min = %1 expected = %2<br>\n" ).arg( myStatistics.minimumValue ).arg( min );
  mReport += QStringLiteral( "max = %1 expected = %2<br>\n" ).arg( myStatistics.maximumValue ).arg( max );

  QVERIFY( sublayer->width() == width );
  QVERIFY( sublayer->height() == height );
  QGSCOMPARENEAR( myStatistics.minimumValue, min, 4 * std::numeric_limits<double>::epsilon() );
  QGSCOMPARENEAR( myStatistics.maximumValue, max, 4 * std::numeric_limits<double>::epsilon() );
  mReport += QLatin1String( "<p>Passed</p>" );
  delete sublayer;
}


QGSTEST_MAIN( TestQgsRasterSubLayer )
#include "testqgsrastersublayer.moc"
